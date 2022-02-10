.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

===============================================
C for Metal with OpenCL and Level Zero runtimes
===============================================

This is work-in-progress.

1 Introduction
==============

With a recent driver, users may compile and launch their CM kernels with the OpenCL and Level Zero host APIs.
This document summarizes the background and necessary changes needed for this feature.

2 The kernel side
=================

Minimal changes to CM kernels are expected so that the existing CM emulation remains functional.
The following simple vector addition kernel illustrates the major differences on SurfaceIndex variables.

.. literalinclude:: vadd.cpp
   :language: c++
   :lines: 18-43

SurfaceIndex is an opaque type that defines a memory handler. The value is the binding
table index (BTI) of the surface state created and managed by the runtime.
For CM, stateful or stateless accesses are specified by kernel developers.
When SurfaceIndex variables are used, all their accesses are stateful by design.

On the other hand, for OpenCL & Level Zero, the base address of the memory object is passed as a kernel argument.
When the memory object size is less than 4GB, compiler may prove that
stateful accesses (with 32-bit offsets) are safe. This allows to convert
some or all stateless accesses (with the 64-bit base pointer) into stateful ones.
This optimization is benefical as 64-bit arithmetics become 32-bit ones, and
send payload sizes are halved.

For CM kernels running on top of OpenCL & Level Zero runtimes, all accesses remain stateful and
the binding table index will be assigned by CM compiler instead, and be communicated
back to the OpenCL & Level Zero runtimes. It is kernel developers' responsibility
to specify proper memory type attribute for every surface argument passing to a kernel.
Otherwise the behavior is undefined.

In case you'd like to use stateless pointer argument instead of stateful one,
please use annotation "svmptr_t" for this argument. The same applies in case
you'd like to use object of sampler type, please annotate it with "sampler_t".

It is kernel developers' responsibility to ensure that CM load or store intrinsics are correctly used. E.g.
CM 2d block reads may only be used for 2d surfaces, and the behavior is undefined otherwise.
As the CM compiler may not be able to distinguish a 1D buffer or a 2D surface from a SurfaceIndex variable,
It is necessary for kernel developers to provide this extra description on the SurfaceIndex variables.
We may extend the CM language to express this information in the future. Right now, they are mandatory to
launch CM kernels on top of the OpenCL & Level Zero runtimes. For 2D surfaces, there are three more optional access
descriptors to improve performance, and the default access descriptor is "read_write".

The list of supported resource descriptors are

- memory type attributes

+------------------------+---------------------------------------------------+
+------------------------+---------------------------------------------------+
|"buffer_t"              |   this resource is a buffer                       |
+------------------------+---------------------------------------------------+
|"svmptr_t"              |   this resource is a SVM buffer                   |
+------------------------+---------------------------------------------------+
|"sampler_t"             |   this resource is a sampler                      |
+------------------------+---------------------------------------------------+
|"image1d_t"             |   this resource is a 1D surface                   |
+------------------------+---------------------------------------------------+
|"image1d_array_t"       |   this resource is a 1D surface array             |
+------------------------+---------------------------------------------------+
|"image1d_buffer_t"      |   this resource is a 1D surface                   |
+------------------------+---------------------------------------------------+
|"image2d_t"             |   this resource is a 2D surface                   |
+------------------------+---------------------------------------------------+
|"image2d_array_t"       |   this resource is a 2D surface array             |
+------------------------+---------------------------------------------------+
|"image2d_media_block_t" |   this resource is a 2D media block surface       |
+------------------------+---------------------------------------------------+
|"image3d_t"             |   this resource is a 3D surface                   |
+------------------------+---------------------------------------------------+

Current implementation of "image2d_t" assumes that it is media block
surface. This behavior is deprecated in favor of plain 2D surface. If
you need to use media block image (for example, to work with
read/write builtins), please use "image2d_media_block_t". Otherwise,
if you need plain 2d image (for example, to work with samplers),
please additionally pass "-vc-use-plain-2d-images" to compilation
options both in offline and online modes to enable new semantics.

- access level attributes

+-------------+---------------------------------------------------------+
+-------------+---------------------------------------------------------+
|"read_only"  |   this resource is read-only, 2D surfaces only          |
+-------------+---------------------------------------------------------+
|"write_only" |   this resource is write-only, 2D Surfaces only         |
+-------------+---------------------------------------------------------+
|"read_write" |   this resource is for read and write, 2D surfaces only |
+-------------+---------------------------------------------------------+

The syntax is in C++ attribute [[type("<desc0> [<desc1>]")]].

.. literalinclude:: vadd.cpp
   :language: c++
   :lines: 28-30

3 The host side
===============

3.1 Threading
-------------

An OpenCL kernel describes the execution of a single work-item. OpenCL compiler
packs multiple work-items (8/16/32) into a HW thread for execution.
On the other hand, OpenCL runtime or HW generates thread local IDs (8/16/32)
stored in some agreed location (part of the thread payload) for a kernel to use.

Unlike OpenCL, a CM kernel describes the execution of a single HW thread,
which only requires a single local thread ID. For this purpose, OpenCL runtime
implemented the SIMD1 dispatch mode. I.e. runtime packs a tuple
(local_idx, local_idy, local_idz) as the per thread payload, and all 32 channels will be
enabled while entering CM kernels.

The above SIMD1 dispatch model implies that the thread group configuration remains consistent.
To be precise, let (NumThreadx, NumThready) be the number
of CM threads needed in a thread group. The OpenCL thread group size should be
(NumThreadx, NumThready) as well. In this example, the CM global sizes are (n / 32, 1)
and CM local sizes are (1, 1). Its corresponding OpenCL ND range shall be defined as follows:

OpenCL example

.. literalinclude:: host.cpp
   :language: c++
   :lines: 18-31

