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

#include <memory.h>

template<typename type1>
bool BMP_Check_Result(char *f_out_str, char *f_gold_str, type1 tolerance)
{
    unsigned char header_out[54];
    unsigned char header_gold[54];
    unsigned int width;
    unsigned int height;
    unsigned char *img_out, *img_gold;
    int i;
    FILE *f_out;
    FILE *f_gold;
    BOOL  compare_result = true;

    f_out = fopen(f_out_str, "rb");
    if (f_out == NULL) {
        perror(f_out_str);
        return false;
    }
    if (fread(header_out, 1, 54, f_out) != 54) {
        perror(f_out_str);
        return false;
    }

    f_gold = fopen(f_gold_str, "rb");
    if (f_gold == NULL) {
        perror(f_gold_str);
        return false;
    }
    if (fread(header_gold, 1, 54, f_gold) != 54) {
        perror(f_gold_str);
        return false;
    }

    if (memcmp(header_out, header_gold, 54) != 0) {
        fclose(f_out);
        fclose(f_gold);
        return false; //Headers are different
    }

    width = abs(*(short *)&header_out[18]);
    height = abs(*(short *)&header_out[22]);

    img_out = (unsigned char*) malloc(width*height*3);
    img_gold = (unsigned char*) malloc(width*height*3);


    fread(img_out, 1, width*height*3, f_out);
    fread(img_gold, 1, width*height*3, f_gold);

    fclose(f_out);
    fclose(f_gold);

    for (i = 0; i < width*height*2; i++) {
        if ((abs(img_out[i] - img_gold[i]) > tolerance) &&
               (!(img_out[i] == 0 && img_gold[i] == 0xff)) &&
               (!(img_out[i] == 0xff && img_gold[i] == 0))
              ) {
            printf("%d:    OUT: %x, GOLD: %x\n", i, img_out[i], img_gold[i]);
            compare_result = false;
        }
    }

    return compare_result;
}
