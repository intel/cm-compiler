.. ========================= begin_copyright_notice ============================

  Copyright (C) 2021-2025 Intel Corporation

  SPDX-License-Identifier: MIT

  =========================== end_copyright_notice =============================

==============
CMC User Guide
==============

.. contents:: Table of Contents
   :depth: 3

1 Introduction
==============

C for Metal package contains a C for Metal compiler called cmc, based on
LLVM with custom front-end changes and extensions for the C for Metal language
and a Gen back-end which is now part of IGC (Intel Graphics Compiler).

This document describes the use of the cmc compiler.

For information about the C for Metal language see the :title:`C for Metal Language Specification`.

.. highlight:: c

.. _SupportedGenTargets:

2 Supported Xe Targets
=======================

The cmc compiler produces a GenISA/XeISA file for a specific Xe target. 
Some C for Metal language features are inherently platform-specific and
are only available when a specific platform is specified. For example,
DPAS instructions are not available on all target platforms.

A specific target may be specified by use of the '-march' command line option,
whose parameter can be one of the following:
  - IP version - a dot separated a three-part number: major.minor.revision,
    e.g. 12.55.8
  - Device ID - a string id assigned when a target gets available to public, dg2-g10
  - Alias - a shorter string id, mapped to a specific IP version,
    e.g. dg2 (alias for 12.55.8)

See the table below for the targets supported by the compiler.


Supplying '-march' command line option implicitly defines several macros
that can be used within C for Metal kernels to identify the target,
control conditional compilation, and include the appropriate header files:

============================ ======== ===============================================
Macro                        Value    Description
============================ ======== ===============================================
__CM_INTEL_TARGET_MAJOR      Integer  Specifies the target platform major version
                                      (the first number in IP version, e.g. 12)
__CM_INTEL_TARGET_MINOR      Integer  Specifies the target platform minor version
                                      (the second number in IP version, e.g. 55)
__CM_INTEL_TARGET_REVISION   Integer  Specifies the target platform revision
                                      (the third number in IP version, e.g. 8)
CM_GENX                      Integer  Identifies the target platform in 4-digit
                                      format (e.g. 1271). *Deprecated*.
CM_GEN12                     N/A      Identifies the target platform. *Deprecated*
CM_XE{name}                  N/A      Identifies the target platform (e.g. CM_XELPG).
                                      *Deprecated*.
============================ ======== ===============================================

Please note that the combination of __CM_INTEL_TARGET_MAJOR, __CM_INTEL_TARGET_MINOR,
and __CM_INTEL_TARGET_REVISION uniquely identifies the target platform.

See the table below for the targets supported by the compiler.

Note: device IDs with '*' are subject to re-assign to another IP version
in the future compiler release.

.. include:: platforms.rst



3 C for Metal Header Files
==========================

The cmc compiler implicitly knows the path to the C for Metal header files, so
no option needs to be used to specify the path.

The main header file cm/cm.h is included implicitly so it does not have to be
included in C for Metal kernels anymore.

If cmtl functions are required, cm/cmtl.h must be included:

``#include <cm/cmtl.h>``

All other header files will be included as needed, depending on the
Gen variant specified.

4 Compiler Options
==================

The following table describes the main options that are useful for CM.  Note
that since cmc is based on LLVM/Clang, there are many other options that are
available. Many of these will not be applicable to C for Metal kernels or Gen
targets, and some may result in unexpected behavior.

Note that all C for Metal specific options (i.e. those starting with Qxcm or
mCM) may use a slash '/' or minus '-' as a prefix. Most Clang options only
accept minus '-'.

============================= =================================================
Option                        Description
============================= =================================================
-help                         Prints a list of compiler options - note that not
                              all options are applicable to CM.

-march=<target>               Specifies the Xe target

-binary-format <value>        Sets in which format should be generated binary;
                              values: 'cm', 'ocl' or 'ze'

-binary-format=<value>        Alias for -binary-format <value>

-fvolatile-global             Treats global variables as volatile, do not
                              promote them to registers early.

-g                            Enable debug info generation.

