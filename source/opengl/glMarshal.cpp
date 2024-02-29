#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

#define MARSHAL_TYPE(type, p, m, s) type* buffer##p; U32 buffer##p##_len; type* marshal##p(CPU* cpu, U32 address, U32 count) {if (!address) return nullptr; if (!count) return (type*)address; if (buffer##p && buffer##p##_len<count) { delete[] buffer##p; buffer##p=nullptr;} if (!buffer##p) {buffer##p = new type[count]; buffer##p##_len = count;}for (U32 i=0;i<count;i++) {buffer##p[i] = cpu->memory->read##m(address);address+=s;} return buffer##p;}

U32 marshalTypeSize(U32 type) {
    switch (type) {
    case GL_UNSIGNED_BYTE: return 1;
    case GL_BYTE: return 1;
    case GL_2_BYTES: return 2;
    case GL_UNSIGNED_SHORT: return 2;
    case GL_SHORT: return 2;
    case GL_3_BYTES: return 3;
    case GL_4_BYTES: return 4;
    case GL_FLOAT: return 4;
    case GL_UNSIGNED_INT: return 4;
    case GL_INT: return 4;
    default:
        kpanic("marshalType unknown type: %d", type);
        return 1;
    }
}

GLvoid* marshalType(CPU* cpu, U32 type, U32 count, U32 address) {
    GLvoid* data= nullptr;

    if (!address)
        return nullptr;

    U32 len = count * marshalTypeSize(type);
    U32 page = address >> K_PAGE_SHIFT;
    U32 pageStop = (address + len - 1) >> K_PAGE_SHIFT;
    if (page == pageStop && cpu->memory->canRead(page) && cpu->memory->canWrite(page)) {
        return (GLvoid*)cpu->memory->getIntPtr(address);
    }

    switch (type) {
        case GL_UNSIGNED_BYTE:
            data = marshalArray<GLubyte>(cpu, address, count);
            break;
        case GL_BYTE: 
            data = marshalArray<GLbyte>(cpu, address, count);
            break;
        case GL_2_BYTES:
            data = marshalArray<GLbyte>(cpu, address, count*2);
            break;
        case GL_UNSIGNED_SHORT:
            data = marshalArray<GLushort>(cpu, address, count);
            break;
        case GL_SHORT: 
            data = marshalArray<GLshort>(cpu, address, count);
            break;
        case GL_3_BYTES:
            data = marshalArray<GLbyte>(cpu, address, count*3);
            break;
        case GL_4_BYTES:
            data = marshalArray<GLbyte>(cpu, address, count*4);
            break;
        case GL_FLOAT:
            data = marshalArray<GLfloat>(cpu, address, count);
            break;
        case GL_UNSIGNED_INT:
            data = marshalArray<GLuint>(cpu, address, count);
            break;
        case GL_INT:
            data = marshalArray<GLint>(cpu, address, count);
            break;
        default:
            kpanic("marshalType unknown type: %d", type);
    }
    return data;
}

void marshalBackType(CPU* cpu, U32 type, U32 count, GLvoid* buffer, U32 address) {
    if (!address)
        return;
    switch (type) {
        case GL_UNSIGNED_BYTE:
            marshalBackArray<GLubyte>(cpu, (GLubyte*)buffer, address, count);
            break;
        case GL_BYTE: 
            marshalBackArray<GLbyte>(cpu, (GLbyte*)buffer, address, count);
            break;
        case GL_2_BYTES:
            marshalBackArray<GLbyte>(cpu, (GLbyte*)buffer, address, count*2);
            break;
        case GL_UNSIGNED_SHORT:
            marshalBackArray<GLushort>(cpu, (GLushort*)buffer, address, count);
            break;
        case GL_SHORT: 
            marshalBackArray<GLshort>(cpu, (GLshort*)buffer, address, count);
            break;
        case GL_3_BYTES:
            marshalBackArray<GLbyte>(cpu, (GLbyte*)buffer, address, count*3);
            break;
        case GL_4_BYTES:
            marshalBackArray<GLbyte>(cpu, (GLbyte*)buffer, address, count*4);
            break;
        case GL_FLOAT:
            marshalBackArray<GLfloat>(cpu, (GLfloat*)buffer, address, count);
            break;
        case GL_UNSIGNED_INT:
            marshalBackArray<GLuint>(cpu, (GLuint*)buffer, address, count);
            break;
        case GL_INT:
            marshalBackArray<GLint>(cpu, (GLint*)buffer, address, count);
            break;
        default:
            kpanic("marshalType unknown type: %d", type);
    }
}

