#ifndef MAPPER_H
#define MAPPER_H

#include <cstdint>

class Mapper {
public:
	Mapper();
	virtual ~Mapper();

public:
	virtual void setWRAM(uint8_t* addr);
	virtual void setPROM(uint8_t* addr, std::size_t size);
	virtual void setCROM(uint8_t* addr, std::size_t size);
	virtual void setPPU(class PPU* ppu);
	virtual void setAPU(class APU* apu);
	virtual void setPAD(class PAD* pad);
	virtual void setNo(uint8_t no);
	virtual void write1Byte(uint16_t addr, uint8_t val);
	virtual void write2Bytes(uint16_t addr, uint16_t val);
	virtual uint8_t read1Byte(uint16_t addr);
	virtual uint16_t read2Bytes(uint16_t addr);
	virtual void push2Bytes(uint16_t addr, uint16_t val);
	virtual uint16_t pop2Bytes(uint16_t addr);

protected:
	uint8_t mNo;
	uint8_t* mWRAM;
	uint8_t* mPROM;
	uint8_t* mCROM;
	class PPU* mPPU;
	class APU* mAPU;
	class PAD* mPAD;
};

#endif
