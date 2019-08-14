#include "boxedwine.h"

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

#define MARSHAL_TYPE(type, p, m, s) type* buffer##p; U32 buffer##p##_len; type* marshal##p(CPU* cpu, U32 address, U32 count) {U32 i; if (!address) return NULL; if (buffer##p && buffer##p##_len<count) { delete[] buffer##p; buffer##p=NULL;} if (!buffer##p) {buffer##p = new type[count]; buffer##p##_len = count;}for (i=0;i<count;i++) {buffer##p[i] = read##m(address);address+=s;} return buffer##p;}

#ifdef BOXEDWINE_64BIT_MMU

#define getSize(pname) 0

//#define marshalPixels(cpu, is3d, width, height, depth, format, type, pixels) (GLvoid*)getPhysicalAddress(pixels)
GLvoid* marshalPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,  U32 pixels) {
    if (pixels == 0)
        return 0;
    return (GLvoid*)getPhysicalAddress(pixels, 0);
}

GLvoid** bufferpp;
U32 bufferpp_len;

GLvoid** marshalpp(CPU* cpu, U32 buffer, U32 count, U32 sizes, S32 bytesPerCount, U32 autoCharWidth) {
    U32 i;

    if (bufferpp && bufferpp_len<count) {
        delete[] bufferpp;
        bufferpp=0;
    }
    if (!bufferpp) {
        bufferpp = new GLvoid*[count];
        bufferpp_len = count;
    }
    for (i=0;i<count;i++) {
        bufferpp[i] = (GLvoid*)getPhysicalAddress(readd(buffer+i*4), 0);
    }
    return bufferpp;
}

GLvoid* marshalp(CPU* cpu, U32 instance, U32 buffer, U32 len) {
    if (buffer == 0)
        return NULL;
    return (GLvoid*)getPhysicalAddress(buffer, 0);
}

// this won't marshal the data, but rather map it into the address space, reserving "size" amount of address space
U32 marshalBackp(CPU* cpu, GLvoid* buffer, U32 size) {
    return cpu->thread->memory->mapNativeMemory(buffer, size);
}

U32 marshalBackSync(CPU* cpu, GLsync sync) {
    return 0;
}

GLsync marshalSync(CPU* cpu, U32 sync) {
    return 0;
}

#else 
#define MARSHAL_TYPE_CUSTOM(type, p, m, s, conv, get, set) type* buffer##p; U32 buffer##p##_len; type* marshal##p(CPU* cpu, U32 address, U32 count) {U32 i; if (!address) return NULL; if (buffer##p && buffer##p##_len<count) { delete[] buffer##p; buffer##p=NULL;} if (!buffer##p) {buffer##p = new type[count]; buffer##p##_len = count;}for (i=0;i<count;i++) {struct conv d; get = read##m(address);address+=s;buffer##p[i] = set;} return buffer##p;}

MARSHAL_TYPE(GLbyte, b, b, 1)
MARSHAL_TYPE(GLbyte, 2b, b, 1)

GLubyte* marshalub(CPU* cpu, U32 address, U32 count) {
    return (GLubyte*)marshalb(cpu, address, count);
}

GLubyte* marshal2ub(CPU* cpu, U32 address, U32 count) {
    return (GLubyte*)marshal2b(cpu, address, count);
}

GLboolean* marshalbool(CPU* cpu, U32 address, U32 count) {
    return (GLboolean*)marshalb(cpu, address, count);
}

GLboolean* marshal2bool(CPU* cpu, U32 address, U32 count) {
    return (GLboolean*)marshal2b(cpu, address, count);
}

MARSHAL_TYPE(GLshort, s, w, 2)
MARSHAL_TYPE(GLshort, 2s, w, 2)

GLushort* marshalus(CPU* cpu, U32 address, U32 count) {
    return (GLushort*)marshals(cpu, address, count);
}

GLushort* marshal2us(CPU* cpu, U32 address, U32 count) {
    return (GLushort*)marshal2s(cpu, address, count);
}

