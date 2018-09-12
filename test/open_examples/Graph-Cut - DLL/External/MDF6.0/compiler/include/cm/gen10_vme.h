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

#ifndef CM_GEN10_VME_H
#define CM_GEN10_VME_H

#include "gen9_vme.h" // Included to obtain legacy functionality
#include "cm_intrin.h"

typedef enum _VMEMsgTypeHEVC_
{
    VME_HEVC_SIC_MSG    = 0,
    VME_HEVC_IME_MSG    = 1,
    VME_HEVC_FBR_MSG    = 2,
    VME_HEVC_HPM_MSG    = 3,
    VME_HEVC_SRM_MSG    = 4,
    VME_HEVC_RPM_MSG    = 5
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

// Gen10 HEVC-specific functions

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

CM_API extern void
cm_vme_hevc_ime_gen10(/*const*/ matrix_ref<uchar, 5, 32> IMEInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInput, 
                      int LengthStreamInput, int curSurfIndex, matrix_ref<uchar, 12, 32> StreamOutput);

CM_API extern void
cm_vme_hevc_sic_gen10(/*const*/ matrix_ref<uchar, 5, 32> SICInput, /*const*/ matrix_ref<uchar, 8, 32> NPInput, int LengthNP,
                      int curSurfIndex, matrix_ref<uchar, 22, 32> SICOutput);

CM_API extern void
cm_vme_hevc_sc_gen10(/*const*/ matrix_ref<uchar, 5, 32> SICInput, int curSurfIndex, matrix_ref<uchar, 2, 32> SICOutput);

CM_API extern void
cm_vme_hevc_hpm_u_gen10(/*const*/ matrix_ref<uchar, 12, 32> HPMInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInputInter,
                        int curSurfIndex, matrix_ref<uchar, 23, 32> HPMOutput);

CM_API extern void
cm_vme_hevc_hpm_b_gen10(/*const*/ matrix_ref<uchar, 12, 32> HPMInput, /*const*/ matrix_ref<uchar, 20, 32> StreamInputInter,
                        int LengthStreamInputInter, int curSurfIndex, matrix_ref<uchar, 23, 32> HPMOutput);

CM_API extern void
cm_vme_hevc_fbr_gen10(/*const*/ matrix_ref<uchar, 3, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                      int ValidCULength, int curSurfIndex, matrix_ref<uchar, 18, 32> FBROutput);

CM_API extern void
cm_vme_hevc_rpm_gen10(/*const*/ matrix_ref<uchar, 7, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                      int ValidCULength, int curSurfIndex, matrix_ref<uchar, 8, 32> RPMOutput);

CM_API extern void
cm_vme_hevc_srm_gen10(/*const*/ matrix_ref<uchar, 8, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                      int ValidCULength, int curSurfIndex, matrix_ref<uchar, 18, 32> SRMOutput);
#endif

#define selrows(r) format<ushort,r,16>().select<r,1,16,1>(0,0)

_GENX_ inline extern void
cm_vme_hevc_ime(/*const*/ matrix_ref<uchar, 5, 32> IMEInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInput, 
                int LengthStreamInput, SurfaceIndex curSurfIndex, matrix_ref<uchar, 12, 32> StreamOutput)
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

        cm_sends(StreamOutput.selrows(12), IMEInput.selrows(5), StreamInput.selrows(10), ExDescriptor, Descriptor, 0);
    } else
        cm_send(StreamOutput.selrows(12), IMEInput.selrows(5), VME_SFID_VME, Descriptor, 0);
