#ifndef SENSORDATATRACKER_H
#define SENSORDATATRACKER_H

#include "Parameter.h"
#include "FullCollectedSensorData.h"

#include <gui/SensorEventQueue.h>
#include <utils/Looper.h>
#include <utils/RefBase.h>
#include <sys/types.h>

class SensorDataTracker
{
    //const parameters
    public:
        SensorDataTracker();
        virtual ~SensorDataTracker(){}

    //framework related
    public:
        bool isAvailable();
        void runListenOnce();
        void runListen();
        void stopListen(){keepListenFlag_=false;}
    protected:
        virtual void performPeriodicFunction_(){}//default do nothing
        bool keepListenFlag_; //set to false to stop listen

    //constructor
    protected:
        void setupSensor_(android::SensorManager& mgr,
                          android::sp<android::SensorEventQueue> q,
                          int sensorType,
                          int rate_ms);

    //init
    protected:
        void initLooperWithCallback_();

    //sensor queue related
    protected:
        bool isAllSensorFunctional_;
        android::sp<android::SensorEventQueue> queue_;
        android::sp<android::Looper> looper_;
        android::sp<android::LooperCallback> collectedSensorUpdater_;
        FullCollectedSensorData curData_;

        ASensorEvent additionalPendingEvent_;
        bool hasAdditionalPendingEvent_;

        friend class CollectedSensorUpdater;
};


#endif // SENSORDATATRACKER_H
