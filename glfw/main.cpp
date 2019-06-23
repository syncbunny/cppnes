#include <iostream>
#include <GLFW/glfw3.h>

#define GL_SILENCE_DEPRECATION 1

int main() {
	if (glfwInit() == GL_FALSE) {
		std::cerr << "glfwInit failed." << std::endl;
	}

	GLFWwindow* const window(glfwCreateWindow(256, 240, "NES", 0, 0));
	if (window == 0) {
		std::cerr << "glfwCreateWindow failed." << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	while(glfwWindowShouldClose(window) == GL_FALSE) {
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	return 0;
}
