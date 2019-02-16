#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "cpu.h"

const uint16_t NMI_VECTOR = 0xFFFA;
const uint16_t RESET_VECTOR = 0xFFFC;
const uint16_t BRK_VECTOR = 0xFFFE;

#define FLG_Z (0x02)
#define FLG_I (0x04)
#define FLG_N (0x80)
#define IFLG_I (0xFD)
#define IFLG_N (0x7F)
#define IFLG_NZ (0x7D)

#define SET_I() (mP |= FLG_I)
#define SET_Z() (mP |= FLG_Z)
#define SET_N() (mP |= FLG_N)
#define UNSET_Z() (mP &= IFLG_I)
#define UNSET_N() (mP &= IFLG_N)
#define UNSET_NZ() (mP &= IFLG_NZ)

#define UPDATE_Z(x) ( ((x)==0)? SET_Z():UNSET_Z() )
#define UPDATE_N(x) ( ((x)&0x80)? SET_Z():UNSET_Z() )
#define UPDATE_NZ(x) ( ((x)&0x80)? SET_N():((x)==0)? SET_Z():UNSET_NZ() )

#define PUSH2(x) (mMapper->write2Bytes(0x0100+mS, (x)), mS -= 2)

#define LDA(x) ((mA = (x)), UPDATE_NZ(mA))
#define STA(x) (mMapper->write1Byte((x), mA))
#define JSR(x) (PUSH2(mPC+1), mPC=(x))
#define BPL(x) (mPC = ((mP&FLG_N)==0)? mPC+1:mPC+1+(signed char)(x))

#define IMM(x) (mMapper->read1Byte(x))
#define ABS(x) (mPC=mPC+2, mMapper->read2Bytes(mPC-2))
#define REL(x) (mMapper->read1Byte(mPC))

int clockTable[] = {
	/* xx    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F */
	/* 00 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 10 */  6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 20 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 30 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 40 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 50 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 60 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 70 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 80 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* 90 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* A0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 
	/* B0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* C0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* D0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* E0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	/* F0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

CPU::CPU(Mapper* mapper)
:mMapper(mapper){
	mClockRemain = 0;
	mResetFlag = false;
}

CPU::~CPU() {
}

void CPU::powerOn() {
	mA = mX = mY = 0;
	mP = 0x34;
	mS = 0xFD;
}

void CPU::reset() {
	mResetFlag = true;
	mClockRemain = 6;
}

void CPU::clock() {
	if (mResetFlag) {
		doReset();
		return;
	}
	
	if (mClockRemain > 0) {
		mClockRemain--;
		return;
	}

	uint8_t o = mMapper->read1Byte(mPC++);
	switch(o) {
	case 0x10: // BPL $XX
		BPL(REL(mPC));
	 	break;
	//case 0x60: // RTS
	//	RTS();
	//	break;
	case 0xAD: // LDA $XXXX
		LDA(ABS(mPC));
		break;
	case 0x20: // JSR $XXXX
		JSR(ABS(mPC));
		break;
	case 0x8D: // STA $XXXX
		STA(ABS(mPC));
		break;
	case 0xA9: // LDA #XX
		LDA(IMM(mPC++));
		break;
	default:
		dump();
		char msg[1024];
		sprintf(msg, "opcode: %02X is unsupported", o);
		throw std::runtime_error(msg);
		break;
	}
	mClockRemain = clockTable[o];
}

void CPU::dump() {
	printf("CPU:PC=%04X, A=%02X, X=%02X, Y=%02X, S=%02X, P=%02X\n", mPC, mA, mX, mY, mS, mP);
}

void CPU::updateP(uint8_t a) {
}

void CPU::doReset() {
	mPC = mMapper->read2Bytes(RESET_VECTOR);
	SET_I();
	mResetFlag = false;
	mClockRemain = 6;
}
