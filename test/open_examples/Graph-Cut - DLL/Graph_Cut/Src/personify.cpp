#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "common_GPU_host.h"
#include "Grap_Cut_pipeline_host.h"

using namespace cv;

#define VERTICAL_PUSH       0
#define HORIZONTAL_PUSH     1

#define NUM_FILES   6

double mygetTickFrequency( void )
{
    LARGE_INTEGER freq;
    if( QueryPerformanceFrequency( &freq ) ) return (double)(freq.QuadPart / 1000.);
    return -1.;
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[])
{
int i;
int result;

CM_Context C1;
char LineBuffer[256];

char iFilename[NUM_FILES][64] = { 0 }; 
char oFilename[NUM_FILES][64] = { 0 }; 

FILE * iFileHandle[NUM_FILES] = { 0 };
FILE * oFileHandle[NUM_FILES] = { 0 };

float * iOrigBuffer; 
short * iBuffer[NUM_FILES] = { 0 }; 

unsigned char * oBuffer[NUM_FILES] = { 0 }; 

int FrameWidth;
int FrameHeight;
int MaskWidth;
int MaskHeight;

float fScalingFactor = 0.5f;

Mat image, output_image;
Mat guide, mask;


	for (i = 1; i < argc; i++) {
		strcpy(LineBuffer, argv[i]);

		if (LineBuffer[0] != '-') {
			printf("%s is an invalid parameter\n", LineBuffer);
			exit(0);
		} else {
			if (LineBuffer[1] == 'i' && LineBuffer[2] == ':')	// Alpha file
				strcpy(iFilename[0], &LineBuffer[3]);
			else if (LineBuffer[1] == 'j' && LineBuffer[2] == ':')	// Guide file
				strcpy(iFilename[1], &LineBuffer[3]);
			else if (LineBuffer[1] == 'k' && LineBuffer[2] == ':')	// Mask file
				strcpy(iFilename[2], &LineBuffer[3]);
			else if (LineBuffer[1] == 'o' && LineBuffer[2] == ':')	// output file
				strcpy(oFilename[0], &LineBuffer[3]);

			else if (LineBuffer[1] == 'w' && LineBuffer[2] == ':')	// Frame width
				sscanf(&LineBuffer[3], "%d", &FrameWidth);
			else if (LineBuffer[1] == 'h' && LineBuffer[2] == ':')	// Frame height
				sscanf(&LineBuffer[3], "%d", &FrameHeight);
			else if (LineBuffer[1] == 's' && LineBuffer[2] == ':')	// Scaling factor
				sscanf(&LineBuffer[3], "%f", &fScalingFactor);
			else {
				printf("%s is an invalid parameter\n", LineBuffer);
				exit(0);
			}
		}
	}


	// Set frame facter to get complete frame size based on the video format given in filename
	float InputFrameFactor = 1.0;
    float OutputFrameFactor = 1.0;

    if ( strstr(iFilename[0], ".Y8") ) {
		InputFrameFactor = 1.0;
	} else if (strstr(iFilename[0], ".F32")) {
		InputFrameFactor = 4.0;
	}


	// Open input file 0
   	if ( strstr(iFilename[0], ".Y8") || strstr(iFilename[0], ".F32") ) {
    	iFileHandle[0] = fopen(iFilename[0], "rb");
	    if (iFileHandle[0] == NULL) {
            printf("Error opening %s\n", iFilename[0]);
            exit(1);
        }
    } else {
        // Read image file with opencv
        image = imread(iFilename[0], 0);   // Read gray scale image
        if (! image.data )                              // Check for invalid input
        {
            printf("Could not open or find the image\n");
            return -1;
        }
        Size s1 = image.size();
        int t1 = image.total();
        int e1 = image.elemSize();

//        namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
//        imshow( "Display window", image );                   // Show our image inside it.
        FrameWidth = image.cols;
        FrameHeight = image.rows;
        
        guide = imread(iFilename[1], 0);
        
        mask = imread(iFilename[2], 0);
        MaskWidth = mask.cols;
        MaskHeight = mask.rows;

        //waitKey(0);     // Wait for a keystroke in the window
    }

	// Allocate original input buffer
	int OrigInputFrameSize = (int) (FrameWidth * FrameHeight * InputFrameFactor);    // float
	iOrigBuffer = (float *) malloc(OrigInputFrameSize);			
	if (!iOrigBuffer) {
        printf("Failed allocate iOrigBuffer\n");
        exit(1);
    }


	// Allocate input buffers
	int InputFrameSize = (int) (FrameWidth * FrameHeight * 2);      // short
    for (int i = 0; i < 3; i++) {
	    iBuffer[i] = (short *) malloc(InputFrameSize);			
	    if (!iBuffer[i]) {
            printf("Failed allocate InputBuffer\n");
            exit(1);
        }
    }

	// Read input data
   	if ( strstr(iFilename[0], ".Y8") ) {
        if (fread(iBuffer[0], sizeof( char ), InputFrameSize, iFileHandle[0]) != InputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }
    } else if ( strstr(iFilename[0], ".F32") ) {
        if (fread(iOrigBuffer, sizeof( char ), OrigInputFrameSize, iFileHandle[0]) != OrigInputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }

        for (int i = 0; i < FrameWidth * FrameHeight; i++)
            *(iBuffer[0] + i) = (short) (*(iOrigBuffer + i) + 0.5f);     // float to short

    } else {
        memcpy(iBuffer[0], image.data, image.cols * image.rows);   // Gray scale
    }

   	if ( strstr(iFilename[1], ".Y8") ) {

    	iFileHandle[1] = fopen(iFilename[1], "rb");
	    if (iFileHandle[1] == NULL) {
            printf("Error opening %s\n", iFilename[1]);
            exit(1);
        }
        if (fread(iBuffer[1], sizeof( char ), InputFrameSize, iFileHandle[1]) != InputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }
    } else if ( strstr(iFilename[1], ".F32") ) {

    	iFileHandle[1] = fopen(iFilename[1], "rb");
	    if (iFileHandle[1] == NULL) {
            printf("Error opening %s\n", iFilename[1]);
            exit(1);
        }
        if (fread(iOrigBuffer, sizeof( char ), OrigInputFrameSize, iFileHandle[1]) != OrigInputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }

        for (int i = 0; i < FrameWidth * FrameHeight; i++)
            *(iBuffer[1] + i) = (short) (*(iOrigBuffer + i) + 0.5f);     // float to short

    } else {
        memcpy(iBuffer[1], guide.data, guide.cols * guide.rows);
    }

   	if ( strstr(iFilename[2], ".Y8") ) {

    	iFileHandle[2] = fopen(iFilename[2], "rb");
	    if (iFileHandle[2] == NULL) {
            printf("Error opening %s\n", iFilename[2]);
            exit(1);
        }
        if (fread(iBuffer[2], sizeof( char ), InputFrameSize, iFileHandle[2]) != InputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }

    } else if ( strstr(iFilename[2], ".F32") ) {

    	iFileHandle[2] = fopen(iFilename[2], "rb");
	    if (iFileHandle[2] == NULL) {
            printf("Error opening %s\n", iFilename[2]);
            exit(1);
        }
        if (fread(iOrigBuffer, sizeof( char ), OrigInputFrameSize, iFileHandle[2]) != OrigInputFrameSize) {
            printf("Unable to read a frame of data.");
            return -1;
        }

        for (int i = 0; i < FrameWidth * FrameHeight; i++)
            *(iBuffer[2] + i) = (short) (*(iOrigBuffer + i) + 0.5f);     // float to short

    } else {
        memcpy(iBuffer[2], mask.data, mask.cols * mask.rows);   // Gray scale
    }

    // Close input files
    for (int i = 0; i < NUM_FILES; i++) {
        if (iFileHandle[i] != 0) {
            fclose(iFileHandle[i]);
            iFileHandle[i] = 0;
        }
    }


    // Allocate debug buffer
	int DebugBufferSize = (int) (FrameWidth * FrameHeight * 2);   // short
	short * pDebug = (short *) malloc(DebugBufferSize);		
	if (!pDebug) {
        printf("Failed allocate pDebug\n");
        exit(1);
    }
    memset(pDebug, 0, DebugBufferSize);

	// Allocate output buffers
	int OutputFrameSize = (int) (FrameWidth * FrameHeight * OutputFrameFactor);   // A8
	oBuffer[1] = (unsigned char *) malloc(OutputFrameSize);		
	if (!oBuffer[1]) {
        printf("Failed allocate OutputBuffer\n");
        exit(1);
    }
    
	int OutputFrameSize2 = (int) (FrameWidth * FrameHeight * 2);   // short     
	oBuffer[2] = (unsigned char *) malloc(OutputFrameSize2);		
	if (!oBuffer[2]) {
        printf("Failed allocate OutputBuffer2\n");
        exit(1);
    }

	int OutputFrameSize3 = (int) (FrameWidth * FrameHeight * OutputFrameFactor);    // A8
    oBuffer[3] = (unsigned char *) malloc(OutputFrameSize3);		
	if (!oBuffer[3]) {
        printf("Failed allocate OutputBuffer3\n");
        exit(1);
    }

	int OutputFrameSize4 = (int) (4 * FrameWidth * FrameHeight * OutputFrameFactor);    // R8G8B8A8, RGBX32
    oBuffer[4] = (unsigned char *) malloc(OutputFrameSize4);		
	if (!oBuffer[4]) {
        printf("Failed allocate OutputBuffer4\n");
        exit(1);
    }

	int OutputFrameSize5 = (int) (1.5 * FrameWidth * FrameHeight * OutputFrameFactor);    // NV12
    oBuffer[5] = (unsigned char *) malloc(OutputFrameSize5);		
	if (!oBuffer[5]) {
        printf("Failed allocate OutputBuffer5\n");
        exit(1);
    }


	// Open output files
    oFileHandle[0] = fopen(oFilename[0], "wb");
    if (oFileHandle[0] == NULL) {
        printf("Error opening %s\n", oFilename[0]);
        exit(1);
    }
    for (int i = 1; i < NUM_FILES; i++) {
        sprintf(oFilename[i], "_%d_%s", i, oFilename[0]);
        oFileHandle[i] = fopen(oFilename[i], "wb");
	    if (oFileHandle[i] == NULL) {
            printf("Error opening %s\n", oFilename[i]);
            exit(1);
        }
    }

    ////////////////////////////////////////////////////////////////////
    // C model

    // Graph Cut
    // Lazy code.  malloc success is not check 
    int BufferSize = sizeof(float) * FrameHeight * FrameWidth;
    short * pExcessFlow = (short *) malloc( BufferSize ); 
    short * pWestCap = (short *) malloc( BufferSize ); 
    short * pNorthCap = (short *) malloc( BufferSize ); 
    short * pEastCap = (short *) malloc( BufferSize ); 
    short * pSouthCap = (short *) malloc( BufferSize ); 
    unsigned short * pHeight = (unsigned short *) malloc( BufferSize ); 
 
    // Create weight, Hcap and Vcap from image input image, source and sink
    // GenerateInputData(iBuffer[0], FrameHeight, FrameWidth, bSource, bSink, pWeight, pHCap, pVCap);

    __int64 time_a = __rdtsc();

//*
    // Graph cut (push-relabel) C model entry
    PushRelabel_Init(iBuffer[0], iBuffer[1], iBuffer[2], FrameHeight, FrameWidth, pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap);
    CModel_Push_Relabel(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, oBuffer[1], FrameHeight, FrameWidth);

    __int64 time_b = __rdtsc();
	printf( "C model time = %g ms\n ", (time_b - time_a)/((double) mygetTickFrequency()*1000.));

    printf("C model output: %s\n", oFilename[1]);
    if (fwrite(oBuffer[1], sizeof( char ), OutputFrameSize, oFileHandle[1]) != OutputFrameSize) {
        printf("Unable to write a frame of data.");
        return -1;
    }
//*/

/*
    CModel_FilterJBF( iBuffer[0],                   // Alpha image
                iBuffer[1],         // Guide image
                iBuffer[2],         // mask size is hlaf of alpha size.
                oBuffer[1],         // Output
                FrameHeight, 
                FrameWidth,
                MaskHeight,
                MaskWidth,
                15 );               // 15x15

    result = pOutputSurf->WriteSurface( oBuffer[1], NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
//*/
    //CModel_RGBtoNV12(Buffer[4], FrameWidth, FrameHeight, Buffer[5]);
    

	////////////////////////////// GPU code //////////////////////////////

     __int64 time0 = __rdtsc();

    InitGPU(C1);
    
    // GPU Query
    size_t CapValueSize = 4;
    unsigned int GpuPlatform, GtPlatform, GpuCurFreq;
    C1.pCmDev->GetCaps(CAP_GPU_PLATFORM, CapValueSize, &GpuPlatform);
    C1.pCmDev->GetCaps(CAP_GT_PLATFORM, CapValueSize, &GtPlatform);
    C1.pCmDev->GetCaps(CAP_GPU_CURRENT_FREQUENCY, CapValueSize, &GpuCurFreq);

    printf("\n");
    if (GpuPlatform == PLATFORM_INTEL_SNB)
        printf("SNB ");
    else if (GpuPlatform == PLATFORM_INTEL_IVB)
        printf("IVB ");
    else if (GpuPlatform == PLATFORM_INTEL_HSW)
        printf("HSW ");
    else if (GpuPlatform == PLATFORM_INTEL_BDW)
        printf("BDW ");

    if (GtPlatform == PLATFORM_INTEL_GT1)
        printf("GT1\n");
    else if (GtPlatform == PLATFORM_INTEL_GT2)
        printf("GT2\n");
    else if (GtPlatform == PLATFORM_INTEL_GT3)
        printf("GT3\n");
    else if (GtPlatform == PLATFORM_INTEL_GT4)
        printf("GT4\n");

    printf("GPU frequency = %d MHz\n", GpuCurFreq);

    result = C1.pCmDev->InitPrintBuffer();

    // Allocate input surface
	CM_SURFACE_FORMAT InputD3DFMT = CM_SURFACE_FORMAT_A8;
    CmSurface2D * pInputSurf = NULL;
    SurfaceIndex * pInputIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pInputSurf, &pInputIndex);

    // Allocate input surface2
    CmSurface2D * pInputSurf2 = NULL;
    SurfaceIndex * pInputIndex2 = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pInputSurf2, &pInputIndex2);

    // Allocate input surface3
    CmSurface2D * pInputSurf3 = NULL;
    SurfaceIndex * pInputIndex3 = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pInputSurf3, &pInputIndex3);


    // Input suirfaces for Push-Relabel
    InputD3DFMT = CM_SURFACE_FORMAT_V8U8; //CM_SURFACE_FORMAT_A8R8G8B8;
    CmSurface2D * pHeightSurf = NULL;
    SurfaceIndex * pHeightIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pHeightSurf, &pHeightIndex);

    InputD3DFMT = CM_SURFACE_FORMAT_V8U8;
    CmSurface2D * pExcessFlowSurf = NULL;
    SurfaceIndex * pExcessFlowIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pExcessFlowSurf, &pExcessFlowIndex);

    CmSurface2D * pWestCapSurf = NULL;
    SurfaceIndex * pWestCapIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pWestCapSurf, &pWestCapIndex);

    CmSurface2D * pNorthCapSurf = NULL;
    SurfaceIndex * pNorthCapIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pNorthCapSurf, &pNorthCapIndex);

    CmSurface2D * pEastCapSurf = NULL;
    SurfaceIndex * pEastCapIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pEastCapSurf, &pEastCapIndex);

    CmSurface2D * pSouthCapSurf = NULL;
    SurfaceIndex * pSouthCapIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, InputD3DFMT, &pSouthCapSurf, &pSouthCapIndex);
    
    // Allocate debug surface
    //CmSurface2D * pDebugSurf = NULL;
    //SurfaceIndex * pDebugIndex = NULL;
    //AllocGPUSurface(C1, FrameWidth, FrameHeight, D3DFMT_V8U8, &pDebugSurf, &pDebugIndex);

    // Allocate status surface
    CmSurface2D * pStatusSurf = NULL;
    SurfaceIndex * pStatusIndex = NULL;
    AllocGPUSurface(C1, 8, 1, CM_SURFACE_FORMAT_A8R8G8B8, &pStatusSurf, &pStatusIndex);

    // Allocate output surface
	CM_SURFACE_FORMAT OutputD3DFMT = CM_SURFACE_FORMAT_A8;
    CmSurface2D * pOutputSurf = NULL;
    SurfaceIndex * pOutputIndex = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, OutputD3DFMT, &pOutputSurf, &pOutputIndex);

    // Allocate output surface2
    CmSurface2D * pOutputSurf2 = NULL;
    SurfaceIndex * pOutputIndex2 = NULL;
    AllocGPUSurface(C1, (int) (fScalingFactor*FrameWidth), (int) (fScalingFactor*FrameHeight), OutputD3DFMT, &pOutputSurf2, &pOutputIndex2);

    // Allocate output surface3
    CmSurface2D * pOutputSurf3 = NULL;
    SurfaceIndex * pOutputIndex3 = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, OutputD3DFMT, &pOutputSurf3, &pOutputIndex3);

    // Allocate output surface4, RGBA32
    CmSurface2D * pOutputSurf4 = NULL;
    SurfaceIndex * pOutputIndex4 = NULL;
    AllocGPUSurface(C1, 4 * FrameWidth, FrameHeight, OutputD3DFMT, &pOutputSurf4, &pOutputIndex4);

    // Allocate output surface5
    OutputD3DFMT = (CM_SURFACE_FORMAT) MAKEFOURCC('N','V','1','2');
    CmSurface2D * pOutputSurf5 = NULL;
    SurfaceIndex * pOutputIndex5 = NULL;
    AllocGPUSurface(C1, FrameWidth, FrameHeight, OutputD3DFMT, &pOutputSurf5, &pOutputIndex5);


	// Init input surface with input images
    result = pInputSurf->WriteSurface( (unsigned char *) iBuffer[0], NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pInputSurf2->WriteSurface( (unsigned char *) iBuffer[1], NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pInputSurf3->WriteSurface( (unsigned char *) iBuffer[2], NULL ); // mask
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }

    //result = pDebugSurf->WriteSurface( (unsigned char *) pDebug, NULL ); // mask
    //if (result != CM_SUCCESS ) {
    //    perror("CM WriteSurface error");
    //    return -1;
    //}

    // Graph cut inputs
    PushRelabel_Init(iBuffer[0], iBuffer[1], iBuffer[2], FrameHeight, FrameWidth, pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap);

    result = pExcessFlowSurf->WriteSurface( (unsigned char *) pExcessFlow, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pHeightSurf->WriteSurface( (unsigned char *) pHeight, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pWestCapSurf->WriteSurface( (unsigned char *) pWestCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pNorthCapSurf->WriteSurface( (unsigned char *) pNorthCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pEastCapSurf->WriteSurface( (unsigned char *) pEastCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }
    result = pSouthCapSurf->WriteSurface( (unsigned char *) pSouthCap, NULL );
    if (result != CM_SUCCESS ) {
        perror("CM WriteSurface error");
        return -1;
    }


    // Get surface index and sampler index from input surface 
   	SamplerIndex * pSamplerIndex = NULL;
    AllocatGPUSampler(C1, pInputSurf, &pInputIndex, &pSamplerIndex);

    // Sampler index for mask surface
   	SamplerIndex * pMaskSamplerIndex = NULL;
    AllocatGPUSampler(C1, pInputSurf3, &pInputIndex3, &pMaskSamplerIndex);

    ////////////////////////////////////////////////////////////////////////////

	__int64 time1 = __rdtsc();
    int TotalLoops = 50;

    for (int LoopC = 0; LoopC < TotalLoops; LoopC++) {

        //***** GPU enqueue list *****
        // Sobel
    //    EnqueueSobel(C1, FrameWidth, FrameHeight, pInputIndex, pOutputIndex );

        // Graph Cut (Push-Relabel)
        int iter = 1;
        int bActiveNode;
        unsigned short HEIGHT_MAX = min( FrameWidth * FrameHeight, USHRT_MAX);
        
        do {
            if (iter % 64) {
                // Relabel
                EnqueueGraphCut_Relabel(C1, pExcessFlowIndex, pHeightIndex, pWestCapIndex, pNorthCapIndex, pEastCapIndex, pSouthCapIndex, FrameWidth, FrameHeight);
            } else {   
                // Global relabel 
                pHeightSurf->ReadSurface( (unsigned char *) pHeight, C1.pEvent );

                // Init Height surface
                for (int i = 0; i < FrameWidth * FrameHeight; i++)
                    if (*(pHeight + i) != 0)
                        *(pHeight + i) = HEIGHT_MAX;
                    result = pHeightSurf->WriteSurface( (unsigned char *) pHeight, NULL );
                    if (result != CM_SUCCESS ) {
                        perror("CM WriteSurface error");
                        return -1;
                    }
                EnqueueGraphCut_Global_Relabel(C1, pHeightIndex, pWestCapIndex, pNorthCapIndex, pEastCapIndex, pSouthCapIndex, pStatusSurf, pStatusIndex, FrameWidth, FrameHeight);
            }

            // Vertical push
            EnqueueGraphCut_Push(C1, pExcessFlowIndex, pHeightIndex, pNorthCapIndex, pSouthCapIndex, pStatusSurf, pStatusIndex, FrameWidth, FrameHeight, VERTICAL_PUSH);
            // Horizontal push
            bActiveNode = EnqueueGraphCut_Push(C1, pExcessFlowIndex, pHeightIndex, pWestCapIndex, pEastCapIndex, pStatusSurf, pStatusIndex, FrameWidth, FrameHeight, HORIZONTAL_PUSH);

            iter++;

        } while (bActiveNode);

        // Scale
    //  EnqueueScale(C1, FrameWidth, FrameHeight, pSamplerIndex, pInputIndex, pOutputIndex2, fScalingFactor);
        
        // JBL filter
    //    EnqueueJBL(C1, pInputIndex, pInputIndex2, pInputIndex3, pMaskSamplerIndex, pOutputIndex, FrameWidth, FrameHeight, MaskWidth, MaskHeight, 15);

        // Median filter on binary image
    //    EnqueueBinaryMedian(C1, pOutputIndex, pOutputIndex3, FrameWidth, FrameHeight, 9);
        
        // Merge RGBX image and alpha mask, output RGBA
    //    EnqueueMergeRGBA(C1, FrameWidth, FrameHeight, pOutputIndex4, pOutputIndex3, pOutputIndex4);

        // RGBA to NV12
    //    EnqueueRGBToNV12(C1, FrameWidth, FrameHeight, pOutputIndex4, pOutputIndex5);

    }

	DWORD dwTimeOutMs = -1;
	result = C1.pEvent->WaitForTaskFinished(dwTimeOutMs);
    if (result != CM_SUCCESS ) {
        printf("CM WaitForTaskFinished error: %d.\n", result);
        return -1;
    }

	__int64 time18 = __rdtsc();
	printf( "Average %d times = %g ms\n ", TotalLoops, (time18 - time1)/((double) mygetTickFrequency()*1000.) / TotalLoops);


    // GPU graphics cut output
    result = pHeightSurf->ReadSurface( (unsigned char *) pHeight, C1.pEvent );
    if (result != CM_SUCCESS ) {
        perror("CM ReadSurface error");
        return -1;
    }

    // Convert output short to black white byte image
    for (int j = 0; j < OutputFrameSize3; j++) 
        *(oBuffer[3]+j) = *(pHeight+j) < OutputFrameSize3 ? 0 : 255;
    
    printf("GPU output: %s\n", oFilename[3]);
    if (fwrite(oBuffer[3], sizeof( char ), OutputFrameSize3, oFileHandle[3]) != OutputFrameSize3) {
        printf("Unable to write a frame of data.");
        return -1;
    }

	result = C1.pCmDev->FlushPrintBuffer();

    result = ::DestroyCmDevice( C1.pCmDev );  
	free(C1.pCommonISACode);

    for (int i = 0; i < NUM_FILES; i++) {
        if (iBuffer[i] != 0) {
            free(iBuffer[i]);
            iBuffer[i] = 0;
        }
        if (oFileHandle[i] != 0) {
	        fclose(oFileHandle[i]);
            oFileHandle[i] = 0;
        }
        if (oBuffer[i] != 0) {
            free(oBuffer[i]);
            oBuffer[i] = 0;
        }

    }

    if (pExcessFlow != 0) {
        free(pExcessFlow);
        pExcessFlow = 0;
    }
    if (pWestCap!= 0) {
        free(pWestCap);
        pWestCap = 0;
    }
    if (pNorthCap!= 0) {
        free(pNorthCap);
        pNorthCap = 0;
    }
    if (pEastCap!= 0) {
        free(pEastCap);
        pEastCap = 0;
    }
    if (pSouthCap!= 0) {
        free(pSouthCap);
        pSouthCap = 0;
    }
    if (pHeight != 0) {
        free(pHeight);
        pHeight = 0;
    }

    waitKey(0);     // Wait for a keystroke in the window

	return 0;
}
