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
#include <fstream>
#include "JpegDecoder.h"
#include <va/va.h>

#define CHECK_VASTATUS(va_status,func)                                  \
   if (va_status != VA_STATUS_SUCCESS) {                                   \
      fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
      exit(1);                                                             \
   }

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

static VAHuffmanTableBufferJPEGBaseline default_huffman_table_param={
    .load_huffman_table = {0, 0},
    .huffman_table =
    {
        // lumiance component
        {
            .num_dc_codes = {0,1,5,1,1,1,1,1,1,0,0,0}, // 12 bits is ok for baseline profile
            .dc_values = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b},
            .num_ac_codes = {0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125},
            .ac_values = {
              0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
              0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
              0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
              0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
              0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
              0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
              0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
              0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
              0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
              0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
              0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
              0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
              0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
              0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
              0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
              0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
              0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
              0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
              0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
              0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
              0xf9, 0xfa
           },//,0xonly,0xthe,0xfirst,0x162,0xbytes,0xare,0xavailable,0x
        },
        // chrom component
        {
            .num_dc_codes = {0,3,1,1,1,1,1,1,1,1,1,0}, // 12 bits is ok for baseline profile
            .dc_values = {0,1,2,3,4,5,6,7,8,9,0xa,0xb},
            .num_ac_codes = {0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119},
            .ac_values = {
              0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
              0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
              0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
              0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
              0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
              0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
              0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
              0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
              0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
              0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
              0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
              0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
              0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
              0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
              0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
              0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
              0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
              0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
              0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
              0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
              0xf9, 0xfa
            },//,0xonly,0xthe,0xfirst,0x162,0xbytes,0xare,0xavailable,0x
        },
    }
};

#define be16_to_cpu(x) (((x)[0] << 8) | (x)[1])

int JpegDecoder::build_default_huffman_tables()
{
   if (m_jdecPriv->default_huffman_table_initialized)
      return 0;

   for (int i = 0; i < 4; i++)
   {
      m_jdecPriv->HTDC_valid[i] = 1;
      memcpy(m_jdecPriv->HTDC[i].bits,
            default_huffman_table_param.huffman_table[i%2].num_dc_codes,
            sizeof(default_huffman_table_param.huffman_table[i%2].num_dc_codes));
      memcpy(m_jdecPriv->HTDC[i].bits,
            default_huffman_table_param.huffman_table[i%2].dc_values,
            sizeof(default_huffman_table_param.huffman_table[i%2].dc_values));

      m_jdecPriv->HTAC_valid[i] = 1;
      memcpy(m_jdecPriv->HTDC[i].bits,
            default_huffman_table_param.huffman_table[i%2].num_ac_codes,
            sizeof(default_huffman_table_param.huffman_table[i%2].num_ac_codes));
      memcpy(m_jdecPriv->HTDC[i].bits,
            default_huffman_table_param.huffman_table[i%2].ac_values,
            sizeof(default_huffman_table_param.huffman_table[i%2].ac_values));
   }

   m_jdecPriv->default_huffman_table_initialized = 1;

   return 0;
}

static void print_SOF(const unsigned char *stream)
{

}

int JpegDecoder::parse_DQT(const unsigned char *stream)
{
   int qi;
   const unsigned char *dqt_block_end;

   dqt_block_end = stream + be16_to_cpu(stream);
   stream += 2;   /* Skip length */

   while (stream < dqt_block_end)
   {
      qi = *stream++;
      memcpy(m_jdecPriv->Q_tables[qi&0x0F], stream, 64);
      m_jdecPriv->Q_tables_valid[qi & 0x0f] = 1;
      stream += 64;
   }

   return 0;
}


