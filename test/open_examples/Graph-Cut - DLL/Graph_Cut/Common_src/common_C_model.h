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

    struct int3
    {
        int x, y, z;
    };

    struct float4
    {
        float x, y, z, w;
    };

    struct float3
    {
        float x, y, z;
    };

    struct uchar4
    {
        unsigned char x, y, z, w;
    };

    typedef struct mat3x4
    {
    	float4 r0_;
	    float4 r1_;
    	float4 r2_;
    } mat3x4;

    typedef struct mat4x4
    {
	    float4 r0_;
	    float4 r1_;
	    float4 r2_;
	    float4 r3_;
    } mat4x4;

    // float4 functions
    inline float4 make_float4 (float x, float y, float z, float w)
    {
	    float4 result = { x, y, z, w };
	    return result;
    }

    inline float dot (const float4 &v, const float4 &w)
    {
	    return v.x*w.x + v.y*w.y + v.z*w.z + v.w*w.w;
    }

    inline float length2 (const float4 &v)
    {
	    return dot (v, v);
    }

    inline float4 operator+ (const float4 &v, const float4 &w)
    {
	    float4 result = { v.x+w.x, v.y+w.y, v.z+w.z, v.w+w.w };
	    return result;
    }

    inline float4 operator- (const float4 &v, const float4 &w)
    {
	    float4 result = { v.x-w.x, v.y-w.y, v.z-w.z, v.w-w.w };
	    return result;
    }

    inline float4 operator* (const float4 &v, const float4 &w)
    {
	    float4 result = { v.x*w.x, v.y*w.y, v.z*w.z, v.w*w.w };
	    return result;
    }

    inline float4 operator* (float k, const float4 &v)
    {
	    float4 result = { k*v.x, k*v.y, k*v.z, k*v.w };
	    return result;
    }

    inline float4 operator/ (const float4 &v, float d)
    {
	    float4 result = { v.x/d, v.y/d, v.z/d, v.w/d };
	    return result;
    }

    inline float4 & operator+= (float4 &v, const float4 &w)
    {
	    v.x += w.x; v.y += w.y; v.z += w.z; v.w += w.w;
	    return v;
    }

    inline float4 & operator-= (float4 &v, const float4 &w)
    {
	    v.x -= w.x; v.y -= w.y; v.z -= w.z; v.w -= w.w;
	    return v;
    }

    inline float4 & operator*= (float4 &v, float k)
    {
	    v.x *= k; v.y *= k; v.z *= k; v.w *= k;
	    return v;
    }
}

extern double GetTimeMS();

////////// Common C model //////////
extern bool Detect_MDF();

extern MDF::float4 GetPixFloat4(MDF::float4 * ptr, int FrameWidth, int x, int y);

extern void ReadRawDataFile(char * filename, unsigned char * ptr, int size);
extern void Dump2File(char * filename, unsigned char * ptr, int size);
extern void Dump2File_8bit(char * filename, HEIGHT_TYPE * ptr, int size);
extern void Dump2File_32_to_8bit(char * filename, unsigned int * ptr, int size);

extern void Reduce2File(char * filename, unsigned char * ptr, int size);

extern void Dump2TxtFile(char * filename, float * ptr, int width, int height);
extern void Array2File(char * filename, float * Range, int count);
extern void _2DArray2File(char * filename, float * pMultiRange, int Total, int RowSize);
extern void DumpFloat4(char * filename, float * p4, int count);
extern void DumpString(char * filename, char * p);
extern void DumpDiff(char * filename, float * p, float * q, int width, int height);
extern void RemoveAlpha(unsigned char * pSrc, unsigned char * pDest, int Cols, int Rows, int Pitch);
extern void Comp2ImageFileFloat(char * filename1, char * filename2, char * filename3, int width, int stride, int height, float delta);
extern void Comp2ImageFileColor(char * filename1, char * filename2, char * filename3, int width, int stride, int height);
extern void Comp2ImageFileInt(char * filename1, char * filename2, char * filename3, int width, int stride, int height);
extern void Comp2ImageFileByte(char * filename1, char * filename2, char * filename3, int width, int stride, int height);

extern void CModel_Dilate(unsigned char * iBuffer, unsigned char * oBuffer, int Cols, int Rows, int KernelSize);
extern void CModel_Erode(unsigned char * iBuffer, unsigned char * oBuffer, int Cols, int Rows, int KernelSize);
extern void CModel_BuildMask(unsigned char * bBuffer1, unsigned char * bBuffer2, int Cols, int Rows);

extern void CModel_FlipX(unsigned int * pInBuf, unsigned int * pOutBuf, int Cols, int Rows, int Pitch);
extern void CModel_RGBA2BGRA(unsigned int * pInBuf, unsigned int * pOutBuf, int Cols, int Rows);


template <class X> X GetPix(X * ptr, int FrameWidth, int x, int y)
{
    X pix = *(ptr + FrameWidth * y + x);
    return pix;
}

template <class X> void PutPix(X * ptr, int FrameWidth, int x, int y, X val)
{
    *(ptr + FrameWidth * y + x) = val;
}

template <class X> void AddPix(X * ptr, int FrameWidth, int x, int y, X val)
{
    *(ptr + FrameWidth * y + x) += val;
}

#endif
