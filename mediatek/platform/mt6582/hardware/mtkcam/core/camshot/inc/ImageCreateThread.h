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

#ifndef _MTK_HAL_MTKATVCAMADAPTER_IMAGECREATETHREAD_H_
#define _MTK_HAL_MTKATVCAMADAPTER_IMAGECREATETHREAD_H_
//
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <mtkcam/common.h>
using namespace android;

//
//
namespace NSCamShot {

/*******************************************************************************
*	Command
*******************************************************************************/
struct Command
{
	//	Command ID.
	enum EID
	{
		eID_EXIT, 
		eID_WAKEUP, 
		eID_YUV_BUF,
		eID_POSTVIEW_BUF, 
	};
	//
	//	Operations.
	Command(EID const _eId = eID_EXIT)
		: eId(_eId)
	{}
	//
	static	char const* getCmdName(EID const _eId)
	{
	
#define CMD_NAME(x) case x: return #x
		switch	(_eId)
		{
		CMD_NAME(eID_EXIT);
		CMD_NAME(eID_WAKEUP);
		CMD_NAME(eID_YUV_BUF);
		CMD_NAME(eID_POSTVIEW_BUF);
		default:
			break;
		}
#undef  CMD_NAME
		return	"";
	}
	inline	char const* name() const	{ return getCmdName(eId); }
	//
	//	Data Members.
	EID 	eId;
};

/*******************************************************************************
* ImageType
*******************************************************************************/
typedef enum
{
	IMAGE_CREATE,
	YUV_IMAGE_CREATE,
	THUMBNAIL_IMAGE_CREATE,
	JPEG_IMAGE_CREATE,
}IMAGE_TYPE;
	
//
/*******************************************************************************
*   IImageCreateThreadHandler
*******************************************************************************/
class IImageCreateThreadHandler : public virtual RefBase
{
public:     ////        Instantiation.
    virtual             ~IImageCreateThreadHandler() {}

public:     ////        Interfaces
    virtual MBOOL        onThreadLoop(IMAGE_TYPE imgType)
    {
    	MBOOL ret = MFALSE;
		switch	(imgType)
		{
		case IMAGE_CREATE: 
			ret = onCreateImage();
			break;
		case YUV_IMAGE_CREATE: 
			ret = onCreateYuvImage();
			break;
		case THUMBNAIL_IMAGE_CREATE:
			ret = onCreateThumbnailImage();
			break;
		case JPEG_IMAGE_CREATE:
			ret = onCreateJpegImage();
			break;
		default:
			break;
		}
		return	ret;
	}
    virtual MBOOL        onCreateImage()  					= 	0;
    virtual MBOOL        onCreateYuvImage()           		= 	0;
    virtual MBOOL        onCreateThumbnailImage()    	= 	0;
	virtual MBOOL		 onCreateJpegImage()		   	= 	0;

};

/*******************************************************************************
*   IImageCreateThread
*******************************************************************************/
class IImageCreateThread : public Thread
{
public:     ////        Instantiation.
    static  IImageCreateThread* createInstance(IMAGE_TYPE imgType, IImageCreateThreadHandler*const pHandler);
    //
public:     ////        Attributes.
    virtual int32_t     getTid() const                              = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Commands.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Interfaces.
    /*
     */
    virtual void        postCommand(Command const& rCmd)            = 0;

};

};  // namespace NSCamShot

#endif  //_MTK_HAL_MTKATVCAMADAPTER_IMAGECREATETHREAD_H_
