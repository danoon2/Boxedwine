#ifndef __GL_MARSHAL_H__
#define __GL_MARSHAL_H__
   
template <typename T, U32 writeSize = sizeof(T)>
void marshalBackArray(CPU* cpu, T* buffer, U32 address, U32 count) {
    if (address) {
        if constexpr (writeSize == 1) {
            cpu->memory->memcpy(address, buffer, count);
        } else {
            for (U32 i = 0; i < count; i++) {
                if constexpr (writeSize == 4) {
                    cpu->memory->writed(address, buffer[i]);
                    address += 4;
                } else if constexpr (writeSize == 2) {
                    cpu->memory->writew(address, buffer[i]);
                    address += 2;
                } else if constexpr (writeSize == 8) {
                    if constexpr (sizeof(T) != writeSize) {
                        if (buffer[i] & 0xFFFFFFFF00000000) {
                            kpanic("oops");
                        }
                    }
                    cpu->memory->writeq(address, buffer[i]);
                    address += 8;
                }
            }
        }
    }
}

template <typename T, U32 readSize = sizeof(T)>
T* marshalArray(CPU* cpu, U32 address, U32 count) {
    static T** buffer;
    static U32* bufferLen;
    static U32 indexCount;

    U32 index = cpu->thread->marshalIndex++;
    if (!address) {
        return nullptr;
    }
    if (address < 0x1000) {
        int ii = 0;
    }
    if (!count) {
        return (T*)(U64)address;
    }
    U32 len = count * sizeof(T);
    U32 page = address >> K_PAGE_SHIFT;
    U32 pageStop = (address + len - 1) >> K_PAGE_SHIFT;
    if (page == pageStop && cpu->memory->canRead(page)) {
        return (T*)cpu->memory->getIntPtr(address);
    }
    if (!buffer) {
        buffer = new T * [index + 1];
        memset(buffer, 0, sizeof(T*) * (index + 1));
        bufferLen = new U32[index + 1];
        memset(bufferLen, 0, sizeof(U32) * (index + 1));
        indexCount = index + 1;
    }
    if (index >= indexCount) {
        T** newBuffer = new T * [index + 1];
        U32* newBufferLen = new U32[index + 1];
        memset(newBuffer, 0, sizeof(T*) * (index + 1));
        memcpy(newBuffer, buffer, sizeof(T*) * indexCount);
        memset(newBufferLen, 0, sizeof(U32) * (index + 1));
        memcpy(newBufferLen, bufferLen, sizeof(U32) * indexCount);
        delete[] buffer;
        delete[] bufferLen;
        buffer = newBuffer;
        bufferLen = newBufferLen;
        indexCount = index + 1;
    }
    if (buffer[index] && bufferLen[index] < count) {
        delete[] buffer[index];
        buffer[index] = nullptr;
    }
    if (!buffer[index]) {
        buffer[index] = new T[count];
        bufferLen[index] = count;
    }
    if constexpr (readSize == 1) {
        cpu->memory->memcpy(buffer[index], address, count);
    } else {
        for (U32 i = 0; i < count; i++) {
            if constexpr (readSize == 4) {
                buffer[index][i] = cpu->memory->readd(address);
                address += 4;
            } else if constexpr (readSize == 2) {
                buffer[index][i] = cpu->memory->readw(address);
                address += 2;
            } else if constexpr (readSize == 8) {
                buffer[index][i] = cpu->memory->readq(address);
                address += 8;
            } else {
                kpanic("oops");
            }
        }
    }
    return buffer[index];
}

template <typename T>
class MarshalReadWrite {
    CPU* cpu = nullptr;
    U32 address = 0;
    U32 count = 0;
    T* buffer = nullptr;
public:
    MarshalReadWrite(CPU* cpu, U32 address, U32 count) : cpu(cpu), address(address), count(count) {
    }
    ~MarshalReadWrite() {
        if (buffer) {
            marshalBackArray<T>(cpu, buffer, address, count);
        }
    }
    T* getPtr() {
        if (!buffer) {
            U32 len = count * sizeof(T);
            U32 page = address >> K_PAGE_SHIFT;
            U32 pageStop = (address + len - 1) >> K_PAGE_SHIFT;
            if (page == pageStop && cpu->memory->canRead(page) && cpu->memory->canWrite(page)) {
                return (T*)cpu->memory->getIntPtr(address);
            }
            buffer = marshalArray<T>(cpu, address, count);
        }
        return buffer;
    }
};

GLfloat* marshalf(CPU* cpu, U32 address, U32 count);
GLfloat* marshal2f(CPU* cpu, U32 address, U32 count);
GLfloat* marshal3f(CPU* cpu, U32 address, U32 count);
GLfloat* marshal4f(CPU* cpu, U32 address, U32 count);
void marshalBackf(CPU* cpu, U32 address, GLfloat* buffer, U32 count);

