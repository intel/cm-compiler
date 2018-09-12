// GC_App.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"  
#include <iostream>  
#include <limits>
#include <stdio.h>
#include <io.h>
#include <direct.h>


#include "GC_API.h"  

#define ALIGNMENT32			32
#define ALIGNMENT4K			4096

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

//////////////////////////////////////////////////////////////////////////////////////////
int ReadInput(char iFile[], short * ptr, int FrameWidth, int FrameHeight)
{
float * iOrigBuffer;
float InputFrameFactor = 4.0f;
float fDataScaler = 1.0;

	// Process node file
   	FILE * fh = fopen(iFile, "rb");
    if (fh == NULL) {
		printf("Error opening file %s.\n", iFile);
        return -1;
	}
	int OrigInputFrameSize = (int) (FrameWidth * FrameHeight * InputFrameFactor);
	iOrigBuffer = (float *) _aligned_malloc(OrigInputFrameSize, ALIGNMENT32);
	if (!iOrigBuffer) {
        printf("Failed allocate iOrigBuffer\n");
        return -1;
    }
	if (fread(iOrigBuffer, sizeof( char ), OrigInputFrameSize, fh) != OrigInputFrameSize) {
		printf("Unable to read a frame of data.");
        return -1;
    }
    for (int i = 0; i < FrameWidth * FrameHeight; i++)
        *(ptr + i) = (short) (*(iOrigBuffer + i) * fDataScaler);		// float to short

	_aligned_free(iOrigBuffer);
	fclose(fh);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[])
{
char LineBuffer[256];

char NodeFilename[64]; 
char HoriWeightFilename[64]; 
char VertWeightFilename[64]; 
char OutputFilename[64]; 

short * pNode;
short * pHoriWeight;
short * pVertWeight;
unsigned char * pOutput;

//char CPU_fn[128];
//char GPU_fn[128];
//char diff_fn[128];

int FrameWidth, FrameHeight;
//int BlkWidth = 16;
//int BlkHeight = 16;

//int nBits  = 5;			// Lower bits to check
//int nRatio = 1 << nBits;
//int ActivePixKnob = 0;
//int iter_AVX2;
float InputFrameFactor = 4.0;
float OutputFrameFactor = 1.0;

	for (int i = 1; i < argc; i++) {
		strcpy(LineBuffer, argv[i]);

		if (LineBuffer[0] != '-') {
			printf("%s is an invalid parameter\n", LineBuffer);
	        return -1;
		} else {
			if (LineBuffer[1] == 'i' && LineBuffer[2] == ':') 		// node_weight file
				strcpy(NodeFilename, &LineBuffer[3]);
			else if (LineBuffer[1] == 'j' && LineBuffer[2] == ':')	// horizontal_weight file
				strcpy(HoriWeightFilename, &LineBuffer[3]);
			else if (LineBuffer[1] == 'k' && LineBuffer[2] == ':')	// vertical_weight file
				strcpy(VertWeightFilename, &LineBuffer[3]);
			else if (LineBuffer[1] == 'o' && LineBuffer[2] == ':')	// Output file
				strcpy(OutputFilename, &LineBuffer[3]);

			else if (LineBuffer[1] == 'w' && LineBuffer[2] == ':')	// Frame width
				sscanf(&LineBuffer[3], "%d", &FrameWidth);
			else if (LineBuffer[1] == 'h' && LineBuffer[2] == ':')	// Frame height
				sscanf(&LineBuffer[3], "%d", &FrameHeight);
			else {
				printf("%s is an invalid parameter\n", LineBuffer);
		        return -1;
			}
		}
	}

	printf("\nInput parameters:\n");
	printf("Node weight file: %s\n", NodeFilename);
	printf("Horizontal weight file: %s\n", HoriWeightFilename);
	printf("Vertical weight file: %s\n", VertWeightFilename);
	printf("Output file: %s\n", OutputFilename);
	printf("Frame width: %d\n", FrameWidth);
	printf("Frame height: %d\n\n", FrameHeight);

	// Call GC dll to detect MDF
	if (!_GC::Detect_MDF()) {
		return -1;
	}

	int InputFrameSize = (int) (FrameWidth * FrameHeight * sizeof(short));

	pNode = (short *) _aligned_malloc(InputFrameSize, ALIGNMENT32);
    if (!pNode) {
		printf("Failed allocate pNode\n");
	    return -1;
	}
	if (ReadInput(NodeFilename, pNode, FrameWidth, FrameHeight) == -1)
		return -1;

	pHoriWeight = (short *) _aligned_malloc(InputFrameSize, ALIGNMENT32);
    if (!pHoriWeight) {
		printf("Failed allocate pHoriWeight\n");
	    return -1;
	}
	if (ReadInput(HoriWeightFilename, pHoriWeight, FrameWidth, FrameHeight) == -1)
		return -1;

	pVertWeight = (short *) _aligned_malloc(InputFrameSize, ALIGNMENT32);
    if (!pVertWeight) {
		printf("Failed allocate pVertWeight\n");
	    return -1;
	}
	if (ReadInput(VertWeightFilename, pVertWeight, FrameWidth, FrameHeight) == -1)
		return -1;

	// Allocate output buffers
	int OutputFrameSize = (int) (FrameWidth * FrameHeight * OutputFrameFactor);   
    pOutput = (unsigned char *) _aligned_malloc(OutputFrameSize, ALIGNMENT32);
	if (!pOutput) {
        printf("Failed allocate OutputBuffer3\n");
		return -1;
    }

	// Call Graph Cut API
	_GC::CM_GraphCut( pNode, pHoriWeight, pVertWeight, pOutput, FrameWidth, FrameHeight );

	// Write the result to a file
    FILE * fh = fopen(OutputFilename, "wb");
    if (fh == NULL) {
        printf("Error opening %s\n", OutputFilename);
        return -1;
    }
    if (fwrite(pOutput, sizeof( char ), OutputFrameSize, fh) != OutputFrameSize) {
        printf("Unable to write data to %s.\n", OutputFilename);
        return -1;
    }
    fclose(fh);

	// Call Graph Cut API (CPU version)
	_GC::CPU_GraphCut( pNode, pHoriWeight, pVertWeight, pOutput, FrameWidth, FrameHeight );

	// Write the result to a file
	char CPUOutput[128];
	strcpy(CPUOutput, "CPU_");
	strcat(CPUOutput, OutputFilename);
    fh = fopen(CPUOutput, "wb");
    if (fh == NULL) {
        printf("Error opening %s\n", CPUOutput);
        return -1;
    }
    if (fwrite(pOutput, sizeof( char ), OutputFrameSize, fh) != OutputFrameSize) {
        printf("Unable to write data to %s.\n", CPUOutput);
        return -1;
    }
    fclose(fh);

	_aligned_free(pNode);
	_aligned_free(pHoriWeight);
	_aligned_free(pVertWeight);
	_aligned_free(pOutput);

    return 0;  
}  

