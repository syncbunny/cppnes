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

protected:
	virtual void resetIn();

protected:
	uint8_t mLastOut;
	uint8_t mInCount1;
	uint8_t mInCount2;
};

#endif
