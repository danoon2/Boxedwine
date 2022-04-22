#include "boxedwine.h"

MappedFileCache::~MappedFileCache() {
    for (U32 i = 0; i < this->dataSize; i++) {
        if (this->data[i]) {
            delete[] this->data[i];
        }
    }
    delete[] this->data;
}