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

#include <cm/cm.h>

#define FEATUREVECTORPERGROUP   8
#define VECTORPERGROUP          8
#define VECTORLENGTH            80

////////////////////////////////////////////////////////////////////////////
// Find SAD between the feature vector group and database vectors
//
extern "C" _GENX_MAIN_ void
vcaOpSAD_Ex(SurfaceIndex FVSI,      // Feature vector data surface index
            SurfaceIndex DBSI,      // Database vector data surface index
            SurfaceIndex SADSI,     // SAD value surface index
            int nGlobalStep,        // Jump step between database vector group
            int nVector,            // Total number of database vectors
            int nFVGroup) {         // Total number of feature vector groups
  int  srcX, blkX, ix, wr_pos, iFV, fvX, wr_pitch;
  // Feature vector group
  vector<uchar, FEATUREVECTORPERGROUP*VECTORLENGTH> FeatureVector;
  matrix_ref<uchar, FEATUREVECTORPERGROUP, VECTORLENGTH> FeatureVector_2D =
      FeatureVector.format<uchar, FEATUREVECTORPERGROUP, VECTORLENGTH>();
  vector<uchar, VECTORPERGROUP * VECTORLENGTH> SrcData;
  matrix_ref<uchar, VECTORPERGROUP, VECTORLENGTH> SrcData_2D =
      SrcData.format<uchar, VECTORPERGROUP, VECTORLENGTH>();
  matrix<unsigned short, VECTORPERGROUP, FEATUREVECTORPERGROUP> dstSAD;
  // Buffered last reduced rows of SAD values
  matrix<unsigned short, 8,16> LastRow_SAD;
  // Buffered last reduced rows of SAD values
  vector_ref<unsigned short, 128> LastRow_SAD_ex =
      LastRow_SAD.format<unsigned short>();

  int tid = get_thread_origin_x();
  srcX = tid * VECTORPERGROUP;
  wr_pitch = nFVGroup * FEATUREVECTORPERGROUP * 2;
  // Loop through entire database until all done.
loopDB_Ex:
  blkX = srcX  * VECTORLENGTH;
  wr_pos = srcX * nFVGroup * FEATUREVECTORPERGROUP * 2;
  // Loading database vector
  // load 8 vectors
#pragma unroll
  for (ix = 0; ix < VECTORPERGROUP * VECTORLENGTH; ix += 128, blkX += 128) {
    read(DBSI, blkX, SrcData.select<128,1>(ix));
  }

  fvX = 0;
  for (iFV = 0; iFV < nFVGroup; iFV++) {
    // load feature vectors
#pragma unroll
    for (ix = 0; ix < FEATUREVECTORPERGROUP * VECTORLENGTH;
        ix += 128, fvX += 128) {
      read(FVSI, fvX, FeatureVector.select<128,1>(ix));
    }

    // Calculate SAD between feature vectors and incoming database vectors
#pragma unroll
    for (int i = 0; i<VECTORPERGROUP; i++) {
#pragma unroll
      for (int j = 0; j<FEATUREVECTORPERGROUP; j++) {
        // first row uses sad2
        LastRow_SAD.select<1,1,16,1>(j,0) =
            cm_sad2<unsigned short>(FeatureVector_2D.select<1,1,16,1>(j,0),
                                    SrcData_2D.select<1,1,16,1>(i,0),
                                    SAT);
#pragma unroll
        // following rows use sada2
        for (int k = 16; k<VECTORLENGTH; k += 16) {
          LastRow_SAD.select<1,1,16,1>(j,0) =
              cm_sada2<unsigned short>(FeatureVector_2D.select<1,1,16,1>(j,k),
                                       SrcData_2D.select<1,1,16,1>(i,k),
                                       LastRow_SAD.select<1,1,16,1>(j,0),
                                       SAT);
        }
      }
      // Reduce all last row SAD to 1 value and store in one row of dstSAD
      LastRow_SAD.select<1,1,8,1>(0,0) = LastRow_SAD.select<2,1,4,4>(0,0)
          + LastRow_SAD.select<2,1,4,4>(0,2);
      LastRow_SAD.select<1,1,8,1>(0,8) = LastRow_SAD.select<2,1,4,4>(2,0)
          + LastRow_SAD.select<2,1,4,4>(2,2);
      LastRow_SAD.select<1,1,8,1>(1,0) = LastRow_SAD.select<2,1,4,4>(4,0)
          + LastRow_SAD.select<2,1,4,4>(4,2);
      LastRow_SAD.select<1,1,8,1>(1,8) = LastRow_SAD.select<2,1,4,4>(6,0)
          + LastRow_SAD.select<2,1,4,4>(6,2);

      LastRow_SAD.select<1,1,16,1>(0,0) = LastRow_SAD.select<2,1,8,2>(0,0)
          + LastRow_SAD.select<2,1,8,2>(0,1);
      dstSAD.select<1,1,8,1>(i,0) = LastRow_SAD.select<1,1,8,2>(0,0)
          + LastRow_SAD.select<1,1,8,2>(0,1);
    }

#pragma unroll
    for (int i = 0; i<VECTORPERGROUP; i++) {
      write(SADSI, wr_pos + i * wr_pitch, dstSAD.format<unsigned short>().select<FEATUREVECTORPERGROUP,1>(i*FEATUREVECTORPERGROUP));
    }
      wr_pos += FEATUREVECTORPERGROUP * 2;
  }
  srcX += nGlobalStep;
  if (srcX < nVector) goto loopDB_Ex;
}

