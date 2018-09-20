/*
 * Copyright (c) 2018, Intel Corporation
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
#include <math.h>
#include <stdio.h>

#include "Graph_Cut_host.h"
#include "common_C_model.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Graph Cut (Push and Relabel)
void CreateBlockMask(short *pExcessFlow, HEIGHT_TYPE *pHeight, int ImgRows,
                     int ImgCols, unsigned char *pBlockMask, int BlkRows,
                     int BlkCols, int BlkWidth, int BlkHeight) {
  HEIGHT_TYPE HEIGHT_MAX = min(ImgRows * ImgCols, TYPE_MAX - 1);

  memset(pBlockMask, 0, BlkRows * BlkCols);

  // Loop through all blocks
  for (int j = 0; j < BlkRows; j++) {
    for (int i = 0; i < BlkCols; i++) {

      // Row OOB check
      int myBlkHeight = ((j + 1) * BlkHeight < ImgRows)
                            ? BlkHeight
                            : (ImgRows - j * BlkHeight);
      int active = 0;

      // Copy a block
      for (int y = 0; y < myBlkHeight; y++) {
        int v = j * BlkHeight + y;
        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;
          if (GetPix(pExcessFlow, ImgCols, u, v) > -100 &&
              GetPix(pHeight, ImgCols, u, v) < HEIGHT_MAX) {
            PutPix(pBlockMask, BlkCols, i, j, (unsigned char)255);
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
void PushRelabel_Init(short *pWeight, short *pHCap, short *pVCap, int Rows,
                      int Cols, short *pExcessFlow, HEIGHT_TYPE *pHeight,
                      short *pWestCap, short *pNorthCap, short *pEastCap,
                      short *pSouthCap) {
  short w;
  HEIGHT_TYPE HEIGHT_MAX =
      min(Rows * Cols, TYPE_MAX - 1); // Must have -1 to prevent overflow, some
                                      // calculation needs HEIGHT_MAX+1

  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {

      // Set excess flow
      w = GetPix(pWeight, Cols, x, y);
      PutPix(pExcessFlow, Cols, x, y,
             w); // w>0 if a cap from source exists, w<0 if cap from sink
                 // exists. ExcessFlow = w.

      PutPix(pHeight, Cols, x, y, (HEIGHT_TYPE)0);

      // Set west and east caps
      w = GetPix(pHCap, Cols, x, y);
      if (x == 0) {
        PutPix(pWestCap, Cols, x, y, (short)0);
      } else if (x < Cols - 1) {
        PutPix(pWestCap, Cols, x, y, w);
        PutPix(pEastCap, Cols, x - 1, y, w);
      } else { // x = Cols-1
        PutPix(pWestCap, Cols, x, y, w);
        PutPix(pEastCap, Cols, x - 1, y, w);
        PutPix(pEastCap, Cols, x, y, (short)0);
      }

      // Set north and south caps
      w = GetPix(pVCap, Cols, x, y);
      if (y == 0) {
        PutPix(pNorthCap, Cols, x, y, (short)0);
      } else if (y < Rows - 1) {
        PutPix(pNorthCap, Cols, x, y, w);
        PutPix(pSouthCap, Cols, x, y - 1, w);
      } else {
        PutPix(pNorthCap, Cols, x, y, w);
        PutPix(pSouthCap, Cols, x, y - 1, w);
        PutPix(pSouthCap, Cols, x, y, (short)0);
      }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void Gap_Relable(HEIGHT_TYPE *pHeight, int Rows, int Cols) {
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
  unsigned int hist[1024];
  HEIGHT_TYPE h;
  int gap;

  memset(hist, 0, 1024 * sizeof(int));

  // Build histogram
  for (int y = 1; y < Rows - 1; y++) { // within the frame
    for (int x = 1; x < Cols - 1; x++) {
      h = GetPix(pHeight, Cols, x, y);
      if (h < 1024)
        hist[h]++;
    }
  }

  for (gap = 0; gap < 1024; gap++) {
    if (!hist[gap])
      break;
  }

  // gap is the gap value
  for (int y = 1; y < Rows - 1; y++) { // within the frame
    for (int x = 1; x < Cols - 1; x++) {
      h = GetPix(pHeight, Cols, x, y);
      if (h > gap && h < HEIGHT_MAX)
        PutPix(pHeight, Cols, x, y, HEIGHT_MAX);
    }
  }
}

///////////////////////////////// Relabel func
/////////////////////////////////////////////
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
// Key point: height is updated after all directions are checked and assign to
// the lowest neighbor height + 1. In kernel, 8 pixels can lock step process and
// upedate height.  The next row will use the updated row.
void CModel_GraphCut_Relabel(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                             short *pWestCap, short *pNorthCap, short *pEastCap,
                             short *pSouthCap, unsigned char *pBlockMask,
                             int Rows, int Cols, int BlkRows, int BlkCols,
                             int BlkWidth, int BlkHeight) {
  short r = 0;

#define SCANLINE_RELABEL 1
#ifdef SCANLINE_RELABEL

  // Scan line based Relabel
  HEIGHT_TYPE NewHeight, temp;
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

  bool rtn = false;

  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {

      // Check for active node
      float CurExcessFlow = GetPix(pExcessFlow, Cols, x, y);
      HEIGHT_TYPE CurHeight = GetPix(pHeight, Cols, x, y);

      if (CurExcessFlow > 0 && CurHeight < HEIGHT_MAX) {

        NewHeight = HEIGHT_MAX;

        // North neighbour
        temp = GetPix(pHeight, Cols, x, (y - 1 < 0) ? 0 : y - 1) + 1;
        if (r = GetPix(pNorthCap, Cols, x, y) > 0 && temp < NewHeight)
          NewHeight = temp;

        // South neighbour
        temp = GetPix(pHeight, Cols, x, (y + 1 >= Rows) ? Rows - 1 : y + 1) + 1;
        if (r = GetPix(pSouthCap, Cols, x, y) > 0 && temp < NewHeight)
          NewHeight = temp;

        // West neighbour
        temp = GetPix(pHeight, Cols, (x - 1 < 0) ? 0 : x - 1, y) + 1;
        if (r = GetPix(pWestCap, Cols, x, y) > 0 && temp < NewHeight)
          NewHeight = temp;

        // East neighbour
        temp = GetPix(pHeight, Cols, (x + 1 >= Cols) ? Cols - 1 : x + 1, y) + 1;
        if (r = GetPix(pEastCap, Cols, x, y) > 0 && temp < NewHeight)
          NewHeight = temp;

        // Update the current Height to new Height
        //  if (CurHeight < NewHeight) {
        PutPix(pHeight, Cols, x, y, NewHeight);
        //      rtn = true;
        //  }
      }
    }
#endif

//#define BLOCK_RELABEL 1
#ifdef BLOCK_RELABEL
  HEIGHT_TYPE temp;
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
  //    bool rtn = false;

  for (int j = 0; j < BlkRows; j++) {
    for (int i = 0; i < BlkCols; i++) {

      // Skip inactive block
      if (!GetPix(pBlockMask, BlkCols, i, j)) {
        continue;
      }

      // Per block
      int myBlk_Rows =
          ((j + 1) * BlkHeight < Rows) ? BlkHeight : (Rows - j * BlkHeight);

      for (int y = 0; y < myBlk_Rows; y++) {

        int v = j * BlkHeight + y;
        HEIGHT_TYPE NewHeight[16];
        unsigned short mask[16];

        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;
          NewHeight[x] = HEIGHT_MAX;
          mask[x] = (GetPix(pExcessFlow, Cols, u, v) > 0 &&
                     GetPix(pHeight, Cols, u, v) < HEIGHT_MAX)
                        ? 1
                        : 0;
        }

        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;
          if (mask[x]) {
            // North neighbour
            temp = GetPix(pHeight, Cols, u, (v - 1 < 0) ? 0 : v - 1) + 1;
            if (GetPix(pNorthCap, Cols, u, v) > 0 && temp < NewHeight[x])
              NewHeight[x] = temp;

            // South neighbour
            temp =
                GetPix(pHeight, Cols, u, (v + 1 >= Rows) ? Rows - 1 : v + 1) +
                1;
            if (GetPix(pSouthCap, Cols, u, v) > 0 && temp < NewHeight[x])
              NewHeight[x] = temp;
          }
        }

        // Now NewHeight[x] has results of north and south.

        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;
          if (mask[x]) {
            // West neighbour
            temp = GetPix(pHeight, Cols, (u - 1 < 0) ? 0 : u - 1, v) + 1;
            if (GetPix(pWestCap, Cols, u, v) > 0 && temp < NewHeight[x]) {
              NewHeight[x] = temp;
            }

            // East neighbour
            temp =
                GetPix(pHeight, Cols, (u + 1 >= Cols) ? Cols - 1 : u + 1, v) +
                1;
            if (GetPix(pEastCap, Cols, u, v) > 0 && temp < NewHeight[x])
              NewHeight[x] = temp;

            // Update Height so the next pixel to the right will use this new
            // value
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
void CModel_Global_Relabel( // float * pExcessFlow,
    HEIGHT_TYPE *pHeight, short *pWestCap, short *pNorthCap, short *pEastCap,
    short *pSouthCap, unsigned char *pBlockMask, int Rows, int Cols,
    int BlkRows, int BlkCols, int BlkWidth, int BlkHeight) {
  HEIGHT_TYPE temp;
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

  HEIGHT_TYPE NewH = 0;
  HEIGHT_TYPE NewHeightBlocks = HEIGHT_MAX;
  int PrevCount;
  int iter = 0;

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
      if (!GetPix(pBlockMask, BlkCols, i, j)) {
        continue;
      }

      // Border block handling
      int myBlk_Rows =
          ((j + 1) * BlkHeight < Rows) ? BlkHeight : (Rows - j * BlkHeight);

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
    PrevCount = NewHeightBlocks;
    NewHeightBlocks = 0;
    /*
    // This version processes each pixel in all directions before updating
    pHeight. for (int y = 0; y < Rows; y++) for (int x = 0; x < Cols; x++) {

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
        if (!GetPix(pBlockMask, BlkCols, i, j)) {
          continue;
        }

        // Per block
        int myBlk_Rows =
            ((j + 1) * BlkHeight < Rows) ? BlkHeight : (Rows - j * BlkHeight);

        for (int y = 0; y < myBlk_Rows; y++) {
          int v = j * BlkHeight + y;

          // Vertical
          for (int x = 0; x < BlkWidth; x++) {
            int u = i * BlkWidth + x;
            // North neighbour
            temp = GetPix(pHeight, Cols, u, (v - 1 < 0) ? 0 : v - 1) + 1;
            if (GetPix(pNorthCap, Cols, u, v) > 0 &&
                temp < GetPix(pHeight, Cols, u, v)) {
              PutPix(pHeight, Cols, u, v, temp);
              NewH = 1;
            }
            // South neighbour
            temp =
                GetPix(pHeight, Cols, u, (v + 1 >= Rows) ? Rows - 1 : v + 1) +
                1;
            if (GetPix(pSouthCap, Cols, u, v) > 0 &&
                temp < GetPix(pHeight, Cols, u, v)) {
              PutPix(pHeight, Cols, u, v, temp);
              NewH = 1;
            }
          }

          // Horizontal
          for (int x = 0; x < BlkWidth; x++) {
            int u = i * BlkWidth + x;
            // West neighbour
            temp = GetPix(pHeight, Cols, (u - 1 < 0) ? 0 : u - 1, v) + 1;
            if (GetPix(pWestCap, Cols, u, v) > 0 &&
                temp < GetPix(pHeight, Cols, u, v)) {
              PutPix(pHeight, Cols, u, v, temp);
              NewH = 1;
            }
            // East neighbour
            temp =
                GetPix(pHeight, Cols, (u + 1 >= Cols) ? Cols - 1 : u + 1, v) +
                1;
            if (GetPix(pEastCap, Cols, u, v) > 0 &&
                temp < GetPix(pHeight, Cols, u, v)) {
              PutPix(pHeight, Cols, u, v, temp);
              NewH = 1;
            }
          } // x
        }   // y

        // Count
        if (NewH) {
          NewHeightBlocks++; // Increase active pixel count
          NewH = 0;
        }
      } // i
    }   // j

#ifdef _DEBUG
    printf("\tGlobal Relabel new height blocks: %d\n", NewHeightBlocks);
#endif

  } while (/*PrevCount > NewHeightCount &&*/ NewHeightBlocks > 0 &&
           ++iter < 32);
}

