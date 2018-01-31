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

#include <algorithm>
#include "BitonicSort.h"

#include "cm_rt.h"
#include "common/cm_rt_helpers.h"
#include "common/isa_helpers.h"

using namespace std;

void cmk_bitonic_sort_256 (SurfaceIndex indx1, SurfaceIndex indx2);
void cmk_bitonic_merge(SurfaceIndex indx, unsigned int n, unsigned int m);

#define LOG2_ELEMENTS 24

BitonicSort::BitonicSort(unsigned int size)
{
  this->size_ = size;
}

void BitonicSort::Setup()
{
  pSortDirections_ = new unsigned int[size_ / base_sort_size_];
  memset(pSortDirections_, 0, sizeof(unsigned int) * (size_ / base_sort_size_));

  pSortDirections_[0] = 0;
  for (unsigned int scale = 1; scale < size_ / 256; scale <<= 1) {
    for (unsigned int i = 0; i < scale; i++) {
      pSortDirections_[scale + i] = !pSortDirections_[i];
    }
  }

  pInputs_ = new unsigned int[size_];
  for (unsigned int i = 0; i < size_; ++i) {
    pInputs_[i] = rand();
    // pInputs_[i] = rand() % (1 << 15);
  }

  pActualOutputs_ = new unsigned int[size_];
  memset(pActualOutputs_, 0, sizeof(unsigned int) * size_);

  pExpectOutputs_ = new unsigned int[size_];
  memcpy(pExpectOutputs_, pInputs_, sizeof(unsigned int) * size_);
}

void BitonicSort::RunActual()
{
  cm_result_check(Solve(pInputs_, pActualOutputs_, size_));
}

void BitonicSort::RunExpect()
{
  std::sort(pExpectOutputs_, pExpectOutputs_ + size_);
}

bool BitonicSort::Validate()
{
  for (unsigned int i = 0; i < size_; ++i) {
    if (pExpectOutputs_[i] != pActualOutputs_[i]) {
      cout << "Difference is detected at i= " << i
	   << " Expect = " << pExpectOutputs_[i]
	   << " Actual = " << pActualOutputs_[i] << endl;
      return false;
    }
  }
  return true;
}

