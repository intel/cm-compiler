#!/bin/bash

#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2021 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT
#============================ end_copyright_notice =============================

ROOT_PATH=`dirname $(readlink -f $BASH_SOURCE)`
export CSDK_DIR=$ROOT_PATH
export CM_INCLUDE_DIR=$CSDK_DIR/usr/include
export OPENCL_VENDOR_PATH=$CSDK_DIR/etc/OpenCL/vendors
export PATH=$CSDK_DIR/usr/bin:$PATH
export LD_LIBRARY_PATH=$CSDK_DIR/usr/lib:$CSDK_DIR/usr/local/lib/intel-opencl:$CSDK_DIR/usr/local/lib

echo "Updated environment:"
echo "CSDK_DIR=$CSDK_DIR"
echo "CM_INCLUDE_DIR=$CM_INCLUDE_DIR"
echo "OPENCL_VENDOR_PATH=$OPENCL_VENDOR_PATH"
echo "PATH=$PATH"
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"