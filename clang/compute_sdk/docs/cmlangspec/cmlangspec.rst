.. ========================= begin_copyright_notice ============================

  Copyright (C) 2021-2026 Intel Corporation

  SPDX-License-Identifier: MIT

  =========================== end_copyright_notice =============================

==================================
C for Metal Language Specification
==================================

Revision |release|

Primary Author(s): Kai Yu Chen, Guei-Yuan Lueh

Contributor(s): Chu-cheow Lim, Somnath Ghosh, Chunling Hu, Biju
George, Weiyu Chen, Alexander Yermolovich, Puyan Lotfi, Gang Chen,
Julia Gould, Wei Pan, David Stuttard, Tim Renouf, Tim Corringham,
Stephen Thomas, Konstantin Vladimirov, Alexander Us, Anatoly Parshintsev,
Anton Sidorenko, Anton Zabaznov, Alexader Bezzubikov,
Dmitry Ryabtsev, Nikita Rudenko, Victor Mustya, Maksim Shelegov,
Vladislav Korovin, Vadim Semenov.

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

Copyright (C) 2009-2023, Intel Corporation. All rights reserved.

.. contents:: Table of Contents
   :depth: 3


1 Introduction
==============

1.1 Purpose / Scope
-------------------
This document provides a specification for the C for Metal language. C for Metal is intended to support
high-level programming of compute and media kernels for the Intel® Graphics Media Accelerators [1]. The language is
based on standard C++ language with some restrictions, plus additional features that are designed for
easy expression of the inherent data parallelism in media applications and simplified interface with the
architecture specific hardware features. The organization of this document is as follows:

* Section 2 describes the data types, including the supported subset of C++ data types, CM-defined
  vector and matrix object types, variable qualifiers, type conversion/casting rules, and restrictions.

* Section 3 describes the operations and member functions, in particular the set of overloaded
  operations on vectors and matrices.

* Section 4 describes the functions, user/kernel function qualifiers, calling conventions and built-in
  functions

* Section 5 describes inline assembly.

* Section 6 describes the functions that can be found in the C for Metal Template Library

* Appendix A provides the media kernel example written in CM.

This document assumes reader familiarity with the standard C++ language. It is not intended to describe
details of the target device, C for Metal software stack, or the C for Metal compiler usage. Some background
information is provided in the relevant sections. Please refer to the references listed in section 1.3 for
further information.

1.2 Definitions, Acronyms, and Abbreviation
-------------------------------------------

=========== ============================================================================
Term        Description
=========== ============================================================================
C++  FE     Standard Intel(R) C++ Compiler Front-End
CM FE       C for Metal Compiler Front-End
SIMD        Single Instruction Multiple Data
GPU         Graphics Processing Unit
GenX        Graphics core generations for Intel® Graphics Media Accelerators [1]
Host        IA-32 and Intel® 64 architecture processors
Device      Intel® GenX GPU
Kernel      A program that can be executed on GenX hardware
Thread      An instance of a kernel program that is executed on a GenX hardware
LSB         Least Significant Bit
GRF         General Register File, a set of general-purpose registers available in GenX
DWORD       Double-word, represents 4 bytes for GenX
F32         Single precision floating point data type
F64         Double precision floating point data type
U8          8-bit unsigned integer
S8          8-bit signed integer
U16         16-bit unsigned integer
S16         16-bit signed integer
U32         32-bit unsigned integer
S32         32-bit signed integer
U64         64-bit unsigned integer
S64         64-bit signed integer
HF or F16   16-bit floating point in IEEE-754 binary16 format
BF or BF16  16-bit floating point format with 7-bit exponent and 8-bit mantissa
BF8 or E5M2 8-bit floating point format with 5-bit exponent and 2-bit mantissa
HF8 or E4M3 8-bit floating point format with 4-bit exponent and 3-bit mantissa
TF32        32-bit tensor precision floating point data type
E8M0        8-bit unsigned floating point format with 8-bit exponent and no mantissa
E2M1        4-bit floating point format with 2-bit exponent and 1-bit mantissa
=========== ============================================================================



1.3 References and Related Information
--------------------------------------

* [1] "Intel® Graphics Media Accelerator Developer's Guide",
  http://software.intel.com/en-us/articles/intel-graphics-media-accelerator-developers-guide/.
* [2] United States Patent 7257695, 2007.
* [3] "Intel® C++ Compiler User and Reference Guides",
  http://www.intel.com/cd/software/products/asmo-na/eng/347618.htm.
* [4] "Graphics documentation", Vol5c.6, Intel Corporation.
* [5] C for Metal Usage Model.
* [6] C for Metal Runtime API specification.
* [7] :title:`CMC User Guide`.
* [8] :title:`C for Metal Porting Guide`.


1.4 Revision History
--------------------

Changes since C for Metal 4.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Updated definition of sample32 API with new Output Format Control field

* Added cm_bf_insert, cm_bf_extract and cm_bf_reverse intrinsics

* Modified scatter/gather read/write to reflect the support for non dword types (e.g. now supports
  char, uchar, short, ushort) as well as being byte addressable (not supported pre IVB)

* Description of Gen10 HEVC VME Interface

Changes since C for Metal 5.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Update the description of the cm_sincos() intrinsic

* Remove the requirement that the arguments to cm_min() and cm_max() have the same kind of type

* Clarified that a mask used as a SIMD control flow condition has each
  element considered "true" if not equal to zero. This is different to a mask's
  use in the merge function, where only the least significant bit is
  considered.

* Added cm_pause intrinsic information

* Added new style write_typed_atomic (cm-llvm/cmc only)

* Removed the deprecated function genx_select, and appendix describing clang-cm

* Removed non-supported cm_fgetround and cm_fsetround intrinsics. Added description of
  _GENX_ROUNDING_MODE_ kernel directive

* Added scalar variant for scatter write (enables writing a scalar to buffers - only way to do this
  as all other variants require 2D surface)

* Moved topics relating to compiler usage to the CMC User Guide document.

Changes since C for Metal 6.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* For oword load read(), documented the deprecated MODIFIED_DWALIGNED modifier
  in addition to DWALIGNED.

* Clarified behaviour of cm_shr for different input types, added docs for
  cm_asr and newly implemented cm_lsr.

* Allowed using ``cm_send`` and ``cm_sends`` inside SIMD control-flow.
  When used inside SIMD control-flow, the width of control-flow decides
  the SIMD width of the resulting ``send`` or ``sends``. Therefore use this new
  feature with caution because the resulting message may or may not be
  supported by hardware.

* Added new _GENX_FLOAT_CONTROL_ support to extend floating point control from just rounding modes
  (that _GENX_ROUNDING_MODE_ implements). The new support includes denorm control and IEEE for
  single precision float.

* Added half-precision floating-point (half) as one of basic scalar data types.

* Added new predicate arguments to write_atomic and typed_write_atomic.

* Added read_scaled and write_scaled.

* Updated global variable wording and added __declspec(genx_volatile) qualifier.

* Enabled SIMD-32 control-flow. Because of this, many gather, scatter,
  and atomic functions are also extended to allow SIMD-32. However,
  since SIMD-32 messages may not be natively supported by the target machine,
  compiler may split them into SIMD-16 or SIMD-8. Splitting and merging may
  require extra temp-registers, hence negatively impact the total registers that
  user code can use.

2 Data Types
============

2.1 Scalar Data Types
---------------------

2.1.1 Overview
^^^^^^^^^^^^^^

C for Metal supports the following basic scalar data types defined in C++:

* char, unsigned char (uchar), short, unsigned short (ushort), int, unsigned int (uint), float, half

  * C for Metal supports IEEE-754 conformant 16-bit half-precision floating point type.
    To enable this, use namespace "half_float" for host code, and keyword "half" for variable declaration.

* double: double-precision floating-point data type is only supported for Gen7+ hardware platform,
  with the following usage restrictions:

  * Double operands can be used with usual C for Metal operators where floating-point operands are
    allowed, except division.
  * Double operands can be used with the following C for Metal intrinsic functions:
    cm_abs/cm_min/cm_max/cm_add/cm_mul/cm_sum (all operands must be of double type).
  * Double operands cannot be used in DWord scattered read/write and DWord atomic write.

* unsigned long long, long long: unsigned and signed long long data types are 64-bit integers that are
  supported for Gen8+ hardware platform with strict restrictions:

  * unsigned long long and long long operands can be used with usual C for Metal operators where long
    long operands are allowed, except multiplication and division.
  * No C for Metal intrinsic functions are allowed for unsigned long long and long long types.
  * Unsigned long long and long long operands cannot be used in DWord scattered read/write
    and DWord atomic write.

* svmptr_t: represents an integer of pointer size for SVM (shared virtual memory, Gen8+). When
  declaring a struct that is in SVM, use svmptr_t for a pointer field. The size of svmptr_t is set by
  compiler options /DCM_PTRSIZE=32 or /DCM_PTRSIZE=64; use the size appropriate to whether the
  C for Metal program will be run from a 32 bit or 64 bit application. See :ref:`SharedVirtualMemory`.

* tfloat32: tensor floating point data type is only supported for Gen12+ hardware platforms,
  when CM_HAS_TF32 macro is defined. It's not a real data type meaning C for Metal compiler
  treats ``int`` as a tensor floating point data type for a function expecting a tensor parameter.

* bfloat16: brain 16-bit floating point data type is only supported for Gen12+ hardware platforms,
  when CM_HAS_BF16 macro is defined.

* bfloat8: brain 8-bit floating point data type is only supported for Xe3+ hardware platforms,
  when CM_HAS_BF8 macro is defined. It's not a real data type meaning C for Metal compiler
  treats ``unsigned char`` as a brain 8-bit floating point data type for functions expecting
  a parameter of the bfloat8 type.

* hfloat8: half 8-bit floating point data type is only supported for Xe3p+ hardware platforms,
  when CM_HAS_HF8 macro is defined. It's not a real data type meaning C for Metal compiler
  treats ``char`` as a half 8-bit floating point data type for functions expecting a parameter
  of the hfloat8 type.


The C for Metal compiler will issue an error message for unsupported data types.

2.1.2 Saturation
^^^^^^^^^^^^^^^^

CM supports return value saturation for the most of arithmetic built-in functions.
Saturation is a clamping operation that converts any data that is outside the saturation target range
for the built-in return type to the closest represented value with the target range.

If the return type is float, saturation target range is [0.0, 1.0].
Any floating-point value greater than 1.0 (including +inf) saturates to 1.0,
while any negative floating value (including -inf) saturates to 0.0. NaN saturates to 0.0.
Floating point values between 0.0 and 1.0 are unchanged by saturation.

For integer data types, the maximum range for the given numerical data type is the saturation target range.
Specifically, if integer arithmetic overflows, with saturation the result will be either the largest
or the smallest representable value of the destination type.

The table below lists the saturation target range for data types that support saturation.

===================== =========== =====================================
Built-in return type  Synonyms    Saturation target range
===================== =========== =====================================
half                              [0.0, 1.0]
float                             [0.0, 1.0]
double                            [0.0, 1.0]
uint8_t               uchar       [0, 255]
int8_t                char        [-128, 127]
uint16_t              ushort      [0, 65535]
int16_t               short       [-32768, 32767]
uint32_t              uint        [0, 2\ :sup:`32`\  - 1]
int32_t               int         [-2\ :sup:`31`\ , 2\ :sup:`31`\  - 1]
uint64_t                          [0, 2\ :sup:`64`\  - 1]
int64_t                           [-2\ :sup:`63`\ , 2\ :sup:`63`\  - 1]
===================== =========== =====================================


2.2 Compound Data Types
-----------------------
2.2.1 Base Object
^^^^^^^^^^^^^^^^^

To facilitate the expression of high-level data-parallel operations, C for Metal provides the users with two kinds of
compound data types for base objects: vector and matrix. These types are defined using syntax similar to C++
template classes. The parameters are the type of data element and the size of a vector/matrix, as described
below, which must be compile-time constants according to the C++ language specification.

* "Vector< type, size>" represents a vector of length "size" with elements of type "type".

* "Matrix<type, rows, columns>" represents a "rows" x "columns" matrix with elements of type
  "type".

Note: The data element type must be a supported scalar data type as described in Section 2.1.

Note: The total size of a matrix or vector must be less than the GRF size available for each
hardware thread on the target platform.

Some examples of base object declarations:

.. image:: cmlangspec_files/base_object_declaration_example.png
  :width: 600 px


2.2.2 Reference Object
^^^^^^^^^^^^^^^^^^^^^^

In addition, C for Metal allows the user to define two types of reference objects: vector_ref and matrix_ref, as
described below:

* "vector_ref<type, size>" represents a reference to the elements of some base object that form a
  vector of length "size" with elements of type "type".
* "matrix_ref<type, rows, columns>" represents a reference to the elements of some base object that
  form a "rows" x "columns" matrix with elements of type "type".

The reference objects represent subsets of the base objects. While a base object occupies a storage space
in the GRF that does not overlap with the storage space of other base objects, a reference object is used
internally to refer to a region in the base object and share the storage space. All operations on a reference
object result in applying these operations to the corresponding elements of the base object.

An example of reference object declaration:

.. literalinclude:: ../../../test/CMFE/cmlangspec/2_2_2_a_codegen.cpp
      :language: c++
      :lines: 28,35

2.2.3 Structures
^^^^^^^^^^^^^^^^

C for Metal supports C structures containing all the supported scalar data types and vector/matrix object types
described above, with the restriction that the structure data members must be properly aligned.  The
reference object cannot be declared as a structure field.

2.2.4 Masks
^^^^^^^^^^^

Masks for SIMD comparison and merge operations are represented in C for Metal as follows:

* int -- a vector mask packed into a single int type scalar.

* vector<ushort, size> -- a vector mask.

* matrix<ushort, rows, columns> -- a matrix mask.

Note: Only LSB of each vector/matrix mask element is significant when used in a
merge function. However, when used as a SIMD control flow condition, each
element is compared with 0, thus any non-zero value signifies "true".


2.3 Other Built-in Data Types
-----------------------------

The following built-in data types are defined in C for Metal to represent different objects created and managed by
the C for Metal host runtime [6]. Variables of such data types must be passed through kernel function
parameters, except the reserved global surface indexes CM_STATIC_BUFFER_0 / CM_STATIC_BUFFER_1 /
CM_STATIC_BUFFER_2 / CM_STATIC_BUFFER_3. C for Metal does not allow the explicit use of local/global
variable or modification of such data types in kernel functions, except used as function call argument.
C for Metal does not allow alias among SurfaceIndexes used for dataport functions, as the ordering
between typed and untyped accesses may not be guaranteed by either compiler or hardware.


* SurfaceIndex: represents a surface object used in dataport or other shared functions.

* SamplerIndex: represents a Sampler state object used in Sampler functions.

* VmeIndex: represents a VME state object used in VME functions.

C for Metal supports the use of vector of SurfaceIndex in kernel function, which must be passed from host as
kernel function parameters. C for Metal does not allow modification, reference, or sub-vector select/iselect
operation of SurfaceIndex vector (only basic member select operation vector(i) is allowed).

Usage example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/2_3_a.cpp
      :language: c++
      :lines: 20-30

2.4 Variable Qualifiers
-----------------------

Global variable example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/2_4_a.cpp
      :language: c++
      :lines: 20-56


C for Metal supports global variables, which can be vector/matrix objects described above.
Such variables are treated as normal local C++ variables declared in kernel functions.
Functions can lexically reference global variables. This allows using them
like global variables in addition to function parameters. Note that these global variables
are thread private (i.e., a global variable is shared among the GenX kernel function marked
with "__declspec(genx_main)" and all its callee functions within the same GenX hardware thread).

C for Metal also supports the "__declspec(genx_volatile)" (#define'd to _GENX_VOLATILE_) qualifier
for global variables to indicate that optimizations on these variables shall be limited,
which often helps to decrease register pressure and increases performance.

Optionally, this declspec takes an offset argument to bind a global variable to a specific byte offset,
__declspec(genx_volatile(Offset)), #define'd to _GENX_VOLATILE_BINDING_(Offset).
It is user's responsibility to ensure there is no overlap to other binding globals or thread payloads.

An example is given above (please refer to Section 3 and Section 4 for description
on the operations and functions). C for Metal does not support the initialization of global variables.

2.5 Type Conversions and Casting
--------------------------------

Mixed type operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/2_5_a.cpp
      :language: c++
      :lines: 18-25

C for Metal allows mixed operations of vector and matrix objects of different shapes if the operands have the
same number of data elements. The operand shape conformance is checked at compile time by C++ FE
using template specialization rules for vector/matrix classes. An example is given above.

In such cases, C for Metal compiler determines the data element type in the destination operand based on the
source operand data types using standard C++ rules for handling mixed type computation for scalars
(using template specialization mechanisms). Just like in standard C++, users may want to add explicit type
conversions to change the default type promotion and conversion rules. An example is given here:

.. literalinclude:: ../../../test/CMFE/cmlangspec/2_5_b.cpp
      :language: c++
      :lines: 18-26

2.6 Restrictions
----------------

C for Metal places restrictions on using the following standard C++ features in kernel functions and user defined
functions written for the graphics device (described in Section 3.4):

* Pointers

* C++ references

* Classes other than the supported C structures and vector/matrix/vector_ref/matrix_ref types

* Class inheritance

* Arrays

* Exception handling

* Dynamic type identification

* Memory allocation

* Static variables

* Volatile variables

* Calls to external functions


3 Operations
============

3.1 Scalar Data Operations
--------------------------

C for Metal supports the standard C++ operations for the allowed scalar data types described in Section 2.1.

For floating point operations, C for Metal supports the operation modes implemented in GenX hardware which
have deviations from the IEEE* Floating-point standard IEEE-754.

3.2 Compound Data Operations
----------------------------

C for Metal provides a set of overloaded operators for manipulating vector and matrix objects. These are
described below.

3.2.1 Assignment Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal allows the user to assign the value for each data element in a vector/matrix object, as well as to use
component-wise assignment between two objects. If a source operand is of scalar type then it is
replicated. An example is given here:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_1_a.cpp
      :language: c++
      :lines: 18-34

The assignment operator can be used between two matrix/vector objects with different shapes, if they
have the same total number of data elements. The data elements are copied in the row-major fashion.

3.2.2 Vector/matrix Constructors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Constructor example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_2_a.cpp
      :language: c++
      :lines: 21-24

Similar to standard C++, constructors can be also used in C for Metal to set the data elements in vector/matrix
objects. One restriction is that vector_ref constructor can only take vector/vector_ref object with the
same data type/size, which must be contiguous. Similarly, the matrix_ref constructor can only take
matrix/matrix_ref object with the same data type/size, which must be contiguous. An example is given
above.

To model the data movement instructions with saturation on the target device, C for Metal provides a special
variant of matrix/vector constructor with the "SAT" parameter, as illustrated here:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_2_b.cpp
      :language: c++
      :lines: 21-26

C for Metal provides the following member functions that return the size of a matrix/vector object.

n_rows(): returns the number of rows in a matrix.

n_cols(): returns the number of columns in a matrix.

n_elems(): returns the number of elements in a vector.

3.2.3 Vector/Matrix Initializers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This feature allows users to initialize matrices and vectors through normal C array-initializers. Even though
arrays cannot be used within GenX functions, this feature enables full C syntax of array-initialization for
initializing matrices and vectors. It is advisable to use this feature instead of initializing matrices and
vectors through assignments in the GenX kernels because the compiler would be able to analyze the
initialization sequence and produce optimized vector-immediate or constant moves whenever possible.
The syntax for initializing a vector or matrix during its declaration is simply passing an initialized global
static array (initializer array) as shown in the example here:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_3_a.cpp
      :language: c++
      :lines: 20-33

The initial values of the global array or the initializer-array are sequentially copied to the
vector or matrix. Both integer and floating-point values can be used for initialization using this feature.
Initializer array could be bigger or smaller than the initialized array -- initialization would be done up
to the minimum of their sizes. Initializer array can have more than one dimension -- this gives more
flexibility to users allowing them to skip initialization of some segments of the matrix or vector.

A matrix can also be efficiently declared and initialized with an arithmetic sequence by the built-in
function cm_matrix as shown below. This function uses optimized GenX instruction sequence to perform
the desired initialization. **Note: requires inclusion of cm/cmtl.h header**

Syntax: cm_matrix(M,T,R,C,I,S);

Where M -- Name of the matrix; T -- Type of the elements; R - #Rows; C - #Columns; I -- Initial value of the
sequence; S -- Step of the sequence.

Example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_3_b.cpp
      :language: c++
      :lines: 18-25

Here, matrix 'm' is declared as a 4x8 ushort matrix and initialized to the values 10, 15, 20, 25, ...
Note that, you must not declare 'm' before as this function declares 'm' and then initializes.

Similarly, a vector can be declared and initialized efficiently with cm_vector function.
**Note: requires inclusion of cm/cmtl.h header**:

Syntax: cm_vector(V,T,N,I,S);

Where V -- Name of the vector; T -- Type of the elements; N - #Elements; I -- Initial value of the sequence; S
-- Step of the sequence.

Example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_3_c.cpp
      :language: c++
      :lines: 18-25

Here, vector 'v' is declared as a 16-element ushort vector and initialized to the values 2, 5, 8, ...
Note that, you must not declare 'v' before as this function declares 'v' and then initializes.

A pre-declared vector or matrix can also be efficiently assigned an arithmetic sequence with the following
cmtl function (requires cm/cmtl header to be included):

Syntax: cmtl::cm_vector_assign(V,I,S);

Where V -- Name of the vector (can use a matrix 'format'-ed to a vector); I -- Initial value of the sequence;
S -- Step of the sequence. Here initial value 'I' can be a variable.

Example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_3_d.cpp
      :language: c++
      :lines: 18-27

Here, 10 elements of vector 'v' are assigned as follows: v(2) =  i; v(3) = i+3; v(4) =  i+6, ...

Note that, here 'v' is assumed to be already declared before.

3.2.4 Arithmetic Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal supports the following arithmetic operators for vector/matrix objects:

+, -, \*, /, %, +=, -=, \*=

These are all component-wise operations that follow standard C++ rules for the corresponding scalar data
computation. The operands must conform to the type conversion rules described in 2.5. Only when an
operand is of scalar type, it is replicated as needed.

Arithmetic operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_4_a.cpp
      :language: c++
      :lines: 18-32

3.2.5 Shift Operators
^^^^^^^^^^^^^^^^^^^^^

C for Metal supports the following shift operators for vector/matrix objects:

>>, <<, >>=, <<=

These are all component-wise operations that follow standard C++ rules for the corresponding scalar data
computation.

3.2.6 Bitwise Operators
^^^^^^^^^^^^^^^^^^^^^^^

C for Metal supports the following bitwise operators for vector/matrix objects:

&, \|, ^, !, &=, \|=, ^=, ~

These are all component-wise operations that follow standard C++ rules for the corresponding scalar data
computation.

3.2.7 Logical Operators
^^^^^^^^^^^^^^^^^^^^^^^

C for Metal supports logical operators which operate on values that are contextually converted to bool. These
are guaranteed to be evaluated left-to-right within an expression.

&& \|\|

3.2.8 Comparison Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides a set of overloaded comparison operators for vector/matrix objects. They perform
component-wise comparison for the operands "x" and "y" and the result value is 0 for False, 1 for True. If
one operand is of scalar type then it is replicated to match the size of another operand.

* vector<ushort, size>  operator OP (VM x, VMC y);

* vector<ushort, size>  operator OP (VMC x, VM y);

Note:

* OP is one of {<; <=; >; >=; ==; !=}
* VM is any type of vector/vector_ref/matrix/matrix_ref
* VMC is any type of vector/vector_ref/matrix/matrix_ref/<scalar_type>

The comparison operations can be used for assignment to a mask, as in the following example.

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_8_a.cpp
      :language: c++
      :lines: 18-29

.. _SelectMemberFunctions:

3.2.9 Select Member Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides a set of "select" functions for referencing a subset of the elements of vector/matrix objects.
All these operations (except "iselect" and "replicate") return a reference to the elements
of matrix/vector objects, so they can be used as L-values in the statements.

() Operator: Standard Matrix/Vector Element-Access Operator
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

The two basic operators are described below:

* operator(ushort i): returns the i-th scalar element of a vector.
* operator(ushort i, ushort j): returns the (i, j)-th scalar element of a matrix, where "i" is the index
  of a row and "j" is the index of a column (the index starts from 0).

() select operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_a.cpp
      :language: c++
      :lines: 22-25

Alternatively, C for Metal supports the following syntax that is similar to the standard C array access operator:

[] Operator: Standard Matrix/Vector Element-Access Operator
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

The two basic operators are described below:

* operator[ushort i]: returns the i-th scalar element of a vector.
* operator[ushort i][ushort j]: returns the [i][j]-th scalar element of a matrix, where "i" is the index
  of a row and "j" is the index of a column (the index starts from 0)..

[] select operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_b.cpp
      :language: c++
      :lines: 22-27

select: Sub-Matrix or Sub-Vector Selection with Regular Stride
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

C for Metal also allows users to select a sub-matrix or sub-vector with regular strides using the following:

* **select<size, stride>(ushort i=0)**: returns a reference to the sub-vector starting from the i-th
  element ("size" indicates the number of selected elements; "stride" indicates the distance
  between two adjacent selected elements).

* **select<v_size, v_stride, h_size, h_stride>(ushort i=0, ushort j=0)**: returns a reference to the sub-
  matrix starting from the (i, j)-th element ("v_size" indicates the number of selected rows;
  "v_stride" indicates the distance between two adjacent selected rows; "h_size" indicates the
  number of selected columns; "h_stride" indicates the distance between two adjacent selected
  columns).

Sub-matrix/sub-vector select operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_c.cpp
      :language: c++
      :lines: 22-26

.. image:: cmlangspec_files/a_dot_select_4,2_(1).png
  :width: 600 px

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_d.cpp
      :language: c++
      :lines: 22-27

.. image:: cmlangspec_files/a_dot_select_4,2_(0).png
  :width: 600 px

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_e.cpp
      :language: c++
      :lines: 22-27

.. image:: cmlangspec_files/m1_dot_select_2,2,2,4_(1,2).png
  :width: 600 px

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_f.cpp
      :language: c++
      :lines: 22-26

.. image:: cmlangspec_files/m1_dot_select_4,1,4,2_(0,0).png
  :width: 600 px

Note: C for Metal currently has the following restrictions on "select" operation:

* The horizontal stride and vertical stride must be greater than 0.

* The horizontal / vertical stride must be 1 if the horizontal / vertical size is 1.

Out-of-bound select may be exploited in CM to facilitate kernel optimization,
in which case the values returned for out-of-bound elements are don't cares and
should not affect final output correctness.

The following can be used in C for Metal to return a reference to the whole object.

* **select_all()**

For instance, for a matrix object this function is equivalent to "select<rows, 1, columns, 1>(0, 0)", where
(rows, columns) specifies the original matrix size. An example is given below.

Select_all( ) operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_g.cpp
      :language: c++
      :lines: 18-33

iselect: Indirect-Select or Vector Indexing
"""""""""""""""""""""""""""""""""""""""""""

C for Metal allows selecting a stream of elements from a vector based on the index-values in another vector.
**Doesn't support Double data type on IVB.**

**iselect(idx)** -- which means indirectly select the vector elements as specified by the index or offset
values  in the vector 'idx', and return a new vector of length that is equal to the length of 'idx' vector. 'idx'
vector could be of any arbitrary length. The data type of idx must be unsigned short. An example of its use
is shown below:

iselect(idx) operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_h.cpp
      :language: c++
      :lines: 18-44

C for Metal also allows selecting a stream of elements from a matrix based on vector offsets.

**iselect(row, col)** -- which means indirectly select the matrix elements as specified by the row and column
indices: row and col, both of which have unsigned short type, and return a new vector of length that is
equal to the length of row or col vector. Note that row and col must have the same length. An example of
its use is shown below.

iselect(row, col) operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_i.cpp
      :language: c++
      :lines: 26-31

The iselect method returns a vector or matrix, rather than a vector_ref or a matrix_ref. Therefore iselect
cannot be used as an lvalue.

replicate: source region replication operations
"""""""""""""""""""""""""""""""""""""""""""""""

The replicate operations all operate on linear (one dimensional) regions of matrix/vectors. A row major
layout is assumed for matrix objects in determining the linear order.

The following can be used in C for Metal to replicate a matrix/vector object "REP" times and return a new vector
of "REP" * "rows" * "columns" length, where (rows, columns) specifies the original matrix size.

* **replicate<REP>()**

The following can be used in C for Metal to replicate "W" consecutive elements starting at  (i,j)/ (i) from the
matrix/vector object "REP" times, and return a new vector of "REP" * "W" length.

* **replicate<REP, W>( ushort i=0, ushort j=0) )**

* **replicate<REP, W>( ushort i=0 )**

The following can be used in C for Metal to select/replicate "REP" blocks of "W" consecutive elements starting at
(i,j)/(i) from the matrix/vector object with each block strided by "VS" elements, and return a new vector
of "REP" * "W" length. Selected blocks of "W" elements will overlap if "VS" < "W".

* **replicate<REP, VS, W>( ushort i=0, ushort j=0) )**

* **replicate<REP, VS, W>( ushort i=0 )**

The following can be used in C for Metal to select/replicate "REP" blocks of "W" sequential elements with a stride
of "HS" starting at  (i,j)/(i) from the matrix/vector object with each block strided by "VS" elements, and
return a new vector of "REP" * "W" length. Selected blocks of "W" elements will overlap if "VS" < "W".

* **replicate<REP, VS, W, HS>( ushort i=0, ushort j=0) )**

* **replicate<REP, VS, W, HS>( ushort i=0 )**

row and column: Row and column region selection operations
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

The following can be used in C for Metal to return a reference to the i-th row/column of a matrix object:

* **row(i)**
* **column(i)**

Row/column select operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_9_j.cpp
      :language: c++
      :lines: 22-26

3.2.10 Format Member Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following can be used in C for Metal to reinterpret the basic type (the type of data elements) of a
matrix/vector and change the shape to another matrix/vector.

* **format<type, rows, columns>( )**: returns a reference to the calling object interpreted as a new
  matrix with the shape determined by the template parameters. The size of the new matrix must
  not exceed the size of the source object.

* **format<type>( )**: returns a reference to the calling object interpreted as a new vector with the
  size determined by the template type parameter.

The object to be formatted must be contiguous and aligned. Generally it is the user's responsibility to
ensure that the format usage is correct. When a discontinuity is detected at compile time the C
for Metal compiler will issue an error message. Compiler is expected to preserve the new format
type as specified by the programmer for code generation.

Format operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_10_a.cpp
      :language: c++
      :lines: 22-38

3.2.11 Merge Member Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To model the masked move operations on the target device, C for Metal provides a set of "merge" functions for
vector/matrix objects.

Merge operations with one source operand are defined as follows:

* void VM::merge(VMC x, int mask)

* void VM::merge(VMC x, VM mask)

Note:

* VM is any type of vector/vector_ref/matrix/matrix_ref
* VMC is any type of vector/vector_ref/matrix/matrix_ref/<scalar_type>

The semantic is that if a bit of the mask (or the LSB of the element of a vector/matrix mask) is set, the
value of x (or the corresponding element of x if it is a vector/matrix object) is copied to the corresponding
position of the method's invoking vector/matrix object.

One-source merge operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_11_a.cpp
      :language: c++
      :lines: 22-38

Merge operations with two source operands are defined as follows:

* void VM::merge(VMC x, VMC y, int mask)

* void VM::merge(VMC x, VMC y, VM mask)

Note:

* VM is any type of vector/vector_ref/matrix/matrix_ref
* VMC is any type of vector/vector_ref/matrix/matrix_ref/<scalar_type>

Two-source merge operation example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_11_b.cpp
      :language: c++
      :lines: 22-30

The semantic is that if a bit of the mask (or the LSB of the element of a vector/matrix mask) is set, the
value of x (or the corresponding element of x if it is a vector/matrix object) is copied to the corresponding
position of the destination vector/matrix. Otherwise, the value of y (or the corresponding element of y if
it is a vector/matrix object) is copied to the corresponding position of the method's invoking
vector/matrix object.

3.2.12 Boolean Reduction Functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To facilitate boolean operations, C for Metal provides two predefined boolean reduction functions on mask
objects:

* ushort vector<ushort, size>::any(void) / ushort matrix<ushort, R, C>::any(void)

  This function will return a 1 if any of the value in the mask is non-zero; it will return 0 otherwise.

* ushort vector<ushort, size>::all(void) / ushort matrix<ushort, R, C>::all(void)

  This function will return a 1 if all the values in the mask are non-zero; it will return 0 otherwise.

There is no restriction on the size of the mask.  The result of either function can be used as a scalar value
and can be used in the standard C++ control-flow constructs.

Boolean reduction example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_2_12_a.cpp
      :language: c++
      :lines: 22-51

3.3 Evaluation Order
--------------------

For all operations (including both scalar and compound data operations), all operands are read first and
then the operation is performed. For the example below, v2 is equal to the result of the
original values of m1's rows 0 and 1 plus m1's row 1 and 2.

Evaluation order example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_3_a.cpp
      :language: c++
      :lines: 22-30

3.4 Control flow
----------------

C for Metal supports both scalar as well as SIMD control flow statements. Control flow statements define control
flow (statement) blocks that are subject to the control condition.

3.4.1 Scalar control flow
^^^^^^^^^^^^^^^^^^^^^^^^^

Scalar control flow in C for Metal is expressed by means of the standard C++ control flow statements -- conditional
statements (if-else/switch), loop statements (for/while/do-while), jump statements (break/continue/goto
/return) or function calls.

In scalar control flow statements, the condition must be a scalar and all channels of statements within
scalar control flow blocks are subject to the same scalar condition. The control flow is thus stated to be
uniform for all SIMD channels within control flow blocks. There are no other restrictions imposed by using
just scalar control flow statements.

The boolean reduction example above shows the use of scalar control flow.

3.4.2 SIMD control flow
^^^^^^^^^^^^^^^^^^^^^^^

SIMD control flow in C for Metal is expressed by means of predefined C++ macros. The predefined macros used to
express SIMD control flow and their scalar equivalents as shown in Table 3-1. The SIMD control flow
statements allow the C for Metal programmer to take advantage of the native SIMD control flow support
available in GenX hardware. The execution semantics of these statements match the corresponding
instructions available in GenX instruction set architecture.

SIMD control flow statements:

=============== ======================================= ==============================
Statement type  SIMD control flow statement             Scalar control flow equivalent
=============== ======================================= ==============================
Conditional     SIMD_IF_BEGIN/SIMD_ELSE/                if-else
                SIMD_ELSEIF/SIMD_IF_END
Loop            SIMD_DO_WHILE_BEGIN/SIMD_DO_WHILE_END   do-while
Jump            SIMD_BREAK/SIMD_CONTINUE                break/continue
=============== ======================================= ==============================

In SIMD control flow statements, the condition is generally a SIMD (vector or matrix) condition in place of
a scalar condition. Optionally the condition may also be a scalar  if the SIMD control flow statement is
nested in a SIMD control flow block;
such a scalar is considered to be replicated to the size of the SIMD width of the enclosing SIMD control flow block.
The scalar size of the SIMD condition is stated to be the SIMD width
of the associated block. The SIMD width of a SIMD control flow block with a scalar condition is defined to
be the SIMD width of its enclosing SIMD control flow block.

SIMD control flow is defined on a per channel basis for all statements within a control flow block such that
each channel may take independent control paths within the block. The control flow is thus stated to be
divergent for different channels computed within the block. Generally the scalar size of all statements
(number of channels) within SIMD control flow blocks must correspond with the scalar size of the SIMD
condition, in which case each scalar condition component associates to each corresponding statement
channel; optionally scalar statements with a SIMD width of one are allowed, in which case the scalar
condition associated with the scalar statements is determined to be true if any of the channels in the
SIMD control flow block is active and false otherwise.

SIMD control flow usage example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/3_4_2_a.cpp
      :language: c++
      :lines: 26-52

The following constraints are placed on statements appearing within SIMD control flow blocks in order to
guarantee consistent semantics and to allow for an efficient implementation.

* The SIMD width of a SIMD control flow block must be a power of two, greater than one and less
  than or equal to 32.

* Nested SIMD control flow blocks must have the same SIMD width as their enclosing SIMD blocks.
  Thus the SIMD width of all nested SIMD control flow blocks is determined by the SIMD width of
  outermost SIMD control flow block.

* All statements within a SIMD control flow block must either have their SIMD widths correspond
  to the SIMD width of its block, or must have a SIMD width equal to one. For statements  that
  have destinations, requiring the statement to have a SIMD width equal to the SIMD width of its
  block implies that the destinations must be a vector/matrix of SIMD width elements; source
  expressions of statements may be either scalars of size one or vectors/matrices of SIMD width
  elements as permitted  by the existing C for Metal semantics.

* Scalar control flow statements and conditional expressions are not permitted in SIMD control
  flow blocks.

* Jump statements other than the SIMD_BREAK and SIMD_CONTINUE statements are not allowed
  in a SIMD control flow blocks. Likewise SIMD_BREAK and SIMD_CONTINUE statements are not
  allowed in a non-SIMD context.

* Calling user-defined functions within a SIMD context is not permitted. There are no restrictions
  imposed on calling intrinsic functions provided none of the other constraints are violated.

* If it is not possible to determine the SIMD width of an statement in a SIMD context, then that
  statement is considered illegal in a SIMD context. For example an statement such as (scalar1 *
  scalar2) without a destination is considered illegal in SIMD context.

* The source operands of C for Metal reduction operators reduction operators (any(), all(), cm_sum(),
  cm_prod(), cm_reduction_max(), cm_reduction_min()) are required to have a size equal to the
  SIMD width of its immediate context. Furthermore reduction functions will perform reduction
  only on active scalar sub-elements of its source as determined by its context's SIMD mask.

* The dot product, the sum of absolute differences and the line operators are not allowed in SIMD
  control flow blocks as they operate on tuple variable which are not well-defined when we use
  SIMD masks.

* Block read/write, sampler, VME and thread communication intrinsics are not allowed in SIMD
  control flow context. It is however possible to use scatter read/write statements in a SIMD
  context provided the read/write destination/source size is equal to the SIMD width of the block.


4 Functions
===========

4.1 Function Qualifiers
-----------------------

C for Metal allows the user to write standard C++ functions. In addition, the following attribute qualifiers are used
to specify functions to be compiled for the graphics device. These cannot be used for the functions
running on the host.

* "_GENX_MAIN_" is used to specify a GenX kernel function, which cannot be called from another
  kernel function.

  _GENX_MAIN_ void kernel(formal_parameters)

  The formal parameter declarations must comply with the parameter passing restrictions described in
  Section 4.2. There can be more than one kernel function in a file.

* "_GENX_" may be used (but is not required) to specify a user defined GenX function, which can be
  called from kernel functions or other user defined functions.

  _GENX_ [inline] return_type function(formal\_ parameters)

  The optional "inline" qualifier is used to indicate if the function should be inlined by the C for Metal compiler at
  the call site. If a user defined function with the "inline" specifier is not inlined, the C for Metal compiler will
  issue a warning/error message.

A "genx_main" kernel function and all of the "genx" functions that it invokes transitively must be in the
same file.

4.2 User-defined functions
--------------------------

C for Metal supports user-defined function calls without recursion.  Parameters to a user-defined C for Metal function
may have scalar type, vector/matrix type, or  SurfaceIndex/SamplerIndex/VmeIndex type.  In addition,
vector_ref and matrix_ref types may be used to pass a vector/matrix object by reference.  A user-defined
function can return scalar values or vector/matrix types. vector_ref/matrix_ref return types are not
supported.

Function usage example:

.. literalinclude:: ../../../test/CMFE/cmlangspec/4_2_a.cpp
      :language: c++
      :lines: 20-65

In the pass-by-value parameter passing scheme a copy of the function call argument value is associated
with the corresponding subprogram parameter, whereas in the pass-by-reference scheme the address of
the argument is associated with the subprogram pass-by-reference parameter. In the subprogram body
every reference to the pass-by-reference formal becomes an indirect access using the address associated
with the parameter. The pass-by-reference arguments are therefore required to be l-values (named
objects having an address).  Pass-by-reference parameters are declared by specifying the parameter types
to be either of a matrix_ref or vector_ref type.  No other pass-by-reference parameter types are allowed.

The pass-by-reference scheme is useful to copy-out values from the subprogram back to the caller. It also
may be useful to convey large inputs to subprograms, as the amount of parameter conveying code will be
reduced.

There is a restriction imposed on arguments passed by reference in order to allow for an efficient CM
implementation. Specifically the restriction is that for a subprogram that uses pass-by-reference, the
behavior must be the same as if we use a copy-in/copy-out semantic to convey the pass-by-reference
argument; otherwise the C for Metal program is said to be erroneous and may produce incorrect results. Such
errors are not caught by the compiler and it's up to the user to guarantee safety.

The implication of the above stated restriction is that no pass-by-reference argument that is written to in
a subprogram (either directly or transitively by means of a nested subprogram call pass-by-reference
argument) may overlap with another pass-by-reference parameter or a global variable that is referenced
in the subprogram; in addition no pass-by-reference subprogram argument that is referenced may
overlap with a global variable that is written to in the subprogram.

The reason for the above rule is to allow the implementation to opportunistically use copy-in/copy-out
operations at the call/return sites in order to produce more optimal code by guaranteeing whole register
alignment to the subprogram parameter.

Pass-by-reference parameters may be semantically classified as:

| IN  - parameters that are only read
| OUT  - parameters that are only written
| INOUT - parameters that are both read and written

An important point to consider when using pass-by-reference parameters is that all reads and writes to
pass-by-reference parameters involve register-indirect operations in GEN ASM. Register-indirect
operations are limited to be SIMD8 (as opposed to SIMD16) in the GEN architecture. In addition there will
be additional address computation code generated. This can result in increased code size if there are a lot
of references to pass-by-ref parameters in the subprogram body. In addition, code involving register-
indirect operations may execute in hardware with more latency because of more conservative
dependency checks in hardware.

Another important point to consider is that arguments of pass-by-reference parameters are enforced to
be whole register aligned at call sites. If the argument cannot be guaranteed to be whole register aligned
then copy-in/copy-out operations are inserted which increases code size. In addition copy-in/copy-out
operations are also inserted if the argument is not contiguous to guarantee correctness; however this is
expected to be rare.

The implications are:

1.  If the IN parameter size is less than or equal to two GRFs it is better to use pass-by-value for it.

2.  OUT parameters are best updated only as a final statement in the subprogram body in order to
    reduce code involving register-indirect.

3.  If the arguments to most of the calls involving pass-by-reference IN parameters, are not whole
    register aligned then it is disadvantages to use pass-by-reference because of the compiler generated
    copy-in operations inserted to guarantee whole register alignment. Pass-by-value should probably
    not be used for such parameters. This is because the price paid involves both the copy-in/copy-out
    operations as well as the impact of the register-indirect operations. It's better to use pass-by-value
    for IN parameters in such situations.

4.  If the arguments to most of the call to pass-by-reference OUT parameters are not whole register
    aligned then it is disadvantages to use pass-by-reference for the same reason as for IN parameters.
    For OUT parameters a global variable could be used to convey the return value back to the caller (a
    user generated copy-out operation).

5.  If the arguments to most of the calls to pass-by-reference are discontinuous then pass-by-reference
    should not be used.

6.  If there are a lot of references to pass-by-reference parameters in the subprogram body, it may be
    disadvantageous to use pass-by-reference for the parameter as the amount of register-indirect code
    produced increases. On the other hand if the parameter size is large and there are a lot of call sites
    then it may be advantageous to use pass-by-reference for the parameter.

4.3 Built-in Functions
----------------------

C for Metal defines a set of built-in or intrinsic functions that are efficiently translated to GenX instructions based
on the target platform. It is advisable to use the intrinsic functions whenever possible to get better
performance. The intrinsic functions have the following format:

       Intrinsic_Function_Name<Template_Parameters> (Function\_ Parameters)

The template parameters cannot be variables and are usually optional as they are determined by the
compiler from the types of the function parameters. If present, template parameter 'T' represents the
data type of the return value. Most intrinsic functions have a flag parameter, which is a bit mask that
specifies the execution mode (e.g., the SAT bit indicates the saturation mode). Other bits in the flags are
reserved. The default value of the flag is 0.

Usually, the intrinsic functions have the same type restrictions as the corresponding GenX instructions.
Note that while a math intrinsic function returns a vector, CM allows assigning it to a matrix
with the same number of elements, as described in Section 2.5.

cm_abs<T>
^^^^^^^^^

Absolute value. The behavior is undefined if the result cannot fit in T.

.. code-block:: c++

  RetTy cm_abs(FstTy src0, int flag = _GENX_NOSAT)

There are several overloads for different types:

* Parameter 1: matrix(_ref), vector(_ref) or scalar;
* Parameter 2: flags (default is 0; use SAT for saturation);
* Return: vector or scalar;

Semantics:

#. Calculates absolute value of operand in infinite precision;
#. Saturates to destination type if flag is SAT;
#. Without saturation, for minimal signed value, result is minimal signed value.

cm_add<T>
^^^^^^^^^

cm_add currently is implemented in quite a strange way taking the worst parts from both C++ and our HW:

.. code-block:: c++

  RetTy cm_add(FstTy src0, SndTy src1, int flag = _GENX_NOSAT)

There are several overloads for different types:

* Parameter 1: matrix(_ref), vector(_ref) or scalar;
* Parameter 2: matrix(_ref), vector(_ref) or scalar;
* Parameter 3: flags (default is 0; use SAT for saturation);
* Return: vector or scalar.

Semantics:

#. Applies C++ integral promotions on its operands;
#. Extends operands to infinite precision;
#. Adds promoted operands;
#. Saturates to destination type if flag is SAT.

cm_addc
^^^^^^^

Add with carry.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: matrix_ref, vector_ref or reference to a scalar
* Return: vector or scalar

Only `unsigned` and `unsigned long long` type arguments are supported.

cm_subb
^^^^^^^

Subtraction with borrow.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: matrix_ref, vector_ref or reference to a scalar
* Return: vector or scalar

Only `unsigned` and `unsigned long long` type arguments are supported.

cm_mul<T>
^^^^^^^^^

Multiply.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

Restrictions:

1. The destination cannot be a float if any source operand is an integer;
2. If one source operand is a float, the other source operand cannot be an integer;
3. If one source operand is int/uint type, the SAT flag cannot be 1.

cm_quot<T>
^^^^^^^^^^

Quotient of division.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; no saturation available)
* Return: vector or scalar

Only integer type arguments are supported.

cm_mod<T>
^^^^^^^^^

Remainder (modulus) of division.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; no saturation available)
* Return: vector or scalar

Only integer type arguments are supported.

cm_fmod<T>
^^^^^^^^^^

Floating-point Remainder, same as C standard function fmod():
cm_fmod(y,x)=r, if y = qx + r where q is an integer and r<x.

NOTE: now part of cmtl; see :ref:`CMTemplateLibrary` (requires cmtl:: namespace)

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

Only floating-point type arguments are supported.

cm_div<T>
^^^^^^^^^

Quotient and remainder of division.

* Parameter 1: matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: matrix(_ref), vector(_ref) or scalar
* Parameter 4: flags (default is 0; no saturation available)
* Return: vector or scalar (quotient); the
  remainder is returned into the 1st parameter (passed by ref).

Only integer type arguments are supported.

cm_avg<T>
^^^^^^^^^

Average value rounded up.

* Parameter 1: matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

cm_dp2, cm_dp3, cm_dp4
^^^^^^^^^^^^^^^^^^^^^^

Two-, three- and four-wide dot product.

* Parameter 1: matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector

Only floating-point type arguments are supported.

Dot product, line and sum of difference operators are not permitted in a SIMD
control flow context.

cm_dp4 performs the 4-wide dot product operation for each 4-tuple of elements
in the input vector/matrix parameters, and sets the same scalar product result
to each element of the corresponding 4-tuple in the return value.

cm_dph
^^^^^^

Four-wide homogeneous dot product.

* Parameter 1: matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector

Only floating-point type arguments are supported.

Dot product, line and sum of difference operators are not permitted in a SIMD
control flow context.

cm_frc
^^^^^^

Fraction.

* Parameter 1: matrix(_ref), vector(_ref)
* Return: vector

Only single precision floating-point type arguments are supported.

This function returns a vector that contains the fractional portion of each component in the
input vector/matrix.

cm_line
^^^^^^^

Component-wise linear equation. First vector provides the scalar
coefficients -- Only 1st and 4th element of the vector are used as the two
scalar coefficients needed for this function.

* Parameter 1: vector of length 4
* Parameter 2: matrix(_ref), vector(_ref), multiple of 8 elements
* Parameter 3: flags (default is NOSAT; use SAT for saturation)
* Return: vector

This function computes the linear equation R[i] = X[0] * Y[i] + X[3] for each element Y[i] in the
second input parameter, and sets the result to the corresponding element R[i] in the return value.
The scalar values X[0] and X[3] used in the linear equation correspond to the first and fourth
elements of the 4-tuple in the first input parameter.

Only floating-point type arguments are supported.

Dot product, line and sum of difference operators are not permitted in a SIMD
control flow context.

cm_lzd
^^^^^^

Leading zero detection.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0)
* Return: vector or scalar

This function computes the number of leading zeros in each component of the
input parameter, and returns the result stored in a vector (for vector/matrix
input parameter) or a scalar (for scalar input parameter).

cm_max<T>, cm_min<T>
^^^^^^^^^^^^^^^^^^^^

Maximum and minimum.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: saturation (default is NOSAT; use SAT for saturation)
* Return: vector or scalar

cm_rndd
^^^^^^^

Round down.

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_floor<T>
^^^^^^^^^^^

floor(x) is the largest integer not greater than x -- same functionality as
cm_rndd.

NOTE: now part of cmtl; see :ref:`CMTemplateLibrary` (requires cmtl:: namespace)

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_rndu
^^^^^^^

Round up.

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_ceil<T>
^^^^^^^^^^

ceil(x) is the smallest integer not less than x -- same functionality as
cm_rndu.

NOTE: now part of cmtl; see :ref:`CMTemplateLibrary` (requires cmtl:: namespace)

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_rnde
^^^^^^^

Round even.

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_rndz
^^^^^^^

Round zero.

* Parameter 1: matrix(_ref), vector(_ref) or scalar.
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar.

cm_sad2<T>
^^^^^^^^^^

Two-wide sum of absolute difference.

* Parameter 1: matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

In a SIMD control flow context reduction functions will perform reduction only
on active channels of its source as determined by its context's SIMD mask.

Dot product, line and sum of difference operators are not permitted in a SIMD
control flow context.


cm_sada2<T>
^^^^^^^^^^^

Compute two-wide sum of absolute difference between src1 and src2, add that to
src3, and store the result to the first channel per 2-tuple in dst.

* Parameter 1: matrix(_ref) or  vector(_ref)
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: matrix(_ref) or vector(_ref)
* Parameter 4: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

In a SIMD control flow context reduction functions will perform reduction only
on active channels of its source as determined by its context's SIMD mask.

Dot product, line and sum of difference operators are not permitted in a SIMD
control flow context.

cm_sum<T>
^^^^^^^^^

Sum of all elements.

* Parameter 1: matrix(_ref) or vector(_ref).
* Parameter 2: saturation (default is NOSAT; use SAT for saturation).
* Return: scalar.

In a SIMD control flow context reduction functions will perform reduction only
on active channels of its source as determined by its context's SIMD mask.

Note: The order of continuant scalar operation is not guaranteed and the
correctness of result should not depend on computation order.

Note: The saturation has a known issue and does not guarantee the correct result.

cm_reduced_min<T>
^^^^^^^^^^^^^^^^^

Find the minimum element of a matrix/vector. T is the type of the elements.

* Parameter 1: matrix(_ref) or vector(_ref).
* Parameter 2: saturation (default is NOSAT; use SAT for saturation).
* Return: scalar.

In a SIMD control flow context reduction functions will perform reduction only
on active channels of its source as determined by its context's SIMD mask.

cm_reduced_max<T>
^^^^^^^^^^^^^^^^^

Find the maximum element of a matrix/vector. T is the type of the elements.

* Parameter 1: matrix(_ref) or vector(_ref).
* Parameter 2: saturation (default is NOSAT; use SAT for saturation).
* Return: scalar.

In a SIMD control flow context reduction functions will perform reduction only
on active channels of its source as determined by its context's SIMD mask.

cm_prod<T>
^^^^^^^^^^

Product of all elements.

* Parameter 1: matrix(_ref) or vector(_ref).
* Parameter 2: saturation (default is NOSAT; use SAT for saturation).
* Return: scalar4.

Restrictions:

1. The destination cannot be a float if any source operand is an integer;
2. If one source operand is a float, the other source operand cannot be an integer;
3. If one source operand is int/uint type, the SAT flag cannot be 1.

The order of continuant scalar operation is not guaranteed and the
correctness of result should not depend on computation order.

This operation is currently not supported in the SIMD control flow context.

cm_inv
^^^^^^

Inversion (dst = 1.0/src).

* Parameter 1: matrix(_ref), vector(_ref) or
  scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

cm_log
^^^^^^

Logarithm of base 2.
* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

Restrictions:

1. The destination cannot be a floating-point type if any source operand is an integer;
2. If one source operand is a floating-point type, the other source operand cannot be an integer;
3. If one source operand is int/uint type, the SAT flag cannot be 1.
4. double-precision floating-point types are not supported.

cm_exp
^^^^^^

Exponent of base 2.

* Parameter 1: matrix(_ref), vector(_ref) or
  scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

cm_sqrt
^^^^^^^

Square root.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

cm_sqrt_ieee
^^^^^^^^^^^^

Square root with IEEE compliant semantics.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only single or double precision floating-point type arguments are supported.

cm_rsqrt
^^^^^^^^

dst = 1.0/sqrt(src).

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

Half, single and double precision floating-point type arguments are supported.

cm_pow
^^^^^^

Power.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: scalar for Gen6, matrix(_ref),
  vector(_ref) or scalar for Gen6.
* Parameter 3: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half or single precision floating-point type arguments are supported.

cm_sin
^^^^^^

Sine.
* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

cm_cos
^^^^^^

Cosine.

* Parameter 1: matrix(_ref), vector(_ref) or
  scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

cm_sincos
^^^^^^^^^

Sine and cosine.

* Parameter 1:  matrix_ref, vector_ref
* Parameter 2: matrix(_ref) or vector(_ref)
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector or matrix (sine) and cosine
  into the 1st parameter (passed by ref)

Only half and single precision floating-point type arguments are supported.

cm_asin
^^^^^^^

Inverse function of sine.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only floating-point type arguments are supported.

cm_acos
^^^^^^^

Inverse function of cosine.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only floating-point type arguments are supported.

cm_atan
^^^^^^^

Inverse function of tangent.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only floating-point type arguments are supported.

cm_atan2
^^^^^^^^

Same as C standard function atan2().

NOTE: now part of cmtl; see :ref:`CMTemplateLibrary` (requires cmtl:: namespace)

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation).
* Return: matrix, vector or scalar

Only floating-point type arguments are supported.

cm_atan2_fast
^^^^^^^^^^^^^

Faster version of cm_atan2, but has a lower precision -- only accurate up to
0.01, where cm_atan2 is precise up to 0.00001.

NOTE: now part of cmtl; see :ref:`CMTemplateLibrary` (requires cmtl:: namespace)

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation).
* Return: matrix, vector or scalar

Only floating-point type arguments are supported.

cm_div_ieee
^^^^^^^^^^^

Division with IEEE compliant semantics.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector or scalar.

Only single or double precision floating-point type arguments are supported.

cm_frem
^^^^^^^

The floating-point remainder with correctly rounded IEEE compliant semantics.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: matrix(_ref), vector(_ref) or scalar
* Return: vector or scalar.

Only single or double precision floating-point type arguments are supported.

cm_tanh
^^^^^^^

Hyperbolic tangent.

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation).
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

These functions are target-dependent and only available
when the ``CM_HAS_TANH`` macro is defined.

cm_sigmoid
^^^^^^^^^^

Sigmoid function:

.. math::
  \sigma(x) = 1 \over {1 + e^{-x}}

* Parameter 1: matrix(_ref), vector(_ref) or scalar
* Parameter 2: flags (default is 0; use SAT for saturation)
* Return: vector or scalar

Only half and single precision floating-point type arguments are supported.

These functions are target-dependent and only available
when the ``CM_HAS_SIGMOID`` macro is defined.

cm_imul
^^^^^^^

Hi 32 bits and low 32 bits of the 64-bit result of integer multiply.

* Parameter 1:  matrix(_ref), vector(_ref)
* Parameter 2: matrix(_ref) or vector(_ref) or scalar
* Parameter 3: matrix(_ref) or vector(_ref) or scalar
* Parameter 4: flags (default is 0; no saturation available)
* Return: vector or scalar (hi 32 bits); the
  low 32 bits is returned into the 1st
  parameter (passed by ref)

Only integer type arguments are supported.

cm_pack_mask
^^^^^^^^^^^^

Pack a vector / matrix mask into an integer

* Parameter 1:  matrix(_ref), vector(_ref) .
* Return: scalar.

The vector/matrix data type must be unsigned short, and the size must be 8, 16, or 32.

This operation is currently not supported in the SIMD control flow context.

cm_unpack_mask<T, SZ>
^^^^^^^^^^^^^^^^^^^^^

Unpack an integer to a vector mask

* Parameter 1:  scalar.
* Return: vector

The vector/matrix data type must be unsigned short, and the size must be 8, 16, or 32.

This operation is currently not supported in the SIMD control flow context.

cm_cbit
^^^^^^^

Count component-wise the total bits set in source operand .

* Parameter 1:  matrix(_ref), vector(_ref) or scalar.
* Return: vector or scalar.

The source operand must be of "int" type. The destination operand must be of
"unsigned int" type.

cm_fbl
^^^^^^

Find component-wise the first bit from LSB side.

* Parameter 1:  matrix(_ref), vector(_ref) or scalar
* Return: vector or scalar

The source operand must be of integer type.
The destination operand must be of "unsigned int" type.
If the source operand is equal to 0, returns 0xFFFFFFFF.

cm_fbh
^^^^^^

Find component-wise the first bit from MSB side.

* Parameter 1:  matrix(_ref), vector(_ref) or scalar.
* Return: vector.

Both the source and destination operands must be of the same "int" or "unsigned
int" type. If the source operand type is unsigned, returns the count of leading
zeros from the MSB side. If the source operand type is signed and positive,
returns the count of leading zeros from MSB side. If the operand type is signed
and negative, returns the count of leading ones from MSB side. If the source
operand is equal to 0 and the type is unsigned, returns 0xFFFFFFFF. If the
source operand is equal to 0 or 0xFFFFFFFF and the type is signed, returns
0xFFFFFFFF.

cm_clock
^^^^^^^^

Returns the low and high parts of the 64-bit timestamp as a vector of 2 32-bit
values.

cm_rdtsc
^^^^^^^^

Returns time stamp information as a vector of 4 32bit values: 1st and 2nd are
the low and high bits of the 64bit timestamp, 3rd contain a bit that is set
when a context switch occurs, and the 4th is unused.

Note: The function is deprecated. cm_clock should be used instead.

* Return: vector

The return vector data type must be unsigned int.

cm_shr
^^^^^^

Perform right bit shift. The sign of parameter 1 dictates what operation is actually carred out and
this matches the behavior of the >> operator (lsr for unsigned, asr for signed)

* Parameter 1:  matrix(_ref), vector(_ref)
* Parameter 2:  matrix(_ref), vector(_ref)
* Parameter 3:  saturation (default is NOSAT; use SAT for saturation)
* Return: vector

Only bits 0-5 are read from second operand, MSBs are disregarded.

Note: It is usually simpler to use the >> operator with appropriate cast to an unsigned or signed
type to get the desired behavior from >>, followed by an optional saturation intrinsic to achieve
the same effect for the same cost and with clearer meaning.

cm_lsr
^^^^^^

Perform logical right bit shift.

* Parameter 1:  matrix(_ref), vector(_ref)
* Parameter 2:  matrix(_ref), vector(_ref)
* Parameter 3:  saturation (default is NOSAT; use SAT for saturation)
* Return: vector

The semantics of the operation are the usual integer promotions but with an implicit cast to an
unsigned type prior to the shift. This can be described by the following pseudo code:

* Parameter 1 is cast according to integer promotion rules for bitwise shift (e.g. char -> int)
* Result of this case is then cast to equivalent unsigned type (e.g. int -> unsigned int)
* Result of this is then shifted (e.g. Val >> Parameter 2)
* Result is then converted to the result type with optional saturation

Only bits 0-5 are read from second operand, MSBs are disregarded.

Note: It is usually simpler to use the >> operator with appropriate cast to an unsigned type,
followed by an optional saturation intrinsic to achieve the same effect for the same cost and with
clearer meaning.

cm_asr
^^^^^^

Perform arithmetic right bit shift.

* Parameter 1:  matrix(_ref), vector(_ref)
* Parameter 2:  matrix(_ref), vector(_ref)
* Parameter 3:  saturation (default is NOSAT; use SAT for saturation)
* Return: vector

The semantics of the operation are the usual integer promotions but with an implicit cast to a
signed type prior to the shift. This can be described by the following pseudo code:

* Parameter 1 is cast according to integer promotion rules for bitwise shift (e.g. unsigned char -> unsigned int)
* Result of this case is then cast to equivalent signed type (e.g. unsigned int -> int)
* Result of this is then shifted (i.e. Val >> Parameter 2)
* Result is then converted to the result type with optional saturation

Only bits 0-5 are read from second operand, MSBs are disregarded.

Note: It is usually simpler to use the >> operator with appropriate cast to a signed type,
followed by an optional saturation intrinsic to achieve the same effect for the same cost and with
clearer meaning.

cm_shl
^^^^^^

Perform logical left bit shift.

* Parameter 1:  matrix(_ref), vector(_ref)
* Parameter 2:  matrix(_ref), vector(_ref)
* Parameter 3:  saturation (default is NOSAT; use SAT for saturation), if
  33 bits are over flown behavior is
  undefined. Only lower 5 bits of src 2
  are used for shifting.
* Return: vector

Parameter 1 should be of unsigned type, otherwise behavior is unspecified.

Only bits 0-5 are read from second operand, MSBs are disregarded.

cm_pln
^^^^^^

Component-wise plane function:
dst[i] =
src0[0] * src1[i] +
src0[1] * src2[i] +
src0[3]

* Parameter 1: vector of length 4
* Parameter 2: matrix(_ref) or vector(_ref)
* Parameter 3: matrix(_ref) or vector(_ref)
* Parameter 4: saturation (default is NOSAT; use SAT for saturation)
* Return: vector.

This is not allowed in SIMD Control Flow context. The size of destination,
2nd and 3rd source operands must be a multiple of 8. The operand type must be
float.

cm_lrp
^^^^^^

Component-wise linear interpolation function:
dst[i] =
src1[i]*src0[i] +
src2[i]*(1.0 - src0[i])

* Parameter 1: matrix(_ref) or vector(_ref)
* Parameter 2: matrix(_ref) or vector(_ref)
* Parameter 3: matrix(_ref) or vector(_ref)
* Return: vector.

This is not allowed in SIMD Control Flow context. The operand type must be
float and the size must be a multiple of 4.

cm_bf_insert
^^^^^^^^^^^^

Bitfield insert.

Dst = src with bits defined as width bits starting at offset
replaced with value.

Only int and uint
supported.

* Parameter 1: width (vector)
* Parameter 2: offset (vector)
* Parameter 3: val (vector)
* Parameter 4: src (vector)
* Return: vector

cm_bf_extract
^^^^^^^^^^^^^

Bitfield extract.

Dst = field from src
at offset for width
bits.

Dst sign extended
for signed types.

Only int and uint
supported.

* Parameter 1: width (vector)
* Parameter 2: offset (vector)
* Parameter 3: src (vector)
* Return: vector

cm_dp4a
^^^^^^^

Four-wide integer dot product and accumulate operation.

* Parameter 1: vector(_ref)
* Parameter 2: vector(_ref)
* Parameter 3: vector(_ref)
* Parameter 3: flags (default is 0; use SAT for saturation)
* Return: vector

Only int/uint type arguments are supported

Each source1's 32-bit channel value and source2's 32-bit channel value is
treated as four element vector of 8-bit integer values. cm_dp4a performs a
32-bit precision dot product of those four bytes and adds it with source0.

These functions are target-dependent and only available
when ``CM_HAS_DP4A`` macro is defined.

cm_bf_cvt
^^^^^^^^^

Bfloat16 to Float or Float to Bfloat16 conversion.

* Template parameter 1: Destination type

* Parameter 1: vector/matrix/scalar
* Return: vector

Only half (used to represent bfloat16 internally) and float type are
supported. If source is half, destination must be float. Otherwise
destination must be half.

Mixed mode operation can be enabled by using cm_bf_cvt to convert a Bfloat16
type operand to Float type, then use the converted operand in a FP operation.

These functions are target-dependent and only available
when ``CM_HAS_BF16`` macro is defined.

cm_bf8_cvt
^^^^^^^^^^

BF8 to HF or HF to BF8 conversion.

* Template parameter 1: Destination type

* Parameter 1: vector/matrix/scalar
* Parameter 2: flags (default is 0; use SAT for saturation);
* Return: vector

Only uchar (used to represent BF8 internally) and HF type are supported. If
source is uchar, destination must be HF. Otherwise destination must be uchar.

These functions are target-dependent and only available
when ``CM_HAS_BF8`` macro is defined.

Note: SAT must be compiler time constant for saturation.


cm_hf8_cvt
^^^^^^^^^^

HF8 to HF or HF to HF8 conversion.

.. code-block:: c++

  template <typename DstTy>
  vector<DstTy, N> cm_hf8_cvt(vector<SrcTy, N> src0, int flag = _GENX_NOSAT);

  template <typename DstTy>
  vector<DstTy, N1 * N2> cm_hf8_cvt(matrix<SrcTy, N1, N2> src, int flag = _GENX_NOSAT);

  template <typename DstTy>
  DstTy cm_hf8_cvt(SrcTy src, int flag = _GENX_NOSAT);


============== =================================================================
Parameters     Description
============== =================================================================
DstTy          Destination type, must be ``half`` or ``char``.
               The latter is used to represent HF8 internally.
               When destination type is ``half``, source must be ``char``.

SrcTy          Source type, must be ``half`` or ``char``.
               The latter is used to represent HF8 internally.
               When source type is ``char``, destination must be ``half``.

N              SIMD width of the operation.

N1, N2         Height and width of input matrix respectively.
               The size of the matrix (N1*N2) must be equal to SIMD width N.

flag           Saturation flag, default is 0. Use SAT for saturation.
============== =================================================================

These functions are target-dependent and only available
when ``CM_HAS_HF8`` macro is defined.

Note: SAT must be compiler time constant for saturation.


cm_tf32_cvt
^^^^^^^^^^^

Float to Tfloat32 conversion.

* Template parameter 1: Destination type

* Parameter 1: vector/matrix/scalar
* Return: vector

Only int (used to represent tf32 internally) and float type are
supported.

Usage Examples:

.. code-block:: c++

  vector<float, 16> float_val;
  read(IN, 0, float_val);
  // convert float -> ctf32
  vector<int, 16> tf32_out = cm_tf32_cvt<float>(float_val);

These functions are target-dependent and only available
when ``CM_HAS_TF32`` macro is defined.

cm_srnd
^^^^^^^

Stochastic converts f32->fp16, fp16->bf8 (Xe3+), f32->bf8 (Xe3+)

* Template parameter T: Destination type (char or half)

* Parameter 1: Data to convert vector/matrix/scalar (float or half)

* Parameter 2: "random" data, type is the same as parameter 1 or
  integer of the same bit width as destination type;

* Parameter 3: flags (default is 0; use SAT for saturation);

* Return: vector<T, N>

Usage Examples:

.. code-block:: c++

  vector<half, 16> H_srnd12;
  vector<char, 16> H_srnd22;
  read(IN, 0, H_srnd12);
  read(IN, 8, H_srnd22);
  // Stochastic convert half -> bf8
  vector<char, 16> H_srnd_out2 = cm_srnd<char>(H_srnd12, H_srnd22);

  vector<float, 16> F_srnd12;
  vector<short, 16> F_srnd22;
  read(IN, 0, F_srnd12);
  read(IN, 8, F_srnd22);
  // Stochastic convert f32 -> half (f16)
  vector<half, 16> F_srnd_out2 = cm_srnd<half>(F_srnd12, F_srnd22);

These functions are target-dependent and only available
when ``CM_HAS_STOCHASTIC_ROUNDING`` macro is defined.


cm_srnd_bf8
^^^^^^^^^^^

Operation for converting ``float`` and  ``half`` type values into ``bfloat8``
with stochastic rounding.

.. code-block:: c++

  template <typename SrcTy, unsigned Width>
  vector<uint8_t, Width> cm_srnd_bf8(vector<SrcTy, Width> Src,
                                    vector<uint8_t, Width> Bias,
                                    int Flag);

  template <typename SrcTy, unsigned Height, unsigned Width>
  matrix<uint8_t, Height, Width> cm_srnd_bf8(matrix<SrcTy, Height, Width> Src,
                                             matrix<uint8_t, Height, Width> Bias,
                                             int Flag);

  template <typename SrcTy>
  uint8_t cm_srnd_bf8(SrcTy Src, uint8_t Bias, int Flag);

============== =================================================================
Parameters     Description
============== =================================================================
SrcTy          Source type, must be ``float``, ``half`` or ``bfloat16``.
               When source type is ``float``, it's converted to ``half`` first,
               and then ``half`` to ``bfloat8``.

Width          SIMD width of the operation.

Height         Height of input and output matrices.

Bias           Stochastic rounding bias.

Flag           Saturation flag, default is 0. Use SAT for saturation.
============== =================================================================

These functions are target-dependent and only available when:
``CM_HAS_SRND_BF16_TO_BF8`` macro is defined and ``SrcTy`` is ``bfloat16``.
``CM_HAS_SRND_FP16_TO_BF8`` macro is defined and ``SrcTy`` is ``float`` or  ``half``.

cm_srnd_hf8
^^^^^^^^^^^

Operation for converting ``float``,  ``half`` and ``bfloat16`` type values into ``hfloat8``
with stochastic rounding.

.. code-block:: c++

  template <typename SrcTy, unsigned Width>
  vector<uint8_t, Width> cm_srnd_hf8(vector<SrcTy, Width> Src,
                                    vector<uint8_t, Width> Bias,
                                    int Flag);

  template <typename SrcTy, unsigned Height, unsigned Width>
  matrix<uint8_t, Height, Width> cm_srnd_hf8(matrix<SrcTy, Height, Width> Src,
                                             matrix<uint8_t, Height, Width> Bias,
                                             int Flag);

  template <typename SrcTy>
  uint8_t cm_srnd_hf8(SrcTy Src, uint8_t Bias, int Flag);

============== =================================================================
Parameters     Description
============== =================================================================
SrcTy          Source type, must be ``float``,  ``half`` and ``bfloat16``.

Width          SIMD width of the operation.

Height         Height of input and output matrices.

Bias           Stochastic rounding bias.

Flag           Saturation flag, default is 0. Use SAT for saturation.
============== =================================================================

These functions are target-dependent and only available when:
``CM_HAS_SRND_BF16_TO_HF8`` macro is defined and ``SrcTy`` is ``bfloat16``.
``CM_HAS_SRND_FP16_TO_HF8`` macro is defined and ``SrcTy`` is ``float`` or  ``half``.


cm_upconvert_4bit_lut
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <int Index, typename SrcTy, unsigned NumSrcElements>
  vector<uint32_t, Width> cm_upconvert_4bit_lut(vector<uint32_t, 16> LUT,
                                                vector<SrcTy, NumSrcElements> Src);

Up-conversion from 4-bit (fp4 or int4) into 8 or 16-bit values using lookup table.
The operation performs strided access to the packed 4-bit source operand.

============== =================================================================
Parameters     Description
============== =================================================================
Index          The source element index within a DWord. Must be compile-time
               constant.

SrcTy          Storage type for packed 4-bit input values. Must be ``uchar`` for
               4-to-16 bit conversion or ``ushort`` for 4-to-8 bit conversion.

NumSrcElements Number of elements of SrcTy in the source vector. Shouldn't be
               passed explicitly, as it is deduced from the source vector type.
               Should be equal to ``Stride * Width``.

Stride         Source vector access stride. The operation access single element
               of ``SrcTy`` per dword, so the ``Stride`` value must be equal to
               number of ``SrcTy`` elements in dword. Deduced. Cannot be passed
               explicitly. When ``SrcTy`` is ``uchar``, the stride is 4. When
               ``SrcTy`` is ``ushort``, the stride is 2.

Width          SIMD width of the operation. The only supported values are 16
               and 32. Cannot be passed explicitly. Deduced as the number of
               elements in the source vector divided by the stride.

LUT            Lookup table used for the upconvert operation. The table must
               contain 16 elements.

Src            Vector of the packed input values.
============== =================================================================

These functions are target-dependent and only available when
``CM_HAS_UPCONVERT_4BIT_LUT`` macro is defined.

Usage example: upconvert VNNI-packed 8x16 fp4 block into bfloat16.

.. code-block:: c++

  vector<uint32_t, 16> LUT = {0x00000000, 0x3f803f80, 0x40004000, 0x40804080,
                              0x41004100, 0x41804180, 0x42004200, 0x42804280,
                              0x7fff7fff, 0xbf80bf80, 0xc000c000, 0xc080c080,
                              0xc100c100, 0xc180c180, 0xc200c200, 0xc280c280};
  vector<uint32_t, 16> Src;
  matrix<uint32_t, 4, 16> Dst;

  vector_ref<uint8_t, 16 * 4> SrcByte = Src.template format<uint8_t>();

  Dst.row(0) = cm_upconvert_4bit_lut<0>(LUT, SrcByte);
  Dst.row(1) = cm_upconvert_4bit_lut<1>(LUT, SrcByte);
  Dst.row(2) = cm_upconvert_4bit_lut<2>(LUT, SrcByte);
  Dst.row(3) = cm_upconvert_4bit_lut<3>(LUT, SrcByte);

Usage example: upconvert VNNI-packed 8x16 fp4 block into bfloat8.

.. code-block:: c++

  vector<uint32_t, 16> LUT = {0x00000000, 0x3c3c3c3c, 0x40404040, 0x44444444,
                              0x48484848, 0x4c4c4c4c, 0x50505050, 0x54545454,
                              0x7f7f7f7f, 0xbcbcbcbc, 0xc0c0c0c0, 0xc4c4c4c4,
                              0xc8c8c8c8, 0xcccccccc, 0xd0d0d0d0, 0xd4d4d4d4};
  vector<uint32_t, 16> Src;
  matrix<uint32_t, 2, 16> Dst;

  vector_ref<uint16_t, 16 * 2> SrcWord = Src.template format<uint16_t>();

  Dst.row(0) = cm_upconvert_4bit_lut<0>(LUT, SrcWord);
  Dst.row(1) = cm_upconvert_4bit_lut<1>(LUT, SrcWord);

Usage example: upconvert VNNI-packed 8x32 fp4 block into bfloat8.

.. code-block:: c++

  vector<uint32_t, 16> LUT = {0x00000000, 0x3c3c3c3c, 0x40404040, 0x44444444,
                              0x48484848, 0x4c4c4c4c, 0x50505050, 0x54545454,
                              0x7f7f7f7f, 0xbcbcbcbc, 0xc0c0c0c0, 0xc4c4c4c4,
                              0xc8c8c8c8, 0xcccccccc, 0xd0d0d0d0, 0xd4d4d4d4};
  vector<uint32_t, 32> Src;
  matrix<uint32_t, 2, 32> Dst;

  vector_ref<uint16_t, 32 * 2> SrcWord = Src.template format<uint16_t>();

  Dst.row(0) = cm_upconvert_4bit_lut<0>(LUT, SrcWord);
  Dst.row(1) = cm_upconvert_4bit_lut<1>(LUT, SrcWord);


cm_downscale
^^^^^^^^^^^^

.. code-block:: c++

  template <downscale::Type OutputTy, downscale::Mode Mode, typename InputTy,
            unsigned Width,
            unsigned IntrWidth = Width * sizeof(InputTy) / sizeof(uint32_t)>
  vector<uint32_t, IntrWidth> cm_downscale(vector<InputTy, Width> Src0,
                                           vector<InputTy, Width> Src1);

  template <downscale::Type OutputTy, downscale::Mode Mode, typename InputTy,
            unsigned Width,
            unsigned IntrWidth = Width * sizeof(InputTy) / sizeof(uint32_t)>
    vector<uint32_t, IntrWidth> cm_downscale(vector<InputTy, Width> Src0,
                                           vector<InputTy, Width> Src1,
                                           vector<uint32_t, IntrWidth> Bias);

Downscale operation for converting ``half`` and ``bfloat16`` values into 4-bit
floating point or integer values.

============== =================================================================
Parameters     Description
============== =================================================================
OutputTy       Output type of the downscale operation. Must be one of the
               following:

               * ``downscale::E2M1`` - 4-bit floating point with 1 sign bit, 2
                 exponent bits and 1 mantissa bit.
               * ``downscale::Int4`` - 4-bit signed integer.

Mode           Downscale mode (see the pseudocode below). Must be one of the
               following:

               * ``downscale::Mode0``
               * ``downscale::Mode1``
               * ``downscale::Mode2``
               * ``downscale::Mode3``

InputTy        Input type of the downscale operation. Must be ``half``,
               ``bfloat16`` or ``int16_t``. The ``int16_t`` is used as
               a storage for ``bfloat16`` values. Can be omitted.

Width          The width of the input vectors. Must be multiple of 2. Can be
               omitted.

Src0           The first input vector. Must be of the same type as the
               ``InputTy`` template parameter.

Src1           The second input vector. Must be of the same type as the
               ``InputTy`` template parameter.

Bias           The bias vector. The width of the bias vector must be half the
               width of the Src0 and Src1 vectors. Optional. If provided,
               stochastic/biased rounding is performed. If omitted, rounding to
               nearest even is performed.
============== =================================================================

These functions are target-dependent and only available when
``CM_HAS_DOWNSCALE_4BIT`` macro is defined.

The downscale operation converts the input vectors into 4-bit floating point or
integer values. The operation is performed in the following way:

.. code-block:: c++

  if (Bias is present) {
    Rounding = Stochastic;
  } else {
    Rounding = NearestOrEven;
    Bias = Undefined;
  }

  for (int I = 0; I < Width / 2; I++) {
    BiasLow = Bias[I] & 0x0000FFFF;
    BiasHigh = Bias[I] >> 16;

    switch (Mode) {
    case downscale::Mode0:
      Res[I] = Downscale(Src0[2 * I], BiasLow, Rounding)
             | Downscale(Src0[2 * I + 1], BiasHigh, Rounding) << 4
             | Downscale(Src1[2 * I], BiasLow, Rounding) << 16
             | Downscale(Src1[2 * I + 1], BiasHigh, Rounding) << 20;
      break;
    case downscale::Mode1:
      Res[I] = Downscale(Src0[2 * I], BiasLow, Rounding)
             | Downscale(Src1[2 * I], BiasHigh, Rounding) << 4
             | Downscale(Src0[2 * I + 1], BiasLow, Rounding) << 16
             | Downscale(Src1[2 * I + 1], BiasHigh, Rounding) << 20;
      break;
    case downscale::Mode2:
      Res[I] = Downscale(Src0[2 * I], BiasLow, Rounding) << 8
             | Downscale(Src0[2 * I + 1], BiasHigh, Rounding) << 12
             | Downscale(Src1[2 * I], BiasLow, Rounding) << 24
             | Downscale(Src1[2 * I + 1], BiasHigh, Rounding) << 28;
      break;
    case downscale::Mode3:
      Res[I] = Downscale(Src0[2 * I], BiasLow, Rounding) << 8
             | Downscale(Src1[2 * I], BiasHigh, Rounding) << 12
             | Downscale(Src0[2 * I + 1], BiasLow, Rounding) << 24
             | Downscale(Src1[2 * I + 1], BiasHigh, Rounding) << 28;
      break;
    }
  }

cm_mxfp_reduce
^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Ty>
  vector<Ty, 32> cm_mxfp_reduce(matrix<Ty, 32, 32> Src);

Block max reduction operation for 32x32 matrix. The operation calculates the
maximum of absolute values for each 32-element row of the input matrix.
The result is a 32-element vector. The supported types are ``half`` and
``bfloat16``. The order of the elements in the result vector is determined by
a predefined interleaved indexing pattern.

The operation is performed in the following way:

.. code-block:: c++

  const vector<int, 32> Indices = {
    0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30,
    1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
  };

  vector<Ty, 32> Res;
  for (int I = 0; I < 32; I++) {
    auto Index = Indices[I];
    auto Row = Src.row(Index);
    Res[I] = cm_reduced_max<Ty>(cm_abs<Ty>(Row));
  }

The function is only available when the ``CM_HAS_MXFP_REDUCE`` macro is defined.

cm_mxfp_linearize
^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Ty>
  vector<Ty, 32> cm_mxfp_linearize(vector<Ty, 32> Src);

Linearization operation for 32-element vector. The operation shuffles the
vector produced by the ``cm_mxfp_reduce`` operation to get the linear order
of the elements. The supported types are ``half`` and ``bfloat16``.

The operation is performed in the following way:

.. code-block:: c++

  const vector<int, 32> Indices = {
    0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30,
    1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
  };

  vector<Ty, 32> Res = Src.iselect(Indices);

The function is only available when the ``CM_HAS_MXFP_REDUCE`` macro is defined.

cm_lfsr
^^^^^^^

.. code-block:: c++

  template <typename Ty, unsigned Width>
  vector<Ty, Width> cm_lfsr(vector<Ty, Width> Seed, vector<Ty, Width> Poly);

  template <typename Ty, unsigned Width>
  vector<Ty, Width> cm_lfsr(vector<Ty, Width> Seed,
                            vector<Ty, sizeof(uint) / sizeof(Ty)> Poly);

  template <typename Ty, unsigned Width>
  vector<Ty, Width> cm_lfsr(vector<Ty, Width> Seed, Ty Poly);

Linear Feedback Shift Register (LFSR) operation. The function performs a step
of the Galois LFSR with the given polynomial. For each element of the input,
the function calculates the next state of the LFSR and returns it as follows:

.. code-block:: c++

  for (int I = 0; I < Width; I++) {
    LSB = Seed[I] & 1;
    Seed[I] >>= 1;
    if (LSB)
      Seed[I] ^= Poly[I];
  }

========== ====================================================================
Parameters Description
========== ====================================================================
Ty         Type of the LFSR seed and polynomial. Must be ``char``, ``uchar``,
           ``short``, ``ushort``, ``int`` or ``uint``. May be omitted.

Width      SIMD width of the operation. May be omitted. The ``Width``
           multiplied by ``sizeof(Ty)`` must be multiple of 4.

Seed       Initial LFSR state. The seed must be non-zero.

Poly       LFSR polynomial. The polynomial is represented as a bitmask of
           the polynomial coefficients. The polynomial must be non-zero.
           The polynomial can be one of the following:

           * A vector of the same type as the seed with the same width.

           * A vector of the same type as the seed with the width equal to
             the number of elements which can fit in a DWord.

           * A scalar of the same type as the seed.
========== ====================================================================

The function is only available when the ``CM_HAS_LFSR`` macro is defined.


cm_bfn
^^^^^^

.. code-block:: c++

  template <BFNT BVAL, typename Ty>
  Ty cm_bfn(Ty Src0, Ty Src1, Ty Src2);

  template <BFNT BVAL, typename Ty, int N>
  vector<Ty, N> cm_bfn(vector<Ty, N> Src0, vector<Ty, N> Src1, vector<Ty, N> Src2);

Boolean function calculation.

Performs specified boolean logical operation with 3 sources.

========== ====================================================================
Parameters Description
========== ====================================================================
Ty         Type of the boolean function. Must be ``short``, ``ushort``, ``int``
           or ``uint``. May be omitted.

BVAL       Function index (boolean expression from BFNT enum values).
           BFNT values: BFN_X, BFN_Y, BFN_Z (correspond to s0, s1 and s2). Any
           boolean expression with these values and ~, &, | and ^ operators is
           allowed.

Src0       The first input. Must be of the same type as ``Ty``.

Src1       The second input. Must be of the same type as ``Ty``.

Src2       The third input. Must be of the same type as ``Ty``.
========== ====================================================================

All parameter types must be the same. Mix of vector and vector_ref is allowed
-- vector_ref will be implicitly converted to vector type.

The function is only available when the ``CM_HAS_BFN`` macro is defined.

Example:

.. code-block:: c++

  d = cm_bfn<~BFN_X & ~BFN_Y & ~BFN_Z>(s0, s1, s2)

cm_bf_reverse
^^^^^^^^^^^^^

.. code-block:: c++

  template <typename T0, typename T1, int SZ>
  vector<T0, SZ> cm_bf_reverse(vector<T1, SZ> src);

  template <typename T0, typename T1, int N1, int N2>
  vector<T0, N1 *N2> cm_bf_reverse(matrix<T1, N1, N2> src);

  template <typename T0, typename T1>
  T0 cm_bf_reverse(T1 src);

Bitfield reverse.

========== ====================================================================
Parameters Description
========== ====================================================================
T0         Output type. It's unsigned 32-bit function, so if the output type
           is not ``uint``, ``uint`` will be converted to the output type.
           The template parameter can be omitted.

T1         Input type. It's unsigned 32-bit function, so if the input type
           is not ``uint``, it will be converted ``uint`` first.
           The template parameter can be omitted.

SZ         Vector size.
           The template parameter can be omitted.

N1         Matrix height.
           The template parameter can be omitted.

N2         Matrix width.
           The template parameter can be omitted.

src        The input. Must be of the same type as ``T0``.
========== ====================================================================

An example of intrinsic function usage:

.. literalinclude:: ../../../test/CMFE/cmlangspec/4_3_a.cpp
      :language: c++
      :lines: 22-46

4.4 Common Types For Memory Operations
--------------------------------------

**ChannelMaskType** -- an
enumeration constant that specifies which of the R,G,B,A channels should be
enabled for a dataport or sampler operation.  At least one of the channels must be
activated, and there are 15 possible values:

* CM_R_ENABLE
* CM_G_ENABLE
* CM_GR_ENABLE
* CM_B_ENABLE
* CM_BR_ENABLE
* CM_BG_ENABLE
* CM_BGR_ENABLE
* CM_A_ENABLE
* CM_AR_ENABLE
* CM_AG_ENABLE
* CM_AGR_ENABLE
* CM_AB_ENABLE
* CM_ABR_ENABLE
* CM_ABG_ENABLE
* CM_ABGR_ENABLE

**CmAtomicOpType** -- an enumeration constant that specifies the operation performed for the various
atomic read-modify-write library functions.  The atomic operation and the returned result for a single
destination location are described in the table below. The new value of the destination (new_dst) is
computed as indicated based on the old value of the destination (old_dst) and up to two sources included
in the message (src0 and src1).  All operations below, except ATOMIC_MAXSINT and ATOMIC_MINSINT,
treat all values as unsigned integers.

======================= ======================================= ====================
Atomic Operation        New Value at address                    Return value
======================= ======================================= ====================
ATOMIC_AND              old_dst & src0                          old_dst
ATOMIC_OR               old_dst | src0                          old_dst
ATOMIC_XOR              old_dst ^ src0                          old_dst
ATOMIC_XCHG             src0                                    old_dst
ATOMIC_INC              old_dst + 1                             old_dst
ATOMIC_DEC              old_dst - 1                             old_dst
ATOMIC_ADD              old_dst + src0                          old_dst
ATOMIC_SUB              old_dst - src0                          old_dst
ATOMIC_REVSUB           src0 - old_dst                          old_dst
ATOMIC_MAXSINT          imax(old_dst, src0)                     old_dst
ATOMIC_MINSINT          imin(old_dst, src0)                     old_dst
ATOMIC_MAX              umax(old_dst, src0)                     old_dst
ATOMIC_MIN              umin(old_dst, src0)                     old_dst
ATOMIC_CMPXCHG          (src1 == old_dst) ? src0 : old_dst      old_dst
ATOMIC_PREDEC           old_dst - 1                             new_dst
======================= ======================================= ====================

4.5 New (LSC) Dataport Interface
--------------------------------

New (LSC) Dataport Interface is available when CM_HAS_LSC macro is defined.

Load cache control policy
^^^^^^^^^^^^^^^^^^^^^^^^^

The table below defines the valid combinations of two-level cache controls used by
load intrinsics.

=============== ================
L1H             L2H
=============== ================
Default         Default
Uncached        Cached/Uncached
Cached          Cached/Uncached
Streaming       Cached/Uncached
=============== ================

The following combinations are also available when CM_HAS_LSC_L1L3CC_HINT macro is defined.

=============== ===============
L1H             L2H
=============== ===============
Uncached        ConstCached
Cached          ConstCached
=============== ===============

The following combination is also availablewhen CM_HAS_LSC_LOAD_L1RI_L3RI_HINT macro is defined.

=============== ===============
L1H             L2H
=============== ===============
ReadInvalidate  ReadInvalidate
=============== ===============

The following combination is also available when CM_HAS_LSC_LOAD_L1RI_L3CA_HINT macro is defined.

=============== ===============
L1H             L2H
=============== ===============
ReadInvalidate  Cached
=============== ===============

The table below defines the valid combinations of three-level cache controls 
when CM_HAS_LSC_L1L2L3_CACHE macro is defined.

================ ================ ========================
L1H              L2H              L3H
================ ================ ========================
Default          Default          Default
Cached           Cached/Uncached  Cached/Uncached/Default
Uncached         Cached/Uncached  Cached/Uncached/Default
Streaming        Cached/Uncached  Cached/Uncached/Default
ReadInvalidate   ReadInvalidate   ReadInvalidate
================ ================ ========================


Prefetch cache control policy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The table below defines the valid combinations of two-level cache controls used by
prefetch intrinsics.

=============== ================
L1H             L2H
=============== ================
Default         Default
Uncached        Cached
Cached          Cached/Uncached
Streaming       Cached/Uncached
=============== ================

The following combinations are also available when CM_HAS_LSC_L1L3CC_HINT macro is defined.

=============== ===============
L1H             L2H
=============== ===============
Cached/Uncached ConstCached
=============== ===============

Three levels of cache level control are available when CM_HAS_LSC_L1L2L3_CACHE macro is defined.

================ ================ =======================
L1H              L2H              L3H
================ ================ =======================
Default          Default          Default
Uncached         Uncached         Cached/Default
Uncached         Cached           Cached/Uncached/Default
Cached           Cached/Uncached  Cached/Uncached/Default
Streaming        Cached/Uncached  Cached/Uncached/Default
================ ================ =======================


Store cache control policy
^^^^^^^^^^^^^^^^^^^^^^^^^^

The table below defines the valid combinations of two-level cache controls used by
store intrinsics.

============= =============
L1H           L2H
============= =============
Default       Default
WriteBack     WriteBack
Uncached      WriteBack
WriteThrough  WriteBack
Streaming     WriteBack
Uncached      Uncached
WriteThrough  Uncached
Streaming     Uncached
============= =============

The table below defines the valid combinations of three-level cache controls 
when CM_HAS_LSC_L1L2L3_CACHE macro is defined.

=============== =============== ===========================
L1H             L2H             L3H
=============== =============== ===========================
Default         Default         Default
Uncached        Uncached        WriteBack/Uncached/Default
Uncached        WriteBack       Uncached/Default
WriteThrough    Uncached        WriteBack/Uncached/Default
WriteThrough    WriteBack       Uncached/Default
Streaming       Uncached        WriteBack/Uncached/Default
Streaming       WriteBack       Uncached/Default
WriteBack       Uncached        WriteBack/Uncached/Default
WriteBack       WriteBack       Uncached/Default
=============== =============== ===========================


Atomic cache control policy
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The table below defines the valid combinations of two-level cache controls used by
atomic intrinsics.

============= =============
L1H           L2H
============= =============
Default       Default
Uncached      Uncached
Uncached      WriteBack
============= =============

The table below defines the valid combinations of three-level cache controls 
when CM_HAS_LSC_L1L2L3_CACHE macro is defined.

=============== =============== ===========================
L1H             L2H             L3H
=============== =============== ===========================
Default         Default         Default
Uncached        Uncached        WriteBack/Uncached/Default
Uncached        WriteBack       Uncached/Default
=============== =============== ===========================


Statefull block load/store/prefetch
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

cm_load
"""""""
.. code-block:: c++

  template <typename T, int NElts, DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, NElts> cm_load(SurfaceIndex Idx, unsigned Offset);

  template <typename T, int NElts, DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, NElts> cm_load(SurfaceIndex Idx, unsigned Offset);

The compiler generates code for the hardware to perform block read from
memory. The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer bounds are considered to be "out-of-bound". Hardware will return 0
for the out-of-bound bytes.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

NElts           Specifies number of elements to be read.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                Only DataSize::U32 or DataSize::U64 are allowed.

Idx             Surface index corresponding to buffer surface.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ==================================================================


cm_store
""""""""
.. code-block:: c++

  template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default>
  void cm_store(SurfaceIndex Idx, unsigned Offset, vector<T, NElts> Data);

  template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
  void cm_store(SurfaceIndex Idx, unsigned Offset, vector<T, NElts> Data);


The compiler generates code for the hardware to perform block write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are considered to be "out-of-bound".
Hardware will not update the out-of-bound bytes in memory.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

NElts           Specifies number of elements to be written.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Else DataSize::U32 or DataSize::U64 are allowed.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ==================================================================


cm_prefetch
"""""""""""
.. code-block:: c++

  template <int NElts, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_prefetch(SurfaceIndex Idx, unsigned Offset);

  template <int NElts, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_prefetch(SurfaceIndex Idx, unsigned Offset);

The compiler generates code for the hardware to perform block prefetch from
memory. The underlying surface must be a buffer.

=============== ===========================================================
Parameter       Description
=============== ===========================================================
NElts           Specifies number of elements to be prefetched.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Only DataSize::U32 or DataSize::U64 are allowed.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ===========================================================


Statefull gather/scatter/prefetch/atomic
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

cm_load
"""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> Data cm_load(SurfaceIndex Idx, vector<unsigned, N> Offset,
                                vector<ushort, N> Pred = 1);

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> Data cm_load(SurfaceIndex Idx, vector<unsigned, N> Offset,
                                vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform gather read from
memory. The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer bounds are considered to be "out-of-bound". Hardware will return 0
for the out-of-bound bytes.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be read for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U8, ``RetTy`` is U8.
                When ``DS`` is DataSize::U16, ``RetTy`` is U16.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                The compiler will emit an error when invalid value is set.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

M               Total number of data elements to be written.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_load4
""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  auto cm_load4(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<ushort, N> Pred = 1);

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  auto cm_load4(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform gather read from
memory. The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer bounds are considered to be "out-of-bound". Hardware will return 0
for the out-of-bound bytes.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Specifies number of elements
                to be read for each offset element.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_store
""""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_store(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<T, M> Data, vector<ushort, N> Pred = 1);

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_store(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<T, M> Data, vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform scatter write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are considered to be "out-of-bound".
Hardware will not update the out-of-bound bytes in memory.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be written for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

M               Total number of data elements to be written.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================


cm_store4
"""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_store4(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<T, N * M> Data, vector<ushort, N> Pred = 1);

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_store4(SurfaceIndex Idx, vector<unsigned, N> Offset,
                vector<T, N * M> Data, vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform scatter write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are considered to be "out-of-bound".
Hardware will not update the out-of-bound bytes in memory.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Only contiguous channel masks are
                supported (R, GR, BGR and ABGR).

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

M               The size ``M`` must be equal to the number of enabled
                channels.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================


cm_prefetch
"""""""""""
.. code-block:: c++

  template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached>
  void cm_prefetch(SurfaceIndex Idx, vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1);

  template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached,
            CacheHint L3H = CacheHint::Cached>
  void cm_prefetch(SurfaceIndex Idx, vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform gather prefetch from
memory. The underlying surface must be a buffer.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
VS              Specifies number of elements to be prefetched
                for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be prefetched
                from the memory. Optional argument.
=============== ==================================================================


cm_atomic
"""""""""

.. code-block:: c++

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                             vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                             vector<T, M> Src0, vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                             vector<T, M> Src0, vector<T, M> Src1,
                             vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                 vector<T, M> Src0, vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset,
                 vector<T, M> Src0, vector<T, M> Src1,
                 vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform atomic memory operation
that modifies memory and returns the original memory data.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
Op              Atomic memory operation kind. See the table below.

T               Data type.

VS              Specifies number of elements to be modified
                for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Allowed values are DataSize::Default, DataSize::U16,
                DataSize::U32 and DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Atomic cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional argument.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U16, ``RetTy`` is U16.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                The compiler will emit an error when invalid value is set.

Idx             Surface index corresponding to buffer surface.

N               Number of the vectors.

M               Total number of data elements to be modified.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Src0            First Operand. It must be set unless ``Op`` is AtomicOp::IINC,
                AtomicOp::IDEC or AtomicOp::LOAD

Src1            Second operand. It must be set if ``Op`` is AtomicOp::ICAS or
                AtomicOp::FCAS

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be prefetched
                from the memory. Optional argument.
=============== ==================================================================

Supported atomic operations:

================ ============================== ============== ===============
Kind             Supported Data types           First Operand  Second Operand
================ ============================== ============== ===============
AtomicOp::IINC   U16, U32, U64                  N              N

AtomicOp::IDEC   U16, U32, U64                  N              N

AtomicOp::LOAD   U16, U32, U64, F16, F32, F64   N              N

AtomicOp::STORE  U16, U32, U64, F16, F32, F64   Y              N

AtomicOp::IADD   U16, U32, U64                  Y              N

AtomicOp::ISUB   U16, U32, U64                  Y              N

AtomicOp::SMIN   U16, U32, U64                  Y              N

AtomicOp::SMAX   U16, U32, U64                  Y              N

AtomicOp::UMIN   U16, U32, U64                  Y              N

AtomicOp::UMAX   U16, U32, U64                  Y              N

AtomicOp::AND    U16, U32, U64                  Y              N

AtomicOp::OR     U16, U32, U64                  Y              N

AtomicOp::XOR    U16, U32, U64                  Y              N

AtomicOp::FADD   F16, F32, F64                  Y              N

AtomicOp::FSUB   F16, F32, F64                  Y              N

AtomicOp::FMIN   F16, F32, F64                  Y              N

AtomicOp::FMAX   F16, F32, F64                  Y              N

AtomicOp::ICAS   F16, U32, U64                  Y              Y

AtomicOp::FCAS   F16, F32, F64                  Y              Y

AtomicOp::BFADD  BF16                           Y              N

AtomicOp::BFSUB  BF16                           Y              N

AtomicOp::BFMIN  BF16                           Y              N

AtomicOp::BFMAX  BF16                           Y              N

AtomicOp::BFCAS  BF16                           Y              Y
================ ============================== ============== ===============

These ``bfloat16`` data types are target-dependent and only available
when ``CM_HAS_BF16_ATOMIC`` macro is defined.


Stateless block load/store/prefetch
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

cm_ptr_load
"""""""""""
.. code-block:: c++

  template <typename T, int NElts, DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, NElts> cm_ptr_load(const T *const Ptr, unsigned Offset);

  template <typename T, int NElts, DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, NElts> cm_ptr_load(const T *const Ptr, unsigned Offset);


The compiler generates code for the hardware to perform block read from
memory. The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer bounds are illegal, and the behavior is *undefined*.


=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

NElts           Specifies number of elements to be read.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                Only DataSize::U32 or DataSize::U64 are allowed.

Ptr             Pointer which holds the address of a buffer in the memory.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ==================================================================


cm_ptr_store
""""""""""""
.. code-block:: c++

  template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, unsigned Offset, vector<T, NElts> Data);

  template <typename T, int NElts, DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L2H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, unsigned Offset, vector<T, NElts> Data);


The compiler generates code for the hardware to perform block write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

NElts           Specifies number of elements to be written.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Else DataSize::U32 or DataSize::U64 are allowed.

L1H, L2H, L3H    Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ==================================================================


cm_ptr_prefetch
"""""""""""""""
.. code-block:: c++

  template <int NElts, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_ptr_prefetch(const void *const Ptr, unsigned Offset);

  template <int NElts, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_ptr_prefetch(const void *const Ptr, unsigned Offset);


The compiler generates code for the hardware to perform block prefetch from
memory. The underlying surface must be a buffer. Any byte(s) of the accessed
block which is outside of the specified buffer bounds are illegal,
and the behavior is *undefined*.

=============== ===========================================================
Parameter       Description
=============== ===========================================================
NElts           Specifies number of elements to be prefetched.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Only DataSize::U32 or DataSize::U64 are allowed.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned.
=============== ===========================================================


Stateless gather/scatter/prefetch/atomic
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

cm_ptr_load
"""""""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  auto cm_ptr_load(const T *const Ptr, vector<unsigned, N> Offset,
                                   vector<ushort, N> Pred = 1);

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  auto cm_ptr_load(const T *const Ptr, vector<unsigned, N> Offset,
                                  vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform gather read from
memory. The underlying surface must be a buffer.

Any byte(s) of the accessed block which is outside of the specified
buffer bounds are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be read for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

N               Number of the vectors.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_ptr_load4
""""""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  auto cm_ptr_load4(const T *const Ptr, vector<unsigned, N> Offset,
                                   vector<ushort, N> Pred = 1);

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  auto cm_ptr_load4(const T *const Ptr, vector<unsigned, N> Offset,
                                  vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform gather read from
memory. The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer bounds are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Specifies number of elements
                to be read for each offset element.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

N               Number of the vectors.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_ptr_store
""""""""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, vector<unsigned, N> Offset,
                    vector<T, M> Data, vector<ushort, N> Pred = 1);

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, vector<unsigned, N> Offset,
                    vector<T, M> Data, vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform scatter write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be written for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

M               Total number of data elements to be written.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================


cm_ptr_store4
"""""""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, vector<unsigned, N> Offset,
                    vector<T, N * M> Data, vector<ushort, N> Pred = 1);

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, vector<unsigned, N> Offset,
                    vector<T, N * M> Data, vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform scatter write to memory.
The underlying surface must be a buffer.
Any byte(s) of the accessed block which is outside of the specified
buffer are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Only contiguous channel masks are
                supported (R, GR, BGR and ABGR).

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.


N               Number of the vectors.

M               The size ``M`` must be equal to the number of enabled
                channels.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================


cm_ptr_prefetch
"""""""""""""""
.. code-block:: c++

  template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached>
  void cm_ptr_prefetch(const void *const Ptr, vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1);

  template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached,
            CacheHint L3H = CacheHint::Cached>
  void cm_ptr_prefetch(const void *const Ptr, vector<unsigned, N> Offset,
                                      vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform gather prefetch from
memory. The underlying surface must be a buffer. Any byte(s) of the accessed
block which is outside of the specified buffer are illegal,
and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
VS              Specifies number of elements to be prefetched
                for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of a buffer in the memory.

N               Number of the vectors.

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be prefetched
                from the memory. Optional argument.
=============== ==================================================================


cm_ptr_atomic
"""""""""""""

.. code-block:: c++

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<T, M> Src0,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<T, M> Src0, vector<T, M> Src1,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<T, M> Src0,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<RetTy, M> cm_ptr_atomic(T *Ptr, vector<unsigned, N> Offset,
                                 vector<T, M> Src0, vector<T, M> Src1,
                                 vector<ushort, N> Pred = 1);


The compiler generates code for the hardware to perform atomic memory operation
that modifies memory and returns the original memory data. The underlying surface
must be a buffer. Any byte(s) of the accessed block which is outside
of the specified buffer are illegal, and the behavior is *undefined*.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
Op              Atomic memory operation kind. See the table below.

T               Data type.

VS              Specifies number of elements to be modified
                for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Allowed values are DataSize::Default, DataSize::U16,
                DataSize::U32 and DataSize::U64.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Atomic cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U16, ``RetTy`` is U16.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                The compiler will emit an error when invalid value is set.

Ptr             Pointer which holds the address of a buffer in the memory.

N               Number of the vectors.

M               Total number of data elements to be modified.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Src0            First Operand. It must be set unless ``Op`` is AtomicOp::IINC,
                AtomicOp::IDEC or AtomicOp::LOAD

Src1            Second operand. It must be set if ``Op`` is AtomicOp::ICAS or
                AtomicOp::FCAS

Offset          Zero based offset of the input buffer in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be prefetched
                from the memory. Optional argument.
=============== ==================================================================

Supported atomic operations:

================ ============================== ============== ===============
Kind             Supported Data types           First Operand  Second Operand
================ ============================== ============== ===============
AtomicOp::IINC   U16, U32, U64                  N              N

AtomicOp::IDEC   U16, U32, U64                  N              N

AtomicOp::LOAD   U16, U32, U64, F16, F32, F64   N              N

AtomicOp::STORE  U16, U32, U64, F16, F32, F64   Y              N

AtomicOp::IADD   U16, U32, U64                  Y              N

AtomicOp::ISUB   U16, U32, U64                  Y              N

AtomicOp::SMIN   U16, U32, U64                  Y              N

AtomicOp::SMAX   U16, U32, U64                  Y              N

AtomicOp::UMIN   U16, U32, U64                  Y              N

AtomicOp::UMAX   U16, U32, U64                  Y              N

AtomicOp::AND    U16, U32, U64                  Y              N

AtomicOp::OR     U16, U32, U64                  Y              N

AtomicOp::XOR    U16, U32, U64                  Y              N

AtomicOp::FADD   F16, F32, F64                  Y              N

AtomicOp::FSUB   F16, F32, F64                  Y              N

AtomicOp::FMIN   F16, F32, F64                  Y              N

AtomicOp::FMAX   F16, F32, F64                  Y              N

AtomicOp::ICAS   F16, U32, U64                  Y              Y

AtomicOp::FCAS   F16, F32, F64                  Y              Y

AtomicOp::BFADD  BF16                           Y              N

AtomicOp::BFSUB  BF16                           Y              N

AtomicOp::BFMIN  BF16                           Y              N

AtomicOp::BFMAX  BF16                           Y              N

AtomicOp::BFCAS  BF16                           Y              Y
================ ============================== ============== ===============

These ``bfloat16`` data types are target-dependent and only available
when ``CM_HAS_BF16_ATOMIC`` macro is defined.


Shared local memory gather/scatter/atomic
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

cm_load_slm
"""""""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default>
  auto cm_load_slm(vector<unsigned, N> Offset,
                   vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform gather read from
the shared local memory.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be read for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

N               Number of the vectors.

Offset          Zero based offset of the shared local memory in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_load_slm4
""""""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default>
  auto cm_load_slm4(vector<unsigned, N> Offset,
                   vector<ushort, N> Pred = 1);

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Specifies number of elements
                to be read for each offset element.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

N               Number of the vectors.

Offset          Zero based offset of the shared local memory in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be loaded
                from the memory. Optional argument.
=============== ==================================================================


cm_store_slm
""""""""""""
.. code-block:: c++

  template <typename T, VectorSize VS = VectorSize::N1,
            DataSize DS = DataSize::Default>
  void cm_store_slm(vector<unsigned, N> Offset,
                    vector<T, N * M> Data,
                    vector<ushort, N> Pred = 1);

  template <typename T, int NElts,
            DataSize DS = DataSize::Default>
  void cm_store_slm(vector<unsigned, N> Offset,
                    vector<T, N * M> Data,
                    vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform scatter write to the shared
local memory.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

VS              Specifies number of elements to be written for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

NElts           Specifies number of elements to be written.
                Allowed values are: 1, 2, 3, 4, 8, 16, 32, 64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

M               The size ``M`` must be equal to the number of elements
                to be written.

Offset          Zero based offset of the shared local memory in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================

cm_store_slm4
"""""""""""""
.. code-block:: c++

  template <typename T, ChannelMaskType Mask,
            DataSize DS = DataSize::Default>
  void cm_store_slm(vector<unsigned, N> Offset,
                    vector<T, N * M> Data,
                    vector<ushort, N> Pred = 1);

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.

Mask            Mask for enabled channels. Only contiguous channel masks are
                supported (R, GR, BGR and ABGR).

DS              Data size.
                When DS is DataSize::Default, it's obtained from ``T`` data type.
                Allowed values are: DataSize::U8, DataSize::U16, DataSize::U32 and
                DataSize::U64.

N               Number of the vectors.

M               The size ``M`` must be equal to the number of enabled
                channels.

Offset          Zero based offset of the shared local memory in *bytes*. Must be
                ``DS`` aligned when ``Mask`` is not a single channel.

Pred            Predicate. if 0, certain element vector won't be written
                to the memory. Optional argument.
=============== ==================================================================

cm_atomic_slm
"""""""""""""

.. code-block:: c++

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default>
  vector<RetTy, M> cm_atomic_slm(vector<unsigned, N> Offset,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default>
  vector<RetTy, M> cm_atomic_slm(vector<unsigned, N> Offset,
                                 vector<T, M> Src0,
                                 vector<ushort, N> Pred = 1);

  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default>
  vector<RetTy, M> cm_atomic_slm(vector<unsigned, N> Offset,
                                 vector<T, M> Src0,
                                 vector<T, M> Src1,
                                 vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform atomic memory operation
that modifies the shared local memory and returns the original memory data.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
Op              Atomic memory operation kind. See the table below.

T               Data type.

VS              Specifies number of elements to be modified
                for each offset element.
                Allowed values are: VectorSize::N1, VectorSize::N2,
                VectorSize::N3, VectorSize::N4, VectorSize::N8, VectorSize::N16,
                VectorSize::N32, VectorSize::N64.
                The compiler will emit an error when invalid value is set.

DS              Data size.
                Allowed values are DataSize::Default, DataSize::U16,
                DataSize::U32 and DataSize::U64.

RetTy           Return data type, obtained from ``T`` and ``DS``.
                When ``DS`` is DataSize::Default, ``RetTy`` is same as ``T``.
                When ``DS`` is DataSize::U16, ``RetTy`` is U16.
                When ``DS`` is DataSize::U32, ``RetTy`` is U32.
                When ``DS`` is DataSize::U64, ``RetTy`` is U64.
                The compiler will emit an error when invalid value is set.

N               Number of the vectors.

M               Total number of data elements to be modified.
                It must be equal to
                :math:`\text{N} \times \text{VS}`.

Src0            First Operand. It must be set unless ``Op`` is AtomicOp::IINC,
                AtomicOp::IDEC or AtomicOp::LOAD

Src1            Second operand. It must be set if ``Op`` is AtomicOp::ICAS or
                AtomicOp::FCAS

Offset          Zero based offset of the shared local memory in *bytes*. Must be
                ``DS`` aligned when ``VS`` is not VectorSize::N1.

Pred            Predicate. if 0, certain element vector won't be prefetched
                from the memory. Optional argument.
=============== ==================================================================

Supported atomic operations:

================ ============================== ============== ===============
Kind             Supported Data types           First Operand  Second Operand
================ ============================== ============== ===============
AtomicOp::IINC   U16, U32, U64                  N              N

AtomicOp::IDEC   U16, U32, U64                  N              N

AtomicOp::LOAD   U16, U32, U64, F16, F32, F64   N              N

AtomicOp::STORE  U16, U32, U64, F16, F32, F64   Y              N

AtomicOp::IADD   U16, U32, U64                  Y              N

AtomicOp::ISUB   U16, U32, U64                  Y              N

AtomicOp::SMIN   U16, U32, U64                  Y              N

AtomicOp::SMAX   U16, U32, U64                  Y              N

AtomicOp::UMIN   U16, U32, U64                  Y              N

AtomicOp::UMAX   U16, U32, U64                  Y              N

AtomicOp::AND    U16, U32, U64                  Y              N

AtomicOp::OR     U16, U32, U64                  Y              N

AtomicOp::XOR    U16, U32, U64                  Y              N

AtomicOp::FADD   F16, F32, F64                  Y              N

AtomicOp::FSUB   F16, F32, F64                  Y              N

AtomicOp::FMIN   F16, F32, F64                  Y              N

AtomicOp::FMAX   F16, F32, F64                  Y              N

AtomicOp::ICAS   F16, U32, U64                  Y              Y

AtomicOp::FCAS   F16, F32, F64                  Y              Y

AtomicOp::BFADD  BF16                           Y              N

AtomicOp::BFSUB  BF16                           Y              N

AtomicOp::BFMIN  BF16                           Y              N

AtomicOp::BFMAX  BF16                           Y              N

AtomicOp::BFCAS  BF16                           Y              Y
================ ============================== ============== ===============

These ``bfloat16`` data types are target-dependent and only available
when ``CM_HAS_BF16_ATOMIC`` macro is defined.


Untyped 2D block load/store/prefetch
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These functions are target-dependent and only available
when ``CM_HAS_LSC_UNTYPED_2D`` macro is defined.

.. note::
   Using these functions may have a negative performance impact, as the
   descriptor needs to be built for every call. It is recommended to use
   `Untyped descriptor based 2D block load/store/prefetch`_ instead, which
   allows the descriptor to be built once and reused across multiple
   operations.

cm_ptr_load
"""""""""""
.. code-block:: c++

  template <typename T, int Width, int Height = 1, int NumBlocks = 1,
            bool Transposed = false, bool Transformed = false,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
  vector<T, N> cm_ptr_load(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                           unsigned SurfacePitch, int X, int Y);

  template <typename T, int Width, int Height = 1, int NumBlocks = 1,
            bool Transpose = false, bool Transform = false,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  vector<T, N> cm_ptr_load(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                           unsigned SurfacePitch, int X, int Y);


The compiler generates code for the hardware to perform 2D block read from
an array of rectangular block(s) in memory. The rectangular blocks in
the array are assumed to be stacked horizontally in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).
Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".
Hardware will return 0 for the out-of-bound bytes.

For normal load (Transpose = false and Transform = false) the block Width
is padded up to the next power-of-two value in the GRF.
E.g. if the 2D block width is 12, and sizeof(T) is 8,
the space allocated in the GRF will be 16 bytes per row (next power-of-two number).

For Transpose load (Transpose = true and Transform = false)
the pre-operation block Height is padded up to the next power-of-two value
(minimum 4 bytes). E.g. if the 2D block height is 12, and sizeof(T) is 8,
the space allocated in the GRF will be 16 bytes (next power-of-two number).
The operation loads a column of elements, and put them back to back in a GRF.
This is useful matrix dA load in backward pass Weight gradient.

For VNNI Transform load (Transpose = false and Transform = true)
the pre-operation Block Height is padded up to multiple of N,
where N = 4 / sizeof(element). The pre-operation block Width is
padded up to next power-of-two value. The padded bytes will be returned as 0.
E.g if the input block is 11x7, the block will be padded up to 12x8
before the VNNI Transform operation. The operation loads N elements
from a column and put back to back in the same GRF.
This transform lays out the GRF that is friendly to the EU Systolic
instruction. Useful for matrix B load in forward pass.

The compiler eliminates the padding by generating extra MOV instructions.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.
                For normal load byte, 2-byte, 4-byte and 8-byte types are allowed.
                For Transpose load only 4-byte and 8-byte types are allowed.
                For VNNI Transform load only byte and 2-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

Height          Specifies the height in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

NumBlocks       Specifies Array Length.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

Transposed      Enable Transpose.
                Only 4-byte and 8-byte types are allowed.
                See the table below for the restrictions.

Transformed     Enable VNNI Transform.
                Only byte and 2-byte types are allowed.
                See the table below for the restrictions.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Ptr             Pointer which holds the address of raw (untyped) data
                in the memory.

SurfaceWidth    Surface Width minus 1 in bytes of the 2D surface.
                Surface Width must be equal or greater than 64B.
                Surface Width must be DWord (i.e. 4 bytes) aligned.

SurfaceHeight   Surface Height minus 1 in number of data elements
                of the Untyped 2D surface.

SurfacePitch    Surface Pitch minus 1 in bytes of the 2D surface.
                Surface Pitch must be greater or equal to Surface Width.
                Surface Pitch must be equal or greater than 64B.
                Surface Pitch must be OWord (i.e. 16 bytes) aligned before Xe3P.
                Surface Pitch must be DWord (i.e. 4 bytes) aligned for Xe3P+.

X               Block start X coordinate.
                Specifies the signed X offset in number of data elements
                from the 2D surface base address for this rectangular region.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements
                from the 2D surface base address for this rectangular region.

Data            The data vector read from memory.
                The size ``N`` is a number of elements.
                ``N`` must be equal to
                :math:`\text{Width} \times \text{Height} \times \text{NumBlocks}`.
                For VNNI Transform, it must be equal to
                :math:`\text{Width} \times \text{RoundHeight} \times \text{NumBlocks}`,
                where ``RoundHeight`` is ``Height`` rounded
                up to be multiple of (4 / sizeof(T))
=============== ==================================================================

Normal load restrictions:
``Width`` X ``NumBlocks`` must not exceed 64 bytes.

========= ========== ========= =============
Data Size ``Height`` ``Width`` ``NumBlocks``
========= ========== ========= =============
byte      1 - 32     4 - 64    1, 2, 4

2-byte    1 - 32     2 - 32    1, 2, 4

4-byte    1 - 32     1 - 16    1, 2

8-byte    1 - 32     1 - 8     1
========= ========== ========= =============

Transpose load restrictions:

========= ========== ============ =============
Data Size ``Height`` ``Width``    ``NumBlocks``
========= ========== ============ =============
4-byte    1 - 32     1 - 8        1

8-byte    8          1, 2, 4      1
========= ========== ============ =============

The above restrictions are relaxed 
when ``CM_HAS_LSC_2D_LARGE`` macro is defined:

========= ========== ============ =============
Data Size ``Height`` ``Width``    ``NumBlocks``
========= ========== ============ =============
4-byte    1 - 32     1 - 8, 16    1

8-byte    8          1, 2, 4, 8   1
========= ========== ============ =============

VNNI Transform load restrictions:
``Width`` X ``NumBlocks`` must not exceed 64 bytes.

========= ========== ========= =============
Data Size ``Height`` ``Width`` ``NumBlocks``
========= ========== ========= =============
byte      4 - 32     4 - 64    1, 2, 4

2-byte    2 - 32     2 - 32    1, 2, 4
========= ========== ========= =============


cm_ptr_store
""""""""""""

.. code-block:: c++

  template <typename T, int Width, int Height = 1,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                    unsigned SurfacePitch, int X, int Y, vector<T, N> Data);

  template <typename T, int Width, int Height = 1,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default>
  void cm_ptr_store(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                    unsigned SurfacePitch, int X, int Y, vector<T, N> Data);


The compiler generates code for the hardware to perform
2D block write from rectangular block in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).
Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".
Hardware will not update the out-of-bound bytes in memory.

=============== =============================================================
Parameter       Description
=============== =============================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

Height          Specifies the height in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hint is set.
                Optional arguments.

Ptr             Pointer which holds the address of raw (untyped) data
                in the memory.

SurfaceWidth    Surface Width minus 1 in bytes of the 2D surface.
                Surface Width must be equal or greater than 64B.
                Surface Width must be DWord (i.e. 4 bytes) aligned.

SurfaceHeight   Surface Height minus 1 in number of data elements
                of the Untyped 2D surface.

SurfacePitch    Surface Pitch minus 1 in bytes of the 2D surface.
                Surface Pitch must be greater or equal to Surface Width.
                Surface Pitch must be equal or greater than 64B.
                Surface Pitch must be OWord (i.e. 16 bytes) aligned before Xe3P.
                Surface Pitch must be DWord (i.e. 4 bytes) aligned for Xe3P+.

X               Block start X coordinate.
                Specifies the signed X offset in number of data elements
                from the 2D surface base address for this rectangular region.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements
                from the 2D surface base address for this rectangular region.

Data            The vector holding data to be written.
                The size ``N`` is a number of elements.
                ``N`` must be equal to
                :math:`\text{Width} \times \text{Height}`.
=============== =============================================================

Store restrictions:

========= ========== =========
Data Size ``Height`` ``Width``
========= ========== =========
byte      1 - 8        4 - 64

2-byte    1 - 8        2 - 32

4-byte    1 - 8        1 - 16

8-byte    1 - 8        1 - 8
========= ========== =========


cm_ptr_prefetch
"""""""""""""""

.. code-block:: c++

  template <typename T, int Width, int Height = 1, int NumBlocks = 1,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached>
  void cm_ptr_prefetch(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                      unsigned SurfacePitch, int X, int Y);

  template <typename T, int Width, int Height = 1, int NumBlocks = 1,
            CacheHint L1H = CacheHint::Cached,
            CacheHint L2H = CacheHint::Cached,
            CacheHint L3H = CacheHint::Default>
  void cm_ptr_prefetch(T *Ptr, unsigned SurfaceWidth, unsigned SurfaceHeight,
                      unsigned SurfacePitch, int X, int Y);


The compiler generates code for the hardware to perform 2D block prefetch from
an array of rectangular block(s) in memory. The rectangular blocks in
the array are assumed to be stacked horizontally in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).

=============== =============================================================
Parameter       Description
=============== =============================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

Height          Specifies the height in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.
                See the table below for the restrictions.

NumBlocks       Specifies Array Length.
                See the table below for the restrictions.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hint is set.
                Optional arguments.

Ptr             Pointer which holds the address of raw (untyped) data
                in the memory.

SurfaceWidth    Surface Width minus 1 in bytes of the 2D surface.
                Surface Width must be equal or greater than 64B.
                Surface Width must be DWord (i.e. 4 bytes) aligned.

SurfaceHeight   Surface Height minus 1 in number of data elements
                of the Untyped 2D surface.

SurfacePitch    Surface Pitch minus 1 in bytes of the 2D surface.
                Surface Pitch must be greater or equal to Surface Width.
                Surface Pitch must be equal or greater than 64B.
                Surface Pitch must be OWord (i.e. 16 bytes) aligned before Xe3P.
                Surface Pitch must be DWord (i.e. 4 bytes) aligned for Xe3P+.

X               Block start X coordinate.
                Specifies the signed X offset in number of data elements
                from the 2D surface base address for this rectangular region.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements
                from the 2D surface base address for this rectangular region.
=============== =============================================================

Prefetch restrictions:

========= ============ =============== =============
Data Size ``Height``   ``Width``       ``NumBlocks``
========= ============ =============== =============
byte      1 - 32       4 - 64          1, 2, 4

2-byte    1 - 32       2 - 32          1, 2, 4

4-byte    1 - 32       1 - 16          1, 2

8-byte    1 - 32       1 - 8           1
========= ============ =============== =============

The above restrictions are relaxed when ``CM_HAS_LSC_2D_LARGE`` macro is defined:

========= ============ ================== =============
Data Size ``Height``   ``Width``         ``NumBlocks``
========= ============ ================== =============
byte      1 - 32       4 - 64 , 128, 256  1, 2, 4

2-byte    1 - 32       2 - 32, 64, 128    1, 2, 4

4-byte    1 - 32       1 - 16, 32, 64     1, 2, 4

8-byte    1 - 32       1 - 8, 16, 32      1, 2, 4
========= ============ ================== =============


Untyped descriptor based 2D block load/store/prefetch
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These functions are target-dependent and only available
when ``CM_HAS_LSC_UNTYPED_2D`` macro is defined.

Unlike the regular untyped LSC 2D block load/store/prefetch above,
the descriptor based ones keep padding as-is in GRF so user should
take care of extra data returned by load. User may initialize
the descriptor only once and use it many times eliminating
the need of creating the descriptor for every operation which removes
extra move instructions.

Block 2D Descriptor
"""""""""""""""""""

.. code-block:: c++

  namespace lsc {
  template <typename T, unsigned NumBlocks, unsigned BlockHeight,
            unsigned BlockWidth>
  struct block_2d_desc {

    block_2d_desc(T *Ptr, unsigned Height, unsigned Width,
                  unsigned Pitch, int BlockX, int BlockY);

    block_2d_desc(__global T *Ptr, unsigned Height, unsigned Width,
                 unsigned Pitch, int BlockX, int BlockY);

    block_2d_desc(uint64_t Base, unsigned Height, unsigned Width,
                            unsigned Pitch, int BlockX, int BlockY);

    block_2d_desc &set_base(uint64_t value);
    block_2d_desc &set_width(uint32_t value);
    block_2d_desc &set_height(uint32_t value);
    block_2d_desc &set_pitch(uint32_t value);
    block_2d_desc &set_block_x(int32_t value);
    block_2d_desc &set_block_y(int32_t value);

    uint64_t get_base() const;
    uint32_t get_width() const;
    uint32_t get_height() const;
    uint32_t get_pitch() const;
    int32_t get_block_x() const;
    int32_t get_block_y() const;

    block_2d_desc &set_base_ptr(T *Ptr);
    T *get_base_ptr() const;

    vector<uint32_t, 16> get_raw_desc();
  };
  } // namespace lsc

=============== ===============================================================
Parameter       Description
=============== ===============================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.
                For Transpose load only 4-byte and 8-byte types are allowed.
                For VNNI Transform load only byte and 2-byte types are allowed.

NumBlocks       Specifies Array Length.
                The compiler will emit an error when invalid value is set.

BlockHeight     Specifies the height in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.

BlockWidth      Specifies the width in number of data elements
                for this rectangular region.
                The compiler will emit an error when invalid value is set.

Ptr             Pointer which holds the address of raw (untyped) data
                in the memory.

Base            Base address of raw (untyped) data in the memory.

Height          Surface Height minus 1 in number of data elements
                of the Untyped 2D surface.

Width           Surface Width minus 1 in bytes of the 2D surface.
                Surface Width must be equal or greater than 64B.
                Surface Width must be DWord (i.e. 4 bytes) aligned.

Pitch           Surface Pitch minus 1 in bytes of the 2D surface.
                Surface Pitch must be greater or equal to Surface Width.
                Surface Pitch must be equal or greater than 64B.
                Surface Pitch must be OWord (i.e. 16 bytes) aligned before Xe3P.
                Surface Pitch must be DWord (i.e. 4 bytes) aligned for Xe3P+.

BlockX          Block start X coordinate.
                Specifies the signed X offset in number of data elements
                from the 2D surface base address for this rectangular region.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

BlockY          Block start Y coordinate.
                Specifies the signed Y offset in number of data elements
                from the 2D surface base address for this rectangular region.
=============== ===============================================================

.. code-block:: c++

  namespace lsc {
  enum LoadOp {
    Normal = 0,
    Transpose = 1,
    VNNI = 2,
  };
  } // namespace lsc

  // determines how many GRF are occupied by the data
  template <typename T>
  constexpr unsigned get_lsc_grf_elements(lsc::LoadOp Op, unsigned BlockW,
                                        unsigned BlockH, unsigned NumBlocks);

cm_load
"""""""

.. code-block:: c++

  // returned data vector_ref
  template <typename T, unsigned BlockH, unsigned BlockW, unsigned NumBlocks = 1,
            lsc::LoadOp Op = lsc::LoadOp::Normal>
  using Block2DRefTy =
      vector_ref<T, get_lsc_grf_elements<T>(Op, BlockW, BlockH, NumBlocks)>;

  template <lsc::LoadOp Op = lsc::LoadOp::Normal,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0>
  void cm_load(details::Block2DRefTy<T, BlockH, BlockW, NBlocks, Op> Res,
              const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
              int16_t Pred = 1);

  template <lsc::LoadOp Op = lsc::LoadOp::Normal,
            CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0>
  void cm_load(details::Block2DRefTy<T, BlockH, BlockW, NBlocks, Op> Res,
              const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
              int16_t Pred = 1);


The compiler generates code for the hardware to perform 2D block read from
an array of rectangular block(s) in memory. The rectangular blocks in
the array are assumed to be stacked horizontally in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).
Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".
Hardware will return 0 for the out-of-bound bytes.

For normal load the block Width is padded up to the next power-of-two value
in the GRF. E.g. if the 2D block width is 12, and sizeof(T) is 8,
the space allocated in the GRF will be 16 bytes
per row (next power-of-two number).

For Transpose load the pre-operation block Height is padded up to the next
power-of-two value (minimum 4 bytes). E.g. if the 2D block height is 12,
and sizeof(T) is 8, the space allocated in the GRF will be
16 bytes (next power-of-two number).
The operation loads a column of elements, and put them back to back in a GRF.
This is useful matrix dA load in backward pass Weight gradient.

For VNNI Transform load the pre-operation Block Height is padded up to
multiple of N, where N = 4 / sizeof(element). The pre-operation block Width is
padded up to next power-of-two value. The padded bytes will be returned as 0.
E.g if the input block is 11x7, the block will be padded up to 12x8
before the VNNI Transform operation. The operation loads N elements
from a column and put back to back in the same GRF.
This transform lays out the GRF that is friendly to the EU Systolic
instruction. Useful for matrix B load in forward pass.

=============== ===============================================================
Parameter       Description
=============== ===============================================================
Op              Load operation type (derived):
                ``lsc::Normal`` is for normal load.
                ``lsc::Transpose`` is for Transpose load.
                ``lsc::VNNI`` is for VNNI Transform load.

L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hint is set.
                Optional arguments.

OffsetX         Block offset X coordinate in addition to block start X
                coordinate set in the descriptor.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

OffsetY         Block start Y coordinate in addition to block start Y
                coordinate set in the descriptor.

Desc            Block 2D descriptor. Must be initialized.
                See ``lsc::block_2d_desc`` for more info.
                See the table below for the restrictions
                per each load operation.

Res             The data vector read from memory. The vector size
                determined by ``Op``, ``BlockW``, ``BlockH`` and ``NumBlocks``
                template parameters. See ``Block2DRefTy`` for info.

Pred            Predicate.
                if set to 1, ``Res`` data will be loaded with data from memory.
                if set to 0, ``Res`` data won't be updated.
=============== ===============================================================

Normal load restrictions:
``Width`` X ``NumBlocks`` must not exceed 64 bytes.

========= ========== ========= =============
Data Size ``Height`` ``Width`` ``NumBlocks``
========= ========== ========= =============
byte      1 - 32     4 - 64    1, 2, 4

2-byte    1 - 32     2 - 32    1, 2, 4

4-byte    1 - 32     1 - 16    1, 2

8-byte    1 - 32     1 - 8     1
========= ========== ========= =============

Transpose load restrictions:

========= ========== ============ =============
Data Size ``Height`` ``Width``    ``NumBlocks``
========= ========== ============ =============
4-byte    1 - 32     1 - 8        1

8-byte    8          1, 2, 4      1
========= ========== ============ =============

The above restrictions are relaxed when ``CM_HAS_LSC_2D_LARGE`` macro is defined:

========= ========== ============ =============
Data Size ``Height`` ``Width``    ``NumBlocks``
========= ========== ============ =============
4-byte    1 - 32     1 - 8, 16    1

8-byte    8          1, 2, 4, 8   1
========= ========== ============ =============

VNNI Transform load restrictions:
``Width`` X ``NumBlocks`` must not exceed 64 bytes.

========= ========== ========= =============
Data Size ``Height`` ``Width`` ``NumBlocks``
========= ========== ========= =============
byte      4 - 32     4 - 64    1, 2, 4

2-byte    2 - 32     2 - 32    1, 2, 4
========= ========== ========= =============


cm_store
""""""""

.. code-block:: c++

  // data vector
  template <typename T, unsigned BlockH, unsigned BlockW>
  using Block2DTy =
      vector<T, get_lsc_grf_elements<T>(lsc::LoadOp::Normal, BlockW, BlockH, 1)>;

  template <CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0>
  void cm_store(const lsc::block_2d_desc<T, 1, BlockH, BlockW> &Desc,
                details::Block2DTy<T, BlockH, BlockW> Src, int16_t Pred = 1);

  template <CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0>
  void cm_store(const lsc::block_2d_desc<T, 1, BlockH, BlockW> &Desc,
                details::Block2DTy<T, BlockH, BlockW> Src, int16_t Pred = 1);


The compiler generates code for the hardware to perform
2D block write from rectangular block in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).
Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".
Hardware will not update the out-of-bound bytes in memory.

=============== ===========================================================
Parameter       Description
=============== ===========================================================
L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

OffsetX         Block offset X coordinate in addition to block start X
                coordinate set in the descriptor.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

OffsetY         Block start Y coordinate in addition to block start Y
                coordinate set in the descriptor.

Desc            Block 2D descriptor. Must be initialized.
                See ``lsc::block_2d_desc`` for more info.
                See the table below for the restrictions.

Src             The data vector written to memory.
                The vector size determined by ``BlockW`` and ``BlockH``
                template parameters. See ``Block2DTy`` for info.

Pred            Predicate.
                if set to 1, ``Src`` data will be written to the memory.
                if set to 0, the memory won't be updated.
=============== ===========================================================

Store restrictions:

========= ========== =========
Data Size ``Height`` ``Width``
========= ========== =========
byte      1 - 8      4 - 64

2-byte    1 - 8      2 - 32

4-byte    1 - 8      1 - 16

8-byte    1 - 8      1 - 8
========= ========== =========


cm_prefetch
"""""""""""

.. code-block:: c++

  template <CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0, typename T = int,
            unsigned NBlocks = 1, unsigned BlockH = 1, unsigned BlockW = 1>
  void cm_prefetch(const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
                   int16_t Pred = 1);

  template <CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default,
            CacheHint L3H = CacheHint::Default,
            int OffsetX = 0, int OffsetY = 0, typename T = int,
            unsigned NBlocks = 1, unsigned BlockH = 1, unsigned BlockW = 1>
  void cm_prefetch(const lsc::block_2d_desc<T, NBlocks, BlockH, BlockW> &Desc,
                   int16_t Pred = 1);

The compiler generates code for the hardware to perform 2D block prefetch from
an array of rectangular block(s) in memory. The rectangular blocks in
the array are assumed to be stacked horizontally in memory.
The underlying surface must be linear (non-tiled) and raw (untyped).

=============== ===========================================================
Parameter       Description
=============== ===========================================================
L1H, L2H, L3H   Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hint is set.
                Optional arguments.

OffsetX         Block offset X coordinate in addition to block start X
                coordinate set in the descriptor.
                For byte data types, it must be a multiple of 4.
                For 2-byte data types, it must be a multiple of 2.

OffsetY         Block start Y coordinate in addition to block start Y
                coordinate set in the descriptor.

Desc            Block 2D descriptor. Must be initialized.
                See ``lsc::block_2d_desc`` for more info.
                See the table below for the restrictions.

Pred            Predicate.
                if set to 1, cache data will be pre-loaded with data
                from memory. if set to 0, cache data won't be updated.
=============== ===========================================================

Prefetch restrictions:

========= ============ =============== =============
Data Size ``Height``   ``Width``       ``NumBlocks``
========= ============ =============== =============
byte      1 - 32       4 - 64          1, 2, 4

2-byte    1 - 32       2 - 32          1, 2, 4

4-byte    1 - 32       1 - 16          1, 2

8-byte    1 - 32       1 - 8           1
========= ============ =============== =============

The above restrictions are relaxed when ``CM_HAS_LSC_2D_LARGE`` macro is defined:

========= ============ ================== =============
Data Size ``Height``   ``Width``         ``NumBlocks``
========= ============ ================== =============
byte      1 - 32       4 - 64 , 128, 256  1, 2, 4

2-byte    1 - 32       2 - 32, 64, 128    1, 2, 4

4-byte    1 - 32       1 - 16, 32, 64     1, 2, 4

8-byte    1 - 32       1 - 8, 16, 32      1, 2, 4
========= ============ ================== =============


Typed load/store/prefetch
^^^^^^^^^^^^^^^^^^^^^^^^^

These functions are target-dependent and only available
when ``CM_HAS_LSC_TYPED`` macro is defined.

cm_load4_typed
""""""""""""""

.. code-block:: c++

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_load4_typed(matrix_ref<T, N, M> Data, SurfaceIndex Surface,
                      vector<unsigned, M> U, vector<unsigned, M> V = 0,
                      vector<unsigned, M> R = 0, vector<unsigned, M> LOD = 0);

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_load4_typed(matrix_ref<T, N, M> Data, vector<uint16_t, M> Pred,
                      SurfaceIndex Surface, vector<unsigned, M> U,
                      vector<unsigned, M> V = 0, vector<unsigned, M> R = 0,
                      vector<unsigned, M> LOD = 0);

The compiler generates code for the hardware to perform gathering read from
the given offsets. The results are returned in ``Data`` matrix with each
enabled channel returned in the next row of the matrix. The enabled channels
are returned in RGBA order with no gap for disabled channels. Out-of-bound
load operations return zero.

=============== ============================================================
Parameter       Description
=============== ============================================================
ChannelMask
                Mask for enabled channels.

L1H, L2H
                Load cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments. Should be both present or both omitted.

Data
                The matrix_ref object to store return values. The ``T`` type
                must be of size of DWord (i.e., int, unsigned or float).

                The size ``N`` must be equal to the number of enabled
                channels.

                The size ``M`` may have any value. HW supports native SIMD
                sizes of 16 or 32. All other sizes will be split and/or
                extended by the compiler into a sequence of SIMD16 and
                SIMD32 load operations.

Pred
                The execution predicate for conditional load.
                Optional, default value is one.

Surface
                Surface index corresponding to 1D, 2D or 3D image surface.

U
                The x coordinates of the data elements (in unit of texels)
                to be loaded from the surface.

V
                The y coordinates of the data elements to be loaded from
                non-1D images. Ignored otherwise. Optional, default value
                is zero.

R
                The z coordinates of the data elements to be loaded from
                3D images. Ignored otherwise. Optional, default value is
                zero.

LOD
                The w coordinates of the data elements to be loaded from
                images. Ignored otherwise. Optional, default value is zero.
=============== ============================================================

cm_store4_typed
"""""""""""""""

.. code-block:: c++

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_store4_typed(matrix<T, N, M> Data, SurfaceIndex Surface,
                       vector<unsigned, M> U, vector<unsigned, M> V = 0,
                       vector<unsigned, M> R = 0, vector<unsigned, M> LOD = 0);

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_store4_typed(matrix<T, N, M> Data, vector<uint16_t, M> Pred,
                       SurfaceIndex Surface, vector<unsigned, M> U,
                       vector<unsigned, M> V = 0, vector<unsigned, M> R = 0,
                       vector<unsigned, M> LOD = 0);

The compiler generates code for the hardware to perform scattered write to
the given offsets. If a required pixel channel (as per Surface State pixel
format) is missing from the channel mask setting, then the memory write
data will be undefined. In other words, the channel mask setting must have
all channels that are present in the Surface's pixel format in memory.
Out-of-bound write operations are ignored by hardware and have no side
effects.

=============== ============================================================
Parameter       Description
=============== ============================================================
ChannelMask
                Mask for enabled channels. Only contiguous channel masks are
                supported (R, GR, BGR and ABGR).

L1H, L2H
                Store cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments. Should be both present or both omitted.

Data
                The matrix holding data to be written. The ``T`` type must
                be of size of DWord (i.e., int, unsigned or float).

                The size ``N`` must be equal to the number of enabled
                channels.

                The size ``M`` may have any value. HW supports native SIMD
                sizes of 16 or 32. All other sizes will be split and/or
                extended by the compiler into a sequence of SIMD16 and
                SIMD32 store operations.

Pred
                The execution predicate for conditional store.
                Optional, default value is one.

Surface
                Surface index corresponding to 1D, 2D or 3D image surface.

U
                The x coordinates of the data elements (in unit of texels)
                to be loaded from the surface.

V
                The y coordinates of the data elements to be stored into
                non-1D images. Ignored otherwise. Optional, default value
                is zero.

R
                The z coordinates of the data elements to be stored into
                3D images. Ignored otherwise. Optional, default value is
                zero.

LOD
                The w coordinates of the data elements to be stored into
                images. Ignored otherwise. Optional, default value is zero.
=============== ============================================================

cm_prefetch4_typed
""""""""""""""""""

.. code-block:: c++

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_prefetch4_typed(SurfaceIndex Surface,
                          vector<unsigned, M> U, vector<unsigned, M> V = 0,
                          vector<unsigned, M> R = 0, vector<unsigned, M> LOD = 0);

  template <ChannelMaskType ChannelMask, CacheHint L1H = Default,
            CacheHint L2H = Default>
  void cm_prefetch4_typed(vector<uint16_t, M> Pred, SurfaceIndex Surface,
                          vector<unsigned, M> U, vector<unsigned, M> V = 0,
                          vector<unsigned, M> R = 0, vector<unsigned, M> LOD = 0);

The compiler generates code for the hardware to perform gathering prefetch from
the given offsets to L1 and/or L2 cache. No results returned. This operation
has no side effects except subsequent loads latency.

=============== ============================================================
Parameter       Description
=============== ============================================================
ChannelMask
                Mask for enabled channels.

L1H, L2H
                Prefech cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Must be present.

Pred (optional)
                The execution predicate for conditional load.

Surface
                Surface index corresponding to 1D, 2D or 3D image surface.

U
                The x coordinates of the data elements (in unit of texels)
                to be fetched from the surface.

V
                The y coordinates of the data elements to be fetched from
                non-1D images. Ignored otherwise. Optional, default value
                is zero.

R
                The z coordinates of the data elements to be fetched from
                3D images. Ignored otherwise. Optional, default value is
                zero.

LOD
                The w coordinates of the data elements to be fetched from
                images. Ignored otherwise. Optional, default value is zero.
=============== ============================================================

Typed 2D block load/store
^^^^^^^^^^^^^^^^^^^^^^^^^

These functions are target-dependent and only available
when ``CM_HAS_LSC_TYPED_2D`` macro is defined.

cm_load
"""""""

.. code-block:: c++

  template <typename T, int Height, int Width, CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_load(SurfaceIndex Idx, int X, int Y, matrix_ref<T, Height, Width> Data);


Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".
Hardware will return 0 for the out-of-bound bytes.

Note: out-of-bound accesses crossing top and/or left borders cause
an undefined behavior, when executed under NEO runtime.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.

Height          Specifies the height in number of data elements
                for this rectangular region.
                ``Height`` X ``Width`` must not exceed 256 bytes.

L1H, L2H        Cache hints. Not all the combinations are supported.
                See `Load cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to 2D image surface.

X               Block start X coordinate.
                Specifies the signed X offset in bytes.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements.

Data            The data vector read from memory.
=============== ==================================================================

cm_store
""""""""

.. code-block:: c++

  template <typename T, int Height, int Width, CacheHint L1H = CacheHint::Default,
             CacheHint L2H = CacheHint::Default>
  void cm_store(SurfaceIndex Idx, int X, int Y, matrix<T, Height, Width> Input);

The Typed 2D Block store writes to one rectangular block of a Typed 2D surface.
Raw pixels are written to memory without any hardware format conversion.
The underlying surface must be Surface 2D and Typed.

Any byte(s) of the accesed 2D block which is outside of the specified 2D Surface
bounds (Width, Height) are considered to be "out-of-bound".
For stores, hardware will not write to the out-of-bound addresses in the memory.

Note: out-of-bound accesses crossing top and/or left borders cause
an undefined behavior, when executed under NEO runtime.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.

Height          Specifies the height in number of data elements
                for this rectangular region.
                ``Height`` X ``Width`` must not exceed 256 bytes.

L1H, L2H        Cache hints. Not all the combinations are supported.
                See `Store cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to 2D image surface.

X               Block start X coordinate.
                Specifies the signed X offset in bytes.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements.

Data            The data vector written into memory.
=============== ==================================================================

cm_prefetch
"""""""""""

.. code-block:: c++

  template <typename T, int Height, int Width, CacheHint L1H = CacheHint::Default,
            CacheHint L2H = CacheHint::Default>
  void cm_prefetch(SurfaceIndex Idx, int X, int Y);

The compiler generates code for the hardware to perform 2D block prefetch from one
rectangular block in memory. The underlying surface must be Surface 2D and Typed.

Any byte(s) of the accessed 2D block which is outside of the specified
2D Surface bounds (Width, Height) are considered to be "out-of-bound".

Note: out-of-bound accesses crossing top and/or left borders cause
an undefined behavior, when executed under NEO runtime.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
T               Data type.
                Byte, 2-byte, 4-byte and 8-byte types are allowed.

Width           Specifies the width in number of data elements
                for this rectangular region.

Height          Specifies the height in number of data elements
                for this rectangular region.
                ``Height`` X ``Width`` must not exceed 256 bytes.

L1H, L2H        Cache hints. Not all the combinations are supported.
                See `Prefetch cache control policy` table.
                The compiler will emit an error when invalid hints are set.
                Optional arguments.

Idx             Surface index corresponding to 2D image surface.

X               Block start X coordinate.
                Specifies the signed X offset in bytes.

Y               Block start Y coordinate.
                Specifies the signed Y offset in number of data elements.
=============== ==================================================================

Fence
^^^^^

cm_fence
""""""""
.. code-block:: c++

  template <LSC_SFID Sfid = LSC_SFID::LSC_UGM,
          LSC_FENCE_OP FenceOp = LSC_FENCE_OP::LSC_FENCE_OP_NONE,
          LSC_SCOPE Scope = LSC_SCOPE::LSC_SCOPE_GROUP>
  void cm_fence(vector<ushort, N> Pred = 1);

The compiler generates code for the hardware to perform a fence operation that is
used to order memory transactions from the point of view of the calling thread.
The calling thread must wait for the fence operation to complete to guarantee
that pending writes, and any flushes, have completed before continuing execution.

=============== ==================================================================
Parameter       Description
=============== ==================================================================
Sfid            Specifies the memory port: LSC_SFID::LSC_UGM, LSC_SFID::LSC_TGM,
                LSC_SFID::LSC_SLM.

FenceOp         Defines what caches and how should be flushed:
                See the table below for supported values.

Scope           Defines where any memory transactions from the issuing thread are
                guaranteed to be observable once the fence is complete.
                See the table below for supported values.
                The compiler will emit an error when invalid scope is specified.
=============== ==================================================================

Supported FenceOp:

======================================== =========================================
FenceOp                                  Description
======================================== =========================================
LSC_FENCE_OP::LSC_FENCE_OP_NONE          No caches are flushed

LSC_FENCE_OP::LSC_FENCE_OP_EVICT         Evict dirty lines and invalidate
                                         clean linesNo caches are flushed

LSC_FENCE_OP::LSC_FENCE_OP_INVALIDATE    Invalidate clean lines in the cache

LSC_FENCE_OP::LSC_FENCE_OP_DISCARD       Invalidate dirty lines without write-back
                                         to next level

LSC_FENCE_OP::LSC_FENCE_OP_CLEAN         Write-back dirty lines to the next level,
                                         but keep in the cache as clean.
                                         Lines that were already marked valid stay
                                         in the valid state.

LSC_FENCE_OP::LSC_FENCE_OP_FLUSHL3       Flush read-write section of the L3 cache,
                                         but leave L1 and L2 caches untouched
======================================== =========================================

Supported Scope:

================================== ===============================================
Scope                              Description
================================== ===============================================
LSC_SCOPE::LSC_SCOPE_GROUP         Wait until all previous memory transactions
                                   from this thread are observed within
                                   the local thread-group

LSC_SCOPE::LSC_SCOPE_LOCAL         Wait until all previous memory transactions
                                   from this thread are observed within
                                   the local sub-slice

LSC_SCOPE::LSC_SCOPE_TILE          Wait until all previous memory transactions
                                   from this thread are observed in the local tile

LSC_SCOPE::LSC_SCOPE_GPU           Wait until all previous memory transactions
                                   from this thread are observed in the local GPU

LSC_SCOPE::LSC_SCOPE_GPUS          Wait until all previous memory transactions
                                   from this thread are observed across all GPUs
                                   in the system.

LSC_SCOPE::LSC_SCOPE_SYSTEM        Wait until all previous memory transactions
                                   from this thread are observed at the system
                                   level. This scope is available for UGM
                                   data-port only.

LSC_SCOPE::LSC_SCOPE_SYSACQ        For GPUs that do not follow PCIe Write ordering
                                   for downstream writes targeting device memory,
                                   it will commit to device memory all downstream
                                   and peer writes that have reached the device.
                                   This scope is available for UGM data-port only.
======================================== =========================================


4.6 Dataport Interface
----------------------

Starting with Xe2, these intrinsics are considered obsolete and thus
translated into LSC intrinsics.

Media Block Read/Write
^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides several qualifiers that can be applied to the surface index:

=============== ============================================================
Qualifier
=============== ============================================================
TOP_FIELD       indicates only the top field surface data are needed.

BOTTOM_FIELD    indicates only the bottom field surface data are needed.
                This may not be used together with TOP_FIELD.
=============== ============================================================

For out-of-bounds read, the address is clamped to the nearest edge of the
surface, and the pixel in that position is returned.  Out-of-bound writes are
dropped.  Out-of-bound behavior is undefined, however, if the surface width is
not DWord-aligned.

The valid combination of block width and height is described in the following table.

======================= ===========================
Block Width (bytes)     Maximum Block Height (rows)
======================= ===========================
1-4                     64
5-8                     32
9-16                    16
17-32                   8
33-64 {BDW+}            4
======================= ===========================

Supported Surfaces:

======= ======= =======================================================
Format  Type    Notes
======= ======= =======================================================
Any     2D      * Block width and offset should be pixel-aligned for read.
                * For CM_SURFACE_FORMAT_YUY2,  Block width and offset should
                  be pixel pair aligned
                * For Block width > 32 read or write, the surface must be linear or x-
                  tiled.
======= ======= =======================================================


read
""""

.. code-block:: c++

  void read(SurfaceIndex IND, int X, int Y, matrix_ref<TYPE, M, N> m);

=============== ============================================================
Parameters
=============== ============================================================
IND
                surface index, which must correspond to a 2D type surface
                (optionally qualified as specified above)

X
                zero based X-coordinate of the left upper rectangle corner in
                BYTES;

                For regular surfaces, the X-offset must be pixel aligned.

                For surfaces with compact format (e.g., YUY2), this must be
                DWord (i.e. 4 bytes) aligned.

Y
                zero based Y-coordinate of the left upper rectangle corner in
                ROWS.

m
                the data location to be read / written:
                the width of matrix m must be pixel aligned.
=============== ============================================================

Usage example:

.. code-block:: c++

  read(TOP_FIELD(ind), ...)  // only reads top field surface data

write
"""""

.. code-block:: c++

  void write(SurfaceIndex IND, int X, int Y, const matrix m);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a 2D type surface
                (optionally qualified as specified above)

X
                zero based X-coordinate of the left upper rectangle corner in BYTES;
                must be DWord (i.e. 4 bytes) aligned.

Y
                zero based Y-coordinate of the left upper rectangle corner in ROWS.

m
                the data location to be read / written:
                the width of matrix m must be DWord (i.e. 4 bytes) aligned.
=============== ================================================================


Media Block Read/Write for Planar Surface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides several qualifiers that can be applied to the surface index:

=============== ================================================================
Qualifier
=============== ================================================================
TOP_FIELD
                indicates only the top field surface data are needed.

BOTTOM_FIELD
                indicates only the bottom field surface data are needed.  This may not be
                used together with TOP_FIELD.
=============== ================================================================

The planar media block read and write have the same restrictions on the block
width and height as the regular media read/write.

Supported Surfaces:

========================= ======= =======================================================
Format                    Type    Notes
========================= ======= =======================================================
CM_SURFACE_FORMAT_NV12    2D      * Block width and offset should be pixel-aligned for read.
                                  * Only Y and UV plane are available.
========================= ======= =======================================================

read_plane
""""""""""

.. code-block:: c++

  void read_plane(SurfaceIndex IND, CmSurfacePlaneIndex plane_index, int X, int Y, matrix_ref m);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a 2D type surface with
                planar format (optionally qualified as specified above)

plane_index
                the index to the plane. CmSurfacePlaneIndex is an enumerator
                type with 4 possible values, as listed below. This must be a
                compile time constant.

                * GENX_SURFACE_Y_PLANE
                * GENX_SURFACE_U_PLANE
                * GENX_SURFACE_UV_PLANE
                * GENX_SURFACE_V_PLANE

X
                zero based X-coordinate of the left upper rectangle corner in BYTES;

                The alignment rule applies to each sub-plane surface
                separately.  For example, for NV12 format, the Y-plane is Byte
                aligned, while the UV plane is Word aligned.

Y
                zero based Y-coordinate of the left upper rectangle corner in ROWS.

m
                the data location to be read / written:
                the width of matrix m must be pixel aligned.
=============== ================================================================

write_plane
"""""""""""
.. code-block:: c++

  void write_plane(SurfaceIndex IND, int plane_index, int X, int Y, const matrix m);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a 2D type surface with
                planar format (optionally qualified as specified above)

plane_index
                the index to the plane. CmSurfacePlaneIndex is an enumerator
                type with 4 possible values, as listed below. This must be a
                compile time constant.

                * GENX_SURFACE_Y_PLANE
                * GENX_SURFACE_U_PLANE
                * GENX_SURFACE_UV_PLANE
                * GENX_SURFACE_V_PLANE

X
                zero based X-coordinate of the left upper rectangle corner in BYTES;
                must be DWord (i.e. 4 bytes) aligned.

Y
                zero based Y-coordinate of the left upper rectangle corner in ROWS.

m
                the data location to be read / written:
                the width of matrix m must be DWord (i.e. 4 bytes) aligned.
=============== ================================================================

OWord Block Read/Write
^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the following qualifiers for specifying the input usage options:

=================== ================================================================
Qualifier
=================== ================================================================
DWALIGNED           indicates that the offset is DWord aligned.
MODIFIED_DWALIGNED  deprecated: same as DWALIGNED, as there is no modified bit in
                    current hardware.
=================== ================================================================

Out-of-bound reads return zero, while out-of-bound writes are dropped.

Note: C for Metal currently only supports read/write operations of OWord Block data with
size <= 128 bytes.

Supported Surfaces:

========================= ======= =======================================================
Format                    Type    Notes
========================= ======= =======================================================
N/A                       Buffer
========================= ======= =======================================================


read
""""

.. code-block:: c++

  void read(SurfaceIndex IND, int offset, vector_ref v);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer
                (optionally qualified as specified above)

offset
                zero based offset of the input buffer in *bytes*.  Must be
                OWord (i.e. 16 bytes) aligned, but need be only DWord (i.e. 4
                bytes) aligned if the DWALIGNED or MODIFIED_DWALIGNED modifier
                is present.

v
                the data location to be read.
                the width of vector v must be OWord (i.e. 16 bytes) aligned.
=============== ================================================================

Usage example:

.. code-block:: c++

  read(DWALIGNED(ind), ...)  // the offset ind is DWord aligned
  read(ind, ...)             // the offset ind is OWord aligned

write
"""""

.. code-block:: c++

  void write(SurfaceIndex IND, int offset, const vector v);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer
                (optionally qualified as specified above)

offset
                zero based offset of the output buffer in *bytes*;
                must be OWord (i.e. 16 bytes) aligned.

v
                the data location to be written.
                the width of vector v can be only 1, 2, 4 or 8 OWords.
=============== ================================================================


Scattered Read/Write
^^^^^^^^^^^^^^^^^^^^

Out-of-bound reads return zero, while out-of-bound writes are dropped.

Scattered read/write may appear in SIMD control flow blocks, as long as it size matches
that of the controlling SIMD expression.

Supported Surfaces:

========================= ======= =======================================================
Format                    Type    Notes
========================= ======= =======================================================
N/A                       Buffer
========================= ======= =======================================================

read
""""

.. code-block:: c++

  void read(SurfaceIndex IND, uint global_offset, vector<uint, N> element_offset, vector_ref<T, N> v);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer

global_offset
                zero based global offset of a set of sattered elements
                to be read from the surface. This offset is in units of the
                size of the elements.

element_offset
                zero based offset of each element (relative to the global
                offset) to be read; N must be a power of 2 and no more than 32.
                This offset is in units of the size of the elements.

v
                the data location to store the return result.
                N must be equal to the length of element_offset.
                T can be either char, uchar, short, ushort, int, uint or float.
=============== ================================================================

write
"""""

.. code-block:: c++

  void write(SurfaceIndex IND, uint global_offset, vector<uint, N> element_offset, vector<T, N> v);

  or

  void write(SurfaceIndex IND, uint global_offset, uint element_offset, T val);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer.

global_offset
                zero based global offset of a set of scattered elements
                to be written to the surface. This offset is in units of the
                size of the elements.

element_offset
                zero based offset of each element (relative to the global
                offset) to be written; N must be a power of 2 and no more than 32, or a scalar.
                This offset is in units of the size of the elements.

v
                data location that contains the data to be written.
                N must be equal to the length of element_offset or else a scalar.
                T can be either char, uchar, short, ushort, int, uint or float.
=============== ================================================================

Scaled Scattered Read/Write
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Scaled scattered read/write are similar to scattered read/write.
The only difference is that the scaled version takes byte offsets and
there is no alignment restrictions on either offsets.

read_scaled
"""""""""""

.. code-block:: c++

  void read_scaled(SurfaceIndex IND, uint global_offset, vector<uint, N> element_offset, vector_ref<T, N> v);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer

global_offset
                zero based global offset of a set of sattered elements
                to be read from the surface. This offset is in bytes.

element_offset
                zero based offset of each element (relative to the global
                offset) to be read; N must be a power of 2 and no more than 32.
                This offset is in bytes.

v
                the data location to store the return result.
                N must be equal to the length of element_offset.
                T can be either char, uchar, short, ushort, int, uint or float.
=============== ================================================================

write_scaled
""""""""""""

.. code-block:: c++

  void write_scaled(SurfaceIndex IND, uint global_offset, vector<uint, N> element_offset, vector<T, N> v);

=============== ================================================================
Parameters
=============== ================================================================
IND
                surface index, which must correspond to a buffer.

global_offset
                zero based global offset of a set of scattered elements
                to be written to the surface. This offset is in bytes.

element_offset
                zero based offset of each element (relative to the global
                offset) to be written; N must be a power of 2 and no more than 32, or a scalar.
                This offset is in bytes.

v
                data location that contains the data to be written.
                N must be equal to the length of element_offset.
                T can be either char, uchar, short, ushort, int, uint or float.
=============== ================================================================

DWord Atomic Write
^^^^^^^^^^^^^^^^^^

Performs atomic read-modify-write operations on the N element offsets
specified, where N can be 1, 2, 4, 8, 16, or 32.

.. code-block:: c++

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset,
               vector<T, N> src0, vector<T, N> src1, vector_ref<T, N> v);

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset,
               vector<T, N> src0, vector<T, N> src1);

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset,
               vector<T, N> src0, vector_ref<T, N> v);

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset,
               vector<T, N> src0);

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset,
               vector_ref<T, N> v);

  template <CmAtomicOpType Op, typename T, int N>
  write_atomic(vector<ushort, N> mask, SurfaceIndex index, vector<uint, N> element_offset);

=============== ============================================================
Parameters
=============== ============================================================
op              the atomic operation kind

mask            (optional) predicate to specify enabled channels.

index           surface index, which must correspond to a buffer.

element_offset  zero based offset of each DWord to be written. This is in units of DWords.

src0            the first source operand for the specified atomic operation.

src1            the second source operand for the specified atomic operation.

v               the data location to store the returned result, which corresponds to the old surface data value.
=============== ============================================================

Out-of-bound reads return zero, while out-of-bound writes are dropped.

Supported Surfaces:

========================= ======= =======================================================
Format                    Type    Notes
========================= ======= =======================================================
N/A                       Buffer
========================= ======= =======================================================

Untyped Surface Read/Write {Gen7+}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Out-of-bound reads return zero, while out-of-bound writes are dropped.

Supported Surfaces:

========================= ======= =======================================================
Format                    Type    Notes
========================= ======= =======================================================
N/A                       Buffer
========================= ======= =======================================================

read_untyped
""""""""""""

.. code-block:: c++

  void read_untyped(SurfaceIndex IND, ChannelMaskType channelMask,
    matrix_ref<T, N1, N2> m,  const vector<uint, N2> u);

=============== ============================================================
Parameters
=============== ============================================================
IND
                surface index, which must correspond to a buffer

channelMask
                enabled channels.  It must be a compile time constant.

m
                the matrix to store the return results. The type T must be of
                size of DWord (i.e., int, uint, or float).

                The size N1 is at least the number of enabled channels and N2
                must be either 8 or 16.

u
                the offsets of the data elements to be read from surface,
                which must be in unit of DWords. The size N2 must be either 8 or 16.
=============== ============================================================

The compiler generates code for GenX hardware to perform scattered read from
offsets given by u.  The results are returned in m with each enabled channel
returned in the next row of m.  The enabled channels are returned in R, G, B, A
order with no gap in m for disabled channels.

write_untyped
"""""""""""""

.. code-block:: c++

  void write_untyped(SurfaceIndex IND, ChannelMaskType channelMask,
    matrix_ref<T, N1, N2> m, const vector<uint, N2> u);

=============== ============================================================
Parameters
=============== ============================================================
IND
                surface index, which must correspond to a buffer

channelMask
                enabled channels. It must be a compile time constant.

m
                the matrix that stores the data to be written. The type T
                must be of size of DWord (i.e., int, uint, or float).

                The size N1 is at least the number of enabled channels and N2
                must be either 8 or 16.

u
                the offsets of the data elements to written surface, which
                must be in unit of DWords. The size N2 must be either 8 or 16.
=============== ============================================================

The compiler generates code for GenX hardware to perform scattered write to
offsets given by u. Only enabled channels are written to the surface.


Typed Surface Read/Write {Gen7+}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Out-of-bound reads return zero, while out-of-bound writes are dropped.

Supported Surfaces:

========================= ======= === === === === =======================================
Format                    Type    R   G   B   A   Message Data Type
========================= ======= === === === === =======================================
CM_SURFACE_FORMAT_R32F    2D/3D   R               float
========================= ======= === === === === =======================================


read_typed
""""""""""

.. code-block:: c++

  void read_typed(SurfaceIndex IND, ChannelMaskType channelMask,
    matrix_ref<T, N1, N2> m, const vector<uint, N2> u,
    const vector<uint, N2> v = 0, const vector<uint, N2> r = 0);

=============== ============================================================
Parameters
=============== ============================================================
IND
                surface index, which must correspond to a 1D, 2D or 3D
                surface

channelMask
                enabled channels.  It must be a compile time constant.

m
                the matrix to store the return results. The type T must be of
                size of DWord (i.e., int, uint, or float).

                The size N1 is at least the number of enabled channels and N2
                must be 8, 16, or 32.

u
                the x coordinates of the data elements to be read from
                surface, which must be in unit of pixels. The size N2 must
                be 8, 16, or 32.

v
                (optional, default = 0) the y coordinates of the data
                elements to be read from non-1D surface types; ignored
                otherwise.

r
                (optional, default = 0) the z coordinates of the data
                elements to be read from 3D surface types; ignored
                otherwise.
=============== ============================================================

The compiler generates code for GenX hardware to perform scattered read from
the given offsets.  The results are returned in m with each enabled channel
returned in the next row of m.  The enabled channels are returned in R, G, B, A
order with no gap in m for disabled channels.

write_typed
"""""""""""

.. code-block:: c++

  void write_typed(SurfaceIndex IND, ChannelMaskType channelMask,
    matrix_ref<T, N1, N2> m, vector<uint, N2> u,
    vector<uint, N2> v = 0, vector<uint, N2> r = 0);

=============== ============================================================
Parameters
=============== ============================================================
IND
                surface index, which must correspond to a 1D, 2D or 3D surface

channelMask
                enabled channels.  It must be a compile time constant.

m
                the matrix that stores the data to be written. The type T must
                be of size of DWord (i.e., int, uint, or float). The size N1 is
                at least the number of enabled channels and N2 must be 8,
                16, or 32.

u
                the x coordinates of the data elements to be written to
                surface, which must be in unit of pixels. The size N2 must be
                8, 16, or 32.

v
                (optional, default = 0) the y coordinates of the data elements
                to be written to non-1D surface types; ignored otherwise.

r
                (optional, default = 0) the z coordinates of the data elements
                to be written to 3D surface types; ignored otherwise.
=============== ============================================================

The compiler generates code for GenX hardware to perform scattered write to the
given offsets. Only enabled channels are written to the surface.

Typed Surface Atomic Write
^^^^^^^^^^^^^^^^^^^^^^^^^^

{Only for new cm-llvm compiler cmc - not supported for legacy cm-icl compiler - icl}

.. code-block:: c++

  void write_typed_atomic<ATOMIC_OP>(vector<ushort, N> mask, SurfaceIndex IND,
    vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1, vector<uint, N> u,
    vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD);

=============== ============================================================
Parameters
=============== ============================================================
ATOMIC_OP
                atomic operation to perform. These are the same enums as the
                other atomic operations.  Please refer to these for details.

mask            (optional) predicate to specify enabled channels.

IND
                surface index, which must correspond to a 1D, 2D or 3D
                surface [6].

ret
                the vector that stores the data returned. This contains the
                values in the surface before the atomic operation was
                performed. The type T must be of size of DWord and must be
                unsigned except for the ATOMIC_IMIN and ATOMIC_IMAX variants
                where it can be signed. The size N can be 1, 2, 4, 8, 16, or 32

Src0
                the vector containing src0 data for the operation. Not
                required for INC and DEC operations. Must be unsigned int.

Src1
                the vector containing src1 data for the operation. Only
                required for CMPXCHG operations. Must be unsigned int.

u
                the x coordinates of the data elements to be written to
                surface, which must be in unit of pixels.

v
                (optional) the y coordinates of the data elements to be
                written to non-1D surface types; ignored otherwise.

r
                (optional) the z coordinates of the data elements to be
                written to 3D surface types; ignored otherwise.

LOD
                (optional) the w coordinates of the data elements to be
                written to 3D surface types; ignored otherwise.
=============== ============================================================

The compiler generates code for GenX hardware to perform scattered write to the
given offsets with the associated atomic operation. Out-of-bound writes are
dropped.

=============== =============== =============== =============== ===============
Surface Type    U               V               R               LOD
=============== =============== =============== =============== ===============
1D              X pixel address N/A             N/A             LOD
1D_array        X pixel address Array index     N/A             LOD
2D              X pixel address Y pixel address N/A             LOD
2D_array        X pixel address Y pixel address Array index     LOD
3D              X pixel address Y pixel address Z pixel address LOD
=============== =============== =============== =============== ===============

Supported Surfaces:

=========================== ===================== === === === === ===============
Format                      Type                  R   G   B   A   Message Data
                                                                  Type
=========================== ===================== === === === === ===============
CM_SURFACE_FORMAT_R32_UINT  1D/2D/3D/CUBE/BUFFER                  Integer
CM_SURFACE_FORMAT_R32_SINT  1D/2D/3D/CUBE/BUFFER                  Integer
=========================== ===================== === === === === ===============



4.7 Shared Local Memory (SLM) and Groups Interface
--------------------------------------------------


The shared local memory (SLM) is a high bandwidth memory that is not backed up by system memory.  It
is only supported for Gen7+. Its contents are uninitialized after creation, and its contents disappear when
de-allocated.

The host program needs to organize all the threads into groups (collection of threads) by specifying the
group-space and the thread-space within each group. This group and thread space information can be
subsequently used to execute a kernel in that space. Formal description of the host API functions can be
found in 'C for Metal runtime API Specification' [6].

A kernel needs to specify which data would be allocated in the SLM (cm_slm_alloc). All the declared SLM
buffers are allocated once for each group generated and are accessible only from the threads within that
group. Runtime and hardware will keep track of each group's SLM data separately.

In the current SLM support in CM, SLM values are not first-class operands which mean they cannot be
used directly in C for Metal operations. They need to be read to GRF registers (cm_slm_read) before being used.
After their use, they could be written back to SLM if needed (cm_slm_write). However, if data written to
SLM by one thread needs to be read by other threads in the group, a barrier (cm_barrier) needs to be
inserted to make the writes visible to the reads. If the SLM writes needed are only for loading data from
global memory to SLM, then the kernel can simply use cm_slm_load to load data from memory to SLM --
cm_slm_load would transparently distribute the work, of reading data from memory and writing to SLM,
amongst the available threads in the current group and insert the necessary barrier needed to make the
loaded data visible to all threads. The kernel could also perform atomic operations on data residing in the
SLM (cm_slm_atomic).

In order to distinguish data and operations between threads in a group, the kernel can get the thread IDs
through cm_local_id and thread counts in a group through cm_local_size. For data distinction across
groups, kernel can call cm_group_id and cm_group_count to get the group IDs and group dimensions
respectively in the group space. For programming convenience, some useful functions are provided to get
the linearized IDs and sizes in one dimensional group and thread spaces.

Note on EMU-mode Support: Currently EMU mode is not supported if the kernel uses cm_barrier().
However, if the only purpose of the barrier is to load data from memory to SLM, the kernel can use
cm_slm_load() which is supported in EMU mode with one restriction -- only one SLM buffer is used, i.e.,
there is a single call to cm_slm_alloc().

Note on Performance: Use cm_slm_read4() and cm_slm_write4() instead of cm_slm_read() and
cm_slm_write() whenever possible, as read4 and write4 versions provide much higher read/write
bandwidth (an order of magnitude higher) on Gen7. Read4 and write4 versions can read/write 4 dwords
per address -- even if all 4 dwords per address are not needed, use read4/write4 (with appropriate mask
argument) for higher performance. Also note that, data read or written using read4/write4 intrinsics are
transposed by the hardware -- the application should try to take advantage of this; otherwise, it would
incur the overhead of transposing the data back to the desired layout.

In this section, we describe the kernel APIs needed to explicitly control and use the SLM.

ID and Size API Functions
^^^^^^^^^^^^^^^^^^^^^^^^^

cm_local_id
"""""""""""
.. code-block:: c++

  uint cm_local_id (uint dim);

Get the thread ID of this thread in the 2D thread-space of its
group along dimension 'dim'.

cm_local_size
"""""""""""""

.. code-block:: c++

  uint cm_local_size (uint dim);

Get the number of threads along dimension 'dim' in the 2D
thread-space of the current group.

cm_group_id
"""""""""""

.. code-block:: c++

  uint cm_group_id (uint dim);

Get the group ID of the group this thread belongs to, in the 2D
group-space, along the dimension 'dim'.

cm_group_count
""""""""""""""

.. code-block:: c++

  uint cm_group_count (uint dim);

Get the number of groups in the 2D group-space along the
dimension 'dim'.

cm_linear_local_id
""""""""""""""""""

.. code-block:: c++

  uint cm_linear_local_id (void);

Get the linear thread ID of this thread in its group.

cm_linear_global_id
"""""""""""""""""""

.. code-block:: c++

  uint cm_linear_global_id (void);

Get the linear global ID of this thread.

cm_linear_local_size
""""""""""""""""""""

.. code-block:: c++

  uint cm_linear_local_size (void);

Get the total number of threads in a group.

cm_linear_group_id
""""""""""""""""""

.. code-block:: c++

  uint cm_linear_group_id (void);

Get the linear group ID of this group.

cm_linear_group_count
"""""""""""""""""""""

.. code-block:: c++

  uint cm_linear_group_count (void);

Get the total number of groups.

cm_linear_global_size
"""""""""""""""""""""

.. code-block:: c++

  uint cm_linear_global_size (void);

Get the total number of threads.

cm_global_id
""""""""""""
.. code-block:: c++

  uint cm_global_id (uint dim);

Get the global thread ID of this thread along dimension 'dim'.


SLM Functions
^^^^^^^^^^^^^

cm_slm_init
"""""""""""

.. code-block:: c++

  void cm_slm_init(uint slmSize);

Initializes SLM for the kernel. SLM size (in Bytes) needed per
group has to be specified in 'slmSize'. Maximum SLM size per
group that can be specified here is 64 KB on Gen7.

cm_slm_alloc
""""""""""""

.. code-block:: c++

  uint cm_slm_alloc(uint bufferSize);

Allocate a SLM buffer of size 'bufferSize' in current-group's SLM
-- this is allocated once per group; Maximum of all SLM
allocations is 64KB and is always rounded up to next multiple of
4KB. Currently, this function would return an integer identifier
of the SLM buffer allocated -- this is the byte-offset of this
buffer in SLM. In the first implementation, there would be no
support of freeing SLM buffer and reusing it -- can be supported
later.

cm_slm_write
""""""""""""

.. code-block:: c++

  void cm_slm_write(uint slmBuffer,
             vector_ref<ushort, N> v_Addr,
             vector_ref<TYPE, N> v_Src);

Write N data elements (byte, word, dword or qword) given in the
vector 'v_Src' into the SLM buffer 'slmBuffer' at the element-
offsets specified in 'v_Addr'. Note that the addresses are in
units of element size.

A variant of this API 'cm_slm_write_scaled" is provided with
the same parameters, except the addresses are in units of bytes.

Note: Writes to overlapping addresses will have undefined
write ordering.

cm_slm_write4
"""""""""""""

.. code-block:: c++

  void cm_slm_write4(uint slmBuffer,
             vector_ref<uint, N> v_Addr,
             vector_ref<TYPE, M> v_Src,
             SLM_ChannelMaskType mask);

Where:

* N = 8, 16, or 32;
* M is at least N*C where C is the
  number of channels enabled in
  'mask'.
* TYPE must be of size dword (so, could be int, uint, or float).
* 'mask' specifies the channels that
  are enabled -- it has to be a compile-time constant of the enum type
  SLM_ChannelMaskType that can have the following 15 values:

  * SLM_R_ENABLE
  * SLM_G_ENABLE
  * SLM_GR_ENABLE
  * SLM_B_ENABLE
  * SLM_BR_ENABLE
  * SLM_BG_ENABLE
  * SLM_BGR_ENABLE
  * SLM_A_ENABLE
  * SLM_AR_ENABLE
  * SLM_AG_ENABLE
  * SLM_AGR_ENABLE
  * SLM_AB_ENABLE
  * SLM_ABR_ENABLE
  * SLM_ABG_ENABLE
  * SLM_ABGR_ENABLE

Write N 4-element vectors, say {R,G,B,A}, where each
element is of size dword and is also referred to as a channel.

The elements to be written must be in the vector 'v_Src' and
organized channel-wise, i.e. all R's followed by all G's, and so
on. Address of each 4-element vector inside the SLM buffer
'slmBuffer' must be specified in 'v_Addr'. Note that the
addresses are in units of element size. One or more channels in
the 4-element vector could be masked, and v_Src must contain
only the unmasked or enabled elements.  The argument 'mask'
specifies the channels that are enabled. Only the enabled
channels are written to SLM.
e.g. if mask = SLM_BR_ENABLE (i.e. only R and B channels
enabled), and v_Src is B7B6B5B4B3B2B1B0 R7R6R5R4R3R2R1R0,
8 vectors written to SLM are as (x B7 x R7), (x B6 x R6), ... (x B0 x
R0) -  where 'x' means the value is not written.

Note: cm_slm_write4() provides an order of magnitude more
bandwidth than cm_slm_write() on Gen7 hardware. Whenever
possible, cm_slm_write4() should be used instead of
cm_slm_write() to achieve higher performance.

Note: Writes to overlapping addresses will have undefined
write ordering.

A variant of this API 'cm_slm_write4_scaled" is provided with
the same parameters, except the addresses are in units of bytes.

cm_slm_read
"""""""""""

.. code-block:: c++

  void cm_slm_read(uint slmBuffer,
                   vector_ref<ushort, N> v_Addr,
                   vector_ref<TYPE, N> v_Dst);

Read N data elements (byte, word, dword or qword) from the
SLM buffer 'slmBuffer' at the element-offsets specified in
'v_Addr', and write back into the vector 'v_Dst'. Note that the
addresses are in units of element size.

A variant of this API 'cm_slm_read_scaled" is provided with
the same parameters, except the addresses are in units of bytes.

cm_slm_read4
""""""""""""

.. code-block:: c++

  void cm_slm_read4(uint slmBuffer,
                    vector_ref<uint, N> v_Addr,
                    vector_ref<TYPE, M> v_Dst,
                    SLM_ChannelMaskType mask);

Where:

* N = 8, 16, or 32;
* M is at least N*C where C is the number of channels enabled in 'mask';
* TYPE must be of size dword (so, could be int, uint, or float);
* 'mask' specifies the channels that are enabled -- it has to be a compile-time
  constant of the enum type SLM_ChannelMaskType that can have the following 15 values:

  * SLM_R_ENABLE
  * SLM_G_ENABLE
  * SLM_GR_ENABLE
  * SLM_B_ENABLE
  * SLM_BR_ENABLE
  * SLM_BG_ENABLE
  * SLM_BGR_ENABLE
  * SLM_A_ENABLE
  * SLM_AR_ENABLE
  * SLM_AG_ENABLE
  * SLM_AGR_ENABLE
  * SLM_AB_ENABLE
  * SLM_ABR_ENABLE
  * SLM_ABG_ENABLE
  * SLM_ABGR_ENABLE

Read N 4-element vectors, say {R,G,B,A}, where each
element is of size dword and is also referred to as a channel.

The elements read from SLM are written back to the vector
'v_Dst' and organized channel-wise, i.e. all R's followed by all
G's, and so on. Address of each 4-element vector inside the
SLM buffer 'slmBuffer' must be specified in 'v_Addr'. Note that
the addresses are in units of element size. One or more
channels in the 4-element vector could be masked, and v_Dst
contains only the unmasked elements. Only the lower 4 bits of
'mask' specify the elements masked. A '1' implies that the
corresponding element of each vector will not be read from
SLM.
e.g. if mask = SLM_BR_ENABLE, 'v_Dst' would contain
B7B6B5B4B3B2B1B0 R7R6R5R4R3R2R1R0, where the data elements
correspond to the 8 vectors (x B7 x R7), (x B6 x R6), ... (x B0 x R0)
read from SLM at the addresses specified in v_Addr, where 'x'
means the value is not read.

Note: This provides much more (order of magnitude) read
bandwidth than cm_slm_read() on Gen7 hardware. Whenever
possible, cm_slm_read4() should be used instead of
cm_slm_read() to achieve higher performance.

A variant of this API 'cm_slm_read4_scaled" is provided with
the same parameters, except the addresses are in units of bytes.

cm_slm_fence
""""""""""""

.. code-block:: c++

  void cm_slm_fence(unsigned char mask);

For Gen10+, cm_slm_fence(CM_GLOBAL_COHERENT_FENCE) must be
added before a barrier to enforce read/write ordering.

cm_barrier
""""""""""

.. code-block:: c++

  void cm_barrier(void);

Inserts a barrier to ensure all writes to SLM before this point
would be henceforth visible to other threads in the same
group.

cm_global_barrier
"""""""""""""""""

.. code-block:: c++

  void cm_global_barrier(void);

Inserts a barrier to synchronize all work-items executing the
kernel. The barrier should only be used in cooperative kernels,
otherwise behavior is undefined.

cm_sbarrier
"""""""""""

.. code-block:: c++

  void cm_sbarrier(uint flag);

Split a barrier into two separate events: 1) signal: cm_sbarrier(1);
2) wait: cm_sbarrier(0). The input parameter flag must be a compile-time constant.

cm_slm_load
"""""""""""

.. code-block:: c++

  // statefull read
  void cm_slm_load(uint slmBuffer,
                   SurfaceIndex memSurfIndex,
                   uint memOffset,
                   uint loadSize);

  // stateless read
  void cm_slm_load(uint slmBuffer,
                   svmptr_t addr,
                   uint memOffset,
                   uint loadSize);

Load 'loadSize' bytes from memory surface 'memSurfIndex' or address 'addr'
starting at 'memOffset' to the SLM buffer 'slmBuffer'.
'loadSize' must be a multiple of 256.

cm_slm_atomic
"""""""""""""

.. code-block:: c++

  void cm_slm_atomic(uint slmBuffer,
                     CmAtomicOpType aop,
                     T  v_Addr,
                     T1 v_Dst,
                     T2 v_Src0,
                     T3 v_Src1);

Where:

* v_Src0 and v_Src1 are optional and are needed based on
  the atomic operation 'aop';
* Type T is vector_ref<ushort, N>, where N is 8 or 16.
* The types T1, T2, T3 are either
  vector_ref<TYPE, N>, where TYPE could be int or uint, and N is 8 or 16.
* v_Src0 and v_Src1 both are needed
  when 'aop' is ATOMIC_CMPXCHG.
  No sources needed when 'aop' is
  ATOMIC_INC, ATOMIC_DEC, AND
  ATOMIC_PREDEC.
  For all other atomic operations, only
  one source v_Src0 is needed.
* v_Dst can optionally be NULL which
  indicates no return value is to be
  used.
* 'aop' specifies the atomic operation.
  It has to be a compile-time constant.

This causes atomic read-modify-write operations on the
destination locations addressed.

The destination locations are given in 'v_Addr' and the desired
atomic operation is specified as 'aop'. The value returned in
'v_Dst' depends on the atomic operation. Whether the two
optional sources, 'v_Src0' and 'v_Src1', are needed also
depends on the atomic operation.

SLM Block Read/Write
^^^^^^^^^^^^^^^^^^^^

Out-of-bound reads return zero, while out-of-bound writes are dropped.

cm_slm_block_read
"""""""""""""""""

.. code-block:: c++

  void cm_slm_block_read(uint slmBuffer, CmBufferAttrib attr, int offset, vector_ref<T, N> v);

Reads N data elements (byte, word, dword, qword) from a single block starting at
(slmBuffer + offset) address to the N addresses given in the vector ‘v’.

=============== ================================================================
Parameters
=============== ================================================================
slmBuffer
                must correspond to a SLM-buffer

attr
                attribute to specify the offset alignment
                can have the following values: GENX_NONE, GENX_DWALIGNED.

offset
                zero based offset of the input buffer in *bytes*. Must be
                OWord (i.e. 16 bytes) aligned for GENX_NONE attribute, but need be only
                DWord (i.e. 4 bytes) aligned for GENX_DWALIGNED attribute.

v
                the data location to be read into.
=============== ================================================================

Note: If dword aligned SLM block load is not supported by HW a gather will be generated instead.

cm_slm_block_write
""""""""""""""""""

.. code-block:: c++

  void cm_slm_block_write(uint slmBuffer, int offset, const vector<T, N> v);

Write N data elements (byte, word, dword, qword) given in the vector 'v' as a
single block starting at (slmBuffer + offset) address.

=============== ================================================================
Parameters
=============== ================================================================
slmBuffer
                must correspond to a SLM-buffer

offset
                zero based offset of the input buffer in *bytes*;
                must be OWord (i.e. 16 bytes) aligned.

v
                the data to be written.
=============== ================================================================



4.8 Sampler Interface
---------------------

C for Metal provides the user with the following sampler function support:

sample16
^^^^^^^^

.. code-block:: c++

  void sample16(matrix_ref<T, N, 16> m, ChannelMaskType channelMask,
    SurfaceIndex surfIndex, SamplerIndex sampIndex, vector<float, 16> u,
    vector<float, 16> v = 0, vector<float, 16> r = 0);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results, where N is at least
                the number of enabled channels. The element type T can be
                one of float, int, or unsigned int based on the format of
                the surface being sampled.

channelMask
                enabled channels.  It must be a compile time constant.

surfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

sampIndex
                the index into the sampler state table. This is an abstract
                handle that represents the sampler state created by C for Metal host
                runtime [6] and must be passed through kernel function
                parameters. C for Metal does not allow the explicit use of
                local/global variable or modification of such abstract data
                types in kernel functions, except used as function call
                argument.

u
                the normalized x coordinates of the texels to be sampled.

v
                (optional, default = 0) the normalized y coordinates for
                non-1D surface types; ignored otherwise.

r
                (optional, default = 0) the normalized z coordinates for any
                3D surface types; ignored otherwise.
=============== ============================================================

The compiler generates code for GenX hardware to sample 16 texels given by u,
v, and r.  The results are returned in m with each enabled channel returned in
the next row of m.  The enabled channels are returned in R, G, B, A order with
no gap in m for disabled channels.

Supported Surfaces:

================================= ======= === === === === =======================================
Format                            Type    R   G   B   A   Return Type
================================= ======= === === === === =======================================
CM_SURFACE_FORMAT_A8R8G8B8        2D/3D   R   G   B   A   float
CM_SURFACE_FORMAT_A8              2D/3D               A   float
CM_SURFACE_FORMAT_YUY2            2D      V   Y   U       float
CM_SURFACE_FORMAT_NV12 {Gen7_5+}  2D      Cr  Y   Cb      float
================================= ======= === === === === =======================================


load16
^^^^^^

.. code-block:: c++

  void load16(matrix_ref<float, N, 16> m, ChannelMaskType channelMask,
    SurfaceIndex surfIndex, vector<uint, 16> u, vector<uint, 16> v = 0,
    vector<uint, 16> r = 0);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results, where N is at least the
                number of enabled channels.

channelMask
                enabled channels.  It must be a compile time constant.

surfIndex
                surface index. This is an abstract handle that represents the
                surface created by C for Metal host runtime [6] and must be passed
                through kernel function parameters. C for Metal does not allow the
                explicit use of local/global variable or modification of such
                abstract data types in kernel functions, except used as
                function call argument.

u
                the unnormalized x coordinates of the texels to be sampled.

v
                (optional, default = 0) the unnormalized y coordinates for
                non-1D surface types; ignored otherwise.

r
                (optional, default = 0) the unnormalized z coordinates for any
                3D surface types; ignored otherwise.
=============== ============================================================

The compiler generates a sampler "ld" message, which samples the surface using
the default sampler state.  The u, v, and r parameters represent integer texel
addresses.  The results, in normalized floating point values of range [0, 1],
are returned in m with each enabled channel returned in the next row of m.  The
enabled channels are returned in R, G, B, A order with no gap in m for disabled
channels.

Supported Surfaces:

================================= ======= === === === === =======================================
Format                            Type    R   G   B   A   Return Type
================================= ======= === === === === =======================================
CM_SURFACE_FORMAT_A8R8G8B8        2D/3D   R   G   B   A   float
CM_SURFACE_FORMAT_A8              2D/3D               A   float
CM_SURFACE_FORMAT_YUY2            2D      V   Y   U       float
CM_SURFACE_FORMAT_NV12 {Gen7_5+}  2D      Cr  Y   Cb      float
================================= ======= === === === === =======================================

cm_3d_sample
^^^^^^^^^^^^

.. code-block:: c++

  template <CM3DSampleOp Op, ChannelMaskType ChannelMask, typename T, int N,
            typename... Args>
  void cm_3d_sample(vector_ref<T, N> Dst, ushort AOffImmI, SamplerIndex Sampler,
                    SurfaceIndex Image, Args... Srcs);

=============== =========================================================================
Parameters
=============== =========================================================================
Op              Sample operation. Please refer to the table below to see what operations
                are available.

ChannelMask     Channel mask selection. See section 4.4 for the permitted values.

Srcs            Between 1 and 15 variadic arguments, each of which must be the same type
                and be a vector or matrix of type ``float`` or ``half``.

                ``half`` support is target-dependent and only available when
                the ``CM_HAS_3D_SAMPLE_HALF`` macro is defined.

                The number of elements in each vector or matrix determines the SIMD
                width of the operation.

                The precise number of arguments depends on the sample operation being
                performed, see the table below for details of what needs to be
                supplied.

Dst             A reference to the destination vector or matrix, where the results of
                the sample operation are stored. This is a vector of type ``half``,
                ``float``, ``ushort``, ``short``, ``int`` or ``uint``.

                For 32-bit return types (``float``, ``int`` or ``uint``):

                For each enabled channel in ``ChannelMask`` the values are returned
                starting from the ``R`` channel, dependent on the SIMD width. Disabled
                channels are skipped in the results, with only enabled channels being
                written.

                For 16-bit return types (``half``, ``short`` or ``ushort``):

                For each enabled channel in ``ChannelMask``, elements are written to
                the destination starting from the ``R`` channel. Disabled channels are
                skipped in the results, with only the enabled pixels being written.

                For ``LOD`` operations the ``R`` channel contains the clamped LOD
                values while the ``G`` channel contains the unclamped LOD values. The
                ``B`` and ``A`` channels have undefined values and should be masked out.

AOffImmI        A ushort value containing the Aofimmi modifier, where:
                  * Bit 0-3: The R offset
                  * Bit 4-7: The V offset
                  * Bit 8-11: The U offset

                These offsets must be in the range [-8,7]. Bit 12-15 are reserved
                and must be zero.

Sampler         The sampler index

Image           The surface index
=============== =========================================================================

This samples ``Image`` using the sampler state ``Sampler``. LOD, bias, ref,
and gradients are computed differently based on the sampler operation.

The type ``CM3DSampleOp`` defines what sample operations are supported and
how any variadic arguments supplied are interpreted according to each specific
operation. For any of the variadic arguments it is possible to give 0 to
indicate a default, all zeros argument, and any missing trailing variadic
arguments will be defaulted to 0 anyway.

The table below lists the operations available and the arguments expected,
in the order they are expected.

==================== =======================================================
3D Sample Operation  Variadic Arguments
==================== =======================================================
CM_3D_SAMPLE         u, v, r, ai
CM_3D_SAMPLE_B       bias, u, v, r, ai
CM_3D_SAMPLE_L       lod, u, v, r, ai
CM_3D_SAMPLE_C       ref, u, v, r, ai
CM_3D_SAMPLE_D       u, dudx, duxy, v, dvdx, dvdy, r, drdx, drdy, ai
CM_3D_SAMPLE_B_C     ref, bias, u, v, r, ai
CM_3D_SAMPLE_L_C     ref, lod, u, v, r, ai
CM_3D_LOD            u, v, r, ai
CM_3D_SAMPLE_D_C     ref, u, dudx, duxy, v, dvdx, dvdy, r, drdx, drdy, ai
CM_3D_SAMPLE_LZ      u, v, r, ai
CM_3D_SAMPLE_C_LZ    ref, u, v, r, ai
==================== =======================================================

Each of the values above can be composed with ``CM_3D_SAMPLE_NULLMASK_ENABLE``
using the | operation to enable a variant of the operation that also
returns pixel null mask information.

The following table lists surface types supported for each operation:

==================== =======================
3D Sample Operation  Surface Types
==================== =======================
CM_3D_SAMPLE         1D, 2D, 3D, CUBE
CM_3D_SAMPLE_B       1D, 2D, 3D, CUBE
CM_3D_SAMPLE_L       1D, 2D, 3D, CUBE
CM_3D_SAMPLE_C       1D, 2D, CUBE
CM_3D_SAMPLE_D       1D, 2D, 3D, CUBE
CM_3D_SAMPLE_B_C     1D, 2D, CUBE
CM_3D_SAMPLE_L_C     1D, 2D, 3D, CUBE
CM_3D_SAMPLE_D_C     1D, 2D, CUBE
CM_3D_SAMPLE_LZ      1D, 2D, 3D, CUBE
CM_3D_SAMPLE_C_LZ    1D, 2D, CUBE
==================== =======================

The following table lists how the common "u", "v", "r", and "ai" arguments are interpreted
against different surface types:

=============== ========================= ============================ ============================ =============================
Surface Type    u                         v                            r                            ai
=============== ========================= ============================ ============================ =============================
1D              Normalized x coordinate   Unnormalized array index     ignored                      ignored
2D              Normalized x coordinate   Normalized y coordinate      Unnormalized array index     ignored
3D              Normalized x coordinate   Normalized y coordinate      Normalized z coordinate      ignored
CUBE            Normalized x coordinate   Normalized y coordinate      Normalized z coordinate      Unnormalized array index
=============== ========================= ============================ ============================ =============================

cm_3d_load
^^^^^^^^^^

{Only for new cm-llvm compiler cmc on SKL+ - not supported for legacy cm-icl compiler - icl}

.. code-block:: c++

  template <CM3DLoadOp Op, ChannelMaskType ChannelMask, typename T, int N,
            typename... Args>
  void cm_3d_load(vector_ref<T, N> Dst, ushort AOffImmI, SurfaceIndex Image,
                  Args... Srcs);

=============== =========================================================================
Parameters
=============== =========================================================================
Op              Load operation. Please refer to the table below to see what operations
                are available.

ChannelMask     Channel mask selection. See section 4.4 for the permitted values.

Srcs            Between 1 and 15 variadic arguments, each of which must be the same type
                and be a vector or matrix of type uint or ushort The number of elements
                in this type determines the SIMD width for the operation.  The precise
                number of arguments depends on the load operation being performed, see
                the table below for details of what needs to be supplied.

Dst             A reference to the destination vector, where the results of the load
                are stored. This is a vector of type ``half``, ``float``, ``ushort``,
                ``short``, ``int`` or ``uint``.

                For 32-bit return types (``float``, ``int`` or ``uint``):

                For each enabled channel in ``ChannelMask`` the values are returned
                starting from the ``R`` channel, dependent on the SIMD width. Disabled
                channels are skipped in the results, with only enabled channels being
                written.

                For 16-bit return types (``half``, ``short`` or ``ushort``):

                For each enabled channel in ``ChannelMask``, elements are written to
                the destination starting from the ``R`` channel. Disabled channels are
                skipped in the results, with only the enabled pixels being written.

AOffImmI        A ushort value containing the Aofimmi modifier, where:
                  * Bit 0-3: The R offset
                  * Bit 4-7: The V offset
                  * Bit 8-11: The U offset

                These offsets must be in the range [-8,7]. Bit 12-15 are reserved
                and must be zero.

Image           The surface index
=============== =========================================================================

This loads data from ``Image`` at the given integer texel addresses.

The type ``CM3DLoadOp`` defines what load operations are supported and how
any variadic arguments supplied are interpreted according to each specific
operation. For any of the variadic arguments it is possible to give 0 to
indicate a default, all zeros argument, and any missing trailing variadic
arguments will defaulted to 0 anyway.

The table below lists the operations available and the arguments expected,
in the order they are expected.

.. table:: Arguments for 3d load opcodes
   :width: 100%

   ============================================ =======================================================
   3D Load Operation                            Variadic Arguments
   ============================================ =======================================================
   CM_3D_LOAD                                   u, v, lod, r
   CM_3D_LOAD_LZ                                u, v, r
   CM_3D_LOAD_L {Xe2+}                          u, v, r, lod
   CM_3D_LOAD_2DMS_W (type ``uint``)            si, mcsl, mcsh, u, v, r, lod
   CM_3D_LOAD_2DMS_W (type ``ushort``) {ICL+}   si, mcs0, mcs1, mcs2, mcs3, u, v, r, lod
   CM_3D_LOAD_MCS                               u, v, r, lod
   ============================================ =======================================================

Each of the values above can be composed with ``CM_3D_LOAD_NULLMASK_ENABLE``
using the | operation to enable a variant of the operation that also
returns pixel null mask information.

The following table lists surface types supported for each operation:

.. table:: Allowed surface types for 3d load opcodes
   :width: 50%

   ==================== =======================
   3D Load Operation    Surface Types
   ==================== =======================
   CM_3D_LOAD           1D, 2D, 3D, BUFFER
   CM_3D_LOAD_LZ        1D, 2D, 3D, BUFFER
   CM_3D_LOAD_L         1D, 2D, 3D, BUFFER
   CM_3D_LOAD_2DMS_W    2D
   CM_3D_LOAD_MCS       2D
   ==================== =======================

The following table lists how the common "u", "v", "r", and "ai" arguments are interpreted
against different surface types:

=============== ========================= ============================ ============================
Surface Type    u                         v                            r
=============== ========================= ============================ ============================
1D              Unnormalized x coordinate Unnormalized array index     ignored
2D              Unnormalized x coordinate Unnormalized y coordinate    Unnormalized array index
3D              Unnormalized x coordinate Unnormalized y coordinate    Unnormalized z coordinate
=============== ========================= ============================ ============================

4.9 Adaptive Video Scaling
--------------------------

cm_avs_sampler
^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_avs_sampler(matrix_ref m, ChannelMaskType channelMask,
    SurfaceIndex surfIndex, SamplerIndex sampIndex, float u, float v,
    float deltaU, float deltaV, float u2d, int GroupID =-1,
    short VertBlockNumber = -1, OutputFormatControl cntrl=0,
    float v2d=0, AVSExecMode execMode=0, bool IEFBypass=0);

=============== ============================================================
Parameters
=============== ============================================================
m
                matrix to store the return results. See below for details.

channelMask
                enabled channels.  It must be a compile time constant.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has a format  with <= 10 bits  per channel.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

deltaU
                the difference in coordinates for adjacent pixels in the X
                direction.

deltaV
                the difference in coordinates for adjacent pixels in the Y
                direction.

u2d
                Defines the change in the delta U for adjacent pixels in the
                X direction

GroupID
                This field is valid and must be set for Gen7+, for all
                previous platforms this field is ignored.  This parameter
                will be used to group messages for reorder for sample_8x8
                messages.  For all messages with the same Group ID they must
                have the following in common: Surface state, Sampler State,
                GRFID, M0, and M1 except for Block number.

VertBlockNumber
                This field is valid and must be set for Gen7+, for all
                previous platforms this field is ignored. This field will
                specify the vertical block offset being sent for this
                sample_8x8 messages.  This will be equal to the vertical
                pixel offset from the given address divided by 4.

                e.g.  A 16x16 macro-block can be processed with 4 16x4
                blocks.  They can share one Group ID, Pix0U, and Pix0V for
                the group.  Top block has VBN=0, the next below has VBN=1,
                and so on.  Here Pix0U, and Pix0V are address for the pixel
                at the top left of the group, not the block origin.

cntrl
                An enumeration constant that specifies the output format for
                each pixel.

                * CM_16_FULL: two bytes will be returned for each pixel
                  channel, and TYPE of m must be short or unsigned short.
                * CM_16_DOWN_SAMPLE {Gen7.5+}: like previous, except that
                  only the even pixels in R and B channels are returned.
                * CM_8_FULL {Gen7.5+}: one byte will be returned for each
                  pixel channel, and TYPE of m must be char or unsigned
                  char.
                * CM_8_DOWN_SAMPLE {Gen7.5+}: like previous, except that
                  only the even pixels in R and B channels are returned.

v2d
                Defines the change in the delta V for adjacent pixels in the
                Y direction. This parameter is for Gen8+ only, ignored for
                previous architectures.

execMode
                an enumeration constant that determines the number of pixels
                to be returned.

                * CM_AVS_16x4
                * CM_AVS_16x8 {Gen8+}
                * CM_AVS_8x4 {Gen9+} output shuffle must be set
                * CM_AVS_4x4 {Gen9+} output shuffle must be set

IEFBypass
                Gen8 only, this field enables EIF pass. Default is 0.
=============== ============================================================

The results are returned
in matrix m with each enabled channel stored in the next row.  The
actual data returned is determined by a combination of
channelMask, cntrl, execMode, as well as whether output
shuffle is enabled in the sampler state.

* If output shuffle is off:

  ========= ========= ========= =========
  R[0:N]    G[0:N]    B[0:N]    A[0:N]
  R[64:127] G[64:127] B[64:127] A[64:127]
  ========= ========= ========= =========

  Where R[0] corresponds to the first element in m.  N is
  equal to the number of pixels specified by the execMode
  except for CM_AVS_16x8, for which N is 63.  For
  CM_AVS_16x8 mode, an additional 64 pixels will be
  delivered.  The disabled channels will be skipped with no
  gap in m.  The cntrl field determines the pixel channel
  size as well as whether the odd pixels will be skipped for
  the R and B channels.

* If output shuffle is on {Gen9+}:

  The format of CM_AVS_16x4 mode becomes

  ============================ ============================ ============================ ============================
  R[0:7][16:23][32:39][48:55]  G[0:7][16:23][32:39][48:55]  B[0:7][16:23][32:39][48:55]  A[0:7][16:23][32:39][48:55]
  R[8:15][24:31][40:47][56:63] G[8:15][24:31][40:47][56:63] B[8:15][24:31][40:47][56:63] A[8:15][24:31][40:47][56:63]
  ============================ ============================ ============================ ============================

  The disabled channels will be skipped with no gap in m.  The
  cntrl field determines the pixel channel size as well as
  whether the odd pixels will be skipped for the R and B
  channels.

  The format of CM_AVS_8x4 and CM_AVS_4x4 is the same as
  before, while CM_AVS_16x8 is not supported.

  For CM_8_DOWN_SAMPLE control mode, if either R or B channel
  (but not both) are disabled, the two channels will not be
  skipped in m, but the disabled channel is not written to m.
  If both R and B channels are disabled they will still be
  skipped.


Supported Surfaces:

================================= ======= === === === === =======================================
Format                            Type    R   G   B   A   Notes
================================= ======= === === === === =======================================
CM_SURFACE_FORMAT_UYVY            2D      Cr  Y   Cb      Swappy
CM_SURFACE_FORMAT_YUY2            2D      Cr  Y   Cb      Normal
CM_SURFACE_FORMAT_A8              2D      R
CM_SURFACE_FORMAT_NV12            2D      Cr  Y   Cb
================================= ======= === === === === =======================================


4.10 Video Analytics Functions (Gen8+)
-------------------------------------

4.10.1 2d Convolve
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_2d_convolve(matrix_ref<short, 4, 16> m, SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, CONVExecMode execMode=0,
    bool big_kernel=false);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Output format is
                16bit SINT.

                * 16x4 mode: matrix<short, 4,16>. Data is laid out in pixel
                  sequential order.
                * 16x1 mode: matrix<short, 1,16>. Data is laid out in pixel
                  sequential order.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

execMode
                Two modes are supported: 16x4, 16x1. The return matrix
                varies depending on mode set.  The default mode is 16x4.

                * CM_CONV_16x4
                * CM_CONV_16x1

big_kernel
                Gen9+ functionality. For Gen8 this value is ignored. Set to
                true when size of the kernel is larger then  15x15.
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 8-bit or 16-bit format
* Mode supported 16x4, 16x1

Supported Surfaces

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

4.10.2 Min Max Filter
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_min_max_filter(matrix_ref<TYPE, N, M> m, SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, OutputFormatControl cntrl,
    MMFExecMode execMode, MMFEnableMode mmfMode);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Number of pixels per
                row depends on the Surface format.  It is up to the user to
                make sure matrix of the correct size is passed in, based on
                surface input format. A Min or Max can be disabled,
                corresponding values are invalid.

                * 16 bit surface 16x4 mode: matrix<short, 4,32>. Data is
                  laid out in pixel sequential order, with min max for each
                  pixel.
                * 8-bit surface 16x4 mode: matrix<uchar,4,32> Data is laid
                  out in pixel sequential order, with min max interleaved
                  for each pixel: pixel0 min, pixel1 min, pixel0 max, pixel1
                  max.
                * 16-bit surface 16x1 mode: matrix<short, 1,32>. Data is
                  laid out in pixel sequential order, with min max for each
                  pixel.
                * 8-bit surface 16x1 mode: matrix<uchar,1,32> Data is laid
                  out in pixel sequential order, with min max interleaved
                  for each pixel: pixel0 min, pixel1 min, pixel0 max, pixel1
                  max.
                * 16-bit surface 1x1 mode: matrix<short, 1,2>. Data is laid
                  out in pixel sequential order, with min max for each
                  pixel.
                * 8-bit surface 1x1 mode: matrix<uchar,1,4> Data is laid out
                  in pixel sequential order, with min max interleaved for
                  each pixel: pixel0 min, pixel1 min, pixel0 max, pixel1
                  max.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16 bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

cntrl
                Specifies output format, the size of the return matrix will
                vary depending on mode. Modes are:

                * CM_16_FULL
                * CM_8_FULL

execMode
                Three modes are supported: 16x4, 16x1, and 1x1. The return
                matrix varies depending on mode set. The default mode is
                16x4.

                * CM_MMF_16x4
                * CM_MMF_16x1
                * CM_MMF_1x1

mmfMode
                Used to enable MinMax functionality.

                * CM_MINMAX_ENABLE
                * CM_MAX_ENABLE
                * CM_MIN_ENABLE
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 8-bit or 16 bit.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================


4.10.3 Min Max
^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_min_max(vector_ref<TYPE, N> vect, SurfaceIndex surfIndex,
    float u, float v, MMFEnableMode mmfMode)

=============== ============================================================
Parameters
=============== ============================================================
vect
                the vector to store the return results. Pixel 0 Max and Min
                are stored in vect[1], vect[0] respectively. The type,
                uchar, or ushort, of matrix needs to correspond to the input
                format: 8- bit, 16-bit.

                * vector_ref<uchar, 32> - for 8-bit input format
                * vector_ref<ushort, 16> - for 16-bit input format

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16-bit format.  This is an abstract
                handle that represents the surface created by C for Metal host
                runtime [6] and must be passed through kernel function
                parameters. C for Metal does not allow the explicit use of
                local/global variable or modification of such abstract data
                types in kernel functions, except used as function call
                argument.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

mmfMode
                Used to enable MinMax functionality. By default both are
                enabled. Modes are:

                * CM_MINMAX_ENABLE
                * CM_MAX_ENABLE
                * CM_MIN_ENABLE
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following
constraints:

* Surface Type must be 2D

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================


4.10.4 Erode
^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_erode(vector_ref<uint, N> vect, SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, EDExecMode execMode);

=============== ============================================================
Parameters
=============== ============================================================
vect
                the vector to store the return results.

                * In 64x4 mode each value in the vector contains 32 values,
                  N is 8.
                * In 64x1 mode each value in the vector contains 32 values,
                  N is 2.
                * In 32x4 mode each even value [0,6] in the vector contains
                  32 values, N is 8.
                * In 32x1 mode each value in the vector contains 32 values,
                  N is 1.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has 32-bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

execMode
                There are four modes: 16x4, 16x1, 32x4, and 32x1. It is up
                to the user to make sure the return matrix has appropriate
                number of pixels and rows. The default mode is: 16x4.

                * CM_ED_64x4
                * CM_ED_32x4
                * CM_ED_64x1
                * CM_ED_32x1
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format is 1 bit per pixel.

Supported Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_A8R8G8B8  2D    Planar 1 bit per pixel format
CM_SURFACE_FORMAT_X8R8G8B8  2D    Planar 1 bit per pixel format
=========================== ===== ============================

4.10.5 Dilate
^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_dilate(vector_ref<uint, N> vect, SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, EDExecMode execMode);

=============== ============================================================
Parameters
=============== ============================================================
vect
                the vector to store the return results.

                * In 64x4 mode each value in the vector contains 32 values,
                  N is 8.
                * In 64x1 mode each value in the vector contains 32 values,
                  N is 2.
                * In 32x4 mode each even value [0,6] in the vector contains
                  32 values, N is 8.
                * In 32x1 mode each value in the vector contains 32 values,
                  N is 1.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has 32-bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

execMode
                There are four modes: 16x4, 16x1, 32x4, and 32x1. It is up
                to the user to make sure the return matrix has appropriate
                number of pixels and rows. The default mode is: 16x4.

                * CM_ED_64x4
                * CM_ED_32x4
                * CM_ED_64x1
                * CM_ED_32x1
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format is 1 bit per pixel.

Supported Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_A8R8G8B8  2D    Planar 1 bit per pixel format
CM_SURFACE_FORMAT_X8R8G8B8  2D    Planar 1 bit per pixel format
=========================== ===== ============================

4.10.6 Centroid
^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_centroid(matrix_ref<int, 4, 8> m, SurfaceIndex surfIndex, float u, float v, uchar vSize)

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Values are returned
                in order [jSum, Division/Sum, ..] starting with  column 0,
                to column 15 inclusive.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16-bit format.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

vSize
                To control the size of the centroid in vertical direction if
                less than 8 is required. Valid range is [0-7]. Default value
                of 0 means size is 8.
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 8-bit

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================


4.10.7 Boolean Centroid
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_boolean_centroid(matrix_ref<uint, 2, 8> m, SurfaceIndex surfIndex, float u, float v,
    uchar vSize, uchar hSize)

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Values are returned
                are iSum, jSum, Sum, column 0 is in row0, etc. The return
                value of Sum from the Sampler8x8 is a signed value.

surfIndex
                surface index, which must correspond to a 2D type surface.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

hSize
                To control the size of the BoolCentroid in horizontal
                direction if less than 64 is required.

vSize
                To control the size of the BoolCentroid in vertical
                direction if less than 4 is required. Valid range is [0-3].
                Default value of 0 means size is 4.
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 1-bit. On the host side and driver side C for Metal uses D3D formats.

Supported Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_A8R8G8B8  2D    Planar 1 bit per pixel format
CM_SURFACE_FORMAT_X8R8G8B8  2D    Planar 1 bit per pixel format
=========================== ===== ============================


4.11 Video Analytics Functions (Gen9+)
--------------------------------------

4.11.1 1d Convolve
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_1d_convolution(matrix_ref<short, N, 16> m,
    SurfaceIndex surfIndex, SamplerIndex sampIndex, bool isHdirection,
    float u, float v, CONVExecMode execMode=0);

=============== ============================================================
Parameters
=============== ============================================================
m
                the vector  to store the return results. Output format is
                16bit SINT.

                * 16x4 mode: matrix<short, 4,16>. Data is layed out in pixel
                  sequential order.
                * 16x1 mode: matrix<short, 1,16>. Data is layed out in pixel
                  sequential order.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

isHdirection
                * TRUE : horizontal 1DConvolve
                * FALSE: vertical 1dConvolve

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

execMode
                Two modes are supported: 16x4, 16x1. The return matrix
                varies depending on mode set. The default mode is 16x4.

                * CM_CONV_16x4
                * CM_CONV_16x1
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 8-bit or 16-bit format
* Mode supported 16x4, 16x1

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

4.11.2 1Pixel Convolve
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_1pixel_convolve(matrix_ref<short, 1, 1> m, SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, CONVExecMode execMode,
    matrix<short, 1, 32> offsets);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Output format is
                16bit SINT.

                * 16x1 mode: matrix_ref<short, 1,16>. Data is layed out in pixel
                  sequential order.
                * 1x1 mode: matrix_ref<short, 1,1>

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

execMode
                Three modes are supported: 16x1 and 1x1. The return matrix
                varies depending on mode set.

                * CM_CONV_16x1
                * CM_CONV_1x1

Offsets
                offsets of pixels from U,V address. A row contains 15 pairs
                of x,y. For 1x1 mode offset is not applicable and should be
                omitted.
=============== ============================================================

In addition, the sampler state setup in the host must satisfy the following constraints:

* Surface Type must be 2D
* Surface Format must be 8-bit or 16-bit format
* Mode supported 16x1, 1x1

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Unsigned planar
======================= ===== ============================

4.11.3 LBP Creation
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_lbp_creation(matrix_ref<uchar, N, 16> m, SurfaceIndex surfIndex,
    float u, float v, LBPCreationExecMode execMode);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Output format is
                16bit SINT.

                * 5x5 mode: matrix_ref<uchar, 4,16>. Data is laid out in pixel
                  sequential order.
                * 3x3  mode: matrix_ref<uchar, 4,16>. Data is laid out in pixel
                  sequential order.
                * 3x3 and 5x5 modes: matrix_ref<uchar, 8, 16> First 4 rows will
                  be results of 3x3 operation, the next 4 rows will be
                  results of 5x5 operation.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

execMode
                Three modes are supported: 5x5, 3x3, and BOTH. The return
                matrix varies depending on mode set. The default mode is
                16x4.

                * CM_LBP_CREATION_5x5
                * CM_LBP_CREATION_3x3
                * CM_LBP_CREATION_BOTH
=============== ============================================================

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Unsigned planar
======================= ===== ============================


4.11.4 LBP Correlation
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_lbp_correlation(matrix_ref<uchar, N, 16> m,
    SurfaceIndex surfIndex, float u, float v, short xoff_disparity);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Output format is
                16bit SINT.

                * matrix_ref<uchar, 4,16>. Data is layed out in pixel sequential
                  order.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

xoff_disparity
                16bit signed x-offset for right image with respect to left
                image
=============== ============================================================

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_A8    2D    Unsigned planar
======================= ===== ============================

4.11.5 Flood Fill
^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_flood_fill(vector_ref<ushort, 8> v, bool is8Connect,
    vector<ushort, 10> pxMaskHDir, ushort pxMaskVDirLeft,
    ushort pxMaskVDirRight, uchar loopCount);

=============== ============================================================
Parameters
=============== ============================================================
v
                the vector to store the return results. Output format is
                1bpp.

                * 16x8 mode: vector_ref<ushort, 8>. Data is laid out in pixel
                  sequential order.

Is8Connect
                * TRUE: 8 connect is used for floodfill
                * FALSE: 4 connect is used for floodfill

pxMaskHDir
                Pixel Mask of the bottom adjacent pixels in horizontal
                direction. Index [0,7] are for rows [0,7], Index 8 is for
                row: -1, Index 9 is for row 8.

pxMaskVDirLeft
                Pixel mask of the adjacent pixels in vertical direction. The
                mask is 10 bits per entry. All MSB bits passed 10 bits must
                be 0.

pxMaskVDirRight
                Pixel mask of the adjacent pixels in vertical direction. The
                mask is 10 bits per entry. All MSB bits passed 10 bits must
                be 0.

loopCount
                Number of times FloodFill will be repeated on HW. Valid
                range is [1,16]
=============== ============================================================


4.11.6 Correlation Search
^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_correlation_search(matrix_ref<T, N, M> m, SurfaceIndex surfIndex,
    float u, float v, float vertOrigin, float horizOrigin,
    uchar xDirectionSize, uchar yDirectionSize, uchar xDirSearchSize,
    uchar yDirSearchSize);

=============== ============================================================
Parameters
=============== ============================================================
m
                the matrix to store the return results. Output format is
                32bit SINT.

                * width less then 8: matrix_ref<int, N,8>. Data is layed out in
                  sequential order. Rows depend on height of search region.
                  Only valid rows are returned.
                * width greater then 8: matrix_ref<int, N,16>. Data is layed out
                  in sequential order. Rows depend on height of search
                  region. Only valid rows are returned.

surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

vertOrigin
                Normalized vertical origin of the reference image.

horizOrigin
                Normalized horizontal origin of the reference image.

xDirectionSize
                X Direction size of the Source Correlation. Valid range
                [1,8]

yDirectionSize
                Y direction size of the Source Correlation. Valid range
                [1,8]

xDirSearchSize
                X direction Search Size. Valid range [3,16]

yDirSearchsize
                Y direction Search Size. Valid range [3,16]
=============== ============================================================

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Unsigned planar
======================= ===== ============================


4.12 Video Analytics Functions HDC Write {Gen9+}
------------------------------------------------

These variants of VA functions write their output to another surface directly instead of GRF.  The output
surface must have the same dimension (width*height in pixels) as the input surface.

4.13.1 2d Convolve
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_2d_convolve_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
    float u, float v, bool big_kernel, CM_FORMAT_SIZE size,
    SurfaceIndex destIndex, ushort x_offset, ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

big_kernel
                Set to true when size of the kernel is larger then  15x15.

size
                Specifies the format of the output surface. Two formats are
                supported:

                * CM_HDC_FORMAT_16S
                * CM_HDC_FORMAT_8U

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface
=============== ============================================================

16x4 pixels will be written to the output surface.  Each pixel is clamped to unsigned char for 8-bit
output surfaces.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R16_SINT  2D    Signed
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.2 Min Max Filter
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_min_max_filter_hdc(SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, MMFEnableMode mmfMode,
    CM_FORMAT_SIZE size, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16 bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

mmfMode
                Used to enable MinMax functionality.

                * CM_MAX_ENABLE
                * CM_MIN_ENABLE

size
                Specifies the format used for the output. Two formats are
                supported:

                * CM_HDC_FORMAT_16S
                * CM_HDC_FORMAT_8U

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface.
=============== ============================================================

16x4 pixels will be written to the output surface.  Each pixel is clamped to unsigned char for 8-bit
output surfaces.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R16_SINT  2D    Signed
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.3 Erode
^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_erode_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
    float u, float v, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has 32-bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface
=============== ============================================================

64x4 pixels with one-bit per pixel will be written to the output surface.

Supported Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_A8R8G8B8  2D    Planar 1 bit per pixel format
CM_SURFACE_FORMAT_X8R8G8B8  2D    Planar 1 bit per pixel format
=========================== ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.4 Dilate
^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_dilate_hdc(SurfaceIndex surfIndex, SamplerIndex sampIndex,
    float u, float v, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has 32-bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface
=============== ============================================================

64x4 pixels with one-bit per pixel will be written to the output surface.

Supported Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_A8R8G8B8  2D    Planar 1 bit per pixel format
CM_SURFACE_FORMAT_X8R8G8B8  2D    Planar 1 bit per pixel format
=========================== ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.5 1d Convolve
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_1d_convolution_hdc(SurfaceIndex surfIndex,
    SamplerIndex sampIndex, bool isHdirection, float u, float v,
    CM_FORMAT_SIZE size, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

isHdirection
                * TRUE : horizontal 1DConvolve
                * FALSE: vertical 1dConvolve

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

size
                Specifies the format used for the output. Two formats are
                supported:

                  * CM_HDC_FORMAT_16S
                  * CM_HDC_FORMAT_8U

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface.
=============== ============================================================

For vertical convolve, 16x4 pixels will be written to the output surface.  For horizontal convolve, 4x16
pixels will be written to the output surface.  Each pixel is clamped to unsigned char for 8-bit output
surfaces.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R16_SINT  2D    Signed
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.6 1Pixel Convolve
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_1pixel_convolve_hdc(SurfaceIndex surfIndex,
    SamplerIndex sampIndex, float u, float v, matrix<short, 1, 32> offsets,
    CM_FORMAT_SIZE size, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset );

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

sampIndex
                the index into the sampler state table.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

offsets
                offsets of pixels from U,V address.

size
                Specifies the format used for the output. Two formats are
                supported:

                * CM_HDC_FORMAT_16S
                * CM_HDC_FORMAT_8U

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface
=============== ============================================================

16x1 pixels will be written to the output surface.  Each pixel is clamped to unsigned char for 8-bit output
surfaces.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R16_SINT  2D    Signed
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.7 LBP Creation
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_lbp_creation_hdc(SurfaceIndex surfIndex, float u, float v,
    LBPCreationExecMode execMode, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

u
                the normalized x coordinate of pixel 0.

v
                the normalized y coordinate of pixel 0.

execMode
                valid values are:

                * CM_LBP_CREATION_5x5
                * CM_LBP_CREATION_3x3

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface.
=============== ============================================================

16x4 pixels will be written to the output surface.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_V8U8  2D    Signed planar
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R16_SINT  2D    Signed
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13.8 LBP Correlation
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_va_lbp_correlation_hdc(SurfaceIndex surfIndex, float u, float v,
    short xoff_disparity, SurfaceIndex destIndex, ushort x_offset,
    ushort y_offset);

=============== ============================================================
Parameters
=============== ============================================================
surfIndex
                surface index, which must correspond to a 2D type surface
                that has either 8-bit or 16bit format.

u
                the normalized x coordinat of pixel 0.

v
                the normalized y coordinat of pixel 0.

xoff_disparity
                16bit signed x-offset for right image with respect to left
                image

destIndex
                surface index of the 2D surface where data will be written.

x_offset
                A byte offset in to the output surface.

y_offset
                A row offset in to the output surface
=============== ============================================================

16x4 pixels will be written to the output surface.

Supported Surfaces:

======================= ===== ============================
Format                  Type  Notes
======================= ===== ============================
CM_SURFACE_FORMAT_A8    2D    Planar
======================= ===== ============================

Supported Output Surfaces:

=========================== ===== ============================
Format                      Type  Notes
=========================== ===== ============================
CM_SURFACE_FORMAT_R8_UINT   2D    Unsigned
=========================== ===== ============================


4.13 VME Interface
------------------

4.13.1 Gen6 VME Interface
^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the user with the following Video Motion Estimation (VME) function support for Gen6
architecture. More detailed information on the VME functionality is provided in reference [4].

The formal parameters for all three functions below are described here:

=============== ============================================================
Parameters
=============== ============================================================
mInput
                the matrix that stores the VME payload data.  For detailed
                description of the payload content, please refer to [4]. The
                input matrix type "vme_InputMrfType" is defined as
                matrix<unsigned char, 4, 32>.

surfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

SPIndex
                search path LUT index. This is an abstract handle that
                represents the VME state created by C for Metal host runtime [6] and
                must be passed through kernel function parameters. C for Metal does
                not allow the explicit use of local/global variable or
                modification of such abstract data types in kernel
                functions, except used as function call argument.

lutSubIndex
                the index into the RDLUT state table, with a range of [0-3].

srcMB
                The position of the left-top integer corner of the source
                macroblock or block located in the surface (in unit of
                pixels, relative to the surface origin).

ref0
                The position of the left-top integer corner of the first
                reference window located in the first reference surface (in
                unit of pixels, relative to the surface origin).

ref1
                The position of the left-top integer corner of the second
                reference window located in the second reference surface (in
                unit of pixels, relative to the surface origin; ignored in
                single reference mode).

topMinus8Pels
                The positions of the top neighborhood pixels. Please refer
                to reference [4] for more detailed description of the data
                members.

leftPels
                The positions of the left neighborhood pixels. Please refer
                to [4] for more detailed description of the data members.

mOutput
                the matrix that stores the VME output data.  For detailed
                description of the output content, refer to [4]. For
                "run_vme_intra", the output matrix type
                vme_OutputGrfShortType" is defined as
                matrix<unsigned char, 1, 32>. For "run_vme_inter" and
                "run_vme_all", the output matrix type vme_OutputGrfType" is
                defined as matrix<unsigned char, 4, 32>.
=============== ============================================================

run_vme_intra
"""""""""""""

.. code-block:: c++

  void run_vme_intra(vme_InputMrfType mInput, SurfaceIndex surfIndex, VmeIndex SPIndex,
    uint lutSubIndex, vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfShortType mOutput);

The compiler generates code for GenX hardware to perform VME function in intra-search only mode.

run_vme_inter
"""""""""""""

.. code-block:: c++

  void run_vme_inter(vme_InputMrfType mInput, SurfaceIndex surfIndex, VmeIndex SPIndex,
    uint lutSubIndex, vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType mOutput);

The compiler generates code for GenX hardware to perform VME function in inter-search only mode.

run_vme_all
"""""""""""

.. code-block:: c++

  void run_vme_all(vme_InputMrfType mInput, SurfaceIndex surfIndex, VmeIndex SPIndex,
    uint lutSubIndex, vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, vme_OutputGrfType mOutput);

The compiler generates code for GenX hardware to perform VME functions in both inter- and intra-search
enabled mode.

4.13.2 Gen7 VME Interface
^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the user with the following Video Motion Estimation (VME) function support for Gen7
architecture.

run_vme_intra
"""""""""""""

.. code-block:: c++

  void run_vme_intra(matrix<uchar, 5, 32> mInput, VMEStreamMode streamMode,
    VMESearchCtrl searchCtrl, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
    vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, matrix_ref<uchar, 1, 32> mOutput);

The compiler generates code for GenX hardware to perform VME function in intra-search only mode.

=============== ============================================================
Parameters
=============== ============================================================
mInput
                the matrix that stores the VME payload data.

streamMode
                VME stream mode, which must be set to VME_STREAM_DISABLE for
                run_vme_intra function.

searchCtrl
                VME search control, which must be set to
                VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START for
                run_vme_intra function.

surfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

SPIndex
                search path LUT index. This is an abstract handle that
                represents the VME state created by C for Metal host runtime [6] and
                must be passed through kernel function parameters. C for Metal does
                not allow the explicit use of local/global variable or
                modification of such abstract data types in kernel
                functions, except used as function call argument.

lutSubIndex
                the index into the RDLUT state table, with a range of [0-3].

srcMB
                The position of the left-top integer corner of the source
                macroblock or block located in the surface (in unit of
                pixels, relative to the picture origin).

ref0
                The position of the left-top integer corner of the first
                reference window located in the first reference surface (in
                unit of pixels, relative to source MB location).

ref1
                The position of the left-top integer corner of the second
                reference window located in the second reference surface (in
                unit of pixels, relative to source MB location; ignored in
                single reference mode).

topMinus8Pels
                The positions of the top neighborhood pixels.

leftPels
                The positions of the left neighborhood pixels.

mOutput
                the matrix that stores the VME output data.
=============== ============================================================

run_vme_inter
"""""""""""""

.. code-block:: c++

  void run_vme_inter(matrix<uchar, N1, 32> mInput, VMEStreamMode streamMode,
    VMESearchCtrl searchCtrl, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
    vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, matrix_ref<uchar, N2, 32> mOutput);

The compiler generates code for GenX hardware to perform VME function in inter-search only mode.

run_vme_all
"""""""""""

.. code-block:: c++

  void run_vme_all(matrix<uchar, N1, 32> mInput, VMEStreamMode streamMode,
    VMESearchCtrl searchCtrl, SurfaceIndex surfIndex, VmeIndex SPIndex, uint lutSubIndex,
    vector<ushort, 2> srcMB, vector<ushort, 2> ref0, vector<ushort, 2> ref1,
    vector<uchar, 32> topMinus8Pels, vector<uchar, 16> leftPels, matrix_ref<uchar, N2, 32> mOutput);

The compiler generates code for GenX hardware to perform VME functions in both inter- and intra-search
enabled mode.

=============== ============================================================
Parameters
=============== ============================================================
mInput
                the matrix that stores the VME payload data, where N1 can be
                the following values:

                * N1 = 5 if stream-in is disabled.
                * N1 = 9 if stream-in is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N1 = 7 if stream-in is enabled and search ctrl is set to
                  other type.

streamMode
                VME stream mode, which is an enumeration type with 4
                possible values, as listed below.  It must be a compile time
                constant.

                * VME_STREAM_DISABLE
                * VME_STREAM_OUT
                * VME_STREAM_IN
                * VME_STREAM_IN_OUT

searchCtrl
                VME search control, which is an enumeration type with 4
                possible values, as listed below. It must be a compile time
                constant.

                * VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START
                * VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START
                * VME_SEARCH_SINGLE_REF_DUAL_REC
                * VME_SEARCH_DUAL_REF_DUAL_REC

surfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

SPIndex
                search path LUT index. This is an abstract handle that
                represents the VME state created by C for Metal host runtime [6] and
                must be passed through kernel function parameters. C for Metal does
                not allow the explicit use of local/global variable or
                modification of such abstract data types in kernel
                functions, except used as function call argument.

lutSubIndex
                the index into the RDLUT state table, with a range of [0-3].

srcMB
                The position of the left-top integer corner of the source
                macroblock or block located in the surface (in unit of
                pixels, relative to the picture origin).

ref0
                The position of the left-top integer corner of the first
                reference window located in the first reference surface (in
                unit of pixels, relative to source MB location).

ref1
                The position of the left-top integer corner of the second
                reference window located in the second reference surface (in
                unit of pixels, relative to source MB location; ignored in
                single reference mode).

topMinus8Pels
                The positions of the top neighborhood pixels.

leftPels
                The positions of the left neighborhood pixels.

mOutput
                the matrix that stores the VME output data, where N2 can be
                the following values:

                * N2 = 6 if stream-out is disabled.
                * N2 = 10 if stream-out is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N2 = 8 if stream-out is enabled and search ctrl is set to
                  other type.
=============== ============================================================


4.13.3 Gen7_5 VME Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the following Video Motion Estimation (VME) APIs for Gen7_5 architecture.

run_vme_ime
"""""""""""

.. code-block:: c++

  void run_vme_ime(matrix<uchar, 3, 32> UNIInput, matrix<uchar, N1, 32> IMEInput,
    VMEStreamMode streamMode, VMESearchCtrl searchCtrl, SurfaceIndex curSurfIndex,
    vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 4> costCenter,
    matrix_ref<uchar, N2, 32> IMEOutput);

The compiler generates code for GenX hardware to perform Integer Motion Estimation (IME). The formal
parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

IMEInput
                the matrix that stores the IME specific payload data, where
                N1 can be the following values:

                * N1 = 2 if stream-in is disabled.
                * N1 = 6 if stream-in is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N1 = 4 if stream-in is enabled and search ctrl is set to
                  other type.

streamMode
                VME stream mode, which is an enumeration type with 4
                possible values, as listed below.  It must be a compile time
                constant.

                * VME_STREAM_DISABLE
                * VME_STREAM_OUT
                * VME_STREAM_IN
                * VME_STREAM_IN_OUT

searchCtrl
                VME search control, which is an enumeration type with 4
                possible values, as listed below. It must be a compile time
                constant.

                * VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START
                * VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START
                * VME_SEARCH_SINGLE_REF_DUAL_REC
                * VME_SEARCH_DUAL_REF_DUAL_REC

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

ref0
                The position of the left-top integer corner of the first
                reference window located in the first reference surface (in
                unit of pixels, relative to the source MB).

ref1
                The position of the left-top integer corner of the second
                reference window located in the second reference surface (in
                unit of pixels, relative to the source MB; ignored in single
                reference mode).

costCenter
                The coordinates for the cost centers relative to the picture
                source MB. The coordinates are specified in the following
                order: {CostCenter0X, CostCenter0Y, CostCenter1X,
                CostCenter1Y}.

IMEOutput
                the matrix that stores the IME output data, where N2 can be
                the following values:

                * N2 = 7 if stream-out is disabled.
                * N2 = 11 if stream-out is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N2 = 9 if stream-out is enabled and search ctrl is set to
                  other type.
=============== ============================================================

run_vme_sic
"""""""""""

.. code-block:: c++

  void run_vme_sic(matrix<uchar, 3, 32> UNIInput, matrix<uchar, 4, 32> SICInput,
    SurfaceIndex curSurfIndex, matrix_ref<uchar, 7, 32> UNIOutput);

The compiler generates code for GenX hardware to perform Skip and Intra Check (SIC). The formal
parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

SICInput
                the matrix that stores the SIC specific payload data.

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

UNIOutput
                the matrix that stores the SIC output data (same structure
                as universal VME output data).
=============== ============================================================

run_vme_fbr
"""""""""""

.. code-block:: c++

  void run_vme_fbr(matrix<uchar, 3, 32> UNIInput, matrix<uchar, 4, 32> FBRInput,
    SurfaceIndex curSurfIndex, uchar FBRMbMode, uchar FBRSubMbShape, uchar FBRSubPredMode,
    matrix_ref<uchar, 7, 32> UNIOutput);

The compiler generates code for GenX hardware to perform Fractional and Bidirectional Refinement (FBR).
The formal parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

FBRInput
                the matrix that stores the FBR specific payload data.

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

FBRMbMode
                the inter macroblock type, which can be the following 2-bit
                values:

                * 00: 16x16
                * 01: 16x8
                * 10: 8x16
                * 11: 8x8

FBRSubMbShape
                the subshape per block for fractional and bidirectional
                refinement, which can be the following combination of 8-bit
                values:

                * Bits [1:0]: SubMbShape[0]
                * Bits [3:2]: SubMbShape[1]
                * Bits [5:4]: SubMbShape[2]
                * Bits [7:6]: SubMbShape[3]

                where each 2-bit correspond to the following shapes:

                * 00: 8x8
                * 01: 8x4
                * 10: 4x8
                * 11: 4x4

FBRSubPredMode
                the selection of shapes from the input message for
                performing FME, which can be the following combination of
                8-bit values:

                * Bits [1:0]: SubMbPredMode[0]
                * Bits [3:2]: SubMbPredMode[1]
                * Bits [5:4]: SubMbPredMode[2]
                * Bits [7:6]: SubMbPredMode[3]

                where each 2-bit correspond to the following selections:

                * 00: Forward
                * 01: Backward
                * 10: Bidirectional
                * 11: Illegal

UNIOutput
                the matrix that stores the FBR output data (same structure
                as universal VME output data).
=============== ============================================================


4.13.4 Gen8 VME Interface
^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the following Video Motion Estimation (VME) APIs for Gen8 architecture.

run_vme_ime
"""""""""""

.. code-block:: c++

  void run_vme_ime(matrix<uchar, 4, 32> UNIInput, matrix<uchar, N1, 32> IMEInput,
    VMEStreamMode streamMode, VMESearchCtrl searchCtrl, SurfaceIndex curSurfIndex,
    vector<short, 2> ref0, vector<short, 2> ref1, vector<ushort, 16> costCenter,
    matrix_ref<uchar, N2, 32> IMEOutput);

The compiler generates code for GenX hardware to perform Integer Motion Estimation (IME). The formal
parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

IMEInput
                the matrix that stores the IME specific payload data, where
                N1 can be the following values:

                * N1 = 2 if stream-in is disabled.
                * N1 = 6 if stream-in is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N1 = 4 if stream-in is enabled and search ctrl is set to
                  other type.

streamMode
                VME stream mode, which is an enumeration type with 4
                possible values, as listed below.  It must be a compile time
                constant.

                * VME_STREAM_DISABLE
                * VME_STREAM_OUT
                * VME_STREAM_IN
                * VME_STREAM_IN_OUT

searchCtrl
                VME search control, which is an enumeration type with 4
                possible values, as listed below. It must be a compile time
                constant.

                * VME_SEARCH_SINGLE_REF_SINGLE_REC_SINGLE_START
                * VME_SEARCH_SINGLE_REF_SINGLE_REC_DUAL_START
                * VME_SEARCH_SINGLE_REF_DUAL_REC
                * VME_SEARCH_DUAL_REF_DUAL_REC

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

ref0
                The position of the left-top integer corner of the first
                reference window located in the first reference surface (in
                unit of pixels, relative to the source MB).

ref1
                The position of the left-top integer corner of the second
                reference window located in the second reference surface (in
                unit of pixels, relative to the source MB; ignored in single
                reference mode).

costCenter
                The coordinates for the cost centers relative to the picture
                source MB. The coordinates are specified in the following
                order: {FWDCostCenter0X, FWDCostCenter0Y, BWDCostCenter0X,
                BWDCostCenter0Y, FWDCostCenter1X, FWDCostCenter1Y,
                BWDCostCenter1X, BWDCostCenter1Y, FWDCostCenter2X,
                FWDCostCenter2Y, BWDCostCenter2X, BWDCostCenter2Y,
                FWDCostCenter3X, FWDCostCenter3Y, BWDCostCenter3X,
                BWDCostCenter3Y}

IMEOutput
                the matrix that stores the IME output data, where N2 can be
                the following values:

                * N2 = 7 if stream-out is disabled.
                * N2 = 11 if stream-out is enabled and search ctrl is set to
                  dual-record and dual-reference.
                * N2 = 9 if stream-out is enabled and search ctrl is set to
                  other type.
=============== ============================================================

run_vme_sic
"""""""""""

.. code-block:: c++

  void run_vme_sic(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> SICInput,
    SurfaceIndex curSurfIndex, matrix_ref<uchar, 7, 32> UNIOutput);

The compiler generates code for GenX hardware to perform Skip and Intra Check (SIC). The formal
parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

SICInput
                the matrix that stores the SIC specific payload data.

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

UNIOutput
                the matrix that stores the SIC output data (same structure
                as universal VME output data).
=============== ============================================================

run_vme_fbr
"""""""""""

.. code-block:: c++

  void run_vme_fbr(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 4, 32> FBRInput,
    SurfaceIndex curSurfIndex, uchar FBRMbMode, uchar FBRSubMbShape, uchar FBRSubPredMode,
    matrix_ref<uchar, 7, 32> UNIOutput);

The compiler generates code for GenX hardware to perform Fractional and Bidirectional Refinement (FBR).
The formal parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

FBRInput
                the matrix that stores the FBR specific payload data.

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

FBRMbMode
                the inter macroblock type, which can be the following 2-bit
                values:

                * 00: 16x16
                * 01: 16x8
                * 10: 8x16
                * 11: 8x8

FBRSubMbShape
                the subshape per block for fractional and bidirectional
                refinement, which can be the following combination of 8-bit
                values:

                * Bits [1:0]: SubMbShape[0]
                * Bits [3:2]: SubMbShape[1]
                * Bits [5:4]: SubMbShape[2]
                * Bits [7:6]: SubMbShape[3]

                where each 2-bit correspond to the following shapes:

                * 00: 8x8
                * 01: 8x4
                * 10: 4x8
                * 11: 4x4

FBRSubPredMode
                the selection of shapes from the input message for
                performing FME, which can be the following combination of
                8-bit values:

                * Bits [1:0]: SubMbPredMode[0]
                * Bits [3:2]: SubMbPredMode[1]
                * Bits [5:4]: SubMbPredMode[2]
                * Bits [7:6]: SubMbPredMode[3]

                where each 2-bit correspond to the following selections:

                * 00: Forward
                * 01: Backward
                * 10: Bidirectional
                * 11: Illegal

UNIOutput
                the matrix that stores the FBR output data (same structure
                as universal VME output data).
=============== ============================================================

run_vme_idm
"""""""""""

.. code-block:: c++

  void run_vme_idm(matrix<uchar, 4, 32> UNIInput, matrix<uchar, 1, 32> IDMInput,
    SurfaceIndex curSurfIndex, matrix_ref<uchar, 16, 32> IDMOutput);

The compiler generates code for GenX hardware to generate distortion mesh output (IDM). The formal
parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                the matrix that stores the universal VME payload data.

IDMInput
                the matrix that stores the IDM specific payload data.

curSurfIndex
                surface index. This is an abstract handle that represents
                the surface created by C for Metal host runtime [6] and must be
                passed through kernel function parameters. C for Metal does not allow
                the explicit use of local/global variable or modification of
                such abstract data types in kernel functions, except used as
                function call argument.

IDMOutput
                the matrix that stores the IDM output data.
=============== ============================================================


4.13.5 Gen10 HEVC VME Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C for Metal provides the following HEVC-specific Video Motion Estimation (VME) APIs for the Gen10 architecture

cm_vme_hevc_ime
"""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_ime (matrix<uchar, 5, 32> IMEInput, matrix<uchar, 10, 32> StreamInput,
    int LenStreamInput, SurfaceIndex curSurfIndex, matrix_ref<uchar, 12, 32> IMEOutput);

These generate code to perform an HEVC Integer Motion Estimation (IME) operation in GenX hardware.

=============== ============================================================
Parameters
=============== ============================================================
IMEInput
                this holds the universal VME and search path (IME) input
                payloads.

LenStreamInput
                the length of Stream Input, it could be either 0 (for an
                IME-F operation) or 10 (for an IME-S operation) .

StreamInput
                this holds the stream-in major shape motion-vector and
                distortion (SISO)  input payload.  Unused if
                LengthStreamInput is 0.

curSurfIndex
                the surface index.

IMEOutput
                this holds the output payload, comprising the universal VME
                return data and the stream out motion-vector and distortion
                data (SISO).
=============== ============================================================

cm_vme_hevc_sic, cm_vme_hevc_sc
"""""""""""""""""""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_sic(matrix_ref<uchar, 5, 32> SICInput, matrix_ref<uchar, 8, 32> NPInput,
    int LengthNP, SurfaceIndex curSurfIndex, matrix_ref<uchar, 22, 32> SICOutput);

  void cm_vme_hevc_sc(matrix_ref<uchar, 5, 32> SICInput, SurfaceIndex curSurfIndex,
    matrix_ref<uchar, 2, 32> SICOutput);

These generate code to perform an HEVC Skip and/or Intra Check (SIC, IC, SC) operation in GenX
hardware.

=============== ============================================================
Parameters
=============== ============================================================
SICInput
                this hold the universal VME and SIC input payloads.

NPInput
                this holds the neighbor pixel (NP) input payload. Not used
                if LengthNP is 0.

LengthNP
                the size of Neighbor Pixel payload, 8 for SIC or IC
                operations, 0 for SC operations.  curSurfIndex -  the
                surface index.

SICOutput
                this stores the SIC output payload, comprising the universal
                VME return data followed the intra steam-in steam-out data
                (SSRA) and Coding Unit (CU) data (these latter two data
                groups are only meaningful if an Intra Check was performed).
=============== ============================================================

cm_vme_hevc_sc() should be used in situations where it is always the case that there will be no need for
an Intra Check. This avoids having to create NPInput matrix just to satisfy the parameter requirements for
cm_vme_hevc_sic() even though it will be unused, and to avoid having SICOutput be unnecessarily large.

cm_vme_hevc_hpm_u
"""""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_hpm_u (matrix_ref<uchar, 12, 32> HPMInput, matrix_ref<uchar, 10, 32> StreamInpInter,
    SurfaceIndex curSurfIndex,  matrix_ref<uchar, 23, 32> HPMOutput);

These generate code to perform an HEVC Partitioning Message (HPM) with single-directional inter
prediction in GenX hardware.

=============== ============================================================
Parameters
=============== ============================================================
HPMInput
                this holds the universal VME, Skip Intra Check (SIC),
                IntraPred and distortion (SSRA) and Neighbor Motion Vector
                (NMV) input payloads.

StreamInpInter
                this holds the stream-in major shape motion vector and
                distortion data (SISO).

curSurfIndex
                the surface index.

HPMOutput
                the matrix that hold the output payload, comprising the
                universal VME return data and major shape stream out data
                (CU).
=============== ============================================================

cm_vme_hevc_hpm_b
"""""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_hpm_b (matrix<uchar, 12, 32> HPMInput, matrix<uchar, 20, 32> StreamInpInter,
    int LenStrmInpInter, SurfaceIndex curSurfIndex,  matrix_ref<uchar, 23, 32> HPMOutput);

These generate code to perform HEVC Partitioning Message (HPM) with single- or bi-directional Inter
Prediction in GenX hardware.

=============== ============================================================
Parameters
=============== ============================================================
HPMInput
                this holds the universal VME, Skip Intra Check (SIC),
                IntraPred and distortion (SSRA) and Neighbor Motion Vector
                (NMV) input payloads.

StreamInpInter
                this holds the stream-in major shape motion vector and
                distortion data (SISO).

LenStrmInpInter
                the size of StreamInputInter payload, 10 for
                single-directional, 20 for bi-directional.

curSurfIndex
                the surface index

HPMOutput
                this  holds the output payload, comprising the universal VME
                return data and major shape stream out data (CU).
=============== ============================================================

cm_vme_hevc_fbr
"""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_fbr(matrix<uchar, 3, 32> UNIInput, matrix<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> FBROutput);

  void cm_vme_hevc_fbr(matrix_ref<uchar, 3, 32> UNIInput, matrix_ref<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> FBROutput);

These generate code to perform a Fractional and Bidirectional Refinement (FBR) operation in GenX
hardware.

=============== ============================================================
Parameters
=============== ============================================================
UNIInput
                this holds the universal VME input payload.

CUInput
                this holds the Coding Unit (CU) input payload.

ValidCULength
                the size of CUInput. Legal value are: 1, 2, 4, 5, 6, 7, 8,
                9, 10, 11, 12, 13, 14, or 16.

curSurfIndex
                the surface index.

FBROutput
                this holds the FBR output payload, comprising the universal
                VME return data and the FBR applied Coding Unit data (CU).
=============== ============================================================

cm_vme_hevc_rpm
"""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_rpm(matrix<uchar, 7, 32> UNIInput, matrix<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 8, 32> RPMOutput);

  void cm_vme_hevc_rpm(matrix_ref<uchar, 7, 32> UNIInput, matrix_ref<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 8, 32> RPMOutput);

These generate code to perform a Residual Prediction Message (RPM) operation in GenX.

=============== ============================================================
Parameters
=============== ============================================================
RPMInput
                this holds the universal VME and Spatial Neighbor Pixel (NP)
                input payloads.

CUInput
                this holds the Coding Unit input payload.

ValidCULength
                the size of CUInput. Legal values are: 1, 4, 7, 10, 13, or
                16.

curSurfIndex
                the surface index.

RPMOutput
                this stores the 4x4 z-order based approximate predicted
                residual value or approximate predicted pixel for 32x32 data
                area (RPM).
=============== ============================================================

cm_vme_hevc_srm
"""""""""""""""

.. code-block:: c++

  void cm_vme_hevc_srm(matrix<uchar, 8, 32> SRMInput, matrix<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> SRMOutput);

  void cm_vme_hevc_srm(matrix_ref<uchar, 8, 32> SRMInput, matrix_ref<uchar, 16, 32> CUInput,
    int ValidCULength, SurfaceIndex curSurfIndex, matrix_ref<uchar, 18, 32> SRMOutput);

These generate code to perform a Skip Replacement Message (SRM) operation in GenX hardware.

=============== ============================================================
Parameters
=============== ============================================================
SRMInput
                this holds the universal VME and Neighbor Motion Vector
                input payloads.

CUInput
                this holds the Coding Unit input payload.

ValidCULength
                the size of CUInput, the legal value as follows: 1, 4, 7,
                10, 13, or 16.

curSurfIndex
                the surface index.

SRMOutput
                this holds the SRM output payload, comprising the universal
                VME return data and SRM applied Coding Unit data (CU).
=============== ============================================================


4.14 Media Walker Interface
---------------------------

To support the Media Walker functionality, C for Metal provides the following intrinsic functions for a kernel to
query the thread position in the thread space. Please refer to C for Metal runtime specification on the definition
of thread space and usage models of Media Walker [6].  The kernel can use these functions to obtain the
per-thread X/Y origin and color values set by the C for Metal runtime host.

get_thread_origin_x
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  unsigned short get_thread_origin_x();

This function returns the X value of the thread origin in the thread space. The return value is from 0 to
predefined thread space width - 1.

get_thread_origin_y
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  unsigned short get_thread_origin_y();

This function returns the Y value of the thread origin in the thread space. The return value is from 0 to
predefined thread space height - 1.

get_color
^^^^^^^^^

.. code-block:: c++

  unsigned short get_color();

This function returns the color value of the thread origin in the thread space. The return value is from 0 to
predefined thread space color value - 1.

If thread space is not defined by C for Metal runtime host, the default thread space width is fixed at 511.

4.15 Synchronization Functions
------------------------------

C for Metal provides the following intrinsic functions for synchronization:

cm_wait
^^^^^^^

.. code-block:: c++

  void cm_wait(unsigned char mask = 0);

If a thread dependency pattern is specified during the creation of the kernel's thread space, this function
causes the thread to wait until all of its dependency threads have terminated before resuming execution.
Refer to [6] for more information on how to create thread dependencies during kernel launch.

=============== ============================================================
Parameters
=============== ============================================================
mask
                the thread dependency clear mask. Each bit corresponds to
                one of the eight threads this thread may depend on; if set,
                the dependency is cleared, and this thread will not wait for
                the corresponding thread's termination.
=============== ============================================================

cm_fence
^^^^^^^^

.. code-block:: c++

  void cm_fence(unsigned char mask);

This function causes thread execution to block until all previous reads and writes issued by the thread
have been globally observed.

=============== ============================================================
Parameters
=============== ============================================================
mask
                {Gen8+} a bit mask that controls additional cache flush or
                fence behavior. Valid masks are:

                * CM_GLOBAL_COHERENT_FENCE: enables commit enable setting
                * CM_L3_FLUSH_INSTRUCTIONS: flushes the instruction cache
                * CM_L3_FLUSH_TEXTURE_DATA: flushes the sampler cache
                * CM_L3_FLUSH_CONSTANT_DATA: flushes the constant cache
                * CM_L3_FLUSH_RW_DATA: flushes the read-write cache
                * CM_LOCAL_BARRIER: enables local memory barrier
                * CM_L1_FLUASH_RO_DATA: flushes L1 read-only cache
                * CM_SW_BARRIER: enables software scheduling barrier

                The masks may be combined if more than one cache is to be
                flushed.

                {pre-Gen8}  The field is ignored and the cache will not be
                flushed.
=============== ============================================================


cm_pause
^^^^^^^^

.. code-block:: c++

  void cm_pause(unsigned short length);

This function causes thread to pause for a length of time specified by
"length". The value is decremented by the hardware thread control every 32 EU
cycles. Note that the EU clock frequency is variable so the pause is at best an
approximate pause. Generally it will be longer than the value written. For
architectures that don't support (pre Gen10) the call will have no effect.

=============== ============================================================
Parameters
=============== ============================================================
length
                number of 32 cycle chunks to pause for
=============== ============================================================

cm_nbarrier_init {PVC+}
^^^^^^^^^^^^^^^^^^^^^^^

void cm_nbarrier_init (uchar count);

Initialize named barrier.

=============== ============================================================
Parameters
=============== ============================================================
count
                The count of named barriers (must be a compile-time constant)
=============== ============================================================

cm_nbarrier_signal {PVC+}
^^^^^^^^^^^^^^^^^^^^^^^^^

void cm_nbarrier_signal (uint barrierId, uint prodConsMode, uint numProducers, uint numConsumers);

Perform signal operation for the given named barrier.

=============== ============================================================
Parameters
=============== ============================================================
barrierId
                The named barrier id (it's value cannot exceed the total count of initialized named barriers)

prodConsMode
                A 2-bit flag to indicate if it's producer mode (0x1) or consumer mode (0x2). Programmer must
                ensure the input value is set in correct range and higher order bits are cleared.

numProducers
                Number of producers

numConsumers
                Number of consumers
=============== ============================================================

cm_nbarrier_wait {PVC+}
^^^^^^^^^^^^^^^^^^^^^^^

void cm_nbarrier_wait (uchar id);

Wait on a named barrier.

=============== ============================================================
Parameters
=============== ============================================================
id
                The named barrier id (it's value cannot exceed the total count of initialized named barriers)
=============== ============================================================


4.16 Raw Send Functions
-----------------------

C for Metal provides the following APIs for issuing raw send messages to GenX shared functions.

cm_send
^^^^^^^

.. code-block:: c++

  void cm_send(matrix_ref<T1, N1, N2> rspVar,
               matrix<T2, N3, N4> msgVar,
               uint exDesc, uint msgDesc, uint sendc);

cm_sends {Gen9+}
^^^^^^^^^^^^^^^^

.. code-block:: c++

  void cm_sends(matrix_ref<T1, N1, N2> rspVar,
                matrix<T2, N3, N4> msgVar,
                matrix<T3, N5, N6> msgVar2,
                uint exDesc, uint msgDesc, uint sendc);

The formal parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
rspVar
                the matrix that stores the message response data.

msgVar
                the matrix that stores the message payload data.

msg2Var
                the matrix that stores the second part of the message
                payload data in a split send (Gen9+)

exDesc
                the extended message descriptor, which must be a compile
                time constant.

msgDesc
                the message descriptor for the send.

sendc
                the flag that indicates whether sendc should be used. It
                must be a compile time constant (0 indicates send should be
                used, while 1 means sendc)
=============== ============================================================

Note: When using raw send API, in general, it is the programmer's responsibility to ensure that the input
parameters match the documentation requirement for a specific shared function message (e.g., payload
size, alignment, descriptor value, etc.). The response data and payload data are 32-bytes aligned.
When the destination is not present, the first parameter can be specified as "NULL", which is
supported by overloaded API with other parameters unchanged.


cm_raw_send
^^^^^^^^^^^

.. code-block:: c++

  void cm_raw_send(matrix_ref<T1, N1, N2> rspVar,
                   matrix<T2, N3, N4> msgVar,
                   matrix<T3, N5, N6> msgVar2,
                   uint exDesc, uint msgDesc,
                   uchar execSize, uchar sfid,
                   uchar numSrc0, uchar numSrc1, uchar numDst,
                   uchar isEOT, uchar isSendc,
                   vector<ushort, N> mask);

The formal parameters are described below:

=============== ============================================================
Parameters
=============== ============================================================
rspVar
                the matrix that stores the message response data.

msgVar
                the matrix that stores the first message payload data.

msgVar2
                the matrix that stores the second message payload data.

exDesc
                the extended message descriptor for the send.

msgDesc
                the message descriptor for the send.

execSize
                The execution size, which must be a compile time constant that conforms to the following format:
                Bit[2..0]: size of the region for source and destination operands

                * 0b000: 1 element (scalar)
                * 0b001: 2 elements
                * 0b010: 4 elements
                * 0b011: 8 elements
                * 0b100: 16 elements
                * 0b101: 32 elements

                Bit[7..4]: execution mask (explicit control over the enabled channels)

                * 0b0000: M1
                * 0b0001: M2
                * 0b0010: M3
                * 0b0011: M4
                * 0b0100: M5
                * 0b0101: M6
                * 0b0110: M7
                * 0b0111: M8
                * 0b1000: M1_NM
                * 0b1001: M2_NM
                * 0b1010: M3_NM
                * 0b1011: M4_NM
                * 0b1100: M5_NM
                * 0b1101: M6_NM
                * 0b1110: M7_NM
                * 0b1111: M8_NM

sfid
                the shared function ID. It must be a compile time constant.

numSrc0
                the number of GRF registers for payload 0. It must be a compile time constant.

numSrc1
                the number of GRF registers for payload 1. It must be a compile time constant.

numDst
                the number of GRF registers for destination. It must be a compile time constant.

isEOT
                the flag that indicates whether this is also EOT message. It must be a compile
                time constant (optional - default to 0)..

isSendc
                the flag that indicates whether sendc should be used. It must be a compile time
                constant (optional - default to 0).

mask
                the predicate to specify enabled channels (optional - default to on).
=============== ============================================================

Note: This replaces the cm_send/cm_sends API above to provide a more flexible
programming interface for raw send. When there is no Dst/Src1 present,
rspVar/msgVar2 may be specified as NULL, in which case numDst/numSrc1 must be 0.


**Cm-icl compiler only:**

The SIMD width of the resulting ``send`` or ``sends`` instruction is always SIMD16.
The ``cm_send`` or ``cm_sends`` call cannot be inside SIMD control flow.

**Cmc compiler only:**

The SIMD width of the resulting ``send`` or ``sends`` instruction defaults to SIMD16.
However, if the ``cm_send`` or ``cm_sends`` is inside SIMD control flow, the
SIMD width of the ``send`` or ``sends`` instruction is adjusted to match the SIMD
control flow. The effect of the SIMD condition on the ``send`` or ``sends`` depends
on how the shared function unit interprets the execution mask.

If the ``SIMD_IF_BEGIN`` has a vector condition that evaluates
to all channels true, then the SIMD control flow code is optimized away but the
SIMD width of the ``send`` or ``sends`` instruction is adjusted anyway.

Note: cm_send and cm_sends are deprecated for PVC+.

cm_raw_sendg {Xe3P+}
^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(vector_ref<DstT, DstWidth> Dst,
                    vector<Src0T, Src0Width> Src0,
                    vector<Src1T, Src1Width> Src1,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(int Dst,
                    vector<Src0T, Src0Width> Src0,
                    vector<Src1T, Src1Width> Src1,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(vector_ref<DstT, DstWidth> Dst,
                    vector<Src0T, Src0Width> Src0,
                    int Src1,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(int Dst,
                    vector<Src0T, Src0Width> Src0,
                    int Src1,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(vector_ref<DstT, DstWidth> Dst,
                    vector<Src0T, Src0Width> Src0,
                    vector<Src1T, Src1Width> Src1,
                    uint64_t Ind0,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(int Dst,
                    vector<Src0T, Src0Width> Src0,
                    vector<Src1T, Src1Width> Src1,
                    uint64_t Ind0,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(vector_ref<DstT, DstWidth> Dst,
                    vector<Src0T, Src0Width> Src0,
                    int Src1,
                    uint64_t Ind0,
                    vector<uint16_t, ExecSize> Mask = 1);

  template <uint8_t SFID, uint64_t Desc, int ExecSize>
  void cm_raw_sendg(int Dst,
                    vector<Src0T, Src0Width> Src0,
                    int Src1,
                    uint64_t Ind0,
                    vector<uint16_t, ExecSize> Mask = 1);

This built-in function represents raw sendg instruction for a low-level shared
functions access.

Note: When using raw sendg API, in general, it is the users's responsibility to
ensure that the input parameters match the hardware requirement for a specific
shared function message (e.g., payload size, alignment, descriptor value, etc).
The compiler doesn't check correctness of the parameters for the
``cm_raw_sendg`` function calls.

The formal parameters are described below:

========== ====================================================================
Parameters
========== ====================================================================
SFID       The shared function ID. Must be compile-time constant.

Desc       64-bit message descriptor. Must be compile-time constant.

ExecSize   SIMD width of the operation. Must be compile-time constant. If the
           ``Mask`` parameter is specified, the definition of ``ExecSize``
           value can be omitted. In this case the ``ExecSize`` value is
           implicitly set to match the ``Mask`` width. If both ``ExecMask`` and
           ``Mask`` are specified, the ``Mask`` vector width must match
           ``ExecSize``.

Dst        Target to write the message response to. Must be ``vector_ref`` or
           ``NULL``.

Src0       Vector holding the first message payload source.

Src1       Vector holding the second (optional) message payload source.
           Is allowed to be ``NULL``, which means no source is present.

Ind0       Indirect message descriptor. Optional, default is ``null`` register.

Mask       Execution mask. Optional, default is "all lanes enabled". The
           ``Mask`` vector width must match ``ExecSize`` value if it's
           explicitly specified.

========== ====================================================================

cm_get_value
^^^^^^^^^^^^

.. code-block:: c++

  unsigned int cm_get_value(T index);

This intrinsic can be used to assemble the message descriptor, since C for Metal does not allow direct
manipulation of abstract handles such as SurfaceIndex. The input parameter must be one of the
abstract data types (SurfaceIndex, SamplerIndex or VmeIndex). The return data is the encapsulated
index value.

cm_get_r0
^^^^^^^^^

.. code-block:: c++

  template <typename T = uint, unsigned Width = 8>
  vector<T, Width> cm_get_r0();

This intrinsic can be used to access the thread payload register R0, needed to assemble the raw
send message's payload. The return data is a vector of the specified type and width. The supported
element types are ``uint``, ``int``, ``float``. The width must be a power of 2, and the default is 8.
The width must not exceed the general-purpose register width for the target architecture.


4.17 C for Metal label function
-------------------------------

The cm_label function is deprecated in cmc. For backwards compatibility it is ignored if used.

4.18 Formatted Output
---------------------

4.18.1 printf
^^^^^^^^^^^^^

Starting with version 3.0, C for Metal includes support for calling printf in-kernel: it prints formatted output
from a C for Metal kernel to stdout on the host. Printf is defined as:

.. code-block:: c++

  int printf(const char* format, ...);

C for Metal device printf works similar to printf in <stdio.h> with the following exceptions:

* Unlike standard printf, which returns the number of bytes of the formatted output written
  to stdout, C for Metal printf returns the total unformatted bytes written to the special surface that is
  reserved to printf.

* The format string for the C for Metal printf must have fewer than 128 bytes.

* There can be no more than 64 parameters passed to a C for Metal printf call.

* The following conversion specifiers are currently not supported:  p, n.

To use printf there are certain changes required in the host code to allocate the printf surface,
read and format the data written, and then print the result to stdout after the kernel finishes
execution. Please refer to the C for Metal Runtime API documentation for more information.

Example

.. literalinclude:: ../../../test/CMFE/cmlangspec/4_17_1_a.cpp
      :language: c++
      :lines: 20-38

For a kernel with four threads the following would print (not necessarily in the same order):

.. code-block:: text

  Number of bytes returned: 224
  Number of bytes returned: 224
  Number of bytes returned: 224
  Number of bytes returned: 224
  Print a double: 4.2
  Print a double: 4.2
  Print a double: 4.2
  Print a double: 4.2
  Print a string: Hello
  Print a string: Hello
  Print a string: Hello
  Print a string: Hello
  Hello from tid 0 with tx/ty: 0/0

.. _SharedVirtualMemory:

4.19 Shared Virtual Memory (SVM)
--------------------------------

Shared Virtual Memory allows the C for Metal program and the host application to share complex pointer-
containing data structures such as trees and lists. A pointer to a location within SVM has the same pointer
value on the host and in the C for Metal program.

In CM, the type svmptr_t represents a pointer. It can be used as a parameter in a kernel (corresponding to
an actual pointer passed from the host application), as the element type in a vector or matrix, and as a
field within a struct declared for use within a data structure in memory shared with the host application.

The svmptr_t type is in fact an integer type, carrying no information on what type it points to. Therefore
adding one makes the pointer point to the next byte in memory, not the next item in an array of whatever
it points to.

Currently offsetof cannot be used in a C for Metal program, so the offset of a field within a struct must be
computed manually by summing sizeof() each field before, taking care to avoid errors from alignment
gaps.

The size of svmptr_t is determined by the /DCM_PTRSIZE=32 or /DCM_PTRSIZE=64 compiler options. If
svmptr_t is used as a kernel parameter or in a data structure shared between host application and CM
program, then the size of svmptr_t must agree with the host application pointer size (and thus the vISA
resulting from compilation is not portable between 32 bit and 64 bit host applications). A mismatch
between pointer size in the host application and in the C for Metal program will cause undefined behavior.

cm_svm_block_read
^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Type, int N>
  void cm_svm_block_read(svmptr_t v_Addr, vector_ref<TYPE, N> v_Src);

Read N data elements (byte, word, dword, qword) into the vector
'v_Src' from a single block starting at address 'v_Addr'.
The address must be oword (16-byte) aligned.

cm_svm_block_read_unaligned
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Type, int N>
  void cm_svm_block_read_unaligned(svmptr_t v_Addr, vector_ref<TYPE, N> v_Src);

Read N data elements (byte, word, dword, qword) into the vector
'v_Src' from a single block starting at address 'v_Addr'.
The address must be dword (4-byte) aligned.

cm_svm_scatter_read
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Type, int N>
  void cm_svm_scatter_read(vector_ref<svmptr_t, N> v_Addr, vector_ref<TYPE, N> v_Src);

Read N data elements (byte, word, dword, qword) into the vector
'v_Src' from the N addresses given in the vector 'v_Addr'.

cm_svm_block_write
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Type, int N>
  void cm_svm_block_write(svmptr_t v_Addr, vector<TYPE, N> v_Src);

Write N data elements (byte, word, dword, qword) given in the vector
'v_Src' as a single block starting at address 'v_Addr'.
The address must be oword (16-byte) aligned.

cm_svm_scatter_write
^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  template <typename Type, int N>
  void cm_svm_scatter_write(vector<svmptr_t, N> v_Addr, vector<TYPE, N> v_Src);

Write N data elements (byte, dword, qword) given in the vector
'v_Src' to the N addresses given in the vector 'v_Addr'.

Note: Writes to overlapping addresses will have undefined
write ordering.

cm_svm_atomic
^^^^^^^^^^^^^

.. code-block:: c++

  template <typename T, int N>
  void cm_svm_atomic(
      CmAtomicOpType aop,
      vector_ref<svmptr_t, N> v_Addr,
      vector_ref<T, N> v_Dst,
      vector_ref<T, N> v_Src0,
      vector_ref<T. N> v_Src1);

Where:

* v_Src0 and v_Src1 are
  optional and are needed based on
  the atomic operation 'aop'.
* N is 8, 16, or 32.
* v_Src0 and v_Src1 both are needed
  when 'aop' is ATOMIC_CMPXCHG.
  No sources needed when 'aop' is
  ATOMIC_INC, ATOMIC_DEC, AND
  ATOMIC_PREDEC.
  For all other atomic operations, only
  one source v_Src0 is needed.
* 'aop' specifies the atomic operation.
  It has to be a compile-time constant.

This causes atomic read-modify-write operations on the
destination locations addressed.
The destination locations are given in 'v_Addr' and the desired
atomic operation is specified as 'aop'. The value returned in
'v_Dst' depends on the atomic operation. Whether the two
optional sources, 'v_Src0' and 'v_Src1', are needed also
depends on the atomic operation.

cm_svm_gather4_scaled
""""""""""""""""""""""

.. code-block:: c++

  template <typename T, int N, int M>
  cm_svm_gather4_scaled(vector<svmptr_t, N> vOffset, vector<T, M> vDst,
                        ChannelMaskType mask)

Where:

* N = 8, 16, or 32;
* M = N*C where C is the number of channels enabled in 'mask';
* TYPE must be of size dword (so, could be int, uint, or float);
* 'mask' specifies the channels that are enabled -- it has to be a
  compile-time constant of the enum type SLM_ChannelMaskType that can
  have one of channel mask values below:

  * CM_R_ENABLE
  * CM_G_ENABLE
  * CM_GR_ENABLE
  * CM_B_ENABLE
  * CM_BR_ENABLE
  * CM_BG_ENABLE
  * CM_BGR_ENABLE
  * CM_A_ENABLE
  * CM_AR_ENABLE
  * CM_AG_ENABLE
  * CM_AGR_ENABLE
  * CM_AB_ENABLE
  * CM_ABR_ENABLE
  * CM_ABG_ENABLE
  * CM_ABGR_ENABLE

Read N 4-element vectors, say {R,G,B,A}, where each element is of size
dword and is also referred to as a channel, from shared virtual memory
into 'v_Dst'. 'v_Dst' must be aligned.

cm_svm_scatter4_scaled
""""""""""""""""""""""

.. code-block:: c++

  template <typename T, int N, int M>
  cm_svm_scatter4_scaled(vector<svmptr_t, N> vOffset, vector<T, M> vSrc,
                         ChannelMaskType mask)

Where:

* N = 8, 16, or 32;
* M = N*C where C is the number of channels enabled in 'mask';
* TYPE must be of size dword (so, could be int, uint, or float);
* 'mask' specifies the channels that are enabled -- it has to be a
  compile-time constant of the enum type SLM_ChannelMaskType that can
  have one of four contiguous channel masks below:

  * CM_R_ENABLE
  * CM_GR_ENABLE
  * CM_BGR_ENABLE
  * CM_ABGR_ENABLE

Write N 4-element vectors, say {R,G,B,A}, where each element is of
size dword and is also referred to as a channel, from 'v_Src' into
shared virtual memory. 'v_Src' must be aligned.

4.20 Xe Matrix Extension (XMX) Functions
----------------------------------------

C for Metal defines a set of builtin functions that are used to perform matrix
multiplication operations on Xe architecture.

The common matrix multiplication operations perform the following operations:

.. math::

  D_{M \times N} = A_{M \times K} \times B_{K \times N} + C_{M \times N}


The matrix parameters are defined as follows:

* M - number of rows in matrix A, C and D.
* N - number of columns in matrix B, C and D.
* K - number of columns in matrix A and rows in matrix B.

The matrix multiplication operations support the following data types,
represented by ``CmPrecisionType`` enum values and passed as template parameters:

* ``CM_PRECISION_U2`` - 2-bit unsigned integer.
* ``CM_PRECISION_S2`` - 2-bit signed integer.
* ``CM_PRECISION_U4`` - 4-bit unsigned integer.
* ``CM_PRECISION_S4`` - 4-bit signed integer.
* ``CM_PRECISION_U8`` - 8-bit unsigned integer.
* ``CM_PRECISION_S8`` - 8-bit signed integer.
* ``CM_PRECISION_BF`` - 16-bit floating point in bfloat16 format.
* ``CM_PRECISION_HF`` - 16-bit floating point in IEEE-754 binary16 format.
* ``CM_PRECISION_TF32`` - TensorFloat32 (TF32) format.
* ``CM_PRECISION_BF8`` - 8-bit floating point format with 5-bit exponent and 2-bit mantissa.
* ``CM_PRECISION_HF8`` - 8-bit floating point format with 4-bit exponent and 3-bit mantissa.
* ``CM_PRECISION_E2M1`` - 4-bit floating point format with 2-bit exponent and 1-bit mantissa.


All the XMX functions depend on the ``ExecSize`` implicit parameter, which
is equal to the matrix multiplication *N* dimension, i.e. the number of
columns in B, C and D matrices. The ``ExecSize`` parameter is calculated as
the register width divided by the size of the elements in the result
matrix D.

All the Xe matrix extension functions take the following parameters:

=============== ===============================================================
Parameter       Description
=============== ===============================================================
Src1Precision   Precision of the elements in matrix B, passed as Src1.
                Must be one of the ``CM_PRECISION_*`` enum values.

Src2Precision   Precision of the elements in matrix A, passed as Src2.
                Must be one of the ``CM_PRECISION_*`` enum values.

SystolicDepth   Systolic depth of the matrix multiplication operation.
                The systolic depth is used to calculate the matrix
                multiplication *K* dimension. The *K* dimension is
                calculated as the product of the systolic depth and
                the number of operations per channel.

                The *K* dimension is equal to the number of columns in
                matrix A and the number of rows in matrix B.

                The systolic depth must be equal to 8 for all the XMX
                functions.

RepeatCount     The matrix multiplication *M* dimension, i.e. the number of
                rows in A, C and D matrices.

ResTy           The type of the elements in the result matrix D.

AccTy           The type of the elements in the matrix C.

Src1Ty          The storage type for the elements in matrix B. The matrix B
                is stored in a vector of this type. The matrix elements are
                VNNI-packed in the vector.

                The ``Src1Ty`` parameter must be ``int`` or ``uint``.

Src2Ty          The storage type for the elements in matrix A. The matrix A
                is stored in a vector of this type. The matrix elements are
                present in the row-major order in the vector.

                The ``Src2Ty`` parameter must be ``int`` or ``uint``.

AccSize         The number of elements in the C and D matrices. The number
                of elements in the C and D matrices is equal to the product
                of the matrix dimensions *M* and *N*.

Src1Size        The number of elements in the ``Src1`` argument, derived
                from the number of matrix B elements. The number of elements
                in the matrix B is equal to the product of the matrix
                dimensions *K* and *N*.

                The ``Src1Size`` parameter must be equal to
                :math:`\frac{\text{SystolicDepth} \times \text{Src1PrecisionBits} \times \text{OpsPerChannel}}{32} \times \text{ExecSize}`.

Src2Size        The number of elements in the ``Src2`` argument, derived
                from the number of matrix A elements. The number of elements
                in the matrix A is equal to the product of the matrix
                dimensions *M* and *K*.

                The ``Src1Size`` parameter must be equal to
                :math:`\text{RepeatCount} \times \frac{\text{SystolicDepth} \times \text{Src2PrecisionBits} \times \text{OpsPerChannel}}{32}`.

Acc             The matrix C, which is the accumulator matrix. It's represented
                as a vector of ``AccSize`` elements of type ``AccTy`` in
                row-major order.

Src1            The matrix B, which is the source matrix. It's represented as
                a vector of ``Src1Size`` elements of type ``Src1Ty`` in VNNI-
                packed format.

Src2            The matrix A, which is the source matrix. It's represented as
                a vector of ``Src2Size`` elements of type ``Src2Ty`` in row-
                major order.
=============== ===============================================================

The XMX functions support only limited combinations of the matrix precision
types. The following table lists the supported combinations. Some targets may
not support all the combinations. The supported combinations are indicated by
the macros defined in the table.

===================== ================== ============= ========================
``ResTy``, ``AccTy``  Source precision   OpsPerChannel Macros
===================== ================== ============= ========================
``int``               CM_PRECISION_U2,         8       ``CM_HAS_DPAS_INT2``
                      CM_PRECISION_S2

``int``               CM_PRECISION_U4,         8       ``CM_HAS_DPAS_INT4``
                      CM_PRECISION_S4

``int``               CM_PRECISION_U8,         4       ``CM_HAS_DPAS_INT8``
                      CM_PRECISION_S8

``int``               CM_PRECISION_U\*,      4 or 8    ``CM_HAS_DPAS_INT_MIX``
                      CM_PRECISION_S\*

``float``             CM_PRECISION_BF          2       ``CM_HAS_DPAS``

``float``, ``bfloat`` CM_PRECISION_BF          2       ``CM_HAS_DPAS_ACC_BF16``

``float``             CM_PRECISION_HF          2       ``CM_HAS_DPAS``

``float``, ``half``   CM_PRECISION_HF          2       ``CM_HAS_DPAS_ACC_HALF``

``float``             CM_PRECISION_TF32        1       ``CM_HAS_TF32``

``float``, ``bfloat`` CM_PRECISION_BF8,        4       ``CM_HAS_DPAS_BF8``,
                      CM_PRECISION_HF8                 ``CM_HAS_DPAS_HF8``

``float``, ``bfloat`` CM_PRECISION_E2M1        8       ``CM_HAS_DPAS_FP4``
===================== ================== ============= ========================


cm_dpas
^^^^^^^

Dot Product Accumulate Systolic (DPAS) operation is a matrix multiplication
operation that supports the following interface:

.. code-block:: c++

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy>
  vector<ResTy, AccSize> cm_dpas(vector<AccTy, AccSize> Acc,
                                 vector<Src1Ty, Src1Size> Src1,
                                 vector<Src2Ty, Src2Size> Src2);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount>
  vector<AccTy, AccSize> cm_dpas(vector<AccTy, AccSize> Acc,
                                 vector<Src1Ty, Src1Size> Src1,
                                 vector<Src2Ty, Src2Size> Src2);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy>
  vector<ResTy, AccSize> cm_dpas(int, // dummy parameter, must be NULL
                                 vector<Src1Ty, Src1Size> Src1,
                                 vector<Src2Ty, Src2Size> Src2);

The following restrictions are applied to the functions:
* RepeatCount must be from 1 to 8.


These functions are target-dependent and only available when the ``CM_HAS_DPAS``
macro is defined.

cm_dpasw
^^^^^^^^

Dot Product Accumulate Systolic Wide (DPASW) operation is a matrix multiplication
operation that supports the following interface:

.. code-block:: c++

  template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
            int RepeatCount>
  vector<AccTy, AccSize> cm_dpasw(vector<AccTy, AccSize> Acc,
                                  vector<T1, Src1Size> Src1,
                                  vector<T2, Src2Size> Src2);

  template <CmPrecisionType Src1Ty, CmPrecisionType Src2Ty, int SystolicDepth,
            int RepeatCount, typename AccTy, typename T1, typename T2,
            int AccSize>
  vector<AccTy, AccSize> cm_dpasw(int Dummy, // dummy parameter, must be NULL
                                  vector<T1, Src1Size> Src1,
                                  vector<T2, Src2Size> Src2);

The following restrictions are applied to the functions:
* RepeatCount must be from 1 to 8.
* Only the integer and 16-bit floating point formats are supported.

These functions are target-dependent and only available when the ``CM_HAS_DPASW``
macro is defined.

cm_dpasw performs similar operation as cm_dpas, with source2 shared by
two fused EU threads, therefore the size of source2 is reduced by half.

cm_dpasw should not be used in partially fused thread groups (e.g., with odd
thread group size) or in divergent code. In such cases the behavior is undefined.


cm_bdpas
^^^^^^^^

Block Scaling Dot Product Accumulate Systolic (BDPAS) operation is a matrix multiplication
operation using block scaling format that supports the following interface:

.. code-block:: c++

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy, 
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale,
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<AccTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale,
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  int NullSrc1Scale, // dummy parameter, must be NULL
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<AccTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  int NullSrc1Scale, // dummy parameter, must be NULL
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<AccTy, AccSize> cm_bdpas(vector<AccTy, AccSize> Acc,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
            typename Src2Ty, int AccSize,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(int Null,  // dummy parameter, must be NULL
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale,
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
            typename Src2Ty, int AccSize,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(int Null,
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  int NullSrc1Scale, // dummy parameter, must be NULL
                                  vector<uint8_t, Src2ScaleSize> Src2Scale);

  template <CmPrecisionType Src1Precision, CmPrecisionType Src2Precision,
            int SystolicDepth, int RepeatCount, typename ResTy, typename Src1Ty,
            typename Src2Ty, int AccSize,
            CmBlockScaleType ScaleTy = CM_BLOCK_SCALE_E8M0>
  vector<ResTy, AccSize> cm_bdpas(int Null,  // dummy parameter, must be NULL
                                  vector<Src1Ty, Src1Size> Src1,
                                  vector<Src2Ty, Src2Size> Src2,
                                  vector<uint8_t, Src1ScaleSize> Src1Scale);

==================== ========================================================================
Parameter            Description
==================== ========================================================================
Src1Precision        Precision of the elements in matrix B, passed as Src1.
                     Must be one of the ``CM_PRECISION_*`` enum values.
                     Only the 16-bit, 8-bit and 4-bit floating point formats are supported.

Src2Precision        Precision of the elements in matrix A, passed as Src2.
                     Must be one of the ``CM_PRECISION_*`` enum values.
                     Only the 16-bit, 8-bit and 4-bit floating point formats are supported.

RepeatCount          The matrix multiplication *M* dimension, i.e. the number of
                     rows in A, C and D matrices. Must be 8.

SystolicDepth        Systolic depth of the matrix multiplication operation. 
                     The systolic depth is used to calculate the matrix multiplication *K* 
                     dimension. The *K* dimension is calculated as the product of the systolic
                     depth and the number of operations per channel ``OpsPerChannel``.

                     The *K* dimension is equal to the number of columns in
                     matrix A and the number of rows in matrix B.

                     The systolic depth must be equal to 8.

ScaleTy              Scaling factor type. See ``CmBlockScaleType`` enum values below. 

Src1Scale            Scaling factor for matrix B in ``ScaleTy`` format.
                     When NULL the scaling elements are 1.0f.
                     Both Src1Scale and Src2Scale cannot be NULL.

Src2Scale            Scaling factor for matrix A in ``ScaleTy`` format.
                     When NULL or omitted the scaling elements are 1.0f.
                     Both Src1Scale and Src2Scale cannot be NULL.

Src1ScaleSize        Depends on ``SystolicDepth``, ``Src1Precision`` and ``ScaleTy``:
                     ``max(1, K / BDpasScaleBlockSize(ScaleTy)) * ExecSize``.

Src2ScaleSize        Depends on ``SystolicDepth``, ``RepeatCount``, ``Src2Precision`` and
                     ``ScaleTy``: ``max(1, K / BDpasScaleBlockSize(ScaleTy)) * RepeatCount``.

ExecSize             The number of 32-bit elements per register.

BDpasScaleBlockSize  The block size for the scaling factors, depends on 
                     the ``ScaleTy`` value. it's 32 for ``CM_BLOCK_SCALE_E8M0``
==================== ========================================================================

OpsPerChannel per source precision is calculated as follows:

=========================== =============
Precision                   OpsPerChannel
=========================== =============
``CM_PRECISION_BF``         2
``CM_PRECISION_HF``         2
``CM_PRECISION_BF8``        4
``CM_PRECISION_HF8``        4
``CM_PRECISION_E2M1``       8
=========================== =============

These functions are target-dependent and only available when the ``CM_HAS_BDPAS``
macro is defined.

Scaling factor type is represented by ``CmBlockScaleType`` enum values and 
passed as template parameters:

* ``CM_BLOCK_SCALE_E8M0`` - E8M0 floating-point format.

For backward compatibility, ``ScaleTy`` may also be provided as the 6th explicitly
specified template argument in forms such as
``cm_bdpas<Src1Precision, Src2Precision, SystolicDepth, RepeatCount, ResTy, ScaleTy>(...)``.


4.21 Preprocessor Directives
----------------------------

Currently C for Metal supports the following preprocessor directives (pragmas):

#pragma unroll (n)
^^^^^^^^^^^^^^^^^^

This pragma may be specified immediately before a loop to instruct the C for Metal compiler to unroll a loop
for n times. The C for Metal compiler may still not unroll the loop if infeasible.

#pragma unroll
^^^^^^^^^^^^^^

This pragma may be specified immediately before a loop to instruct the C for Metal compiler to completely
unroll a loop. The C for Metal compiler may not unroll the loop if infeasible.

4.22 Rounding mode and float control support
--------------------------------------------

C for Metal supports setting of floating point rounding mode and other floating point control on a per-kernel
basis. This is achieved by using the _GENX_FLOAT_CONTROL_ or _GENX_ROUNDING_MODE_ kernel
attributes. _GENX_FLOAT_CONTROL_ supports all control. _GENX_ROUNDING_MODE_ is a synonym for
_GENX_FLOAT_CONTROL_ and is retained for backward compatibility.

The values that the attribute can be set to are a subset of the following that can be OR'ed
together. The compiler will not enforce using mutually exclusive values (for instance 2 rounding
modes) and the results in this case may be undefined.

Valid values for the float control setting are as follows:

======================== =======================================================
Attribute Name           Description
======================== =======================================================
CM_RTE                   Set round to nearest even (default)
CM_RTP                   Set round towards +ve infinity
CM_RTN                   Set round towards -ve infinity
CM_RTZ                   Set round towards zero
CM_DENORM_RTZ            Set all denorm setting to Flush To Zero (no denorms,
                         default)
CM_DENORM_D_ALLOW        Set ``double`` to allow denorms
CM_DENORM_F_ALLOW        Set ``float`` to allow denorms
CM_DENORM_HF_ALLOW       Set ``half`` to allow denorms
CM_DENORM_ALLOW          Set all float types (``double``, ``float``, ``half``)
                         to allow denorms
CM_FLOAT_MODE_IEEE       Set single float mode to IEEE (default)
======================== =======================================================


Round to even (RTE) (default)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_RTE) void rte_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }


Round to positive infinity (RTP)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_RTP) void rtp_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }

Round to negative infinity (RTN)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_RTN) void rtn_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }

Round to zero (RTZ)
^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_RTZ) void rtz_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }

Denorm behaviour
^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_DENORM_ALLOW) void denorm_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }
  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_DENORM_D_ALLOW | CM_DENORM_HF_ALLOW) void denorm_d_hf_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }

Float mode behaviour
^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_FLOAT_MODE_IEEE) void float_ieee_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }

Mixed control examples
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_DENORM_F_ALLOW | CM_RTN) void denorm_and_round_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }
  _GENX_MAIN_ _GENX_FLOAT_CONTROL_(CM_DENORM_ALLOW | CM_RTN | CM_FLOAT_MODE_IEEE) void multiple_settings_kernel(SurfaceIndex ExampleBuffer) {
  ...
  }


4.23 Miscellaneous Functions
----------------------------

cm_get_hwid
^^^^^^^^^^^
.. code-block:: c++

  uint cm_get_hwid();

This intrinsic can be used to access the hardware global thread id.

cm_get_tileid
^^^^^^^^^^^^^
.. code-block:: c++

  uint cm_get_tileid();

This intrinsic can be used to access the device tile id.

cm_assert
^^^^^^^^^

.. code-block:: c++

  #define cm_assert(condition) /* unspecified */

The definition of the ``cm_assert`` macro depends on another macro ``CM_ENABLE_ASSERTS``,
which should set over as a command line option ``-D CM_ENABLE_ASSERTS=1``.
if it is not set, the ``cm_assert`` does nothing.

Otherwise, the assertion is enabled. The ``cm_assert`` checks if its argument
(which must have scalar type) compares equal to zero. If it does, the assertion
outputs implementation-specific diagnostic information and terminates the kernel
execution.

Note: the assertions are only supported, when the kernel is invoked by the Level Zero runtime.

.. _CMInlineAssembly:

5 CM Inline Assembly
====================

The C for Metal implementation provides GNU inline assembly syntax support:

.. code-block:: c++

  asm asm-qualifiers ( AssemblerTemplate
                     [ : OutputOperands ]
                     [ : InputOperands  ])

asm keyword allows to embed vISA assembly instruction into CM code. Asm statements are allowed only inside a function. CM inline assembly doesn't support clobbers since vISA uses virtual registers and there is no memory operands.

Goto labels and asm goto syntax are not supported

+--------------------+--------------------------------------------------------------+
| Parameters         |                     Description                              |
+====================+==============================================================+
| AssemblerTemplate  | This is a literal string that is the template for the vISA   |
|                    | assembly code. It is a combination of fixed text and tokens  |
|                    | that refer to the input and output operands.                 |
+--------------------+--------------------------------------------------------------+
| OutputOperands     | A comma-separated list of the CM variables modified          |
|                    | by the instructions in the AssemblerTemplate.                |
|                    | An empty list is permitted                                   |
+--------------------+--------------------------------------------------------------+
| InputOperands      | A comma-separated list of CM expressions read                |
|                    | by the instructions in the AssemblerTemplate.                |
|                    | An empty list is permitted                                   |
+--------------------+--------------------------------------------------------------+

An assembler template is a literal string containing assembler instructions. The compiler replaces tokens in the template that refer to inputs and outputs.

**Simple example:**

.. code-block:: c++

  int src0, src1, dst;
  // .... here goes initialization ....
  asm ("add (M1, 1) %0 %1 %2" : "=r"(dst) : "r"(src0), "r"(src1));

**Results in:**

.. code-block:: c++

  add (M1, 1)  V34(0,0)<1>  V32(0,0)<0;1,0>  V33(0,1)<0;1,0>

Compiler does not parse the assembler instructions themselves and does not know what they mean or even whether they are valid assembler input.

5.1 Variable types
------------------

GEN Virtual ISA Specification defines several types of variables

- **General variables** -- regular read/write variables, often denoted in VISA ASM listings as VXX

  In your inline assembler you most often will not use VXX explicitly, instead you can operate on CM variables and let VISA register allocator to insert proper decls

  If you want to use, say, V56 explicitly, you need to manually insert decl for this variable. Now compiler will not guarantee that this decl will not be inserted second time on register allocation (it will be obvious bug). In future some mechanism for this may be provided (if requested) and this document will be updated

- **Address variables** -- used to perform indirect access to elements in a general variables

  Inline assembler do not support these explicitly. But implicitly they participate in 'a' constraint processing

- **Predicate variables** -- used to facilitate conditional execution of instructions

  Implicitly they participate in 'cr' constraint processing

- **Sampler Variables** -- represents a handle to the sampler state information when accessing the sampling engine

  We do not support these in any way

- **Surface Variables** -- represents a handle to the underlying surface when performing memory accesses

  We do not support these in any way

5.2 Constraints and Modifiers overview
--------------------------------------

Constraints can say whether an operand may be in a register, and which kinds of register; whether the operand may be an immediate constant, and which possible values it may have

The simplest kind of constraint is a string of letters, each of which describes one kind of operand that is permitted. Table below lists CM specific constraints

+--------------------+----------------------------------------------------------------------------------+
| Constraint string  |                     Description                                                  |
+====================+==================================================================================+
| r                  | General variable operand with row and column offsets and region-based addressing |
+--------------------+----------------------------------------------------------------------------------+
| rw                 | General variable operand only with a variable number                             |
+--------------------+----------------------------------------------------------------------------------+
| a                  | General variable operand with indirect addressing                                |
+--------------------+----------------------------------------------------------------------------------+
| cr                 | Predicate operand                                                                |
+--------------------+----------------------------------------------------------------------------------+
| i                  | Immediate operand                                                                |
+--------------------+----------------------------------------------------------------------------------+
| F                  | Floating-point only immediate operand                                            |
+--------------------+----------------------------------------------------------------------------------+
| n                  | Compile-time known operand                                                       |
+--------------------+----------------------------------------------------------------------------------+

Also we are supporting positional (digit) constraints: 0, 1, etc... Such constraint (for example, "0") indicates that specified input must be in the same place as the output constraint at the (zero-based) index in the output constraint list

Some examples of usage of aforementioned constraints will be introduced later

Constraints may be used with appropriate modifiers

+--------------------+----------------------------------------------------------------------------------+
| Modified string    |                     Description                                                  |
+====================+==================================================================================+
| <no modifier>      | Input-only operands and immediates (read only)                                   |
+--------------------+----------------------------------------------------------------------------------+
| =                  | Output-only operand (write only)                                                 |
+--------------------+----------------------------------------------------------------------------------+
| \+                 | Input and output operand (read and write)                                        |
+--------------------+----------------------------------------------------------------------------------+

**Warning:**
Do not modify the contents of input-only operands (except for inputs tied to outputs). The compiler assumes that on exit from the asm statement these operands contain the same values as they had before executing the statement.

5.3 Read/write operands
-----------------------

Operands using the ‘+’ constraint modifier count as two operands (that is, both as input and output). It means that this operand is both read and written by the instruction. When the compiler fixes up the operands to satisfy the constraints, it needs to know which operands are read by the instruction and which are written by it.

.. code-block:: c++

  //The following X86 inline assembly instructions are fully equivalent
  asm("add %0 %0" : "+r"(x));
  ...
  asm("add %0 %1" : "=r"(x) : "0"(x));

Since there is difference in syntax between source and destination for general variables in vISA, you have to separate source and destination operand in the assembly string for 'r' constraint

.. code-block:: c++

  vector<int, 8> in1, in2, in3, tmp, out;
  // ... here in1, in2, in3, tmp are set to some values ....

  // tmp vector represented by %1 (as output) and %5 (as input) tokens
  asm("add (M1, 8) %1 %2 %3\n"
      "add (M1, 8) %0 %4 %5"
      : "=r"(out), "+r"(tmp)
      : "r"(in1), "r"(in2), "r"(in3));

Results in

.. code-block:: c++

  // tmp is treated as both input and destination
  add (M1, 8)  V36(0,0)<1>  V32(0,0)<1;1,0>  V33(0,0)<1;1,0>
  add (M1, 8)  V37(0,0)<1>  V34(0,0)<1;1,0>  V36(0,0)<1;1,0>

Some of examples below may be rewritten using '+' constraint, but for simplicity we will assume only '=' from now on

5.4 Constraints explained
-------------------------

Here some examples of concrete constraints are elaborated in more details. Proper inline assembler usage involves some intuition and it is critical to go through some simple cases to build up understanding.

5.4.1 'r' constraint
^^^^^^^^^^^^^^^^^^^^

For general variable declared with 'r' constraint, compiler deduces region-based addressing and substitutes strides into the assembly string. Scalars, vectors and matrices are allowed as operands.

.. code-block:: c++

  vector<int, 8> in, out;
  ...
  asm("mov (M1, 8) %0 (-)%1" : "=r"(out) : "r"(in));

Results in

.. code-block:: c++

  mov (M1, 8)  V33(0,0)<1> (-) V32(0,0)<1;1,0>

Also 'r' can accept vector_ref or matrix_ref as an input. Here is an example of select:

.. code-block:: c++

  matrix<float, 8, 4> m1;
  vector<float, 4> out;
  ...
  asm("mov (M1, 4) %0 %1" :
      "=r"(out) :
      "r"(m1.select<2, 2, 2, 2>(0, 0)));

Results in

.. code-block:: c++

  mov (M1, 4)  V33(0,0)<1>  V32(0,0)<8;2,2>

If you don't want compiler to deduce region-based addressing you can manually specify it in the assembly string using 'rw' constraint.

5.4.2 'rw' constraint
^^^^^^^^^^^^^^^^^^^^^

Introduced to express region-based addressing features of vISA. Programmer declares strides and offsets directly in the inline assembly string for source and destination, compiler deduces only the number of general variable

.. code-block:: c++

  matrix<ushort, 8, 8> m;
  ...
  asm("mov (M1, 32) %0(0,0)<1> (-)%0(0,0)<1;1,0>\n"
      "mov (M1, 32) %0(2,0)<1> (-)%0(2,0)<1;1,0>"
      : "+rw"(m));

Results in:

.. code-block:: c++

  mov (M1, 32) V32(0,0)<1> (-)V32(0,0)<1;1,0>
  mov (M1, 32) V32(2,0)<1> (-)V32(2,0)<1;1,0>

In this approach programmer can not be sure that variable actually spans those strides. Since CM supports vector primitives it's safer to let the compiler deduce strides and use 'r' when it's possible.

5.4.3 'a' constraint
^^^^^^^^^^^^^^^^^^^^

Can be used for indirect access to general variable

.. code-block:: c++

  vector<uint32_t, 256> counter(0);
  ...
  vector_ref<uint8_t, 32> idx;
  ...
  #pragma unroll
  for (uint8_t i = 0; i < 32; i++)
    asm("add (M1, 1) %0 %1 %2"
        : "=a"(counter(idx(i)))
        : "a"(counter(idx(i))), "i"(1));

Results in

.. code-block:: c++

  add (M1, 1)  r[A1(0),0]<1>:d  r[A1(0),0]<0;1,0>:d  0x1:d
  ...
  add (M1, 1)  r[A1(1),0]<1>:d  r[A1(1),0]<0;1,0>:d  0x1:d
  ...
  add (M1, 1)  r[A1(2),0]<1>:d  r[A1(2),0]<0;1,0>:d  0x1:d
  ...
  add (M1, 1)  r[A1(3),0]<1>:d  r[A1(3),0]<0;1,0>:d  0x1:d
  ...
  add (M1, 1)  r[A1(4),0]<1>:d  r[A1(4),0]<0;1,0>:d  0x1:d
  ...
  ...
  add (M1, 1)  r[A2(0),0]<1>:d  r[A2(0),0]<0;1,0>:d  0x1:d
  ...
  add (M1, 1)  r[A2(1),0]<1>:d  r[A2(1),0]<0;1,0>:d  0x1:d

Since indirect addressing requires address variable to be initialized it's incorrect to use 'a' with a variable that's can't be indirected. In this case the compiler error will occur.

5.4.4 'cr' constraint
^^^^^^^^^^^^^^^^^^^^^

Accesses operands as predicate variables. It only allows to accept operands of integer types and integer vectors.

Integer vector size shall not be greater then execution size of predicated instruction and shall be power of two.

.. code-block:: c++

  vector<float, 16> src, dst;
  vector<uint16_t, 16> pred_vec;
  ...
  asm ("(%2) mov (M1, 16) %0 %1"
       : "=r"(dst)
       : "r"(src), "cr"(pred_vec));

Compiler creates new predicate variable and intializes it with input vector:

.. code-block:: c++

  .decl V34 v_type=G type=w num_elts=16 align=GRF
  .decl P1 v_type=P num_elts=16
  ...
  cmp.ne (M1, 16) P1 V34(0,0)<1;1,0> 0x0:w
  (P1) mov (M1, 16)  V35(0,0)<1>  V33(0,0)<1;1,0>

pred_vec is interpreted as predicate variable and compiler generates appropriate .decl for it.

5.4.5 Immediate constraints
^^^^^^^^^^^^^^^^^^^^^^^^^^^

CM Inline assembly supports immediate constraints such as 'i', 'F' and 'n'.

'i' and 'F' represent immediates as source operands in inline assembly and allow scalar and vector constants (whenever they are compile-time known or just const)

.. code-block:: c++

  const short init_0_7[8] = {0,1,2,3,4,5,6,7};

  _GENX_MAIN_ void imm_test(SurfaceIndex dst_surf, SurfaceIndex src_surf) {
    vector<ushort, 8> const_v(init_0_7);

    vector<ushort, 8> src, dst;
    read(src_surf, 0, 0, src1);

    asm("mul (M1, 8) %0 %1 %2" : "=r"(dst_vec) : "r"(src1), "i"(const_v);

    write(dst_surf, 0, 0, dst);
  }

Results in:

.. code-block:: c++

  mul (M1, 8)  V34(0,0)<1>  V33(0,0)<1;1,0>  0x76543210:v

'n' stands for compile-time known constant which is written into inline assembly string in decimal representation. It can be used to specify execution size of instruction:

.. code-block:: c++

  template <typename T, unsigned EXEC_SIZE>
  inline _GENX_ vector<T, EXEC_SIZE> madd(vector<T, EXEC_SIZE> s1,
                                          vector<T, EXEC_SIZE> s2,
                                          vector<T, EXEC_SIZE> s3) {
    vector<T, EXEC_SIZE> dst;

    asm("mad (M1, %4) %0 %1 %2 %3"
        : "=r"(dst)
        : "r"(s1), "r"(s2), "r"(s3), "n"(EXEC_SIZE));

    return dst;
  }

  _GENX_MAIN_ void test(SurfaceIndex src1, SurfaceIndex src2, SurfaceIndex src3,
                        SurfaceIndex dst) {

    vector<ushort, 8> s1, s2, s3;

    read(src1, 0, 0, s1);
    read(src2, 0, 0, s2);
    read(src3, 0, 0, s3);

    vector<ushort, 8> d = madd(s1, s2, s3);

    write(dst, 0, 0, d);
  }

Distinction between compile-time known immediates (like n) and general immediates (like i) in VISA, where no linker exists, is all about finalizer

5.4.6 Symbolic names
^^^^^^^^^^^^^^^^^^^^

It is also possible to specify input and output operands using symbolic names which can be referenced within the assembler code. These names are specified inside square brackets preceding the constraint string, and can be referenced inside the assembler code using %[name] instead of a percentage sign followed by the operand number. Using named operands the above example could look like:

.. code-block:: c++

  vector<int , 8> out;
  for (int i = 0; i < 8; i++) {
      asm ("add (M1, 1) %[dst] %[src1] %[src2]"
                      : [dst] "=a"(out)
                      : [src1] "r"(i), [src2] "i"(10));
  }

Results in:

.. code-block:: c++

  add (M1, 1)  r[A0(0),0]<1>:d  V33(0,0)<0;1,0>  0xa:d

Note that the symbolic operand names have no relation whatsoever to other CM identifiers. You may use any name you like, even those of existing CM symbols, but you must ensure that no two operands within the same assembler construct use the same symbolic name.

.. _CMTemplateLibrary:

6 C for Metal Template Library
==============================

The C for Metal implementation also provides  a template library of functions.  Inclusion of the cm/cmtl.h header file
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

  template<T, uint HEIGHT, uint WIDTH> void ReadBlock(SurfaceIndex ibuf,
                                                      int h_pix_pos, int v_pix_pos,
                                                      matrix_ref<T, HEIGHT, WIDTH> block,
                                                      uint surfaceWidth = 0);
  template<T, uint HEIGHT, uint WIDTH> void ReadBlock(SurfaceIndex ibuf,
                                                      CmBufferAttrib buf_attrib,
                                                      int h_pix_pos, int v_pix_pos,
                                                      matrix_ref<T, HEIGHT, WIDTH> block,
                                                      uint surfaceWidth = 0);



=============== ============================================================
Parameters
=============== ============================================================
T
                A type whose size is the same as the size of a single
                surface pixel.

                Example surface types and T values:

                * CM_SURFACE_FORMAT_A8, char
                * CM_SURFACE_FORMAT_V8U8, short
                * CM_SURFACE_FORMAT_A8R8G8B8, int

HEIGHT
                Number of rows for the matrix

WIDTH
                Number of columns

ibuf
                Surface index for input buffer

h_pix_pos
                Horizontal offset in pixels

v_pix_pos
                Vertical offset in pixels

block
                Ref to output matrix (matrix_ref only)

surfaceWidth
                Surface width in pixels. When set ReadBlock will properly
                replicate pixels across the right hand side boundary for
                non-dword aligned surfaces.
=============== ============================================================

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

  template<T, uint HEIGHT, uint WIDTH> void WriteBlock(SurfaceIndex obuf,
                                                       int h_pix_pos, int v_pix_pos,
                                                       matrix_ref<T, HEIGHT, WIDTH> block);



=============== ============================================================
Parameters
=============== ============================================================
T
                A type whose size is the same as the size of a single
                surface pixel.

                Example surface types and T values:

                * CM_SURFACE_FORMAT_A8, char
                * CM_SURFACE_FORMAT_V8U8, short
                * CM_SURFACE_FORMAT_A8R8G8B8, int

HEIGHT
                Number of rows for the matrix

WIDTH
                Number of columns

obuf
                Surface index for output buffer

h_pix_pos
                Horizontal offset in pixels

v_pix_pos
                Vertical offset in pixels

block
                Ref to matrix to write out to surface (matrix_ref only)
=============== ============================================================

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

  template<T, uint WIDTH> void ReadLinear(SurfaceIndex ibuf,
                                          int pix_pos,
                                          vector_ref<T, WIDTH> block);



=============== ============================================================
Parameters
=============== ============================================================
T
                A type whose size is the same as the size of a single
                surface pixel.

                Example surface types and T values:
                * CM_SURFACE_FORMAT_A8, char
                * CM_SURFACE_FORMAT_V8U8, short
                * CM_SURFACE_FORMAT_A8R8G8B8, int

WIDTH
                Number of elements

Ibuf
                Surface index for input buffer

pix_pos
                Offset in pixels

Block
                Ref to vector to read from surface (vector_ref only)
=============== ============================================================

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

  template<T, uint WIDTH> void WriteLinear(SurfaceIndex obuf,
                                           int pix_pos,
                                           vector_ref<T, WIDTH> block);



=============== ============================================================
Parameters
=============== ============================================================
T
                A type whose size is the same as the size of a single
                surface pixel.

                Example surface types and T values:

                * CM_SURFACE_FORMAT_A8, char
                * CM_SURFACE_FORMAT_V8U8, short
                * CM_SURFACE_FORMAT_A8R8G8B8, int

WIDTH
                Number of elements

Obuf
                Surface index for input buffer

pix_pos
                Offset in pixels

Block
                Ref to vector to write to surface (vector_ref only)
=============== ============================================================

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_1_4_a.cpp
      :language: c++
      :lines: 27-37

6.2 Vectorization
-----------------

6.2.1 Vectorize2DKRNL
^^^^^^^^^^^^^^^^^^^^^

6.2.2 _2Dto1D
^^^^^^^^^^^^^

6.3 SLM
-------

NOTE: These routines are not ready for general use at the moment (but are documented here for future
inclusion)

6.3.1 DumpSLM
^^^^^^^^^^^^^

Limited testing

Dump content of SLM to memory

**Description:** Dump the contents of SLM to memory. This is primarily intended as a debugging tool. Note:
this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

**Functions:**

.. code-block:: c++

  void DumpSLM(uint slmX, SurfaceIndex slmDebugSurface, uint size);

=============== ============================================================
Parameters
=============== ============================================================
slmX
                SLM handle

slmDebugSurface
                Linear surface in which to dump contents of SLM

Size
                Number of bytes to dump
=============== ============================================================

**Example:**

.. literalinclude:: ../../../test/CMFE/cmlangspec/5_3_1_a.cpp
      :language: c++
      :lines: 27-41

6.3.2 TransposeFromSLM
^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose from SLM

**Description:** Transpose from SLM. Note: this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

.. code-block:: c++

  template <uint N> void TransposeFromSLM(vector_ref<uint, N*4> dst,
                                          vector_ref<uint, N*4> src);

6.3.3 TransposeToSLM
^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose to SLM

**Description:** Transpose to SLM. Note: this routine is not ready for general use at the moment.

**Author:** Aharon Robbins, Liron Atedgi

.. code-block:: c++

  template <uint N> void TransposeToSLM(vector_ref<uint, N*4> dst,
                                        vector_ref<uint, N*4> src);

6.4 Iselect
-----------

The following templates can be used to pack two small data elements into a single larger data element.
This is useful when one needs  to simultaneously update multiple matrices through iselect. As iselect is a
SIMD-1 for left hand side assignment.

6.4.1 Pack
^^^^^^^^^^

Limited testing

Pack 2 "small" elements into a single double sized element

**Description:**  Use this as part of iselect optimization, see example for more info. Typically packing would
be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

Pack into double is supported on HSW+

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template <Tpackd, Tcompst, uint WD>
  void Pack(vector_ref<Tpackd, WD> src1, vector_ref<Tpackd, WD> src2,
            vector_ref<Tcompst, WD> dst);

=============== ============================================================
Parameters
=============== ============================================================


Tpackd
      Data type of the 2 sources

Tcompst
      Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)

WD
      WIDTH of data vectors

src1
      First source vector

src2
      Second source vector

Dst
      Destination composite vector

6.4.2 Unpack
^^^^^^^^^^^^

Limited testing
UnPack a compisite element previously generated through Pack(), into 2 elements

**Description:**  Use this as part of iselect optimization. Typically packing would be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

Pack into double is supported on HSW+

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template <Tpackd, Tcompst, uint WD>
  void UnPack(vector_ref<Tpackd, WD> dst1, vector_ref<Tpackd, WD> dst2,
              vector_ref<Tcompst, WD> src);

=============== ============================================================
Parameters
=============== ============================================================
Tpackd
                Data type of the 2 sources

Tcompst
                Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)

WD
                WIDTH of data vectors

src
                Source Composite vector

dst1
                First destination vector,  corresponding  to first source
                provided to Pack()

dst2
                Second destination vector, corresponding  to second source
                provided to Pack()
=============== ============================================================

Example:
See the example for Pack

6.4.3 UnpackSingle
^^^^^^^^^^^^^^^^^^

Limited testing

UnPack a composite element previously generated through Pack(), return only 1 of the 2 elements

**Description:**  Use this as part of iselect optimization, Use this API, instead of Unpack, when only 1 element
is required. Typically packing would be:

* uchar/char -> ushort
* short/ushort -> uint
* int/uint -> double

Pack into double is supported on HSW+

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  #define CMRT_LOC_FIRST 0
  #define CMRT_LOC_SECOND 1

  template <Tpackd, Tcompst, uint WD>
  void UnpackSingle(vector_ref<Tpackd, WD> dst, int location, vector_ref<Tcompst, WD> src);

=============== ============================================================
Parameters
=============== ============================================================
Tpackd
                Data type of the 2 sources

Tcompst
                Data  type of output sizeof(Tcompst) = 2sizeof(Tpackd)

WD
                WIDTH of data vectors

Src
                Source Composite vector

Dst
                destination packed element vector

Location
                Indicates which packed vector to retrieve, either
                CMRT_LOC_FIRST or CMRT_LOC_SECOND should be returned.

                The order of vectors is based on the order provided to Pack
                from left to right,
                e.g.
                | Pack(src1,src,2,dst)
                | UnpackSingle(dst, src, CMRT_LOC_FIRST)
                | Will set src  with values from src1
=============== ============================================================


6.5 CachedStack
---------------

A Cached Stack object represents Multi-Channel (SIMD) Stacks, (optionally) stored in a host memory
buffer, with the elements of the stack cached in GRF. The fundamental functionality provided by a Stack
object are:

* Push() -- add an element to top of stack
* Pop()  - extract top element from Stack
* Top()  - read the top element  without extracting it.

6.5.1 CachedStackInit
^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Initializes a CachedStack Object

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<T, uint W, uint CACHESIZE>
  void CachedStackInit(matrix_ref<short, 3, W*STACKCOUNT> context,
                       matrix_ref<int, 2, W> context_ii,
                       uint MaxSize);

=============== ============================================================
Parameters
=============== ============================================================
CACHESIZE
                Pixel Size of Stack cache stored in GRF, currently MUST be:
                CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)

T
                Type of elements stored in stack

W
                Number of channels (SIMD) for an element on stack context,
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

context_ii
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

MaxSize
                Maximum number of elements on stack for a single channel.
                Caller must allocate the linear surface with:

                size >= MaxSize*NumThreads*W*Sizeof(T)

                Optimization note: MaxSize should be aligned to:
                (CACHELINE / sizeof(T))  (CACHELINE = 64 bytes)
=============== ============================================================

**Example:**

.. code-block:: c++

  #include <cm/cmtl.h>

  #define ALIGN_CACHELINE_SHORT(x) (((x) + 64 - 1) & (~ (64 - 1)))

  _GENX_MAIN_ void
  CachedStackExample(SurfaceIndex stack_surf, int ImageHeight)
  {
      ushort x_pos = get_thread_origin_x() * SZ;

      vector<ushort, 16> element = 1;
      vector<ushort, 16> mask = 1;
      int aligned_image_hight = ALIGN_CACHELINE_SHORT(ImageHeight);

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

6.5.2 CachedStackTop
^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Returns a vector containing top elements in the stack

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<T, uint W, uint CACHESIZE>
  void CachedStackTop(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                      matrix_ref<int, 2, W> context_ii,
                      matrix_ref<T, W, CACHESIZE> stack,
                      vector_ref<T,W> element);

=============== ============================================================
Parameters
=============== ============================================================
CACHESIZE
                Pixel Size of Stack cache stored in GRF, currently MUST be:
                CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)

T
                Type of elements stored in stack

W
                Number of channels (SIMD) for an element on stack

Surf
                Pointer to linear surface used for stack

Context
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

context_ii
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

stack
                User allocated stack cache

Element
                Returned top element vector
=============== ============================================================

**Example:**
See example for CachedStackInit


6.5.3 CachedStackPop
^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Conditionaly pop top elements from stack based on mask, for channels with a 0 mask pixel element is
returned to caller but not popped (see :ref:`CachedStackTop`). Note: Caller must avoid Pop() over empty stack
channels. See example for details of how to avoid this.

**Author:** Dori Eldar

**Functions:**

.. code-block:: c++

  template<T, uint W, uint CACHESIZE>
  void CachedStackPop(SurfaceIndex surf, matrix_ref<short,2,W> context,
                      matrix_ref<int,2,W> context_ii,
                      matrix_ref<T,W, CACHESIZE> stack,
                      vector_ref<T,W> element,
                      vector_ref<short,W> mask);

=============== ============================================================
Parameters
=============== ============================================================
CACHESIZE
                Pixel Size of Stack cache stored in GRF, currently MUST be:
                | CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)

T
                Type of elements stored in stack

W
                Number of channels (SIMD) for an element on stack

Surf
                Pointer to linear surface used for stack

Context
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

context_ii
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

Stack
                User allocated stack cache

Element
                Returned top element vector

Mask
                A 0-1 mask indicating top elements to Pop() (mask = 1) vs
                elements to Top() (mask = 0).  Note: currently only a 0,1
                mask is supported, if mask contains other values behavior
                will be undefined
=============== ============================================================

**Example:**

Note: Caller must avoid Pop() over empty stack channels. Caller can use CacheStackEmpty() to set a mask
to avoid this condition:

.. code-block:: text

  vector<short, W> isEmpty;
  vector<short, W> appMask; CallerSetMask(mask);
  CachedStackEmpty<...>(..., isEmpty);
  mask |= (1-isEmpty);
  CachedStackPop<..>(..., mask);

6.5.4 CachedStackPush
^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Push a vector element to top of stack. Note: Caller must not push() over a full stack (top element at
MaxSize -- 1)

**Author:** Dori Eldar

Function:

.. code-block:: c++

  template<T, int W, int CACHESIZE>
  void CachedStackPush(SurfaceIndex surf, matrix_ref<short, 2, W> context,
                       matrix_ref<int, 2, W> context_ii
                       matrix_ref<T, W, CACHESIZE> stack,
                       vector_ref<T, W> element);

=============== ============================================================
Parameters
=============== ============================================================
CACHESIZE
                Pixel Size of Stack cache stored in GRF, currently MUST be:
                | CACHESIZE \*sizeof(T) % 128 == 0 (preferable for optimal IO)

T
                Type of elements stored in stack

W
                Number of channels (SIMD) for an element on stack

surf
                Pointer to linear surface used for stack

context
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

context_ii
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

stack
                User allocated stack cache

element
                Returned top elements vector
=============== ============================================================

**Example:**
See example for CachedStackInit

6.5.5 CachedStackEmpty
^^^^^^^^^^^^^^^^^^^^^^

Limited testing

**Description:**
Returns a per element mask if Stack is empty or not

**Author:** Dori Eldar

.. code-block:: c++

  template<uint W> void CachedStackEmpty(matrix_ref<short, 2, W> context,
                                         matrix_ref<int, 2, W> context_ii,
                                         vector_ref<short, W> isEmpty);

=============== ============================================================
Parameters
=============== ============================================================
W
                Number of channels (SIMD) for an element on stack

context
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

context_ii
                Internally used by CachedStack to maintain cache pointers,
                caller must not modify this parameter.

isEmpty
                0-1 mask,  1 Stack Empty 0 - Otherwise
=============== ============================================================

**Example:**
See example for CachedStackPop

6.6 Matrix transform
--------------------

A series of functions (mainly for 16x16 matrices) to perform various rotation and mirror operations on
matrix_ref matrices.

6.6.1 MirrorVertical
^^^^^^^^^^^^^^^^^^^^

Limited testing

Vertical mirror matrix transform

**Description:** Perform a vertical mirror transformation on a matrix, transforming it to a different output
* - matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T, uint H, uint W> void MirrorVertical(matrix_ref<T,H,W> in, matrix_ref<T,H,W> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

H
                Height of the matrix (number of rows)

W
                Width of the matrix (number of columns)

in
                The input matrix (matrix_ref<T,H,W>)

out
                The output matrix (matrix_ref<T,H,W>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 32, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 32, 16> mirrored_test_matrix;
  cmtl::MirrorVertical(test_matrix.select_all(), mirrored_test_matrix.select_all());

6.6.2 MirrorHorizontal_16x16
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Horizontal mirror for 16x16 matrix transform

**Description:** Perform a horizontal mirror transformation on a 16x16 matrix, transforming it to a different
output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void MirrorVertical(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> mirrored_test_matrix;
  cmtl::MirrorHorizontal_16x16(test_matrix.select_all(), mirrored_test_matrix.select_all());

6.6.3 Rotate90_16x16
^^^^^^^^^^^^^^^^^^^^

Limited testing

Rotate 90 clockwise for 16x16 matrix transform

**Description:** Perform a 90 degree clockwise rotation transformation on a 16x16 matrix, transforming it to
a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void Rotate90_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate90_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.6.4 Rotate270_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing
Rotate 270 clockwise for 16x16 matrix transform

**Description:** Perform a 270 degree clockwise rotation transformation on a 16x16 matrix, transforming it
to a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void Rotate270_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate270_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.6.5 Rotate180_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing

Rotate 180 for 16x16 matrix transform

**Description:** Perform a 180 degree rotation transformation on a 16x16 matrix, transforming it to a
different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void Rotate180_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> rotated_test_matrix;
  cmtl::Rotate180_16x16(test_matrix.select_all(), rotated_test_matrix.select_all());

6.6.6 Transpose_16x16
^^^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose for 16x16 matrix

**Description:** Perform a transpose on a 16x16 matrix, transforming it to a different output matrix

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void Transpose_16x16(matrix_ref<T,16,16> in, matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> transpose_test_matrix;
  cmtl::Transpose_16x16(test_matrix.select_all(), transpose_test_matrix.select_all());

6.6.7 ReverseTranspose16x16
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Limited testing

Reverse transpose for 16x16 matrix

**Description:** Perform a reverse transpose on a 16x16 matrix, transforming it to a different output matrix.
Note: reverse transpose is not an inverse transpose (which is the same as a transpose), but instead a
transpose about the other diagonal than the normal transpose.

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T> void ReverseTranspose_16x16(matrix_ref<T,16,16> in,
                                          matrix_ref<T,16,16> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

in
                The input matrix (matrix_ref<T,16,16>)

out
                The output matrix (matrix_ref<T,16,16>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 16, 16> reverse_transp_test_matrix;
  cmtl::ReverseTranspose_16x16(test_matrix.select_all(),
                               reverse_transp_test_matrix.select_all());

6.6.8 Transpose_8x8
^^^^^^^^^^^^^^^^^^^

Limited testing

Transpose for 8x8 matrix

**Description:** Perform a transpose on an 8x8 matrix, transforming it to a different output matrix

**Author:** Michele Casula

**Functions:**

.. code-block:: c++

  template<T> void Transpose_8x8(matrix_ref<T,8,8> in,
                                 matrix_ref<T,8,8> out);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

In
                The input matrix (matrix_ref<T,8,8>)

Out
                The output matrix (matrix_ref<T,8,8>)
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 8, 8> test_matrix;
  // Set up the values in test matrix in some way
  matrix<float, 8, 8> transpose_test_matrix;
  cmtl::Transpose_8x8(test_matrix.select_all(),
                      transpose_test_matrix.select_all());

6.6.9 map
^^^^^^^^^

Limited testing

Generic square matrix mapping

**Description:** A generic square matrix mapping function. A mapping vector provides the source for
mapping each element. For function T(x) -> T(mapping(x)) = x
e.g. mapping for 2x2 90 deg clockwise turn: mapping = { 2, 0, 3, 1 }

**Author:** Danny Barash, Chaim Rand

**Functions:**

.. code-block:: c++

  template<T, uint W> void map(matrix_ref<T, W,W> in,
                               matrix_ref<T, W,W> out,
                               vector_ref<ushort, W*W> mapping);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the matrix elements

W
                The width and height (rows and columns) for the square matrix

In
                The input matrix (matrix_ref)

Out
                The output matrix (matrix_ref)

mapping
                The mapping vector (vector_ref). Elements proceed in row
                order (e.g. row 0 is defined, then row 1 etc)
=============== ============================================================

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

6.7 Init and assignment
-----------------------

6.7.1 cm_vector_assign
^^^^^^^^^^^^^^^^^^^^^^

Initialise a vector with some simple rules

**Description:**
Initialise a vector with a sequence of values defined using an initial value and a step.

**Functions:**

.. code-block:: c++

  template<T, uint Size> void cm_vector_assign(vector_ref<T,Size> v,
                                               int InitValue, int Step);

=============== ============================================================
Parameters
=============== ============================================================
T
                The type of the vector elements

Size
                The size of the vector

InitValue
                The value to assign to the first element in the vector

Step
                The amount to increase by for each subsequent element
                assigned
=============== ============================================================

**Example:**

.. code-block:: c++

  vector<int, 100> test_vector;
  cmtl::cm_vector_assign(test_vector.select_all(), 20, 10);
  // Assigns vector values 20, 30, 40, ... 1000, 1010

6.7.2 cm_matrix
^^^^^^^^^^^^^^^

Macro to create and initialize a matrix

**Description:**
Create and initialize a new matrix and initialize with some simple values in an optimal way.

**Functions:**

.. code-block:: c++

  #define cm_matrix(M, T, R, C, I, S)

=============== ============================================================
Parameters
=============== ============================================================
M
                Name of the matrix to be created

T
                Type of the matrix to be created

R
                Number of rows in the new matrix

C
                Number of columns in the new matrix

I
                Initial value for the new matrix

S
                Step value between each element
=============== ============================================================

**Example:**

.. code-block:: c++

  cm_matrix(test_matrix, short, 10, 10, 1, 2)
  // Assigns a matrix with values 1, 3, 5 etc. (starting row 0, col 0,
  // row 0, col 1 and so on

6.7.3 cm_vector
^^^^^^^^^^^^^^^

Macro to create and initialize a vector

**Description:**
Create and initialize a new vector and initialize with some simple values in an optimal way.

**Functions:**

.. code-block:: c++

  #define cm_vector(M, T, N, I, S)

=============== ============================================================
Parameters
=============== ============================================================
M
                Name of the vector to be created

T
                Type of the vector  to be created

N
                Number of elements in the new vector

I
                Initial value for the new vector

S
                Step value between each element
=============== ============================================================

**Example:**

.. code-block:: c++

  cm_vector(test_vector, char, 20, 20, -1)
  // Assigns a vector with values 20, 19, 18 etc

6.8 Extended math
-----------------

6.8.1 cm_atan2_fast
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

=============== ============================================================
Parameters
=============== ============================================================
R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

N
                Number of elements (vector variant)

T
                Type for atan2 parameters (scalar variant)

y
                y parameter for standard atan2(y,x) mathematical function

x
                x parameter for standard atan2(y,x) mathematical function

flags
                Set the execution mode flags  (e.g. SAT bit). Defaults to 0
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  Matrix<float, 16, 16> result = cmtl::cm_atan2_fast(y, x);

6.8.2 cm_atan2
^^^^^^^^^^^^^^

Fast atan2

**Description:**
Fast atan2 implementation for various input types (see functions for details). The results of this call are
generated less  quickly but are more accurate than the standard cm_atan2_fast function. It is accurate up
to 0.00001, where cm_atan2 _fast is only precise up to 0.01.
The flag parameter can be used to set the execution mode (e.g. SAT bit) in the same way as other intrinsic
functions.

**Functions:**

.. code-block:: c++

  template <uint R, uint C> matrix<float,R,C> cm_atan2(
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

=============== ============================================================
Parameters
=============== ============================================================
R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

N
                Number of elements (vector variant)

T
                Type for atan2 parameters (scalar variant)

Y
                y parameter for standard atan2(y,x) mathematical function

X
                x parameter for standard atan2(y,x) mathematical function

Flags
                Set the execution mode flags  (e.g. SAT bit). Defaults to 0
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  Matrix<float, 16, 16> result = cmtl::cm_atan2(y, x);

6.8.3 cm_fmod
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

=============== ============================================================
Parameters
=============== ============================================================
R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

N
                Number of elements (vector variant)

T
                Type for fmod parameters (scalar variant)

Y
                y parameter for standard fmod(y,x) mathematical function

X
                x parameter for standard fmod(y,x) mathematical function

Flags
                Set the execution mode flags  (e.g. SAT bit). Defaults to 0
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> y, x;
  // Set up the y and x values in some way ...
  Matrix<float, 16, 16> result = cmtl::cm_fmod(y, x);

6.8.4 cm_floor
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

=============== ============================================================
Parameters
=============== ============================================================
RT
                Return type (elements of return object -- scalar, vector or
                matrix) -- usually float

R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

SZ
                Number of elements (vector variant)

src
                Input parameter which will be floor'ed

flags
                Set the execution mode flags  (e.g. SAT bit). Defaults to 0
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_floor(x);

6.8.5 cm_ceil
^^^^^^^^^^^^^

Floating point ceil function

**Description:**
Produces the smallesT integer not less than input operand (round up) -- this is effectively the same
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

=============== ============================================================
Parameters
=============== ============================================================
RT
                Return type (elements of return object -- scalar, vector or
                matrix) -- usually float

R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

SZ
                Number of elements (vector variant)

src
                Input parameter which will be ceil'ed

flags
                Set the execution mode flags  (e.g. SAT bit). Defaults to 0
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 16, 16> x;
  // Set up the x values in some way ...
  matrix<float, 16, 16> result = cmtl::cm_ceil(x);

6.8.6 cm_tanh
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


=============== ============================================================
Parameters
=============== ============================================================
R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

N
                Number of elements (vector variant)

x
                x represents angle (in radians) to compute tanh
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 4, 4>     v_x;
  // Set up the x values in some way ...
  matrix<float, 4, 4> result = cmtl::cm_tanh(v_x);

6.8.7 cm_tanh_cody_waite
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

=============== ============================================================
Parameters
=============== ============================================================
R
                Number of rows (matrix variant)

C
                Number of columns (matrix variant)

N
                Number of elements (vector variant)
x
                x represents angle (in radians) to compute tanh
=============== ============================================================

**Example:**

.. code-block:: c++

  matrix<float, 4, 4>     v_x;
  // Set up the x values in some way ...
  matrix<float, 4, 4> result = cmtl::cm_tanh_cody_waite(v_x);

7 Address spaces
==============================

The C for Metal programming language supports the utilization of the subsequent address space qualifiers:

.. code-block:: c++

  __private
  __global
  __constant
  __local
  __generic

Variables declared as pointers are considered to point to the private address space if
an address space qualifier is not specified.

7.1 Conversions
------------------------

The following conversion rules between pointers to different address spaces apply in C for Metal:

* A pointer to the generic address space can be cast to a pointer to global, local or private address space.

* A pointer to a global, local or private address space can be cast to a pointer to the generic address space.

* A pointer to a global, local or private address space can be implicitly converted to a pointer to the generic address space, but the inverse implicit conversion is not allowed.

* A pointer to a named address space can not be directly implicitly converted or cast to a pointer of a different named address space

**Example:**

.. code-block:: c++

  __generic int *genptr; // points to generic address space

  // generic -> named address space conversions
  __private int *privptr = reinterpret_cast<__private int *>(genptr); // ok
  __private int *privptr = genptr; // error

  // named -> generic address space conversion
  __generic int *newgenptr = privptr; // ok

  // disjoint address space conversion
  __constant int *constptr = reinterpret_cast<__constant int *>(genptr); // error

7.2 Address Space Qualifier Functions
-------------------------------------

**Description:** Returns a pointer that points to a region in the named address space if the function
can cast a pointer to the named address space. Otherwise it returns a null pointer.

**Functions:**

.. code-block:: c++

  template <typename T> __private T *cm_to_private(__generic T *ptr);
  template <typename T> __global T *cm_to_global(__generic T *ptr);
  template <typename T> __local T *cm_to_local(__generic T *ptr);

**Note:** Both address space qualifier functions and reinterpret_cast are permitted to convert a generic
pointer to a private/local/global pointer. Nonetheless, address space qualifier functions will generate
additional runtime code to identify any address space mismatch. The following example shows the difference
between reinterpret_cast and cm_to_local() function:

**Example:**

.. code-block:: c++

  void bar(__global int *gptr) {
    __generic int *genptr = gptr;
    // The generic pointer encapsulates pointer to __global address space.
    // When it is cast to a pointer to __local we get an invalid pointer as
    // a result since global and local address spaces are disjoint.
    __local int *lptr = reinterpret_cast<__local int *>(genptr);
  }

  void foo(__global int *gptr) {
    __generic int *genptr = gptr;
    __local int *lptr = cm_to_local(genptr); // lptr is null
  }

7.3 Qualifiers for kernel arguments
-----------------------------------

In C for Metal, pointers provided as kernel arguments can target the __local, __global
or __constant address spaces.

**Example:**

.. code-block:: c++

  _GENX_MAIN_ void kernel(__global int *gptr, __local float *lptr, __constant int *cptr) {...}

7.4 Variables in global, local and constant memory
--------------------------------------------------

7.4.1 Global/Constant
^^^^^^^^^^^^^^^^^^^^^

In C for Metal variables allocated in global or constant address spaces must be defined in program scope.
Variables allocated in constant address space must be explicitly initialized at definition.
The values of these variables persist between kernels.

**Example:**

.. code-block:: c++

 __global int globalGV; // ok
 __global int globalGVInit = 42; // ok

 __constant int constGV; // error
 __constant int constGVInit = 55; // ok

 void foo() {
   static __global int globalGVInScope; // ok
   static __constant int constGVInScope = 56; // ok

  __global int varInGlobal; // error
  __constant int varInConst = 57; // error
 }

7.4.2 Local
^^^^^^^^^^^
In C for Metal variables in the local address space may only be declared in the outermost scope of a kernel
function and can be defined with any C or matrix/vector type. They may not have initializers.

**Example:**

.. code-block:: c++

  __local int globalScope; // error

  void foo() {
    __local int funcScope; // error
  }

  _GENX_MAIN_ void kernel(__global int *p) {
    __local float localVar; // ok

    __local int initVar = 5; // error
    if(*p == 0xff) {
      __local float ifScope; // error
    }
  }

7.5 Vector and matrices with elements of a pointer type
-------------------------------------------------------

Vectors and matrices can contain elements of a pointer type with an optional address space qualifier.
Vector/matrix of elements of an integral type can be cast to a vector/matrix of elements of a pointer type
and vice versa.

The conversion between vectors and matrices of pointers to different address spaces is determined
by the rules governing the conversion of their individual elements, as specified in section 7.1.

**Example:**

.. code-block:: c++

  void foo(vector<svmptr_t, 4> in) {
    // integer to pointer conversion
    vector<__global int *, 4> v = reinterpret_cast<vector<__global int *, 4> >(in);

    // address space implicit conversion
    vector<__generic int *, 4> genVPtrs = v;

    // conversion of disjoint address spaces
    vector<__local int *, 4> localVPtrs = v; // error
  }

7.6 Unified memory interface
---------------------------------------
A unified memory interface has been introduced in C for Metal to facilitate memory
manipulations by using of gather, scatter, load and store LLVM instructions.

gather and scatter
^^^^^^^^^^^^^^^^^^

**Description:**

Gather: Reads scalar values from arbitrary memory locations and returns them as a single
vector/matrix.

Scatter: Writes scalar values to arbitrary memory locations.

**Functions:**

.. code-block:: c++

  template <typename T, int N, int A = Align::ELEM_SIZE>
  vector<T, N> gather(vector<_AS T *, N> ptrs, vector<ushort, N> mask = 1);
  template <typename T, int N, int A = Align::ELEM_SIZE>
  vector<T, N> gather(vector<_AS T *, N> ptrs, vector<ushort, N> mask,
                      vector<T, N> passthru);

  template <typename T, int N, int M, int A = Align::ELEM_SIZE>
  matrix<T, N, M> gather(matrix<_AS T *, N, M> ptrs, matrix<ushort, N, M> mask = 1);
  template <typename T, int N, int M, int A = Align::ELEM_SIZE>
  matrix<T, N, M> gather(matrix<_AS T *, N, M> ptrs, matrix<ushort, N, M> mask,
                         matrix<T, N, M> passthru);

  template <typename T, int N, int A = Align::ELEM_SIZE>
  void scatter(vector<T, N> src, vector<_AS T *, N> ptrs,
               vector<ushort, N> mask = 1)
  template <typename T, int N, int M, int A = Align::ELEM_SIZE>
  void scatter(matrix<T, N, M> src, matrix<_AS T *, N, M> ptrs,
               matrix<ushort, N, M> mask = 1)

=============== ============================================================
Parameters
=============== ============================================================
T
                Element type of matrix/vector.
N
                Number of elements (vector variant).
N
                Number of rows (matrix variant).
M
                Number of columns (matrix variant).
A
                Alignment of memory addresses. If not explicitly specified,
                it is assumed to be the size of an element of type T.
_AS
                Optional address space qualifier. Can be one of the following
                values: __private, __global, __local, __constant(only for gather)
                or __generic.
ptrs
                Vector/matrix of pointers which provides the memory locations
                to read(gather)/write(scatter).
mask
                Mask to prevent the memory accesses to the masked-off lanes.
passthru
                Gather-only: the masked-off lanes in the result vector/matrix
                are taken from the corresponding lanes of the ‘passthru’ operand
                if it is specified. Otherwise, masked-off lanes in the resulted
                vector/matrix will be undefined.
=============== ============================================================

load and store
^^^^^^^^^^^^^^

**Description:**

load: Reads data from memory and returns the result.

store: Writes data to memory.

**Functions:**

.. code-block:: c++

  template <typename T, int A = Align::ELEM_SIZE>
  T load(_AS const T *const ptr);

  template <typename T, int A = Align::ELEM_SIZE>
  void store(T val, _AS T *const ptr);

=============== ============================================================
Parameters
=============== ============================================================
T
                Type to store or load. Can be matrix/vector.
A
                Alignment of the memory address. If not explicitly specified,
                it is assumed to be the size of an of type T if T is a scalar
                type or the size of the element of matrix/vector otherwise.
_AS
                Optional address space qualifier. Can be one of the following
                values: __private, __global, __local, __constant(only for load)
                or __generic.
ptr
                Pointer which holds the address of memory to be written/readen.
=============== ============================================================

Appendix A Media Kernel Example
===============================

Linear Filter Example
---------------------

Linear filter effect illustration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: cmlangspec_files/linear_in.png

==> linear filter ==>

.. image:: cmlangspec_files/linear_gold_hw.png

Pseudo code
^^^^^^^^^^^

.. image:: cmlangspec_files/linear_pseudocode.png

C for Metal implementation
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

  _GENX_MAIN_ void
  linear(SurfaceIndex ibuf, SurfaceIndex obuf, uint h_pos, uint v_pos)
  {
  // declare a 8x32 input matrix of uchar elements
      matrix<uchar, 8, 32> in;
  // declare a 6x24 output matrix of uchar elements
      matrix<uchar, 6, 24> out;
      matrix<float, 6, 24> m;

  // C for Metal intrinsic to read the input matrix via dataport
      read(ibuf, h_pos*24, v_pos*6, in);

  // Compute the sums of neighbor elements
      m  = in.select<6,1,24,1>(1,3);
      m += in.select<6,1,24,1>(0,0);
      m += in.select<6,1,24,1>(0,3);
      m += in.select<6,1,24,1>(0,6);
      m += in.select<6,1,24,1>(1,0);
      m += in.select<6,1,24,1>(1,6);
      m += in.select<6,1,24,1>(2,0);
      m += in.select<6,1,24,1>(2,3);
      m += in.select<6,1,24,1>(2,6);

  // Compute the average
      out = m * 0.111f;

  // Write the result to dataport
      write(obuf, h_pos*24, v_pos*6, out);
  }

