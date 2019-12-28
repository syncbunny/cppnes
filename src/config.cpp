#include "config.h"
#include "logger.h"

Config::Config() {
	mVarbose = false;
	mProfileEnabled = false;
	mLoglevel = Logger::NONE;
}

Config::~Config() {
}
