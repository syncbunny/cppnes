#include <cstring>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "core.h"

#define WRAM_SIZE (0x0800)

Core::Core() {
}

Core::~Core() {
}

void Core::setCPU(struct CPUCore& cpu) {
	memcpy(&mCPUCore, &cpu, sizeof(struct CPUCore));
}

void Core::setPPU(struct PPUCore& ppu) {
	memcpy(&mPPUCore, &ppu, sizeof(struct PPUCore));
}

void Core::setWRAM(uint8_t* WRAM) {
	memcpy(mWRAM, WRAM, WRAM_SIZE);
}

const struct CPUCore Core::getCPU() const {
	return mCPUCore;
}

const struct PPUCore Core::getPPU() const {
	return mPPUCore;
}

const uint8_t* Core::getWRAM() const {
	return mWRAM;
}

void Core::dump(const char* path) {
	FILE* f = fopen(path, "w");
	if (f == 0) {
		fprintf(stderr, "fopen faild(%d)\n",errno);
		return;
	}
	
	fwrite(&mCPUCore, sizeof(struct CPUCore), 1, f);
	fwrite(&mPPUCore, sizeof(struct PPUCore), 1, f);
	fwrite(mWRAM, WRAM_SIZE, 1, f);

	fclose(f);
}

Core* Core::load(const char* path) {
	FILE* f = fopen(path, "r");
	if (f == 0) {
		fprintf(stderr, "fopen faild(%d)\n",errno);
		return 0;
	}

	Core* core = new Core();
	int n;
	n = fread(&(core->mCPUCore), sizeof(struct CPUCore), 1, f);
	if (n != 1) {
		fprintf(stderr, "invalid core file.\n");
		goto BAIL;
	}
	n = fread(&(core->mPPUCore), sizeof(struct PPUCore), 1, f);
	if (n != 1) {
		fprintf(stderr, "invalid core file.\n");
		goto BAIL;
	}
	n = fread(&(core->mWRAM), WRAM_SIZE, 1, f);
	if (n != 1) {
		fprintf(stderr, "invalid core file.\n");
		goto BAIL;
	}

	fclose(f);
	return core;

BAIL:
	if (core) {
		delete core;
	}
	fclose(f);
	return 0;
}
