#include "boxedwine.h"
#include "concurrentqueue.h"
#include "ptrpool.h"

#include <charconv>

class BStringData {
public:
    BStringData() = default;

    void reset() {
        refCount = 0;
        len = 0;
        level = 0;
        str = nullptr;
    }

    void incRefCount() {
        refCount.fetch_add(1, std::memory_order_relaxed);
    }

    void decRefCount();

    char* str = nullptr;
    int len = 0;
    int level = 0;
    std::atomic_int refCount;
};

#define SMALLEST_LEVEL 4
#define LARGEST_LEVEL 16
#define TOTAL_LEVEL 13

static PtrPool<BStringData>* freeStringData;
static PtrPool<char>* freeMemoryBySize[TOTAL_LEVEL];

char* getNewString(int level) {
    if (!freeMemoryBySize[level - SMALLEST_LEVEL]) {
        freeMemoryBySize[level - SMALLEST_LEVEL] = new PtrPool<char>(0);
    }
    char* result = freeMemoryBySize[level - SMALLEST_LEVEL]->get();
    if (!result) {
        U32 size = 1 << level;
        result = new char[size * 100];
        for (int i = 1; i < 100; i++, result += size) {
            freeMemoryBySize[level - SMALLEST_LEVEL]->put(result);
        }
    }
    return result;
}

void releaseString(int level, char* str) {
    if (!freeMemoryBySize[level - SMALLEST_LEVEL]) {
        freeMemoryBySize[level - SMALLEST_LEVEL] = new PtrPool<char>(0);
    }
    freeMemoryBySize[level - SMALLEST_LEVEL]->put(str);
}

int powerOf2(int requestedSize) {
    int size = 1 << SMALLEST_LEVEL;
    int powerOf2Size = SMALLEST_LEVEL;
    while (size < requestedSize) {
        size <<= 1;
        powerOf2Size++;
    }
    return powerOf2Size;
}

void BStringData::decRefCount() {
    if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        if (level != 0) {
            releaseString(level, str);
        }
        if (!freeStringData) {
            freeStringData = new PtrPool<BStringData>();
        }
        freeStringData->put(this);
    }
}

static BStringData* allocNewData() {
    if (!freeStringData) {
        freeStringData = new PtrPool<BStringData>();
    }
    return freeStringData->get();
}

template <typename T>
void appendData(BString& s, T i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    int len = (int)strlen(tmp);
    s.makeWritable(len);
    strcpy(s.data->str + s.data->len, tmp);
    s.data->len += len;
    s.data->str[s.data->len] = 0;
}

template <typename T>
void appendData(BString& s, T i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    int len = (int)strlen(tmp);
    s.makeWritable(len);
    strcpy(s.data->str + s.data->len, tmp);
    s.data->len += len;
    s.data->str[s.data->len] = 0;
}

BString BString::empty("", true);

BString::BString(const char* str, bool literal) {
    data = allocNewData();
    data->incRefCount();
    if (literal) {
        data->len = -1;
        data->level = 0;
        data->str = (char*)str;
    } else {
        kpanic("BString::BString oops");
    }
}

BString::BString() {
    if (!empty.data) {
        data = allocNewData();
    } else {
        data = empty.data;
    }
    data->incRefCount();
}

BString::BString(U32 size, char value) {
    data = allocNewData();
    data->str = getNewString(SMALLEST_LEVEL);
    data->len = size-1;
    data->level = getLevel();
    data->str = getNewString(data->level);
    memset(data->str, value, size);
    data->incRefCount();
}

BString::BString(BStringData* data) {
    this->data = data;
    this->data->incRefCount();
}

BString::BString(const BString& s) {
    data = s.data;
    data->incRefCount();
}

BString::BString(char c) {
    data = allocNewData();
    data->str = getNewString(SMALLEST_LEVEL);
    data->len = 1;
    data->level = SMALLEST_LEVEL;
    data->str[0] = c;
    data->str[1] = 0;
    data->incRefCount();
}

BString::~BString() {
    if (data) {
        data->decRefCount();
    }
}

const char* BString::c_str() const {
    return data->str;
}

char* BString::str() {
    return data->str;
}

