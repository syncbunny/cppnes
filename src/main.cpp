#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfwrenderer.h"
#include "nes.h"
#include "core.h"
#include "config.h"
#include "pad.h"

#define GL_SILENCE_DEPRECATION 1

bool analyzeOpt(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	if (!analyzeOpt(argc, argv)) {
		return 0;
	}

	GLFWRenderer* renderer = new GLFWRenderer();
	NES* nes = new NES(renderer);
	renderer->bindPAD(nes->getPAD());

	Config* conf = Config::getInstance();
	nes->powerOn();
	nes->loadCartridge(conf->getROMPath().c_str());
	if (conf->getCorePath() != "") {
		Core* core = Core::load(conf->getCorePath().c_str());
		nes->loadCore(core);
		delete core;
	} else {
		nes->reset();
	}

	while(true) {
		nes->clock();
	}
	return 0;
}

bool analyzeOpt(int argc, char* argv[]) {
	Config* conf = Config::getInstance();
	int opt;
	opterr = 0;

	while((opt = getopt(argc, argv, "c:v")) != -1) {
		switch(opt) {
		case 'c':
			conf->setCorePath(std::string(optarg));
			break;
		case 'g':
			conf->setProfileEnabled(true);
			break;
		case 'v':
			conf->setVarbose(true);
			break;
		default:
			fprintf(stderr, "Usage: nes [-v] [-c <corepath>] <rompath>\n");
			return false;
			break;
		}
	}

	for (int i = optind; i < argc; i++) {
		conf->setROMPath(std::string(argv[i]));
	}
	if (conf->getROMPath() == "") {
		fprintf(stderr, "Usage: nes [-v] [-c <corepath>] <rompath>\n");
		return false;
	}

	return true;
}
