#include "boxedwine.h"
#include "../boxedwineui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

#ifdef BOXEDWINE_IMGUI_DX9
#include <d3d9.h>
extern LPDIRECT3DDEVICE9 g_pd3dDevice;

void UnloadTexture(void* texture) {
    if (texture) {
        ((IDirect3DTexture9*)texture)->Release();
    }
}


void* MakeRGBATexture(const unsigned char* data, int width, int height) {
    IDirect3DTexture9* texture = NULL;
    D3DLOCKED_RECT r;

    if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL) != D3D_OK)
        return false;
    if (texture->LockRect(0, &r, NULL, D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE) != D3D_OK)
        return false;
    for (int y = 0; y < height; y++)
    {
        BYTE* dest = ((BYTE*)r.pBits + r.Pitch*y);
        BYTE* src = ((BYTE*)data + width * 4 * y);
        for (int x = 0; x < width*4; x+=4) {
            dest[x+3] = src[x+3];
            dest[x+2] = src[x];
            dest[x+1] = src[x+1];
            dest[x] = src[x+2];
        }
    }
    texture->UnlockRect(0);
    return (void*)texture;
}

#else 

void* MakeRGBATexture(const unsigned char* data, int width, int height) {
    // Create a OpenGL texture identifier
    GLuint image_texture=0;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    return (void*)(U64)image_texture;
}

#endif

unsigned char* LoadImageFromFile(const char* filename, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return NULL;

    *out_width = image_width;
    *out_height = image_height;

    return image_data;
}