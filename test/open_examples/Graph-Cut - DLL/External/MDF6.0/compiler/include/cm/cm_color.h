/**             
*** ---------------------------------------------------------------------------
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
**/

#ifndef CM_COLOR_H
#define CM_COLOR_H

#ifndef CM_EMU
 _GENX_ inline ushort get_color()
{
    vector<unsigned int, 8> Color = cm_get_r0<unsigned int>();
    return (Color(2) & 0xF);
}
 #endif

#endif
