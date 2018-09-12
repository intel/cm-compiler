#pragma once

#ifdef CM_EMU
#include <stdio.h>
#else
#include "cm.h"
#include "cm_printf_base.h"

extern const uchar _offset[8] = {0,1,2,3,4,5,6,7};

inline _GENX_ uint cm_print_datatype( float   i) { return CM_PRINT_DATA_TYPE_FLOAT  ; }
inline _GENX_ uint cm_print_datatype(double   i) { return CM_PRINT_DATA_TYPE_DOUBLE ; }

inline _GENX_ uint cm_print_datatype( int8_t  i) { return CM_PRINT_DATA_TYPE_CHAR   ; }
inline _GENX_ uint cm_print_datatype(int16_t  i) { return CM_PRINT_DATA_TYPE_SHORT  ; }
inline _GENX_ uint cm_print_datatype(int32_t  i) { return CM_PRINT_DATA_TYPE_INT    ; }
inline _GENX_ uint cm_print_datatype(int64_t  i) { return CM_PRINT_DATA_TYPE_QWORD  ; }

inline _GENX_ uint cm_print_datatype( uint8_t i) { return CM_PRINT_DATA_TYPE_UCHAR  ; }
inline _GENX_ uint cm_print_datatype(uint16_t i) { return CM_PRINT_DATA_TYPE_USHORT ; }
inline _GENX_ uint cm_print_datatype(uint32_t i) { return CM_PRINT_DATA_TYPE_UINT   ; }
inline _GENX_ uint cm_print_datatype(uint64_t i) { return CM_PRINT_DATA_TYPE_UQWORD ; }

/// Structure of header:
/// vector<int, 8> header
/// [0]: Object type: matrix,vector,scalar,string, or format string.
/// [1]: Data type: [u]*int[136]*[6248]_t, float, or double.
/// [2]: reserved0
/// [3]: reserved1
/// [4]: reserved2
/// [5]: reserved3
/// [6]: Scalar lower 32bits: [u]*int[13]*[628]_t, float. Lower 32bits of double and [u]*int64_t.
/// [7]: Scalar upper 32bits: Upper 32bits of double and [u]*int64_t.

template <typename T>
inline _GENX_ uint cm_printf_compute_size(const T  scalar) { return       PRINT_HEADER_SIZE; }
template <>
inline _GENX_ uint cm_printf_compute_size(const char* str) { return 128 + PRINT_HEADER_SIZE; }

#define CM_PRINTF_WRITE_8BYTE(buff, dv, scalar)                                     \
    vector<uint, 2> iv = dv.format<uint>();                                         \
    vector<uint, 8> header(0);                                                      \
    header(CM_PRINT_OBJECT_TYPE_ENTRY_INDEX) = CM_PRINT_OBJECT_TYPE_SCALAR;         \
    header(CM_PRINT_DATA___TYPE_ENTRY_INDEX) = cm_print_datatype(scalar);           \
    header.select<3,1>(CM_PRINT_LOWER32BITS_ENTRY_INDEX).format<uint>()(0) = iv(0); \
    header.select<3,1>(CM_PRINT_UPPER32BITS_ENTRY_INDEX).format<uint>()(0) = iv(1); \
    write(buff, offset, header)

#define CM_PRINTF_WRITE_STRING(buff, header, strVect, str, STRING_TYPE)             \
    vector<uchar, 128> strVect(str);                                                \
    vector<uint, 8> header(0);                                                      \
    header(CM_PRINT_OBJECT_TYPE_ENTRY_INDEX) = STRING_TYPE;                         \
    header(CM_PRINT_DATA___TYPE_ENTRY_INDEX) = CM_PRINT_DATA_TYPE_CHAR;             \
    write(buff,                     offset,  header);                               \
    write(buff, PRINT_HEADER_SIZE + offset, strVect)

