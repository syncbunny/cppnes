#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "cpu.h"

const uint16_t NMI_VECTOR = 0xFFFA;
const uint16_t RESET_VECTOR = 0xFFFC;
const uint16_t BRK_VECTOR = 0xFFFE;

#define FLG_C (0x01)
#define FLG_Z (0x02)
#define FLG_I (0x04)
#define FLG_N (0x80)
#define IFLG_C (0xFE)
#define IFLG_I (0xFD)
#define IFLG_N (0x7F)
#define IFLG_NZ (0x7D)

#define SET_C() (mP != FLG_C)
#define SET_I() (mP |= FLG_I)
#define SET_Z() (mP |= FLG_Z)
#define SET_N() (mP |= FLG_N)
#define UNSET_C() (mP &= IFLG_C)
#define UNSET_Z() (mP &= IFLG_I)
#define UNSET_N() (mP &= IFLG_N)
#define UNSET_NZ() (mP &= IFLG_NZ)

#define UPDATE_Z(x) ( ((x)==0)? SET_Z():UNSET_Z() )
#define UPDATE_N(x) ( ((x)&0x80)? SET_Z():UNSET_Z() )
#define UPDATE_NZ(x) ( ((x)&0x80)? SET_N():((x)==0)? SET_Z():UNSET_NZ() )

#define PUSH(x) (mMapper->write1Byte(0x0100+(mS--), (x)))
#define PUSH2(x) (mMapper->push2Bytes(0x0100+mS, (x)), mS -= 2)
#define POP2() (mS += 2, mMapper->pop2Bytes(0x0100+mS-2))

#define LDA(x) ((mA = (x)), UPDATE_NZ(mA))
#define LDX(x) ((mX = (x)), UPDATE_NZ(mX))
#define LDY(x) ((mY = (x)), UPDATE_NZ(mY))
#define STA(x) (mMapper->write1Byte((x), mA))
#define PHA() (PUSH(mA))
#define INC(x) (_addr = (x), mMapper->write1Byte(_addr, _mem = (mMapper->read1Byte(_addr)+1)), UPDATE_NZ(_mem))
#define DEX() (UPDATE_NZ(--mX))
#define DEY() (UPDATE_NZ(--mY))
#define ORA(x) (mA |= (x), UPDATE_NZ(mA))
#define ASL_A() ((mA&0x80)? SET_C():UNSET_C(), UPDATE_NZ(mA<<=2))
#define JMP(x) (mPC = (x))
#define JSR(x) (PUSH2(mPC+1), mPC=(x))
#define BPL(x) (mPC = ((mP&FLG_N)==0)? mPC+1:mPC+1+(signed char)(x))
#define BNE(x) (mPC = ((mP&FLG_Z)==0)? mPC+1:mPC+1+(signed char)(x))
#define BMI(x) (mPC = ((mP&FLG_N)==0)? mPC+1:mPC+1+(signed char)(x))
#define RTS() (mPC = POP2(), mPC+=1)
#define SEI() (SET_I())
#define TAX() (mX = mA, UPDATE_NZ(mX))
#define TAY() (mY = mA, UPDATE_NZ(mY))

#define IMM(x) (mPC=mPC+1, mMapper->read1Byte(mPC-1))
#define ABS(x) (mPC=mPC+2, mMapper->read2Bytes(mPC-2))
#define REL(x) (mMapper->read1Byte(mPC))
#define ZERO_PAGE(x) (mPC=mPC+1, mMapper->read1Byte(mPC-1))
#define ZERO_PAGE_IND(x,y) (mPC=mPC+1, mMapper->read1Byte(mPC-1)+(y))
#define INDIRECT(x) (mMapper->read2Bytes((x)))
#define IND_Y(x) ((x)+(uint16_t)(mY))

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

	// temporary variables
	uint8_t _mem;
	uint8_t _addr;

	// read opcode and exam it.
	uint8_t o = mMapper->read1Byte(mPC++);
	switch(o) {
	// at this point, mPC is first byte of operand.
	case 0x09: // ORA #XX
		ORA(IMM(mPC));
		break;
	case 0x0A: // ASL A
		ASL_A();
		break;
	case 0x10: // BPL $XX
		BPL(REL(mPC));
	 	break;
	case 0x20: // JSR $XXXX
		JSR(ABS(mPC));
		break;
	case 0x30: // BMI $XX
		BMI(REL(mPC));
	 	break;
	case 0x48: // PHA
		PHA();
		break;
	case 0x4C: // JMP $XXXX
		JMP(ABS());
		break;
	case 0x60: // RTS
		RTS();
		break;
	case 0x85: // STA $ZZ
		STA(ZERO_PAGE(mPC));
		break;
	case 0xAD: // LDA $XXXX
		LDA(ABS(mPC));
		break;
	case 0x78: //SEI
		SEI();
		break;
	case 0x88: // DEY
		DEY();
		break;
	case 0x8D: // STA $XXXX
		STA(ABS(mPC));
		break;
	case 0x91: // STA ($NN), Y
		STA(IND_Y(INDIRECT(ZERO_PAGE(mPC))));
		break;
	case 0xA0: // LDY #$XX
		LDY(IMM(mPC));
		break;
	case 0xA2: // LDX #$XX
		LDX(IMM(mPC));
		break;
	case 0xA8: // TAX
		TAY();
		break;
	case 0xA9: // LDA #XX
		LDA(IMM(mPC));
		break;
	case 0xAA: // TAX
		TAX();
		break;
	case 0xB5: // LDA(ZeroPage, X)
		LDA(ZERO_PAGE_IND(mPC, mX));
		break;
	case 0xCA: // DEX
		DEX();
		break;
	case 0xD0: // BNE $XX
		BNE(REL(mPC));
		break;
	case 0xE6: // INC $00XX
		INC(ZERO_PAGE(mPC));
		break;
	default:
		dump();
		char msg[1024];
		sprintf(msg, "opcode: %02X is unsupported", o);
		throw std::runtime_error(msg);
		break;
	}
	mClockRemain = clockTable[o];
	this->dump();
}

void CPU::dump() {
	printf("CPU:PC=%04X, A=%02X, X=%02X, Y=%02X, S=%02X, P=%02X\n", mPC, mA, mX, mY, mS, mP);
}

void CPU::doReset() {
	mPC = mMapper->read2Bytes(RESET_VECTOR);
	SET_I();
	mResetFlag = false;
	mClockRemain = 6;
}
