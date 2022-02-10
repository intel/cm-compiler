/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

extern "C" _GENX_MAIN_ void
linear(SurfaceIndex ibuf, SurfaceIndex obuf)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<float, 6, 24> m;

    uint h_pos = get_thread_origin_x();
    uint v_pos = get_thread_origin_y();

    read(ibuf, h_pos*24, v_pos*6, in);

    m  = in.select<6,1,24,1>(1,3);

    m += in.select<6,1,24,1>(0,0);
    m += in.select<6,1,24,1>(0,3);
    m += in.select<6,1,24,1>(0,6);

    m += in.select<6,1,24,1>(1,0);
    m += in.select<6,1,24,1>(1,6);

    m += in.select<6,1,24,1>(2,0);
    m += in.select<6,1,24,1>(2,3);
    m += in.select<6,1,24,1>(2,6);

    out = m * 0.111f;

    write(obuf, h_pos*24, v_pos*6, out);
}
