#ifndef MOTION_H
#define MOTION_H

#include "FullCollectedSensorData.h"

//static const int __motion_type_number_ = 1;

enum
{
    MotionType_RaiseToHead
};

class Motion
{
    public:
        Motion();
        virtual ~Motion(){}
        virtual int type() const =0;

    //trigger related
    public:
        bool isTriggered() const {return triggered_;}
        void testTriggered(FullCollectedSensorData* senserDataWindow, int windowSize)
        {triggered_=detect_(senserDataWindow,windowSize);}
    protected:
        bool detect_(FullCollectedSensorData* senserDataWindow, int windowSize);
        virtual bool detectImp_(FullCollectedSensorData* senserDataWindow, int windowSize) const =0;
        bool triggered_;

    protected:
        virtual int cooldownTimes_() const =0;
        int cooldownCounter_;
};

class MotionRaiseToHead : public Motion
{
    public:
        virtual ~MotionRaiseToHead(){}
    public:
        virtual int type() const {return MotionType_RaiseToHead;}
    protected:
        virtual bool detectImp_(FullCollectedSensorData* senserDataWindow, int windowSize) const;
    protected:
        virtual int cooldownTimes_() const {return 120;}

    protected:
        bool calPitchActivatedFlag_(FullCollectedSensorData* senserDataWindow, int windowSize) const;
        bool calRollActivatedFlag_(FullCollectedSensorData* senserDataWindow, int windowSize) const ;
        bool calLinearXFastMoveFlag_(const int dropLowerBoundFrame, FullCollectedSensorData* senserDataWindow, int windowSize) const ;
};


#endif // MOTION_H
