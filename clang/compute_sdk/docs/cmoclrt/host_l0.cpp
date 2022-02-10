/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

ze_group_count_t groupCount = {n/32, 1, 1};

zeCommandListAppendLaunchKernel(hCommandList, // handle of the command list
                                hKernel,      // handle of the kernel
                                &groupCount,  // pointer to the kernel dispatch group count
                                nullptr,      // handle of the event to signal on completion
                                0,            // number of events to wait on before launching
                                nullptr);     // handle of the events to wait on before launching

// Passing SPIR-V to L0 RT
ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC, 
    nullptr,
    ZE_MODULE_FORMAT_IL_SPIRV,
    length, 
    binary,
    "-vc-codegen", 
    nullptr};

zeModuleCreate(context, device, &moduleDesc, &module, nullptr);

// Passing binary to L0 RT
ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC, 
    nullptr,
    ZE_MODULE_FORMAT_IL_NATIVE,
    length, 
    binary,
    "-vc-codegen", 
    nullptr};

zeModuleCreate(context, device, &moduleDesc, &module, nullptr);
