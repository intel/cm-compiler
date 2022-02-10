/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// The header define CM intrinsics and macros.
// Note that std library headers are not supported.
#include <cm/cm.h>

// extern "C" is used to avoid C++ name mangling.
//
// __GENX_MAIN_ is a macro to declare a CM kernel. This micro expends to __declspec(genx_main).
//
// CM kernels are expected to return void.
//
extern "C" _GENX_MAIN_ void vadd(SurfaceIndex ibuf0 [[type("buffer_t")]],
                                 SurfaceIndex ibuf1 [[type("buffer_t")]],
                                 SurfaceIndex obuf  [[type("buffer_t")]])
{
    unsigned tid = cm_group_id(0) * cm_local_size(0) + cm_local_id(0);

    vector<unsigned, 32> in0;
    read(ibuf0, tid * 32 * sizeof(unsigned), in0);

    vector<unsigned, 32> in1;
    read(ibuf1, tid * 32 * sizeof(unsigned), in1);

    in0 += in1;

    write(obuf,  tid * 32 * sizeof(unsigned), in0);
}
