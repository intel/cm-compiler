.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

==================================
Tutorial 1. Basic Host Programming
==================================

Most of the code in these tutorials are extracted from our `open examples
<../../../../../test/open_examples>`_.

In this tutorial, we are showing the sample host code that uses the CM
runtime API directly. The sample code may look a little verbose, however,
developers can come up some higher-level utility library on top of
the C for Metal runtime to make their code more concise.


Step 1. Create C for Metal Device
=================================

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 67-73

Step 2. Load Program
====================

* C for Metal compilation happens at two stages

  * Offline: cmc compiles C for Metal to virtual ISA

  * Just-In-Time: virtual ISA to target ISA

* LoadProgram: load virtual ISA into runtime

  * JIT-compilation happens during LoadProgram

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 74-94

Step 3. Create Kernel
=====================

Retrieve the target binary of a kernel from a loaded program.

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 95-103

Step 4. Create Surfaces
=======================

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 104-128

Step 5. Create Thread Space
===========================

This function sets up a hardware mechanism called media-walker for
launching threads. Media-walker generates thread-identifiers, and puts them
into thread-payloads. C for Metal kernel program can get thread-ids using
C for Metal intrinsics.

Media-walker is the preferred way of doing GEN media programming, which has
lower driver overhead (less work in preparing the commands) and
faster enqueue time.

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 129-145

Step 6. Set Kernel Arguments
============================

Kernel argument is dynamic constant for all threads. Value is logged at
the time of setting kernel arg. The size of total kernel arguments has
to be less than 256 bytes. For linear filter, we need to pass surface index
as kernel arguments.

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 146-167

Step 7. Enqueue Kernels/Launch GPU Work
=======================================

Notice that a C for Metal event is created for the enqueue call. That is
for tracking the job status.

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 168-196

Step 8. Getting Results and Execution Time
==========================================

Notice that C for Metal event is used when we read the output surface, and
it is used to query execution time. CmEvent must be destroyed by user
explicitly.

.. literalinclude:: linear_walker1.cpp
   :language: c++
   :lines: 197-237