-g<N>                         Enable debug info generation of given level. -g0
                              disables debug info, -g1 enables line numbers,
                              -g2 enables full debug info.

-mCM_disable_jmpi             Disables jmpi

-mCM_init_global              Always initialize CM global variables

-mCM_jit_option=<value>       Passes the option to the GenX Finalizer.

-mCM_no_debug                 Disables debug info (line tables) when -g is not
                              specified.

-mCM_no_vector_decomposition  Disables vector decomposition optimization.

-mCM_old_asm_name             Emits the kernel asm name in old style
                              (``<filename>_<idx>.(visa)asm``).

-mCM_printfargs               Prints arguments used for finalizer invocation.

-mCM_printregusage            Prints number of GRFs used by each kernel. Note
                              that local register allocation is turned off.

-mCM_reverse_kernels          Emits the kernel asm name in reversed order.

-mCM_translate_legacy         Translates legacy intrinsics.

-mCM_warn_callable            Generates warning instead of error if callable is
                              called in the middle.

-mCM_collect_cost_info        Enable loop cost information gathering.

-mdump_asm                    Requests creation of assembly dumps for the
                              compiled kernels. It's recommended to use shader
                              dumps instead of this option.

-menableiga                   Enable IGA assembler syntax.

-Qxcm_opt_report              Prints GenX Finalizer optimization report.

-Qxcm_preschedule_ctrl<value> Passes the -presched-ctrl <ctrl> to the Finalizer.

-Qxcm_preschedule_rp<value>   Passes the -presched-rp <rp> to the Finalizer.

-Qxcm_print_asm_count         Prints gen instruction count for each kernel.

-Qxcm_release                 Strips debug information from generated .isa file

-Qxcm_register_file_size=<N>  Specifies number of registers to use for register
                              allocation. The values allowed with this option
                              are *128*, *256* and *auto* for XeHP and further
                              platforms. For pre-XeHP platforms *128* is the
                              only allowed value. Auto value enables compiler
                              heuristics to determine the number of registers.


                              The default value is *128*.

-Qxcm_doubleGRF               Alias for ``-Qxcm_register_file_size=256``.

-vc-use-plain-2d-images       Treat "image2d_t" annotated surfaces as non-media
                              2D images.

-vc-use-bindless-buffers      Enable bindless buffer access.

-vc-use-bindless-images       Enable bindless image access.

-###                          This option causes the cmc driver to print the
                              commands that would be used to perform the
                              compilation.

============================= =================================================

5 Implicit Macros
=================

A number of macros are predefined by cmc which may be used to control
conditional compilation within C for Metal kernels. These are described in the
following table.

============================== =======================================================
Macro                          Description
============================== =======================================================
__CM                           Always defined (without a value) to indicate that this
                               is a C for Metal compilation.

__CMC                          Always defined (without a value) to indicate that the
                               compiler is cmc.

CM_HAS_LONG_LONG               Defined (with value 1) if the specified target supports
                               the ``long long`` type.

CM_HAS_DOUBLE                  Defined (with value 1) if the specified target supports
                               the ``double`` type.

CM_HAS_IEEE_DIV_SQRT           Defined (with value 1) if the specified target supports
                               IEEE-compliant division and square root operations.

CM_HAS_BIT_ROTATE              Defined (with value 1) if the specified target supports
                               the ``cm_rol`` and ``cm_ror`` built-in functions.

CM_HAS_BIT_ROTATE_64BIT        Defined (with value 1) if the specified target supports
                               64-bit integer data types for the ``cm_rol`` and
                               ``cm_ror`` built-in functions.

CM_HAS_DP4A                    Defined (with value 1) if the specified target supports
                               the ``cm_dp4a`` built-in function.

CM_HAS_BFN                     Defined (with value 1) if the specified target supports
                               the ``cm_bfn`` built-in function.

CM_HAS_BF8                     Defined (with value 1) if the specified target supports
                               the BFloat8 data type and ``cm_bf8_cvt`` built-in.

