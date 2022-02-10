/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm/cm.h"

extern "C" _GENX_MAIN
void kernel(svmptr_t ibuf [[type("svmptr_t")]],
            ...)
{
    uint offset = cm_group_id(0) * cm_local_size(0) + cm_local_id(0);
    vector<float, 4> v;
    cm_svm_block_read(ibuf + offset, out);
    ...
}
