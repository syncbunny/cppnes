#ifndef PAD_H
#define PAD_H

#include <cstdint>

class PAD {
public:
	PAD();
	virtual ~PAD();

public:
	virtual void out(uint8_t val); // CPU: 0x4016 (write)
	virtual uint8_t in1(); // CPU: 0x4016 (read)
	virtual uint8_t in2(); // CPU: 0x4017 (read)
	virtual void setA(int n, bool b) {
		if (n == 0) mA_[0] = b;
		if (n == 1) mA_[1] = b;
	}
	virtual void setB(int n, bool b) {
		if (n == 0) mB_[0] = b;
		if (n == 1) mB_[1] = b;
	}
	virtual void setSelect(int n, bool b) {
		if (n == 0) mSelect_[0] = b;
		if (n == 1) mSelect_[1] = b;
	}
	virtual void setStart(int n, bool b) {
		if (n == 0) mStart_[0] = b;
		if (n == 1) mStart_[1] = b;
	}
	virtual void setUp(int n, bool b) {
		if (n == 0) mUp_[0] = b;
		if (n == 1) mUp_[1] = b;
	}
	virtual void setDown(int n, bool b) {
		if (n == 0) mDown_[0] = b;
		if (n == 1) mDown_[1] = b;
	}
	virtual void setLeft(int n, bool b) {
		if (n == 0) mLeft_[0] = b;
		if (n == 1) mLeft_[1] = b;
	}
	virtual void setRight(int n, bool b) {
		if (n == 0) mRight_[0] = b;
		if (n == 1) mRight_[1] = b;
	}
	virtual void strobe();

protected:
	virtual void resetIn();

protected:
	uint8_t mLastOut;
	uint8_t mInCount1;
	uint8_t mInCount2;

	bool mA[2];
	bool mB[2];
	bool mSelect[2];
	bool mStart[2];
	bool mUp[2];
	bool mDown[2];
	bool mLeft[2];
	bool mRight[2];

	bool mA_[2];
	bool mB_[2];
	bool mSelect_[2];
	bool mStart_[2];
	bool mUp_[2];
	bool mDown_[2];
	bool mLeft_[2];
	bool mRight_[2];
};

#endif
