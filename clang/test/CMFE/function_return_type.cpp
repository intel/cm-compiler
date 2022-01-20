/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

typedef enum { 
  twenty = 20,
  twenty_one = 21,
  twenty_two = 22,
  twenty_three = 23
} MyEnum;

_GENX_ void func_void(int a) { return; }

_GENX_ char func_char(int a) { return a; }

_GENX_ signed char func_signed_char(int a) { return a; }

_GENX_ uchar func_uchar(int a) { return a; }

_GENX_ unsigned char func_unsigned_char(int a) { return a; } 

_GENX_ short func_short(int a) { return a; }

_GENX_ signed short func_signed_short(int a) { return a; }

_GENX_ ushort func_ushort(int a) { return a; }

_GENX_ unsigned short func_unsigned_short(int a) { return a; }

_GENX_ int func_int(int a) { return a; }

_GENX_ signed int func_signed_int(int a) { return a; }

_GENX_ uint func_uint(int a) { return a; }

_GENX_ unsigned int func_unsigned_int(int a) { return a; }

_GENX_ long func_long(int a) { return a; }

_GENX_ signed long func_signed_long(int a) { return a; }

_GENX_ unsigned long func_unsigned_long(int a) { return a; }

_GENX_ long long func_long_long(int a) { return a; }

_GENX_ signed long long func_signed_long_long(int a) { return a; }

_GENX_ unsigned long long func_unsigned_long_long(int a) { return a; }

_GENX_ half func_half(int a) { return a; }

_GENX_ float func_float(int a) { return a; }

_GENX_ double func_double(int a) { return a; }

_GENX_ bool func_bool(int a) { return a; }

_GENX_ MyEnum func_enum(int a) { return MyEnum(a); }

_GENX_ SamplerIndex func_SamplerIndex(int a) { return a; }  // Unsupported return type // expected-error{{unsupported function return type 'SamplerIndex'}}

_GENX_ SurfaceIndex func_SurfaceIndex(int a) { return a; }  // Unsupported return type // expected-error{{unsupported function return type 'SurfaceIndex'}}

_GENX_ VmeIndex func_VmeIndex(int a) { VmeIndex vi; return vi; }  // Unsupported return type // expected-error{{unsupported function return type 'VmeIndex'}}

_GENX_ matrix<int,4,4> func_matrix(int a) { return a; }

_GENX_ matrix_ref<int,4,4> func_matrix_ref(int a) { matrix<int,4,4> r = a; return r; }  // Unsupported return type // expected-error{{unsupported function return type 'matrix_ref<int,4,4>'}}

_GENX_ vector<int,4> func_vector(int a) { return a; }

_GENX_ vector_ref<int,4> func_vector_ref(int a) { vector<int,4> r = a; return r; }  // Unsupported return type // expected-error{{unsupported function return type 'vector_ref<int,4>'}}


_GENX_MAIN_ void foo(SurfaceIndex idx)
{
    func_void(0);

    char r1 = func_char(1);

    signed char r2 = func_signed_char(2);

    uchar r3 = func_uchar(3);

    unsigned char r4 = func_unsigned_char(4);

    short r5 = func_short(5);

    signed short r6 = func_signed_short(6);

    ushort r7 = func_ushort(7);

    unsigned short r8 = func_unsigned_short(8);

    int r9 = func_int(9);

    signed int r10 = func_signed_int(10);

    uint r11 = func_uint(11);

    unsigned int r12 = func_unsigned_int(12);

    long r13 = func_long(13);

    signed long r14 = func_signed_long(14);

    unsigned long r15 = func_unsigned_long(15);

    long long r16 = func_long_long(16);

    signed long long r17= func_signed_long_long(17);

    unsigned long long r18 = func_unsigned_long_long(18);

    half r19 = func_half(19);

    float r20 = func_float(20);

    double r21 = func_double(21);

    bool r22 = func_bool(22);

    MyEnum r23 = func_enum(23);

    SamplerIndex r24 = func_SamplerIndex(24);

    SurfaceIndex r25 = func_SurfaceIndex(25);

    VmeIndex r26 = func_VmeIndex(26);

    matrix<int,4,4> r27 = func_matrix(27);

    matrix<int,4,4> r28 = func_matrix_ref(28);

    vector<int,4> r29 = func_vector(29);

    vector<int,4> r30 = func_vector_ref(30);
}

// CM vector and matrix types are (currently) limited to less than 4096 bytes - we generate a 
// helpful front-end error in order to avoid a more obscure error from the finalizer.
// RUN: %cmc -march=SKL -emit-llvm -ferror-limit=999 -Xclang -verify -Xclang -verify-ignore-unexpected -- %s

