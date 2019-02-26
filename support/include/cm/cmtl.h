/*
 * Copyright (c) 2019, Intel Corporation
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

#pragma once
#ifndef _CMTL_H_
#define _CMTL_H_

#include <cm/cm.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcm-bounds-check"

// Debug support
#ifdef CMRT_EMU
CM_INLINE void  cm_assert(bool x)
{
    assert(x);
}
#else
#define cm_assert(x)
#endif

// All the CM template library (cmtl) is enclosed in the cmtl namespace
namespace cmtl {
/* ---------------------------------------------------------------------------*/
/* ------------------------- API Definition
 * -----------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* ------------------------- I/O Routines
 * -------------------------------------------------------*/

// A read function with a fix for the border-duplication issue. This issue means
// that when reading a matrix which extends beyond the surface boundary,
// duplication-to-the-right doesn't work as expected for surfaces whose width
// isn't d-word aligned (i.e., a multiple of 4). Vertical duplication (above and
// below) and duplication-to-the-left do work as expected and are not handled by
// this function.
// This function is used internally and is not expected to be used by the end
// user
//
// Parameters:
//   MAT_TYPE:
//      A type whose size is the same as the size of a single surface pixel.
//      Example surface types and MAT_TYPE values:
//          CM_SURFACE_FORMAT_A8:       char
//          CM_SURFACE_FORMAT_V8U8:     short
//          CM_SURFACE_FORMAT_A8R8G8B8: int
//   buf_id:
//      Surface index.
//   x_pos_bytes:
//      X coordinate of position from which to start reading surface, in
//      *bytes*.
//   y_pos_bytes:
//      Y coordinate of position from which to start reading surface, in
//      *bytes*.
//   in:
//      Matrix to fill with values from surface (or duplicated values when asked
//      to read beyond the surface's edge).
//   surface_width_pixels:
//      Surface width in *pixels*. The surface width in bytes will be deduced
//      by multiplying this with sizeof(SUFF_TYPE).
template <typename MAT_TYPE, int R, int C>
void _Read4Borders(SurfaceIndex buf_id, int x_pos_bytes, int y_pos_bytes,
                   matrix_ref<MAT_TYPE, R, C> in, int surface_width_pixels);

template <typename MAT_TYPE, int R, int C>
void _Read4Borders(SurfaceIndex buf_id, int x_pos_bytes, int y_pos_bytes,
                   matrix<MAT_TYPE, R, C> in, int surface_width_pixels);

#ifndef INLINE_READ
template <typename MAT_TYPE, int R, int C>
void Boundaries(SurfaceIndex buf_id, int x_pos_bytes, int y_pos_bytes,
                matrix_ref<MAT_TYPE, R, C> in, int surface_width_pixels,
                int surface_width_bytes);
#endif

// Read a pixel block of size HEIGHT x WIDTH  from surface, all input are in
// pixel sizes. HEIGHT and WIDTH can be any integer
// Note: to enable efficient IO segmentation set the following define (default
// is disabled due
// to CM Compiler bug when using multiple select)
//
// #define IOBLOCK_ENABLE_MULTI_STRIDE
//
// Parameters:
// h_pix_pos    - horizontal offset in pixel size
// v_pix_pos    - vertical offset in pixel size
// matrix_ref   - ref to output matrix
// surfaceWidth - surface width  in pixel size, when set ReadBlock will properly
// pixels across
//                right hand side boundary, for non-dword aligned surfaces)
template <typename T, int HEIGHT, int WIDTH>
void ReadBlock(SurfaceIndex obuf, int h_pix_pos, int v_pix_pos,
               matrix_ref<T, HEIGHT, WIDTH> block, uint surfaceWidth = 0);

template <typename T, int HEIGHT, int WIDTH>
void ReadBlock(SurfaceIndex obuf, CmBufferAttrib buf_attrib, int h_pos,
               int v_pos, matrix_ref<T, HEIGHT, WIDTH> block,
               uint surfaceWidth = 0);

// Write a pixel block of size HEIGHT x WIDTH from surface, all inputs are in
// pixel sizes. HEIGHT and WIDTH can be any integer
// Note: to enable efficient IO segmentation set the following define (default
// is disabled due to CM Compiler bug when using multiple select)
//
// #define IOBLOCK_ENABLE_MULTI_STRIDE
//
// Parameters:
// h_pix_pos - horizontal offset in pixel size
// v_pix_pos - vertical offset in pixel size
template <typename T, int HEIGHT, int WIDTH>
void WriteBlock(SurfaceIndex obuf, int h_pix_pos, int v_pix_pos,
                matrix_ref<T, HEIGHT, WIDTH> block);

/* Read a pixel vector of size WIDTH from linear buffer, all inputs are in pixel
   sizes. WIDTH can be any integer
   h_pix_pos - horizontal offset in pixel size
*/
template <typename T, int WIDTH>
void ReadLinear(SurfaceIndex ibuf, int pix_pos, vector_ref<T, WIDTH> block);

/* Write a pixel vector of size WIDTH to linear buffer, all inputs are in pixel
   sizes. WIDTH can be any integer
   h_pix_pos - horizontal offset in pixel size
*/
template <typename T, int WIDTH>
void WriteLinear(SurfaceIndex obuf, int pix_pos, vector_ref<T, WIDTH> block);

/* ------------------------- Vectorization Routines
 * ---------------------------------------------*/

/* Creates a nighborhood matrix of a 2D Kernel, whereby for each pixel in src,
   nighbor (i,j) is
   located at row(_2D2_1D<KRNL_SZ>(i,j)) "above it".
   Useful to ensure register alignment irrespective of pixel offset (since we
   create copies for every offset),
   src - matrix representing pixel line and rows above / below it that fit into
   the kernel
   dst - output matrix including vectorized kernel nighborhood.
*/
template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ>
void Vectorize2DKRNL(matrix_ref<Tsrc, KRNL_SZ, SIMD_SZ + KRNL_SZ - 1> src,
                     matrix_ref<Tdst, KRNL_SZ *KRNL_SZ, SIMD_SZ> dst);

/* updatea a single "row" into a vectorized kernel segment of KRNL_SZ rows
   src - input vector for update
   dstRow the row segment to update in dst
*/
template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ>
void Vectorize2DKRNLRow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ - 1> src,
                        matrix_ref<Tdst, KRNL_SZ *KRNL_SZ, SIMD_SZ> dst,
                        int dstRow);

/* translates a matrix representation into vector represnetaiton, use when
   accessing elements in a
   vectorized kernel
*/
template <uint KRNL_SZ> int _2Dto1D(uint x, uint y);

/* ------------------------- SLM Routines
 * -------------------------------------------------------*/

/* Dump content of SLM to memory
   slmX - SLM handle
   slmDebugSurface -  linear surface to dump to
   size  - number of bytes to dump
*/
void DumpSLM(uint slmX, SurfaceIndex slmDebugSurface, uint size);

// TODO add API description
template <int N>
void TransposeFromSLM(vector_ref<uint, N * 4> dst, vector_ref<uint, N * 4> src);

// TODO add API description
template <int N>
void TransposeToSLM(vector_ref<uint, N * 4> dst, vector_ref<uint, N * 4> src);

/* ------------------------- iselect Routines
 * -------------------------------------------------------*/

/* The following templates can be used to pack t2 "small" data elements into 1
   "large" data element
   useful when need to track multiple matrix/vectors through iselect. since
   iselect is SIMD-1 for LHS
   e.g. instead of:
    vector<short, SZ> src1, src2;
    matrix<short, SZ> dst1, dst2;
    dst1.iselect(x,y) = src1;  //SZ SIMD-1 operation + 2 ops to calc x,y coord
    dst2.iselect(x,y) = src2   //SZ SIMD-1 operation + 2 ops to calc x,y coord

   use:
    vector<short, SZ> src1, src2;
    vector<uint, SZ> dst;
    matrix<uint, SOME_SIZE, SZ> data;
    Pack<short, uint, SZ>(src1,src2, src); // ~3-4 SIMD-SZ ops
    dst.iselect<uint, SZ>(x,y) = src; //SZ SIMD-1 ops + 2 ops to cal x,y coord
*/

/* unpack 2 "small elements" from 1 larger element
   Note: order of elements must match between Pack and Unpack calls
   parameters:
   dst1, dst2, packed elements
   src - source element to retrieve the data from
*/
template <typename Tpackd, typename Tcompst, int WD>
void Unpack(vector_ref<Tpackd, WD> dst1, vector_ref<Tpackd, WD> dst2,
            vector_ref<Tcompst, WD> src);

#define CMRT_LOC_FIRST 0
#define CMRT_LOC_SECOND 1

/* unpacks  a single "small" element from 1 larger element
   Note: Use CMRT_LOC_FIRST/CMRT_LOC_SECOND defines to specify which element to
   return
   dst, packed elements
   location: Use CMRT_LOC_FIRST/CMRT_LOC_SECOND to specify which element to
   return
   src - source element to retrieve the data from
*/

template <typename Tpackd, typename Tcompst, int WD>
void UnpackSingle(vector_ref<Tpackd, WD> dst, int location,
                  vector_ref<Tcompst, WD> src);

/* pack 2 "small" elements to 1 larger element
   Note: order of elements must match between Pack and Unpack calls
   parameters:
   src1, src2, elements to pack
*/
template <typename Tpackd, typename Tcompst, int WD>
void Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
          vector_ref<Tcompst, WD> dst);

/* ------------------------- CachedStack Routines
 * -------------------------------------------------------*/

/* A multi-channel (SIMD) Stack object, Stack is stored in Linear surface, with
   a GRF based Cache for top of stack elements
   top of stack pointer can be different for different channels.
   with current implement: Push() will update all channels, a masked Pop() is
   used to conditionaly pop values from stack channels.
   See the distance transform kernel in sample directory for a working example
*/

/* CachedStackInit - initializes a CachedStack Object with:
   template params:
   CACHESIZE: size in pixel of Stack cached stored in GRF, currently MUST be:
   CACHESIZE *sizeof(T) % 128 == 0 (preferable for optimal IO)
   T - type of elements stored in stack
   W - number of channels (SIMD) for a single element on stack
   function params:
   context, context_ii - internaly used by CachedStack to maintain cache
   pointers, caller must not modify this params.
   MaxSize - maximum number of elements on stack for a single channel.
   Caller must allocate the linaer surface with size >=
   MaxSize*NumThreads*W*Sizeof(T)
   Optimization note: MaxSize should be aligned to: ( CACHELINE / sizeof(T))
   (CACHELINE = 64 bytes)
*/
template <typename T, int W, uint CACHESIZE>
void CachedStackInit(matrix_ref<short, 3, W> context,
                     matrix_ref<int, 2, W> context_ii, uint MaxSize);

/* CachedStackTop - Returns the element from top of Stack, w/o removing from
   stack:
   function params:
   surf - pointer to linear sruface used for stack
   context, context_ii - internaly used by CachedStack to maintain cache
   pointers, caller must not modify this params.
   stack - user allocated stack cache
   element - returned top element
*/
template <typename T, int W, uint CACHESIZE>
void CachedStackTop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                    matrix_ref<int, 2, W> context_ii,
                    matrix_ref<T, W, CACHESIZE> stack,
                    vector_ref<T, W> element);

/* CachedStackPop - Conditionaly Pop the top element form that stack based on
   mask, for channels with a 0 mask
   pixel element is returned to caller but not Poped (see CacheStackTop)
   function params:
   surf - pointer to linear sruface used for stack
   context, context_ii - internaly used by CachedStack to maintain cache
   pointers, caller must not modify this params.
   stack - user allocated stack cache
   element - returned top element for all channels.
   mask:  a 0-1 mask indicating top elements to Pop() (mask = 1) vs elements to
   Top() (mask = 0). Note: currently only a 0,1 mask is supported

   Note: Caller must avoid Pop() over an empty channels, Caller can use the
   CacheStackEmpty() to set the mask , e,g.
   vector<short, W> isEmpty;
   vector<short, W> appMask; CallerSetMask(mask);
   CachedStackEmpty<...>(..., isEmpty);
   mask *= (1-isEmpty);
   CachedStackPop<..>(..., mask);
*/
template <typename T, int W, uint CACHESIZE>
void CachedStackPop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                    matrix_ref<int, 2, W> context_ii,
                    matrix_ref<T, W, CACHESIZE> stack, vector_ref<T, W> element,
                    vector_ref<short, W> mask);

/* CachedStackPush - Push an element to top of Stack
   function params:
   surf - pointer to linear sruface used for stack
   context, context_ii - internaly used by CachedStack to maintain cache
   pointers, caller must not modify this params.
   stack - user allocated stack cache
   element - element to push.

   Note: Caller must not  Push() over a full stack (top element at MaxSize -1)
*/
template <typename T, int W, uint CACHESIZE>
void CachedStackPush(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                     matrix_ref<int, 2, W> context_ii,
                     matrix_ref<T, W, CACHESIZE> stack,
                     vector_ref<T, W> element);

