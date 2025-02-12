#ifndef __SOFT_MMU_H__
#define __SOFT_MMU_H__

#include "soft_page.h"

class MMU {
public:
    U32 ramIndex : 20;
    U32 type : 3;
    U32 pad : 1;    
    U32 flags : 6;        
    U32 canReadRam : 1;    
    U32 canWriteRam : 1;

    RamPage getRamPageIndex() {
        if (getPageType() != PageType::File) {
            return (RamPage)ramIndex;
        }
        return (RamPage)0;
    }

    PageType getPageType() {
        return (PageType)type;
    }

    Page* getPage() {
        return Page::getPage(getPageType());
    }

    void setPageType(KMemoryData* mem, U32 page, PageType type);
    void setPage(KMemoryData* mem, U32 page, PageType type, RamPage ram);
    void setPermissions(U32 permissions);
    void setFlags(U32 flags);

private:
    void onPageChanged();
};

#endif