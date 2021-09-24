#!/bin/bash

#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

ROOT_PATH=`dirname $(readlink -f $BASH_SOURCE)`
export CSDK_DIR=$ROOT_PATH
export CM_INCLUDE_DIR=$CSDK_DIR/usr/include
export OPENCL_VENDOR_PATH=$CSDK_DIR/etc/OpenCL/vendors
export PATH=$CSDK_DIR/usr/bin:$CSDK_DIR/usr/local/bin:$PATH
export LD_LIBRARY_PATH=$CSDK_DIR/usr/lib:$CSDK_DIR/usr/lib/x86_64-linux-gnu/:$CSDK_DIR/usr/local/lib/intel-opencl:$CSDK_DIR/usr/local/lib

echo "Updated environment:"
echo "CSDK_DIR=$CSDK_DIR"
echo "CM_INCLUDE_DIR=$CM_INCLUDE_DIR"
echo "OPENCL_VENDOR_PATH=$OPENCL_VENDOR_PATH"
echo "PATH=$PATH"
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"