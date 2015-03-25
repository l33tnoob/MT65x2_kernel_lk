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
*created by mingliangzhong(mtk80309)@2010-08-30
*modify history:
*
******************************************************/

#ifndef _MEDIA_BUFFER_MANAGEMENT_H_
#define _MEDIA_BUFFER_MANAGEMENT_H_
#include "minterface.hpp"
#ifndef DISABLE_PMEMORY_MODE
#include <binder/MemoryHeapPmem.h>
#endif
#include <binder/MemoryHeapBase.h>
#include <ui/GraphicBuffer.h>
//#include <ui/Overlay.h>
//#include <gui/ISurface.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/android_pmem.h>

//#undef LOG_TAG
//#define LOG_TAG "VTMAL@BufferMgr"
#pragma GCC diagnostic ignored "-Wpointer-arith"
#pragma GCC diagnostic ignored "-Wformat"
namespace videotelephone
{

enum MediaMemory_Type{
	MMT_PMEMORY,
	MMT_ASHMEMORY
	};

#define VT_PMEM_DEVICE_NAME "/dev/pmem_multimedia"

template<int memorytype>
class MediaMemoryAllocator:public IMediaBufferAllocator
{
public:
	
	// the samplesize is calculate as bytes.
	MediaMemoryAllocator(const int samplenum ,const int samplesize)
		:IMediaBufferAllocator()
		,m_samplenums(samplenum)
		,m_samplesize(samplesize)
	{
		AutolockEx _l(m_lock);
		int heapsize = samplenum*samplesize; //bytes
		if(heapsize)
		{
			if(memorytype == MMT_PMEMORY)
			{
#ifndef DISABLE_PMEMORY_MODE
				int alignSize, pageSize;
				pageSize = getpagesize();
				alignSize = ((heapsize + pageSize - 1) / pageSize) * pageSize;
				
				sp<MemoryHeapBase> p = new MemoryHeapBase(VT_PMEM_DEVICE_NAME,alignSize,MemoryHeapBase::NO_CACHING);
				sp<MemoryHeapPmem> pmemHeap  =  new MemoryHeapPmem(p, MemoryHeapBase::NO_CACHING);
				if (pmemHeap->getHeapID() < 0) 
				{
					VTMAL_LOGERR;	
				}else
				{
					pmemHeap->slap();   // Call pmem's remap, create region list
					m_memoryheap = pmemHeap;
				}			
#if 0
					struct pmem_region {
						unsigned long offset;
						unsigned long len;
					};
#endif //
					pmem_region pmem_reg;
					int err = ioctl(pmemHeap->getHeapID(), PMEM_GET_PHYS, &pmem_reg);
					if (-1 == err){
					       LOGE("VT PMEM_GET_PHYS failed !");
					}else{
						 LOGE("VT PMEM_PHYS: %x!",pmem_reg.offset);
					}
#else
                    LOGE("PMEM_PHYS not implement!");
#endif
				
			}
			else
			{
				//m_memoryheap = new MemoryHeapBase(heapsize, MemoryHeapBase::READ_ONLY, "AshMem");
				m_memoryheap = new MemoryHeapBase(heapsize, 0, "AshMem");  //change by mtk80691 07-14 for cant't pass the check of m4u
				
			}
		}


		//add to m_availablememory list;
		for(int i = 0; i < samplenum; i++)
		{
			sp<IMemory> p(new MemoryBase(m_memoryheap,i*samplesize,samplesize));
			m_availablememory.push_back(p);
		}
		VTMAL_LOGDEBUG;	
	  
	}

     virtual ~MediaMemoryAllocator()
	{
		VTMAL_LOGDEBUG;	
		m_memoryheap.clear();
		m_availablememory.clear();
	}

     virtual int  GetMediaBuffer(sp<IMediaBuffer>& Sample);


