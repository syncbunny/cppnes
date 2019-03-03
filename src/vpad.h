#ifndef VPAD_H
#define VPAD_H

#include "pad.h"

class VPAD:public PAD {
public:
	VPAD();
	virtual ~VPAD();

public:
	void out(uint8_t val);
	uint8_t in1();
};

#endif
