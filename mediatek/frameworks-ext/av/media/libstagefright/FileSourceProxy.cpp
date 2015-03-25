
#define LOG_TAG "FileSourceProxy"
#include <media/stagefright/foundation/ADebug.h>
#include <cutils/xlog.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cutils/compiler.h>
#include "FileSourceProxy.h"

// Big-Little file cache
#define FILE_CACHE_SIZE_LITTLE 20*1024    // default little file cache block size: 20k
#define FILE_CACHE_COUNT_LITTLE 5             // default little file cache block count: 5 
#define FILE_CACHE_SIZE_BIG (1*1024*1024)     // default big file cache block size: 1M
#define FILE_CACHE_COUNT_BIG 5                // default big file cache block count: 5
#define FILE_CACHE_BIGLITTLE_THRESHOLD  4096  // default big-little file cache threshold: 4096

#define FILE_CACHE_NUMBER 5

#define USE_FILE_SOURCE_CACHE
//#define USE_CACHE_MMAP

namespace android {

struct FileSourceProxy::ProxyThread : public Thread {
	ProxyThread(FileSourceProxy *proxy, bool canCallJava)
		: Thread(canCallJava),
		  mProxy(proxy),
		  mThreadId(NULL) {
	}

	virtual status_t readyToRun() {
		mThreadId = androidGetThreadId();
		return Thread::readyToRun();
	}

	virtual bool threadLoop() {
		return mProxy->loop();
	}

protected:
	virtual ~ProxyThread() {}

private:
	FileSourceProxy *mProxy;
	android_thread_id_t mThreadId;

    DISALLOW_EVIL_CONSTRUCTORS(ProxyThread);
};

class FileCache : public RefBase
{
public:
	FileCache(int fd, 
		      int64_t offset, 
			  int64_t length,
			  size_t cacheSize=FILE_CACHE_SIZE_LITTLE,
			  size_t cacheCount=FILE_CACHE_COUNT_LITTLE,
			  FileSourceProxy *observer = NULL);
	~FileCache();


	enum CACHE_TYPE {
		LITTLE_CACHE,
		BIG_CACHE	
	};

	ssize_t read(off64_t offset, void *data, size_t size);

	void updateCacheNodeList(CACHE_TYPE ct, off64_t offset);

	void reset() {Mutex::Autolock autoLock(mLock); mFd = -1;}

	float hitRate() {return (mCacheReadCount == 0)? 0.0 : ((float)mCacheHitCount/(float)mCacheReadCount)*100;}

private:

	class CacheNode
	{
	public:
		CacheNode(off64_t offset, size_t size, int64_t totalLength);
		~CacheNode();

		typedef int32_t cache_id;

		cache_id id() const {
			return mId;
		}
     
		bool hit(off64_t offset);

		size_t size() {return mCacheSize;}

		off64_t offset() {return mOffset;}

		void setOffset(off64_t offset) {mOffset = offset; mId = mOffset/mCacheSize;}

		ssize_t fill(int fd);

		bool fillFlag(){return mFilled;}

		ssize_t read(off64_t offset, void *data, size_t ize);

        void release();

		void clear();
		
	private:
		Mutex mCacheNodeLock;
		cache_id mId;
		off64_t mOffset;
		int64_t mPos;
		size_t  mCacheSize;
		ssize_t  mDataLen;
		int64_t  mTotalLength;
		void *mData;
		bool mFilled;
	};
    Mutex mLock;
	Mutex mCacheLock;
	Mutex mFileLock;
    int mFd;
	int mDupFd;
	int64_t mOffset;
	int64_t mLength;
	size_t mCacheSize;
	size_t mCacheCount;
	int64_t mCacheOffset;
	size_t mCacheTotalLen;

	FileSourceProxy *mObserver;
	
	List<CacheNode *> mLittleCacheNodes;
	List<CacheNode *> mBigCacheNodes;

	uint32_t mCacheReadCount;
	uint32_t mCacheHitCount;
	uint32_t mCacheMissCount;

	bool hit(List<CacheNode *> *cn, off64_t offset);

	bool hit(off64_t offset) {return (mCacheOffset <= offset && offset < mCacheOffset + mCacheTotalLen);}

    void triggerUpdate(CACHE_TYPE ct, off64_t offset);

