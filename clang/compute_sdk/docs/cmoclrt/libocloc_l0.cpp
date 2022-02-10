/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

const char *name = "vadd_genx.cpp";
FILE *fp = fopen(name, "rb");
fseek(fp, 0, SEEK_END);
size_t sz = ftell(fp);
fclose(fp);

const char *argv[] = { "ocloc", "compile", "-device", "skl", "-file", name, "-options", "-cmc /c /Qxcm /mCM_emit_common_isa -v -fcmocl -m64"};
const unsigned char *sources[] = {  };
size_t sourcesLengths[] = { sz + 1 };
const char *sourcesNames[] = { mainFileName };
unsigned int numOutputs = 0U;
unsigned char **outputs = nullptr;
size_t *ouputLengths = nullptr;
char **outputNames = nullptr;

int result = oclocInvoke(countof<int>(argv),
                         argv,
                         countof<int>(sources),
                         sources,
                         sourcesLengths,
                         sourcesNames,
                         0,
                         nullptr,
                         nullptr,
                         nullptr,
                         &numOutputs,
                         &outputs,
                         &ouputLengths,
                         &outputNames);
...
// Passing SPIR-V to L0 RT
ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC,
    nullptr,
    ZE_MODULE_FORMAT_IL_SPIRV,
    ouputLengths[0],
    outputs[0],
    "-vc-codegen",
    nullptr};

zeModuleCreate(context, device, &moduleDesc, &module, nullptr);

// Passing binary to L0 RT
ze_module_desc_t moduleDesc = {
    ZE_STRUCTURE_TYPE_MODULE_DESC,
    nullptr,
    ZE_MODULE_FORMAT_IL_NATIVE,
    ouputLengths[1],
    outputs[1],
    "-vc-codegen",
    nullptr};

zeModuleCreate(context, device, &moduleDesc, &module, nullptr);
