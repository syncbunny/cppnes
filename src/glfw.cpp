#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfw.h"

GLFW::GLFW() {
	initialize();
}

GLFW::~GLFW() {
	glfwTerminate();
}

void GLFW::initialize() {
	// init glfw
	if (glfwInit() == GL_FALSE) {
		throw std::runtime_error("glfwInit failed.");
	}
}

GLuint GLFW::createProgcamFromFile(const char* vsPath, const char* fsPath) {
	const GLuint program(glCreateProgram());

	if (vsPath) {
		const std::string vsStr = this->loadSourceFromFile(vsPath);
		const GLchar* vsSrc = vsStr.c_str();

		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsSrc, 0);
		glCompileShader(vobj);

		if (this->printShaderInfoLog(vobj, "vertex shader") == 0) {
			glAttachShader(program, vobj);
		}
		glDeleteShader(vobj);
	}

	if (fsPath) {
		const std::string fsStr = this->loadSourceFromFile(fsPath);
		const GLchar* fsSrc = fsStr.c_str();

		const GLuint vobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(vobj, 1, &fsSrc, 0);
		glCompileShader(vobj);

		if (this->printShaderInfoLog(vobj, "fragment shader") == 0) {
			glAttachShader(program, vobj);
		}
		glDeleteShader(vobj);
	}

	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragment");
	glLinkProgram(program);

	if (this->printProgramInfoLog(program) == 0) {
		return program;
	} else {
		glDeleteProgram(program);
		return 0;
	}
	this->checkError("createProgramFromFile");
}

GLenum GLFW::checkError(const char* msg) {
	GLenum err = glGetError();
	switch (err) {
	case GL_NO_ERROR:
		/* NOP */
		break;
	case GL_INVALID_ENUM:
		std::cerr << "GL(" << msg << "): GL_INVALID_ENUM" << std::endl;
		break;
	case GL_INVALID_VALUE:
		std::cerr << "GL(" << msg << "): GL_INVALID_VALUE" << std::endl;
		break;
	case GL_INVALID_OPERATION:
		std::cerr << "GL(" << msg << "): GL_INVALID_OPERATION" << std::endl;
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		std::cerr << "GL(" << msg << "): GL_INVALID_ FRAMEBUFFER_OPERATION" << std::endl;
		break;
	case GL_OUT_OF_MEMORY:
		std::cerr << "GL(" << msg << "): GL_OUT_OF_MEMORY" << std::endl;
		break;
	default:
		std::cerr << "GL: unknow error(" << err << ")" << std::endl;
		break;
	}
	return err;
}

GLsizei GLFW::printShaderInfoLog(GLuint shader, const char* str) {
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		std::cerr << "Compile Error in " << str << std::endl;
	}

	GLsizei bufSize;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1) {
		std::vector<GLchar> infoLog(bufSize);
		GLsizei length;
		glGetShaderInfoLog(shader, bufSize, &length, &infoLog[0]);
		std::cerr << &infoLog[0] << std::endl;
	}

	return bufSize;
}

GLsizei GLFW::printProgramInfoLog(GLuint program) {
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		std::cerr << "Link error." << std::endl;
	}

	GLsizei bufSize;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufSize);
	if (bufSize > 1) {
		std::vector<GLchar> infoLog(bufSize);
		GLsizei length;
		glGetProgramInfoLog(program, bufSize, &length, &infoLog[0]);
		std::cerr << &infoLog[0] << std::endl;
	}

	return bufSize;
}

std::string GLFW::loadSourceFromFile(const char* path) {
	std::ifstream ifs(path);
	std::string ret;
	std::string str;
	
	if (ifs.fail()) {
		throw std::runtime_error(std::string("open ") + std::string(path) + std::string(" failed."));
	}
	while (getline(ifs, str)) {
		ret += str + "\n";
	}

	return ret;
}

