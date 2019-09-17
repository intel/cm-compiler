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

#ifndef _CM_LAYER_H
#define _CM_LAYER_H

#ifdef _DEBUG
#include <opencv2/core/types_c.h>
#endif

#include "cm_rt.h"

#ifdef _DEBUG
	#define _FUNC_TRACE (std::cout << __FUNCTION__ << " : ");
#else
	#define _FUNC_TRACE 
#endif

#ifdef __LINUX__
#define CM_DNN_API  
#else
#ifdef CM_EXPORTS  
#define CM_DNN_API __declspec(dllexport)   
#else  
#define CM_DNN_API __declspec(dllimport)   
#endif  
#endif


#define INACTIVE_LAYER			0
#define ACTIVE_LAYER			1

#define MEMORY_2D				0
#define MEMORY_1D				1

#define MAX_LAYERS				100
#define MAX_LOOPS				100

#define BLK8x8CNT				4		// Same value is set in kernel code.
#define MULTIBLOCKS				4

#define TILE_ROWS				2

#define MULTI_KERNEL_ENQUEUE	0		// 1: yes.  0: no.

#define ALEXNET					0
#define	VGG16					1
#define VGG19					2
#define RESNET50				3

const char NETWORK[10][20] = {	"Alexnet",
								"VGG16",
								"VGG19",
								"Resnet50" };

/////////////////////////////////////////////////////////////////////////////////
class CM_DNN_API CM_Base
{
public:
	CM_Base();
	virtual ~CM_Base();

    UINT Version;           // CM version

    CmDevice * pCmDev;
    FILE * pISA;
    void * pCommonISACode;

    CmProgram * pCmProgram;
    CmQueue * pCmQueue;

    unsigned int GpuPlatform; 
    unsigned int GtPlatform; 
	unsigned int GpuMinFreq;
	unsigned int GpuMaxFreq;
    unsigned int GpuCurFreq;
	unsigned int HWThreadCount;
	unsigned int ThreadsPerGroup;

    int InitGPU(char * isa_file);
	static double GetTimeMS();

private:
	int DetectIntelGPU();
	int DetectCM();
};

/////////////////////////////// //////////////////////////////////////////////
class CM_DNN_API Layer_Param {
public:
	Layer_Param();
	~Layer_Param();

	int IsInputlayer;
	int IsWithAvg;
	int IsActive;

	char LayerType[32];
	int BatchSize;

	char InputFile[128];
	char WeightFile[128];
	char BiasFile[128];
	char RefFile[128];

	int SrcWidth;						// Input image parameters
	int SrcHeight;
	int SrcDepth;

	int KernelWidth;					// Convol kernel filter size, also Maxpool window size.
	int KernelHeight;
	int KernelStride;

	int WindowChannelSize;				// LRN kernel parameters
	double Alpha;
	double Beta;
	double K;

	int DestWidth;					// Output image parameter, size in pixel
	int DestHeight;
	int DestDepth;

	int InputBorderWidth;
	int OutputBorderWidth;
	int NumGroups;					// 1 or 2, for AlexNet
	int EnableReLU;

	void ReadParam(char * ParamFile);
	void PrintParam(int Layer);

private:
	void GetStringParam(char * SrcBuffer, char * DestBuffer);
	void GetIntParam(char * SrcBuffer, int & Dest);
	void GetFloatParam(char * SrcBuffer, float & Dest);
	void GetDoubleParam(char * SrcBuffer, double & Dest);
	char * FindChar(char * buffer, int sLength, char c);
};

/////////////////////////////////////////////////////////////
class CM_DNN_API CM_LAYER
{
public:
	CM_LAYER();
    ~CM_LAYER();

	CM_Base * pCm;
	Layer_Param Param;

	int NumInputs;
	CM_LAYER * pPrevLayer;		// For the 1st input
	CM_LAYER * pPrevLayer2;		// For the 2nd input

	UINT LayerID;
	int UseFP16;
	int qFCWt;					// Quantized FC weights
	
	int IsActiveLayer();
	float * GetInputBuffer();
	void PrintLayerInfo(int CurLayer);

	void Config_Input_Layer();
	void Config_Convol_Layer();
	void Config_LRN_Layer();
	void Config_MaxPool_Layer();
	void Config_AvgPool_Layer();
	void Config_FC_Layer();
	void Config_ReLU_Layer();
	void Config_SurfConv_Layer();
	void Config_SoftMax_Layer();

	int Enqueue_A_Kernel();
	int GPUSync();
	float GetLayerPerformance();
	void GetLayerOutput(float * pMWOutput);
	void CompareOutput(int netIdx);

	float * getOutputBuffer() { return OutputBuffer; }

	// Input and output buffers in system memory
	float * InputBuffer;				// Input data
	float * InputBuffer2;				// cnnAvg
	int InputBufferIsMalloced;
	float * OutputBuffer;				// Output data
	int OutputBufferIsMalloced;

