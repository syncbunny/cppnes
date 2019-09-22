#ifndef APU_H
#define APU_H

#include <cstdint>

class APU {
public:
	APU();
	virtual ~APU();

public:
	virtual void clock();
	virtual void setSW1C1(uint8_t val) {
		mSW1C1 = val;
	}
	virtual void setSW1C2(uint8_t val) {
		mSW1C2 = val;
	}
	virtual void setSW1FQ1(uint8_t val);
	virtual void setSW1FQ2(uint8_t val);
	virtual void setSW2C1(uint8_t val) {
		mSW2C1 = val;
	}
	virtual void setSW2C2(uint8_t val) {
		mSW2C2 = val;
	}
	virtual void setSW2FQ1(uint8_t val);
	virtual void setSW2FQ2(uint8_t val);
	virtual void setTWC(uint8_t val);
	virtual void setTWFQ1(uint8_t val);
	virtual void setTWFQ2(uint8_t val);
	virtual void setNZC(uint8_t val) {
		mNZC = val;
	}
	virtual void setNZFQ1(uint8_t val) {
		mNZFQ1 = val;
	}
	virtual void setNZFQ2(uint8_t val) {
		mNZFQ1 = val;
	}
	virtual void setDMC1(uint8_t val) {
		mDMC1 = val;
	}
	virtual void setDMC2(uint8_t val) {
		mDMC2 = val;
	}
	virtual void setDMC3(uint8_t val) {
		mDMC3 = val;
	}
	virtual void setDMC4(uint8_t val) {
		mDMC4 = val;
	}
	virtual void setChCtrl(uint8_t val);
	virtual uint8_t getChCtrl() const {
		return mChCtrl;
	}
	virtual void setFrameCounter(uint8_t val);

protected:
	virtual void triangleClock();
	virtual void frameClock();
	virtual void triangleLinerCounterClock();
	virtual void render();

protected:
	enum {
		DATA_LENGTH = 44100,
		CLOCK_DATA_LENGTH = 184,
	};

protected:
	uint8_t mSW1C1;        // 0x4000
	uint8_t mSW1C2;        // 0x4001
	uint8_t mSW1FQ1;       // 0x4002
	uint8_t mSW1FQ2;       // 0x4003
	uint8_t mSW2C1;        // 0x4004
	uint8_t mSW2C2;        // 0x4005
	uint8_t mSW2FQ1;       // 0x4006
	uint8_t mSW2FQ2;       // 0x4007
	uint8_t mTWC;          // 0x4008
	uint8_t mTWFQ1;        // 0x400A
	uint8_t mTWFQ2;        // 0x400B
	uint8_t mNZC;          // 0x400C
	uint8_t mNZFQ1;        // 0x400E
	uint8_t mNZFQ2;        // 0x400F
	uint8_t mDMC1;         // 0x4010
	uint8_t mDMC2;         // 0x4011
	uint8_t mDMC3;         // 0x4012
	uint8_t mDMC4;         // 0x4013
	uint8_t mChCtrl;       // 0x4015
	uint8_t mFrameCounter; // 0x4017

	short* mData;
	int mWritePoint;
	int mWriteLen;
	int mReadPoint;

	int mSWClock;
	int mSW1FQ;
	int mSW1Len;
	int mSW2FQ;
	int mSW2Len;

	int mTDClk;
	int mTSeq[32];                                    
	int mTSeqIndex;
	int mTFQ;
	int mTTimer;
	int mTChVal;

	int mTLen;
	int mTLCnt;
	int mTLCntReload;

	// FrameSequencer stuff
	int mDFrameClock;
	int mFrameSQCount; // [0, 1, 2, 3] or [0, 1, 2, 3, 4]

	int mRenderClock;
	int mNextRenderClock;
	int mCPUfq;

	class Envelope;

	class Square {
	public:
		Square(uint8_t& reg1, int& len, int& fq, APU::Envelope* env);
		virtual ~Square();

	public:
		void clock();
		int val() const {
			return mVal;
		}

	protected:
		uint8_t& mReg1; // $4000, $4004
		int& mLen;
		int& mFQ;

		APU::Envelope* mEnv;
		int mDClock;
		int mSQIndex;
		int mVal;
	};

	class Noise {
	public:
		Noise(Envelope* env);
		virtual ~Noise();

	public:
		APU::Envelope* mEnv;
	};

	class Sweep {
	public:
		Sweep(uint8_t& reg, int& fq, int& len);
		virtual ~Sweep();

	public:
		void reset();
		void clock();

	protected:
		uint8_t& mReg;
		int& mFQ;
		int& mSWLen;

		int mDClock;
		int mDirection; // 0: down, 1: up
	};

	class Envelope {
	public:
		Envelope(uint8_t& reg);
		virtual ~Envelope();

	public:
		void reset();
		void clock();
		uint8_t getVol();

	protected:
		uint8_t& mReg;

		int mDClock;
		uint8_t   mVal;
		bool mReset;
	};

	Sweep*    mSweep1;
	Envelope* mEnv1;
	Square*   mSquare1;

	Sweep*    mSweep2;
	Envelope* mEnv2;
	Square*   mSquare2;

	Noise*    mNoise;
	Envelope* mNoiseEnv;
};

#endif
