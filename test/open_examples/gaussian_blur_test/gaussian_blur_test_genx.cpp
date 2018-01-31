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


// number of components per pixel
#define NUM_COMPONENTS 4

// for vertical direction
// number of rows we read in at once
#define NUM_ROWS_PER_ITER 8
// number of columns per thread
#define NUM_COLS_PER_THREAD 8
#define SIMD_SIZE (NUM_COLS_PER_THREAD * NUM_COMPONENTS)

#define CLAMP_TO_EDGE 1


// for horizontal direction
// number of rows per thread
#define NUM_ROWS_PER_THREAD 8
// number of columns we read in at once
#define NUM_COLS_PER_ITER 8

// Each thread processes 32 columns independently
// For now assume height is divisible by 8
extern "C" _GENX_MAIN_ void
gaussianVertical( SurfaceIndex INBUF, SurfaceIndex OUTBUF, int width, int height, float a0, float a1, float a2, float a3, float b1, float b2, float coefp, float coefn )
{

  matrix<uchar, NUM_ROWS_PER_ITER, SIMD_SIZE> image;
  matrix<uchar, NUM_ROWS_PER_ITER, SIMD_SIZE> outImage;
  vector<float, SIMD_SIZE> in;
  vector<float, SIMD_SIZE> out;
  vector<float, SIMD_SIZE> inMinusOne;
  vector<float, SIMD_SIZE> outMinusOne;
  vector<float, SIMD_SIZE> outMinusTwo;

  uint id = get_thread_origin_x();

#if CLAMP_TO_EDGE
  matrix<uchar, 1, SIMD_SIZE> firstRow;
  read( INBUF, id * SIMD_SIZE, 0, firstRow);
  inMinusOne = firstRow;
  inMinusOne *= 1/255.0f;
  outMinusTwo = coefp * inMinusOne;
  outMinusOne = outMinusTwo;
#else
  inMinusOne = 0;
  outMinusOne = 0;
  outMinusTwo = 0;
#endif

  //read in 8 rows at a time
  for( int i = 0; i < height; i += NUM_ROWS_PER_ITER ) {
    read( INBUF, id * SIMD_SIZE, i, image );

    #pragma unroll
    for( unsigned j = 0; j < NUM_ROWS_PER_ITER; j++ ) {
      in = image.row(j);
      in *= 1/255.0f;
      //out = a0 * in + a1 * inMinusOne - b1 * outMinusOne - b2 * outMinusTwo;
      out = a0 * in + a1 * inMinusOne - (b1 * outMinusOne + b2 * outMinusTwo);
      inMinusOne = in;
      outMinusTwo = outMinusOne;
      outMinusOne = out;

      //clamp the value to [0,1]
      out = cm_add<float>(out, 0.0f, SAT);
      outImage.row(j) = out * 255.0f;
    }

    //write back to surface
    write( OUTBUF, id*SIMD_SIZE, i, outImage );
  }

  vector<float, SIMD_SIZE> inPlusOne;
  vector<float, SIMD_SIZE> inPlusTwo;
  vector<float, SIMD_SIZE> outPlusOne;
  vector<float, SIMD_SIZE> outPlusTwo;
  vector<float, SIMD_SIZE> temp;

#if CLAMP_TO_EDGE
  matrix<uchar, 1, SIMD_SIZE> lastRow;
  read( INBUF, id * SIMD_SIZE, height - 1, lastRow );
  inPlusOne = lastRow;
  inPlusOne *= 1/255.0f;
  inPlusTwo = inPlusOne;
  outPlusOne = coefn * inPlusOne;
  outPlusTwo = outPlusOne;
#else
  inPlusOne = 0;
  inPlusTwo = 0;
  outPlusOne = 0;
  outPlusTwo = 0;
#endif

  //read 8 rows at a time, in reverse direction
  for( int i = height - NUM_ROWS_PER_ITER; i >= 0; i -= NUM_ROWS_PER_ITER ) {
    read( INBUF, id * SIMD_SIZE, i, image );
    read( MODIFIED(OUTBUF), id * SIMD_SIZE, i, outImage );

    #pragma unroll
    for( int j = NUM_ROWS_PER_ITER - 1; j >= 0; j-- ) {
      in = image.row(j);
      in *= 1 / 255.0f;
      //temp = a2 * inPlusOne + a3 * inPlusTwo - b1 * outPlusOne - b2 * outPlusTwo;
      temp = a2 * inPlusOne + a3 * inPlusTwo - (b1 * outPlusOne + b2 * outPlusTwo);
      inPlusTwo = inPlusOne;
      inPlusOne = in;
      outPlusTwo = outPlusOne;
      outPlusOne = temp;

      out = outImage.row(j);
      out = cm_add<float>( out * (1/255.0f), temp, SAT );
      outImage.row(j) = out * 255;
    }

    //write back to surface
    write( OUTBUF, id*SIMD_SIZE, i, outImage );
  }
}

