.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

=========================
C for Metal porting guide
=========================

Version 1, 2015-11-25

1 Introduction
==============

MDF 6.0 contains a new Cm compiler called cmc, based on LLVM with custom front-end changes and
extensions for the Cm language and a new Gen back-end. The MDF 5.0 compiler (cm-icl) is still
available and is included in the package but will no longer be developed. New features will be
available in cmc.

When developers move from using the current icl based cm compiler to the new llvm based cmc compiler
there are some issues that may be encountered. This document attempts to outline some of these
issues and what might be required when moving across to the new compiler.

The intent with the new cmc compiler is that it is a drop in replacement for the icl
compiler. However, in some cases decisions were made to tighten the adherence to the C for Metal Language
Specification (in others, the spec has been relaxed to allow for greater backwards
compatibility). In other areas implementation difficulties meant that some slight changes were
introduced.

All in all, in most cases porting an existing kernel to the new cmc compiler should be a relatively
quick process. This guide outlines the main ones we are currently aware of.

The language compiled by cmc is *almost* the same as the language compiled by cm-icl, but some
changes have been required due to:

* the way LLVM works
* the way we have implemented things
* a deliberate decision to consider a feature obsolete or unnecessary

.. contents::
   :local:
   :depth: 3

.. highlight:: c

2 Coding practices
==================

These outline some good coding practices that are not essential when using cmc (or cm-icl) but are
good practice nonetheless. These have been noticed during porting exercises for sample kernels and
are worth highlighting here.

2.1 Header files
----------------

cmc (and cm-icl for that matter) only requires kernels to include ``cm/cm.h`` to have all language
features and APIs available, provided an appropriate target is specified

* Manually including ``genx_vme.h``, ``genx_sample.h`` or *worse* a header for a specific Gen target,
  for example, is not necessary or supported.
* Specifying the target on the command line will get you all the support you need. Do not do anything
  else.

cmc does not support (and will fault) attempts to include standard C++ header files such as
``math.h``, ``limits.h``, etc. (cm-icl should never have allowed it either, but in some cases it
worked - as long as functions that are used from these headers are actually defined as macros or
template functions).
The main utility of this seems to have been wanting things like ``MAXINT`` or ``abs()``. Either use
proper Cm language features (``cm_abs()``) or defined the symbols yourself.

2.2 Builtin enum types
----------------------

C for Metal provides a number of builtin enum types that are used to specify modes or attributes for many
builtin functions, e.g. ChannelMaskType used in memory operations.

With cm-icl these were compatible with int values, and there was no compile time checking to
detect the use of a constant from one enum where another enum type was expected. The programmer
was expected to ensure that the correct types were used.

With cmc, these enum types are now distinct types and are no longer compatible with int or each other.
This means that the use of an inappropriate enum constant will result in a compile time error diagnostic.
This extra level of type safety detects many situations that could otherwise result in a latent bug.

3 Command-line options
======================

There has been considerable effort put into making sure that cmc recognizes the same options that
cm-icl recognizes, and that they do the same thing. There are some differences:

* ``/mGLOB_override_limits``: Recognized but has no effect. It is not necessary on cmc.
* ``/Qxcm_emu``: Not supported
* ``/Qxno_clang_cm_diag``: Recognized but ignored


4 Deprecated features
=====================

4.1 genx_select
---------------

``genx_select()`` **is no longer supported** and cmc will report an error if it is used::

  VABDW_MinMaxFilter_genx.cpp(57,25) : error: genx_select() is deprecated - use replicate()

          Min.row(0) = Writeback.genx_select<8, 4, 2, 1>(0,0);

However, the error message tells you what you can do. Every use of ``genx_select()`` can be replace
with ``replicate()``. The method prototypes are the same too, it should just be a purely textual
substitution.

4.2 cm_label
------------

``cm_label()`` **is deprecated**, although the compiler will only generate a warning if it is used
rather than an error. Otherwise, its presence has no effect in a kernel compiled by cmc.

4.3 int *n* types
-----------------

cm_icl defined ``int1, int2, int4`` (and corresponding unsigned types) if genx_vme.h was included
(which it is by default).

**These are no longer supported in cmc**. Cm has always defined the size of the standard types and
the decision was made to deliberately omit these extra types from the headers as these are considered
ambiguous and error-prone.

* ``char`` and ``uchar``: 8 bits; use these instead of ``int1`` or ``uint1``
* ``short`` and ``ushort``: 16 bits; use these instead of ``int2`` or ``uint2``
* ``int`` and ``uint``: 32 bits; use these instead of ``int4`` or ``uint4``
* ``long long`` and ``unsigned long long`` : 64 bits; use these instead of ``int8`` or ``uint8``