Level Zero example

.. literalinclude:: host_l0.cpp
   :language: c++
   :lines: 18-25


3.2 Surfaces
------------

In OpenCL host code clCreateBuffer and clCreateImage are expected to be used for 1D and 2D surfaces, respectively.
In Level Zero host code zeMemAllocShared and zeImageCreate are expected to be used for 1D and 2D surfaces, respectively.

3.3 Shared virtual memory
-------------------------

This feature enables OpenCL developers to write code with extensive use of pointer-linked
data structures like linked lists or trees that are shared between the host and a device
side of an OpenCL application. Please refer to official OpenCL documentation for details.

This is a simple host code that allocates an SVM buffer and passes it to the kernel.

.. literalinclude:: svm.cpp
   :language: c++
   :lines: 18-

This is a simple kernel code that accesses an SVM buffer.

.. literalinclude:: svm_genx.cpp
   :language: c++
   :lines: 18-

Please refer to :title:`C for Metal Language Specification` for more details.

3.4 Create OpenCL program
-------------------------

OpenCL supports three ways to create kernels from source, from SPIR-V or from pre-compiled OpenCL binaries.
For creating kernel from source, the compile option "-cmc" shall be present, which directs the OpenCL
runtime to compile the source string as CM kernels. For creating kernel from SPIR-V, the compile option
"-vc-codegen" shall be present, which directs the OpenCL runtime to compile the source string as CM kernels.

.. literalinclude:: host.cpp
   :language: c++
   :lines: 34-

3.5 Create Level Zero module
----------------------------

Level Zero supports two ways to create kernels from SPIR-V and from precompiled binaries.
Option "-vc-codegen" shall be present, which directs level Zero runtime to compile provided binary as CM kernels.

.. literalinclude:: host_l0.cpp
   :language: c++
   :lines: 27-


3.6 Options
-----------

Some CM specific options are supported, and the option string works
for both ocloci (tool for managing Intel Compute GPU device binary format)
and OpenCL API clBuildProgram. There are two sets of compilation options:
"-cmc" options for compilation with source and "-vc-codegen" options for
compilation with SPIRV.

+----------------+--------------------------------------------------------+
+----------------+--------------------------------------------------------+
|-cmc            | compile this kernel as a CM kernel from source code    |
+----------------+--------------------------------------------------------+

Additional options for "-cmc" mode can be found in CMC User Guide.

+----------------+--------------------------------------------------------+
+----------------+--------------------------------------------------------+
|-vc-codegen     | compile this kernel as a CM kernel from SPIR-V         |
+----------------+--------------------------------------------------------+

"-vc-codegen" mode has the following additional options:

+-------------------------------------+----------------------------------------------------------+
|-Xfinalizer <arg>                    | options passed through to the vISA compiler              |
+-------------------------------------+----------------------------------------------------------+
|-g                                   | enable debug info generation                             |
+-------------------------------------+----------------------------------------------------------+
|-no-vector-decomposition             | disable splitting of big vectors                         |
+-------------------------------------+----------------------------------------------------------+
|-ze-no-vector-decomposion            | L0 alias for "-no-vector-decomposition"                  |
+-------------------------------------+----------------------------------------------------------+
|-fno-jump-tables                     | disable generation of jump tables (SWITCHJMP)            |
+-------------------------------------+----------------------------------------------------------+
|-ftranslate-legacy-memory-intrinsics | translate legacy dataport messages for newer platforms   |
+-------------------------------------+----------------------------------------------------------+
|-double-GRF                          | twice number of GRF allowed to use in kernel             |
+-------------------------------------+----------------------------------------------------------+
|-ze-opt-large-register-file          | L0 alias for "-double-GRF"                               |
+-------------------------------------+----------------------------------------------------------+
|-vc-use-plain-2d-images              | Treat "image2d_t" annotated surfaces as non-media images |
+-------------------------------------+----------------------------------------------------------+


For example, user could use the following option string to override
the default vISA compilation options:

"-vc-codegen -Xfinalizer '-printregusage -output -nocompaction -noLocalSplit'"

Compilation may fail if unsupported vISA options are supplied.

3.6 Restrictions
----------------

- Global work offset is not supported in CM.
- Surface size may not exceed 4GB in CM

4 Standalone tools
==================

4.1 cmc
-------

As an offline tool, cmc can be used to produce either ben binary or
SPIR-V output with option "-emit-spirv".
To be compiled for OpenCL runtime, the option "-fcmocl" is required.
Please see the CM user manual for more cmc options.

cmc vadd.cpp -isystem <path-to-cm-header> -fcmocl -march=SKL -m64
cmc vadd.cpp -isystem <path-to-cm-header> -fcmocl -march=SKL -m64 -emit-spirv

The list of CPU strings supported can be found in :doc:`../cmcuserguide/cmcuserguide`.

4.2 ocloc
---------

ocloc offline compiler could be used to compile both CM source
and SPIR-V out of cmc into OpenCL binaries

ocloc.exe -file vadd.cpp -device skl  -options "-cmc"

ocloc.exe -file vadd.spv -device skl -spirv_input -options "-vc-codegen" -internal_options "-dumpvisa"

The list of device strings supported are:

* skl,kbl,glk,cfl
* icllp
* tgllp

4.3 libocloc.so
---------------

It is possible to use libocloc.so to conduct compilation for the kernel. To do that
please use oclocInvoke function provided by libocloc.so. You could load either gen binary
or SPIR-V files into OpenCL & Level Zero runtimes.

Please find usage examples below.

For OpenCL

.. literalinclude:: libocloc.cpp
   :language: c++
   :lines: 18-

For Level Zero

.. literalinclude:: libocloc_l0.cpp
   :language: c++
   :lines: 18-

FAQ
===