extern "C" _GENX_MAIN_ void
transpose( SurfaceIndex INBUF, SurfaceIndex OUTBUF, unsigned id, int width, int height ) {

  matrix<uint, 8, 8> in;
  matrix<uint, 8, 8> out;

  for( int i = 0; i < height; i += 8 ) {
    read( INBUF, id * 32, i, in );
    out.row(0) = in.column(0);
    out.row(1) = in.column(1);
    out.row(2) = in.column(2);
    out.row(3) = in.column(3);
    out.row(4) = in.column(4);
    out.row(5) = in.column(5);
    out.row(6) = in.column(6);
    out.row(7) = in.column(7);

    write( OUTBUF, i * 4, id * 8, out );
  }
}

// Like gaussianVertical, except we process 8 independent rows at once
extern "C" _GENX_MAIN_ void
gaussianHorizontal( SurfaceIndex INBUF, SurfaceIndex OUTBUF, int width, int height, float a0, float a1, float a2, float a3, float b1, float b2, float coefp, float coefn )
{

  matrix<uchar, NUM_ROWS_PER_THREAD, NUM_COLS_PER_ITER * NUM_COMPONENTS> image;
  matrix<uchar, NUM_ROWS_PER_THREAD, NUM_COLS_PER_ITER * NUM_COMPONENTS> outImage;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> in;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> out;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> inMinusOne;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> outMinusOne;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> outMinusTwo;

  uint id = get_thread_origin_x();

#if CLAMP_TO_EDGE
  matrix<uchar, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> firstColumn;
  read( MODIFIED(INBUF), 0, id * NUM_ROWS_PER_THREAD, firstColumn );
  inMinusOne = firstColumn;
  inMinusOne *= 1/255.0f;
  outMinusTwo = coefp * inMinusOne;
  outMinusOne = outMinusTwo;
#else
  inMinusOne = 0;
  outMinusOne = 0;
  outMinusTwo = 0;
#endif

  //read 8 rows at a time
  for( int i = 0; i < width; i += NUM_COLS_PER_ITER ) {
    read( MODIFIED(INBUF), i * NUM_COMPONENTS, id * NUM_ROWS_PER_THREAD, image );

    #pragma unroll
    for( unsigned j = 0; j < NUM_COLS_PER_ITER; j++ ) {

      in = image.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j * NUM_COMPONENTS);
      in *= 1/255.0f;
      //out = a0 * in + a1 * inMinusOne - b1 * outMinusOne - b2 * outMinusTwo;
      out = a0 * in + a1 * inMinusOne - (b1 * outMinusOne + b2 * outMinusTwo);
      inMinusOne = in;
      outMinusTwo = outMinusOne;
      outMinusOne = out;

      //clamp the value to [0,1]
      out = cm_add<float>( out, 0.0f, SAT ) * 255.0f;
      outImage.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j*NUM_COMPONENTS) = out;
    }

    //write back to surface
    write( OUTBUF, i * NUM_COMPONENTS, id * NUM_ROWS_PER_THREAD, outImage );
  }

  //reverse direction
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> inPlusOne;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> inPlusTwo;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> outPlusOne;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> outPlusTwo;
  matrix<float, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> temp;

#if CLAMP_TO_EDGE
  matrix<uchar, NUM_ROWS_PER_THREAD, NUM_COMPONENTS> lastColumn;
  read( MODIFIED(INBUF), width - NUM_COMPONENTS, id * 8, lastColumn );
  inPlusOne = lastColumn;
  inPlusOne *= 1/255.0f;
  inPlusTwo = inPlusOne;
  outPlusOne = coefn * inPlusOne;
  outPlusTwo = outPlusOne;
#else
  inPlusOne = 0;
  inPlusTwo = 0;
  outPlusOne = 0;
  outPlusTwo = 0;
#endif

  for( int i = width - NUM_COLS_PER_ITER; i >= 0; i -= NUM_COLS_PER_ITER ) {
    read( MODIFIED(INBUF), i * NUM_COMPONENTS, id * NUM_ROWS_PER_THREAD, image );
    read( MODIFIED(OUTBUF), i * NUM_COMPONENTS, id * NUM_ROWS_PER_THREAD, outImage );

    #pragma unroll
    for( int j = NUM_COLS_PER_ITER - 1; j >= 0; j-- ) {
      in = image.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j*NUM_COMPONENTS);
      in *= 1/255.0f;
      //temp = a2 * inPlusOne + a3 * inPlusTwo - b1 * outPlusOne - b2 * outPlusTwo;
      temp = a2 * inPlusOne + a3 * inPlusTwo - (b1 * outPlusOne + b2 * outPlusTwo);
      inPlusTwo = inPlusOne;
      inPlusOne = in;
      outPlusTwo = outPlusOne;
      outPlusOne = temp;

      //The mul * 1 forces out to not be coalesced with outImage, so we can use SIMD16
      //operations instead of SIMD4
      out = outImage.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j*NUM_COMPONENTS) * 1.0f;
      //out = outImage.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j*NUM_COMPONENTS);

      out = cm_add<float>( out * (1/255.0f), temp, SAT );
      outImage.select<NUM_ROWS_PER_THREAD, 1, NUM_COMPONENTS, 1>(0, j*NUM_COMPONENTS) = out * 255.0f;
    }

    write( OUTBUF, i * NUM_COMPONENTS, id * 8, outImage );
  }
}

