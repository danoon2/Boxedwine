/*
 * GLES guest stub.
 *
 * This intentionally builds from the same generated pass-through source as
 * tools/fs_passthrough/opengl/libGL.so so GLES entry points use the same BoxedWine call
 * indexes from tools/fs_passthrough/opengl/gldef.h and tools/fs_passthrough/opengl/gldef_ext.h.
 */
#include "../opengl/gl.c"
