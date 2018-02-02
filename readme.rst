=============================
Intel(R) C-for-Media Compiler
=============================

.. contents::
   :local:
   :depth: 2

Introduction
============

The Intel(R) C-for-Media compiler is a new open source compiler that implements CM (C for Media) programming language. CM is a new GPU kernel programming language for Intel HD Graphics. 

This document is a starting guide for setting up the development environment, 
building and using this compiler.

License
=======

The Intel(R) C-for-Media Compiler is distributed under the MIT license. You may obtain a copy of the License at: https//opensource.org/licenses/MIT

Linux prerequisites
===================

Currently We test our compiler on Centos-7.3 and Ubuntu-16.04.

In order to download and build the compiler, we need the following:

- CMake
- gcc 
- git
- make
- Python

Dependences (Intel Components)
==============================

There is no other dependences in order to build the compiler.

However, in order to build and run those sample applications we provide, the easiest way to get start is to download the following package:

- Intel(R) C-for-Media development package 
  https://01.org/c-for-media-development-package/ 

Please refer to the readme included in the package for its usage.

The source code for all the components in this development package are published on github.com/intel. However, putting them together still takes some effort (WIP). 

Building the compiler
=====================

The compiler can be built using the provided script. 

To build the compiler in the default way, but in debug mode (so you get asserts
and you can debug the compiler):

.. code-block:: text

  support/scripts/build.bash -d

The compiler has now been built to a single cmc executable in a build
directory whose name depends on the options to build.bash.
For example, ``build.64.linux/bin/cmc``.

To see the other options available with build.bash, use

.. code-block:: text

  support/scripts/build.bash -h

Running the compiler
====================

.. code-block:: text

  build.64.linux/bin/cmc -isystem support/include test/open_examples/linear_walker/linear_walker_genx.cpp -march=SKL

That will generate a vISA file ``linear_walker_genx.isa`` in the current directory.

See document `cmcuserguide` for further command line options.

Documentation
=============

The formatted documentation starts at

  support/docs/_build/html/index.html

(This path is relative to your cm-compiler root, i.e. where your llvm, support
and test directories are.)

Building and running the examples
=================================

Having installed the CM runtime + driver (from the developement package).  You may use the compiler binary (cmc) you build from source to replace the cmc binary coming with the developement package (under compiler/bin).

You can build and run examples under test/open_examples.

Refer to the readme under test/open_examples.

Supported Platforms
===================

Intel Atom and Core processors supporting Gen9/Gen10 graphics device

Known Issues and Limitations
============================

<insert instructions here>

(*) Other names and brands maybe claimed as property of others.

