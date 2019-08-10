#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfwrenderer.h"
#include "nes.h"
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
	nes->reset();

	while(true) {
		nes->clock();
	}
	return 0;
}

bool analyzeOpt(int argc, char* argv[]) {
	Config* conf = Config::getInstance();
	int opt;
	opterr = 0;

	while((opt = getopt(argc, argv, "v")) != -1) {
		switch(opt) {
		case 'v':
			conf->setVarbose(true);
			break;
		}
	}

	for (int i = optind; i < argc; i++) {
		conf->setROMPath(std::string(argv[i]));
	}
	if (conf->getROMPath() == "") {
		fprintf(stderr, "Usage: nes <rompath>\n");
		return false;
	}

	return true;
}