int JpegDecoder::parse_SOF(const unsigned char *stream)
{
   int width, height, nr_components, cid, sampling_factor;
   unsigned char Q_table;
   struct component *c;

   print_SOF(stream);

   height = be16_to_cpu(stream+3);
   width = be16_to_cpu(stream+5);
   nr_components = stream[7];
   m_jdecPriv->nf_components = nr_components;

   stream += 8;
   for (int i = 0; i < nr_components; i++)
   {
      cid = *stream++;
      sampling_factor = *stream++;
      Q_table = *stream++;
      c = &m_jdecPriv->component_infos[i];
      c->cid = cid;
      if (Q_table >= COMPONENTS)
         cout << "Error: Bad Quantization table index (got " << Q_table << ", max allowed " << COMPONENTS-1 << endl;
      c->Vfactor = sampling_factor & 0xf;
      c->Hfactor = sampling_factor >> 4;
      c->quant_table_index = Q_table;
   }

   m_jdecPriv->width = width;
   m_jdecPriv->height = height;

   return 0;
}


int JpegDecoder::parse_SOS(const unsigned char *stream)
{
   unsigned int cid, table;
   unsigned int nr_components = stream[2];

   m_jdecPriv->cur_sos.nr_components = nr_components;

   stream += 3;

   for (int i = 0; i < nr_components; i++)
   {
      cid = *stream++;
      table = *stream++;
      m_jdecPriv->cur_sos.components[i].component_id = cid;
      m_jdecPriv->cur_sos.components[i].dc_selector = ((table>>4) & 0x0F);
      m_jdecPriv->cur_sos.components[i].ac_selector = (table & 0x0F);
   }

   m_jdecPriv->stream = stream+3;

   return 0;
}

int JpegDecoder::parse_DHT(const unsigned char *stream)
{
   unsigned int count, i;
   int length, index;
   unsigned char Tc, Th;

   length = be16_to_cpu(stream) - 2;
   stream += 2;   /* Skip length */

   while (length > 0)
   {
      index = *stream++;

      Tc = index & 0xf0;   // it is not important to <<4
      Th = index & 0x0f;
      if (Tc) {
         memcpy(m_jdecPriv->HTAC[Th].bits, stream, 16);
      } else {
         memcpy(m_jdecPriv->HTDC[Th].bits, stream, 16);
      }

      count = 0;
      for (i = 0; i < 16; i++) {
         count +=  *stream++;
      }

      if (Tc) {
         memcpy(m_jdecPriv->HTAC[Th].values, stream, count);
         m_jdecPriv->HTAC_valid[Th] = 1;
      } else {
         memcpy(m_jdecPriv->HTDC[Th].values, stream, count);
         m_jdecPriv->HTDC_valid[Th] = 1;
      }

      length -= 1;
      length -= 16;
      length -= count;
      stream += count;

   }
   return 0;
}

int JpegDecoder::parse_DRI(const unsigned char *stream)
{
   unsigned int length;

   length = be16_to_cpu(stream);

   m_jdecPriv->restart_interval = be16_to_cpu(stream+2);

   return 0;
}

int JpegDecoder::findEOI(const unsigned char *stream)
{
   while (stream <= m_jdecPriv->stream_end &&  !(*stream == 0xff && *(stream+1) == 0xd9)) // searching for end of image marker
   {
      stream++;
      continue;
   }
   m_jdecPriv->stream_scan = stream;
   return 0;
}

int JpegDecoder::findSOI(const unsigned char *stream)
{
   while (!(*stream == 0xff && *(stream+1) == 0xd8))  // searching for the start of image marker
   {
      if (stream <= m_jdecPriv->stream_end)
      {
         stream++;
         continue;
      }
      else
         return 0;   // No more images in the file.
   }
   m_jdecPriv->stream = stream + 2;
   return 1;
}

