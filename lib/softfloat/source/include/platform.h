#define SOFTFLOAT_FAST_INT64
#define LITTLEENDIAN

#define SOFTFLOAT_ROUND_ODD
//#define INLINE_LEVEL 5
#define SOFTFLOAT_FAST_DIV32TO16
#define SOFTFLOAT_FAST_DIV64TO32

#ifdef BOXEDWINE_MSVC
#define THREAD_LOCAL __declspec( thread )
#else
#define THREAD_LOCAL __thread
#endif