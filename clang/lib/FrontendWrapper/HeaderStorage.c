/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "HeaderStorage.h"

#include <assert.h>

// MSVC issues a warning on auto-generated code because it uses C feature
// that null character can be ignored in string initialization of array.
// Here it is used intentionally to concatenate split strings so
// disable warning.
#ifdef _MSC_VER
#pragma warning(disable : 4295)
#endif

#define CM_GET_RESOURCE_STORAGE
#include "EmbeddedHeaders.inc"
#undef CM_GET_RESOURCE_STORAGE

static const struct CmHeaderDesc Descs[] = {
#define CM_GET_RESOURCE_DESCS
#include "EmbeddedHeaders.inc"
#undef CM_GET_RESOURCE_DESCS
};

const struct CmHeaderDesc *CmDescBegin = Descs;
const struct CmHeaderDesc *CmDescEnd =
    &Descs[0] + (sizeof(Descs) / sizeof(Descs[0]));