int JpegDecoder::parse_JFIF(const unsigned char *stream)
{
   int chuck_len;
   int marker;
   int sos_marker_found = 0;
   int dht_marker_found = 0;
   int dqt_marker_found = 0;
   const unsigned char *next_chunck;

   findSOI(stream);
   stream = m_jdecPriv->stream;

   while (!sos_marker_found && stream<=m_jdecPriv->stream_end)
   {
      while((*stream == 0xff))
         stream++;
      marker = *stream++;
      chuck_len = be16_to_cpu(stream);
      next_chunck = stream + chuck_len;
      switch (marker)
      {
      case SOF:
         if (parse_SOF(stream) < 0)
            return -1;
         break;
      case DQT:
         if (parse_DQT(stream) < 0)
            return -1;
         dqt_marker_found = 1;
         break;
      case SOS:
         if (parse_SOS(stream) < 0)
            return -1;
         sos_marker_found = 1;
         break;
      case DHT:
         if (parse_DHT(stream) < 0)
            return -1;
         dht_marker_found = 1;
         break;
      case DRI:
         if (parse_DRI(stream) < 0)
            return -1;
         break;
      default:
         cout << "Unknown marker "<<marker<<endl;
         break;
      }
      stream = next_chunck;
   }

   if (!dht_marker_found) {
      cout<<"No Huffman table loaded, using the default one. "<<endl;
      build_default_huffman_tables();
   }
   if (!dqt_marker_found) {
      cout<<"ERROR: No Quantization table loaded, using the default one"<<endl;
   }

   findEOI(stream);

   return 1;

}

int GetVASurfaceAttrib(jdec_private * jdec, VASurfaceAttrib *fourcc, int *surface_type)
{
   assert(jdec);

   fourcc->type = VASurfaceAttribPixelFormat;
   fourcc->flags = VA_SURFACE_ATTRIB_SETTABLE;
   fourcc->value.type = VAGenericValueTypeInteger;

   int h[3], v[3];

   for (int i = 0; i < jdec->nf_components; i++)
   {
      h[i] = jdec->component_infos[i].Hfactor;
      v[i] = jdec->component_infos[i].Vfactor;
   }

   if (h[0] == 2 && h[1] == 1 && h[2] == 1 &&
         v[0] == 2 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV420;
      fourcc->value.value.i = VA_FOURCC_IMC3;
   }
   else if (h[0] == 2 && h[1] == 1 && h[2] == 1 &&
         v[0] == 1 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV422;
      fourcc->value.value.i = VA_FOURCC_422H;
   }
   else if (h[0] == 1 && h[1] == 1 && h[2] == 1 &&
         v[0] == 1 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV444;
      fourcc->value.value.i = VA_FOURCC_444P;
      //fourcc->value.value.i = VA_FOURCC_RGBP;
   }
   else if (h[0] == 4 && h[1] == 1 && h[2] == 1 &&
         v[0] == 1 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV411;
      fourcc->value.value.i = VA_FOURCC_411P;
   }
   else if (h[0] == 1 && h[1] == 1 && h[2] == 1 &&
         v[0] == 2 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV422;
      fourcc->value.value.i = VA_FOURCC_422V;
   }
   else if (h[0] == 2 && h[1] == 1 && h[2] == 1 &&
         v[0] == 2 && v[1] == 2 && v[2] == 2)
   {
      *surface_type = VA_RT_FORMAT_YUV422;
      fourcc->value.value.i = VA_FOURCC_422H;
   }
   else if (h[0] == 2 && h[1] == 2 && h[2] == 2 &&
         v[0] == 2 && v[1] == 1 && v[2] == 1)
   {
      *surface_type = VA_RT_FORMAT_YUV422;
      fourcc->value.value.i = VA_FOURCC_422V;
   }
   else
   {
      *surface_type = VA_RT_FORMAT_YUV400;
      fourcc->value.value.i = VA_FOURCC('Y','8','0','0');
   }

   return 0;

}

JpegDecoder::JpegDecoder(CmDevice *pdevice, VADisplay va_dpy)
{
   VAStatus va_status;

   m_pCmDev = pdevice;
   m_pVADpy  = va_dpy;

   m_jdecPriv = (struct jdec_private *)calloc(1, sizeof(struct jdec_private));
   assert(m_jdecPriv != NULL);

}

