======================================
C-for-Media Compiler developers' guide
======================================

This document provides more guidance for a developer of the compiler.

Debugging and testing
=====================

Debugging the compiler
----------------------

What cmc does is gather command line options and then spawn another copy of
itself to do the compilation, and it is the second cmc process that we need to
debug. Therefore you need to run the compiler first with the -### option
to get the command line it passes to the second cmc.

   .. code-block:: text

     $ build.64.linux/bin/cmc -isystem support/include test/open_examples/linear_walker/linear_walker_genx.cpp -march=SKL -###
     clang version 3.5.0 (MDF6.0.63156a3) (MDF6.0.9586dd4)
     Target: genx32-unknown-linux-gnu
     Thread model: posix
     "/home/gangche1/cm-llvm/build.64.linux/bin/clang-3.5" "-cc1" "-triple" "genx32-unknown-linux-gnu" "-S" "-disable-free" "-disable-llvm-verifier" "-main-file-name" "linear_walker_genx.cpp" "-mrelocation-model" "static" "-mdisable-fp-elim" "-fmath-errno" "-no-integrated-as" "-mconstructor-aliases" "-target-cpu" "SKL" "-target-feature" "+svmptr-64" "-fdiagnostics-format" "msvc" "-Wshadow" "-Wuninitialized" "-fvldst" "-gline-tables-only" "-dwarf-column-info" "-resource-dir" "/home/gangche1/cm-llvm/build.64.linux/bin/../lib/clang/3.5.0" "-isystem" "support/include" "-internal-isystem" "/home/gangche1/cm-llvm/build.64.linux/bin/../include_llvm" "-fno-dwarf-directory-asm" "-fdebug-compilation-dir" "/home/gangche1/cm-llvm" "-ferror-limit" "19" "-fmessage-length" "80" "-mstackrealign" "-fobjc-runtime=gcc" "-fdiagnostics-show-option" "-o" "linear_walker_genx.isa" "-x" "cm" "test/open_examples/linear_walker/linear_walker_genx.cpp"


The LLVM and Clang source
=========================

Within the llvm directory:

* Public include files are in ``include/llvm/``. The ones that define the IR are
  in ``include/llvm/IR/``.

* .cpp files are in ``lib/``.

* The GenX target (including its header files) is in ``lib/Target/GenX/``. Code
  in ``lib/Target/GenX/GenXTargetMachine.cpp`` determines the flow of passes in
  the GenX backend by creating the passes and adding them to the pass
  manager.

* The declarations of GenX-specific intrinsics are in
  ``include/llvm/IR/IntrinsicsGenX.td``.

* There are a few passes specific to the CM compiler that are not part of the
  GenX backend. They are in
  ``lib/Transforms/Scalar/CM*.cpp``

* Clang is in ``tools/clang/``

Basic debugging hints
---------------------

The LLVM options -print-before-all and -print-after-all cause the LLVM IR to be
dumped to stdout before or after each pass, including the GenX specific ones.
Where applicable, the GenX specific ones dump the IR in bales, and also dump
the liveness or register allocation information. An LLVM option can be specified
on the cmc command line by preceding it with -mllvm.

The cmc option -emit-llvm causes the compiler to stop just before entering the
GenX backend and output an LLVM bitcode file with suffix .bc. This file can be
disassembled using llvm-dis.

Many LLVM objects, including all the LLVM IR objects, have a ``dump()`` method that
can be used from within the debugger. For example, if I am stopped somewhere
with ``Instruction *Inst``, I can F9 for quickwatch and type the expression
``Inst->dump()`` to get a meaningful dump of what that instruction is in the
application's console window.


Submiting and publishing changes
================================

<insert instructions here>