	float * pWeights;
	unsigned int WeightSize;				// One kernel weight size
	unsigned int TotalWeightSize;

	float * pBias;
	unsigned int BiasSize;				// One bias size
	unsigned int TotalBiasSize;

	float * pWeightsBias;
	unsigned int WeightBiasSize;
	unsigned int TotalWeightBiasSize;

	UINT pitch_inputSurf, size_inputSurf;
	UINT pitch_outputSurf, size_outputSurf;

	int InputMemType;							// 1D buffer or 2D surface
	int OutputMemType;

	int CopyFromGPUSurface();
	int CopyFromGPUBuffer();

private:

	// Input tile 
	unsigned int iFrameWidth, iFrameHeight, iDepth; 
	unsigned int iTileHoriCount, iTileVertCount;
	unsigned int iTileWidth, iTileHeight;
	unsigned int iBlockWidth, iBlockHeight;

	// Output tile
	unsigned int oFrameWidth, oFrameHeight, oDepth; 
	unsigned int oTileHoriCount, oTileVertCount;
	unsigned int oTileWidth, oTileHeight;
	unsigned int oBlockWidth, oBlockHeight;

	// Thread space
	unsigned int ThreadsWidth, ThreadsHeight; 
	unsigned int ThreadsWidthPerTile, ThreadsHeightPerTile;


	// Input CM surface/buffer
	CmSurface2D * pInputSurf;
	CmBuffer * pInputBuf;
	SurfaceIndex * pInputIndex;

	CmSurface2D * pInputSurf2;					// 2nd input
	CmBuffer * pInputBuf2;
	SurfaceIndex * pInputIndex2;

	// Output CM surface/buffer
	CmSurface2D * pOutputSurf;
	CmBuffer * pOutputBuf;
	SurfaceIndex * pOutputIndex;

	// CM UP surface/buffer
	CmSurface2DUP *  pInputSurfUP;
	CmBufferUP * pInputBufUP;
	CmSurface2DUP *  pOutputSurfUP;
	CmBufferUP * pOutputBufUP;

	// Weight and bias 
	CmBuffer * pWeightBuf;
	CmSurface2D * pWeightSurf;
	CmBuffer	 *pWeightSurf_1D;			// Weight surface (UP buffer)
	SurfaceIndex * pWeightIndex;

	CmBuffer * pBiasBuf;
	SurfaceIndex * pBiasIndex;

	CmBuffer * pWeightBiasBuf;
	SurfaceIndex * pWeightBiasIndex;

	CmKernel * pCmKernel;
	CmThreadSpace * pThrdSpace;
	CmThreadGroupSpace * pTGS;
    CmTask * pKernelArray;

	CmEvent * pEvent;

	float fWMin, fWMax;			// Minimum/maximum FC weights
	float fKernelTimeMS;		// Kenrel execution time in ms

	void CModel_Convolution();
	void CModel_Convol(int SrcIdx, int DstIdx, int GroupID);

	void FindImageTile(int ImageCnt, int BatchSize, unsigned int & TileHoriCount, unsigned int & TileVertCount);

	void Define_Convol_iBlockSize();

	int CreateOutputSurface();
	int CreateInputSurface();
	int CreateOutputBuffer();
	int CreateInputBuffer();

	int ReadInputImage(const char * InputFile);
	int ReadImage2D();
	int ReadImage1D();
	int ReadWeightsBias(const char * MW_mangled_weights_file, const char * MW_mangled_bias_file);

	void ConvertInputToTiledImage(float * pSrc, float * pDst);
	void ConvertInputToLinearImage(float * pSrc, float * pDst);

	void Convol1x1_Rearrange_Weight(void * pWeights, int ChunkSize, int *nWeightSize, void ** pTemp);

	template <typename WT>		// Transpose the weight matrix
	int FC_Transpose_Weight(
		float* pInWeights,
		int mWidth,
		int mHeight,
		WT* pOutWeights
		);

	template <typename WT>		// Rearrange FC weight data for 1D buffer access
	int FC_Rearrange_Weight(
		WT* pInWeight,			// Input weight data
		int nBlockWidth,		// Features output from FC kernel
		int nBlockHeight,		// Height of the block FC kernel loops through
		int *nWeightSize,		// Size of input feature
		WT** pOutWeight			// Remapped weight data aligned to 4KB memory location
		);

	// GPU functions
	void SetupGPUInputSurface();
	void SetupGPUInputBuffer();

	void SetupGPUOutputSurface();
	void SetupGPUOutputBuffer();

	void SetupGPUWeightBiasBuffer();
	void SetupGPUWeightBuffer();
	void SetupGPUBiasBuffer();
	void SetupGPUWeightSurface();

