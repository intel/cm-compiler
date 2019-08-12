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

/* The jpegencoder implementation was originally based on Intel libva-utils
 * jpegenc.c.
 * https://github.com/intel/libva-utils
 */

#include <assert.h>
#include "JpegEncoder.h"
#include <va/va.h>
#include <va/va_enc_jpeg.h>
#include "jpeg_utils.h"

#define CHECK_VASTATUS(va_status,func)                                  \
   if (va_status != VA_STATUS_SUCCESS) {                                   \
      fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
      exit(1);                                                             \
   }

void jpegenc_pic_param_init(VAEncPictureParameterBufferJPEG *pic_param, int width,
      int height, int quality, YUVComponentSpecs yuvComp)
{
   assert(pic_param);

   pic_param->picture_width = width;
   pic_param->picture_height = height;
   pic_param->quality = quality;

   pic_param->pic_flags.bits.profile = 0;       //Profile = Baseline
   pic_param->pic_flags.bits.progressive = 0;   //Sequential encoding
   pic_param->pic_flags.bits.huffman = 1;       //Uses Huffman coding
   pic_param->pic_flags.bits.interleaved = 0;   //Input format is interleaved (YUV)
   pic_param->pic_flags.bits.differential = 0;  //non-Differential Encoding

   pic_param->sample_bit_depth = 8;             //Only 8 bit sample depth is supported 
   pic_param->num_scan = 1;
   pic_param->num_components = yuvComp.num_components;   // Supporting only up to 3 components
   // set component id Ci and Tqi
   if (yuvComp.fourcc_val == VA_FOURCC_Y800) {
      pic_param->component_id[0] = 0;
      pic_param->quantiser_table_selector[0] = 0;
   } else {
      pic_param->component_id[0] = pic_param->quantiser_table_selector[0] = 0;
      pic_param->component_id[1] = pic_param->quantiser_table_selector[1] = 1;
      pic_param->component_id[2] = 2; pic_param->quantiser_table_selector[2] = 1;
   }

   pic_param->quality = quality;
}

void jpegenc_qmatrix_init(VAQMatrixBufferJPEG *quantization_param,
      YUVComponentSpecs yuvComp)
{
   int i;

   quantization_param->load_lum_quantiser_matrix = 1;
   for (i = 0; i<NUM_QUANT_ELEMENTS; i++) {
      quantization_param->lum_quantiser_matrix[i] = jpeg_luma_quant[jpeg_zigzag[i]];
   }

   if (yuvComp.fourcc_val == VA_FOURCC_Y800) {
      quantization_param->load_chroma_quantiser_matrix = 0;
   } else {
      quantization_param->load_chroma_quantiser_matrix = 1;
      for (i = 0; i<NUM_QUANT_ELEMENTS; i++) {
         quantization_param->chroma_quantiser_matrix[i] = jpeg_chroma_quant[jpeg_zigzag[i]];
      }
   }

}

void jpegenc_hufftable_init(VAHuffmanTableBufferJPEGBaseline *hufftable_param,
      YUVComponentSpecs yuvComp)
{
   hufftable_param->load_huffman_table[0] = 1;  //Load Luma Hufftable

   if (yuvComp.fourcc_val == VA_FOURCC_Y800) {
      hufftable_param->load_huffman_table[1] = 0; //Do not load Chroma Hufftable for Y8
   } else {
      hufftable_param->load_huffman_table[1] = 1;  //Load Chroma Hufftable for other formats
   }

   /* Load Luma hufftable values */
   /* Load DC codes */
   memcpy(hufftable_param->huffman_table[0].num_dc_codes, jpeg_hufftable_luma_dc+1, 16);
   /* Load DC values */
   memcpy(hufftable_param->huffman_table[0].dc_values, jpeg_hufftable_luma_dc+17, 12);
   /* Load AC codes */
   memcpy(hufftable_param->huffman_table[0].num_ac_codes, jpeg_hufftable_luma_ac+1, 16);
   /* Load AC values */
   memcpy(hufftable_param->huffman_table[0].ac_values, jpeg_hufftable_luma_ac+17, 162);
   memset(hufftable_param->huffman_table[0].pad, 0, 2);

   /* Load Chroma hufftable values if needed */
   if (yuvComp.fourcc_val != VA_FOURCC_Y800) {
      /* Load DC codes */
      memcpy(hufftable_param->huffman_table[1].num_dc_codes, jpeg_hufftable_chroma_dc+1, 16);
      /* Load DC values */
      memcpy(hufftable_param->huffman_table[1].dc_values, jpeg_hufftable_chroma_dc+17, 12);
      /* Load AC codes */
      memcpy(hufftable_param->huffman_table[1].num_ac_codes, jpeg_hufftable_chroma_ac+1, 16);
      /* Load AC values */
      memcpy(hufftable_param->huffman_table[1].ac_values, jpeg_hufftable_chroma_ac+17, 162);
      memset(hufftable_param->huffman_table[1].pad, 0, 2);
   }
}

