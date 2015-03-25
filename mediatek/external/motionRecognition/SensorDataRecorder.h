#ifndef SENSORDATARECORDER_H
#define SENSORDATARECORDER_H

#include <cstdlib>
#include <cstdio>
#include "SensorDataTracker.h"

//#define ___redirect_stdout__

class SensorDataRecorder : public SensorDataTracker
{
    public:
        SensorDataRecorder();
        virtual ~SensorDataRecorder();

        void flipTag(){recordTag_=(!recordTag_);}

    protected:
        void recordFormat1_();
        void recordFormat2_();

    protected:
        virtual void performPeriodicFunction_();
        bool recordTag_;

        #ifndef ___redirect_stdout__
        FILE* fp;
        #endif
};

#endif // SENSORDATARECORDER_H
