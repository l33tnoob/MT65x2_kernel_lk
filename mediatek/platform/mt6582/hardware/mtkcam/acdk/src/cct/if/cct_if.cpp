#define LOG_TAG "CCTIF"

extern "C" {
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/mtkfb.h>
}

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkLog.h"
#include "AcdkBase.h"
#include "cct_main.h"
#include "cct_calibration.h"
#include "mtkcam/hal/sensor_hal.h"
//#include "cct_ErrCode.h"
#include "AcdkErrCode.h"

#include "cct_if.h"
#include "cct_ErrCode.h"
/*
*    @CCT_IF.CPP
*    CCT_IF provides user to do camera calibration or tuning by means of
*    mixing following commands. Which involoved with
*    3A, ISP, sensor, NVRAM, Calibration, ACDK releated.
*
*    CCT FW working model is based on ACDK framework, so it is required make
*    sure ACDKBase object is available before CCTIF Open.
*
*    CCT FW presents the following APIs:
*
*    CCTIF_Open
*    CCTIF_Close
*    CCTIF_Init
*    CCTIF_DeInit
*    CCTIF_IOControl
*/



/*static*/
extern AcdkBase *g_pAcdkBaseObj;
static CCTIF *g_pCCTIFObj = NULL;
static AcdkCalibration *g_pCCTCalibrationObj = NULL;


#ifdef __cplusplus
extern "C" {
#endif

MBOOL CCTIF_Open()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);
    MINT32 err = MTRUE;

    if(!g_pAcdkBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    g_pCCTIFObj = CCTIF::createInstance();

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        err &= MFALSE;

    }

    g_pCCTCalibrationObj = new AcdkCalibration();

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n", __FUNCTION__);
        err &= MFALSE;

        g_pCCTIFObj->destroyInstance();

    }

    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return err;

}


MBOOL CCTIF_Close()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(g_pCCTIFObj)
        g_pCCTIFObj->destroyInstance();

    if(g_pCCTCalibrationObj) {
        delete g_pCCTCalibrationObj;
        g_pCCTCalibrationObj = NULL;
    }


    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return MTRUE;
}


MBOOL CCTIF_Init(MINT32 dev)
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(!g_pAcdkBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n");
        return MFALSE;
    }

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n");
        return MFALSE;
    }

    MINT32 err;//, dev = SENSOR_DEV_MAIN;

    /*
        g_pAcdkBaseObj->get_sensor_info(sensor_info);
        dev = sensor_info.dev;
    */

    err = g_pCCTIFObj->setCCTSensorDev(dev);

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] Unsupported sensor type \n", __FUNCTION__);
        return /*err*/MFALSE;
    }

    err = g_pCCTIFObj->init(dev);

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] CCTIF init fail\n", __FUNCTION__);
        return /*err*/MFALSE;
    }

    g_pCCTCalibrationObj->init(g_pAcdkBaseObj);


    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return /*err*/MTRUE;

}


MBOOL CCTIF_DeInit()
{
    ACDK_LOGD("[%s] Start\n", __FUNCTION__);

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n", __FUNCTION__);
        return MFALSE;
    }

    MINT32 err;

    err = g_pCCTIFObj->uninit();

    if(err != CCTIF_NO_ERROR) {
        ACDK_LOGE("[%s] CCTIFObj uninit fail\n", __FUNCTION__);
        return MFALSE;
    }

    err = g_pCCTCalibrationObj->uninit();

	/*
    if(err == MFALSE) {
        ACDK_LOGE("[%s] CCTCalibrationObj uninit fail\n", __FUNCTION__);
        return err;
    }
	*/

    return /*err*/MTRUE;

}