It is usually pretty trivial to search/replace all instances of these types, but if you must have
``intn`` or ``uintn`` types you should put appropriate typedefs at the head of your kernel source.

4.4 write_atomic
----------------

The **write-with-atomic-operation interface is deprecated**. Code of the form::

  write(pOutputIndex1, ATOMIC_INC, 0, idx_even, NULL, x);

will result in a warning, although it will still work. You should replace these calls with the new
form which looks as follows::

  write_atomic<ATOMIC_INC>(pOutputIndex1, idx_even, x);

Note that this new form will not work on the cm-icl compiler.

4.5 CM_STATIC_BUFFER_n
----------------------

cm_icl defined various ``CM_STATIC_BUFFER_n`` macros to refer to a number of global
reserved surface indexes. **These are not defined in cmc.** Instead, cmc supports
``SurfaceIndex`` constant values with numeric literal initializers (which represent
specific binding table indices), which allows the same sort of functionality in a
more flexible way (but remember to use ``SetSurfaceBTI()`` in the application host
code to associate a surface with the index). If you have a substantial kernel that uses these
macros and you want to minimize the number of edits you can add the following code near the
start of your kernel::

  #ifdef __CMC
  // Macro definitions only used on cmc. cm-icl already has them built into the
  // compiler.
  #if CM_GENX < 900
  // BDW and earlier
  #define CM_STATIC_BUFFER_0 (SurfaceIndex)0xf3
  #define CM_STATIC_BUFFER_1 (SurfaceIndex)0xf4
  #define CM_STATIC_BUFFER_2 (SurfaceIndex)0xf5
  #define CM_STATIC_BUFFER_3 (SurfaceIndex)0xf6
  #else
  // SKL+
  #define CM_STATIC_BUFFER_0 (SurfaceIndex)1
  #define CM_STATIC_BUFFER_1 (SurfaceIndex)2
  #define CM_STATIC_BUFFER_2 (SurfaceIndex)3
  #define CM_STATIC_BUFFER_3 (SurfaceIndex)4
  #endif
  #endif

4.6 #pragma cm_nonstrict
------------------------

The cmc compiler will implicitly use a narrower integer type for computation whenever that is
guaranteed not to change the result of the computation.

Therefore, the ``cm_nonstrict`` pragma is deprecated by the cmc compiler and a warning diagnostic
will be generated if it is used - it will be ignored, so will not affect the result of the compilation.

For situations where a narrower integer type is not guaranteed to produce the correct answer but the
data values are known to be such that incorrect results won't be generated, or where incorrect results
are acceptable, explicit casts can be added to force the desired integer width.

5 Relaxed requirements
======================

5.1 ``template`` keyword
------------------------

Use of the template keyword inside a template function for template member functions is no longer
required. The icl compiler sometimes required the addition of the ``template`` keyword to
disambiguate template methods. cmc removes this requirement. Consider the following example for
clarification:

.. code-block:: c
  :emphasize-lines: 5

  template <typename IndexType, typename ValueType>
  __GENX_MAIN_ int spmv_csr_kernel(...) {
    ...
    vector<ValueType, 32> v_dp4 = cm_dp4<float>(v_ax, v_x);
    v_y(i) += cm_sum<ValueType>(v_dp4.template select<8, 4>(0));
    ...
  }

In this case we have a member call on a type that is dependent on template parameters. The
``template`` keyword used to be required because clang-cm (a front-end used by cm-icl) complained
otherwise (it could not disambiguate). cmc, on the other hand, **no longer requires this** (but does
not complain if one is present).

6 Language changes
==================

There have been some cases where the language or intrinsics have been changed due to being unable to
support the feature properly in the new compiler. In some cases this evaluation has exposed
inconsistencies in the language spec and intrinsic definition and this has been an opportunity to
tighten the language appropriately.

6.1 Scalar auxiliary return (cm_div and cm_imul)
-------------------------------------------------

``cm_div()`` and ``cm_imul()`` both return two values, the normal function return and a value passed
back via reference in the first parameter.

Both these operations support scalar inputs and cm-icl allows the reference return parameter to be
scalar. **cmc does not support this**. Allowing scalar by-ref arguments in this case actually
violates the cm language spec. The decision was made to tidy up the ``cm_div`` and ``cm_imul``
implementation and specification to bring them in line.

Where you might have had::

  int q, r, a, b;
  q = cm_div<int>(r, a, b);

