#ifndef _IMAGE_CONVERT_H_
#define _IMAGE_CONVERT_H_

#include <stdint.h>

class ImageConvert
{
public:
    ImageConvert();

    // NV21转RGB
    void Nv212Rgb24(const uint8_t* src_y, const uint8_t* src_vu, uint8_t* dst, int width, int height);

private:
    void initSelectTable();

    uint8_t r_yv[256][256];
    uint8_t b_yu[256][256];
    uint8_t g_yuv[256][256][256];
};

#endif // end of _IMAGE_CONVERT_H_