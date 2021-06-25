#include <chrono>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "nes.h"
#include "config.h"
#include "cpu.h"
#include "ppu.h"
#include "vppu.h"
#include "apu.h"
#include "openalapu.h"
#include "pad.h"
#include "vpad.h"
#include "mapper.h"
#include "vmapper.h"
#include "events.h"
#include "renderer.h"
#include "core.h"
#include "logger.h"

#define FLAG6_V_MIRROR           (0x01)
#define FLAG6_HAS_BATTARY_BACKUP (0x02)
#define FLAG6_HAS_TRAINER        (0x04)
#define FLAG6_HAS_OWN_MIRROR     (0x80)
#define FLAG6_MAPPAER_LOW        (0xF0)

#define FLAG7_VS_UNISYSTEM (0x01)
#define FLAG7_PLAYCHOICE10 (0x02)
#define FLAG7_NES_2_0      (0x0C)
#define FLAG7_MAPPER_HIGH  (0xF0)

#define FLAG9_TV_SYSTEM (0x01) // 0: NTSC, 1: PAL

NES::NES(Renderer* r) {
	Config* conf = Config::getInstance();

	if (conf->getVarbose()) {
		mMapper = new VMapper();
		mPPU = new VPPU();
	}
	else {
		mMapper = new Mapper();
		mPPU = new PPU();
	}
	mCPU = new CPU(mMapper);
	OpenALAPU* openalAPU = new OpenALAPU();
	mAPU = openalAPU;
	mPAD = new PAD();

	mDClockCPU = 0;
	mDClockPPU = 0;
	mDClockAPU = 0;
	mCartridgeMem = 0;
	mClocks = 0;

	if (r) {
		mPPU->bindRenderer(r);
	}
	mWRAM = new uint8_t[0x0800];
	for (int i = 0; i < 0x0800; i++) mWRAM[i] = 0x4F;
	mERAM = new uint8_t[0x2000];
	for (int i = 0; i < 0x2000; i++) mERAM[i] = '\0';
	mMapper->setWRAM(mWRAM);
	mMapper->setERAM(mERAM);
	mMapper->setPPU(mPPU);
	mMapper->setAPU(mAPU);
	mMapper->setPAD(mPAD);

	mFrameWorkers.push_back(openalAPU);
	if (conf->getProfileEnabled()) {
		mProfiler = new Profiler();
		mFrameWorkers.push_back(mProfiler);
	} else {
		mProfiler = 0;
	}
}

NES::~NES() {
	if (mWRAM) {
		delete[] mWRAM;
	}
	if (mERAM) {
		delete[] mERAM;
	}
	delete mCPU;
	delete mPPU;
	delete mMapper;
}

void NES::powerOn() {
	mCPU->powerOn();
}

