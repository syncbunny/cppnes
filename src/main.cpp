#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfwrenderer.h"
#include "nes.h"
#include "pad.h"

#define GL_SILENCE_DEPRECATION 1

struct Config {
	std::string romPath;
};

bool analyzeOpt(int argc, char* argv[], Config& conf);

int main(int argc, char* argv[]) {
	Config conf;
	
	if (!analyzeOpt(argc, argv, conf)) {
		return 0;
	}

	GLFWRenderer* renderer = new GLFWRenderer();
	NES* nes = new NES(renderer);
	renderer->bindPAD(nes->getPAD());

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
