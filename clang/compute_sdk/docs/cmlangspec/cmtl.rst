.. ========================= begin_copyright_notice ============================

  Copyright (C) 2021-2026 Intel Corporation

  SPDX-License-Identifier: MIT

  =========================== end_copyright_notice =============================

.. _CMTemplateLibrary:

6 C for Metal Template Library
==============================

The C for Metal implementation also provides a template library of functions.  Inclusion of the cm/cmtl.h header file
is required to make use of these functions.

The functions defined in cm/cmtl.h provide an implementation which has been defined to be as optimal as
possible for the C for Metal language. In general, if a function exists in the C for Metal Template Library it is advisable to
use it. To some extent this will also better enable code to cope with future variations and enhancements.

The C for Metal Template Library uses a namespace to stop pollution of the global namespace as much as
possible. The namespace used for the C for Metal Template Library is cmtl. Here is an example:

.. code-block:: c++

  #include <cm/cmtl.h>

  _GENX_MAIN_ void my_func(SurfaceIndex ibuf, SurfaceIndex obuf) {
    matrix<short, HEIGHT, WIDTH> io;
    cmtl::ReadBlock<short, HEIGHT, WIDTH>(ibuf, 0, 0, io);
    ...
  }

Some functions have not been fully integrated into the compiler regression suite and are marked as such
in the reference for each function with limited testing.

6.1 I/O
-------

A set of template functions to enhance the I/O routines.

6.1.1 ReadBlock
^^^^^^^^^^^^^^^

Read a pixel block.

**Description:** Read a pixel block of size HEIGHT x WIDTH from a surface, all input are in pixel sizes. HEIGHT
and WIDTH can be any integer.

The function will deal with any border issues using border replication where necessary when the block
being read falls out of bounds of the input surface.

**Author:** Dori Eldar, Noam Teomim (ReadBlock with border handling)

**Functions:**

.. code-block:: c++

  template<typename T, uint HEIGHT, uint WIDTH> void ReadBlock(SurfaceIndex ibuf,
                                                      int h_pix_pos, int v_pix_pos,
                                                      matrix_ref<T, HEIGHT, WIDTH> block,
                                                      uint surfaceWidth = 0);
  template<typename T, uint HEIGHT, uint WIDTH> void ReadBlock(SurfaceIndex ibuf,
                                                      CmBufferAttrib buf_attrib,
                                                      int h_pix_pos, int v_pix_pos,
                                                      matrix_ref<T, HEIGHT, WIDTH> block,
                                                      uint surfaceWidth = 0);



.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - A type whose size is the same as the size of a single
       surface pixel.

       Example surface types and T values:

       * CM_SURFACE_FORMAT_A8, char
       * CM_SURFACE_FORMAT_V8U8, short
       * CM_SURFACE_FORMAT_A8R8G8B8, int
   * - HEIGHT
     - Number of rows for the matrix
   * - WIDTH
     - Number of columns
   * - ibuf
     - Surface index for input buffer
   * - h_pix_pos
     - Horizontal offset in pixels
   * - v_pix_pos
     - Vertical offset in pixels
   * - block
     - Ref to output matrix (matrix_ref only)
   * - surfaceWidth
     - Surface width in pixels. When set ReadBlock will properly
       replicate pixels across the right hand side boundary for
       non-dword aligned surfaces.

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_1_1_a.cpp
      :language: c++
      :lines: 28-39


6.1.2 WriteBlock
^^^^^^^^^^^^^^^^

Write a pixel block.

**Description:** Write a pixel block of size HEIGHT x WIDTH to a surface, all input are in pixel sizes. HEIGHT
and WIDTH can be any integer.

The function will deal correctly with any border issues

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint HEIGHT, uint WIDTH> void WriteBlock(SurfaceIndex obuf,
                                                       int h_pix_pos, int v_pix_pos,
                                                       matrix_ref<T, HEIGHT, WIDTH> block);



.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - A type whose size is the same as the size of a single
       surface pixel.

       Example surface types and T values:

       * CM_SURFACE_FORMAT_A8, char
       * CM_SURFACE_FORMAT_V8U8, short
       * CM_SURFACE_FORMAT_A8R8G8B8, int
   * - HEIGHT
     - Number of rows for the matrix
   * - WIDTH
     - Number of columns
   * - obuf
     - Surface index for output buffer
   * - h_pix_pos
     - Horizontal offset in pixels
   * - v_pix_pos
     - Vertical offset in pixels
   * - block
     - Ref to matrix to write out to surface (matrix_ref only)

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_1_2_a.cpp
      :language: c++
      :lines: 28-39

6.1.3 ReadLinear
^^^^^^^^^^^^^^^^

Read a vector from a linear buffer.

**Description:** Read a pixel vector of size WIDTH from a linear buffer. WIDTH can be any integer.

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint WIDTH> void ReadLinear(SurfaceIndex ibuf,
                                          int pix_pos,
                                          vector_ref<T, WIDTH> block);



.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - A type whose size is the same as the size of a single
       surface pixel.

       Example surface types and T values:
       * CM_SURFACE_FORMAT_A8, char
       * CM_SURFACE_FORMAT_V8U8, short
       * CM_SURFACE_FORMAT_A8R8G8B8, int
   * - WIDTH
     - Number of elements
   * - Ibuf
     - Surface index for input buffer
   * - pix_pos
     - Offset in pixels
   * - Block
     - Ref to vector to read from surface (vector_ref only)

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_1_3_a.cpp
      :language: c++
      :lines: 27-34

6.1.4 WriteLinear
^^^^^^^^^^^^^^^^^

Write a vector to a linear buffer.

**Description:** Write a pixel vector of size WIDTH to a linear buffer. WIDTH can be any integer.

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint WIDTH> void WriteLinear(SurfaceIndex obuf,
                                           int pix_pos,
                                           vector_ref<T, WIDTH> block);