MARSHAL_TYPE(GLchar, c, b, 1)
MARSHAL_TYPE(GLchar, 2c, b, 1)
MARSHAL_TYPE(GLcharARB, ac, b, 1)
MARSHAL_TYPE(GLcharARB, 2ac, b, 1)
MARSHAL_TYPE(GLenum, e, d, 4)
MARSHAL_TYPE(GLenum, 2e, d, 4)
MARSHAL_TYPE(GLenum, 3e, d, 4)
MARSHAL_TYPE(GLint, i, d, 4)
MARSHAL_TYPE(GLint, 2i, d, 4)
MARSHAL_TYPE(GLint, 3i, d, 4)
MARSHAL_TYPE(GLint, 4i, d, 4)
MARSHAL_TYPE(GLint, 5i, d, 4)

GLuint* marshalui(CPU* cpu, U32 address, U32 count) {
    return (GLuint*)marshali(cpu, address, count);
}

GLuint* marshal2ui(CPU* cpu, U32 address, U32 count) {
    return (GLuint*)marshal2i(cpu, address, count);
}

GLuint* marshal3ui(CPU* cpu, U32 address, U32 count) {
    return (GLuint*)marshal3i(cpu, address, count);
}

GLuint* marshal4ui(CPU* cpu, U32 address, U32 count) {
    return (GLuint*)marshal4i(cpu, address, count);
}

GLuint* marshal5ui(CPU* cpu, U32 address, U32 count) {
    return (GLuint*)marshal5i(cpu, address, count);
}

MARSHAL_TYPE(GLuint64, ui64, q, 8)
MARSHAL_TYPE(GLint64, i64, q, 8)

MARSHAL_TYPE(GLsizei, si, d, 4)

MARSHAL_TYPE_CUSTOM(GLfloat, f, d, 4, int2Float, d.i, d.f)
MARSHAL_TYPE_CUSTOM(GLfloat, 2f, d, 4, int2Float, d.i, d.f)
MARSHAL_TYPE_CUSTOM(GLfloat, 3f, d, 4, int2Float, d.i, d.f)
MARSHAL_TYPE_CUSTOM(GLfloat, 4f, d, 4, int2Float, d.i, d.f)

MARSHAL_TYPE_CUSTOM(GLdouble, d, q, 8, long2Double, d.l, d.d)
MARSHAL_TYPE_CUSTOM(GLdouble, 2d, q, 8, long2Double, d.l, d.d)

void marshalBackd(CPU* cpu, U32 address, GLdouble* buffer, U32 count) {
    U32 i;

    if (address) {
        for (i=0;i<count;i++) {
            struct long2Double d;
            d.d = buffer[i];
            writeq(address, d.l);
            address+=8;
        }
    }
}

void marshalBackf(CPU* cpu, U32 address, GLfloat* buffer, U32 count) {
    U32 i;

    if (address) {
        for (i=0;i<count;i++) {
            struct int2Float f;
            f.f = buffer[i];
            writed(address, f.i);
            address+=4;
        }
    }
}

void marshalBacki(CPU* cpu, U32 address, GLint* buffer, U32 count) {
    U32 i;

    if (address) {
        for (i=0;i<count;i++) {
            writed(address, buffer[i]);
            address+=4;
        }
    }
}

void marshalBackui(CPU* cpu, U32 address, GLuint* buffer, U32 count) {
    marshalBacki(cpu, address, (GLint*)buffer, count);
}

void marshalBacki64(CPU* cpu, U32 address, GLint64* buffer, U32 count) {
    U32 i;

    if (address) {
        for (i=0;i<count;i++) {
            writeq(address, buffer[i]);
            address+=8;
        }
    }
}

void marshalBackui64(CPU* cpu, U32 address, GLuint64* buffer, U32 count) {
    marshalBacki64(cpu, address, (GLint64*)buffer, count);
}

void marshalBackus(CPU* cpu, U32 address, GLushort* buffer, U32 count) {
    U32 i;

    if (address) {
        for (i=0;i<count;i++) {
            writew(address, buffer[i]);
            address+=2;
        }
    }
}

void marshalBacks(CPU* cpu, U32 address, GLshort* buffer, U32 count) {
    marshalBackus(cpu, address, (GLushort*)buffer, count);
}

void marshalBackb(CPU* cpu, U32 address, GLbyte* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count);
}

void marshalBackc(CPU* cpu, U32 address, GLchar* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count*sizeof(GLchar));
}

void marshalBacke(CPU* cpu, U32 address, GLenum* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count*sizeof(GLenum));
}

void marshalBackac(CPU* cpu, U32 address, GLcharARB* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count*sizeof(GLcharARB));
}

