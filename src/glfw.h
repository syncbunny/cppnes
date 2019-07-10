#ifndef GLFW_H
#define GLFW_H

#include <string>
#include <GLFW/glfw3.h>

class GLFW {
public:
	GLFW();
	virtual ~GLFW();

public:
	GLuint createProgcamFromFile(const char* vsPath, const char* fsPath);
	GLenum checkError(const char* msg = "");

protected:
	void initialize();
	GLsizei printShaderInfoLog(GLuint shader, const char* str);
	GLsizei printProgramInfoLog(GLuint program);
	std::string loadSourceFromFile(const char* path);
};

#endif
