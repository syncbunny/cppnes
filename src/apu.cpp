#include "apu.h"
#include "events.h"

// $4000, $4004
#define ENVELOPE_LOOP     (0x20)
#define ENVELOPE_CONSTANT (0x10)
#define ENVELOPE_FQ       (0x0F)

// $4001, $4005
#define SWEEP_ENABLE (0x80)
#define SWEEP_FQ     (0x70)
#define SWEEP_DIR    (0x08)
#define SWEEP_VAL    (0x07)

// $4008
#define TWC_CTL      (0x80)
#define TWC_VAL      (0x7F)

// $4015
#define CH_CTL_INTERRUPT (0x40)
#define CH_CTL_DMC   (0x10)
#define CH_CTL_NOIZE (0x08)
#define CH_CTL_TRI   (0x04)
#define CH_CTL_SQ2   (0x02)
#define CH_CTL_SQ1   (0x01)

// $4017 (mFrameCounter)
#define SEQUENCER_MODE (0x80)	// 0: 4-step, 1: 5-step
#define NO_IRQ (0x40)

#define CPU_FQ (1789772)	// NTSC
#define RENDER_FQ (44100)

#define CALC_SW1FQ()  (mSW1FQ = mSW1FQ2 & 0x07, mSW1FQ <<=8, mSW1FQ |= mSW1FQ1)
#define CALC_SW2FQ()  (mSW2FQ = mSW2FQ2 & 0x07, mSW2FQ <<=8, mSW2FQ |= mSW2FQ1)

int gLengthVal[] = {
/* 00-0F */  10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
/* 10-1F */  12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};
int gNoiseLenVal[] = {
	0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0060, 0x0080, 0x00A0,
	0x00CA, 0x00FE, 0x017C, 0x01FC, 0x02FA, 0x03F8, 0x07F2, 0x0FE4
};

int gSQ12Val[] = {0, 1, 0, 0, 0, 0, 0, 0};
int gSQ25Val[] = {0, 1, 1, 0, 0, 0, 0, 0};
int gSQ50Val[] = {0, 1, 1, 1, 1, 0, 0, 0};
int gSQ75Val[] = {1, 0, 0, 0, 0, 0, 0, 1};
int* gSQVal[]   = {gSQ12Val, gSQ25Val, gSQ50Val, gSQ75Val};

int gNoiseFQ[] = {
	0x004, 0x008, 0x010, 0x020,
	0x040, 0x060, 0x080, 0x0A0,
	0x0CA, 0x0FE, 0x17C, 0x1FC,
	0x2FA, 0x3F8, 0x7F2, 0xFE4
};

APU::APU()
:mData(0) {
	mData = new short[DATA_LENGTH];
	for (int i = 0; i < DATA_LENGTH; i++) {
		mData[i] = 0;
	}

	// Clear registers
	mSW1C1        = 0;
        mSW1C2        = 0;
        mSW1FQ1       = 0;
        mSW1FQ2       = 0;
        mSW2C1        = 0;
        mSW2C2        = 0;
        mSW2FQ1       = 0;
        mSW2FQ2       = 0;
        mTWC          = 0;
        mTWFQ1        = 0;
        mTWFQ2        = 0;
        mNC           = 0;
        mNFQ1         = 0;
        mNFQ2         = 0;
        mDMC1         = 0;
        mDMC2         = 0;
        mDMC3         = 0;
        mDMC4         = 0;
        mChCtrl       = 0;
        mFrameCounter = 0;

	mReadPoint = 0;
	mWritePoint = 0;
	mWriteLen = 0;
	mDFrameClock = 0;
	mFrameSQCount = 0;
	mFrameInterrupt = false;

	// Square channel stuff
	mSWClock = 0;
	mSW1Len = 0;
	mSW2Len = 0;

	// Triangle channel stuff
	mTTimer = 0;
	mTSeq[0] = 0x0F;
	mTSeq[1] = 0x0E;
	mTSeq[2] = 0x0D;
	mTSeq[3] = 0x0C;
	mTSeq[4] = 0x0B;
	mTSeq[5] = 0x0A;
	mTSeq[6] = 0x09;
	mTSeq[7] = 0x08;
	mTSeq[8] = 0x07;
	mTSeq[9] = 0x06;
	mTSeq[10] = 0x05;
	mTSeq[11] = 0x04;
	mTSeq[12] = 0x03;
	mTSeq[13] = 0x02;
	mTSeq[14] = 0x01;
	mTSeq[15] = 0x00;
	mTSeq[16] = 0x00;
	mTSeq[17] = 0x01;
	mTSeq[18] = 0x02;
	mTSeq[19] = 0x03;
	mTSeq[20] = 0x04;
	mTSeq[21] = 0x05;
	mTSeq[22] = 0x06;
	mTSeq[23] = 0x07;
	mTSeq[24] = 0x08;
	mTSeq[25] = 0x09;
	mTSeq[26] = 0x0A;
	mTSeq[27] = 0x0B;
	mTSeq[28] = 0x0C;
	mTSeq[29] = 0x0D;
	mTSeq[30] = 0x0E;
	mTSeq[31] = 0x0F;
	mTSeqIndex = 0;
	mTDClk = 0;
	mTChVal = 0;
	mTLen = 0;
	mTLCnt = 0;
	mTLCntReload = 0;

	// Noise channel stuff
	mNLen = 0;

	mCPUfq = CPU_FQ;
	mNextRenderClock = mCPUfq/RENDER_FQ;
	mRenderClock = 0;
	mSW1FQ = mSW2FQ = 0;

	mSweep1  = new Sweep(this->mSW1C2, this->mSW1FQ, this->mSW1Len);
	mEnv1    = new Envelope(this->mSW1C1);
	mSquare1 = new Square(this->mSW1C1, this->mSW1Len, this->mSW1FQ, this->mEnv1);

	mSweep2  = new Sweep(this->mSW2C2, this->mSW2FQ, this->mSW2Len);
	mEnv2    = new Envelope(this->mSW2C1);
	mSquare2 = new Square(this->mSW2C1, this->mSW2Len, this->mSW2FQ, this->mEnv2);

	mNoiseEnv = new Envelope(this->mNC);
	mNoise    = new Noise(this->mNC, mNFQ1, mNFQ2, mNLen, this->mNoiseEnv);
}

