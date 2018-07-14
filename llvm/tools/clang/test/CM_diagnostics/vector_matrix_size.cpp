#include <cm/cm.h>

_GENX_MAIN_ void foo(SurfaceIndex idx)
{
    vector<char, 4095> v10;			// OK
    vector<char, 4096> v11;			// Size exceeds maximum
    vector<char, 5000> v12;			// Size exceeds maximum

    vector<uchar, 4095> v13;			// OK
    vector<uchar, 4096> v14;			// Size exceeds maximum
    vector<uchar, 4200> v15;			// Size exceeds maximum

    vector<short, 2047> v20;			// OK
    vector<short, 2048> v21;			// Size exceeds maximum
    vector<short, 2049> v22;			// Size exceeds maximum

    vector<ushort, 2047> v23;			// OK
    vector<ushort, 2048> v24;			// Size exceeds maximum
    vector<ushort, 2200> v25;			// Size exceeds maximum

    vector<int, 1023> v30;			// OK
    vector<int, 1024> v31;			// Size exceeds maximum
    vector<int, 1100> v32;			// Size exceeds maximum

    vector<uint, 1023> v33;			// OK
    vector<uint, 1024> v34;			// Size exceeds maximum
    vector<uint, 2048> v35;			// Size exceeds maximum

    vector<long, 1023> v40;			// OK
    vector<long, 1024> v41;			// Size exceeds maximum
    vector<long, 1100> v42;			// Size exceeds maximum

    vector<unsigned long, 1023> v43;		// OK
    vector<unsigned long, 1024> v44;		// Size exceeds maximum
    vector<unsigned long, 1025> v45;		// Size exceeds maximum

    vector<long long, 511> v50;			// OK
    vector<long long, 512> v51;			// Size exceeds maximum
    vector<long long, 520> v52;			// Size exceeds maximum

    vector<unsigned long long, 511> v53;	// OK
    vector<unsigned long long, 512> v54;	// Size exceeds maximum
    vector<unsigned long long, 1000> v55;	// Size exceeds maximum

    vector<float, 1023> v63;			// OK
    vector<float, 1024> v64;			// Size exceeds maximum
    vector<float, 2048> v65;			// Size exceeds maximum

    vector<double, 511> v70;			// OK
    vector<double, 512> v71;			// Size exceeds maximum
    vector<double, 520> v72;			// Size exceeds maximum

    matrix<char, 1, 4095> m10;			// OK
    matrix<char, 1, 4096> m11;			// Size exceeds maximum
    matrix<char, 8, 512> m12;			// Size exceeds maximum
    matrix<char, 64, 64> m13;			// Size exceeds maximum

    matrix<uchar, 4095, 1> m14;			// OK
    matrix<uchar, 4096, 1> m15;			// Size exceeds maximum
    matrix<uchar, 63,65> m16;			// OK

    matrix<short, 1, 2047> m20;			// OK
    matrix<short, 2048, 1> m21;			// Size exceeds maximum
    matrix<short, 2049, 1> m22;			// Size exceeds maximum

    matrix<ushort, 2047, 1> m23;		// OK
    matrix<ushort, 1, 2048> m24;		// Size exceeds maximum
    matrix<ushort, 2, 1024> m25;		// Size exceeds maximum

    matrix<int, 1023, 1> m30;			// OK
    matrix<int, 1024, 1> m31;			// Size exceeds maximum
    matrix<int, 11, 99> m32;			// Size exceeds maximum

    matrix<uint, 1, 1023> m33;			// OK
    matrix<uint, 1, 1024> m34;			// Size exceeds maximum
    matrix<uint, 20, 100> m35;			// Size exceeds maximum

    matrix<long, 1023, 1> m40;			// OK
    matrix<long, 1024, 1> m41;			// Size exceeds maximum
    matrix<long, 2, 512> m42;			// Size exceeds maximum

    matrix<unsigned long, 1023, 1> m43;		// OK
    matrix<unsigned long, 1024, 1> m44;		// Size exceeds maximum
    matrix<unsigned long, 1025, 1> m45;		// Size exceeds maximum

    matrix<long long, 1, 511> m50;		// OK
    matrix<long long, 1, 512> m51;		// Size exceeds maximum
    matrix<long long, 33, 16> m52;		// Size exceeds maximum

    matrix<unsigned long long, 511, 1> m53;	// OK
    matrix<unsigned long long, 512, 1> m54;	// Size exceeds maximum
    matrix<unsigned long long, 8, 65> m55;	// Size exceeds maximum

    matrix<float, 1023, 1> m60;			// OK
    matrix<float, 1024, 1> m61;			// Size exceeds maximum
    matrix<float, 1, 1023> m62;			// OK
    matrix<float, 1, 1024> m63;			// Size exceeds maximum
    matrix<float, 14, 73> m64;			// OK
    matrix<float, 14, 74> m65;			// Size exceeds maximum

    matrix<double, 1, 511> m70;			// OK
    matrix<double, 1, 512> m71;			// Size exceeds maximum
    matrix<double, 1, 520> m72;			// Size exceeds maximum
    matrix<double, 17, 31> m73;			// Size exceeds maximum

}


_GENX_ void func1(vector<char, 4095> v) {	// OK
}

_GENX_ void func2(vector<char, 4096> v) {	// Size exceeds maximum
}

_GENX_ void func3(matrix<char, 63, 65> m) {	// OK
}

_GENX_ void func4(matrix<char, 64, 64> m) {	// Size exceeds maximum
}

// CM vector and matrix types are (currently) limited to less than 8192 bytes - we generate a 
// helpful front-end error in order to avoid a more obscure error from the finalizer.
// RUN: %cmc -ferror-limit=999 %w 2>&1 | FileCheck %w

// CHECK: vector_matrix_size.cpp(27,5):  error: size of vector (8192 bytes) exceeds maximum supported size (must be less than 8192 bytes)
// CHECK: vector_matrix_size.cpp(47,5):  error: size of vector (8192 bytes) exceeds maximum supported size (must be less than 8192 bytes)
// CHECK: 2 errors generated.
