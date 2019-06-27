#include "pad.h"

PAD::PAD()
:mLastOut(0xFF), mInCount1(0), mInCount2(0) {
}

PAD::~PAD() {
}

void PAD::out(uint8_t val) {
	if (mLastOut == 0x01 && val == 0x00) {
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

	mInCount1++;
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
	return ret;
}

void PAD::resetIn() {
	mInCount1 = 0;
	mInCount2 = 0;
}

