#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include "eeprom_drv.h"
#include "eeprom_drv_imp.h"

#ifdef EEPROM_SUPPORT  //seanlin 120919 it's for compiling mt6589fpga_ca7_ldvt
#include "camera_custom_msdk.h"
#endif
#include "camera_custom_eeprom.h"
#include "kd_camera_feature.h"
#include "camera_common_calibration.h"

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
//#undef LOG_TAG
#define EEPROM_LOG_TAG "EepromDrv"

//#define EEPROM_DRV_LOG(fmt, arg...)    LOGD(LOG_TAG " "fmt, ##arg)
//#define EEPROM_DRV_ERR(fmt, arg...)    LOGE(LOG_TAG "Err: %5d: "fmt, __LINE__, ##arg)

#define EEPROM_DRV_LOG(fmt, arg...)    XLOGD(EEPROM_LOG_TAG " "fmt, ##arg)
#define EEPROM_DRV_ERR(fmt, arg...)    XLOGE(EEPROM_LOG_TAG "Err: %5d: "fmt, __LINE__, ##arg)


#define INVALID_HANDLE_VALUE (-1)

/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*
********************************************************************************/
static unsigned long const g_u4EepromDataSize[CAMERA_EEPROM_TYPE_NUM] =
{
    sizeof(NVRAM_CAMERA_SHADING_STRUCT)+sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
#if EEPROM_DRV_DEFECT_EN  //seanlin 120919 to avoid 658x has no Defect
    sizeof(NVRAM_CAMERA_DEFECT_STRUCT),
#endif
    sizeof(NVRAM_CAMERA_3A_STRUCT)
};

  EEPROM_DATA_STRUCT EepromDrv::eEepromCaldata;
  MINT32 EepromDrv::m32EepromDataValidation=EEPROM_ERR_NO_DATA;