template <int W>
void CachedStackEmpty(matrix_ref<short, 2, W> context,
                      matrix_ref<int, 2, W> context_ii,
                      vector_ref<short, W> isEmpty);

/* ---------------Cached Stack with multiple logical stacks Routines
 * ----------------------------*/

/* Combine multiple logical stack storage into a single stack object, for
   efficient I/O and stack accounting
   See DistanceTransform example for usage.
   STACKCOUNT: template variable specifies the number of logical stack storage
   to combine.
   CACHESIZE: size in pixel of Stack cached stored in GRF, currently MUST be:
   (CACHESIZE *sizeof(T)*STACKCOUNT) % 128 == 0 (preferable for optimal IO)
   MaxSize - maximum number of elements on stack for a single channel.
   Caller must allocate the linaer surface with size >=
   MaxSize*NumThreads*W*Sizeof(T)*STACKCOUNT
   Optimization note: MaxSize should be aligned to: ( CACHELINE /
   (sizeof(T)*STACKCOUNT)  (CACHELINE = 64 bytes)
   All other function parameters and function behavior is identical with API
   above.
*/

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void CachedStackInit(matrix_ref<short, 2, W *STACKCOUNT> context,
                     matrix_ref<int, 2, W> context_ii, uint MaxSize);

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void CachedStackTop(SurfaceIndex surf,
                    matrix_ref<short, 2, W *STACKCOUNT> context,
                    matrix_ref<int, 2, W> context_ii,
                    matrix_ref<T, W, CACHESIZE *STACKCOUNT> stack,
                    matrix_ref<T, STACKCOUNT, W> element);

/* retrieve element only from the specified stack index */
template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void CachedStackTop(SurfaceIndex surf,
                    matrix_ref<short, 2, W *STACKCOUNT> context,
                    matrix_ref<int, 2, W> context_ii,
                    matrix_ref<T, W, CACHESIZE *STACKCOUNT> stack,
                    vector_ref<T, W> element, int index);

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void
CachedStackPop(SurfaceIndex surf, matrix_ref<short, 2, W *STACKCOUNT> context,
               matrix_ref<int, 2, W> context_ii,
               matrix_ref<T, W, CACHESIZE *STACKCOUNT> stack,
               matrix_ref<T, STACKCOUNT, W> element, vector_ref<short, W> mask);

/* pop but don't return top elements */
template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void CachedStackPop(SurfaceIndex surf,
                    matrix_ref<short, 2, W *STACKCOUNT> context,
                    matrix_ref<int, 2, W> context_ii,
                    matrix_ref<T, W, CACHESIZE *STACKCOUNT> stack,
                    vector_ref<short, W> mask);

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
void CachedStackPush(SurfaceIndex surf,
                     matrix_ref<short, 2, W *STACKCOUNT> context,
                     matrix_ref<int, 2, W> context_ii,
                     matrix_ref<T, W, CACHESIZE *STACKCOUNT> stack,
                     matrix_ref<T, STACKCOUNT, W> element);

template <int W, uint STACKCOUNT>
void CachedStackEmpty(matrix_ref<short, 2, W *STACKCOUNT> context,
                      matrix_ref<int, 2, W> context_ii,
                      vector_ref<short, W> isEmpty);

/* ------------------------- Matrix transform Routines
 * ------------------------------------------*/

/* Symmetries */
template <typename T, int H, int W>
void MirrorVertical(matrix_ref<T, H, W> in, matrix_ref<T, H, W> out);
template <typename T>
void MirrorHorizontal_16x16(matrix_ref<T, 16, 16> in,
                            matrix_ref<T, 16, 16> out);
template <typename T>
void Rotate90_16x16(matrix_ref<T, 16, 16> in, matrix_ref<T, 16, 16> out);
template <typename T>
void Rotate270_16x16(matrix_ref<T, 16, 16> in, matrix_ref<T, 16, 16> out);
template <typename T>
void Rotate180_16x16(matrix_ref<T, 16, 16> in, matrix_ref<T, 16, 16> out);
template <typename T>
void Transpose_16x16(matrix_ref<T, 16, 16> in, matrix_ref<T, 16, 16> out);
template <typename T>
void ReverseTranspose_16x16(matrix_ref<T, 16, 16> in,
                            matrix_ref<T, 16, 16> out);

template <typename T>
void Transpose_8x8(matrix_ref<T, 8, 8> in, matrix_ref<T, 8, 8> out);

/* Generic Matrix mapping  routines. e.g. diagonals --> rows
   the mapping vector provides the source for mapping for each pixel, for
   function T(x) --> T(mapping(x)) = x
   e.g. mapping for 2x2 90 deg clockwise turn:
   mapping = { 2,0,3,1}
*/
template <typename T, int W>
void Map(matrix_ref<T, W, W> in, matrix_ref<T, W, W> out,
         vector_ref<ushort, W *W> mapping);

/* ------------------------- Init / Assignment Routines
 * ----------------------------------*/

template <typename T, uint Size>
CM_INLINE void cm_vector_assign(vector_ref<T, Size> v, int InitValue, int Step);

/* Note: The following macros are not scoped in the cmtl namespace since they
 * are macros! */
#define cm_matrix(M, T, R, C, I, S)                                            \
  matrix<T, R, C> M(cmtl::__CM_init_array_0_7);                                \
  cmtl::__CM_vector_init<T, R *C, I, S>(M.format<T>());

#define cm_vector(V, T, N, I, S)                                               \
  vector<T, N> V(cmtl::__CM_init_array_0_7);                                   \
  cmtl::__CM_vector_init<T, N, I, S>(V);

/* ------------------------- Extended Math Routines
 * ----------------------------------------------*/

#define CM_CONST_E 2.71828f
#define CM_CONST_PI 3.14159f
#define CM_CONST_2PI 6.28318f

#define CM_DBL_EPSILON                                                         \
  0.00001f /* smallest such that 1.0+CM_DBL_EPSILON != 1.0 */

template <typename RT, int R, int C>
CM_INLINE vector<RT, R *C> cm_floor(const matrix<float, R, C> src0,
                                    const uint flags = 0) {
  return cm_rndd<RT, R *C>(src0, flags);
}
template <typename RT, int R, int C>
CM_INLINE vector<RT, R *C> cm_floor(const matrix_ref<float, R, C> src0,
                                    const uint flags = 0) {
  return cm_rndd<RT, R *C>(src0, flags);
}

template <typename RT, int SZ>
CM_INLINE vector<RT, SZ> cm_floor(const vector<float, SZ> src0,
                                  const uint flags = 0) {
  return cm_rndd<RT, SZ>(src0, flags);
}
template <typename RT, int SZ>
CM_INLINE vector<RT, SZ> cm_floor(const vector_ref<float, SZ> src0,
                                  const uint flags = 0) {
  return cm_rndd<RT, SZ>(src0, flags);
}

template <typename RT>
CM_INLINE RT cm_floor(const float &src0, const uint flags = 0) {
  return cm_rndd<RT, 1U>(src0, flags)(0);
}

template <typename RT, int R, int C>
CM_INLINE vector<RT, R *C> cm_ceil(const matrix<float, R, C> src0,
                                   const uint flags = 0) {
  return cm_rndu<RT, R *C>(src0, flags);
}
template <typename RT, int R, int C>
CM_INLINE vector<RT, R *C> cm_ceil(const matrix_ref<float, R, C> src0,
                                   const uint flags = 0) {
  return cm_rndu<RT, R *C>(src0, flags);
}

template <typename RT, int SZ>
CM_INLINE vector<RT, SZ> cm_ceil(const vector<float, SZ> src0,
                                 const uint flags = 0) {
  return cm_rndu<RT, SZ>(src0, flags);
}
template <typename RT, int SZ>
CM_INLINE vector<RT, SZ> cm_ceil(const vector_ref<float, SZ> src0,
                                 const uint flags = 0) {
  return cm_rndu<RT, SZ>(src0, flags);
}

template <typename RT>
CM_INLINE RT cm_ceil(const float &src0, const uint flags = 0) {
  return cm_rndu<RT, 1U>(src0, flags);
}

/* cm_atan2_fast - a fast atan2 implementation */
/* matrix input */
template <int R, int C>
matrix<float, R, C> cm_atan2_fast(matrix<float, R, C> y, matrix<float, R, C> x,
                                  const uint flags = 0);
/* Matrix_ref input */
template <int R, int C>
matrix<float, R, C> cm_atan2_fast(matrix_ref<float, R, C> y,
                                  matrix_ref<float, R, C> x,
                                  const uint flags = 0);
/* vector input */
template <int N>
vector<float, N> cm_atan2_fast(vector<float, N> y, vector<float, N> x,
                               const uint flags = 0);
/* vector_ref input */
template <int N>
vector<float, N> cm_atan2_fast(vector_ref<float, N> y, vector_ref<float, N> x,
                               const uint flags = 0);
/* scalar input */
template <typename T> float cm_atan2_fast(T y, T x, const uint flags = 0);

/* cm_atan2 - atan2 implementation */
/* For matrix input */
template <int R, int C>
matrix<float, R, C> cm_atan2(matrix<float, R, C> y, matrix<float, R, C> x,
                             const uint flags = 0);
/* matrix_ref input */
template <int R, int C>
matrix<float, R, C> cm_atan2(matrix_ref<float, R, C> y,
                             matrix_ref<float, R, C> x, const uint flags = 0);
/* For Vector input */
template <int N>
vector<float, N> cm_atan2(vector<float, N> y, vector<float, N> x,
                          const uint flags = 0);
/* vector_ref input */
template <int N>
vector<float, N> cm_atan2(vector_ref<float, N> y, vector_ref<float, N> x,
                          const uint flags = 0);
/* scalar Input */
template <typename T> float cm_atan2(T y, T x, const uint flags = 0);

/* cm_fmod: */
/* matrix input */
template <int R, int C>
matrix<float, R, C> cm_fmod(matrix<float, R, C> y, matrix<float, R, C> x,
                            const uint flags = 0);
/* matrix_ref input */
template <int R, int C>
matrix<float, R, C> cm_fmod(matrix_ref<float, R, C> y,
                            matrix_ref<float, R, C> x, const uint flags = 0);
/* vector input */
template <int N>
vector<float, N> cm_fmod(vector<float, N> y, vector<float, N> x,
                         const uint flags = 0);
/* vector_ref input */
template <int N>
vector<float, N> cm_fmod(vector_ref<float, N> y, vector_ref<float, N> x,
                         const uint flags = 0);
/* scalar Input */
template <typename T> float cm_fmod(T y, T x, const uint flags = 0);

/* cm_sin_emu - EU emulation for sin(x) */
/* For matrix input */
template <int R, int C>
matrix<float, R, C> cm_sin_emu(matrix<float, R, C> x, const uint flags = 0);
/* matrix_ref input */
template <int R, int C>
matrix<float, R, C> cm_sin_emu(matrix_ref<float, R, C> x, const uint flags = 0);
/* For Vector input */
template <int N>
vector<float, N> cm_sin_emu(vector<float, N> x, const uint flags = 0);
/* vector_ref input */
template <int N>
vector<float, N> cm_sin_emu(vector_ref<float, N> x, const uint flags = 0);
/* scalar Input */
template <typename T> float cm_sin_emu(T x, const uint flags = 0);

/* cm_cos_emu - EU emulation for cos(x) */
/* For matrix input */
template <int R, int C>
matrix<float, R, C> cm_cos_emu(matrix<float, R, C> x, const uint flags = 0);
/* matrix_ref input */
template <int R, int C>
matrix<float, R, C> cm_cos_emu(matrix_ref<float, R, C> x, const uint flags = 0);
/* For Vector input */
template <int N>
vector<float, N> cm_cos_emu(vector<float, N> x, const uint flags = 0);
/* vector_ref input */
template <int N>
vector<float, N> cm_cos_emu(vector_ref<float, N> x, const uint flags = 0);
/* scalar Input */
template <typename T> float cm_cos_emu(T x, const uint flags = 0);


/* ------------------------- Support Routines
 * ---------------------------------------------------*/

template <typename T, int SZ> void cm_assert_range(vector<T, SZ>, float, float);