template <typename T>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, const T scalar)
{
    vector<uint, 8> header(0);
    header(CM_PRINT_OBJECT_TYPE_ENTRY_INDEX) = CM_PRINT_OBJECT_TYPE_SCALAR;
    header(CM_PRINT_DATA___TYPE_ENTRY_INDEX) = cm_print_datatype(scalar);
    header.select<3,1>(CM_PRINT_LOWER32BITS_ENTRY_INDEX).format<T>()(0)= scalar;
    write(buff, offset, header);
    return cm_printf_compute_size(scalar);
}

template <>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, const double scalar)
{
    vector<double, 1> dv = scalar;
    CM_PRINTF_WRITE_8BYTE(buff, dv, scalar);
    return cm_printf_compute_size(scalar);
}

template <>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, const int64_t scalar)
{
    vector<int64_t, 1> dv = scalar;
    CM_PRINTF_WRITE_8BYTE(buff, dv, scalar);
    return cm_printf_compute_size(scalar);
}

template <>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, const uint64_t scalar)
{
    vector<uint64_t, 1> dv = scalar;
    CM_PRINTF_WRITE_8BYTE(buff, dv, scalar);
    return cm_printf_compute_size(scalar);
}

template <>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, const char* str)
{
    CM_PRINTF_WRITE_STRING(buff, header, strVect, str, CM_PRINT_OBJECT_TYPE_STRING);
    return cm_printf_compute_size(str);
}

inline _GENX_ uint cm_printf_write_format(SurfaceIndex& buff, uint offset, const char* str)
{
    CM_PRINTF_WRITE_STRING(buff, header, strVect, str, CM_PRINT_OBJECT_TYPE_FORMAT);
    return cm_printf_compute_size(str);
}

/// cm_printf() definition is below.
/// This requires C++11 variadic templates.
#if __VARIADIC_TEMPLATES

template<typename T, typename... Args>
inline _GENX_ uint cm_printf_compute_size(T value, Args... args)
{
    return cm_printf_compute_size(value  )+
           cm_printf_compute_size(args...);
}

template<typename T, typename... Args>
inline _GENX_ uint cm_printf_write_output(SurfaceIndex& buff, uint offset, T value, Args... args)
{
    uint bytesWritten = 0;
    bytesWritten += cm_printf_write_output(buff, bytesWritten + offset, value  );
    bytesWritten += cm_printf_write_output(buff, bytesWritten + offset, args...);
    return bytesWritten;
}

template<typename T, typename... Args>
inline _GENX_ uint __cm_printf_internal(SurfaceIndex& buff, T value, Args... args)
{
    vector<uint, 8> offpos(_offset);
    vector<uint, 8> off(0);
    vector<uint, 8> size;
    size(0) = cm_printf_compute_size(value, args...);
    write(buff, ATOMIC_ADD, 0, offpos, size, off);

    uint offset = off(0);
    uint bytesWritten = 0;
    bytesWritten += cm_printf_write_format(buff, bytesWritten + offset, value  );
    bytesWritten += cm_printf_write_output(buff, bytesWritten + offset, args...);
    return bytesWritten;
}

/// Support the base case arg count of cm_printf().
template<typename T>
inline _GENX_ uint __cm_printf_internal(SurfaceIndex& buff, T value)
{
    vector<uint, 8> offpos(_offset);
    vector<uint, 8> off(0);
    vector<uint, 8> size;
    size(0) = cm_printf_compute_size(value);
    write(buff, ATOMIC_ADD, 0, offpos, size, off);
    return cm_printf_write_format(buff, off(0), value);
}

#define cm_printf(...) __cm_printf_internal(CM_PRINT_BUFFER, __VA_ARGS__)
#define printf(...) __cm_printf_internal(CM_PRINT_BUFFER, __VA_ARGS__)
#define cm_fprintf(...) __cm_printf_internal( __VA_ARGS__ )
#define fprintf(...) __cm_printf_internal( __VA_ARGS__ )

#else
#define printf(...)
#define fprintf(...)
#warning "To use CM device printf, your kernel must be compiled with -Qstd=c++11"
#endif

#endif /* CM_EMU */
