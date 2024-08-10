#ifndef __X_PROPERTIES_H__
#define __X_PROPERTIES_H__

#include "x11.h"

class XProperty {
public:
	XProperty(U32 type, U32 format, U32 length, U8* value) : type(type), format(format), length(length), value(value) {}
	~XProperty() {
		delete[] value;
	}
	U32 type;

	U32 format; // 8, 16 or 32

	U32 length;
	U8* value;

	bool contains32(U32 v) {
		U32* value32 = (U32*)value;
		for (U32 i = 0; i < length / 4; i++) {
			if (value32[i] == v) {
				return true;
			}
		}
		return false;
	}
};

typedef std::shared_ptr<XProperty> XPropertyPtr;

class XProperties {
public:
	XPropertyPtr getProperty(U32 atom);
	void setProperty(U32 atom, U32 type, U32 format, U32 length, U8* value);
	void setProperty(U32 atom, U32 type, U32 format, U32 length, U32 value);
	void deleteProperty(U32 atom);

private:
	BOXEDWINE_MUTEX propertiesMutex;
	BHashTable<U32, XPropertyPtr> properties;
};

struct XTextProperty {
	static void create(KThread* thread, U32 encoding, S8** list, U32 count, XTextProperty* property);
	static void create(KThread* thread, U32 encoding, U32 list, U32 count, XTextProperty* property);

	XTextProperty() {}
	XTextProperty(KMemory* memory, U32 address) { read(memory, address); }
	void read(KMemory* memory, U32 address) {
		value = memory->readd(address); address += 4;
		encoding = memory->readd(address); address += 4;
		format = (S32)memory->readd(address); address += 4;
		nitems = memory->readd(address);
	}
	void write(KMemory* memory, U32 address) {
		memory->writed(address, value); address += 4;
		memory->writed(address, encoding); address += 4;
		memory->writed(address, format); address += 4;
		memory->writed(address, nitems);
	}

	U32 byteLen(KMemory* memory);

	U32 value;		/* same as Property routines */
	U32 encoding;			/* prop type */
	S32 format;				/* prop data format: 8, 16, or 32 */
	U32 nitems;		/* number of data items in value */
};

#endif