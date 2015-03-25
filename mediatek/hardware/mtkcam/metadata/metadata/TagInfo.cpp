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

#include <utils/String8.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/KeyedVector.h>
using namespace android;
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
//


/******************************************************************************
 *  Type Info Utility
 ******************************************************************************/
namespace
{
    typedef mtk_camera_metadata_tag Tag_t;

    template <typename _T> struct Type2TypeEnum{};
    template <> struct Type2TypeEnum<MUINT8   >{ enum { typeEnum = TYPE_MUINT8 };   };
    template <> struct Type2TypeEnum<MINT32   >{ enum { typeEnum = TYPE_MINT32 };   };
    template <> struct Type2TypeEnum<MFLOAT   >{ enum { typeEnum = TYPE_MFLOAT };   };
    template <> struct Type2TypeEnum<MINT64   >{ enum { typeEnum = TYPE_MINT64 };   };
    template <> struct Type2TypeEnum<MDOUBLE  >{ enum { typeEnum = TYPE_MDOUBLE };  };
    template <> struct Type2TypeEnum<MRational>{ enum { typeEnum = TYPE_MRational };};
    template <> struct Type2TypeEnum<MPoint   >{ enum { typeEnum = TYPE_MPoint };   };
    template <> struct Type2TypeEnum<MSize    >{ enum { typeEnum = TYPE_MSize };    };
    template <> struct Type2TypeEnum<MRect    >{ enum { typeEnum = TYPE_MRect };    };
    template <> struct Type2TypeEnum<IMetadata>{ enum { typeEnum = TYPE_IMetadata };};

    template <Tag_t tag>
    struct Tag2TypeInfo
    {
        typedef typename MetadataTagInfo<tag>::type type;
        enum { typeEnum = Type2TypeEnum<type>::typeEnum };
    };

};


/******************************************************************************
 *  Type Info Definition
 ******************************************************************************/
namespace
{
    struct TagInfoBase : public RefBase
    {
        Tag_t                   mTag;
        String8                 mName;
        MINT32                  mTypeEnum;

                                TagInfoBase(
                                    Tag_t const tag, 
                                    char const* name, 
                                    MINT32 const typeEnum
                                )
                                 : mTag(tag)
                                 , mName(name)
                                 , mTypeEnum(typeEnum)
                                {
                                }

        Tag_t                   tag() const         { return mTag; }
        char const*             name() const        { return mName.string(); }
        MINT32                  typeEnum() const    { return mTypeEnum; }
    };
};


/******************************************************************************
 *  Type Info Map
 ******************************************************************************/
namespace
{
    struct TagInfoMap : public DefaultKeyedVector<MUINT32, sp<TagInfoBase> >
    {
        TagInfoMap()
        {
#define _IMP_SECTION_INFO_(...)
#undef  _IMP_TAG_INFO_
#define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
            add(_tag_, new TagInfoBase(_tag_, _name_, Tag2TypeInfo<_tag_>::typeEnum));

            #include <mtkcam/metadata/mtk_metadata_tag_info.inl>

#undef  _IMP_TAG_INFO_
        }
    } gTagInfoMap;
};


/******************************************************************************
 *  
 ******************************************************************************/
int get_mtk_metadata_tag_type(unsigned int tag)
{
    sp<TagInfoBase> p = gTagInfoMap.valueFor(tag);
    if  ( p != 0 )
    {
        return  p->typeEnum();
    }
    //
    return  -1;
}


/******************************************************************************
 *  
 ******************************************************************************/
char const* get_mtk_metadata_tag_name(unsigned int tag)
{
    sp<TagInfoBase> p = gTagInfoMap.valueFor(tag);
    if  ( p != 0 )
    {
        return  p->name();
    }
    //
    return  NULL;
}


/******************************************************************************
 *  
 ******************************************************************************/
namespace NSCam {
void
dumpMetadataEntry(IMetadata::IEntry const& entry, android::String8& rs8Log)
{
    int const typeEnum = get_mtk_metadata_tag_type(entry.tag());

    rs8Log += String8::format("TAG <%s> ", get_mtk_metadata_tag_name(entry.tag()));
    for (MUINT i = 0; i < entry.count(); i++)
    {
        switch  ( typeEnum )
        {
        case TYPE_MUINT8:{
            MUINT8 const& v = entry.itemAt(i, Type2Type<MUINT8>());
            rs8Log += String8::format("%d ", v);
            }break;
        case TYPE_MINT32:{
            MINT32 const& v = entry.itemAt(i, Type2Type<MINT32>());
            rs8Log += String8::format("%d ", v);
            }break;
        case TYPE_MFLOAT:{
            MFLOAT const& v = entry.itemAt(i, Type2Type<MFLOAT>());
            rs8Log += String8::format("%f ", v);
            }break;
        case TYPE_MINT64:{
            MINT64 const& v = entry.itemAt(i, Type2Type<MINT64>());
            rs8Log += String8::format("%lld ", v);
            }break;
        case TYPE_MDOUBLE:{
            MDOUBLE const& v = entry.itemAt(i, Type2Type<MDOUBLE>());
            rs8Log += String8::format("%lf ", v);
            }break;
        case TYPE_MRational:{
            MRational const& v = entry.itemAt(i, Type2Type<MRational>());
            rs8Log += String8::format("%d/%d ", v.numerator, v.denominator);
            }break;
        case TYPE_MPoint:{
            MPoint const& v = entry.itemAt(i, Type2Type<MPoint>());
            rs8Log += String8::format("(%d %d)", v.x, v.y);
            }break;
        case TYPE_MSize:{
            MSize const& v = entry.itemAt(i, Type2Type<MSize>());
            rs8Log += String8::format("(%d %d)", v.w, v.h);
            }break;
        case TYPE_MRect:{
            MRect const& v = entry.itemAt(i, Type2Type<MRect>());
            rs8Log += String8::format("(%d %d %d %d)", v.p.x, v.p.y, v.s.w, v.s.h);
            }break;
        case TYPE_IMetadata:{
            rs8Log += String8("\n");
            IMetadata const& v = entry.itemAt(i, Type2Type<IMetadata>());
            dumpMetadata(v, rs8Log);
            }break;
        default:
            break;
        }
    }
}
}


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
void
dumpMetadata(IMetadata const& metadata, android::String8& rs8Log)
{
    rs8Log += String8::format("<IMetadata> count:%d \n", metadata.count());
    for (MUINT i = 0; i < metadata.count(); i++)
    {
        IMetadata::IEntry const& entry = metadata.entryAt(i);
        dumpMetadataEntry(entry, rs8Log);
        rs8Log += String8("\n");
    }
}
}

