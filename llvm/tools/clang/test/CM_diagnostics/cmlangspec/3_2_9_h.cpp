#include <cm/cm.h>

_GENX_ void test1()
{
  vector<int, 65> inVector = 0;
  vector<int, 8>  outVector = 0;
  vector<int, 12> tempVector = 0;
  vector<ushort, 12>  idx;

  inVector.select<2,3>(4) = 19; // Here: inVector(4) = 19, inVector(7) = 19
  //Now: inVector = {0,0,0,19,0,0,0,19,0...}

  outVector = inVector.select<8,1>(0);
  //Now: outVector = {0,0,0,19,0,0,0,19}
 
  idx = 7; idx(0) = 2; idx(2) = 4; idx(3) = 10;
  // Now: idx = {2,7,4,10,7,7,7,7,7,7,7,7}

  tempVector = inVector.iselect(idx);
  // Now: tempVector = {0,19,19,0,19,19,0,0,0,0}
  
  tempVector = tempVector * 2;
  // Now: tempVector = {0,38,38,0,38,38, ... }

  outVector.select<8,1>(0) += tempVector.select<8,1>(1);
  // Now: outVector = {38,38,0,57,38,38,57,38}
}

// RUN: %cmc %w 2>&1 | FileCheck -allow-empty --implicit-check-not error %w
// RUN: rm %W.isa
