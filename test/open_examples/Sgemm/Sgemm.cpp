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

#ifdef _WIN32
#include <tchar.h>
#else
#include <unistd.h>
#endif
#include "share.h"
#include "Matrix.h"
#ifdef LINUX
#define FALSE 0
#define TRUE  1
#endif

#include "common/cm_rt_helpers.h"

CmProgram* LoadProgram(CmDevice* pCmDev, char *code)
{
    FILE* pISA = fopen(code, "rb");

    fseek (pISA, 0, SEEK_END);
    int codeSize = ftell (pISA);
    rewind(pISA);

    void *pCommonISACode = (BYTE*) malloc(codeSize);

    fread(pCommonISACode, 1, codeSize, pISA);
    fclose(pISA);

    CmProgram* program = NULL;
    cm_result_check(pCmDev->LoadProgram(pCommonISACode, codeSize, program));
    free(pCommonISACode);

    return program;
}

int RunGemm(int m, int niterations, int gx, int gy)
{
    storage_type_t st = RowMajor;
    float alpha=+1.0, beta=+1.0;

    // Each thread computes 32x16 block of result matrix
    int nthreadsY    = GEMM_BLOCK/32;
    int nthreadsX    = GEMM_BLOCK/16;

    int n=m, k=m;

    // Initialization
    m = (m / TILE_m) * TILE_m;
    n=k=m;

    int lda = ((k+15)&~15);
    int ldb = ((n+15)&~15);
    int ldc = ldb;
    printf("SGEMM: C(%d, %d) = %.2f * C(%d, %d) + %.2f A(%d, %d) * B(%d, %d)\n", m,n,beta, m,n, alpha, m,k,k,n);
    printf("Row Threads:%d Col Threads:%d\n", nthreadsY, nthreadsX);
    printf("Thread-group setting: %d x %d \n", gx, gy);

    // Create a CM Device
    CmDevice* pCmDev = NULL;;
    UINT version = 0;
    cm_result_check(::CreateCmDevice( pCmDev, version ));
    if( version < CM_1_0 ){
        printf(" The runtime API version is later than runtime DLL version");
        return -1;
    }

    CmProgram* program = LoadProgram(pCmDev, "Sgemm_genx.isa");
    // Create a kernel
    CmKernel* kernel = NULL;
    cm_result_check(pCmDev->CreateKernel(program, _NAME(sgemm_kernel) , kernel));

    // Allocate matrices
    Matrix A(m,k, lda, NULL, true, "A", st);
    Matrix B(k, n,ldb, NULL, true, "B", st);
    Matrix C_gold(m, n, ldc, NULL, false, "C_gold",  st);
    Matrix C(C_gold, "C");
    Matrix zero(C_gold, "C");

    if (niterations == 1) {
        printf("** validation run, only one iteration **\n");
        printf("** For performance run, add cmd-args: Sgemm 2048 1000 ** \n");
        // Compute gold result
        printf("Compute gold result\n");

        // To use the CBLAS function below from MKL,
        // add the header files in common.h in order to compile with MKL.
        //cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
        //            m, n, k, alpha, &A(0,0), A.l_dim(),
        //            &B(0,0), B.l_dim(), beta, &C_gold(0,0), C_gold.l_dim());
        sgemmNxN(m, n, k, alpha, &A(0,0), A.l_dim(),
                 &B(0,0), B.l_dim(), beta, &C_gold(0,0), C_gold.l_dim());

    }
    else 
        printf("CPU result not computed: Make #iterations=1 to compute CPU result\n");

    /**/
    // GEN
    kernel->SetKernelArg(0, 4, &m);
    kernel->SetKernelArg(1, 4, &n);
    kernel->SetKernelArg(2, 4, &k);

    // Create a task queue
    CmQueue* pCmQueue = NULL;
    cm_result_check(pCmDev->CreateQueue( pCmQueue ));

    // copy surfaces
    CmSurface2D*  pASurf = NULL;
    cm_result_check(pCmDev->CreateSurface2D( A.l_dim(), m, CM_SURFACE_FORMAT_R32F, pASurf ));
    cm_result_check(pASurf->WriteSurface((unsigned char*)&A(0,0), NULL ));

    CmEvent *copy_event = CM_NO_EVENT;
    cm_result_check(pCmQueue->EnqueueCopyCPUToGPU(pASurf, (uint8_t *)&A(0,0), copy_event));

    CmSurface2D*  pBSurf = NULL;
    cm_result_check(pCmDev->CreateSurface2D(B.l_dim(), B.n_row(), CM_SURFACE_FORMAT_R32F,  pBSurf ));
    copy_event = CM_NO_EVENT;
    cm_result_check(pCmQueue->EnqueueCopyCPUToGPU(pBSurf, (uint8_t *)&B(0,0), copy_event));
    CmSurface2D*  pCSurf = NULL;
    cm_result_check(pCmDev->CreateSurface2D( C.l_dim(), m, CM_SURFACE_FORMAT_R32F, pCSurf ));
    copy_event = CM_NO_EVENT;
    cm_result_check(pCmQueue->EnqueueCopyCPUToGPU(pCSurf, (uint8_t *)&C(0,0), copy_event));

    // send surface as parameters
    SurfaceIndex * ABUFFER= NULL;
    pASurf->GetIndex(ABUFFER);
    kernel->SetKernelArg(6,sizeof(SurfaceIndex),ABUFFER);
    SurfaceIndex * BBUFFER= NULL;
    pBSurf->GetIndex(BBUFFER);
    kernel->SetKernelArg(7,sizeof(SurfaceIndex),BBUFFER);
    SurfaceIndex * CBUFFER= NULL;
    pCSurf->GetIndex(CBUFFER);
    kernel->SetKernelArg(8,sizeof(SurfaceIndex),CBUFFER);

    CmTask *pKernelArray = NULL;
    cm_result_check(pCmDev->CreateTask(pKernelArray));
    cm_result_check(pKernelArray->AddKernel (kernel));

    CmThreadGroupSpace *pts = nullptr;
    pCmDev->CreateThreadGroupSpace(gx, gy, nthreadsX/gx, nthreadsY/gy, pts);

    CmEvent* e = NULL;
    CM_STATUS s;
    UINT64 kernel_ns = 0;
    double thost = 0.0f;

    for(int i=0; i<niterations; i++)
    {
        for(int ib=0; ib < m; ib += GEMM_BLOCK)
        {
            kernel->SetKernelArg(3, 4, &ib);
            for(int jb=0; jb < n; jb += GEMM_BLOCK)
            {
                kernel->SetKernelArg(4, 4, &jb);
                for(int kb=0; kb < k; kb += GEMM_BLOCK)
                {
                    kernel->SetKernelArg(5, 4, &kb);
		    UINT64 time_in_ns = 0;
                    double start = getTimeStamp();
                    cm_result_check(pCmQueue->EnqueueWithGroup(pKernelArray, e, pts));
                    for( e->GetStatus(s); s != CM_STATUS_FINISHED; 
                         e->GetStatus(s)); // poll for the event to finish

                    double end = getTimeStamp();
                    thost += (end - start);
		    cm_result_check(e->GetExecutionTime(time_in_ns));
		    kernel_ns += time_in_ns;
                }
            }
        }
    }
    // average time in msec
    thost = thost * 1000.0f / niterations;
    double tkern = kernel_ns / 1000000.0f / niterations;

    Matrix C_test(C_gold, "C");
    copy_event = CM_NO_EVENT;
    cm_result_check(pCmQueue->EnqueueCopyGPUToCPU(pCSurf, (uint8_t *)&C_test(0,0), copy_event));

    printf("%-18s%.2lf msec\n","kern time:", tkern);
    printf("%-18s%.2lf msec\n","host time:", thost);

    double gflops= ((2000.0f*m*n*k) / (1.0f*1024*1024*1024)) / tkern;
    printf("GEN SGEMM (kern-timer): %8.2lf Gflops\n",  gflops);
    gflops= ((2000.0f*m*n*k) / (1.0f*1024*1024*1024)) / thost;
    printf("GEN SGEMM (host-timer): %8.2lf Gflops\n", gflops);

    // We do not initialize result matrix C to zero after each run
    // So check result only when niterations=1; Higher niterations is used
    // to get average performance number.
    bool pass=FALSE;
    if (niterations == 1) {
        if(C_test == C_gold) {
            printf("PASSED\n");
            pass = TRUE;
	}
        else
            printf("FAILED\n");
    }
    else printf("Result not checked - make #iterations=1 to check result!\n");

    printf("----------------------------\n");

    cm_result_check(pCmDev->DestroySurface(pASurf));
    cm_result_check(pCmDev->DestroySurface(pBSurf));
    cm_result_check(pCmDev->DestroySurface(pCSurf));
    cm_result_check(pCmDev->DestroyTask(pKernelArray));
    cm_result_check(pCmDev->DestroyKernel(kernel));
    cm_result_check(pCmDev->DestroyThreadGroupSpace(pts));

    cm_result_check(::DestroyCmDevice( pCmDev ));

    if (pass)
        return 0;
    else
        return 1;
}


int _tmain(int argc, _TCHAR* argv[])
{
    int m = GEMM_BLOCK;
    int niterations = 1;
    if( argc == 3 ) {
        m = atoi(argv[1]);
        niterations = atoi(argv[2]);
    }

    int success = 0;
    if (niterations == 1)
        success |= RunGemm( m, niterations, 1, 4 );
    else {
        int success = 0;
        success |= RunGemm( m, niterations, 1, 1 );
        success |= RunGemm( m, niterations, 1, 4 );
        success |= RunGemm( m, niterations, 4, 1 );
        success |= RunGemm( m, niterations, 2, 2 );
        success |= RunGemm( m, niterations, 1, 8 );
        success |= RunGemm( m, niterations, 8, 1 );
        success |= RunGemm( m, niterations, 2, 4 );
        success |= RunGemm( m, niterations, 4, 2 );
    }
    return success; 
}
