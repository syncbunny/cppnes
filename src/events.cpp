#include "events.h"

EventQueue* EventQueue::instance = 0;

void EventQueue::push(Event* evt) {
	mQueue.push_back(evt);
}

Event* EventQueue::pop() {
	Event* ret = 0;

	if (!mQueue.empty()) {	
		mQueue.front();
		mQueue.pop_front();
	}

	return ret;
}
