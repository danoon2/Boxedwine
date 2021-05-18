#include "boxedwine.h"
#ifdef BOXEDWINE_DYNAMIC
#include <sys/mman.h>

void freeExecutable64kBlock(void* p) {
    // :TODO:
    // munmap(p, 0);
}

void* allocExecutable64kBlock(int count) {
    void* result = mmap(NULL, 64 * 1024 * count, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE | MAP_BOXEDWINE, -1, 0);
    if (result == MAP_FAILED) {
        kpanic("allocExecutable64kBlock: failed to commit memory %s", strerror(errno));
    }
    return result;
}
#endif
