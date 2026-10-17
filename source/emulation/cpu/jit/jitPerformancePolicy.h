/* Validated native defaults and explicit performance experiment controls. */
#ifndef BOXEDWINE_JIT_PERFORMANCE_POLICY_H
#define BOXEDWINE_JIT_PERFORMANCE_POLICY_H
#include <cstdlib>
#include <cstring>
inline bool nativeJitPerformanceEnabled(const char* name, bool productionDefault) {
#if defined(__TEST) || defined(BOXEDWINE_JIT_MEMORY_EXPERIMENT)
    const char* value = std::getenv(name);
    if (value) return std::strcmp(value, "0") != 0;
#ifdef __TEST
    // Exercise the host's release policies by default. Serial policy tests
    // cover the fallbacks in the same run; explicit overrides still win.
    const char* baseline = std::getenv("BOXEDWINE_TEST_JIT_BASELINE");
    if (baseline && std::strcmp(baseline, "0") != 0) return false;
#else
    // Standalone performance experiments retain their opt-in controls.
    return false;
#endif
#endif
#if defined(BOXEDWINE_JIT_X64) && defined(__linux__)
    (void)name;
    return productionDefault;
#elif defined(BOXEDWINE_JIT_X64) && defined(_WIN32)
    // Only the carry group has shown a repeatable Windows x64 benefit.
    // Keep the memory, return and FPU policies independently selectable.
    return productionDefault && (
        !std::strcmp(name, "BOXEDWINE_TEST_TRUST_ADJACENT_CARRY") ||
        !std::strcmp(name, "BOXEDWINE_TEST_TRUST_HOST_CARRY") ||
        !std::strcmp(name, "BOXEDWINE_TEST_ZERO_HOST_CARRY") ||
        !std::strcmp(name, "BOXEDWINE_TEST_TRUST_CARRY_MOVES") ||
        !std::strcmp(name, "BOXEDWINE_TEST_NATIVE_CARRY_RR_ONLY") ||
        !std::strcmp(name, "BOXEDWINE_TEST_NATIVE_CARRY_ARITH") ||
        !std::strcmp(name, "BOXEDWINE_TEST_FLAGS_MOVZX") ||
        !std::strcmp(name, "BOXEDWINE_TEST_MOVZX_ADC_ONLY") ||
        !std::strcmp(name, "BOXEDWINE_TEST_MOVZX_CARRY_ONLY"));
#else
    (void)name;
    (void)productionDefault;
    return false;
#endif
}
#endif
