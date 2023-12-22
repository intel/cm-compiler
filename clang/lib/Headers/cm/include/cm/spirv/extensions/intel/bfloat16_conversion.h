/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CLANG_CM_SPIRV_EXT_INTEL_BFLOAT16_CONVERSION_H_
#define _CLANG_CM_SPIRV_EXT_INTEL_BFLOAT16_CONVERSION_H_

// SPV_INTEL_bfloat16_conversion extension
// https://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/INTEL/SPV_INTEL_bfloat16_conversion.html

float __spirv_ConvertBF16ToFINTEL(short);
short __spirv_ConvertFToBF16INTEL(float);

template <int Width>
vector<float, Width> __spirv_ConvertBF16ToFINTEL(vector<short, Width>);
template <int Width>
vector<short, Width> __spirv_ConvertFToBF16INTEL(vector<float, Width>);

#endif // _CLANG_CM_SPIRV_EXT_INTEL_BFLOAT16_CONVERSION_H_
