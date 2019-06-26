#ifndef RENDERER_H
#define RENDERER_H

class Renderer {
public:
	Renderer();
	virtual ~Renderer();

public:
	virtual void render();
};

#endif
