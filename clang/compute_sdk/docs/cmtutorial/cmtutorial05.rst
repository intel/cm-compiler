.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

================================================
Tutorial 5. Builtin Matrix and Vector Operations
================================================

In this tutorial, we show the usage of several more C for Metal builtin operations
for matrix and vector, such as select, merge, replicate, and cm_dp4.
Refer to the C for Metal spec for more operations and their specification, such as
``format``.

.. literalinclude:: sepia2_genx.cpp
   :lines: 23-126

