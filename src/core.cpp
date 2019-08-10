#include <cstring>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "core.h"

#define WRAM_SIZE (0x0800)

Core::Core() {
}

Core::~Core() {
}

void Core::setCPU(struct CPUCore& cpu) {
	memcpy(&mCPUCore, &cpu, sizeof(struct CPUCore));
}

void Core::setPPU(struct PPUCore& ppu) {
	memcpy(&mPPUCore, &ppu, sizeof(struct PPUCore));
}

void Core::setWRAM(uint8_t* WRAM) {
	memcpy(&mWRAM, WRAM, WRAM_SIZE);
}

void Core::dump(const char* path) {
	FILE* f = fopen(path, "w");
	if (f == 0) {
		fprintf(stderr, "fopen faild(%d)\n",errno);
		return;
	}
	
	fwrite(&mCPUCore, sizeof(struct CPUCore), 1, f);
	fwrite(&mPPUCore, sizeof(struct PPUCore), 1, f);
	fwrite(mWRAM, WRAM_SIZE, 1, f);

	fclose(f);
}