void populate_quantdata(JPEGQuantSection *quantVal, int type)
{
    uint8_t zigzag_qm[NUM_QUANT_ELEMENTS];
    int i;

    quantVal->DQT = DQT;
    quantVal->Pq = 0;
    quantVal->Tq = type;
    if(type == 0) {
        for(i=0; i<NUM_QUANT_ELEMENTS; i++) {
            zigzag_qm[i] = jpeg_luma_quant[jpeg_zigzag[i]];
        }

        memcpy(quantVal->Qk, zigzag_qm, NUM_QUANT_ELEMENTS);
    } else {
        for(i=0; i<NUM_QUANT_ELEMENTS; i++) {
            zigzag_qm[i] = jpeg_chroma_quant[jpeg_zigzag[i]];
        }
        memcpy(quantVal->Qk, zigzag_qm, NUM_QUANT_ELEMENTS);
    }
    quantVal->Lq = 3 + NUM_QUANT_ELEMENTS;
}

void populate_frame_header(JPEGFrameHeader *frameHdr, YUVComponentSpecs yuvComp, int picture_width, int picture_height)
{
    int i=0;
    frameHdr->SOF = SOF0;
    frameHdr->Lf = 8 + (3 * yuvComp.num_components); //Size of FrameHeader in bytes without the Marker SOF
    frameHdr->P = 8;
    frameHdr->Y = picture_height;
    frameHdr->X = picture_width;
    frameHdr->Nf = yuvComp.num_components;

    for(i=0; i<yuvComp.num_components; i++) {
        frameHdr->JPEGComponent[i].Ci = i+1;
        if(i == 0) {
            frameHdr->JPEGComponent[i].Hi = yuvComp.y_h_subsample;
            frameHdr->JPEGComponent[i].Vi = yuvComp.y_v_subsample;
            frameHdr->JPEGComponent[i].Tqi = 0;

        } else {
            //Analyzing the sampling factors for U/V, they are 1 for all formats except for Y8. 
            //So, it is okay to have the code below like this. For Y8, we wont reach this code.
            frameHdr->JPEGComponent[i].Hi = yuvComp.u_h_subsample;
            frameHdr->JPEGComponent[i].Vi = yuvComp.u_v_subsample;
            frameHdr->JPEGComponent[i].Tqi = 1;
        }
    }
}

void populate_huff_section_header(JPEGHuffSection *huffSectionHdr, int th, int tc)
{
    int i=0, totalCodeWords=0;

    huffSectionHdr->DHT = DHT;
    huffSectionHdr->Tc = tc;
    huffSectionHdr->Th = th;

    if(th == 0) { //If Luma
        //If AC
        if(tc == 1) {
            memcpy(huffSectionHdr->Li, jpeg_hufftable_luma_ac+1, NUM_AC_RUN_SIZE_BITS);
            memcpy(huffSectionHdr->Vij, jpeg_hufftable_luma_ac+17, NUM_AC_CODE_WORDS_HUFFVAL);
        }

        //If DC        
        if(tc == 0) {
            memcpy(huffSectionHdr->Li, jpeg_hufftable_luma_dc+1, NUM_DC_RUN_SIZE_BITS);
            memcpy(huffSectionHdr->Vij, jpeg_hufftable_luma_dc+17, NUM_DC_CODE_WORDS_HUFFVAL);
        }

        for(i=0; i<NUM_AC_RUN_SIZE_BITS; i++) {
            totalCodeWords += huffSectionHdr->Li[i];
        }

        huffSectionHdr->Lh = 3 + 16 + totalCodeWords;

    } else { //If Chroma
        //If AC
        if(tc == 1) {
            memcpy(huffSectionHdr->Li, jpeg_hufftable_chroma_ac+1, NUM_AC_RUN_SIZE_BITS);
            memcpy(huffSectionHdr->Vij, jpeg_hufftable_chroma_ac+17, NUM_AC_CODE_WORDS_HUFFVAL);
        }

        //If DC        
        if(tc == 0) {
            memcpy(huffSectionHdr->Li, jpeg_hufftable_chroma_dc+1, NUM_DC_RUN_SIZE_BITS);
            memcpy(huffSectionHdr->Vij, jpeg_hufftable_chroma_dc+17, NUM_DC_CODE_WORDS_HUFFVAL);
        }

    }
}

