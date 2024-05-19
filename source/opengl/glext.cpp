#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"

extern BHashTable<BString, void*> glFunctionMap;

const char* glIsLoaded[GL_FUNC_COUNT];

void glExtensionsLoaded() {
    void* pfn = nullptr;

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) glIsLoaded[func]="gl"#func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) glIsLoaded[func]="gl"#func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) if (ext_gl##func) glIsLoaded[func]="gl"#func;

#include "glfunctions.h"
    for (U32 i=0;i<GL_FUNC_COUNT;i++) {
        if (glIsLoaded[i])
            glFunctionMap.set(B(glIsLoaded[i]), pfn);
    }

}
#endif
