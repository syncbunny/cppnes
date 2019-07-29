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
	const char* map = "-";
	if (mWriteAddr >= 0x0000 && mWriteAddr <= 0x0FFF) {
		map = "pattern table 1";
	}
	if (mWriteAddr >= 0x1000 && mWriteAddr <= 0x0FFF) {
		map = "pattern table 2";
	}
	if (mWriteAddr >= 0x2000 && mWriteAddr <= 0x23BF) {
		map = "name table 0";
	}
	if (mWriteAddr >= 0x23C0 && mWriteAddr <= 0x23FF) {
		map = "attribure table 0";
	}
	if (mWriteAddr >= 0x2400 && mWriteAddr <= 0x27BF) {
		map = "name table 1";
	}
	if (mWriteAddr >= 0x27C0 && mWriteAddr <= 0x27FF) {
		map = "attribure table 1";
	}
	if (mWriteAddr >= 0x2800 && mWriteAddr <= 0x2BBF) {
		map = "name table 2";
	}
	if (mWriteAddr >= 0x2BC0 && mWriteAddr <= 0x2BFF) {
		map = "attribure table 2";
	}
	if (mWriteAddr >= 0x2C00 && mWriteAddr <= 0x2CBF) {
		map = "name table 2";
	}
	if (mWriteAddr >= 0x2FC0 && mWriteAddr <= 0x2FFF) {
		map = "attribure table 2";
	}
	if (mWriteAddr >= 0x3000 && mWriteAddr <= 0x3EFF) {
		map = "MIRROR 1";
	}
	if (mWriteAddr >= 0x3F00 && mWriteAddr <= 0x3F0F) {
		map = "BG palette";
	}
	if (mWriteAddr >= 0x3F10 && mWriteAddr <= 0x3F1F) {
		map = "SP palette";
	}
	if (mWriteAddr >= 0x3F20 && mWriteAddr <= 0x3FFF) {
		map = "MIRROR 2";
	}
	printf("PPU::write: %04x(%s):%02x\n", mWriteAddr, map, val);
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
//		dumpNameTable();
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

void VPPU::dumpNameTable() {
	char fname[256];
	sprintf(fname, "NAME_TABLE_%04d.txt", mFrames);
	FILE* f = fopen(fname, "w");
	if (f) {
		for (int v = 0; v < 30; v++) {
			for (int u = 0; u < 32; u++) {
				if (u == 0) {
					fprintf(f, "%02d %02X", v, mMem[0x2000 + v*32 + u]);
				} else {
					fprintf(f, " %02X", mMem[0x2000 + v*32 + u]);
				}
			}
			for (int u = 0; u < 32; u++) {
				fprintf(f, " %02X", mMem[0x2400 + v*32 + u]);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "\n");
		for (int v = 0; v < 30; v++) {
			for (int u = 0; u < 32; u++) {
				if (u == 0) {
					fprintf(f, "%02d %02X", v, mMem[0x2800 + v*32 + u]);
				} else {
					fprintf(f, " %02X", mMem[0x2800 + v*32 + u]);
				}
			}
			for (int u = 0; u < 32; u++) {
				fprintf(f, " %02X", mMem[0x2C00 + v*32 + u]);
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}
}