void populate_scan_header(JPEGScanHeader *scanHdr, YUVComponentSpecs yuvComp)
{

    scanHdr->SOS = SOS;
    scanHdr->Ns = yuvComp.num_components;

    //Y Component
    scanHdr->ScanComponent[0].Csj = 1;
    scanHdr->ScanComponent[0].Tdj = 0;
    scanHdr->ScanComponent[0].Taj = 0;

    if(yuvComp.num_components > 1) {
        //U Component
        scanHdr->ScanComponent[1].Csj = 2;
        scanHdr->ScanComponent[1].Tdj = 1;
        scanHdr->ScanComponent[1].Taj = 1;

        //V Component
        scanHdr->ScanComponent[2].Csj = 3;
        scanHdr->ScanComponent[2].Tdj = 1;
        scanHdr->ScanComponent[2].Taj = 1;
    }

    scanHdr->Ss = 0;  //0 for Baseline
    scanHdr->Se = 63; //63 for Baseline
    scanHdr->Ah = 0;  //0 for Baseline
    scanHdr->Al = 0;  //0 for Baseline

    scanHdr->Ls = 3 + (yuvComp.num_components * 2) + 3;

}

// This method packs the header information which is to be sent to the driver through LibVA.
// All the information that needs to be inserted in the encoded buffer should be built and sent.
// It is the responsibility of the app talking to LibVA to build this header and send it.
// This includes Markers, Quantization tables (normalized with quality factor), Huffman tables,etc.
int build_packed_jpeg_header_buffer(unsigned char **header_buffer, YUVComponentSpecs yuvComp, int picture_width, int picture_height, uint16_t restart_interval, int quality)
{
    bitstream bs;
    int i=0, j=0;
    uint32_t temp=0;

    bitstream_start(&bs);

    //Add SOI
    bitstream_put_ui(&bs, SOI, 16);

    //Add AppData
    bitstream_put_ui(&bs, APP0, 16);  //APP0 marker
    bitstream_put_ui(&bs, 16, 16);    //Length excluding the marker
    bitstream_put_ui(&bs, 0x4A, 8);   //J
    bitstream_put_ui(&bs, 0x46, 8);   //F
    bitstream_put_ui(&bs, 0x49, 8);   //I
    bitstream_put_ui(&bs, 0x46, 8);   //F
    bitstream_put_ui(&bs, 0x00, 8);   //0
    bitstream_put_ui(&bs, 1, 8);      //Major Version
    bitstream_put_ui(&bs, 1, 8);      //Minor Version
    bitstream_put_ui(&bs, 1, 8);      //Density units 0:no units, 1:pixels per inch, 2: pixels per cm
    bitstream_put_ui(&bs, 72, 16);    //X density
    bitstream_put_ui(&bs, 72, 16);    //Y density
    bitstream_put_ui(&bs, 0, 8);      //Thumbnail width
    bitstream_put_ui(&bs, 0, 8);      //Thumbnail height

    // Regarding Quantization matrices: As per JPEG Spec ISO/IEC 10918-1:1993(E), Pg-19:
    // "applications may specify values which customize picture quality for their particular
    // image characteristics, display devices, and viewing conditions"


    //Normalization of quality factor
    quality = (quality < 50) ? (5000/quality) : (200 - (quality*2));

    //Add QTable - Y
    JPEGQuantSection quantLuma;
    populate_quantdata(&quantLuma, 0);

    bitstream_put_ui(&bs, quantLuma.DQT, 16);
    bitstream_put_ui(&bs, quantLuma.Lq, 16);
    bitstream_put_ui(&bs, quantLuma.Pq, 4);
    bitstream_put_ui(&bs, quantLuma.Tq, 4);
    for(i=0; i<NUM_QUANT_ELEMENTS; i++) {
        //scale the quantization table with quality factor
        temp = (quantLuma.Qk[i] * quality)/100;
        //clamp to range [1,255]
        temp = (temp > 255) ? 255 : temp;
        temp = (temp < 1) ? 1 : temp;
        quantLuma.Qk[i] = (unsigned char)temp;
        bitstream_put_ui(&bs, quantLuma.Qk[i], 8);
    }

    //Add QTable - U/V
    if (yuvComp.fourcc_val != VA_FOURCC_Y800) {
       JPEGQuantSection quantChroma;
       populate_quantdata(&quantChroma, 1);
       bitstream_put_ui(&bs, quantChroma.DQT, 16);
       bitstream_put_ui(&bs, quantChroma.Lq, 16);
       bitstream_put_ui(&bs, quantChroma.Pq, 4);
       bitstream_put_ui(&bs, quantChroma.Tq, 4);
       for(i=0; i<NUM_QUANT_ELEMENTS; i++) {
          //scale the quantization table with quality factor
          temp = (quantChroma.Qk[i] * quality)/100;
          //clamp to range [1,255]
          temp = (temp > 255) ? 255 : temp;
          temp = (temp < 1) ? 1 : temp;
          quantChroma.Qk[i] = (unsigned char)temp;
          bitstream_put_ui(&bs, quantChroma.Qk[i], 8);
       }
    }

    //Add FrameHeader
    JPEGFrameHeader frameHdr;
    memset(&frameHdr,0,sizeof(JPEGFrameHeader));
    populate_frame_header(&frameHdr, yuvComp, picture_width, picture_height);

    bitstream_put_ui(&bs, frameHdr.SOF, 16);
    bitstream_put_ui(&bs, frameHdr.Lf, 16);
    bitstream_put_ui(&bs, frameHdr.P, 8);
    bitstream_put_ui(&bs, frameHdr.Y, 16);
    bitstream_put_ui(&bs, frameHdr.X, 16);
    bitstream_put_ui(&bs, frameHdr.Nf, 8);
    for(i=0; i<frameHdr.Nf;i++) {
        bitstream_put_ui(&bs, frameHdr.JPEGComponent[i].Ci, 8);
        bitstream_put_ui(&bs, frameHdr.JPEGComponent[i].Hi, 4);
        bitstream_put_ui(&bs, frameHdr.JPEGComponent[i].Vi, 4);
        bitstream_put_ui(&bs, frameHdr.JPEGComponent[i].Tqi, 8);
    }

    //Add HuffTable AC and DC for Y,U/V components
    JPEGHuffSection acHuffSectionHdr, dcHuffSectionHdr;

    for(i=0; (i<yuvComp.num_components && (i<=1)); i++) {
        //Add DC component (Tc = 0)
        populate_huff_section_header(&dcHuffSectionHdr, i, 0);

        bitstream_put_ui(&bs, dcHuffSectionHdr.DHT, 16);
        bitstream_put_ui(&bs, dcHuffSectionHdr.Lh, 16);
        bitstream_put_ui(&bs, dcHuffSectionHdr.Tc, 4);
        bitstream_put_ui(&bs, dcHuffSectionHdr.Th, 4);
        for(j=0; j<NUM_DC_RUN_SIZE_BITS; j++) {
            bitstream_put_ui(&bs, dcHuffSectionHdr.Li[j], 8);
        }

        for(j=0; j<NUM_DC_CODE_WORDS_HUFFVAL; j++) {
            bitstream_put_ui(&bs, dcHuffSectionHdr.Vij[j], 8);
        }

        //Add AC component (Tc = 1)
        populate_huff_section_header(&acHuffSectionHdr, i, 1);

        bitstream_put_ui(&bs, acHuffSectionHdr.DHT, 16);
        bitstream_put_ui(&bs, acHuffSectionHdr.Lh, 16);
        bitstream_put_ui(&bs, acHuffSectionHdr.Tc, 4);
        bitstream_put_ui(&bs, acHuffSectionHdr.Th, 4);
        for(j=0; j<NUM_AC_RUN_SIZE_BITS; j++) {
            bitstream_put_ui(&bs, acHuffSectionHdr.Li[j], 8);
        }

        for(j=0; j<NUM_AC_CODE_WORDS_HUFFVAL; j++) {
            bitstream_put_ui(&bs, acHuffSectionHdr.Vij[j], 8);
        }

        if (yuvComp.fourcc_val == VA_FOURCC_Y800)
           break;
    }

    //Add Restart Interval if restart_interval is not 0
    if(restart_interval != 0) {
        JPEGRestartSection restartHdr;
        restartHdr.DRI = DRI;
        restartHdr.Lr = 4;
        restartHdr.Ri = restart_interval;

        bitstream_put_ui(&bs, restartHdr.DRI, 16);
        bitstream_put_ui(&bs, restartHdr.Lr, 16);
        bitstream_put_ui(&bs, restartHdr.Ri, 16);
    }

    //Add ScanHeader
    JPEGScanHeader scanHdr;
    populate_scan_header(&scanHdr, yuvComp);

    bitstream_put_ui(&bs, scanHdr.SOS, 16);
    bitstream_put_ui(&bs, scanHdr.Ls, 16);
    bitstream_put_ui(&bs, scanHdr.Ns, 8);

    for(i=0; i<scanHdr.Ns; i++) {
        bitstream_put_ui(&bs, scanHdr.ScanComponent[i].Csj, 8);
        bitstream_put_ui(&bs, scanHdr.ScanComponent[i].Tdj, 4);
        bitstream_put_ui(&bs, scanHdr.ScanComponent[i].Taj, 4);
    }

    bitstream_put_ui(&bs, scanHdr.Ss, 8);
    bitstream_put_ui(&bs, scanHdr.Se, 8);
    bitstream_put_ui(&bs, scanHdr.Ah, 4);
    bitstream_put_ui(&bs, scanHdr.Al, 4);

    bitstream_end(&bs);
    *header_buffer = (unsigned char *)bs.buffer;

    return bs.bit_offset;
}

