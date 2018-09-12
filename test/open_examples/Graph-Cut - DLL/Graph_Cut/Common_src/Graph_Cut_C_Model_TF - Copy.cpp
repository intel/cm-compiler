#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>

#include "common_C_model.h"
#include "Graph_Cut_host.h"

////////////////////////////////////////////////////
void XYtoTF(int x, int y, int &u, int &v) 
{
	u = x;
	v = y + x;
}

void RestoreTFCoordinate(int x, int y, int &u, int &v) 
{
	u = x;
	v = y - x;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Graph Cut (Push and Relabel) Transformed MWYI
void PushRelabel_Init_TF(	short * pWeight, 
						short * pHCap, 
						short * pVCap, 
						int Rows, 
						int Cols, 
						short * pExcessFlowTF, 
						HEIGHT_TYPE * pHeightTF, 
						short * pWestCapTF, 
						short * pNorthCapTF, 
						short * pEastCapTF, 
						short * pSouthCapTF)
{
    int ColsTF = Cols;
	short w;
    HEIGHT_TYPE HEIGHT_MAX = min( Rows * Cols, TYPE_MAX-1);	// Must have -1 to prevent overflow, some calculation needs HEIGHT_MAX+1

    for (int y = 0; y < Rows; y++) {
        // Index for destination
//		int index = y;

		for (int x = 0; x < Cols; x++) {
			
			int u, v;
			XYtoTF(x, y, u, v);			// (u,v) is the tranformed address of (x,y)
			
			// Set excess flow
            w = GetPix(pWeight, Cols, x, y);
            PutPix(pExcessFlowTF, ColsTF, u, v, w);    // w>0 if a cap from source exists, w<0 if cap from sink exists. ExcessFlow = w.
            PutPix(pHeightTF, ColsTF, u, v, (HEIGHT_TYPE) 0);

            // Set west and east caps
            w = GetPix(pHCap, Cols, x, y);
            if (x == 0) {
                PutPix(pWestCapTF, ColsTF, u, v, (short) 0);
            } else if (x < Cols-1) {
                PutPix(pWestCapTF, ColsTF, u, v, w);
                PutPix(pEastCapTF, ColsTF, u-1, v, w);
            } else {    // x = Cols-1
                PutPix(pWestCapTF, ColsTF, u, v, w);
                PutPix(pEastCapTF, ColsTF, u-1, v, w);
                PutPix(pEastCapTF, ColsTF, u, v, (short) 0);
            }

            // Set north and south caps
            w = GetPix(pVCap, Cols, x, y);
            if (y == 0) {
                PutPix(pNorthCapTF, ColsTF, u, v, (short) 0);
            } else if (y < Rows-1) {
                PutPix(pNorthCapTF, ColsTF, u, v, w);
                PutPix(pSouthCapTF, ColsTF, u, v-1, w);
            } else {
                PutPix(pNorthCapTF, ColsTF, u, v, w);
                PutPix(pSouthCapTF, ColsTF, u, v-1, w);
                PutPix(pSouthCapTF, ColsTF, u, v, (short) 0);
            }
			
//			index++;
        }
/*
		printf("%d ", GetPix(pWeight, Cols, 0, 0));
		printf("%d ", GetPix(pWeight, Cols, 1, 0));
		printf("%d ", GetPix(pWeight, Cols, 2, 0));
		printf("%d ", GetPix(pWeight, Cols, 3, 0));
		printf("\n");

		printf("%d ", GetPix(pExcessFlow, Cols, 0, 0));
		printf("%d ", GetPix(pExcessFlow, Cols, 1, 1));
		printf("%d ", GetPix(pExcessFlow, Cols, 2, 2));
		printf("%d ", GetPix(pExcessFlow, Cols, 3, 3));
		printf("\n----Row 0-----------\n");

		printf("%d ", GetPix(pWeight, Cols, 0, 1));
		printf("%d ", GetPix(pWeight, Cols, 1, 1));
		printf("%d ", GetPix(pWeight, Cols, 2, 1));
		printf("%d ", GetPix(pWeight, Cols, 3, 1));
		printf("\n");

		printf("%d ", GetPix(pExcessFlow, Cols, 0, 1));
		printf("%d ", GetPix(pExcessFlow, Cols, 1, 2));
		printf("%d ", GetPix(pExcessFlow, Cols, 2, 3));
		printf("%d ", GetPix(pExcessFlow, Cols, 3, 4));
		printf("\n----Row 1-----------\n");

		printf("%d ", GetPix(pWeight, Cols, 0, 2));
		printf("%d ", GetPix(pWeight, Cols, 1, 2));
		printf("%d ", GetPix(pWeight, Cols, 2, 2));
		printf("%d ", GetPix(pWeight, Cols, 3, 2));
		printf("\n");

		printf("%d ", GetPix(pExcessFlow, Cols, 0, 2));
		printf("%d ", GetPix(pExcessFlow, Cols, 1, 3));
		printf("%d ", GetPix(pExcessFlow, Cols, 2, 4));
		printf("%d ", GetPix(pExcessFlow, Cols, 3, 5));
		printf("\n----Row 2-----------\n");

		printf("%d ", GetPix(pWeight, Cols, 0, 3));
		printf("%d ", GetPix(pWeight, Cols, 1, 3));
		printf("%d ", GetPix(pWeight, Cols, 2, 3));
		printf("%d ", GetPix(pWeight, Cols, 3, 3));
		printf("\n");

		printf("%d ", GetPix(pExcessFlow, Cols, 0, 3));
		printf("%d ", GetPix(pExcessFlow, Cols, 1, 4));
		printf("%d ", GetPix(pExcessFlow, Cols, 2, 5));
		printf("%d ", GetPix(pExcessFlow, Cols, 3, 6));
		printf("\n----Row 3-----------\n");
		*/
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Check if (x,y) is an OOB pixel
//	+----+
//	|*	 |
//	|**	 |
//	|*** |
//	|****|
//	|****|
//	| ***|
//	|  **|
//	|   *|
//	+----+
// Return: 
// true, if (x,y) is out of * area
// false, if (x,y) is in * area
bool inline IsPixelOOB(int x, int y, int RowsTF, int ColsTF)
{
int FrameHeight = RowsTF - ColsTF;		// Restoe the original image height

	// Rows and Cols are expanded size
	if (x < 0 || x > ColsTF-1 || y < 0 || y > RowsTF)
		return true;

	if (x > y || y >= x + FrameHeight)
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// All input data are in shift-down format.
void CreateBlockMaskTF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, int RowsTF, int ColsTF, 
						unsigned char * pBlockMaskTF, int BlkRowsTF, int BlkColsTF, int BlkWidth, int BlkHeight)
{
	HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
	
	memset(pBlockMaskTF, 0, BlkRowsTF*BlkColsTF);

	int BlkRows = BlkRowsTF - BlkColsTF;
	int BlkCols = BlkColsTF;
	int ImgRows = RowsTF - ColsTF;
	int ImgCols = ColsTF;

	// (i,j) is global block coordinate before TF.
	// (iTF,jTF) is global block coordinate after TF.
	// (x,y) is local pixel coordinate within a block before TF.
	// (u,v) is global pixel coordinate before TF.
	// (uTF,vTF) is global pixel coordinate after TF.
/*
	// Loop through all blocks
    for (int j = 0; j < BlkRows; j++) {
        for (int i = 0; i < BlkCols; i++) {
			int iTF, jTF;
			XYtoTF(i, j, iTF, jTF);	

            // Row OOB check
            int myBlkHeight = ((j+1)*BlkHeight < ImgRows) ? BlkHeight : (ImgRows - j*BlkHeight);
			int active = 0;

            for (int y = 0; y < myBlkHeight; y++) {
				int	v = j * BlkHeight + y;
				for (int x = 0; x < BlkWidth; x++) {
					int u = i * BlkWidth + x;
					int uTF, vTF;
					XYtoTF(u, v, uTF, vTF);	

					if ( GetPix(pExcessFlowTF, ColsTF, uTF, vTF) > -100 && GetPix(pHeightTF, ColsTF, uTF, vTF) < HEIGHT_MAX ) {
						PutPix(pBlockMaskTF, BlkColsTF, iTF, jTF, (unsigned char) 255);  
						active = 1;
						break;
					}
				}
				if (active)
					break;
			}
		}
	}
*/
//*
	// Loop through all blocks
    for (int j = 0; j < BlkRowsTF; j++) {
        for (int i = 0; i < BlkColsTF; i++) {

            // Row OOB check
            int myBlkHeight = ((j+1)*BlkHeight < RowsTF) ? BlkHeight : (RowsTF - j*BlkHeight);
			int active = 0;

			// Copy a block
            for (int y = 0; y < myBlkHeight; y++) {
				int	v = j * BlkHeight + y;
				for (int x = 0; x < BlkWidth; x++) {
					int u = i * BlkWidth + x;

					if (IsPixelOOB(u, v, RowsTF, ColsTF))		// Check if (u,v) is an OOB pixel
						continue;

					if ( GetPix(pExcessFlowTF, ColsTF, u, v) > -100 && GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX ) {
						PutPix(pBlockMaskTF, BlkColsTF, i, j, (unsigned char) 255);  
						active = 1;
						break;
					}
				}
				if (active)
					break;
			}

        }
    }
//*/
}


////////////////////////////////////////////////////////////////////////////////
int GetActivePixelsTF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, int RowsTF, int ColsTF)
{
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
	int ActivePixels = 0;
    short CurExcessFlow;
    HEIGHT_TYPE CurHeight;

    for (int y = 0; y < RowsTF; y++)
        for (int x = 0; x < ColsTF; x++) {

			if (IsPixelOOB(x, y, RowsTF, ColsTF))		// Skip if (u,v) is an OOB pixel.  Rows-Cols is the original frame height.
				continue;

	        CurHeight = *(pHeightTF + y*ColsTF + x);
		    if (CurHeight < HEIGHT_MAX) {
			    CurExcessFlow = *(pExcessFlowTF + y*ColsTF + x);
				if (CurExcessFlow > 0)
					ActivePixels++;
			}
		}

    return ActivePixels;
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
void CModel_GraphCut_RelabelTF( short * pExcessFlowTF, 
                                HEIGHT_TYPE * pHeightTF,
                                short * pWestCapTF,
                                short * pNorthCapTF,
                                short * pEastCapTF,
                                short * pSouthCapTF,
								unsigned char * pBlockMaskTF,
                                int RowsTF,
                                int ColsTF,
								int BlkRowsTF, 
								int BlkColsTF, 
								int BlkWidth, 
								int BlkHeight)   
{
	short r = 0;
	int v;

#define SCANLINE_RELABEL 1
#ifdef SCANLINE_RELABEL

// Scan line based Relabel
    HEIGHT_TYPE NewHeight, temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);

    for (int y = 0; y < RowsTF; y++)
        for (int x = 0; x < ColsTF; x++) {

			// Skip if (x,y) is an OOB pixel.
			if (IsPixelOOB(x, y, RowsTF, ColsTF))
				continue;

            // Check for active node
            float CurExcessFlow = GetPix(pExcessFlowTF, ColsTF, x, y);
            HEIGHT_TYPE CurHeight = GetPix(pHeightTF, ColsTF, x, y);
        
            if (CurExcessFlow > 0 && CurHeight < HEIGHT_MAX) {  

                NewHeight = HEIGHT_MAX;

                // North neighbour
				v = (IsPixelOOB(x, y-1, RowsTF, ColsTF)) ? y : y-1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, x, v) + 1;
                if (r = GetPix(pNorthCapTF, ColsTF, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // South neighbour
				v = (IsPixelOOB(x, y+1, RowsTF, ColsTF)) ? y : y+1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, x, v) + 1;
                if (r = GetPix(pSouthCapTF, ColsTF, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // West neighbour at (x-1, y-1)
				if (IsPixelOOB(x-1, y-1, RowsTF, ColsTF))   // Check if west neighbour is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, x, y) + 1;
				else 
	                temp = GetPix(pHeightTF, ColsTF, x-1, y-1) + 1;
                if (r = GetPix(pWestCapTF, ColsTF, x, y) > 0 && temp < NewHeight)
                    NewHeight = temp;
               
                // East neighbour at (x+1, y+1)
				if (IsPixelOOB(x+1, y+1, RowsTF, ColsTF))   // Check if east neighbour is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, x, y) + 1;	
				else
					temp = GetPix(pHeightTF, ColsTF, x+1, y+1) + 1;	
                if (r = GetPix(pEastCapTF, ColsTF, x, y) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // Update the current Height to new Height
				PutPix(pHeightTF, ColsTF, x, y, NewHeight);
            }
        }
#endif

//#define BLOCK_RELABEL 1
#ifdef BLOCK_RELABEL
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
//    bool rtn = false;

    for (int j = 0; j < BlkRowsTF; j++) {
        for (int i = 0; i < BlkColsTF; i++) {

			// Skip inactive block
			if ( !GetPix(pBlockMaskTF, BlkColsTF, i, j) ) {
				continue;
			}

            // Per block
            int myBlk_Rows = ((j+1)*BlkHeight < RowsTF) ? BlkHeight : (RowsTF - j*BlkHeight);

            for (int y = 0; y < myBlk_Rows; y++) {

                int v = j * BlkHeight + y;
                HEIGHT_TYPE NewHeight[16];
                unsigned short mask[16];

                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    NewHeight[x] = HEIGHT_MAX;
                    mask[x] = ( GetPix(pExcessFlowTF, ColsTF, u, v) > 0 && GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX ) ? 1 : 0;
                }
                
                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    if (mask[x]) {
                        // North neighbour
                        temp = GetPix(pHeightTF, ColsTF, u, (v-1 < 0) ? 0 : v-1) + 1;
                        if (GetPix(pNorthCapTF, ColsTF, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                        
                        // South neighbour    
                        temp = GetPix(pHeightTF, ColsTF, u, (v+1 >= RowsTF) ? RowsTF-1 : v+1 ) + 1;
                        if (GetPix(pSouthCapTF, ColsTF, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                    }
                }

                // Now NewHeight[x] has results of north and south.

                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;
                    if (mask[x]) {
                        // West neighbour
                        temp = GetPix(pHeightTF, ColsTF, (u-1 < 0) ? 0 : u-1, v) + 1;
                        if (GetPix(pWestCapTF, ColsTF, u, v) > 0 && temp < NewHeight[x]) {
                            NewHeight[x] = temp;
                        }

                        // East neighbour
                        temp = GetPix(pHeightTF, ColsTF, (u+1 >= ColsTF) ? ColsTF-1 : u+1, v) + 1;
                        if (GetPix(pEastCapTF, ColsTF, u, v) > 0 && temp < NewHeight[x]) 
                            NewHeight[x] = temp;
                            
                        // Update Height so the next pixel to the right will use this new value
                        PutPix(pHeightTF, ColsTF, u, v, NewHeight[x]);  
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
void CModel_Global_RelabelTF( //float * pExcessFlowTF, 
                            HEIGHT_TYPE * pHeightTF,
                            short * pWestCapTF,
                            short * pNorthCapTF,
                            short * pEastCapTF,
                            short * pSouthCapTF,
							unsigned char * pBlockMaskTF,
                            int RowsTF,
                            int ColsTF,
							int BlkRowsTF, 
							int BlkColsTF, 
							int BlkWidth, 
							int BlkHeight)   
{
	HEIGHT_TYPE NewHeight;
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightBlocks = HEIGHT_MAX;
	int PrevCount;
	int iter = 0;
	int v;

    // Init active pixel Height
	for (int y = 0; y < RowsTF; y++) {
        for (int x = 0; x < ColsTF; x++) {

			// Skip if (x,y) is an OOB pixel.
			if (IsPixelOOB(x, y, RowsTF, ColsTF))
				continue;

            if (GetPix(pHeightTF, ColsTF, x, y) != 0) 
                PutPix(pHeightTF, ColsTF, x, y, HEIGHT_MAX);
        }
	}
/*
	// Block based Height init
    for (int j = 0; j < BlkRowsTF; j++) {
        for (int i = 0; i < BlkColsTF; i++) {

			// Skip inactive block
			if ( !GetPix(pBlockMaskTF, BlkColsTF, i, j) ) {
				continue;
			}

            // Border block handling
            int myBlk_Rows = ((j+1)*BlkHeight < RowsTF) ? BlkHeight : (RowsTF - j*BlkHeight);

            for (int y = 0; y < myBlk_Rows; y++) {
                int v = j * BlkHeight + y;

                // Vertical
                for (int x = 0; x < BlkWidth; x++) {
                    int u = i * BlkWidth + x;

		            if (GetPix(pHeightTF, ColsTF, u, v) != 0) 
				        PutPix(pHeightTF, ColsTF, u, v, HEIGHT_MAX);
                }
			}
        }
	}
*/

    // Global relabel
    do {
		PrevCount = NewHeightBlocks;
		NewHeightBlocks = 0;

#define SCANLINE_GR 1
#ifdef SCANLINE_GR
		// This version processes each pixel in all directions before updating pHeight.
        for (int y = 0; y < RowsTF; y++) {
            for (int x = 0; x < ColsTF; x++) {

				// Skip if (x,y) is an OOB pixel.
				if (IsPixelOOB(x, y, RowsTF, ColsTF))
					continue;

                NewHeight = GetPix(pHeightTF, ColsTF, x, y);

                // North neighbour
				v = (IsPixelOOB(x, y-1, RowsTF, ColsTF)) ? y : y-1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, x, v) + 1;
                if (GetPix(pNorthCapTF, ColsTF, x, y) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // South neighbour
				v = (IsPixelOOB(x, y+1, RowsTF, ColsTF)) ? y : y+1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, x, v) + 1;
                if (GetPix(pSouthCapTF, ColsTF, x, y) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // West neighbour
				if (IsPixelOOB(x-1, y-1, RowsTF, ColsTF))   // Check if west neighbour (x-1,y-1) is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, x, y) + 1;
				else 
	                temp = GetPix(pHeightTF, ColsTF, x-1, y-1) + 1;
                if (GetPix(pWestCapTF, ColsTF, x, y) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // East neighbour
				if (IsPixelOOB(x+1, y+1, RowsTF, ColsTF))   // Check if east neighbour (x+1,y+1) is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, x, y) + 1;	
				else
					temp = GetPix(pHeightTF, ColsTF, x+1, y+1) + 1;	
                if (GetPix(pEastCapTF, ColsTF, x, y) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // Update the current Height to new Height
				if (NewH) {
                    PutPix(pHeightTF, ColsTF, x, y, NewHeight);
					NewHeightBlocks++;	// Increase active pixel count
					NewH = 0;
				}
            }
		}
#endif

//#define BLOCK_GR 1
#ifdef BLOCK_GR
        // Block based global relabel
	    for (int j = 0; j < BlkRowsTF; j++) {
		    for (int i = 0; i < BlkColsTF; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMaskTF, BlkColsTF, i, j) ) {
					continue;
				}

                // Per block
                int myBlk_Rows = ((j+1)*BlkHeight < RowsTF) ? BlkHeight : (RowsTF - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;
                        // North neighbour
                        temp = GetPix(pHeightTF, ColsTF, u, (v-1 < 0) ? 0 : v-1) + 1;
                        if (GetPix(pNorthCapTF, ColsTF, u, v) > 0 && temp < GetPix(pHeightTF, ColsTF, u, v)) {
                            PutPix(pHeightTF, ColsTF, u, v, temp);
                            NewH = 1; 
                        }
                        // South neighbour
                        temp = GetPix(pHeightTF, ColsTF, u, (v+1 >= RowsTF) ? RowsTF-1 : v+1) + 1;
						if (GetPix(pSouthCapTF, ColsTF, u, v) > 0 && temp < GetPix(pHeightTF, ColsTF, u, v)) {
                            PutPix(pHeightTF, ColsTF, u, v, temp);
                            NewH = 1; 
                        }
                    }

                    // Horizontal
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;
                        // West neighbour
                        temp = GetPix(pHeightTF, ColsTF, (u-1 < 0) ? 0 : u-1, v) + 1;
                        if (GetPix(pWestCapTF, ColsTF, u, v) > 0 && temp < GetPix(pHeightTF, ColsTF, u, v)) {
                            PutPix(pHeightTF, ColsTF, u, v, temp);
                            NewH = 1; 
                        }
                        // East neighbour
                        temp = GetPix(pHeightTF, ColsTF, (u+1 >= ColsTF) ? ColsTF-1 : u+1, v) + 1;
                        if (GetPix(pEastCapTF, ColsTF, u, v) > 0 && temp < GetPix(pHeightTF, ColsTF, u, v)) {
                            PutPix(pHeightTF, ColsTF, u, v, temp);
                            NewH = 1; 
                        }
                    } // x
                } // y

				// Count
				if (NewH) {
					NewHeightBlocks++;	// Increase active pixel count
					NewH = 0;
				}
            }	// i
        }	// j
#endif

#ifdef _DEBUG
        printf("\tGlobal Relabel new height nodes: %d\n", NewHeightBlocks);

//		char fn[128];
//		sprintf(fn, ".\\Output\\_%d_CPU_TF_GR_Height.%dx%d.Y8", iter, ColsTF, RowsTF);
//		Dump2File_8bit(fn, pHeightTF, RowsTF*ColsTF);

#endif

    } while (/*PrevCount > NewHeightCount &&*/ NewHeightBlocks > 0 && ++iter < 32); 
}

///////////////////////// Push func ////////////////////////////////
// Push func pseudo code
/*
if active(x) do
    foreach y = neighbor(x)
        if height(y) == height(x)-1 do // check height
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
void CModel_GraphCut_Vertical_PushTF(short * pExcessFlowTF, 
									HEIGHT_TYPE * pHeightTF, 
									short * pNorthCapTF, 
									short * pSouthCapTF, 
									unsigned char * pBlockMaskTF,
									int RowsTF, 
									int ColsTF, 
									int BlkRowsTF, 
									int BlkColsTF, 
									int BlkWidth, 
									int BlkHeight)
{
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
    short flow;
	int nPush = 0;
	int sPush = 0;

#define SCANLINE_V_PUSH 1
#ifdef SCANLINE_V_PUSH
    for (int y = 0; y < RowsTF; y++)
        for (int x = 0; x < ColsTF; x++) {

			// Skip if (x,y) is an OOB pixel.
			if (IsPixelOOB(x, y, RowsTF, ColsTF))
				continue;

            if (GetPix(pHeightTF, ColsTF, x, y) < HEIGHT_MAX) {

                // North neighbour (x, y-1)
                if (!IsPixelOOB(x, y-1, RowsTF, ColsTF))		// Check if (x,y-1) is an OOB pixel.
				{
                    if (GetPix(pExcessFlowTF, ColsTF, x, y) > 0 && GetPix(pHeightTF, ColsTF, x, y-1) == GetPix(pHeightTF, ColsTF, x, y) - 1) {
                        flow = min( GetPix(pNorthCapTF, ColsTF, x, y), GetPix(pExcessFlowTF, ColsTF, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, x, y, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, x, y-1, flow);
                            AddPix(pNorthCapTF, ColsTF, x, y, (short) -flow);
                            AddPix(pSouthCapTF, ColsTF, x, y-1, flow);
							nPush++;
                        }
                    }
                }

                // South neighbour (x, y+1)
                if (!IsPixelOOB(x, y+1, RowsTF, ColsTF))		// Check if (x,y+1) is an OOB pixel.
				{
                    if (GetPix(pExcessFlowTF, ColsTF, x, y) > 0 && GetPix(pHeightTF, ColsTF, x, y+1) == GetPix(pHeightTF, ColsTF, x, y) - 1) {
                        flow = min( GetPix(pSouthCapTF, ColsTF, x, y), GetPix(pExcessFlowTF, ColsTF, x, y) );

                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, x, y, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, x, y+1, flow);
                            AddPix(pSouthCapTF, ColsTF, x, y, (short) -flow);
                            AddPix(pNorthCapTF, ColsTF, x, y+1, flow);
							sPush++;
                        }
                    }
                }

            }
        }
#endif

//#define BLOCK_V_PUSH 1
#ifdef BLOCK_V_PUSH
	    for (int j = 0; j < BlkRowsTF; j++) {
		    for (int i = 0; i < BlkColsTF; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMaskTF, BlkColsTF, i, j) ) {
					continue;
				}

                // Handle block border
                int myBlk_Rows = ((j+1)*BlkHeight < RowsTF) ? BlkHeight : (RowsTF - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;

			            if (GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX) {

			                // North neighbour (u, v-1)
			                if (v > 0) {
						        if (GetPix(pExcessFlowTF, ColsTF, u, v) > 0 && GetPix(pHeightTF, ColsTF, u, v-1) == GetPix(pHeightTF, ColsTF, u, v) - 1) {
			                        flow = min( GetPix(pNorthCapTF, ColsTF, u, v), GetPix(pExcessFlowTF, ColsTF, u, v) );
						            if (flow != 0) {
									    AddPix(pExcessFlowTF, ColsTF, u, v, (short) -flow);
			                            AddPix(pExcessFlowTF, ColsTF, u, v-1, flow);
						                AddPix(pNorthCapTF, ColsTF, u, v, (short) -flow);
									    AddPix(pSouthCapTF, ColsTF, u, v-1, flow);
										nPush++;
						            }
			                    }
						    }

			                // South neighbour (u, v+1)
							if (v < RowsTF-1) {
								if (GetPix(pExcessFlowTF, ColsTF, u, v) > 0 && GetPix(pHeightTF, ColsTF, u, v+1) == GetPix(pHeightTF, ColsTF, u, v) - 1) {
									flow = min( GetPix(pSouthCapTF, ColsTF, u, v), GetPix(pExcessFlowTF, ColsTF, u, v) );

			                        if (flow != 0) {
						                AddPix(pExcessFlowTF, ColsTF, u, v, (short) -flow);
									    AddPix(pExcessFlowTF, ColsTF, u, v+1, flow);
			                            AddPix(pSouthCapTF, ColsTF, u, v, (short) -flow);
						                AddPix(pNorthCapTF, ColsTF, u, v+1, flow);
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
int CModel_GraphCut_Horizontal_PushTF(short * pExcessFlowTF, 
									HEIGHT_TYPE * pHeightTF, 
									short * pWestCapTF, 
									short * pEastCapTF, 
									unsigned char * pBlockMaskTF,
									int RowsTF, 
									int ColsTF, 
									int BlkRowsTF, 
									int BlkColsTF, 
									int BlkWidth, 
									int BlkHeight)
{
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);
    short flow;
    int rtn = 0;
	int wPush = 0;
	int ePush = 0;

#define SCANLINE_H_PUSH 1
#ifdef SCANLINE_H_PUSH
    for (int y = 0; y < RowsTF; y++)
        for (int x = 0; x < ColsTF; x++) {

			// Skip if (x,y) is an OOB pixel.
			if (IsPixelOOB(x, y, RowsTF, ColsTF))
				continue;

            if (GetPix(pHeightTF, ColsTF, x, y) < HEIGHT_MAX) {

                // West neighbour at (x-1, y-1)
                if (!IsPixelOOB(x-1, y-1, RowsTF, ColsTF)) {		// Check if (x-1,y-1) is an OOB pixel.
                    if ( GetPix(pExcessFlowTF, ColsTF, x, y) > 0 && GetPix(pHeightTF, ColsTF, x-1, y-1) == GetPix(pHeightTF, ColsTF, x, y) - 1 ) {
                        flow = min( GetPix(pWestCapTF, ColsTF, x, y), GetPix(pExcessFlowTF, ColsTF, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, x, y, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, x-1, y-1, flow);
                            AddPix(pWestCapTF, ColsTF, x, y, (short) -flow);
                            AddPix(pEastCapTF, ColsTF, x-1, y-1, flow);
							wPush++;
                        }
                    }
                }

                // East neighbour at (x+1, y+1)
                if (!IsPixelOOB(x+1, y+1, RowsTF, ColsTF)) {		// Check if (x+1,y+1) is an OOB pixel.
                    if (GetPix(pExcessFlowTF, ColsTF, x, y) > 0 && GetPix(pHeightTF, ColsTF, x+1, y+1) == GetPix(pHeightTF, ColsTF, x, y) - 1) {
                        flow = min( GetPix(pEastCapTF, ColsTF, x, y), GetPix(pExcessFlowTF, ColsTF, x, y) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, x, y, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, x+1, y+1, flow);
                            AddPix(pEastCapTF, ColsTF, x, y, (short) -flow);
                            AddPix(pWestCapTF, ColsTF, x+1, y+1, flow);
							ePush++;
                        }
                    }
                }

            }
        }
#endif

//#define BLOCK_H_PUSH 1
#ifdef BLOCK_H_PUSH
	    for (int j = 0; j < BlkRows; j++) {
		    for (int i = 0; i < BlkCols; i++) {

				// Skip inactive block
				if ( !GetPix(pBlockMaskTF, BlkColsTF, i, j) ) {
					continue;
				}

                // Handle block border
                int myBlk_Rows = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);

                for (int y = 0; y < myBlk_Rows; y++) {
                    int v = j * BlkHeight + y;

                    // Vertical
                    for (int x = 0; x < BlkWidth; x++) {
                        int u = i * BlkWidth + x;

			            if (GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX) {

							// West neighbour (u-1, v)
                            if (u > 0) {
                                if ( GetPix(pExcessFlowTF, ColsTF, u, v) > 0 && GetPix(pHeightTF, ColsTF, u-1, v) == GetPix(pHeightTF, ColsTF, u, v) - 1 ) {
                                    flow = min( GetPix(pWestCapTF, ColsTF, u, v), GetPix(pExcessFlowTF, ColsTF, u, v) );
                                    if (flow != 0) {
                                        AddPix(pExcessFlowTF, ColsTF, u, v, (short) -flow);
                                        AddPix(pExcessFlowTF, ColsTF, u-1, v, flow);
                                        AddPix(pWestCapTF, ColsTF, u, v, (short) -flow);
                                        AddPix(pEastCapTF, ColsTF, u-1, v, flow);
										wPush++;
                                    }
                                }
                            }

                            // East neighbour (u+1, v)
                            if (u < Cols-1) {
                                if (GetPix(pExcessFlowTF, ColsTF, u, v) > 0 && GetPix(pHeightTF, ColsTF, u+1, v) == GetPix(pHeightTF, ColsTF, u, v) - 1) {
                                    flow = min( GetPix(pEastCapTF, ColsTF, u, v), GetPix(pExcessFlowTF, ColsTF, u, v) );
                                    if (flow != 0) {
                                        AddPix(pExcessFlowTF, ColsTF, u, v, (short) -flow);
                                        AddPix(pExcessFlowTF, ColsTF, u+1, v, flow);
                                        AddPix(pEastCapTF, ColsTF, u, v, (short) -flow);
                                        AddPix(pWestCapTF, ColsTF, u+1, v, flow);
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

	rtn = GetActivePixelsTF(pExcessFlowTF, pHeightTF, RowsTF, ColsTF);
    return rtn;
}


/////////////////// Graph Cut entry function MWYI//////////////////////////////////////////////////////////////////
int CModel_Push_Relabel_TF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, short * pWestCapTF, short * pNorthCapTF, 
						short * pEastCapTF, short * pSouthCapTF, unsigned char * pBlockMaskTF, unsigned char * pOutput, 
						int RowsTF, int ColsTF, int BlkRowsTF, int BlkColsTF, int BlkWidth, int BlkHeight, int nRatio)
{
    int iter = 1;
    int ActivePixels;

	CreateBlockMaskTF(pExcessFlowTF, pHeightTF, RowsTF, ColsTF, pBlockMaskTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);

    do {
#ifdef _DEBUG
		printf("%3d, ", iter);
#endif
        // Relabel all active nodes
        if (iter % nRatio) {
            CModel_GraphCut_RelabelTF(pExcessFlowTF, pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF, pBlockMaskTF, RowsTF, ColsTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);

        } else {    // Global relabel 
            CModel_Global_RelabelTF(pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF, pBlockMaskTF, RowsTF, ColsTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);
			CreateBlockMaskTF(pExcessFlowTF, pHeightTF, RowsTF, ColsTF, pBlockMaskTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);
        }
//		CreateBlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
		char fn[128];
        sprintf(fn, ".\\Output\\_%d_CPU_TF_Height.%dx%d.Y8", iter, sizeof(HEIGHT_TYPE)*ColsTF, RowsTF);
        Dump2File(fn, (unsigned char *) pHeightTF, RowsTF*ColsTF*sizeof(HEIGHT_TYPE));

		// Reverse TF for file
		unsigned char TempBlockMask[11][20];
		for (int j = 0; j < 11; j++)
			for (int i = 0; i < 20; i++) {
				int u, v;
				XYtoTF(i, j, u, v);
				TempBlockMask[j][i] = *(pBlockMaskTF + BlkColsTF * v + u);
			}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_BlockMask.%dx%d.Y8", iter, BlkColsTF, BlkRowsTF-BlkColsTF);
        Dump2File(fn, (unsigned char *) TempBlockMask, (BlkRowsTF - BlkColsTF) * BlkColsTF);

//        sprintf(fn, ".\\Output\\_%d_CPU_TF_BlockMask.%dx%d.Y8", iter, BlkColsTF, BlkRowsTF);
//        Dump2File(fn, (unsigned char *) pBlockMaskTF, BlkRowsTF*BlkColsTF);
#endif
        // Push all active nodes in all directions
        CModel_GraphCut_Vertical_PushTF(pExcessFlowTF, pHeightTF, pNorthCapTF, pSouthCapTF, pBlockMaskTF, RowsTF, ColsTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);
        ActivePixels = CModel_GraphCut_Horizontal_PushTF(pExcessFlowTF, pHeightTF, pWestCapTF, pEastCapTF, pBlockMaskTF, RowsTF, ColsTF, BlkRowsTF, BlkColsTF, BlkWidth, BlkHeight);

#ifdef _DEBUG
		printf("Active pixels = %6d\n", ActivePixels);
#endif
    } while (ActivePixels > 0 && ++iter < 256);
	
    HEIGHT_TYPE HEIGHT_MAX = min(RowsTF * ColsTF, TYPE_MAX-1);

    // Write pOutput Transformed back
	int Rows = RowsTF - ColsTF;
	int Cols = ColsTF;

    for (int y = 0; y < Rows; y++) {
        for (int x = 0; x < Cols; x++) {
			int u, v;
			RestoreTFCoordinate(x, y, u, v);			// (u,v) is the tranformed address of (x,y)

            unsigned char val = GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX ? 0 : 255;
            PutPix(pOutput, Cols, x, y, val);
        }
	}

	return iter;
}

