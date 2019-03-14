#include <stdio.h>
#include "events.h"

EventQueue* EventQueue::instance = 0;

void EventQueue::push(Event* evt) {
	printf("EventQueue::push (%d)\n", evt->getType());
	mQueue.push_back(evt);
}

Event* EventQueue::pop() {
	Event* ret = 0;

	if (!mQueue.empty()) {	
		ret = mQueue.front();
		mQueue.pop_front();
		printf("EventQueue::pop (%d)\n", ret->getType());
	}

	return ret;
}
