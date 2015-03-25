#ifndef _MTK_UTIL_COMMON_H_
#define _MTK_UTIL_COMMON_H_

#include "MTKUtilErrCode.h"
#include "MTKUtilType.h"

#define UTL_IUL_I_TO_X(i)           ((i) << 16)                     ///< Convert from integer to S15.16 fixed-point
#define UTL_IUL_X_TO_I(x)           (((x) + (1 << 15)) >> 16)       ///< Convert from S15.16 fixed-point to integer (round)
#define UTL_IUL_X_TO_I_CHOP(x)      ((x) >> 16)                     ///< Convert from S15.16 fixed-point to integer (chop)
#define UTL_IUL_X_TO_I_CARRY(x)     (((x) + 0x0000FFFF) >> 16)      ///< Convert from S15.16 fixed-point to integer (carry)
#define UTL_IUL_X_FRACTION(x)       ((x) & 0x0000FFFF)              ///< Get fractional part from S15.16 fixed-point

/// linear interpolation with two values and a weighting(S15.16)
#define LINEAR_INTERPOLATION(val1, val2, weighting2)   \
   UTL_IUL_X_TO_I((val1) * (UTL_IUL_I_TO_X(1) - (weighting2)) + (val2) * (weighting2))

/**
 *  \details utility image format enumerator
 */
typedef enum UTL_IMAGE_FORMAT_ENUM
{
    UTL_IMAGE_FORMAT_RGB565=1,
    UTL_IMAGE_FORMAT_BGR565,
    UTL_IMAGE_FORMAT_RGB888,
    UTL_IMAGE_FORMAT_BGR888,
    UTL_IMAGE_FORMAT_ARGB888,
    UTL_IMAGE_FORMAT_ABGR888,
    UTL_IMAGE_FORMAT_BGRA8888,
    UTL_IMAGE_FORMAT_RGBA8888,
    UTL_IMAGE_FORMAT_YUV444,
    UTL_IMAGE_FORMAT_YUV422,
    UTL_IMAGE_FORMAT_YUV420,
    UTL_IMAGE_FORMAT_YUV411,
    UTL_IMAGE_FORMAT_YUV400,
    UTL_IMAGE_FORMAT_PACKET_UYVY422,
    UTL_IMAGE_FORMAT_PACKET_YUY2,
    UTL_IMAGE_FORMAT_PACKET_YVYU,
    UTL_IMAGE_FORMAT_NV21,
    UTL_IMAGE_FORMAT_YV12,

    UTL_IMAGE_FORMAT_RAW8=100,
    UTL_IMAGE_FORMAT_RAW10,
    UTL_IMAGE_FORMAT_EXT_RAW8,
    UTL_IMAGE_FORMAT_EXT_RAW10,
    UTL_IMAGE_FORMAT_JPEG=200
} UTL_IMAGE_FORMAT_ENUM;

/**
 *  \details utility basic image structure
 */
typedef struct UTIL_BASE_IMAGE_STRUCT
{
    MINT32 width;   ///< image width (column)
    MINT32 height;  ///< image height (row)
    void *data;     ///< data pointer    
} UTIL_BASE_IMAGE_STRUCT, *P_UTIL_BASE_IMAGE_STRUCT;

/**
 *  \details utility clipping image structure
 */
typedef struct UTIL_CLIP_IMAGE_STRUCT : UTIL_BASE_IMAGE_STRUCT
{
    MINT32 clip_x;      ///< clipping horizontal offset
    MINT32 clip_y;      ///< clipping vertical offset
    MINT32 clip_width;  ///< clipping width
    MINT32 clip_height; ///< clipping height
} UTIL_CLIP_IMAGE_STRUCT, *P_UTIL_CLIP_IMAGE_STRUCT;

#endif /* _MTK_UTIL_COMMON_H_ */

