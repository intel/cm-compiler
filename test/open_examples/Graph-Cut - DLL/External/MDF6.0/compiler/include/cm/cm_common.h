/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: cm_common.h 25899 2011-08-05 18:44:07Z kchen24 $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 1985-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             Kaiyu Chen
***                      
***                      
***                      
***
*** Description: Cm sampler APIs
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_COMMON_H
#define CM_COMMON_H

#ifdef CMRT_EMU
#define CM_EMU
#endif

#ifndef CM_NOINLINE
    #ifndef CM_EMU
    #ifndef __GNUC__
    #define CM_NOINLINE __declspec(noinline) 
    #else
    #define CM_NOINLINE __attribute__((noinline)) 
    #endif
    #else
    #define CM_NOINLINE
    #endif /* CM_EMU */
#endif

class SurfaceIndex
{
public:
    CM_NOINLINE SurfaceIndex() { index = 0; };
    CM_NOINLINE SurfaceIndex(const SurfaceIndex& _src) { index = _src.index; };
    CM_NOINLINE SurfaceIndex(const unsigned int& _n) { index = _n; };
    CM_NOINLINE SurfaceIndex& operator = (const unsigned int& _n) { this->index = _n; return *this; };
    CM_NOINLINE SurfaceIndex& operator + (const unsigned int& _n) { this->index += _n; return *this; };
    virtual unsigned int get_data(void) { return index; };

private:
    unsigned int index;
#ifdef __GNUC__
    /*
     * Do not delete this line:
     * SurfaceIndex is commonly used as CM kernel function's parameter. 
     * It has virutal table and has copy constructor, so GNU calling convetion will pass the object's pointer to kernel function.
     * This is different with Windows VC++, which always copies the entire object transferred on the callee's stack. 
     * 
     * Depending on the special object size after adding below "extra_byte",  
     * SetKernelArg and SetThreadArg can recognize this object and follow GNU's convention to construct kernel function's stack. 
     * Detail can be found on "http://wiki.ith.intel.com/display/VPGCompiler/CM+Linux"
     */
    unsigned char extra_byte;
#endif
};

class SamplerIndex
{
public:
    CM_NOINLINE SamplerIndex() { index = 0; };
    CM_NOINLINE SamplerIndex(SamplerIndex& _src) { index = _src.get_data(); };
    CM_NOINLINE SamplerIndex(const unsigned int& _n) { index = _n; };
    CM_NOINLINE SamplerIndex& operator = (const unsigned int& _n) { this->index = _n; return *this; };
    virtual unsigned int get_data(void) { return index; };

private:
    unsigned int index;
#ifdef __GNUC__
    /*
     * Do not delete this line:
     * Same reason as SurfaceIndex.
     */
    unsigned char extra_byte;
#endif
};

class VmeIndex

{
public:
    CM_NOINLINE VmeIndex () { index = 0; };
    CM_NOINLINE VmeIndex (VmeIndex& _src) { index = _src.get_data(); };
    CM_NOINLINE VmeIndex (const unsigned int& _n) { index = _n; };
    CM_NOINLINE VmeIndex & operator = (const unsigned int& _n) { this->index = _n; return *this; };
    virtual unsigned int get_data(void) { return index; };
private:
    unsigned int index;
#ifdef __GNUC__
    /*
     * Do not delete this line:
     * Same reason as SurfaceIndex.
     */
    unsigned char extra_byte;
#endif
};

#endif /* CM_SAMPLER_H */
