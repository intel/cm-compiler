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
#include <io.h>
//#include <cm/half.h>
#include <cm/cm.h>
#endif

#include "cmDNN.h"

///////////////////////////////////////////////////////////////////
CM_PIPELINE::CM_PIPELINE()
{
	NumLayers = 0;
}

///////////////////////////////////////////////////////////////////
CM_PIPELINE::~CM_PIPELINE()
{
	
}

//////////////////////////////////////////////////////////////////
void CM_PIPELINE::UpdateLayerID()
{
	if (NumLayers + 1 > MAX_LAYERS) {
		printf("Exceed maximum allowed layers.\n");
		exit(-1);
	}
	else
		NumLayers++;		// Increase by 1 for next layer creation.
}

///////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateInputLayer(	char * ParamFile, 
									char * InputFile,
									char * cnnAvgFile,
									int BatchSize, 
									int UseFP16,
									int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "Input");
	strcpy(CurParam->InputFile, InputFile);
	strcpy(CurParam->BiasFile, cnnAvgFile);

	CurLayer->LayerID		= NumLayers;
	CurLayer->pCm			= &cmBase;
	CurLayer->UseFP16		= UseFP16;
	CurParam->IsActive		= IsActive;
	CurParam->BatchSize		= BatchSize;
	CurParam->IsWithAvg		= 1;
	CurParam->KernelWidth	= 1;					// flag
	CurParam->IsInputlayer	= 1;

	CurParam->ReadParam(ParamFile);

	//	pWeights = MW_inputData;				// hold input data pointer, data will be filled later
	//	pBias = MW_mangled_a;					// hold cnnAvg data

	UpdateLayerID();
}

/////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateConvolLayer(char * ParamFile,
									char * weight_file,
									char * bias_file,
									int BatchSize,
									int UseFP16,
									int NumOfInputs,
									CM_LAYER * pPrevLayerA,
									CM_LAYER * pPrevLayerB,
									int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "Convol");
	strcpy(CurParam->WeightFile, weight_file);
	strcpy(CurParam->BiasFile, bias_file);

	CurLayer->LayerID			= NumLayers;
	CurLayer->pCm				= &cmBase;
	CurLayer->UseFP16			= UseFP16;
	CurLayer->NumInputs			= NumOfInputs;
	CurParam->BatchSize			= BatchSize;
	CurParam->IsActive			= IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayerA;
	if (NumOfInputs == 2)
		CurLayer->pPrevLayer2 = pPrevLayerB;

	CurParam->DestWidth = (CurParam->SrcWidth - CurParam->KernelWidth + 2 * CurParam->InputBorderWidth) / CurParam->KernelStride + 1;
	CurParam->DestHeight = (CurParam->SrcHeight - CurParam->KernelHeight + 2 * CurParam->InputBorderWidth) / CurParam->KernelStride + 1;

	UpdateLayerID();
}

//////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateLRNLayer(char * ParamFile,
								int BatchSize,
								int UseFP16,
								int NumOfInputs,
								CM_LAYER * pPrevLayer,
								int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "LRN");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

//////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateMaxPoolLayer(char * ParamFile,
									int BatchSize,
									int UseFP16,
									int NumOfInputs,
									CM_LAYER * pPrevLayer,
									int IsActive) 
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "MaxPool");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

//////////////////////////////////////////////////////////
void CM_PIPELINE::CreateAvgPoolLayer(char * ParamFile,
									int BatchSize,
									int UseFP16,
									int NumOfInputs,
									CM_LAYER * pPrevLayer,
									int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "AvgPool");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

///////////////////////////////////////////////////////
void CM_PIPELINE::CreateFCLayer(char * ParamFile,
								char * weight_file,
								char * bias_file,
								int BatchSize,
								int UseFP16,
								int NumOfInputs,
								CM_LAYER * pPrevLayer,
								int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "FC");
	strcpy(CurParam->WeightFile, weight_file);
	strcpy(CurParam->BiasFile, bias_file);

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

//////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateReLULayer(char * ParamFile,
									int BatchSize,
									int UseFP16,
									int NumOfInputs,
									CM_LAYER * pPrevLayer,
									int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "ReLU");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

//////////////////////////////////////////////////////////////////
void CM_PIPELINE::CreateSurfConvLayer(	char * ParamFile,
										int BatchSize,
										int UseFP16,
										int NumOfInputs,
										CM_LAYER * pPrevLayer,
										int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "SurfConv");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

////////////////////////////////////////////////////////
void CM_PIPELINE::CreateSoftMaxLayer(char * ParamFile,
									int BatchSize,
									int UseFP16,
									int NumOfInputs,
									CM_LAYER * pPrevLayer,
									int IsActive)
{
	// NumLayers is the current layer ID
	CM_LAYER * CurLayer = &cmLayer[NumLayers];
	Layer_Param * CurParam = &CurLayer->Param;

	strcpy(CurParam->LayerType, "SoftMax");

	CurLayer->LayerID = NumLayers;
	CurLayer->pCm = &cmBase;
	CurLayer->UseFP16 = UseFP16;
	CurLayer->NumInputs = NumOfInputs;
	CurParam->BatchSize = BatchSize;
	CurParam->IsActive = IsActive;

	CurParam->ReadParam(ParamFile);

	CurLayer->pPrevLayer = pPrevLayer;

	UpdateLayerID();
}

