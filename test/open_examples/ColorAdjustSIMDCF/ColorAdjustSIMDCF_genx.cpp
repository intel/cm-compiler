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

#include <cm/cm.h>

#define INDEX_RED    0
#define INDEX_GREEN  1
#define INDEX_BLUE   2

#define FIND_MAX_MIN                                    \
{                                                       \
    mask = (R < B);                                     \
    Min.merge(R, B, mask);                              \
    Max.merge(B, R, mask);                              \
    MinIndex.merge(INDEX_RED, INDEX_BLUE, mask);        \
    MaxIndex.merge(INDEX_BLUE, INDEX_RED, mask);        \
    mask = (G < Min);                                   \
    Min.merge(G, mask);                                 \
    MinIndex.merge(INDEX_GREEN, mask);                  \
    mask = (G > Max);                                   \
    Max.merge(G, mask);                                 \
    MaxIndex.merge(INDEX_GREEN, mask);                  \
    Mid = (R+G+B)-(Max+Min);                            \
}

#define INIT_SCALE_TABLE            \
{                                   \
    ScaleTable(0) = 1;              \
    for(int i = 1; i < 256; i++)    \
    {                               \
        ScaleTable(i) = 87381 / i;  \
    }                               \
}

extern "C" _GENX_MAIN_  void
AdjustHueSaturation(SurfaceIndex in, SurfaceIndex out, int nDeltaHue, int nDeltaSaturation, int width)
{
    int v_pos = get_thread_origin_y() * 8;

    vector<int, 256> ScaleTable;
    //read in 8 row and 16 col pixels in ARGB format.
    matrix<uchar, 8, 64> buf;
    vector<ushort, 16> Max;
    vector<ushort, 16> Min;
    vector<ushort, 16> Mid;
    vector<ushort, 16> Range;
    vector<ushort, 16> Brightness;
    vector<ushort, 16> MaxIndex;
    vector<ushort, 16> MinIndex;
    vector<int, 16> Hue = 0;
    vector<ushort, 16> mask;
    vector<int, 16> R_dst;
    vector<int, 16> G_dst;
    vector<int, 16> B_dst;

    INIT_SCALE_TABLE;

    for (int col = 0; col < width; col += 16)
    {
        //read in 8x16 pixel block
        int colX4 = col*4;
        read(MODIFIED(in),   colX4,    v_pos, buf.select<8, 1, 32, 1>(0, 0));
        read(MODIFIED(in),   colX4+32, v_pos, buf.select<8, 1, 32, 1>(0, 32));

        for (uint row = 0; row<8; row ++)
        {
            vector<ushort, 16> B = buf.select<1, 1, 16, 4>(row, 0);
            vector<ushort, 16> G = buf.select<1, 1, 16, 4>(row, 1);
            vector<ushort, 16> R = buf.select<1, 1, 16, 4>(row, 2);

            FIND_MAX_MIN;

            Range = Max - Min;
            Brightness = (Max + Min)/2;

            mask = (Range == 0);
            B_dst.merge(Max, mask);
            G_dst.merge(Max, mask);
            R_dst.merge(Max, mask);

            vector<int, 16> temp1 = ScaleTable.iselect(Range) * (Max-Mid);
            vector<int, 16> temp2 = ScaleTable.iselect(Range) * (Mid-Min);

            // RGB to HLS conversion
            SIMD_IF_BEGIN (MinIndex == INDEX_GREEN) {
                //R>B>G
                SIMD_IF_BEGIN (MaxIndex == INDEX_RED) {
                    Hue     = 436906 + temp1;
                } SIMD_ELSEIF(MaxIndex == INDEX_BLUE) {
                    Hue     = 349525 + temp2;
                } SIMD_IF_END;
            } SIMD_ELSEIF(MinIndex == INDEX_BLUE) {
                //R>G>B
                SIMD_IF_BEGIN (MaxIndex == INDEX_RED) {
                    Hue     = temp2;
                } SIMD_ELSEIF (MaxIndex == INDEX_GREEN) {
                    Hue     = 87381 + temp1;
                } SIMD_IF_END;
            } SIMD_ELSEIF (MinIndex == INDEX_RED) {
                //G>B>R
                SIMD_IF_BEGIN (MaxIndex == INDEX_GREEN) {
                    Hue     = 174762 + temp2;
                } SIMD_ELSEIF (MaxIndex == INDEX_BLUE) {
                    Hue     = 262144 + temp1;
                } SIMD_IF_END;
            } SIMD_IF_END;

            //clamp Hue in the [0, 524287] range
            Hue     = cm_min<int>(cm_max<int>(Hue, 0), 524287);
            Hue     += nDeltaHue;

            // HLS to RGB conversion
            mask    = (Hue > 524287);
            Hue.merge((Hue-524288), mask);

            mask    = (Hue < 0);
            Hue.merge((Hue+524288), mask);

            mask    = (Brightness <= 127);

            temp1 = Range + ((2 * Brightness * nDeltaSaturation + 128) >> 8);
            temp2 = Range + ((2 * (255 - Brightness) * nDeltaSaturation + 128) >> 8);

            temp1 = cm_min<int>(temp1, 2*Brightness);
            temp2 = cm_min<int>(temp2, 2*(255-Brightness));

            vector<int, 16> newRange;
            newRange.merge(temp1, temp2, mask);
            newRange = cm_max<int>(newRange, 0);

            SIMD_IF_BEGIN (Range != 0) {
                SIMD_IF_BEGIN (newRange == 0) {
                    B_dst = Brightness;
                    G_dst = Brightness;
                    R_dst = Brightness;
                } SIMD_ELSE {
                    vector<int, 16> tmp = (newRange/2);
                    vector<int, 16> newMax = Brightness + tmp;
                    vector<int, 16> newMin = Brightness - tmp;

                    vector<int, 16> Index    = ((Hue * 6) >> 19);
                    vector<int, 16> Color    = Hue - (Index * 87381);
                    vector<int, 16> Offset   = ((Color * newRange + 43690) * 6) >> 19;

                    B_dst = newMin;
                    G_dst = newMin + Offset;
                    R_dst = newMax;

                    SIMD_IF_BEGIN (Index == 1) {
                        B_dst = newMin;
                        G_dst = newMax;
                        R_dst = newMax - Offset;
                    } SIMD_ELSEIF(Index == 2) {
                        B_dst = newMin + Offset;
                        G_dst = newMax;
                        R_dst = newMin;
                    } SIMD_ELSEIF(Index == 3) {
                        B_dst = newMax;
                        G_dst = newMax - Offset;
                        R_dst = newMin;
                    } SIMD_ELSEIF(Index == 4) {
                        B_dst = newMax;
                        G_dst = newMin;
                        R_dst = newMin + Offset;
                    } SIMD_ELSEIF(Index == 5) {
                        B_dst = newMax - Offset;
                        G_dst = newMin;
                        R_dst = newMax;
                    } SIMD_IF_END;
                } SIMD_IF_END;
            } SIMD_IF_END;

            buf.select<1, 1, 16, 4>(row, 0) = vector<uchar, 16>(B_dst, SAT);
            buf.select<1, 1, 16, 4>(row, 1) = vector<uchar, 16>(G_dst, SAT);
            buf.select<1, 1, 16, 4>(row, 2) = vector<uchar, 16>(R_dst, SAT);
        }

        write(out,   colX4,    v_pos, buf.select<8, 1, 32, 1>(0, 0));
        write(out,   colX4+32, v_pos, buf.select<8, 1, 32, 1>(0, 32));
    }
}

