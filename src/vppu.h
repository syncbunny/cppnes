#ifndef VPPU_H
#define VPPU_H

#include "ppu.h"

class VPPU:public PPU {
public:
	VPPU();
	virtual ~VPPU();

public:
	virtual void setMirror(int m);
	virtual void write(uint8_t val);
	virtual uint8_t getSR();

protected:
	virtual void startVR();
	virtual void frameEnd();
	virtual void writeFrame();
};

#endif
