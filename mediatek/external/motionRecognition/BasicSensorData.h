#ifndef BASICSENSORDATA_H
#define BASICSENSORDATA_H

#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>

static const float __initialSensorValue_ = 0.7654321;

enum
{
    SensorType_Acceleration = android::Sensor::TYPE_ACCELEROMETER,
    SensorType_MagneticField = android::Sensor::TYPE_MAGNETIC_FIELD,
    SensorType_Orientation = 3,
    SensorType_Light = android::Sensor::TYPE_LIGHT,
    SensorType_Proximity = android::Sensor::TYPE_PROXIMITY
};

class BasicSensorData
{
    public:
        virtual ~BasicSensorData(){}
        void update(const ASensorEvent& event)
        {
            if(isEventMatch(event))
            {updateContent_(event);}
        }

        bool isEventMatch(const ASensorEvent& event){return event.type==sensorType_();}

    protected:
        virtual int sensorType_()=0;
        virtual void updateContent_(const ASensorEvent& event)=0;
};

class AccelerationData : public BasicSensorData
{
    public:
        AccelerationData():x(__initialSensorValue_), y(__initialSensorValue_), z(__initialSensorValue_){}
        float x,y,z;
    protected:
        virtual int sensorType_(){return SensorType_Acceleration;}
        virtual void updateContent_(const ASensorEvent& event)
        {
            x=event.acceleration.x;
            y=event.acceleration.y;
            z=event.acceleration.z;
        }
};

class MagneticFieldData : public BasicSensorData
{
    public:
        MagneticFieldData():x(__initialSensorValue_), y(__initialSensorValue_), z(__initialSensorValue_){}
        float x, y, z;
    protected:
        virtual int sensorType_(){return SensorType_MagneticField;}
        virtual void updateContent_(const ASensorEvent& event)
        {
            x=event.magnetic.x;
            y=event.magnetic.y;
            z=event.magnetic.z;
        }
};

//class GyroscopeData : public BasicSensorData
//{
//    public:
//        GyroscopeData():azimuth(__initialSensorValue_), pitch(__initialSensorValue_), roll(__initialSensorValue_){}
//        float azimuth, pitch, roll;
//    protected:
//        virtual int sensorType_(){return ;}
//        virtual void updateContent_(const ASensorEvent& event)
//        {
//            azimuth=event.magnetic.azimuth;
//            pitch=event.magnetic.pitch;
//            roll=event.magnetic.roll;
//        }
//};

class OrientationData : public BasicSensorData
{
    public:
        OrientationData():azimuth(__initialSensorValue_), pitch(__initialSensorValue_), roll(__initialSensorValue_){}
        float azimuth, pitch, roll;
    protected:
        virtual int sensorType_(){return SensorType_Orientation;}
        virtual void updateContent_(const ASensorEvent& event)
        {
            azimuth=event.vector.azimuth;
            pitch=event.vector.pitch;
            roll=event.vector.roll;
        }
};


class ProximityData : public BasicSensorData
{
    public:
        ProximityData():distance(__initialSensorValue_){}
        float distance;
    protected:
        virtual int sensorType_(){return SensorType_Proximity;}
        virtual void updateContent_(const ASensorEvent& event)
        {
            distance=event.distance;
        }
};

class LightData : public BasicSensorData
{
    public:
        LightData():light(__initialSensorValue_){}
        float light;
    protected:
        virtual int sensorType_(){return SensorType_Light;}
        virtual void updateContent_(const ASensorEvent& event)
        {
            light=event.light;
        }
};

class CollectedBasicSensorData
{
    public:
        AccelerationData accelerationData;
        OrientationData orientationData;
        MagneticFieldData magneticFieldData;
        //GyroscopeData gyroscopeData;
        ProximityData proximityData;
        LightData lightData;

    public:
        void update(const ASensorEvent& event)
        {
            accelerationData.update(event);
            orientationData.update(event);
            magneticFieldData.update(event);
            //gyroscopeData.update(event);
            proximityData.update(event);
            lightData.update(event);
        }
};
#endif // BASICSENSORDATA_H
