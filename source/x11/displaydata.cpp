#include "boxedwine.h"
#include "x11.h"
#include "kunixsocket.h"

DisplayData::DisplayData()
#ifndef BOXEDWINE_MULTI_THREADED
	: cond(std::make_shared<BoxedWineCondition>(B("DisplayData::lockCond"))),
	eventCond(std::make_shared<BoxedWineCondition>(B("DisplayData::eventCond")))
#endif
{
}

void DisplayData::putEvent(const XEvent& event, bool inFront) {
	{
		BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMutex);
		if (inFront) {
			eventQueue.push_front(event);
		} else {
			eventQueue.push_back(event);
		}
		KProcessPtr process = this->process.lock();
		if (!process) {
			return;
		}
		KFileDescriptorPtr server = process->getFileDescriptor(this->serverFd);
		KFileDescriptorPtr client = process->getFileDescriptor(this->clientFd);
		if (server && server->kobject->type == KTYPE_UNIX_SOCKET) {
			std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(server->kobject);
			std::shared_ptr<KUnixSocketObject> c = std::dynamic_pointer_cast<KUnixSocketObject>(client->kobject);
			if (c && !c->isReadReady()) {
				s->writeNative((U8*)"1", 1); // so that poll will work
			}
		}
	}
}

bool DisplayData::findAndRemoveEvent(U32 window, U32 type, XEvent& event) {
#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMutex);
#endif
	U32 count = (U32)eventQueue.size();
	for (U32 i = 0; i < count; i++) {
		XEvent* e = &eventQueue.at(i);
		if (e->type == type && e->xany.window == window) {
			event = *e;
			removeEvent(i);
			return true;
		}
	}
	return false;
}

U32 DisplayData::lockEvents() {
	
#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_MUTEX_LOCK(eventMutex);
#else
	if (eventQueueIsLocked) {
		eventCond->wait();
		return 0;
	}
	eventQueueIsLocked = true;
#endif
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
	if (eventQueue.empty()) {
		KProcessPtr process = this->process.lock();
		if (!process) {
			return;
		}
		KFileDescriptorPtr fd = process->getFileDescriptor(this->clientFd);
		if (fd && fd->kobject->type == KTYPE_UNIX_SOCKET) {
			std::shared_ptr<KUnixSocketObject> s = std::dynamic_pointer_cast<KUnixSocketObject>(fd->kobject);
			while (s->isReadReady()) {
				U8 b;
				if (s->readNative(&b, 1) != 1) {
					break;
				}
			}
		}
	}
}

void DisplayData::unlockEvents() {
#ifdef BOXEDWINE_MULTI_THREADED
	BOXEDWINE_MUTEX_UNLOCK(eventMutex);
#else
	eventQueueIsLocked = false;
	eventCond->signal();
#endif
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

U32 DisplayData::getInput2Mask(U32 window) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	U32 result = 0;
	perWindowEventMask2.get(window, result);
	return result;
}

int DisplayData::setInput2Mask(U32 window, U32 mask) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(eventMaskMutex);
	perWindowEventMask2.set(window, mask);
	return Success;
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

void DisplayData::close(KThread* thread) {
	if (xrrData) {
		thread->process->free(xrrData->ratesAddress);
		thread->process->free(xrrData->sizesAddress);
		delete xrrData;
		xrrData = nullptr;
	}
}