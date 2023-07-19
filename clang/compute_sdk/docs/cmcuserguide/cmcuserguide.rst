.. ========================= begin_copyright_notice ============================

  Copyright (C) 2021-2023 Intel Corporation

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

2 Supported Gen Targets
=======================

By default the cmc compiler produces a GenISA/XeISA file that is targeted a specific
Gen/Xe variant.
Gen mostly used to specify obsolete platforms (before Iris Xe).
Xe used to specify Iris Xe+ and Arc platforms.

Some C for Metal language features are by their nature platform specific,
and are only available when a specific platform is specified.
For example, DPAS instructions are not available on all target platforms.

A specific target may be specified by use of the -march=\ *gen* compiler
option. This option implicitly predefines several macros that can be used
within C for Metal kernels to control conditional compilation, and also
includes appropriate header files.

The Gen/Xe target may be specified using a codename (e.g. TGLLP) or number (e.g. gen12).
The case is not significant, so tgllp, TGLLP, and Tgllp are all equivalent.

Some Xe targets have steppings: A0, B0, etc... Stepping is different GPU version
inside single Xe target. It is used to update GPU, fix HW bugs, perf improvements.

To specify stepping in close-to-metal way, revision id is used.
Option Qxcm_revid passes revision id, corresponding to stepping.

When a Gen/Xe target is specified, two macros are predefined.
The macro CM_GENX is given a value which identifies the target.
For Gen targets only, macro of the form CM_GEN\ *n* is defined (without a value).
For Xe targets special macros are predefined with Xe name, like CM_XEHP.
The targets supported by cmc, and the corresponding macros, are given in the table below.

========= ======== =========== ============= ===================
Gen/Xe    Name     Macro       CM_GENX value CM_GENX_REVID value
========= ======== =========== ============= ===================
GEN7_5    HSW      CM_GEN7_5   750           0
GEN8      BDW      CM_GEN8     800           0
GEN8_5    CHV      CM_GEN8_5   850           0
GEN9      SKL      CM_GEN9     900           0
..        BXT      CM_GEN9     920           0
GEN9_5    KBL      CM_GEN9_5   950           0
..        GLK      CM_GEN9_5   970           0
GEN11     ICLLP    CM_GEN11    1150          0
GEN12     TGLLP    CM_GEN12    1200          0
...       RKL      CM_GEN12    1201          0
...       DG1      CM_GEN12    1210          0
...       ADLP     CM_GEN12    1220          0
...       ADLS     CM_GEN12    1230          0
...       ADLN     CM_GEN12    1240          0
XEHP_SDV  XEHP_SDV CM_XEHP     1270          0
XeHPG     DG2      CM_XEHPG    1271          0
XeLPG     MTL      CM_XELPG    1275          0
XeHPC     PVC      CM_XEHPC    1280          0
...       PVCXT    CM_XEHPC    1280          5
========= ======== =========== ============= ===================


Also you may use CM_GENX_REVID to query revision id for given platform if
specified. Default revision id is 0.

Preferable way is to use CM_GENX and CM_GENX_REVID only

Macros like CM_GEN12, etc, will be deprecated soon and will probnably not
appear for new features.

3 C for Metal Header Files
==========================

The cmc compiler implicitly knows the path to the C for Metal header files, so
no option needs to be used to specify the path.

Only the main header file cm/cm.h needs to be included in C for Metal kernels
unless cmtl functions are required, in which case cm/cmtl.h should also be
included - all other header files will be included as needed, depending on the
Gen variant specified.

The include statement should appear in each C for Metal source file as follows:

``#include <cm/cm.h>``

or if C for Metal Template Library functions are required

``#include <cm/cm.h>``

``#include <cm/cmtl.h>``

4 Compiler Options
==================

The following table describes the main options that are useful for CM.  Note
that since cmc is based on LLVM/Clang, there are many other options that are
available. Many of these will not be applicable to C for Metal kernels or Gen
targets, and some may result in unexpected behavior.

Note that all C for Metal specific options (i.e. those starting with Qxcm or
mCM) may use a slash '/' or minus '-' as a prefix. Most Clang options only
accept minus '-'.

============================= ==============
Option                        Description
============================= ==============
-help                         Prints a list of compiler options - note that not
                              all options are applicable to CM.

-march=\ *gen*                Specifies the Gen/Xe target

                              This may use numeric or mnemonic notations, e.g.
                              gen8 and bdw both specify a Broadwell target.
                              Case is not significant.

                              The macros CM_GENX and CM_GEN<x> will be predefined
                              according to the target that is specified - e.g.
                              for BDW, CM_GENX will have a value of 800,
                              and CM_GEN8 will be defined (without a value).