    virtual  int FreeMediaBuffer(IMediaBuffer* pSample)
	{
		if(pSample)
		{
			 if(((pSample->GetMemory())->getMemory()) == m_memoryheap)
			 {
				AutolockEx _l(m_lock);
				m_availablememory.push_back(pSample->GetMemory());
				return OK;
			 }
		}
		VTMAL_LOGDEBUG;	
		return NO_MEMORY;	
	}

	virtual sp<IMemoryHeap> GetMemoryHeap() const
	{
		return m_memoryheap;
	}
	
	//add by mtk80691 for "hide you"  feature, Hide one memory for hide you picture which can't be seen by allocator
	virtual sp<IMemory> HideOneMemoryForSpecUse()
	{
		if(m_availablememory.size())
		{
			sp<IMemory> hide_mem((m_availablememory.begin())->get());
			m_availablememory.erase(m_availablememory.begin());
			return hide_mem;
		}
		else
		{
			VTMAL_LOGERR;
			return NULL;
		}
		
	}
	
	 virtual List< sp<IMemory> > GetMemoryPoolList() const
	{
		return m_availablememory;
	}
	

private:
	
	sp<IMemoryHeap> m_memoryheap;

	int m_samplenums;
	
	int m_samplesize;
	
	List< sp<IMemory> > m_availablememory;
	
	mutable LockEx       m_lock;
};

#ifdef DISABLE_PMEMORY_MODE
	#define CPMemMediaMemoryAllocator MediaMemoryAllocator<MMT_ASHMEMORY> 
#else
        #define CPMemMediaMemoryAllocator MediaMemoryAllocator<MMT_PMEMORY> 
#endif 
#define CAshmMediaMemoryAllocator MediaMemoryAllocator<MMT_ASHMEMORY> 

class CMediaBufferAllocator:public IMediaBufferAllocator
{
public:	
	CMediaBufferAllocator(const int samplenum ,const int samplesize);

	virtual ~CMediaBufferAllocator();

	virtual int  GetMediaBuffer(sp<IMediaBuffer>& Sample);
	
	virtual int FreeMediaBuffer(IMediaBuffer* pSample);

	virtual sp<IMemoryHeap> GetMemoryHeap() const;	
	virtual sp<IMemory> HideOneMemoryForSpecUse() {return NULL;}

private:
	void* m_pBufferBase;
	int m_buffersize;
	sp<IMemoryHeap> m_fack_heep;
	List< void* > m_availablememory;
	int m_samplenums;	
	int m_samplesize;
	mutable LockEx       m_lock;
};


class CMediaBuffer:public IMediaBuffer
{
public:	

	virtual void* pointer() const;

	virtual size_t size() const;

	virtual int SetRealSize(size_t size);

	virtual sp<IMemory>& GetMemory() ;

	virtual sp<MetaData>& GetMetaData();

	virtual ~CMediaBuffer();
	

	//for Audio Sink 
	virtual size_t range_offset() const;

	//for Audio Sink 
	virtual size_t range_length() const;

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length);
	
private:

	CMediaBuffer(const sp<IMediaBufferAllocator>& pAllocator,
		const sp<IMemory>& pbuffer);
	
	CMediaBuffer(const sp<IMediaBufferAllocator>& pAllocator,
		void* pBuffer,size_t size);
	
	 
	sp<IMemory> m_bufferdata; 
	sp<IMediaBufferAllocator> m_Allocator;
	sp<MetaData> m_metadata;
	 void* m_pBuffer;
	 


	 size_t m_realsize;
	 
	 size_t m_offset;
	 size_t m_length;
#ifndef DISABLE_PMEMORY_MODE	 
	 friend class CPMemMediaMemoryAllocator;
#endif //DISABLE_PMEMORY_MODE	 
	 friend class CAshmMediaMemoryAllocator;
		 
	 friend class CMediaBufferAllocator;
		 
};