APU::~APU() {
	if (mData) {
		delete[] mData;
	}
	delete mSweep1;
	delete mEnv1;
	delete mSquare1;
	delete mSweep2;
	delete mEnv2;
	delete mSquare2;
}

void APU::clock() {
	if (mDFrameClock == 7456) {
		frameClock();
		mDFrameClock= 0;
	} else {
		mDFrameClock++;
	}

	if (mSWClock) {
		mSquare1->clock();
		mNoise->clock();
		mSWClock = 0;
	} else {
		mSquare2->clock();
		mSWClock = 1;
	}
	triangleClock();

	mRenderClock++;
	if (mRenderClock == mNextRenderClock) {
		render();

		mCPUfq += CPU_FQ - (RENDER_FQ*mNextRenderClock);
		mNextRenderClock = mCPUfq/RENDER_FQ;
		mRenderClock = 0;
	}
}

void APU::render() {
	//printf("mNextRenderClock=%d\n", mNextRenderClock);
	mData[mWritePoint] = mTChVal + mSquare1->val() + mSquare2->val() + mNoise->val();

	mWriteLen++;
	mWritePoint++;
	if (mWritePoint >= DATA_LENGTH) {
		mWritePoint = 0;
	}
}

void APU::triangleClock() {
	if (mTDClk > 0) {
		mTDClk--;
	}
	if (mTDClk == 0) {
		mTChVal = mTSeq[mTSeqIndex]*512-256;
		if (mTLen != 0 && mTLCnt != 0) {
			mTSeqIndex++;
		}
		if (mTSeqIndex >= 32) {
			mTSeqIndex = 0;
		}
		mTDClk = mTWFQ2 & 0x07;
		mTDClk <<=8;
		mTDClk |= mTWFQ1;
	}

	if ((mChCtrl & CH_CTL_TRI) == 0) {
		mTChVal = 0;
	}
}