MarshalReadWriteType::~MarshalReadWriteType() {
    if (buffer) {
        marshalBackType(cpu, type, count, buffer, address);
    }
}

GLvoid* MarshalReadWriteType::getPtr() {
    if (!buffer) {
        buffer = marshalType(cpu, type, count, address);
    }
    return buffer;
}

GLvoid* marshalPixel(CPU* cpu, GLenum format, GLenum type, U32 pixel) {
    int len = components_in_format (format);
    
    if (!pixel)
        return nullptr;

    switch (type) {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_BYTE: 
        return marshalArray<GLubyte>(cpu, pixel, 1);
    case GL_BYTE:
        return marshalArray<GLbyte>(cpu, pixel, 1);
    case GL_BITMAP:
        return marshalArray<GLubyte>(cpu, pixel, 1);
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT:
        return marshalArray<GLushort>(cpu, pixel, len);
    case GL_SHORT:
        return marshalArray<GLshort>(cpu, pixel, len);
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT:
        return marshalArray<GLuint>(cpu, pixel, len);
    case GL_INT:
        return marshalArray<GLint>(cpu, pixel, len);
    case GL_FLOAT:
        return marshalArray<GLfloat>(cpu, pixel, len);
    default:
        kpanic("glcommongl.c marshalPixels uknown type: %d", type);
        return nullptr;
    }
}

U32 getPixelsLen(bool read, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, int& bytes_per_comp, int& isSigned) {
    int bytes_per_row = 0;
    int remainder = 0;

    GLint skipPixels = 0;
    GLint skipRows = 0;
    GLint alignment = 0;
    GLint pixels_per_row = 0;
    GLint skipImages = 0;

    if (read) {
        GL_FUNC(pglGetIntegerv)(GL_UNPACK_ROW_LENGTH, &pixels_per_row);
        GL_FUNC(pglGetIntegerv)(GL_UNPACK_SKIP_PIXELS, &skipPixels);
        GL_FUNC(pglGetIntegerv)(GL_UNPACK_SKIP_ROWS, &skipRows);
        GL_FUNC(pglGetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
        if (is3d) {
            GL_FUNC(pglGetIntegerv)(GL_UNPACK_SKIP_IMAGES, &skipImages);
        }
    } else {
        GL_FUNC(pglGetIntegerv)(GL_PACK_ROW_LENGTH, &pixels_per_row);
        GL_FUNC(pglGetIntegerv)(GL_PACK_SKIP_PIXELS, &skipPixels);
        GL_FUNC(pglGetIntegerv)(GL_PACK_SKIP_ROWS, &skipRows);
        GL_FUNC(pglGetIntegerv)(GL_PACK_ALIGNMENT, &alignment);
        if (is3d) {
            GL_FUNC(pglGetIntegerv)(GL_PACK_SKIP_IMAGES, &skipImages);
        }
    }
    if (!pixels_per_row)
        pixels_per_row = width;
    if (type == GL_BITMAP) {
        bytes_per_comp = 1;
        bytes_per_row = (pixels_per_row + 7) / 8;
    } else {
        int bytes_per_pixel;

        bytes_per_pixel = get_bytes_per_pixel(format, type);
        bytes_per_row = pixels_per_row * bytes_per_pixel;
    }
    remainder = bytes_per_row % alignment;
    if (remainder > 0)
        bytes_per_row += (alignment - remainder);

    switch (type) {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_BYTE: 
        bytes_per_comp = 1;
        break;
    case GL_BYTE:
        bytes_per_comp = 1;
        isSigned = 1;
        break;
    case GL_BITMAP:
        break;
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT:
        bytes_per_comp = 2;
        break;
    case GL_SHORT:
        bytes_per_comp = 2;
        isSigned = 1;
        break;
    case GL_UNSIGNED_INT_24_8:
        bytes_per_comp = 3;
        isSigned = 1;
        break;
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT:
        bytes_per_comp = 4;
        break;
    case GL_INT:
        bytes_per_comp = 4;
        isSigned = 1;
        break;
    case GL_FLOAT:
        bytes_per_comp = 0;
        break;
    default:
        kpanic("glcommongl.c marshalBackPixels uknown type: %d", type);
    }
    return bytes_per_row * (height + skipRows) * (depth + skipImages);
}

GLvoid* marshalPixels(CPU* cpu, int bytes_per_comp, int isSigned, U32 pixels, U32 len) {
    if (bytes_per_comp == 0) {
        return marshalArray<GLfloat>(cpu, pixels, len / 4);
    } else if (bytes_per_comp == 1) {
        if (isSigned) {
            return marshalArray<GLbyte>(cpu, pixels, len);
        } else {
            return marshalArray<GLubyte>(cpu, pixels, len);
        }
    } else if (bytes_per_comp == 2) {
        if (isSigned) {
            return marshalArray<GLshort>(cpu, pixels, len / 2);
        } else {
            return marshalArray<GLushort>(cpu, pixels, len / 2);
        }
    } else if (bytes_per_comp == 3) {
        if (isSigned) {
            return marshalArray<GLbyte>(cpu, pixels, len);
        } else {
            return marshalArray<GLubyte>(cpu, pixels, len);
        }
    } else if (bytes_per_comp == 4) {
        if (isSigned) {
            return marshalArray<GLint>(cpu, pixels, len / 4);
        } else {
            return marshalArray<GLuint>(cpu, pixels, len / 4);
        }
    }
    kpanic("glcommongl.c marshalPixels unknown bytes_per_comp %d", bytes_per_comp);
    return nullptr;
}

GLvoid* marshalPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, U32 pixels, U32 xoffset, U32 yoffset, U32 level) {
    if (!pixels) {
        return nullptr;
    }
    int bytes_per_comp = 0;
    int isSigned = 0;
    GLint w = width + xoffset;
    GLint h = height;
    GLint alignment = 0;
    GL_FUNC(pglGetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);

    if (alignment) {
        h = (h + alignment - 1) / alignment * alignment; // I don't see any specs on this, but it will crash if I don't do this
        w = (w + alignment - 1) / alignment * alignment;
    }
    U32 len = getPixelsLen(true, is3d, w, h, depth, format, type, bytes_per_comp, isSigned);

    return marshalPixels(cpu, bytes_per_comp, isSigned, pixels, len);
}

