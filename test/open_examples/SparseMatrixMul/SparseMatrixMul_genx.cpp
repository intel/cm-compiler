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

///////////////////////////////////////////////////////////////////////////////
// Configuration parameters for CM host and kernel
///////////////////////////////////////////////////////////////////////////////

#define OWORD_BUF_ALIGNMENT    (4)  // Required alignment for OWORD reads
#define THREAD_SPACE_WIDTH     (256) // Thread space width
#define ROWS_PER_THREAD        (8) // Process 8 row per thread

//////////////////////////////////////////////////////////////////////////////
// CM kernel:
//    Generic SpMV kernel for CSR format.
//////////////////////////////////////////////////////////////////////////////

const ushort a_init[] = {0, 1, 2, 3, 4, 5, 6, 7};

template <typename IndexType, typename ValueType>
_GENX_MAIN_ void
SpmvCsr(SurfaceIndex ANZ_BUF,                   // CURBE  parameter
         SurfaceIndex ACOL_BUF,                  // CURBE  parameter
         SurfaceIndex AROW_BUF,                  // CURBE  parameter
         SurfaceIndex X_BUF,                     // CURBE  parameter
         SurfaceIndex Y_BUF,                     // CURBE  parameter
         IndexType row_start,                    // CURBE  parameter
         short row_stride,                       // CURBE  parameter
         IndexType max_rows,                     // CURBE  parameter
         vector<IndexType, ROWS_PER_THREAD> v_st // CURBE  parameter
         ) {
  //--------------------------------------------------------------------
  // Read in the AROW vector with a stride calculated in v_st using a
  // scattered read. We do this in order to increase chances that
  // concurrently running threads access contiguous rows from the ANZ/ACOL
  // vector.
  //--------------------------------------------------------------------
  vector<IndexType, ROWS_PER_THREAD> v_ar;
  vector<IndexType, ROWS_PER_THREAD> v_sz;

  int row_number = row_start +
                   cm_group_id(1) * THREAD_SPACE_WIDTH * ROWS_PER_THREAD +
                   cm_group_id(0);

  // Read arow(row_number)
  read(AROW_BUF, row_number, v_st, v_ar);
  vector<IndexType, ROWS_PER_THREAD> v_sz_st;

  // Read arow(row_number + 1)
  v_sz_st = v_st + 1;
  read(AROW_BUF, row_number, v_sz_st, v_sz);

  //--------------------------------------------------------------------
  // Compute sz = arow(row_number + 1) - arow(row_number)
  //--------------------------------------------------------------------
  v_sz = v_sz - v_ar;
  if (v_sz.any()) {
    //--------------------------------------------------------------------
    // Read in the Y vector with a stride calculated in v_st using a
    // scattered read for the same reason as for the AROW vector.
    //--------------------------------------------------------------------
    vector<uint, ROWS_PER_THREAD> v_y_dw;
    read(Y_BUF, row_number, v_st, v_y_dw);

    //--------------------------------------------------------------------
    // Process all elements of one row at a time.
    //--------------------------------------------------------------------
    vector_ref<ValueType, ROWS_PER_THREAD> v_y = v_y_dw.format<ValueType>();

    for (ushort i = 0; (i < ROWS_PER_THREAD) && (row_number + i * row_stride < max_rows); i++) {
      IndexType row_begin = v_ar(i);
      IndexType row_length = v_sz(i);

      for (IndexType j = 0; j < row_length; j += 32) {
        IndexType row_slice_index = row_begin + j;
        IndexType row_slice_offset = row_slice_index * sizeof(ValueType);

        //--------------------------------------------------------------------
        // Read in the ANZ and ACOL vector using block reads. We process in the
        // ANZ and ACOL vector in slices of size 32, 16, 8 or 4 depending on the
        // row length.
        //--------------------------------------------------------------------
        IndexType row_remain_length = row_length - j;
        ushort row_slice_length = cm_min<IndexType>(row_remain_length, 32);

        if (row_slice_length > 16) {
          vector<ValueType, 32> v_an;
          vector<uint, 32> v_ac;
          read(DWALIGNED(ACOL_BUF), row_slice_offset, v_ac);
          read(DWALIGNED(ANZ_BUF), row_slice_offset, v_an);

          //----------------------------------------------------------
          // We need to zero out the overfill portion of the final slice.
          //----------------------------------------------------------
          vector<ushort, 8> v_init8(a_init);
          vector<ushort, 16> v_init16;
          v_init16.select<8, 1>(0) = v_init8 + 16;
          v_init16.select<8, 1>(8) = v_init16.select<8, 1>(0) + 8;
          v_ac.select<8, 1>(16).merge(v_ac.select<8, 1>(16), 0,
                                      v_init16.select<8, 1>(0) < row_slice_length);
          v_ac.select<8, 1>(24).merge(v_ac.select<8, 1>(24), 0,
                                      v_init16.select<8, 1>(8) < row_slice_length);

          //--------------------------------------------------------------------
          // Read in the X vector using a scatter read and then perform the
          // actual compute.
          //--------------------------------------------------------------------
          vector<uint, 32> v_x_dw;
          read(X_BUF, 0, v_ac.select<16, 1>(0), v_x_dw.select<16, 1>(0));
          read(X_BUF, 0, v_ac.select<16, 1>(16), v_x_dw.select<16, 1>(16));
          vector_ref<ValueType, 32> v_x = v_x_dw.format<ValueType>();
          vector<ValueType, 32> v_dp4 = cm_dp4<float>(v_an, v_x);
          v_y(i) += cm_sum<ValueType>(v_dp4.template select<8, 4>(0));
        } else if (row_slice_length > 8) {
          vector<ValueType, 16> v_an;
          vector<uint, 16> v_ac;
          read(DWALIGNED(ACOL_BUF), row_slice_offset, v_ac);
          read(DWALIGNED(ANZ_BUF), row_slice_offset, v_an);

          //----------------------------------------------------------
          // We need to zero out the overfill portion of the final slice.
          //----------------------------------------------------------
          vector<ushort, 8> v_init8(a_init);
          v_init8 += 8;
          v_ac.select<8, 1>(8).merge(v_ac.select<8, 1>(8), 0, v_init8 < row_slice_length);

          //-------------------------------------------------------------------------------
          // Read in the X vector using a scatter read and then perform the actual compute.
          //-------------------------------------------------------------------------------
          vector<uint, 16> v_x_dw;
          read(X_BUF, 0, v_ac.select<16, 1>(0), v_x_dw.template select<16, 1>(0));
          vector_ref<ValueType, 16> v_x = v_x_dw.template format<ValueType>();
          vector<ValueType, 16> v_dp4 = cm_dp4<float>(v_an.template select<16, 1>(0), v_x);
          v_y(i) += cm_sum<ValueType>(v_dp4.template select<4, 4>(0));
        } else if (row_slice_length > 4) {
          vector<ValueType, 8> v_an;
          vector<uint, 8> v_ac;
          read(DWALIGNED(ACOL_BUF), row_slice_offset, v_ac);
          read(DWALIGNED(ANZ_BUF), row_slice_offset, v_an);

          //----------------------------------------------------------
          // We need to zero out the overfill portion of the final slice.
          //----------------------------------------------------------
          vector<ushort, 8> v_init8(a_init);
          v_ac.merge(v_ac, 0, v_init8 < row_slice_length);

          //-------------------------------------------------------------------------------
          // Read in the X vector using a scatter read and then perform the
          // actual compute.
          //-------------------------------------------------------------------------------
          vector<uint, 8> v_x_dw;
          read(X_BUF, 0, v_ac.select<8, 1>(0), v_x_dw.template select<8, 1>(0));
          vector_ref<ValueType, 8> v_x = v_x_dw.template select<8, 1>(0).format<ValueType>();
          vector<ValueType, 8> v_dp4 = cm_dp4<float>(v_an.template select<8, 1>(0), v_x);
          v_y(i) += v_dp4(0) + v_dp4(4);
        } else {
          vector<ValueType, 8> v_an;
          vector<uint, 8> v_ac;
          read(DWALIGNED(ACOL_BUF), row_slice_offset, v_ac.template select<4, 1>(0));
          read(DWALIGNED(ANZ_BUF), row_slice_offset, v_an.template select<4, 1>(0));

          //----------------------------------------------------------
          // We need to zero out the overfill portion of the final slice.
          //----------------------------------------------------------
          vector<ushort, 8> v_init8(a_init);
          v_ac.merge(v_ac, 0, v_init8 < row_slice_length);

          //-------------------------------------------------------------------------------
          // Read in the X vector using a scatter read and then perform the
          // actual compute.
          //-------------------------------------------------------------------------------
          vector<uint, 8> v_x_dw;
          read(X_BUF, 0, v_ac.select<8, 1>(0), v_x_dw.template select<8, 1>(0));
          vector_ref<ValueType, 4> v_x = v_x_dw.template select<4, 1>(0).format<ValueType>();
          vector<ValueType, 4> v_dp4 = cm_dp4<float>(v_an.template select<4, 1>(0), v_x);
          v_y(i) += v_dp4(0);
        }
      }
    }

    //--------------------------------------------------------------------
    // Write out the Y vector with a stride calculated in v_st using a
    // scattered write.
    //--------------------------------------------------------------------
    write(Y_BUF, row_number, v_st, v_y);
  }
}