/*
//////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::SetFP16(CM_LAYER * Layers[], int NumLayers, int UseFP16)
{
	for (int i = 0; i < NumLayers; i++) {
		Layers[i]->UseFP16 = UseFP16;
	}

	printf("\nInternal data type: %s\n", UseFP16 ? "FP16" : "FP32");
}

/////////////////////////////////////////////////////////////////////////
// Quantized FC weight (0: No quantization; 1: Int8; 2: Short)
void CM_PIPELINE::SetqFCWt(CM_LAYER * Layers[], int NumLayers, int qFCWt)
{
	// Set qFCWt for FC layers only
	for (int i = 0; i < NumLayers; i++) {
		Layers[i]->qFCWt = (!strcmp(Layers[i]->Param.LayerType, "FC")) ? qFCWt : 0;
	}
	
	if (qFCWt == 0)
		printf("FC weight data type: FP32\n\n");
	else if (qFCWt == 1)
		printf("FC weight data type: In8\n\n");
}
*/

///////////////////////////////////////////////////////////////////////
// Reserve input and output border width for use by future layes.
void CM_PIPELINE::UpdateBorderWidth()
{
	// update input layer[0] borders to follow layer[1] input border.
	if (cmLayer[0].Param.OutputBorderWidth < cmLayer[1].Param.InputBorderWidth) {
		cmLayer[0].Param.OutputBorderWidth = cmLayer[1].Param.InputBorderWidth;
		cmLayer[0].Param.InputBorderWidth = cmLayer[0].Param.OutputBorderWidth;
	}

	// for all other cmLayer, update output border.  The current layer output border should be the biggest input border of the all following layer. 
	for (int j = 1; j < NumLayers; j++) {
		for (int i = j+1; i < NumLayers; i++) {
			// Find the biggest input border in all cmLayer after the current layer.
			if (cmLayer[j].Param.OutputBorderWidth < cmLayer[i].Param.InputBorderWidth)
				cmLayer[j].Param.OutputBorderWidth = cmLayer[i].Param.InputBorderWidth;
		}
	}

	// Update input border.  The current layer input border should be the same as previous layer's output border if it is smaller.
	for (int j = 2; j < NumLayers; j++) {
		if (cmLayer[j].Param.InputBorderWidth < cmLayer[j-1].Param.OutputBorderWidth)
			cmLayer[j].Param.InputBorderWidth = cmLayer[j-1].Param.OutputBorderWidth;
	}

//*
	// if FP16, round up border width to next even number.
//	for (int j = 1; j < NumLayers; j++) {
	for (int j = 0; j < NumLayers; j++) {
		if (cmLayer[j].UseFP16) {		
			if (cmLayer[j].Param.InputBorderWidth & 0x01)
				cmLayer[j].Param.InputBorderWidth += 1;
			if (cmLayer[j].Param.OutputBorderWidth & 0x01)
				cmLayer[j].Param.OutputBorderWidth += 1;
		}
	}
//*/
}

///////////////////////////////////////////////////////////////////////
void CM_PIPELINE::AllocateCMLayers()
{
	for (int i = 0; i < NumLayers; i++) {
		if (!strcmp(cmLayer[i].Param.LayerType, "Input"))
			cmLayer[i].Config_Input_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "Convol"))
			cmLayer[i].Config_Convol_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "ReLU"))
			cmLayer[i].Config_ReLU_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "LRN"))
			cmLayer[i].Config_LRN_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "MaxPool"))
			cmLayer[i].Config_MaxPool_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "AvgPool"))
			cmLayer[i].Config_AvgPool_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "SurfConv"))
			cmLayer[i].Config_SurfConv_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "FC"))
			cmLayer[i].Config_FC_Layer();
		else if (!strcmp(cmLayer[i].Param.LayerType, "SoftMax"))
			cmLayer[i].Config_SoftMax_Layer();
		else
			printf("Unknown layer type: %s\n", cmLayer[i].Param.LayerType);

		cmLayer[i].Param.PrintParam(i);
		cmLayer[i].PrintLayerInfo(i);
	}
}


///////////////////////////////////////////////////////////////////////
int CM_PIPELINE::ReadInputData(char * InputFile)
{
	// Read input file
	FILE * fh = fopen(InputFile, "rb");
	if (fh == NULL) {
		_FUNC_TRACE
		printf("Fail to open input file %s.\n", InputFile);
		return -1;
	}

	int InputWidth = cmLayer[0].Param.SrcWidth;
	int InputHeight = cmLayer[0].Param.SrcHeight;
	int InputChannels = cmLayer[0].Param.SrcDepth;
	int InputBatchSize = cmLayer[0].Param.BatchSize;

	cmLayer[0].pWeights = (float *) malloc(cmLayer[0].size_inputSurf);

	int SampleSize = InputWidth * InputHeight * InputChannels;
	int rtn = 0;

	// Repeat reading for multi batch input.  The current input file has 1 sample for single batch.
	for (int b = 0; b < InputBatchSize; b++) {
		for (int i = 0; i < SampleSize; i++) {
			// Read input data to pWeights. It will be converted to tiled format to save in InputBuffer.
			rtn = fscanf(fh, "%f\n", &cmLayer[0].pWeights[SampleSize * b + i]);
			
			if (rtn != 1)
				printf("Error reading input file\n");
		}
		fseek(fh, 0L, SEEK_SET);
	}

	fclose(fh);
	return 0;
}

///////////////////////////////////////////////////////////////////
int CM_PIPELINE::Enqueue_KernelArray()
{
	for (int i = 0; i < NumLayers; i++) {
		if (cmLayer[i].IsActiveLayer())
			cmLayer[i].Enqueue_A_Kernel();
	}
	return 0;
}

