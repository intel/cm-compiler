/*========================== begin_copyright_notice ============================

Copyright (C) 2018-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/*========================== begin_copyright_notice ============================

This file is distributed under the University of Illinois Open Source License.
See LICENSE.TXT for details.

============================= end_copyright_notice ===========================*/

#ifndef LLVM_CLANG_DRIVER_OPTIONS_H
#define LLVM_CLANG_DRIVER_OPTIONS_H

#include <memory>

namespace llvm {
namespace opt {
class OptTable;
}
}

namespace clang {
namespace driver {

namespace options {
/// Flags specifically for clang options.  Must not overlap with
/// llvm::opt::DriverFlag.
enum ClangFlags {
  DriverOption = (1 << 4),
  LinkerInput = (1 << 5),
  NoArgumentUnused = (1 << 6),
  Unsupported = (1 << 7),
  CoreOption = (1 << 8),
  CLOption = (1 << 9),
  CC1Option = (1 << 10),
  CC1AsOption = (1 << 11),
  NoDriverOption = (1 << 12),
  CMOption = (1 << 13),          // MDF CM options
  Ignored = (1 << 14)
};

enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR, VALUES)                                      \
  OPT_##ID,
#include "clang/Driver/Options.inc"
    LastOption
#undef OPTION
  };
}

std::unique_ptr<llvm::opt::OptTable> createDriverOptTable();
}
}

#endif
