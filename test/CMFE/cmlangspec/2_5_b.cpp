void foo()
{
  vector<float, 8> f;
  vector<int, 8> i;
  // ...
  f = i;                    // implicit type conversion
  f = vector<short, 8>(i);  // explicit type conversion
  // ...
}

// RUN: %cmc -emit-llvm -- %s 2>&1 | FileCheck -allow-empty --implicit-check-not error %s
