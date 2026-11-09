/* Native i386 and Boxedwine regression: pthreads inherit both FP environments.
 * gcc -m32 -O2 -msse2 pthread-fenv.c -pthread -lm -o pthread-fenv
 */
#include <fenv.h>
#include <pthread.h>
#include <stdio.h>
#include <xmmintrin.h>

static unsigned short expected_cw;
static unsigned expected_mxcsr;
static int failures;

static unsigned short control_word(void)
{
    unsigned short cw;
    __asm__ volatile ("fnstcw %0" : "=m" (cw));
    return cw;
}

static void *worker(void *unused)
{
    unsigned short cw = control_word();
    unsigned mxcsr = _mm_getcsr();
    printf("child x87=%04x SSE=%04x; expected %04x/%04x\n", cw, mxcsr, expected_cw, expected_mxcsr);
    if (cw != expected_cw || mxcsr != expected_mxcsr) ++failures;
    /* A child's changes must not change its creator's environment. */
    fesetround(FE_TOWARDZERO);
    return NULL;
}

int main(void)
{
    pthread_t thread;
    int modes[] = { FE_TONEAREST, FE_DOWNWARD, FE_UPWARD };
    for (unsigned i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
        fesetenv(FE_DFL_ENV);
        fesetround(modes[i]);
        expected_cw = control_word();
        expected_mxcsr = _mm_getcsr();
        if (pthread_create(&thread, NULL, worker, NULL) || pthread_join(thread, NULL)) return 2;
        if (control_word() != expected_cw || _mm_getcsr() != expected_mxcsr) ++failures;
    }
    printf("pthread floating-point environment: %d failures\n", failures);
    return failures != 0;
}
