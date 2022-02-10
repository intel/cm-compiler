.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

Tutorial 2. Basic Kernel Programming
====================================

This is the kernel program for the linear filtering. In this example, you
can see how to get thread-ids when using media-walker. Also how to use
the C for Metal matrix type, and the select operation.

.. literalinclude:: linear_walker1_genx.cpp
   :lines: 23-93

