#include "boxedwine.h"
#include "x11.h"

void DisplayData::putEvent(const XEvent& event) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMutex);
	eventQueue.push_back(event);
}

U32 DisplayData::lockEvents() {
	BOXEDWINE_MUTEX_LOCK(eventMutex);
	return (U32)eventQueue.size();
}

XEvent* DisplayData::getEvent(U32 index) {
	if (index >= eventQueue.size()) {
		return nullptr;
	}
	return &eventQueue.at(index);
}

void DisplayData::removeEvent(U32 index) {
	if (index >= eventQueue.size()) {
		return;
	}
	eventQueue.erase(eventQueue.begin()+index);
}

void DisplayData::unlockEvents() {
	BOXEDWINE_MUTEX_UNLOCK(eventMutex);
}

U32 DisplayData::getEventMask(U32 window) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	U32 result = 0;
	perWindowEventMask.get(window, result);
	return result;
}

void DisplayData::setEventMask(U32 window, U32 mask) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	perWindowEventMask.set(window, mask);
}

U32 DisplayData::getNextEventSerial() {
	return ++nextEventSerial;
}

int DisplayData::getContextData(U32 context, U32 contextType, U32& ptr) {
	ContextDataPtr data = contextData.get(context);
	if (!data) {
		return XCNOENT;
	}
	if (!data->get(contextType, ptr)) {
		return XCNOENT;
	}
	return XCSUCCESS;
}

int DisplayData::setContextData(U32 context, U32 contextType, U32 ptr) {
	ContextDataPtr data = contextData.get(context);
	if (!data) {
		data = std::make_shared<ContextData>();
		contextData.set(context, data);
	}
	data->put(contextType, ptr);
	return XCSUCCESS;
}

int DisplayData::deleteContextData(U32 context, U32 contextType) {
	ContextDataPtr data = contextData.get(context);
	if (!data) {
		return XCNOENT;
	}
	if (!data->remove(contextType)) {
		return XCNOENT;
	}
	return XCSUCCESS;
}