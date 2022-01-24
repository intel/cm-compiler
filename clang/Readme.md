<!---======================= begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ==========================-->

# Intel(R) C for Metal Compiler

## Introduction

The Intel(R) C for Metal Compiler (CMC) is an open source compiler that implements C for Metal (CM) programming language.
CM is GPU kernel programming language for Intel Graphics (including ARC and Xe).

This document is a starting guide for setting up the development environment, building and using CMC.

Public packages can be downloaded from https://01.org/c-for-metal-development-package. No special access rights or registration required.

## Introduction

CMC compiles CM sources to SPIRV for use with CM (legacy), OpenCL and L0 runtimes.

It is optionally used as a FE wrapper library by

* IGC graphics compiler - https://github.com/intel/intel-graphics-compiler

To enable offline compilation from CM sources to OpenCL or L0 binary, cmc tool requires libocloc as a runtime dependency.

## License

The Intel(R) C for Metal Compiler is distributed under the MIT license.

You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

## Dependencies

### Source code

* LLVM Project - https://github.com/llvm/llvm-project
* VC intrinsics - https://github.com/intel/vc-intrinsics
* SPIRV LLVM Translator - https://github.com/KhronosGroup/SPIRV-LLVM-Translator.git

### Tools

To build libraries:

* CMake - https://cmake.org/ - 3.13.4 or later
* Python - https://www.python.org/ - 2.7 or later
* C++ compiler - anything that can compile LLVM

## Building

CMC can be built in two major modes: in-tree and external.

### In-tree build

Download sources for LLVM tree structure:

```shell
cd $ROOT
git clone https://github.com/intel/cm-compiler.git -b cmc_monorepo_80 llvm-project
git clone https://github.com/intel/vc-intrinsics.git llvm-project/llvm/projects/vc-intrinsics
git clone https://github.com/KhronosGroup/SPIRV-LLVM-Translator.git -b llvm_release_80 llvm-project/llvm/projects/SPIRV-LLVM-Translator
```

Now configure and make:

```shell
cd $BUILD
cmake -DLLVM_ENABLE_Z3_SOLVER=OFF -DCLANG_ANALYZER_ENABLE_Z3_SOLVER=OFF -DCMAKE_INSTALL_PREFIX=$INSTALL -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="" $ROOT/llvm-project/llvm
make install
```

### External build

For external build consider you already have LLVM v8, SPIRV-Translator v8 and VC-Intrinsics installed and install folders are known.

Then just configure, specifying cmake install folders for all components:

```shell
cd $CLANG_BUILD
cmake -DCMAKE_INSTALL_PREFIX=$INSTALL -DLLVM_CMAKE_PATH=$LLVM_INSTALL_CMAKE -DLLVMGenXIntrinsics_DIR=$INTR_INSTALL_CMAKE -DSPIRV_TRANSLATOR_DIR=$SPIRV_INSTALL_CMAKE $CLANG_SOURCES
make install
```

### Running the compiler

You may run compiler to generate SPIRV from CM sources:

```shell
$INSTALL/bin/cmc -march=SKL test.genx.cc -fcmocl -emit-spirv -o test.genx.spv
```

If you have libigc and libocloc in path, you can run cmc generate to OpenCL binary:

```shell
$INSTALL/bin/cmc -march=SKL test.genx.cc -binary-format=ocl -o test.genx.bin
```

Or alternatively:

```shell
$INSTALL/bin/cmc -march=SKL test.genx.cc -fcmocl -o test.genx.bin
```

Or you may generate L0 binary:

```shell
$INSTALL/bin/cmc -march=SKL test.genx.cc -binary-format=ze -o test.genx.zebin
```

### Documentation

Now documentation is not part of github, so please refer to docs inside package:

https://01.org/c-for-metal-development-package

## Testing

CMC project contains conformance testing provided by LIT tests. To run them go to clang build folder and run

```shell
$ make check-cmc
```

## How to provide feedback

Please submit an issue using native github.com interface:
https://github.com/intel/cm-compiler/issues.

## How to contribute

Create a pull request on github.com with your patch. A maintainer
will contact you if there are questions or concerns.
