#ifndef VPPU_H
#define VPPU_H

#include "ppu.h"

class VPPU:public PPU {
public:
	VPPU();
	virtual ~VPPU();

public:
	virtual void write(uint8_t val);
};

#endif
