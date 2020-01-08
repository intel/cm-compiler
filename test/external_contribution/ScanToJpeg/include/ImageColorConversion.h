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

#include <va/va.h>
#include "Scan2FilePipeline.h"

#ifndef IMAGECOLORCONVERSION_H
#define IMAGECOLORCONVERSION_H

class ImageColorConversion : public Scan2FilePipeline
{
public:
   ImageColorConversion(VADisplay va_dpy);
   ~ImageColorConversion(void);

   int PreRun(VASurfaceID inSurfID, unsigned int inWidth, unsigned int inHeight,
         VASurfaceID outSurfID, unsigned int outWidth, unsigned int outHeight);

   int CreateInputSurfaces(const char * filename, const int picture_width,
         const int picture_height, unsigned int fourcc,  int surface_type,
         VASurfaceID *OutVASurfaceID);

   int CreateSurfaces(const int picture_width, const int picture_height,
         unsigned int fourcc, int surface_type, VASurfaceID *VASurfaceID);
   int WriteOut(VASurfaceID vaSurfaceID, const char *filename) override;
   int Execute(VASurfaceID inputSurfaceID) override;
   void Destroy(VASurfaceID vaSurfaceID) override;

private:
   VADisplay         m_pVADpy;
   VAContextID       m_contextID;
   VAConfigID        m_configID;
   VABufferID        m_pipeline_param_buf_id;
};

#endif
