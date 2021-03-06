#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glfwrenderer.h"
#include "pad.h"
#include "events.h"

GLFWRenderer::GLFWRenderer()
:Renderer(), mGLFW(0), mWindow(0) {
	mGLFW = new GLFW();
	createWindow();
	createObj();
	createTexture();
	mProg = mGLFW->createProgcamFromFile("base.vs", "base.fs");
	mPAD = 0;
}

GLFWRenderer::~GLFWRenderer() {
	if (mGLFW) {
		delete mGLFW;
	}
}

void GLFWRenderer::render(const uint8_t* p) {
	memcpy(mPix, p, 256*240*3);

	glBindTexture(GL_TEXTURE_2D, mTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, mPix);

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(mProg);

	glBindVertexArray(mVAO);
	mGLFW->checkError("glBindTexture");
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	mGLFW->checkError("DrawArrays");
	glBindVertexArray(0);
	mGLFW->checkError("glVertexArray");

	glfwSwapBuffers(mWindow);
	glfwPollEvents();
	
	this->setPAD();
	this->checkSPKey();

	mGLFW->checkError();
}

void GLFWRenderer::createWindow() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mWindow = glfwCreateWindow(256, 240, "NES", 0, 0);
	if (mWindow == 0) {
		throw std::runtime_error("glfwCreateWindow failed.");
	}
	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("glewInit failed.");
	}
}

void GLFWRenderer::createObj() {
	GLfloat positionData[] = {
		-1.0f, +1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		+1.0f, +1.0f, 0.0f,
		+1.0f, -1.0f, 0.0f,
	};
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glGenBuffers(1, &mVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*4, positionData, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	mGLFW->checkError("createObj");
}

void GLFWRenderer::createTexture() {
	mPix = new uint8_t[256*240*3];

	glGenTextures(1, &mTex);
	glBindTexture(GL_TEXTURE_2D, mTex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, mPix);
	glBindTexture(GL_TEXTURE_2D, 0);
	mGLFW->checkError("createTexture");
}

void GLFWRenderer::setPAD() {
	if (mPAD == 0) {
		return;
	}

	mPAD->setStart(0, glfwGetKey(mWindow, GLFW_KEY_ENTER));
	mPAD->setSelect(0, glfwGetKey(mWindow, GLFW_KEY_RIGHT_SHIFT));
	mPAD->setA(0, glfwGetKey(mWindow, GLFW_KEY_A));
	mPAD->setB(0, glfwGetKey(mWindow, GLFW_KEY_S));
	mPAD->setUp(0, glfwGetKey(mWindow, GLFW_KEY_UP));
	mPAD->setDown(0, glfwGetKey(mWindow, GLFW_KEY_DOWN));
	mPAD->setLeft(0, glfwGetKey(mWindow, GLFW_KEY_LEFT));
	mPAD->setRight(0, glfwGetKey(mWindow, GLFW_KEY_RIGHT));
}

void GLFWRenderer::checkSPKey() {
	if (glfwGetKey(mWindow, GLFW_KEY_C)) {
		EventQueue& eq = EventQueue::getInstance();
		eq.push(new EventCapture());		
	}

	if (glfwGetKey(mWindow, GLFW_KEY_D)) {
		EventQueue& eq = EventQueue::getInstance();
		eq.push(new EventCoreDump());		
	}

	if (glfwGetKey(mWindow, GLFW_KEY_K)) {
		EventQueue& eq = EventQueue::getInstance();
		eq.push(new EventKill());		
	}
}
