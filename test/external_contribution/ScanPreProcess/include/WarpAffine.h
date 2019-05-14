/*
 * Copyright (c) 2017, Intel Corporation
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
using namespace std;

#include "ScanPreProcess.h"

enum SkewType {
   SKEW,
   DESKEW,
};

class WarpAffine : public ScanPreProcess
{
   public:
      WarpAffine(CmDevice *pdevice);
      ~WarpAffine(void){}

      int GetSkew(
         SkewType type,
         unsigned int inputWidth,
         unsigned int inputHeight,
         unsigned int *outputWidth,
         unsigned int *outputHeight,
         int skewAngle
         );

      int PreRun(
         CmKernel *pKernel,
         SurfaceIndex *pSI_SrcSurf,
         SurfaceIndex *pSI_DstRSurf,
         SurfaceIndex *pSI_DstGSurf,
         SurfaceIndex *pSI_DstBSurf,
         unsigned int inputWidth,
         unsigned int inputHeight,
         unsigned int outputWidth,
         unsigned int outputHeight
         );
      char* GetISA() { return "WarpAffine/WarpAffine_genx.isa"; }
      char* GetKernelName() { return "WarpAffine_GENX"; }

   private:
      CmDevice *m_pCmDev;
      CmKernel *m_pKernel;
      float coord_unit_u;
      float coord_unit_v;
      float Matrix[3][2];
};