template<int memorytype>
int  MediaMemoryAllocator<memorytype>::GetMediaBuffer(sp<IMediaBuffer>& Sample)
{
	AutolockEx _l(m_lock);
	if(m_availablememory.size())
	{
		Sample.clear();
		sp<IMediaBufferAllocator> p(this);
		sp<IMemory> pb((m_availablememory.begin())->get());
		Sample = new CMediaBuffer(p,pb);
		m_availablememory.erase(m_availablememory.begin());
		return OK;
	}
	
	LOGI("[VTMAL]MediaMemoryAllocator<%d> GetMediaBuffer fail,m_samplenums=%d,m_samplesize=%d",\
		memorytype,m_samplenums,m_samplesize);
	 return NO_MEMORY;

}

template<typename T>
class spQueue
{
public:
	spQueue()
	{
	}
	
	~spQueue()
	{
		Reset();
	}


	int Add(const sp<T>& p)
	{
		AutolockEx _l(m_lock);
		m_Queue.push_back(p);
		return m_Queue.size();
	}

	sp<T> Remove()
	{
		AutolockEx _l(m_lock);
		if(m_Queue.size())
		{
			sp<T>  p((m_Queue.begin())->get());
			m_Queue.erase(m_Queue.begin());
			return p;
		}

		return 0;
	}

	int size()
	{
		AutolockEx _l(m_lock);
		return m_Queue.size();
	}

	void Reset()
	{
		AutolockEx _l(m_lock);
		m_Queue.clear();
	}
private:
	LockEx       m_lock;
	List<sp<T> > m_Queue;	
};

class CAshmMediaBuffer:public IMediaBuffer
{
public:	
	CAshmMediaBuffer(int size)
		:IMediaBuffer(),m_metadata(new MetaData)
	{
		const sp<IMemoryHeap>& heap =  new MemoryHeapBase(size, MemoryHeapBase::READ_ONLY, "AshMem");
		m_buffer = new MemoryBase(heap,0,size);
		m_realsize = m_buffer->size();
		m_offset = 0;
		m_length = 0;
	}

	CAshmMediaBuffer(const sp<IMemoryHeap>& heap, int size)
		:IMediaBuffer(),m_metadata(new MetaData)
	{
		m_buffer = new MemoryBase(heap,0,size);
		m_realsize = m_buffer->size();
		m_offset = 0;
		m_length = 0;
	}

	virtual ~CAshmMediaBuffer()
	{
		m_buffer.clear();
	}
	
	virtual void* pointer() const 
	{
		if(m_buffer.get())
		return m_buffer->pointer();
		return NULL;
	}

	int SetRealSize(size_t size)
	{
		if(!m_buffer.get()){
			VTMAL_LOGERR;
			 return NO_MEMORY;
		}
		if(size > m_buffer->size())
		{
			VTMAL_LOGERR;
		return INVALID_OPERATION;
	}
	
		m_realsize = size;
		m_length = m_realsize;
		m_offset = 0;
		return OK;
	}
	
	virtual size_t size() const 
	{
		return m_realsize;
	}

	virtual sp<IMemory>& GetMemory() 
	{
		return m_buffer;
	}

	virtual sp<MetaData>& GetMetaData() 
	{
		return m_metadata;
	}


	//for Audio Sink 
	virtual size_t range_offset() const
	{
		return m_offset;
	}
	//for Audio Sink 
	virtual size_t range_length() const
	{
		return m_length;
	}

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length)
	{
		if(offset >= m_realsize)
		{
			VTMAL_LOGDEBUG;
			m_offset = 0;
			m_length = 0;
			return;
		}

		if((length + offset) > m_realsize) 
		{
			VTMAL_LOGDEBUG;
			m_offset = offset;
			m_length = m_realsize - offset;
			return;
		}
	
		m_offset = offset;
		m_length = length;
	}

private:
	
	sp<IMemory>  m_buffer;

	sp<MetaData> m_metadata;	

	size_t m_realsize;
	size_t m_offset;
	size_t m_length;
};

	
#define IMediaBufferQueue spQueue<IMediaBuffer> 

class CSimpleMediaBuffer:public IMediaBuffer
{
public:	
	CSimpleMediaBuffer(int size)
		:IMediaBuffer(),m_metadata(new MetaData),
		m_buffer(NULL),
		m_size(size)
	{
		m_buffer = malloc(size);
		m_realsize = size;
		m_offset = 0;
		m_length = size;
	}

