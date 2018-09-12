/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: gen8_vme.h 26170 2011-10-29 00:48:24Z kchen24 $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Kaiyu Chen
***                      Stephen Thomas
***                      
***                      
***                      
***
*** Description: Cm VME APIs
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_GEN11_VME_H
#define CM_GEN11_VME_H

#include "gen9_vme.h" // Included to obtain legacy functionality
#include "cm_intrin.h"

typedef enum _VMEMsgTypeHEVC_
{
    VME_HEVC_SKIP_RDE_MSG    = 0,
    VME_HEVC_IME_MSG         = 1,
    VME_HEVC_INTER_RDE_MSG   = 2,
    VME_HEVC_INTRA_RDE_MSG   = 3,
    VME_HEVC_FME_MESH_MSG    = 4,
} VMEMsgTypeHEVC;

typedef enum _VMESFType_
{
    VME_SFID_VME      = 8,
    VME_SFID_CRE      = 13
} VMESFType;

// Pre-Gen10 functionality expressed using the new API convention

template <uint N1, uint N2>
_GENX_ inline extern void
cm_vme_ime(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, N1, 32> &IMEInput, 
           VMEStreamMode streamMode, VMESearchCtrl searchCtrl, uint curSurfIndex,
           vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 16> costCenter,
           matrix<uchar, N2, 32> &IMEOutput)
{
    run_vme_ime(UNIInput, IMEInput, streamMode, searchCtrl, curSurfIndex, ref0, ref1, costCenter, IMEOutput);
}

template <uint N1, uint N2>
_GENX_ inline extern void
cm_vme_ime(matrix_ref<uchar, 4, 32> &UNIInput, matrix_ref<uchar, N1, 32> &IMEInput, 
           VMEStreamMode streamMode, VMESearchCtrl searchCtrl, uint curSurfIndex,
           vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 16> costCenter,
           matrix_ref<uchar, N2, 32> &IMEOutput)
{
    run_vme_ime(UNIInput, IMEInput, streamMode, searchCtrl, curSurfIndex, ref0, ref1, costCenter, IMEOutput);
}

template <uint N1, uint N2>
_GENX_ inline extern void
cm_vme_ime(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, N1, 32> &IMEInput, 
           VMEStreamMode streamMode, VMESearchCtrl searchCtrl, SurfaceIndex curSurfIndex,
           vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 16> costCenter,
           matrix<uchar, N2, 32> &IMEOutput)
{
    run_vme_ime(UNIInput, IMEInput, streamMode, searchCtrl, cm_get_value(curSurfIndex),
                ref0, ref1, costCenter, IMEOutput);
}