MBOOL CCTIF_FeatureCtrl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen)
{
    MBOOL err = MTRUE;
	MINT32 errID = 0;

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n");
        return MFALSE;
    }
    
    if (a_u4Ioctl >= CCT_ISP_FEATURE_START && a_u4Ioctl < CCT_ISP_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] ISP Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->ispCCTFeatureControl(a_u4Ioctl,
                                                puParaIn,
                                                u4ParaInLen,
                                                puParaOut,
                                                u4ParaOutLen,
                                                pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;

    }
    else if (a_u4Ioctl >= CCT_SENSOR_FEATURE_START && a_u4Ioctl < CCT_SENSOR_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] Sensor Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->sensorCCTFeatureControl(a_u4Ioctl,
                                                   puParaIn,
                                                   u4ParaInLen,
                                                   puParaOut,
                                                   u4ParaOutLen,
                                                   pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }
    else if (a_u4Ioctl >= CCT_NVRAM_FEATURE_START && a_u4Ioctl < CCT_NVRAM_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] NVRAM Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->nvramCCTFeatureControl(a_u4Ioctl,
                                                  puParaIn,
                                                  u4ParaInLen,
                                                  puParaOut,
                                                  u4ParaOutLen,
                                                  pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;
    }
    else if (a_u4Ioctl >= CCT_3A_FEATURE_START && a_u4Ioctl < CCT_3A_FEATURE_START + MAX_SUPPORT_CMD)
    {
        ACDK_LOGD("[%s] 3A Feature(0x%08x)\n", __FUNCTION__, a_u4Ioctl);

        errID = g_pCCTIFObj->aaaCCTFeatureControl(a_u4Ioctl,
                                                puParaIn,
                                                u4ParaInLen,
                                                puParaOut,
                                                u4ParaOutLen,
                                                pu4RealParaOutLen);
        if (errID == CCTIF_NO_ERROR) err = MTRUE;
        else err = MFALSE;          
    }
    /*else if(a_u4Ioctl > ACDK_COMMAND_START && a_u4Ioctl < ACDK_COMMAND_END)
    {
        ACDK_LOGD("[%s] Acdk cmd\n", __FUNCTION__);

        err = g_pAcdkBaseObj->sendcommand(a_u4Ioctl,
                                          puParaIn,
                                          u4ParaInLen,
                                          puParaOut,
                                          u4ParaOutLen,
                                          pu4RealParaOutLen);
    }*/
    else
    {
        ACDK_LOGD("[%s] Can't interpret CCT cmd(0x%08x)\n", __FUNCTION__, a_u4Ioctl);
        err = MFALSE;
    }

    return err;
}

