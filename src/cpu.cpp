#include <cstdio>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "cpu.h"
#include "core.h"
#include "config.h"

const uint16_t NMI_VECTOR = 0xFFFA;
const uint16_t RESET_VECTOR = 0xFFFC;
const uint16_t BRK_VECTOR = 0xFFFE;

#define FLG_C (0x01)
#define FLG_Z (0x02)
#define FLG_I (0x04)
#define FLG_D (0x08)
#define FLG_B (0x10)
#define FLG_5 (0x20)
#define FLG_V (0x40)
#define FLG_N (0x80)
#define IFLG_C (0xFE)
#define IFLG_Z (0xFD)
#define IFLG_I (0xFB)
#define IFLG_D (0xF7)
#define IFLG_B (0xEF)
#define IFLG_V (0xBF)
#define IFLG_N (0x7F)
#define IFLG_NZ (0x7D)
#define IFLG_VC (0xBE)

#define SET_C() (mP |= FLG_C)
#define SET_Z() (mP |= FLG_Z)
#define SET_I() (mP |= FLG_I)
#define SET_D() (mP |= FLG_D)
#define SET_B() (mP |= FLG_B)
#define SET_V() (mP |= FLG_V)
#define SET_N() (mP |= FLG_N)
#define UNSET_C() (mP &= IFLG_C)
#define UNSET_Z() (mP &= IFLG_Z)
#define UNSET_I() (mP &= IFLG_I)
#define UNSET_D() (mP &= IFLG_D)
#define CLEAR_B() (mP &= IFLG_B)
#define UNSET_V() (mP &= IFLG_V)
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
#define STX(addr) (mMapper->write1Byte(addr, mX))
#define STY(addr) (mMapper->write1Byte(addr, mY))
#define INC(addr) (_addr = addr, _mem=mMapper->read1Byte(_addr), _mem+=1, mMapper->write1Byte(_addr, _mem), UPDATE_NZ(_mem))
#define DEC(addr) (_addr = addr, _mem=mMapper->read1Byte(_addr), _mem-=1, mMapper->write1Byte(_addr, _mem), UPDATE_NZ(_mem))
#define AND(addr) (mA &= mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define ORA(addr) (mA |= mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define EOR(addr) (mA ^= mMapper->read1Byte(addr), UPDATE_NZ(mA))
#define ADC(addr) (_mem = mMapper->read1Byte(addr), _c = (mP&FLG_C)? 1:0, _A_Pvc=mADC_APvcTable[((uint16_t)mA*256 + _mem)*2 + _c], mA=_A_Pvc.a, mP=(mP&IFLG_VC)|(_A_Pvc.p_vc), UPDATE_NZ(mA))
#define SBC(addr) (_mem = mMapper->read1Byte(addr), _c = (mP&FLG_C)? 1:0, _A_Pvc=mSBC_APvcTable[((uint16_t)mA*256 + _mem)*2 + _c], mA=_A_Pvc.a, mP=(mP&IFLG_VC)|(_A_Pvc.p_vc), UPDATE_NZ(mA))
#define CMP(addr) (_a = mA, _mem = mMapper->read1Byte(addr), mP = (_a>=_mem)? SET_C():UNSET_C(),  _a-=_mem, UPDATE_NZ(_a))
#define CPX(addr) (_x = mX, _mem = mMapper->read1Byte(addr), mP = (_x>=_mem)? SET_C():UNSET_C(),  _x-=_mem, UPDATE_NZ(_x))
#define CPY(addr) (_y = mY, _mem = mMapper->read1Byte(addr), mP = (_y>=_mem)? SET_C():UNSET_C(),  _y-=_mem, UPDATE_NZ(_y))
#define BIT(addr) (_mem = mMapper->read1Byte(addr), mP = (_mem&0xC0)|(mP&0x3F), _mem &= mA, UPDATE_Z(_mem))
#define JMP(addr) (mPC = addr)
#define JSR(addr) (PUSH2(mPC+1), mPC=addr)
#define BCS(addr) (mPC = ((mP&FLG_C)!=0)? addr:mPC+1)
#define BCC(addr) (mPC = ((mP&FLG_C)==0)? addr:mPC+1)
#define BEQ(addr) (mPC = ((mP&FLG_Z)!=0)? addr:mPC+1)
#define BNE(addr) (mPC = ((mP&FLG_Z)==0)? addr:mPC+1)
#define BVS(addr) (mPC = ((mP&FLG_V)!=0)? addr:mPC+1)
#define BVC(addr) (mPC = ((mP&FLG_V)==0)? addr:mPC+1)
#define BMI(addr) (mPC = ((mP&FLG_N)!=0)? addr:mPC+1)
#define BPL(addr) (mPC = ((mP&FLG_N)==0)? addr:mPC+1)
#define PHA() (PUSH(mA))
#define PLA() (mA=POP(), UPDATE_NZ(mA))
#define PHP() (SET_B(), PUSH(mP))
#define PLP() (mP=POP())
#define INX() (mX++, UPDATE_NZ(mX))
#define INY() (mY++, UPDATE_NZ(mY))
#define DEX() (mX--, UPDATE_NZ(mX))
#define DEY() (mY--, UPDATE_NZ(mY))
#define ASL_A() ((mA&0x80)? SET_C():UNSET_C(), mA<<=1,UPDATE_NZ(mA))
#define ASL(addr) (_addr = addr, _mem = mMapper->read1Byte(_addr), (_mem&0x80)? SET_C():UNSET_C(), _mem<<= 1, mMapper->write1Byte(_addr, _mem), UPDATE_NZ(_mem))
#define LSR_A() ((mA&0x01)? SET_C():UNSET_C(), mA>>=1,UPDATE_NZ(mA))
#define LSR(addr) (_addr = addr, _mem = mMapper->read1Byte(_addr), (_mem&0x01)? SET_C():UNSET_C(), _mem>>= 1, mMapper->write1Byte(_addr, _mem), UPDATE_NZ(_mem))
#define ROL_A() (_a=mA, mA<<=1, mA|=(mP&FLG_C)? 0x01:0x00, (_a&0x80)? SET_C():UNSET_C(), UPDATE_NZ(mA))
#define ROR_A() (_a=mA, mA>>=1, mA|=(mP&FLG_C)? 0x80:0x00, (_a&0x01)? SET_C():UNSET_C(), UPDATE_NZ(mA))
#define ROL(addr) (_addr=addr,_mem=mMapper->read1Byte(_addr), _mem2=_mem, _mem<<=1, _mem|=(mP&FLG_C)? 0x01:0x00, mMapper->write1Byte(_addr, _mem), (_mem2&0x80)? SET_C():UNSET_C(), UPDATE_NZ(_mem))
#define ROR(addr) (_addr=addr,_mem=mMapper->read1Byte(_addr), _mem2=_mem, _mem>>=1, _mem|=(mP&FLG_C)? 0x80:0x00, mMapper->write1Byte(_addr, _mem), (_mem2&0x01)? SET_C():UNSET_C(), UPDATE_NZ(_mem))
#define RTS() (mPC = POP2(), mPC+=1)
#define RTI() (mP = POP(), mPC = POP2())
#define SEC() (SET_C())
#define SEI() (SET_I())
#define SED() (SET_D())
#define CLC() (UNSET_C())
#define CLI() (UNSET_I())
#define CLD() (UNSET_D())
#define CLV() (UNSET_V())
#define TAX() (mX = mA, UPDATE_NZ(mX))
#define TXA() (mA = mX, UPDATE_NZ(mA))
#define TAY() (mY = mA, UPDATE_NZ(mY))
#define TYA() (mA = mY, UPDATE_NZ(mA))
#define TXS() (mS = mX) // No flag update
#define TSX() (mX = mS, UPDATE_NZ(mX))

