.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

==========================
Tutorial 8. Using CmBuffer
==========================

In the previous examples, we have been using CmSurface to store image data. In this tutorial, we show the usage of CmBuffer to store generic data, and use oword-block read and write to access such data. The following is what we do in
`the nbody example
<nbody_SLM_release.cpp>`_

Host Program: Set up CmBuffers before enqueue
=============================================

.. literalinclude:: nbody_SLM_release.cpp
   :language: c++
   :lines: 199-230

Host Program: read output buffer after enqueue
==============================================

.. literalinclude:: nbody_SLM_release.cpp
   :language: c++
   :lines: 309-317

Kernel Program: buffer reads and writes
=======================================

Here we only show the use of block reads and block writes from single
address. C for Metal also provide various scattered reads and writes using
a vector of addresses.

Read example
------------

.. literalinclude:: nbody_SLM_release_genx.cpp
   :language: c++
   :lines: 157-161

Write example
-------------

.. code-block:: c++
.. literalinclude:: nbody_SLM_release_genx.cpp
   :language: c++
   :lines: 209-213

