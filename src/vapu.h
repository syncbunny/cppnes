#ifndef VAPU_H
#define VAPU_H

#include "apu.h"

class VAPU: public APU {
public:
	virtual void setSW1C1(uint8_t val) {
		printf("APU::setSW1C1(%02x)\n", val);
		APU::setSW1C1(val);
	}
	virtual void setSW1C2(uint8_t val) {
		printf("APU::setSW1C2(%02x)\n", val);
		APU::setSW1C2(val);
	}
	virtual void setSW1FQ1(uint8_t val) {
		printf("APU::setSW1FQ1(%02x)\n", val);
		APU::setSW1FQ1(val);
	}
	virtual void setSW1FQ2(uint8_t val) {
		printf("APU::setSW1FQ2(%02x)\n", val);
		APU::setSW1FQ2(val);
	}
	virtual void setSW2C1(uint8_t val) {
		printf("APU::setSW2C1(%02x)\n", val);
		APU::setSW2C1(val);
	}
	virtual void setSW2C2(uint8_t val) {
		printf("APU::setSW2C2(%02x)\n", val);
		APU::setSW2C2(val);
	}
	virtual void setSW2FQ1(uint8_t val) {
		printf("APU::setSW2FQ1(%02x)\n", val);
		APU::setSW2FQ1(val);
	}
	virtual void setSW2FQ2(uint8_t val) {
		printf("APU::setSW2FQ2(%02x)\n", val);
		APU::setSW2FQ2(val);
	}
	virtual void setTWC(uint8_t val) {
		printf("APU::setTWC(%02x)\n", val);
		APU::setTWC(val);
	}
	virtual void setTWFQ1(uint8_t val) {
		printf("APU::setTWFQ1(%02x)\n", val);
		APU::setTWFQ1(val);
	}
	virtual void setTWFQ2(uint8_t val) {
		printf("APU::setTWFQ2(%02x)\n", val);
		APU::setTWFQ2(val);
	}
	virtual void setNZC(uint8_t val) {
		printf("APU::setNZC(%02x)\n", val);
		APU::setNZC(val);
	}
	virtual void setNZFQ1(uint8_t val) {
		printf("APU::setNZFQ1(%02x)\n", val);
		APU::setNZFQ1(val);
	}
	virtual void setNZFQ2(uint8_t val) {
		printf("APU::setNZFQ2(%02x)\n", val);
		APU::setNZFQ2(val);
	}
	virtual void setDMC1(uint8_t val) {
		printf("APU::setDMC1(%02x)\n", val);
		APU::setDMC1(val);
	}
	virtual void setDMC2(uint8_t val) {
		printf("APU::setDMC2(%02x)\n", val);
		APU::setDMC2(val);
	}
	virtual void setDMC4(uint8_t val) {
		printf("APU::setDMC4(%02x)\n", val);
		APU::setDMC4(val);
	}
	virtual void setChCtrl(uint8_t val) {
		printf("APU::setChCtrl(%02x)\n", val);
		APU::setChCtrl(val);
	}
	virtual uint8_t getChCtrl() {
		uint8_t ret = APU::getChCtrl();
		printf("APU::getChCtrl() = %02x\n", ret);
		return ret;
	}
	virtual void setFrameCounter(uint8_t val) {
		printf("APU::setFrameCounter(%02x)\n", val);
		APU::setFrameCounter(val);
	}
};

#endif