void jpegenc_slice_param_init(VAEncSliceParameterBufferJPEG *slice_param, YUVComponentSpecs yuvComp)
{
   slice_param->restart_interval = 0;

   slice_param->num_components = yuvComp.num_components;

   slice_param->components[0].component_selector = 1;
   slice_param->components[0].dc_table_selector = 0;
   slice_param->components[0].ac_table_selector = 0;

   if (yuvComp.num_components > 1) {
      slice_param->components[1].component_selector = 2;
      slice_param->components[1].dc_table_selector = 1;
      slice_param->components[1].ac_table_selector = 1;

      slice_param->components[2].component_selector = 3;
      slice_param->components[2].dc_table_selector = 1;
      slice_param->components[2].ac_table_selector = 1;
   }
}


void init_yuv_component(YUVComponentSpecs *yuvComponent, VASurfaceAttrib *fourcc, unsigned int vaSurfaceType)
{
   yuvComponent->va_surface_format = vaSurfaceType;
   yuvComponent->fourcc_val = fourcc->value.value.i;

   switch(yuvComponent->fourcc_val)
   {
   case VA_FOURCC_RGBA:
      yuvComponent->num_components = 3;
      yuvComponent->y_h_subsample = 1;
      yuvComponent->y_v_subsample = 1;
      yuvComponent->u_h_subsample = 1;
      yuvComponent->u_v_subsample = 1;
      yuvComponent->v_h_subsample = 1;
      yuvComponent->v_v_subsample = 1;
      break;
   case VA_FOURCC_Y800:
      yuvComponent->num_components = 1;
      yuvComponent->y_h_subsample = 1;
      yuvComponent->y_v_subsample = 1;
      yuvComponent->u_h_subsample = 0;
      yuvComponent->u_v_subsample = 0;
      yuvComponent->v_h_subsample = 0;
      yuvComponent->v_v_subsample = 0;
      break;
   default:
      break;
   }

}

