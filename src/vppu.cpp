#include <cstdio>
#include <cstring>
#include "vppu.h"

typedef PPU super;

VPPU::VPPU()
:PPU() {
}

VPPU::~VPPU() {
}

void VPPU::setMirror(int m) {
	printf("PPU::setMirror: %s\n", (m==0)? "Horizontal":"Vertical");
	super::setMirror(m);
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

void VPPU::frameEnd() {
	if ((mFrames %10) == 0) {
		writeFrame();
	}
	super::frameEnd();
}

void VPPU::writeFrame() {
	char fname[256];
	sprintf(fname, "FRAME_%04d.ppm", mFrames);
	FILE* f = fopen(fname, "w");
	if (f) {
		fprintf(f, "P6\n");
		fprintf(f, "256 240\n");
		fprintf(f, "255\n");
		fwrite(mScreen, 1, 256*240*3, f);
		fclose(f);
	}
}
