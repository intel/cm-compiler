/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_EXT_INTEL_SIGMOID_H_
#define _CLANG_CM_SPIRV_EXT_INTEL_SIGMOID_H_

half __spirv_FSigmoidINTEL(half);
template <int Width>
vector<half, Width> __spirv_FSigmoidINTEL(vector<half, Width>);

float __spirv_FSigmoidINTEL(float);
template <int Width>
vector<float, Width> __spirv_FSigmoidINTEL(vector<float, Width>);

#ifdef CM_HAS_BF16
__bf16 __spirv_FSigmoidINTEL(__bf16);
template <int Width>
vector<__bf16, Width> __spirv_FSigmoidINTEL(vector<__bf16, Width>);
#endif // CM_HAS_BF16

#endif // _CLANG_CM_SPIRV_EXT_INTEL_SIGMOID_H_
