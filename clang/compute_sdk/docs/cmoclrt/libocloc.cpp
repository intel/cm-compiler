/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

const char *mainFileName = "vadd_genx.cpp";
const char *argv[] = { "ocloc", "compile", "-device", "skl", "-file", mainFileName, "-options", "-cmc /c /Qxcm /mCM_emit_common_isa -v -fcmocl -m64"};
const unsigned char *sources[] = { reinterpret_cast<const unsigned char*>(kernelSourceValid) };
size_t sourcesLengths[] = { strlen(kernelSourceValid) + 1 };
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
// Passing SPIR-V to OCL RT
cl_program prog = clCreateProgramWithIL(context,
                                        outputs[0],
                                        ouputLengths[0],
                                        &errNum);
...
// Passing binary to OCL RT
constexpr int NPROGS = 1;
uint8_t *progs[NPROGS] = {outputs[1]};
size_t progssize[NPROGS] = {ouputLengths[1]};
cl_program prog = clCreateProgramWithBinary(context,
                                            NPROGS,
                                            &device,
                                            progssize,
                                            (const unsigned char **)progs,
                                            &error,
                                            &errNum);
