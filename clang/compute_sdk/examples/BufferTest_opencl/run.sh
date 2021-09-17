#!/bin/bash

#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2020-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

# Compile and run example using commands that are documented in README.txt
OUT=tmp.$(date +%y%m%d_%H%M)
mkdir -p ${OUT}
OUT=`realpath ${OUT}`
PLATFORM=SKL

function help()
{
  echo "Usage:"
  echo "  ./run.sh [-h|-?|--help]"
  echo "  ./run.sh [SKL|TGLLP|ICLLP]"
  echo " Where SKL stands for Skylake, TGLLP stands for Tiger Lake LP, ICLLP stands for Ice Lake LP"
}

if [[ "$1" == "-h" || "$1" == "--help" || "$1" == "-?" ]]
then
  help
  exit 0
fi
if [[ "$1" == "" || "$1" == "SKL" ]]
then
  PLATFORM=SKL
  PLATFORM_EXTENSION=skl
elif [[ "$1" == "TGLLP" ]]
then
  PLATFORM=TGLLP
  PLATFORM_EXTENSION=tgllp
elif [[ "$1" == "ICLLP" ]]
then
  PLATFORM=ICLLP
  PLATFORM_EXTENSION=icllp
else
  echo "Unknown platform"
  help
  exit 0
fi

echo "Verifying environment variables"
if [ -z "${CSDK_DIR}" ];
then
  echo "CSDK_DIR environment variable is not set - did you run 'source setupenv.sh' ?"
  exit 255
fi
if [ -z "${OPENCL_VENDOR_PATH}" ];
then
  echo "OPENCL_VENDOR_PATH environment variable is not set - did you run 'source setupenv.sh' ?"
  exit 255
fi

echo "Verifying installed packages"
for package in g++ clinfo ocl-icd-opencl-dev ocl-icd-libopencl1 cmake
do
  dpkg -l $package >/dev/null 2>&1
  if [ $? -ne 0 ];
  then
      echo "PACKAGE: ${package} is not installed"
      exit 250
  fi
done

HAVE_GPU=1
clinfo | grep 'Device Name' > ${OUT}/clinfo.out 2>&1
grep -E 'Intel\(R\).*Graphics' ${OUT}/clinfo.out > /dev/null
if [ $? -ne 0 ];
then
  echo "Did not detect Intel GPU. Running only compilation"
  HAVE_GPU=0
fi

pushd ${OUT} >/dev/null

echo -n "Building tests... "
cmake .. -DINSTALL_DIR=${OUT}/test_bin -DCMAKE_CONFIG_TYPE=Release >cmake.out 2>&1
if [ $? -ne 0 ];
then
  echo "FAILED on cmake execution"
  cat ${OUT}/cmake.out
  exit 1
fi

cmake --build . --target install -j >build.out 2>&1
if [ $? -ne 0 ];
then
  echo "FAILED on build"
  cat ${OUT}/build.out
  exit 1
fi
echo "PASSED"

if [ ${HAVE_GPU} -ne 0 ];
then
  echo "Running tests on ${PLATFORM}"
  pushd ${OUT}/test_bin >/dev/null

  echo -n "Running test using binary kernel... "
  ./BufferTest_bin >${OUT}/test_bin.out 2>&1
  if [ $? -ne 0 ];
  then
    echo "FAILED"
    cat ${OUT}/test_bin.out
    exit 1
  fi
  echo "PASSED"

  echo -n "Running test using SPIRV kernel... "
  ./BufferTest_spv >${OUT}/test_spv.out 2>&1
  if [ $? -ne 0 ];
  then
    echo "FAILED"
    cat ${OUT}/test_spv.out
    exit 1
  fi
  echo "PASSED"

  echo -n "Running test using kernel source... "
  ./BufferTest_src >${OUT}/test_src.out 2>&1
  if [ $? -ne 0 ];
  then
    echo "FAILED"
    cat ${OUT}/test_src.out
    exit 1
  fi
  echo "PASSED"

  popd >/dev/null
fi
popd >/dev/null
#############################

rm -rf ${OUT}
exit 0