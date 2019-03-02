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