void marshalBackub(CPU* cpu, U32 address, GLubyte* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count);
}

void marshalBackbool(CPU* cpu, U32 address, GLboolean* buffer, U32 count) {
    memcopyFromNative(address, (char*)buffer, count);
}

GLvoid* marshalType(CPU* cpu, U32 type, U32 count, U32 address) {
    GLvoid* data=0;

    if (!address)
        return NULL;
    switch (type) {
        case GL_UNSIGNED_BYTE:
            data = marshalub(cpu, address, count);
            break;
        case GL_BYTE: 
            data = marshalb(cpu, address, count);
            break;
        case GL_2_BYTES:
            data = marshalb(cpu, address, count*2);
            break;
        case GL_UNSIGNED_SHORT:
            data = marshalus(cpu, address, count);
            break;
        case GL_SHORT: 
            data = marshals(cpu, address, count);
            break;
        case GL_3_BYTES:
            data = marshalb(cpu, address, count*3);
            break;
        case GL_4_BYTES:
            data = marshalb(cpu, address, count*4);
            break;
        case GL_FLOAT:
            data = marshalf(cpu, address, count);
            break;
        case GL_UNSIGNED_INT:
            data = marshalui(cpu, address, count);
            break;
        case GL_INT:
            data = marshali(cpu, address, count);
            break;
        default:
            kpanic("marshalType unknown type: %d", ARG2);
    }
    return data;
}

void marshalBackType(CPU* cpu, U32 type, U32 count, GLvoid* buffer, U32 address) {
    if (!address)
        return;
    switch (type) {
        case GL_UNSIGNED_BYTE:
            marshalBackub(cpu, address, (GLubyte*)buffer, count);
            break;
        case GL_BYTE: 
            marshalBackb(cpu, address, (GLbyte*)buffer, count);
            break;
        case GL_2_BYTES:
            marshalBackb(cpu, address, (GLbyte*)buffer, count*2);
            break;
        case GL_UNSIGNED_SHORT:
            marshalBackus(cpu, address, (GLushort*)buffer, count);
            break;
        case GL_SHORT: 
            marshalBacks(cpu, address, (GLshort*)buffer, count);
            break;
        case GL_3_BYTES:
            marshalBackb(cpu, address, (GLbyte*)buffer, count*3);
            break;
        case GL_4_BYTES:
            marshalBackb(cpu, address, (GLbyte*)buffer, count*4);
            break;
        case GL_FLOAT:
            marshalBackf(cpu, address, (GLfloat*)buffer, count);
            break;
        case GL_UNSIGNED_INT:
            marshalBackui(cpu, address, (GLuint*)buffer, count);
            break;
        case GL_INT:
            marshalBacki(cpu, address, (GLint*)buffer, count);
            break;
        default:
            kpanic("marshalType unknown type: %d", ARG2);
    }
}

GLvoid* marshalPixel(CPU* cpu, GLenum format, GLenum type, U32 pixel) {
    int bytes_per_comp;
    int isSigned=0;
    int len = components_in_format (format);
    
    if (!pixel)
        return 0;

    if (type == GL_BITMAP) {
        bytes_per_comp = 1;
    } else {		
        int bytes_per_pixel;

        bytes_per_pixel = get_bytes_per_pixel(format, type);
    }

    switch (type) {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_BYTE: 
        return marshalub(cpu, pixel, 1);
    case GL_BYTE:
        return marshalb(cpu, pixel, 1);
    case GL_BITMAP:
        return marshalub(cpu, pixel, 1);
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_SHORT:
        return marshalus(cpu, pixel, len);
    case GL_SHORT:
        return marshals(cpu, pixel, len);
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT:
        return marshalui(cpu, pixel, len);
    case GL_INT:
        return marshali(cpu, pixel, len);
    case GL_FLOAT:
        return marshalf(cpu, pixel, len);
    default:
        kpanic("glcommongl.c marshalPixels uknown type: %d", type);
        return NULL;
    }
}

