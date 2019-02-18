#ifndef VMAPPER_H
#define VMAPPER_H

#include "mapper.h"

class VMapper: public Mapper {
public:
	VMapper();
	virtual ~VMapper();

public:
	virtual void setPROM(uint8_t* addr, std::size_t size);
	virtual void setCROM(uint8_t* addr, std::size_t size);
	virtual void setNo(uint8_t no);
	virtual uint8_t read1Byte(uint16_t addr);
	virtual uint16_t read2Bytes(uint16_t addr);
	virtual void write1Byte(uint16_t addr, uint8_t val);
	virtual void push2Bytes(uint16_t addr, uint16_t val);
	virtual uint16_t pop2Bytes(uint16_t addr);
};

#endif
