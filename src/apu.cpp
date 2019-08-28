#include <stdio.h>
#include "apu.h"

// $4015
#define CH_CTL_DMC   (0x10)
#define CH_CTL_NOIZE (0x08)
#define CH_CTL_TRI   (0x04)
#define CH_CTL_SQ2   (0x02)
#define CH_CTL_SQ1   (0x01)

// $4017
#define SEQUENCER_MODE (0x80)	// 0: 4-step, 1: 5-step
#define NO_IRQ (0x40)

#define CPU_FQ (1789772)	// NTSC
#define RENDER_FQ (44100)

int gLengthVal[] = {
/* 00-0F */  10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
/* 10-1F */  12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

APU::APU()
:mData(0) {
	mDClock = 0;

	mData = new short[DATA_LENGTH];
	for (int i = 0; i < DATA_LENGTH; i++) {
		mData[i] = 0;
	}

	mReadPoint = 0;
	mWritePoint = 0;
	mWriteLen = 0;

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
	mTSeq[1] = 0x05;
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
	mTChVal = 0;
	mTLen = 0;

	mCPUfq = CPU_FQ;
	mNextRenderClock = mCPUfq/RENDER_FQ;
	mRenderClock = 0;
}

APU::~APU() {
	if (mData) {
		delete[] mData;
	}
}

void APU::clock() {
	if (mDClock == 7456) {
		frameClock();
		mDClock = 0;
	} else {
		mDClock++;
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
	mData[mWritePoint] = mTChVal;

	mWriteLen++;
	mWritePoint++;
	if (mWritePoint >= DATA_LENGTH) {
		mWritePoint = 0;
	}
}

void APU::triangleClock() {
	mTDClk--;
	if (mTDClk == 0) {
		mTChVal = mTSeq[mTSeqIndex]*4096-32767;
		if (mTLen != 0) {
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
	} else {
		mTChVal = mTSeq[mTSeqIndex]*4096-32767;
	}
}

void APU::frameClock() {
	if (((mTWC&0x80) == 0) && (mTLen > 0)) mTLen--;
}

void APU::setFrameCounter(uint8_t val) {
	mFrameCounter = val;
	mDClock = 0;
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
}