	virtual ~CSimpleMediaBuffer()
	{
		if(m_buffer)
			 free(m_buffer);
		m_buffer = NULL;
	}
	
	virtual void* pointer() const 
	{
		return m_buffer;
	}

	int SetRealSize(size_t size)
	{
		if(size > m_size)
		{
			 return INVALID_OPERATION;
		}

		m_realsize = size;
		m_length = m_realsize;
		m_offset = 0;
		return OK;
	}
	
	virtual size_t size() const 
	{
		return m_realsize;
	}

	virtual sp<IMemory>& GetMemory() 
	{
		return m_fake_memory;
	}

	virtual sp<MetaData>& GetMetaData() 
	{
		return m_metadata;
	}


	//for Audio Sink 
	virtual size_t range_offset() const
	{
		return m_offset;
	}
	//for Audio Sink 
	virtual size_t range_length() const
	{
		return m_length;
	}

	//for Audio Sink 
	virtual void set_range(size_t offset, size_t length)
	{
		if(offset >= m_realsize)
		{
			VTMAL_LOGDEBUG;
			m_offset = 0;
			m_length = 0;
			return;
		}

		if((length + offset) > m_realsize) 
		{
			VTMAL_LOGDEBUG;
			m_offset = offset;
			m_length = m_realsize - offset;
			return;
		}
	
		m_offset = offset;
		m_length = length;
	}

private:
	sp<MetaData> m_metadata;	
	void*  m_buffer;
	size_t m_size;
	sp<IMemory> m_fake_memory;
	size_t m_realsize;

	size_t m_offset;
	size_t m_length;
	
};

#if 0
class SillySurface: public ISurface
{

private:	
	
	sp<ISurface> m_realsurface;
	
	void* m_pBackBaseAddress;
	void* m_pDisBaseAddress;

	//ISurface::BufferHeap m_bufferheap;
	sp<IMemoryHeap> m_backheap;
	bool m_fsurfacereg;
	int m_samplesize;
	
public:
	
	SillySurface(const sp<ISurface>& surface)
	{
		m_fsurfacereg = false;
		m_pDisBaseAddress = NULL;
		m_pBackBaseAddress= NULL;
		m_samplesize = 0;
		m_realsurface = surface;
		m_backheap.clear();
		//m_bufferheap.heap.clear();
		
	}

	virtual ~SillySurface()
	{
		VTMAL_LOGINFO;
		m_realsurface.clear();
		m_backheap.clear();
		//m_bufferheap.heap.clear();
		VTMAL_PRINTINFO;
	}
	
	sp<ISurface> GetRealSurface(){
		 return m_realsurface;
	}
	
    sp<ISurfaceTexture> getSurfaceTexture() const {
        return m_realsurface->getSurfaceTexture();
    }

	virtual IBinder*            onAsBinder()
	{
		VTMAL_LOGERR;
		return NULL;
	}
	
#if 0
#ifdef VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE	
   virtual sp<GraphicBuffer> requestBuffer(int bufferIdx,
        uint32_t w, uint32_t h, uint32_t format, uint32_t usage) 
#else
	virtual sp<GraphicBuffer> requestBuffer(int bufferIdx, int usage)
#endif //	VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE
	{
		if(m_realsurface.get())
		{
#ifdef VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE		
			return m_realsurface->requestBuffer(bufferIdx,w,h,format,usage);
#else
			return m_realsurface->requestBuffer(bufferIdx,usage);
#endif //#ifdef VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE			
		}
		return NULL;
	}