void APU::frameClock() {
	if ((mFrameCounter & SEQUENCER_MODE) == 0) {
		// 4-Step
		switch (mFrameSQCount) {
		case 0:
			mEnv1->clock();
			mSweep1->clock();
			mEnv2->clock();
			mSweep2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			mFrameSQCount = 1;
			break;
		case 1:
			mEnv1->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			if (((mTWC&0x80) == 0) && (mTLen > 0)) mTLen--;
			if (((mSW1C1&0x20) == 0) && (mSW1Len > 0)) mSW1Len--;
			if (((mSW2C1&0x20) == 0) && (mSW2Len > 0)) mSW2Len--;
			if (((mNC&0x20) == 0) && (mNLen > 0)) mNLen--;
			mFrameSQCount = 2;
			break;
		case 2:
			mEnv1->clock();
			mSweep1->clock();
			mEnv2->clock();
			mSweep2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			mFrameSQCount = 3;
			break;
		case 3:
			mEnv1->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			if (((mTWC&0x80) == 0) && (mTLen > 0)) mTLen--;
			if (((mSW1C1&0x20) == 0) && (mSW1Len > 0)) mSW1Len--;
			if (((mSW2C1&0x20) == 0) && (mSW2Len > 0)) mSW2Len--;
			if (((mNC&0x20) == 0) && (mNLen > 0)) mNLen--;
			if ((mFrameCounter & NO_IRQ) == 0) {
				mFrameInterrupt = true;
	                	EventQueue& eq = EventQueue::getInstance();
				eq.push(new EventIRQ());
			}

			mFrameSQCount = 0;
			break;
		}
	} else {
		// 5-Step
		switch (mFrameSQCount) {
		case 0:
			mEnv1->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			mFrameSQCount = 1;
			break;
		case 1:
			mSweep1->clock();
			mEnv1->clock();
			mSweep2->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			if (((mTWC&0x80) == 0) && (mTLen > 0)) mTLen--;
			if (((mSW1C1&0x20) == 0) && (mSW1Len > 0)) mSW1Len--;
			if (((mSW2C1&0x20) == 0) && (mSW2Len > 0)) mSW2Len--;
			if (((mNC&0x20) == 0) && (mNLen > 0)) mNLen--;
			mFrameSQCount = 2;
			break;
		case 2:
			mEnv1->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			mFrameSQCount = 3;
			break;
		case 3:
			mFrameSQCount = 4;
			break;
		case 4:
			mSweep1->clock();
			mEnv1->clock();
			mSweep2->clock();
			mEnv2->clock();
			mNoiseEnv->clock();
			this->triangleLinerCounterClock();
			if (((mTWC&0x80) == 0) && (mTLen > 0)) mTLen--;
			if (((mSW1C1&0x20) == 0) && (mSW1Len > 0)) mSW1Len--;
			if (((mSW2C1&0x20) == 0) && (mSW2Len > 0)) mSW2Len--;
			if (((mNC&0x20) == 0) && (mNLen > 0)) mNLen--;
			mFrameSQCount = 0;
			break;
		}
	}
}

void APU::triangleLinerCounterClock() {
	if (mTLCntReload) {
		mTLCnt = mTWC & TWC_VAL;
	} else {
		if (mTLCnt > 0) {
			mTLCnt--;
		}
	}
	if ((mTWC & TWC_CTL) == 0) {
		mTLCntReload = 0;
	}
}

void APU::setFrameCounter(uint8_t val) {
	mFrameCounter = val;
	mDFrameClock = 0;
	mFrameSQCount = 0;
}

void APU::setSW1FQ1(uint8_t val) {
	mSW1FQ1 = val;
	CALC_SW1FQ();
}

void APU::setSW2FQ1(uint8_t val) {
	mSW2FQ1 = val;
	CALC_SW2FQ();
}

void APU::setSW1FQ2(uint8_t val) {
	mSW1FQ2 = val;
	CALC_SW1FQ();

	mSW1Len = gLengthVal[mSW1FQ2>>3];

	mSweep1->reset();
	mEnv1->reset();
}

void APU::setSW2FQ2(uint8_t val) {
	mSW2FQ2 = val;
	CALC_SW2FQ();

	mSW2Len = gLengthVal[mSW2FQ2>>3];

	mSweep2->reset();
	mEnv2->reset();
}

void APU::setTWC(uint8_t val) {
	mTWC = val;

	mTDClk = mTWFQ2 & 0x07;
	mTDClk <<=8;
	mTDClk |= mTWFQ1;
}

void APU::setTWFQ1(uint8_t val) {
	mTWFQ1 = val;

	mTDClk = mTWFQ2 & 0x07;
	mTDClk <<=8;
	mTDClk |= mTWFQ1;
}

void APU::setTWFQ2(uint8_t val) {
	mTWFQ2 = val;

	mTDClk = mTWFQ2 & 0x07;
	mTDClk <<=8;
	mTDClk |= mTWFQ1;

	mTLen = gLengthVal[mTWFQ2>>3];
	mTLCntReload = 1;
}

void APU::setNFQ1(uint8_t val) {
	mNFQ1 = val;

	mNLen = gNoiseFQ[mNFQ1&0x0F];
}

void APU::setNFQ2(uint8_t val) {
	mNFQ2 = val;
	mNLen = gLengthVal[mNFQ2>>3];

	mNoiseEnv->reset();
}

void APU::setChCtrl(uint8_t val) {
	mChCtrl = val;

	if ((mChCtrl & CH_CTL_NOIZE) == 0) {
		mNLen = 0;
	}
	if ((mChCtrl & CH_CTL_TRI) == 0) {
		mTLen = 0;
	}
	if ((mChCtrl & CH_CTL_SQ1) == 0) {
		mSW1Len = 0;
	}
	if ((mChCtrl & CH_CTL_SQ2) == 0) {
		mSW2Len = 0;
	}
}

