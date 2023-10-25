#=========================== begin_copyright_notice ============================
#
# Copyright (C) 2014-2021 Intel Corporation
#
# SPDX-License-Identifier: MIT
#
#============================ end_copyright_notice =============================

# This directory contains tests of error and warning diagnostics for which we
# expect the compiler to return non-zero return codes. We only care about the
# return code from FileCheck so we set the pipefail option to False here.
config.pipefail = False
