#ifndef SENSOR_LISTENER_IMP_H
#define SENSOR_LISTENER_IMP_H
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
//-----------------------------------------------------------------------------
bool
setThreadPriority(
    int policy,
    int priority);
bool
getThreadPriority(
    int& policy,
    int& priority);
//-----------------------------------------------------------------------------
class SensorListenerImpThread : public Thread
{
    public:
        SensorListenerImpThread(Looper* looper) : Thread(false)
        {
            mLooper = sp<Looper>(looper);
        }
        //
        ~SensorListenerImpThread()
        {
            mLooper.clear();
        }
        //
        status_t readyToRun()
        {
            int policy = SCHED_OTHER, priority = 0;
            //
            ::prctl(PR_SET_NAME,"Mtkcam@SensorListenerImpThread", 0, 0, 0);
            //
            setThreadPriority(policy,priority);
            getThreadPriority(policy,priority);
            MY_LOGD("policy(%d),priority(%d)",policy,priority);
            //
            return NO_ERROR;
        }
        //
        virtual bool threadLoop()
        {
            mLooper->pollOnce(-1);
            return true;
        }
        // force looper wake up
        void wake() {
            mLooper->wake();
        }
    private:
        sp<Looper> mLooper;
};
//-----------------------------------------------------------------------------
class SensorListenerImp : public SensorListener
{
    public:
        SensorListenerImp();
        ~SensorListenerImp();
        //
        virtual void    destroyInstance(void);
        virtual MBOOL   setListener(Listener func);
        virtual MBOOL   enableSensor(
                            SensorTypeEnum  sensorType,
                            MUINT32         periodInMs);
        virtual MBOOL   disableSensor(SensorTypeEnum sensorType);
        //
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   getEvents(void);
        //
    private:
        #define SENSOR_TYPE_AMOUNT  (SensorType_Proxi+1)
        //
        mutable Mutex               mLock;
        volatile MINT32             mUser;
        Sensor const*               mpSensor[SENSOR_TYPE_AMOUNT];
        volatile MUINT32            mSensorEnableCnt;
        sp<Looper>                  mspLooper;
        sp<SensorEventQueue>        mspSensorEventQueue;
        SensorManager*              mpSensorManager;
        Listener                    mpListener;
        sp<SensorListenerImpThread> mspThread;
};
//-----------------------------------------------------------------------------
#endif

