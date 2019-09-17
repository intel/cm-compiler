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

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

#ifdef WIN32
#include <dxgi.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#define _access access
#include <sys/types.h>
#include <sys/stat.h>
#define _mkdir mkdir
#endif

#ifdef _DEBUG
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

#include "cmDNN.h"

/////////////////////////////////////////////////////////////////////////////////////
double ElapsedMS(LARGE_INTEGER StartingTime, LARGE_INTEGER EndingTime)
{
	LARGE_INTEGER Frequency, ElapsedMicroseconds;

	QueryPerformanceFrequency(&Frequency); 
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;			// In microsecond
	return (double) (ElapsedMicroseconds.QuadPart / 1000.0);	// In millisecond
}

/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[])
{
	// Default values if not specified on command line
	int netIdx = ALEXNET;
	int batchSize = 1;
	int numIter = 50;
	char InputFile[128] = {0};

	int UseFP16 = 0;	// 0 for FP32, 1 for FP16

	float totalTime = 0;
	LARGE_INTEGER StartingTime, EndingTime;

	// Parse command line parameters
	char LineBuffer[256];
	for (int i = 1; i < argc; i++) {
		strcpy(LineBuffer, argv[i]);

		if (LineBuffer[0] != '-') {
			printf("%s is an invalid parameter\n", LineBuffer);
			exit(0);
		} else {
			if (LineBuffer[1] == 'n' && LineBuffer[2] == ':')		// Network. 0: ALEXNET, 1: VGG16, 2:VGG19, 3: RESNET50		
				sscanf(&LineBuffer[3], "%d", &netIdx);
			else if (LineBuffer[1] == 'b' && LineBuffer[2] == ':')	// Batch size
				sscanf(&LineBuffer[3], "%d", &batchSize);
			else if (LineBuffer[1] == 'l' && LineBuffer[2] == ':')	// Iterations
				sscanf(&LineBuffer[3], "%d", &numIter);
			else {
				printf("%s is an invalid parameter\n", LineBuffer);
				exit(0);
			}
		}
	}

	if (batchSize > 32) {
		printf("Batch size exceeds the maximum size 32.\n");
		exit(0);
	}

	printf("Executing %s %d times with batch size = %d.\n\n", NETWORK[netIdx], numIter, batchSize);

	// Create ouptut folder
#ifdef _DEBUG
#ifdef WIN32
	char OutputFolder[] = ".\\Output";
	if (_access(OutputFolder, 0) == -1)
		_mkdir(OutputFolder);
	else
		system("del /q .\\Output\\*.*");
#else
	char OutputFolder[] = "Output";
	if (_access(OutputFolder, 0) == -1)
		mkdir(OutputFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	else
		system("del /q ./Output/*.*");
#endif
#endif

	// Create cm pipeline, a placeholder for layers.
	CM_PIPELINE cmPipe;
	cmPipe.cmBase.InitGPU("cmDNN_genx.isa");	// Init iGPU

	if (netIdx == ALEXNET)
		cmPipe.Build_AlexNet(InputFile, batchSize, UseFP16);
	else if (netIdx == VGG16)
		cmPipe.Build_VGG16(InputFile, batchSize, UseFP16);
	else if (netIdx == VGG19)
		cmPipe.Build_VGG19(InputFile, batchSize, UseFP16);
	else if (netIdx == RESNET50)
		cmPipe.Build_Resnet50(InputFile, batchSize, UseFP16);
	else {
		printf("Unkown network. Exit\n");
		exit(0);
	}
	
	cmPipe.UpdateBorderWidth();			// Update border
	cmPipe.AllocateCMLayers();			// Allocate mem

	CM_LAYER * LastLayer = &cmPipe.cmLayer[cmPipe.NumLayers-1];

	for (int iter=0 ; iter<numIter ; iter++){

		// Check for input file exists.
		sprintf(InputFile, "../Data/%s/Input/Input%d.txt", NETWORK[netIdx], batchSize);
		if (_access(InputFile, 0) == -1) {
			printf("Input file %s does not exist.\n", InputFile);
			exit(0);
		}

		cmPipe.ReadInputData(InputFile);
		
		QueryPerformanceCounter(&StartingTime);

		cmPipe.Enqueue_KernelArray();

		// Wait for the last loop finished
		LastLayer->GPUSync();

		QueryPerformanceCounter(&EndingTime);

		// Get each layer's performance
		for (int layerIdx = 0; layerIdx < cmPipe.NumLayers; layerIdx++) {
			float LayerPerf = cmPipe.cmLayer[layerIdx].GetLayerPerformance();  // Get layer performance in ms
			printf("layer %3d avg GPU time = %9.2f ms\n", layerIdx, LayerPerf);
		}


#ifdef _DEBUG
		// Compare ouptut with refernece, save output images
		for (int layerIdx = 0; layerIdx < cmPipe.NumLayers; layerIdx++)
			cmPipe.cmLayer[layerIdx].CompareOutput(netIdx);
#endif

		//cmPipe.cmLayer[cmPipe.NumLayers-1]->getLayerOutput(net->outputData);	// Copy output to app provided buffer.
		// Copy to CM OutputBuffer
		if (LastLayer->OutputMemType == MEMORY_2D)
			LastLayer->CopyFromGPUSurface();
		else
			LastLayer->CopyFromGPUBuffer();

		// Final output to a file, and find the highest detection rate in output.
		float Pix, MaxValue = 0.0f;
		int MaxIdx = 0; 
		FILE * fh = fopen("iGPU_Output.csv", "w");

		float * pBase = LastLayer->OutputBuffer;
		int oWidth = LastLayer->Param.DestWidth;
		int oHeight = LastLayer->Param.DestHeight;
		int oDepth = LastLayer->Param.DestDepth;

		for (int i = 0; i < oWidth*oHeight*oDepth; i++) {
			fprintf(fh, "%.5f\n", pBase[i]);
			if (pBase[i] > MaxValue) {
				MaxIdx = i;
				MaxValue = pBase[i];
			}
		}
		fclose(fh);
		printf("\nDetection Rate = %f%% at offset %d.\n", 100 * MaxValue, MaxIdx);

		double ElapsedTime = ElapsedMS(StartingTime, EndingTime);
		printf("Sample normalized predict time = %.2f ms.\n\n", ElapsedTime/batchSize);

		totalTime += ElapsedTime;
	}

	printf("Batch size = %d\n", batchSize);
	printf("Average time to run %s = %.2f ms  (%.2f fps)\n", 
		NETWORK[netIdx], totalTime/numIter/batchSize, 1000.0/(totalTime/numIter/batchSize));
	
	return 0;
}
