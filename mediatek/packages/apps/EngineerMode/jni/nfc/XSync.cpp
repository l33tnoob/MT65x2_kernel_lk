/*
 * XSemphore.cpp
 *
 *  Created on: 2011-7-28
 *      Author: mtk80905
 */

#include "XSync.h"
#define LOG_TAG "nfc"
#include <cutils/xlog.h>

XSemphore::XSemphore() :
	m_valid(false) {
}

XSemphore::~XSemphore() {
	if (m_valid)
		sem_destroy(&m_sema);
}

bool XSemphore::Init(int initsps) {
	int ret = sem_init(&m_sema, 0, initsps);
	if (ret == 0) {
		m_valid = true;
	} else {
		m_valid = false;
	}
	return m_valid;
}
//return 0 is OK
int XSemphore::Post() {
	return sem_post(&m_sema);
}

//return 0 is OK
int XSemphore::Wait() {
	int rc = -1;
	while ((rc = sem_wait(&m_sema)) == -1 && errno == EINTR)
		continue;

	return rc;

}

//return 0 is OK
int XSemphore::TimedWait(int delay) {
	int rc = -1;
	timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += delay;
	while ((rc = sem_timedwait(&m_sema, &ts)) == -1 && errno == EINTR)
		continue;
	//XLOGD("XSemphore::TimedWait delay %d, errno %d, rc %d, EINTR %d, ", delay, errno, rc, EINTR);
	return rc;

}

bool XSemphore::IsValid() {
	return m_valid;

}





// XLock implementations
XLock::XLock()
{
	pthread_mutex_init(&m_mutex,NULL);
}

XLock::~XLock()
{
	pthread_mutex_destroy( &m_mutex );
}

void XLock::Lock()
{
	int ret = pthread_mutex_lock( &m_mutex );
}

void XLock::Unlock()
{
	int ret = pthread_mutex_unlock( &m_mutex );

}
// end of XLock implementations

// XSingleLock implementations
XSingleLock::XSingleLock(XLock *lock)
	: mlock(lock)
{
}

XSingleLock::~XSingleLock()
{
	if(mlock) mlock->Unlock();
}

void XSingleLock::Lock()
{
	if(mlock) mlock->Lock();
}
// end of XSingleLock implementations