    void clearCacheQueue(List<CacheNode *> *cn);

    int64_t getNowMs(); 

    void dumpFileCacheInfo();

    DISALLOW_EVIL_CONSTRUCTORS(FileCache);
};

FileCache::CacheNode::CacheNode(off64_t offset, size_t size, int64_t totalLength) 
    : mOffset(offset),
	  mId(offset/size),
	  mPos(0ll),
	  mCacheSize(size),
	  mDataLen(-1),
	  mTotalLength(totalLength),
	  mData(NULL),
	  mFilled(false) {
#ifndef USE_CACHE_MMAP
	mData = (void*)new char[mCacheSize];
#endif
}

FileCache::CacheNode::~CacheNode() {
#ifdef USE_CACHE_MMAP	
	if (mData != MAP_FAILED) munmap(mData, mDataLen);
#else
	delete [] mData;
#endif
    mData = NULL;
}

bool FileCache::CacheNode::hit(off64_t offset) {
	if (mDataLen > 0 && mOffset <= offset && offset < mOffset + mDataLen){
		return true;
	}
	return false;
}

ssize_t FileCache::CacheNode::fill(int fd) {
	AutoMutex _l(mCacheNodeLock);
	
#ifdef USE_CACHE_MMAP
	size_t sz = (mCacheSize > mTotalLength - mOffset) ? (mTotalLength - mOffset) : (mCacheSize);
    mData = mmap(NULL, sz, PROT_READ, MAP_SHARED, fd, mOffset);
	if (mData != MAP_FAILED) {
        mDataLen = (ssize_t)sz;
    }
#else
    off64_t result = lseek64(fd, mOffset, SEEK_SET);
	if (result == -1) {
		SXLOGE("seek to %lld failed", mOffset);
		return UNKNOWN_ERROR;
	}
	
    mDataLen = (ssize_t)::read(fd, mData, mCacheSize);
#endif	
    mFilled = true;
	
    if (mDataLen < mCacheSize) SXLOGV("End of Stream!!!");
    return mDataLen;

}

ssize_t FileCache::CacheNode::read(off64_t offset, void *data, size_t size) {
	AutoMutex _l(mCacheNodeLock);
	
	ssize_t sz = 0;

    if (mDataLen <= 0 || mOffset < 0 ) return 0;
	if (mData && offset >= mOffset && offset < (mOffset + mDataLen)) {

	    int64_t pos = offset - mOffset;
		sz = (size > mDataLen - pos) ? (mDataLen - pos):(size);
        
        if (pos + sz > mDataLen) { 
            SXLOGE("pos(%lld)+sz(%d) > mDataLen(%d)",pos, sz, mDataLen);
            return 0;
        }    
		memcpy(data, mData + pos, sz);
	}

	return sz;

}

void FileCache::CacheNode::release() {
    delete this;
}

void FileCache::CacheNode::clear() {
    AutoMutex _l(mCacheNodeLock);
    mOffset = 0;
    mId = 0;
    mPos = 0;
    mFilled = false;
#ifdef USE_CACHE_MMAP	
	if (mData != MAP_FAILED) munmap(mData, mDataLen);
	mData = NULL;
#endif
    mDataLen = -1;
	
}


FileCache::FileCache(int fd, 
		  int64_t offset, 
		  int64_t length,
		  size_t cacheSize,
		  size_t cacheCount,
		  FileSourceProxy *observer) 
    : mFd(fd),
      mDupFd(-1),
	  mOffset(offset),
	  mLength(length),
	  mCacheSize(cacheSize),
	  mCacheCount(cacheCount),
      mObserver(observer),
      mCacheReadCount(0),
      mCacheHitCount(0),
      mCacheMissCount(0) {
      
#ifndef USE_CACHE_MMAP
	char path[256];
	char filename[256];
	memset(path, 0, 256);
	memset(filename, 0, 256);
	sprintf(path, "/proc/%d/fd/%d", gettid(), fd);
	int len = 0;
	len = readlink(path, filename, sizeof(filename));
	if (len) {
		mDupFd = open(filename, O_LARGEFILE | O_RDONLY);
        if (mDupFd == -1) {
            SXLOGE("Open dupFd fail for file %s", filename);
        }
        else {
            SXLOGD("Open file %s for File Cache!", filename);
        }
	}
#endif	

}

FileCache::~FileCache() {
	Mutex::Autolock autoLock(mLock);
	
	List<CacheNode *>::iterator it;
    while (!mLittleCacheNodes.empty()) {
        it = mLittleCacheNodes.begin();
        (*it)->release();
        mLittleCacheNodes.erase(it);
    }
	
	while (!mBigCacheNodes.empty()) {
        it = mBigCacheNodes.begin();
        (*it)->release();
        mBigCacheNodes.erase(it);
    }

	if (mDupFd >= 0) {
        close(mDupFd);
        mDupFd = -1;
    }

}

bool FileCache::hit(List<CacheNode *> *cn, off64_t offset) {
	List<CacheNode *>::iterator it = cn->begin();
	while (it != cn->end()) {
        if ((*it)->hit(offset)){
            return true;
        }
        ++it;
    }
    SXLOGV("FileCache hit miss!!!");

    return false;
}

void FileCache::updateCacheNodeList(CACHE_TYPE ct, off64_t offset) {
	List<CacheNode *> *cn;
	size_t cacheSize;
	size_t cacheCount;

	Mutex::Autolock autoLock(mLock);
    SXLOGV("updateCacheNodeList, ct = %d, offset = %lld", ct, offset);
    //dumpFileCacheInfo();

	if (ct == LITTLE_CACHE) {
		cacheSize = FILE_CACHE_SIZE_LITTLE;
		cacheCount = FILE_CACHE_COUNT_LITTLE;
		cn = &mLittleCacheNodes;
	}
	else {
		cacheSize = FILE_CACHE_SIZE_BIG;
		cacheCount = FILE_CACHE_COUNT_BIG;
		cn = &mBigCacheNodes;
	}
	
    cacheSize = mCacheSize ? (mCacheSize) : (cacheSize);
	cacheCount = mCacheCount ? (mCacheCount) : (cacheCount);
	

    if (!cn->empty() && !hit(cn, offset)) {
		clearCacheQueue(cn);
	}
	else {
		int nodes = cn->empty() ? 0 : cn->size();
		List<CacheNode *>::iterator it = cn->begin();
		if (nodes && (*it)->id() > (offset/cacheSize) ) {
			while (it != cn->end()) {
				if ((*it)->fillFlag()) (*it)->clear();
				it++;
			}
		}
		else {
			for (int i=0; i<nodes; i++) {
				List<CacheNode *>::iterator it = cn->begin();
				if ((*it)->id() < (offset/cacheSize) && (*it)->fillFlag()) {				
	                (*it)->clear();
					
					Mutex::Autolock autoLock(mCacheLock);	
					cn->push_back(*it);
					cn->erase(it);
				}
	            else {
				    break;
	            }
			}
		}
	}

	offset = (cn->size() && (*(cn->begin()))->fillFlag()) ? (*(cn->begin()))->offset() : ((offset/cacheSize)*cacheSize);
	size_t readLen = 0;
	bool cacheInit = cn->empty() ? true : false;

	List<CacheNode *>::iterator it = cn->begin();
	for (int i=0; i<cacheCount; i++) {
		if (mFd < 0 || (offset + i*cacheSize) > mLength ) break;

        CacheNode *p;
		if(cacheInit) {
			p = new CacheNode(offset + i*cacheSize, cacheSize, mLength);

			Mutex::Autolock autoLock(mCacheLock);
			cn->push_back(p);
		}
		else {
			if (i >= cn->size()) break;
			p = (*it);
			++it;
		    if (p->fillFlag()) {mCacheTotalLen += p->size(); continue;}
			p->setOffset(offset + i*cacheSize);
		}
	
	    readLen = p->fill(mFd);	

		mCacheTotalLen += p->size();
		if (readLen < cacheSize) break;
	}

	mCacheOffset = offset;
	
}

void FileCache::triggerUpdate(CACHE_TYPE ct, off64_t offset) {  
    List<CacheNode *> *cn;
	cn = (ct == LITTLE_CACHE) ? (&mLittleCacheNodes) : (&mBigCacheNodes);
	
    if (mCacheLock.tryLock() == NO_ERROR ) {
        if (!cn->empty()) {
            List<CacheNode *>::iterator it = cn->begin();
            if ((*it)->id() == (offset/(*it)->size())) {
                mCacheLock.unlock();
                return;
            }
        }
        mCacheLock.unlock();
        
        FileSourceProxy::Event event;
        event.fd = mFd;
        event.type = ct;
        event.offset = offset;
        mObserver->post(event);
    }
}

ssize_t FileCache::read(off64_t offset, void *data, size_t size) {

	List<CacheNode *> *cn;
    ssize_t resultLength = 0;
	size_t readSize = size;
	off64_t readOffset = offset;
    CACHE_TYPE ct;

#ifdef USE_FILE_SOURCE_CACHE   
	uint64_t readTime = getNowMs();
    SXLOGV("FileCache::read, offest %lld, size %ld, file length %lld", offset, size, mLength);
	
	if (size > FILE_CACHE_BIGLITTLE_THRESHOLD) {
		cn = &mBigCacheNodes;
        ct = BIG_CACHE;
	}
	else {
		cn = &mLittleCacheNodes;
        ct = LITTLE_CACHE;
	}
	
	mCacheReadCount++;

    if (mCacheLock.tryLock() == NO_ERROR ) {
	    List<CacheNode *>::iterator it = cn->begin();
	    size_t readCacheLength = 0;
	    while (it != cn->end()) {
		    if ((*it)->hit(readOffset)){
			    readCacheLength = (*it)->read(readOffset, data + resultLength, readSize);
			    if ( readCacheLength < 0) {
			        SXLOGD("%s, readCacheLength = %d", __FUNCTION__, readCacheLength);
			        readCacheLength = 0;
			    }
			    readSize -= readCacheLength;
			    readOffset += readCacheLength;
			    resultLength += readCacheLength;
			    if (!readSize) {
				    break;
			    }
		    }
	        ++it;
	    }
		if (resultLength) mCacheHitCount++;
		mCacheLock.unlock();
    }

    if (readSize) {
        SXLOGV("read from file system, size %d, offset %lld", readSize, readOffset);
#ifdef USE_CACHE_MMAP
		int fd = mFd; 
#else
        int fd =  mDupFd;
#endif
	    off64_t result = lseek64(fd, readOffset, SEEK_SET);
        ssize_t sz;
	    if (result == -1) {
		    SXLOGV("FileCache, seek to %lld failed", readOffset);
		    return UNKNOWN_ERROR;
		}
	
        sz = ::read(fd, data + resultLength, readSize);
        readOffset += sz;
		resultLength += sz;
	}
	triggerUpdate(ct, readOffset);
	readTime  = getNowMs() - readTime;

    SXLOGV("------ FileCache read fd:%ld,offset:%16lld,size:%8ld,time:%16lld",mFd,offset,size,readTime);
	
	return resultLength;

#else
	uint64_t readTime = getNowMs();
    off64_t result = lseek64(mFd, offset, SEEK_SET);
	ssize_t sz = 0;
    if (result == -1) {
        SXLOGV("FileSystem, seek to %lld failed", offset);
        return UNKNOWN_ERROR;
    }
	sz = ::read(mFd, data, size);
	readTime = getNowMs() - readTime;
	
	SXLOGV("------ FileSystem read offset:%16lld,size:%8ld,time:%16lld",offset,size,readTime);
	
    return sz;
	
#endif
}

void FileCache::clearCacheQueue(List<CacheNode *> *cn) {
    SXLOGV("%s, cn = %p", __FUNCTION__, cn);
	List<CacheNode *>::iterator it = cn->begin();
	while (it != cn->end()) {
        (*it)->clear();
        ++it;
    }
}

int64_t FileCache::getNowMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec*1000ll + tv.tv_usec/1000ll);
}

