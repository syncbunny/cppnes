#include <stdio.h>
#include "profiler.h"

Profiler::Profiler() {
	mCPUElapsed = 0;
	mPPUElapsed = 0;
	mAPUElapsed = 0;
}

Profiler::~Profiler() {
}

void Profiler::atFrameStart() {
	printf("Profiler:CPU: %f msec\n", mCPUElapsed);
	printf("Profiler:PPU: %f msec\n", mPPUElapsed);
	printf("Profiler:APU: %f msec\n", mAPUElapsed);

	mCPUElapsed = 0;
	mPPUElapsed = 0;
	mAPUElapsed = 0;
}
