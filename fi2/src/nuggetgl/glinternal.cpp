#include "glinternal.h"

namespace nugget::gl {

// definition
#define GLDEF(a,b,c) GLFUNC_##a a;
#include "gldefs.h"
#undef GLDEF

// initialisation
void InitFunctionPointers() {
    // example: Clear = GetGLFunctionAddress<GLFUNC_Clear>("gl" "Clear")
#define GLDEF(a,b,c) a = GetGLFunctionAddress<GLFUNC_##a>(#a);
#include "gldefs.h"
#undef GLDEF
}

}