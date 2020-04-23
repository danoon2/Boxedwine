#ifndef __PLATFORM_HELPER_H__
#define __PLATFORM_HELPER_H__

void* LoadTextureFromFile(const char* filename, int* out_width, int* out_height);
void* MakeRGBATexture(const unsigned char* data, int width, int height);

#endif