uint8_t APU::getChCtrl() {
	uint8_t ret = 0;

	if (mFrameInterrupt) {
		ret |= CH_CTL_INTERRUPT;
		mFrameInterrupt = false;
	}

	if (mNLen) {
		ret |= CH_CTL_NOIZE;
	}
	if (mTLen) {
		ret |= CH_CTL_TRI;
	}
	if (mSW1Len) {
		ret |= CH_CTL_SQ1;
	}
	if (mSW2Len) {
		ret |= CH_CTL_SQ2;
	}

	return ret;
}

APU::Square::Square(uint8_t& reg1, int& len, int& fq, APU::Envelope* env)
:mReg1(reg1), mLen(len), mFQ(fq), mEnv(env) {
	this->mDClock = 0;
	this->mSQIndex = 0;
	this->mVal = 0;
}

APU::Square::~Square() {
}

void APU::Square::clock() {
	if (this->mDClock > 0) {
		this->mDClock--;
	}

	if (this->mDClock == 0) {
		uint8_t vol = mEnv->getVol(); // [0 - 15]

		int* sqval;
		switch(mReg1 >> 6) {
		case 0x00:
			sqval = gSQ12Val;
			break;
		case 0x01:
			sqval = gSQ25Val;
			break;
		case 0x02:
			sqval = gSQ50Val;
			break;
		case 0x03:
			sqval = gSQ75Val;
			break;
		}
		this->mVal = sqval[mSQIndex]*vol*256;
		this->mVal -= vol*128; // [-2048 - 2047]

		if (this->mLen != 0) {
			this->mSQIndex++;
			if (this->mSQIndex >= 8) {
				this->mSQIndex = 0;
			}
		}
		this->mDClock = mFQ;
	}
}

APU::Noise::Noise(uint8_t& reg1, uint8_t& reg2, uint8_t& reg3, int& len, Envelope* env)
: mReg1(reg1), mReg2(reg2), mReg3(reg3), mLen(len),  mEnv(env) {
	mDClock = 0;
	mSR = 1;
	mVal = 0;
}

APU::Noise::~Noise() {
}

void APU::Noise::clock() {
	if (mDClock > 0) {
		mDClock--;
	}

	if (mDClock == 0) {
		uint8_t vol = mEnv->getVol();

		if (mLen == 0) {
			// Shift and xor
			uint16_t tmp;
			mSR &= 0x7FFF; // clear bit 15
			if (mReg2&0x80) {
				tmp = mSR >> 6;
			} else {
				tmp = mSR >> 1;
			}
			uint8_t exor = (tmp&1) ^ (mSR&1);
			mSR >>= 1;
			if (exor) {
				mSR |= 0x4000;
			}

	//		mLen = gNoiseFQ[mReg2&0x0F];
		} else {
			mLen--;
		}

		if ((mSR & 1) == 0) {
			mVal = vol*256;
		} else {
			mVal = 0;
		}
		
		mVal -= vol*128;

		mDClock = gLengthVal[mReg3>>3];
	}
}

APU::Sweep::Sweep(uint8_t& reg, int& fq, int& len)
:mReg(reg), mFQ(fq), mSWLen(len) {
	mDClock = 0;
}

APU::Sweep::~Sweep() {
}

void APU::Sweep::reset() {
	this->mDClock = (uint8_t)((mReg & SWEEP_FQ)) >> 4;
}

void APU::Sweep::clock() {
	if ((mReg & SWEEP_ENABLE) == 0) {
		return;
	}

	if (this->mDClock > 0) {
		this->mDClock--;
		return;
	}

	int val = mReg & SWEEP_VAL;
	if (val == 0) {
		return;
	}

	int newFQ;
	if ((mReg & SWEEP_DIR) == 0) {	
		newFQ = mFQ + (mFQ >> val);
	} else {
		newFQ = mFQ - (mFQ >> val);
	}
	if (newFQ >= 8 && newFQ <= 0x7ff) {
		mFQ = newFQ;
	} else {
		mSWLen = 0;
	}
}

APU::Envelope::Envelope(uint8_t& reg)
: mReg(reg){
	this->mDClock = 0;
	this->mVal = 0;
	this->mReset = false;
}

APU::Envelope::~Envelope() {
}

void APU::Envelope::reset() {
	mReset = true;
}

void APU::Envelope::clock() {
	if (mReset) {
		this->mDClock = this->mReg & ENVELOPE_FQ;
		this->mVal = 0x0F;
		mReset = false;
		return;
	}

	if (this->mDClock > 0) {
		this->mDClock--;
		return;
	}

	if (this->mVal > 0) {
		this->mVal--;
	} else if (this->mReg & ENVELOPE_LOOP) {
		this->mDClock = this->mReg & ENVELOPE_FQ;
		mVal = 0x0F;
	}
}

uint8_t APU::Envelope::getVol() {
	if (this->mReg & ENVELOPE_CONSTANT) {
		return (this->mReg & 0x0F);
	} else {
		return this->mVal;
	}
}
