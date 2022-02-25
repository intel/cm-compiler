/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// Check that dependent CM expressions do not cause compiler crash.

// RUN: %cmc -march=SKL -S -emit-llvm -- %s

template<int N>
int vec_size(vector<int, N> x) {
  vector<int, x.n_elems()> y;
  (void)y;
}

template<int N>
int mat_size(matrix<int, N, N> x) {
  vector<int, x.n_rows() + x.n_cols()> y;
  (void)y;
}