const ushort __CM_init_array_0_7[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
template <uint RemSize, typename T, uint Size> struct __RemainderInit;
template <typename T, uint Size, int InitValue, int Step>
void __CM_vector_init(vector_ref<T, Size> v);

/* ---------------------------------------------------------------------------*/
/* ------------------------- Implementation
 * -----------------------------------------------------*/
/* ---------------------------------------------------------------------------*/

/* ------------------------- Support functions / template functions
 * -----------------------------*/
/* All the following macros can be used in template instantiations so we have to
   use macros
   rather than template functions */
#define _CMTL_PIXEL_REG(T) (32 / sizeof(T))
#define _CMTL_PIXEL_STRIDE(STRIDE, TYPE) ((STRIDE) / sizeof(TYPE))
#define _CMTL_SAFE_SIZE(SZ) ((SZ) ? (SZ) : 1)
#define _CMTL_PIXEL_MOD(SIZE, TYPE) ((SIZE) % _CMTL_PIXEL_STRIDE(32, TYPE))
#define _CMTL_PIXEL_DELTA(SIZE, TYPE) ((int)SIZE - _CMTL_PIXEL_MOD(SIZE, TYPE))
#define _CMTL_SAFE_PIXEL_MOD(SIZE, TYPE)                                       \
  (_CMTL_SAFE_SIZE(_CMTL_PIXEL_MOD(SIZE, TYPE)))
#define _CMTL_SAFE_PIXEL_DELTA(SIZE, TYPE)                                     \
  (_CMTL_SAFE_SIZE(_CMTL_PIXEL_DELTA(SIZE, TYPE)))
#define _CMTL_PIXEL_MOD_GT(SIZE, STRIDE, TYPE)                                 \
  (int)(_CMTL_PIXEL_MOD(SIZE, TYPE) > _CMTL_PIXEL_STRIDE(STRIDE, TYPE))
#define _CMTL_PIXEL_MOD_LTE(SIZE, STRIDE, TYPE)                                \
  (int)(_CMTL_PIXEL_MOD(SIZE, TYPE) <= _CMTL_PIXEL_STRIDE(STRIDE, TYPE))
#define _CMTL_ROWS_P_LINE(T, LENGTH)                                           \
  (((int)(LENGTH) + _CMTL_PIXEL_REG(T) - 1) / _CMTL_PIXEL_REG(T))

/* ------------------------- I/O Routines
 * -------------------------------------------------------*/
const unsigned int IO_READ = 0;
const unsigned int IO_WRITE = 1;

/***************************** Implementation  *******************************/
// Generates a mask of length SZ consisting of numZeros zeros followed by
// (SZ-numZeros) ones.
template <typename T, int SZ>
CM_INLINE void vectorMaskFirstZerosThenOnes(vector_ref<T, SZ> v,
                                            uint numZeros) {
  v = 1;
  const uint numFullZeroBlocks = numZeros >> 3;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (uint blockIdx = 0; blockIdx < numFullZeroBlocks; blockIdx++) {
    v.template select<8, 1>(blockIdx * 8) = 0;
  }

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (uint remIdx = numFullZeroBlocks * 8; remIdx < numZeros; remIdx++) {
    v(remIdx) = 0;
  }
}

// A read function with a fix for the border-duplication issue. This issue means
// that when reading a matrix which extends beyond the surface boundary,
// duplication-to-the-right doesn't work as expected for surfaces whose width
// isn't d-word aligned (i.e., a multiple of 4). Vertical duplication (above and
// below) and duplication-to-the-left do work as expected and are not handled by
// this function.
//
// Parameters:
//   MAT_TYPE:
//      A type whose size is the same as the size of a single surface pixel.
//      Example surface types and MAT_TYPE values:
//          CM_SURFACE_FORMAT_A8:       char
//          CM_SURFACE_FORMAT_V8U8:     short
//          CM_SURFACE_FORMAT_A8R8G8B8: int
//   buf_id:
//      Surface index.
//   x_pos_bytes:
//      X coordinate of position from which to start reading surface, in
//      *bytes*.
//   y_pos_bytes:
//      Y coordinate of position from which to start reading surface, in
//      *bytes*.
//   in:
//      Matrix to fill with values from surface (or duplicated values when asked
//      to read beyond the surface's edge).
//   surface_width_pixels:
//      Surface width in *pixels*. The surface width in bytes will be deduced
//      by multiplying this with sizeof(SUFF_TYPE).
template <typename MAT_TYPE, int R, int C>
CM_INLINE void _Read4Borders(SurfaceIndex buf_id, int x_pos_bytes,
                             int y_pos_bytes, matrix_ref<MAT_TYPE, R, C> in,
                             int surface_width_pixels) {
  // Compile-time check whether the fix is necessary; if not, we do a simple
  // read and return. The compile-time check has been seperated from the
  // run-time check in the hope that the compiler will just optimize this
  // whole function away if it is unnecessary.
  if (sizeof(MAT_TYPE) % 4 == 0) {
    read(buf_id, x_pos_bytes, y_pos_bytes, in);
    return;
  }

  // Calculate surface width in bytes.
  const int surface_width_bytes = surface_width_pixels * (int)sizeof(MAT_TYPE);

  // Run-time check whether the fix is necessary; if not, we do a simple read
  // and return.
  if (surface_width_bytes % 4 == 0 ||
      x_pos_bytes + (int)(C * sizeof(MAT_TYPE)) <= surface_width_bytes) {
    read(buf_id, x_pos_bytes, y_pos_bytes, in);
    return;
  }

#ifndef INLINE_READ
  Boundaries<MAT_TYPE>(buf_id, x_pos_bytes, y_pos_bytes, in,
                       surface_width_pixels, surface_width_bytes);
}

template <typename MAT_TYPE, int R, int C>
void Boundaries(SurfaceIndex buf_id, int x_pos_bytes, int y_pos_bytes,
                matrix_ref<MAT_TYPE, R, C> in, int surface_width_pixels,
                int surface_width_bytes) {
#endif

  // Modify x_pos_bytes to guarantee that after read matrix will contain at
  // least one valid column.
  if (x_pos_bytes >= surface_width_bytes) {
    x_pos_bytes = surface_width_bytes - sizeof(MAT_TYPE);
  }

  // Read from surface into a MAT_TYPE-matrix of same size as target matrix.
  read(buf_id, x_pos_bytes, y_pos_bytes, in);

  // Calculate the right-most column of the MAT_TYPE-matrix which contains
  // good data. All columns to the right of this column will be set to equal
  // it.
  int last_good_column_index =
      surface_width_pixels - x_pos_bytes / (int)sizeof(MAT_TYPE) - 1;

  // Generate a mask vector whose first (last_good_column_index + 1) values
  // are zero and whose remaining values are one.
  vector<ushort, C> mask;
  vectorMaskFirstZerosThenOnes(mask.template select<C, 1>(0),
                               (uint)last_good_column_index + 1);

  // Going row by row, fix the MAT_TYPE-matrix.
  vector<MAT_TYPE, C> filler;
  for (int i = 0; i < R; i++) {
    filler = in(i, last_good_column_index);
    in.row(i).merge(filler, mask);
  }
}

template <typename MAT_TYPE, int R, int C>
CM_INLINE void _Read4Borders(SurfaceIndex buf_id, int x_pos_bytes,
                             int y_pos_bytes, matrix<MAT_TYPE, R, C> in,
                             int surface_width_pixels) {
  _Read4Borders(buf_id, x_pos_bytes, y_pos_bytes, in.select_all(),
                surface_width_pixels);
}

template <typename T, int HEIGHT, int WIDTH>
CM_INLINE void
_ReadWrapper(SurfaceIndex obuf, CmBufferAttrib buf_attrib, int h_pos, int v_pos,
             matrix_ref<T, HEIGHT, WIDTH> block, uint surfaceWidth) {
  if (buf_attrib == GENX_NUM_BUFFER_ATTRIB)
    _Read4Borders(obuf, h_pos, v_pos, block, surfaceWidth);
  if (buf_attrib != GENX_NUM_BUFFER_ATTRIB)
    // read modified is not supported with replicate bounderies.
    read(obuf, buf_attrib, h_pos, v_pos, block);
}

/*
  Block Width (bytes) Maximum Block Height (rows)
  1-4                    64
  5-8                    32
  9-16                16
  17-32                8
  33-64 {BDW+}        4
*/

template <typename T, int HEIGHT, int WIDTH, uint OP_TYPE>
CM_INLINE void _ioOp(SurfaceIndex iobuf, CmBufferAttrib buf_attrib, int h_pos,
                     int v_pos, matrix_ref<T, HEIGHT, WIDTH> block,
                     uint surfaceWidth) {
  cm_assert(OP_TYPE == IO_READ || OP_TYPE == IO_WRITE);
  if (OP_TYPE == IO_WRITE)
    write(iobuf, h_pos, v_pos, block);
  if (OP_TYPE == IO_READ)
    _ReadWrapper(iobuf, buf_attrib, h_pos, v_pos, block, surfaceWidth);
}

template <typename T, int HEIGHT, int WIDTH, int H_STRIDE, int MAX_V_STRIDE,
          int OP_TYPE>
CM_INLINE void IoFixedStrideBlock(SurfaceIndex iobuf, CmBufferAttrib buf_attrib,
                                  int h_pos, int v_pos,
                                  matrix_ref<T, HEIGHT, WIDTH> block,
                                  uint surfaceWidth) {
  int vblock, hblock;
  int h_pos_abs = sizeof(T) * h_pos;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (vblock = 0; vblock < (int)HEIGHT - (MAX_V_STRIDE - 1);
       vblock += MAX_V_STRIDE) {
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
    for (hblock = 0; hblock < (int)WIDTH - (H_STRIDE - 1); hblock += H_STRIDE) {
      _ioOp<T, MAX_V_STRIDE, H_STRIDE, OP_TYPE>(
          iobuf, buf_attrib, sizeof(T) * hblock + h_pos_abs, v_pos + vblock,
          block.template select<MAX_V_STRIDE, 1, H_STRIDE, 1>(vblock, hblock),
          surfaceWidth);
    }
    if (WIDTH % H_STRIDE) {
      _ioOp<T, MAX_V_STRIDE, _CMTL_SAFE_SIZE(WIDTH % H_STRIDE), OP_TYPE>(
          iobuf, buf_attrib, sizeof(T) * hblock + h_pos_abs, v_pos + vblock,
          block.template select<MAX_V_STRIDE, 1,
                                _CMTL_SAFE_SIZE(WIDTH % H_STRIDE), 1>(vblock,
                                                                      hblock),
          surfaceWidth);
    }
  }
  if (HEIGHT % MAX_V_STRIDE) {
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
    for (hblock = 0; hblock < (int)WIDTH - (H_STRIDE - 1); hblock += H_STRIDE) {
      _ioOp<T, _CMTL_SAFE_SIZE(HEIGHT % MAX_V_STRIDE), H_STRIDE, OP_TYPE>(
          iobuf, buf_attrib, sizeof(T) * hblock + h_pos_abs, v_pos + vblock,
          block.template select<_CMTL_SAFE_SIZE(HEIGHT % MAX_V_STRIDE), 1,
                                H_STRIDE, 1>(vblock, hblock),
          surfaceWidth);
    }
    if (WIDTH % H_STRIDE) {
      _ioOp<T, _CMTL_SAFE_SIZE(HEIGHT % MAX_V_STRIDE),
            _CMTL_SAFE_SIZE(WIDTH % H_STRIDE), OP_TYPE>(
          iobuf, buf_attrib, sizeof(T) * hblock + h_pos_abs, v_pos + vblock,
          block.template select<_CMTL_SAFE_SIZE(HEIGHT % MAX_V_STRIDE), 1,
                                _CMTL_SAFE_SIZE(WIDTH % H_STRIDE), 1>(vblock,
                                                                      hblock),
          surfaceWidth);
    }
  }
}

// TODO add support for double
// TODO support 64 byte stride for BDW +
template <typename T, int HEIGHT, int WIDTH, int OP_TYPE>
CM_INLINE void IoBlock(SurfaceIndex iobuf, CmBufferAttrib buf_attrib, int h_pos,
                       int v_pos, matrix_ref<T, HEIGHT, WIDTH> block,
                       uint surfaceWidth = 0) {
  cm_assert(sizeof(T) <= 4);
  if (_CMTL_PIXEL_DELTA(WIDTH, T) > 0) {
    IoFixedStrideBlock<T, HEIGHT, _CMTL_SAFE_PIXEL_DELTA(WIDTH, T),
                       _CMTL_PIXEL_STRIDE(32, T), 8, OP_TYPE>(
        iobuf, buf_attrib, h_pos, v_pos,
        block.template select<HEIGHT, 1, _CMTL_SAFE_PIXEL_DELTA(WIDTH, T), 1>(
            0, 0),
        surfaceWidth);
    h_pos += _CMTL_PIXEL_DELTA(WIDTH, T);
  }

  // deal with left over
  if (_CMTL_PIXEL_MOD_GT(WIDTH, 16, T)) {
    IoFixedStrideBlock<T, HEIGHT, _CMTL_SAFE_PIXEL_MOD(WIDTH, T),
                       _CMTL_PIXEL_STRIDE(32, T), 8, OP_TYPE>(
        iobuf, buf_attrib, h_pos, v_pos,
        block.template select<HEIGHT, 1, _CMTL_SAFE_PIXEL_MOD(WIDTH, T), 1>(
            0, _CMTL_PIXEL_DELTA(WIDTH, T)),
        surfaceWidth);
  }

  if (_CMTL_PIXEL_MOD_GT(WIDTH, 8, T) && _CMTL_PIXEL_MOD_LTE(WIDTH, 16, T)) {
    IoFixedStrideBlock<T, HEIGHT, _CMTL_SAFE_PIXEL_MOD(WIDTH, T),
                       _CMTL_PIXEL_STRIDE(16, T), 16, OP_TYPE>(
        iobuf, buf_attrib, h_pos, v_pos,
        block.template select<HEIGHT, 1, _CMTL_SAFE_PIXEL_MOD(WIDTH, T), 1>(
            0, _CMTL_PIXEL_DELTA(WIDTH, T)),
        surfaceWidth);
  }

  if (_CMTL_PIXEL_MOD_GT(WIDTH, 4, T) && _CMTL_PIXEL_MOD_LTE(WIDTH, 8, T)) {
    IoFixedStrideBlock<T, HEIGHT, _CMTL_SAFE_PIXEL_MOD(WIDTH, T),
                       _CMTL_PIXEL_STRIDE(8, T), 32, OP_TYPE>(
        iobuf, buf_attrib, h_pos, v_pos,
        block.template select<HEIGHT, 1, _CMTL_SAFE_PIXEL_MOD(WIDTH, T), 1>(
            0, _CMTL_PIXEL_DELTA(WIDTH, T)),
        surfaceWidth);
  }

  if (_CMTL_PIXEL_MOD_GT(WIDTH, 0, T) && _CMTL_PIXEL_MOD_LTE(WIDTH, 4, T)) {
    IoFixedStrideBlock<T, HEIGHT, _CMTL_SAFE_PIXEL_MOD(WIDTH, T),
                       _CMTL_PIXEL_STRIDE(4, T), 64, OP_TYPE>(
        iobuf, buf_attrib, h_pos, v_pos,
        block.template select<HEIGHT, 1, _CMTL_SAFE_PIXEL_MOD(WIDTH, T), 1>(
            0, _CMTL_PIXEL_DELTA(WIDTH, T)),
        surfaceWidth);
  }
}

template <typename T, int HEIGHT, int WIDTH>
CM_INLINE void ReadBlock(SurfaceIndex iobuf, int h_pos, int v_pos,
                         matrix_ref<T, HEIGHT, WIDTH> block,
                         uint surfaceWidth) {
  IoBlock<T, HEIGHT, WIDTH, IO_READ>(iobuf, GENX_NUM_BUFFER_ATTRIB, h_pos,
                                     v_pos, block, surfaceWidth);
}

template <typename T, int HEIGHT, int WIDTH>
CM_INLINE void
ReadBlock(SurfaceIndex iobuf, CmBufferAttrib buf_attrib, int h_pos, int v_pos,
          matrix_ref<T, HEIGHT, WIDTH> block, uint surfaceWidth) {
  cm_assert(buf_attrib != GENX_NUM_BUFFER_ATTRIB);
  IoBlock<T, HEIGHT, WIDTH, IO_READ>(iobuf, buf_attrib, h_pos, v_pos, block,
                                     surfaceWidth);
}

template <typename T, int HEIGHT, int WIDTH>
CM_INLINE void WriteBlock(SurfaceIndex iobuf, int h_pos, int v_pos,
                          matrix_ref<T, HEIGHT, WIDTH> block) {
  IoBlock<T, HEIGHT, WIDTH, IO_WRITE>(iobuf, GENX_NUM_BUFFER_ATTRIB, h_pos,
                                      v_pos, block);
}

template <typename T, int WIDTH, uint OP_TYPE>
CM_INLINE void _ioLinearOp(SurfaceIndex iobuf, int h_pos,
                           vector_ref<T, WIDTH> block) {
  // cm_assert(OP_TYPE == IO_READ || OP_TYPE == IO_WRITE);
  if (OP_TYPE == IO_WRITE)
    write(iobuf, h_pos, block);
  if (OP_TYPE == IO_READ)
    read(MODIFIED(iobuf), h_pos, block); // TODO : need to understand behavior
                                         // of MODIFED flag and when/how to use
                                         // it...
}

#define _CMTL_MAXIOLINEAR(TYPE) (128 / sizeof(TYPE))

template <typename T, int WIDTH, uint PIX_STRD, uint OP_TYPE>
CM_INLINE void IOLinear(SurfaceIndex iobuf, int pix_pos,
                        vector_ref<T, WIDTH> block) {
  int hblock;
  int h_pos_abs = sizeof(T) * pix_pos;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (hblock = 0; hblock < (int)WIDTH - (PIX_STRD - 1); hblock += PIX_STRD) {
    _ioLinearOp<T, PIX_STRD, OP_TYPE>(
        iobuf, sizeof(T) * hblock + h_pos_abs,
        block.template select<PIX_STRD, 1>(hblock));
  }
  if (WIDTH % PIX_STRD) {
    _ioLinearOp<T, _CMTL_SAFE_SIZE(WIDTH % PIX_STRD), OP_TYPE>(
        iobuf, sizeof(T) * hblock + h_pos_abs,
        block.template select<_CMTL_SAFE_SIZE(WIDTH % PIX_STRD), 1>(hblock));
  }
}

template <typename T, int WIDTH>
CM_INLINE void WriteLinear(SurfaceIndex obuf, int pix_pos,
                           vector_ref<T, WIDTH> block) {
  IOLinear<T, WIDTH, _CMTL_MAXIOLINEAR(T), IO_WRITE>(obuf, pix_pos, block);
}

template <typename T, int WIDTH>
CM_INLINE void ReadLinear(SurfaceIndex ibuf, int pix_pos,
                          vector_ref<T, WIDTH> block) {
  IOLinear<T, WIDTH, _CMTL_MAXIOLINEAR(T), IO_READ>(ibuf, pix_pos, block);
}

/* ------------------------- Vectorization Routines
 * -------------------------------------------------------*/
template <uint KRNL_SZ> CM_INLINE int _2Dto1D(int x, int y) {
  return x * KRNL_SZ + y;
}

template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ>
CM_INLINE void
Vectorize2DKRNLRow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ - 1> src,
                   matrix_ref<Tdst, KRNL_SZ *KRNL_SZ, SIMD_SZ> dst,
                   int dstRow) {
  cm_assert(KRNL_SZ % 2 == 1);
  cm_assert(dstRow < KRNL_SZ * KRNL_SZ);
  int radius = KRNL_SZ / 2;
  int anchor = KRNL_SZ / 2;
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int clmn = -radius; clmn < radius + 1; clmn++) {
    dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) =
        src.template select<SIMD_SZ, 1>(clmn + anchor);
  }
}

