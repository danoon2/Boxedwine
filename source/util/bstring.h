#ifndef __BSTRING_H__
#define __BSTRING_H__

#define B(x) BString(x, true)

class BStringData;

// this class represents the local holder of the shared data, BStringData
//
// BStringData is reference counted, if someone tries to write to BStringData and it has a reference count greater than one, then it will make a copy
class BString {	
public:
	BString();
	BString(const char*, bool litteral);
	BString(U32 size, char value);
	BString(BString&& from) noexcept {
		this->data = from.data;
		from.data = nullptr;
	}
	//BString(const char* s);
	//BString(const char* s, int len);
	BString(const BString& s);
	BString(char c);
	~BString();

	const char* c_str() const;
	char* str();
	void w_str(wchar_t* w, int len) const;

	char charAt(int i) const;
	int compareTo(const BString& s, bool ignoreCase = false, int offset = 0, int len = -1) const;
	int compareTo(const char* s, bool ignoreCase = false, int offset = 0, int len = -1) const;
	bool contains(const BString& s, bool ignoreCase = false) const;
	bool contains(const char* s, bool ignoreCase = false) const;
	bool endsWith(const BString& s, bool ignoreCase = false) const;
	bool endsWith(const char* s, bool ignoreCase = false) const;
	bool endsWith(char c, bool ignoreCase = false) const;
	int indexOf(const BString& s, int fromIndex = 0) const;
	int indexOf(const char* s, int fromIndex = 0) const;
	int indexOf(char c, int fromIndex = 0) const;
	bool isEmpty() const;
	int lastIndexOf(const BString& s, int fromIndex = -1) const;
	int lastIndexOf(const char* s, int fromIndex = -1) const;
	int lastIndexOf(char c, int fromIndex = -1) const;
	int length() const;
	void split(char c, std::vector<BString>& results) const;
	void split(const char* s, std::vector<BString>& results) const;
	void split(BString s, std::vector<BString>& results) const;
	bool startsWith(const BString& s, bool ignoreCase = false) const;
	bool startsWith(const char* s, bool ignoreCase = false) const;
	bool startsWith(char s, bool ignoreCase = false) const;
	BString substr(int beginIndex) const;
	BString substr(int beginIndex, int len) const;
	int32_t toInt() const;
	int64_t toInt64() const;

	template<typename ... Args>
	void sprintf(const char* format, Args ... args) {
		int size = std::snprintf(nullptr, 0, format, args ...);
		makeWritable(size);
		std::snprintf(str(), size + 1, format, args ...);
		setLength(size);
	}

	// modifying function
	void append(const BString& s);
	void append(const BString& s, int offset, int len);
	void append(const char* s);
	void append(const char* s, int len);
	void appendAfterNull(const BString& s);
	void appendAfterNull(const char* s);
	void appendAfterNull(const char* s, int len);
	void append(bool b);
	void append(char c);
	// missing on older gcc
	// void append(double d);
	// void append(float f);
	void append(U16 i, int base = 10);
	void append(U32 i, int base = 10);
	void append(U64 i, int base = 10);
	void append(S16 i, int base = 10);
	void append(S32 i, int base = 10);
	void append(S64 i, int base = 10);

	void remove(int index, int len = -1);
	void removeAll();

	BString replace(char oldChar, char newChar);
	BString replace(const char* oldString, const char* newString);
	BString replace(BString oldString, const char* newString);
	BString replace(const char* oldString, BString newString);
	BString replace(BString oldString, BString newString);

	BString toLowerCase() const;
	BString toUpperCase() const;
	BString trim();
	void resize(U32 len);
	void clear();

	BString operator+(const BString& s) const;
	BString operator+(const char* s) const;

	// guarantees single path separator between
	BString operator^(const BString& s) const;
	BString operator^(const char* s) const;

	BString& operator+=(const BString& s);
	BString& operator+=(const char* s);
	BString& operator+=(bool b);
	BString& operator+=(char c);
	// missing on older gcc
	// BString& operator+=(double d);
	// BString& operator+=(float f);
	BString& operator+=(U16 i);
	BString& operator+=(U32 i);
	BString& operator+=(U64 i);
	BString& operator+=(S16 i);
	BString& operator+=(S32 i);
	BString& operator+=(S64 i);

	BString& operator=(const BString& s);
	BString& operator=(BString&& s) noexcept;
	BString& operator=(const char* s);
	BString& operator=(bool b);
	BString& operator=(char c);
	// missing on older gcc
	// BString& operator=(double d);
	// BString& operator=(float f);
	BString& operator=(U16 i);
	BString& operator=(U32 i);
	BString& operator=(U64 i);
	BString& operator=(S16 i);
	BString& operator=(S32 i);
	BString& operator=(S64 i);

	static BString empty;

	static BString copy(const char* s);
	static BString copy(const char* s, int len);

	static BString join(BString delimiter, const std::vector<BString>& values);
	static BString join(const char* delimiter, const std::vector<BString>& values);

	static BString valueOf(bool b);
	static BString valueOf(signed char c);
	static BString valueOf(unsigned char c);
	// missing on older gcc
	// static BString valueOf(double d);
	// static BString valueOf(float f);
	static BString valueOf(U16 i);
	static BString valueOf(U16 i, int base);
	static BString valueOf(U32 i);
	static BString valueOf(U32 i, int base);
	static BString valueOf(U64 i);
	static BString valueOf(U64 i, int base);
	static BString valueOf(S16 i);
	static BString valueOf(S16 i, int base);
	static BString valueOf(S32 i);
	static BString valueOf(S32 i, int base);
	static BString valueOf(S64 i);
	static BString valueOf(S64 i, int base);

protected:
	friend bool operator==(const BString& s2, const BString& s1);

	template <typename T>
	friend void appendData(BString& s, T i);

	template <typename T>
	friend void appendData(BString& s, T i, int base);

	BString(BStringData* data);

	BStringData* data;
	int getLevel() const;	

	// len is what we need in addition to current length, pass 0 just to make sure we are writable
	void makeWritable(int len);
	void setLength(int len);
};

namespace std {
	template <>
	struct hash<BString> {
		std::size_t operator()(const BString& k) const {
			return std::hash<std::string_view>()(std::string_view(k.c_str(), k.length()));
		}
	};
}

BString operator+(const char* s1, BString s2);
bool operator==(const BString& s2, const BString& s1);
bool operator==(const BString& s2, const char* s1);
bool operator!=(const BString& s2, const BString& s1);
bool operator!=(const BString& s2, const char* s1);
bool operator<(const BString& s2, const BString& s1);
bool operator<(const BString& s2, const char* s1);
bool operator>(const BString& s2, const BString& s1);
bool operator>(const BString& s2, const char* s1);

#endif