#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <math.h>

#include "common_C_model.h"
#include "Graph_Cut_host.h"

////////////////////////////////////////////////////
void inline XYtoTF(int x, int y, int &u, int &v) 
{
	u = x;
	v = y + x;
}

void inline TFtoXY(int u, int v, int &x, int &y) 
{
	x = u;
	y = v - u;
}

////////////////////////////////////////////////////
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
//                PutPix(pEastCapTF, ColsTF, u-1, v, w);
                PutPix(pEastCapTF, ColsTF, u-1, v-1, w);
            } else {    // x = Cols-1
                PutPix(pWestCapTF, ColsTF, u, v, w);
//                PutPix(pEastCapTF, ColsTF, u-1, v, w);
                PutPix(pEastCapTF, ColsTF, u-1, v-1, w);
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
        }
	}
}
/*
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
bool inline Is_TF_Pixel_OOB(int xTF, int yTF, int RowsTF, int ColsTF)
{
	// TFtoXY(int u, int v, int &x, int &y) 
	int Rows = RowsTF - ColsTF;		
	int Cols = ColsTF;
	int x = xTF;
	int y = yTF - xTF;

	if (x < 0 || x >= Cols || y < x || y >= Rows)
		return true;
	else
		return false;
}
*/
/////////////////////////////////////////////////////////////////////
bool inline Is_Pixel_OOB(int x, int y, int Rows, int Cols)
{
	if (x < 0 || x >= Cols || y < 0 || y >= Rows)
		return true;
	else
		return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// All input data are in shift-down format.
void CreateBlockMaskTF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, int Rows, int Cols, 
						unsigned char * pBlockMaskTF, int BlkRows, int BlkCols, int BlkWidth, int BlkHeight)
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

	HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
	
	memset(pBlockMaskTF, 0, BlkRowsTF * BlkColsTF);

	// (i,j) is global block coordinate before TF.
	// (iTF,jTF) is global block coordinate after TF.
	// (x,y) is local pixel coordinate within a block before TF.
	// (u,v) is global pixel coordinate before TF.
	// (uTF,vTF) is global pixel coordinate after TF.

	// Loop through all blocks
    for (int j = 0; j < BlkRows; j++) {
        for (int i = 0; i < BlkCols; i++) {
			int iTF, jTF;
			XYtoTF(i, j, iTF, jTF);	

            // Row OOB check
            int myBlkHeight = ((j+1)*BlkHeight < Rows) ? BlkHeight : (Rows - j*BlkHeight);
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
}

////////////////////////////////////////////////////////////////////////////////
int GetActivePixelsTF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, int Rows, int Cols)
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;

    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
	int ActivePixels = 0;
    short CurExcessFlow;
    HEIGHT_TYPE CurHeight;

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

			int xTF, yTF;
			XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

	        CurHeight = *(pHeightTF + yTF * ColsTF + xTF);
		    if (CurHeight < HEIGHT_MAX) {
			    CurExcessFlow = *(pExcessFlowTF + yTF * ColsTF + xTF);
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
                                int Rows,
                                int Cols,
								int BlkRows, 
								int BlkCols, 
								int BlkWidth, 
								int BlkHeight)   
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

	short r = 0;
	int v;

#define SCANLINE_RELABEL 1
#ifdef SCANLINE_RELABEL