template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ, int OFFSET>
CM_INLINE void glue_vector(vector_ref<Tsrc, SIMD_SZ> src_1,
                           vector_ref<Tsrc, KRNL_SZ - 1> src_2,
                           vector_ref<Tdst, SIMD_SZ> dst) {
  dst.template select<SIMD_SZ - OFFSET, 1>(0) =
      src_1.template select<SIMD_SZ - OFFSET, 1>(OFFSET);
  dst.template select<OFFSET, 1>(SIMD_SZ - OFFSET) =
      src_2.template select<OFFSET, 1>(0);
}

template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ>
CM_INLINE void
Vectorize2DKRNLRow(/*vector_ref<Tsrc, SIMD_SZ> src,
                          vector_ref<Tsrc, KRNL_SZ -1> src_2,*/
                   matrix_ref<Tsrc,
                              _CMTL_ROWS_P_LINE(Tsrc, SIMD_SZ + KRNL_SZ - 1),
                              _CMTL_PIXEL_REG(Tsrc)> src,
                   matrix_ref<Tdst, KRNL_SZ *KRNL_SZ, SIMD_SZ> dst,
                   int dstRow) {
  cm_assert(KRNL_SZ % 2 == 1);
  cm_assert(dstRow < (int)KRNL_SZ * KRNL_SZ);
  cm_assert(SIMD_SZ % 8 == 0);
  int radius = KRNL_SZ / 2;
  int anchor = KRNL_SZ / 2;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int clmn = -radius; clmn < radius + 1; clmn++) {
    // vector<Tsrc, SIMD_SZ + KRNL_SZ -1> glue;
    vector<Tsrc, _CMTL_ROWS_P_LINE(Tsrc, SIMD_SZ + KRNL_SZ -
                                             1) *_CMTL_PIXEL_REG(Tsrc)> glue =
        src;

    dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) =
        glue.template select<SIMD_SZ, 1>(clmn + anchor);

    /* not working yet, limited to 3x3
    int clmn = -1; //glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,0>(src, src_2,
        dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) = src;

    clmn = 0; glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,1>(src, src_2,
        dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)));

    clmn = 1; glue_vector<Tsrc, Tdst, KRNL_SZ, SIMD_SZ,2>(src, src_2,
        dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)));
    */
  }
}

template <typename Tsrc, typename Tdst, uint KRNL_SZ, uint SIMD_SZ>
CM_INLINE void
Vectorize2DKRNL(matrix_ref<Tsrc, KRNL_SZ, SIMD_SZ + KRNL_SZ - 1> src,
                matrix_ref<Tdst, KRNL_SZ *KRNL_SZ, SIMD_SZ> dst) {
  cm_assert(KRNL_SZ % 2 == 1);

  int radius = KRNL_SZ / 2;
  int anchor = KRNL_SZ / 2;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int row = -radius; row < radius + 1; row++) {

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
    for (int clmn = -radius; clmn < radius + 1; clmn++) {
      dst.row(_2Dto1D<KRNL_SZ>(row + anchor, clmn + anchor)) =
          src.template select<1, 1, SIMD_SZ, 1>(row + anchor, clmn + anchor);
    }
  }
}

template <typename Tsrc, typename Tdst, uint ROWS, uint KRNL_SZ, uint SIMD_SZ>
CM_INLINE void Vectorize2DKRNLRow(vector_ref<Tsrc, SIMD_SZ + KRNL_SZ - 1> src,
                                  matrix_ref<Tdst, ROWS *KRNL_SZ, SIMD_SZ> dst,
                                  int dstRow) {
  cm_assert(KRNL_SZ % 2 == 1);
  cm_assert(dstRow < ROWS * KRNL_SZ);
  int radius = KRNL_SZ / 2;
  int anchor = KRNL_SZ / 2;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int clmn = -radius; clmn < radius + 1; clmn++) {
    dst.row(_2Dto1D<KRNL_SZ>(dstRow, clmn + anchor)) =
        src.template select<SIMD_SZ, 1>(clmn + anchor);
  }
}

template <typename T, uint ROWS, uint KRNL_SZ, uint SIMD_SZ>
CM_INLINE void Vectorize2DKRNLRow(vector_ref<T, SIMD_SZ + KRNL_SZ - 1> src,
                                  matrix_ref<T, ROWS *KRNL_SZ, SIMD_SZ> dst,
                                  int dstRow) {
  Vectorize2DKRNLRow<T, T, ROWS, KRNL_SZ, SIMD_SZ>(src, dst, dstRow);
}

/* ------------------------- SLM Routines
 * -------------------------------------------------------*/

// Addresses are in units of element size. The offset indicate a write location
// of 16 bytes.
static const ushort init_slm_offsets[] = { 0, 1, 2,  3,  4,  5,  6,  7,
                                           8, 9, 10, 11, 12, 13, 14, 15 };

// data read from SLM using the cm_slm_read4 is organized per channel:
// i.e. RGBARGBARGBA ... data in SLM is returned as
// RRR ... GGG ... BB ... AAA ...
template <int N>
CM_INLINE void TransposeFromSLM(vector_ref<uint, N * 4> dst,
                                vector_ref<uint, N * 4> src) {
  cm_assert(N == 8 || N == 16);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < 4; i++)
    dst.template select<N, 4>(i) = src.template select<N, 1>(i * N);
}

// data write to SLM using the cm_slm_write4 should be organized per channel:
// i.e. RGBARGBARGBA ... data should be written to SLM as
// RRR ... GGG ... BB ... AAA ...
template <int N>
CM_INLINE void TransposeToSLM(vector_ref<uint, N * 4> dst,
                              vector_ref<uint, N * 4> src) {
  cm_assert(N == 8 || N == 16);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < 4; i++)
    dst.template select<N, 1>(N *i) = src.template select<N, 4>(i);
}

