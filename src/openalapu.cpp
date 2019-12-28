#include <stdio.h>
#include <stdexcept>
#include "openalapu.h"
#include "logger.h"

OpenALAPU::OpenALAPU()
:APU(), FrameWorker() {
	this->initialize();
}

OpenALAPU::~OpenALAPU() {
}

void OpenALAPU::initialize() {
	ALenum error;

	mDevice = alcOpenDevice(0);
	if (mDevice == 0) {
		std::runtime_error("alcOpenDevice failed.");
	}

	mContext = alcCreateContext(mDevice, 0);
	alcMakeContextCurrent(mContext);

	alGenBuffers(NUM_BUFFERS, mBuffers);
	if ((error = alGetError()) != AL_NO_ERROR) {
		char msg[128];
		sprintf(msg, "alGenBuffers() failed(%d)", error);
		throw std::runtime_error(msg);
	}
	for (int i = 0; i < NUM_BUFFERS; i++) {
		this->mBufferInfo[i].bufId = mBuffers[i];
		this->mBufferInfo[i].inUse = false;
	}

	alGenSources(1, mSources);
	if ((error = alGetError()) != AL_NO_ERROR) {
		char msg[128];
		sprintf(msg, "alGenSource() failed(%d)", error);
		throw std::runtime_error(msg);
	}
}

bool gc = false;

void OpenALAPU::atFrameStart() {
	ALenum error;

	Logger::getInstance()->log(Logger::DEBUG, "mWriteLen=%d\n", mWriteLen);
	// en-queue buffer
	while (mWriteLen > 0) {
		ALuint bufId;
		if (!this->getUnusedBuffer(&bufId)) {
			Logger::getInstance()->log(Logger::DEBUG, "NO BUFFER\n");
			break;
		}
		ALuint bufLen = mWriteLen;
		if (bufLen > FRAME_DATA_LEN/2) {
			bufLen = FRAME_DATA_LEN/2;
		}
		if (mReadPoint + bufLen >= DATA_LENGTH) {
			bufLen = DATA_LENGTH - mReadPoint;
		}
		Logger::getInstance()->log(Logger::DEBUG, "alBufferData(%d, AL_FORMAT_MONO16, %d, %lu, 44100)\n", bufId, mReadPoint, bufLen*sizeof(short));
		alBufferData(bufId, AL_FORMAT_MONO16, &mData[mReadPoint], bufLen*sizeof(short), 44100);
		if ((error = alGetError()) != AL_NO_ERROR) {
			char msg[128];
			sprintf(msg, "alBufferData() failed(%d)", error);
			throw std::runtime_error(msg);
		}
		alSourceQueueBuffers(mSources[0], 1, &bufId);
		if ((error = alGetError()) != AL_NO_ERROR) {
			char msg[128];
			sprintf(msg, "alSourceQueueBuffers() failed(%d)", error);
			throw std::runtime_error(msg);
		}
		this->bufferQueued(bufId);

		mWriteLen -= bufLen;
		mReadPoint += bufLen;
		if (mReadPoint >= DATA_LENGTH) {
			mReadPoint -= DATA_LENGTH;
		}
	}

	ALint state;
	alGetSourcei(mSources[0], AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING) {
		alSourcePlay(mSources[0]);
		if ((error = alGetError()) != AL_NO_ERROR) {
			char msg[128];
			sprintf(msg, "alSourcePlay() failed(%d)", error);
			throw std::runtime_error(msg);
		}
	}

	Logger::getInstance()->log(Logger::DEBUG, "alu state=0x%04x\n", state);
	// un-queue processed buffer
	ALint nrProcessed;
	alGetSourcei(mSources[0], AL_BUFFERS_PROCESSED, &nrProcessed);
	for (int i = 0; i < nrProcessed; i++) {
		ALuint bufId;
		alSourceUnqueueBuffers(mSources[0], 1, &bufId);
		this->bufferProcessed(bufId);
	}
}

void OpenALAPU::bufferQueued(ALuint bufId) {
	Logger::getInstance()->log(Logger::DEBUG, "bufferQueued(%d)\n", bufId);
	for (int i = 0; i < NUM_BUFFERS; i++) {
		if (this->mBufferInfo[i].bufId == bufId) {
			this->mBufferInfo[i].inUse = true;
			break;
		}
	}
}

void OpenALAPU::bufferProcessed(ALuint bufId) {
	Logger::getInstance()->log(Logger::DEBUG, "bufferProcessed(%d)\n", bufId);
	for (int i = 0; i < NUM_BUFFERS; i++) {
		if (this->mBufferInfo[i].bufId == bufId) {
			this->mBufferInfo[i].inUse = false;
			break;
		}
	}
}

bool OpenALAPU::getUnusedBuffer(ALuint* bufId) {
	for (int i = 0; i < NUM_BUFFERS; i++) {
		if (!this->mBufferInfo[i].inUse) {
			*bufId = this->mBufferInfo[i].bufId;
			Logger::getInstance()->log(Logger::DEBUG, "getUnusedBuffer(->%d)\n", *bufId);
			return true;
		}
	}

	return false;
}
