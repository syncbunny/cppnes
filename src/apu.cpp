#include <stdio.h>
#include "apu.h"

// $4017
#define SEQUENCER_MODE (0x80)	// 0: 4-step, 1: 5-step
#define NO_IRQ (0x40)

#define CPU_FQ (1789772)	// NTSC

APU::APU()
:mData(0), mTriangleWeve(0) {
	mDClock = 0;

	mData = new short[DATA_LENGTH];
	for (int i = 0; i < DATA_LENGTH; i++) {
		mData[i] = 0;
	}

	mReadPoint = 0;
	mWritePoint = 0;
	mWriteLen = 0;

	makeTriangleWave();
	mTFQ = 0;
}

APU::~APU() {
	if (mData) {
		delete[] mData;
	}
}

void APU::clock() {
	if (mDClock == 7456) {
		_clock();
		mDClock = 0;
	} else {
		mDClock++;
	}
}

void APU::makeTriangleWave() {
	mTriangleWeve = new short[DATA_LENGTH];
	for (int i = 0; i < DATA_LENGTH/2; i++) {
		mTriangleWeve[i] = 65536*i/(DATA_LENGTH/2) - 32767;
		mTriangleWeve[i] &= 0xF000;
	}
	for (int i = 0; i < DATA_LENGTH/2; i++) {
		int k = i+DATA_LENGTH/2;
		mTriangleWeve[k] = (65536*(i-DATA_LENGTH/2))/(DATA_LENGTH/2) - 32767;
		mTriangleWeve[k] &= 0xF000;
	}
	mTriangleReadPoint = 0;
}

void APU::_clock() {
	// Write wave data into data-buffer
	int tfq = mTFQ % DATA_LENGTH;
	for (int i = 0; i < CLOCK_DATA_LENGTH; i++) {
		// TODO
		
		mData[mWritePoint] = mTriangleWeve[mTriangleReadPoint];
		mTriangleReadPoint += tfq;
		if (mTriangleReadPoint >= DATA_LENGTH) {
			mTriangleReadPoint -= DATA_LENGTH;
		}

		mWritePoint++;
		if (mWritePoint >= DATA_LENGTH) {
			mWritePoint = 0;
		}
	}
	mWriteLen += CLOCK_DATA_LENGTH;

	printf("clock:mWriteLen=%d\n", mWriteLen);
}

void APU::setFrameCounter(uint8_t val) {
	mFrameCounter = val;
	mDClock = 0;
}

void APU::setTWC(uint8_t val) {
	mTWC = val;
	calcTFQ();
}

void APU::setTWFQ1(uint8_t val) {
	mTWFQ1 = val;
	calcTFQ();
}

void APU::setTWFQ2(uint8_t val) {
	mTWFQ2 = val;
	calcTFQ();
}

void APU::calcTFQ() {
	uint8_t linearCounter;
	linearCounter = mTWC & 0x7f;
	if (linearCounter == 0) {
		mTFQ = 0;
		return;
	}
 
	// f = fCPU/(32*(tval + 1))
	uint32_t fq;
	uint32_t t;
	t = mTWFQ2 & 0x07;
	t <<= 8;
	t |= mTWFQ1;
	mTFQ = CPU_FQ/(32*(t+1));
}