CM_INLINE void DumpSLM(uint slmX, SurfaceIndex slmDebugSurface, uint size) {
  vector<uint, 16> slm_addresses(init_slm_offsets);
  vector<uint, 64> io_data;
  vector<uint, 64> dst;
  vector<uchar, 256> toOuputSurface;

  slm_addresses *= 4;

  int i;
  for (i = 0; i < size; i += 256) {
    cm_slm_read4(slmX, slm_addresses, io_data, SLM_ABGR_ENABLE);
    TransposeFromSLM<16>(dst, io_data);
    toOuputSurface = dst.format<uchar>();

    write(slmDebugSurface, i, toOuputSurface.select<128, 1>(0));
    write(slmDebugSurface, i + 128, toOuputSurface.select<128, 1>(128));

    slm_addresses += 64;
  }
}

/* ------------------------- iselect Routines
 * -------------------------------------------------------*/
template <typename Tpackd, typename Tcompst, int WD>
CM_INLINE void Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
                    vector_ref<double, WD> dst) {
  vector<Tpackd, WD * 2> temp;

  temp.template select<WD, 2>(0) = src1;
  temp.template select<WD, 2>(1) = src2;
  dst = temp.template format<double>();
}

template <typename T, uint SIMD_SZ>
CM_INLINE void _Unpack_double(vector_ref<T, SIMD_SZ> dst1,
                              vector_ref<T, SIMD_SZ> dst2,
                              vector_ref<double, SIMD_SZ> src) {
  vector<T, SIMD_SZ * 2> temp = src.template format<T>();
  dst1 = temp.template select<SIMD_SZ, 2>(0);
  dst2 = temp.template select<SIMD_SZ, 2>(1);
}

template <typename Tpackd, typename TUpackd, typename Tcompst,
          typename TUcompst, int WD>
CM_INLINE void _Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
                     vector_ref<Tcompst, WD> dst) {
  vector<TUpackd, WD> usrc1 = src1.template format<TUpackd, WD, 1>();
  vector<TUpackd, WD> usrc2 = src2.template format<TUpackd, WD, 1>();

  vector<TUcompst, WD> udst = (usrc2 << (sizeof(Tpackd) * 8)) | usrc1;
  dst = udst.template format<Tcompst, WD, 1>();
}

template <typename Tpackd, typename Tcompst, int WD>
CM_INLINE void Unpack(vector_ref<Tpackd, WD> dst1, vector_ref<Tpackd, WD> dst2,
                      vector_ref<Tcompst, WD> src) {
  cm_assert(sizeof(Tpackd) * 2 <= sizeof(Tcompst));

  vector<Tpackd, WD * 2> src_cast = src.template format<Tpackd, WD * 2, 1>();
  dst1 = src_cast.template select<WD, 2>(CMRT_LOC_FIRST); // LSB ordered
  dst2 = src_cast.template select<WD, 2>(CMRT_LOC_SECOND);
}

template <typename Tpackd, typename Tcompst, int WD>
CM_INLINE void UnpackSingle(vector_ref<Tpackd, WD> dst, int location,
                            vector_ref<Tcompst, WD> src) {
  cm_assert(location == CMRT_LOC_FIRST || location == CMRT_LOC_SECOND);
  vector<Tpackd, WD * 2> src_cast = src.template format<Tpackd, WD * 2, 1>();
  dst = src_cast.template select<WD, 2>(location);
}

template <typename Tpackd, typename Tcompst, int WD>
CM_INLINE void Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
                    vector_ref<Tcompst, WD> dst) {
  cm_assert(sizeof(Tpackd) * 2 <= sizeof(Tcompst));

  if ((sizeof(Tpackd) == 1) && sizeof(Tcompst) == 2)
    _Pack<Tpackd, uchar, Tcompst, ushort, WD>(src1, src2, dst);
  if ((sizeof(Tpackd) == 1) && sizeof(Tcompst) == 4)
    _Pack<Tpackd, uchar, Tcompst, uint, WD>(src1, src2, dst);
  if ((sizeof(Tpackd) == 2) && sizeof(Tcompst) == 4)
    _Pack<Tpackd, ushort, Tcompst, uint, WD>(src1, src2, dst);
}

/* ------------------------- CachedStack Routines
 * ---------------------------------------------------*/
#define _CMTL_CSTK_K 0
#define _CMTL_CSTK_CHNL 1

#define _CMTL_CNTXT_SIZE 2, W *STACKCOUNT
#define _CMTL_CNTXT_SIZE_1 (2, W)
#define _CMTL_STACK_SIZE W, CACHESIZE *STACKCOUNT

#define _CMTL_CNTX_II_OFFSET_DYNAMIC 0
#define _CMTL_CNTX_II_OFFSET_INITIAL 1

#define _CMTL_CSTK_DECIPHER_CONTEXT(cntxt)                                     \
  vector_ref<short, W> k = cntxt.row(_CMTL_CSTK_K).template select<W, 1>(0);   \
  vector_ref<short, W *STACKCOUNT> chnls_ = cntxt.row(_CMTL_CSTK_CHNL);        \
  vector_ref<short, W *STACKCOUNT> k_dup = cntxt.row(_CMTL_CSTK_K);

#define _CMTL_NUM_IO(T) ((CACHESIZE *STACKCOUNT) / _CMTL_MAXIOLINEAR(T))

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void
_CSTK_setOffsets(matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                 vector_ref<int, W> surf_pos,
                 matrix_ref<int, _CMTL_NUM_IO(T), W> surf_offset) {
  cm_assert((CACHESIZE * STACKCOUNT) % _CMTL_MAXIOLINEAR(T) == 0);
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  surf_offset.row(0) = surf_pos;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int offset = 1; offset < (int)_CMTL_NUM_IO(T); offset++) {
    surf_offset.row(offset) =
        surf_offset.row(offset - 1) + _CMTL_MAXIOLINEAR(T) * sizeof(T);
  }
}

template <int W, uint STACKCOUNT>
CM_INLINE void _CSTKAssert(matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                           matrix_ref<int, 2, W> context_ii) {
  vector<short, W> empty;
  CachedStackEmpty<W, STACKCOUNT>(context, context_ii, empty);
  cm_assert(empty.any() == false);
  // TODO add check for full stack
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void
_CSTKWriteBuffers(SurfaceIndex surf,
                  matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                  matrix_ref<int, 2, W> context_ii,
                  matrix_ref<T, _CMTL_STACK_SIZE> stack) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  vector_ref<int, W> surf_pos = context_ii.row(_CMTL_CNTX_II_OFFSET_DYNAMIC);
  matrix<int, _CMTL_NUM_IO(T), W> surf_offset;

  _CSTK_setOffsets<T, W, CACHESIZE, STACKCOUNT>(context, surf_pos, surf_offset);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int chnl = 0; chnl < W; chnl++) {
    if (k(chnl) == CACHESIZE) {
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
      for (int s = 0; s < (int)_CMTL_NUM_IO(T); s++) {
        vector_ref<T, _CMTL_MAXIOLINEAR(T)> temp =
            stack.row(chnl).template select<_CMTL_MAXIOLINEAR(T), 1>(
                s * _CMTL_MAXIOLINEAR(T));
        write(surf, surf_offset(s, chnl), temp);
      }
    }
  }

  surf_pos.merge(surf_pos + CACHESIZE * STACKCOUNT * sizeof(T),
                 (k == CACHESIZE));
  k.merge(0, k == CACHESIZE);
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void _CSTKReadBuffers(SurfaceIndex surf,
                                matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                                matrix_ref<int, 2, W> context_ii,
                                matrix_ref<T, _CMTL_STACK_SIZE> stack) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  vector_ref<int, W> surf_pos = context_ii.row(_CMTL_CNTX_II_OFFSET_DYNAMIC);

  surf_pos.merge(surf_pos - CACHESIZE * STACKCOUNT * sizeof(T), k == -1);

  matrix<int, _CMTL_NUM_IO(T), W> surf_offset;
  _CSTK_setOffsets<T, W, CACHESIZE, STACKCOUNT>(context, surf_pos, surf_offset);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int chnl = 0; chnl < W; chnl++) {
    if (k(chnl) == -1) {
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
      for (int s = 0; s < (int)_CMTL_NUM_IO(T); s++) {
        vector_ref<T, _CMTL_MAXIOLINEAR(T)> temp =
            stack.row(chnl).template select<_CMTL_MAXIOLINEAR(T), 1>(
                s * _CMTL_MAXIOLINEAR(T));
        read(MODIFIED(surf), surf_offset(s, chnl), temp);
      }
    }
  }
  k.merge(CACHESIZE - 1, k == -1);
  _CSTKAssert<W, STACKCOUNT>(context, context_ii);
}

const static int _CSTK_c_channels[16] = { 0, 1, 2,  3,  4,  5,  6,  7,
                                          8, 9, 10, 11, 12, 13, 14, 15 };

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackInit(matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                               matrix_ref<int, 2, W> context_ii, uint MaxSize) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);

  cm_assert(STACKCOUNT > 0);

  vector<short, 16> _chnls(_CSTK_c_channels);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int s = 0; s < (int)STACKCOUNT; s++) {
    chnls_.template select<W, 1>(s *W) = _chnls.template select<W, 1>(0);
  }

  k = -1;

  ushort x_pos = get_thread_origin_x() * W;

  context_ii.row(_CMTL_CNTX_II_OFFSET_DYNAMIC) =
      (x_pos + _chnls.template select<W, 1>(0)) * MaxSize * STACKCOUNT *
      sizeof(T);

  context_ii.row(_CMTL_CNTX_II_OFFSET_INITIAL) =
      context_ii.row(_CMTL_CNTX_II_OFFSET_DYNAMIC);
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackTop(SurfaceIndex surf,
                              matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                              matrix_ref<int, 2, W> context_ii,
                              matrix_ref<T, _CMTL_STACK_SIZE> stack,
                              matrix_ref<T, STACKCOUNT, W> element) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  cm_assert_range<short, W>(k, 0, CACHESIZE - 1);

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int s = W; s < (int)W * STACKCOUNT; s += W) {
    k_dup.template select<W, 1>(s) =
        k_dup.template select<W, 1>(s - W) + CACHESIZE;
  }

  element = stack.iselect(chnls_, k_dup);
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackTop(SurfaceIndex surf,
                              matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                              matrix_ref<int, 2, W> context_ii,
                              matrix_ref<T, _CMTL_STACK_SIZE> stack,
                              vector_ref<T, W> element, int index) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  cm_assert_range<short, W>(k, 0, CACHESIZE - 1);

  int s = index * W;
  { k_dup.template select<W, 1>(s) = k + CACHESIZE * index; }

  element = stack.iselect(chnls_.template select<W, 1>(0),
                          k_dup.template select<W, 1>(s));
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackPop(SurfaceIndex surf,
                              matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                              matrix_ref<int, 2, W> context_ii,
                              matrix_ref<T, _CMTL_STACK_SIZE> stack,
                              vector_ref<short, W> mask) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);

  cm_assert_range<short, W>(k, 0, CACHESIZE - 1);
  cm_assert_range<short, W>(mask, 0, 1);

  k -= mask;
  _CSTKAssert<W, STACKCOUNT>(context, context_ii);
  vector<short, W> isStartOfCache = k + 1;

  if (isStartOfCache.all() == false) {
    _CSTKReadBuffers<T, W, CACHESIZE, STACKCOUNT>(surf, context, context_ii,
                                                  stack);
  }
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackPop(SurfaceIndex surf,
                              matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                              matrix_ref<int, 2, W> context_ii,
                              matrix_ref<T, _CMTL_STACK_SIZE> stack,
                              matrix_ref<T, STACKCOUNT, W> element,
                              vector_ref<short, W> mask) {
  CachedStackTop<T, W, CACHESIZE, STACKCOUNT>(surf, context, context_ii, stack,
                                              element);
  CachedStackPop<T, W, CACHESIZE, STACKCOUNT>(surf, context, context_ii, stack,
                                              mask);
}

template <typename T, int W, uint CACHESIZE, uint STACKCOUNT>
CM_INLINE void CachedStackPush(SurfaceIndex surf,
                               matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                               matrix_ref<int, 2, W> context_ii,
                               matrix_ref<T, _CMTL_STACK_SIZE> stack,
                               matrix_ref<T, STACKCOUNT, W> element) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);

  k += 1;
  cm_assert_range<short, W>(k, 0, CACHESIZE);

  vector<short, W> endOfcache = k - CACHESIZE;
  if (endOfcache.all() == false) {
    _CSTKWriteBuffers<T, W, CACHESIZE, STACKCOUNT>(surf, context, context_ii,
                                                   stack);
  }
  vector<short, W *STACKCOUNT> k_sat;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int s = 0; s < (int)STACKCOUNT; s++) {
    k_sat.template select<W, 1>(s *W) =
        k_dup.template select<W, 1>(0) + CACHESIZE * s;
  }

  stack.iselect(chnls_, k_sat) = element;
}

