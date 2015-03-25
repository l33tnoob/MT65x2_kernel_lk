#ifndef FILE_SOURCE_PROXY_H_
#define FILE_SOURCE_PROXY_H_

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

namespace android {

class FileCache;

class FileSourceProxy : public RefBase {
public:	
	FileSourceProxy();
	
	virtual ~FileSourceProxy();

	struct Event {
		int fd;
        int type;
		off64_t offset;
    };

	status_t registerFd(int fd, int64_t offset, int64_t length);

	void unregisterFd(int fd);

	ssize_t read(int fd, off64_t offset, void *data, size_t size);

	void post(const Event &event);

private:
	Mutex mLock;
	Condition mQueueChangedCondition;
	KeyedVector<int, sp<FileCache> > mFileCaches;
	List<Event> mEventQueue;
	AString mName;

	class ProxyThread;
    sp<ProxyThread> mThread;

	bool loop();
	
    DISALLOW_EVIL_CONSTRUCTORS(FileSourceProxy);

};

}

#endif  //FILE_SOURCE_PROXY_H_
