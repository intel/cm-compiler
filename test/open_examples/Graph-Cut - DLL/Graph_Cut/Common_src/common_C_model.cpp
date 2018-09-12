#ifdef WIN32
#include "stdafx.h"
#endif

#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>

#include "common_C_model.h"
#include "MDF_Base.h"

////////////////// Time Functions ///////////////////////////
double mygetTickFrequency( void )
{
    LARGE_INTEGER freq;
    if( QueryPerformanceFrequency( &freq ) )
        return (double)(freq.QuadPart / 1000.);
    return -1.;
}

// Return in millisecond
double GetTimeMS()
{

#ifdef WIN32
    __int64 time_a = __rdtsc();
    return (double) time_a/(mygetTickFrequency()*1000.0);
#elif ANDROID
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) now.tv_sec*1000.0 + now.tv_nsec/1000000.0;
#endif

}

////////////////////////////////////////////////////////////////////////////////
/*
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
*/

MDF::float4 GetPixFloat4(MDF::float4 * ptr, int FrameWidth, int x, int y)
{
    MDF::float4 pix = *(ptr + FrameWidth * y + x);
    return pix;
}

/*
unsigned char GetPixU8(unsigned char * ptr, int FrameWidth, int x, int y)
{
    unsigned char pix = *(ptr + FrameWidth * y + x);
    return pix;
}

short GetPixS16(short * ptr, int FrameWidth, int x, int y)
{
    short pix = *(ptr + FrameWidth * y + x);
    return pix;
}

unsigned int GetPixU32(unsigned int * ptr, int FrameWidth, int x, int y)
{
    unsigned int pix = *(ptr + FrameWidth * y + x);
    return pix;
}
*/
unsigned char GetPixU8C4(unsigned char * ptr, int FrameWidth, int x, int y, int color_offset)
{
    unsigned char pix = *(ptr + 4 * (FrameWidth * y + x) + color_offset);
    return pix;
}
/*
float GetPixF32(float * ptr, int FrameWidth, int x, int y)
{
    float pix = *(ptr + FrameWidth * y + x);
    return pix;
}

void PutPixU8(unsigned char * ptr, int FrameWidth, int x, int y, unsigned char val)
{
    *(ptr + FrameWidth * y + x) = val;
}

void PutPixS16(short * ptr, int FrameWidth, int x, int y, short val)
{
    *(ptr + FrameWidth * y + x) = val;
}

void PutPixU32(unsigned int * ptr, int FrameWidth, int x, int y, unsigned int val)
{
    *(ptr + FrameWidth * y + x) = val;
}

void PutPixF32(float * ptr, int FrameWidth, int x, int y, float val)
{
    *(ptr + FrameWidth * y + x) = val;
}
*/

// ReadRawDataFile
void ReadRawDataFile(char * filename, unsigned char * ptr, int size)
{
FILE * fh;

    fh = fopen(filename, "rb");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    if (fread(ptr, sizeof( char ), size, fh) != size) {
        printf("Unable to read data from %s.\n", filename);
        return;
    }
    fclose(fh);
}


// Dump2File
void Dump2File(char * filename, unsigned char * ptr, int size)
{
FILE * fh;

    fh = fopen(filename, "wb");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    if (fwrite(ptr, sizeof( char ), size, fh) != size) {
        printf("Unable to write data to %s.\n", filename);
        return;
    }
    fclose(fh);
}

// 16 to 8 or 32 to 8
void Dump2File_8bit(char * filename, HEIGHT_TYPE * ptr, int size)
{
	FILE * fh;
	unsigned char * pBuffer = (unsigned char *) malloc(size);

	for (int i = 0; i < size; i++) 
		pBuffer[i] = (ptr[i] < 255) ? ptr[i] : 255;
	
    fh = fopen(filename, "wb");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    if (fwrite(pBuffer, sizeof( char ), size, fh) != size) {
        printf("Unable to write data to %s.\n", filename);
        return;
    }
    fclose(fh);
	free(pBuffer);
}

