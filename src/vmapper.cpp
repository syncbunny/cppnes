#include <stdio.h>
#include "vmapper.h"

typedef Mapper super;

VMapper::VMapper() {
}

VMapper::~VMapper() {
}

void VMapper::setPROM(uint8_t* base, std::size_t size) {
	printf("Mapper::setPROM(%p, %ld)\n", base, size);
	super::setPROM(base, size);
}

void VMapper::setCROM(uint8_t* base, std::size_t size) {
	printf("Mapper::setCROM(%p, %ld)\n", base, size);
	super::setCROM(base, size);
}

void VMapper::setNo(uint8_t no) {
	printf("Mapper::setNo(%d)\n", no);
	super::setNo(no);
}

uint8_t VMapper::read1Byte(uint16_t addr) {
	printf("Mapper::read1Bytes(%02x)", addr);
	uint8_t ret = super::read1Byte(addr);
	printf(" => %02X\n", ret);
	return ret;
}

uint16_t VMapper::read2Bytes(uint16_t addr) {
	printf("Mapper::read2Bytes(%02x)", addr);
	uint16_t ret = super::read2Bytes(addr);
	printf(" => %04x\n", ret);
	return ret;
}
