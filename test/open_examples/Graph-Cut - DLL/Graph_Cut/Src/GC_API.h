#pragma once  

#ifdef GC_EXPORTS  
#define GC_API __declspec(dllexport)   
#else  
#define GC_API __declspec(dllimport)   
#endif  

// This class is exported from the GC.dll  
class _GC  
{  
public:
	static GC_API int Detect_MDF();

	static GC_API int CM_GraphCut( short * pNodes, short * pHoriWeights, short * pVertWeights, unsigned char * pOutput, int FrameWidth, int FrameHeight );  

	static GC_API int CPU_GraphCut( short * pNodes, short * pHoriWeights, short * pVertWeights, unsigned char * pOutput, int FrameWidth, int FrameHeight );  
};  