.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - A type whose size is the same as the size of a single
       surface pixel.

       Example surface types and T values:

       * CM_SURFACE_FORMAT_A8, char
       * CM_SURFACE_FORMAT_V8U8, short
       * CM_SURFACE_FORMAT_A8R8G8B8, int
   * - WIDTH
     - Number of elements
   * - Obuf
     - Surface index for input buffer
   * - pix_pos
     - Offset in pixels
   * - Block
     - Ref to vector to write to surface (vector_ref only)

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_1_4_a.cpp
      :language: c++
      :lines: 27-37

6.2 SLM
-------

NOTE: These routines are not ready for general use at the moment (but are documented here for future
inclusion)

6.2.1 DumpSLM
^^^^^^^^^^^^^

Limited testing

Dump content of SLM to memory

**Description:** Dump the contents of SLM to memory. This is primarily intended as a debugging tool. Note:
this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

**Functions:**

.. code-block:: c++

  void DumpSLM(uint slmX, SurfaceIndex slmDebugSurface, uint size);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - slmX
     - SLM handle
   * - slmDebugSurface
     - Linear surface in which to dump contents of SLM
   * - Size
     - Number of bytes to dump

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_3_1_a.cpp
      :language: c++
      :lines: 27-41

6.2.2 TransposeFromSLM
^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose from SLM

**Description:** Transpose from SLM. Note: this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

.. code-block:: c++

  template <uint N> void TransposeFromSLM(vector_ref<uint, N*4> dst,
                                          vector_ref<uint, N*4> src);

6.2.3 TransposeToSLM
^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose to SLM

**Description:** Transpose to SLM. Note: this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

.. code-block:: c++

  template <uint N> void TransposeToSLM(vector_ref<uint, N*4> dst,
                                        vector_ref<uint, N*4> src);

6.3 Iselect
-----------

The following templates can be used to pack two small data elements into a single larger data element.
This is useful when one needs  to simultaneously update multiple matrices through iselect. As iselect is a
SIMD-1 for left hand side assignment.

6.3.1 Pack
^^^^^^^^^^

Limited testing

Pack 2 "small" elements into a single double sized element

**Description:**  Use this as part of iselect optimization, see example for more info. Typically packing would
be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template <typename Tpackd, typename Tcompst, uint WD>
  void Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
            vector_ref<Tcompst, WD> dst);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - Tpackd
     - Data type of the 2 sources
   * - Tcompst
     - Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)
   * - WD
     - WIDTH of data vectors
   * - src1
     - First source vector
   * - src2
     - Second source vector
   * - Dst
     - Destination composite vector

6.3.2 Unpack
^^^^^^^^^^^^

Limited testing

UnPack a composite element previously generated through Pack(), into 2 elements

**Description:**  Use this as part of iselect optimization. Typically packing would be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template <typename Tpackd, typename Tcompst, uint WD>
  void UnPack(vector_ref<Tpackd, WD> dst1, vector_ref<Tpackd, WD> dst2,
              vector_ref<Tcompst, WD> src);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - Tpackd
     - Data type of the 2 sources
   * - Tcompst
     - Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)
   * - WD
     - WIDTH of data vectors
   * - src
     - Source Composite vector
   * - dst1
     - First destination vector,  corresponding  to first source
       provided to Pack()
   * - dst2
     - Second destination vector, corresponding  to second source
       provided to Pack()


6.3.3 UnpackSingle
^^^^^^^^^^^^^^^^^^

Limited testing

UnPack a composite element previously generated through Pack(), return only 1 of the 2 elements

**Description:**  Use this as part of iselect optimization, Use this API, instead of Unpack, when only 1 element
is required. Typically packing would be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  #define CMRT_LOC_FIRST 0
  #define CMRT_LOC_SECOND 1

  template <typename Tpackd, typename Tcompst, uint WD>
  void UnpackSingle(vector_ref<Tpackd, WD> dst, int location, vector_ref<Tcompst, WD> src);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - Tpackd
     - Data type of the 2 sources
   * - Tcompst
     - Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)
   * - WD
     - WIDTH of data vectors
   * - Src
     - Source Composite vector
   * - Dst
     - destination packed element vector
   * - Location
     - Indicates which packed vector to retrieve, either
       CMRT_LOC_FIRST or CMRT_LOC_SECOND should be returned.

       The order of vectors is based on the order provided to Pack
       from left to right,
       e.g.
       | Pack(src1,src,2,dst)
       | UnpackSingle(dst, src, CMRT_LOC_FIRST)
       | Will set src  with values from src1

6.4 CachedStack
---------------

A Cached Stack object represents Multi-Channel (SIMD) Stacks, (optionally) stored in a host memory
buffer, with the elements of the stack cached in GRF. The fundamental functionality provided by a Stack
object are:

* Push() -- add an element to top of stack
* Pop()  - extract top element from Stack
* Top()  - read the top element  without extracting it.

6.4.1 CachedStackInit
^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Initializes a CachedStack Object

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint W, uint CACHESIZE>
  void CachedStackInit(matrix_ref<short, 3, W*STACKCOUNT> context,
                       matrix_ref<int, 2, W> context_ii,
                       uint MaxSize);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - CACHESIZE
     - Pixel Size of Stack cache stored in GRF, currently MUST be:
       CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)
   * - T
     - Type of elements stored in stack
   * - W
     - Number of channels (SIMD) for an element on stack context,
       Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - context_ii
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - MaxSize
     - Maximum number of elements on stack for a single channel.
       Caller must allocate the linear surface with:

       size >= MaxSize*NumThreads*W*Sizeof(T)

       Optimization note: MaxSize should be aligned to:
       (CACHELINE / sizeof(T))  (CACHELINE = 64 bytes)

**Example:**

