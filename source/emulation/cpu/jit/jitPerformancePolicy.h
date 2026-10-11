/* Validated native defaults and explicit performance experiment controls. */
#ifndef BOXEDWINE_JIT_PERFORMANCE_POLICY_H
#define BOXEDWINE_JIT_PERFORMANCE_POLICY_H
#include <cstdlib>
#include <cstring>
inline bool nativeJitPerformanceEnabled(const char* name, bool productionDefault) {
#if defined(__TEST) || defined(BOXEDWINE_JIT_MEMORY_EXPERIMENT)
    (void)productionDefault;
    const char* value = std::getenv(name);
    return value && std::strcmp(value, "0") != 0;
#elif defined(BOXEDWINE_JIT_X64) && defined(__linux__)
    (void)name;
    return productionDefault;
#else
    (void)name;
    (void)productionDefault;
    return false;
#endif
}
#endif
