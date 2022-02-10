.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

Tutorial 11. Kernel Programming: Register Usage
===============================================

Here we show three different algorithms for the seemingly simple linear 
filtering.  All three algorithms use 2 1-d convolutions, horizontal then vertical, to implement the 2-d convolution. This approach minimizes computation,
however it needs some more storage for intermediate results.

.. literalinclude:: linear_walker1_genx.cpp
   :language: c++
   :lines: 94-158

.. literalinclude:: linear_walker1_genx.cpp
   :language: c++
   :lines: 160-205

.. literalinclude:: linear_walker1_genx.cpp
   :language: c++
   :lines: 206-249
