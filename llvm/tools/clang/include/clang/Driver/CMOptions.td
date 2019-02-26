/*
 * Copyright (c) 2019, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

//===----------------------------------------------------------------------===//
//
//  This file defines the options accepted by cmc
//
//===----------------------------------------------------------------------===//

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

// CM specific options

def cm_svmptr : CMJoined<"cm_svmptr">,
  HelpText<"svmptr_t size in bits - 32 or 64">;

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
  HelpText<"specify the target architecture: hsw | bdw | chv | skl | bxt | glv | kbl | cnl | icl">;

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

def mCM_no_vector_decomposition : CMCompileFlag<"mCM_no_vector_decomposition">, HelpText<"Disable vector decomposition optimization">;

def mCM_disable_jmpi : CMCompileFlag<"mCM_disable_jmpi">, HelpText<"disable jmpi (only available if -Qxcm_jit_target=... is also specified)">;

def mCM_no_debug : CMCompileFlag<"mCM_no_debug">, HelpText<"Disable debug info (line tables) when -g is not specified.">;

def mCM_warn_callable : CMCompileFlag<"mCM_warn_callable">, HelpText<"Generate warning instead of error if callable is called in the middle.">;

def mCM_init_global : Flag<["-", "/"], "mCM_init_global">, Group<cm_Group>,
  HelpText<"Always initialize CM global variables">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def mCM_reverse_kernels : Flag<["-", "/"], "mCM_reverse_kernels">, Group<cm_Group>,
  HelpText<"Emit the kernel asm name in reversed order">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

// Passed to cc1 to specify the GenX assembly file base name
def cm_asm_name : CMCC1<"cm-asm-name">;

def fcm_pointer : Flag<["-", "/"], "fcm-pointer">, Group<cm_Group>,
  HelpText<"enable experimental pointer support in CM">,
  Flags<[CMOption, CC1Option]>;

def fvldst : Flag<["-", "/"], "fvldst">, Group<cm_Group>,
  HelpText<"enable experimental pass by reference codegen and optimization">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

def fno_vldst : Flag<["-", "/"], "fno-vldst">, Group<cm_Group>,
  HelpText<"disable experimental pass by reference codegen and optimization">,
  Flags<[CMOption, CC1Option, CC1AsOption]>;

// Options passed to GenX backend.
def mdump_regalloc : CMFlag<"mdump_regalloc">,
  HelpText<"dump register allocation information from the genx backend">;


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
