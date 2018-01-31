/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cm/cm.h>

#define BODIES_CHUNK 32
#define ELEMS_BODY 4 // pos.xyz + mass
#define SIZE_ELEM sizeof(float)

#define BODIES_PER_RW 8

#define BODY_SIZE ELEMS_BODY *SIZE_ELEM      // 16B
#define ELEMS_RW ELEMS_BODY *BODIES_PER_RW   // 32 floats
#define BODIES_PER_SLM 1024                  // 1KB
#define SLM_SIZE BODIES_PER_SLM *BODY_SIZE   // 16KB
#define ELEMS_CHUNK BODIES_CHUNK *ELEMS_BODY // 32*4
//---------------------------------------------------

#define GPGPU_MODE

static const ushort init_seq[8] = {0, 1, 2, 3, 4, 5, 6, 7};

_GENX_ vector<float, 4 * BODIES_CHUNK> force0;
_GENX_ vector<float, 4 * BODIES_CHUNK> force1;
_GENX_ vector<float, 4 * BODIES_CHUNK> force2;

_GENX_ int gThreadID;

inline _GENX_ void cmk_Nbody_ForEachMB_ForEachSLMBlock(
    SurfaceIndex INPOS, float deltaTime, float softeningSquared,
    int slmBodies,  // Number of SLM bodies
    uint slmBuffer, // SLM buffer identifier
    int iMB         // Macro-Block number for this thread
    ) {
    vector<float, BODIES_CHUNK * ELEMS_BODY> chunk;
    vector<ushort, 16> v_Offset(init_seq);
    v_Offset.select<8, 1>(8) = v_Offset.select<8, 1>(0) + 8;

    // Read this thread's position data from memory
    // ---------------------------------------------
    int thisMB_ID = 4 * gThreadID + iMB; // Global Macro-Block ID

#pragma unroll
    for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW) {
        read(INPOS, (thisMB_ID * BODIES_CHUNK + i) * BODY_SIZE,
             chunk.select<ELEMS_RW, 1>(ELEMS_BODY * i));
    }

    vector<float, BODIES_CHUNK * ELEMS_BODY> tpos1;
    int forceOffset = iMB * BODIES_CHUNK;

    // Compute interaction with other bodies in SLM - 32 bodies in each
    // iteration
    //----------------------------------------------
    for (int i = 0; i < slmBodies; i += BODIES_CHUNK) {
        // Read 32 SLM bodies
        // One cm_slm_read can read upto 16 dwords or 64 bytes
#pragma unroll
        for (int j = 0; j < BODIES_CHUNK; j += BODIES_PER_RW) {
            vector<ushort, 16> v_Off;
            v_Off = (i + j) * ELEMS_BODY + v_Offset;
            cm_slm_read(slmBuffer, v_Off,
                        tpos1.select<ELEMS_RW / 2, 1>(ELEMS_BODY * j));
            v_Off += 16;
            cm_slm_read(slmBuffer, v_Off,
                        tpos1.select<ELEMS_RW / 2, 1>(ELEMS_BODY * j + 16));
        }

        //-------------------------------------------------
        // Note:
        // From here on, all code is same as our earlier code for nbody
        // computation
        // except slight changes in the read and write memory operations - where
        // 'id'
        // is changed to thisMB_ID
        //-------------------------------------------------

        matrix<float, 3, BODIES_CHUNK> pos1;
        pos1.row(0) = tpos1.select<BODIES_CHUNK, 4>(0);
        pos1.row(1) = tpos1.select<BODIES_CHUNK, 4>(1);
        pos1.row(2) = tpos1.select<BODIES_CHUNK, 4>(2);

        for (int j = 0; j < BODIES_CHUNK; j++) {
            //
            // replicate xyzw   --> row0: xxx  row1: yyy  row2: zzz
            //
            vector<float, ELEMS_BODY> pos =
                chunk.select<ELEMS_BODY, 1>(j * ELEMS_BODY);

            matrix<float, 3, BODIES_CHUNK> r01;
            r01.row(0) = pos1.row(0) - pos(0);
            r01.row(1) = pos1.row(1) - pos(1);
            r01.row(2) = pos1.row(2) - pos(2);

            // d^2 + e^2 [6 FLOPS]
            // r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
            vector<float, BODIES_CHUNK> distSqr;
            distSqr = r01.row(0) * r01.row(0) + r01.row(1) * r01.row(1) +
                      r01.row(2) * r01.row(2) + softeningSquared;

            // invDistCube =1/distSqr^(3/2)  [4 FLOPS (2 mul, 1 sqrt, 1 inv)]
            vector<float, BODIES_CHUNK> invDist;
            invDist = cm_rsqrt(distSqr);

            vector<float, BODIES_CHUNK> invDistCube;
            invDistCube = invDist * invDist * invDist;

            // s = m_j * invDistCube [1 FLOP]
            // s = posMass1[3] * invDistCube;
            vector<float, BODIES_CHUNK> s;
            s = invDistCube * pos(3);

            // (m_1 * r_01) / (d^2 + e^2)^(3/2)  [6 FLOPS]
            matrix<float, 3, BODIES_CHUNK> acc;
            acc.row(0) = cm_dp4<float>(r01.row(0), s);
            acc.row(1) = cm_dp4<float>(r01.row(1), s);
            acc.row(2) = cm_dp4<float>(r01.row(2), s);
            force0(forceOffset + j) +=
                cm_sum<float>(acc.row(0).select<BODIES_CHUNK / 4, 4>(0));
            force1(forceOffset + j) +=
                cm_sum<float>(acc.row(1).select<BODIES_CHUNK / 4, 4>(0));
            force2(forceOffset + j) +=
                cm_sum<float>(acc.row(2).select<BODIES_CHUNK / 4, 4>(0));
        }
    }
}

