.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

==================================
Tutorial 7. Using Printf in Kernel
==================================

Developers can use printf in C for Metal kernel programs for debugging purpose.
Using printf also requires some corresponding setup in the host
program.

Host-Program: initialize the printf-buffer before enqueue
=========================================================

.. code-block:: c++

    /// allocate the buffer for printf
    device->InitPrintBuffer();

Host Program: flush the printf buffer after enqueue
===================================================

.. code-block:: c++

    // This function prints the message on the screen which is dumped
    // by kernel. It should be called after the task being finished.
    // The order of printf output is not deterministic due to thread
    // scheduling and the fact that different threads may be interleaved.
    // To distinguish which thread the printf string comes from, it is
    // better to print the thread id as the first value on the string.
    // Alternatively you could always put the printf inside an if
    // which limits the printf to a given thread. If one task has more
    //  than one kernels call printf(), their outputs could mix together
    device->FlushPrintBuffer();

Printf in kernel
================

.. code-block:: c++

    int h_pos = get_thread_origin_x();
    int v_pos = get_thread_origin_y();
    int thread_id = v_pos * width + h_pos;
    printf("%d Simple printf testing\n", thread_id);
    if (thread_id == 5) {
      printf("hello CM\n");
    }

The usage of printf is demonstrated in `hello_world
<hello_world_genx.cpp>`_

Notice that you also can redirect print message to a file:

.. code-block:: c++

   CM_RT_API INT CmDevice_RT::FlushPrintBufferIntoFile(const char* filename);


