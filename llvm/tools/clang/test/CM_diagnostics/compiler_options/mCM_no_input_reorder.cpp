#include <cm/cm.h>

_GENX_MAIN_ void foo(SurfaceIndex ibuf, SurfaceIndex obuf, char val, uint h_pos, uint v_pos)
{
    matrix<uchar, 8, 32> in;
    matrix<uchar, 6, 24> out;
    matrix<float, 6, 24> m;

    read(ibuf, h_pos*24, v_pos*6, in); 

    m  = in.select<6,1,24,1>(1,3);

    m += in.select<6,1,24,1>(0,0);
    m += in.select<6,1,24,1>(0,3);
    m += in.select<6,1,24,1>(0,6);

    m += in.select<6,1,24,1>(1,0);
    m += in.select<6,1,24,1>(1,6);

    m += in.select<6,1,24,1>(2,0);
    m += in.select<6,1,24,1>(2,3);
    m += in.select<6,1,24,1>(2,6);
    m += (float)val;

    out = m * 0.111f;

    write(obuf, h_pos*24, v_pos*6, out); 
}

// RUN: %cmc -Qxcm_jit_target=BDW -mCM_no_input_reorder %w | FileCheck %w
//
// CHECK: -platform BDW
// CHECK-NOT: error
// CHECK-NOT: warning

// We check that the generated asm contains arg reorder information that indicates
// no reordering has taken place (default would be to reorder)
//
// RUN: FileCheck -input-file=%W_0.asm -check-prefix=ASM %w
//
// ASM: //.kernel_reordering_info_start
// ASM-NEXT: //id   	byte_offset    	byte_size      	kind   	implicit_kind
// ASM-NEXT: //.arg_1       	32     	4      	2      	0
// ASM-NEXT: //.arg_2       	36     	4      	2      	0
// ASM-NEXT: //.arg_3       	40     	1      	0      	0
// ASM-NEXT: //.arg_4       	44     	4      	0      	0
// ASM-NEXT: //.arg_5       	48     	4      	0      	0
// ASM-NEXT: //.kernel_reordering_info_end

// tidy up the generated files
// RUN: rm %W_0.dat %W_0.visaasm %W_0.asm %W.isa
