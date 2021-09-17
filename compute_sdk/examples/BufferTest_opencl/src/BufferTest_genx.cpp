
/*========================== begin_copyright_notice ============================

Copyright (C) 2009-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

extern "C" _GENX_MAIN_ void
BufferTest(SurfaceIndex ibuf [[type("buffer_t")]], SurfaceIndex obuf [[type("buffer_t")]])
{
    vector<uchar, 32> in;

    uint h_pos = cm_group_id(0) * cm_local_size(0) + cm_local_id(0);

    read(ibuf, h_pos * 32, in);
    write(obuf,  h_pos * 32, in);
}
