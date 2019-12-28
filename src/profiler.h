#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <chrono>
#include "frameworker.h"

class Profiler:public FrameWorker {
public:
	Profiler();
	virtual ~Profiler();

public:
	inline void cpuStart() {
		mCPUStart = std::chrono::system_clock::now();
	}
	inline void cpuEnd() {
		mCPUEnd = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::microseconds>(mCPUEnd-mCPUStart).count();
		mCPUElapsed += elapsed;
	}
	inline void ppuStart() {
		mPPUStart = std::chrono::system_clock::now();
	}
	inline void ppuEnd() {
		mPPUEnd = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::microseconds>(mPPUEnd-mPPUStart).count();
		mPPUElapsed += elapsed;
	}
	inline void apuStart() {
		mAPUStart = std::chrono::system_clock::now();
	}
	inline void apuEnd() {
		mAPUEnd = std::chrono::system_clock::now();
		float elapsed = std::chrono::duration_cast<std::chrono::microseconds>(mAPUEnd-mAPUStart).count();
		mAPUElapsed += elapsed;
	}
	void atFrameStart();

protected:
	std::chrono::system_clock::time_point mCPUStart;
	std::chrono::system_clock::time_point mCPUEnd;
	std::chrono::system_clock::time_point mPPUStart;
	std::chrono::system_clock::time_point mPPUEnd;
	std::chrono::system_clock::time_point mAPUStart;
	std::chrono::system_clock::time_point mAPUEnd;
	float mCPUElapsed;
	float mPPUElapsed;
	float mAPUElapsed;
};

#endif
