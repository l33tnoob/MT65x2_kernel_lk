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

/******************************************************************************
 *
 ******************************************************************************/
#define LOG_TAG "MtkCam/Metadata"

#include <mtkcam/Log.h>
#include <mtkcam/utils/common.h>
//
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <stdio.h>
using namespace NSCam;
using namespace android;

/******************************************************************************
 * 
 ******************************************************************************/
#if MTKCAM_HAVE_AEE_FEATURE == '1'
#include <aee.h>
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
        aee_system_exception( \
            "mtkcam/Metadata", \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String) 
#endif
/******************************************************************************
 * 
 ******************************************************************************/
class IMetadata::IEntry::Implementor
{
private:
    Tag_t                           mTag;

public:     ////                    Instantiation.
    virtual                         ~Implementor();
                                    Implementor(Tag_t tag);
    Implementor&                    operator=(Implementor const& other);
                                    Implementor(Implementor const& other);

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
    virtual MVOID                   push_back(_T const& item) { mStorage_##_T.push_back(item); } \
    virtual _T&                     editItemAt(MUINT index, Type2Type<_T> type){ return mStorage_##_T.editItemAt(index); } \
    virtual _T const&               itemAt(MUINT index, Type2Type<_T> type) const { return reinterpret_cast<const _T&>(mStorage_##_T.itemAt(index)); } \
    Vector<_T>                      mStorage_##_T;
    
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

}; 

/******************************************************************************
 * 
 ******************************************************************************/
#define RETURN_TYPE_OPS(_Type, _Ops) \
    _Type == TYPE_MUINT8 ? \
        mStorage_MUINT8._Ops : \
    _Type == TYPE_MINT32 ? \
        mStorage_MINT32._Ops : \
    _Type == TYPE_MFLOAT ? \
        mStorage_MFLOAT._Ops : \
    _Type == TYPE_MINT64 ? \
        mStorage_MINT64._Ops : \
    _Type == TYPE_MDOUBLE ? \
        mStorage_MDOUBLE._Ops : \
    _Type == TYPE_MRational ? \
        mStorage_MRational._Ops : \
    _Type == TYPE_MPoint ? \
        mStorage_MPoint._Ops : \
    _Type == TYPE_MSize ? \
        mStorage_MSize._Ops : \
    _Type == TYPE_MRect ? \
        mStorage_MRect._Ops : \
        mStorage_IMetadata._Ops; \
               
#define SET_TYPE_OPS(_Type, _Ops, _Val) \
    _Type == TYPE_MUINT8 ? \
        mStorage_MUINT8._Ops(_Val) : \
    _Type == TYPE_MINT32 ? \
        mStorage_MINT32._Ops(_Val) : \
    _Type == TYPE_MFLOAT ? \
        mStorage_MFLOAT._Ops(_Val) : \
    _Type == TYPE_MINT64 ? \
        mStorage_MINT64._Ops(_Val) : \
    _Type == TYPE_MDOUBLE ? \
        mStorage_MDOUBLE._Ops(_Val) : \
    _Type == TYPE_MRational ? \
        mStorage_MRational._Ops(_Val) : \
    _Type == TYPE_MPoint ? \
        mStorage_MPoint._Ops(_Val) : \
    _Type == TYPE_MSize ? \
        mStorage_MSize._Ops(_Val) : \
    _Type == TYPE_MRect ? \
        mStorage_MRect._Ops(_Val) : \
        mStorage_IMetadata._Ops(_Val); 

#define SRC_DST_OPERATOR(_Type, _Ops, _Src) \
    if(_Type == TYPE_MUINT8) \
        (mStorage_MUINT8 _Ops _Src.mStorage_MUINT8); \
    else if(_Type == TYPE_MINT32) \
        (mStorage_MINT32 _Ops _Src.mStorage_MINT32); \
    else if(_Type == TYPE_MFLOAT) \
        (mStorage_MFLOAT _Ops _Src.mStorage_MFLOAT); \
    else if(_Type == TYPE_MINT64) \
        (mStorage_MINT64 _Ops _Src.mStorage_MINT64); \
    else if(_Type == TYPE_MDOUBLE) \
        (mStorage_MDOUBLE _Ops _Src.mStorage_MDOUBLE); \
    else if(_Type == TYPE_MRational) \
        (mStorage_MRational _Ops _Src.mStorage_MRational); \
    else if(_Type == TYPE_MPoint) \
        (mStorage_MPoint _Ops _Src.mStorage_MPoint); \
    else if(_Type == TYPE_MSize) \
        (mStorage_MSize _Ops _Src.mStorage_MSize); \
    else if(_Type == TYPE_MRect) \
        (mStorage_MRect _Ops _Src.mStorage_MRect); \
    else if(_Type == TYPE_IMetadata) \
        (mStorage_IMetadata _Ops _Src.mStorage_IMetadata);
/******************************************************************************
 * 
 ******************************************************************************/
IMetadata::IEntry::Implementor::
Implementor(Tag_t tag)
    : mTag(tag)
#define STORAGE_DECLARATION(_T) \
    , mStorage_##_T()
    
    STORAGE_DECLARATION(MUINT8)
    STORAGE_DECLARATION(MINT32)
    STORAGE_DECLARATION(MFLOAT)
    STORAGE_DECLARATION(MINT64)
    STORAGE_DECLARATION(MDOUBLE)
    STORAGE_DECLARATION(MRational)
    STORAGE_DECLARATION(MPoint)
    STORAGE_DECLARATION(MSize)
    STORAGE_DECLARATION(MRect)
    STORAGE_DECLARATION(IMetadata)

#undef STORAGE_DECLARATION
{
}


IMetadata::IEntry::Implementor::
Implementor(IMetadata::IEntry::Implementor const& other)
    : mTag(other.mTag)
#define STORAGE_DECLARATION(_T) \
    , mStorage_##_T(other.mStorage_##_T)

    STORAGE_DECLARATION(MUINT8)
    STORAGE_DECLARATION(MINT32)
    STORAGE_DECLARATION(MFLOAT)
    STORAGE_DECLARATION(MINT64)
    STORAGE_DECLARATION(MDOUBLE)
    STORAGE_DECLARATION(MRational)
    STORAGE_DECLARATION(MPoint)
    STORAGE_DECLARATION(MSize)
    STORAGE_DECLARATION(MRect)
    STORAGE_DECLARATION(IMetadata)
    
#undef STORAGE_DECLARATION
{
}


IMetadata::IEntry::Implementor&
IMetadata::IEntry::Implementor::
operator=(IMetadata::IEntry::Implementor const& other)
{
    if (this != &other)
    {
        mTag = other.mTag;
        SRC_DST_OPERATOR(get_mtk_metadata_tag_type(mTag), =, other);
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }
    
    return *this;
}


IMetadata::IEntry::Implementor::
~Implementor()
{
}


MUINT32                 
IMetadata::IEntry::Implementor::
tag() const
{
    return mTag;
}


MBOOL
IMetadata::IEntry::Implementor::
isEmpty() const
{
    return RETURN_TYPE_OPS(get_mtk_metadata_tag_type(mTag), isEmpty());
}


MUINT                   
IMetadata::IEntry::Implementor::
count() const
{
    return RETURN_TYPE_OPS(get_mtk_metadata_tag_type(mTag), size());
}

 
MUINT
IMetadata::IEntry::Implementor::
capacity() const
{
    return RETURN_TYPE_OPS(get_mtk_metadata_tag_type(mTag), capacity());
}
 

MBOOL
IMetadata::IEntry::Implementor::
setCapacity(MUINT size)
{
    MERROR ret = SET_TYPE_OPS(get_mtk_metadata_tag_type(mTag), setCapacity, size);
    return ret == NO_MEMORY ? MFALSE : MTRUE;
}


MVOID
IMetadata::IEntry::Implementor::
clear()
{
    RETURN_TYPE_OPS(get_mtk_metadata_tag_type(mTag), clear());
}


MERROR
IMetadata::IEntry::Implementor::
removeAt(MUINT index)
{
    MERROR ret = SET_TYPE_OPS(get_mtk_metadata_tag_type(mTag), removeAt, index);
    return ret == BAD_VALUE ? BAD_VALUE : OK;
}


#undef RETURN_TYPE_OPS
#undef SET_TYPE_OPS
#undef SRC_DST_OPERATOR
/******************************************************************************
 * 
 ******************************************************************************/
#define AEE_IF_TAG_ERROR(_TAG_) \
    if (_TAG_ == (uint32_t)-1 || get_mtk_metadata_tag_type(_TAG_) == -1) \
    { \
        CAM_LOGE("tag(%d) error", _TAG_); \
        AEE_ASSERT("tag error"); \
    } 

IMetadata::IEntry::
IEntry(Tag_t tag)
    : mpImp(new Implementor(tag))
{
}


IMetadata::IEntry::
IEntry(IMetadata::IEntry const& other)
    : mpImp(new Implementor(*(other.mpImp))) 
{
}


IMetadata::IEntry::
~IEntry() 
{
     if(mpImp) delete mpImp;

}


IMetadata::IEntry& 
IMetadata::IEntry::
operator=(IMetadata::IEntry const& other) 
{ 
    if (this != &other) {
        delete mpImp;
        mpImp = new Implementor(*(other.mpImp));
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


MUINT32
IMetadata::IEntry::
tag() const 
{ 
    return mpImp->tag(); 
}


MBOOL 
IMetadata::IEntry::
isEmpty() const 
{
    AEE_IF_TAG_ERROR(tag())
    return mpImp->isEmpty(); 
}


MUINT
IMetadata::IEntry::    
count() const 
{ 
    //AEE_IF_TAG_ERROR(tag())
    return mpImp->count(); 
}


MUINT
IMetadata::IEntry::    
capacity() const 
{
    AEE_IF_TAG_ERROR(tag())
    return mpImp->capacity(); 
}


MBOOL
IMetadata::IEntry::    
setCapacity(MUINT size) 
{ 
    AEE_IF_TAG_ERROR(tag())
    return mpImp->setCapacity(size); 
}


MVOID
IMetadata::IEntry:: 
clear() 
{ 
    AEE_IF_TAG_ERROR(tag())
    mpImp->clear(); 
}


MERROR
IMetadata::IEntry:: 
removeAt(MUINT index) 
{
    AEE_IF_TAG_ERROR(tag())
    return mpImp->removeAt(index); 
} 


#define ASSERT_CHECK(_defaultT, _T) \
      CAM_LOGE_IF( TYPE_##_T != _defaultT, "tag(%x), type(%d) should be (%d)", tag(), TYPE_##_T, _defaultT); \
      if (TYPE_##_T != _defaultT) { \
          Utils::dumpCallStack(); \
          AEE_ASSERT("type mismatch"); \
      } 

#define IMETADATA_IENTRY_OPS_DECLARATION(_T) \
MVOID \
IMetadata::IEntry:: \
push_back(_T const& item, Type2Type<_T> type) \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    ASSERT_CHECK(get_mtk_metadata_tag_type(tag()), _T) \
    mpImp->push_back(item); \
} \
_T& \
IMetadata::IEntry:: \
editItemAt(MUINT index, Type2Type<_T> type) \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    return mpImp->editItemAt(index, type); \
} \
_T const& \
IMetadata::IEntry:: \
itemAt(MUINT index, Type2Type<_T> type) const \
{ \
    AEE_IF_TAG_ERROR(tag()) \
    return mpImp->itemAt(index, type); \
} 

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
#undef  ASSERT_CHECK
#undef  AEE_IF_TAG_ERROR
/******************************************************************************
 *
 ******************************************************************************/
class IMetadata::Implementor
{
public:     ////                        Instantiation.
    virtual                            ~Implementor();
                                        Implementor();
    Implementor&                        operator=(Implementor const& other);
                                        Implementor(Implementor const& other);

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
     */
    virtual IEntry&                     editEntryAt(MUINT index); 

    /**
     * Get metadata entry by index, with no editing.
     */
    virtual IEntry const&               entryAt(MUINT index) const; 

    
protected: 
    DefaultKeyedVector<Tag_t, IEntry>   mMap;    
};


/******************************************************************************
 *
 ******************************************************************************/
IMetadata::Implementor::
Implementor()
    : mMap()
{

}


IMetadata::Implementor::
Implementor(IMetadata::Implementor const& other)
    : mMap(other.mMap)
{
}


IMetadata::Implementor::
~Implementor()
{

}


IMetadata::Implementor&
IMetadata::Implementor::
operator=(IMetadata::Implementor const& other)
{
    if (this != &other)
    {
        //release mMap'storage
        //assign other.mMap's storage pointer to mMap
        //add 1 to storage's sharebuffer
        mMap = other.mMap;
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }
    
    return *this;
}


MBOOL
IMetadata::Implementor::
isEmpty() const
{
   
return mMap.isEmpty();
}


MUINT
IMetadata::Implementor::
count() const
{
    return mMap.size();
}


MVOID         
IMetadata::Implementor::
clear()
{

    mMap.clear();
}


MERROR
IMetadata::Implementor::
remove(Tag_t tag)
{

    return mMap.removeItem(tag);
}


MERROR
IMetadata::Implementor::
sort()
{
  
    //keyedVector always sorted.
    return OK;
}


MERROR
IMetadata::Implementor::
update(Tag_t tag, IEntry const& entry)
{
    return mMap.add(tag, entry);

}


IMetadata::IEntry&
IMetadata::Implementor::
editEntryFor(Tag_t tag)
{
    return mMap.editValueFor(tag);
}


IMetadata::IEntry const&
IMetadata::Implementor::
entryFor(Tag_t tag) const
{
    return mMap.valueFor(tag);
}


IMetadata::IEntry&
IMetadata::Implementor::
editEntryAt(MUINT index) 
{
    return mMap.editValueAt(index);

}


IMetadata::IEntry const&
IMetadata::Implementor::
entryAt(MUINT index) const
{
    return mMap.valueAt(index);

}

/******************************************************************************
 * 
 ******************************************************************************/
IMetadata::
IMetadata() 
    : mpImp(new Implementor())
{

}


IMetadata::IMetadata(IMetadata const& other)
    : mpImp(new Implementor(*(other.mpImp)))
{ 
}


IMetadata::
~IMetadata() 
{
     if(mpImp) delete mpImp;

}


IMetadata&
IMetadata::operator=(IMetadata const& other)
{
    if (this != &other) {
        delete mpImp;
        mpImp = new Implementor(*(other.mpImp));
    }
    else {
        CAM_LOGW("this(%p) == other(%p)", this, &other);
    }

    return *this;
}


MBOOL
IMetadata::
isEmpty() const
{
   
return mpImp->isEmpty();
}


MUINT
IMetadata::
count() const
{
    return mpImp->count();

}


MVOID         
IMetadata::
clear()
{

    mpImp->clear();
}


MERROR
IMetadata::
remove(Tag_t tag)
{
    return mpImp->remove(tag) >= 0 ? OK : BAD_VALUE;
}


MERROR
IMetadata::
sort()
{
  
    return mpImp->sort();
}


MERROR
IMetadata::
update(Tag_t tag, IMetadata::IEntry const& entry)
{
    MERROR ret = mpImp->update(tag, entry);  //keyedVector has two possibilities: BAD_VALUE/NO_MEMORY
    return ret >= 0 ? (MERROR)OK : (MERROR)ret;
}


IMetadata::IEntry&
IMetadata::
editEntryFor(Tag_t tag)
{
    return mpImp->editEntryFor(tag);
}


IMetadata::IEntry const&
IMetadata::
entryFor(Tag_t tag) const
{
    return mpImp->entryFor(tag);
}

IMetadata::IEntry&
IMetadata::
editEntryAt(MUINT index) 
{
    return mpImp->editEntryAt(index);

}


IMetadata::IEntry const&
IMetadata::
entryAt(MUINT index) const
{
    return mpImp->entryAt(index);

}