////////////////////////////////////////////////////////////////////////////////
int GetActivePixels(short *pExcessFlow, HEIGHT_TYPE *pHeight, int Rows,
                    int Cols) {
  int ActivePixels = 0;

  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

  short CurExcessFlow;
  HEIGHT_TYPE CurHeight;

  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {
      CurHeight = *(pHeight + y * Cols + x);
      if (CurHeight < HEIGHT_MAX) {
        CurExcessFlow = *(pExcessFlow + y * Cols + x);
        if (CurExcessFlow > 0)
          ActivePixels++;
      }
    }

  return ActivePixels;
}

////////////////////////////////////////////////////////////////////////////////
int GetActiveBlocks(unsigned char *pBlockMask, int BlkRows, int BlkCols) {
  int ActiveBlocks = 0;
  for (int i = 0; i < BlkRows * BlkCols; i++) {
    if (*(pBlockMask + i))
      ActiveBlocks++;
  }
  return ActiveBlocks;
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
void CModel_GraphCut_Vertical_Push(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                                   short *pNorthCap, short *pSouthCap,
                                   unsigned char *pBlockMask, int Rows,
                                   int Cols, int BlkRows, int BlkCols,
                                   int BlkWidth, int BlkHeight) {
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
  short flow;
  int nPush = 0;
  int sPush = 0;

#define SCANLINE_V_PUSH 1
#ifdef SCANLINE_V_PUSH
  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {

      if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {

        // North neighbour (x, y-1)
        if (y > 0) {
          if (GetPix(pExcessFlow, Cols, x, y) > 0 &&
              GetPix(pHeight, Cols, x, y - 1) ==
                  GetPix(pHeight, Cols, x, y) - 1) {
            flow = min(GetPix(pNorthCap, Cols, x, y),
                       GetPix(pExcessFlow, Cols, x, y));
            if (flow != 0) {
              AddPix(pExcessFlow, Cols, x, y, (short)-flow);
              AddPix(pExcessFlow, Cols, x, y - 1, flow);
              AddPix(pNorthCap, Cols, x, y, (short)-flow);
              AddPix(pSouthCap, Cols, x, y - 1, flow);
              nPush++;
            }
          }
        }

        // South neighbour (x, y+1)
        if (y < Rows - 1) {
          if (GetPix(pExcessFlow, Cols, x, y) > 0 &&
              GetPix(pHeight, Cols, x, y + 1) ==
                  GetPix(pHeight, Cols, x, y) - 1) {
            flow = min(GetPix(pSouthCap, Cols, x, y),
                       GetPix(pExcessFlow, Cols, x, y));

            if (flow != 0) {
              AddPix(pExcessFlow, Cols, x, y, (short)-flow);
              AddPix(pExcessFlow, Cols, x, y + 1, flow);
              AddPix(pSouthCap, Cols, x, y, (short)-flow);
              AddPix(pNorthCap, Cols, x, y + 1, flow);
              sPush++;
            }
          }
        }
      }
    }
#endif

//#define BLOCK_V_PUSH 1
#ifdef BLOCK_V_PUSH
  for (int j = 0; j < BlkRows; j++) {
    for (int i = 0; i < BlkCols; i++) {

      // Skip inactive block
      if (!GetPix(pBlockMask, BlkCols, i, j)) {
        continue;
      }

      // Handle block border
      int myBlk_Rows =
          ((j + 1) * BlkHeight < Rows) ? BlkHeight : (Rows - j * BlkHeight);

      for (int y = 0; y < myBlk_Rows; y++) {
        int v = j * BlkHeight + y;

        // Vertical
        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;

          if (GetPix(pHeight, Cols, u, v) < HEIGHT_MAX) {

            // North neighbour (u, v-1)
            if (v > 0) {
              if (GetPix(pExcessFlow, Cols, u, v) > 0 &&
                  GetPix(pHeight, Cols, u, v - 1) ==
                      GetPix(pHeight, Cols, u, v) - 1) {
                flow = min(GetPix(pNorthCap, Cols, u, v),
                           GetPix(pExcessFlow, Cols, u, v));
                if (flow != 0) {
                  AddPix(pExcessFlow, Cols, u, v, (short)-flow);
                  AddPix(pExcessFlow, Cols, u, v - 1, flow);
                  AddPix(pNorthCap, Cols, u, v, (short)-flow);
                  AddPix(pSouthCap, Cols, u, v - 1, flow);
                  nPush++;
                }
              }
            }

            // South neighbour (u, v+1)
            if (v < Rows - 1) {
              if (GetPix(pExcessFlow, Cols, u, v) > 0 &&
                  GetPix(pHeight, Cols, u, v + 1) ==
                      GetPix(pHeight, Cols, u, v) - 1) {
                flow = min(GetPix(pSouthCap, Cols, u, v),
                           GetPix(pExcessFlow, Cols, u, v));

                if (flow != 0) {
                  AddPix(pExcessFlow, Cols, u, v, (short)-flow);
                  AddPix(pExcessFlow, Cols, u, v + 1, flow);
                  AddPix(pSouthCap, Cols, u, v, (short)-flow);
                  AddPix(pNorthCap, Cols, u, v + 1, flow);
                  sPush++;
                }
              }
            }
          }

        } // x
      }   // y
    }     // i
  }       // j

#endif

#ifdef _DEBUG
  printf("nPush = %6d, sPush = %6d, ", nPush, sPush);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CModel_GraphCut_Horizontal_Push(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                                    short *pWestCap, short *pEastCap,
                                    unsigned char *pBlockMask, int Rows,
                                    int Cols, int BlkRows, int BlkCols,
                                    int BlkWidth, int BlkHeight) {
  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);
  short flow;
  int rtn = 0;
  int wPush = 0;
  int ePush = 0;

#define SCANLINE_H_PUSH 1
#ifdef SCANLINE_H_PUSH
  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {

      if (GetPix(pHeight, Cols, x, y) < HEIGHT_MAX) {

        // West neighbour (x-1, y)
        if (x > 0) {
          if (GetPix(pExcessFlow, Cols, x, y) > 0 &&
              GetPix(pHeight, Cols, x - 1, y) ==
                  GetPix(pHeight, Cols, x, y) - 1) {
            flow = min(GetPix(pWestCap, Cols, x, y),
                       GetPix(pExcessFlow, Cols, x, y));
            if (flow != 0) {
              AddPix(pExcessFlow, Cols, x, y, (short)-flow);
              AddPix(pExcessFlow, Cols, x - 1, y, flow);
              AddPix(pWestCap, Cols, x, y, (short)-flow);
              AddPix(pEastCap, Cols, x - 1, y, flow);
              wPush++;
            }
          }
        }

        // East neighbour (x+1, y)
        if (x < Cols - 1) {
          if (GetPix(pExcessFlow, Cols, x, y) > 0 &&
              GetPix(pHeight, Cols, x + 1, y) ==
                  GetPix(pHeight, Cols, x, y) - 1) {
            flow = min(GetPix(pEastCap, Cols, x, y),
                       GetPix(pExcessFlow, Cols, x, y));
            if (flow != 0) {
              AddPix(pExcessFlow, Cols, x, y, (short)-flow);
              AddPix(pExcessFlow, Cols, x + 1, y, flow);
              AddPix(pEastCap, Cols, x, y, (short)-flow);
              AddPix(pWestCap, Cols, x + 1, y, flow);
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
      if (!GetPix(pBlockMask, BlkCols, i, j)) {
        continue;
      }

      // Handle block border
      int myBlk_Rows =
          ((j + 1) * BlkHeight < Rows) ? BlkHeight : (Rows - j * BlkHeight);

      for (int y = 0; y < myBlk_Rows; y++) {
        int v = j * BlkHeight + y;

        // Vertical
        for (int x = 0; x < BlkWidth; x++) {
          int u = i * BlkWidth + x;

          if (GetPix(pHeight, Cols, u, v) < HEIGHT_MAX) {

            // West neighbour (u-1, v)
            if (u > 0) {
              if (GetPix(pExcessFlow, Cols, u, v) > 0 &&
                  GetPix(pHeight, Cols, u - 1, v) ==
                      GetPix(pHeight, Cols, u, v) - 1) {
                flow = min(GetPix(pWestCap, Cols, u, v),
                           GetPix(pExcessFlow, Cols, u, v));
                if (flow != 0) {
                  AddPix(pExcessFlow, Cols, u, v, (short)-flow);
                  AddPix(pExcessFlow, Cols, u - 1, v, flow);
                  AddPix(pWestCap, Cols, u, v, (short)-flow);
                  AddPix(pEastCap, Cols, u - 1, v, flow);
                  wPush++;
                }
              }
            }

            // East neighbour (u+1, v)
            if (u < Cols - 1) {
              if (GetPix(pExcessFlow, Cols, u, v) > 0 &&
                  GetPix(pHeight, Cols, u + 1, v) ==
                      GetPix(pHeight, Cols, u, v) - 1) {
                flow = min(GetPix(pEastCap, Cols, u, v),
                           GetPix(pExcessFlow, Cols, u, v));
                if (flow != 0) {
                  AddPix(pExcessFlow, Cols, u, v, (short)-flow);
                  AddPix(pExcessFlow, Cols, u + 1, v, flow);
                  AddPix(pEastCap, Cols, u, v, (short)-flow);
                  AddPix(pWestCap, Cols, u + 1, v, flow);
                  ePush++;
                }
              }
            }
          }

        } // x
      }   // y
    }     // i
  }       // j

#endif

#ifdef _DEBUG
  printf("wPush = %6d, ePush = %6d, ", wPush, ePush);
#endif

  rtn = GetActivePixels(pExcessFlow, pHeight, Rows, Cols);
  return rtn;
}

/////////////////// Graph Cut entry function
/////////////////////////////////////////////////////////////////////
int CModel_Push_Relabel(short *pExcessFlow, HEIGHT_TYPE *pHeight,
                        short *pWestCap, short *pNorthCap, short *pEastCap,
                        short *pSouthCap, unsigned char *pBlockMask,
                        unsigned char *pOutput, int Rows, int Cols, int BlkRows,
                        int BlkCols, int BlkWidth, int BlkHeight, int nRatio) {
  int iter = 1;
  int ActivePixels;

  CreateBlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows,
                  BlkCols, BlkWidth, BlkHeight);

  do {
#ifdef _DEBUG
    printf("%3d, ", iter);
#endif
    // Relabel all active nodes
    if (iter % nRatio) {
      CModel_GraphCut_Relabel(pExcessFlow, pHeight, pWestCap, pNorthCap,
                              pEastCap, pSouthCap, pBlockMask, Rows, Cols,
                              BlkRows, BlkCols, BlkWidth, BlkHeight);

    } else { // Global relabel
      CModel_Global_Relabel(pHeight, pWestCap, pNorthCap, pEastCap, pSouthCap,
                            pBlockMask, Rows, Cols, BlkRows, BlkCols, BlkWidth,
                            BlkHeight);
      CreateBlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask, BlkRows,
                      BlkCols, BlkWidth, BlkHeight);
    }
    //        CreateBlockMask(pExcessFlow, pHeight, Rows, Cols, pBlockMask,
    //        BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
    char fn[128];
    sprintf(fn, ".\\Output\\_%d_CPU_Height.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pHeight, Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_ExcessFlow1.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pExcessFlow,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_NorthCap1.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pNorthCap,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_SouthCap1.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pSouthCap,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_BlockMask.%dx%d.Y8", iter, BlkCols,
            BlkRows);
    Dump2File(fn, (unsigned char *)pBlockMask, BlkRows * BlkCols);
#endif
    // Push all active nodes in all directions
    CModel_GraphCut_Vertical_Push(pExcessFlow, pHeight, pNorthCap, pSouthCap,
                                  pBlockMask, Rows, Cols, BlkRows, BlkCols,
                                  BlkWidth, BlkHeight);

#ifdef _DEBUG
    sprintf(fn, ".\\Output\\_%d_CPU_ExcessFlow2.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pExcessFlow,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_NorthCap2.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pNorthCap,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_SouthCap2.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pSouthCap,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_WestCap2.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pWestCap, Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_EastCap2.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pEastCap, Rows * Cols * sizeof(HEIGHT_TYPE));

#endif
    ActivePixels = CModel_GraphCut_Horizontal_Push(
        pExcessFlow, pHeight, pWestCap, pEastCap, pBlockMask, Rows, Cols,
        BlkRows, BlkCols, BlkWidth, BlkHeight);

#ifdef _DEBUG
    sprintf(fn, ".\\Output\\_%d_CPU_ExcessFlow3.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pExcessFlow,
              Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_WestCap3.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pWestCap, Rows * Cols * sizeof(HEIGHT_TYPE));

    sprintf(fn, ".\\Output\\_%d_CPU_EastCap3.%dx%d.Y8", iter,
            sizeof(HEIGHT_TYPE) * Cols, Rows);
    Dump2File(fn, (unsigned char *)pEastCap, Rows * Cols * sizeof(HEIGHT_TYPE));

    printf("Active pixels = %6d\n", ActivePixels);
#endif
    //    } while (ActivePixels > 0 && ++iter < 256);
  } while (ActivePixels > 0 && ++iter < 100000);

  HEIGHT_TYPE HEIGHT_MAX = min(Rows * Cols, TYPE_MAX - 1);

  // Write pOutput
  for (int y = 0; y < Rows; y++)
    for (int x = 0; x < Cols; x++) {
      unsigned char val = GetPix(pHeight, Cols, x, y) < HEIGHT_MAX ? 0 : 255;
      PutPix(pOutput, Cols, x, y, val);
    }

  return iter;
}
