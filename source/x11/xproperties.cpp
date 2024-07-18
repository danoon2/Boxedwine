#include "boxedwine.h"
#include "xproperties.h"

XPropertyPtr XProperties::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.get(atom);
}

void XProperties::setProperty(U32 atom, U32 type, U32 format, U32 length, U8* value, bool needsCopy) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	if (needsCopy) {
		U8* oldValue = value;
		value = new U8[length];
		memcpy(value, oldValue, length);
	}
	XPropertyPtr property = std::make_shared<XProperty>(type, format, length, value, !needsCopy);
	properties.set(atom, property);
}

void XProperties::deleteProperty(U32 atom) {
	properties.remove(atom);
}