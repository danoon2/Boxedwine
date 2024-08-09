#include "boxedwine.h"
#include "x11.h"

XPropertyPtr XProperties::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.get(atom);
}

void XProperties::setProperty(U32 atom, U32 type, U32 format, U32 length, U8* value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	U8* oldValue = value;
	value = new U8[length];
	memcpy(value, oldValue, length);
	XPropertyPtr property = std::make_shared<XProperty>(type, format, length, value);
	properties.set(atom, property);
}

void XProperties::setProperty(U32 atom, U32 type, U32 format, U32 length, U32 address) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	U8* value = new U8[length];
	KThread::currentThread()->memory->memcpy(value, address, length);
	XPropertyPtr property = std::make_shared<XProperty>(type, format, length, value);
	properties.set(atom, property);
}

void XProperties::deleteProperty(U32 atom) {
	properties.remove(atom);
}

U32 XTextProperty::byteLen(KMemory* memory) {
	U32 result = 0;
	U32 address = value;

	for (U32 i = 0; i < nitems; i++) {
		while (true) {
			U32 v = 0;

			switch (format) {
			case 8:
				v = memory->readb(address);
				address++;
				result++;
				break;
			case 16:
				v = memory->readw(address);
				address += 2;
				result += 4;
				break;
			case 32:
				v = memory->readd(address);
				address += 4;
				result += 4;
				break;
			default:
				kpanic("XTextProperty.len bad format %d", format);
			}
			if (v == 0) {
				break;
			}
		}
	}
	return result;
}

void XTextProperty::create(KThread* thread, U32 encoding, S8** list, U32 count, XTextProperty* property) {
	U32 len = 0;
	KMemory* memory = thread->memory;

	for (U32 i = 0; i < count; i++) {
		len += (U32)strlen((const char*)list[i]) + 1;
	}
	U32 value = thread->process->alloc(thread, len);
	property->value = value;
	for (U32 i = 0; i < count; i++) {
		U32 len = (U32)strlen((const char*)list[i]) + 1;
		memory->memcpy(value, list[i], len);
		value += len;
	}
	property->nitems = count;
	property->encoding = encoding;
	property->format = 8;
}

void XTextProperty::create(KThread* thread, U32 encoding, U32 list, U32 count, XTextProperty* property) {
	U32 len = 0;
	KMemory* memory = thread->memory;

	for (U32 i = 0; i < count; i++) {
		len += memory->strlen(memory->readd(list + i*4)) + 1;
	}
	U32 value = thread->process->alloc(thread, len);
	property->value = value;
	for (U32 i = 0; i < count; i++) {
		U32 src = memory->readd(list + i * 4);
		U32 len = memory->strlen(src) + 1;
		memory->memcpy(value, src, len);
		value += len;
	}	
	property->nitems = count;
	property->encoding = encoding;
	property->format = 8;
}