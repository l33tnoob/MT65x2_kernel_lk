#ifndef EXTENDEDSENSORDATA_H
#define EXTENDEDSENSORDATA_H

//ExtendedSensorData use BasicSensorData

#include "BasicSensorData.h"

class ExtendedSensorData
{
    public:
        virtual ~ExtendedSensorData(){}
        void update(const ASensorEvent& event, const CollectedBasicSensorData& cbsd)
        {
            if(isTriggeredToUpdate_(event))
            {updateContent_(cbsd);}
        }

    protected:
        virtual bool isTriggeredToUpdate_(const ASensorEvent& event) = 0;
        virtual void updateContent_(const CollectedBasicSensorData& cbsd)=0;
};

class GravityVectorData : public ExtendedSensorData
{
    public:
        GravityVectorData():x(__initialSensorValue_), y(__initialSensorValue_), z(__initialSensorValue_){}
        float x,y,z;

    protected:
        virtual bool isTriggeredToUpdate_(const ASensorEvent& event)
        {return (event.type==SensorType_Orientation);}
        virtual void updateContent_(const CollectedBasicSensorData& cbsd);
};

class CollectedExtendedSensorData
{
    public:
        GravityVectorData gravityVectorData;

    public:
        void update(const ASensorEvent& event, const CollectedBasicSensorData& cbsd)
        {
            gravityVectorData.update(event,cbsd);
        }
};

#endif // EXTENDEDSENSORDATA_H