void BString::w_str(wchar_t* w, int len) const {
    std::string s(data->str);
    std::wstring ws(s.begin(), s.end());
    memcpy(w, ws.c_str(), std::min((int)ws.size() * 2 + 2, len * 2));
}

void BString::append(const BString& s) {
    append(s.data->str, s.length());
}

void BString::append(const BString& s, int offset, int len) {
    if (offset <= s.length()) {
        append(s.data->str + offset, len);
    }
}

void BString::append(const char* s) {
    int len = (int)strlen(s);
    makeWritable(len);
    strcpy(data->str + data->len, s);
    data->len += len;
    data->str[data->len] = 0;
}

void BString::append(const char* s, int len) {
    makeWritable(len);
    strncpy(data->str + data->len, s, len);
    data->len += len;
    data->str[data->len] = 0;
}

void BString::appendAfterNull(const BString& s) {
    appendAfterNull(s.data->str, s.length());
}

void BString::appendAfterNull(const char* s) {
    int len = (int)strlen(s);
    makeWritable(len + 1);
    strcpy(data->str + 1 + data->len, s);
    data->len += len + 1;
    data->str[data->len] = 0;
}

void BString::appendAfterNull(const char* s, int len) {
    makeWritable(len + 1);
    strncpy(data->str + 1 + data->len, s, len);
    data->len += len + 1;
    data->str[data->len] = 0;
}

void BString::append(bool b) {
    const char* tmp = b ? "true" : "false";
    int len = (int)strlen(tmp);
    makeWritable(len);
    strcpy(data->str + data->len, tmp);
    data->len += len;
    data->str[data->len] = 0;
}

void BString::append(char c) {
    makeWritable(1);
    data->str[data->len] = c;
    data->len++;
    data->str[data->len] = 0;
}

/*
void BString::append(double d) {
    appendData(*this, d);
}

void BString::append(float f) {
    appendData(*this, f);
}
*/
void BString::append(U16 i, int base) {
    appendData(*this, i, base);
}

void BString::append(U32 i, int base) {
    appendData(*this, i, base);
}

void BString::append(U64 i, int base) {
    appendData(*this, i, base);
}

void BString::append(S16 i, int base) {
    appendData(*this, i, base);
}

void BString::append(S32 i, int base) {
    appendData(*this, i, base);
}

void BString::append(S64 i, int base) {
    appendData(*this, i, base);
}

char BString::charAt(int i) const {
    if (i < data->len) {
        return data->str[i];
    }
    return 0;
}

int BString::compareTo(const BString& s, bool ignoreCase, int offset, int len) const {
    return compareTo(s.data->str, ignoreCase, offset, len);
}

int BString::compareTo(const char* s, bool ignoreCase, int offset, int len) const {
    if (!s) {
        return 1;
    }
    if (isEmpty()) {
        if (strlen(s) == 0) {
            return 0;
        }
        return -1;
    }
    if (offset < 0) {
        return -1;
    }
    if (offset > data->len) {
        offset = data->len;
    }

    if (ignoreCase) {
        if (len == -1) {
            return strcasecmp(data->str + offset, s);
        }
        return strncasecmp(data->str + offset, s, len);
    } else {
        if (len == -1) {
            return strcmp(data->str + offset, s);
        }
        return strncmp(data->str + offset, s, len);
    }
}

bool BString::contains(const BString& s, bool ignoreCase) const {
    return contains(s.data->str, ignoreCase);
}

bool BString::contains(const char* s, bool ignoreCase) const {
    if (isEmpty()) {
        return false;
    }
    if (!ignoreCase) {
        return strstr(data->str, s) != nullptr;
    }
    return strcasestr(data->str, s) != nullptr;
}

bool BString::endsWith(const BString& s, bool ignoreCase) const {
    if (s.isEmpty()) {
        return false;
    }
    return compareTo(s.data->str, ignoreCase, length() - s.length()) == 0;
}

bool BString::endsWith(const char* s, bool ignoreCase) const {
    int len = (int)strlen(s);
    if (length() < len) {
        return false;
    }
    return compareTo(s, ignoreCase, length() - len) == 0;
}

bool BString::endsWith(char c, bool ignoreCase) const {
    if (isEmpty()) {
        return false;
    }
    if (ignoreCase) {
        return std::tolower(data->str[length() - 1]) == std::tolower(c);
    }
    return data->str[length() - 1] == c;
}

