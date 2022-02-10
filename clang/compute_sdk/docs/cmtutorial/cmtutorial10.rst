.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

=========================================
Tutorial 10. Event-Driven Synchronization
=========================================

In `linear_up_walker
<linear_up_walker1.cpp>`_
, since the output surface is created in user-provided system memory,
there is no need to call ReadSurface to access those data after enqueue.
However, we still need to know when the GPU task is finished, and the output
is complete. An event-driven synchronization method is used here

.. literalinclude:: linear_up_walker1.cpp
   :language: c++
   :lines: 242-246