.. code-block:: c++

  #include <cm/cmtl.h>

  #define ALIGN_CACHELINE_SHORT(x) (((x) + 64 - 1) & (~ (64 - 1)))

  _GENX_MAIN_ void
  CachedStackExample(SurfaceIndex stack_surf, int ImageHeight) {
      ushort x_pos = get_thread_origin_x() * SZ;

      vector<ushort, 16> element = 1;
      vector<ushort, 16> mask = 1;
      int aligned_image_height = ALIGN_CACHELINE_SHORT(ImageHeight);

      CachedStackInit<uint, 16, CACHESIZE>(stack_context,
                                           stack_context_ii,
                                           aligned_image_height);
      CachedStackPush<uint, 16, CACHESIZE>(stack_surf,
                                           stack_context,
                                           stack_context_ii,
                                           stack, element);

      CachedStackPop<uint, SIMD_SZ, CACHESIZE>(stack_surf,
                                               stack_context,
                                               stack_context_ii, stack,
                                               element, mask);
  }

.. _CachedStackTop:

6.4.2 CachedStackTop
^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Returns a vector containing top elements in the stack

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint W, uint CACHESIZE>
  void CachedStackTop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                      matrix_ref<int, 2, W> context_ii,
                      matrix_ref<T, W, CACHESIZE> stack,
                      vector_ref<T,W> element);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - CACHESIZE
     - Pixel Size of Stack cache stored in GRF, currently MUST be:
       CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)
   * - T
     - Type of elements stored in stack
   * - W
     - Number of channels (SIMD) for an element on stack
   * - Surf
     - Pointer to linear surface used for stack
   * - Context
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - context_ii
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - stack
     - User allocated stack cache
   * - Element
     - Returned top element vector

**Example:**
See example for CachedStackInit


6.4.3 CachedStackPop
^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Conditionally pop top elements from stack based on mask, for channels with a 0 mask pixel element is
returned to caller but not popped (see :ref:`CachedStackTop`). Note: Caller must avoid Pop() over empty stack
channels. See example for details of how to avoid this.

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<typename T, uint W, uint CACHESIZE>
  void CachedStackPop(SurfaceIndex surf, matrix_ref<short,2,W> context,
                      matrix_ref<int,2,W> context_ii,
                      matrix_ref<T,W, CACHESIZE> stack,
                      vector_ref<T,W> element,
                      vector_ref<short,W> mask);

.. list-table::
   :header-rows: 1
   :widths: 15 62

   * - Parameters
     - 
   * - CACHESIZE
     - Pixel Size of Stack cache stored in GRF, currently MUST be:
       | CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)
   * - T
     - Type of elements stored in stack
   * - W
     - Number of channels (SIMD) for an element on stack
   * - Surf
     - Pointer to linear surface used for stack
   * - Context
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - context_ii
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - Stack
     - User allocated stack cache
   * - Element
     - Returned top element vector
   * - Mask
     - A 0-1 mask indicating top elements to Pop() (mask = 1) vs
       elements to Top() (mask = 0).  Note: currently only a 0,1
       mask is supported, if mask contains other values behavior
       will be undefined

**Example:**

Note: Caller must avoid Pop() over empty stack channels. Caller can use CacheStackEmpty() to set a mask
to avoid this condition:

.. code-block:: text

  vector<short, W> isEmpty;
  vector<short, W> appMask; CallerSetMask(mask);
  CachedStackEmpty<...>(..., isEmpty);
  mask |= (1-isEmpty);
  CachedStackPop<..>(..., mask);

6.4.4 CachedStackPush
^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Push a vector element to top of stack. Note: Caller must not push() over a full stack (top element at
MaxSize -- 1)

**Author:** Dori Eldar

Function:

.. code-block:: c++

  template <typename T, uint W, uint CACHESIZE>
  void CachedStackPush(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                       matrix_ref<int, 2, W> context_ii,
                       matrix_ref<T, W, CACHESIZE> stack,
                       vector_ref<T, W> element);

.. list-table::
   :header-rows: 1
   :widths: 15 62

   * - Parameters
     - 
   * - CACHESIZE
     - Pixel Size of Stack cache stored in GRF, currently MUST be:
       | CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)
   * - T
     - Type of elements stored in stack
   * - W
     - Number of channels (SIMD) for an element on stack
   * - surf
     - Pointer to linear surface used for stack
   * - context
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - context_ii
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - stack
     - User allocated stack cache
   * - element
     - Returned top elements vector

**Example:**
See example for CachedStackInit

6.4.5 CachedStackEmpty
^^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Returns a per element mask if Stack is empty or not

**Author:** Dori Eldar

.. code-block:: c++

  template<uint W> void CachedStackEmpty(matrix_ref<short, 2, W> context,
                                         matrix_ref<int, 2, W> context_ii,
                                         vector_ref<short, W> isEmpty);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - W
     - Number of channels (SIMD) for an element on stack
   * - context
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - context_ii
     - Internally used by CachedStack to maintain cache pointers,
       caller must not modify this parameter.
   * - isEmpty
     - 0-1 mask,  1 Stack Empty 0 - Otherwise

**Example:**
See example for CachedStackPop

6.5 Matrix transform
--------------------

A series of functions (mainly for 16x16 matrices) to perform various rotation and mirror operations on
matrix_ref matrices.

6.5.1 MirrorVertical
^^^^^^^^^^^^^^^^^^^^

Limited testing

Vertical mirror matrix transform