// Scan line based Relabel
    HEIGHT_TYPE NewHeight, temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

			int xTF, yTF;
			XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

            // Check for active node
            float CurExcessFlow = GetPix(pExcessFlowTF, ColsTF, xTF, yTF);
            HEIGHT_TYPE CurHeight = GetPix(pHeightTF, ColsTF, xTF, yTF);
        
            if (CurExcessFlow > 0 && CurHeight < HEIGHT_MAX) {  

                NewHeight = HEIGHT_MAX;

                // North neighbour
				v = (Is_Pixel_OOB(x, y-1, Rows, Cols)) ? yTF : yTF-1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, xTF, v) + 1;
                if (r = GetPix(pNorthCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // South neighbour
				v = (Is_Pixel_OOB(x, y+1, Rows, Cols)) ? yTF : yTF+1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, xTF, v) + 1;
                if (r = GetPix(pSouthCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // West neighbour 
				if (Is_Pixel_OOB(x-1, y, Rows, Cols))   // Check if west neighbour is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, xTF, yTF) + 1;
				else	
	                temp = GetPix(pHeightTF, ColsTF, xTF-1, yTF-1) + 1;	// at (xTF-1, yTF-1)
                if (r = GetPix(pWestCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight)
                    NewHeight = temp;
               
                // East neighbour
				if (Is_Pixel_OOB(x+1, y, Rows, Cols))   // Check if east neighbour is an OOB pixel.
					temp = GetPix(pHeightTF, ColsTF, xTF, yTF) + 1;	
				else
					temp = GetPix(pHeightTF, ColsTF, xTF+1, yTF+1) + 1;	// at (xTF+1, yTF+1)
                if (r = GetPix(pEastCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) 
                    NewHeight = temp;

                // Update the current Height to new Height
				PutPix(pHeightTF, ColsTF, xTF, yTF, NewHeight);
            }
        }
#endif

//#define BLOCK_RELABEL 1
#ifdef BLOCK_RELABEL
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
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
                            int Rows,
                            int Cols,
							int BlkRows, 
							int BlkCols, 
							int BlkWidth, 
							int BlkHeight)   
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

	HEIGHT_TYPE NewHeight;
    HEIGHT_TYPE temp; 
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
	HEIGHT_TYPE NewH = 0;
	HEIGHT_TYPE NewHeightBlocks = HEIGHT_MAX;
	int PrevCount;
	int iter = 0;
	int v;

    // Init active pixel Height
	for (int y = 0; y < Rows; y++) {
        for (int x = 0; x < Cols; x++) {

			int xTF, yTF;
			XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

            if (GetPix(pHeightTF, ColsTF, xTF, yTF) != 0) 
                PutPix(pHeightTF, ColsTF, xTF, yTF, HEIGHT_MAX);
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
        for (int y = 0; y < Rows; y++) {
            for (int x = 0; x < Cols; x++) {

				int xTF, yTF;
				XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

                NewHeight = GetPix(pHeightTF, ColsTF, xTF, yTF);

                // North neighbour
				v = (Is_Pixel_OOB(x, y-1, Rows, Cols)) ? yTF : yTF-1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, xTF, v) + 1;
                if (GetPix(pNorthCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // South neighbour
				v = (Is_Pixel_OOB(x, y+1, Rows, Cols)) ? yTF : yTF+1;	// Check if (x,y-1) is an OOB pixel.
                temp = GetPix(pHeightTF, ColsTF, xTF, v) + 1;
                if (GetPix(pSouthCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // West neighbour
				if (Is_Pixel_OOB(x-1, y, Rows, Cols))
					temp = GetPix(pHeightTF, ColsTF, xTF, yTF) + 1;
				else 
	                temp = GetPix(pHeightTF, ColsTF, xTF-1, yTF-1) + 1;  // at (xTF-1, yTF-1)
                if (GetPix(pWestCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // East neighbour
				if (Is_Pixel_OOB(x+1, y, Rows, Cols))
					temp = GetPix(pHeightTF, ColsTF, xTF, yTF) + 1;	
				else
					temp = GetPix(pHeightTF, ColsTF, xTF+1, yTF+1) + 1;	
                if (GetPix(pEastCapTF, ColsTF, xTF, yTF) > 0 && temp < NewHeight) {
                    NewHeight = temp;
					NewH = 1; 
				}
                // Update the current Height to new Height
				if (NewH) {
                    PutPix(pHeightTF, ColsTF, xTF, yTF, NewHeight);
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
									int Rows, 
									int Cols, 
									int BlkRows, 
									int BlkCols, 
									int BlkWidth, 
									int BlkHeight)
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
    short flow;
	int nPush = 0;
	int sPush = 0;

#define SCANLINE_V_PUSH 1
#ifdef SCANLINE_V_PUSH
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

			int xTF, yTF;
			XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

            if (GetPix(pHeightTF, ColsTF, xTF, yTF) < HEIGHT_MAX) {

                // North neighbour (x, y-1)
                if (!Is_Pixel_OOB(x, y-1, Rows, Cols))		// Check if (x,y-1) is an OOB pixel.
				{
                    if (GetPix(pExcessFlowTF, ColsTF, xTF, yTF) > 0 && GetPix(pHeightTF, ColsTF, xTF, yTF-1) == GetPix(pHeightTF, ColsTF, xTF, yTF) - 1) {
                        flow = min( GetPix(pNorthCapTF, ColsTF, xTF, yTF), GetPix(pExcessFlowTF, ColsTF, xTF, yTF) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF-1, flow);
                            AddPix(pNorthCapTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pSouthCapTF, ColsTF, xTF, yTF-1, flow);
							nPush++;
                        }
                    }
                }

                // South neighbour (x, y+1)
                if (!Is_Pixel_OOB(x, y+1, Rows, Cols))		// Check if (x,y+1) is an OOB pixel.
				{
                    if (GetPix(pExcessFlowTF, ColsTF, xTF, yTF) > 0 && GetPix(pHeightTF, ColsTF, xTF, yTF+1) == GetPix(pHeightTF, ColsTF, xTF, yTF) - 1) {
                        flow = min( GetPix(pSouthCapTF, ColsTF, xTF, yTF), GetPix(pExcessFlowTF, ColsTF, xTF, yTF) );

                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF+1, flow);
                            AddPix(pSouthCapTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pNorthCapTF, ColsTF, xTF, yTF+1, flow);
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
									int Rows, 
									int Cols, 
									int BlkRows, 
									int BlkCols, 
									int BlkWidth, 
									int BlkHeight)
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols;
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);
    short flow;
    int rtn = 0;
	int wPush = 0;
	int ePush = 0;

#define SCANLINE_H_PUSH 1
#ifdef SCANLINE_H_PUSH
    for (int y = 0; y < Rows; y++)
        for (int x = 0; x < Cols; x++) {

			int xTF, yTF;
			XYtoTF(x, y, xTF, yTF);			// (u,v) is the tranformed address of (x,y)

            if (GetPix(pHeightTF, ColsTF, xTF, yTF) < HEIGHT_MAX) {

                // West neighbour
                if (!Is_Pixel_OOB(x-1, y, Rows, Cols)) {
                    if ( GetPix(pExcessFlowTF, ColsTF, xTF, yTF) > 0 && GetPix(pHeightTF, ColsTF, xTF-1, yTF-1) == GetPix(pHeightTF, ColsTF, xTF, yTF) - 1 ) {
                        flow = min( GetPix(pWestCapTF, ColsTF, xTF, yTF), GetPix(pExcessFlowTF, ColsTF, xTF, yTF) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, xTF-1, yTF-1, flow);
                            AddPix(pWestCapTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pEastCapTF, ColsTF, xTF-1, yTF-1, flow);
							wPush++;
                        }
                    }
                }

                // East neighbour at (x+1, y+1)
                if (!Is_Pixel_OOB(x+1, y, Rows, Cols)) {
                    if (GetPix(pExcessFlowTF, ColsTF, xTF, yTF) > 0 && GetPix(pHeightTF, ColsTF, xTF+1, yTF+1) == GetPix(pHeightTF, ColsTF, xTF, yTF) - 1) {
                        flow = min( GetPix(pEastCapTF, ColsTF, xTF, yTF), GetPix(pExcessFlowTF, ColsTF, xTF, yTF) );
                        if (flow != 0) {
                            AddPix(pExcessFlowTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pExcessFlowTF, ColsTF, xTF+1, yTF+1, flow);
                            AddPix(pEastCapTF, ColsTF, xTF, yTF, (short) -flow);
                            AddPix(pWestCapTF, ColsTF, xTF+1, yTF+1, flow);
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

	rtn = GetActivePixelsTF(pExcessFlowTF, pHeightTF, Rows, Cols);
    return rtn;
}


/////////////////// Graph Cut entry function MWYI//////////////////////////////////////////////////////////////////
int CModel_Push_Relabel_TF(short * pExcessFlowTF, HEIGHT_TYPE * pHeightTF, short * pWestCapTF, short * pNorthCapTF, 
						short * pEastCapTF, short * pSouthCapTF, unsigned char * pBlockMaskTF, unsigned char * pOutput, 
						int Rows, int Cols,  int BlkRows, int BlkCols, int BlkWidth, int BlkHeight, int nRatio)
{
	int RowsTF = Rows + Cols;
	int ColsTF = Cols; 
	int BlkRowsTF = BlkRows + BlkCols;
	int BlkColsTF = BlkCols;

    int iter = 1;
    int ActivePixels;

	CreateBlockMaskTF(pExcessFlowTF, pHeightTF, Rows, Cols, pBlockMaskTF, BlkRows, BlkCols, BlkWidth, BlkHeight);

    do {
#ifdef _DEBUG
		printf("%3d, ", iter);
#endif
        // Relabel all active nodes
        if (iter % nRatio) {
            CModel_GraphCut_RelabelTF(pExcessFlowTF, pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF, pBlockMaskTF, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
        } else {    // Global relabel 
            CModel_Global_RelabelTF(pHeightTF, pWestCapTF, pNorthCapTF, pEastCapTF, pSouthCapTF, pBlockMaskTF, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);
			CreateBlockMaskTF(pExcessFlowTF, pHeightTF, Rows, Cols, pBlockMaskTF, BlkRows, BlkCols, BlkWidth, BlkHeight);
        }

#ifdef _DEBUG
		char fn[128];

		// Reverse TF for file
		int pitch = sizeof(HEIGHT_TYPE)*Cols;
		unsigned short * pWord = (unsigned short *) malloc(Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pHeightTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_Height.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pExcessFlowTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_ExcessFlow1.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pNorthCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_NorthCap1.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pSouthCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_SouthCap1.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

//		free(pWord);

//        sprintf(fn, ".\\Output\\_%d_CPU_Height.%dx%d.Y8", iter, sizeof(HEIGHT_TYPE)*ColsTF, RowsTF);
//        Dump2File(fn, (unsigned char *) pHeightTF, RowsTF*ColsTF*sizeof(HEIGHT_TYPE));

		// Reverse TF for file
		unsigned char * pTemp = (unsigned char *) malloc(BlkRows * BlkCols);
		for (int y = 0; y < BlkRows; y++)
			for (int x = 0; x < BlkCols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pTemp + BlkCols * y + x) = *(pBlockMaskTF + BlkColsTF * v + u);
			}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_BlockMask.%dx%d.Y8", iter, BlkCols, BlkRows);
        Dump2File(fn, (unsigned char *) pTemp, BlkRows * BlkCols);
		free(pTemp);

//        sprintf(fn, ".\\Output\\_%d_CPU_BlockMask.%dx%d.Y8", iter, BlkColsTF, BlkRowsTF);
//        Dump2File(fn, (unsigned char *) pBlockMaskTF, BlkRowsTF*BlkColsTF);
#endif
        // Push all active nodes in all directions
        CModel_GraphCut_Vertical_PushTF(pExcessFlowTF, pHeightTF, pNorthCapTF, pSouthCapTF, pBlockMaskTF, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
		// Reverse TF for dump 
		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pExcessFlowTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_ExcessFlow2.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pNorthCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_NorthCap2.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pSouthCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_SouthCap2.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pWestCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_WestCap2.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pEastCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_EastCap2.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

#endif

		ActivePixels = CModel_GraphCut_Horizontal_PushTF(pExcessFlowTF, pHeightTF, pWestCapTF, pEastCapTF, pBlockMaskTF, Rows, Cols, BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pExcessFlowTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_ExcessFlow3.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pWestCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_WestCap3.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		for (int y = 0; y < Rows; y++) {
			for (int x = 0; x < Cols; x++) {
				int u, v;
				XYtoTF(x, y, u, v);
				*(pWord + Cols * y + x) = *(pEastCapTF + Cols * v + u);
			}
		}
        sprintf(fn, ".\\Output\\_%d_CPU_TF_EastCap3.%dx%d.Y8", iter, pitch, Rows);
        Dump2File(fn, (unsigned char *) pWord, Rows * pitch);

		free(pWord);


		printf("Active pixels = %6d\n", ActivePixels);
#endif
    } while (ActivePixels > 0 && ++iter < 256);
	
    HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX-1);

    // Write pOutput Transformed back
//	int Rows = RowsTF - ColsTF;
//	int Cols = ColsTF;
    for (int y = 0; y < Rows; y++) {
        for (int x = 0; x < Cols; x++) {
			int u, v;
			XYtoTF(x, y, u, v);			// (u,v) is the tranformed address of (x,y)
            unsigned char val = GetPix(pHeightTF, ColsTF, u, v) < HEIGHT_MAX ? 0 : 255;
            PutPix(pOutput, Cols, x, y, val);
        }
	}

	return iter;
}

