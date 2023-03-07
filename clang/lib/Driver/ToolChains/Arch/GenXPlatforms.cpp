/*========================== begin_copyright_notice ============================

Copyright (C) 2023 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "GenX.h"

#include <string>
#include <unordered_map>

using clang::driver::tools::GenX::encodeGmdId;

// clang-format off
static const std::unordered_map<std::string, uint32_t> ReleaseId = {
  {"xe-lpg", encodeGmdId(12, 71, 4)},
  {"xe-hpc", encodeGmdId(12, 60, 7)},
  {"xe-hpg", encodeGmdId(12, 57, 0)},
  {"xe-hp", encodeGmdId(12, 50, 4)},
  {"gen12lp", encodeGmdId(12, 10, 0)},
  {"gen11", encodeGmdId(11, 2, 0)},
  {"gen9", encodeGmdId(9, 7, 0)},
  {"gen8", encodeGmdId(8, 0, 0)},
};

static const std::unordered_map<std::string, uint32_t> DeviceId = {
  {"mtl-p", encodeGmdId(12, 71, 4)},
  {"mtl-m", encodeGmdId(12, 70, 4)},
  {"mtl-s", encodeGmdId(12, 70, 4)},
  {"pvc", encodeGmdId(12, 60, 7)},
  {"pvc-sdv", encodeGmdId(12, 60, 1)},
  {"acm-g12", encodeGmdId(12, 57, 0)},
  {"dg2-g12", encodeGmdId(12, 57, 0)},
  {"acm-g11", encodeGmdId(12, 56, 5)},
  {"dg2-g11", encodeGmdId(12, 56, 5)},
  {"acm-g10", encodeGmdId(12, 55, 8)},
  {"dg2-g10", encodeGmdId(12, 55, 8)},
  {"dg1", encodeGmdId(12, 10, 0)},
  {"adl-n", encodeGmdId(12, 4, 0)},
  {"adl-p", encodeGmdId(12, 3, 0)},
  {"rpl-s", encodeGmdId(12, 2, 0)},
  {"adl-s", encodeGmdId(12, 2, 0)},
  {"rkl", encodeGmdId(12, 1, 0)},
  {"tgllp", encodeGmdId(12, 0, 0)},
  {"ehl", encodeGmdId(11, 2, 0)},
  {"jsl", encodeGmdId(11, 2, 0)},
  {"icllp", encodeGmdId(11, 0, 0)},
  {"cml", encodeGmdId(9, 7, 0)},
  {"aml", encodeGmdId(9, 6, 0)},
  {"whl", encodeGmdId(9, 5, 0)},
  {"glk", encodeGmdId(9, 4, 0)},
  {"apl", encodeGmdId(9, 3, 0)},
  {"bxt", encodeGmdId(9, 3, 0)},
  {"cfl", encodeGmdId(9, 2, 9)},
  {"kbl", encodeGmdId(9, 1, 9)},
  {"skl", encodeGmdId(9, 0, 9)},
  {"bdw", encodeGmdId(8, 0, 0)},
};

static const std::unordered_map<std::string, uint32_t> PciId {
  {"0x7d70", encodeGmdId(12, 70, 0)},
  {"0x7d40", encodeGmdId(12, 70, 0)},
  {"0x7d45", encodeGmdId(12, 70, 0)},
  {"0x7d60", encodeGmdId(12, 70, 0)},
  {"0x7de0", encodeGmdId(12, 70, 0)},
  {"0x7d50", encodeGmdId(12, 70, 0)},
  {"0x7d55", encodeGmdId(12, 70, 0)},
  {"0x7dd0", encodeGmdId(12, 70, 0)},
  {"0x7dd5", encodeGmdId(12, 70, 0)},
  {"0x7d75", encodeGmdId(12, 70, 0)},
  {"0x7d79", encodeGmdId(12, 70, 0)},
  {"0x7dc0", encodeGmdId(12, 70, 0)},
  {"0x7dc5", encodeGmdId(12, 70, 0)},
  {"0x0bd5", encodeGmdId(12, 60, 7)},
  {"0x0bd6", encodeGmdId(12, 60, 7)},
  {"0x0bd7", encodeGmdId(12, 60, 7)},
  {"0x0bd8", encodeGmdId(12, 60, 7)},
  {"0x0bd9", encodeGmdId(12, 60, 7)},
  {"0x0bda", encodeGmdId(12, 60, 7)},
  {"0x0bdb", encodeGmdId(12, 60, 7)},
  {"0x0bd0", encodeGmdId(12, 60, 0)},
  {"0x5693", encodeGmdId(12, 56, 5)},
  {"0x5694", encodeGmdId(12, 56, 5)},
  {"0x5695", encodeGmdId(12, 56, 5)},
  {"0x5698", encodeGmdId(12, 56, 5)},
  {"0x56a5", encodeGmdId(12, 56, 5)},
  {"0x56a6", encodeGmdId(12, 56, 5)},
  {"0x56b0", encodeGmdId(12, 56, 5)},
  {"0x56b1", encodeGmdId(12, 56, 5)},
  {"0x56c1", encodeGmdId(12, 56, 5)},
  {"0x5690", encodeGmdId(12, 55, 8)},
  {"0x5691", encodeGmdId(12, 55, 8)},
  {"0x5692", encodeGmdId(12, 55, 8)},
  {"0x56a0", encodeGmdId(12, 55, 8)},
  {"0x56a1", encodeGmdId(12, 55, 8)},
  {"0x56a2", encodeGmdId(12, 55, 8)},
  {"0x56c0", encodeGmdId(12, 55, 8)},
  {"0x0201", encodeGmdId(12, 50, 4)},
  {"0x0203", encodeGmdId(12, 50, 4)},
  {"0x0208", encodeGmdId(12, 50, 4)},
  {"0x0204", encodeGmdId(12, 50, 4)},
  {"0x0209", encodeGmdId(12, 50, 4)},
  {"0x020b", encodeGmdId(12, 50, 4)},
  {"0x0202", encodeGmdId(12, 50, 4)},
  {"0x0205", encodeGmdId(12, 50, 4)},
  {"0x020e", encodeGmdId(12, 50, 4)},
  {"0x0210", encodeGmdId(12, 50, 4)},
  {"0x0206", encodeGmdId(12, 50, 4)},
  {"0x020a", encodeGmdId(12, 50, 4)},
  {"0x020c", encodeGmdId(12, 50, 4)},
  {"0x0207", encodeGmdId(12, 50, 4)},
  {"0x020d", encodeGmdId(12, 50, 4)},
  {"0x020f", encodeGmdId(12, 50, 4)},
  {"0x4905", encodeGmdId(12, 10, 0)},
  {"0x4906", encodeGmdId(12, 10, 0)},
  {"0x4907", encodeGmdId(12, 10, 0)},
  {"0x4908", encodeGmdId(12, 10, 0)},
  {"0x4909", encodeGmdId(12, 10, 0)},
  {"0x46d0", encodeGmdId(12, 4, 0)},
  {"0x46d1", encodeGmdId(12, 4, 0)},
  {"0x46d2", encodeGmdId(12, 4, 0)},
  {"0x46a2", encodeGmdId(12, 3, 0)},
  {"0x46b2", encodeGmdId(12, 3, 0)},
  {"0x46c2", encodeGmdId(12, 3, 0)},
  {"0x46a1", encodeGmdId(12, 3, 0)},
  {"0x46b1", encodeGmdId(12, 3, 0)},
  {"0x46c1", encodeGmdId(12, 3, 0)},
  {"0x46a0", encodeGmdId(12, 3, 0)},
  {"0x46b0", encodeGmdId(12, 3, 0)},
  {"0x46c0", encodeGmdId(12, 3, 0)},
  {"0x4626", encodeGmdId(12, 3, 0)},
  {"0x4628", encodeGmdId(12, 3, 0)},
  {"0x462a", encodeGmdId(12, 3, 0)},
  {"0x4636", encodeGmdId(12, 3, 0)},
  {"0x4638", encodeGmdId(12, 3, 0)},
  {"0x463a", encodeGmdId(12, 3, 0)},
  {"0x46a3", encodeGmdId(12, 3, 0)},
  {"0x46a6", encodeGmdId(12, 3, 0)},
  {"0x46a8", encodeGmdId(12, 3, 0)},
  {"0x46aa", encodeGmdId(12, 3, 0)},
  {"0x46b3", encodeGmdId(12, 3, 0)},
  {"0x46b6", encodeGmdId(12, 3, 0)},
  {"0x46b8", encodeGmdId(12, 3, 0)},
  {"0x46ba", encodeGmdId(12, 3, 0)},
  {"0x46c3", encodeGmdId(12, 3, 0)},
  {"0xa780", encodeGmdId(12, 2, 0)},
  {"0xa781", encodeGmdId(12, 2, 0)},
  {"0xa788", encodeGmdId(12, 2, 0)},
  {"0xa789", encodeGmdId(12, 2, 0)},
  {"0xa782", encodeGmdId(12, 2, 0)},
  {"0xa78a", encodeGmdId(12, 2, 0)},
  {"0xa783", encodeGmdId(12, 2, 0)},
  {"0xa784", encodeGmdId(12, 2, 0)},
  {"0xa785", encodeGmdId(12, 2, 0)},
  {"0xa786", encodeGmdId(12, 2, 0)},
  {"0xa787", encodeGmdId(12, 2, 0)},
  {"0xa78b", encodeGmdId(12, 2, 0)},
  {"0xa78c", encodeGmdId(12, 2, 0)},
  {"0xa78d", encodeGmdId(12, 2, 0)},
  {"0xa78e", encodeGmdId(12, 2, 0)},
  {"0xa78f", encodeGmdId(12, 2, 0)},
  {"0xa790", encodeGmdId(12, 2, 0)},
  {"0xa791", encodeGmdId(12, 2, 0)},
  {"0xa792", encodeGmdId(12, 2, 0)},
  {"0xa793", encodeGmdId(12, 2, 0)},
  {"0xa794", encodeGmdId(12, 2, 0)},
  {"0xa795", encodeGmdId(12, 2, 0)},
  {"0xa796", encodeGmdId(12, 2, 0)},
  {"0xa797", encodeGmdId(12, 2, 0)},
  {"0xa798", encodeGmdId(12, 2, 0)},
  {"0xa799", encodeGmdId(12, 2, 0)},
  {"0xa79a", encodeGmdId(12, 2, 0)},
  {"0xa79b", encodeGmdId(12, 2, 0)},
  {"0xa79c", encodeGmdId(12, 2, 0)},
  {"0xa79d", encodeGmdId(12, 2, 0)},
  {"0xa79e", encodeGmdId(12, 2, 0)},
  {"0x4680", encodeGmdId(12, 2, 0)},
  {"0x4681", encodeGmdId(12, 2, 0)},
  {"0x4688", encodeGmdId(12, 2, 0)},
  {"0x4689", encodeGmdId(12, 2, 0)},
  {"0x4690", encodeGmdId(12, 2, 0)},
  {"0x4691", encodeGmdId(12, 2, 0)},
  {"0x4698", encodeGmdId(12, 2, 0)},
  {"0x4699", encodeGmdId(12, 2, 0)},
  {"0x469a", encodeGmdId(12, 2, 0)},
  {"0x469b", encodeGmdId(12, 2, 0)},
  {"0x469c", encodeGmdId(12, 2, 0)},
  {"0x469d", encodeGmdId(12, 2, 0)},
  {"0x469e", encodeGmdId(12, 2, 0)},
  {"0x4682", encodeGmdId(12, 2, 0)},
  {"0x468a", encodeGmdId(12, 2, 0)},
  {"0x4692", encodeGmdId(12, 2, 0)},
  {"0x4683", encodeGmdId(12, 2, 0)},
  {"0x4684", encodeGmdId(12, 2, 0)},
  {"0x4685", encodeGmdId(12, 2, 0)},
  {"0x4686", encodeGmdId(12, 2, 0)},
  {"0x4687", encodeGmdId(12, 2, 0)},
  {"0x468b", encodeGmdId(12, 2, 0)},
  {"0x468c", encodeGmdId(12, 2, 0)},
  {"0x468d", encodeGmdId(12, 2, 0)},
  {"0x468e", encodeGmdId(12, 2, 0)},
  {"0x468f", encodeGmdId(12, 2, 0)},
  {"0x4693", encodeGmdId(12, 2, 0)},
  {"0x4694", encodeGmdId(12, 2, 0)},
  {"0x4695", encodeGmdId(12, 2, 0)},
  {"0x4696", encodeGmdId(12, 2, 0)},
  {"0x4697", encodeGmdId(12, 2, 0)},
  {"0x4c80", encodeGmdId(12, 1, 0)},
  {"0x4c81", encodeGmdId(12, 1, 0)},
  {"0x4c82", encodeGmdId(12, 1, 0)},
  {"0x4c83", encodeGmdId(12, 1, 0)},
  {"0x4c84", encodeGmdId(12, 1, 0)},
  {"0x4c85", encodeGmdId(12, 1, 0)},
  {"0x4c86", encodeGmdId(12, 1, 0)},
  {"0x4c87", encodeGmdId(12, 1, 0)},
  {"0x4c88", encodeGmdId(12, 1, 0)},
  {"0x4c89", encodeGmdId(12, 1, 0)},
  {"0x4c8a", encodeGmdId(12, 1, 0)},
  {"0x4c90", encodeGmdId(12, 1, 0)},
  {"0x4c91", encodeGmdId(12, 1, 0)},
  {"0x4c92", encodeGmdId(12, 1, 0)},
  {"0x4c93", encodeGmdId(12, 1, 0)},
  {"0x4c94", encodeGmdId(12, 1, 0)},
  {"0x4c95", encodeGmdId(12, 1, 0)},
  {"0x4c96", encodeGmdId(12, 1, 0)},
  {"0x4c97", encodeGmdId(12, 1, 0)},
  {"0x4c98", encodeGmdId(12, 1, 0)},
  {"0x4c99", encodeGmdId(12, 1, 0)},
  {"0x4c9a", encodeGmdId(12, 1, 0)},
  {"0x4c9b", encodeGmdId(12, 1, 0)},
  {"0x4c9c", encodeGmdId(12, 1, 0)},
  {"0x4c9d", encodeGmdId(12, 1, 0)},
  {"0x4c9e", encodeGmdId(12, 1, 0)},
  {"0x4c8b", encodeGmdId(12, 1, 0)},
  {"0x4c8c", encodeGmdId(12, 1, 0)},
  {"0x4c8d", encodeGmdId(12, 1, 0)},
  {"0x4c8e", encodeGmdId(12, 1, 0)},
  {"0x4c8f", encodeGmdId(12, 1, 0)},
  {"0x9a68", encodeGmdId(12, 0, 0)},
  {"0x9a60", encodeGmdId(12, 0, 0)},
  {"0x9a70", encodeGmdId(12, 0, 0)},
  {"0x9a78", encodeGmdId(12, 0, 0)},
  {"0x9af8", encodeGmdId(12, 0, 0)},
  {"0x9a40", encodeGmdId(12, 0, 0)},
  {"0x9a49", encodeGmdId(12, 0, 0)},
  {"0x9a59", encodeGmdId(12, 0, 0)},
  {"0x9ac0", encodeGmdId(12, 0, 0)},
  {"0x9ac9", encodeGmdId(12, 0, 0)},
  {"0x9ad9", encodeGmdId(12, 0, 0)},
  {"0x9a7f", encodeGmdId(12, 0, 0)},
  {"0x4e71", encodeGmdId(11, 2, 0)},
  {"0x4e61", encodeGmdId(11, 2, 0)},
  {"0x4e51", encodeGmdId(11, 2, 0)},
  {"0x4e55", encodeGmdId(11, 2, 0)},
  {"0x4571", encodeGmdId(11, 2, 0)},
  {"0x4551", encodeGmdId(11, 2, 0)},
  {"0x4541", encodeGmdId(11, 2, 0)},
  {"0x4555", encodeGmdId(11, 2, 0)},
  {"0x8a71", encodeGmdId(11, 0, 0)},
  {"0x8a56", encodeGmdId(11, 0, 0)},
  {"0x8a58", encodeGmdId(11, 0, 0)},
  {"0x8a5b", encodeGmdId(11, 0, 0)},
  {"0x8a5d", encodeGmdId(11, 0, 0)},
  {"0x8a54", encodeGmdId(11, 0, 0)},
  {"0x8a57", encodeGmdId(11, 0, 0)},
  {"0x8a59", encodeGmdId(11, 0, 0)},
  {"0x8a5a", encodeGmdId(11, 0, 0)},
  {"0x8a5c", encodeGmdId(11, 0, 0)},
  {"0x8a50", encodeGmdId(11, 0, 0)},
  {"0x8a51", encodeGmdId(11, 0, 0)},
  {"0x8a52", encodeGmdId(11, 0, 0)},
  {"0x8a53", encodeGmdId(11, 0, 0)},
  {"0x9b21", encodeGmdId(9, 7, 0)},
  {"0x9b41", encodeGmdId(9, 7, 0)},
  {"0x9ba2", encodeGmdId(9, 7, 0)},
  {"0x9ba4", encodeGmdId(9, 7, 0)},
  {"0x9ba5", encodeGmdId(9, 7, 0)},
  {"0x9ba8", encodeGmdId(9, 7, 0)},
  {"0x9baa", encodeGmdId(9, 7, 0)},
  {"0x9bac", encodeGmdId(9, 7, 0)},
  {"0x9bc2", encodeGmdId(9, 7, 0)},
  {"0x9bc4", encodeGmdId(9, 7, 0)},
  {"0x9bc5", encodeGmdId(9, 7, 0)},
  {"0x9bc6", encodeGmdId(9, 7, 0)},
  {"0x9bc8", encodeGmdId(9, 7, 0)},
  {"0x9bca", encodeGmdId(9, 7, 0)},
  {"0x9bcc", encodeGmdId(9, 7, 0)},
  {"0x9be6", encodeGmdId(9, 7, 0)},
  {"0x9bf6", encodeGmdId(9, 7, 0)},
  {"0x591c", encodeGmdId(9, 6, 0)},
  {"0x87c0", encodeGmdId(9, 6, 0)},
  {"0x87ca", encodeGmdId(9, 6, 0)},
  {"0x3ea0", encodeGmdId(9, 5, 0)},
  {"0x3ea1", encodeGmdId(9, 5, 0)},
  {"0x3ea2", encodeGmdId(9, 5, 0)},
  {"0x3ea3", encodeGmdId(9, 5, 0)},
  {"0x3ea4", encodeGmdId(9, 5, 0)},
  {"0x3184", encodeGmdId(9, 4, 0)},
  {"0x3185", encodeGmdId(9, 4, 0)},
  {"0x1a85", encodeGmdId(9, 3, 0)},
  {"0x5a85", encodeGmdId(9, 3, 0)},
  {"0x0a84", encodeGmdId(9, 3, 0)},
  {"0x1a84", encodeGmdId(9, 3, 0)},
  {"0x5a84", encodeGmdId(9, 3, 0)},
  {"0x3e90", encodeGmdId(9, 2, 9)},
  {"0x3e91", encodeGmdId(9, 2, 9)},
  {"0x3e92", encodeGmdId(9, 2, 9)},
  {"0x3e93", encodeGmdId(9, 2, 9)},
  {"0x3e94", encodeGmdId(9, 2, 9)},
  {"0x3e96", encodeGmdId(9, 2, 9)},
  {"0x3e98", encodeGmdId(9, 2, 9)},
  {"0x3e99", encodeGmdId(9, 2, 9)},
  {"0x3e9a", encodeGmdId(9, 2, 9)},
  {"0x3e9b", encodeGmdId(9, 2, 9)},
  {"0x3e9c", encodeGmdId(9, 2, 9)},
  {"0x3ea5", encodeGmdId(9, 2, 9)},
  {"0x3ea6", encodeGmdId(9, 2, 9)},
  {"0x3ea7", encodeGmdId(9, 2, 9)},
  {"0x3ea8", encodeGmdId(9, 2, 9)},
  {"0x3ea9", encodeGmdId(9, 2, 9)},
  {"0x5902", encodeGmdId(9, 1, 9)},
  {"0x5906", encodeGmdId(9, 1, 9)},
  {"0x5908", encodeGmdId(9, 1, 9)},
  {"0x590a", encodeGmdId(9, 1, 9)},
  {"0x590b", encodeGmdId(9, 1, 9)},
  {"0x590e", encodeGmdId(9, 1, 9)},
  {"0x5913", encodeGmdId(9, 1, 9)},
  {"0x5915", encodeGmdId(9, 1, 9)},
  {"0x5912", encodeGmdId(9, 1, 9)},
  {"0x5916", encodeGmdId(9, 1, 9)},
  {"0x5917", encodeGmdId(9, 1, 9)},
  {"0x591a", encodeGmdId(9, 1, 9)},
  {"0x591b", encodeGmdId(9, 1, 9)},
  {"0x591d", encodeGmdId(9, 1, 9)},
  {"0x591e", encodeGmdId(9, 1, 9)},
  {"0x5921", encodeGmdId(9, 1, 9)},
  {"0x5923", encodeGmdId(9, 1, 9)},
  {"0x5926", encodeGmdId(9, 1, 9)},
  {"0x5927", encodeGmdId(9, 1, 9)},
  {"0x193a", encodeGmdId(9, 0, 9)},
  {"0x193b", encodeGmdId(9, 0, 9)},
  {"0x193d", encodeGmdId(9, 0, 9)},
  {"0x192a", encodeGmdId(9, 0, 9)},
  {"0x1932", encodeGmdId(9, 0, 9)},
  {"0x1926", encodeGmdId(9, 0, 9)},
  {"0x1927", encodeGmdId(9, 0, 9)},
  {"0x192b", encodeGmdId(9, 0, 9)},
  {"0x192d", encodeGmdId(9, 0, 9)},
  {"0x1923", encodeGmdId(9, 0, 9)},
  {"0x1912", encodeGmdId(9, 0, 9)},
  {"0x1916", encodeGmdId(9, 0, 9)},
  {"0x191a", encodeGmdId(9, 0, 9)},
  {"0x191b", encodeGmdId(9, 0, 9)},
  {"0x191d", encodeGmdId(9, 0, 9)},
  {"0x191e", encodeGmdId(9, 0, 9)},
  {"0x1921", encodeGmdId(9, 0, 9)},
  {"0x1913", encodeGmdId(9, 0, 9)},
  {"0x1915", encodeGmdId(9, 0, 9)},
  {"0x1917", encodeGmdId(9, 0, 9)},
  {"0x190a", encodeGmdId(9, 0, 9)},
  {"0x190b", encodeGmdId(9, 0, 9)},
  {"0x190e", encodeGmdId(9, 0, 9)},
  {"0x1902", encodeGmdId(9, 0, 9)},
  {"0x1906", encodeGmdId(9, 0, 9)},
  {"0x1626", encodeGmdId(8, 0, 0)},
  {"0x162b", encodeGmdId(8, 0, 0)},
  {"0x1622", encodeGmdId(8, 0, 0)},
  {"0x162a", encodeGmdId(8, 0, 0)},
  {"0x1612", encodeGmdId(8, 0, 0)},
  {"0x1616", encodeGmdId(8, 0, 0)},
  {"0x161e", encodeGmdId(8, 0, 0)},
  {"0x1606", encodeGmdId(8, 0, 0)},
};
// clang-format on

uint32_t clang::driver::tools::GenX::getDeviceId(const std::string &Name) {
  auto ReleaseIt = ReleaseId.find(Name);
  if (ReleaseIt != ReleaseId.end())
    return ReleaseIt->second;
  auto DeviceIt = DeviceId.find(Name);
  if (DeviceIt != DeviceId.end())
    return DeviceIt->second;
  auto PciIt = PciId.find(Name);
  if (PciIt != PciId.end())
    return PciIt->second;

  uint32_t Major = 0, Minor = 0, Revision = 0;
  if (std::sscanf(Name.c_str(), "%u.%u.%u", &Major, &Minor, &Revision) == 3)
    return encodeGmdId(Major, Minor, Revision);
  return 0;
}
