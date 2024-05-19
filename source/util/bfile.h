#ifndef __BFILE_H__

class BReadFile {
public:
	BReadFile();
	BReadFile(const char* pPath);
	BReadFile(BString path);
	~BReadFile();

	bool open(const char* pPath);
	bool open(BString path);

	bool isOpen();
	void close();

	U32 read(char* buffer, U64 len);
	U32 read(S8* buffer, U64 len);
	U32 read(U8* buffer, U64 len);
	bool readLine(BString& line);
	BString readAll();

	bool read(char& value);
	bool read(U8& value);
	bool read(S8& value);
	bool read(U16& value);
	bool read(S16& value);
	bool read(U32& value);
	bool read(S32& value);

	U64 setPos(U64 pos);
	U64 advance(U64 amount);
	U64 getPos();
	U64 length();

	static U64 npos;
private:
	std::FILE* file = nullptr;
};

class BWriteFile {
public:
	BWriteFile();
	BWriteFile(const char* pPath, bool truncate = true);
	BWriteFile(BString path, bool truncate = true);
	~BWriteFile();

	BWriteFile(const BWriteFile& file) = delete;
	BWriteFile(BWriteFile&& file) = delete;
	BWriteFile& operator=(const BWriteFile&) = delete;
	BWriteFile& operator=(BWriteFile&&) = delete;

	bool createNew(const char* pPath);
	bool createNew(BString path);
	bool createOrExisting(const char* pPath);
	bool createOrExisting(BString path);

	bool isOpen();
	void close();

	U32 write(const char* buffer, U32 len);
	U32 write(const char* buffer);
	U32 write(const S8* buffer, U32 len);
	U32 write(const U8* buffer, U32 len);
	U32 write(BString buffer);

	template <class... Args>
	U32 writeFormat(const char* format, Args&&... args) {
		auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
		BString msg(size + 1, '\0');
		std::snprintf(msg.str(), size + 1, format, std::forward<Args>(args)...);
		return write(msg);
	}

	bool write(U8 value);
	bool write(S8 value);
	bool write(U16 value);
	bool write(S16 value);
	bool write(U32 value);
	bool write(S32 value);

	void flush();

	U64 setPos(U64 pos);
	U64 advance(U64 amount);
	U64 getPos();
	U64 length();

	static U64 npos;
private:
	std::FILE* file = nullptr;
};

#endif