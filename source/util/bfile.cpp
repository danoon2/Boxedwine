#include "boxedwine.h"
#include <cstdio>
#include UNISTD

U64 BReadFile::npos = (U64)-1;
U64 BWriteFile::npos = (U64)-1;

BReadFile::BReadFile() {
}

BReadFile::BReadFile(const char* pPath) {
	file = std::fopen(pPath, "rb");
}

BReadFile::BReadFile(BString path) {
	file = std::fopen(path.c_str(), "rb");
}

BReadFile::~BReadFile() {
	close();
}

bool BReadFile::open(const char* pPath) {
	if (isOpen()) {
		return false;
	}
	file = std::fopen(pPath, "rb");
	return file != nullptr;
}

bool BReadFile::open(BString path) {
	if (isOpen()) {
		return false;
	}
	file = std::fopen(path.c_str(), "rb");
	return file != nullptr;
}

bool BReadFile::isOpen() {
	return file != nullptr;
}

void BReadFile::close() {
	if (file) {
		std::fclose(file);
		file = nullptr;
	}
}

BString BReadFile::readAll() {
	char c = 0;
	BString line;

	while (read(c)) {
		line.append(c);
	}
	return line;
}

U32 BReadFile::read(char* buffer, U64 len) {
	return (U32)std::fread(buffer, 1, (size_t)len, file);
}

U32 BReadFile::read(S8* buffer, U64 len) {
	return (U32)std::fread(buffer, 1, (size_t)len, file);
}

U32 BReadFile::read(U8* buffer, U64 len) {
	return (U32)std::fread(buffer, 1, (size_t)len, file);
}

bool BReadFile::readLine(BString& line) {
	char c = 0;
	bool result = false;
	line.removeAll();
	while (read(c)) {
		if (c == '\n') {
			return true;
		}
		if (c == '\r') {
			continue;
		}
		line.append(c);
		result = true;
	}
	return result;
}

bool BReadFile::read(char& value) {
	return std::fread(&value, 1, 1, file) == 1;
}

bool BReadFile::read(U8& value) {
	return std::fread(&value, 1, 1, file) == 1;
}

bool BReadFile::read(S8& value) {
	return std::fread(&value, 1, 1, file) == 1;
}

bool BReadFile::read(U16& value) {
	return std::fread(&value, 1, 2, file) == 2;
}

bool BReadFile::read(S16& value) {
	return std::fread(&value, 1, 2, file) == 2;
}

bool BReadFile::read(U32& value) {
	return std::fread(&value, 1, 4, file) == 4;
}
	
bool BReadFile::read(S32& value) {
	return std::fread(&value, 1, 4, file) == 4;
}

U64 BReadFile::setPos(U64 pos) {
	U64 cur = getPos();
	if (std::fseek(file, (U32)pos, SEEK_SET) == 0) {
		return cur;
	}
	return npos;
}

U64 BReadFile::advance(U64 amount) {
	U64 cur = getPos();
	if (std::fseek(file, (U32)amount, SEEK_CUR) == 0) {
		return cur;
	}
	return npos;
}

U64 BReadFile::getPos() {
	return (U64)std::ftell(file);
}

U64 BReadFile::length() {
	U64 pos = getPos();
	std::fseek(file, 0, SEEK_END);
	U64 size = std::ftell(file);
	std::fseek(file, (U32)pos, SEEK_SET);
	return size;
}

BWriteFile::BWriteFile() {
}

BWriteFile::BWriteFile(const char* pPath, bool truncate) {
	if (truncate) {
		createNew(pPath);
	} else {
		createOrExisting(pPath);
	}
}

BWriteFile::BWriteFile(BString path, bool truncate) {
	if (truncate) {
		createNew(path);
	} else {
		createOrExisting(path);
	}
}

BWriteFile::~BWriteFile() {
	close();
}

bool BWriteFile::createNew(const char* pPath) {
	if (isOpen()) {
		return false;
	}
	file = std::fopen(pPath, "wb");
	return file != nullptr;
}

bool BWriteFile::createNew(BString path) {
	return createNew(path.c_str());
}

bool BWriteFile::createOrExisting(const char* pPath) {
	if (isOpen()) {
		return false;
	}
	if (::access(pPath, 0) != -1) {
		// this will fail to open an existing file for random access write unless it exists
		file = std::fopen(pPath, "rb+");
	} else {
		// this will always truncate
		file = std::fopen(pPath, "wb");
	}
	return file != nullptr;
}

bool BWriteFile::createOrExisting(BString path) {
	return createOrExisting(path.c_str());
}

bool BWriteFile::isOpen() {
	return file != nullptr;
}

void BWriteFile::close() {
	if (file) {
		std::fclose(file);
		file = nullptr;
	}
}

U32 BWriteFile::write(const char* buffer) {
	return (U32)std::fwrite(buffer, 1, strlen(buffer), file);
}

U32 BWriteFile::write(const char* buffer, U32 len) {
	return (U32)std::fwrite(buffer, 1, len, file);
}

U32 BWriteFile::write(const S8* buffer, U32 len) {
	return (U32)std::fwrite(buffer, 1, len, file);
}

U32 BWriteFile::write(const U8* buffer, U32 len) {
	return (U32)std::fwrite(buffer, 1, len, file);
}

U32 BWriteFile::write(BString buffer) {
	return (U32)std::fwrite(buffer.c_str(), 1, buffer.length(), file);
}

bool BWriteFile::write(U8 value) {
	return std::fwrite(&value, 1, 1, file) == 1;
}

bool BWriteFile::write(S8 value) {
	return std::fwrite(&value, 1, 1, file) == 1;
}

bool BWriteFile::write(U16 value) {
	return std::fwrite(&value, 1, 2, file) == 2;
}

bool BWriteFile::write(S16 value) {
	return std::fwrite(&value, 1, 2, file) == 2;
}

bool BWriteFile::write(U32 value) {
	return std::fwrite(&value, 1, 4, file) == 4;
}

bool BWriteFile::write(S32 value) {
	return std::fwrite(&value, 1, 4, file) == 4;
}

void BWriteFile::flush() {
	std::fflush(file);
}

U64 BWriteFile::setPos(U64 pos) {
	U64 cur = getPos();
	if (std::fseek(file, (U32)pos, SEEK_SET) == 0) {
		return cur;
	}
	return npos;
}

U64 BWriteFile::advance(U64 amount) {
	U64 cur = getPos();
	if (std::fseek(file, (U32)amount, SEEK_CUR) == 0) {
		return cur;
	}
	return npos;
}

U64 BWriteFile::getPos() {
	return (U64)std::ftell(file);
}

U64 BWriteFile::length() {
	U64 pos = getPos();
	std::fseek(file, 0, SEEK_END);
	U64 size = std::ftell(file);
	std::fseek(file, (U32)pos, SEEK_SET);
	return size;
}