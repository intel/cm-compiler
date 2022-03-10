/*========================== begin_copyright_notice ============================

Copyright (C) 2015-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file defines the options accepted by cmc

def cm_Group : OptionGroup<"<cmc options>">,
  HelpText<"CM SPECIFIC OPTIONS">;

def cm_compile_Group : OptionGroup<"<cmc compile-only options>">,
  Group<cm_Group>;

def cm_ignored_Group : OptionGroup<"<cmc ignored options>">,
  Group<cm_Group>;

class CMFlag<string name> : Option<["-", "/"], name, KIND_FLAG>,
  Group<cm_Group>, Flags<[CMOption, DriverOption, NoArgumentUnused]>;

class CMCompileFlag<string name> : Option<["-", "/"], name, KIND_FLAG>,
  Group<cm_compile_Group>,
  Flags<[CMOption, DriverOption, NoArgumentUnused]>;

class CMIgnoredFlag<string name> : Option<["-", "/"], name, KIND_FLAG>,
  Group<cm_ignored_Group>,
  Flags<[CMOption, DriverOption, HelpHidden, NoArgumentUnused]>;

class CMJoined<string name> : Option<["-", "/"], name, KIND_JOINED>,
  Group<cm_Group>, Flags<[CMOption, DriverOption, NoArgumentUnused]>;

class CMIgnoredJoined<string name> : Option<["-", "/"], name, KIND_JOINED>,
  Group<cm_ignored_Group>,
  Flags<[CMOption, DriverOption, HelpHidden, NoArgumentUnused]>;

class CMCC1<string name> : Option<["-"], name, KIND_JOINED_OR_SEPARATE>,
  Group<cm_Group>, Flags<[DriverOption, CC1Option]>;

class CMCC1Joined<string name> : Option<["-", "/"], name, KIND_JOINED>,
  Group<cm_Group>, Flags<[CMOption, CC1Option, DriverOption, NoArgumentUnused]>;

// CM specific options

def mCM_emit_common_isa : CMCompileFlag<"mCM_emit_common_isa">,
  HelpText<"emit common isa (option ignored)">;

def mCM_genx_assembler : CMJoined<"mCM_genx_assembler">,
  HelpText<"specify the genx assembler to use">;

def mCM_jit_option : CMJoined<"mCM_jit_option">,
  HelpText<"pass specified value to the GenX Finalizer as an option">;
def Qxcm_jit_option : CMJoined<"Qxcm_jit_option">, Alias<mCM_jit_option>;

// Qxcm is special in that it can take an optional value.
// We represent it as a flag for the case without a value, and as expecting
// a joined value if it is "Qxcm=" or "Qxcm:".
def Qxcm : CMCompileFlag<"Qxcm">,
  HelpText<"specify MDF CM mode (default)">;
def Qxcm_EQUALS : CMIgnoredJoined<"Qxcm=">;
def Qxcm_COLON : CMIgnoredJoined<"Qxcm:">;

def Qxcm_asm_output : CMIgnoredJoined<"Qxcm_asm_output">;

def Qxcm_B : CMIgnoredFlag<"Qxcm_B">;

def Qxcm_conform_mul : CMIgnoredFlag<"Qxcm_conform_mul">;

def Qxcm_emu : CMIgnoredFlag<"Qxcm_emu">,
  HelpText<"compile in emulation mode (currently unsupported)">;

def Qxcm_forcefirstfit : CMIgnoredFlag<"Qxcm_forcefirstfit">;

def Qxcm_inst_compaction : CMIgnoredFlag<"Qxcm_inst_compaction">;

def Qxcm_jit_target : CMJoined<"Qxcm_jit_target">,
  HelpText<"specify the target architecture: hsw | bdw | chv | skl | bxt | kbl | cnl | icl">;

def Qxcm_no_cr_fix : CMIgnoredFlag<"Qxcm_no_cr_fix">;

def Qxcm_no_du_fix : CMIgnoredFlag<"Qxcm_no_du_fix">;

def Qxcm_noconv : CMIgnoredFlag<"Qxcm_noconv">;

def Qxcm_noddoff : CMIgnoredFlag<"Qxcm_noddoff">;

def Qxcm_nofoldaddrimmed : CMIgnoredFlag<"Qxcm_nofoldaddrimmed">;

def Qxcm_noipa : CMIgnoredFlag<"Qxcm_noipa">;

def Qxcm_nonstrict : CMIgnoredFlag<"Qxcm_nonstrict">;

def Qxcm_noschedule : CMIgnoredFlag<"Qxcm_noschedule">,
  HelpText<"pass the -noschedule flag to the GenX Finalizer">;

def Qxcm_preschedule : CMIgnoredFlag<"Qxcm_preschedule">,
  HelpText<"pass the -presched flag to the GenX Finalizer">;

def Qxcm_preschedule_ctrl : CMJoined<"Qxcm_preschedule_ctrl">,
  HelpText<"pass the -presched-ctrl <ctrl> to the GenX Finalizer">;

def Qxcm_preschedule_rp : CMJoined<"Qxcm_preschedule_rp">,
  HelpText<"pass the -presched-rp <rp> to the GenX Finalizer">;

def Qxcm_opt_report : CMCompileFlag<"Qxcm_opt_report">,
  HelpText<"prints GenX Finalizer optimization report (only available if -Qxcm_jit_target=... is also specified)">;

def Qxcm_print_asm_count : CMCompileFlag<"Qxcm_print_asm_count">,
  HelpText<"prints gen instruction count for each kernel (only available if -Qxcm_jit_target=... is also specified)">;

def mCM_printregusage : CMCompileFlag<"mCM_printregusage">,
  HelpText<"prints number of GRFs used by each kernel. Note that local register allocation is turned off. (only available if -Qxcm_jit_target=... is also specified)">;

def mCM_printfargs : CMCompileFlag<"mCM_printfargs">,
  HelpText<"prints arguments used for finalizer invocation">;

def Qxcm_release : CMCompileFlag<"Qxcm_release">,
  HelpText<"strip debug information from generated .isa file">;

def Qxcm_S : CMIgnoredFlag<"Qxcm_S">;

def Qxcm_symbolreg : CMIgnoredFlag<"Qxcm_symbolreg">;

def mCM_unique_labels : CMIgnoredJoined<"mCM_unique_labels">;

def Qxcm_vme_arch : CMJoined<"Qxcm_vme_arch">,
  HelpText<"specify the target vme architecture: gen7_5 | gen8 | gen9 | gen9_5 | gen10 | gen11">;

def Qxdisable_clang_cm : CMIgnoredFlag<"Qxdisable_clang_cm">;

def Qxno_clang_cm_diag : CMIgnoredFlag<"Qxno_clang_cm_diag">;

def menableiga : CMCompileFlag<"menableiga">, HelpText<"enable IGA assembler syntax (only available if -Qxcm_jit_target=... is also specified)">;
def mCM_enableiga : CMCompileFlag<"mCM_enableiga">;

def mCM_no_input_reorder : CMCompileFlag<"mCM_no_input_reorder">, HelpText<"Stop the compiler from reordering kernel arguments">;

def binary_format : Separate<["-"], "binary-format">, Group<cm_Group>,
  HelpText<"Set in which format should be generated binary; values: 'cm', 'ocl' or 'ze'">,
  Flags<[CMOption, CC1Option]>;

def binary_format_eq : Joined<["-"], "binary-format=">, Group<cm_Group>,
  HelpText<"Alias for -binary-format <value>">, Alias<binary_format>, Flags<[CMOption, CC1Option]>;

def fcmocl : Flag<["-", "/"], "fcmocl">, Group<cm_Group>,
  HelpText<"Alias for -binary-format=ocl">,
  Alias<binary_format>, AliasArgs<["ocl"]>, Flags<[CMOption, CC1Option]>;

def mCM_no_vector_decomposition : CMCompileFlag<"mCM_no_vector_decomposition">, HelpText<"Disable vector decomposition optimization">;

def mCM_disable_jmpi : CMCompileFlag<"mCM_disable_jmpi">, HelpText<"disable jmpi (only available if -Qxcm_jit_target=... is also specified)">;

def mCM_no_debug : CMCompileFlag<"mCM_no_debug">, HelpText<"Disable debug info (line tables) when -g is not specified.">;

def mCM_warn_callable : CMCompileFlag<"mCM_warn_callable">, HelpText<"Generate warning instead of error if callable is called in the middle.">;

def mCM_translate_legacy : CMCompileFlag<"mCM_translate_legacy">, HelpText<"Translate legacy intrinsics.">;

// Passed to cc1 to specify the file name for importing builtin-function module
def mCM_import_bif : CMCC1<"mCM_import_bif">;

def fvolatile_global : Flag<["-", "/"], "fvolatile-global">, Group<cm_Group>,
  HelpText<"treat global variables as volatile, not to promote them to register early">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def mCM_init_global : Flag<["-", "/"], "mCM_init_global">, Group<cm_Group>,
  HelpText<"Always initialize CM global variables">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def mCM_max_slm : CMCC1<"mCM_max_slm">,
  HelpText<"Maximum SLM size in kilobytes (base is target dependent)">;

def mCM_max_obr : CMCC1<"mCM_max_obr">, 
  HelpText<"Maximum owords per vector (base is target dependent)">;

def mCM_iefbypass : Flag<["-", "/"], "mCM_iefbypass">, Group<cm_Group>,
  HelpText<"Switch on IEF bypass (base is target dependent)">,
  Flags<[CMOption, CC1Option]>;

def mCM_reverse_kernels : Flag<["-", "/"], "mCM_reverse_kernels">, Group<cm_Group>,
  HelpText<"Emit the kernel asm name in reversed order">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def mCM_enable_stackcalls : Flag<["-", "/"], "mCM_enable_stackcalls">, Group<cm_Group>,
  HelpText<"Enable stack calls by default for all non-main kernel functions">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def mCM_optimize_none : Flag<["-", "/"], "mCM_optimize_none">, Group<cm_Group>,
  Flags<[CC1Option]>;

def s : Separate<["-"], "s">, Group<cm_Group>,
  HelpText<"Set (override) internal temporary source code file name 'src.cm' for online compilation">,
  Flags<[CMOption]>;

def s_eq : Joined<["-"], "s=">, Group<cm_Group>,
  HelpText<"Alias for -s <value>">, Alias<s>, Flags<[CMOption]>;

def cm_src : Separate<["-"], "cm-src">, Group<cm_Group>,
  HelpText<"Alias for -s <value>">, Alias<s>, Flags<[CMOption]>;

def cm_src_eq : Joined<["-"], "cm-src=">, Group<cm_Group>,
  HelpText<"Alias for -cm-src <value>">, Alias<s>, Flags<[CMOption]>;

// Passed to cc1 to specify the GenX assembly file base name
def cm_asm_name : CMCC1<"cm-asm-name">;

def mCM_old_asm_name : Flag<["-", "/"], "mCM_old_asm_name">, Group<cm_Group>,
  HelpText<"Emit the kernel asm name in old style (<filename>_<idx>.(visa)asm)">,
  Flags<[CMOption, CC1Option]>;

def fcm_pointer : Flag<["-", "/"], "fcm-pointer">, Group<cm_Group>,
  HelpText<"enable experimental pointer support in CM">,
  Flags<[CMOption, CC1Option]>;

def mCM_enforce_disable_free : Flag<["-", "/"], "mCM_enforce_disable_free">, Group<cm_Group>,
  HelpText<"Force -disable-free to take effect, unless this is passed, the -disable-free is ignored.">,
  Flags<[CC1Option]>;

def fvldst : CMIgnoredFlag<"fvldst">;
def fno_vldst : CMIgnoredFlag<"fno-vldst">;

def mdump_asm : CMFlag<"mdump_asm">,
  HelpText<"Enable genx backend to dump visa and genisa assembly "
           "files corresponding to the processed kernels">;

def show_stats : Flag<["-"], "show-stats">, Group<cm_Group>,
  HelpText<"Show LLVM statistics counters">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def dump_stats : Joined<["-"], "dump-stats=">, Group<cm_Group>,
  HelpText<"Dump LLVM statistics in JSON file">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def dump_stats_cwd : Flag<["-"], "dump-stats">, Group<cm_Group>,
  HelpText<"Alias for -dump-stats=cwd">, Alias<dump_stats>, AliasArgs<["cwd"]>,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def Qxcm_doubleGRF : CMIgnoredFlag<"Qxcm_doubleGRF">,
  HelpText<"increase total GRF number to 256">;

def Qxcm_stepping: CMJoined<"Qxcm_stepping">,
  HelpText<"Generate code taking platform stepping into account. "
           "Valid only for some platforms">;

def Qxcm_revid: CMCC1Joined<"Qxcm_revid=">,
  HelpText<"Pass specified revision ID to libocloc "
           "Valid only for some platforms">;

def femulate_i64 : CMIgnoredFlag<"femulate_i64">,
  HelpText<"emulate all 64-bit operations on *lp devices (enabled)">;

def mCM_no_emulate_i64 : Flag<["-", "/"], "mCM_no_emulate_i64">, Group<cm_Group>,
  HelpText<"disable 64-bit operations emulation">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def cm_printf_spec : Separate<["-"], "cm-printf-spec">, Group<cm_ignored_Group>,
  Flags<[CMOption, CC1Option, HelpHidden]>;

def cm_printf_spec_eq : Joined<["-"], "cm-printf-spec=">, Group<cm_ignored_Group>,
  Flags<[CMOption, CC1Option, HelpHidden]>;

def vc_use_plain_2d_images : Flag<["-"], "vc-use-plain-2d-images">,
  Group<cm_Group>, HelpText<"Treat image2d_t annotations as non-media images">,
  Flags<[CMOption, CC1Option]>;

def cm_disable_strong_inline : Flag<["-"], "cm_disable_strong_inline">,
  Group<cm_Group>, HelpText<"Disable treating of <inline> directives as "
                            "<always_inline>">,
  Flags<[CMOption, CC1Option, HelpHidden]>;

// Catch-alls for a range of options - options starting with these strings will
// be silently ignored, whether preceded by '-' or '/'.
// Note: as options starting with a '/' could be a Linux file path we need to
// be careful to make the prefix as specific as possible - to avoid ignoring
// source files - while still catching all the options we wish to ignore!
// This list is unlikely to be complete, but should catch most commonly used
// icl options.
def mCM : CMIgnoredJoined<"mCM">;
def mGLOB_ : CMIgnoredJoined<"mGLOB_">;
def mP2OPT_ : CMIgnoredJoined<"mP2OPT_">;
def mPAROPT_ : CMIgnoredJoined<"mPAROPT_">;
def Qopt : CMIgnoredJoined<"Qopt">;
