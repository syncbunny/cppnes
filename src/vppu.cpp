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

uint8_t VPPU::getSR() {
	uint8_t sr = super::getSR();
	printf("PPU::getSR() => %02d\n", sr);
	return sr;
}

void VPPU::startVR() {
	printf("PPU::startVR:%d frame\n", mFrames);
	super::startVR();
}
