/*
 * XSync.h
 *
 *  Created on: 2011-7-28
 *      Author: mtk80905
 */

#ifndef XSYNC_H_
#define XSYNC_H_

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

class XSemphore {
public:
	XSemphore();
	~XSemphore();
	bool	Init(int initsps);
	int		Post();
	int		Wait();
	int		TimedWait(int delay);
	bool	IsValid();

private:
	bool		m_valid;
	sem_t		m_sema;
};

/* The object was locked until Unlock() was called. */
class XLock
{
public:
	XLock();
	~XLock();
	void Lock();
	void Unlock();

private:
	pthread_mutex_t m_mutex;
};


class XSingleLock
{
public:
	XSingleLock(XLock *lock);
	~XSingleLock();
	void Lock();

private:
	XLock*		mlock;
};


#endif /* XSYNC_H_ */