JpegDecoder::~JpegDecoder(void)
{
   if (m_jdecPriv != NULL)
      free(m_jdecPriv);

   vaDestroyBuffer(m_pVADpy, m_pic_param_buf);
   vaDestroyBuffer(m_pVADpy, m_iqmatrix_buf);
   vaDestroyBuffer(m_pVADpy, m_huffmantable_buf);
   vaDestroyBuffer(m_pVADpy, m_slice_param_buf);
   vaDestroyBuffer(m_pVADpy, m_slice_data_buf);
}

int JpegDecoder::ParseHeader(
      const unsigned char *imgBuf,
      unsigned int         imgSize
      )
{
   int ret;

   assert((imgBuf[0] == 0xFF) || (imgBuf[1] == SOI));

   m_jdecPriv->stream_begin = imgBuf;
   m_jdecPriv->stream_length = imgSize;
   m_jdecPriv->stream_end = m_jdecPriv->stream_begin + m_jdecPriv->stream_length;

   m_jdecPriv->stream = m_jdecPriv->stream_begin;

   ret = parse_JFIF(m_jdecPriv->stream);

   return ret;

}

int JpegDecoder::PreRun(VASurfaceID inputSurfID)
{
   VAEntrypoint entrypoints[5];
   VAConfigAttrib attrib;
   int num_entrypoints, vld_entrypoint;
   VAStatus va_status;
   int max_h_factor, max_v_factor;
   int i;

   va_status = vaQueryConfigEntrypoints(m_pVADpy, VAProfileJPEGBaseline,
         entrypoints, &num_entrypoints);

   CHECK_VASTATUS(va_status, "vaQueryConfigEntrypoints");

   for (vld_entrypoint = 0; vld_entrypoint < num_entrypoints; vld_entrypoint++)
   {
      if (entrypoints[vld_entrypoint] == VAEntrypointVLD)
         break;
   }
   if (vld_entrypoint == num_entrypoints)
   {
      /* not find VLD entry point */
      assert(0);
   }

   /* Assuming finding VLD, find out the format for the render target */
   attrib.type = VAConfigAttribRTFormat;
   vaGetConfigAttributes(m_pVADpy, VAProfileJPEGBaseline, VAEntrypointVLD,
         &attrib, 1);

   if ((attrib.value & VA_RT_FORMAT_YUV420) == 0) {
      /* not find desired YUV420 RT Format */
      /* TODO: Add YUV444 checking */
      assert(0);
   }

   va_status = vaCreateConfig(m_pVADpy, VAProfileJPEGBaseline, VAEntrypointVLD,
         &attrib, 1, &m_configID);
   CHECK_VASTATUS(va_status, "vaCreateConfig");

   VAPictureParameterBufferJPEGBaseline pic_param;
   memset(&pic_param, 0, sizeof(pic_param));
   pic_param.picture_width = m_jdecPriv->width;
   pic_param.picture_height = m_jdecPriv->height;
   pic_param.num_components = m_jdecPriv->nf_components;

   for (i = 0; i < pic_param.num_components; i++)
   {
      pic_param.components[i].component_id =
         m_jdecPriv->component_infos[i].cid;
      pic_param.components[i].h_sampling_factor =
         m_jdecPriv->component_infos[i].Hfactor;
      pic_param.components[i].v_sampling_factor =
         m_jdecPriv->component_infos[i].Vfactor;
      pic_param.components[i].quantiser_table_selector =
         m_jdecPriv->component_infos[i].quant_table_index;
   }

   /* Create a context for this decode pipe */
   va_status = vaCreateContext(m_pVADpy, m_configID,
         m_jdecPriv->width, m_jdecPriv->height,
         VA_PROGRESSIVE, &inputSurfID, 1, &m_contextID);
   CHECK_VASTATUS(va_status, "vaCreateContext");

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VAPictureParameterBufferType, sizeof(VAPictureParameterBufferJPEGBaseline),
         1, &pic_param, &m_pic_param_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   VAIQMatrixBufferJPEGBaseline iq_matrix;
   const unsigned int num_quant_tables = MIN(COMPONENTS,
            ARRAY_ELEMS(iq_matrix.load_quantiser_table));
      /* TODO: only mask it if non-default quant matrix is used, do we need
       * build default quant matrix?
       */

   memset(&iq_matrix, 0, sizeof(VAIQMatrixBufferJPEGBaseline));
   for (i = 0; i < num_quant_tables; i++)
   {
      if (!m_jdecPriv->Q_tables_valid[i])
         continue;
      iq_matrix.load_quantiser_table[i] = 1;
      for (int j = 0; j < 64; j++)
         iq_matrix.quantiser_table[i][j] = m_jdecPriv->Q_tables[i][j];
   }

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VAIQMatrixBufferType,   //VAIQMatrixBufferJPEGBaseline ?
         sizeof(VAIQMatrixBufferJPEGBaseline),
         1, &iq_matrix,
         &m_iqmatrix_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   VAHuffmanTableBufferJPEGBaseline huffman_table;
   const unsigned int num_huffman_tables = MIN(COMPONENTS,
         ARRAY_ELEMS(huffman_table.load_huffman_table));
   memset(&huffman_table, 0, sizeof(VAHuffmanTableBufferJPEGBaseline));
   assert(sizeof(huffman_table.huffman_table[0].num_dc_codes) ==
         sizeof(m_jdecPriv->HTDC[0].bits));
   assert(sizeof(huffman_table.huffman_table[0].dc_values[0]) ==
         sizeof(m_jdecPriv->HTDC[0].values[0]));
   for (i = 0; i < num_huffman_tables; i++)
   {
      if (!m_jdecPriv->HTDC_valid[i] || !m_jdecPriv->HTAC_valid[i])
         continue;
      huffman_table.load_huffman_table[i] = 1;
      memcpy(huffman_table.huffman_table[i].num_dc_codes,
            m_jdecPriv->HTDC[i].bits,
            sizeof(huffman_table.huffman_table[i].num_dc_codes));
      memcpy(huffman_table.huffman_table[i].dc_values,
            m_jdecPriv->HTDC[i].values,
            sizeof(huffman_table.huffman_table[i].dc_values));
      memcpy(huffman_table.huffman_table[i].num_ac_codes,
            m_jdecPriv->HTAC[i].bits,
            sizeof(huffman_table.huffman_table[i].num_ac_codes));
      memcpy(huffman_table.huffman_table[i].ac_values,
            m_jdecPriv->HTAC[i].values,
            sizeof(huffman_table.huffman_table[i].ac_values));
      memset(huffman_table.huffman_table[i].pad, 0,
            sizeof(huffman_table.huffman_table[i].pad));

   }
   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VAHuffmanTableBufferType,  // VAHuffmanTableBufferJPEGBaseline?
         sizeof(VAHuffmanTableBufferJPEGBaseline),
         1, &huffman_table,
         &m_huffmantable_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   // one slice for whole image?
   max_h_factor = m_jdecPriv->component_infos[0].Hfactor;
   max_v_factor = m_jdecPriv->component_infos[0].Vfactor;
   static VASliceParameterBufferJPEGBaseline slice_param;
   slice_param.slice_data_size = (m_jdecPriv->stream_scan - m_jdecPriv->stream);
   slice_param.slice_data_offset = 0;
   slice_param.slice_data_flag = VA_SLICE_DATA_FLAG_ALL;
   slice_param.slice_horizontal_position = 0;
   slice_param.slice_vertical_position = 0;
   slice_param.num_components = m_jdecPriv->cur_sos.nr_components;
   for (i = 0; i < slice_param.num_components; i++)
   {
      /* FIXME: set to values specified in SOS */
      slice_param.components[i].component_selector =
         m_jdecPriv->cur_sos.components[i].component_id;
      /* FIXME: set to values specified in SOS */
      slice_param.components[i].dc_table_selector =
         m_jdecPriv->cur_sos.components[i].dc_selector;
      /* FIXME: set to values specified in SOS */
      slice_param.components[i].ac_table_selector =
         m_jdecPriv->cur_sos.components[i].ac_selector;
   }

   slice_param.restart_interval = m_jdecPriv->restart_interval;
   slice_param.num_mcus = ((m_jdecPriv->width+max_h_factor*8-1)/(max_h_factor*8))*
      ((m_jdecPriv->height+max_v_factor*8-1)/(max_v_factor*8));  // ?? 720/16?
   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VASliceParameterBufferType,      // VASliceParameterBufferJPEGBaseline ? 
         sizeof(VASliceParameterBufferJPEGBaseline),
         1,
         &slice_param, &m_slice_param_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VASliceDataBufferType,
         m_jdecPriv->stream_scan - m_jdecPriv->stream,
         1,
         (void*) m_jdecPriv->stream,   //jpeg-clip,
         &m_slice_data_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   return va_status;
}

