#include <stdio.h>
#include "vppu.h"

typedef PPU super;

VPPU::VPPU()
:PPU() {
}

VPPU::~VPPU() {
}

void VPPU::write(uint8_t val) {
	printf("PPU::write: %04x:%02x\n", mWriteAddr, val);
	super::write(val);
}