void marshalBackPixels(CPU* cpu, int bytes_per_comp, int isSigned, U32 address, GLvoid* pixels, U32 len) {
    if (bytes_per_comp == 0) {
        marshalBackArray<GLfloat>(cpu, (GLfloat*)pixels, address, len / 4);
    } else if (bytes_per_comp == 1) {
        if (isSigned) {
            marshalBackArray<GLbyte>(cpu, (GLbyte*)pixels, address, len);
        } else {
            marshalBackArray<GLubyte>(cpu, (GLubyte*)pixels, address, len);
        }
    } else if (bytes_per_comp == 2) {
        if (isSigned) {
            marshalBackArray<GLshort>(cpu, (GLshort*)pixels, address, len / 2);
        } else {
            marshalBackArray<GLushort>(cpu, (GLushort*)pixels, address, len / 2);
        }
    } else if (bytes_per_comp == 4) {
        if (isSigned) {
            marshalBackArray<GLint>(cpu, (GLint*)pixels, address, len / 4);
        } else {
            marshalBackArray<GLuint>(cpu, (GLuint*)pixels, address, len / 4);
        }
    } else {
        kpanic("glcommongl.c marshalBackPixels unknown bytes_per_comp %d", bytes_per_comp);
    }
}

void marshalBackPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, U32 address, GLvoid* pixels) {
    if (!pixels) {
        return;
    }

    int bytes_per_comp = 0;
    int isSigned = 0;
    U32 len = getPixelsLen(false, is3d, width, height, depth, format, type, bytes_per_comp, isSigned);
    marshalBackPixels(cpu, bytes_per_comp, isSigned, address, pixels, len);    
}

MarshalReadWritePackedPixels::~MarshalReadWritePackedPixels() {
    if (buffer) {
        marshalBackPixels(cpu, bytes_per_comp, isSigned, pixels, buffer, len);
    }
}

GLvoid* MarshalReadWritePackedPixels::getPtr() {
    if (packed) {
        return (GLvoid*)(uintptr_t)(pixels);
    }
    if (!buffer) {
        len = getPixelsLen(false, is3d, width, height, depth, format, type, bytes_per_comp, isSigned);
        if (!len) {
            return (GLvoid*)(U64)pixels;
        }
        U32 page = pixels >> K_PAGE_SHIFT;
        U32 pageStop = (pixels + len - 1) >> K_PAGE_SHIFT;
        if (page == pageStop && cpu->memory->canRead(page) && cpu->memory->canWrite(page)) {
            return (GLvoid*)cpu->memory->getIntPtr(pixels, true);
        }
        buffer = marshalPixels(cpu, bytes_per_comp, isSigned, pixels, len);
    }
    return buffer;
}