class MarshalReadWriteFloat {
    CPU* cpu = nullptr;
    U32 address = 0;
    U32 count = 0;
    GLfloat* buffer = nullptr;
public:
    MarshalReadWriteFloat(CPU* cpu, U32 address, U32 count) : cpu(cpu), address(address), count(count) {
    }
    ~MarshalReadWriteFloat() {
        if (buffer) {
            marshalBackf(cpu, address, buffer, count);
        }
    }
    GLfloat* getPtr() {
        if (!buffer) {
            buffer = marshalf(cpu, address, count);
        }
        return buffer;
    }
};

GLdouble* marshald(CPU* cpu, U32 address, U32 count);
GLdouble* marshal2d(CPU* cpu, U32 address, U32 count);
void marshalBackd(CPU* cpu, U32 address, GLdouble* buffer, U32 count);

const GLchar* marshalsz(CPU* cpu, U32 address);
const GLchar** marshalszArray(CPU* cpu, U32 count, U32 address, U32 addressLengths);
const GLcharARB** marshalszArrayARB(CPU* cpu, U32 count, U32 address, U32 addressLengths);

// type can be GL_UNSIGNED_BYTE, GL_BYTE, GL_2_BYTES, GL_UNSIGNED_SHORT, GL_SHORT, GL_3_BYTES, 
// GL_4_BYTES, GL_FLOAT, GL_UNSIGNED_INT, GL_INT
//
// base on the type, the correct marshal function will be called
GLvoid* marshalType(CPU* cpu, U32 type, U32 count, U32 address);
void marshalBackType(CPU* cpu, U32 type, U32 count, GLvoid* buffer, U32 address);

// will call the correct marshal function based on the type the count passed to the marshal
// function will be set to the correct number to include all the data for a single pixel for the format
GLvoid* marshalPixel(CPU* cpu, GLenum format, GLenum type, U32 pixel);

// Used by glBitmap, glDrawPixels, glTexImage1D, glTexImage2D, glTexSubImage1D, glTexSubImage2D
//
// This will take into account packing, like GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS,
// GL_UNPACK_ALIGNMENT, GL_PACK_SKIP_IMAGES
GLvoid* marshalPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,  U32 pixels);
void marshalBackPixels(CPU* cpu, U32 is3d, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, U32 address, GLvoid* pixels);