inline _GENX_ void
cmk_Nbody_OutputVelPos_ForEachMB(SurfaceIndex INPOS, SurfaceIndex INVEL,
                                 SurfaceIndex OUTPOS, SurfaceIndex OUTVEL,
                                 float deltaTime, float damping,
                                 int iMB // Macro-Block number for this thread
                                 ) {
    vector<float, BODIES_CHUNK * ELEMS_BODY> chunk;
    int thisMB_ID = 4 * gThreadID + iMB; // Global Macro-Block ID

#pragma unroll
    for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW) {
        read(INPOS, (thisMB_ID * BODIES_CHUNK + i) * BODY_SIZE,
             chunk.select<ELEMS_RW, 1>(ELEMS_BODY * i));
    }

    // read velocity
    // use loop to reduce register use

    vector<float, BODIES_CHUNK * ELEMS_BODY> tvel;

#pragma unroll
    for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW) {
        read(INVEL, (thisMB_ID * BODIES_CHUNK + i) * BODY_SIZE,
             tvel.select<ELEMS_RW, 1>(ELEMS_BODY * i));
    }

    matrix<float, 3, BODIES_CHUNK> vel;
    vector<float, BODIES_CHUNK> invMass;

    vel.row(0) = tvel.select<BODIES_CHUNK, ELEMS_BODY>(0);
    vel.row(1) = tvel.select<BODIES_CHUNK, ELEMS_BODY>(1);
    vel.row(2) = tvel.select<BODIES_CHUNK, ELEMS_BODY>(2);
    invMass = tvel.select<BODIES_CHUNK, ELEMS_BODY>(3);

    // acceleration = force / mass;
    // new velocity = old velocity + acceleration * deltaTime
    vector<float, BODIES_CHUNK> thisForce;

    int forceOffset = iMB * BODIES_CHUNK;
    thisForce = force0.select<BODIES_CHUNK, 1>(forceOffset);
    vel.row(0) += (thisForce * invMass) * deltaTime;
    thisForce = force1.select<BODIES_CHUNK, 1>(forceOffset);
    vel.row(1) += (thisForce * invMass) * deltaTime;
    thisForce = force2.select<BODIES_CHUNK, 1>(forceOffset);
    vel.row(2) += (thisForce * invMass) * deltaTime;

    // new position = old position + velocity * deltaTime
    vel *= damping;

    matrix<float, 3, BODIES_CHUNK> dis;
    dis = vel * deltaTime;

    // mass
    // int chunkOffset = iMB * ELEMS_CHUNK;
    chunk.select<BODIES_CHUNK, ELEMS_BODY>(0) =
        chunk.select<BODIES_CHUNK, ELEMS_BODY>(0) + dis.row(0);
    chunk.select<BODIES_CHUNK, ELEMS_BODY>(1) =
        chunk.select<BODIES_CHUNK, ELEMS_BODY>(1) + dis.row(1);
    chunk.select<BODIES_CHUNK, ELEMS_BODY>(2) =
        chunk.select<BODIES_CHUNK, ELEMS_BODY>(2) + dis.row(2);

