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

/////////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::Build_AlexNet(char * InputFile, int BatchSize, int UseFP16)
{
	CreateInputLayer("../Data/Alexnet/Param/L0_Input.txt", InputFile, "../Data/Alexnet/Input/cnn_CnnMain_avg", BatchSize, UseFP16, true);
	CreateConvolLayer("../Data/Alexnet/Param/L1_Convol.txt", "../Data/Alexnet/Input/cnn_CnnMain_conv1_w", "../Data/Alexnet/Input/cnn_CnnMain_conv1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], NULL, true);
	CreateLRNLayer("../Data/Alexnet/Param/L2_LRN.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	CreateMaxPoolLayer("../Data/Alexnet/Param/L3_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);

	CreateConvolLayer("../Data/Alexnet/Param/L4_Convol.txt", "../Data/Alexnet/Input/cnn_CnnMain_conv2_w", "../Data/Alexnet/Input/cnn_CnnMain_conv2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], NULL, true);
	CreateLRNLayer("../Data/Alexnet/Param/L5_LRN.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	CreateMaxPoolLayer("../Data/Alexnet/Param/L6_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	
	CreateConvolLayer("../Data/Alexnet/Param/L7_Convol.txt", "../Data/Alexnet/Input/cnn_CnnMain_conv3_w", "../Data/Alexnet/Input/cnn_CnnMain_conv3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], NULL, true);
	CreateConvolLayer("../Data/Alexnet/Param/L8_Convol.txt", "../Data/Alexnet/Input/cnn_CnnMain_conv4_w", "../Data/Alexnet/Input/cnn_CnnMain_conv4_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], NULL, true);
	CreateConvolLayer("../Data/Alexnet/Param/L9_Convol.txt", "../Data/Alexnet/Input/cnn_CnnMain_conv5_w", "../Data/Alexnet/Input/cnn_CnnMain_conv5_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], NULL, true);
	CreateMaxPoolLayer("../Data/Alexnet/Param/L10_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	
	CreateSurfConvLayer("../Data/Alexnet/Param/L11_SurfConv.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	
	CreateFCLayer("../Data/Alexnet/Param/L12_FC.txt", "../Data/Alexnet/Input/cnn_CnnMain_fc6_w", "../Data/Alexnet/Input/cnn_CnnMain_fc6_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	CreateFCLayer("../Data/Alexnet/Param/L13_FC.txt", "../Data/Alexnet/Input/cnn_CnnMain_fc7_w", "../Data/Alexnet/Input/cnn_CnnMain_fc7_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	CreateFCLayer("../Data/Alexnet/Param/L14_FC.txt", "../Data/Alexnet/Input/cnn_CnnMain_fc8_w", "../Data/Alexnet/Input/cnn_CnnMain_fc8_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
	CreateSoftMaxLayer("../Data/Alexnet/Param/L15_Softmax.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers-1], true);
}

///////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::Build_VGG16(char * InputFile, int BatchSize, int UseFP16)
{
	CreateInputLayer("../Data/VGG16/Param/L0_Input.txt", InputFile, "../Data/VGG16/Input/cnn_CnnMain_avg", BatchSize, UseFP16, true);

	CreateConvolLayer("../Data/VGG16/Param/L1_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv1_1_w", "../Data/VGG16/Input/cnn_CnnMain_conv1_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L2_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv1_2_w", "../Data/VGG16/Input/cnn_CnnMain_conv1_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG16/Param/L3_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG16/Param/L4_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv2_1_w", "../Data/VGG16/Input/cnn_CnnMain_conv2_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L5_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv2_2_w", "../Data/VGG16/Input/cnn_CnnMain_conv2_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG16/Param/L6_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG16/Param/L7_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv3_1_w", "../Data/VGG16/Input/cnn_CnnMain_conv3_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L8_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv3_2_w", "../Data/VGG16/Input/cnn_CnnMain_conv3_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L9_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv3_3_w", "../Data/VGG16/Input/cnn_CnnMain_conv3_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG16/Param/L10_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG16/Param/L11_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv4_1_w", "../Data/VGG16/Input/cnn_CnnMain_conv4_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L12_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv4_2_w", "../Data/VGG16/Input/cnn_CnnMain_conv4_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L13_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv4_3_w", "../Data/VGG16/Input/cnn_CnnMain_conv4_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG16/Param/L14_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG16/Param/L15_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv5_1_w", "../Data/VGG16/Input/cnn_CnnMain_conv5_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L16_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv5_2_w", "../Data/VGG16/Input/cnn_CnnMain_conv5_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG16/Param/L17_Convol.txt", "../Data/VGG16/Input/cnn_CnnMain_conv5_3_w", "../Data/VGG16/Input/cnn_CnnMain_conv5_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG16/Param/L18_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateSurfConvLayer("../Data/VGG16/Param/L19_SurfConv.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateFCLayer("../Data/VGG16/Param/L20_FC.txt", "../Data/VGG16/Input/cnn_CnnMain_fc6_w", "../Data/VGG16/Input/cnn_CnnMain_fc6_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateFCLayer("../Data/VGG16/Param/L21_FC.txt", "../Data/VGG16/Input/cnn_CnnMain_fc7_w", "../Data/VGG16/Input/cnn_CnnMain_fc7_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateFCLayer("../Data/VGG16/Param/L22_FC.txt", "../Data/VGG16/Input/cnn_CnnMain_fc8_w", "../Data/VGG16/Input/cnn_CnnMain_fc8_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateSoftMaxLayer("../Data/VGG16/Param/L23_Softmax.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
}

///////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::Build_VGG19(char * InputFile, int BatchSize, int UseFP16)
{
	CreateInputLayer("../Data/VGG19/Param/L0_Input.txt", InputFile, "../Data/VGG19/Input/cnn_CnnMain_avg", BatchSize, UseFP16, true);

	CreateConvolLayer("../Data/VGG19/Param/L1_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv1_1_w", "../Data/VGG19/Input/cnn_CnnMain_conv1_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L2_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv1_2_w", "../Data/VGG19/Input/cnn_CnnMain_conv1_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG19/Param/L3_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG19/Param/L4_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv2_1_w", "../Data/VGG19/Input/cnn_CnnMain_conv2_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L5_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv2_2_w", "../Data/VGG19/Input/cnn_CnnMain_conv2_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG19/Param/L6_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG19/Param/L7_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv3_1_w", "../Data/VGG19/Input/cnn_CnnMain_conv3_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L8_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv3_2_w", "../Data/VGG19/Input/cnn_CnnMain_conv3_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L9_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv3_3_w", "../Data/VGG19/Input/cnn_CnnMain_conv3_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L10_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv3_4_w", "../Data/VGG19/Input/cnn_CnnMain_conv3_4_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG19/Param/L11_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG19/Param/L12_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv4_1_w", "../Data/VGG19/Input/cnn_CnnMain_conv4_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L13_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv4_2_w", "../Data/VGG19/Input/cnn_CnnMain_conv4_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L14_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv4_3_w", "../Data/VGG19/Input/cnn_CnnMain_conv4_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L15_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv4_4_w", "../Data/VGG19/Input/cnn_CnnMain_conv4_4_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG19/Param/L16_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateConvolLayer("../Data/VGG19/Param/L17_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv5_1_w", "../Data/VGG19/Input/cnn_CnnMain_conv5_1_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L18_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv5_2_w", "../Data/VGG19/Input/cnn_CnnMain_conv5_2_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L19_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv5_3_w", "../Data/VGG19/Input/cnn_CnnMain_conv5_3_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateConvolLayer("../Data/VGG19/Param/L20_Convol.txt", "../Data/VGG19/Input/cnn_CnnMain_conv5_4_w", "../Data/VGG19/Input/cnn_CnnMain_conv5_4_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], NULL, true);
	CreateMaxPoolLayer("../Data/VGG19/Param/L21_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateSurfConvLayer("../Data/VGG19/Param/L22_SurfConv.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);

	CreateFCLayer("../Data/VGG19/Param/L23_FC.txt", "../Data/VGG19/Input/cnn_CnnMain_fc6_w", "../Data/VGG19/Input/cnn_CnnMain_fc6_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateFCLayer("../Data/VGG19/Param/L24_FC.txt", "../Data/VGG19/Input/cnn_CnnMain_fc7_w", "../Data/VGG19/Input/cnn_CnnMain_fc7_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateFCLayer("../Data/VGG19/Param/L25_FC.txt", "../Data/VGG19/Input/cnn_CnnMain_fc8_w", "../Data/VGG19/Input/cnn_CnnMain_fc8_b", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
	CreateSoftMaxLayer("../Data/VGG19/Param/L26_Softmax.txt", BatchSize, UseFP16, 1, &cmLayer[NumLayers - 1], true);
}

//////////////////////////////////////////////////////////////////////////////
void CM_PIPELINE::Build_Resnet50(char * InputFile, int BatchSize, int UseFP16)
{
	// Layer 0-2
	CreateInputLayer("../Data/Resnet50/Param/L0_Input.txt", InputFile, "../Data/Resnet50/Input/cnn_CnnMain_avg", BatchSize, UseFP16, true);
	CreateConvolLayer("../Data/Resnet50/Param/L1_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_conv1_w", "../Data/Resnet50/Input/cnn_CnnMain_conv1_b", BatchSize, UseFP16, 1, &cmLayer[0], NULL, true);
	CreateMaxPoolLayer("../Data/Resnet50/Param/L2_MaxPool.txt", BatchSize, UseFP16, 1, &cmLayer[1], true);
	// Layer 3-6
	CreateConvolLayer("../Data/Resnet50/Param/L3_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[2], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L4_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[3], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L5_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch1_w", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch1_b", BatchSize, UseFP16, 1, &cmLayer[2], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L6_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res2a_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[4], &cmLayer[5], true);
	// Layer 7-9
	CreateConvolLayer("../Data/Resnet50/Param/L7_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[6], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L8_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[7], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L9_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res2b_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[8], &cmLayer[6], true);
	// Layer 10-12
	CreateConvolLayer("../Data/Resnet50/Param/L10_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[9], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L11_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[10], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L12_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res2c_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[11], &cmLayer[9], true);
	// Layer 13-16
	CreateConvolLayer("../Data/Resnet50/Param/L13_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[12], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L14_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[13], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L15_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch1_w", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch1_b", BatchSize, UseFP16, 1, &cmLayer[12], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L16_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res3a_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[14], &cmLayer[15], true);
	// Layer 17-19
	CreateConvolLayer("../Data/Resnet50/Param/L17_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[16], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L18_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[17], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L19_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res3b_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[18], &cmLayer[16], true);
	// Layer 20-22
	CreateConvolLayer("../Data/Resnet50/Param/L20_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[19], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L21_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[20], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L22_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res3c_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[21], &cmLayer[19], true);
	// Layer 23-25
	CreateConvolLayer("../Data/Resnet50/Param/L23_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[22], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L24_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[23], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L25_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res3d_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[24], &cmLayer[22], true);
	// Layer 26-29
	CreateConvolLayer("../Data/Resnet50/Param/L26_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[25], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L27_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[26], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L28_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch1_w", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch1_b", BatchSize, UseFP16, 1, &cmLayer[25], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L29_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4a_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[27], &cmLayer[28], true);
	// Layer 30-32
	CreateConvolLayer("../Data/Resnet50/Param/L30_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[29], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L31_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[30], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L32_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4b_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[31], &cmLayer[29], true);
	// Layer 33-35
	CreateConvolLayer("../Data/Resnet50/Param/L33_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[32], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L34_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[33], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L35_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4c_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[34], &cmLayer[32], true);
	// Layer 36-38
	CreateConvolLayer("../Data/Resnet50/Param/L36_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[35], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L37_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[36], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L38_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4d_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[37], &cmLayer[35], true);
	// Layer 39-41
	CreateConvolLayer("../Data/Resnet50/Param/L39_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[38], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L40_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[39], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L41_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4e_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[40], &cmLayer[38], true);
	// Layer 42-44
	CreateConvolLayer("../Data/Resnet50/Param/L42_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[41], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L43_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[42], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L44_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res4f_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[43], &cmLayer[41], true);
	// Layer 45-48
	CreateConvolLayer("../Data/Resnet50/Param/L45_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[44], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L46_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[45], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L47_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch1_w", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch1_b", BatchSize, UseFP16, 1, &cmLayer[44], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L48_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res5a_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[46], &cmLayer[47], true);
	// Layer 49-51
	CreateConvolLayer("../Data/Resnet50/Param/L49_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[48], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L50_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[49], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L51_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res5b_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[50], &cmLayer[48], true);
	// Layer 52-54
	CreateConvolLayer("../Data/Resnet50/Param/L52_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2a_w", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2a_b", BatchSize, UseFP16, 1, &cmLayer[51], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L53_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2b_w", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2b_b", BatchSize, UseFP16, 1, &cmLayer[52], NULL, true);
	CreateConvolLayer("../Data/Resnet50/Param/L54_Convol.txt", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2c_w", "../Data/Resnet50/Input/cnn_CnnMain_res5c_branch2c_b", BatchSize, UseFP16, 2, &cmLayer[53], &cmLayer[51], true);
	// Layer 55-57
	CreateAvgPoolLayer("../Data/Resnet50/Param/L55_AvgPool.txt", BatchSize, UseFP16, 1, &cmLayer[54], true);
	CreateFCLayer("../Data/Resnet50/Param/L56_FC.txt", "../Data/Resnet50/Input/cnn_CnnMain_fc1000_w", "../Data/Resnet50/Input/cnn_CnnMain_fc1000_b", BatchSize, UseFP16, 1, &cmLayer[55], true);
	CreateSoftMaxLayer("../Data/Resnet50/Param/L57_Softmax.txt", BatchSize, UseFP16, 1, &cmLayer[56], true);
}