bool NES::loadCartridge(const char* path) {
	struct stat st;
	if (stat(path, &st) != 0) {
		fprintf(stderr, "stat(%s) faild(%d)\n", path, errno);
		return false;
	}

	int fd = open(path, 0);
	if (fd == -1) {
		fprintf(stderr, "open faild(%d)\n",errno);
		return false;
	}

	mCartridgeMem = (unsigned char*)mmap(0, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (mCartridgeMem == MAP_FAILED) {
		fprintf(stderr, "mmap() faild(%d)\n",errno);
		return false;
	}

	auto checkHeader = [](void* mem) -> bool {
		uint8_t* p = (uint8_t*)mem;
		if (p[0] != 0x4E || p[1] != 0x45 || p[2] != 0x53 || p[3] != 0x1A) {
			return false;
		} else {
			return true;
		}
	};
	if (!checkHeader(mCartridgeMem)) {
		close(fd);
		fprintf(stderr, "cartridge header is invalid.");
		return false;
	}

	uint8_t pROMSize; // in 16k
	uint8_t cROMSize; // in 8k
	pROMSize = mCartridgeMem[4];
	cROMSize = mCartridgeMem[5];

	int offset = 16;
	if (this->cartridgeHasTrainer()) {
		offset += 512;
	}
	Logger::getInstance()->log(Logger::DEBUG, "mapper:%d\n", this->getMapperNo());
	mMapper->setPROM(&mCartridgeMem[offset], pROMSize*16*1024);
	mMapper->setCROM(&mCartridgeMem[offset+pROMSize*16*1024], cROMSize*8*1024);
	mMapper->setNo(this->getMapperNo());

	uint8_t flag6 = mCartridgeMem[6];
	mPPU->setMirror((flag6&1)? PPU::MIRROR_V:PPU::MIRROR_H);

	uint8_t flag9 = mCartridgeMem[9];
	if (flag9 & FLAG9_TV_SYSTEM) {
		printf("TV=PAL\n");
	} else {
		printf("TV=NTSC\n");
	}

	return true;
}

void NES::reset() {
	mCPU->reset();
}

void NES::clock() {
	//       Master          CPU      PPU    APU
	// NTSC: 21477272.72 Hz  Base/12  Base/4 Base/(12*7457)

	Config* conf = Config::getInstance();
	Logger* logger = Logger::getInstance();
	Event* evt = EventQueue::getInstance().pop();
	if (evt) {
		switch(evt->getType()) {
		case Event::TYPE_NMI:
			logger->log(Logger::DEBUG, "NES::clock: NMI!\n");
			mCPU->nmi();
			break;
		case Event::TYPE_IRQ:
			logger->log(Logger::DEBUG, "NES::clock: IRQ!\n");
			mCPU->irq();
			break;
		case Event::TYPE_DMA:
			mDClockCPU = 514*12; // Stop CPU 514 clocks
			break;
		case Event::TYPE_CAPTURE:
			mPPU->capture();
			break;
		case Event::TYPE_COREDUMP:
		case Event::TYPE_KILL:
			Core core;
			this->coreDump(&core);
			char fname[32];
			sprintf(fname, "nes_%010d.dump", mClocks);
			core.dump(fname);
			if (evt->getType() == Event::TYPE_KILL) {
				throw std::runtime_error("KILLED");
			}
			break;
		}
		delete evt;
	}

	bool profile = conf->getProfileEnabled();
	if (mDClockAPU == 0) {
		if (profile) { mProfiler->apuStart(); }
		mAPU->clock();
		if (profile) { mProfiler->apuEnd(); }
		mDClockAPU = 11;
	} else {
		mDClockAPU--;
	}

	if (mDClockPPU == 0) {
		if (mPPU->isFrameStart()) {
			this->frameStart();
		}
		if (profile) { mProfiler->ppuStart(); }
		mPPU->clock();
		if (profile) { mProfiler->ppuEnd(); }
		mDClockPPU = 3;
	} else {
		mDClockPPU--;
	}

	if (mDClockCPU == 0) {
		if (profile) { mProfiler->cpuStart(); }
		mCPU->clock();
		if (profile) { mProfiler->cpuEnd(); }
		mDClockCPU = 11;
	} else {
		mDClockCPU--;
	}

	mClocks++;
}

bool NES::cartridgeHasTrainer() {
	if (mCartridgeMem[6] & FLAG6_HAS_TRAINER) {
		return true;
	} else {
		return false;
	}
}

uint8_t NES::getMapperNo() {
	uint8_t n = 0;

	n = mCartridgeMem[7] & FLAG7_MAPPER_HIGH;
	n |= (mCartridgeMem[6] & FLAG6_MAPPAER_LOW) >> 4;

	return n;
}

void NES::dump6000() {
	uint16_t addr = 0x6004;
	static int sCheckState = 0;

	printf("0x6000=0x%02x\n", mERAM[0]);
	printf("0x6001-0x6003=%02x %02x %02x\n", mERAM[1], mERAM[2], mERAM[3]);
	putchar('[');
	for (addr = 0x6004; addr < 0x8000; addr++) {
		if (mERAM[addr - 0x6000] == '\0') {
			break;
		}
		putchar(mERAM[addr - 0x6000]);
	}
	putchar(']');
	putchar('\n');
#if 0
	if (sCheckState == 0) {
		if (mERAM[0] == 0x80) {
			sCheckState = 1;
		}
	} else if (sCheckState == 1) {
		if (mERAM[0] != 0x80 && mERAM[0] != 0x81) {
			throw std::runtime_error("error detected.");
		}
	}
#endif
}

void NES::coreDump(Core* core) const {
	mCPU->coreDump(core);
	mPPU->coreDump(core);
	core->setWRAM(mWRAM);
}

void NES::loadCore(Core* core) {
	memcpy(mWRAM, core->getWRAM(), 0x0800);
	mCPU->loadCore(core);
	mPPU->loadCore(core);
}

void NES::frameStart() {
	std::list<FrameWorker*>::iterator it;
	for (it = mFrameWorkers.begin(); it != mFrameWorkers.end(); ++it) {
		FrameWorker* fw = *it;
		fw->atFrameStart();
	}
}