JpegEncoder::JpegEncoder(CmDevice *pdevice, VADisplay va_dpy, unsigned int yuv_type)
{
   VAStatus va_status;
   VAEntrypoint entrypoints[5];

   m_pCmDev = pdevice;
   m_pVADpy  = va_dpy;

   /* Query for the entrypoints for the JPEGBaseline profile */
   int num_entrypoints, enc_entrypoint;
   va_status = vaQueryConfigEntrypoints(m_pVADpy, VAProfileJPEGBaseline,
         entrypoints, &num_entrypoints);
   /* We need picture level encoding (VAEntrypointEncPicture).  Find if it is
    * supported
    */
   for (enc_entrypoint = 0; enc_entrypoint < num_entrypoints; enc_entrypoint++)
   {
      if (entrypoints[enc_entrypoint] == VAEntrypointEncPicture)
         break;
   }

   if (enc_entrypoint == num_entrypoints)
   {
      assert(0);
   }

   /* Query for the render target format supported for YUV444 */
   m_attrib[0].type = VAConfigAttribRTFormat;
   m_attrib[1].type = VAConfigAttribEncJPEG;
   vaGetConfigAttributes(m_pVADpy, VAProfileJPEGBaseline,
         VAEntrypointEncPicture, &m_attrib[0], 2);

   if (!((m_attrib[0].value & VA_RT_FORMAT_YUV444) ||
         (m_attrib[0].value & VA_RT_FORMAT_YUV420) ||
         (m_attrib[0].value & VA_RT_FORMAT_YUV400)
         ))
   {
      assert(0);
   }

   VAConfigAttribValEncJPEG jpeg_attrib_val;
   jpeg_attrib_val.value = m_attrib[1].value;

   /* Set JPEG profile attribs */
   jpeg_attrib_val.bits.arithmatic_coding_mode = 0;
   jpeg_attrib_val.bits.progressive_dct_mode = 0;
   jpeg_attrib_val.bits.non_interleaved_mode = 1;
   jpeg_attrib_val.bits.differential_mode = 0;

   m_attrib[1].value = jpeg_attrib_val.value;

   m_fourcc[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
   m_fourcc[0].type = VASurfaceAttribPixelFormat;
   m_fourcc[0].value.type = VAGenericValueTypeInteger;
   m_fourcc[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
   m_fourcc[1].type = VASurfaceAttribUsageHint;
      m_fourcc[1].value.value.i = VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER;

   switch(yuv_type)
   {
   case 0:  //YUV444
      m_fourcc[0].value.value.i = VA_FOURCC_RGBA;
      m_vaSurfaceType = VA_RT_FORMAT_YUV444;
      break;
   case 1:
      m_fourcc[0].value.value.i = VA_FOURCC_Y800;
      m_vaSurfaceType = VA_RT_FORMAT_YUV400;
      break;
   default:
      throw std::out_of_range("Unsupported YUV format");
   }

}

JpegEncoder::~JpegEncoder(void)
{
    vaDestroyBuffer(m_pVADpy, m_pic_paramBufID);
    vaDestroyBuffer(m_pVADpy, m_qmatrixBufID);
    vaDestroyBuffer(m_pVADpy, m_slice_paramBufID);
    vaDestroyBuffer(m_pVADpy, m_huffmantableBufID);
    vaDestroyBuffer(m_pVADpy, m_codedbufBufID);
    vaDestroyBuffer(m_pVADpy, m_packed_raw_header_paramBufID);
    vaDestroyBuffer(m_pVADpy, m_packed_raw_headerBufID);
    vaDestroyContext(m_pVADpy,m_contextID);
    vaDestroyConfig(m_pVADpy, m_configID);
}

int JpegEncoder::GetVASurfaceAttrib(
      VASurfaceAttrib  fourcc[],
      unsigned int & surface_format
      )
{
   if (fourcc != NULL)
   {
      std::copy(m_fourcc, m_fourcc+2, fourcc);

      surface_format = m_vaSurfaceType;
   }
}

int JpegEncoder::CreateSurfaces(int picture_width, int picture_height, int
      picture_pitch, unsigned char *gray_surface, VASurfaceID *OutVASurfaceID)
{
   VAStatus va_status;
   VASurfaceAttrib attrib_list[4];
   VASurfaceAttribExternalBuffers vaSurfaceExternalBuf;
   unsigned int attrib_count = 2;

   std::copy(m_fourcc, m_fourcc+2, attrib_list);

   va_status = vaCreateSurfaces(m_pVADpy, m_vaSurfaceType, picture_width,
         picture_height, OutVASurfaceID, 1, &attrib_list[0], attrib_count);
   CHECK_VASTATUS(va_status, "vaCreateSurfaces");

   /* 
    * Input is gray scale image and doesn't require RGBtoYUV444
    * conversion.  Upload to GPU memory to run JPEG encoder directly 
    */
   if (gray_surface != NULL)
   {
      VAImage surface_image;
      void *surface_p = NULL;
      unsigned char *y_dst, *y_src;

      va_status = vaDeriveImage(m_pVADpy, *OutVASurfaceID, &surface_image);
      CHECK_VASTATUS(va_status, "vaDeriveImage");

      vaMapBuffer(m_pVADpy, surface_image.buf, &surface_p);
      assert(VA_STATUS_SUCCESS == va_status);

      y_dst = (unsigned char *)surface_p + surface_image.offsets[0];
      y_src = gray_surface;

      for (int row = 0; row < surface_image.height; row++)
      {
         memcpy(y_dst, y_src, surface_image.width);
         y_dst += surface_image.pitches[0];
         y_src += picture_width;
      }

      vaUnmapBuffer(m_pVADpy, surface_image.buf);
      vaDestroyImage(m_pVADpy, surface_image.image_id);
   }

   return va_status;

}

int JpegEncoder::PreRun(
      VASurfaceID    inputSurfID,
      int            nPicWidth,
      int            nPicHeight,
      int            quality,
      int            frameSize
      )
{
   VASurfaceAttrib fourcc[2];
   VAStatus va_status;
   VAEncPictureParameterBufferJPEG pic_param;  /* Picture parameter buffer */
   VAEncSliceParameterBufferJPEG slice_param;  /* Slice parameter buffer */
   VAQMatrixBufferJPEG quantization_param;     /* Quantization Matrix buffer */
   VAHuffmanTableBufferJPEGBaseline hufftable_param; /* Huffmantable buffer */
   YUVComponentSpecs yuvComponent;

   /* Clamp the quality factor value to [1,100] */
   if (quality >= 100) quality=100;
   if (quality <= 0) quality=1;

   init_yuv_component(&yuvComponent, &m_fourcc[0], m_vaSurfaceType);

   /* Create Config for the profile=VAProfileJPEGBaseline, entrypoint=VAEntrypointEncPicture,
    * with RT format attribute */
   va_status = vaCreateConfig(m_pVADpy, VAProfileJPEGBaseline, VAEntrypointEncPicture,
                              &m_attrib[0], 2, &m_configID);
   CHECK_VASTATUS(va_status, "vaCreateConfig");

   /* Create Context for the encode pipe */
   va_status = vaCreateContext(m_pVADpy, m_configID, nPicWidth, nPicHeight,
         VA_PROGRESSIVE, &inputSurfID, 1, &m_contextID);
   CHECK_VASTATUS(va_status, "vaCreateContext");

   /* Create buffer for encoded data to be stored */
   va_status = vaCreateBuffer(m_pVADpy, m_contextID, VAEncCodedBufferType,
         frameSize, 1, NULL, &m_codedbufBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAEncCodedBufferType");

   /* Initialize the picture parameter buffer */
   pic_param.coded_buf = m_codedbufBufID;
   jpegenc_pic_param_init(&pic_param, nPicWidth, nPicHeight, quality, yuvComponent);

   va_status = vaCreateBuffer(m_pVADpy, m_contextID, VAEncPictureParameterBufferType,
         sizeof(VAEncPictureParameterBufferJPEG), 1, &pic_param, &m_pic_paramBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAEncPictureParameterBufferType");

   /* Load the Quatization Matrix */
   jpegenc_qmatrix_init(&quantization_param, yuvComponent);

   va_status = vaCreateBuffer(m_pVADpy, m_contextID, VAQMatrixBufferType,
         sizeof(VAQMatrixBufferJPEG), 1, &quantization_param, &m_qmatrixBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAQMatrixBufferType");

   /* Load the Huffman Tables */
   jpegenc_hufftable_init(&hufftable_param, yuvComponent);

   va_status = vaCreateBuffer(m_pVADpy, m_contextID, VAHuffmanTableBufferType,
         sizeof(VAHuffmanTableBufferJPEGBaseline), 1, &hufftable_param, &m_huffmantableBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAHuffmanTableBufferType");

   /* Initialize slice parameter buffer */
   jpegenc_slice_param_init(&slice_param, yuvComponent);

   va_status = vaCreateBuffer(m_pVADpy, m_contextID, VAEncSliceParameterBufferType,
         sizeof(slice_param), 1, &slice_param, &m_slice_paramBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAEncSliceParameterBufferType");

   /* Pack headers and send using Raw data buffer */
   VAEncPackedHeaderParameterBuffer packed_header_param_buffer;
   unsigned int length_in_bits;
   unsigned char *packed_header_buffer = NULL;

   length_in_bits = build_packed_jpeg_header_buffer(&packed_header_buffer,
         yuvComponent, nPicWidth, nPicHeight, slice_param.restart_interval,
         quality);
   packed_header_param_buffer.type = VAEncPackedHeaderRawData;
   packed_header_param_buffer.bit_length = length_in_bits;
   packed_header_param_buffer.has_emulation_bytes = 0;

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VAEncPackedHeaderParameterBufferType,
         sizeof(packed_header_param_buffer), 1, &packed_header_param_buffer,
         &m_packed_raw_header_paramBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAEncPackedHeaderParameterBufferType");

   va_status = vaCreateBuffer(m_pVADpy, m_contextID,
         VAEncPackedHeaderDataBufferType,
         (length_in_bits + 7) / 8, 1, packed_header_buffer,
         &m_packed_raw_headerBufID);
   CHECK_VASTATUS(va_status, "vaCreateBuffer for VAEncPackedHeaderDataBufferType");
}

int JpegEncoder::Run(VASurfaceID inputSurfID)
{
   VAStatus va_status;

   /* Begin picture */
   va_status = vaBeginPicture(m_pVADpy, m_contextID, inputSurfID);
   CHECK_VASTATUS(va_status, "vaBeginPicture");

   /* Render picture for all the VA buffers created */
   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_pic_paramBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_qmatrixBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_huffmantableBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_slice_paramBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_packed_raw_header_paramBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaRenderPicture(m_pVADpy, m_contextID, &m_packed_raw_headerBufID, 1);
   CHECK_VASTATUS(va_status, "vaRenderPicture");

   va_status = vaEndPicture(m_pVADpy, m_contextID);
   CHECK_VASTATUS(va_status, "vaEndPicture");

   va_status = vaSyncSurface(m_pVADpy, inputSurfID);
   CHECK_VASTATUS(va_status, "vaSyncSurface");

}

int JpegEncoder::GetCodedBufferAddress(VASurfaceID inputSurfID, unsigned char *& coded_mem, unsigned int &slice_length)
{
   VAStatus va_status;
   VACodedBufferSegment *coded_buffer_segment;

   va_status = vaMapBuffer(m_pVADpy, m_codedbufBufID, (void **)(&coded_buffer_segment));
   CHECK_VASTATUS(va_status, "vaMapBuffer");

   coded_mem = (unsigned char*) coded_buffer_segment->buf;

   if (coded_buffer_segment->status & VA_CODED_BUF_STATUS_SLICE_OVERFLOW_MASK)
   {
      vaUnmapBuffer(m_pVADpy, m_codedbufBufID);
      printf("ERROR......Coded buffer too small\n");
   }

   slice_length = coded_buffer_segment->size;
}

int JpegEncoder::WriteOut(VASurfaceID inputSurfID, const char* filename)
{
   VAStatus va_status;
   size_t w_items;
   VACodedBufferSegment *coded_buffer_segment;
   unsigned char *coded_mem;
   unsigned int slice_data_length;

   GetCodedBufferAddress(inputSurfID, coded_mem, slice_data_length);

   FILE *jpeg_fp;
   jpeg_fp = fopen(filename, "wb");

   if (jpeg_fp != NULL)
   {
      do {
         w_items = fwrite(coded_mem, slice_data_length, 1, jpeg_fp);
      } while (w_items != 1);
      fclose(jpeg_fp);
   }

   vaUnmapBuffer(m_pVADpy, m_codedbufBufID);
}

