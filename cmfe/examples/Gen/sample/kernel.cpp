/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#include <cm/cm.h>


#define SURFACE_TYPE [[type("buffer_t")]]

#define SZ 16
_GENX_MAIN_ void vector_add(
	SurfaceIndex isurface1 SURFACE_TYPE,
	SurfaceIndex isurface2 SURFACE_TYPE,
	SurfaceIndex osurface SURFACE_TYPE
	)
{
    vector<int, SZ> ivector1;
    vector<int, SZ> ivector2;
    vector<int, SZ> ovector;

    //printf("gid(0)=%d, gid(1)=%d, lid(0)=%d, lid(1)=%d\n", cm_group_id(0), cm_group_id(1), cm_local_id(0), cm_local_id(1));
    unsigned offset = sizeof(unsigned) * SZ * cm_group_id(0);
    //
    // read-in the arguments
    read(isurface1, offset, ivector1);
    read(isurface2, offset, ivector2);
    // perform addition
    ovector = ivector1 + ivector2;
    // write-out the results
    write (osurface, offset, ovector);
}