int BString::indexOf(const BString& s, int fromIndex) const {
    return indexOf(s.data->str, fromIndex);
}

int BString::indexOf(const char* s, int fromIndex) const {
    if (isEmpty()) {
        return -1;
    }
    if (fromIndex > length()) {
        return -1;
    }
    char* p = strstr(data->str + fromIndex, s);
    if (!p) {
        return -1;
    }
    return (int)(p - data->str);
}

int BString::indexOf(char c, int fromIndex) const {
    if (isEmpty()) {
        return -1;
    }
    if (fromIndex > length()) {
        return -1;
    }
    char* p = strchr(data->str + fromIndex, c);
    if (!p) {
        return -1;
    }
    return (int)(p - data->str);
}

bool BString::isEmpty() const {
    return length() == 0;
}

int BString::lastIndexOf(const BString& s, int fromIndex) const {
    if (isEmpty()) {
        return -1;
    }
    if (s.length() == 1) {
        return lastIndexOf(s.data->str[0]);
    }
    return lastIndexOf(s.data->str);
}

int BString::lastIndexOf(const char* s, int fromIndex) const {
    if (isEmpty()) {
        return -1;
    }
    size_t pos = 0;
    if (fromIndex > length() || fromIndex < 0) {
        pos = std::string_view::npos;
    } else {
        pos = (size_t)fromIndex;
    }
    std::string_view view = { data->str };
    pos = view.find_last_of(s, pos);
    if (pos == std::string_view::npos) {
        return -1;
    }
    return (int)pos;
}

int BString::lastIndexOf(char c, int fromIndex) const {
    if (isEmpty()) {
        return -1;
    }
    size_t pos = 0;
    if (fromIndex > length() || fromIndex < 0) {
        pos = std::string_view::npos;
    } else {
        pos = (size_t)fromIndex;
    }
    std::string_view view = { data->str };
    pos = view.find_last_of(c, pos);
    if (pos == std::string_view::npos) {
        return -1;
    }
    return (int)pos;
}

int BString::length() const {
    if (data->len < 0) {
        if (data->str) {
            data->len = (int)strlen(data->str);
        } else {
            data->len = 0;
        }
    }
    return data->len;
}

void BString::remove(int index, int len) {
    makeWritable(0);
}

void BString::removeAll() {
    clear();
}

BString BString::replace(char oldChar, char newChar) {
    makeWritable(0);
    for (int i = 0; i < data->len; i++) {
        if (data->str[i] == oldChar) {
            data->str[i] = newChar;
        }
    }
    return *this;
}

BString BString::replace(const char* oldString, const char* newString) {
    if (isEmpty()) {
        return *this;
    }
    int pos = indexOf(oldString);
    if (pos == -1) {
        return *this;
    }

    int lastPos = 0;
    int oldLen = (int)strlen(oldString);
    int newLen = (int)strlen(newString);

    makeWritable(0);
    while (pos >= 0) {
        if (oldLen == newLen) {
            memcpy(data->str + pos, newString, newLen);
        } else if (oldLen > newLen) {
            memcpy(data->str + pos, newString, newLen);
            int startPos = pos + oldLen;
            memmove(data->str + pos + newLen, data->str + startPos, data->len - startPos);
            data->len -= (oldLen - newLen);
            data->str[data->len] = 0;
        } else {
            int diff = newLen - oldLen;
            makeWritable(diff);
            memmove(data->str + pos + newLen, data->str + pos + oldLen, data->len - (pos + oldLen));
            memcpy(data->str + pos, newString, newLen);
            data->len += diff;
            data->str[data->len] = 0;
        }

        lastPos = pos + newLen;
        pos = indexOf(oldString, lastPos);
    }
    return *this;
}

BString BString::replace(BString oldString, const char* newString) {
    return replace(oldString.data->str, newString);
}

BString BString::replace(const char* oldString, BString newString) {
    return replace(oldString, newString.c_str());
}

BString BString::replace(BString oldString, BString newString) {
    return replace(oldString.data->str, newString.data->str);
}

