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

#include "Scan2FilePipeline.h"

class KernelBase
{
public:
   KernelBase(VADisplay va_dpy);
   ~KernelBase(void) { ::DestroyCmDevice( m_pCmDev ); }
   int Init();

   void GetSurface2DInfo(int width, int height, CM_SURFACE_FORMAT format,
        unsigned int & pitch, unsigned int & surface_size);
   int CreateKernel(char *isaFile, char *kernelName, CmKernel *& pKernel);
   void AddKernel(CmTask *pTask, CmKernel *pKernel);
   int ExecuteInternal();
   void DestroyInternal();
   virtual char* GetIsa() { };
   virtual char* GetKernelName () { };

protected:
   VADisplay   m_vaDpy;
   CmDevice    *m_pCmDev;
   CmTask      *m_pTask;
   CmQueue      *m_pCmQueue;
   CmEvent      *e_Pipeline;
};

class Rgb2YCbCr : public Scan2FilePipeline, public KernelBase
{
public:
   Rgb2YCbCr(VADisplay va_dpy);
   ~Rgb2YCbCr(void);
   int CreateInputSurface(const char *filename, const int width, const int
         height, unsigned int & outPitch, unsigned int &outSurfaceSize);
   int WrapSurface(VASurfaceID vaSurface);
   int PreRun(int nPicWidth, int nPicHeight);
   int Execute(VASurfaceID dummyID) override;
   void Destroy(VASurfaceID dummyID) override;

private:
   char* GetIsa() override { return "CMKernels/Rgb2YCbCr_genx.isa"; }
   char* GetKernelName() override { return "Rgb2YCbCr_GENX"; }

   CmKernel      *m_pKernel;
   SurfaceIndex  *m_pSI_SrcSurf;
   SurfaceIndex  *m_pSI_DstSurf;
   CmSurface2DUP *m_pSrcSurf;
   float          m_coeffs[9];
   float          m_offsets[3];
   unsigned char *m_pSrc;
};

class YCbCr2Rgb : public Scan2FilePipeline, public KernelBase
{
public:
   YCbCr2Rgb(VADisplay va_dpy);
   ~YCbCr2Rgb(void);
   int PreRun(
         int nPicWidth,
         int nPicHeight
         );
   int WrapSurface(VASurfaceID vaSurfaceID);
   int CreateOutputSurfaces(const int width, const int height);
   int WriteOut(VASurfaceID dummyID, const char *filename) override;
   int Execute(VASurfaceID dummyID) override;
   void Destroy(VASurfaceID dummyID) override;

private:
   char* GetIsa() { return "CMKernels/YCbCr2Rgb_genx.isa"; }
   char* GetKernelName() { return "YCbCr2Rgb_GENX"; }

   CmKernel      *m_pKernel;
   SurfaceIndex  *m_pSI_SrcSurf;
   SurfaceIndex  *m_pSI_DstSurf;
   CmSurface2DUP *m_pDstSurf;
   float          m_coeffs[4];
   float          m_offsets[3];
   unsigned char *m_pDst;
   int            m_PicWidth;
   int            m_PicHeight;
};