you must now say::

  int q, a, b;
  vector<int, 1> vr;
  q = cm_div<int>(vr, a, b);
  int r = vr(0);

This does not result in less efficient code.

6.2 Vectors and 1-dimensional matrices
--------------------------------------

Broadly speaking, cm-icl implicitly converts ``matrix<T, 1, N>`` or ``matrix<T, N, 1>`` types to
appropriate vector types if a vector operation is performed on a matrix value. Consider::

  r = m.column(2).select<1,1>(3)

is permitted in cm-icl, even though ``column()`` returns a ``matrix<T, N, 1>`` value. **cmc will
fault this**

Instead you can assign or initialize a vector from a matrix::

  vector<T, N> v = m.column(3);
  r = v.select<1,1>(3);

For the ``row()`` operation (which returns a ``matrix<T, M, N>`` type) a ``format()`` operation can
be used if M is 1.

6.3 Floating point rounding modes
---------------------------------

**cmc does not support cm_fsetround() or cm_fgetround()**. Instead you can apply a rounding mode to
a whole kernel::

  extern "C" _GENX_MAIN_ _GENX_ROUNDING_MODE_(CM_RTE) void
  k_rte(SurfaceIndex INBUF, SurfaceIndex OUTBUF)
  {
    matrix<float, 4, 8> in;
    matrix<float, 4, 8> out, tmp;
    // CmRoundingMode old_fpround;
    read(INBUF, 0, 0, in);

    // old_fpround = cm_fgetround();
    // cm_fsetround(CM_RTE);

    tmp = in;
    out = tmp * tmp;

    // cm_fsetround(old_fpround);
    write(OUTBUF, 0, 0, out);
  }

7 Tightened rules
=================

cmc attempts to enforce language constraints more effectively. The items listed in this section were
also a restriction for cm-icl but were not necessarily enforced by the compiler. Sometimes these
things will work, sometimes they will not. cmc attempts to catch violations at compilation instead.

7.1 Horizontal stride and horizontal size of 1
----------------------------------------------

The cm language requires the horizontal stride to be 1 when the horizontal size is 1::

  res = m.select<2,1,1,3>

  gives error: when select h_size is 1, the h_stride must be 1

7.2 Overlapping by-ref function arguments
-----------------------------------------

The C for Metal language spec explicitly states the following:

.. note::

  No pass-by-reference argument that is written to in
  a subprogram (either directly or transitively by means of a nested subprogram call pass-by-reference
  argument) may overlap with another pass-by-reference parameter or a global variable that is referenced
  in the subprogram; in addition no pass-by-reference subprogram argument that is referenced may
  overlap with a global variable that is written to in the subprogram.

This is to allow the compiler to implement copy-in / copy-out for function calls. The cmc compiler
can in some cases detect a violation of this rule and emit an error message. Clearly for some cases
where the address is not compile-time determined it cannot detect the problem and a runtime failure
may occur.

7.3 Replicate syntax more rigidly enforced
------------------------------------------

The cm language spec rules for replicate are more rigidly enforced by cmc. For instance, one of the
rules is that if only one template argument is specified, then no offset parameters are allowed (as
you are asking for the whole object to be replicated some number of times), even if the offset is
0::

  res = m.replicate<1>(0,0);

  gives error: replicate does not accept an offset when only the REP argument is specified

7.4 Goto into protected scope
-----------------------------

The cmc compiler will give additional errors when a goto statement is jumping to a label that jumps
over variable initializations that are still live at the target label. This is sensible as there is
a danger that the programmer might not realize that variable values are not valid through the goto
route. The solution to this is usually to move the declaration and initialization of the variable to
before the goto statement causing the error.

7.5 Stricter enforcement of intrinsic argument types
----------------------------------------------------

The cmc compiler is stricter in the way it checks the argument types to some intrinsics and built-in
functions. Read the error messages carefully and refer to the Language Spec to determine what they
need to be changed to.

This might seem to be an irritation but in actual fact the types used are often reflected in the
hardware implementation of the intrinsic and allowing different types may in some cases lead to
undefined or surprising results. The stricter adherence by the cmc compiler is an attempt to reduce
the scope for unintended behavior.

8 Features only supported in cmc
================================

cmc adds quite a few new MDF features that are only supported by cmc.

* 16 bit floating point types (``half``)
* 2D atomic writes (``write_typed_atomic()``)
* Hardware thread synchronization monitors
* Z-index support for ``local_size``, ``group_count``, ``local_id`` and ``group_id``
* Null pixel mask return for ``sample16()`` (and support for the ``3D_SAMPLE`` vISA instruction in
  general)


