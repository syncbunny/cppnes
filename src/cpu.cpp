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
#define FLG_B (0x10)
#define FLG_V (0x40)
#define FLG_N (0x80)
#define IFLG_C (0xFE)
#define IFLG_Z (0xFD)
#define IFLG_I (0xFB)
#define IFLG_B (0xEF)
#define IFLG_V (0xBF)
#define IFLG_N (0x7F)
#define IFLG_NZ (0x7D)
#define IFLG_VC (0xBE)

#define SET_C() (mP |= FLG_C)
#define SET_I() (mP |= FLG_I)
#define SET_Z() (mP |= FLG_Z)
#define SET_B() (mP |= FLG_B)
#define SET_N() (mP |= FLG_N)
#define UNSET_C() (mP &= IFLG_C)
#define UNSET_I() (mP &= IFLG_I)
#define UNSET_Z() (mP &= IFLG_Z)
#define CLEAR_B() (mP &= IFLG_B)
#define UNSET_N() (mP &= IFLG_N)
#define UNSET_NZ() (mP &= IFLG_NZ)

#define UPDATE_Z(x)  ( ((x)==0)? SET_Z():UNSET_Z() )
#define UPDATE_N(x)  ( (((x)&0x80) != 0)? SET_N():UNSET_N() )
#define UPDATE_NZ(x) (UPDATE_N(x), UPDATE_Z(x) )

#define PUSH(x) (mMapper->write1Byte(0x0100+(mS--), (x)))
#define PUSH2(x) (mMapper->push2Bytes(0x0100+mS, (x)), mS -= 2)
#define POP() (mS+=1, mMapper->read1Byte(0x0100+mS))
#define POP2() (mS += 2, mMapper->pop2Bytes(0x0100+mS-2))

#define LDA(addr) (mA = mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define LDX(addr) (mX = mMapper->read1Byte(addr), UPDATE_NZ(mX))
#define LDY(addr) (mY = mMapper->read1Byte(addr), UPDATE_NZ(mY))
#define STA(addr) (mMapper->write1Byte(addr, mA))
#define INC(addr) (_zpaddr = addr, mMapper->write1Byte(_zpaddr, _mem = (mMapper->read1Byte(_zpaddr)+1)), UPDATE_NZ(_mem))
#define AND(addr) (mA &= mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define ORA(addr) (mA |= mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define ADC(addr) (_a = mA, _mem = mMapper->read1Byte(addr), mA+=_mem, mA+=(mP&FLG_C)? 1:0, mP&=IFLG_VC, mP|=mADC_VCTable[_a][_mem], UPDATE_NZ(mA))
#define SBC(addr) (_a = mA, _mem = mMapper->read1Byte(addr), mA-=_mem, mA-=(mP&FLG_C)? 0:1, mP&=IFLG_VC, mP|=mSBC_VCTable[_a][_mem], UPDATE_NZ(mA))
#define CMP(addr) (_a = mA, _mem = mMapper->read1Byte(addr), mP = (_a>=_mem)? SET_C():UNSET_C(),  _a-=_mem, UPDATE_NZ(_a))
#define JMP(addr) (mPC = addr)
#define JSR(addr) (PUSH2(mPC+1), mPC=addr)
#define BPL(addr) (mPC = ((mP&FLG_N)==0)? addr:mPC+1)
#define BNE(addr) (mPC = ((mP&FLG_Z)==0)? addr:mPC+1)
#define BEQ(addr) (mPC = ((mP&FLG_Z)!=0)? addr:mPC+1)
#define BMI(addr) (mPC = ((mP&FLG_N)==0)? mPC+1:addr)
#define BCS(addr) (mPC = ((mP&FLG_C)==0)? mPC+1:addr)
#define PHA() (PUSH(mA))
#define PLA() (mA=POP(), UPDATE_NZ(mA))
#define INY() (mY++, UPDATE_NZ(mY))
#define DEX() (mX--, UPDATE_NZ(mX))
#define DEY() (mY--, UPDATE_NZ(mY))
#define ASL_A() ((mA&0x80)? SET_C():UNSET_C(), mA<<=1,UPDATE_NZ(mA))
#define LSR_A() ((mA&0x01)? SET_C():UNSET_C(), mA>>=1,UPDATE_NZ(mA))
#define ROL_A() (_a=mA, mA<<=1, mA|=(mP&FLG_C)? 0x01:0x00, (_a&0x80)? SET_C():UNSET_C(), UPDATE_NZ(mA))
#define ROR(addr) (_addr=addr,_mem=mMapper->read1Byte(_addr), _mem2=_mem, _mem>>=1, _mem|=(mP&FLG_C)? 0x80:0x00, mMapper->write1Byte(_addr, _mem), (_mem2&0x01)? SET_C():UNSET_C(), UPDATE_NZ(_mem))
#define RTS() (mPC = POP2(), mPC+=1)
#define SEC() (SET_C())
#define SEI() (SET_I())
#define CLC() (UNSET_C())
#define CLI() (UNSET_I())
#define TYA() (mA = mY, UPDATE_NZ(mA))
#define TAX() (mX = mA, UPDATE_NZ(mX))
#define TAY() (mY = mA, UPDATE_NZ(mY))

