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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IMETADATA_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IMETADATA_H_
//
#ifndef USING_MTK_LDVT
#include <utils/String8.h>
#endif
#include <mtkcam/common.h>
#include "mtk_metadata_tag.h"
#include "mtk_metadata_tag_type.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class IMetadata;
#include "mtk_metadata_tag_info.inl"


/******************************************************************************
 *  Camera Metadata Interface
 ******************************************************************************/
class IMetadata
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MUINT32                     Tag_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Entry Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    class   IEntry
    {
    public:     ////                    Instantiation.
        virtual                         ~IEntry();
                                        IEntry(Tag_t tag = -1);

        /**
         * Copy constructor and copy assignment.
         */
                                        IEntry(IEntry const& other);
        IEntry&                         operator=(IEntry const& other);

    public:     ////                    Accessors.

        /**
         * Return the tag.
         */
        virtual MUINT32                 tag() const;

        /**
         * Check to see whether it is empty (no items) or not.
         */
        virtual MBOOL                   isEmpty() const;

        /**
         * Return the number of items.
         */
        virtual MUINT                   count() const;

        /**
         * Return how many items can be stored without reallocating the backing store.
         */
        virtual MUINT                   capacity() const;

        /**
         * Set the capacity.
         */
        virtual MBOOL                   setCapacity(MUINT size);

    public:     ////                    Operations.

        /**
         * Clear all items.
         * Note: Copy-on write.
         */
        virtual MVOID                   clear();

        /**
         * Delete an item at a given index.
         * Note: Copy-on write.
         */
        virtual MERROR                  removeAt(MUINT index);

#define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
        virtual MVOID                   push_back(_T const& item, Type2Type<_T>); \
        virtual _T&                     editItemAt(MUINT index, Type2Type<_T>); \
        virtual _T const&               itemAt(MUINT index, Type2Type<_T>) const;

        IMETADATA_IENTRY_OPS_DECLARATION(MUINT8)
        IMETADATA_IENTRY_OPS_DECLARATION(MINT32)
        IMETADATA_IENTRY_OPS_DECLARATION(MFLOAT)
        IMETADATA_IENTRY_OPS_DECLARATION(MINT64)
        IMETADATA_IENTRY_OPS_DECLARATION(MDOUBLE)
        IMETADATA_IENTRY_OPS_DECLARATION(MRational)
        IMETADATA_IENTRY_OPS_DECLARATION(MPoint)
        IMETADATA_IENTRY_OPS_DECLARATION(MSize)
        IMETADATA_IENTRY_OPS_DECLARATION(MRect)
        IMETADATA_IENTRY_OPS_DECLARATION(IMetadata)

#undef  IMETADATA_IENTRY_OPS_DECLARATION

    protected:  ////                    Implementor.
                                        class Implementor;
        Implementor*                    mpImp;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Instantiation.
    virtual                             ~IMetadata();
                                        IMetadata();

    /**
     * Copy constructor and copy assignment.
     */
                                        IMetadata(IMetadata const& other);
    IMetadata&                          operator=(IMetadata const& other);

public:     ////                        Accessors.

    /**
     * Check to see whether it is empty (no entries) or not.
     */
    virtual MBOOL                       isEmpty() const;

    /**
     * Return the number of entries.
     */
    virtual MUINT                       count() const;

public:     ////                        Operations.

    /**
     * Clear all entries.
     * Note: Copy-on write.
     */
    virtual MVOID                       clear();

    /**
     * Delete an entry by tag.
     * Note: Copy-on write.
     */
    virtual MERROR                      remove(Tag_t tag);

    /**
     * Sort all entries for faster find.
     * Note: Copy-on write.
     */
    virtual MERROR                      sort();

    /**
     * Update metadata entry. An entry will be created if it doesn't exist already.
     * Note: Copy-on write.
     */
    virtual MERROR                      update(Tag_t tag, IEntry const& entry);

    /**
     * Get metadata entry by tag for editing.
     * Note: Copy-on write.
     */
    virtual IEntry&                     editEntryFor(Tag_t tag);

    /**
     * Get metadata entry by tag, with no editing.
     */
    virtual IEntry const&               entryFor(Tag_t tag) const;

    /**
     * Get metadata entry by index for editing.
     * Note: Copy-on write.
     */
    virtual IEntry&                     editEntryAt(MUINT index); 

    /**
     * Get metadata entry by index, with no editing.
     */
    virtual IEntry const&               entryAt(MUINT index) const; 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Bridge.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Implementor.
                                        class Implementor;
    Implementor*                        mpImp;
};


/******************************************************************************
 *
 ******************************************************************************/
 
#ifndef USING_MTK_LDVT
void dumpMetadataEntry(IMetadata::IEntry const& entry, android::String8& rs8Log);
void dumpMetadata(IMetadata const& metadata, android::String8& rs8Log);
#endif


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_UTILS_IMETADATA_H_