GLvoid* marshalPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,  U32 pixels) {
    int bytes_per_comp;
    int isSigned=0;
    int bytes_per_row;
    int len;
    int remainder;
    
    GLint skipPixels;
    GLint skipRows;
    GLint alignment;
    GLint pixels_per_row;
    GLint skipImages = 0;

    if (!pixels)
        return 0;

    GL_FUNC(glGetIntegerv)(GL_UNPACK_ROW_LENGTH, &pixels_per_row);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_SKIP_ROWS, &skipRows);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
    if (is3d) {
        GL_FUNC(glGetIntegerv)(GL_PACK_SKIP_IMAGES, &skipImages);
    }    

    if (!pixels_per_row)
        pixels_per_row = width;
    if (type == GL_BITMAP) {
        bytes_per_comp = 1;
        bytes_per_row = (pixels_per_row+7)/8;
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
        kpanic("glcommongl.c marshalPixels uknown type: %d", type);
    }
    len = bytes_per_row*(height+skipRows)*(depth+skipImages);
    //printf("marshal pixels: width=%d height=%d depth=%d format=%d type=%d pixels_per_row=%d bytes_per_comp=%d len=%d\n", width, height, depth, format, type, pixels_per_row, bytes_per_comp, len);
    if (bytes_per_comp==0) {
        return marshalf(cpu, pixels, len/4);
    } else if (bytes_per_comp == 1) {
        if (isSigned) {
            return marshalb(cpu, pixels, len);
        } else {
            return marshalub(cpu, pixels, len);
        }
    } else if (bytes_per_comp == 2) {
        if (isSigned) {
            return marshals(cpu, pixels, len/2);
        } else {
            return marshalus(cpu, pixels, len/2);
        }
    } else if (bytes_per_comp == 4) {
        if (isSigned) {
            return marshali(cpu, pixels, len/4);
        } else {
            return marshalui(cpu, pixels, len/4);
        }
    }
    kpanic("glcommongl.c marshalPixels unknown bytes_per_comp %d", bytes_per_comp);
    return 0;
}

void marshalBackPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, U32 address, GLvoid* pixels) {
    int bytes_per_comp;
    int isSigned=0;
    int bytes_per_row;
    int len;
    int remainder;

    GLint skipPixels;
    GLint skipRows;
    GLint alignment;
    GLint pixels_per_row;
    GLint skipImages = 0;

    if (!pixels)
        return;

    GL_FUNC(glGetIntegerv)(GL_UNPACK_ROW_LENGTH, &pixels_per_row);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_SKIP_ROWS, &skipRows);
    GL_FUNC(glGetIntegerv)(GL_UNPACK_ALIGNMENT, &alignment);
    if (is3d) {
        GL_FUNC(glGetIntegerv)(GL_PACK_SKIP_IMAGES, &skipImages);
    }    

    if (!pixels_per_row)
        pixels_per_row = width;
    if (type == GL_BITMAP) {
        bytes_per_comp = 1;
        bytes_per_row = (pixels_per_row+7)/8;
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
    len = bytes_per_row*(height+skipRows)*(depth+skipImages);

    if (bytes_per_comp==0) {
        marshalBackf(cpu, address, (GLfloat*)pixels, len/4);
    } else if (bytes_per_comp == 1) {
        if (isSigned) {
            marshalBackb(cpu, address, (GLbyte*)pixels, len);
        } else {
            marshalBackub(cpu, address, (GLubyte*)pixels, len);
        }
    } else if (bytes_per_comp == 2) {
        if (isSigned) {
            marshalBacks(cpu, address, (GLshort*)pixels, len/2);
        } else {
            marshalBackus(cpu, address, (GLushort*)pixels, len/2);
        }
    } else if (bytes_per_comp == 4) {
        if (isSigned) {
            marshalBacki(cpu, address, (GLint*)pixels, len/4);
        } else {
            marshalBackui(cpu, address, (GLuint*)pixels, len/4);
        }
    } else {
        kpanic("glcommongl.c marshalBackPixels unknown bytes_per_comp %d", bytes_per_comp);
    }
}

U32 marshalBackp(CPU* cpu, GLvoid* buffer, U32 size) { 
    return cpu->thread->memory->mapNativeMemory(buffer, size);
}

// instance is in the instance number within the function, so if the same function calls this 3 times, each call will have a difference instance
GLvoid* marshalp(CPU* cpu, U32 instance, U32 buffer, U32 len) {
    if (buffer == 0)
        return NULL;
    if (buffer <0x10000) {
        return (GLvoid*)(uintptr_t)buffer;
    }
    if ((buffer & 0xFFF) + len > 0xFFF) {
        return marshalub(cpu, buffer, len);
    }
    // :TODO: a lot of work needs to be done here, marshalp needs to be removed and instead marshal the correct type of array, like marshalf.
    // This is also important to make things work with UNALIGNED_MEMORY
    return (GLvoid*)getPhysicalAddress(buffer, 1);
}

