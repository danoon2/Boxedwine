#if !defined(SIMDE_TEST_X86_TEST_SSE_H)
#define SIMDE_TEST_X86_TEST_SSE_H

#include "test-x86.h"
#include "test-mmx.h"
#include "../../simde/x86/sse.h"

SIMDE_TEST_X86_GENERATE_FLOAT_TYPE_FUNCS_(__m128, 32, 4, simde_mm_storeu_ps)

#define simde_test_x86_assert_equal_f32x4(a, b, precision) do { if (simde_test_x86_assert_equal_f32x4_(a, b, 1e-##precision##f, __FILE__, __LINE__, #a, #b)) { return 1; } } while (0)

#endif /* !defined(SIMDE_TEST_X86_TEST_SSE_H) */
