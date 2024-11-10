#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"

extern BHashTable<BString, void*> glFunctionMap;

const char* glIsLoaded[GL_FUNC_COUNT];

void glExtensionsLoaded() {


}
#endif
