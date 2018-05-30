#include "boxedwine.h"

MappedFileCache::~MappedFileCache() {
    KSystem::eraseFileCache(this->name); 
    delete[] this->data;
}