.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

======================================
Tutorial 3. Enqueuing Multiple Kernels
======================================

You may have noticed that Enqueue function takes an array of kernels.
So you can enqueue multiple kernels.

Enqueuing two independent kernels
=================================

The following code-block is extracted from `multi_kernels
<../../../../../test/open_examples/multi_kernels/multi_kernels.cpp>`_.

In this example, two kernels are launched independently (no specific execution order). The linear kernel processes the top-half of the image, and the sepia kernel processes the bottom-half of the image.

First, create the linear kernel, notice the thread-count and thread-space are
only for the half of the image.

.. literalinclude:: ../../../test/open_examples/multi_kernels/multi_kernels.cpp
   :language: c++
   :lines: 150-178

Second, create the sepia kernel, notice the thread-count and thread-space are
also for the half of the image. Also the image height is passed into the
sepia kernel. Sepia kernel is modified to process the bottom-half of the
image.

.. literalinclude:: ../../../test/open_examples/multi_kernels/multi_kernels.cpp
   :language: c++
   :lines: 189-209

Finally add both kernels to the kernel-array, and enqueue.

.. literalinclude:: ../../../test/open_examples/multi_kernels/multi_kernels.cpp
   :language: c++
   :lines: 224-251

Enqueuing two kernels with sync
===============================

The following code-block is extracted from `BufferTest_EnqueueWithSync
<BufferTest_EnqueueWithSync1.cpp>`_.

In order to force an execution order among multiple kernels in the kernel
array, you need to add synchronization.

.. literalinclude:: BufferTest_EnqueueWithSync1.cpp
   :language: c++
   :lines: 140-195