template <int W, uint STACKCOUNT>
CM_INLINE void CachedStackEmpty(matrix_ref<short, _CMTL_CNTXT_SIZE> context,
                                matrix_ref<int, 2, W> context_ii,
                                vector_ref<short, W> isEmpty) {
  _CMTL_CSTK_DECIPHER_CONTEXT(context);
  vector_ref<int, W> surf_pos = context_ii.row(_CMTL_CNTX_II_OFFSET_DYNAMIC);
  vector_ref<int, W> surf_initial_pos =
      context_ii.row(_CMTL_CNTX_II_OFFSET_INITIAL);

  isEmpty = (((surf_pos + k) - surf_initial_pos) < 0);
}

template <typename T, int W, uint CACHESIZE>
CM_INLINE void CachedStackInit(matrix_ref<short, 2, W> context,
                               matrix_ref<int, 2, W> context_ii, uint MaxSize) {
  CachedStackInit<T, W, CACHESIZE, 1>(context, context_ii, MaxSize);
}

template <typename T, int W, uint CACHESIZE>
CM_INLINE void
CachedStackTop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
               matrix_ref<int, 2, W> context_ii,
               matrix_ref<T, W, CACHESIZE> stack, vector_ref<T, W> element) {
  CachedStackTop<T, W, CACHESIZE, 1>(surf, context, context_ii, stack, element);
}

template <typename T, int W, uint CACHESIZE>
CM_INLINE void
CachedStackPush(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                matrix_ref<int, 2, W> context_ii,
                matrix_ref<T, W, CACHESIZE> stack, vector_ref<T, W> element) {
  CachedStackPush<T, W, CACHESIZE, 1>(surf, context, context_ii, stack,
                                      element);
}

template <typename T, int W, uint CACHESIZE>
CM_INLINE void
CachedStackPop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
               matrix_ref<int, 2, W> context_ii,
               matrix_ref<T, W, CACHESIZE> stack, vector_ref<T, W> element,
               vector_ref<short, W> mask) {
  CachedStackPop<T, W, CACHESIZE, 1>(surf, context, context_ii, stack, element,
                                     mask);
}

template <int W>
CM_INLINE void CachedStackEmpty(matrix_ref<short, 2, W> context,
                                matrix_ref<int, 2, W> context_ii,
                                vector_ref<short, W> isEmpty) {
  CachedStackEmpty<W, 1>(context, context_ii, isEmpty);
}

/* ------------------------- Matrix transform Routines
 * ----------------------------------------------*/
template <typename T, int H, int W>
CM_INLINE void MirrorVertical(matrix_ref<T, H, W> in, matrix_ref<T, H, W> out) {
#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < (int)H; i++) {
    out.row(i) = in.row(H - i);
  }
}

template <typename T>
CM_INLINE void mirror_horizontal_16x16(matrix_ref<uchar, 16, 16> in,
                                       matrix_ref<uchar, 16, 16> out) {
  matrix_ref<uchar, 8, 32> in_temp = in.format<uchar, 8, 32>();
  matrix_ref<uint, 8, 8> in_temp2 = in_temp.format<uint, 8, 8>();
  matrix<uint, 8, 8> out_temp;

  matrix_ref<uchar, 8, 32> out_temp1 = out.format<uchar, 8, 32>();
  matrix_ref<uchar, 8, 32> out_temp2 = out_temp.format<uchar, 8, 32>();
// matrix_ref <uint, 8, 8> out_temp = out_temp1.format<uint, 8, 8>();;

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < 8; i++) {
    out_temp.row(i).template select<4, 2>(0).merge(
        in_temp2.row(i).replicate<2, 4, 2, 0>(1),
        in_temp2.row(i).replicate<2, 4, 2, 0>(3), 0xAAAA);
    out_temp.row(i).template select<4, 2>(1).merge(
        in_temp2.row(i).replicate<2, 4, 2, 0>(0),
        in_temp2.row(i).replicate<2, 4, 2, 0>(2), 0xAAAA);
  }

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < 8; i++) {
    out_temp1.row(i).template select<16, 2>(0).merge(
        out_temp2.row(i).replicate<8, 4, 2, 0>(1),
        out_temp2.row(i).replicate<8, 4, 2, 0>(3), 0xAAAA);
    out_temp1.row(i).template select<16, 2>(1).merge(
        out_temp2.row(i).replicate<8, 4, 2, 0>(0),
        out_temp2.row(i).replicate<8, 4, 2, 0>(2), 0xAAAA);
  }
}

template <typename T>
CM_INLINE void MirrorHorizontal_16x16(matrix_ref<T, 16, 16> in,
                                      matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> out_temp;
  Rotate270_16x16<T>(in, out_temp);
  Transpose_16x16<T>(out_temp, out);
}

template <typename T>
CM_INLINE void Rotate90_16x16(matrix_ref<T, 16, 16> in,
                              matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> out_temp;
  MirrorVertical<T, 16, 16>(in, out_temp);
  Transpose_16x16<T>(out_temp, out);
}

template <typename T>
CM_INLINE void Rotate270_16x16(matrix_ref<T, 16, 16> in,
                               matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> bBuf;
  // transpose + v_mirror
  bBuf.row(0) = in.template select<4, 1, 4, 4>(0, 0);   // 0,4,8,c
  bBuf.row(1) = in.template select<4, 1, 4, 4>(4, 0);   // 0,4,8,c
  bBuf.row(2) = in.template select<4, 1, 4, 4>(8, 0);   // 0,4,8,c
  bBuf.row(3) = in.template select<4, 1, 4, 4>(12, 0);  // 0,4,8,c
  bBuf.row(4) = in.template select<4, 1, 4, 4>(0, 1);   // 1,5,9,d
  bBuf.row(5) = in.template select<4, 1, 4, 4>(4, 1);   // 1,5,9,d
  bBuf.row(6) = in.template select<4, 1, 4, 4>(8, 1);   // 1,5,9,d
  bBuf.row(7) = in.template select<4, 1, 4, 4>(12, 1);  // 1,5,9,d
  bBuf.row(8) = in.template select<4, 1, 4, 4>(0, 2);   // 2,6,a,e
  bBuf.row(9) = in.template select<4, 1, 4, 4>(4, 2);   // 2,6,a,e
  bBuf.row(10) = in.template select<4, 1, 4, 4>(8, 2);  // 2,6,a,e
  bBuf.row(11) = in.template select<4, 1, 4, 4>(12, 2); // 2,6,a,e
  bBuf.row(12) = in.template select<4, 1, 4, 4>(0, 3);  // 3,7,b,f
  bBuf.row(13) = in.template select<4, 1, 4, 4>(4, 3);  // 3,7,b,f
  bBuf.row(14) = in.template select<4, 1, 4, 4>(8, 3);  // 3,7,b,f
  bBuf.row(15) = in.template select<4, 1, 4, 4>(12, 3); // 3,7,b,f

  out.row(15) = bBuf.template select<4, 1, 4, 4>(0, 0);  // 0
  out.row(14) = bBuf.template select<4, 1, 4, 4>(4, 0);  // 1
  out.row(13) = bBuf.template select<4, 1, 4, 4>(8, 0);  // 2
  out.row(12) = bBuf.template select<4, 1, 4, 4>(12, 0); // 3
  out.row(11) = bBuf.template select<4, 1, 4, 4>(0, 1);  // 4
  out.row(10) = bBuf.template select<4, 1, 4, 4>(4, 1);  // 5
  out.row(9) = bBuf.template select<4, 1, 4, 4>(8, 1);   // 6
  out.row(8) = bBuf.template select<4, 1, 4, 4>(12, 1);  // 7
  out.row(7) = bBuf.template select<4, 1, 4, 4>(0, 2);   // 8
  out.row(6) = bBuf.template select<4, 1, 4, 4>(4, 2);   // 9
  out.row(5) = bBuf.template select<4, 1, 4, 4>(8, 2);   // a
  out.row(4) = bBuf.template select<4, 1, 4, 4>(12, 2);  // b
  out.row(3) = bBuf.template select<4, 1, 4, 4>(0, 3);   // c
  out.row(2) = bBuf.template select<4, 1, 4, 4>(4, 3);   // d
  out.row(1) = bBuf.template select<4, 1, 4, 4>(8, 3);   // e
  out.row(0) = bBuf.template select<4, 1, 4, 4>(12, 3);  // f
}

template <typename T>
CM_INLINE void Rotate180_16x16(matrix_ref<T, 16, 16> in,
                               matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> out_temp;
  Rotate270_16x16<T>(in, out_temp);
  Rotate270_16x16<T>(out_temp, out);
}

template <typename T>
CM_INLINE void Transpose_16x16(matrix_ref<T, 16, 16> in,
                               matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> bBuf;
  bBuf.row(0) = in.template select<4, 1, 4, 4>(0, 0);   // 0,4,8,c
  bBuf.row(1) = in.template select<4, 1, 4, 4>(4, 0);   // 0,4,8,c
  bBuf.row(2) = in.template select<4, 1, 4, 4>(8, 0);   // 0,4,8,c
  bBuf.row(3) = in.template select<4, 1, 4, 4>(12, 0);  // 0,4,8,c
  bBuf.row(4) = in.template select<4, 1, 4, 4>(0, 1);   // 1,5,9,d
  bBuf.row(5) = in.template select<4, 1, 4, 4>(4, 1);   // 1,5,9,d
  bBuf.row(6) = in.template select<4, 1, 4, 4>(8, 1);   // 1,5,9,d
  bBuf.row(7) = in.template select<4, 1, 4, 4>(12, 1);  // 1,5,9,d
  bBuf.row(8) = in.template select<4, 1, 4, 4>(0, 2);   // 2,6,a,e
  bBuf.row(9) = in.template select<4, 1, 4, 4>(4, 2);   // 2,6,a,e
  bBuf.row(10) = in.template select<4, 1, 4, 4>(8, 2);  // 2,6,a,e
  bBuf.row(11) = in.template select<4, 1, 4, 4>(12, 2); // 2,6,a,e
  bBuf.row(12) = in.template select<4, 1, 4, 4>(0, 3);  // 3,7,b,f
  bBuf.row(13) = in.template select<4, 1, 4, 4>(4, 3);  // 3,7,b,f
  bBuf.row(14) = in.template select<4, 1, 4, 4>(8, 3);  // 3,7,b,f
  bBuf.row(15) = in.template select<4, 1, 4, 4>(12, 3); // 3,7,b,f

  out.row(0) = bBuf.template select<4, 1, 4, 4>(0, 0);   // 0
  out.row(1) = bBuf.template select<4, 1, 4, 4>(4, 0);   // 1
  out.row(2) = bBuf.template select<4, 1, 4, 4>(8, 0);   // 2
  out.row(3) = bBuf.template select<4, 1, 4, 4>(12, 0);  // 3
  out.row(4) = bBuf.template select<4, 1, 4, 4>(0, 1);   // 4
  out.row(5) = bBuf.template select<4, 1, 4, 4>(4, 1);   // 5
  out.row(6) = bBuf.template select<4, 1, 4, 4>(8, 1);   // 6
  out.row(7) = bBuf.template select<4, 1, 4, 4>(12, 1);  // 7
  out.row(8) = bBuf.template select<4, 1, 4, 4>(0, 2);   // 8
  out.row(9) = bBuf.template select<4, 1, 4, 4>(4, 2);   // 9
  out.row(10) = bBuf.template select<4, 1, 4, 4>(8, 2);  // a
  out.row(11) = bBuf.template select<4, 1, 4, 4>(12, 2); // b
  out.row(12) = bBuf.template select<4, 1, 4, 4>(0, 3);  // c
  out.row(13) = bBuf.template select<4, 1, 4, 4>(4, 3);  // d
  out.row(14) = bBuf.template select<4, 1, 4, 4>(8, 3);  // e
  out.row(15) = bBuf.template select<4, 1, 4, 4>(12, 3); // f
}

