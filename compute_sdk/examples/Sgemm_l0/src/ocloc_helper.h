/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <tuple>
#include <cassert>
#include <ocloc_api.h>

class oclocInfo_t {
    unsigned int numOutputs_;
    unsigned char** outputs_;
    size_t* outputLengths_;
    char** outputNames_;
public:
    oclocInfo_t(unsigned int numOutputs, unsigned char** outputs,
                size_t* outputLengths, char** outputNames) noexcept
    :
         numOutputs_(numOutputs),
         outputs_(outputs),
         outputLengths_(outputLengths),
         outputNames_(outputNames)
    {}

    ~oclocInfo_t()
    {
         oclocFreeOutput(&numOutputs_, &outputs_, &outputLengths_, &outputNames_);
    }

    oclocInfo_t(const oclocInfo_t&) = delete;
    oclocInfo_t& operator= (const oclocInfo_t&) = delete;

    unsigned int GetOutputsNum() const noexcept
    {
         return numOutputs_;
    }

    unsigned char* GetOutput(int idx) noexcept
    {
         return outputs_[idx];
    }

    size_t GetOutputLength(int idx) const noexcept
    {
         return outputLengths_[idx];
    }

    char* GetOutputName(int idx) const noexcept
    {
         return outputNames_[idx];
    }
};

static oclocInfo_t oclocOnlineCompile(const char* mainFileName, std::vector<const char*>& oclocArgv)
{
// reading source
    std::ifstream ifs(mainFileName);
    assert(ifs.is_open());

    ifs.seekg(0, std::ios::end);
    auto len = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string kernelSource(len, '\0');
    ifs.read(kernelSource.data(), len);
//
    const unsigned char *sources[] = { reinterpret_cast<const unsigned char*>(kernelSource.c_str()) };
    size_t sourcesLengths[] = { kernelSource.size() + 1 };
    const char *sourcesNames[] = { mainFileName };

    unsigned int numOutputs = 0U;
    unsigned char** outputs = nullptr;
    size_t* outputLengths = nullptr;
    char** outputNames = nullptr;

    oclocInvoke(oclocArgv.size(), &oclocArgv[0],
                1, sources, sourcesLengths, sourcesNames,
                0, nullptr, nullptr, nullptr,
                &numOutputs, &outputs, &outputLengths, &outputNames);

    return {numOutputs, outputs, outputLengths, outputNames};
}

static std::string GetCMCCompileLine() {
#if defined(__LP64__) || defined(_M_AMD64)
    std::string args = "-cmc -m64 -I.";
#else
    std::string args = "-cmc -m32 -I.";
#endif

    return args;
}
