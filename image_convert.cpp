#include "image_convert.h"

ImageConvert::ImageConvert()
{
    initSelectTable();
}

void ImageConvert::Nv212Rgb24(const uint8_t* src_y, const uint8_t* src_vu, uint8_t* dst, int width, int height)
{
    int size = width*height;
    const uint8_t* y = src_y;
    const uint8_t* v = src_vu;
    const uint8_t* u = src_vu+1;

    for(int j = 0; j < height; j++)
    {
        for(int i = 0; i < width; i++)
        {
            // R
            dst[(width*j+i)*3] = r_yv[*y][*v];
            // G
            dst[(width*j+i)*3+1] = g_yuv[*y][*u][*v];
            // B
            dst[(width*j+i)*3+2] = b_yu[*y][*u];

            y++;
            if(i%2 == 1)
            {
                v += 2;
                u += 2;
            }
        }
        if(j%2 == 0)
        {
            v -= width;
            u -= width;
        }
    }
}

void ImageConvert::initSelectTable()
{
    /**************RGB*****************/
    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < 256; j++)
        {
            r_yv[i][j] = i + 1.403*(j-128);
            b_yu[i][j] = i + 1.770*(j-128);
            for(int k = 0; k < 256; k++)
                g_yuv[i][j][k] = i - 0.343*(j-128) - 0.714*(k-128);
        }
    }
}