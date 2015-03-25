#define LOG_TAG "MtkCam/IHWScenario"

#include <utils/Vector.h>

#include <mtkcam/common.h>
#include <mtkcam/imageio/IPipe.h>

#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
using namespace NSImageio;
using namespace NSIspio;

#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
#include "hwUtility.h"
#include "VSSScenario.h"
#include "ZSDScenario.h"




/*******************************************************************************
*
********************************************************************************/

IhwScenario*
IhwScenario::createInstance(EhwMode const& mode, halSensorType_e const & type,
                            halSensorDev_e const &dev, ERawPxlID const &bitorder)
{
    EScenarioFmt SensorType = mapSensorType(type);
    if (mode == eHW_VSS){
        return VSSScenario::createInstance(SensorType, dev, bitorder);
    }
    else if (mode == eHW_ZSD){
        return ZSDScenario::createInstance(SensorType, dev, bitorder);
    }
    else {
        return NULL; //suppose no other scenario would use IhwScenario
    }
}