/*******************************************************************************
*
********************************************************************************/
EepromDrvBase*
EepromDrvBase::createInstance()
{
    return EepromDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
EepromDrvBase*
EepromDrv::getInstance()
{
    static EepromDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
EepromDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
EepromDrv::EepromDrv()
    : EepromDrvBase()
{

}

/*******************************************************************************
*
********************************************************************************/
EepromDrv::~EepromDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int EepromDrv::GetEepromCalData(unsigned long a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_EEPROM_TYPE_ENUM a_eEepromDataType,
	                      void *a_pEepromData)
{
//    int err = EEPROM_NO_ERROR;
    MUINT32 i;
    static BOOL bfFirstCalData = FALSE;
    PGET_SENSOR_CALIBRATION_DATA_STRUCT la_pEepromData = (PGET_SENSOR_CALIBRATION_DATA_STRUCT)a_pEepromData;
    PEEPROM_DATA_STRUCT pEerom_data = &eEepromCaldata;
    if ((a_eSensorType > DUAL_CAMERA_SENSOR_MAX) ||(a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
        (a_eEepromDataType > CAMERA_EEPROM_DATA_PREGAIN) ||(a_eEepromDataType < CAMERA_EEPROM_DATA_SHADING_TABLE) ||
        (a_pEepromData == NULL) )
    {
        m32EepromDataValidation =EEPROM_ERR_NO_DATA|EEPROM_UNKNOWN;
        EEPROM_DRV_LOG("[EEPROM_READ_PARAMETER_ERROR]0x%x \n",m32EepromDataValidation);
        return m32EepromDataValidation;
    }
    //Sync the eeprom data from ISP
    if( la_pEepromData->pCameraPara!= NULL)
    {
        pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD];
        pEerom_data->Defect.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD];
        for(i=0;i<EEPROM_PART_NUMBERS_COUNT;i++)
        {
            pEerom_data->Shading.ISPComm.CommReg[gulPartNumberReg[i]] = la_pEepromData->pCameraPara->ISPComm.CommReg[gulPartNumberReg[i]];
        }
        for(i=0;i<EEPROM_SENSOR_RESOLUTION_COUNT;i++)
        {
            pEerom_data->Shading.ISPComm.CommReg[gulSensorResol[i]] = la_pEepromData->pCameraPara->ISPComm.CommReg[gulSensorResol[i]];
        }
    }
    //finish data extraction from Eeprom
    if(!bfFirstCalData)
    {
        Init(u4SensorID,a_pEepromData);
        bfFirstCalData = TRUE;
    }

    switch(a_eEepromDataType) {
    case CAMERA_EEPROM_DATA_SHADING_TABLE:
#if 0 // vend_edwin.yang
        if(m32EepromDataValidation&EEPROM_ERR_NO_SHADING)
        {
            EEPROM_DRV_LOG("(0x%x)No Shading \n",  m32EepromDataValidation);
        }
        else
        {
            if((la_pEepromData->pCameraPara!=NULL)&&(la_pEepromData->pCameraShading!=NULL))
            {
                if(eEepromCaldata.Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] != CAL_DATA_LOAD)
                {
                    if(m32EepromDataValidation&EEPROM_ERR_SENSOR_SHADING)
                    {
                        la_pEepromData->pCameraPara->ISPRegs.Idx.LSC = 2; //for diable ISP Shading.
                        EEPROM_DRV_LOG("Disable Shading and using Sensor Shading\n 0x%x\n",m32EepromDataValidation);
                        la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_SHADING_TYPE] = CAL_SHADING_TYPE_SENSOR;
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.SensorCalTable[0],(UINT8*)&eEepromCaldata.Shading.SensorCalTable[0], MAX_SENSOR_CAL_SIZE);
                    }//
                    else
                    {
                        la_pEepromData->pCameraShading->Shading.PreviewSize = eEepromCaldata.Shading.PreviewSize;
                        la_pEepromData->pCameraShading->Shading.CaptureSize = eEepromCaldata.Shading.CaptureSize;
                        la_pEepromData->pCameraPara->ISPRegs.LSC[0].ctl2.val=eEepromCaldata.Shading.PreRegister.shading_ctrl2;
                        la_pEepromData->pCameraPara->ISPRegs.LSC[0].lblock.val=eEepromCaldata.Shading.PreRegister.shading_last_blk;
                        la_pEepromData->pCameraPara->ISPRegs.LSC[1].ctl2.val=eEepromCaldata.Shading.CapRegister.shading_ctrl2;
                        la_pEepromData->pCameraPara->ISPRegs.LSC[1].lblock.val=eEepromCaldata.Shading.CapRegister.shading_last_blk;
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.CaptureTable[2][0],(UINT8*)&eEepromCaldata.Shading.CaptureTable[2][0], eEepromCaldata.Shading.CaptureSize*4);
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.CaptureTable[1][0],(UINT8*)&eEepromCaldata.Shading.CaptureTable[1][0], eEepromCaldata.Shading.CaptureSize*4);
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.CaptureTable[0][0],(UINT8*)&eEepromCaldata.Shading.CaptureTable[0][0], eEepromCaldata.Shading.CaptureSize*4);
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.PreviewTable[2][0],(UINT8*)&eEepromCaldata.Shading.PreviewTable[2][0], eEepromCaldata.Shading.PreviewSize*4);
                        memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.PreviewTable[1][0],(UINT8*)&eEepromCaldata.Shading.PreviewTable[1][0], eEepromCaldata.Shading.PreviewSize*4);
	                    memcpy((UINT8*)& la_pEepromData->pCameraShading->Shading.PreviewTable[0][0],(UINT8*)&eEepromCaldata.Shading.PreviewTable[0][0], eEepromCaldata.Shading.PreviewSize*4);
                        la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_SHADING_TYPE] = CAL_SHADING_TYPE_ISP;
                        EEPROM_DRV_LOG("Shading update CAL_SHADING_TYPE_ISP\n");
                    }
                    for(i=0;i<EEPROM_PART_NUMBERS_COUNT;i++)
                    {
                        la_pEepromData->pCameraPara->ISPComm.CommReg[gulPartNumberReg[i]] = eEepromCaldata.Shading.ISPComm.CommReg[gulPartNumberReg[i]];
//                        EEPROM_DRV_LOG("la_pEepromData->pCameraPara->ISPComm.CommReg[gulPartNumberReg[%d]] =  0x%x\n",gulPartNumberReg[i],la_pEepromData->pCameraPara->ISPComm.CommReg[gulPartNumberReg[i]]);
                    }
                    la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = eEepromCaldata.Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] ; //sync the ISP data
                }
            }
        }