MBOOL CCTIF_IOControl(MUINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo)
{
    ACDK_LOGD("[%s] CCTIF cmd = 0x%x\n", __FUNCTION__, a_u4Ioctl);

    if(!g_pAcdkBaseObj) {
        ACDK_LOGE("[%s] no AcdjBaseObj\n", __FUNCTION__);
        return MFALSE;
    }

    if(!g_pCCTIFObj) {
        ACDK_LOGE("[%s] CCTIFObj create fail\n");
        return MFALSE;
    }

    if(!g_pCCTCalibrationObj) {
        ACDK_LOGE("[%s] CCTCalibrationObj create fail\n");
        return MFALSE;
    }

    MBOOL err = MTRUE;
	MINT32 errID = 0;

    if(a_u4Ioctl >= ACDK_CCT_CDVT_START && a_u4Ioctl < ACDK_CCT_CDVT_END)
    {
        ACDK_LOGD("[%s] CCT CDVT\n", __FUNCTION__);
        errID= g_pCCTCalibrationObj->sendcommand(a_u4Ioctl,
                                                  a_prAcdkFeatureInfo->puParaIn,
                                                  a_prAcdkFeatureInfo->u4ParaInLen,
                                                  a_prAcdkFeatureInfo->puParaOut,
                                                  a_prAcdkFeatureInfo->u4ParaOutLen,
                                                  a_prAcdkFeatureInfo->pu4RealParaOutLen);
		if (errID == S_CCT_CALIBRATION_OK) err = MTRUE;
		else err = MFALSE;
    }
    else if(a_u4Ioctl == ACDK_CCT_V2_OP_SHADING_CAL)
    {
        ACDK_LOGD("[%s] CCT LSC cal\n", __FUNCTION__);
        errID = g_pCCTCalibrationObj->sendcommand(a_u4Ioctl,
                                                  a_prAcdkFeatureInfo->puParaIn,
                                                  a_prAcdkFeatureInfo->u4ParaInLen,
                                                  a_prAcdkFeatureInfo->puParaOut,
                                                  a_prAcdkFeatureInfo->u4ParaOutLen,
                                                  a_prAcdkFeatureInfo->pu4RealParaOutLen);
		if (errID == S_CCT_CALIBRATION_OK) err = MTRUE;
		else err = MFALSE;
    }
    else {

        if (a_u4Ioctl >= CCT_ISP_FEATURE_START && a_u4Ioctl < CCT_ISP_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] ISP Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->ispCCTFeatureControl(a_u4Ioctl,
                                                    a_prAcdkFeatureInfo->puParaIn,
                                                    a_prAcdkFeatureInfo->u4ParaInLen,
                                                    a_prAcdkFeatureInfo->puParaOut,
                                                    a_prAcdkFeatureInfo->u4ParaOutLen,
                                                    a_prAcdkFeatureInfo->pu4RealParaOutLen);
			if (errID == CCTIF_NO_ERROR) err = MTRUE;
			else err = MFALSE;

        }
        else if (a_u4Ioctl >= CCT_SENSOR_FEATURE_START && a_u4Ioctl < CCT_SENSOR_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] Sensor Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->sensorCCTFeatureControl(a_u4Ioctl,
                                                       a_prAcdkFeatureInfo->puParaIn,
                                                       a_prAcdkFeatureInfo->u4ParaInLen,
                                                       a_prAcdkFeatureInfo->puParaOut,
                                                       a_prAcdkFeatureInfo->u4ParaOutLen,
                                                       a_prAcdkFeatureInfo->pu4RealParaOutLen);
			if (errID == CCTIF_NO_ERROR) err = MTRUE;
			else err = MFALSE;
        }
        else if (a_u4Ioctl >= CCT_NVRAM_FEATURE_START && a_u4Ioctl < CCT_NVRAM_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] NVRAM Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->nvramCCTFeatureControl(a_u4Ioctl,
                                                      a_prAcdkFeatureInfo->puParaIn,
                                                      a_prAcdkFeatureInfo->u4ParaInLen,
                                                      a_prAcdkFeatureInfo->puParaOut,
                                                      a_prAcdkFeatureInfo->u4ParaOutLen,
                                                      a_prAcdkFeatureInfo->pu4RealParaOutLen);
			if (errID == CCTIF_NO_ERROR) err = MTRUE;
			else err = MFALSE;
        }
        else if (a_u4Ioctl >= CCT_3A_FEATURE_START && a_u4Ioctl < CCT_3A_FEATURE_START + MAX_SUPPORT_CMD)
        {
            ACDK_LOGD("[%s] 3A Feature\n", __FUNCTION__);

            errID = g_pCCTIFObj->aaaCCTFeatureControl(a_u4Ioctl,
                                                    a_prAcdkFeatureInfo->puParaIn,
                                                    a_prAcdkFeatureInfo->u4ParaInLen,
                                                    a_prAcdkFeatureInfo->puParaOut,
                                                    a_prAcdkFeatureInfo->u4ParaOutLen,
                                                    a_prAcdkFeatureInfo->pu4RealParaOutLen);
			if (errID == CCTIF_NO_ERROR) err = MTRUE;
			else err = MFALSE;			
        }
        /*else if(a_u4Ioctl > ACDK_COMMAND_START && a_u4Ioctl < ACDK_COMMAND_END)
        {
            ACDK_LOGD("[%s] Acdk cmd\n", __FUNCTION__);

            err = g_pAcdkBaseObj->sendcommand(a_u4Ioctl,
                                              a_prAcdkFeatureInfo->puParaIn,
                                              a_prAcdkFeatureInfo->u4ParaInLen,
                                              a_prAcdkFeatureInfo->puParaOut,
                                              a_prAcdkFeatureInfo->u4ParaOutLen,
                                              a_prAcdkFeatureInfo->pu4RealParaOutLen);
        }*/
        else
        {
            ACDK_LOGD("[%s] Can't interpret CCT cmd\n", __FUNCTION__);
			err = MFALSE;
        }

    }


    ACDK_LOGD("[%s] End\n", __FUNCTION__);

    return err;

}

#ifdef __cplusplus
} // extern "C"
#endif