CM_HAS_HF8                     Defined (with value 1) if the specified target supports
                               the HFloat8 data type and ``cm_hf8_cvt`` built-in.

CM_HAS_BF16                    Defined (with value 1) if the specified target supports
                               the BFloat16 data type and ``cm_bf_cvt`` built-in.

CM_HAS_TF32                    Defined (with value 1) if the specified target supports
                               the TFloat32 data type and ``cm_tf32_cvt`` built-in.

CM_HAS_DPAS                    Defined (with value 1) if the specified target supports
                               the ``cm_dpas`` built-in function.

CM_HAS_DPAS_INT2               Defined (with value 1) if the specified target supports
                               the 2-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT4               Defined (with value 1) if the specified target supports
                               the 4-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT8               Defined (with value 1) if the specified target supports
                               the 8-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT_MIX            Defined (with value 1) if the specified target supports
                               the mix of integer data types as sources for
                               ``cm_dpas`` built-in function.

CM_HAS_DPAS_ACC_HALF           Defined (with value 1) if the specified target supports
                               the ``half`` data type as an accumulator for the
                               ``cm_dpas`` built-in function.

CM_HAS_DPAS_ACC_BF16           Defined (with value 1) if the specified target supports
                               the BFloat16 data type as an accumulator for the
                               ``cm_dpas`` built-in function.

CM_HAS_DPASW                   Defined (with value 1) if the specified target supports
                               the ``cm_dpasw`` built-in function.

CM_HAS_LSC                     Defined (with value 1) if the specified target supports
                               LSC data port messages.

CM_HAS_UNTYPED_2D              Defined (with value 1) if the specified target supports
                               Untyped 2D block LSC data port messages.

CM_HAS_SAMPLE_UNORM            Defined (with value 1) if the specified target supports
                               media sample32 function.

CM_HAS_SRND_FP16_TO_BF8        Defined (with value 1) if the specified target supports
                               operation for converting ``half`` type values into
                               ``bfloat8`` with stochastic rounding.

CM_HAS_STOCHASTIC_ROUNDING     Defined (with value 1) if the specified target supports
                               the ``cm_srnd`` built-in function.

CM_HAS_GATEWAY_EVENT           Defined (with value 1) if the specified target supports
                               the gateway event functions.

CM_HAS_LSC_LOAD_L1RI_L3CA_HINT Defined (with value 1) if the specified target supports
                               L1 "read invalidate" and L3 "cached" cache hints
                               combination.

CM_HAS_LSC_SYS_FENCE           Defined (with value 1) if the specified target supports
                               system fence messages.

CM_MAX_SLM_SIZE                Maximum shared local memory per group.

============================== =======================================================


6 Environment Variables
=======================

====================== =========================================================
Environment variable   Description
====================== =========================================================
ENABLE_IGA             By default the GenX finalizer uses the legacy assembler
                       syntax for the assembly files it generates for platforms
                       before Gen11. If the ENABLE_IGA environment variable has
                       a non-zero value then IGA assembler syntax will be used.
                       This is equivalent to specifying the -menableiga compiler
                       option.

CM_FORCE_ASSEMBLY_DUMP Enables "-mCM_old_asm_name -mdump_asm" options if set.

CM_INCLUDE_DIR         Directory with the include files.

IGC_ShaderDumpEnable=1 (default=0) causes all LLVM, assembly, and ISA code
                       generated by the CM compiler to be written to
                       ``/tmp/IntelIGC/<application_name>``.

IGC_DumpToCurrentDir=1 (default=0) writes all the files created by
                       ``IGC_ShaderDumpEnable`` to your current directory
                       instead of ``/tmp/IntelIGC/<application_name>``.

====================== =========================================================


7 Reporting Compiler Bugs
=========================

Like most compilers, cmc is a complex piece of software and may sometimes
encounter a condition that isn't currently accounted for. This may exhibit
in one of a number of ways - ranging from an internal error, a failed
compilation, or incorrect execution of the resulting kernel. Please submit
new issues to https://github.com/intel/cm-compiler/issues with all information
required to reproduce failures.
