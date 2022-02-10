.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

Documentation for CM
====================


CM (C for Metal) is the kernel programming language for the
Media Development Framework (MDF) for Intel HD Graphics.

This is the language and compiler documentation for CM.

Contents
========

.. toctree::
   :hidden:

   cmlangspec/cmlangspec
   cmcuserguide/cmcuserguide
   cmoclrt/cmocl

:doc:`cmlangspec/cmlangspec`

:doc:`cmcuserguide/cmcuserguide`

:doc:`cmoclrt/cmocl`

Much of the cm-llvm design documentation concerns the GenX backend and the LLVM
IR it expects, and is thus part of the LLVM documentation. Assuming you have a
standard cm-llvm source tree and you have built all the documentation,
the `GenX backend documentation starts here
<../../../../llvm/docs/_build/html/GenXIndex.html>`_.
The rest of the LLVM documentation is also accessible from there.

