#ifndef __PAGE_TYPE_H__
#define __PAGE_TYPE_H__

// this index and later will have ram pages
#define HAS_RAM_PAGE_INDEX 2

// can't have more than 8, see struct MemInfo
//
// keep in sync with KMemoryData::KMemoryData
enum class PageType {
    Invalid_Page = 0,
    File_Page,
    RAM_Page,
    Code_Page,
    Copy_On_Write_Page,    
};

#endif