#define IMM() (mPC++)
#define ABS() (mPC=mPC+2, mMapper->read2Bytes(mPC-2))
#define ABS_INDEXED(ind) (mPC+=2,mMapper->read2Bytes(mPC-2)+ind)
#define REL() (_mem=mMapper->read1Byte(mPC), mPC+1+(int8_t)_mem)
#define ZERO_PAGE(x) (mPC=mPC+1, mMapper->read1Byte(mPC-1))
#define ZERO_PAGE_INDEXED(x) (mPC=mPC+1, _zpaddr = mMapper->read1Byte(mPC-1), _zpaddr+=(x))
#define INDIRECT(x) (mMapper->read2Bytes((x)))
#define INDIRECT_X() (mMapper->indirect_x(mPC++, mX))
#define INDIRECT_Y() (mMapper->indirect_y(mPC++, mY))
#define JMP_INDIRECT(x) (mMapper->read2BytesSp((x)))

int clockTable[] = {
	/* xx    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F */
	/*  0 */  1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 0, 3, 3, 0,
	/* 10 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
	/* 20 */  3, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* 30 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
	/* 40 */  1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* 50 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
	/* 60 */  1, 2, 0, 0, 0, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* 70 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
	/* 80 */  0, 2, 0, 0, 2, 2, 2, 0, 1, 0, 1, 0, 3, 3, 3, 0,
	/* 90 */  2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 0, 3, 0, 0,
	/* a0 */  2, 2, 2, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* b0 */  2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
	/* c0 */  2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* d0 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
	/* e0 */  2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
	/* f0 */  2, 2, 0, 0, 0, 2, 2, 0, 1, 3, 0, 0, 0, 3, 3, 0,
};

