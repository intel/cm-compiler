.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

================================================
Tutorial 6. Shared Local Memory and Thread Group
================================================

C for Metal also allows users to use shared-local-memory (SLM) that can be shared among
a group of threads. On GEN, SLM is carved out of the level-3 cache, and
reconfigured to be 16-way banked. A group of threads that share SLM
will be dispatched to the same half-slice. The maximum size for SLM is 64KB.

SLM is useful when you want data-sharing among a group of threads. Because it
has more banks than L3 and is user-program controlled. It can be more efficient
than L3-data-cache for scattered read and write. The following are the typical
steps for using SLM and thread-grouping in CM.

These code are extracted from `nbody_SLM_release
<nbody_SLM_release.cpp>`_.

Host Program: CreateThreadGroupSpace
====================================

One important note: CreateThreadGroupSpace will put GPU thread-dispatching into
GPGPU mode, which is different from the media-Walker mode, therefore the thread dependence setting, which is associated with the media-walker, are not available when thread groups are in use.

.. literalinclude:: nbody_SLM_release.cpp
   :language: c++
   :lines: 166-198

Host Program: EnqueueWithGroup
==============================

.. literalinclude:: nbody_SLM_release.cpp
   :language: c++
   :lines: 285-296

Kernel Program
==============

Several builtin function worth attention in this programs are ``cm_slm_init``, ``cm_slm_alloc``, and ``cm_slm_load``.

.. literalinclude:: nbody_SLM_release_genx.cpp
   :language: c++
   :lines: 227-287