U32 marshalBackSync(CPU* cpu, GLsync sync) {
    klog("marshalBackSync not implemented");
    return 0;
}

GLsync marshalSync(CPU* cpu, U32 sync) {
    klog("marshalSync not implemented");
    return 0;
}

GLvoid** bufferpp;
U32 bufferpp_len;

GLvoid** marshalpp(CPU* cpu, U32 buffer, U32 count, U32 sizes, S32 bytesPerCount, U32 autoCharWidth) {
    U32 i;

    if (bytesPerCount==-1 && autoCharWidth!=1) {
        kpanic("OpenGL marshalpp can't handle multi-byte strings");
    }
    if (!buffer)
        return NULL;

    if (bufferpp && bufferpp_len<count) {
        delete[] bufferpp;
        bufferpp=NULL;
    }
    if (!bufferpp) {
        bufferpp = new GLvoid*[count];
        bufferpp_len = count;
    }
    for (i=0;i<count;i++) {
        S32 len = 0;
        U32 p = readd(buffer+i*4);
        if (sizes) {
            U32 address = readd(sizes+i*4);
            len = (S32)readd(address);
        }
        if (bytesPerCount) {
            if (bytesPerCount==-1 && len<=0) {
                len = getNativeStringLen(p)+1;
            } else {
                len*=bytesPerCount;
            }
        }
        // :TODO: this is wrong if the host address isn't used directly
        bufferpp[i] = marshalp(cpu, i, p, len);
    }
    return bufferpp;
}

// :TODO: not thread safe
const GLchar* marshalsz(CPU* cpu, U32 address) {
    static char* tmp;
    static U32 tmpLen;
    U32 len = getNativeStringLen(address)+1;
    if (len>tmpLen) {
        if (tmpLen!=0)
            delete[] tmp;
        tmp = new char[tmpLen];
        tmpLen = len;
    }
    return getNativeString(address, tmp, tmpLen);
}

#endif

void* marshalunhandled(const char* func, const char* param, CPU* cpu, U32 address) {
    klog("%s parameter in OpenGL function, %s, was not marshalled", func, param);
    return 0;
}

// GLsizeiptr is 64-bit on x64 and 32-bit on x86
MARSHAL_TYPE(GLsizeiptr, sip, d, 4)

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
    U32 size = 20 + 24*vertexCount;

    kpanic("DrawArraysIndirectBindlessCommandNV was not marshalled");
    return NULL;
}

// :TODO: for mac this is a void*
MARSHAL_TYPE(GLhandleARB, handle, d, 4)

void marshalBackhandle(CPU* cpu, U32 address, GLhandleARB* buffer, U32 count) {
    U32 i;

    // :TODO:
    if (sizeof(GLhandleARB)!=4)
        kpanic("marshalBackhandle not supported on this platform");
    for (i=0;i<count;i++) {
        writed(address, buffer[i]);
        address+=4;
    }
}

GLvoid* marshalGetConvolutionFilter(CPU* cpu, U32 target, U32 format, U32 type, U32 image) {
    GLint width = 0;
    GLint height = 0;
    U32 len = 0;

    if (PIXEL_PACK_BUFFER())
        return (GLubyte*)(uintptr_t)image;
    if (ext_glGetConvolutionParameteriv) {
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &width);
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &height);
    }
    return marshalType(cpu, type, components_in_format(format)*width*height, image);
}

GLint marshalGet(GLenum param) {
    GLint result = 0;
    glGetIntegerv(param, &result);
    return result;
}

GLboolean PIXEL_PACK_BUFFER() {
    return marshalGet(GL_PIXEL_PACK_BUFFER_BINDING)!=0;
}

GLboolean ARRAY_BUFFER() {
    return marshalGet(GL_ARRAY_BUFFER_BINDING)!=0;
}

GLboolean ELEMENT_ARRAY_BUFFER() {
    return marshalGet(GL_ELEMENT_ARRAY_BUFFER_BINDING)!=0;
}

GLboolean PIXEL_UNPACK_BUFFER() {
    return marshalGet(GL_PIXEL_UNPACK_BUFFER_BINDING)!=0;
}
#endif