#else
    cm_vme_hevc_ime_gen10(IMEInput, StreamInput, LengthStreamInput, cm_get_value(curSurfIndex), StreamOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_ime(/*const*/ matrix<uchar, 5, 32> &IMEInput, /*const*/ matrix<uchar, 10, 32> &StreamInput, 
                int LengthStreamInput, SurfaceIndex curSurfIndex, matrix<uchar, 12, 32> &StreamOutput)
{
    cm_vme_hevc_ime(IMEInput.select_all(), StreamInput.select_all(), LengthStreamInput, curSurfIndex, StreamOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_sic(/*const*/ matrix_ref<uchar, 5, 32> SICInput, /*const*/ matrix_ref<uchar, 8, 32> NPInput, int LengthNP,
                SurfaceIndex curSurfIndex, matrix_ref<uchar, 22, 32> SICOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, SICInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_SIC_MSG);

    if (LengthNP == 8) {
        Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, SICOutput.n_rows());
        U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, NPInput.n_rows());

        cm_sends(SICOutput.selrows(22), SICInput.selrows(5), NPInput.selrows(8), ExDescriptor, Descriptor, 0);
    } else {
        Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2);

        cm_send(SICOutput.selrows(2), SICInput.selrows(5), VME_SFID_CRE, Descriptor, 0);
    }
