#include <stdio.h>
#include <stdarg.h>
#include "logger.h"
#include "config.h"

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::log(int lvl, const char* fmt, ...) {
	Config* conf = Config::getInstance();
	if (lvl < conf->getLoglevel()) {
		return;
	}

	va_list argp;
	va_start(argp, fmt);
	vprintf(fmt, argp);
	va_end(argp);
}
