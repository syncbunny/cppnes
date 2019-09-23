#ifndef OPENALAPU_H
#define OPENALAPU_H

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "apu.h"
#include "vapu.h"
#include "frameworker.h"

//class OpenALAPU:public APU, public FrameWorker {
class OpenALAPU:public VAPU, public FrameWorker {
public:
	OpenALAPU();
	virtual ~OpenALAPU();

public:
	void atFrameStart();

protected:
	void initialize();
	void bufferQueued(ALuint bufId);
	void bufferProcessed(ALuint bufId);
	bool getUnusedBuffer(ALuint* bufId);

protected:
	enum {
		NUM_BUFFERS = 16,
		FRAME_DATA_LEN = 750, // 44100/60
	};
	struct BufferInfo {
		bool inUse;
		ALuint bufId;
	};

protected:
	ALCdevice* mDevice;
	ALCcontext* mContext;
	ALuint mBuffers[NUM_BUFFERS];
	ALuint mSources[1];
	struct BufferInfo mBufferInfo[NUM_BUFFERS];
};

#endif
