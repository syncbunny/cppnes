#include <string>
#include <unistd.h>
#include "nes.h"

struct Config {
	std::string romPath;
};

bool analyzeOpt(int argc, char* argv[], Config& conf);

int main(int argc, char* argv[]) {
	Config conf;
	
	if (!analyzeOpt(argc, argv, conf)) {
		return 0;
	}

	NES* nes = new NES();

	nes->powerOn();
	nes->loadCartridge(conf.romPath.c_str());
	nes->reset();
	while(true) {
		nes->clock();
	}

	return 0;
}

bool analyzeOpt(int argc, char* argv[], Config& conf) {
	if (argc != 2) {
		fprintf(stderr, "Usage: nes <rompath>\n");
		return false;
	}
	conf.romPath = std::string(argv[1]);

	return true;
}