-binary-format <value>        Sets in which format should be generated binary;
                              values: 'cm', 'ocl' or 'ze'

-binary-format=<value>        Alias for -binary-format <value>

-fcm-pointer                  Enables experimental pointer support in CM.

-fcmocl                       Alias for -binary-format=ze.

-femulate_i64                 Emulates all 64-bit integer operations.

-fvolatile-global             Treats global variables as volatile, do not promote them
                              to registers early.

-g                            Enable debug info generation.

-g<N>                         Enable debug info generation of given level. -g0 disables
                              debug info, -g1 enables line numbers, -g2 enables full
                              debug info.

-mCM_disable_jmpi             Disables jmpi (only available if -Qxcm_jit_target=... is
                              also specified).

-mCM_init_global              Always initialize CM global variables

-mCM_jit_option<value>        Passes specified value to the GenX Finalizer as an option.

-mCM_no_debug                 Disables debug info (line tables) when -g is not specified.

-mCM_no_vector_decomposition  Disables vector decomposition optimization.

-mCM_old_asm_name             Emits the kernel asm name in old style
                              (<filename>_<idx>.(visa)asm).

-mCM_printfargs               Prints arguments used for finalizer invocation.

-mCM_printregusage            Prints number of GRFs used by each kernel. Note that
                              local register allocation is turned off.
                              (only available if -Qxcm_jit_target=... is also specified)

-mCM_reverse_kernels          Emits the kernel asm name in reversed order.

-mCM_translate_legacy         Translates legacy intrinsics.

-mCM_warn_callable            Generates warning instead of error if callable is called
                              in the middle.

-mdump_asm                    Requests creation of assembly dumps for the
                              compiled kernels.
                              If <CM_FORCE_ASSEMBLY_DUMP> environment variable
                              is set then this option is enabled implicitly.
                              But it's recommended to use IGC_ShaderDumpEnable=1 instead
                              to get dumps.

-menableiga                   Enable IGA assembler syntax (only available if
                              -Qxcm_jit_target=... is also specified)

-Qxcm_jit_target<value>       Specifies the target architecture:
                              hsw | bdw | chv | skl | bxt | kbl | icl | tgllp
                              But it's recommended to use -march instead.

-Qxcm_opt_report              Prints GenX Finalizer optimization report
                              (only available if -Qxcm_jit_target=... is also specified).

-Qxcm_preschedule_ctrl<value> Passes the -presched-ctrl <ctrl> to the GenX Finalizer.

-Qxcm_preschedule_rp<value>   Passes the -presched-rp <rp> to the GenX Finalizer.

-Qxcm_print_asm_count         Prints gen instruction count for each kernel
                              (only available if -Qxcm_jit_target=... is also specified).

-Qxcm_release                 Strips debug information from generated .isa file

-Qxcm_revid                   Specifies revision id inside given target (use with march=)
                              Valid only for some platforms.

-Qxcm_register_file_size=<N>  Specifies number of registers to use for register allocation.
                              The values allowed with this option are 128, 256 and auto for
                              XeHP and further platforms. For pre-XeHP platforms 128 is the
                              only allowed value. Auto value enables compiler heuristics to
                              determine the number of registers.


-Qxcm_doubleGRF               Alias for ``-Qxcm_register_file_size=256``.

-vc-use-plain-2d-images       Treat "image2d_t" annotated surfaces as non-media 2D images.

-###                          This option causes the cmc driver to print the commands
                              that would be used to perform the compilation
                              (cmc front-end and Gen Finalizer commands).

============================= ==============

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

CM_GENX                        Defined whenever a specific Gen target has been
                               specified (-march option). See the table in
                               :ref:`SupportedGenTargets` for the value for each
                               target.

CM_\ *gen*                     Defined (without a value) when the corresponding
                               target has been specified, e.g. if the target is
                               specified to be SKL then CM_GEN9 will be defined. See
                               the table in :ref:`SupportedGenTargets` for the name
                               of the macro defined for each Gen target.

CM_HAS_LONG_LONG               Defined (with value 1) if the specifed target supports
                               the ``long long`` type.

CM_HAS_DOUBLE                  Defined (with value 1) if the specifed target supports
                               the ``double`` type.

CM_HAS_IEEE_DIV_SQRT           Defined (with value 1) if the specifed target supports
                               IEEE-compliant division and square root operations.

