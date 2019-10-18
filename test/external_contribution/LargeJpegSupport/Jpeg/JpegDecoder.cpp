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


int JpegDecoder::update_SOF(unsigned char *istream, int width,  int height)
{

   while (istream <= m_jdecPriv->stream_end)
   {
      if (!(*istream == 0xff && *(istream+1) == SOF))
      {
         istream++;
         continue;
      }
      else
      {
         *(istream+5) = (height >> 8) & 0xff;
         *(istream+6) = height & 0xff;
         *(istream+7) = (width >> 8) & 0xff;
         *(istream+8) = width & 0xff;

         return 1;
      }
   }

   return 0;

}

int JpegDecoder::parse_JFIF(const unsigned char *stream)
{
   int chuck_len;
   int marker;
   int sos_marker_found = 0;
   int dht_marker_found = 0;
   int dqt_marker_found = 0;
   const unsigned char *next_chunck;

   const unsigned char *stream_start, *stream_end;
   stream_start = stream;

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
         stream_end = m_jdecPriv->stream;
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
      case SOF2:
         cout << "Unsupported Progressive format" <<endl;
         return -1;
         break;
      default:
         if ((marker & APP0) != APP0)
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

//   tileJpegHeader.reserve(tileJpegHeader.size() + stream_end - stream_start);


   tileJpegHeader.insert(tileJpegHeader.end(), stream_start, stream_end);

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

JpegDecoder::JpegDecoder(JpegDecoder *&obj)
{
   m_pCmDev = obj->m_pCmDev;
   m_pVADpy = obj->m_pVADpy;

   m_jdecPriv = (struct jdec_private *)calloc(1, sizeof(struct jdec_private));
   memcpy(m_jdecPriv, obj->m_jdecPriv, sizeof(struct jdec_private));

   m_totalRowBlocks = obj->m_totalRowBlocks;
   m_totalColumnBlocks = obj->m_totalColumnBlocks;
   m_tileHeight = obj->m_tileHeight;
   m_tileWidth = obj->m_tileWidth;
   m_totalRowBlocks = obj->m_totalRowBlocks;
   m_totalColumnBlocks = obj->m_totalColumnBlocks;
   m_isTiled = obj->m_isTiled;
   tileJpegHeader = obj->tileJpegHeader;

}

JpegDecoder::~JpegDecoder(void)
{
   if (m_jdecPriv != NULL)
      free(m_jdecPriv);

   /* Free up the buffer */
   std::vector<unsigned char>().swap(tileJpegHeader);

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
   int i;

   int picture_width, picture_height;

   if (m_isTiled)
   {
      picture_width = m_curWidth;
      picture_height = m_curHeight;
   }
   else
   {
      picture_width = m_jdecPriv->width;
      picture_height = m_jdecPriv->height;

   }

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
   pic_param.picture_width = picture_width;
   pic_param.picture_height = picture_height;
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
         picture_width, picture_height,
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

unsigned int JpegDecoder::GetRestartInterval()
{
   assert(m_jdecPriv);

   return m_jdecPriv->restart_interval;
}

void JpegDecoder::SetTileInfo(unsigned int tileHeight, unsigned int tileWidth,
         unsigned int totalRowBlock, unsigned int totalColumnBlock)
{
   m_tileHeight = tileHeight;
   m_tileWidth = tileWidth;
   m_totalRowBlocks = totalRowBlock;
   m_totalColumnBlocks = totalColumnBlock;
   m_isTiled = true;
}

const unsigned char * JpegDecoder::GetLastStream()
{
   return m_tileStreamPtr;
}

void JpegDecoder::SetStartStream(const unsigned char *startStream)
{
   assert (startStream);

   m_tileStreamPtr = startStream;
}


int JpegDecoder::CreateSurfaces(const int picture_width, const int picture_height,
      VASurfaceID *OutVASurfaceID)
{
   VASurfaceAttrib fourcc;
   int surface_type;
   VAStatus va_status;

   if (m_isTiled)
   {
      m_curWidth = picture_width;
      m_curHeight = picture_height;
   }

   GetVASurfaceAttrib(m_jdecPriv, &fourcc, &surface_type);

   va_status = vaCreateSurfaces(m_pVADpy, surface_type, picture_width,
         picture_height, OutVASurfaceID, 1, &fourcc, 2);
   CHECK_VASTATUS(va_status, "vaCreateSurface");

   return va_status;

}

/* 
 * SplitXY only work with JPEG files with embedded restart marker !!
 *  
 * Given a location into the JPEG coded bitstream and number of rows to extract
 * (curTileHeight), count number MCU blocks need to extract and add into
 * tileJpegDataOut.
 *
 * A tileJpegDataOut is a valid JPEG coded buffer, but without JPEG header.  
 * Each MCU blocks has a restart marker based on the restart interval.  For example, 
 * if restart interval is 4, every 4 MCUs (8x8), will have a restart marker.
 * Restart marker are 0xFFD0, 0xFFD1, 0xFFD2 ... 0xFFD7.  
 *
 * The first MCU block should not have restart marker, and then, next MCU block 
 * restart marker will start from D0 .. D7 and wrap again.  This function will
 * not alter the location of the restart marker, but only update the value.   
 * otherwise, after the split, the restart marker is wrong. 
 *
 * SplitXY will extract JPEG based on horizontal and vertical.  Currently, only
 * support two column blocks.  With HW image width/height limit to 16K, two column
 * blocks able to handle up to 32768 pixels width, and no limit to image height if
 * use tileheight < 16K. 
 *
 * When colIdx = 0, it will always process two column blocks, and insert second
 * column block to tmpJpegDataOut.  When colIdx = 1, it just return the tile
 * from tmpJpegDataOut.
 * 
 *                 -----------------
 *                 |   T0  |   T1  | 
 *                 -----------------
 *                 |   T2  |   T3  |
 * stream_start->  -----------------
 *                 |   T4  |   T5  |
 *                 ----------------- 
 *                 |  .... |  .... |
 *                 -----------------
 */
int JpegDecoder::SplitXY(const unsigned char *stream_start, const int rowIdx, const int colIdx,
      int curTileHeight, vector<unsigned char> &tileJpegDataOut)
{
   const unsigned char *stream_part_s, *stream_part_e;
   bool isDone;
   int mcuBlkIdx[2] = { -1, -1 };
   int mcuBlkCount = 0;

   int mcuBlkInRow = m_jdecPriv->width/m_totalColumnBlocks/(m_jdecPriv->restart_interval * 8) ;
   int mcuBlkTotal = (m_jdecPriv->width/m_totalColumnBlocks)/(m_jdecPriv->restart_interval * 8) *
                        curTileHeight/8 * 2;

   stream_part_s = stream_start;
   stream_part_e = stream_part_s;

   if (colIdx == 1)
   {
      tileJpegDataOut.insert(tileJpegDataOut.end(), tmpJpegDataOut.begin(),
            tmpJpegDataOut.end());
      /* Free up the buffer */
      std::vector<unsigned char>().swap(tmpJpegDataOut);
   }
   else
   {
      // For each row, use row MCU counter to count each MCU (rowMCU) 
      // if rowMCU/(mcuBlkInRow + 1) < mcuBlkInRow/2, insert to tileJpegDataOut,
      // else go to tmpJpegDataOut
      // if the MCU > mcuBlkInRow, reset rowMCU back to 0

      while (mcuBlkCount < mcuBlkTotal)
      {
         isDone=0;

         while (!isDone)
         {
            if  ((*stream_part_e == 0xff) && (((*(stream_part_e+1) & ~0x7) == 0xd0)))
            {
               vector<unsigned char> mcuBlkData;
               mcuBlkData.insert(mcuBlkData.end(), stream_part_s, stream_part_e);

               if ((mcuBlkIdx[0] >= 0) || (mcuBlkIdx[1] >= 0))
               {
                  mcuBlkData.erase(mcuBlkData.begin());
                  mcuBlkData.erase(mcuBlkData.begin());
                  mcuBlkData.insert(mcuBlkData.begin(), 0xD0 + (mcuBlkIdx[0]%8));
                  mcuBlkData.insert(mcuBlkData.begin(), 0xff);
               }
               else if ((mcuBlkIdx[1] == -1) && (mcuBlkCount == 1))
               {
                  mcuBlkData.erase(mcuBlkData.begin());
                  mcuBlkData.erase(mcuBlkData.begin());
               }

               if (mcuBlkCount%2)
               {
                  tmpJpegDataOut.insert(tmpJpegDataOut.end(), mcuBlkData.begin(), mcuBlkData.end());
                  mcuBlkIdx[0]++;
                  mcuBlkIdx[1]++;
               }
               else
               {
                  tileJpegDataOut.insert(tileJpegDataOut.end(), mcuBlkData.begin(), mcuBlkData.end());
               }
               // Update 
               stream_part_s = stream_part_e;
               isDone = 1;
               mcuBlkCount++;

            }

            *stream_part_e++;

            /* EOF */
            if ((stream_part_e + 1) > m_jdecPriv->stream_scan)
            {
               tmpJpegDataOut.insert(tmpJpegDataOut.end(), stream_part_s, stream_part_e);
               isDone = 1;
               mcuBlkCount = mcuBlkTotal;
            }
         }
      }

      m_tileStreamPtr = stream_part_e + 1;
   }
}

/* 
 * SplitY only work with JPEG files with embedded restart marker !!
 *  
 * Given a location into the JPEG coded bitstream and number of rows to extract
 * (curTileHeight), count number MCU blocks need to extract and add into
 * tileJpegDataOut.
 *
 * A tileJpegDataOut is a valid JPEG coded buffer, but without JPEG header.  
 * Each MCU blocks has a restart marker based on the restart interval.  For example, 
 * if restart interval is 4, every 4 MCUs (8x8), will have a restart marker.
 * Restart marker are 0xFFD0, 0xFFD1, 0xFFD2 ... 0xFFD7.  
 *
 * The first MCU block should not have restart marker, and then, next MCU block 
 * restart marker will start from D0 .. D7 and wrap again.  This function will
 * not alter the location of the restart marker, but only update the value.   
 * otherwise, after the split, the restart marker is wrong. 
 *
 * SplitY will extract JPEG based on Y rows (curTileHeight) like below.
 * 
 *                ------------
 *                |   T0     |
 *                ------------
 *                |   T1     |
 * stream_start-> ------------
 *                |   T2     |
 *                ----------- 
 *                |   ....   |
 *                ------------
 */


int JpegDecoder::SplitY(const unsigned char *stream_start, int curTileHeight,
      vector<unsigned char> &tileJpegDataOut)
{
   const unsigned char *stream_part_s, *stream_part_e;
   bool isDone;
   int mcuBlkIdx = -1;

   int mcuBlkTotal = (m_jdecPriv->width/m_totalColumnBlocks)/(m_jdecPriv->restart_interval * 8) * (curTileHeight/8);

   mcuBlkTotal -= 1;
   stream_part_s = stream_start;
   stream_part_e = stream_part_s;

   while (mcuBlkIdx < mcuBlkTotal)
   {
      isDone=0;

      while (!isDone)
      {
         if  ((*stream_part_e == 0xff) && (((*(stream_part_e+1) & ~0x7) == 0xd0)))
         {
            //*(stream_part_e+1) = 0xD0 + mcuBlkIdx;
            vector<unsigned char> tempRowData;
            tempRowData.insert(tempRowData.end(), stream_part_s, stream_part_e);

            /* Update the restart marker */
            if (mcuBlkIdx > 0)
            {
               tempRowData.erase(tempRowData.begin());
               tempRowData.erase(tempRowData.begin());
               tempRowData.insert(tempRowData.begin(), 0xD0 + (mcuBlkIdx%8));
               tempRowData.insert(tempRowData.begin(), 0xff);
            }

            stream_part_s = stream_part_e;
            isDone = 1;
            mcuBlkIdx++;
            tileJpegDataOut.insert(tileJpegDataOut.end(), tempRowData.begin(), tempRowData.end());
         }

         *stream_part_e++;

         /* EOF */
         if ((stream_part_e + 1) > m_jdecPriv->stream_scan)
         {
            tileJpegDataOut.insert(tileJpegDataOut.end(), stream_part_s, stream_part_e);
            mcuBlkIdx++;
            isDone = 1;
         }

      }
   }

   m_tileStreamPtr = stream_part_e + 1;

   return 0;
}

//#define DEBUG_JPEG

int JpegDecoder::SplitTile(int rowBlkIdx, int colBlkIdx, int curTileHeight,
      vector<unsigned char> &tileJpegDataOut)
{
#ifdef DEBUG_JPEG
   unsigned char *jpegHeader = reinterpret_cast<unsigned char*>(tileJpegHeader.data());
#endif
   const unsigned char *stream_start;

#ifdef DEBUG_JPEG
   update_SOF(jpegHeader, m_tileWidth, curTileHeight);

   tileJpegDataOut.insert(tileJpegDataOut.end(), tileJpegHeader.begin(),
         tileJpegHeader.end());
#endif

   if ((rowBlkIdx == 0) && (colBlkIdx == 0))
      stream_start = m_jdecPriv->stream;
   else
      stream_start = m_tileStreamPtr;

   if (m_totalColumnBlocks == 1)
      SplitY(stream_start, curTileHeight, tileJpegDataOut);
   else
      SplitXY(stream_start, rowBlkIdx, colBlkIdx, curTileHeight, tileJpegDataOut);

#ifdef DEBUG_JPEG
   tileJpegDataOut.push_back(0xff);
   tileJpegDataOut.push_back(0xd9);
#endif

}


int JpegDecoder::CreateSliceParam(int tileHeight, vector<unsigned char>
      tileJpegBuffer)
{
   VAStatus va_status;
   static VASliceParameterBufferJPEGBaseline slice_param;
   int max_h_factor, max_v_factor;
   const unsigned char *stream_end, *stream_start;

   max_h_factor = m_jdecPriv->component_infos[0].Hfactor;
   max_v_factor = m_jdecPriv->component_infos[0].Vfactor;

   if (m_isTiled)
   {
      const unsigned char *tileJpeg = reinterpret_cast<unsigned char*>(tileJpegBuffer.data());
      stream_end = tileJpeg + tileJpegBuffer.size();

      stream_start = tileJpeg;
      slice_param.num_mcus = ((m_tileWidth+max_h_factor*8-1)/(max_h_factor*8))*
         ((tileHeight+max_v_factor*8-1)/(max_v_factor*8));  // ?? 720/16?

#ifdef DEBUG_JPEG
      std::ofstream outfile("tilejpegout.jpg", std::ios::out |
            std::ios::binary);
      outfile.write((char*) stream_start, tileJpegBuffer.size());
#endif
   }
   else
   {
      stream_end = m_jdecPriv->stream_scan;
      stream_start = m_jdecPriv->stream;
      slice_param.num_mcus = ((m_jdecPriv->width+max_h_factor*8-1)/(max_h_factor*8))*
         ((m_jdecPriv->height+max_v_factor*8-1)/(max_v_factor*8));  // ?? 720/16?
   }

   // one slice for whole image?
   slice_param.slice_data_size = (stream_end - stream_start);
   slice_param.slice_data_offset = 0;
   slice_param.slice_data_flag = VA_SLICE_DATA_FLAG_ALL;
   slice_param.slice_horizontal_position = 0;
   slice_param.slice_vertical_position = 0;
   slice_param.num_components = m_jdecPriv->cur_sos.nr_components;
   for (int i = 0; i < slice_param.num_components; i++)
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
   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VASliceParameterBufferType,      // VASliceParameterBufferJPEGBaseline ? 
         sizeof(VASliceParameterBufferJPEGBaseline),
         1,
         &slice_param, &m_slice_param_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VASliceDataBufferType,
         stream_end - stream_start,
         1,
         (void*) stream_start,   //jpeg-clip,
         &m_slice_data_buf);
   CHECK_VASTATUS(va_status, "vaCreateBuffer");

}

int JpegDecoder::Run(VASurfaceID inputSurfID, int rowBlkIdx, int colBlkIdx)
{
   VAStatus va_status;
   vector<unsigned char> tileJpegBuffer;

   /* 
    * If the images are divided into till, extract one tile from the JPEG file
    * and only decode a particular tiles based on row/column index
    */
   if (m_isTiled)
      SplitTile(rowBlkIdx, colBlkIdx, m_curHeight, tileJpegBuffer);

   CreateSliceParam(m_curHeight, tileJpegBuffer);

   /* Ensure earlier task on inputSurfID has completed */
   va_status = vaSyncSurface(m_pVADpy, inputSurfID);
   CHECK_VASTATUS(va_status, "vaSyncSurface");

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

   return va_status;
}

/*
 * The function takes decoded raw data (grayscale) only, and add into the output
 * CPU memory address.  The decoded raw data is in GPU memory based on
 * outputSurfID.  
 *
 * Currently only support two column tiles.  If m_totalColumnBlocks ==1, it
 * means the tile will append row by row.
 *
 * pDst ---->  ------------
 *             |   T0     |
 *             ------------
 *             |   T1     |
 *             ------------
 *             |   T2     |
 *             ----------- 
 *             |   ....   |
 *             ------------
 *
 * If m_totalColumnBlocks == 2,  tiles will be append in following order until
 * reach the end of the tiles.
 *
 * pDst ---->   ------------
 *              | T0  | T1 |
 *              ------------
 *              | T2  | T3 |
 *              ------------   
 *              | ... | ...|
 *              ------------
 *
 *    FIXME:  CPU memcpy is slow, move to use GPU CM function, 
 *    EnqueueCopyGPUtoCPUFullStride, but due to CreateSurface2D for U8 surface
 *    is not supported yet, temporary use the CPU memcpy which is about 2-3x
 *    slower.  For combine RGB tiles to pDst, GPU to CPU copy already used.
 */
int JpegDecoder::AppendTileToOutput(unsigned char *pDst, int rowIdx, int colIdx,
      VASurfaceID outputSurfID)
{
   VAStatus va_status;
   VAImage imgout;
   void *vaddrout = NULL;

   va_status = vaDeriveImage(m_pVADpy, outputSurfID, &imgout);
   CHECK_VASTATUS(va_status, "vaDeriveImage");

   va_status = vaMapBuffer(m_pVADpy, imgout.buf, &vaddrout);
   CHECK_VASTATUS(va_status, "vaMapBuffer");

   if (m_totalColumnBlocks == 1)
   {
      unsigned char * pDstTilePtr;
      unsigned int offset = rowIdx * m_tileHeight * m_jdecPriv->width;

      pDstTilePtr = pDst + offset;

      for (int y = 0; y < m_curHeight; y++)
      {
         memcpy(pDstTilePtr,vaddrout, imgout.pitches[0]);
         vaddrout += imgout.pitches[0];
         pDstTilePtr += m_jdecPriv->width;
      }
   }
   else if (m_totalColumnBlocks == 2)
   {
      unsigned char *pDstTilePtr;
      unsigned int offset = rowIdx * m_tileHeight * m_jdecPriv->width + colIdx *
         m_tileWidth;

      pDstTilePtr = pDst + offset;

      for (int y = 0; y < m_curHeight; y++)
      {
         memcpy(pDstTilePtr,vaddrout, m_tileWidth);
         vaddrout += imgout.pitches[0];
         pDstTilePtr += m_jdecPriv->width;
      }
   }
   else
   {
      assert(0);
   }

   va_status = vaUnmapBuffer(m_pVADpy, imgout.buf);
   CHECK_VASTATUS(va_status, "vaMapBuffer");

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

