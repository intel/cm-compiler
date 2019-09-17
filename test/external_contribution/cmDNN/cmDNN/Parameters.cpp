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
#include <direct.h>
#else
#include <unistd.h>
#define _getcwd getcwd
#endif
//#include "half.h"
#include <cm_rt.h>

#include "cmDNN.h"

///////////////////////////////////////////////////////////////////////////////////////////////
Layer_Param::Layer_Param()
{
	IsInputlayer = 0;
	IsActive = 1;

	memset(LayerType, 0, sizeof(LayerType));
	memset(InputFile, 0, sizeof(InputFile));
	memset(RefFile, 0, sizeof(RefFile));
	memset(WeightFile, 0, sizeof(WeightFile));	
	memset(BiasFile, 0, sizeof(BiasFile));	

	SrcWidth = 0;				
	SrcHeight = 0;
	SrcDepth = 1;

	BatchSize = 1;

	KernelWidth = 0;
	KernelHeight = 0;
	KernelStride = 1;

	WindowChannelSize = 0;
	Alpha = 0.0;
	Beta = 0.0;
	K = 0.0;

	DestWidth = 0;
	DestHeight = 0;
	DestDepth = 1;

	InputBorderWidth = 0;
	OutputBorderWidth = 0;
	NumGroups = 1;
	EnableReLU = 0;
}

Layer_Param::~Layer_Param()
{
}

char * Layer_Param::FindChar(char * buffer, int sLength, char c)
{
char * p = buffer;
int count = 0;

	while (*p != c && count < sLength) {
		p++;
		count++;
	}
	p++;
	return p;
}


void Layer_Param::GetStringParam(char * SrcBuffer, char * DestBuffer)
{
	int sLength = strlen(SrcBuffer);
	char * ptr = FindChar(SrcBuffer, sLength, '=');
	strcpy(DestBuffer, ptr);
	DestBuffer[strlen(DestBuffer)-1] = 0;
}

void Layer_Param::GetIntParam(char * SrcBuffer, int & Dest)
{
	int sLength = strlen(SrcBuffer);
	char * ptr = FindChar(SrcBuffer, sLength, '=');
	char * token = strtok(ptr, ",\n");
	Dest = atoi(token);
}

void Layer_Param::GetFloatParam(char * SrcBuffer, float & Dest)
{
	int sLength = strlen(SrcBuffer);
	char * ptr = FindChar(SrcBuffer, sLength, '=');
	char * token = strtok(ptr, ",\n");
	Dest = atof(token);
}

void Layer_Param::GetDoubleParam(char * SrcBuffer, double & Dest)
{
	int sLength = strlen(SrcBuffer);
	char * ptr = FindChar(SrcBuffer, sLength, '=');
	char * token = strtok(ptr, ",\n");
	Dest = atof(token);
}

///////////////////////////////////////////////////////////////////
void Layer_Param::ReadParam(char * param_filename)
{
char buffer[1024];

	FILE * fh = fopen(param_filename, "r");
	if (!fh ) {
		printf("errno = %d\n", errno);
		_getcwd(buffer, 1024);
		printf("Error opening parameter file %s in %s\n", param_filename, buffer);
		exit(-1);
	}

	// Parse parameters
	while (feof(fh) == 0) {
		memset(buffer, 0, sizeof(buffer));
		if (fgets(buffer, sizeof(buffer)-1, fh) == NULL)
			continue;
		if (buffer[0] == '\n' || buffer[0] == '/' && buffer[1] == '/')	// Skip empty line and comment line
			continue;
#ifndef WIN32
		if (buffer[0] == '\r' && buffer[1] == '\n')
			continue;
		int bufLen = strlen(buffer);
		if (buffer[bufLen-1] == '\n' && buffer[bufLen-2] == '\r')
		{
			buffer[bufLen-2] = '\n';
			buffer[bufLen-1] = '\0';
		}
#endif

		if (strstr(buffer, "LayerType")) {
			GetStringParam(buffer, LayerType);
		} else if (strstr(buffer, "InputFile")) {
			GetStringParam(buffer, InputFile);
		} else if (strstr(buffer, "RefFile")) {
			GetStringParam(buffer, RefFile);
		} else if (strstr(buffer, "WeightFile")) {
			GetStringParam(buffer, WeightFile);
		} else if (strstr(buffer, "BiasFile")) {
			GetStringParam(buffer, BiasFile);

		} else if (strstr(buffer, "SrcWidth")) {
			GetIntParam(buffer, SrcWidth);
		} else if (strstr(buffer, "SrcHeight")) {
			GetIntParam(buffer, SrcHeight);
		} else if (strstr(buffer, "SrcDepth")) {
			GetIntParam(buffer, SrcDepth);

		} else if (strstr(buffer, "KernelWidth")) {
			GetIntParam(buffer, KernelWidth);
		} else if (strstr(buffer, "KernelHeight")) {
			GetIntParam(buffer, KernelHeight);
		} else if (strstr(buffer, "KernelStride")) {
			GetIntParam(buffer, KernelStride);

		} else if (strstr(buffer, "WindowChannelSize")) {
			GetIntParam(buffer, WindowChannelSize);
		} else if (strstr(buffer, "Alpha")) {
			GetDoubleParam(buffer, Alpha);
		} else if (strstr(buffer, "Beta")) {
			GetDoubleParam(buffer, Beta);
		} else if (strstr(buffer, "K")) {
			GetDoubleParam(buffer, K);

		} else if (strstr(buffer, "DestWidth")) {
			GetIntParam(buffer, DestWidth);
		} else if (strstr(buffer, "DestHeight")) {
			GetIntParam(buffer, DestHeight);
		} else if (strstr(buffer, "DestDepth")) {
			GetIntParam(buffer, DestDepth);

		} else if (strstr(buffer, "InputBorderWidth")) {
			GetIntParam(buffer, InputBorderWidth);
		} else if (strstr(buffer, "OutputBorderWidth")) {
			GetIntParam(buffer, OutputBorderWidth);
		} else if (strstr(buffer, "NumGroups")) {
			GetIntParam(buffer, NumGroups);
		} else if (strstr(buffer, "EnableReLU")) {
			GetIntParam(buffer, EnableReLU);
		}

	}	// while
	fclose(fh);
}

//////////////////////////////////////////////////////////
void Layer_Param::PrintParam(int Layer)
{
	printf("Layer %d %s parameters:\n", Layer, LayerType); 
//	printf("\tInput image file: %s\n", InputFile); 
	if (strlen(WeightFile) != 0)
		printf("\tWeight file: %s\n", WeightFile); 
	if (strlen(BiasFile) != 0)
		printf("\tBias file: %s\n", BiasFile); 
	printf("\tInput image size: WxHxD = %dx%dx%d pixels\n", SrcWidth, SrcHeight, SrcDepth); 
	printf("\tOutput image size: WxHxD = %dx%dx%d pixels\n", DestWidth, DestHeight, DestDepth); 
	printf("\tBatch size: %d\n", BatchSize);
	printf("\tKernel size: %dx%d\n", KernelWidth, KernelHeight);
	printf("\tKernel Stride: %d\n", KernelStride);
	printf("\tInput border width: %d\n", InputBorderWidth);
	printf("\tOutput border width: %d\n", OutputBorderWidth);
	printf("\t# of groups: %d\n", NumGroups);
	printf("\tEnable ReLU: %d\n", EnableReLU);
}
