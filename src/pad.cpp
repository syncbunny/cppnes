#include <iostream>
#include "pad.h"

PAD::PAD()
:mLastOut(0xFF), mInCount1(0), mInCount2(0) {
	mA[0]      = mA[1]      = false;
	mB[0]      = mB[1]      = false;
	mSelect[0] = mSelect[1] = false;
	mStart[0]  = mStart[1]  = false;
	mUp[0]     = mUp[1]     = false;
	mDown[0]   = mDown[1]   = false;
	mLeft[0]   = mLeft[1]   = false;
	mRight[0]  = mRight[1]  = false;
}

PAD::~PAD() {
}

void PAD::out(uint8_t val) {
	val &= 0x01;
	if (mLastOut == 0x01 && val == 0x00) {
		this->strobe();
		this->resetIn();
	}

	mLastOut = val;
}

uint8_t PAD::in1() {
	uint8_t ret = 0;
	switch(mInCount1) {
	case 0: // A
		if (mA[0]) ret = 1;
		break;
	case 1: // B
		if (mB[0]) ret = 1;
		break;
	case 2: // Select
		if (mSelect[0]) ret = 1;
		break;
	case 3: // Start
		if (mStart[0]) ret = 1;
		break;
	case 4: // Up
		if (mUp[0]) ret = 1;
		break;
	case 5: // Down
		if (mDown[0]) ret = 1;
		break;
	case 6: // Left
		if (mLeft[0]) ret = 1;
		break;
	case 7: // Right
		if (mRight[0]) ret = 1;
		break;
	}
	ret |= 0x40;

	mInCount1++;
	mInCount1 %=8;

	return ret;
}

uint8_t PAD::in2() {
	uint8_t ret = 0;

	switch(mInCount2) {
	case 0: // A
		break;
	case 1: // B
		break;
	case 2: // Select
		break;
	case 3: // Start
		break;
	case 4: // Up
		break;
	case 5: // Down
		break;
	case 6: // Left
		break;
	case 7: // Right
		break;
	}

	mInCount2++;
	mInCount2 %= 8;
	return ret;
}

void PAD::strobe() {
	mA[0]      = mA_[0];
	mB[0]      = mB_[0];
	mSelect[0] = mSelect_[0];
	mStart[0]  = mStart_[0];
	mUp[0]     = mUp_[0];
	mDown[0]   = mDown_[0];
	mLeft[0]   = mLeft_[0];
	mRight[0]  = mRight_[0];
}

void PAD::resetIn() {
	mInCount1 = 0;
	mInCount2 = 0;
}

