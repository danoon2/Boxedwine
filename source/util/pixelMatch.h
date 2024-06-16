#ifndef __PIXEL_MATCH_H__

U32 pixelmatch(const U8* img1, U32 stride1, const U8* img2, U32 stride2, U32 width, U32 height, U8* output = nullptr, double threshold = 0.1, bool includeAA = false);

#endif