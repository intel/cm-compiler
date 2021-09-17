/*========================== begin_copyright_notice ============================

Copyright (C) 2017-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef Matrix_H
#define Matrix_H

#include <string.h>

#include "cm_utils.h"

#include "share.h"

typedef  enum{RowMajor, ColMajor, Nd} storage_type_t;
static const char *storagename[Nd] = {"RowMajor", "ColMajor"};


#define CORRECTNESS_THRESHOLD 0.00002

class Matrix {
  float *M;
  int _size_;
  int nrow;
  int ncol;
  int ld;
  storage_type_t st;
  const char *mtxname;

public:
  fptype& operator() (int r, int c) {
    return ((st == ColMajor) ? M[r+c*ld] : M[r*ld+c]);
  }

  Matrix(int nrow, int ncol, int ld, const char *surfname, bool init, const char *mtxname, storage_type_t st=RowMajor) {
    if(st == ColMajor) assert(ld >= nrow);
    else {
      if(ld < ncol) {
	fprintf(stderr, "ld(%d) should be >= ncol(%d)\n", ld, ncol);
	exit(123);
      }
    }

    this->nrow=nrow;
    this->st=st;
    this->ncol=ncol;
    this->ld=ld;

    if(st == ColMajor)
      _size_=(long long int)(sizeof(M[0])*this->ncol*this->ld);
    else
      _size_=(long long int)(sizeof(M[0])*this->nrow*this->ld);

    this->mtxname=strdup(mtxname);

    M = (fptype *)cm_utils::aligned_alloc(4096, _size_);

    for(int c=0; c < this->ncol; c++) {
      for(int r=0; r < this->nrow; r++)	{
	(*this)(r,c)=(init == true) ? randData(0.0f, 1.0f) : 0.0f;
      }
    }
  }


 Matrix(Matrix & mat, const char *mtxname)
   :nrow(mat.nrow), ncol(mat.ncol), ld(mat.ld), st(mat.st)
  {
     this->mtxname=strdup(mtxname);
	 // printf("Allocating %s \n", mtxname);
     M = (fptype *)cm_utils::aligned_alloc(4096, mat._size_);
     for(int c=0; c < this->ncol; c++)
     for(int r=0; r < this->nrow; r++)
     {
       (*this)(r,c)=mat(r,c);
     }

  }
  bool operator == (Matrix& m)
  {
     if( m.n_col()!=this->n_col() ) return false;

     double max_relerror=0.0;
     double max_abserror=0.0;
     for(int c=0; c < ncol; c++)
       for(int r=0; r < nrow; r++)
       {

	 // printf("I=%3d N=%3d  %08x  %08x\n", r, c, *(unsigned int*)&(*this)(r,c), *(unsigned int *)&m(r,c));

	 double relerror=fabs((*this)(r,c)-m(r,c))/max(fabs((*this)(r,c)), fabs(m(r,c)));
	 double abserror=fabs((*this)(r,c)-m(r,c));

	 max_relerror = max(max_relerror, relerror);
	 max_abserror = max(max_abserror, abserror);

	 if(relerror >  CORRECTNESS_THRESHOLD)
         {
	   printf("Failure %f %f relerror: %lf at [%d, %d]\n", (*this)(r,c), m(r,c), relerror, r, c);
           exit(-1);
         }
       }
     printf("max_relerror = %e  absolute error = %e\n", max_relerror, max_abserror);
     return (max_relerror >  CORRECTNESS_THRESHOLD) ? false : true;
     return true;
  }

  // friend ostream& operator<<(ostream&, const Matrix&);
  void Print(const char *str=NULL)
  {
      if(str) printf("%s ", str);
	  printf(" %d x %d\n",  this->n_row(), this->l_dim());
	  for (int i=0; i<this->n_row(); i++) {
	    for (int j=0; j<this->n_col(); j++)
            printf("C(%d,%d)=%f \n",i,j,(*this)(i,j));
          printf("\n");
      }
  }

  int n_row() {return nrow;}
  int n_col() {return ncol;}
  int l_dim() {return ld;}
  ~Matrix() {
    /*printf("Deallocating %s \n", mtxname); */
    cm_utils::aligned_free(M);
  }
};

// C := alpha*A*B + beta*C,
// A(m x k) , B(k x n) , C(m x n)
static int sgemmNxN(int m, int n, int k, float alpha, float *A, int lda, float *B, int ldb,
                    float beta, float *C, int ldc, storage_type_t st = RowMajor)
{

  for(int r=0; r < m; r++)
    for(int c=0; c < n; c++)
    {
      float tmp=0.0f;
      if(st == ColMajor)
      {
        for(int t=0; t < k; t++)
          tmp+=A[r+t*lda]*B[t+c*ldb];
        C[r+c*ldc] = alpha*tmp + beta*C[r+c*ldc];
      }
      else
      {
        for(int t=0; t < k; t++)
          tmp+=A[r*lda+t]*B[t*ldb+c];
        C[r*ldc+c] = alpha*tmp + beta*C[r*ldc+c];
      }
    }

  return 1;
}

#endif
