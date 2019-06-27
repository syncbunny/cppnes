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
		if (n == 0) mA[0] = b;
		if (n == 1) mA[1] = b;
	}
	virtual void setB(int n, bool b) {
		if (n == 0) mB[0] = b;
		if (n == 1) mB[1] = b;
	}
	virtual void setSelect(int n, bool b) {
		if (n == 0) mSelect[0] = b;
		if (n == 1) mSelect[1] = b;
	}
	virtual void setStart(int n, bool b) {
		if (n == 0) mStart[0] = b;
		if (n == 1) mStart[1] = b;
	}
	virtual void setUp(int n, bool b) {
		if (n == 0) mUp[0] = b;
		if (n == 1) mUp[1] = b;
	}
	virtual void setDown(int n, bool b) {
		if (n == 0) mDown[0] = b;
		if (n == 1) mDown[1] = b;
	}
	virtual void setLeft(int n, bool b) {
		if (n == 0) mLeft[0] = b;
		if (n == 1) mLeft[1] = b;
	}
	virtual void setRight(int n, bool b) {
		if (n == 0) mRight[0] = b;
		if (n == 1) mRight[1] = b;
	}

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
};

#endif
