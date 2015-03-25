/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UITYPES_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UITYPES_H_


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Camera UI Types.
 ******************************************************************************/
struct  MPoint;
struct  MSize;
struct  MRect;


/**  
 *
 * @brief Camera point type.
 *
 */
struct MPoint
{
    typedef int                 value_type;

    value_type                  x;
    value_type                  y;

#ifdef __cplusplus
public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions
    inline                      MPoint(int _x = 0, int _y = 0)
                                    : x(_x), y(_y)
                                {
                                }

public:     ////                Operators.

    // Checks for equality between two points.
    inline  bool                operator == (MPoint const& rhs) const
                                {
                                    return (x == rhs.x) && (y == rhs.y);
                                }

    // Checks for inequality between two points.
    inline  bool                operator != (MPoint const& rhs) const
                                {
                                    return ! operator == (rhs);
                                }

    inline  bool                operator < (MPoint const& rhs) const
                                {
                                    return y<rhs.y || (y==rhs.y && x<rhs.x);
                                }

    inline  MPoint&             operator += (MPoint const& rhs)
                                {
                                    x += rhs.x;
                                    y += rhs.y;
                                    return *this;
                                }

    inline  MPoint&             operator -= (MPoint const& rhs)
                                {
                                    x -= rhs.x;
                                    y -= rhs.y;
                                    return *this;
                                }

    inline  MPoint              operator + (MPoint const& rhs) const
                                {
                                    MPoint const result(x+rhs.x, y+rhs.y);
                                    return result;
                                }

    inline  MPoint              operator - (MPoint const& rhs) const
                                {
                                    MPoint const result(x-rhs.x, y-rhs.y);
                                    return result;
                                }

    inline  MPoint&             operator - ()
                                {
                                    x = -x;
                                    y = -y;
                                    return *this;
                                }

public:     ////                Attributes.

    inline  bool                isOrigin() const
                                {
                                    return !(x|y);
                                }
#endif //__cplusplus
};


/**  
 *
 * @brief Camera size type.
 *
 */
struct MSize
{
    typedef int                 value_type;
    value_type                  w;
    value_type                  h;

#ifdef __cplusplus
public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions

    inline                      MSize(int _w = 0, int _h = 0)
                                    : w(_w), h(_h)
                                {
                                }

    inline                      MSize(MPoint const& topLeft, MPoint const& bottomRight)
                                    : w(bottomRight.x-topLeft.x), h(bottomRight.y-topLeft.y)
                                {
                                }

public:     ////                Operations.

    // Returns the product of w and h.
    inline  value_type          size() const
                                {
                                    return  w * h;
                                }

public:     ////                Operators.

    // Checks for invalid size with width <= 0 or height <= 0.
    inline  bool                operator ! () const
                                {
                                    return  (w <= 0) || (h <= 0);
                                }

    // Checks for equality between two sizes.
    inline  bool                operator == (MSize const& rhs) const
                                {
                                    return (w == rhs.w) && (h == rhs.h);
                                }

    // Checks for inequality between two sizes.
    inline  bool                operator != (MSize const& rhs) const
                                {
                                    return ! operator == (rhs);
                                }

    // Adds a size to this size.
    inline  MSize&              operator += (MSize const& rhs)
                                {
                                    w += rhs.w;
                                    h += rhs.h;
                                    return *this;
                                }

    // Subtracts a size from this size.
    inline  MSize&              operator -= (MSize const& rhs)
                                {
                                    w -= rhs.w;
                                    h -= rhs.h;
                                    return *this;
                                }

    // Adds two sizes.
    inline  MSize               operator + (MSize const& rhs) const
                                {
                                    MSize const result(w+rhs.w, h+rhs.h);
                                    return result;
                                }

    // Subtracts two sizes.
    inline  MSize               operator - (MSize const& rhs) const
                                {
                                    MSize const result(w-rhs.w, h-rhs.h);
                                    return result;
                                }

    // Multiplies a size by a scalar.
    inline  MSize               operator * (value_type scalar) const
                                {
                                    MSize const result(w*scalar, h*scalar);
                                    return result;
                                }

    // Divides a size by a scalar.
    inline  MSize               operator / (value_type scalar) const
                                {
                                    MSize const result(w/scalar, h/scalar);
                                    return result;
                                }

    // Shifts bits in a size to the right.
    inline  MSize               operator >> (value_type shift) const
                                {
                                    MSize const result(w>>shift, h>>shift);
                                    return result;
                                }

    // Shifts bits in a size to the left.
    inline  MSize               operator << (value_type shift) const
                                {
                                    MSize const result(w<<shift, h<<shift);
                                    return result;
                                }

#endif //__cplusplus
};


/**  
 *
 * @brief Camera rectangle type.
 *
 */
struct MRect
{
    typedef int                 value_type;
    MPoint                      p;      //  left-top corner
    MSize                       s;      //  width, height

#ifdef __cplusplus
public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions

    inline                      MRect(int _w = 0, int _h = 0)
                                    : p(0, 0), s(_w, _h)
                                {
                                }

    inline                      MRect(MPoint const& topLeft, MPoint const& bottomRight)
                                    : p(topLeft), s(topLeft, bottomRight)
                                {
                                }

    inline                      MRect(MPoint const& _p, MSize const& _s)
                                    : p(_p), s(_s)
                                {
                                }

public:     ////                Operators.

    // Checks for equality between two rectangles.
    inline  bool                operator == (MRect const& rhs) const
                                {
                                    return  (p == rhs.p) && (s == rhs.s);
                                }

    // Checks for inequality between two rectangles.
    inline  bool                operator != (MRect const& rhs) const
                                {
                                    return ! operator == (rhs);
                                }

public:     ////                Accessors.

    inline  MPoint              leftTop()       const   { return p; }
    inline  MPoint              leftBottom()    const   { return MPoint(p.x, p.y+s.h); }
    inline  MPoint              rightTop()      const   { return MPoint(p.x+s.w, p.y); }
    inline  MPoint              rightBottom()   const   { return MPoint(p.x+s.w, p.y+s.h); }

    inline  MSize const&        size()          const   { return s; }

    inline  value_type          width()         const   { return s.w; }
    inline  value_type          height()        const   { return s.h; }

public:     ////                Operations.

    inline  void                clear()
                                {
                                    p.x = p.y = s.w = s.h = 0;
                                }

#endif //__cplusplus
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UITYPES_H_

