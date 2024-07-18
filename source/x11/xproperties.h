#ifndef __X_PROPERTIES_H__
#define __X_PROPERTIES_H__

class XProperty {
public:
	XProperty(U32 type, U32 format, U32 length, U8* value, bool needsDelete = true) : type(type), format(format), length(length), value(value), needsDelete(needsDelete) {}
	~XProperty() {
		if (value && needsDelete) {
			delete[] value;
		}
	}
	U32 type;

	U32 format; // 8, 16 or 32

	U32 length;
	U8* value;
	bool needsDelete;
};

typedef std::shared_ptr<XProperty> XPropertyPtr;

class XProperties {
public:
	XPropertyPtr getProperty(U32 atom);
	void setProperty(U32 atom, U32 type, U32 format, U32 length, U8* value, bool needsCopy = true);
	void deleteProperty(U32 atom);

private:
	BOXEDWINE_MUTEX propertiesMutex;
	BHashTable<U32, XPropertyPtr> properties;
};

#endif