void FileCache::dumpFileCacheInfo() {
    if (mLittleCacheNodes.size()) {
        List<CacheNode *>::iterator it = mLittleCacheNodes.begin();
        off64_t offset = (*it)->offset();
        size_t size = 0;
        while(it != mLittleCacheNodes.end()) {
            size += (*it)->size();
            ++it;
        }
        SXLOGD("Dump FileCache Information: little cache number: %d, offset: %lld, size: %d", mLittleCacheNodes.size(), offset, size);
    }
    else {
        SXLOGD("Dump FileCache Information: little cache is empty!!!");
    }

    if (mBigCacheNodes.size()) {
        List<CacheNode *>::iterator it = mBigCacheNodes.begin();
        off64_t offset = (*it)->offset();
        size_t size = 0;
        while(it != mBigCacheNodes.end()) {
            size += (*it)->size();
            ++it;
        }
        SXLOGD("Dump FileCache Information: big cache number: %d, offset: %lld, size: %d", mBigCacheNodes.size(), offset, size);
    }
    else {
        SXLOGD("Dump FileCache Information: big cache is empty!!!");
    }
}

FileSourceProxy::FileSourceProxy() 
    : mThread(NULL) {
   
}

FileSourceProxy::~FileSourceProxy() {
    if (mThread != NULL) {
        mThread->requestExit();
	    mQueueChangedCondition.signal();
	    mThread->requestExitAndWait();
    }
}