CM_HAS_VA                      Defined (with value 1) if BDW+ video analytics features
                               are available on the specified target.

CM_HAS_VA_PLUS                 Defined (with value 1) if SKL+ video analytics features
                               are available on the specified target.

CM_HAS_BIT_ROTATE              Defined (with value 1) if the specifed target supports
                               the ``cm_rol`` and ``cm_ror`` built-in functions.

CM_HAS_BIT_ROTATE_64BIT        Defined (with value 1) if the specifed target supports
                               64-bit integer data types for the ``cm_rol`` and
                               ``cm_ror`` built-in functions.

CM_HAS_DP4A                    Defined (with value 1) if the specifed target supports
                               the ``cm_dp4a`` built-in function.

CM_HAS_BFN                     Defined (with value 1) if the specifed target supports
                               the ``cm_bfn`` built-in function.

CM_HAS_BF16                    Defined (with value 1) if the specifed target supports
                               the BFloat16 data type and ``cm_bf_cvt`` built-in.

CM_HAS_TF32                    Defined (with value 1) if the specifed target supports
                               the TFloat32 data type and ``cm_tf32_cvt`` built-in.

CM_HAS_DPAS                    Defined (with value 1) if the specifed target supports
                               the ``cm_dpas`` built-in function.

CM_HAS_DPAS_INT2               Defined (with value 1) if the specifed target supports
                               the 2-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT4               Defined (with value 1) if the specifed target supports
                               the 4-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT8               Defined (with value 1) if the specifed target supports
                               the 8-bit integer data as sources for ``cm_dpas``
                               built-in function.

CM_HAS_DPAS_INT_MIX            Defined (with value 1) if the specifed target supports
                               the mix of integer data types as sources for
                               ``cm_dpas`` built-in function.

CM_HAS_DPAS_ACC_HALF           Defined (with value 1) if the specifed target supports
                               the ``half`` data type as an accumulator for the
                               ``cm_dpas`` built-in function.

CM_HAS_DPAS_ACC_BF16           Defined (with value 1) if the specifed target supports
                               the BFloat16 data type as an accumulator for the
                               ``cm_dpas`` built-in function.

CM_HAS_DPAS_ODD                DEPRECATED. Defined (with value 1) if the specifed
                               target supports odd values for as RepeatCount the
                               ``cm_dpas`` built-in function.

CM_HAS_DPASW                   Defined (with value 1) if the specifed target supports
                               the ``cm_dpasw`` built-in function.

CM_HAS_LSC                     Defined (with value 1) if the specifed target supports
                               LSC data port messages.

CM_HAS_UNTYPED_2D              Defined (with value 1) if the specifed target supports
                               Untyped 2D block LSC data port messages.

CM_HAS_SAMPLE_UNORM            Defined (with value 1) if the specifed target supports
                               media sample32 function.

CM_HAS_STOCHASTIC_ROUNDING     Defined (with value 1) if the specifed target supports
                               the ``cm_srnd`` built-in function.

CM_HAS_GATEWAY_EVENT           Defined (with value 1) if the specifed target supports
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

====================== ==================
Environment variable   Description
====================== ==================
ENABLE_IGA             By default the GenX finalizer uses the legacy assembler
                       syntax for the assembly files it generates for platforms
                       before Gen11. If the ENABLE_IGA environment variable has
                       a non-zero value then IGA assembler syntax will be used.
                       This is equivalent to specifying the -menableiga compiler
                       option.

CM_FORCE_ASSEMBLY_DUMP Enables "-mCM_old_asm_name -mdump_asm" options if set.

CM_INCLUDE_DIR         Directory with the include files.

IGC_ShaderDumpEnable=1 (default=0) causes all LLVM, assembly, and ISA code generated by
                       the CM compiler to be written to /tmp/IntelIGC/<application_name>.

IGC_DumpToCurrentDir=1 (default=0) writes all the files created by IGC_ShaderDumpEnable
                       to your current directory instead of /tmp/IntelIGC/<application_name>.
                       Since this is potentially a lot of files, it is recommended to create
                       a temporary directory just for the purpose of holding these files.

====================== ==================


7 Reporting Compiler Bugs
=========================

Like most compilers, cmc is a complex piece of software and may sometimes
encounter a condition that isn't currently accounted for. This may exhibit
in one of a number of ways - ranging from an internal error, a failed
compilation, or incorrect execution of the resulting kernel. Please submit
new issues to https://github.com/intel/cm-compiler/issues with all information
required to reproduce failures.
