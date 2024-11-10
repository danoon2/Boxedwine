#include "boxedwine.h"
#include "x11.h"

void XIEventMask::read(KMemory* memory, U32 address) {
	deviceid = memory->readd(address);
	mask_len = memory->readd(address + 4);
	maskAddress = memory->readd(address + 8);
}

void XIDeviceInfo::write(KMemory* memory, U32 address, S32 deviceid, U32 name, S32 use, S32 attachment, Bool enabled, S32 num_classes, U32 classes) {
	memory->writed(address, deviceid);
	memory->writed(address + 4, name);
	memory->writed(address + 8, use);
	memory->writed(address + 12, attachment);
	memory->writed(address + 16, enabled);
	memory->writed(address + 20, num_classes);
	memory->writed(address + 24, classes);
}

void XIValuatorClassInfo::write(KMemory* memory, U32 address, S32 type, S32 sourceid, S32 number, Atom label, double min, double max, double value, S32 resolution, S32 mode) {
	memory->writed(address, type);
	memory->writed(address + 4, sourceid);
	memory->writed(address + 8, number);
	memory->writed(address + 12, label);

	long2Double d;
	d.d = min;
	memory->writeq(address + 16, d.l);
	d.d = max;
	memory->writeq(address + 24, d.l);
	d.d = value;
	memory->writeq(address + 32, d.l);

	memory->writed(address + 36, resolution);
	memory->writed(address + 40, mode);
}

void XIRawEvent::serialize(U32* data) {
	data[0] = type;
	data[1] = serial;
	data[2] = send_event;
	data[3] = displayAddress;
	data[4] = extension;
	data[5] = evtype;
	data[6] = time;
	data[7] = deviceid;
	data[8] = sourceid;
	data[9] = detail;
	data[10] = flags;
	data[11] = valuators.mask_len;
	data[12] = valuators.maskAddress;
	data[13] = valuators.valuesAddress;
	data[14] = raw_values;
}