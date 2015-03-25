#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H

#include "FixedSizeQueue.h"
#include "SensorDataTracker.h"
#include "SensorDataRecorder.h"
#include "Motion.h"

#include <utils/Vector.h>
#include <utils/String8.h>
#include <utils/Mutex.h>

class MotionDetector : public SensorDataRecorder /*public SensorDataTracker*/
{
    public:
        static int getVersion();

    public:
        MotionDetector();
        virtual ~MotionDetector();

    protected:
        virtual void performPeriodicFunction_();

    protected:
        FixedSizeQueue<FullCollectedSensorData> bufferedCollectedSensorData_;

    //motion related
    public:
        const char* getTriggeredState();
        bool queryMotionTriggered(int motionType);

    protected:
        void registerMotion_(Motion* newMotion);
        android::Vector<Motion*> motionVector_;
        android::String8 triggeredState_;
        char* cachedTriggeredState_;
        android::Mutex triggeredStateMutex_;
};

#endif // MOTIONDETECTOR_H