U32 marshalBackp(CPU* cpu, GLvoid* buffer, U32 size) { 
    if (size == 0) {
        kpanic("bad OpenGL marshalBackp");
    }
    kpanic("need to implement marshalBackp");
    return 0;
}

class BufferedTarget {
public:
    BufferedTarget() = default;
    BufferedTarget(U32 bufferedAddress, S8* originalBufferedAddress, U32 size) : bufferedAddress(bufferedAddress), originalBufferedAddress(originalBufferedAddress), size(size) {}
    U32 bufferedAddress = 0;
    S8* originalBufferedAddress = nullptr;
    U32 size = 0;
};

static BHashTable<U32, std::shared_ptr<BufferedTarget>> bufferedTargets;

U32 mapBufferRange(CPU* cpu, GLenum target, GLvoid* buffer, U32 offset, U32 size) {
    if (bufferedTargets.contains(target)) {
        kwarn("mapBufferRange already mapped");
    }
    U32 result = cpu->memory->mapNativeMemory(buffer, size);
    bufferedTargets.set(target, std::make_shared<BufferedTarget>(result, (S8*)buffer, size));
    return result;
}

void unmapBuffer(CPU* cpu, GLenum target) {
    std::shared_ptr<BufferedTarget> t = bufferedTargets[target];
    if (t) {
        cpu->memory->unmapNativeMemory(t->bufferedAddress, t->size);
        bufferedTargets.remove(target);
    }
}

// instance is in the instance number within the function, so if the same function calls this 3 times, each call will have a difference instance
GLvoid* marshalp(CPU* cpu, U32 instance, U32 buffer, U32 len) {
    if (buffer == 0)
        return nullptr;
    if (buffer <0x10000) {
        return (GLvoid*)(uintptr_t)buffer;
    }
    if ((buffer & 0xFFF) + len > 0xFFF) {
        return marshalArray<GLubyte>(cpu, buffer, len);
    }
    // :TODO: a lot of work needs to be done here, marshalp needs to be removed and instead marshal the correct type of array, like marshalf.
    // This is also important to make things work with UNALIGNED_MEMORY
    return (GLvoid*)cpu->memory->getIntPtr(buffer);
}

U32 marshalBackSync(CPU* cpu, GLsync sync) {
    //klog("marshalBackSync not implemented");
#ifdef BOXEDWINE_64
    return (U32)(U64)sync;
#else
    return (U32)sync;
#endif
}

GLsync marshalSync(CPU* cpu, U32 sync) {
    //klog("marshalSync not implemented");
    return (GLsync)(U32)sync;
}

GLvoid** bufferpp;
U32 bufferpp_len;

GLvoid** marshalpp(CPU* cpu, U32 buffer, U32 count, U32 sizes, S32 bytesPerCount, U32 autoCharWidth) {
    if (bytesPerCount==-1 && autoCharWidth!=1) {
        kpanic("OpenGL marshalpp can't handle multi-byte strings");
    }
    if (!buffer)
        return nullptr;

    if (bufferpp && bufferpp_len<count) {
        delete[] bufferpp;
        bufferpp= nullptr;
    }
    if (!bufferpp) {
        bufferpp = new GLvoid*[count];
        bufferpp_len = count;
    }
    for (U32 i=0;i<count;i++) {
        S32 len = 0;
        U32 p = cpu->memory->readd(buffer+i*4);
        if (sizes) {
            U32 address = cpu->memory->readd(sizes+i*4);
            len = (S32)cpu->memory->readd(address);
        }
        if (bytesPerCount) {
            if (bytesPerCount==-1 && len<=0) {
                len = cpu->memory->strlen(p)+1;
            } else {
                len*=bytesPerCount;
            }
        }
        // :TODO: this is wrong if the host address isn't used directly
        bufferpp[i] = marshalp(cpu, i, p, len);
    }
    return bufferpp;
}

void* marshalunhandled(const char* func, const char* param, CPU* cpu, U32 address) {
    klog("%s parameter in OpenGL function, %s, was not marshalled", func, param);
    return nullptr;
}