void BString::split(char c, std::vector<BString>& results) const {
    if (!isEmpty()) {
        char* s = data->str;
        char* p = s;

        while ((p = strchr(s, c))) {
            int startIndex = (int)(s - data->str);
            int stopIndex = (int)(p - data->str);
            results.push_back(this->substr(startIndex, stopIndex - startIndex));
            s = p + 1;
        }
        results.push_back(BString::copy(s));
    }
}

void BString::split(const char* c, std::vector<BString>& results) const {
    if (!isEmpty()) {
        char* s = data->str;
        char* p = s;
        int cLen = (int)strlen(c);

        while ((p = strstr(s, c))) {
            int startIndex = (int)(s - data->str);
            int stopIndex = (int)(p - data->str);
            results.push_back(this->substr(startIndex, stopIndex - startIndex));
            s = p + cLen;
        }
        results.push_back(BString::copy(s));
    }
}

void BString::split(BString s, std::vector<BString>& results) const {
    split(s.data->str, results);
}

bool BString::startsWith(const BString& s, bool ignoreCase) const {
    return compareTo(s, ignoreCase, 0, s.length()) == 0;
}

bool BString::startsWith(const char* s, bool ignoreCase) const {
    return compareTo(s, ignoreCase, 0, (int)strlen(s)) == 0;
}

bool BString::startsWith(char c, bool ignoreCase) const {
    if (isEmpty()) {
        return false;
    }
    if (ignoreCase) {
        return std::tolower(data->str[0]) == std::tolower(c);
    }
    return data->str[0] == c;
}

BString BString::substr(int beginIndex) const {
    return substr(beginIndex, length() - beginIndex);
}

BString BString::substr(int beginIndex, int len) const {
    // :TODO in the future maybe don't make a copy, just a ref and use offsets into the data
    if (beginIndex > length()) {
        return empty;
    }
    if (beginIndex + len > length() || len < 0) {
        len = length() - beginIndex;
    }
    BStringData* d = allocNewData();
    d->len = len;
    d->level = powerOf2(d->len + 1);
    d->str = getNewString(d->level);
    memcpy(d->str, data->str + beginIndex, d->len);
    d->str[d->len] = 0;
    return BString(d);
}

int32_t BString::toInt() const {
    return std::atoi(data->str);
}

int64_t BString::toInt64() const {
    return std::atoll(data->str);
}

int BString::getLevel() const {
    if (data->level) {
        return data->level;
    }
    return powerOf2(length() + 1);
}

BString BString::toLowerCase() const {
    if (isEmpty()) {
        return *this;
    }
    // can't just copy len and level, they might not be set yet for literals
    BStringData* d = allocNewData();
    d->level = getLevel();
    d->str = getNewString(d->level);
    d->len = length();
    d->incRefCount();

    std::locale loc;
    for (int i = 0; i < d->len; i++) {
        d->str[i] = std::tolower(data->str[i], loc);
    }
    d->str[d->len] = 0;
    return BString(d);
}

BString BString::toUpperCase() const {
    if (isEmpty()) {
        return *this;
    }

    BStringData* d = allocNewData();
    d->level = getLevel();
    d->str = getNewString(d->level);
    d->len = length();
    d->incRefCount();

    std::locale loc;
    for (int i = 0; i < d->len; i++) {
        d->str[i] = std::toupper(data->str[i], loc);
    }
    d->str[d->len] = 0;
    return BString(d);
}

void BString::resize(U32 len) {
    makeWritable(len);
}

BString BString::trim() {
    if (isEmpty()) {
        return *this;
    }
    int startIndex = 0;
    int len = length();
    int endIndex = len;
    for (int i = 0; i < len; i++) {
        if (std::isspace(data->str[i])) {
            startIndex++;
        } else {
            break;
        }
    }
    for (int i = len - 1; i > startIndex; i--) {
        if (std::isspace(data->str[i])) {
            endIndex--;
        } else {
            break;
        }
    }
    if (startIndex == 0 && endIndex == len) {
        return *this;
    }
    makeWritable(0);
    data->len = endIndex - startIndex;
    if (startIndex) {
        memmove(data->str, data->str + startIndex, data->len);
    }
    data->str[data->len] = 0;
    return *this;
}