// Dump2File_32_8bit
/*
void Dump2File_32_to_8bit(char * filename, unsigned int * ptr, int size)
{
	FILE * fh;
	unsigned char * pBuffer = (unsigned char *) malloc(size);

	for (int i = 0; i < size; i++) 
		pBuffer[i] = (ptr[i] < 255) ? ptr[i] : 255;
	
    fh = fopen(filename, "wb");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    if (fwrite(pBuffer, sizeof( char ), size, fh) != size) {
        printf("Unable to write data to %s.\n", filename);
        return;
    }
    fclose(fh);
	free(pBuffer);
}
*/


// Reduce2File: convert data from int to short, and dump
void Reduce2File(char * filename, unsigned char * ptr, int size)
{
	int size2 = size/2;
	unsigned short * usPtr = (unsigned short *) malloc(size2);
	unsigned short * temp = usPtr;

	unsigned int * iPtr = (unsigned int *) ptr;

	for (int i = 0; i < size/4; i++, temp++, iPtr++) {
		if (*iPtr >= 320*240)
			*temp = 0xFFFE;
		else 
			*temp = *iPtr;	// int to short
	}

	Dump2File(filename, (unsigned char *) usPtr, size2);
	free(usPtr);
}

void Dump2TxtFile(char * filename, float * ptr, int width, int height)
{
FILE * fh;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            if (*(ptr + j*width + i) != 0.0f)
                fprintf(fh, "%d, %d, %f\n", j, i, *(ptr + j*width + i));
            //fprintf(fh, "%d, %d, %8.6f\n", j, i, *(ptr + j*width + i));
        }
        fprintf(fh, "\n");
    }
    fclose(fh);
}

void Array2File(char * filename, float * Range, int count)
{
FILE * fh;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fh, "%d, %f\n", i, *(Range+i));
    }

    fclose(fh);
}

void _2DArray2File(char * filename, float * pMultiRange, int Total, int RowSize)
{
FILE * fh;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    for (int i = 0; i < Total; i++) {
		if (i && i%RowSize == 0)
			fprintf(fh, "\n");
        fprintf(fh, "%12.6g, ", *(pMultiRange+i));
    }

    fclose(fh);

}

void DumpFloat4(char * filename, float * p, int count)
{
FILE * fh;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    MDF::float4 * p4 = (MDF::float4 *) p;
    for (int i = 0; i < count; i++) {
        if ((p4+i)->x != 0.0f) {
            fprintf(fh, "%d) %f, %f, %f, %f\n", i, (p4+i)->x, (p4+i)->y, (p4+i)->z, (p4+i)->w); 
        }
    }

    fclose(fh);
}

void DumpString(char * filename, char * p)
{
FILE * fh;

    fh = fopen(filename, "a");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    fprintf(fh, "%s\n", p); 

    fclose(fh);
}


void DumpDiff(char * filename, float * p, float * q, int width, int height)
{
FILE * fh;

    fh = fopen(filename, "w");
    if (fh == NULL) {
        printf("Error opening %s\n", filename);
        return;
    }

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            float diff = p[j*width+i] - q[j*width+i];
            if (diff > 0.0001) {
                fprintf(fh, "%d, %d, %f, %f, %f\n", j, i, p[j*width+i], q[j*width+i], diff);
            }
        }
    fclose(fh);
}


