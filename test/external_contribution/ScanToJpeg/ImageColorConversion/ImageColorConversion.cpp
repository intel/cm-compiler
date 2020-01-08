/*
 *
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

/* The jpegencoder implementation was originally based on Intel libva-utils
 * jpegenc.c.
 * https://github.com/intel/libva-utils
 */

#include <assert.h>
#include <string.h>
#include "ImageColorConversion.h"
#include <va/va.h>

#define CHECK_VASTATUS(va_status,func)                                  \
   if (va_status != VA_STATUS_SUCCESS) {                                   \
      fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
      exit(1);                                                             \
   }

ImageColorConversion::ImageColorConversion(VADisplay va_dpy)
{
   m_pVADpy  = va_dpy;
}

ImageColorConversion::~ImageColorConversion(void)
{
   vaDestroyContext(m_pVADpy, m_contextID);
   vaDestroyConfig(m_pVADpy, m_configID);
}

int ImageColorConversion::PreRun(VASurfaceID inSurfID, unsigned int inputWidth,
      unsigned int inputHeight, VASurfaceID outSurfID, unsigned int outputWidth,
      unsigned int outputHeight
      )
{
   VAEntrypoint entrypoints[5];
   VAConfigAttrib attrib;
   int num_entrypoints, vpp_entrypoint;
   VAStatus va_status;
   VAProcPipelineParameterBuffer pipeline_param;
   VARectangle *input_region, *output_region;

   va_status = vaQueryConfigEntrypoints(m_pVADpy, VAProfileNone, entrypoints, &num_entrypoints);

   CHECK_VASTATUS(va_status, "vaQueryConfigEntrypoints");

   for (vpp_entrypoint = 0; vpp_entrypoint < num_entrypoints; vpp_entrypoint++)
   {
      if (entrypoints[vpp_entrypoint] == VAEntrypointVideoProc)
         break;
   }
   if (vpp_entrypoint == num_entrypoints)
   {
      /* not find video proc entry point */
      assert(0);
   }

   /* Check for render target surface */
   attrib.type = VAConfigAttribRTFormat;
   vaGetConfigAttributes(m_pVADpy, VAProfileNone, VAEntrypointVideoProc, &attrib, 1);

   va_status = vaCreateConfig(m_pVADpy, VAProfileNone, VAEntrypointVideoProc,
         &attrib, 1, &m_configID);
   CHECK_VASTATUS(va_status, "vaCreateConfig");

   /* Create a context for this decode pipe */
   va_status = vaCreateContext(m_pVADpy, m_configID, outputWidth, outputHeight,
         VA_PROGRESSIVE, &outSurfID, 1, &m_contextID);
   CHECK_VASTATUS(va_status, "vaCreateContext");

   input_region = (VARectangle *) malloc(sizeof(VARectangle));
   output_region = (VARectangle *) malloc(sizeof(VARectangle));

   input_region->x = 0;
   input_region->y = 0;
   input_region->width = inputWidth;
   input_region->height = inputHeight;

   output_region->x = 0;
   output_region->y = 0;
   output_region->width = outputWidth;
   output_region->height = outputHeight;

   memset(&pipeline_param, 0, sizeof(pipeline_param));
   pipeline_param.surface = inSurfID;
   pipeline_param.surface_region = input_region;
   pipeline_param.output_region = output_region;

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
                  VAProcPipelineParameterBufferType,
                  sizeof(pipeline_param),
                  1,
                  &pipeline_param,
                  &m_pipeline_param_buf_id);

   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   return va_status;
}

