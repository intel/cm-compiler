/*
 * Copyright (c) 2018, Intel Corporation
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

#include <assert.h>
#include <iostream>
#include <limits>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "MDF_Base.h"
#include "common_C_model.h"

#ifdef WIN32
#include "stdafx.h"
#include <io.h>
#endif

/////////////////////////////////////////////////////////////
bool Detect_MDF() {
#ifdef WIN32
  char *buffer;
  DWORD length;

  // Find new MDF driver in registry key
  HKEY hKey;
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Intel\\MDF", 0, KEY_READ,
                    &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, NULL, &length) ==
        ERROR_SUCCESS) {
      buffer = (char *)LocalAlloc(LPTR, length);
      // query
      if (RegQueryValueExA(hKey, "DriverStorePath", NULL, NULL, (BYTE *)buffer,
                           &length) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        if (strstr(buffer, "igdlh64.inf_")) {
          printf("Detected MDF on this system\n");
          return true;
        }
      }
      LocalDiscard(buffer);
    }
  }

  size_t ReturnValue;
  length = 4096;
  buffer = (char *)LocalAlloc(LPTR, length);
  // Find older MDF driver in location
  getenv_s(&ReturnValue, buffer, length, "SystemRoot");

  char FullPath[8192];
  sprintf(FullPath, "%s\\System32\\igfxcmrt64.dll", buffer);
  int status = _access(FullPath, 0);
  if (status == 0) {
    printf("Detected MDF on this system.\n");
    return true;
  }
  LocalDiscard(buffer);
  printf("Did not detect MDF on this system.\n");
  return false;
#else
  return true;
#endif
}

////////////////// Time Functions ///////////////////////////
double mygetTickFrequency(void) {
  LARGE_INTEGER freq;
  if (QueryPerformanceFrequency(&freq))
    return (double)(freq.QuadPart / 1000.);
  return -1.;
}

// Return in millisecond
double GetTimeMS() {

#ifdef WIN32
  __int64 time_a = __rdtsc();
  return (double)time_a / (mygetTickFrequency() * 1000.0);
#elif ANDROID
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (double)now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
#endif
}

// ReadRawDataFile
void ReadRawDataFile(char *filename, unsigned char *ptr, int size) {
  FILE *fh;

  fh = fopen(filename, "rb");
  if (fh == NULL) {
    printf("Error opening %s\n", filename);
    return;
  }

  if (fread(ptr, sizeof(char), size, fh) != size) {
    printf("Unable to read data from %s.\n", filename);
    return;
  }
  fclose(fh);
}

// Dump2File
void Dump2File(char *filename, unsigned char *ptr, int size) {
  FILE *fh;

  fh = fopen(filename, "wb");
  if (fh == NULL) {
    printf("Error opening %s\n", filename);
    return;
  }

  if (fwrite(ptr, sizeof(char), size, fh) != size) {
    printf("Unable to write data to %s.\n", filename);
    return;
  }
  fclose(fh);
}

void Comp2ImageFileByte(char *filename1, char *filename2, char *filename3,
                        int width, int stride, int height) {
  unsigned int Count = 0;
  int size = stride * height; // in bytes
  unsigned char *Ptr1 = (unsigned char *)malloc(size);
  unsigned char *Ptr2 = (unsigned char *)malloc(size);

  ReadRawDataFile(filename1, (unsigned char *)Ptr1, size);
  ReadRawDataFile(filename2, (unsigned char *)Ptr2, size);

  FILE *fh = fopen(filename3, "w");

  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++) {
      unsigned char p = *(Ptr1 + j * stride + i);
      unsigned char q = *(Ptr2 + j * stride + i);
      int diff = abs(p - q);
      if (diff) {
        fprintf(fh, "offset=%d:\t %08x, %08x, %08x\n", j * stride + i, p, q,
                diff);
        Count++;
      }
    }

  fprintf(fh, "%s %s byte difference = %3.3f%%\n", filename1, filename2,
          100.0f * Count / height / width);
  printf("%s %s byte difference = %3.3f%%\n", filename1, filename2,
         100.0f * Count / height / width);

  free(Ptr1);
  free(Ptr2);

  fclose(fh);
}
