.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

====================
C for Metal Tutorial
====================

Revision |release|

Primary Author(s): Gang Chen and Guei-Yuan Lueh (with sources from current and past C for Metal members)

Contributor(s): Kaiyu Chen, Michael Liao, Fang Liu, Wei Pan, Yuting Yang

Special thanks to our previous teammates in UK, David Stuttard, Tim Renouf, Tim Corringham, and Stephen Thomas.

Intro: Comparing C for Metal with CUDA/OpenCL
=============================================

Similarities
------------

* Program for GPU acceleration: host program on CPU; kernel program on GPU.

* Host API provided in C++.  GPU kernel programming in a subset of C++.

Differences
-----------

* Explicit SIMD-programming model. Allow varying SIMD width.

* One C for Metal thread is equivalent to one CUDA warp or one OCL subgroup, operates on a block of pixels instead of one pixel.

  * access the entire vector register file instead of one-lane of the register file. Enable developers to achieve the most efficient register usage.

* Predefined vector and matrix type.

  * Natural representation of media data

  * Parallelism expressed through vector and matrix operations

* Expose GEN hardware media-acceleration functions

* Cm is good for applications that

  * Need cross SIMD-lane operations

  * Need mixed SIMD width

  * Need to change data layout (e.g., transpose)

Contents
========

.. toctree::
   :titlesonly:
   :glob:

   cmtutorial*

Legal Notices and Disclaimers
=============================

INFORMATION IN THIS DOCUMENT IS PROVIDED IN CONNECTION WITH INTEL® PRODUCTS. NO LICENSE,
EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE, TO ANY INTELLECTUAL PROPERTY RIGHTS IS
GRANTED BY THIS DOCUMENT. EXCEPT AS PROVIDED IN INTEL'S TERMS AND CONDITIONS OF SALE FOR
SUCH PRODUCTS, INTEL ASSUMES NO LIABILITY WHATSOEVER, AND INTEL DISCLAIMS ANY EXPRESS OR
IMPLIED WARRANTY, RELATING TO SALE AND/OR USE OF INTEL PRODUCTS INCLUDING LIABILITY OR
WARRANTIES RELATING TO FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABILITY, OR
INFRINGEMENT OF ANY PATENT, COPYRIGHT OR OTHER INTELLECTUAL PROPERTY RIGHT.
UNLESS OTHERWISE AGREED IN WRITING BY INTEL, THE INTEL PRODUCTS ARE NOT DESIGNED NOR
INTENDED FOR ANY APPLICATION IN WHICH THE FAILURE OF THE INTEL PRODUCT COULD CREATE A
SITUATION WHERE PERSONAL INJURY OR DEATH MAY OCCUR.
Intel may make changes to specifications and product descriptions at any time, without notice. Designers
must not rely on the absence or characteristics of any features or instructions marked "reserved" or
"undefined." Intel reserves these for future definition and shall have no responsibility whatsoever for
conflicts or incompatibilities arising from future changes to them. The information here is subject to
change without notice. Do not finalize a design with this information.
The products described in this document may contain design defects or errors known as errata which may
cause the product to deviate from published specifications. Current characterized errata are available on
request.
Contact your local Intel sales office or your distributor to obtain the latest specifications and before
placing your product order.
Copies of documents which have an order number and are referenced in this document, or other Intel
literature, may be obtained by calling 1-800-548-4725, or by visiting Intel's Web Site.
Intel processor numbers are not a measure of performance. Processor numbers differentiate features
within each processor family, not across different processor families. See
http://www.intel.com/products/processor_number for details.
This document contains information on products in the design phase of development.
BunnyPeople, Celeron, Celeron Inside, Centrino, Centrino Atom, Centrino Atom Inside, Centrino Inside,
Centrino logo, Core Inside, FlashFile, i960, InstantIP, Intel, Intel logo, Intel386, Intel486, IntelDX2,
IntelDX4, IntelSX2, Intel Atom, Intel Atom Inside, Intel Core, Intel Inside, Intel Inside logo, Intel. Leap
ahead., Intel. Leap ahead. logo, Intel NetBurst, Intel NetMerge, Intel NetStructure, Intel SingleDriver, Intel
SpeedStep, Intel StrataFlash, Intel Viiv, Intel vPro, Intel XScale, Itanium, Itanium Inside, MCS, MMX, Oplus,
OverDrive, PDCharm, Pentium, Pentium Inside, skoool, Sound Mark, The Journey Inside, Viiv Inside, vPro
Inside, VTune, Xeon, and Xeon Inside are trademarks of Intel Corporation in the U.S. and other countries.
* Other names and brands may be claimed as the property of others.

Copyright (C) 2009-2017, Intel Corporation. All rights reserved.