	int CreateKernel_InputProc();
	int CreateKernel_Convol_IPT(unsigned int KernelWidth);
	int CreateKernel_Convol_IPT_HF(unsigned int KernelWidth);
	int CreateKernel_Convol_BPT(unsigned int KernelWidth);
	int CreateKernel_Convol_BPT_HF(unsigned int KernelWidth);
	int CreateKernel_ReLU();
	int CreateKernel_LRN();
	int CreateKernel_MaxPool();
	int CreateKernel_AvgPool();
	int CreateKernel_FC();
	int CreateKernel_FC_NPatch();
	int CreateKernel_SurfConv();
	int CreateKernel_SoftMax();

	int SaveImage(char RefFile[]);
	int ReadRefFile(char RefFile[], float * pRef, int RefSize);
	unsigned int CompareRefernece(float * OutputBuffer, float * pRef);
	void ReadOneTiledImage(float * OutputBuffer, int ImgIdx, float * pMyImage, int BorderWidth);
	void ReadOneLinearImage(float * OutputBuffer, int ImgIdx, float * pMyImage, int BorderWidth);

	// Surface allocation wrappers.  Moved over from CM_Base 
    int AllocGPUSurface(int iFrameWidth, int iFrameHeight, CM_SURFACE_FORMAT format, CmSurface2D ** pSurf, SurfaceIndex ** pIndex);
    int AllocGPUSurfaceUP(int iFrameWidth, int iFrameHeight, CM_SURFACE_FORMAT format, unsigned char * pBuffer, CmSurface2DUP ** pSurf, SurfaceIndex ** pIndex);
    int AllocGPUBuffer(unsigned int size, CmBuffer ** pBuffer, SurfaceIndex ** pIndex);
    int AllocGPUBufferUP(unsigned int size, unsigned char * pSysMem, CmBufferUP ** pBuffer, SurfaceIndex ** pIndex);

    void * AlignedMalloc(unsigned int size, int alignment);
    void AlignedFree(void * pBuffer);

#ifdef WIN32
#ifdef CM_DX9
    int ImportDX9Surface(IDirect3DSurface9 * pD3DSurf, CmSurface2D ** pSurf, SurfaceIndex ** pIndex);
#endif
#ifdef CM_DX11
    int ImportDX11Surface(ID3D11Texture2D * pD3DSurf, CmSurface2D ** pSurf, SurfaceIndex ** pIndex);
#endif
#endif
    int AllocatGPUSampler(CmSurface2D * pInputSurf, SurfaceIndex ** pSurfIndex, SamplerIndex ** pSamplerIndex ); 
    int AllocatGPUSamplerUP(CmSurface2DUP * pInputSurfUP, SurfaceIndex ** pSurfIndex, SamplerIndex ** pSamplerIndex );
};


///////////////////////////////////////////////
class CM_DNN_API CM_PIPELINE
{
public:
	CM_PIPELINE();
    ~CM_PIPELINE();

	CM_Base cmBase;
	CM_LAYER cmLayer[MAX_LAYERS];
//	CmEvent * pPipeEvent[MAX_LOOPS];

	float * InputBuffer;

	UINT NumLayers;

	void UpdateLayerID();

	void CreateInputLayer(char * ParamFile, char * InputFile, char * AvgCnnFile, int BatchSize, int UseFP16, int IsActive);
	void CreateConvolLayer(char * ParamFile, char * weight_file, char * bias_file, int BatchSize, int UseFP16, int NumOfInputs,
		CM_LAYER * pPrevLayerA, CM_LAYER * pPrevLayerB, int IsActive);
	void CreateLRNLayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);
	void CreateMaxPoolLayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);
	void CreateAvgPoolLayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);
	void CreateFCLayer(char * ParamFile, char * weight_file, char * bias_file, int BatchSize, int UseFP16, int NumOfInputs, 
		CM_LAYER * pPrevLayer, int IsActive);
	void CreateReLULayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);
	void CreateSurfConvLayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);
	void CreateSoftMaxLayer(char * ParamFile, int BatchSize, int UseFP16, int NumOfInputs, CM_LAYER * pPrevLayer, int IsActive);

	void Build_AlexNet(char * InputFile, int BatchSize, int UseFP16);
	void Build_VGG16(char * InputFile, int BatchSize, int UseFP16);
	void Build_VGG19(char * InputFile, int BatchSize, int UseFP16);
	void Build_Resnet50(char * InputFile, int BatchSize, int UseFP16);

//	void SetFP16(CM_LAYER * Layers[], int NumLayers, int UseFP16);
//	void SetqFCWt(CM_LAYER * Layers[], int NumLayers, int qFCWt);
	
	void UpdateBorderWidth();
	void AllocateCMLayers();
	int ReadInputData(char * InputFile);
	int Enqueue_KernelArray();

};


#endif