BString BString::operator+(const BString& s) const {
    if (isEmpty()) {
        return s;
    }
    BStringData* d = allocNewData();
    d->len = length() + s.length();
    d->level = powerOf2(d->len + 1);
    d->str = getNewString(d->level);
    memcpy(d->str, data->str, data->len);
    memcpy(d->str + data->len, s.data->str, s.length());
    d->str[d->len] = 0;
    return BString(d);
}

BString BString::operator+(const char* s) const {
    if (isEmpty()) {
        return BString::copy(s);
    }
    BStringData* d = allocNewData();
    int sLen = (int)strlen(s);
    d->len = length() + sLen;
    d->level = powerOf2(d->len + 1);
    d->str = getNewString(d->level);
    memcpy(d->str, data->str, data->len);
    memcpy(d->str + data->len, s, sLen);
    d->str[d->len] = 0;
    return BString(d);
}

BString& BString::operator+=(const BString& s) {
    append(s);
    return *this;
}

BString& BString::operator+=(const char* s) {
    append(s);
    return *this;
}

BString& BString::operator+=(bool b) {
    append(b);
    return *this;
}

BString& BString::operator+=(char c) {
    append(c);
    return *this;
}

/*
BString& BString::operator+=(double d) {
    append(d);
    return *this;
}

BString& BString::operator+=(float f) {
    append(f);
    return *this;
}
*/
BString& BString::operator+=(U16 i) {
    append(i);
    return *this;
}

BString& BString::operator+=(U32 i) {
    append(i);
    return *this;
}

BString& BString::operator+=(U64 i) {
    append(i);
    return *this;
}

BString& BString::operator+=(S16 i) {
    append(i);
    return *this;
}

BString& BString::operator+=(S32 i) {
    append(i);
    return *this;
}

BString& BString::operator+=(S64 i) {
    return *this;
}

BString& BString::operator = (const BString& other) {
    if (this->data != other.data) {
        if (this->data) {
            this->data->decRefCount();
        }
        this->data = other.data;
        this->data->incRefCount();
    }
    return *this;
}

BString& BString::operator = (BString&& other) noexcept {
    if (this->data != other.data) {
        if (this->data) {
            this->data->decRefCount();
        }
        this->data = other.data;
        other.data = nullptr;
    }
    return *this;
}

BString& BString::operator=(const char* s) {
    clear();
    append(s);
    return *this;
}

BString& BString::operator=(bool b) {
    clear();
    append(b);
    return *this;
}

BString& BString::operator=(char c) {
    clear();
    append(c);
    return *this;
}

/*
BString& BString::operator=(double d) {
    clear();
    append(d);
    return *this;
}

BString& BString::operator=(float f) {
    clear();
    append(f);
    return *this;
}
*/

BString& BString::operator=(U16 i) {
    clear();
    append(i);
    return *this;
}

BString& BString::operator=(U32 i) {
    clear();
    append(i);
    return *this;
}

BString& BString::operator=(U64 i) {
    clear();
    append(i);
    return *this;
}

BString& BString::operator=(S16 i) {
    clear();
    append(i);
    return *this;
}

BString& BString::operator=(S32 i) {
    clear();
    append(i);
    return *this;
}

BString& BString::operator=(S64 i) {
    clear();
    append(i);
    return *this;
}

void BString::clear() {
    makeWritable(0);
    data->len = 0;
    data->str[0] = 0;
}

void BString::setLength(int len) {
    data->len = len;
    data->str[len] = 0;
}

// additional size to add
void BString::makeWritable(int len) {
    if (data->level == 0 || data->refCount > 1) {
        BStringData* d = allocNewData();
        d->len = length();
        d->level = powerOf2(d->len + len + 1);
        d->str = getNewString(d->level);
        if (data->str) {
            memcpy(d->str, data->str, data->len + 1);
        } else {
            d->str[0] = 0;
        }
        d->incRefCount();
        data->decRefCount();
        data = d;
    } else if (1 << data->level < data->len + len + 1) {
        int level = powerOf2(data->len + len + 1);
        char* s = getNewString(level);
        memcpy(s, data->str, data->len + 1);
        releaseString(data->level, data->str);
        data->str = s;
        data->level = level;
    }
}