**Description:** Perform a vertical mirror transformation on a matrix, transforming it to a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template <typename T, uint H, uint W> void MirrorVertical(matrix_ref<T,H,W> in, matrix_ref<T,H,W> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - H
     - Height of the matrix (number of rows)
   * - W
     - Width of the matrix (number of columns)
   * - in
     - The input matrix (matrix_ref<T,H,W>)
   * - out
     - The output matrix (matrix_ref<T,H,W>)

**Example:**

.. code-block:: c++

  matrix<float, 32, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 32, 16> mirrored_test_matrix;
  cmtl::MirrorVertical(test_matrix.select_all(), mirrored_test_matrix.select_all());

6.5.2 MirrorHorizontal_16x16
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Horizontal mirror for 16x16 matrix transform

**Description:** Perform a horizontal mirror transformation on a 16x16 matrix, transforming it to a different
output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void MirrorHorizontal_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> mirrored_test_matrix;
  cmtl::MirrorHorizontal_16x16(test_matrix.select_all(), mirrored_test_matrix.select_all());

6.5.3 Rotate90_16x16
^^^^^^^^^^^^^^^^^^^^

Limited testing

Rotate 90 clockwise for 16x16 matrix transform

**Description:** Perform a 90 degree clockwise rotation transformation on a 16x16 matrix, transforming it to
a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void Rotate90_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate90_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.5.4 Rotate270_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing
Rotate 270 clockwise for 16x16 matrix transform

**Description:** Perform a 270 degree clockwise rotation transformation on a 16x16 matrix, transforming it
to a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void Rotate270_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate270_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.5.5 Rotate180_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing

Rotate 180 for 16x16 matrix transform

**Description:** Perform a 180 degree rotation transformation on a 16x16 matrix, transforming it to a
different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void Rotate180_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate180_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.5.6 Transpose_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose for 16x16 matrix

**Description:** Perform a transpose on a 16x16 matrix, transforming it to a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void Transpose_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> transpose_test_matrix;
  cmtl::Transpose_16x16(test_matrix.select_all(), transpose_test_matrix.select_all());

6.5.7 ReverseTranspose16x16
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Reverse transpose for 16x16 matrix

**Description:** Perform a reverse transpose on a 16x16 matrix, transforming it to a different output matrix.
Note: reverse transpose is not an inverse transpose (which is the same as a transpose), but instead a
transpose about the other diagonal than the normal transpose.

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T>
  void ReverseTranspose_16x16(matrix_ref<T,16,16> in,
                              matrix_ref<T,16,16> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - in
     - The input matrix (matrix_ref<T,16,16>)
   * - out
     - The output matrix (matrix_ref<T,16,16>)

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> reverse_transp_test_matrix;
  cmtl::ReverseTranspose_16x16(test_matrix.select_all(),
                               reverse_transp_test_matrix.select_all());

6.5.8 Transpose_8x8
^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose for 8x8 matrix

**Description:** Perform a transpose on an 8x8 matrix, transforming it to a different output matrix

**Author:** Michele Casula

**Functions:**

.. code-block:: c++

  template<typename T> void Transpose_8x8(matrix_ref<T,8,8> in,
                                 matrix_ref<T,8,8> out);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - In
     - The input matrix (matrix_ref<T,8,8>)
   * - Out
     - The output matrix (matrix_ref<T,8,8>)

**Example:**

.. code-block:: c++

  matrix<float, 8, 8> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 8, 8> transpose_test_matrix;
  cmtl::Transpose_8x8(test_matrix.select_all(),
                      transpose_test_matrix.select_all());

6.5.9 map
^^^^^^^^^

Limited testing

Generic square matrix mapping

**Description:** A generic square matrix mapping function. A mapping vector provides the source for
mapping each element. For function T(x) -> T(mapping(x)) = x
e.g. mapping for 2x2 90 deg clockwise turn: mapping = { 2, 0, 3, 1 }

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<typename T, uint W> void map(matrix_ref<T, W,W> in,
                               matrix_ref<T, W,W> out,
                               vector_ref<ushort, W*W> mapping);

.. list-table::
   :header-rows: 1
   :widths: 15 61

   * - Parameters
     - 
   * - T
     - The type of the matrix elements
   * - W
     - The width and height (rows and columns) for the square matrix
   * - In
     - The input matrix (matrix_ref)
   * - Out
     - The output matrix (matrix_ref)
   * - mapping
     - The mapping vector (vector_ref). Elements proceed in row
       order (e.g. row 0 is defined, then row 1 etc)

**Example:**

.. code-block:: c++

  matrix<float, 2, 2> test_matrix;

  // Set up the values in test matrix in some way
  matrix<float, 2, 2> map_test_matrix;
  vector<ushort, 4> idx;
  idx[0] = 3;
  idx[1] = 1;
  idx[2] = 2;
  idx[3] = 0;

  cmtl::map(test_matrix.select_all(), map_test_matrix.select_all(), idx.select_all());

6.6 Init and assignment
-----------------------

6.6.1 cm_vector_assign
^^^^^^^^^^^^^^^^^^^^^^

Initialise a vector with some simple rules

**Description:**
Initialise a vector with a sequence of values defined using an initial value and a step.

**Functions:**

.. code-block:: c++

  template<typename T, uint Size> void cm_vector_assign(vector_ref<T,Size> v,
                                               int InitValue, int Step);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - The type of the vector elements
   * - Size
     - The size of the vector
   * - InitValue
     - The value to assign to the first element in the vector
   * - Step
     - The amount to increase by for each subsequent element
       assigned

**Example:**

.. code-block:: c++

  vector<int, 100> test_vector;
  cmtl::cm_vector_assign(test_vector.select_all(), 20, 10);
  // Assigns vector values 20, 30, 40, ... 1000, 1010

6.6.2 cm_matrix
^^^^^^^^^^^^^^^

Macro to create and initialize a matrix

**Description:**
Create and initialize a new matrix and initialize with some simple values in an optimal way.

**Functions:**

.. code-block:: c++

  #define cm_matrix(M, T, R, C, I, S)

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - M
     - Name of the matrix to be created
   * - T
     - Type of the matrix to be created
   * - R
     - Number of rows in the new matrix
   * - C
     - Number of columns in the new matrix
   * - I
     - Initial value for the new matrix
   * - S
     - Step value between each element

**Example:**

.. code-block:: c++

  cm_matrix(test_matrix, short, 10, 10, 1, 2)
  // Assigns a matrix with values 1, 3, 5 etc. (starting row 0, col 0,
  // row 0, col 1 and so on

6.6.3 cm_vector
^^^^^^^^^^^^^^^

Macro to create and initialize a vector

**Description:**
Create and initialize a new vector and initialize with some simple values in an optimal way.

**Functions:**

.. code-block:: c++

  #define cm_vector(M, T, N, I, S)

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - M
     - Name of the vector to be created
   * - T
     - Type of the vector  to be created
   * - N
     - Number of elements in the new vector
   * - I
     - Initial value for the new vector
   * - S
     - Step value between each element

**Example:**

.. code-block:: c++

  cm_vector(test_vector, char, 20, 20, -1)
  // Assigns a vector with values 20, 19, 18 etc

6.7 Extended math
-----------------

6.7.1 cm_atan2_fast
^^^^^^^^^^^^^^^^^^^

Fast atan2

**Description:**
Fast atan2 implementation for various input types (see functions for details). The results of this call are
generated more quickly but are less accurate than the standard atan2 function. It is only accurate up to
0.01, where cm_atan2 is precise up to 0.00001.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C> cm_atan2_fast(
                              matrix<float,R,C> y, matrix<float,R,C> x,
                              const uint flags = 0);
  template <uint R, uint C> matrix<float,R,C> cm_atan2_fast(
                              matrix_ref<float,R,C> y,
                              matrix_ref<float,R,C> x,
                              const uint flags = 0);
  template <uint N> vector<float,N> cm_atan2_fast(vector<float,N> y,
                                                  vector<float,N> x,
                                                  const uint flags = 0);
  template <uint N> vector<float,N> cm_atan2_fast(vector_ref<float,N> y,
                                                  vector_ref<float,N> x,
                                                  const uint flags = 0);
  template <typename T> float cm_atan2_fast(T y, T x,
                                            const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - T
     - Type for atan2 parameters (scalar variant)
   * - y
     - y parameter for standard atan2(y,x) mathematical function
   * - x
     - x parameter for standard atan2(y,x) mathematical function
   * - flags
     - Set the execution mode flags  (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_atan2_fast(y, x);

6.7.2 cm_atan2
^^^^^^^^^^^^^^

Fast atan2

**Description:**
Fast atan2 implementation for various input types (see functions for details). The results of this call are
generated less quickly but are more accurate than the standard cm_atan2_fast function. It is accurate up
to 0.00001, where cm_atan2 _fast is only precise up to 0.01.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C>cm_atan2(
                              matrix<float,R,C> y, matrix<float,R,C> x,
                              const uint flags = 0);
  template <uint R, uint C> matrix<float,R,C> cm_atan2(
                              matrix_ref<float,R,C> y,
                              matrix_ref<float,R,C> x,
                              const uint flags = 0);
  template <uint N> vector<float,N> cm_atan2(vector<float,N> y,
                                             vector<float,N> x,
                                             const uint flags = 0);
  template <uint N> vector<float,N> cm_atan2(vector_ref<float,N> y,
                                             vector_ref<float,N> x,
                                             const uint flags = 0);
  template <typename T> float cm_atan2(T y, T x,
                                       const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - T
     - Type for atan2 parameters (scalar variant)
   * - Y
     - y parameter for standard atan2(y,x) mathematical function
   * - X
     - x parameter for standard atan2(y,x) mathematical function
   * - Flags
     - Set the execution mode flags  (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_atan2(y, x);

6.7.3 cm_fmod
^^^^^^^^^^^^^

Floating point remainder function

**Description:**
Floating-point Remainder. This is the same as C standard function fmod(), namely:
cm_fmod(y,x)=r, if y = qx + r where q is an integer and r<x.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C> cm_fmod (
                              matrix<float,R,C> y, matrix<float,R,C> x,
                              const uint flags = 0);
  template <uint R, uint C> matrix<float,R,C> cm_fmod(
                              matrix_ref<float,R,C> y,
                              matrix_ref<float,R,C> x,
                              const uint flags = 0);
  template <uint N> vector<float,N> cm_fmod(vector<float,N> y,
                                            vector<float,N> x,
                                            const uint flags = 0);
  template <uint N> vector<float,N> cm_fmod(vector_ref<float,N> y,
                                            vector_ref<float,N> x,
                                            const uint flags = 0);
  template <typename T> float cm_fmod(T y, T x,
                                      const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - T
     - Type for fmod parameters (scalar variant)
   * - Y
     - y parameter for standard fmod(y,x) mathematical function
   * - X
     - x parameter for standard fmod(y,x) mathematical function
   * - Flags
     - Set the execution mode flags  (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_fmod(y, x);

6.7.4 cm_floor
^^^^^^^^^^^^^^

Floating point floor function

**Description:**
Produces the largest integer not greater than input operand (round down) -- this is effectively the same
functionality as cm_rndd. This is the same as C standard function floorf().
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <typename RT, uint R, uint C> vector<RT, R*C> cm_floor(
                                         const matrix<float,R,C> src,
                                         const uint flags = 0);
  template <typename RT, uint SZ> vector<RT, SZ> cm_floor(
                                         const vector<float,SZ> src,
                                         const uint flags = 0);
  template <typename RT> RT cm_floor(const float src,
                                     const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - RT
     - Return type (elements of return object -- scalar, vector or
       matrix) -- usually float
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - SZ
     - Number of elements (vector variant)
   * - src
     - Input parameter which will be floor'ed
   * - flags
     - Set the execution mode flags  (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_floor(x);

6.7.5 cm_ceil
^^^^^^^^^^^^^

Floating point ceil function

**Description:**
Produces the smallest integer not less than input operand (round up) -- this is effectively the same
functionality as cm_rndu. This is the same as C standard function ceilf().
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <typename RT, uint R, uint C> vector<RT, R*C> cm_ceil(
                                         const matrix<float,R,C> src,
                                         const uint flags = 0);
  template <typename RT, uint SZ> vector<RT, SZ> cm_ceil(
                                         const vector<float,SZ> src,
                                         const uint flags = 0);
  template <typename RT> RT cm_ceil(const float src,
                                    const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - RT
     - Return type (elements of return object -- scalar, vector or
       matrix) -- usually float
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - SZ
     - Number of elements (vector variant)
   * - src
     - Input parameter which will be ceil'ed
   * - flags
     - Set the execution mode flags  (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_ceil(x);

6.7.6 cm_tanh
^^^^^^^^^^^^^

Limited testing

hyperbolic tangent

**Description:**

Computes the hyperbolic tangent. Angle is specified in radians

tanh(x) is defined as sinh(x) / cosh(x)

Minimal accuracy is expected to be <= 5 ULP.

**Author:** Alexander Paige (?), Trubenkov Dmitry

**Functions:**

.. code-block:: c++

    float cm_tanh(float x);
    template <int N> vector<float, N> cm_tanh(vector<float, N> x);
    template <int R, int C> matrix<float, R, C> cm_tanh(matrix<float, R, C> x);


.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - x
     - x represents angle (in radians) to compute tanh

**Example:**

.. code-block:: c++

  matrix<float, 4, 4>     v_x;
  // Set up the x values in some way ...
  matrix<float, 4, 4> result = cmtl::cm_tanh(v_x);

6.7.7 cm_tanh_cody_waite
^^^^^^^^^^^^^^^^^^^^^^^^

Limited testing

hyperbolic tangent (using Cody-Waite algorithm)

**Description:**

Computes the hyperbolic tangent using Cody-Waite algorithm. Angle is specified in radians.

tanh(x) is defined as sinh(x) / cosh(x)

Minimal accuracy is expected to be <= 8 ULP.

**Author:** Trubenkov Dmitry

**Functions:**

.. code-block:: c++

    float cm_tanh_cody_waite(float x);
    template <int N> vector<float, N>
        cm_tanh_cody_waite(vector<float, N> x);
    template <int R, int C> matrix<float, R, C>
        cm_tanh_cody_waite(matrix<float, R, C> x);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - x
     - x represents angle (in radians) to compute tanh

**Example:**

.. code-block:: c++

  matrix<float, 4, 4>     v_x;
  // Set up the x values in some way ...
  matrix<float, 4, 4> result = cmtl::cm_tanh_cody_waite(v_x);

6.7.8 cm_sin_emu
^^^^^^^^^^^^^^^^

Limited testing

EU emulation for sine

**Description:**
EU (Execution Unit) emulation implementation of the sine function. Angle is specified in radians.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C> cm_sin_emu(
                              matrix<float,R,C> x, const uint flags = 0);
  template <uint R, uint C> matrix<float,R,C> cm_sin_emu(
                              matrix_ref<float,R,C> x,
                              const uint flags = 0);
  template <uint N> vector<float,N> cm_sin_emu(vector<float,N> x,
                                               const uint flags = 0);
  template <uint N> vector<float,N> cm_sin_emu(vector_ref<float,N> x,
                                               const uint flags = 0);
  template <typename T> float cm_sin_emu(T x, const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - T
     - Type for the parameter (scalar variant)
   * - x
     - x represents angle (in radians) to compute sine
   * - flags
     - Set the execution mode flags (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_sin_emu(x);

6.7.9 cm_cos_emu
^^^^^^^^^^^^^^^^

Limited testing

EU emulation for cosine

**Description:**
EU (Execution Unit) emulation implementation of the cosine function. Angle is specified in radians.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C> cm_cos_emu(
                              matrix<float,R,C> x, const uint flags = 0);
  template <uint R, uint C> matrix<float,R,C> cm_cos_emu(
                              matrix_ref<float,R,C> x,
                              const uint flags = 0);
  template <uint N> vector<float,N> cm_cos_emu(vector<float,N> x,
                                               const uint flags = 0);
  template <uint N> vector<float,N> cm_cos_emu(vector_ref<float,N> x,
                                               const uint flags = 0);
  template <typename T> float cm_cos_emu(T x, const uint flags = 0);

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - R
     - Number of rows (matrix variant)
   * - C
     - Number of columns (matrix variant)
   * - N
     - Number of elements (vector variant)
   * - T
     - Type for the parameter (scalar variant)
   * - x
     - x represents angle (in radians) to compute cosine
   * - flags
     - Set the execution mode flags (e.g. SAT bit). Defaults to 0

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_cos_emu(x);


6.8 Dataport
-------------

The ``cmtl::dataport`` namespace provides ``read``/``write`` (and related) helper functions for accessing
Shared Local Memory (SLM), Shared Virtual Memory (SVM), and typed surfaces using pointer/offset-based
addressing. Many of the functions have an optional trailing ``size`` argument that enables automatic
bounds checking: when an access would go out of bounds, the function prints a diagnostic message and
either returns without performing the access (writes) or zero-fills the destination (reads), instead of
performing an out-of-bounds access.

These declarations live in ``cm/cmtl/dataport.h``, which is **not** included automatically by ``cm/cmtl.h``
and must be included directly:

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

6.8.1 SLM
^^^^^^^^^^

Read/write helpers for Shared Local Memory (SLM), plus a bulk SLM <-> SVM copy helper.

**Description:**
``read``/``write`` access an SLM buffer, addressed by a linear byte offset (``buf``), as a vector or matrix.
``cm_slm_copy`` copies ``size`` bytes between an SVM buffer and an SLM buffer using a sequence of block
reads/writes of type ``T`` and vector width ``width``.

**Functions:**

.. code-block:: c++

  namespace cmtl { namespace dataport {

  template <typename T = int, unsigned width = 32>
  void cm_slm_copy(unsigned dstBuffer, svmptr_t srcBuffer, unsigned size);
  template <typename T = int, unsigned width = 32>
  void cm_slm_copy(svmptr_t dstBuffer, unsigned srcBuffer, unsigned size);

  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void read(unsigned buf, vector_ref<T, width> vec, int off = 0);
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  vector<T, width> read(unsigned buf, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void read(unsigned buf, matrix_ref<T, height, width> m, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  matrix<T, height, width> read(unsigned buf, int off = 0);

  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void write(unsigned buf, vector<T, width> vec, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void write(unsigned buf, matrix<T, height, width> m, int off = 0);

  // Bounds-checked variants
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void read(unsigned buf, vector_ref<T, width> vec, int off, unsigned size);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void read(unsigned buf, matrix_ref<T, height, width> m, int off,
           unsigned size);
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void write(unsigned buf, vector<T, width> vec, int off, unsigned size);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void write(unsigned buf, matrix<T, height, width> m, int off,
            unsigned size);

  }} // namespace cmtl::dataport

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - Element type of the data being read/written.
   * - width
     - Number of elements (vector variant).
   * - height, width
     - Number of rows and columns (matrix variant).
   * - Algn
     - Alignment tag, e.g. ``AlignTag<Align::OWORD>`` (default) or
       ``AlignTag<Align::GRF>``.
   * - dstBuffer, srcBuffer
     - SLM byte offset (``unsigned``) or SVM pointer (``svmptr_t``),
       depending on copy direction.
   * - buf
     - SLM buffer byte offset.
   * - vec, m
     - Vector/matrix (or vector_ref/matrix_ref) to write, or to fill on
       read.
   * - off
     - Element offset (in units of ``T``), added to ``buf``. Defaults to 0.
   * - size
     - Optional. Total accessible size in elements; when given, enables
       bounds checking (see description above).

**Example:**

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

  _GENX_MAIN_ void my_kernel(svmptr_t svm_addr) {
    cm_slm_init(1024);
    unsigned slm_off = cm_slm_alloc(1024);

    cmtl::dataport::cm_slm_copy<int, 32>(slm_off, svm_addr, 1024);

    vector<int, 32> data = cmtl::dataport::read<int, 32>(slm_off);
    data += 1;
    cmtl::dataport::write(slm_off, data);
  }

6.8.2 SVM
^^^^^^^^^^

Block read/write helpers for Shared Virtual Memory (SVM).

**Description:**
``read``/``write`` access an SVM buffer, addressed by an ``svmptr_t`` pointer, as a vector or matrix, using
block (OWORD-aligned by default) memory accesses.

**Functions:**

.. code-block:: c++

  namespace cmtl { namespace dataport {

  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void read(svmptr_t addr, vector_ref<T, width> vec, int off = 0);
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  vector<T, width> read(svmptr_t addr, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void read(svmptr_t addr, matrix_ref<T, height, width> m, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  matrix<T, height, width> read(svmptr_t addr, int off = 0);

  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void write(svmptr_t addr, vector<T, width> vec, int off = 0);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void write(svmptr_t addr, matrix<T, height, width> m, int off = 0);

  // Bounds-checked variants
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void read(svmptr_t addr, vector_ref<T, width> vec, int off, unsigned size);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void read(svmptr_t addr, matrix_ref<T, height, width> m, int off,
           unsigned size);
  template <typename T, unsigned width, typename Algn = AlignTag<Align::OWORD>>
  void write(svmptr_t addr, vector<T, width> vec, int off, unsigned size);
  template <typename T, unsigned height, unsigned width,
            typename Algn = AlignTag<Align::OWORD>>
  void write(svmptr_t addr, matrix<T, height, width> m, int off,
            unsigned size);

  }} // namespace cmtl::dataport

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - Element type of the data being read/written.
   * - width
     - Number of elements (vector variant).
   * - height, width
     - Number of rows and columns (matrix variant).
   * - Algn
     - Alignment tag, e.g. ``AlignTag<Align::OWORD>`` (default) or
       ``AlignTag<Align::GRF>``.
   * - addr
     - SVM pointer (``svmptr_t``) to the buffer.
   * - vec, m
     - Vector/matrix (or vector_ref/matrix_ref) to write, or to fill on
       read.
   * - off
     - Element offset (in units of ``T``), added to ``addr``. Defaults to 0.
   * - size
     - Optional. Total accessible size in elements; when given, enables
       bounds checking (see the SLM section above for behavior).

**Example:**

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

  _GENX_MAIN_ void my_kernel(svmptr_t addr) {
    vector<int, 32> data = cmtl::dataport::read<int, 32>(addr);
    data += 1;
    cmtl::dataport::write(addr, data);
  }

6.8.3 SVM scatter/gather
^^^^^^^^^^^^^^^^^^^^^^^^^

Scatter/gather (per-element offset) read/write helpers for Shared Virtual Memory (SVM), plus scalar
read/write.

**Description:**
``read``/``write`` access an SVM buffer using a base pointer plus a *vector* of per-element offsets
(``offset``), allowing each vector/matrix element to be scattered/gathered to/from an independent
address. Scalar overloads (no ``offset`` vector) are also provided for single-element access. ``check``
performs the bounds-checking test used internally by the bounds-checked overloads and can also be called
directly.

**Functions:**

.. code-block:: c++

  namespace cmtl { namespace dataport {

  template <typename T, unsigned width>
  void read(svmptr_t addr, vector_ref<uint, width> offset,
           vector_ref<T, width> vec, int off = 0);
  template <typename T, unsigned width>
  vector<T, width> read(svmptr_t addr, vector_ref<uint, width> offset,
                        int off = 0);
  template <typename T, unsigned height, unsigned width>
  matrix<T, height, width> read(svmptr_t addr,
                                vector_ref<uint, height * width> offset,
                                int off = 0);

  template <typename T, unsigned width>
  void write(svmptr_t addr, vector_ref<uint, width> offset,
            vector_ref<T, width> vec, int off = 0);
  template <typename T, unsigned height, unsigned width>
  void write(svmptr_t addr, vector_ref<uint, height * width> offset,
            matrix<T, height, width> m, int off = 0);

  // Bounds-checked variants
  template <unsigned width>
  int check(vector_ref<uint, width> offset, int off, unsigned size);
  template <typename T, unsigned width>
  void read(svmptr_t addr, vector_ref<uint, width> offset,
           vector_ref<T, width> vec, int off, unsigned size);
  template <typename T, unsigned width>
  void write(svmptr_t addr, vector_ref<uint, width> offset,
            vector<T, width> vec, int off, int size);

  // scalar (no offset vector)
  template <typename T>
  T read(svmptr_t addr, int off = 0);
  template <typename T>
  T read(svmptr_t addr, int off, int size);
  template <typename T>
  void write(svmptr_t addr, T data, int off = 0);
  template <typename T>
  void write(svmptr_t addr, T data, int off, int size);

  }} // namespace cmtl::dataport

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - T
     - Element type of the data being read/written.
   * - width
     - Number of elements (vector variant).
   * - height, width
     - Number of rows and columns (matrix variant).
   * - addr
     - Base SVM pointer (``svmptr_t``).
   * - offset
     - Vector of per-element offsets (in units of ``T``), added to
       ``off`` and scaled by ``sizeof(T)``.
   * - vec, m, data
     - Vector/matrix/scalar (or vector_ref) to write, or to fill on read.
   * - off
     - Element offset (in units of ``T``) added to every element of
       ``offset``. Defaults to 0.
   * - size
     - Optional. Total accessible size in elements; when given, enables
       bounds checking: if any ``off + offset[i]`` is out of range, the
       whole access is skipped (or zero-filled for reads) with a
       diagnostic message.

**Example:**

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

  _GENX_MAIN_ void my_kernel(svmptr_t addr) {
    vector<uint, 16> offsets;
    // ... offsets computed in some way ...

    vector<int, 16> data = cmtl::dataport::read<int, 16>(addr, offsets);
    data += 1;
    cmtl::dataport::write<int, 16>(addr, offsets, data);
  }

6.8.4 SVM atomic
^^^^^^^^^^^^^^^^^

Atomic read-modify-write helper for Shared Virtual Memory (SVM).

**Description:**
``write_atomic`` performs an atomic memory operation ``op`` (one of the ``CmAtomicOpType`` values, e.g.
``ATOMIC_ADD``, ``ATOMIC_INC``, ``ATOMIC_CMPXCHG``) on each element of an SVM buffer, addressed by a
base pointer plus a vector of per-element offsets. Overloads are provided depending on whether ``op``
requires zero, one, or two source operands (``ATOMIC_INC``/``ATOMIC_DEC`` take none, most operations take
one, ``ATOMIC_CMPXCHG``/``ATOMIC_FCMPWR`` take two), and whether the previous memory value should be
returned.

**Functions:**

.. code-block:: c++

  namespace cmtl { namespace dataport {

  // most ops (one source operand)
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                    vector<T, N> src0, vector_ref<T, N> ret);
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                    vector<T, N> src0);

  // ATOMIC_INC / ATOMIC_DEC (no source operand)
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                    vector_ref<T, N> ret);
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset);

  // ATOMIC_CMPXCHG / ATOMIC_FCMPWR (two source operands)
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                    vector<T, N> src0, vector<T, N> src1,
                    vector_ref<T, N> ret);
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset,
                    vector<T, N> src0, vector<T, N> src1);

  // Bounds-checked variants (add a `size` argument to any overload above)
  template <CmAtomicOpType op, typename T, unsigned N>
  void write_atomic(svmptr_t addr, vector<uint, N> elementOffset, uint size,
                    vector<T, N> src0, vector_ref<T, N> ret);

  }} // namespace cmtl::dataport

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - op
     - The atomic operation to perform, a ``CmAtomicOpType`` value (e.g.
       ``ATOMIC_ADD``, ``ATOMIC_INC``, ``ATOMIC_CMPXCHG``); determines
       which overload (0, 1, or 2 source operands) applies.
   * - T
     - Element type of the data being operated on.
   * - N
     - Number of elements.
   * - addr
     - Base SVM pointer (``svmptr_t``).
   * - elementOffset
     - Vector of per-element offsets (in units of ``T``), added to
       ``addr``.
   * - src0, src1
     - Source operand(s) for the atomic operation; required depending
       on ``op`` (see description above).
   * - ret
     - Optional; receives the memory value from before the atomic
       operation was applied.
   * - size
     - Optional. Total accessible size in elements; when given, enables
       bounds checking, skipping the whole operation with a diagnostic
       message if any offset is out of range.

**Example:**

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

  _GENX_MAIN_ void my_kernel(svmptr_t addr) {
    vector<uint, 8> offsets;
    // ... offsets computed in some way ...

    vector<int, 8> src0 = 1;
    vector<int, 8> old_value;
    cmtl::dataport::write_atomic<ATOMIC_ADD, int, 8>(addr, offsets, src0,
                                                     old_value);
  }

6.8.5 Typed
^^^^^^^^^^^^

Bounds-safe compile-time-offset 2D typed-surface read.

**Description:**
Reads a ``matrix_ref<T, N, M>`` block from a typed surface at position ``(X, Y)``, shifted by the
compile-time offsets ``(x_offset, y_offset)``. If the shifted position would read outside the surface on
the top and/or left edge, the out-of-bounds rows/columns of ``output`` are left as zero instead of being
read, rather than performing an invalid access.

**Functions:**

.. code-block:: c++

  namespace cmtl { namespace dataport {

  template <int x_offset, int y_offset, typename T, int N, int M>
  void read(SurfaceIndex idx, int X, int Y, matrix_ref<T, N, M> output);

  }} // namespace cmtl::dataport

.. list-table::
   :header-rows: 1
   :widths: 15 60

   * - Parameters
     - 
   * - x_offset, y_offset
     - Compile-time X/Y offsets (in elements) applied to ``X``/``Y``;
       negative values shift the read towards the top-left of the
       surface.
   * - T
     - Element type of the data being read.
   * - N, M
     - Number of rows and columns of ``output``.
   * - idx
     - Surface index for the typed surface.
   * - X, Y
     - X/Y position (in elements), must be non-negative.
   * - output
     - Matrix to fill with the read data (matrix_ref only).

**Example:**

.. code-block:: c++

  #include <cm/cmtl/dataport.h>

  _GENX_MAIN_ void my_kernel(SurfaceIndex idx, int X, int Y) {
    matrix<int, 8, 8> output;
    // Read an 8x8 block shifted one element up and to the left; the top
    // row and left column are zero-filled if that falls outside the surface.
    cmtl::dataport::read<-1, -1, int, 8, 8>(idx, X, Y, output.select_all());
  }
