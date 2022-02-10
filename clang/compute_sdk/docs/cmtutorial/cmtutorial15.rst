.. ========================= begin_copyright_notice ============================
  
  Copyright (C) 2021 Intel Corporation
  
  SPDX-License-Identifier: MIT
  
  =========================== end_copyright_notice =============================

=======================================
Tutorial 15. Kernel Example - Graph-Cut
=======================================

The graph-cut algorithm solves the max-flow min-cut problem, which can be found in many global optimization problems, such as background segmentation, image stitching and Network analysis, etc. One such graph-cut algorithm is Push and Relabel, which was designed by Andrew V. Goldberg and Robert Tarjan. This algorithm uses a breadth first approach that makes it possible to run on GPU efficiently with some challenges. The main challenge is how to resolve inter-node dependencies while revealing parallel operations.

There are multiple nodes in a system. Each node has a set of variables such as excess flow and capacities in north, south, east and west directions. The high-level idea of Push and Relabel algorithm is to call Relabel and Push in turn until no active nodes can be further pushed. For background segmentation, the final segmentation result is in height matrix. 

Below is Relabel pseudo code.

.. code-block:: php

    if active(x) do
        my_height = HEIGHT_MAX;
        for each y = neighbor(x)
            if capacity(x,y) > 0 do
                my_height = min(my_height, height(y)+1);
            done
        end
        height(x) = my_height;// update height
    done

The parallel operations of Relabel are applied to reference north, south and east neighbors, and leave west neighbor reference to SIMD1 for algorithm convergence.

Below is Push pseudo code.

.. code-block:: php

    if active(x) do
        foreach y = neighbor(x)
            if height(y) == height(x) – 1 do
                flow = min( capacity(x,y), excess_flow(x));
                excess_flow(x) -= flow;
                excess_flow(y) += flow;
                capacity(x,y) -= flow;
                capacity(y,x) += flow;
            done
        end
    done

The parallel operations of Push are applied with separate row and column push, and preserve thread dependencies horizontally and vertically during push operations. 

The following are the representative C for Metal kernel code.

GC_Relabel_u32

.. literalinclude:: Graph_Cut_genx.cpp
   :language: c++
   :lines: 231-355

GC_V_Push_VWF_u32

.. literalinclude:: Graph_Cut_genx.cpp
   :language: c++
   :lines: 1217-1367

GC_H_Push_NR_VWF_u32

.. literalinclude:: Graph_Cut_genx.cpp
   :language: c++
   :lines: 1525-1672

The source code also includes more variations of these kernels for higher performance. They are beyond of tutorial scope.
