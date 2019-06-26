#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>

class Renderer {
public:
	Renderer();
	virtual ~Renderer();

public:
	virtual void render(const uint8_t* p);
};

#endif
