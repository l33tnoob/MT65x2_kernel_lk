#ifndef HYBRIDSENSORDATA_H
#define HYBRIDSENSORDATA_H

//HybridSensorData use ExtendedSensorData and BasicSensorData

#include "ExtendedSensorData.h"

class HybridSensorData
{
    public:
        virtual ~HybridSensorData(){}
        void update(const ASensorEvent& event,
                    const CollectedBasicSensorData& cbsd,
                    const CollectedExtendedSensorData& cesd)
        {
            if(isTriggeredToUpdate_(event))
            {updateContent_(cbsd,cesd);}
        }

    protected:
        virtual bool isTriggeredToUpdate_(const ASensorEvent& event) = 0;
        virtual void updateContent_(const CollectedBasicSensorData& cbsd,
                                    const CollectedExtendedSensorData& cesd)=0;
};

class LinearAccelerationData : public HybridSensorData
{
    public:
        LinearAccelerationData():x(__initialSensorValue_), y(__initialSensorValue_), z(__initialSensorValue_){}
        float x,y,z;

    protected:
        virtual bool isTriggeredToUpdate_(const ASensorEvent& event)
        {
            return  (event.type==SensorType_Orientation) ||
                    (event.type==SensorType_Acceleration);
        }

        virtual void updateContent_(const CollectedBasicSensorData& cbsd,
                                    const CollectedExtendedSensorData& cesd)
        {
            x = cbsd.accelerationData.x - cesd.gravityVectorData.x;
            y = cbsd.accelerationData.y - cesd.gravityVectorData.y;
            z = cbsd.accelerationData.z - cesd.gravityVectorData.z;
        }
};

class CollectedHybridSensorData
{
    public:
        LinearAccelerationData linearAccelerationData;

    public:
        void update(const ASensorEvent& event,
                    const CollectedBasicSensorData& cbsd,
                    const CollectedExtendedSensorData& cesd)
        {
            linearAccelerationData.update(event,cbsd,cesd);
        }
};

#endif // HYBRIDSENSORDATA_H
