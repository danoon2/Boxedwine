#ifndef __MEMINFO_H__
#define __MEMINFO_H__

// if ramPageIndex, write or read change position, JITs will need to be updated
struct MemInfo {
    static MemInfo empty;

    void updatePermissionCache();

    U32 ramPageIndex : 20;
    U32 type : 3; // see Page::Type
    U32 flags : 7; // see kmemory.h
    U32 write : 1;
    U32 read : 1; // direct read is in the top bit, makes it easy to check with a signed < 0 compare
};

#endif