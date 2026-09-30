#ifndef BMP_H
#define BMP_H

#include <def.h>

#pragma pack(push, 1)
typedef struct {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} bmp_file_header_t;

typedef struct {
    u32 biSize;
    i32 biWidth;
    i32 biHeight;
    u16 biPlanes; 
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    i32 biXPelsPerMeter;
    i32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} bmp_info_header_t;
#pragma pack(pop)

#endif