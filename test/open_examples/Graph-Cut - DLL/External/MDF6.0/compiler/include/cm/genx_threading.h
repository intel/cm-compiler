/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: genx_threading.h 25249 2011-04-01 22:31:22Z kchen24 $"
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
*** Authors:             Alexey V. Aliev
***                      
***                      
***                      
***
*** Description: Header for GenX threading model emulation
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef GENX_THREADING_H
#define GENX_THREADING_H

typedef ushort thread_id; 

//Temporary
#ifdef CM_EMU
#define _genx_asm(a)
#endif /* CM_EMU */
// = get_urb_ptr

namespace CmEmulSys {
CMRT_LIBCM_API extern bool volatile is_tm_initialized;
CM_API extern void finalize_tm(); 
extern void enter_dataport_cs();
extern void leave_dataport_cs();
extern void urb_write_bs(int urb_handle, int urb_offset, basic_stream& m);
extern void open_gateway_bs(basic_stream &m, ushort key);
extern int send_msg_bs(thread_id id, ushort key, basic_stream &m, int offset,
                    bool notify = true, bool acknowledge = false);
}; //:: CmEmulSys


CM_API thread_id get_thread_id(void); 
CM_API ushort get_thread_origin_x(void); 
CM_API ushort get_thread_origin_y(void); 
CM_API void *get_urb_ptr(char *reg);
CM_API void close_gateway(void);
CM_API void wait_for_msg(void);
CM_API void spawn(void (*pt2Function)(), int urb_handle, int urb_length, 
                  int urb_offset = 0);
CM_API void end_of_thread(bool dereferenced = false, bool root = false);
CM_API unsigned int get_idp(void (*pt2Function)());

template<typename T, uint SZ>
CM_API int
send_msg(thread_id id, ushort key, const stream<T,SZ> &m, int offset,
         bool notify = true, bool acknowledge = false) {

    int i;
    vector<T, SZ> v;

    for (i = 0; i < SZ; i++) {
        v(i) = m.get(i);
    }

    int ret = CmEmulSys::send_msg_bs(id, key, v, offset, notify, acknowledge);
    return ret;
}

/* Note: Cm spec 1.0 extension. Send scalar variable to MGW. */
template<typename T>
CM_API extern int
send_msg_scalar(thread_id id, ushort key, T scalar, int offset,
                bool notify = true, bool acknowledge = false)
{
    vector<T,1> temp_v(scalar);
    int ret = send_msg(id, key, temp_v, offset, notify, acknowledge);
    return ret;
}

template<typename T, uint SZ>
CM_API void
open_gateway(stream<T, SZ> &m, ushort key) {
    CmEmulSys::open_gateway_bs(m, key);
}

template<typename T, uint SZ>
CM_API void
urb_write(int urb_handle, int urb_offset, const stream<T, SZ> &m) {
    int i;
    vector<T, SZ> v;

    for (i = 0; i < SZ; i++) {
        v(i) = m.get(i);
    }

    CmEmulSys::urb_write_bs(urb_handle, urb_offset, v);
}


#endif /* GENX_THREADING */
