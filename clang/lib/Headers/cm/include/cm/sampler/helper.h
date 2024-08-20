/*========================== begin_copyright_notice ============================

Copyright (C) 2024 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#if (__INCLUDE_LEVEL__ == 1)
static_assert(0, "CM:w:sampler/helper.h should not be included explicitly - "
                 "only <cm/cm.h> is required");
#endif

#ifndef _CLANG_CM_SAMPLER_HELPER_H_
#define _CLANG_CM_SAMPLER_HELPER_H_

#include <cm/cm_common.h>
#include <cm/cm_has_instr.h>
#include <cm/cm_traits.h>

namespace details {
constexpr unsigned getNumChannels(ChannelMaskType ChannelMask) {
  switch (ChannelMask) {
  case CM_R_ENABLE:
  case CM_G_ENABLE:
  case CM_B_ENABLE:
  case CM_A_ENABLE:
    return 1;
  case CM_GR_ENABLE:
  case CM_BR_ENABLE:
  case CM_BG_ENABLE:
  case CM_AR_ENABLE:
  case CM_AG_ENABLE:
  case CM_AB_ENABLE:
    return 2;
  case CM_BGR_ENABLE:
  case CM_AGR_ENABLE:
  case CM_ABR_ENABLE:
  case CM_ABG_ENABLE:
    return 3;
  case CM_ABGR_ENABLE:
    return 4;
  default:
    return 0;
  }
}
} // namespace details

#endif // _CLANG_CM_SAMPLER_HELPER_H_
