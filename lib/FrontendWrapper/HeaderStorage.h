/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef HEADER_STRINGS_H
#define HEADER_STRINGS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CmHeaderDesc {
  // Full header name with path (C-string with null-terminator).
  const char *Name;
  // Begin of header data (no null-terminator at end).
  const char *Begin;
  // Size of header data.
  size_t Size;
};

// Range with all header descriptors.
extern const struct CmHeaderDesc *CmDescBegin;
extern const struct CmHeaderDesc *CmDescEnd;

#ifdef __cplusplus
}
#endif

#endif