template <uint N1, uint N2>
_GENX_ inline extern void
cm_vme_ime(matrix_ref<uchar, 4, 32> &UNIInput, matrix_ref<uchar, N1, 32> &IMEInput, 
           VMEStreamMode streamMode, VMESearchCtrl searchCtrl, SurfaceIndex curSurfIndex,
           vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 16> costCenter,
           matrix_ref<uchar, N2, 32> &IMEOutput)
{
    run_vme_ime(UNIInput, IMEInput, streamMode, searchCtrl, cm_get_value(curSurfIndex),
                ref0, ref1, costCenter, IMEOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &SICInput, 
           uint curSurfIndex, matrix<uchar, N, 32> &UNIOutput) 
{
    run_vme_sic(UNIInput, SICInput, curSurfIndex, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &SICInput, 
           SurfaceIndex curSurfIndex, matrix<uchar, N, 32> &UNIOutput)
{
    run_vme_sic(UNIInput, SICInput, cm_get_value(curSurfIndex), UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &SICInput, 
           SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_sic(UNIInput, SICInput, cm_get_value(curSurfIndex), UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> SICInput, 
           uint curSurfIndex, matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_sic(UNIInput, SICInput, curSurfIndex, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> SICInput, 
           SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_sic(UNIInput, SICInput, cm_get_value(curSurfIndex), UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_sic(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> SICInput, 
           SurfaceIndex curSurfIndex, matrix<uchar, N, 32> &UNIOutput)
{
    run_vme_sic(UNIInput, SICInput, cm_get_value(curSurfIndex), UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &FBRInput,
           uint curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix<uchar, N, 32> &UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, curSurfIndex, FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &FBRInput,
           SurfaceIndex curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix<uchar, N, 32> &UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, cm_get_value(curSurfIndex), FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 4, 32> &FBRInput,
           SurfaceIndex curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, cm_get_value(curSurfIndex), FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> FBRInput,
           uint curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, curSurfIndex, FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> FBRInput,
           SurfaceIndex curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix_ref<uchar, N, 32> UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, cm_get_value(curSurfIndex), FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_fbr(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 4, 32> FBRInput,
           SurfaceIndex curSurfIndex, uchar FBRMbMode,  uchar FBRSubMbShape, uchar FBRSubPredMode,
           matrix<uchar, N, 32> &UNIOutput)
{
    run_vme_fbr(UNIInput, FBRInput, cm_get_value(curSurfIndex), FBRMbMode, FBRSubMbShape, FBRSubPredMode, UNIOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 1, 32> &IDMInput, 
           uint curSurfIndex, matrix<uchar, N, 32> &IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, curSurfIndex, IDMOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 1, 32> &IDMInput, 
           SurfaceIndex curSurfIndex, matrix<uchar, N, 32> &IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, cm_get_value(curSurfIndex), IDMOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix<uchar, 4, 32> &UNIInput, matrix<uchar, 1, 32> &IDMInput, 
           SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, cm_get_value(curSurfIndex), IDMOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 1, 32> IDMInput, 
           uint curSurfIndex, matrix_ref<uchar, N, 32> IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, curSurfIndex, IDMOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 1, 32> IDMInput, 
           SurfaceIndex curSurfIndex, matrix_ref<uchar, N, 32> IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, cm_get_value(curSurfIndex), IDMOutput);
}

template <uint N>
_GENX_ inline extern void
cm_vme_idm(matrix_ref<uchar, 4, 32> UNIInput, matrix_ref<uchar, 1, 32> IDMInput, 
           SurfaceIndex curSurfIndex, matrix<uchar, N, 32> &IDMOutput)
{
    run_vme_idm(UNIInput, IDMInput, cm_get_value(curSurfIndex), IDMOutput);
}

// Gen11 HEVC-specific functions

// Macros for forming descriptor values for cm_send() and cm_sends()
// The macros use a functional style - they take a descriptor value and any
// required additional parameters and return an appropriate new descriptor
// value

// Set number of inputs (0..15) 
#define SEND_SET_Descriptor_Num_Inputs(v, i) ((v) | (((i) & 0xf) << 25))
// Set number of inputs (0..31) in Extended Descriptor
#define SEND_SET_Descriptor_Num_InputsExt(v, i) ((v) | (((i) & 0x1f) << 6))
// Set number of outputs (0..31)
#define SEND_SET_Descriptor_Num_Outputs(v, o) ((v) | (((o) & 0x1f) << 20))
#define SEND_SET_Descriptor_Header_Present(v) ((v) | (1 << 19))
#define SEND_SET_Descriptor_HEVC_Mode(v) ((v) | (1 << 18))
#define SEND_SET_Descriptor_StreamIn_Enabled(v) ((v) | (1 << 16))
// Set function, valid values in VMEMsgTypeHEVC
#define SEND_SET_Descriptor_Function(v, f) ((v) | (((f) & 0x7) << 8))

// Create a descriptor value from a surface index (0..255)
#define SEND_INIT_Descriptor_Surface(s) (cm_get_value(s) & 0xff)

// TODO: Once const variants of select_all() and cm_send() and cm_sends()
// are available, uncomment the const modifiers on the input payload parameters

#ifdef CM_EMU
// Prototypes for emulation functions
//
// TODO: Actual implementations of these functions do not exist yet, and will
// not until proper C-Model support for Gen11 HEVC VME is in place
CM_API extern void
cm_vme_hevc_skip_rde_implicit_gen11(/*const*/ matrix_ref<uchar, 10, 32> RDEInput,
                                    int curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput);

CM_API extern void
cm_vme_hevc_skip_rde_explicit_gen11(/*const*/ matrix_ref<uchar, 6, 32> RDEInput,
                                    int curSurfIndex, matrix_ref<uchar, 12, 32> RDEOutput);

CM_API extern void
cm_vme_hevc_ime_gen11(/*const*/ matrix_ref<uchar, 5, 32> IMEInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInput, 
                      int LengthStreamInput, int curSurfIndex, matrix_ref<uchar, 11, 32> StreamOutput);

CM_API extern void
cm_vme_hevc_inter_rde_implicit_gen11(/*const*/ matrix_ref<uchar, 10, 32> RDEInput,
                                     int curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput);

CM_API extern void
cm_vme_hevc_inter_rde_explicit_gen11(/*const*/ matrix_ref<uchar, 5, 32> RDEInput,
                                     int curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput);

CM_API extern void
cm_vme_hevc_intra_rde_gen11(/*const*/ matrix_ref<uchar, 14, 32> RDEInput, int LengthLCNPInput,
                            int curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput);

CM_API extern void
cm_vme_hevc_fme_mesh_gen11(/*const*/ matrix_ref<uchar, 4, 32> FMEInput,
                            int curSurfIndex, matrix_ref<uchar, 3, 32> FMEOutput);
#endif

#define selrows(r) select<r,1,32,1>(0,0).format<ushort,r,16>()

_GENX_ inline extern void
cm_vme_hevc_skip_rde_implicit(/*const*/ matrix_ref<uchar, 10, 32> RDEInput, SurfaceIndex curSurfIndex,
                              matrix_ref<uchar, 4, 32> RDEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_SKIP_RDE_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

    cm_send(RDEOutput.selrows(4), RDEInput.selrows(10), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_skip_rde_implicit_gen11(RDEInput, cm_get_value(curSurfIndex), RDEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_skip_rde_implicit(/*const*/ matrix<uchar, 10, 32> &RDEInput,
                              SurfaceIndex curSurfIndex, matrix<uchar, 4, 32> &RDEOutput)
{
    cm_vme_hevc_skip_rde_implicit(RDEInput.select_all(), curSurfIndex, RDEOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_skip_rde_explicit(/*const*/ matrix_ref<uchar, 6, 32> RDEInput,
                              SurfaceIndex curSurfIndex, matrix_ref<uchar, 12, 32> RDEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_SKIP_RDE_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

    cm_send(RDEOutput.selrows(12), RDEInput.selrows(6), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_skip_rde_explicit_gen11(RDEInput, cm_get_value(curSurfIndex), RDEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_skip_rde_explicit(/*const*/ matrix<uchar, 6, 32> &RDEInput,
                              SurfaceIndex curSurfIndex, matrix<uchar, 12, 32> &RDEOutput)
{
    cm_vme_hevc_skip_rde_explicit(RDEInput.select_all(), curSurfIndex, RDEOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_ime(/*const*/ matrix_ref<uchar, 5, 32> IMEInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInput, 
                int LengthStreamInput, SurfaceIndex curSurfIndex, matrix_ref<uchar, 11, 32> StreamOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, IMEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_IME_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, StreamOutput.n_rows());

    if (LengthStreamInput == 10) {
        Descriptor = SEND_SET_Descriptor_StreamIn_Enabled(Descriptor);
        U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_VME, StreamInput.n_rows());

        cm_sends(StreamOutput.selrows(11), IMEInput.selrows(5), StreamInput.selrows(10), ExDescriptor, Descriptor, 0);
    } else
        cm_send(StreamOutput.selrows(11), IMEInput.selrows(5), VME_SFID_VME, Descriptor, 0);
#else
    cm_vme_hevc_ime_gen11(IMEInput, StreamInput, LengthStreamInput, cm_get_value(curSurfIndex), StreamOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_ime(/*const*/ matrix<uchar, 5, 32> &IMEInput, /*const*/ matrix<uchar, 10, 32> &StreamInput, 
                int LengthStreamInput, SurfaceIndex curSurfIndex, matrix<uchar, 11, 32> &StreamOutput)
{
    cm_vme_hevc_ime(IMEInput.select_all(), StreamInput.select_all(), LengthStreamInput, curSurfIndex, StreamOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_inter_rde_implicit(/*const*/ matrix_ref<uchar, 10, 32> RDEInput,
                               SurfaceIndex curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_INTER_RDE_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

    cm_send(RDEOutput.selrows(4), RDEInput.selrows(10), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_inter_rde_implicit_gen11(RDEInput, cm_get_value(curSurfIndex), RDEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_inter_rde_implicit(/*const*/ matrix<uchar, 10, 32> &RDEInput,
                               SurfaceIndex curSurfIndex, matrix<uchar, 4, 32> &RDEOutput)
{
    cm_vme_hevc_inter_rde_implicit(RDEInput.select_all(), curSurfIndex, RDEOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_inter_rde_explicit(/*const*/ matrix_ref<uchar, 5, 32> RDEInput,
                               SurfaceIndex curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, RDEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_INTER_RDE_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

    cm_send(RDEOutput.selrows(4), RDEInput.selrows(5), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_inter_rde_explicit_gen11(RDEInput, cm_get_value(curSurfIndex), RDEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_inter_rde_explicit(/*const*/ matrix<uchar, 5, 32> &RDEInput,
                              SurfaceIndex curSurfIndex, matrix<uchar, 4, 32> &RDEOutput)
{
    cm_vme_hevc_inter_rde_explicit(RDEInput.select_all(), curSurfIndex, RDEOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_intra_rde(/*const*/ matrix_ref<uchar, 14, 32> RDEInput, int LengthLCNPInput,
                      SurfaceIndex curSurfIndex, matrix_ref<uchar, 4, 32> RDEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, 4 + LengthLCNPInput);
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_INTRA_RDE_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RDEOutput.n_rows());

    cm_send(RDEOutput.selrows(4), RDEInput.selrows(14), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_intra_rde_gen11(RDEInput, LengthLCNPInput, cm_get_value(curSurfIndex), RDEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_intra_rde(/*const*/ matrix<uchar, 14, 32> &RDEInput, int LengthLCNPInput,
                      SurfaceIndex curSurfIndex, matrix<uchar, 4, 32> &RDEOutput)
{
    cm_vme_hevc_intra_rde(RDEInput.select_all(), LengthLCNPInput, curSurfIndex, RDEOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_fme_mesh(/*const*/ matrix_ref<uchar, 4, 32> FMEInput,
                      SurfaceIndex curSurfIndex, matrix_ref<uchar, 3, 32> FMEOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, FMEInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_FME_MESH_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, FMEOutput.n_rows());

    cm_send(FMEOutput.selrows(3), FMEInput.selrows(4), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_fme_mesh_gen11(FMEInput, cm_get_value(curSurfIndex), FMEOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_fme_mesh(/*const*/ matrix<uchar, 4, 32> &FMEInput,
                      SurfaceIndex curSurfIndex, matrix<uchar, 3, 32> &FMEOutput)
{
    cm_vme_hevc_fme_mesh(FMEInput.select_all(), curSurfIndex, FMEOutput.select_all());
}

#endif /* CM_GEN11_VME_H */