int ImageColorConversion::CreateInputSurfaces(const char * filename,
      const int picture_width, const int picture_height, unsigned int fourcc,
      int surface_type, VASurfaceID *OutVASurfaceID)
{
   VAStatus va_status = 0;
   VASurfaceAttrib surface_attrib;

   surface_attrib.type = VASurfaceAttribPixelFormat;
   surface_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
   surface_attrib.value.type = VAGenericValueTypeInteger;
   surface_attrib.value.value.i = fourcc;

   va_status = vaCreateSurfaces(m_pVADpy, surface_type, picture_width,
         picture_height, OutVASurfaceID, 1, &surface_attrib , 2);
   CHECK_VASTATUS(va_status, "vaCreateSurface");

   if (va_status == VA_STATUS_SUCCESS)
   {
      VAImage surface_image;
      void *surface_p = NULL;
      unsigned char *y_dst;

      va_status = vaDeriveImage(m_pVADpy, *OutVASurfaceID, &surface_image);
      CHECK_VASTATUS(va_status, "vaDeriveImage");

      va_status = vaMapBuffer(m_pVADpy, surface_image.buf, &surface_p);
      CHECK_VASTATUS(va_status, "vaMapBuffer");

      y_dst = (unsigned char *) surface_p + surface_image.offsets[0];

      if (UploadInputImage(filename, y_dst, picture_width, picture_height, surface_image.pitches[0], false) < 0)
      {
         va_status = vaUnmapBuffer(m_pVADpy, surface_image.buf);
         va_status = vaDestroyImage(m_pVADpy, surface_image.image_id);
         return -1;
      }

      va_status = vaUnmapBuffer(m_pVADpy, surface_image.buf);
      CHECK_VASTATUS(va_status, "vaUnmapBuffer");

      va_status = vaDestroyImage(m_pVADpy, surface_image.image_id);
      CHECK_VASTATUS(va_status, "vaDestroyBuffer");
   }

   if (va_status != VA_STATUS_SUCCESS)
      return -1;
   else
      return 0;

}


int ImageColorConversion::CreateSurfaces(const int picture_width, const int picture_height,
      unsigned int fourcc, int surface_type, VASurfaceID *OutVASurfaceID)
{
   VAStatus va_status;
   VASurfaceAttrib surface_attrib;

   surface_attrib.type = VASurfaceAttribPixelFormat;
   surface_attrib.flags = VA_SURFACE_ATTRIB_SETTABLE;
   surface_attrib.value.type = VAGenericValueTypeInteger;
   surface_attrib.value.value.i = fourcc;

   va_status = vaCreateSurfaces(m_pVADpy, surface_type, picture_width,
         picture_height, OutVASurfaceID, 1, &surface_attrib , 2);
   CHECK_VASTATUS(va_status, "vaCreateSurface");

   return va_status;
}

int ImageColorConversion::Execute(VASurfaceID outSurfID)
{
   VAStatus va_status;

   /* Begin picture */
   va_status = vaBeginPicture(m_pVADpy, m_contextID, outSurfID);
   CHECK_VASTATUS(va_status, "vaBeginPicture");

   /* Render picture for all the VA buffers created */
   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_pipeline_param_buf_id, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaEndPicture(m_pVADpy, m_contextID);
   CHECK_VASTATUS(va_status, "vaEndPicture");

   va_status = vaSyncSurface(m_pVADpy, outSurfID);
   CHECK_VASTATUS(va_status, "vaSyncSurface");

   return va_status;
}

void ImageColorConversion::Destroy(VASurfaceID vaSurfaceID)
{
   /* FIXME: Need to track the source surface to destroy instead of the
    * destination surface */
   //CHECK_VASTATUS(vaDestroySurfaces(m_pVADpy, &vaSurfaceID, 1), "vaDestroySurfaces");
}

int ImageColorConversion::WriteOut(VASurfaceID outSurfID, const char* filename)
{
   VAStatus va_status;
   VAImage  imgout;
   void     *vaddrout = NULL;

   va_status = vaDeriveImage(m_pVADpy, outSurfID, &imgout);
   CHECK_VASTATUS(va_status, "vaDeriveImage");

   va_status = vaMapBuffer(m_pVADpy, imgout.buf, &vaddrout);
   CHECK_VASTATUS(va_status, "vaMapBuffer");

   if (WriteToFile(filename, (unsigned char *)vaddrout, imgout.width, imgout.height,
         imgout.pitches[0], false) < 0)
   {
      vaUnmapBuffer(m_pVADpy, imgout.buf);
      vaDestroyImage(m_pVADpy, imgout.image_id);

      return -1;
   }

   va_status = vaUnmapBuffer(m_pVADpy, imgout.buf);
   CHECK_VASTATUS(va_status, "vaMapBuffer");

   va_status = vaDestroyImage(m_pVADpy, imgout.image_id);
   CHECK_VASTATUS(va_status, "vaDestroyBuffer");

   if (va_status != VA_STATUS_SUCCESS)
      return -1;
   else
      return 0;
}

