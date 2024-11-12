#include "boxedwine.h"
#include "x11.h"

static const char* getState(U32 value) {
	if (value == WithdrawnState) {
		return "WithdrawnState";
	} else if (value == NormalState) {
		return "NormalState";
	} else if (value == IconicState) {
		return "IconicState";
	}
	return "";
}

static BString getMwmFlags(U32 func) {
	BString result;

	if (func & 1) {
		result += " MWM_HINTS_FUNCTIONS";
	}
	if (func & 2) {
		result += " MWM_HINTS_DECORATIONS";
	}
	if (func & 4) {
		result += " MWM_HINTS_INPUT_MODE";
	}
	if (func & 8) {
		result += " MWM_HINTS_STATUS";
	}
	return result;
}

static BString getMwmFunc(U32 func) {
	BString result;

	if (func & 1) {
		result += " MWM_FUNC_ALL";
	}
	if (func & 2) {
		result += " MWM_FUNC_RESIZE";
	}
	if (func & 4) {
		result += " MWM_FUNC_MOVE";
	}
	if (func & 8) {
		result += " MWM_FUNC_MINIMIZE";
	}
	if (func & 0x10) {
		result += " MWM_FUNC_MAXIMIZE";
	}
	if (func & 0x20) {
		result += " MWM_FUNC_CLOSE";
	}
	return result;
}

static BString getMwmDecor(U32 func) {
	BString result;

	if (func & 1) {
		result += " MWM_DECOR_ALL";
	}
	if (func & 2) {
		result += " MWM_DECOR_BORDER";
	}
	if (func & 4) {
		result += " MWM_DECOR_RESIZEH";
	}
	if (func & 8) {
		result += " MWM_DECOR_TITLE";
	}
	if (func & 0x10) {
		result += " MWM_DECOR_MENU";
	}
	if (func & 0x20) {
		result += " MWM_DECOR_MINIMIZE";
	}
	if (func & 0x40) {
		result += " MWM_DECOR_MAXIMIZE";
	}
	return result;
}

BString XProperty::description() {
	BString log;
	BString name;
	BString typeName;
	XServer::getServer()->getAtom(atom, name);
	XServer::getServer()->getAtom(atom, typeName);
	log = "property=";
	log.append(atom, 16);
	log.append("(");
	log.append(name);
	log.append(") type=");
	log.append(type, 16);
	log.append("(");
	log.append(typeName);
	log.append(") ");

	if (type == XA_ATOM) {
		U32* atoms = (U32*)value;
		for (U32 i = 0; i < length / 4; i++) {
			BString name;
			if (XServer::getServer()->getAtom(atoms[i], name)) {
				log += " ";
				log += name;
			}
		}
	} else if (type == WM_STATE) {
		U32* atoms = (U32*)value;
		log += getState(atoms[0]);
		log += " ";
		log += atoms[1];
	} else if (atom == XA_WM_HINTS) {
		U32* atoms = (U32*)value;
		log += " flags=";
		log.append(atoms[0], 16);
		log += " input=";
		log += atoms[1];
		log += " initial_state=";
		log += getState(atoms[2]);
		log += " icon_pixmap=";
		log += atoms[3];
		log += " icon_window=";
		log += atoms[4];
		log += " icon_x=";
		log += atoms[5];
		log += " icon_y=";
		log += atoms[6];
		log += " icon_mask=";
		log += atoms[7];
		log += " window_group=";
		log += atoms[8];
	} else if (atom == _NET_WM_NAME) {
		log += " ";
		log.append((char*)value, length);
	} else if (atom == _MOTIF_WM_HINTS) {
		U32* atoms = (U32*)value;
		log += " flags =";
		log += getMwmFlags(atoms[0]);
		log += " functions =";
		log += getMwmFunc(atoms[1]);
		log += " decorations =";
		log += getMwmDecor(atoms[2]);
		log += " input_mode = ";
		log += atoms[3];
		log += " status = ";
		log += atoms[4];
	} else if (type == XA_WINDOW) {
		U32* atoms = (U32*)value;
		for (U32 i = 0; i < length / 4; i++) {
			log += " ";
			log.append(atoms[i], 16);
		}
	} else if (type == XA_CARDINAL && atom != _NET_WM_ICON) {
		U32* atoms = (U32*)value;
		for (U32 i = 0; i < length / 4; i++) {
			log += " ";
			log.append(atoms[i], 16);
		}
	}
	log.replace(B("%"), B("%%"));
	return log;
}

XPropertyPtr XProperties::getProperty(U32 atom) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	return properties.get(atom);
}

void XProperties::setProperty(U32 atom, U32 type, U32 format, U32 length, const U8* value) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);
	U8* newValue = new U8[length];
	memcpy(newValue, value, length);
	XPropertyPtr property = std::make_shared<XProperty>(atom, type, format, length, newValue);
	properties.set(atom, property);	
}

void XProperties::setProperty(U32 atom, U32 type, U32 format, U32 length, U32 address) {
	BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(propertiesMutex);	
	U8* value = new U8[length];
	KThread::currentThread()->memory->memcpy(value, address, length);
	XPropertyPtr property = std::make_shared<XProperty>(atom, type, format, length, value);
	properties.set(atom, property);
}

void XProperties::deleteProperty(U32 atom) {
	properties.remove(atom);
}

BString XProperties::description() {
	BString log;

	for (auto& it : properties) {
		if (log.length()) {
			log += "\n";
		}
		log += it.value->description();
	}
	return log;
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