	virtual status_t registerBuffers(const BufferHeap& buffers)
	{
		m_bufferheap.w = buffers.w;
		m_bufferheap.h = buffers.h;
		m_bufferheap.hor_stride = buffers.hor_stride ;
		m_bufferheap.ver_stride = buffers.ver_stride;
		m_bufferheap.format = buffers.format;
		m_bufferheap.transform = buffers.transform;
		m_bufferheap.flags = buffers.flags;

		m_samplesize = ( m_bufferheap.w * m_bufferheap.h *3)/2; //YUV


		m_backheap = buffers.heap;
		int heapid = m_backheap->getHeapID();
		size_t size_ = m_backheap->getSize();
		void* pbase = m_backheap->getBase();
		m_pBackBaseAddress = pbase;

		LOGD("VideoSurface::registerBuffers heap id: %d,heap->pointer() = %x\n",heapid,(unsigned int)pbase);

		//create surfaceheap:
		sp<CPMemMediaMemoryAllocator> palloc = new  CPMemMediaMemoryAllocator(1,size_);
		m_bufferheap.heap = palloc->GetMemoryHeap();
	
		m_pDisBaseAddress =  m_bufferheap.heap->getBase();	

		if(m_realsurface.get())
		{
			m_fsurfacereg = true;
			return m_realsurface->registerBuffers(m_bufferheap);		
		}else
		{
			m_fsurfacereg = false;
		}
		VTMAL_LOGERR;
		return 0;
	
	}

	virtual void postBuffer(ssize_t offset)
	{
		if(m_realsurface.get() && m_fsurfacereg)
		{
			if(m_pDisBaseAddress && m_pBackBaseAddress)
			{
				memcpy(m_pDisBaseAddress+ offset,m_pBackBaseAddress + offset,m_samplesize);
				m_realsurface->postBuffer(offset);	
			}
		}
		else
		{
			VTMAL_LOGERR;
		}
	}
#ifdef VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE
	 virtual status_t setBufferCount(int bufferCount)
	{
		if(m_realsurface.get())
		{
			return m_realsurface->setBufferCount(bufferCount);
		}
		return OK;
	}
#endif //VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE
	virtual void unregisterBuffers()
	{
		m_fsurfacereg = false;
		if(m_realsurface.get())	
		{
			m_realsurface->unregisterBuffers();		
		}
		m_pDisBaseAddress = NULL;
		m_pBackBaseAddress= NULL;
		m_samplesize = 0;
		m_backheap.clear();
		m_bufferheap.heap.clear();
		
	}

	virtual sp<OverlayRef> createOverlay(
	    uint32_t w, uint32_t h, int32_t format, int32_t orientation)
	{
		if(m_realsurface.get())
		{
			return m_realsurface->createOverlay(w,h,format,orientation);
		}
		return NULL;
	}

	virtual status_t setLayerType(uint32_t type, uint32_t para = 0)
	{
#ifdef DEF_ISURFACE_EXTEN_API	
		if(m_realsurface.get())
		{
			return m_realsurface->setLayerType(type, para);
		}
#endif // DEF_ISURFACE_EXTEN_API		
		return 0;
	}

	virtual uint32_t getLayerType()
	{
#ifdef DEF_ISURFACE_EXTEN_API	
		if(m_realsurface.get())
		{
			return m_realsurface->getLayerType();
		}	
#endif //DEF_ISURFACE_EXTEN_API			
		return 0;
	}
	

	// variable position
    virtual void postBuffer(ssize_t offset, int32_t x, int32_t y)
	{
#ifdef DEF_ISURFACE_VARIABLE_POST
		if(m_realsurface.get())
		{
			return m_realsurface->postBuffer(offset,x,y);
		}	
#endif //DEF_ISURFACE_VARIABLE_POST
	}
    // variable scope
    virtual void postBuffer(ssize_t offset, uint32_t w, uint32_t h,uint32_t w_stride, uint32_t h_stride)
    {
 #ifdef DEF_ISURFACE_VARIABLE_POST
		if(m_realsurface.get())
		{
			return m_realsurface->postBuffer(offset,w,h,w_stride,h_stride);
		}	
#endif //DEF_ISURFACE_VARIABLE_POST   	
    	}
#endif
};
#endif
}


#endif //_MEDIA_BUFFER_MANAGEMENT_H_
