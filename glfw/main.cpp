#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfwrenderer.h"

#define GL_SILENCE_DEPRECATION 1

int main() {
	GLFWRenderer* renderer = new GLFWRenderer();
	while(true) {
		renderer->render(0);
	}
	return 0;
}