/*
typedef  struct {
    uint  count;
    uint  primCount;
    uint  first;
    uint  baseInstance;
} DrawArraysIndirectCommand;

typedef struct {
    GLuint   index;
    GLuint   reserved;
    GLuint64 address;
    GLuint64 length;
} BindlessPtrNV;

typedef struct {
    DrawArraysIndirectCommand   cmd;
    GLuint                      reserved;
    BindlessPtrNV               vertexBuffers[];
} DrawArraysIndirectBindlessCommandNV;
*/

void* marshalDrawArraysIndirectBindlessCommandNV(CPU* cpu, U32 address, U32 count, U32 stride, U32 vertexCount) {
    //U32 size = 20 + 24*vertexCount;

    kpanic("DrawArraysIndirectBindlessCommandNV was not marshalled");
    return nullptr;
}

#ifndef DISABLE_GL_EXTENSIONS
GLvoid* marshalGetConvolutionFilter(CPU* cpu, U32 target, U32 format, U32 type, U32 image) {
    GLint width = 0;
    GLint height = 0;
    //U32 len = 0;

    if (PIXEL_PACK_BUFFER())
        return (GLubyte*)(uintptr_t)image;
    if (ext_glGetConvolutionParameteriv) {
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &width);
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &height);
    }
    return marshalType(cpu, type, components_in_format(format)*width*height, image);
}
#endif

GLint marshalGet(GLenum param) {
    GLint result = 0;
    GL_FUNC(pglGetIntegerv)(param, &result);
    return result;
}

GLboolean PIXEL_PACK_BUFFER() {
    return marshalGet(GL_PIXEL_PACK_BUFFER_BINDING)!=0;
}

GLboolean ARRAY_BUFFER() {
    return marshalGet(GL_ARRAY_BUFFER_BINDING)!=0;
}

GLboolean RESULT_BUFFER() {
    return marshalGet(GL_QUERY_BUFFER_BINDING) != 0;
}

GLboolean ELEMENT_ARRAY_BUFFER() {
    return marshalGet(GL_ELEMENT_ARRAY_BUFFER_BINDING)!=0;
}

GLboolean PIXEL_UNPACK_BUFFER() {
    return marshalGet(GL_PIXEL_UNPACK_BUFFER_BINDING)!=0;
}

const char* glcommon_glLightv_print_name(GLenum e) {
    thread_local static BString buffer; // static so that c_str can be returned
    if (e >= GL_LIGHT0 && e < GL_LIGHT0 + GL_MAX_LIGHTS) {
        buffer = "GL_LIGHT" + BString::valueOf(e - GL_LIGHT0);
    }
    else {
        buffer = BString::valueOf(e);
    }
    return buffer.c_str();
}

const char* glcommon_glLightv_print_buffer(GLenum e, GLfloat* buffer) {
    thread_local static char tmp[64]; // static so that it can be returned

    switch (e) {
    case GL_SPOT_EXPONENT:
    case GL_SPOT_CUTOFF:
    case GL_CONSTANT_ATTENUATION:
    case GL_LINEAR_ATTENUATION:
    case GL_QUADRATIC_ATTENUATION:
        snprintf(tmp, sizeof(tmp), "%0.2f", buffer[0]);
        return tmp;
    case GL_SPOT_DIRECTION:
        snprintf(tmp, sizeof(tmp), "{%0.2f,%0.2f,%0.2f}", buffer[0], buffer[1], buffer[2]);
        return tmp;
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_POSITION:
        snprintf(tmp, sizeof(tmp), "{%0.2f,%0.2f,%0.2f,%0.2f}", buffer[0], buffer[1], buffer[2], buffer[3]);
        return tmp;
    default:
        return "Unknown";
    }
}

const char* glcommon_glLightv_print_pname(GLenum e)
{
    switch (e) {
    case GL_SPOT_EXPONENT: return "GL_SPOT_EXPONENT";
    case GL_SPOT_CUTOFF: return "GL_SPOT_CUTOFF";
    case GL_CONSTANT_ATTENUATION: return "GL_CONSTANT_ATTENUATION";
    case GL_LINEAR_ATTENUATION: return "GL_LINEAR_ATTENUATION";
    case GL_QUADRATIC_ATTENUATION: return "GL_QUADRATIC_ATTENUATION";
    case GL_SPOT_DIRECTION: return "GL_SPOT_DIRECTION";
    case GL_AMBIENT: return "GL_AMBIENT";
    case GL_DIFFUSE: return "GL_DIFFUSE";
    case GL_SPECULAR: return "GL_SPECULAR";
    case GL_POSITION: return "GL_POSITION";
    default: {
        thread_local static BString buffer; // static so it can return c_str
        buffer = BString::valueOf(e);
        return buffer.c_str();
    }
    }
}

