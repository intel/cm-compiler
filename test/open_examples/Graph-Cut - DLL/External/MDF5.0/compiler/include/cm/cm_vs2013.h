/**             
***
*** Copyright  (C) 2014 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             
***                      
***
*** Description: This header contains definitions that allows the new 
***              type_traits present in the VS2013 header files to be
***              parsed, but they are not implemented semantically so
***              cannot be used successfully.
***              This header file is implicitly included before other
***              header files whenever this workaround is required.
***
*** -----------------------------------------------------------------------------------------------
**/

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
