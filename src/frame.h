#ifndef FRAME_H
#define FRAME_H

class Frame {
protected:
	Frame();
public:
	virtual ~Frame();

public:
	static Frame* getInstance() {
		static Frame* instance = 0;
		if (!instance) {
			instance = new Frame();
		}
		return instance;
	}
	void start();

protected:
	void _start();
	static void _entryPoint(Frame* frame);
};

#endif
