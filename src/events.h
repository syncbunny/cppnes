#ifndef EVENTS_H
#define EVENTS_H

#include <cstdint>
#include <deque>

class EventQueue {
protected:
	EventQueue() {
	}
	virtual ~EventQueue() {
	}
	static EventQueue* instance;

public:
	static EventQueue& getInstance() {
		if (instance == 0) {
			instance = new EventQueue();
		}
		return *instance;
	}
public:
	void push(class Event* evt);
	class Event* pop();

protected:
	std::deque<class Event*> mQueue;
};

class Event {
public:
	enum {
		TYPE_NMI,
		TYPE_DMA,

		TYPE_CAPTURE,
		TYPE_COREDUMP,
		TYPE_KILL,
	};
protected:
	Event(uint16_t type)
        : mType(type) {
	}
public:
	virtual ~Event() {
	}

public:
	uint16_t getType() const {
		return mType;
	}
protected:
	uint16_t mType;
};

class EventNMI: public Event {
public:
	EventNMI()
        :Event(TYPE_NMI) {
	}
};

class EventDMA: public Event {
public:
	EventDMA(uint16_t startAddr)
	:Event(TYPE_DMA), mStartAddr(startAddr) {
	}

protected:
	uint16_t mStartAddr;
};

class EventCapture: public Event {
public:
	EventCapture()
	:Event(TYPE_CAPTURE) {
	}
};

class EventCoreDump: public Event {
public:
	EventCoreDump()
	:Event(TYPE_COREDUMP) {
	}
};

class EventKill: public Event {
public:
	EventKill()
	:Event(TYPE_KILL) {
	}
};

#endif
