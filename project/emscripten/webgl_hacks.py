#!/usr/bin/python
import sys, os, subprocess

def error(s):
    print >> sys.stderr, s
    sys.exit(1)

def getfiletext(fn):
    try:
        f = open(fn, 'r')
        txt = f.read()
    except Exception, e:
        error('Error reading file: %s' % (str(e)))
    f.close
    return txt

try:
    txt = getfiletext('boxedwine.js')
    
    func_emscriptenWebGLGet = """\
    var result = null;//kev-hack
    if (name_ == 34811) { //0x87FB GL_VBO_FREE_MEMORY_ATI
        result = new Int32Array(4);
        result0 = 524288;
        result1 = 0;
        result2 = 0;
        result3 = 0;
    } else if (name_ == 36935) {//0x9047 GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
        result = new Int32Array(1);
        result0 = 524288;
    } else if (name_ == 32883) {//0x8073 GL_MAX_3D_TEXTURE_SIZE
        result = 2048;
    } else if (name_ == 35658) {//0x8B4A GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB
        result = 16384;
    } else if (name_ == 35657) {//0x8B49 GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
        result = 2048; //16384;
    } else if (name_ == 35659) {//0x8B4B GL_MAX_VARYING_FLOATS_ARB or GL_MAX_VARYING_VECTORS ?
        result = 16;
    } else if (name_ == 36183) {//0x8D57 GL_MAX_SAMPLES
        result = 8;
    } else {
        try {
            result = GLctx.getParameter(name_);
        } catch (ex) {
            console.log("emscriptenWebGLGet ex=" + ex);
        }
    }
    """
    txt = txt.replace("var result = GLctx.getParameter(name_);", func_emscriptenWebGLGet)
    
    func_glTexImage2D = """\
    let tex = GLctx["createTexture"]()//kev-hack
    GLctx["bindTexture"](3553, tex)
    GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null)
    """
    txt = txt.replace("GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixels ? emscriptenWebGLGetTexPixelData(type, format, width, height, pixels, internalFormat) : null);", func_glTexImage2D)
    outf = open('boxedwine.js', 'w')
    outf.write(txt)
    outf.close
except Exception, e:
    error('Error ')

