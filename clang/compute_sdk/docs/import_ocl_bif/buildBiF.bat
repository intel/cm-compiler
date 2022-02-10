::========================== begin_copyright_notice ============================
::
:: Copyright (C) 2021 Intel Corporation
::
:: SPDX-License-Identifier: MIT
::
::=========================== end_copyright_notice =============================

REM The following is the example command to build the OpenCL math builtins 
REM into a bitcode file 
REM
REM before use these commands, need to copy two top-level cl files into 
REM IGC/BiFModule corresponding folder
REM
REM -ffp-contrace=off to avoid generate intrinsic::fmuladd
REM -D__OPENCL_C_VERSION__=120 to avoid generic address-space
REM
cd d:\cmc3\build.64.vs2017\Debug\bin

clang.exe -cc1 -x cl -ffp-contract=off -fblocks -fpreserve-vec3-type -opencl-builtins -triple=spir64 -w -emit-llvm-bc -o D:/cmc3/support/docs/import_ocl_bif/BiF_impl.bc -include D:/cmc3/support/docs/import_ocl_bif/opencl-c-common.h -include D:/cmc3/support/docs/import_ocl_bif/opencl_cth.h -I D:/gfx/gfx-driver/Source/IGC/BiFModule/Implementation/include -I D:/gfx/gfx-driver/Source/IGC/BiFModule/Headers -D__EXECUTION_MODEL_DEBUG=1 -D__OPENCL_C_VERSION__=120 -D__IGC_BUILD__ -Dcl_khr_fp16 -Dcl_khr_fp64 -cl-std=CL1.2 D:/gfx/gfx-driver/Source/IGC/BiFModule/Implementation/IBiF_Impl_math.cl

clang.exe -cc1 -x cl -ffp-contract=off -fblocks -fpreserve-vec3-type -opencl-builtins -triple=spir64 -w -emit-llvm-bc -o D:/cmc3/support/docs/import_ocl_bif/BiF_ocl.bc -include D:/cmc3/support/docs/import_ocl_bif/opencl-c-common.h -include D:/cmc3/support/docs/import_ocl_bif/opencl_cth.h -I D:/gfx/gfx-driver/Source/IGC/BiFModule/Languages/OpenCL -I D:/gfx/gfx-driver/Source/IGC/BiFModule/Languages/OpenCL/PointerSize -I D:/gfx/gfx-driver/Source/IGC/BiFModule/Headers -D__EXECUTION_MODEL_DEBUG=1 -D__OPENCL_C_VERSION__=120 -D__IGC_BUILD__ -Dcl_khr_fp16 -Dcl_khr_fp64 -cl-std=CL1.2 D:/gfx/gfx-driver/Source/IGC/BiFModule/Languages/OpenCL/IBiF_OpenCL_math.cl

llvm-link.exe -o D:/cmc3/support/docs/import_ocl_bif/BiF0.bc D:/cmc3/support/docs/import_ocl_bif/BiF_ocl.bc D:/cmc3/support/docs/import_ocl_bif/BiF_impl.bc

opt.exe -scalarizer -scalarize-load-store D:/cmc3/support/docs/import_ocl_bif/BiF0.bc -o D:/cmc3/support/docs/import_ocl_bif/BiFs.bc
opt.exe -O2 D:/cmc3/support/docs/import_ocl_bif/BiFs.bc -o D:/cmc3/support/docs/import_ocl_bif/BiF.bc 

REM after getting the BiF.bc, you can use the following cmc option 
REM to import with your code 
REM -mCM_import_bif BiF.bc

REM Last, make sure to include the modified opencl-c-common.h in the CM kernel 
REM code 