CPU::CPU(Mapper* mapper)
:mMapper(mapper){
	mClockRemain = 0;
	mResetFlag = false;
	mNMIFlag = false;

	buildADC_APvcTable();
	buildSBC_APvcTable();
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

void CPU::irq() {
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
	if (mIRQFlag) {
		if ((mP&FLG_I) == 0) {
			doIRQ();
			return;
		}
		mIRQFlag = false;
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
	uint8_t _a, _x, _y;
	uint8_t _mem;
	uint8_t _mem2;
	uint8_t _zpaddr;
	uint8_t _c;
	uint16_t _addr;
	struct APvc _A_Pvc;

	// read opcode and exam it.
	uint8_t o = mMapper->read1Byte(mPC++);
	switch(o) {
	// at this point, mPC is first byte of operand.
	case 0x01: // ORA Indirect,X
		ORA(INDIRECT_X());
		break;
	case 0x05: // ORA ZeroPage
		ORA(ZERO_PAGE(mPC));
		break;
	case 0x06: // ASL ZeroPage
		ASL(ZERO_PAGE(mPC));
		break;
	case 0x08: // PHP Implied
		PHP();
		break;
	case 0x09: // ORA Immediate
		ORA(IMM());
		break;
	case 0x0A: // ASL Accumulator
		ASL_A();
		break;
	case 0x0D: // ORA Absolute
		ORA(ABS());
		break;
	case 0x0E: // ASL Absolute
		ASL(ABS());
		break;
	case 0x10: // BPL Relative
		BPL(REL());
		break;
	case 0x11: // ORA Indirect,Y
		ORA(INDIRECT_Y());
		break;
	case 0x15: // ORA ZeroPage,X
		ORA(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x16: // ASL ZeroPage,X
		ASL(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x18: // CLC Implied
		CLC();
		break;
	case 0x19: // ORA Absolute,Y
		ORA(ABS_INDEXED(mY));
		break;
	case 0x1D: // ORA Absolute,X
		ORA(ABS_INDEXED(mX));
		break;
	case 0x1E: // ASL Absolute,X
		ASL(ABS_INDEXED(mX));
		break;
	case 0x20: // JSR Absolute
		JSR(ABS());
		break;
	case 0x21: // AND Indirect,X
		AND(INDIRECT_X());
		break;
	case 0x24: // BIT ZeroPage
		BIT(ZERO_PAGE(mPC));
		break;
	case 0x25: // AND ZeroPage
		AND(ZERO_PAGE(mPC));
		break;
	case 0x26: // ROL ZeroPage 
		ROL(ZERO_PAGE(mPC));
		break;
	case 0x28: // PLP Implied
		PLP();
		break;
	case 0x29: // AND Immediate 
		AND(IMM());
		break;
	case 0x2A: // ROL Accumulator
		ROL_A();
		break;
	case 0x2C: // BIT Absolute
		BIT(ABS());
		break;
	case 0x2D: // AND Absolute
		AND(ABS());
		break;
	case 0x2E: // ROL Absolute
		ROL(ABS());
		break;
	case 0x30: // BMI Relative
		BMI(REL());
	 	break;
	case 0x31: // AND Indirect,Y
		AND(INDIRECT_Y());
		break;
	case 0x35: // AND ZeroPage,X
		AND(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x36: // ROL ZeroPage,X
		ROL(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x38: // SEC Implied
		SEC();
		break;
	case 0x39: // AND Absolute,Y
		AND(ABS_INDEXED(mY));
		break;
	case 0x3D: // AND Absolute,X
		AND(ABS_INDEXED(mX));
		break;
	case 0x3E: // ROL Absolute,X
		ROL(ABS_INDEXED(mX));
		break;
	case 0x40: // RTI Implied
		RTI();
		break;
	case 0x41: // EOR Indirect,X
		EOR(INDIRECT_X());
		break;
	case 0x45: // EOR ZeroPage
		EOR(ZERO_PAGE(mPC));
		break;
	case 0x46: // LSR ZeroPage
		LSR(ZERO_PAGE(mPC));
		break;
	case 0x48: // PHA Implied
		PHA();
		break;
	case 0x49: // EOR Immediate
		EOR(IMM());
		break;
	case 0x4A: // LSR Accumulator
		LSR_A();
		break;
	case 0x4C: // JMP Absolute
		JMP(ABS());
		break;
	case 0x4D: // EOR Absolute
		EOR(ABS());
		break;
	case 0x4E: // LSR Absolute
		LSR(ABS());
		break;
	case 0x50: // BVC Relative
		BVC(REL());
		break;
	case 0x51: // EOR Indirect,Y
		EOR(INDIRECT_Y());
		break;
	case 0x55: // EOR ZeroPage,X
		EOR(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x56: // LSR ZeroPage,X
		LSR(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x58: // CLI Implied
		CLI();
		break;
	case 0x59: // EOR Absolute,Y
		EOR(ABS_INDEXED(mY));
		break;
	case 0x5D: // EOR Absolute,X
		EOR(ABS_INDEXED(mX));
		break;
	case 0x5E: // LSR Absolute,X
		LSR(ABS_INDEXED(mX));
		break;
	case 0x60: // RTS Implied
		RTS();
		break;
	case 0x61: // ADC Indirect,X
		ADC(INDIRECT_X());
		break;
	case 0x65: // ADC ZeroPage
		ADC(ZERO_PAGE(mPC));
		break;
	case 0x66: // ROR ZeroPage
		ROR(ZERO_PAGE(mPC));
		break;
	case 0x68: // PLA Implied
		PLA();
		break;
	case 0x69: // ADC Immediate
		ADC(IMM());
		break;
	case 0x6A: // ROR Accumulator
		ROR_A();
		break;
	case 0x6C: // JMP Indirect
		JMP(JMP_INDIRECT(ABS()));
		break;
	case 0x6D: // ADC Absolute
		ADC(ABS());
		break;
	case 0x6E: // ROR Absolute
		ROR(ABS());
		break;
	case 0x70: // BVS Relative
		BVS(REL());
		break;
	case 0x71: // ADC Indirect,Y
		ADC(INDIRECT_Y());
		break;
	case 0x75: // ADC ZeroPage,X
		ADC(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x76: // ROR ZeroPage,X
		ROR(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x78: // SEI Implied
		SEI();
		break;
	case 0x79: // ADC Absolute,Y
		ADC(ABS_INDEXED(mY));
		break;
	case 0x7D: // ADC Absolute,X
		ADC(ABS_INDEXED(mX));
		break;
	case 0x7E: // ROR Absolute,X
		ROR(ABS_INDEXED(mX));
		break;
	case 0x81: // STA Indirect,X
		STA(INDIRECT_X());
		break;
	case 0x84: // STY ZeroPage
		STY(ZERO_PAGE(mPC));
		break;
	case 0x85: // STA ZeroPage
		STA(ZERO_PAGE(mPC));
		break;
	case 0x86: // STX ZeroPage
		STX(ZERO_PAGE(mPC));
		break;
	case 0x88: // DEY Implied
		DEY();
		break;
	case 0x8A: // TXA Implied
		TXA();
		break;
	case 0x8C: // STY Absolute
		STY(ABS());
		break;
	case 0x8D: // STA Absolute
		STA(ABS());
		break;
	case 0x8E: // STX Absolute
		STX(ABS());
		break;
	case 0x90: // BCC Relative 
		BCC(REL());
		break;
	case 0x91: // STA Indirect,Y
		STA(INDIRECT_Y());
		break;
	case 0x94: // STY ZeroPage,X
		STY(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x95: // STA ZeroPage,X
		STA(ZERO_PAGE_INDEXED(mX));
		break;
	case 0x96: // STX ZeroPage,Y
		STX(ZERO_PAGE_INDEXED(mY));
		break;
	case 0x98: // TYA Implied
		TYA();
		break;
	case 0x99: // STA Absolute,Y
		STA(ABS_INDEXED(mY));
		break;
	case 0x9A: // TXS Implied
		TXS();
		break;
	case 0x9D: // STA Absolute,X
		STA(ABS_INDEXED(mX));
		break;
	case 0xA0: // LDY Immediate
		LDY(IMM());
		break;
	case 0xA1: // LDA Indirect,X
		LDA(INDIRECT_X());
		break;
	case 0xA2: // LDX Immediate
		LDX(IMM());
		break;
	case 0xA4: // LDY ZeroPage
		LDY(ZERO_PAGE(mPC));
		break;
	case 0xA5: // LDA ZeroPage
		LDA(ZERO_PAGE(mPC));
		break;
	case 0xA6: // LDX ZeroPage
		LDX(ZERO_PAGE(mPC));
		break;
	case 0xA8: // TAY Implied
		TAY();
		break;
	case 0xA9: // LDA Immediate
		LDA(IMM());
		break;
	case 0xAA: // TAX Implied
		TAX();
		break;
	case 0xAC: // LDY Absolute
		LDY(ABS());
		break;
	case 0xAD: // LDA Absolute
		LDA(ABS());
		break;
	case 0xAE: // LDX Absolute
		LDX(ABS());
		break;
	case 0xB0: // BCS Relative
		BCS(REL());
		break;
	case 0xB1: // LDA Indirect,Y
		LDA(INDIRECT_Y());
		break;
	case 0xB4: // LDY ZeroPage,X
		LDY(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xB5: // LDA ZeroPage,X
		LDA(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xB6: // LDX ZeroPage,Y
		LDX(ZERO_PAGE_INDEXED(mY));
		break;
	case 0xB8: // CLV Implied
		CLV();
		break;
	case 0xB9: // LDA Absolute,Y
		LDA(ABS_INDEXED(mY));
		break;
	case 0xBA: // TSX Implied
		TSX();
		break;
	case 0xBC: // LDY Absolute,X
		LDY(ABS_INDEXED(mX));
		break;
	case 0xBD: // LDA Absolute,X
		LDA(ABS_INDEXED(mX));
		break;
	case 0xBE: // LDX Absolute,Y
		LDX(ABS_INDEXED(mY));
		break;
	case 0xC0: // CPY Immediate
		CPY(IMM());
		break;
	case 0xC1: // CMP Indirect,X
		CMP(INDIRECT_X());
		break;
	case 0xC4: // CPY ZeroPage
		CPY(ZERO_PAGE(mPC));
		break;
	case 0xC5: // CMP ZeroPage
		CMP(ZERO_PAGE(mPC));
		break;
	case 0xC6: // DEC ZeroPage
		DEC(ZERO_PAGE(mPC));
		break;
	case 0xC8: // INY Implied
		INY();
		break;
	case 0xC9: // CMP Immediate
		CMP(IMM());
		break;
	case 0xCA: // DEX Implied
		DEX();
		break;
	case 0xCC: // CPY Absolute
		CPY(ABS());
		break;
	case 0xCD: // CMP Absolute
		CMP(ABS());
		break;
	case 0xCE: // DEC Absolute
		DEC(ABS());
		break;
	case 0xD0: // BNE Relative
		BNE(REL());
		break;
	case 0xD1: // CMP Indirect,Y
		CMP(INDIRECT_Y());
		break;
	case 0xD5: // CMP ZeroPage,X
		CMP(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xD6: // DEC ZeroPage,X
		DEC(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xD8: // CLD Implied
		CLD();
		break;
	case 0xD9: // CMP Absolute,Y
		CMP(ABS_INDEXED(mY));
		break;
	case 0xDD: // CMP Absolute,X
		CMP(ABS_INDEXED(mX));
		break;
	case 0xDE: // DEC Absolute,X
		DEC(ABS_INDEXED(mX));
		break;
	case 0xE0: // CPX Immediate
		CPX(IMM());
		break;
	case 0xE1: // SBC Indirect,X
		SBC(INDIRECT_X());
		break;
	case 0xE4: // CPX ZeroPage
		CPX(ZERO_PAGE(mPC));
		break;
	case 0xE5: // SBC ZeroPage
		SBC(ZERO_PAGE(mPC));
		break;
	case 0xE6: // INC ZeroPage
		INC(ZERO_PAGE(mPC));
		break;
	case 0xE8: // INX Implied
		INX();
		break;
	case 0xEA: // NOP Implied
		break;
	case 0xE9: // SBC Immediate
		SBC(IMM());
		break;
	case 0xEC: // CPX Absolute
		CPX(ABS());
		break;
	case 0xEE: // INC Absolute
		INC(ABS());
		break;
	case 0xED: // SBC Absolute
		SBC(ABS());
		break;
	case 0xF0: // BEQ Relative
		BEQ(REL());
		break;
	case 0xF1: // SBC Indirect,Y
		SBC(INDIRECT_Y());
		break;
	case 0xF5: // SBC ZeroPage,X
		SBC(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xF6: // INC ZeroPage,X
		INC(ZERO_PAGE_INDEXED(mX));
		break;
	case 0xF8: // SED Implied
		SED();
		break;
	case 0xF9: // SBC Absolute,Y
		SBC(ABS_INDEXED(mY));
		break;
	case 0xFD: // SBC Absolute,X
		SBC(ABS_INDEXED(mX));
		break;
	case 0xFE: // INC Absolute,X
		INC(ABS_INDEXED(mX));
		break;
	default:
		dump();
		char msg[1024];
		sprintf(msg, "opcode: %02X is unsupported", o);
		throw std::runtime_error(msg);
		break;
	}
	mClockRemain = clockTable[o];
	mP |= FLG_5; // bit5 is always '1'

	Config* conf = Config::getInstance();
	if (conf->getVarbose()) { 
		this->dump();
	}
}

void CPU::coreDump(Core* c) const {
	struct CPUCore _cpu;

	_cpu.a           = mA;
	_cpu.x           = mX;
	_cpu.y           = mY;
	_cpu.s           = mS;
	_cpu.p           = mP;
	_cpu.pc          = mPC;
	_cpu.clockRemain = mClockRemain;
	_cpu.resetFlag   = mResetFlag;
	_cpu.NMIFlag     = mNMIFlag;
	_cpu.IRQFlag     = mIRQFlag;
	_cpu.BRKFlag     = mBRKFlag;

	c->setCPU(_cpu);
}

void CPU::loadCore(Core* c) {
	const struct CPUCore _cpu = c->getCPU();

	this->mA           = _cpu.a;
	this->mX           = _cpu.x;
	this->mY           = _cpu.y;
	this->mS           = _cpu.s;
	this->mP           = _cpu.p;
	this->mPC          = _cpu.pc;
	this->mClockRemain = _cpu.clockRemain;
	this->mResetFlag   = _cpu.resetFlag;
	this->mNMIFlag     = _cpu.NMIFlag;
	this->mIRQFlag     = _cpu.IRQFlag;
	this->mBRKFlag     = _cpu.BRKFlag;
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
	CLEAR_B();
	PUSH(mPC >> 8);
	PUSH(mPC&0xFF);
	PUSH(mP);
	SET_I();
	mPC = mMapper->read2Bytes(NMI_VECTOR);

	mNMIFlag = false;
	mClockRemain = 6;
}

void CPU::doIRQ() {
	CLEAR_B();
	PUSH(mPC >> 8);
	PUSH(mPC&0xFF);
	PUSH(mP);
	SET_I();
	mPC = mMapper->read2Bytes(BRK_VECTOR);

	mClockRemain = 6;
}

void CPU::startDMA() {
	mClockRemain = 514;
}

void CPU::buildADC_APvcTable() {
	mADC_APvcTable = new struct APvc[256*256*2];
	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			for (int c = 0; c < 2; c++) {
				int cc = a+b+c;
				mADC_APvcTable[((uint16_t)a*256 +b)*2 +c].a = cc;
				mADC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc = 0;
 
				if (cc >= 256) {
					mADC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc |= FLG_C;
				}

				if (((a ^ b) ^ 0x80) & (a ^ cc) & 0x80) {
					mADC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc |= FLG_V;
				}
			}
		}
	}
}

void CPU::buildSBC_APvcTable() {
	mSBC_APvcTable = new APvc[256*256*2];
	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			for (int c = 0; c < 2; c++) {
				uint8_t _c = (c==0)? 1:0;
				int cc = 0x100 + a - b - _c;
				mSBC_APvcTable[((uint16_t)a*256 +b)*2 +c].a = cc;
				mSBC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc = 0;
				if (cc >= 0x100) {
					mSBC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc |= FLG_C;
				}
				if ((a ^ b) & (a ^ cc) & 0x80) {
					mSBC_APvcTable[((uint16_t)a*256 +b)*2 +c].p_vc |= FLG_V;
				}
			}
		}
	}
}

