#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>

#include "common_C_model.h"
#include "Personify_host.h"

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

///////////////////////////////////////////////////////////////////////////////////////////////////
void Get2dGaussianKernel(int ksize, float kstd, float * kernel)
{
    float divider = 2 * kstd * kstd;
    float ksum = 0;
    int s2 = (ksize - 1) / 2;

    //check if ksize is even. This function only accepts odd ksize
    if (ksize % 2 == 0)
        ksize++;

    for (int i = 0, yy = -s2; i < ksize; i++, yy++)
        for (int j = 0, xx = -s2; j < ksize; j++, xx++) {
            float fTemp = -(xx * xx + yy * yy) / divider;
            fTemp = pow(2.718281828f, fTemp); 
            *(kernel + ksize*i + j) = fTemp;
            ksum += fTemp;
        }
    
    if (ksum != 0)
        for (int i = 0; i < ksize*ksize; i++)
            *(kernel+i) /= ksum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Bilateral filter
void CModel_FilterJBF( unsigned char * alpha, 
                        unsigned char * guide,
                        unsigned char * mask,
                        unsigned char * out,
                        unsigned char * ext_mask,
                        int rows, 
                        int cols,
                        int maskRows,
                        int maskCols,
                        int winSize )
{
    int s2 = (winSize - 1) / 2;
//    float mRsigma = winSize / 4.f;
//    float mSsigma = winSize / 2.f;
    float mRsigma = float(winSize * 4);
    float mSsigma = float(winSize);

    float ratio = (float) maskRows / rows;      // image to mask reduction ratio

    float * kernel = (float *) malloc( 4 * winSize * winSize);
    if (kernel == NULL) {
        printf("Failed to allocate kernel space\n");
        return;
    }

    Get2dGaussianKernel(winSize, mSsigma, kernel);

    //memset(ext_mask, 0, rows*cols);
    memcpy(out, alpha, rows*cols);

    for (int r = s2; r < rows-s2; r++)
        for (int c = s2; c < cols-s2; c++) {

            float sum = 0;
            float sum_weight = 0;

            // Check for active pixel indicated by mask
            unsigned char emask = GetPix(mask, maskCols, (int) (c * ratio), (int) (r * ratio));
            PutPix(ext_mask, cols, c, r, emask);

            if (emask) { 
                unsigned char cur_pix = GetPix(guide, cols, c, r);

                for (int j = r-s2; j <= r+s2; j++)
                    for (int i = c-s2; i <= c+s2; i++) {
                        
                        int diff = GetPix(guide, cols, i, j) - cur_pix;
                        
                        //diff = diff >= 0 ? diff : -diff;
                        diff *= diff;

                        float weight_range = pow(2.718281828f, -float(diff) / (2 * mRsigma * mRsigma));

                        float weight = weight_range * GetPix(kernel, winSize, i-(c-s2), j-(r-s2) );

                        sum += weight * GetPix(alpha, cols, i, j);
                        sum_weight += weight;
                    }

                unsigned char val = ((sum_weight > 0) ? sum / sum_weight : GetPix(alpha, cols, c, r)) > 128 ? 255 : 0;
                PutPix(out, cols, c, r, val);
            }
        }

    free(kernel);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CModel_BinaryMedian(unsigned char * in, unsigned char * out, unsigned char * ext_mask, int Rows, int Cols, int winSize, bool cMaskOn)
{
    if ((winSize & 0x0001) == 0) {
        printf("Invalid winSize %d\n", winSize);
        return;
    }

    int thrd = (int)(winSize*winSize/2)*255;
    int s2 = winSize/2;
    unsigned char val = 0;

    memcpy(out, in, Rows*Cols);

    for (int r = s2; r < Rows-s2; r++) {
        for (int c = s2; c < Cols-s2; c++) {

if (r == 82 && c == 208)
    r = r;
            if (GetPix(ext_mask, Cols, c, r)) {
                int sum = 0;
                for (int j = r-s2; j <= r+s2; j++)
                    for (int i = c-s2; i <= c+s2; i++) {
//                        sum += GetPix(in, Cols, i, j);
                        sum += *(in + j*Cols + i);
                    }
                val = (sum <= thrd) ? 0 : 255; // for 9x9, thrd = (int)(winSize*winSize/2)*255 = 40*255 = 10200.
                PutPix(out, Cols, c, r, val);  
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Graph Cut (Push and Relabel)
void CModel_BlockMask(short * pExcessFlow, HEIGHT_TYPE * pHeight, int ImgRows, int ImgCols, 
						unsigned char * pBlockMask, int BlkRows, int BlkCols, int BlkWidth, int BlkHeight)
{
	// Block based Relabel
//    int Blk_Cols = 16;
//    int Blk_Rows = 16;
//	char BlockMask[16][16];
//	short Block_ExcessFlow[16][16];
//	HEIGHT_TYPE Block_Height[16][16];

//    int nBlk_X = (int) (((float) Cols) / Blk_Cols + 0.5f);
//    int nBlk_Y = (int) (((float) Rows) / Blk_Rows + 0.5f);

	HEIGHT_TYPE HEIGHT_MAX = min(ImgRows * ImgCols, TYPE_MAX-1);

	memset(pBlockMask, 0, BlkRows*BlkCols);

	// Loop through all blocks
    for (int j = 0; j < BlkRows; j++) {
        for (int i = 0; i < BlkCols; i++) {

            // Row OOB check
            int myBlkHeight = ((j+1)*BlkHeight < ImgRows) ? BlkHeight : (ImgRows - j*BlkHeight);
			int active = 0;

			// Copy a block
            for (int y = 0; y < myBlkHeight; y++) {
				int	v = j * BlkHeight + y;
				for (int x = 0; x < BlkWidth; x++) {
					int u = i * BlkWidth + x;
					if ( GetPix(pExcessFlow, ImgCols, u, v) > -100 && GetPix(pHeight, ImgCols, u, v) < HEIGHT_MAX ) {
						PutPix(pBlockMask, BlkCols, i, j, (unsigned char) 255);  
						active = 1;
						break;
					}
				}
				if (active)
					break;
			}

        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Graph Cut (Push and Relabel)
void PushRelabel_Init(	short * pWeight, 
						short * pHCap, 
						short * pVCap, 
						int Rows, 
						int Cols, 
						short * pExcessFlow, 
						HEIGHT_TYPE * pHeight, 
						short * pWestCap, 
						short * pNorthCap, 
						short * pEastCap, 
						short * pSouthCap)
{
    short w;
    HEIGHT_TYPE HEIGHT_MAX = min( Rows * Cols, TYPE_MAX-1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {
            
            // Set excess flow
            w = GetPix(pWeight, Cols, x, y);
            PutPix(pExcessFlow, Cols, x, y, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
/*
			HEIGHT_TYPE h = (w > 0) ? w : 0;
			if (w == FG_THRESHOLD)
				h = HEIGHT_MAX;
			PutPix(pHeight, Cols, x, y, h);
*/
//            PutPix((int *) pHeight, Cols, x, y, 0);
            PutPix(pHeight, Cols, x, y, (HEIGHT_TYPE) 0);

            // Set west and east caps
            w = GetPix(pHCap, Cols, x, y);
            if (x == 0) {
                PutPix(pWestCap, Cols, x, y, (short) 0);
            } else if (x < Cols-1) {
                PutPix(pWestCap, Cols, x, y, w);
                PutPix(pEastCap, Cols, x-1, y, w);
            } else {    // x = Cols-1
                PutPix(pWestCap, Cols, x, y, w);
                PutPix(pEastCap, Cols, x-1, y, w);
                PutPix(pEastCap, Cols, x, y, (short) 0);
            }

            // Set north and south caps
            w = GetPix(pVCap, Cols, x, y);
            if (y == 0) {
                PutPix(pNorthCap, Cols, x, y, (short) 0);
            } else if (y < Rows-1) {
                PutPix(pNorthCap, Cols, x, y, w);
                PutPix(pSouthCap, Cols, x, y-1, w);
            } else {
                PutPix(pNorthCap, Cols, x, y, w);
                PutPix(pSouthCap, Cols, x, y-1, w);
                PutPix(pSouthCap, Cols, x, y, (short) 0);
            }
        }
}


///////////////////////////////// Relabel func //////////////////////////////////////////
/*
if active(x) do
    my_height= HEIGHT_MAX;  // init to max height
    for each y=neighbor(x)
        if capacity(x,y) > 0 do
            my_height= min(my_height, height(y)+1);// minimum height + 1
        done
    end
    height(x) = my_height;// update height
done
*/
// Key point: height is updated after all directions are checked and assign to the lowest neighbor height + 1.
// In kernel, 8 pixels can lock step process and upedate height.  The next row will use the updated row.
void CModel_GraphCut_Relabel(   short * pExcessFlow, 
                                HEIGHT_TYPE * pHeight,
                                short * pWestCap,
                                short * pNorthCap,
                                short * pEastCap,
                                short * pSouthCap,
								unsigned char * pBlockMask,
                                int Rows,
                                int Cols,
								int BlkRows, 
								int BlkCols, 
								int BlkWidth, 
								int BlkHeight)   
{
//#define SCANLINE_RELABEL 1
#ifdef SCANLINE_RELABEL

// Scan line based Relabel
    HEIGHT_TYPE NewHeight, temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

	bool rtn = false;

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

            // Check for active node
            float CurExcessFlow = GetPix(pExcessFlow, Cols, x, y);
            HEIGHT_TYPE CurHeight = GetPix(pHeight, Cols, x, y);
        
            if (CurExcessFlow > 0 && CurHeight < HEIGHT_MAX) {  

                NewHeight = HEIGHT_MAX;

                // North neighbour
                temp = GetPix(pHeight, Cols, x, y-1) + 1;
                if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // South neighbour
                temp = GetPix(pHeight, Cols, x, y+1) + 1;
                if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // West neighbour
                temp = GetPix(pHeight, Cols, x-1, y) + 1;
                if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
                    NewHeight = temp;
               
                // East neighbour
                temp = GetPix(pHeight, Cols, x+1, y) + 1;
                if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // Update the current Height to new Height
              //  if (CurHeight < NewHeight) {
                    PutPix(pHeight, Cols, x, y, NewHeight);
              //      rtn = true;
              //  }

            }
        }
#endif

#define BLOCK_RELABEL 1
#ifdef BLOCK_RELABEL
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
    bool rtn = false;

    for (int j = 0; j < BlkRows; j++) {
        for (int i = 0; i < BlkCols; i++) {

			// Skip inactive block
			if ( !GetPix(pBlockMask, BlkCols, i, j) ) {
				continue;
			}

            // Per block
            int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

            for (int y = 0; y < myBlk_Rows; y++) {

                int v = j * BlkHeight + y;
                HEIGHT_TYPE NewHeight[16];
                unsigned short mask[16];

                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    NewHeight[x] = HEIGHT_MAX;
                    mask[x] = ( GetPix(pExcessFlow, Cols, u, v) > 0 && GetPix(pHeight, Cols, u, v) < HEIGHT_MAX ) ? 1 : 0;
                }
                
                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    if (mask[x]) {
                        // North neighbour
                        temp = GetPix(pHeight, Cols, u, (v-1 < 0) ? 0 : v-1) + 1;
                        if (GetPix(pNorthCap, Cols, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                        
                        // South neighbour    
                        temp = GetPix(pHeight, Cols, u, (v+1 >= Rows) ? Rows-1 : v+1 ) + 1;
                        if (GetPix(pSouthCap, Cols, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                    }
                }

                // Now NewHeight[x] has results of north and south.

                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    if (mask[x]) {
                        // West neighbour
                        temp = GetPix(pHeight, Cols, (u-1 < 0) ? 0 : u-1, v) + 1;
                        if (GetPix(pWestCap, Cols, u, v) > 0 && temp < NewHeight[x]) {
                            NewHeight[x] = temp;
                        }

                        // East neighbour
                        temp = GetPix(pHeight, Cols, (u+1 >= Cols) ? Cols-1 : u+1, v) + 1;
                        if (GetPix(pEastCap, Cols, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                            
                        // Update Height so the next pixel to the right will use this new value
                        PutPix(pHeight, Cols, u, v, NewHeight[x]);  
                    }
                }

            }
        }
    }
#endif

}


///////////////////////// Global relabel func ////////////////////////////////
/*
void globalrelabeltile(height)
{
    done=0;
    while (!done) {
        done=1;
        foreach(x of tile) { 
        myheight=height(x);
        foreach(y=neighbor(x)) {
            if (edgecapacity(x,y)>0)
                myheight=min(myheight,height(y)+1);
            if(height(x)!=myheight){
                height(x)=myheight;
                done=0;
            }
        }
    }
}
*/
void CModel_Global_Relabel( //float * pExcessFlow, 
                            HEIGHT_TYPE * pHeight,
                            short * pWestCap,
                            short * pNorthCap,
                            short * pEastCap,
                            short * pSouthCap,
							unsigned char * pBlockMask,
                            int Rows,
                            int Cols,
							int BlkRows, 
							int BlkCols, 
							int BlkWidth, 
							int BlkHeight)   
{
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightCount = HEIGHT_MAX;
	int PrevCount;

    // Init Height
/*
	for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {
            if (GetPix(pHeight, Cols, x, y) != 0) 
                PutPix(pHeight, Cols, x, y, HEIGHT_MAX);
        }
*/
    // Init active pixel Height
    for (int j = 0; j < BlkRows; j++) {
        for (int i = 0; i < BlkCols; i++) {

			// Skip inactive block
			if ( !GetPix(pBlockMask, BlkCols, i, j) ) {
				continue;
			}

            // Border block handling
            int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

            for (int y = 0; y < myBlk_Rows; y++) {
                int v = j * BlkHeight + y;

                // Vertical
                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;

		            if (GetPix(pHeight, Cols, u, v) != 0) 
				        PutPix(pHeight, Cols, u, v, HEIGHT_MAX);
                }
			}
        }
	}


    // Global relabel
    do {
		PrevCount = NewHeightCount;
		NewHeightCount = 0;
/*
// This version processes each pixel in all directions before updating pHeight.
        for (int y = 0; y < Rows; y++)
            for (int x = 0; x < Cols; x++) {

                CurHeight = GetPix(pHeight, Cols, x, y);
                NewHeight = CurHeight;

                // West neighbour
                temp = GetPix(pHeight, Cols, x-1, y) + 1;
                if (GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
                    NewHeight = temp;
                
                // East neighbour
                temp = GetPix(pHeight, Cols, x+1, y) + 1;
                if (GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // North neighbour
                temp = GetPix(pHeight, Cols, x, y-1) + 1;
                if (GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // South neighbour
                temp = GetPix(pHeight, Cols, x, y+1) + 1;
                if (GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // Update the current Height to new Height
                if (CurHeight != NewHeight) {
                    PutPix(pHeight, Cols, x, y, NewHeight);
                    done = 0;
                }
            }
//*/
        // Block based global relabel
	    for (int j = 0; j < BlkRows; j++) {
		    for (int i = 0; i < BlkCols; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMask, BlkCols, i, j) ) {
					continue;
				}

                // Per block
                int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;
                        // North neighbour
                        temp = GetPix(pHeight, Cols, u, (v-1 < 0) ? 0 : v-1) + 1;
                        if (GetPix(pNorthCap, Cols, u, v) > 0 && temp < GetPix(pHeight, Cols, u, v)) {
                            PutPix(pHeight, Cols, u, v, temp);
                            NewH = 1; 
                        }
                        // South neighbour
                        temp = GetPix(pHeight, Cols, u, (v+1 >= Rows) ? Rows-1 : v+1) + 1;
						if (GetPix(pSouthCap, Cols, u, v) > 0 && temp < GetPix(pHeight, Cols, u, v)) {
                            PutPix(pHeight, Cols, u, v, temp);
                            NewH = 1; 
                        }
                    }

                    // Horizontal
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;
                        // West neighbour
                        temp = GetPix(pHeight, Cols, (u-1 < 0) ? 0 : u-1, v) + 1;
                        if (GetPix(pWestCap, Cols, u, v) > 0 && temp < GetPix(pHeight, Cols, u, v)) {
                            PutPix(pHeight, Cols, u, v, temp);
                            NewH = 1; 
                        }
                        // East neighbour
                        temp = GetPix(pHeight, Cols, (u+1 >= Cols) ? Cols-1 : u+1, v) + 1;
                        if (GetPix(pEastCap, Cols, u, v) > 0 && temp < GetPix(pHeight, Cols, u, v)) {
                            PutPix(pHeight, Cols, u, v, temp);
                            NewH = 1; 
                        }

						// Count
						if (NewH) {
							NewHeightCount++;
							NewH = 0;
						}
                    }
                } 
            } 
        } 

#ifdef _DEBUG
        printf("\tGlobal relabel new heights: %d\n", NewHeightCount);
#endif

    } while (/*PrevCount > NewHeightCount &&*/ NewHeightCount > 0); 
}

////////////////////////////////////////////////////////////////////////////////
int ActiveNodes(short * pExcessFlow, HEIGHT_TYPE * pHeight, int Rows, int Cols)
{
	int ActiveNodeCount = 0;

    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

    short CurExcessFlow;
    HEIGHT_TYPE CurHeight;

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {
	        CurHeight = *(pHeight + y*Cols + x);
		    if (CurHeight < HEIGHT_MAX) {
			    CurExcessFlow = *(pExcessFlow + y*Cols + x);
				if (CurExcessFlow > 0)
					ActiveNodeCount++;
			}
		}

    return ActiveNodeCount;
}

///////////////////////// Push func ////////////////////////////////
// Push func pseudo code
/*
if active(x) do
    foreach y = neighbor(x)
        if height(y) == height(x)¨C1 do // check height
            flow = min( capacity(x,y), excess_flow(x)); // pushed flow
            excess_flow(x) -= flow; // update excess flow 
            excess_flow(y) += flow;
            capacity(x,y) -= flow;  // update edge cap 
            capacity(y,x) += flow;
        done
    end
done
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CModel_GraphCut_Vertical_Push(short * pExcessFlow, 
									HEIGHT_TYPE * pHeight, 
									short * pNorthCap, 
									short * pSouthCap, 
									unsigned char * pBlockMask,
									int Rows, 
									int Cols, 
									int BlkRows, 
									int BlkCols, 
									int BlkWidth, 
									int BlkHeight)
{
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
    short flow;
	int nPush = 0;
	int sPush = 0;

//#define SCANLINE_V_PUSH 1
#ifdef SCANLINE_V_PUSH
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

            if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {

                // North neighbour (x, y-1)
                if (y > 0) {
                    if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x, y-1) == GetPix(pHeight, Cols, x, y) - 1) {
                        flow = min( GetPix(pNorthCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlow, Cols, x, y, (short) -flow);
                            AddPix(pExcessFlow, Cols, x, y-1, flow);
                            AddPix(pNorthCap, Cols, x, y, (short) -flow);
                            AddPix(pSouthCap, Cols, x, y-1, flow);
							nPush++;
                        }
                    }
                }

                // South neighbour (x, y+1)
                if (y < Rows-1) {
                    if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x, y+1) == GetPix(pHeight, Cols, x, y) - 1) {
                        flow = min( GetPix(pSouthCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y) );

                        if (flow != 0) {
                            AddPix(pExcessFlow, Cols, x, y, (short) -flow);
                            AddPix(pExcessFlow, Cols, x, y+1, flow);
                            AddPix(pSouthCap, Cols, x, y, (short) -flow);
                            AddPix(pNorthCap, Cols, x, y+1, flow);
							sPush++;
                        }
                    }
                }

            }
        }
#endif

#define BLOCK_V_PUSH 1
#ifdef BLOCK_V_PUSH
	    for (int j = 0; j < BlkRows; j++) {
		    for (int i = 0; i < BlkCols; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMask, BlkCols, i, j) ) {
					continue;
				}

                // Handle block border
                int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;

			            if (GetPix(pHeight, Cols, u, v) < HEIGHT_MAX) {

			                // North neighbour (u, v-1)
			                if (v > 0) {
						        if (GetPix(pExcessFlow, Cols, u, v) > 0 && GetPix(pHeight, Cols, u, v-1) == GetPix(pHeight, Cols, u, v) - 1) {
			                        flow = min( GetPix(pNorthCap, Cols, u, v), GetPix(pExcessFlow, Cols, u, v) );
						            if (flow != 0) {
									    AddPix(pExcessFlow, Cols, u, v, (short) -flow);
			                            AddPix(pExcessFlow, Cols, u, v-1, flow);
						                AddPix(pNorthCap, Cols, u, v, (short) -flow);
									    AddPix(pSouthCap, Cols, u, v-1, flow);
										nPush++;
						            }
			                    }
						    }

			                // South neighbour (u, v+1)
							if (v < Rows-1) {
								if (GetPix(pExcessFlow, Cols, u, v) > 0 && GetPix(pHeight, Cols, u, v+1) == GetPix(pHeight, Cols, u, v) - 1) {
									flow = min( GetPix(pSouthCap, Cols, u, v), GetPix(pExcessFlow, Cols, u, v) );

			                        if (flow != 0) {
						                AddPix(pExcessFlow, Cols, u, v, (short) -flow);
									    AddPix(pExcessFlow, Cols, u, v+1, flow);
			                            AddPix(pSouthCap, Cols, u, v, (short) -flow);
						                AddPix(pNorthCap, Cols, u, v+1, flow);
										sPush++;
						            }
			                    }
			                }
						}

                    } // x
                } // y
            } // i
        } // j

#endif

#ifdef _DEBUG
		printf("nPush = %6d, sPush = %6d, ", nPush, sPush);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CModel_GraphCut_Horizontal_Push(short * pExcessFlow, 
									HEIGHT_TYPE * pHeight, 
									short * pWestCap, 
									short * pEastCap, 
									unsigned char * pBlockMask,
									int Rows, 
									int Cols, 
									int BlkRows, 
									int BlkCols, 
									int BlkWidth, 
									int BlkHeight)
{
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
    short flow;
    int rtn = 0;
	int wPush = 0;
	int ePush = 0;

//#define SCANLINE_H_PUSH 1
#ifdef SCANLINE_H_PUSH
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

            if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {

                // West neighbour (x-1, y)
                if (x > 0) {
                    if ( GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x-1, y) == GetPix(pHeight, Cols, x, y) - 1 ) {
                        flow = min( GetPix(pWestCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlow, Cols, x, y, (short) -flow);
                            AddPix(pExcessFlow, Cols, x-1, y, flow);
                            AddPix(pWestCap, Cols, x, y, (short) -flow);
                            AddPix(pEastCap, Cols, x-1, y, flow);
							wPush++;
                        }
                    }
                }

                // East neighbour (x+1, y)
                if (x < Cols-1) {
                    if (GetPix(pExcessFlow, Cols, x, y) > 0 && GetPix(pHeight, Cols, x+1, y) == GetPix(pHeight, Cols, x, y) - 1) {
                        flow = min( GetPix(pEastCap, Cols, x, y), GetPix(pExcessFlow, Cols, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlow, Cols, x, y, (short) -flow);
                            AddPix(pExcessFlow, Cols, x+1, y, flow);
                            AddPix(pEastCap, Cols, x, y, (short) -flow);
                            AddPix(pWestCap, Cols, x+1, y, flow);
							ePush++;
                        }
                    }
                }

            }
        }
#endif

#define BLOCK_H_PUSH 1
#ifdef BLOCK_H_PUSH
	    for (int j = 0; j < BlkRows; j++) {
		    for (int i = 0; i < BlkCols; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMask, BlkCols, i, j) ) {
					continue;
				}

                // Handle block border
                int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;

			            if (GetPix(pHeight, Cols, u, v) < HEIGHT_MAX) {

							// West neighbour (u-1, v)
                            if (u > 0) {
                                if ( GetPix(pExcessFlow, Cols, u, v) > 0 && GetPix(pHeight, Cols, u-1, v) == GetPix(pHeight, Cols, u, v) - 1 ) {
                                    flow = min( GetPix(pWestCap, Cols, u, v), GetPix(pExcessFlow, Cols, u, v) );
                                    if (flow != 0) {
                                        AddPix(pExcessFlow, Cols, u, v, (short) -flow);
                                        AddPix(pExcessFlow, Cols, u-1, v, flow);
                                        AddPix(pWestCap, Cols, u, v, (short) -flow);
                                        AddPix(pEastCap, Cols, u-1, v, flow);
										wPush++;
                                    }
                                }
                            }

                            // East neighbour (u+1, v)
                            if (u < Cols-1) {
                                if (GetPix(pExcessFlow, Cols, u, v) > 0 && GetPix(pHeight, Cols, u+1, v) == GetPix(pHeight, Cols, u, v) - 1) {
                                    flow = min( GetPix(pEastCap, Cols, u, v), GetPix(pExcessFlow, Cols, u, v) );
                                    if (flow != 0) {
                                        AddPix(pExcessFlow, Cols, u, v, (short) -flow);
                                        AddPix(pExcessFlow, Cols, u+1, v, flow);
                                        AddPix(pEastCap, Cols, u, v, (short) -flow);
                                        AddPix(pWestCap, Cols, u+1, v, flow);
										ePush++;
                                    }
                                }
                            }
						}

                    } // x
                } // y
            } // i
        } // j

#endif


#ifdef _DEBUG
	printf("wPush = %6d, ePush = %6d, ", wPush, ePush);
#endif

	rtn = ActiveNodes(pExcessFlow, pHeight, Rows, Cols);
    return rtn;
}


/////////////////// Graph Cut entry function //////////////////////////////////////////////////////////////////
void CModel_Push_Relabel(short * pExcessFlow, HEIGHT_TYPE * pHeight, short * pWestCap, short * pNorthCap, 
						short * pEastCap, short * pSouthCap, unsigned char * pBlockMask, unsigned char * pOutput, 
						int Rows, int Cols,  int BlkRows, int BlkCols, int BlkWidth, int BlkHeight, int nRatio)
{
    int iter = 1;
    int ActiveNode;

	CModel_BlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows, BlkCols, BlkWidth, BlkHeight);

    do {
#ifdef _DEBUG
		printf("%3d, ", iter);
#endif
        // Relabel all active nodes
        if (iter % nRatio) {
            CModel_GraphCut_Relabel(pExcessFlow, pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);

        } else {    // Global relabel 
            CModel_Global_Relabel(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap, pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
        }

#ifdef _DEBUG
		char fn[128];
        sprintf(fn, ".\\Output\\_%d_CPU_Height.%dx%d.Y8", iter, sizeof(HEIGHT_TYPE)*Cols, Rows);
        Dump2File(fn, (unsigned char *) pHeight, Rows*Cols*sizeof(HEIGHT_TYPE));

        sprintf(fn, ".\\Output\\_%d_CPU_BlockMask.%dx%d.Y8", iter, BlkCols, BlkRows);
        Dump2File(fn, (unsigned char *) pBlockMask, BlkRows*BlkCols);
#endif
        // Push all active nodes in all directions
        CModel_GraphCut_Vertical_Push(pExcessFlow, pHeight, pNorthCap, pSouthCap, pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
        ActiveNode = CModel_GraphCut_Horizontal_Push(pExcessFlow, pHeight, pWestCap, pEastCap, pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
		CModel_BlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
		printf("Active pixels = %6d\n", ActiveNode);
#endif
    } while (ActiveNode > 0 && ++iter < 256);
	
	printf( "CPU Graph-Cut loops = %d\n", iter);

    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

    // Write pOutput
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {
            unsigned char val = GetPix(pHeight, Cols, x, y) < HEIGHT_MAX ? 0 : 255;
            PutPix(pOutput, Cols, x, y, val);
        }

}

////////////////////////////////////////////////////////////////////////////////////////////
void CModel_Edge_Stabilization( unsigned char * color,              // RGB, 3 bytes per pixel
                                unsigned char * last_color,         // RGB, 3 bytes per pixel
                                unsigned char * region_map,         // byte
                                unsigned char * last_region_map,    // byte
                                float * finger_region,              // float
                                char * confidence,                  // byte
                                unsigned char * output,             // byte
                                int width, 
                                int height )
{
    short COLOR_THRES_BG = 25;
    short COLOR_THRES_FG = 40;
    short MORPH_SIZE = 5;
    char MAX_CONFIDENCE = 5;
    unsigned char REGION_FOREGROUND = 255; //8;
    unsigned char REGION_BACKGROUND = 0;   //4;

    unsigned char * pDilate = (unsigned char * ) malloc(width * height);
    unsigned char * pErode = (unsigned char * ) malloc(width * height);
    unsigned char * pStabilizingROI = (unsigned char * ) malloc(width * height);

    // Filtering applies only to pixels around the cut.
    CModel_Dilate(region_map, pDilate, width, height, 5);
    CModel_Erode(region_map, pErode, width, height, 5);

    for (int j = 0; j < width * height; j++) {
        pStabilizingROI[j] = pDilate[j] - pErode[j];
    }

//    Dump2File("CPU_StabilizingROI.320x180.Y8", pStabilizingROI, width*height);
//    Dump2File("CPU_Confidence1.320x180.Y8", (unsigned char *) confidence, width*height);

    int i = 0;
 
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++, i++) {

            // Update confidence map. Increase when it's FG. Decrease when it's BG.
            if (region_map[i] == REGION_FOREGROUND) {
                confidence[i] = min(confidence[i]+1, MAX_CONFIDENCE);
            } else {
                confidence[i] = max(confidence[i]-1, -MAX_CONFIDENCE);
            }

            //if (pStabilizingROI[i]) //only apply inside the stabilizing ROI
            if (pStabilizingROI[i] && finger_region[i] == 0.0f) //only apply inside the stabilizing ROI and not finger area
            {
                if (last_region_map[i] != region_map[i])  // AND when there is a change in region map
                {
                    //check if its color doesn't change between current and last frame
                    // L1 difference of two color pixels. RGB
                    int d = abs(color[3*i] - last_color[3*i]) + abs(color[3*i+1] - last_color[3*i+1]) + abs(color[3*i+2] - last_color[3*i+2]);

                    if (d < COLOR_THRES_FG) //AND when there is a considerable change in color.
                    {
                        if (confidence[i] == MAX_CONFIDENCE)
                        {
                            // This means graph cut has consistently labeled this pixel as FG
                            // in the last MAX_CONFIDENCE frames, so we should trust it.
                            // This helps avoid propagating mistakes when stabilizing edges
                            // between frames (if happens).
                            region_map[i] = REGION_FOREGROUND;
                        }
                        else if (confidence[i] == -MAX_CONFIDENCE)
                        {
                            //Similarly
                            region_map[i] = REGION_BACKGROUND;
                        }
                        else
                        {
                            // For those pixels where the result of graph cut is not stable,
                            // we choose to keep the region label of the previous frame to stabilize edges.
                            if ((region_map[i] == REGION_FOREGROUND) ||
                                (d < COLOR_THRES_BG && region_map[i] == REGION_BACKGROUND))
                                // COLOR_THRES_BG is smaller than COLOR_THRES_FG.
                                // It means that we are more careful when converting a BG pixel into FG pixel.
                            {
                                region_map[i] = last_region_map[i];
                            }
                        }
                    }
                }
            }
        }
    }

    //cv::Mat roi(height, width, CV_8U, (uint8_t*)_roi.data());
    //cv::medianBlur(roi, roi, 3);  // Algorithm: sum up 3x3 pixels, if sum >= 5, its median = 1, otherwise = 0.
    // src: region_map

//    Dump2File("CPU_before_median.320x180.Y8", region_map, width*height);
//    Dump2File("CPU_Confidence4.320x180.Y8", (unsigned char *) confidence, width*height);

    CModel_BinaryMedian(region_map, output, pStabilizingROI, height, width, 3, 0);
    //memcpy(output, region_map, height*width);

    //Save the current roi for later use.
/*
    for (int i = 0; i < width * height; i++) {
        last_region_map[i] = output[i];
        last_color[i] = color[i];
    }
*/
    // Caller is responsible to passing correct last_region_map and last_color.  No need to copy here

    free(pDilate);
    free(pErode);
    free(pStabilizingROI);
}
