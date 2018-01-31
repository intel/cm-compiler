/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _COLORADJUSTSIMDCF_H_
#define _COLORADJUSTSIMDCF_H_

typedef void (*HueSaturationFuncPtr)(unsigned char *src,
                                     unsigned char *dst,
                                     int delta_hue,
                                     int delta_saturation,
                                     int *scale_table256);

typedef void (*ProcessPixeFuncPtr)(unsigned char *src,
                                   unsigned char *dst,
                                   int pixel_per_line,
                                   int delta_hue,
                                   int delta_saturation,
                                   int *scale_table256,
                                   HueSaturationFuncPtr hls_func_ptr,
                                   unsigned char *gray_map);

struct DsColorAdjust {
    unsigned long size;
    unsigned long version;
    int hue;
    int sat;
    int type;
    int scroll_value1[5];
    int scroll_value2[5];
    int scroll_value3[5];
    int adjust_gamma[5];
    unsigned char adjust_value[5][4];
    double inverse_gamma[5];
};

struct DsInfo {
    unsigned char *in_table;
    unsigned char *out_table;
    unsigned char *temp_out_table;
    unsigned char *arbitrary_map;
    unsigned char *gray_map;
    unsigned char *temp_out_buf;
};

class ColorAdjust {
public:
    ColorAdjust();
    ~ColorAdjust();

public:
    int GetDefaultOption(int key, void *option);

    int scale_table[256];

    int ProcessPic(bool do_Hns,
                   bool do_Bnc,
                   unsigned char *src,
                   unsigned char *dst,
                   int pixel_per_line,
                   int lines,
                   int bytes_per_pixel,
                   int row_bytes,
                   void *v_option,
                   void *inst_data);

    int ProcessPicCm(bool do_Hns,
                     bool do_Bnc,
                     unsigned char *src,
                     unsigned char *dst,
                     int width,
                     int height,
                     void *v_option,
                     void *inst_data);

    DsColorAdjust current_option;
    bool bnc_table_init;
};
#endif // #ifndef _COLORADJUSTSIMDCF_H_
