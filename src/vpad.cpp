#include <stdio.h>
#include "vpad.h"

typedef PAD super;

VPAD::VPAD()
:PAD() {
}

VPAD::~VPAD() {
}

void VPAD::out(uint8_t val) {
	printf("PAD(%02X)\n",  val);
	super::out(val);
}

uint8_t VPAD::in1() {
	switch(mInCount1) {
	case 0: printf("PAD1::in(A)\n"); break;
	case 1: printf("PAD1::in(START)\n"); break;
	case 2: printf("PAD1::in(SELECT)\n"); break;
	case 3: printf("PAD1::in(UP)\n"); break;
	case 4: printf("PAD1::in(DOWN)\n"); break;
	case 5: printf("PAD1::in(LEFT)\n"); break;
	case 6: printf("PAD1::in(RIGHT)\n"); break;
	default: printf("PAD1::in(UNKNOWN)\n"); break;
	}

	return super::in1();
}
