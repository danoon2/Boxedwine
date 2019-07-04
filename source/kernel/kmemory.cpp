#include "boxedwine.h"

MappedFileCache::~MappedFileCache() {
    delete[] this->data;
}