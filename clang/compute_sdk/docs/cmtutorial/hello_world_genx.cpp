/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm/cm.h"

extern "C" _GENX_MAIN_ void hello_world(int threadwidth) {

    // Gets the x,y coordinates
    unsigned int x = get_thread_origin_x();
    unsigned int y = get_thread_origin_y();

    // Converts the x,y coordinates to a linearized thread ID
    unsigned int threadid = x + y*threadwidth;
    vector<ushort, 16> allTrue = 1;
    // also test printf in simd-if
    SIMD_IF_BEGIN(allTrue)
    {
        // Prints the thread ID along with a string message
        printf("%u   Hello from GPU land\n", threadid);
    }
    SIMD_IF_END
}