#endif
    break;
    case CAMERA_EEPROM_DATA_DEFECT_TABLE:
        if(m32EepromDataValidation&EEPROM_ERR_NO_DEFECT)
        {
            EEPROM_DRV_LOG("(0x%x)No Defect \n",  m32EepromDataValidation);
        }
        else
        {
            #if EEPROM_DRV_DEFECT_EN
            if((la_pEepromData->pCameraPara!=NULL)&&(la_pEepromData->pCameraDefect!=NULL))
            {
                if(eEepromCaldata.Defect.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] != CAL_DATA_LOAD)
                {
                    la_pEepromData->pCameraDefect->Defect.PreviewSize = eEepromCaldata.Defect.PreviewSize;
                    la_pEepromData->pCameraDefect->Defect.CaptureSize = eEepromCaldata.Defect.CaptureSize;
                    memcpy((UINT8*)&la_pEepromData->pCameraDefect->Defect.PreviewTable[0],(UINT8*)&eEepromCaldata.Defect.PreviewTable[0], eEepromCaldata.Defect.PreviewSize*4);
                    memcpy((UINT8*)&la_pEepromData->pCameraDefect->Defect.CaptureTable1[0],(UINT8*)&eEepromCaldata.Defect.CaptureTable1[0], eEepromCaldata.Defect.CaptureSize*4);
                }
            }
            #else  //seanlin 120918 for 658x
            EEPROM_DRV_LOG("CHIP has no NVRAM for Defect \n");
            #endif
        }
    break;
    case CAMERA_EEPROM_DATA_PREGAIN:
        if(m32EepromDataValidation&EEPROM_ERR_NO_PREGAIN)
        {
            EEPROM_DRV_LOG("(0x%x)No pregain \n",  m32EepromDataValidation);
        }
        else
        {
#if 0//seanlin 120918 for 657x>>
            la_pEepromData->rCalGain.u4R = eEepromCaldata.Pregain.rCalGainu4R;
            la_pEepromData->rCalGain.u4G = eEepromCaldata.Pregain.rCalGainu4G;
            la_pEepromData->rCalGain.u4B = eEepromCaldata.Pregain.rCalGainu4B;
//seanlin 120918 for 657x<<
#else//seanlin 120918 for 658x>>
            la_pEepromData->rCalGain.i4R = eEepromCaldata.Pregain.rCalGainu4R; //seanlin 120713 change UINT32 as INT32
            la_pEepromData->rCalGain.i4G = eEepromCaldata.Pregain.rCalGainu4G;	//seanlin 120713 change UINT32 as INT32
            la_pEepromData->rCalGain.i4B = eEepromCaldata.Pregain.rCalGainu4B;//seanlin 120713 change UINT32 as INT32
#endif//seanlin 120918 for 658x>>
        }
    break;
    default:
        EEPROM_DRV_ERR("Unknown eeprom command 0x%x\n",a_eEepromDataType);
    break;
    }
    return m32EepromDataValidation;
}

/*******************************************************************************
*
********************************************************************************/

int
EepromDrv::Init(unsigned long u4SensorID,void *a_pEepromData)
{
    PEEPROM_DATA_STRUCT pEerom_data = &eEepromCaldata;
    PGET_SENSOR_CALIBRATION_DATA_STRUCT la_pEepromData = (PGET_SENSOR_CALIBRATION_DATA_STRUCT)a_pEepromData;
    MUINT32 i;
    EEPROM_DRV_LOG("EepromDrv::Init \n");
/*   No matter first or not, we should sync the eeprom data flag
    if( la_pEepromData->pCameraPara!= NULL)
    {
        pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD];
        pEerom_data->Defect.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD];
        for(i=0;i<EEPROM_PART_NUMBERS_COUNT;i++)
        {
            pEerom_data->Shading.ISPComm.CommReg[gulPartNumberReg[i]] = la_pEepromData->pCameraPara->ISPComm.CommReg[gulPartNumberReg[i]];
        }
        for(i=0;i<EEPROM_SENSOR_RESOLUTION_COUNT;i++)
        {
            pEerom_data->Shading.ISPComm.CommReg[gulSensorResol[i]] = la_pEepromData->pCameraPara->ISPComm.CommReg[gulSensorResol[i]];
        }
    }
*/
#ifdef EEPROM_SUPPORT  //seanlin 120919 it's for compiling mt6589fpga_ca7_ldvt
    m32EepromDataValidation= GetCameraCalData(u4SensorID,(MUINT32*)pEerom_data);
#else
    m32EepromDataValidation = EEPROM_ERR_NO_DATA|EEPROM_UNKNOWN; //seanlin 120716 Eeprom driver w.o. camera_custom_msdk.cpp
#endif

    if((&pEerom_data->Shading!=NULL&&(pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD]!=CAL_DATA_LOAD))&&(!(m32EepromDataValidation&EEPROM_ERR_NO_SHADING)))
    {
        EEPROM_DRV_LOG("CamCommShadingTableConvert()add= 0x%x\n", (unsigned int)&pEerom_data->Shading);
        CamCommShadingTableConvert((PEEPROM_SHADING_STRUCT)&pEerom_data->Shading);
    }
    if(m32EepromDataValidation == 0xFF)
    {
        m32EepromDataValidation =EEPROM_ERR_NO_DATA|EEPROM_UNKNOWN;
        EEPROM_DRV_LOG("m32EepromDataValidation= 0x%x\n", m32EepromDataValidation);
    }
    la_pEepromData->pCameraPara->ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] = pEerom_data->Shading.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] ; //sync the ISP data

    return m32EepromDataValidation;
}