#define IMM() (mPC++)
#define ABS() (mPC=mPC+2, mMapper->read2Bytes(mPC-2))
#define ABS_IND(ind) (mPC+=2,mMapper->read2Bytes(mPC-2)+ind)
#define ABS_IND_Y() (mPC=mPC+2, mMapper->read1Byte(mMapper->read2Bytes(mPC-2)+(uint16_t)mY))
#define REL() (_mem=mMapper->read1Byte(mPC), mPC+1+(int8_t)_mem)
#define ZERO_PAGE(x) (mPC=mPC+1, mMapper->read1Byte(mPC-1))
#define ZERO_PAGE_IND(x) (mPC=mPC+1, _zpaddr = mMapper->read1Byte(mPC-1), _zpaddr+=(x))
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
	mNMIFlag = false;

	buildADC_VCTable();
	buildSBC_VCTable();
}

CPU::~CPU() {
}

void CPU::powerOn() {
	mA = mX = mY = 0;
	mP = 0x34;
	mS = 0xFD;
}

void CPU::nmi() {
	mNMIFlag = true;
	mClockRemain = 6;
}

void CPU::reset() {
	mResetFlag = true;
	mClockRemain = 6;
}

void CPU::clock() {
	if (mNMIFlag) {
		doNMI();
		return;
	}
	if (mResetFlag) {
		doReset();
		return;
	}
	
	if (mClockRemain > 0) {
		mClockRemain--;
		return;
	}

	// temporary variables
	uint8_t _a;
	uint8_t _mem;
	uint8_t _mem2;
	uint8_t _zpaddr;
	uint16_t _addr;

	// read opcode and exam it.
	uint8_t o = mMapper->read1Byte(mPC++);
	switch(o) {
	// at this point, mPC is first byte of operand.
	case 0x05: // ORA (ZeroPage)
		ORA(ZERO_PAGE(mPC));
		break;
	case 0x09: // ORA #XX
		ORA(IMM());
		break;
	case 0x0A: // ASL A
		ASL_A();
		break;
	case 0x10: // BPL $XX
		BPL(REL());
		break;
	case 0x18: // CLC
		CLC();
		break;
	case 0x20: // JSR $XXXX
		JSR(ABS());
		break;
	case 0x29: // AND (Imm)
		AND(IMM());
		break;
	case 0x2A: // ROL_A
		ROL_A();
		break;
	case 0x30: // BMI $XX
		BMI(REL());
	 	break;
	case 0x38: // SEC
		SEC();
		break;
	case 0x48: // PHA
		PHA();
		break;
	case 0x4A: // LSR A
		LSR_A();
		break;
	case 0x4C: // JMP $XXXX
		JMP(ABS());
		break;
	case 0x58: // CLI
		CLI();
		break;
	case 0x60: // RTS
		RTS();
		break;
	case 0x66: // ROR (ZeroPage)
		ROR(ZERO_PAGE(mPC));
		break;
	case 0x68: // PLA
		PLA();
		break;
	case 0x69: // ADC (Imm)
		ADC(IMM());
		break;
	case 0x85: // STA $ZZ
		STA(ZERO_PAGE(mPC));
		break;
	case 0xAD: // LDA $XXXX
		LDA(ABS());
		break;
	case 0x78: //SEI
		SEI();
		break;
	case 0x88: // DEY
		DEY();
		break;
	case 0x8D: // STA (ABS)
		STA(ABS());
		break;
	case 0x91: // STA ($NN), Y
		STA(IND_Y(INDIRECT(ZERO_PAGE(mPC))));
		break;
	case 0x95: // STA (ZEROPage, X)
		STA(ZERO_PAGE_IND(mX));
		break;
	case 0x98: // TYA
		TYA();
		break;
	case 0x99: // STA (Absolute X)
		STA(ABS_IND(mX));
		break;
	case 0xA0: // LDY #$XX
		LDY(IMM());
		break;
	case 0xA1: // LDA (Indirect, X)
		LDA(INDIRECT(ZERO_PAGE_IND(mX)));
		break;
	case 0xA2: // LDX #$XX
		LDX(IMM());
		break;
	case 0xA5: // LDA (ZeroPage)
		LDA(ZERO_PAGE(mPC));
		break;
	case 0xA8: // TAY
		TAY();
		break;
	case 0xA9: // LDA #XX
		LDA(IMM());
		break;
	case 0xAA: // TAX
		TAX();
		break;
	case 0xB0: // BCS(Rel)
		BCS(REL());
		break;
	case 0xB1: // LDA(Indirect Y), 
		LDA(IND_Y(INDIRECT(ZERO_PAGE(mPC))));
		break;
	case 0xB5: // LDA(ZeroPage, X)
		LDA(ZERO_PAGE_IND( mX));
		break;
	case 0xB9: // LDA(Absolute, Y)
		LDA(ABS_IND(mY));
		break;
	case 0xC8: // INY
		INY();
		break;
	case 0xC9: // CMP (Imm)
		CMP(IMM());
		break;
	case 0xCA: // DEX
		DEX();
		break;
	case 0xD0: // BNE $XX
		BNE(REL());
		break;
	case 0xE6: // INC $00XX
		INC(ZERO_PAGE(mPC));
		break;
	case 0xEA: // NOP
		break;
	case 0xF0: // BEQ (Rel)
		BEQ(REL());
		break;
	case 0xFD: // SBC (ABS_IND(X))
		SBC(ABS_IND(mX));
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

void CPU::doNMI() {
	mPC = mMapper->read2Bytes(NMI_VECTOR);
	SET_I();
	CLEAR_B();
	mNMIFlag = false;
	mClockRemain = 6;
}

void CPU::buildADC_VCTable() {
	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			mADC_VCTable[a][b] = 0;
			if (a+b >= 256) {
				mADC_VCTable[a][b] |= FLG_C;
			}
			uint8_t aa = a, bb = b;
			uint8_t cc = a+b;

			if ((aa <= 0x7F && cc >= 0x80) || (aa >= 0x80 && cc <= 0x7F)) {
				mADC_VCTable[a][b] |= FLG_V;
			}
		}
	}
}

void CPU::buildSBC_VCTable() {
	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			mSBC_VCTable[a][b] = 0;
			if (a >= b) {
				mSBC_VCTable[a][b] |= FLG_C;
			}
			uint8_t aa = a, bb = b;
			uint8_t cc = a-b;

			if ((aa <= 0x7F && cc >= 0x80) || (aa >= 0x80 && cc <= 0x7F)) {
				mSBC_VCTable[a][b] |= FLG_V;
			}
		}
	}
}