void Comp2ImageFileFloat(char * filename1, char * filename2, char * filename3, int width, int stride, int height, float delta)
{
    unsigned int Count = 0;
    int size = stride*height;    // in bytes
    float * Ptr1 = (float *) malloc(size);
    float * Ptr2 = (float *) malloc(size);

    ReadRawDataFile(filename1, (unsigned char *) Ptr1, size);
    ReadRawDataFile(filename2, (unsigned char *) Ptr2, size);

    FILE * fh = fopen(filename3, "w");

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            float diff = *(Ptr1+j*stride/4+i) - *(Ptr2+j*stride/4+i);
            if (diff < -delta || diff > delta) {
                fprintf(fh, "x=%d, y=%d, z=%d :\t%g, %g, %g\n", i, j%256, j/256, Ptr1[j*stride/4+i], Ptr2[j*stride/4+i], diff);
                Count++;
            }
        }

    fprintf(fh, "%s %s difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);
    printf("%s %s difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);

    free(Ptr1);
    free(Ptr2);

    fclose(fh);
}

void Comp2ImageFileColor(char * filename1, char * filename2, char * filename3, int width, int stride, int height)
{
    unsigned int Count = 0;
    unsigned int size = sizeof(int)*stride*height;    // in bytes
    unsigned int * Ptr1 = (unsigned int *) malloc(size);
    unsigned int * Ptr2 = (unsigned int *) malloc(size);

    ReadRawDataFile(filename1, (unsigned char *) Ptr1, size);
    ReadRawDataFile(filename2, (unsigned char *) Ptr2, size);

    FILE * fh = fopen(filename3, "w");

    for (int j = 0; j < height; j++)
        for (int i = 0; i < stride; i++) {
			if (i >= width)
				continue;
			int p = Ptr1[j*stride+i];
			int q = Ptr2[j*stride+i];
			int diff = abs(p-q);
            if (diff) {
                fprintf(fh, "offset=%d:\t %08X, %08X, %08X\n", j*stride+i, p, q, diff);
                Count++;
            }
		}

    fprintf(fh, "%s %s byte difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);
    printf("%s %s byte difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);

    free(Ptr1);
    free(Ptr2);

    fclose(fh);
}

void Comp2ImageFileInt(char * filename1, char * filename2, char * filename3, int width, int stride, int height)
{
	unsigned int hist[5];
	for (int i = 0; i < 5; i++)
		hist[i] = 0;

    unsigned int Count = 0;
    int size = stride*height;    // in bytes
    int * Ptr1 = (int *) malloc(size);
    int * Ptr2 = (int *) malloc(size);

    ReadRawDataFile(filename1, (unsigned char *) Ptr1, size);
    ReadRawDataFile(filename2, (unsigned char *) Ptr2, size);

    FILE * fh = fopen(filename3, "w");

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i+=2) {
            int zMin_diff = abs(*(Ptr1+j*stride/4+i) - *(Ptr2+j*stride/4+i));
            int zMax_diff = abs(*(Ptr1+j*stride/4+i+1) - *(Ptr2+j*stride/4+i+1));
            if (zMin_diff | zMax_diff) {
                fprintf(fh, "x=%d, y=%d:\t%d, %d, %d,, %d, %d, %d\n", i, j, 
						Ptr1[j*stride/4+i], Ptr2[j*stride/4+i], zMin_diff,
						Ptr1[j*stride/4+i+1], Ptr2[j*stride/4+i+1], zMax_diff);
                if (zMin_diff)
					Count++;
                if (zMax_diff)
					Count++;
            }

			if (zMin_diff < 5)
				hist[zMin_diff]++;
			else 
				hist[4]++;
			if (zMax_diff < 5)
				hist[zMax_diff]++;
			else
				hist[4]++;
        }

    fprintf(fh, "%s %s difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);
    printf("%s %s difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);
	
	printf("Window distribution:\n");
	for (int i = 0; i < 5; i++)
		printf("%d: %d, %3.3f%%\n", i, hist[i], 100.0f*hist[i]/height/width);

    free(Ptr1);
    free(Ptr2);

    fclose(fh);
}

void Comp2ImageFileByte(char * filename1, char * filename2, char * filename3, int width, int stride, int height)
{
    unsigned int Count = 0;
    int size = stride*height;    // in bytes
    unsigned char * Ptr1 = (unsigned char *) malloc(size);
    unsigned char * Ptr2 = (unsigned char *) malloc(size);

    ReadRawDataFile(filename1, (unsigned char *) Ptr1, size);
    ReadRawDataFile(filename2, (unsigned char *) Ptr2, size);

    FILE * fh = fopen(filename3, "w");

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
			unsigned char p = *(Ptr1+j*stride+i);
			unsigned char q = *(Ptr2+j*stride+i);
            int diff = abs(p - q); 
            if (diff) {
                fprintf(fh, "offset=%d:\t %08x, %08x, %08x\n", j*stride+i, p, q, diff);
                Count++;
            }
        }

    fprintf(fh, "%s %s byte difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);
    printf("%s %s byte difference = %3.3f%%\n", filename1, filename2, 100.0f*Count/height/width);

    free(Ptr1);
    free(Ptr2);

    fclose(fh);
}

// Remove alpha channel
void RemoveAlpha(unsigned char * pSrc, unsigned char * pDest, int Cols, int Rows, int Pitch)
{
    int k = 0;
    for (int j = 0; j < Rows; j++)
        for (int i = 0; i < Cols; i++, k+=3) {
            memcpy(pDest+k, pSrc+Pitch*j+4*i, 3);
        }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
void CModel_RGBtoNV12(unsigned char * pRGBA, int FrameWidth, int FrameHeight, unsigned char * pNV12)
{
    unsigned char pixR, pixG, pixB, pixY, pixU, pixV;

    unsigned char * pY = pNV12;
    unsigned char * pUV = pNV12 + FrameHeight * FrameWidth;

    //unsigned char * pY = (unsigned char *) malloc(FrameHeight * FrameWidth);
    unsigned char * pU = (unsigned char *) malloc(FrameHeight * FrameWidth);
    unsigned char * pV = (unsigned char *) malloc(FrameHeight * FrameWidth);

    for (int y = 0; y < FrameHeight; y++)
        for (int x = 0; x < FrameWidth; x++) {
            pixR = GetPixU8C4(pRGBA, FrameWidth, x, y, R);
            pixG = GetPixU8C4(pRGBA, FrameWidth, x, y, G);
            pixB = GetPixU8C4(pRGBA, FrameWidth, x, y, B);

            // CSC weights from msdn
            // y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
            // u = ( ( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128;
            // v = ( ( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128;

            pixY = ((66 * pixR + 129 * pixG +  25 * pixB + 128) >> 8) +  16;
            pixU = ( ( -38 * pixR -  74 * pixG + 112 * pixB + 128) >> 8) + 128;
            pixV = ( ( 112 * pixR -  94 * pixG -  18 * pixB + 128) >> 8) + 128;

            PutPix(pY, FrameWidth, x, y, pixY);
            PutPix(pU, FrameWidth, x, y, pixU);
            PutPix(pV, FrameWidth, x, y, pixV);
        }

    // Merge UV
    for (int y = 0; y < FrameHeight; y+=2)
        for (int x = 0; x < FrameWidth; x+=2) {
            pixU = (GetPix(pU, FrameWidth, x, y) + GetPix(pU, FrameWidth, x+1, y) +
                    GetPix(pU, FrameWidth, x, y+1) + GetPix(pU, FrameWidth, x+1, y+1)) >> 2;
            pixV = (GetPix(pV, FrameWidth, x, y) + GetPix(pV, FrameWidth, x+1, y) +
                    GetPix(pV, FrameWidth, x, y+1) + GetPix(pV, FrameWidth, x+1, y+1)) >> 2;
            PutPix(pUV, FrameWidth, x, y/2, pixU);
            PutPix(pUV, FrameWidth, x+1, y/2, pixV);
        }

    free(pU);
    free(pV);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CModel_Dilate(unsigned char * iBuffer, unsigned char * oBuffer, int Cols, int Rows, int KernelSize)
{    
    if ( KernelSize != 3 && KernelSize != 5 ) {
        printf("Invalid kernel size %d\n", KernelSize);
        return;
    }

    if (KernelSize == 3) {  // 3x3
        for (int y = 0; y < Rows; y++)
            for (int x = 0; x < Cols; x++) {

                int xl = (x-1 < 0) ? 0 : x-1;
                int xr = (x+1 < Cols) ? x+1 : Cols-1;
                int yt = (y-1 < 0) ? 0 : y-1;
                int yb = (y+1 < Rows) ? y+1 : Rows-1;
                
            // Set true if any of neightbors is true 
                if (GetPix(iBuffer, Cols, xl, yt) || GetPix(iBuffer, Cols, x, yt) || GetPix(iBuffer, Cols, xr, yt) ||
                    GetPix(iBuffer, Cols, xl, y) ||                                  GetPix(iBuffer, Cols, xr, y) ||
                    GetPix(iBuffer, Cols, xl, yb) || GetPix(iBuffer, Cols, x, yb) || GetPix(iBuffer, Cols, xr, yb)) {
                    
                    PutPix(oBuffer, Cols, x, y, (unsigned char) 0xFF);
                } else {
                    PutPix(oBuffer, Cols, x, y, GetPix(iBuffer, Cols, x, y));
                }
            }
    } else {    // 5x5
        for (int y = 0; y < Rows; y++)
            for (int x = 0; x < Cols; x++) {

                int xl2 = (x-2 < 0) ? 0 : x-2;
                int xl1 = (x-1 < 0) ? 0 : x-1;
                int xr1 = (x+1 < Cols) ? x+1 : Cols-1;
                int xr2 = (x+2 < Cols) ? x+2 : Cols-1;

                int yt2 = (y-2 < 0) ? 0 : y-2;
                int yt1 = (y-1 < 0) ? 0 : y-1;
                int yb1 = (y+1 < Rows) ? y+1 : Rows-1;
                int yb2 = (y+2 < Rows) ? y+2 : Rows-1;

                if (GetPix(iBuffer, Cols, xl2, yt2) || GetPix(iBuffer, Cols, xl1, yt2) || GetPix(iBuffer, Cols, x, yt2) || GetPix(iBuffer, Cols, xr1, yt2) || GetPix(iBuffer, Cols, xr2, yt2) ||
                    GetPix(iBuffer, Cols, xl2, yt1) || GetPix(iBuffer, Cols, xl1, yt1) || GetPix(iBuffer, Cols, x, yt1) || GetPix(iBuffer, Cols, xr1, yt1) || GetPix(iBuffer, Cols, xr2, yt1) ||
                    GetPix(iBuffer, Cols, xl2, y)   || GetPix(iBuffer, Cols, xl1, y)   ||                                  GetPix(iBuffer, Cols, xr1, y)   || GetPix(iBuffer, Cols, xr2, y) ||
                    GetPix(iBuffer, Cols, xl2, yb1) || GetPix(iBuffer, Cols, xl1, yb1) || GetPix(iBuffer, Cols, x, yb1) || GetPix(iBuffer, Cols, xr1, yb1) || GetPix(iBuffer, Cols, xr2, yb1) ||
                    GetPix(iBuffer, Cols, xl2, yb2) || GetPix(iBuffer, Cols, xl1, yb2) || GetPix(iBuffer, Cols, x, yb2) || GetPix(iBuffer, Cols, xr1, yb2) || GetPix(iBuffer, Cols, xr2, yb2) ) {
                    
                    PutPix(oBuffer, Cols, x, y, (unsigned char) 0xFF);
                } else {
                    PutPix(oBuffer, Cols, x, y, GetPix(iBuffer, Cols, x, y));
                }

            }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3x3
void CModel_Erode(unsigned char * iBuffer, unsigned char * oBuffer, int Cols, int Rows, int KernelSize)
{    
    if ( KernelSize != 3 && KernelSize != 5 ) {
        printf("Invalid kernel size %d\n", KernelSize);
        return;
    }

    if (KernelSize == 3) {  // 3x3
        for (int y = 0; y < Rows; y++)
            for (int x = 0; x < Cols; x++) {

                int xl = (x-1 < 0) ? 0 : x-1;
                int xr = (x+1 < Cols) ? x+1 : Cols-1;
                int yt = (y-1 < 0) ? 0 : y-1;
                int yb = (y+1 < Rows) ? y+1 : Rows-1;

                // Set to false if any of neightbors are false 
                if (!GetPix(iBuffer, Cols, xl, yt) || !GetPix(iBuffer, Cols, x, yt) || !GetPix(iBuffer, Cols, xr, yt) ||
                    !GetPix(iBuffer, Cols, xl, y) ||                                   !GetPix(iBuffer, Cols, xr, y) ||
                    !GetPix(iBuffer, Cols, xl, yb) || !GetPix(iBuffer, Cols, x, yb) || !GetPix(iBuffer, Cols, xr, yb)) {
                    
                    PutPix(oBuffer, Cols, x, y, (unsigned char) 0);
                } else {
                    PutPix(oBuffer, Cols, x, y, GetPix(iBuffer, Cols, x, y));
                }
            }
    } else {    // 5x5
        for (int y = 0; y < Rows; y++)
            for (int x = 0; x < Cols; x++) {

                int xl2 = (x-2 < 0) ? 0 : x-2;
                int xl1 = (x-1 < 0) ? 0 : x-1;
                int xr1 = (x+1 < Cols) ? x+1 : Cols-1;
                int xr2 = (x+2 < Cols) ? x+2 : Cols-1;

                int yt2 = (y-2 < 0) ? 0 : y-2;
                int yt1 = (y-1 < 0) ? 0 : y-1;
                int yb1 = (y+1 < Rows) ? y+1 : Rows-1;
                int yb2 = (y+2 < Rows) ? y+2 : Rows-1;

                if (!GetPix(iBuffer, Cols, xl2, yt2) || !GetPix(iBuffer, Cols, xl1, yt2) || !GetPix(iBuffer, Cols, x, yt2) || !GetPix(iBuffer, Cols, xr1, yt2) || !GetPix(iBuffer, Cols, xr2, yt2) ||
                    !GetPix(iBuffer, Cols, xl2, yt1) || !GetPix(iBuffer, Cols, xl1, yt1) || !GetPix(iBuffer, Cols, x, yt1) || !GetPix(iBuffer, Cols, xr1, yt1) || !GetPix(iBuffer, Cols, xr2, yt1) ||
                    !GetPix(iBuffer, Cols, xl2, y)   || !GetPix(iBuffer, Cols, xl1, y)   ||                                   !GetPix(iBuffer, Cols, xr1, y)   || !GetPix(iBuffer, Cols, xr2, y) ||
                    !GetPix(iBuffer, Cols, xl2, yb1) || !GetPix(iBuffer, Cols, xl1, yb1) || !GetPix(iBuffer, Cols, x, yb1) || !GetPix(iBuffer, Cols, xr1, yb1) || !GetPix(iBuffer, Cols, xr2, yb1) ||
                    !GetPix(iBuffer, Cols, xl2, yb2) || !GetPix(iBuffer, Cols, xl1, yb2) || !GetPix(iBuffer, Cols, x, yb2) || !GetPix(iBuffer, Cols, xr1, yb2) || !GetPix(iBuffer, Cols, xr2, yb2) ) {
                    
                    PutPix(oBuffer, Cols, x, y, (unsigned char) 0);
                } else {
                    PutPix(oBuffer, Cols, x, y, GetPix(iBuffer, Cols, x, y));
                }
            }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// X = X & ~Y
void CModel_BuildMask(unsigned char * bBuffer1, unsigned char * bBuffer2, int Cols, int Rows)
{

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

            unsigned char Y = GetPix(bBuffer2, Cols, x, y);
            if (Y) {
                unsigned char X = GetPix(bBuffer1, Cols, x, y);
                PutPix(bBuffer1, Cols, x, y, (unsigned char) (X & ~Y));
            }
        }
}

///////////////////////////////////////////////////////////////////////////////////////
// Flip left to right
void CModel_FlipX(unsigned int * pInBuf, unsigned int * pOutBuf, int Cols, int Rows, int Pitch)
{
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x <= Cols/2; x++) {
            
            unsigned int pix = GetPix(pInBuf, Pitch, x, y);
            unsigned int pix2 = GetPix(pInBuf, Pitch, (Cols-1)-x, y);
            PutPix(pOutBuf, Pitch, (Cols-1)-x, y, pix);
            PutPix(pOutBuf, Pitch, x, y, pix2);
        }
}

///////////////////////////////////////////////////////////////////////////////////////
// RGBA to BGRA, swap R and B
void CModel_RGBA2BGRA(unsigned int * pInBuf, unsigned int * pOutBuf, int Cols, int Rows)
{
unsigned int pix;
unsigned char * p;
unsigned char tmp;

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {
            
            pix = GetPix(pInBuf, Cols, x, y);
            p = (unsigned char *) &pix;

            tmp = *p;
            *p = *(p+2);
            *(p+2) = tmp;

            PutPix(pOutBuf, Cols, x, y, pix);
        }
}