status_t FileSourceProxy::registerFd(int fd, int64_t offset, int64_t length) {
    Mutex::Autolock autoLock(mLock);
	SXLOGD("Fd: %d register!", fd);

	ssize_t index = mFileCaches.indexOfKey(fd);
	
    if (index >= 0) {
		SXLOGW("Fd: %d has been resitered!", fd);
        return INVALID_OPERATION;
    }

	if (mFileCaches.size() > FILE_CACHE_NUMBER) {
		SXLOGW("Resiter fd has reach the max: %d", FILE_CACHE_NUMBER);
        return INVALID_OPERATION;
    }
    
	sp<FileCache> fc = new FileCache(fd, offset, length, 0, 0, this);

	mFileCaches.add(fd, fc);

    if (mThread == NULL) {
	    mThread = new ProxyThread(this, false);

        status_t err = mThread->run(
		    mName.empty() ? "FileSourceProxy" : mName.c_str(), ANDROID_PRIORITY_NORMAL);
        if (err != OK) {
            mThread.clear();
            return UNKNOWN_ERROR;
        }
    }

    return OK;
}

void FileSourceProxy::unregisterFd(int fd) {
    Mutex::Autolock autoLock(mLock);
    SXLOGD("Fd: %d unregister!", fd);

    ssize_t index = mFileCaches.indexOfKey(fd);
	
    if (index < 0) {
        return;
    }

	sp<FileCache> fc = mFileCaches.valueAt(index);
	SXLOGD("Fd: %d, File Cache hit rate: %2.2f%", fd, fc->hitRate());
	fc->reset();
	
    mFileCaches.removeItemsAt(index);
}

