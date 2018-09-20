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

#ifndef COMMON_C_MODEL_H
#define COMMON_C_MODEL_H

#include "cm_rt.h"

#define HEIGHT_SHORT 1

#if HEIGHT_SHORT
typedef unsigned short HEIGHT_TYPE;
#define SIDE_SQUARE 16
#define LOG_SIZE 4
#else
typedef unsigned int HEIGHT_TYPE;
#define SIDE_SQUARE 8
#define LOG_SIZE 3
#endif

namespace MDF {

struct int3 {
  int x, y, z;
};

struct float4 {
  float x, y, z, w;
};

struct float3 {
  float x, y, z;
};

struct uchar4 {
  unsigned char x, y, z, w;
};

typedef struct mat3x4 {
  float4 r0_;
  float4 r1_;
  float4 r2_;
} mat3x4;

typedef struct mat4x4 {
  float4 r0_;
  float4 r1_;
  float4 r2_;
  float4 r3_;
} mat4x4;

// float4 functions
inline float4 make_float4(float x, float y, float z, float w) {
  float4 result = {x, y, z, w};
  return result;
}

inline float dot(const float4 &v, const float4 &w) {
  return v.x * w.x + v.y * w.y + v.z * w.z + v.w * w.w;
}

inline float length2(const float4 &v) { return dot(v, v); }

inline float4 operator+(const float4 &v, const float4 &w) {
  float4 result = {v.x + w.x, v.y + w.y, v.z + w.z, v.w + w.w};
  return result;
}

inline float4 operator-(const float4 &v, const float4 &w) {
  float4 result = {v.x - w.x, v.y - w.y, v.z - w.z, v.w - w.w};
  return result;
}

inline float4 operator*(const float4 &v, const float4 &w) {
  float4 result = {v.x * w.x, v.y * w.y, v.z * w.z, v.w * w.w};
  return result;
}

inline float4 operator*(float k, const float4 &v) {
  float4 result = {k * v.x, k * v.y, k * v.z, k * v.w};
  return result;
}

inline float4 operator/(const float4 &v, float d) {
  float4 result = {v.x / d, v.y / d, v.z / d, v.w / d};
  return result;
}

inline float4 &operator+=(float4 &v, const float4 &w) {
  v.x += w.x;
  v.y += w.y;
  v.z += w.z;
  v.w += w.w;
  return v;
}

inline float4 &operator-=(float4 &v, const float4 &w) {
  v.x -= w.x;
  v.y -= w.y;
  v.z -= w.z;
  v.w -= w.w;
  return v;
}

inline float4 &operator*=(float4 &v, float k) {
  v.x *= k;
  v.y *= k;
  v.z *= k;
  v.w *= k;
  return v;
}
} // namespace MDF

extern double GetTimeMS();

////////// Common C model //////////
extern bool Detect_MDF();

extern void ReadRawDataFile(char *filename, unsigned char *ptr, int size);
extern void Dump2File(char *filename, unsigned char *ptr, int size);
extern void Comp2ImageFileByte(char *filename1, char *filename2,
                               char *filename3, int width, int stride,
                               int height);

template <class X> X GetPix(X *ptr, int FrameWidth, int x, int y) {
  X pix = *(ptr + FrameWidth * y + x);
  return pix;
}

template <class X> void PutPix(X *ptr, int FrameWidth, int x, int y, X val) {
  *(ptr + FrameWidth * y + x) = val;
}

template <class X> void AddPix(X *ptr, int FrameWidth, int x, int y, X val) {
  *(ptr + FrameWidth * y + x) += val;
}

#endif