#pragma unroll
    for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW) {
        write(OUTPOS, (thisMB_ID * BODIES_CHUNK + i) * BODY_SIZE,
              chunk.select<ELEMS_RW, 1>(ELEMS_BODY * i));
    }

    vector<float, BODIES_CHUNK * ELEMS_BODY> out; // write new velocity
    out.select<BODIES_CHUNK, ELEMS_BODY>(0) = vel.row(0);
    out.select<BODIES_CHUNK, ELEMS_BODY>(1) = vel.row(1);
    out.select<BODIES_CHUNK, ELEMS_BODY>(2) = vel.row(2);
    out.select<BODIES_CHUNK, ELEMS_BODY>(3) = invMass;

#pragma unroll
    for (int i = 0; i < BODIES_CHUNK; i += BODIES_PER_RW) {
        write(OUTVEL, (thisMB_ID * BODIES_CHUNK + i) * BODY_SIZE,
              out.select<ELEMS_RW, 1>(ELEMS_BODY * i));
    }
}

extern "C" _GENX_MAIN_ void cmNBody(SurfaceIndex INPOS, SurfaceIndex INVEL,
                                    SurfaceIndex OUTPOS, SurfaceIndex OUTVEL,
                                    float deltaTime, float damping,
                                    float softeningSquared, int numBodies) {

    // Only 4K bodies fit in SLM
    // 1. Foreach 4K bodies - For a total of 16K bodies
    // 2.   LOAD 4K bodies to SLM: i.e. Read from Memory and Write to SLM
    // 3.   Foreach MB (32 bodies here) - For a total of 4 MBs
    // 4.     READ from Memory: Position of thisThreadBodies
    // 6.     Foreach set of 32 bodies In 4K SLM bodies
    // 7        READ from SLM: Position of 32 bodies
    // 8.       Compute Interaction between thisThreadBodies and the 32
    //            bodies read from SLM; Compute and update force0, force1,
    //            force2 for forces in 3D
    // 9.     READ from Memory: Velocity of thisThreadBodies
    // 10.     Compute New Velocity and New Position of thisThreadBodies
    // 11.     WRITE to Memory: New Velocity of thisThreadBodies
    // 12.     WRITE to Memory: New Position of thisThreadBodies

    cm_slm_init(SLM_SIZE);
    uint bodiesInSLM = cm_slm_alloc(SLM_SIZE);

    gThreadID = cm_linear_global_id();
    force0 = force1 = force2 = 0.0f;

    // 1. Foreach 4K bodies - For a total of 16K bodies
    for (int iSLM = 0; iSLM < 4; iSLM++) {

        // 2. LOAD 4K bodies to SLM: i.e. Read from Memory and Write to SLM

        cm_slm_load(
            bodiesInSLM,     // slmBuffer   : SLM buffer
            INPOS,           // memSurfIndex: Memory SurfaceIndex
            iSLM * SLM_SIZE, // memOffset   : Byte-Offset in Memory Surface
            SLM_SIZE         // loadSize    : Bytes to be Loaded from Memory
            );

        // Each thread needs to process 4 Macro-Blocks (MB):
        //            One MB = 32 Bodies; Total 2 Groups with 64 threads/Group
        //            => #Bodies/Thread = TotalNumBodies/TotalNumThreads
        //                              = 16384/128 = 128 = 4 MBs
        //   - Depending on the number of threads, the number of MBs per
        //     threads can be changed by just changing this loop-count
        //   - For optimization purpose, if there are enough GRFs we can
        //     process more MBs per iteration of this loop - in that case
        //     need to change the loop-stride accordingly; If all MBs can
        //     be processed in the GRF, we can eliminate this loop

        for (int iMB = 0; iMB < 4; iMB++) {
            cmk_Nbody_ForEachMB_ForEachSLMBlock(
                INPOS, deltaTime, softeningSquared, BODIES_PER_SLM, bodiesInSLM,
                iMB);
        } // end foreach(MB)
    }     // end foreach(SLM block)

    for (int iMB = 0; iMB < 4; iMB++) {
        cmk_Nbody_OutputVelPos_ForEachMB(INPOS, INVEL, OUTPOS, OUTVEL,
                                         deltaTime, damping, iMB);
    } // end foreach(MB)
}
