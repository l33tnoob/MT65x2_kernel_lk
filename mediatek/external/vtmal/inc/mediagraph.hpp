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

/******************************************************
*created by mingliangzhong(mtk80309)@2010-08-31
*modify history:
*
******************************************************/

#ifndef _VT_MEDIA_GRAPH_H_
#define _VT_MEDIA_GRAPH_H_

#include "mediaobject.hpp"

namespace videotelephone
{
template <typename XXX>
class TMediaEvent : public IMediaEvent
{
public:
	TMediaEvent(XXX* pdelegation):IMediaEvent(),m_pdelegation(pdelegation)
	{
		
	}
	virtual ~TMediaEvent()
	{
		//LOGD(	
	}
	
	void SetEvent(int msg, int params1,int params2)
	{
		if(m_pdelegation)
			 m_pdelegation->HandleSetEvent(msg, params1,params2);
	}
		
		
private:
		XXX* m_pdelegation;
};

class CMediaGraph:public IMediaGraph
{
public:
	CMediaGraph();

	virtual ~CMediaGraph();	
	
	virtual int Connect(const sp<IMediaObject>& pUpObject,
		const sp<IMediaObject>& pDownObject, MetaData*pMediaType = 0);

	virtual void Disconnect();
	
	virtual int Start();
	
	virtual int Pause();
	
	virtual int Stop();

	virtual int Reset();

	void HandleSetEvent(int msg, int params1,int params2);

	virtual void SetEventObserver(const wp<IMediaEventObserver>& pObserver);

private:

	int AddMediaObject(const sp<IMediaObject>&  pObject);
	
	inline int _addMediaObject(Vector<sp<IMediaObject> >& list,const sp<IMediaObject>&  pObject);

	mutable LockEx m_Lock;
	
	Vector<sp<IMediaObject> > m_Sources;
	
	Vector<sp<IMediaObject> > m_Transforms;
	
	Vector<sp<IMediaObject> > m_Sinks;	

	sp<IMediaEvent> m_EventSet;
		
 	wp<IMediaEventObserver> m_MediaEventObserver; 

	sp<IRefClock> m_clock;

};

//help functions!!!!
int  FindMatchMetaData(IMediaObject* p,IMediaObject* pNext,sp<MetaData>& findData );

}
#endif //_VT_MEDIA_GRAPH_H_
