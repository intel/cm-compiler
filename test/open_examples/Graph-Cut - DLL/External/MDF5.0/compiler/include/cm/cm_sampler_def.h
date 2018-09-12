#ifndef CM_SAMPLER_DEF_H
#define CM_SAMPLER_DEF_H

#ifndef __GNUC__
#include <Windows.h>
#endif 
#include "cm_def.h"
typedef struct _CM_AVS_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
}CM_AVS_COEFF_TABLE;

#define CM_NUM_COEFF_ROWS 17
typedef struct _CM_AVS_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    BYTE DefaultSharpLvl;
    BYTE maxDerivative4Pixels;
    BYTE maxDerivative8Pixels;
    BYTE transitionArea4Pixels;
    BYTE transitionArea8Pixels;    
    CM_AVS_COEFF_TABLE Tbl0X[CM_NUM_COEFF_ROWS];
    CM_AVS_COEFF_TABLE Tbl0Y[CM_NUM_COEFF_ROWS];
    CM_AVS_COEFF_TABLE Tbl1X[CM_NUM_COEFF_ROWS];
    CM_AVS_COEFF_TABLE Tbl1Y[CM_NUM_COEFF_ROWS];
}CM_AVS_NONPIPLINED_STATE;

typedef struct _CM_AVS_STATE_MSG{
    bool AVSTYPE; //true nearest, false adaptive        
    bool EightTapAFEnable; //HSW+
    bool BypassIEF; //ignored for BWL, moved to sampler8x8 payload.
    bool ShuffleOutputWriteback; //SKL mode only to be set when AVS msg sequence is 4x4 or 8x4
    unsigned short GainFactor;
    unsigned char GlobalNoiseEstm;
    unsigned char StrongEdgeThr;
    unsigned char WeakEdgeThr;
    unsigned char StrongEdgeWght;
    unsigned char RegularWght;
    unsigned char NonEdgeWght;
    //For Non-piplined states
    unsigned short stateID;
    CM_AVS_NONPIPLINED_STATE * AvsState;
} CM_AVS_STATE_MSG;

/*
 *  CONVOLVE STATE DATA STRUCTURES
 */

typedef struct _CM_CONVOLVE_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
    float   FilterCoeff_0_8;
    float   FilterCoeff_0_9;
    float   FilterCoeff_0_10;
    float   FilterCoeff_0_11;
    float   FilterCoeff_0_12;
    float   FilterCoeff_0_13;
    float   FilterCoeff_0_14;
    float   FilterCoeff_0_15;
    float   FilterCoeff_0_16;    
    float   FilterCoeff_0_17;
    float   FilterCoeff_0_18;
    float   FilterCoeff_0_19;
    float   FilterCoeff_0_20;
    float   FilterCoeff_0_21;
    float   FilterCoeff_0_22;
    float   FilterCoeff_0_23;
    float   FilterCoeff_0_24;
    float   FilterCoeff_0_25;
    float   FilterCoeff_0_26;
    float   FilterCoeff_0_27;
    float   FilterCoeff_0_28;
    float   FilterCoeff_0_29;
    float   FilterCoeff_0_30;
    float   FilterCoeff_0_31;
}CM_CONVOLVE_COEFF_TABLE;

#define CM_NUM_CONVOLVE_ROWS 16
#define CM_NUM_CONVOLVE_ROWS_SKL 32
typedef struct _CM_CONVOLVE_STATE_MSG{
        bool CoeffSize; //true 16-bit, false 8-bit
        BYTE SclDwnValue; //Scale down value
        BYTE Width; //Kernel Width
        BYTE Height; //Kernel Height   
        //SKL mode
        bool isVertical32Mode;
        bool isHorizontal32Mode;
    CM_CONVOLVE_COEFF_TABLE Table[CM_NUM_CONVOLVE_ROWS_SKL];
} CM_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_MISC_STATE {
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    }DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    }DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    }DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    }DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    }DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    }DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    }DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    }DW7;
} CM_MISC_STATE;

typedef struct _CM_MISC_STATE_MSG{
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    }DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    }DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    }DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    }DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    }DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    }DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    }DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    }DW7;
} CM_MISC_STATE_MSG;

typedef enum _CM_SAMPLER_STATE_TYPE_
{
    CM_SAMPLER8X8_AVS = 0,
    CM_SAMPLER8X8_CONV = 1,
    CM_SAMPLER8X8_MISC = 3,
    CM_SAMPLER8X8_CONV1DH = 4,
    CM_SAMPLER8X8_CONV1DV = 5,
    CM_SAMPLER8X8_NONE
}CM_SAMPLER_STATE_TYPE;

typedef struct _CM_SAMPLER_8X8_DESCR{
    CM_SAMPLER_STATE_TYPE stateType;
    union
    {
        CM_AVS_STATE_MSG * avs;
        CM_CONVOLVE_STATE_MSG * conv;
        CM_MISC_STATE_MSG * misc; //ERODE/DILATE/MINMAX
    };
} CM_SAMPLER_8X8_DESCR;

#endif