template <typename T>
CM_INLINE void Transpose_8x8(matrix_ref<T, 8, 8> in, matrix_ref<T, 8, 8> out) {
  matrix<T, 8, 8> temp;
  temp.row(0) = in.template select<2, 1, 4, 2>(0, 0);
  temp.row(1) = in.template select<2, 1, 4, 2>(2, 0);
  temp.row(2) = in.template select<2, 1, 4, 2>(4, 0);
  temp.row(3) = in.template select<2, 1, 4, 2>(6, 0);
  temp.row(4) = in.template select<2, 1, 4, 2>(0, 1);
  temp.row(5) = in.template select<2, 1, 4, 2>(2, 1);
  temp.row(6) = in.template select<2, 1, 4, 2>(4, 1);
  temp.row(7) = in.template select<2, 1, 4, 2>(6, 1);

  out.row(0) = temp.template select<4, 1, 2, 4>(0, 0);
  out.row(2) = temp.template select<4, 1, 2, 4>(0, 1);
  out.row(4) = temp.template select<4, 1, 2, 4>(0, 2);
  out.row(6) = temp.template select<4, 1, 2, 4>(0, 3);
  out.row(1) = temp.template select<4, 1, 2, 4>(4, 0);
  out.row(3) = temp.template select<4, 1, 2, 4>(4, 1);
  out.row(5) = temp.template select<4, 1, 2, 4>(4, 2);
  out.row(7) = temp.template select<4, 1, 2, 4>(4, 3);
}

template <typename T>
CM_INLINE void ReverseTranspose_16x16(matrix_ref<T, 16, 16> in,
                                      matrix_ref<T, 16, 16> out) {
  matrix<T, 16, 16> out_temp;
  MirrorVertical<T, 16, 16>(in, out_temp);
  Rotate270_16x16<T>(out_temp, out);
}

template <typename T, int W>
CM_INLINE void Map(matrix_ref<T, W, W> in, matrix_ref<T, W, W> out,
                   vector_ref<ushort, W *W> mapping) {
  out = in.template format<T>().iselect(mapping);
}

/* ------------------------- Assignment Routines
 * ----------------------------------------------------*/
template <typename T, uint Size>
CM_INLINE void cm_vector_assign(vector_ref<T, Size> v, int InitValue,
                                int Step) {
  T nextInitValue;
  vector<T, 8> initVec(__CM_init_array_0_7);

  v.template select<8, 1>(0) = initVec * Step;
  v.template select<8, 1>(0) += InitValue;
  nextInitValue = 8 * Step;

  enum {
    Size8x = (Size / 8) * 8
  };

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < (Size8x - 8); i += 8) {
    v.template select<8, 1>(i + 8) = v.template select<8, 1>(i) + nextInitValue;
  }

  // Remainder elements
  __RemainderInit<(Size - Size8x), T, Size> rInit;
  rInit.__CM_remainder_init(v, nextInitValue);
}

/* ------------------------- Extended Math Routines
 * -------------------------------------------------*/

template <int R, int C>
CM_INLINE matrix<float, R, C>
cm_atan2_fast(matrix<float, R, C> y, matrix<float, R, C> x, const uint flags) {
  vector<float, R *C> a0;
  vector<float, R *C> a1;
  matrix<float, R, C> atan2;

  vector<unsigned short, R *C> mask = (y >= 0);
  a0.merge(CM_CONST_PI * 0.5, CM_CONST_PI * 1.5, mask);
  a1.merge(0, CM_CONST_PI * 2, mask);

  a1.merge(CM_CONST_PI, x < 0);

  matrix<float, R, C> xy = x * y;
  matrix<float, R, C> x2 = x * x;
  matrix<float, R, C> y2 = y * y;

  a0 -= (xy / (y2 + 0.28f * x2 + CM_DBL_EPSILON));
  a1 += (xy / (x2 + 0.28f * y2 + CM_DBL_EPSILON));

  atan2.merge(a1, a0, y2 <= x2);
  if (flags & SAT)
    atan2 = matrix<float, R, C>(atan2, SAT);
  return atan2;
}

template <int R, int C>
CM_INLINE matrix<float, R, C> cm_atan2_fast(matrix_ref<float, R, C> y,
                                            matrix_ref<float, R, C> x,
                                            const uint flags) {
  vector<float, R *C> a0;
  vector<float, R *C> a1;
  matrix<float, R, C> atan2;

  vector<unsigned short, R *C> mask = (y >= 0);
  a0.merge(CM_CONST_PI * 0.5, CM_CONST_PI * 1.5, mask);
  a1.merge(0, CM_CONST_PI * 2, mask);

  a1.merge(CM_CONST_PI, x < 0);

  matrix<float, R, C> xy = x * y;
  matrix<float, R, C> x2 = x * x;
  matrix<float, R, C> y2 = y * y;

  a0 -= (xy / (y2 + 0.28f * x2 + CM_DBL_EPSILON));
  a1 += (xy / (x2 + 0.28f * y2 + CM_DBL_EPSILON));

  atan2.merge(a1, a0, y2 <= x2);
  if (flags & SAT)
    atan2 = matrix<float, R, C>(atan2, SAT);
  return atan2;
}

// For vector input
template <int N>
CM_INLINE vector<float, N> cm_atan2_fast(vector<float, N> y, vector<float, N> x,
                                         const uint flags) {
  vector<float, N> a0;
  vector<float, N> a1;
  vector<float, N> atan2;

  vector<unsigned short, N> mask = (y >= 0.0f);
  a0.merge(CM_CONST_PI * 0.5f, CM_CONST_PI * 1.5f, mask);
  a1.merge(0, CM_CONST_PI * 2.0f, mask);

  a1.merge(CM_CONST_PI, x < 0.0f);

  vector<float, N> xy = x * y;
  vector<float, N> x2 = x * x;
  vector<float, N> y2 = y * y;

  a0 -= (xy / (y2 + 0.28f * x2 + CM_DBL_EPSILON));
  a1 += (xy / (x2 + 0.28f * y2 + CM_DBL_EPSILON));

  atan2.merge(a1, a0, y2 <= x2);
  if (flags & SAT)
    atan2 = vector<float, N>(atan2, SAT);
  return atan2;
}

// For vector_ref input
template <int N>
CM_INLINE vector<float, N> cm_atan2_fast(vector_ref<float, N> y,
                                         vector_ref<float, N> x,
                                         const uint flags) {
  vector<float, N> a0;
  vector<float, N> a1;
  vector<float, N> atan2;

  vector<unsigned short, N> mask = (y >= 0.0f);
  a0.merge(CM_CONST_PI * 0.5f, CM_CONST_PI * 1.5f, mask);
  a1.merge(0, CM_CONST_PI * 2.0f, mask);

  a1.merge(CM_CONST_PI, x < 0.0f);

  vector<float, N> xy = x * y;
  vector<float, N> x2 = x * x;
  vector<float, N> y2 = y * y;

  a0 -= (xy / (y2 + 0.28f * x2 + CM_DBL_EPSILON));
  a1 += (xy / (x2 + 0.28f * y2 + CM_DBL_EPSILON));

  atan2.merge(a1, a0, y2 <= x2);
  if (flags & SAT)
    atan2 = vector<float, N>(atan2, SAT);
  return atan2;
}

//   For Scalar Input
template <> CM_INLINE float cm_atan2_fast(float y, float x, const uint flags) {
  vector<float, 1> vy = y;
  vector<float, 1> vx = x;
  vector<float, 1> atan2 =
      cm_atan2_fast(vy.select_all(), vx.select_all(), flags);
  return atan2(0);
}

// cm_atan2
// For matrix input
template <int R, int C>
CM_INLINE matrix<float, R, C>
cm_atan2(matrix<float, R, C> y, matrix<float, R, C> x, const uint flags) {
  vector<float, R *C> v_distance;
  vector<float, R *C> v_y0;
  matrix<float, R, C> atan2;
  vector<unsigned short, R *C> mask;

  mask = (x < 0);
  v_y0.merge(CM_CONST_PI, 0, mask);
  v_distance = cm_sqrt(x * x + y * y);
  mask = (cm_abs<float>(y - 0.0f) < 0.000001f);
  atan2.merge(v_y0, (2 * cm_atan((v_distance - x) / y)), mask);
  if (flags & SAT)
    atan2 = matrix<float, R, C>(atan2, SAT);

  return atan2;
}

// For matrix ref input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_atan2(matrix_ref<float, R, C> y,
                                       matrix_ref<float, R, C> x,
                                       const uint flags) {
  vector<float, R *C> v_distance;
  vector<float, R *C> v_y0;
  matrix<float, R, C> atan2;
  vector<unsigned short, R *C> mask;

  mask = (x < 0);
  v_y0.merge(CM_CONST_PI, 0, mask);
  v_distance = cm_sqrt(x * x + y * y);
  mask = (cm_abs<float>(y - 0.0f) < 0.000001f);
  atan2.merge(v_y0, (2 * cm_atan((v_distance - x) / y)), mask);
  if (flags & SAT)
    atan2 = matrix<float, R, C>(atan2, SAT);

  return atan2;
}

// For Vector input
template <int N>
CM_INLINE vector<float, N> cm_atan2(vector<float, N> y, vector<float, N> x,
                                    const uint flags) {
  vector<float, N> v_distance;
  vector<float, N> v_y0;
  vector<float, N> atan2;
  vector<unsigned short, N> mask;

  mask = (x < 0);
  v_y0.merge(CM_CONST_PI, 0, mask);
  v_distance = cm_sqrt(x * x + y * y);
  mask = (cm_abs<float>(y - 0.0f) < 0.000001f);
  atan2.merge(v_y0, (2 * cm_atan((v_distance - x) / y)), mask);
  if (flags & SAT)
    atan2 = vector<float, N>(atan2, SAT);

  return atan2;
}

// For Vector ref input
template <int N>
CM_INLINE vector<float, N> cm_atan2(vector_ref<float, N> y,
                                    vector_ref<float, N> x, const uint flags) {
  vector<float, N> v_distance;
  vector<float, N> v_y0;
  vector<float, N> atan2;
  vector<unsigned short, N> mask;

  mask = (x < 0);
  v_y0.merge(CM_CONST_PI, 0, mask);
  v_distance = cm_sqrt(x * x + y * y);
  mask = (cm_abs<float>(y - 0.0f) < 0.000001f);
  atan2.merge(v_y0, (2 * cm_atan((v_distance - x) / y)), mask);
  if (flags & SAT)
    atan2 = vector<float, N>(atan2, SAT);

  return atan2;
}

// For Scalar Input
template <> CM_INLINE float cm_atan2(float y, float x, const uint flags) {
  float v_distance;
  float v_y0;
  vector<float, 1> atan2;
  unsigned short mask;

  mask = (x < 0);
  v_y0 = mask ? CM_CONST_PI : 0;
  v_distance = cm_sqrt<float>(x * x + y * y);
  mask = (cm_abs<float>(y - 0.0f) < 0.000001f);
  atan2.merge(v_y0, (2 * cm_atan((v_distance - x) / y)), mask);
  if (flags & SAT)
    atan2 = vector<float, 1>(atan2, SAT);

  return atan2(0);
}

// cm_fmod:
// For matrix input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_fmod(matrix<float, R, C> y,
                                      matrix<float, R, C> x, const uint flags) {
  vector<int, R *C> v_quot;
  matrix<float, R, C> fmod;

  v_quot = vector<int, R *C>(y / x);
  fmod = y - x * matrix<float, R, C>(v_quot);
  if (flags & SAT)
    fmod = matrix<float, R, C>(fmod, SAT);

  return fmod;
}

// For matrix ref input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_fmod(matrix_ref<float, R, C> y,
                                      matrix_ref<float, R, C> x,
                                      const uint flags) {
  vector<int, R *C> v_quot;
  matrix<float, R, C> fmod;

  v_quot = vector<int, R *C>(y / x);
  fmod = y - x * matrix<float, R, C>(v_quot);
  if (flags & SAT)
    fmod = matrix<float, R, C>(fmod, SAT);

  return fmod;
}

// For Vector input
template <int N>
CM_INLINE vector<float, N> cm_fmod(vector<float, N> y, vector<float, N> x,
                                   const uint flags) {
  vector<int, N> v_quot;
  vector<float, N> fmod;

  v_quot = vector<int, N>(y / x);
  fmod = y - x * vector<float, N>(v_quot);
  if (flags & SAT)
    fmod = vector<float, N>(fmod, SAT);

  return fmod;
}

// For Vector ref input
template <int N>
CM_INLINE vector<float, N> cm_fmod(vector_ref<float, N> y,
                                   vector_ref<float, N> x, const uint flags) {
  vector<int, N> v_quot;
  vector<float, N> fmod;

  v_quot = vector<int, N>(y / x);
  fmod = y - x * vector<float, N>(v_quot);
  if (flags & SAT)
    fmod = vector<float, N>(fmod, SAT);

  return fmod;
}