unsigned int JpegDecoder::GetPicWidth()
{
   assert(m_jdecPriv);

   return m_jdecPriv->width;
}

unsigned int JpegDecoder::GetPicHeight()
{
   assert(m_jdecPriv);

   return m_jdecPriv->height;
}



int JpegDecoder::CreateSurfaces(const int picture_width, const int picture_height,
      VASurfaceID *OutVASurfaceID)
{
   VASurfaceAttrib fourcc;
   int surface_type;
   VAStatus va_status;

   GetVASurfaceAttrib(m_jdecPriv, &fourcc, &surface_type);

   va_status = vaCreateSurfaces(m_pVADpy, surface_type, picture_width,
         picture_height, OutVASurfaceID, 1, &fourcc, 2);
   CHECK_VASTATUS(va_status, "vaCreateSurface");

   return va_status;

}

int JpegDecoder::Run(VASurfaceID inputSurfID)
{
   VAStatus va_status;

   /* Begin picture */
   va_status = vaBeginPicture(m_pVADpy, m_contextID, inputSurfID);
   CHECK_VASTATUS(va_status, "vaBeginPicture");

   /* Render picture for all the VA buffers created */
   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_pic_param_buf, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_iqmatrix_buf, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_huffmantable_buf, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_slice_param_buf, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_slice_data_buf, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaEndPicture(m_pVADpy, m_contextID);
   CHECK_VASTATUS(va_status, "vaEndPicture");

   va_status = vaSyncSurface(m_pVADpy, inputSurfID);
   CHECK_VASTATUS(va_status, "vaSyncSurface");

   return va_status;
}

int JpegDecoder::WriteOut(VASurfaceID inputSurfID, const char* filename)
{
   VAStatus va_status;
   VAImage  imgout;
   void     *vaddrout = NULL;

   std::ofstream outfile(filename, std::ios::out | std::ios::binary);
   int bytes_per_line = GetPicWidth();

   va_status = vaDeriveImage(m_pVADpy, inputSurfID, &imgout);
   CHECK_VASTATUS(va_status, "vaDeriveImage");

   va_status = vaMapBuffer(m_pVADpy, imgout.buf, &vaddrout);
   CHECK_VASTATUS(va_status, "vaMapBuffer");

   for (int y = 0; y < GetPicHeight() ; y++)
   {
      outfile.write((char *)vaddrout, bytes_per_line);
      vaddrout += imgout.pitches[0];
   }

   outfile.close();

   va_status = vaUnmapBuffer(m_pVADpy, imgout.buf);
   CHECK_VASTATUS(va_status, "vaMapBuffer");


   return va_status;
}