extern "C" _GENX_MAIN_  void
AdjustHueOnly(SurfaceIndex in, SurfaceIndex out, int nDeltaHue, int nDeltaSaturation, int width)
{
    int v_pos = get_thread_origin_y() * 8;

    vector<int, 256> ScaleTable;

    //read in 8 row and 16 col pixels in ARGB format.
    matrix<uchar, 8, 64> buf;
    vector<ushort, 16> Max;
    vector<ushort, 16> Min;
    vector<ushort, 16> Mid;
    vector<ushort, 16> Range;
    vector<ushort, 16> MaxIndex;
    vector<ushort, 16> MinIndex;
    vector<int, 16> Hue;
    vector<ushort, 16> mask;
    vector<int, 16> R_dst;
    vector<int, 16> G_dst;
    vector<int, 16> B_dst;

    INIT_SCALE_TABLE;

    for (int col = 0; col < width; col += 16)
    {
        //read in 8x16 pixel block
        int colX4 = col*4;
        read(MODIFIED(in),   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        read(MODIFIED(in),   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        read(MODIFIED(in),   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        read(MODIFIED(in),   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));

        for (uint row = 0; row<8; row ++)
        {
            vector<ushort, 16> B = buf.select<1, 1, 16, 4>(row, 0);
            vector<ushort, 16> G = buf.select<1, 1, 16, 4>(row, 1);
            vector<ushort, 16> R = buf.select<1, 1, 16, 4>(row, 2);

            FIND_MAX_MIN;

            Range   = Max - Min;
            Hue     = 0;

            vector<int, 16> temp1 = ScaleTable.iselect(Range) * (Max-Mid);
            vector<int, 16> temp2 = ScaleTable.iselect(Range) * (Mid-Min);

            // RGB to HLS conversion

            SIMD_IF_BEGIN (MinIndex == INDEX_GREEN) {
                //R>B>G
                SIMD_IF_BEGIN (MaxIndex == INDEX_RED) {
                    Hue     = 436906 + temp1;
                } SIMD_ELSEIF (MaxIndex == INDEX_BLUE) {
                    Hue     = 349525 + temp2;
                } SIMD_IF_END;
            } SIMD_ELSEIF (MinIndex == INDEX_BLUE) {
                //R>G>B
                SIMD_IF_BEGIN (MaxIndex == INDEX_RED) {
                    Hue     = temp2;
                } SIMD_ELSEIF(MaxIndex == INDEX_GREEN) {
                    Hue     = 87381 + temp1;
                } SIMD_IF_END;
            } SIMD_ELSEIF (MinIndex == INDEX_RED) {
                //G>B>R
                SIMD_IF_BEGIN (MaxIndex == INDEX_GREEN) {
                    Hue     = 174762 + temp2;
                } SIMD_ELSEIF (MaxIndex == INDEX_BLUE) {
                    Hue     = 22144 + temp1;
                } SIMD_IF_END;
            } SIMD_IF_END;

            //clamp Hue in the [0, 524287] range
            Hue     = cm_min<int>(cm_max<int>(Hue, 0), 524287);

            Hue     += nDeltaHue;

            // HLS to RGB conversion
            mask    = (Hue > 524287);
            Hue.merge((Hue-524288), mask);

            mask    = (Hue < 0);
            Hue.merge((Hue+524288), mask);

            vector<int, 16> Index    = ((Hue * 6) >> 19);
            vector<int, 16> Color    = Hue - (Index * 87381);
            vector<int, 16> Offset   = ((Color * Range + 43690) * 6) >> 19;

            SIMD_IF_BEGIN (Range == 0) {
                B_dst = Max;
                G_dst = Max;
                R_dst = Max;
            } SIMD_ELSE {
                B_dst = Min;
                G_dst = Min + Offset;;
                R_dst = Max;
                SIMD_IF_BEGIN (Index == 1) {
                    B_dst = Min;
                    G_dst = Max;
                    R_dst = Max - Offset;
                } SIMD_ELSEIF (Index == 2) {
                    B_dst = Min + Offset;;
                    G_dst = Max;
                    R_dst = Min;
                } SIMD_ELSEIF(Index == 3) {
                    B_dst = Max;
                    G_dst = Max - Offset;
                    R_dst = Min;
                } SIMD_ELSEIF (Index == 4) {
                    B_dst = Max;
                    G_dst = Min;
                    R_dst = Min + Offset;;
                } SIMD_ELSEIF (Index == 5) {
                    B_dst = Max - Offset;
                    G_dst = Min;
                    R_dst = Max;
                } SIMD_IF_END;
            } SIMD_IF_END;

            vector<uchar, 16> B_Final = vector<uchar, 16>(B_dst, SAT);
            vector<uchar, 16> G_Final = vector<uchar, 16>(G_dst, SAT);
            vector<uchar, 16> R_Final = vector<uchar, 16>(R_dst, SAT);

            buf.select<1, 1, 16, 4>(row, 0) = B_Final;
            buf.select<1, 1, 16, 4>(row, 1) = G_Final;
            buf.select<1, 1, 16, 4>(row, 2) = R_Final;
        }

        write(out,   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        write(out,   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        write(out,   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        write(out,   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));
    }
}

extern "C" _GENX_MAIN_  void
AdjustSaturationOnly(SurfaceIndex in, SurfaceIndex out, int nDeltaHue, int nDeltaSaturation, int width)
{
    int v_pos = get_thread_origin_y() * 8;
    vector<int, 256> ScaleTable;

    //read in 8 row and 16 col pixels in ARGB format.
    matrix<uchar, 8, 64> buf;
    vector<ushort, 16> Max;
    vector<ushort, 16> Min;
    vector<ushort, 16> Mid;
    vector<ushort, 16> Range;
    vector<ushort, 16> Brightness;
    vector<ushort, 16> MaxIndex;
    vector<ushort, 16> MinIndex;
    vector<ushort, 16> mask;
    vector<int, 16> R_dst;
    vector<int, 16> G_dst;
    vector<int, 16> B_dst;

    INIT_SCALE_TABLE;

    for (int col = 0; col < width; col += 16)
    {
        //read in 8x16 pixel block
        int colX4 = col*4;
        read(MODIFIED(in),   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        read(MODIFIED(in),   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        read(MODIFIED(in),   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        read(MODIFIED(in),   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));

        for (uint row = 0; row<8; row ++)
        {
            vector<ushort, 16> B = buf.select<1, 1, 16, 4>(row, 0);
            vector<ushort, 16> G = buf.select<1, 1, 16, 4>(row, 1);
            vector<ushort, 16> R = buf.select<1, 1, 16, 4>(row, 2);

            FIND_MAX_MIN;

            Range       = Max - Min;
            Brightness = (Max + Min)/2;

            SIMD_IF_BEGIN (Range == 0) {
                B_dst = Max;
                G_dst = Max;
                R_dst = Max;
            } SIMD_ELSE {
                vector<int, 16> temp1 = Range + ((2 * Brightness * nDeltaSaturation + 128) >> 8);
                vector<int, 16> temp2 = Range + ((2 * (255 - Brightness) * nDeltaSaturation + 128) >> 8);
                temp1 = cm_min<int>(temp1, 2*Brightness);
                temp2 = cm_min<int>(temp2, 2*(255-Brightness));
                mask = (Brightness <= 127);
                vector<int, 16> newRange;
                newRange.merge(temp1, temp2, mask);

                SIMD_IF_BEGIN (newRange <= 0) {
                    B_dst = Brightness;
                    G_dst = Brightness;
                    R_dst = Brightness;
                } SIMD_ELSE {
                    vector<int, 16> tmp = (newRange/2);
                    vector<int, 16> newMax = Brightness + tmp;
                    vector<int, 16> newMin = Brightness - tmp;
                    temp1   = ScaleTable.iselect(Range) * (Max - Mid);
                    temp1   = ((temp1 * newRange + 43690) * 6) >> 19;
                    temp2   = ScaleTable.iselect(Range) * (Mid - Min);
                    temp2   = ((temp2 * newRange + 43690) * 6) >> 19;

                    SIMD_IF_BEGIN (MinIndex == INDEX_GREEN) {
                        SIMD_IF_BEGIN (MaxIndex == INDEX_RED) {
                            B_dst = newMax-temp1;
                            G_dst = newMin;
                            R_dst = newMax;
                        } SIMD_ELSEIF (MaxIndex == INDEX_BLUE) {
                            B_dst = newMax;
                            G_dst = newMin;
                            R_dst = newMin + temp2;
                        } SIMD_IF_END;
                    } SIMD_ELSEIF (MinIndex == INDEX_BLUE) {
                        SIMD_IF_BEGIN (MaxIndex == INDEX_GREEN) {
                            B_dst = newMin;
                            G_dst = newMax;
                            R_dst = newMax-temp1;
                        } SIMD_ELSEIF (MaxIndex == INDEX_RED) {
                            B_dst = newMin;
                            G_dst = newMin + temp2;
                            R_dst = newMax;
                        } SIMD_IF_END;
                    } SIMD_ELSEIF (MaxIndex == INDEX_BLUE) {
                        B_dst = newMax;
                        G_dst = newMax-temp1;
                        R_dst = newMin;
                    } SIMD_ELSEIF (MaxIndex == INDEX_GREEN) {
                        B_dst = newMin + temp2;
                        G_dst = newMax;
                        R_dst = newMin;
                    } SIMD_IF_END;
                } SIMD_IF_END;
            } SIMD_IF_END;

            vector<uchar, 16> B_Final = vector<uchar, 16>(B_dst, SAT);
            vector<uchar, 16> G_Final = vector<uchar, 16>(G_dst, SAT);
            vector<uchar, 16> R_Final = vector<uchar, 16>(R_dst, SAT);

            buf.select<1, 1, 16, 4>(row, 0) = B_Final;
            buf.select<1, 1, 16, 4>(row, 1) = G_Final;
            buf.select<1, 1, 16, 4>(row, 2) = R_Final;
        }

        write(out,   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        write(out,   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        write(out,   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        write(out,   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));
    }
}


extern "C" _GENX_MAIN_  void
ProcessBNC(SurfaceIndex in, SurfaceIndex out, SurfaceIndex GrayMap, int width)
{
    int v_pos = get_thread_origin_y() * 8;
    matrix<uchar, 8, 64> buf;
    vector<uint, 16> R_dst;
    vector<uint, 16> G_dst;
    vector<uint, 16> B_dst;

    for (int col = 0; col < width; col += 16)
    {
        //read in 8x16 pixel block
        int colX4 = col*4;
        read(MODIFIED(in),   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        read(MODIFIED(in),   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        read(MODIFIED(in),   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        read(MODIFIED(in),   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));

        for (uint row = 0; row<8; row ++)
        {
            vector<uint, 16> B = buf.select<1, 1, 16, 4>(row, 0);
            vector<uint, 16> G = buf.select<1, 1, 16, 4>(row, 1);
            vector<uint, 16> R = buf.select<1, 1, 16, 4>(row, 2);

            read(GrayMap, 0,   B, B_dst);
            read(GrayMap, 256, G, G_dst);
            read(GrayMap, 512, R, R_dst);

            buf.select<1, 1, 16, 4>(row, 0) = vector<uchar, 16>(B_dst, SAT);
            buf.select<1, 1, 16, 4>(row, 1) = vector<uchar, 16>(G_dst, SAT);
            buf.select<1, 1, 16, 4>(row, 2) = vector<uchar, 16>(R_dst, SAT);
        }

        write(out,   colX4,    v_pos, buf.select<8, 1, 16, 1>(0, 0));
        write(out,   colX4+16, v_pos, buf.select<8, 1, 16, 1>(0, 16));
        write(out,   colX4+32, v_pos, buf.select<8, 1, 16, 1>(0, 32));
        write(out,   colX4+48, v_pos, buf.select<8, 1, 16, 1>(0, 48));
    }
}