void updateVertexPointers(CPU* cpu, U32 count);
GLvoid* marshalVetextPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalNormalPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalIndexPointer(CPU* cpu,  GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalTexCoordPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalEdgeFlagPointer(CPU* cpu, GLsizei stride, U32 ptr);
const GLboolean* marshalEdgeFlagPointerEXT(CPU* cpu, GLsizei stride, GLsizei count, U32 ptr);
GLvoid* marshalSecondaryColorPointer(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalSecondaryColorPointerEXT(CPU* cpu, GLint size, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalFogPointer(CPU* cpu, GLenum type, GLsizei stride, U32 ptr);
GLvoid* marshalFogPointerEXT(CPU* cpu, GLenum type, GLsizei stride, U32 ptr);
const GLvoid* marshalInterleavedPointer(CPU* cpu, GLenum format, GLsizei stride, U32 ptr);

U32 getDataSize(GLenum type);

U32 marshalGetColorTableWidth(U32 target);
U32 marshalGetColorTableWidthEXT(U32 target);
U32 marshalGetColorTableWidthSGI(U32 target);
U32 marshalGetCompressedImageSize(GLenum target, GLint level);
U32 marshalGetCompressedImageSizeARB(GLenum target, GLint level);
U32 marshalGetCompressedMultiImageSizeEXT(GLenum texunit, GLenum target, GLint level);
U32 marshalGetCompressedTextureSizeEXT(GLuint texture, GLenum target, GLint lod);
U32 marshalGetConvolutionWidth(U32 target);
U32 marshalGetConvolutionHeight(U32 target);
GLint components_in_format(GLenum format );
GLsizei marshalHistogramWidth(GLenum target);

GLintptr* marshalip(CPU* cpu, U32 address, U32 count);
GLintptr* marshal2ip(CPU* cpu, U32 address, U32 count);

GLsizeiptr* marshalsip(CPU* cpu, U32 address, U32 count);

// GLhandleARB is a U32 on Win32, but on Mac it is a void*
GLhandleARB* marshalhandle(CPU* cpu, U32 address, U32 count);
void marshalBackhandle(CPU* cpu, U32 address, GLhandleARB* buffer, U32 count);

GLsync marshalSync(CPU* cpu, U32 sync);
U32 marshalBackSync(CPU* cpu, GLsync sync);

GLvoid* marshalp_and_check_array_buffer(CPU* cpu, U32 instance, U32 buffer, U32 len);
GLvoid* marshalp(CPU* cpu, U32 instance, U32 buffer, U32 len);
U32 marshalBackp(CPU* cpu, GLvoid* buffer, U32 size);
U32 mapBufferRange(CPU* cpu, GLenum target, GLvoid* buffer, U32 offset, U32 size);
void flushBufferRange(CPU* cpu, GLenum target, U32 offset, U32 size);
void unmapBuffer(CPU* cpu, GLenum target);

GLvoid** marshalpp(CPU* cpu, U32 buffer, U32 count, U32 sizes, S32 bytesPerCount, U32 autoCharWidth);

void* marshalDrawArraysIndirectBindlessCommandNV(CPU* cpu, U32 address, U32 count, U32 stride, U32 vertexCount);

void* marshalunhandled(const char* func, const char* param, CPU* cpu, U32 address);

// These functions help calculate the count that will be passed to various marshal function
int getSize(GLenum pname);
GLsizei floatPerTransformList(GLenum transformType);
U32 marshalGetActiveAtomicCountersCount(U32 program, U32 bufferIndex);
U32 marshalGetCompatibleSubroutinesCount(U32 program, U32 shadertype, U32 index);
U32 marshalGetUniformBlockActiveUnformsCount(U32 program, U32 uniformBlockIndex);
int glcommon_glLightv_size(GLenum e);
int glcommon_glLightModelv_size(GLenum e);
int glcommon_glMaterialv_size(GLenum e);
U32 getMap1Count(GLenum target);
U32 getMap2Count(GLenum target);
GLint glcommon_glGetPixelMap_size(GLenum map);
GLint get_bytes_per_pixel(GLenum format, GLenum type);
GLint marshalGet(GLenum param);
GLboolean PIXEL_PACK_BUFFER();
GLboolean ARRAY_BUFFER();
GLboolean ELEMENT_ARRAY_BUFFER();
GLboolean PIXEL_UNPACK_BUFFER();
void OPENGL_CALL_TYPE debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

#ifdef BOXEDWINE_GLHANDLE_ARB_POINTER
U32 marshalHandleToIndex(GLhandleARB h);
GLhandleARB marshalIndexToHandle(U32 i);
void marshalDeleteHandleIndex(U32 i);

#define HANDLE_TO_INDEX(h) marshalHandleToIndex(h)
#define INDEX_TO_HANDLE(i) marshalIndexToHandle(i)
#define DELETE_HANDLE_INDEX(i) marshalDeleteHandleIndex(i)
#else
#define HANDLE_TO_INDEX(h) h
#define INDEX_TO_HANDLE(i) i
#define DELETE_HANDLE_INDEX(i)
#endif

// logging
const char* glcommon_glLightv_print_pname(GLenum e);
const char* glcommon_glLightv_print_buffer(GLenum e, GLfloat* buffer);
const char* glcommon_glLightv_print_name(GLenum e);
const char* glcommon_glClear_mask(GLbitfield mask);

template <typename T>
class MarshalReadWritePacked {
    CPU* cpu = nullptr;
    U32 address = 0;
    U32 count = 0;
    T* buffer = nullptr;
    bool packed = false;
    std::function<U32()> fnGetCount;
public:
    MarshalReadWritePacked(CPU* cpu, U32 address, U32 count) : cpu(cpu), address(address), count(count) {
        packed = PIXEL_PACK_BUFFER();
    }
    MarshalReadWritePacked(CPU* cpu, U32 address, std::function<U32()> fnGetCount) : cpu(cpu), address(address), fnGetCount(fnGetCount) {
        packed = PIXEL_PACK_BUFFER();
    }
    ~MarshalReadWritePacked() {
        if (buffer) {
            marshalBackArray<T>(cpu, buffer, address, count);
        }
    }
    bool isPacked() {
        return packed;
    }
    bool getCount() {
        if (fnGetCount) {
            count = fnGetCount();
            fnGetCount = nullptr;
        }
        return count;
    }
    T* getPtr() {
        if (packed) {
            return (T*)(uintptr_t)(address);
        }
        if (!buffer) {
            if (fnGetCount) {
                count = fnGetCount();
                fnGetCount = nullptr;
            }
            if (count) {
                U32 len = count * sizeof(T);
                U32 page = address >> K_PAGE_SHIFT;
                U32 pageStop = (address + len - 1) >> K_PAGE_SHIFT;
                if (page == pageStop && cpu->memory->canRead(page) && cpu->memory->canWrite(page)) {
                    return (T*)cpu->memory->getIntPtr(address);
                }
            }
            buffer = marshalArray<T>(cpu, address, count);
        }
        return buffer;
    }
};

#endif