template void
SpmvCsr<unsigned, float>(SurfaceIndex, SurfaceIndex, SurfaceIndex,
                         SurfaceIndex, SurfaceIndex, unsigned, short,
                         unsigned, vector<unsigned, ROWS_PER_THREAD>);

template <typename IndexType, typename ValueType>
_GENX_MAIN_ void
SpmvCsrSimple(SurfaceIndex ANZ_BUF,              // CURBE  parameter
         SurfaceIndex ACOL_BUF,                  // CURBE  parameter
         SurfaceIndex AROW_BUF,                  // CURBE  parameter
         SurfaceIndex X_BUF,                     // CURBE  parameter
         SurfaceIndex Y_BUF,                     // CURBE  parameter
         IndexType row_start                     // CURBE  parameter
	 )
{
  vector<IndexType, ROWS_PER_THREAD+4> v_ar;
  vector<IndexType, ROWS_PER_THREAD> v_ac;
  vector<float, ROWS_PER_THREAD> v_nz;
  vector<float, ROWS_PER_THREAD> v_x;
  vector<float, ROWS_PER_THREAD> v_y;

  int row_number = row_start +
                   cm_group_id(1) * THREAD_SPACE_WIDTH * ROWS_PER_THREAD +
                   cm_group_id(0) * ROWS_PER_THREAD;

  // oword block read, arow and dst-vector
  read(Y_BUF, row_number*4, v_y);
  read(AROW_BUF, row_number*4, v_ar);

  vector<IndexType, ROWS_PER_THREAD> v_ar0 = v_ar.select<ROWS_PER_THREAD, 1>(0);
  vector<IndexType, ROWS_PER_THREAD> v_ar1 = v_ar.select<ROWS_PER_THREAD, 1>(1);
  SIMD_IF_BEGIN(v_ar1 > v_ar0) {
      SIMD_DO_WHILE_BEGIN {
         // scatter-read column indices
         read(ACOL_BUF, 0, v_ar0, v_ac);
         // scatter-read nonzero values
         read(ANZ_BUF, 0, v_ar0, v_nz);
         // scatter-read x values
         read(X_BUF, 0, v_ac, v_x);
         v_y = v_y + v_x * v_nz;
         v_ar0 += 1;
      } SIMD_DO_WHILE_END (v_ar1 > v_ar0);
  } SIMD_IF_END;
  // oword block write
  write(Y_BUF, row_number*4, v_y);
}

template void
SpmvCsrSimple<unsigned, float>(SurfaceIndex, SurfaceIndex, SurfaceIndex,
                         SurfaceIndex, SurfaceIndex, unsigned);
