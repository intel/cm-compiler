/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "GenX.h"

#include <unordered_map>

static inline constexpr uint32_t encodeGmdId(uint32_t Major, uint32_t Minor,
                                             uint32_t Revision) {
  return ((Major & 0x3ff) << 22 | (Minor & 0xff) << 14 | (Revision & 0x3f));
}

struct TargetProperties {
  bool HasFP64;
  unsigned MaxSLMSize;
};

// clang-format off
static const std::unordered_map<uint32_t, TargetProperties> TargetProps = {
  { encodeGmdId(12, 71, 4), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 71, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 70, 4), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 70, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 60, 7), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 60, 6), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 60, 5), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 60, 3), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 60, 1), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 60, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 128 }, },
  { encodeGmdId(12, 57, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 56, 5), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 56, 4), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 56, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 55, 8), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 55, 4), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 55, 1), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 55, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 50, 4), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 10, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 4, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 3, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 2, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 2, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 1, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(12, 0, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(11, 2, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(11, 0, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 7, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 6, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 5, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 4, 0), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 3, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 2, 9), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 1, 9), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(9, 0, 9), { /*.HasFP64 =*/ true, /*.MaxSLMSize =*/ 64 }, },
  { encodeGmdId(8, 0, 0), { /*.HasFP64 =*/ false, /*.MaxSLMSize =*/ 64 }, },
};
// clang-format on

void clang::targets::GenXTargetInfo::setCPUProperties() {
  auto CPUId = encodeGmdId(Major, Minor, Revision);
  auto It = TargetProps.find(CPUId);
  if (It == std::end(TargetProps))
    return;

  const auto &Prop = It->second;
  HasFP64 = Prop.HasFP64;
  MaxSLMSize = Prop.MaxSLMSize;
}