//     For Scalar Input
template <> CM_INLINE float cm_fmod(float y, float x, const uint flags) {
  int v_quot;
  vector<float, 1> fmod;

  v_quot = (int)y / x;
  fmod(0) = y - x * v_quot;
  if (flags & SAT)
    fmod = vector<float, 1>(fmod, SAT);

  return fmod(0);
}

#define CMPI			3.14159265f

// cm_sin_emu - EU emulation for sin(x)

// For matrix input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_sin_emu(matrix<float, R, C> x, const uint flags) {
  matrix<float, R, C> x1;
  matrix<float, R, C> x2;
  matrix<float, R, C> t3;

  matrix<float, R, C> sign;
  matrix<float, R, C> fTrig;
  matrix <float, R, C> TwoPI(6.2831853f);

  x = cm_fmod(x, TwoPI);

  // 1st quadrant: sin(x) = sin(x)
  // 2nd quadrant: sin(x) = sin(PI-x)
  // 3rd quadrant: sin(x) = -sin(x-PI)
  // 4th quadrant: sin(x) = -sin(2*PI-x)
  x1.merge(CMPI -x, x- CMPI, (x <= CMPI));
  x1.merge(x, (x <= CMPI*0.5f));
  x1.merge(2* CMPI -x, (x > CMPI*1.5f));

  sign.merge(-1, 1, (x > CMPI));

  // Sine Taylor series
  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;	// 1/6 = 0.1666667f

  //t5 = t3 * x2/20;
  //t7 = t5 * x2/42;
  //t9 = t7 * x2/72;
  //t11 = t9 * x2/110;
  //fTrig = x1 - t3 + t5 - t7 + t9 - t11;

  // Optimized:
  //fTrig = x - t3 + t5 - t7 + t9 - t11;
  //fTrig = x - t3 + (x2/20 * t3) - (x2/42 * x2/20 * t3) + (x2/72 * x2/42 * x2/20 * t3) - (x2/110 * x2/72 * x2/42 * x2/20 * t3);
  //fTrig = x + t3*(-1 + x2/20 - x2/42 * x2/20 + x2/72 * x2/42 * x2/20 - x2/110 * x2/72 * x2/42 * x2/20);
  //fTrig = x + t3*(-1 + x2/20 * (1 - x2/42 + x2/72 * x2/42 - x2/110 * x2/72 * x2/42));
  //fTrig = x + t3*(-1 + x2/20 * (1 + x2/42 * (-1 + x2/72 - x2/110 * x2/72)));
  //fTrig = x + t3*(-1 + x2/20 * (1 + x2/42 * (-1 + x2/72 *(1 - x2/110))));
  // 1/20 = 0.05f
  // 1/42 = 0.0238095f
  // 1/72 = 0.0138889f
  // 1/110 = 0.0090909f
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = matrix<float, R, C>(fTrig, SAT);

  return fTrig;
}

// matrix_ref input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_sin_emu(matrix_ref<float, R, C> x, const uint flags) {
  matrix<float, R, C> x1;
  matrix<float, R, C> x2;
  matrix<float, R, C> t3;

  matrix<float, R, C> sign;
  matrix<float, R, C> fTrig;
  matrix<float, R, C> TwoPI(6.2831853f);

  x = cm_fmod(x, TwoPI);

  x1.merge(CMPI -x, x- CMPI, (x <= CMPI));
  x1.merge(x, (x <= CMPI*0.5f));
  x1.merge(2* CMPI -x, (x > CMPI*1.5f));

  sign.merge(-1, 1, (x > CMPI));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;

  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = matrix<float, R, C>(fTrig, SAT);

  return fTrig;
}

// For Vector input
template <int N>
CM_INLINE vector<float, N> cm_sin_emu(vector<float, N> x, const uint flags) {
  vector<float, N> x1;
  vector<float, N> x2;
  vector<float, N> t3;

  vector<float, N> sign;
  vector<float, N> fTrig;
  vector<float, N> TwoPI(6.2831853f);

  x = cm_fmod(x, TwoPI);

  x1.merge(CMPI - x, x - CMPI, (x <= CMPI));
  x1.merge(x, (x <= CMPI*0.5f));
  x1.merge(2 * CMPI - x, (x > CMPI*1.5f));

  sign.merge(-1, 1, (x > CMPI));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;

  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, N>(fTrig, SAT);

  return fTrig;
}

// vector_ref input
template <int N>
CM_INLINE  vector<float, N> cm_sin_emu(vector_ref<float, N> x, const uint flags) {
  vector<float, N> x1;
  vector<float, N> x2;
  vector<float, N> t3;

  vector<float, N> sign;
  vector<float, N> fTrig;
  vector<float, N> TwoPI(6.2831853f);

  x = cm_fmod(x, TwoPI);

  x1.merge(CMPI - x, x - CMPI, (x <= CMPI));
  x1.merge(x, (x <= CMPI*0.5f));
  x1.merge(2 * CMPI - x, (x > CMPI*1.5f));

  sign.merge(-1, 1, (x > CMPI));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;

  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, N>(fTrig, SAT);

  return fTrig;
}

// scalar Input
template <typename T>
CM_INLINE float cm_sin_emu(T x, const uint flags) {
  vector<float, 1> x1;
  vector<float, 1> x2;
  vector<float, 1> t3;

  vector<float, 1> sign;
  vector<float, 1> fTrig;
  float TwoPI = CMPI*2.0f;

  x = cm_fmod(x, TwoPI);

  x1.merge(CMPI - x, x - CMPI, (x <= CMPI));
  x1.merge(x, (x <= CMPI*0.5f));
  x1.merge(2 * CMPI - x, (x > CMPI*1.5f));

  sign.merge(-1, 1, (x > CMPI));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;

  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, 1>(fTrig, SAT);

  return fTrig(0);
}

// cm_cos_emu - EU emulation for sin(x)

// For matrix input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_cos_emu(matrix<float, R, C> x, const uint flags) {
  matrix<float, R, C> x1;
  matrix<float, R, C> x2;
  matrix<float, R, C> t2;
  matrix<float, R, C> t3;

  matrix<float, R, C> sign;
  matrix<float, R, C> fTrig;
  matrix <float, R, C> TwoPI(6.2831853f);

  x = cmtl::cm_fmod(x, TwoPI);

  // Use 1st quadrant sin(x) to derive cos(x) of all 4 quadrants.
  // 1st quadrant: cos(x) = sin(PI/2-x)
  // 2nd quadrant: cos(x) = -sin(x-PI/2)
  // 3rd quadrant: cos(x) = -sin(PI*3/2-x)
  // 4th quadrant: cos(x) = sin(x-PI*3/2)
  x1.merge(x - CMPI*0.5f, CMPI*1.5f - x, (x <= CMPI));
  x1.merge(CMPI*0.5f - x, (x <= CMPI*0.5f));
  x1.merge(x - CMPI*1.5f, (x > CMPI*1.5f));

  sign.merge(1, -1, ((x < CMPI*0.5f) | (x >= CMPI*1.5f)));

  // Sine Taylor series
  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;	// 1/6 = 0.1666667f
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = matrix<float, R, C>(fTrig, SAT);

  return fTrig;
}

// matrix_ref input
template <int R, int C>
CM_INLINE matrix<float, R, C> cm_cos_emu(matrix_ref<float, R, C> x, const uint flags) {
  matrix<float, R, C> x1;
  matrix<float, R, C> x2;
  matrix<float, R, C> t2;
  matrix<float, R, C> t3;

  matrix<float, R, C> sign;
  matrix<float, R, C> fTrig;
  matrix <float, R, C> TwoPI(6.2831853f);

  x = cmtl::cm_fmod(x, TwoPI);

  x1.merge(x - CMPI*0.5f, CMPI*1.5f - x, (x <= CMPI));
  x1.merge(CMPI*0.5f - x, (x <= CMPI*0.5f));
  x1.merge(x - CMPI*1.5f, (x > CMPI*1.5f));

  sign.merge(1, -1, ((x < CMPI*0.5f) | (x >= CMPI*1.5f)));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = matrix<float, R, C>(fTrig, SAT);

  return fTrig;
}

// For Vector input
template <int N>
CM_INLINE vector<float, N> cm_cos_emu(vector<float, N> x, const uint flags) {
  vector<float, N> x1;
  vector<float, N> x2;
  vector<float, N> t2;
  vector<float, N> t3;

  vector<float, N> sign;
  vector<float, N> fTrig;
  vector<float, N> TwoPI(6.2831853f);

  x = cmtl::cm_fmod(x, TwoPI);

  x1.merge(x - CMPI*0.5f, CMPI*1.5f - x, (x <= CMPI));
  x1.merge(CMPI*0.5f - x, (x <= CMPI*0.5f));
  x1.merge(x - CMPI*1.5f, (x > CMPI*1.5f));

  sign.merge(1, -1, ((x < CMPI*0.5f) | (x >= CMPI*1.5f)));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, N>(fTrig, SAT);

  return fTrig;
}

// vector_ref input
template <int N>
CM_INLINE  vector<float, N> cm_cos_emu(vector_ref<float, N> x, const uint flags) {
  vector<float, N> x1;
  vector<float, N> x2;
  vector<float, N> t3;

  vector<float, N> sign;
  vector<float, N> fTrig;
  vector<float, N> TwoPI(6.2831853f);

  x = cmtl::cm_fmod(x, TwoPI);

  x1.merge(x - CMPI*0.5f, CMPI*1.5f - x, (x <= CMPI));
  x1.merge(CMPI*0.5f - x, (x <= CMPI*0.5f));
  x1.merge(x - CMPI*1.5f, (x > CMPI*1.5f));

  sign.merge(1, -1, ((x < CMPI*0.5f) | (x >= CMPI*1.5f)));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, N>(fTrig, SAT);

  return fTrig;
}

// scalar Input
template <typename T>
CM_INLINE float cm_cos_emu(T x, const uint flags) {
  vector<float, 1> x1;
  vector<float, 1> x2;
  vector<float, 1> t3;

  vector<float, 1> sign;
  vector<float, 1> fTrig;
  float TwoPI = CMPI*2.0f;

  x = cmtl::cm_fmod(x, TwoPI);

  x1.merge(x - CMPI*0.5f, CMPI*1.5f - x, (x <= CMPI));
  x1.merge(CMPI*0.5f - x, (x <= CMPI*0.5f));
  x1.merge(x - CMPI*1.5f, (x > CMPI*1.5f));

  sign.merge(1, -1, ((x < CMPI*0.5f) | (x >= CMPI*1.5f)));

  x2 = x1 * x1;
  t3 = x2 * x1 * 0.1666667f;
  fTrig = x1 + t3 * (-1 + x2*0.05f * (1 + x2*0.0238095f * (-1 + x2*0.0138889f * (1 - x2*0.0090909f))));
  fTrig *= sign;

  if (flags & SAT)
    fTrig = vector<float, 1>(fTrig, SAT);

  return fTrig(0);
}


/* ------------------------- Support Routines
 * -------------------------------------------------------*/

template <typename T, int SZ>
CM_INLINE void cm_assert_range(vector<T, SZ> tst, float min_val,
                               float max_val) {
#ifdef CMRT_EMU
  T min = cm_reduced_min<T, T, SZ>(tst);
  T max = cm_reduced_max<T, T, SZ>(tst);
  cm_assert(min >= min_val && max <= max_val);
#endif
}

template <uint RemSize, typename T, uint Size> struct __RemainderInit {
  enum {
    Size8x = (Size / 8) * 8
  };
  CM_INLINE void __CM_remainder_init(vector_ref<T, Size> v, int Value) {
    v.template select<(RemSize), 1>(Size8x) =
        v.template select<(RemSize), 1>(Size8x - 8) + Value;
  }
};

template <typename T, uint Size> struct __RemainderInit<0, T, Size> {
  CM_INLINE void __CM_remainder_init(vector_ref<T, Size> v, int Value) {};
};

template <typename T, uint Size, int InitValue, int Step>
CM_INLINE void __CM_vector_init(vector_ref<T, Size> v) {
  T nextInitValue;
  v.template select<8, 1>(0) *= Step;
  v.template select<8, 1>(0) += InitValue;
  nextInitValue = 8 * Step;

  enum {
    Size8x = (Size / 8) * 8
  };

#if defined(__ICL) || defined(__CMC)
#pragma unroll
#endif
  for (int i = 0; i < (Size8x - 8); i += 8) {
    v.template select<8, 1>(i + 8) = v.template select<8, 1>(i) + nextInitValue;
  }

  // Remainder elements
  __RemainderInit<(Size - Size8x), T, Size> rInit;
  rInit.__CM_remainder_init(v, nextInitValue);
}
};

#pragma clang diagnostic pop

/* Close idempotent guard */
#endif /* _CMTL_H_ */
