/*===================== begin_copyright_notice ==================================

INTEL CONFIDENTIAL
Copyright 2014
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and confidential
information of Intel or its suppliers and licensors. The Material is protected by
worldwide copyright and trade secret laws and treaty provisions. No part of the
Material may be used, copied, reproduced, modified, published, uploaded, posted,
transmitted, distributed, or disclosed in any way without Intel’s prior express
written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel or
otherwise. Any license under such intellectual property rights must be express
and approved by Intel in writing.

======================= end_copyright_notice ==================================*/

#ifndef __CISA_LD_H__
#define __CISA_LD_H__

#ifdef STANDALONE_MODE
	#define CISA_LD_DECLSPEC
#else
	#ifdef DLL_MODE
	#define CISA_LD_DECLSPEC __declspec(dllexport)
	#else
	#define CISA_LD_DECLSPEC __declspec(dllimport)
	#endif
#endif

typedef void* ExternalHeapAllocator(unsigned size);

extern "C" CISA_LD_DECLSPEC int
linkCisaMemObjs(
    const char *kernelName,
    int numCisaObjs, const void *cisaBufs[], unsigned cisaBufSizes[],
    const void* *cisaLinkedBuf, unsigned *cisaLinkedBufSize,
    int numOptions, const char *options[],
    ExternalHeapAllocator *customAllocator);

extern "C" CISA_LD_DECLSPEC int
linkCisaFileObjs(
    const char *kernelName,
    int numCisaObjs, const char *cisaObjs[],
    const char *cisaLinkedObj,
    int numOptions, const char *options[]);

#endif // __CISA_LD_H__
