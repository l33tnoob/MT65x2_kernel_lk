/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_AUDIO_MUTEX_H
#define ANDROID_AUDIO_MUTEX_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>


#include <pthread.h>

#include <utils/Errors.h>
#include <utils/Timers.h>

#include "AudioType.h"
#include "AudioAssert.h"

namespace android
{

static const uint32_t kDefaultTimeoutMilliseconds = 3000;

class AudioCondition;


/**
 * Simple mutex class.
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class AudioLock
{
    public:
        AudioLock();
        ~AudioLock();

        // lock or unlock the mutex
        status_t    lock();
        void        unlock();

        // lock if possible; returns 0 on success, error otherwise
        status_t    tryLock();

        // lock if possible; returns 0 on success, error otherwise
        status_t    lock_timeout(const uint32_t milliseconds); // TODO(Harvey): rename to lock_timeout()?

        // Manages the mutex automatically. It'll be locked when Autolock is
        // constructed and released when Autolock goes out of scope.
        class AudioAutoTimeoutLock
        {
            public:
                inline AudioAutoTimeoutLock(AudioLock &mutex) : mLock(mutex)
                {
                    status_t retval = mLock.lock_timeout(kDefaultTimeoutMilliseconds);
                    ASSERT(retval == NO_ERROR);
                }
                inline AudioAutoTimeoutLock(AudioLock &mutex, const uint32_t milliseconds) : mLock(mutex)
                {
                    status_t retval = mLock.lock_timeout(milliseconds);
                    ASSERT(retval == NO_ERROR);
                }
                inline ~AudioAutoTimeoutLock() { mLock.unlock(); }
            private:
                AudioLock &mLock;
        };

    private:
        friend class AudioCondition;

        // A mutex cannot be copied
        AudioLock(const AudioLock &);
        AudioLock &operator = (const AudioLock &);

        pthread_mutex_t mMutex;
};


// ---------------------------------------------------------------------------

/**
 * Automatic mutex.  Declare one of these at the top of a function.
 * When the function returns, it will go out of scope, and release the
 * mutex.
 */
typedef AudioLock::AudioAutoTimeoutLock AudioAutoTimeoutLock;


// ---------------------------------------------------------------------------

inline AudioLock::AudioLock()
{
    pthread_mutex_init(&mMutex, NULL);
}
inline AudioLock::~AudioLock()
{
    pthread_mutex_destroy(&mMutex);
}
inline status_t AudioLock::lock()
{
    return -pthread_mutex_lock(&mMutex);
}
inline void AudioLock::unlock()
{
    pthread_mutex_unlock(&mMutex);
}
inline status_t AudioLock::tryLock()
{
    return -pthread_mutex_trylock(&mMutex);
}
inline status_t AudioLock::lock_timeout(const uint32_t milliseconds)
{
    return -pthread_mutex_lock_timeout_np(&mMutex, milliseconds);
}


// ---------------------------------------------------------------------------

/**
 * AudioCondition variable class.
 * AudioCondition variables are paired up with mutexes.  Lock the mutex,
 * call wait(), then either re-wait() if things aren't quite what you want,
 * or unlock the mutex and continue.  All threads calling wait() must
 * use the same mutex for a given AudioCondition.
 */
class AudioCondition
{
    public:
        AudioCondition();
        ~AudioCondition();
        // Wait on the condition variable.  Lock the mutex before calling.
        status_t wait(AudioLock &mutex);
        // same with relative timeout
        status_t waitRelative(AudioLock &mutex, nsecs_t reltime);
        // Signal the condition variable, allowing one thread to continue.
        void signal();
        // Signal the condition variable, allowing all threads to continue.
        void broadcast();

    private:
        pthread_cond_t mCond;
};


// ---------------------------------------------------------------------------

inline AudioCondition::AudioCondition()
{
    pthread_cond_init(&mCond, NULL);
}
inline AudioCondition::~AudioCondition()
{
    pthread_cond_destroy(&mCond);
}
inline status_t AudioCondition::wait(AudioLock &mutex)
{
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}
inline status_t AudioCondition::waitRelative(AudioLock &mutex, nsecs_t reltime)
{
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE)
    struct timespec ts;
    ts.tv_sec  = reltime / 1000000000;
    ts.tv_nsec = reltime % 1000000000;
    return -pthread_cond_timedwait_relative_np(&mCond, &mutex.mMutex, &ts);
#else // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
    struct timespec ts;

    ts.tv_sec += reltime / 1000000000;
    ts.tv_nsec += reltime % 1000000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
#endif // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE
}
inline void AudioCondition::signal()
{
    pthread_cond_signal(&mCond);
}
inline void AudioCondition::broadcast()
{
    pthread_cond_broadcast(&mCond);
}

// ---------------------------------------------------------------------------

} // end namespace android

#endif // end of ANDROID_AUDIO_MUTEX_H
