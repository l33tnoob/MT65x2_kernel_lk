#ifndef VECTOR_MATH_GENERIC_H
#define VECTOR_MATH_GENERIC_H
#include "demac_config.h"
#include <utils/Log.h>

#if APE_24BIT_SUPPORT
libdemac_inline void vector_add(filter_int* vec0, filter_int* vec1)
{
#if FILTER_LEN > 32
    int order = (FILTER_LEN >> 5);
    while (order--)
#endif
    {
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
#if FILTER_LEN > 16
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
#endif
    }
}

libdemac_inline void vector_sub(filter_int* vec0, filter_int* vec1)
{
#if FILTER_LEN > 32
    int order = (FILTER_LEN >> 5);
    while (order--)
#endif
    {
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
#if FILTER_LEN > 16
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
#endif
    }
}

libdemac_inline int32_t scalarproduct(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

#if FILTER_LEN > 32
    int order = (FILTER_LEN >> 5);
    while (order--)
#endif
    {
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
#if FILTER_LEN > 16
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
#endif
    }
    return ret;
}

/*Followed functions is copyed by zhengwen*/
libdemac_inline void vector_add_16(filter_int* vec0, filter_int* vec1)
{
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
}

libdemac_inline void vector_sub_16(filter_int* vec0, filter_int* vec1)
{
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
}

libdemac_inline int32_t scalarproduct_16(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
    return ret;
}

libdemac_inline void vector_add_32(filter_int* vec0, filter_int* vec1)
{
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;

        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;

}

libdemac_inline void vector_sub_32(filter_int* vec0, filter_int* vec1)
{
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;

        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
}

libdemac_inline int32_t scalarproduct_32(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;

    return ret;
}

libdemac_inline void vector_add_64(filter_int* vec0, filter_int* vec1)
{
    int order = (64 >> 5);
    while (order--)
    {
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;

        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline void vector_sub_64(filter_int* vec0, filter_int* vec1)
{
    int order = (64 >> 5);
    while (order--)
    {
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;

        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline int32_t scalarproduct_64(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

    int order = (64 >> 5);
    while (order--)
    {
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
    }
    return ret;
}

libdemac_inline void vector_add_256(filter_int* vec0, filter_int* vec1)
{
    int order = (256>> 5);
    while (order--)
    {
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;

        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline void vector_sub_256(filter_int* vec0, filter_int* vec1)
{
    int order = (256 >> 5);
    while (order--)
    {
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;

        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline int32_t scalarproduct_256(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

    int order = (256 >> 5);
    while (order--)
    {
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
    }
    return ret;
}

libdemac_inline void vector_add_1280(filter_int* vec0, filter_int* vec1)
{
    int order = (1280 >> 5);
    while (order--)
    {
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;

        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
        vec0[0] += vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline void vector_sub_1280(filter_int* vec0, filter_int* vec1)
{
    int order = (1280 >> 5);
    while (order--)
    {
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;

        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
        vec0[0] -= vec1[0]; vec0++; vec1++;
    }
}

libdemac_inline int32_t scalarproduct_1280(filter_int* vec0, filter_int* vec1)
{
    int ret = 0;

    int order = (1280 >> 5);
    while (order--)
    {
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;

        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
        ret += vec0[0] * vec1[0]; vec0++; vec1++;
    }
    return ret;
}
/*Followed functions is copyed by zhengwen*/
#endif

#endif
