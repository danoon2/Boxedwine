#ifndef __PLATFORM_HELPER_H__
#define __PLATFORM_HELPER_H__

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
void UnloadTexture(GLuint t);
GLuint MakeRGBATexture(const unsigned char* data, int width, int height);

#endif