.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

=========================================================
Tutorial 9. Zero-Copy with User-Provided Surfaces
=========================================================

C for Metal also provides a way for user to create surface in system memory.
This way, CPU and GPU share the physical memory. CPU access memory
through pointer, GPU access memory through surface handle.
It is user's responsibility to avoid data race between GPU and CPU

Also be aware that media-block read/write from user-provided surface
can be slower because, unlike regular 2D surfaces which has tiled
layout, user-provided surface has a linear layout.

CreateSurface2DUP – 2D user provided memory
===========================================

`linear_up_walker
<linear_up_walker1.cpp>`_
is an example that uses 2D user provided memory.

.. literalinclude:: linear_up_walker1.cpp
   :language: c++
   :lines: 78-129

CreateBufferUP – 1D user provided memory
========================================

`vector matching example
<vcaOpSAD.cpp>`_
is an example that uses 1D user provided memory

.. literalinclude:: vcaOpSAD.cpp
   :language: c++
   :lines: 199-207

There is no difference in using those surfaces on the kernel-side.