const char* glcommon_glClear_mask(GLbitfield mask) {
    thread_local static BString buffer; // static so that it can return c_str
    buffer = "";
    if (mask & GL_COLOR_BUFFER_BIT) {
        mask &= ~GL_COLOR_BUFFER_BIT;
        buffer += "GL_COLOR_BUFFER_BIT";
    }
    if (mask & GL_DEPTH_BUFFER_BIT) {
        mask &= ~GL_DEPTH_BUFFER_BIT;
        if (buffer.length()) {
            buffer += "|";
        }
        buffer += "GL_DEPTH_BUFFER_BIT";
    }
    if (mask & GL_ACCUM_BUFFER_BIT) {
        mask &= ~GL_ACCUM_BUFFER_BIT;
        if (buffer.length()) {
            buffer += "|";
        }
        buffer += "GL_ACCUM_BUFFER_BIT";
    }
    if (mask & GL_STENCIL_BUFFER_BIT) {
        mask &= ~GL_STENCIL_BUFFER_BIT;
        if (buffer.length()) {
            buffer += "|";
        }
        buffer += "GL_STENCIL_BUFFER_BIT";
    }
    if (mask) {
        if (buffer.length()) {
            buffer += "|";
        }
        buffer += BString::valueOf(mask);
    }
    return buffer.c_str();
}

#ifdef BOXEDWINE_GLHANDLE_ARB_POINTER
GLhandleARB* handleBuffer;
U32 handleBufferSize;

U32 marshalHandleToIndex(GLhandleARB h) {
    if (!handleBufferSize) {
        handleBuffer = new GLhandleARB[1024];
        handleBufferSize = 1024;
        memset(handleBuffer, 0, sizeof(GLhandleARB)*handleBufferSize);
    }
    for (U32 i=0;i<handleBufferSize;i++) {
        if (handleBuffer[i]==h) {
            return i;
        }
    }
    for (U32 i=0;i<handleBufferSize;i++) {
        if (handleBuffer[i]==NULL) {
            handleBuffer[i] = h;
            return i;
        }
    }
    GLhandleARB* b = new GLhandleARB[handleBufferSize*2];
    memset(handleBuffer, 0, sizeof(GLhandleARB)*handleBufferSize*2);
    memcpy(b, handleBuffer, handleBufferSize);
    U32 result = handleBufferSize;
    handleBufferSize*=2;
    delete[] handleBuffer;
    handleBuffer = b;
    return result;
}

GLhandleARB marshalIndexToHandle(U32 i) {
    if (handleBuffer && i<handleBufferSize) {
        return handleBuffer[i];
    }
    return NULL;
}

void marshalDeleteHandleIndex(U32 i) {
    if (handleBuffer && i<handleBufferSize) {
        handleBuffer[i] = NULL;
    }
}

GLhandleARB* bufferhandle;
U32 bufferhandle_len;

GLhandleARB* marshalhandle(CPU* cpu, U32 address, U32 count) {
    if (!address)
        return NULL;
    if (bufferhandle && bufferhandle_len<count) {
        delete[] bufferhandle;
        bufferhandle=NULL;
    }
    if (!bufferhandle) {
        bufferhandle = new GLhandleARB[count];
        bufferhandle_len = count;
    }
    /*
     // not needed
    for (i=0;i<count;i++) {
        bufferhandle[i] = INDEX_TO_HANDLE(readd(address));
        address+=4;
    }
     */
    return bufferhandle;
}

void marshalBackhandle(CPU* cpu, U32 address, GLhandleARB* buffer, U32 count) {
    U32 i;
    
    for (i=0;i<count;i++) {
        cpu->memory->writed(address, HANDLE_TO_INDEX(buffer[i]));
        address+=4;
    }
}
#else
GLhandleARB* marshalhandle(CPU* cpu, U32 address, U32 count) {
    return marshalArray<GLhandleARB, 4>(cpu, address, count);
}

void marshalBackhandle(CPU* cpu, U32 address, GLhandleARB* buffer, U32 count) {
    marshalBackArray<GLhandleARB, 4>(cpu, buffer, address, count);
}
#endif

#endif