#else
    cm_vme_hevc_sic_gen10(SICInput, NPInput, LengthNP, cm_get_value(curSurfIndex), SICOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_sic(/*const*/ matrix<uchar, 5, 32> &SICInput, /*const*/ matrix<uchar, 8, 32> &NPInput, int LengthNP,
                SurfaceIndex curSurfIndex, matrix<uchar, 22, 32> &SICOutput)
{
    cm_vme_hevc_sic(SICInput.select_all(), NPInput.select_all(), LengthNP, curSurfIndex, SICOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_sc(/*const*/ matrix_ref<uchar, 5, 32> SICInput, SurfaceIndex curSurfIndex, matrix_ref<uchar, 2, 32> SICOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, SICInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_SIC_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, SICOutput.n_rows());

    cm_send(SICOutput.selrows(2), SICInput.selrows(5), VME_SFID_CRE, Descriptor, 0);
#else
    cm_vme_hevc_sc_gen10(SICInput, cm_get_value(curSurfIndex), SICOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_sc(/*const*/ matrix<uchar, 5, 32> &SICInput, SurfaceIndex curSurfIndex, matrix<uchar, 2, 32> &SICOutput)
{
    cm_vme_hevc_sc(SICInput.select_all(), curSurfIndex, SICOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_hpm_u(/*const*/ matrix_ref<uchar, 12, 32> HPMInput, /*const*/ matrix_ref<uchar, 10, 32> StreamInputInter,
                  SurfaceIndex curSurfIndex, matrix_ref<uchar, 23, 32> HPMOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, StreamInputInter.n_rows());

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, HPMInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_HPM_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, HPMOutput.n_rows());

    cm_sends(HPMOutput.selrows(23), HPMInput.selrows(12), StreamInputInter.selrows(10), ExDescriptor, Descriptor, 0);
#else
    cm_vme_hevc_hpm_u_gen10(HPMInput, StreamInputInter, cm_get_value(curSurfIndex), HPMOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_hpm_u(/*const*/ matrix<uchar, 12, 32> &HPMInput, /*const*/ matrix<uchar, 10, 32> &StreamInputInter,
                  SurfaceIndex curSurfIndex, matrix<uchar, 23, 32> &HPMOutput)
{
    cm_vme_hevc_hpm_u(HPMInput.select_all(), StreamInputInter.select_all(), curSurfIndex, HPMOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_hpm_b(/*const*/ matrix_ref<uchar, 12, 32> HPMInput, /*const*/ matrix_ref<uchar, 20, 32> StreamInputInter,
                  int LengthStreamInputInter, SurfaceIndex curSurfIndex, matrix_ref<uchar, 23, 32> HPMOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, LengthStreamInputInter);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, HPMInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_HPM_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, HPMOutput.n_rows());

    cm_sends(HPMOutput.selrows(23), HPMInput.selrows(12), StreamInputInter.selrows(20), ExDescriptor, Descriptor, 0);
#else
    cm_vme_hevc_hpm_b_gen10(HPMInput, StreamInputInter, LengthStreamInputInter, cm_get_value(curSurfIndex), HPMOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_hpm_b(/*const*/ matrix<uchar, 12, 32> &HPMInput, /*const*/ matrix<uchar, 20, 32> &StreamInputInter,
                  int LengthStreamInputInter, SurfaceIndex curSurfIndex, matrix<uchar, 23, 32> &HPMOutput)
{
    cm_vme_hevc_hpm_b(HPMInput.select_all(), StreamInputInter.select_all(), LengthStreamInputInter, curSurfIndex,
                      HPMOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_fbr(/*const*/ matrix_ref<uchar, 3, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> FBROutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, ValidCULength);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_FBR_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2 + ValidCULength);

    cm_sends(FBROutput.selrows(18), UNIInput.selrows(3), CUInput.selrows(16), ExDescriptor, Descriptor, 0);
#else
    cm_vme_hevc_fbr_gen10(UNIInput, CUInput, ValidCULength, cm_get_value(curSurfIndex), FBROutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_fbr(/*const*/ matrix<uchar, 3, 32> &UNIInput, /*const*/ matrix<uchar, 16, 32> &CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix<uchar, 18, 32> &FBROutput)
{
    cm_vme_hevc_fbr(UNIInput.select_all(), CUInput.select_all(), ValidCULength, curSurfIndex, FBROutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_rpm(/*const*/ matrix_ref<uchar, 7, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 8, 32> RPMOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, ValidCULength);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_RPM_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, RPMOutput.n_rows());

    cm_sends(RPMOutput.selrows(8), UNIInput.selrows(7), CUInput.selrows(16), ExDescriptor, Descriptor, 0);
#else
    cm_vme_hevc_rpm_gen10(UNIInput, CUInput, ValidCULength, cm_get_value(curSurfIndex), RPMOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_rpm(/*const*/ matrix<uchar, 7, 32> &UNIInput, /*const*/ matrix<uchar, 16, 32> &CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix<uchar, 8, 32> &RPMOutput)
{
    cm_vme_hevc_rpm(UNIInput.select_all(), CUInput.select_all(), ValidCULength, curSurfIndex, RPMOutput.select_all());
}

_GENX_ inline extern void
cm_vme_hevc_srm(/*const*/ matrix_ref<uchar, 8, 32> UNIInput, /*const*/ matrix_ref<uchar, 16, 32> CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> SRMOutput)
{
#ifndef CM_EMU
    U32 Descriptor = SEND_INIT_Descriptor_Surface(curSurfIndex);
    U32 ExDescriptor = SEND_SET_Descriptor_Num_InputsExt(VME_SFID_CRE, ValidCULength);

    Descriptor = SEND_SET_Descriptor_Num_Inputs(Descriptor, UNIInput.n_rows());
    Descriptor = SEND_SET_Descriptor_Header_Present(Descriptor);
    Descriptor = SEND_SET_Descriptor_HEVC_Mode(Descriptor);
    Descriptor = SEND_SET_Descriptor_Function(Descriptor, VME_HEVC_SRM_MSG);
    Descriptor = SEND_SET_Descriptor_Num_Outputs(Descriptor, 2 + ValidCULength);

    cm_sends(SRMOutput.selrows(18), UNIInput.selrows(8), CUInput.selrows(16), ExDescriptor, Descriptor, 0);
#else
    cm_vme_hevc_srm_gen10(UNIInput, CUInput, ValidCULength, cm_get_value(curSurfIndex), SRMOutput);
#endif
}

_GENX_ inline extern void
cm_vme_hevc_srm(/*const*/ matrix<uchar, 8, 32> &UNIInput, /*const*/ matrix<uchar, 16, 32> &CUInput,
                int ValidCULength, SurfaceIndex curSurfIndex, matrix<uchar, 18, 32> &SRMOutput)
{
    cm_vme_hevc_srm(UNIInput.select_all(), CUInput.select_all(), ValidCULength, curSurfIndex, SRMOutput.select_all());
}

#endif /* CM_GEN10_VME_H */
