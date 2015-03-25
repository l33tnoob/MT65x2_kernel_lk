#ifndef FULLCOLLECTEDSENSORDATA_H
#define FULLCOLLECTEDSENSORDATA_H

#include "BasicSensorData.h"
#include "ExtendedSensorData.h"
#include "HybridSensorData.h"

class FullCollectedSensorData
{
    public:
        void update(const ASensorEvent& event)
        {
            cbsd_.update(event);
            cesd_.update(event,cbsd_);
            chsd_.update(event,cbsd_,cesd_);
        }

        const AccelerationData  accelerationData()  const {return cbsd_.accelerationData;}
        const OrientationData   orientationData()   const {return cbsd_.orientationData;}
        const MagneticFieldData magneticFieldData() const {return cbsd_.magneticFieldData;}
        const ProximityData     proximityData()     const {return cbsd_.proximityData;}
        const LightData         lightData()         const {return cbsd_.lightData;}

        const GravityVectorData gravityVectorData() const {return cesd_.gravityVectorData;}

        const LinearAccelerationData linearAccelerationData() const {return chsd_.linearAccelerationData;}

    protected:
        CollectedBasicSensorData cbsd_;
        CollectedExtendedSensorData cesd_;
        CollectedHybridSensorData chsd_;
};

#endif // FULLCOLLECTEDSENSORDATA_H
