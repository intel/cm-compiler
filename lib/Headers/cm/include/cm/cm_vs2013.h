/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// this file is added to make the old compiler happy when the include path
// is renamed 
 
#ifndef CM_VS2013_H
#define CM_VS2013_H

#if _MSC_VER >= 1800
// Workaround for VS2013 type traits  - these are not supported by icl
// so we force them all to false here to allow them to appear without
// compiler errors being generated.
// Note: They should not be used as they are not actually implemented!
#define __is_nothrow_assignable(X,...) (false)
#define __is_nothrow_constructible(X,...) (false)
#define __is_nothrow_destructible(X,...) (false)
#define __is_constructible(X,...) (false)
#define __is_destructible(X,...) (false)
#define __is_trivially_assignable(X,...) (false)
#define __is_trivially_constructible(X,...) (false)
#endif

#endif /* CM_VS2013_H */