// guarantees single path separator between
BString BString::operator^(const BString& s) const {
    if (!s.data->str) {
        return *this ^ "";
    }
    return *this ^ s.data->str;
}

BString BString::operator^(const char* s) const {
    BStringData* d = allocNewData();
    int sLen = (int)strlen(s);
    char sep = (char)std::filesystem::path::preferred_separator;
    bool needAddSep = false;
    bool needRemoveSep = false;

    if (length() == 0) {
        needAddSep = s[0] != sep;
        needRemoveSep = false;
    } else {
        needAddSep = s[0] != sep && data->str[data->len - 1] != sep;
        needRemoveSep = s[0] == sep && data->str[data->len - 1] == sep;;
    }
    if (needRemoveSep) {
        sLen--;
        s++;
    }
    d->len = length() + sLen;
    if (needAddSep) {
        d->len++;
    }
    d->level = powerOf2(d->len + 1);
    d->str = getNewString(d->level);
    memcpy(d->str, data->str, data->len);
    int offset = 0;
    if (needAddSep) {
        d->str[data->len] = sep;
        offset = 1;
    }
    memcpy(d->str + data->len + offset, s, sLen);
    d->str[d->len] = 0;
    return BString(d);
}

BString BString::copy(const char* s) {
    if (!s || s[0]==0) {
        return empty;
    }
    BString result(allocNewData());
    result.data->len = (int)strlen(s);
    result.data->level = powerOf2(result.data->len + 1);
    result.data->str = getNewString(result.data->level);
    memcpy(result.data->str, s, result.data->len + 1);
    return result;
}

BString BString::copy(const char* s, int len) {
    if (!s) {
        return empty;
    }
    BString result(allocNewData());

    result.data->len = len;
    result.data->level = powerOf2(result.data->len + 1);
    result.data->str = getNewString(result.data->level);
    memcpy(result.data->str, s, result.data->len);
    result.data->str[result.data->len] = 0;
    return result;
}

BString BString::join(BString delimiter, const std::vector<BString>& values) {
    return join(delimiter.data->str, values);
}

BString BString::join(const char* delimiter, const std::vector<BString>& values) {
    BString result;
    for (int i = 0; i < (int)values.size(); i++) {
        if (i != 0) {
            result += delimiter;
        }
        result += values[i];
    }
    return result;
}

BString BString::valueOf(bool b) {
    return B(b ? "true" : "false");
}

BString BString::valueOf(signed char c) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, c);
    return BString::copy(tmp);
}

BString BString::valueOf(unsigned char c) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, c);
    return BString::copy(tmp);
}

/*
BString BString::valueOf(double d) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, d);
    return BString::copy(tmp);
}

BString BString::valueOf(float f) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, f);
    return BString::copy(tmp);
}
*/

BString BString::valueOf(U16 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(U16 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString BString::valueOf(U32 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(U32 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString BString::valueOf(U64 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(U64 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString BString::valueOf(S16 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(S16 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString BString::valueOf(S32 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(S32 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString BString::valueOf(S64 i) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i);
    return BString::copy(tmp);
}

BString BString::valueOf(S64 i, int base) {
    char tmp[32] = {};
    std::to_chars(tmp, tmp + 32, i, base);
    return BString::copy(tmp);
}

BString operator+(const char* s1, BString s2) {
    return B(s1) + s2;
}

bool operator==(const BString& s2, const BString& s1) {
    if (s2.data == s1.data) {
        return true;
    }
    return s2.compareTo(s1) == 0;
}

bool operator==(const BString& s2, const char* s1) {
    return s2.compareTo(s1) == 0;
}

bool operator!=(const BString& s2, const BString& s1) {
    return s2.compareTo(s1) != 0;
}

bool operator!=(const BString& s2, const char* s1) {
    return s2.compareTo(s1) != 0;
}

bool operator<(const BString& s2, const BString& s1) {
    return s2.compareTo(s1) < 0;
}

bool operator<(const BString& s2, const char* s1) {
    return s2.compareTo(s1) < 0;
}

bool operator>(const BString& s2, const BString& s1) {
    return s2.compareTo(s1) > 0;
}

bool operator>(const BString& s2, const char* s1) {
    return s2.compareTo(s1) > 0;
}