int BitonicSort::Solve(const unsigned int * pInputs, unsigned int * pOutputs, unsigned int size)
{
  // Creates a CmDevice
  CmDevice *device = nullptr;
  unsigned int version = 0;
  cm_result_check(::CreateCmDevice(device, version));

  // load kernel
  std::string isa_code = cm::util::isa::loadFile("BitonicSortK_genx.isa");
  if (isa_code.size() == 0) {
    std::cerr << "Error: empty ISA binary.\n";
    exit(1);
  }

  // Creates a CmProgram object
  CmProgram *program = nullptr;
  cm_result_check(device->LoadProgram(const_cast<char*>(isa_code.data()),
                                      isa_code.size(),
                                      program));

  // Setup surface
  CmBuffer *pCmBuf1 = nullptr;
  cm_result_check(device->CreateBuffer(size*sizeof(unsigned int), pCmBuf1));
  cm_result_check(pCmBuf1->WriteSurface((unsigned char *)pInputs, nullptr));
  SurfaceIndex *buf1Idx = nullptr;
  cm_result_check(pCmBuf1->GetIndex(buf1Idx));


  CmBuffer *pCmBuf2 = nullptr;
  cm_result_check(device->CreateBuffer(size*sizeof(unsigned int), pCmBuf2));
  SurfaceIndex *buf2Idx = nullptr;
  cm_result_check(pCmBuf2->GetIndex(buf2Idx));

  // Setup kernels
  CmKernel* bitonic256 = nullptr;
  cm_result_check(device->CreateKernel(program,
			               "cmk_bitonic_sort_256",
				       bitonic256));
  bitonic256->SetKernelArg(0, sizeof(SurfaceIndex), buf1Idx);
  bitonic256->SetKernelArg(1, sizeof(SurfaceIndex), buf2Idx);

  unsigned int width, height; // thread space width and height
  unsigned int total_threads = size / base_sort_size_;
  if (total_threads < MAX_TS_WIDTH) {
    width = total_threads;
    height = 1;
  }
  else {
    width = MAX_TS_WIDTH;
    height = total_threads / MAX_TS_WIDTH;
  }
  CmThreadSpace *sortSpace = nullptr;
  cm_result_check(device->CreateThreadSpace(width, height, sortSpace));

  // Creates a CmTask object.
  CmTask *sortTask = nullptr;
  cm_result_check(device->CreateTask(sortTask));

  // Adds a CmKernel pointer to CmTask.
  cm_result_check(sortTask->AddKernel(bitonic256));

  // Creates a task queue.
  CmQueue *cmd_queue = nullptr;
  cm_result_check(device->CreateQueue(cmd_queue));

  // enqueue sort265 kernel
  CmEvent *sortEvent = nullptr;
  cm_result_check(cmd_queue->Enqueue(sortTask, sortEvent, sortSpace));

  // Each HW thread swap two 256-element chunks. Hence, we only need
  // to launch size/ (base_sort_size*2) HW threads
  total_threads = size / (base_sort_size_ * 2);
  if (total_threads < MAX_TS_WIDTH) {
    width = total_threads;
    height = 1;
  }
  else {
    width = MAX_TS_WIDTH;
    height = total_threads / MAX_TS_WIDTH;
  }
  CmThreadSpace *mergeSpace = nullptr;
  cm_result_check(device->CreateThreadSpace(width, height, mergeSpace));
  // create merge kernel
  CmKernel *bitonicMerge = nullptr;
  cm_result_check(device->CreateKernel(program,
			               "cmk_bitonic_merge",
				       bitonicMerge));
  // buffer that holds stage 7's result. All subsequent stages are writing
  // intermediate and final results to the same buffer so that we only need
  // to set this argument once
  bitonicMerge->SetKernelArg(0, sizeof(SurfaceIndex), buf2Idx);

  // Creates a CmTask object.
  CmTask *mergeTask = nullptr;
  cm_result_check(device->CreateTask(mergeTask));
  cm_result_check(mergeTask->AddKernel(bitonicMerge));

  // enqueue merge kernel multiple times
  // this loop is for stage 8 to stage LOG2_ELEMENTS.
  int k = 0;
  CmEvent *mergeEvent[(LOG2_ELEMENTS-8)*(LOG2_ELEMENTS-7)/2];
  for (int i = 8; i < LOG2_ELEMENTS; i++) {
    // each step halves the stride distance of its prior step.
    // 1<<j is the stride distance that the invoked step will handle.
    // The recursive steps continue until stride distance 1 is complete.
    // For stride distance less than 1<<8, no global synchronization
    // is needed, i.e., all work can be done locally within HW threads.
    // Hence, the invocation of j==8 cmk_bitonic_merge finishes stride 256
    // compare-and-swap and then performs stride 128, 64, 32, 16, 8, 4, 2, 1
    // locally.
    for (int j = i; j >= 8; j--) {
      bitonicMerge->SetKernelArg(1, sizeof(int), &j);
      // need i to determine bitonic order direction
      bitonicMerge->SetKernelArg(2, sizeof(int), &i);
      mergeEvent[k] = nullptr;
      cm_result_check(cmd_queue->Enqueue(mergeTask, mergeEvent[k], mergeSpace));
      k++;
    }
  }

  cm_result_check(pCmBuf2->ReadSurface((unsigned char *)pOutputs,
        ((k > 0) ? mergeEvent[k-1] : sortEvent) ));
  // Destroys a CmTask object.
  cm_result_check(device->DestroyTask(sortTask));
  cm_result_check(device->DestroyTask(mergeTask));
  cm_result_check(device->DestroyThreadSpace(sortSpace));
  cm_result_check(device->DestroyThreadSpace(mergeSpace));

  UINT64 total_time = 0;
  cm_result_check(sortEvent->GetExecutionTime(total_time));
  cm_result_check(cmd_queue->DestroyEvent(sortEvent));
  for (int i = 0; i < k; ++i) {
    UINT64 merge_time = 0;
    cm_result_check(mergeEvent[i]->GetExecutionTime(merge_time));
    total_time += merge_time;
    cm_result_check(cmd_queue->DestroyEvent(mergeEvent[i]));
  }
  cout  << " Sorting Time = " << total_time << " nanosec " << endl;
  //
  cm_result_check(::DestroyCmDevice(device));

  return CM_SUCCESS;
}


void BitonicSort::Teardown()
{
  delete []pExpectOutputs_;
  delete []pInputs_;
  delete []pActualOutputs_;
  delete []pSortDirections_;
}

int main(int argc, char * argv[])
{
  int size = 1 << LOG2_ELEMENTS;
  cout << "BitonicSort (" << size << ") Start..." << endl;

  BitonicSort bitonicSort(size);

  bool passed = bitonicSort.Test();

  cout << (passed ? "=> PASSED" : "=> FAILED") << endl << endl;

  return !passed;
}