ssize_t FileSourceProxy::read(int fd, off64_t offset, void *data, size_t size) {
    sp<FileCache> fc = NULL;
    {
        Mutex::Autolock autoLock(mLock);
        ssize_t index = mFileCaches.indexOfKey(fd);
        
        if (index < 0) {
            return INVALID_OPERATION;
        }

        fc = mFileCaches.valueAt(index);
    }
	return fc->read(offset, data, size);
}

void FileSourceProxy::post(const Event &event) {
    if (mLock.tryLock() == NO_ERROR ) {
		bool newEvent = true;	
		List<Event>::iterator it = mEventQueue.begin();
	    while (it != mEventQueue.end()) {
			if (it->fd == event.fd) {
				it->type = event.type;
				it->offset = event.offset;
				newEvent = false;
				break;
			}
			++it;
	    }
		if (CC_UNLIKELY(newEvent)) mEventQueue.push_back(event);
		mQueueChangedCondition.signal();
		mLock.unlock();
    }
}

bool FileSourceProxy::loop() {
    Event event;	
	sp<FileCache> fc = NULL;
    {
		Mutex::Autolock autoLock(mLock);
        if (mThread == NULL) {
            return false;
        }
		
        if (mEventQueue.empty()) {
            mQueueChangedCondition.wait(mLock);
            return true;
        }
		
        event = *mEventQueue.begin();
        mEventQueue.erase(mEventQueue.begin());

		ssize_t index = mFileCaches.indexOfKey(event.fd);
		if (index < 0) {
	    	return true;
    	}
		fc = mFileCaches.valueFor(event.fd);
    }

	if (fc.get()) {
		fc->updateCacheNodeList((FileCache::CACHE_TYPE)event.type, event.offset);
	}
   
    return true;
}

}
