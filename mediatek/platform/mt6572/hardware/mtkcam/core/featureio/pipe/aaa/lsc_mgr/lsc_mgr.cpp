/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "lsc_mgr"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif


//#include <linux/cache.h>
#include <sys/prctl.h>
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_log.h>
#include <mtkcam/common.h>
#include <eeprom_drv.h>
#include <awb_param.h>
#include "lsc_mgr.h"
#include "cam_cal_drv.h"
#include "camera_custom_cam_cal.h"
#include "kd_camera_feature.h"
#include "kd_imgsensor_define.h"
#include "camera_common_calibration.h"
#include <config/PriorityDefs.h>
#include "buf_mgr.h"
#include "dbg_isp_param.h"
#include "dbg_cam_shading_param.h"
#include "CameraProfile.h"

#if ENABLE_TSF
//#define D65_IDX     (1)
#include "MTKTsf.h"
#include "tsf_tuning_custom.h"
#endif

namespace NSIspTuning {
#include "lsc_data.h"
void  *LscMgr::main       = NULL;
void  *LscMgr::mainsecond = NULL;
void  *LscMgr::sub        = NULL;
void  *LscMgr::n3d        = NULL;
ESensorDev_T LscMgr::curSensorDev = ESensorDev_Main;
// calculate tables with difference between per unit calibration and golden sample


ACDK_SCENARIO_ID_ENUM
LscMgr::
getSensorScenarioByLscScenario(ELscScenario_T lsc_scenario)
{
    MY_LOG("[%s]  ", __FUNCTION__);
    switch(lsc_scenario) {
        case LSC_SCENARIO_01:
            return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
        case LSC_SCENARIO_03:
            return ACDK_SCENARIO_ID_CAMERA_ZSD;
        case LSC_SCENARIO_04:
            return ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        case LSC_SCENARIO_09_17:
            return ACDK_SCENARIO_ID_VIDEO_PREVIEW;
        case LSC_SCENARIO_30:
            return ACDK_SCENARIO_ID_VIDEO_PREVIEW;
        case LSC_SCENARIO_37:
            return ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        default:
            return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
}

//NSIspTuning::LscMgr::ELscScenario_T
LscMgr::ELscScenario_T
LscMgr::
getLscScenarioBySensorScenario(ACDK_SCENARIO_ID_ENUM sensor_scenario) {
    switch(sensor_scenario) {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            return LSC_SCENARIO_01;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            return LSC_SCENARIO_04;
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            return LSC_SCENARIO_09_17;
        case ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            return LSC_SCENARIO_30;
        case ACDK_SCENARIO_ID_CAMERA_ZSD:
            return LSC_SCENARIO_04;
        case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
        case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
        case ACDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        default:
            return LSC_SCENARIO_04;
    }
}

MBOOL
LscMgr::
fillTblInfoByLscScenarionCT(SHADING_TBL_SPEC &tbl_sepc,
        ELscScenario_T ref_lsc,
        ELscScenario_T cur_lsc,
        MUINT8 ct,
        LSCMGR_TRANS_TYPE type)
{
    SENSOR_CROP_INFO crop_info;
    UINT32* tbl_addr;
    UINT8   LSCParamIdx = 0;
    MUINT32 sensor_scenario, ref_scenario;
    MY_LOG("[%s]  ", __FUNCTION__);

    ref_scenario = getSensorScenarioByLscScenario(ref_lsc);
    sensor_scenario = getSensorScenarioByLscScenario(cur_lsc);

    if (type == TRANS_INPUT) {
        tbl_sepc.img_width   = m_SensorCrop[sensor_scenario].u4CropW;
        tbl_sepc.img_height  = m_SensorCrop[sensor_scenario].u4CropH;
        tbl_sepc.offset_x    = 0;
        tbl_sepc.offset_y    = 0;
        tbl_sepc.crop_width  = m_SensorCrop[sensor_scenario].u4CropW;
        tbl_sepc.crop_height = m_SensorCrop[sensor_scenario].u4CropH;
        tbl_sepc.bayer       = BAYER_B;
        tbl_sepc.grid_x      = (int)m_rIspLscCfg[cur_lsc].ctl2.bits.SDBLK_XNUM+2;
        tbl_sepc.grid_y      = (int)m_rIspLscCfg[cur_lsc].ctl3.bits.SDBLK_YNUM+2;
        tbl_sepc.lwidth      = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lWIDTH;
        tbl_sepc.lheight     = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lHEIGHT;
        tbl_sepc.ratio_idx   = 0;//m_rIspLscCfg[lsc_scenario].ratio.bits.RATIO00/32;
        tbl_sepc.grgb_same   = SHADING_GRGB_SAME_NO;
#if DEBUG_ALIGN_FUNC
        tbl_sepc.table       = (MUINT32*)golden_cct;
#else
        tbl_sepc.table       = (MUINT32*)(stRawLscInfo[cur_lsc].virtAddr + (MUINT32)ct * getPerLutSize(cur_lsc));
#endif
        tbl_sepc.data_type   = SHADING_TYPE_COEFF;
    } else {

        // suppose no Sensor-side crop+scaling

        // scale down and same aspect ratio
        if (m_SensorCrop[sensor_scenario].u4CropW <= m_SensorCrop[ref_scenario].u4CropW/2) {     // scale down
            tbl_sepc.img_width   = m_SensorCrop[ref_scenario].u4CropW;
            tbl_sepc.img_height  = m_SensorCrop[ref_scenario].u4CropH;
            tbl_sepc.offset_x    = 0;
            tbl_sepc.offset_y    = 0;
            tbl_sepc.crop_width  = m_SensorCrop[ref_scenario].u4CropW;
            tbl_sepc.crop_height = m_SensorCrop[ref_scenario].u4CropH;
            MY_LOG("[%s] Scaled down", __FUNCTION__);

            if (m_SensorCrop[ref_scenario].u4CropW * m_SensorCrop[sensor_scenario].u4CropH !=
                    m_SensorCrop[sensor_scenario].u4CropW * m_SensorCrop[ref_scenario].u4CropH ) {
                MY_ERR("[%s] Not Support case, input/output scale down and different aspect ratio!!", __FUNCTION__);
            }
        } else {    // crop down, different aspect ratio
            tbl_sepc.img_width   = m_SensorCrop[ref_scenario].u4CropW;
            tbl_sepc.img_height  = m_SensorCrop[ref_scenario].u4CropH;
            tbl_sepc.offset_x    = (m_SensorCrop[ref_scenario].u4CropW - m_SensorCrop[sensor_scenario].u4CropW)/2;
            tbl_sepc.offset_y    = (m_SensorCrop[ref_scenario].u4CropH - m_SensorCrop[sensor_scenario].u4CropH)/2;
            tbl_sepc.crop_width  = m_SensorCrop[sensor_scenario].u4CropW;
            tbl_sepc.crop_height = m_SensorCrop[sensor_scenario].u4CropH;
            MY_LOG("[%s] Croped down", __FUNCTION__);
        }

        tbl_sepc.bayer       = BAYER_B;
        tbl_sepc.grid_x      = (int)m_rIspLscCfg[cur_lsc].ctl2.bits.SDBLK_XNUM+2;
        tbl_sepc.grid_y      = (int)m_rIspLscCfg[cur_lsc].ctl3.bits.SDBLK_YNUM+2;
        tbl_sepc.lwidth      = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lWIDTH;
        tbl_sepc.lheight     = m_rIspLscCfg[cur_lsc].lblock.bits.SDBLK_lHEIGHT;
        tbl_sepc.ratio_idx   = 0;//m_rIspLscCfg[lsc_scenario].ratio.bits.RATIO00/32;
        tbl_sepc.grgb_same   = SHADING_GRGB_SAME_YES;
#if DEBUG_ALIGN_FUNC
        tbl_sepc.table       = (MUINT32*)golden_cct;
#else
        tbl_sepc.table       = (MUINT32*)(stRawLscInfo[cur_lsc].virtAddr + (MUINT32)ct * getPerLutSize(cur_lsc));
#endif
        tbl_sepc.data_type   = SHADING_TYPE_COEFF;
    }
    MY_LOG("[%s]  \n"
            "img_width  = %d\n"
            "img_height = %d\n"
            "offset_x   = %d\n"
            "offset_y   = %d\n"
            "crop_width = %d\n"
            "crop_height= %d\n"
            "bayer      = %d\n"
            "grid_x     = %d\n"
            "grid_y     = %d\n"
            "lwidth     = %d\n"
            "lheight    = %d\n"
            "ratio_idx  = %d\n"
            "grgb_same  = %d\n", __FUNCTION__,
            tbl_sepc.img_width   ,
            tbl_sepc.img_height  ,
            tbl_sepc.offset_x    ,
            tbl_sepc.offset_y    ,
            tbl_sepc.crop_width  ,
            tbl_sepc.crop_height ,
            tbl_sepc.bayer       ,
            tbl_sepc.grid_x      ,
            tbl_sepc.grid_y      ,
            tbl_sepc.lwidth      ,
            tbl_sepc.lheight     ,
            tbl_sepc.ratio_idx   ,
            tbl_sepc.grgb_same
    );
    return MTRUE;
}

// when import eeprom data, calculate golen-aligned tables
MBOOL
LscMgr::
importEEPromData(void)
{

    UINT32 ret = 0;
    MINT32  SensorDevId = m_SensorDev;
    CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType = CAMERA_CAM_CAL_DATA_SHADING_TABLE;
    CAM_CAL_DATA_STRUCT cal_data;
    MUINT8 table_type = 0;
    UINT8 tbl_buf[2048];

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_1to3 = 0;
    property_get("debug.lsc_mgr.1to3", value, "-1");
    dbg_1to3 = atoi(value);

    if (dbg_1to3 != -1) {
        MY_LOG("[LscMgr:%s] skip 1to3 %d", __FUNCTION__,
                dbg_1to3);
        return MTRUE;
    }

    MY_LOG("[LscMgr] %s \n", __FUNCTION__);

    if (m_bIsEEPROMImported == MTRUE)
        return MTRUE;

    switch(m_eActive) {
        case ESensorDev_Main      :
            SensorDevId = SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub       :
            SensorDevId = SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            SensorDevId = SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_Main3D    :
            SensorDevId = SENSOR_DEV_MAIN_3D;
            break;
        default:
            SensorDevId = SENSOR_DEV_MAIN;
            break;
    }
    MY_LOG("[%s]  GetCamCalCalData", __FUNCTION__);
    CamCalDrvBase *m_pCamCalDrvBaseObj = CamCalDrvBase::createInstance();
    if (!m_pCamCalDrvBaseObj) {
        MY_LOG("[%s] CamCalDrvBase is NULL", __FUNCTION__);
        return MFALSE;
    }

#if DEBUG_ALIGN_FUNC
#else
    ret = m_pCamCalDrvBaseObj->GetCamCalCalData(SensorDevId, a_eCamCalDataType, &cal_data);
    MY_LOG("[%s] (0x%8x)=m_pCamCalDrvObj->GetCamCalCalData", __FUNCTION__, ret);
    if(ret&CamCalReturnErr[a_eCamCalDataType])
    {
        MY_LOG("[%s] err (%s)", __FUNCTION__,
                CamCalErrString[a_eCamCalDataType]);
        m_bIsEEPROMImported = MTRUE;
        return MFALSE;
    }
    else
    {
        MY_LOG("[%s] NO err (%s)", __FUNCTION__,
                CamCalErrString[a_eCamCalDataType]);
    }
#endif

    // (1) table type
    MUINT32 Rotation[2] = {0, 0};
    CAM_CAL_DATA_VER_ENUM module_type = cal_data.DataVer;
    CAM_CAL_LSC_DATA            *lsc_data[2] = {0, 0};
    CAM_CAL_LSC_SENSOR_TYPE     *sensor_lsc = NULL;
    CAM_CAL_LSC_MTK_TYPE        *mtk_lsc = NULL;

    MY_LOG("[%s]  module_type %d", __FUNCTION__, module_type);
    switch(module_type) {
        case CAM_CAL_SINGLE_EEPROM_DATA:
            MY_LOG("[%s]  CAM_CAL_SINGLE_EEPROM_DATA", __FUNCTION__);
        case CAM_CAL_SINGLE_OTP_DATA:
            MY_LOG("[%s]  CAM_CAL_SINGLE_OTP_DATA", __FUNCTION__);
            lsc_data[0] = &cal_data.SingleLsc.LscTable;
            Rotation[0] = cal_data.SingleLsc.TableRotation;
            break;
        default:
        case CAM_CAL_N3D_DATA:
            MY_LOG("[%s]  CAM_CAL_N3D_DATA or nuknown", __FUNCTION__);
            lsc_data[0] = &cal_data.N3DLsc.Data[0].LscTable;
            lsc_data[1] = &cal_data.N3DLsc.Data[1].LscTable;
            Rotation[0] = cal_data.N3DLsc.Data[0].TableRotation;
            Rotation[1] = cal_data.N3DLsc.Data[1].TableRotation;
            break;
    }
    table_type = lsc_data[0]->MtkLcsData.MtkLscType;

#if DEBUG_ALIGN_FUNC
    {
        ISP_NVRAM_LSC_T eeprom, golden;
        SHADIND_ALIGN_CONF  align_conf;
        ACDK_SCENARIO_ID_ENUM cali_scenario = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        MY_LOG("[%s]  PerUnit mtk lsc, type %d, Rotation (%d, %d)", __FUNCTION__,
                module_type,
                Rotation[0], Rotation[1]);

        char *gWorkinBuffer   = new char[SHADIND_FUNC_WORKING_BUFFER_SIZE]; // 139,116 (int) = 556,464 (bytes)
        if (!gWorkinBuffer)
        {
            MY_LOG("[LscMgr:%s]  gWorkinBuffer is NULL", __FUNCTION__);
            m_bIsEEPROMImported = MTRUE;
            return MFALSE;
        }

        align_conf.working_buff_addr = (void*)gWorkinBuffer;
        align_conf.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;


        UINT32* align_out_coef      = new UINT32 [MAX_TIL_COEFF_SIZE/sizeof(UINT32)];
        UINT32* trfm_out_coef       = new UINT32 [MAX_TIL_COEFF_SIZE/sizeof(UINT32)];

        //=============== cali ==================//
        align_conf.cali.img_width    = 2564;
        align_conf.cali.img_height   = 1926;
        align_conf.cali.offset_x     = 0;
        align_conf.cali.offset_y     = 0;
        align_conf.cali.crop_width   = 2564;
        align_conf.cali.crop_height  = 1926;
        align_conf.cali.bayer        = BAYER_B;
        align_conf.cali.grid_x       = 16;
        align_conf.cali.grid_y       = 16;
        align_conf.cali.lwidth       = 0; //doesn't use
        align_conf.cali.lheight      = 0; //doesn't use
        align_conf.cali.ratio_idx    = 0;
        align_conf.cali.grgb_same    = SHADING_GRGB_SAME_NO;
        align_conf.cali.data_type    = SHADING_TYPE_GAIN;
        align_conf.cali.table       = (UINT32*)per_unit_dnp;


        //=============== golden ==================//
        align_conf.golden.img_width      = 2564;
        align_conf.golden.img_height     = 1926;
        align_conf.golden.offset_x       = 0;
        align_conf.golden.offset_y       = 0;
        align_conf.golden.crop_width     = 2564;
        align_conf.golden.crop_height    = 1926;
        align_conf.golden.bayer          = BAYER_R;
        align_conf.golden.grid_x         = 16;
        align_conf.golden.grid_y         = 16;
        align_conf.golden.lwidth         = 0; //doesn't use
        align_conf.golden.lheight        = 0; //doesn't use
        align_conf.golden.ratio_idx      = 0;
        align_conf.golden.grgb_same      = SHADING_GRGB_SAME_NO;
        align_conf.golden.data_type      = SHADING_TYPE_GAIN;
        align_conf.golden.table       = (UINT32*)golden_dnp;

        //=============== input ==================//
        align_conf.input.img_width       = 2564;
        align_conf.input.img_height      = 1926;
        align_conf.input.offset_x        = 0;
        align_conf.input.offset_y        = 0;
        align_conf.input.crop_width      = 2564;
        align_conf.input.crop_height     = 1926;
        align_conf.input.bayer           = BAYER_R;
        align_conf.input.grid_x          = 16;
        align_conf.input.grid_y          = 16;
        align_conf.input.lwidth          = 0; //doesn't use
        align_conf.input.lheight         = 0; //doesn't use
        align_conf.input.ratio_idx       = 0;
        align_conf.input.grgb_same       = SHADING_GRGB_SAME_NO;
        align_conf.input.data_type       = SHADING_TYPE_COEFF;
        align_conf.input.table           = (UINT32*)golden_cct;

        //=============== output ==================//
        align_conf.output.img_width      = 2564;
        align_conf.output.img_height     = 1926;
        align_conf.output.offset_x       = 0;
        align_conf.output.offset_y       = 0;
        align_conf.output.crop_width     = 2564;
        align_conf.output.crop_height    = 1926;
        align_conf.output.bayer          = BAYER_R;
        align_conf.output.grid_x         = 16;
        align_conf.output.grid_y         = 16;
        align_conf.output.lwidth         = 0; //doesn't use
        align_conf.output.lheight        = 0; //doesn't use
        align_conf.output.ratio_idx      = 4;
        align_conf.output.grgb_same      = SHADING_GRGB_SAME_YES;
        align_conf.output.data_type      = SHADING_TYPE_COEFF;
        align_conf.output.table          = (UINT32*)align_out_coef;

        MY_LOG("[LscMgr:%s]  shading_align_golden", __FUNCTION__);
        if (ret != shading_align_golden(align_conf))
            MY_ERR("[%s] Fail shading_align_golden %d", __FUNCTION__,
                    ret);
        // chose the right LSC scenario to apply
        for (int i = 0; i < sizeof(align_result)/sizeof(align_result[0]); i++)
        {
            if (align_result[i] != align_out_coef[i])
            {
                MY_ERR("[%s] idx %d, (result, out) = (0x%08x, 0x%08x)", __FUNCTION__,
                        i, align_result[i], align_out_coef[i]);
            } else
                MY_LOG("[%s] OK idx %d, (result, out) = (0x%08x, 0x%08x)", __FUNCTION__,
                                        i, align_result[i], align_out_coef[i]);
        }
        MY_LOG("[%s] ALIGN_CHECK done!!", __FUNCTION__);

    }
#else
    if (table_type & (1<<0))          // sensor lsc
    {
        SET_SENSOR_CALIBRATION_DATA_STRUCT scali_struct;
        //        typedef struct {
        //            MUINT8      MtkLscType; //LSC Table type    "[0]sensor[1]MTK"   1
        //            MUINT8      PixId; //0,1,2,3: B,Gb,Gr,R
        //            MUINT16     TableSize; //TABLE SIZE      2
        //            MUINT8      SensorTable[MAX_SENSOR_SHADING_TALE_SIZE]; //LSC Data (Max 2048)        2048
        //            MUINT8      Reserve[CAM_CAL_MAX_LSC_SIZE-sizeof(MUINT8)-sizeof(MUINT8)-sizeof(MUINT16)-(sizeof(MUINT8)*MAX_SENSOR_SHADING_TALE_SIZE)]; //
        //        }CAM_CAL_LSC_SENSOR_TYPE;
        MY_LOG("[%s]  sensor lsc", __FUNCTION__);
        scali_struct.DataFormat = 0x00010001;
        scali_struct.DataSize = lsc_data[0]->SensorLcsData.TableSize;

        if (MAX_SHADING_DATA_TBL >= scali_struct.DataSize)
        {
            memcpy(&scali_struct.ShadingData,
                    &lsc_data[0]->SensorLcsData.SensorTable,
                    scali_struct.DataSize);
        }
        else
        {
            MY_ERR("Max:%d, scali_struct.DataSize is %d",
                    MAX_SHADING_DATA_TBL,
                    scali_struct.DataSize);
            m_bIsEEPROMImported = MTRUE;
            memcpy(&scali_struct.ShadingData,
                    &lsc_data[0]->SensorLcsData.SensorTable,
                    MAX_SHADING_DATA_TBL);
            //return MFALSE;
        }

        if (!m_pSensorHal)
        {
            MY_ERR("m_pSensorHal is NULL");
            m_pSensorHal = SensorHal::createInstance();
            if (m_pSensorHal->init())
            {
                MY_ERR("m_pSensorHal re-instanate fail!!");
                m_pSensorHal->destroyInstance();
                m_pSensorHal = NULL;
                return MFALSE;
            }
        }

        m_pSensorHal->sendCommand(m_SensorDev, (int)SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,
                (int)(&scali_struct),
                0, 0);
        m_bIsEEPROMImported = MTRUE;
        return MTRUE;
    }
    else if (table_type & (1<<1))   // mtk lsc
    {

        ISP_NVRAM_LSC_T eeprom, golden;
        SHADIND_ALIGN_CONF  align_conf;
        ACDK_SCENARIO_ID_ENUM cali_scenario = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
        MY_LOG("[%s]  PerUnit mtk lsc, type %d, Rotation (%d, %d)", __FUNCTION__,
                module_type,
                Rotation[0], Rotation[1]);

        char *gWorkinBuffer   = new char[SHADIND_FUNC_WORKING_BUFFER_SIZE]; // 139,116 (int) = 556,464 (bytes)
        if (!gWorkinBuffer)
        {
            MY_ERR("[LscMgr:%s]  gWorkinBuffer is NULL");
            m_bIsEEPROMImported = MTRUE;
            return MFALSE;
        }

        align_conf.working_buff_addr = (void*)gWorkinBuffer;
        align_conf.working_buff_size = SHADIND_FUNC_WORKING_BUFFER_SIZE;

        BAYER_ID_T bayer = BAYER_B;
        mtk_lsc = &lsc_data[0]->MtkLcsData;
        // get pixel id
        switch(mtk_lsc->PixId)   //0,1,2,3: B,Gb,Gr,R
        {
            case 1:
                bayer = BAYER_B;
                break;
            case 2:
                bayer = BAYER_GB;
                break;
            case 4:
                bayer = BAYER_GR;
                break;
            case 8:
                bayer = BAYER_R;
                break;
        }

        eeprom.ctl2.bits.SDBLK_XNUM     = ((mtk_lsc->CapIspReg[1] >> 28) & 0x0000000F);
        eeprom.ctl3.bits.SDBLK_YNUM     = ((mtk_lsc->CapIspReg[1] >> 12) & 0x0000000F);
        eeprom.ctl2.bits.SDBLK_WIDTH    = ((mtk_lsc->CapIspReg[1] >> 16) & 0x00000fFF);
        eeprom.ctl3.bits.SDBLK_HEIGHT   =  (mtk_lsc->CapIspReg[1]    & 0x00000fFF);
        eeprom.lblock.bits.SDBLK_lWIDTH = ((mtk_lsc->CapIspReg[3] >> 16) & 0x00000fFF);
        eeprom.lblock.bits.SDBLK_lHEIGHT=  (mtk_lsc->CapIspReg[3]        & 0x00000fFF);
        eeprom.ratio.val                = (mtk_lsc->CapIspReg[4]);

        MY_LOG("[%s:eeprom] XNUM, YNUM, WIDTH, HEIGHT, LWIDTH, LHEIGHT\n (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", __FUNCTION__,
                eeprom.ctl2.bits.SDBLK_XNUM,
                eeprom.ctl3.bits.SDBLK_YNUM ,
                eeprom.ctl2.bits.SDBLK_WIDTH ,
                eeprom.ctl3.bits.SDBLK_HEIGHT,
                eeprom.lblock.bits.SDBLK_lWIDTH,
                eeprom.lblock.bits.SDBLK_lHEIGHT);

        if (eeprom.ctl2.bits.SDBLK_WIDTH == 0 || eeprom.ctl3.bits.SDBLK_HEIGHT == 0)
        {
            MY_ERR("[%s:eeprom] Calibration data is not correct!");
            m_bIsEEPROMImported = MTRUE;
            return MTRUE;
        }

        golden.ctl2.bits.SDBLK_XNUM         = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 28) & 0x0000000F);
        golden.ctl3.bits.SDBLK_YNUM         = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 12) & 0x0000000F);
        golden.ctl2.bits.SDBLK_WIDTH        = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1] >> 16) & 0x00000fFF);
        golden.ctl3.bits.SDBLK_HEIGHT       =  (m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[1]    & 0x00000fFF);
        golden.lblock.bits.SDBLK_lWIDTH     = ((m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[3] >> 16) & 0x00000fFF);
        golden.lblock.bits.SDBLK_lHEIGHT    =  (m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[3]        & 0x00000fFF);
        golden.ratio.val                    = m_rIspShadingLut.SensorGoldenCalTable.IspLSCReg[4];

        ///////////// DEBUG
        mtk_lsc->CaptureTblSize = ((MUINT32)eeprom.ctl2.bits.SDBLK_XNUM+2)*((MUINT32)eeprom.ctl3.bits.SDBLK_YNUM+2)*2;
        // backup tbl
        memcpy((void*)tbl_buf, (void*)mtk_lsc->CapTable, mtk_lsc->CaptureTblSize*4);

//        UINT32 *pUnitTbl = (UINT32*)mtk_lsc->CapTable;
//        for (MUINT32 idx = 0; idx < mtk_lsc->CaptureTblSize; idx += 4) {
//            MY_LOG("[%s:unit] Check GainUnitTbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                    *(pUnitTbl+idx), *(pUnitTbl+idx+1), *(pUnitTbl+idx+2), *(pUnitTbl+idx+3));
//        }

        if (golden.ctl2.bits.SDBLK_WIDTH == 0 || golden.ctl3.bits.SDBLK_HEIGHT == 0)
        {
            MY_ERR("[%s:golden] No golden setting, using eeprom setting instead", __FUNCTION__);

            golden.ctl2.bits.SDBLK_XNUM         = eeprom.ctl2.bits.SDBLK_XNUM     ;
            golden.ctl3.bits.SDBLK_YNUM         = eeprom.ctl3.bits.SDBLK_YNUM     ;
            golden.ctl2.bits.SDBLK_WIDTH        = eeprom.ctl2.bits.SDBLK_WIDTH    ;
            golden.ctl3.bits.SDBLK_HEIGHT       = eeprom.ctl3.bits.SDBLK_HEIGHT   ;
            golden.lblock.bits.SDBLK_lWIDTH     = eeprom.lblock.bits.SDBLK_lWIDTH ;
            golden.lblock.bits.SDBLK_lHEIGHT    = eeprom.lblock.bits.SDBLK_lHEIGHT;
            golden.ratio.val                    = eeprom.ratio.val;
            MUINT32 size = mtk_lsc->CaptureTblSize*4;//mtk_lsc->TableSize;
#if 1
            UINT32 *pGoldenTbl = (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable;
            for (MUINT32 idx = 0; idx < 32; idx += 4) {
                MY_ERR("[golden] GoldenTbl 0x%08x/0x%08x/0x%08x/0x%08x",
                        *(pGoldenTbl+idx), *(pGoldenTbl+idx+1), *(pGoldenTbl+idx+2), *(pGoldenTbl+idx+3));
            }
            memcpy((void *)m_rIspShadingLut.SensorGoldenCalTable.GainTable, (void*)mtk_lsc->CapTable, size);//mtk_lsc->CaptureTblSize);
#else
            MUINT32 debug_golden[] =
            {
                    0x5a2952c8,    0x5bca589e,    0x4a5144c9,    0x4a6149be,
                    0x3eab39f5,    0x3e643e26,    0x37483466,    0x37773745,
                    0x37a03454,    0x371a377f,    0x3e553a59,    0x3d923e30,
                    0x49d54506,    0x48e34938,    0x59425343,    0x585b5836,
                    0x4f4e4986,    0x4f714e97,    0x3de039e5,    0x3d403d62,
                    0x308e2df8,    0x2fb0304f,    0x29d2287f,    0x29642998,
                    0x29c0282d,    0x28a92977,    0x30162de7,    0x2eec2fd2,
                    0x3d5d3a1a,    0x3b8f3d28,    0x4f194b3b,    0x4e3b4e6c,
                    0x47b5434b,    0x48014720,    0x36d43357,    0x362d361e,
                    0x298027c6,    0x28d92948,    0x221321b6,    0x2228220b,
                    0x219f2164,    0x2185219b,    0x28a82776,    0x27e728be,
                    0x35cc33a7,    0x35113580,    0x47774418,    0x46f14700,
                    0x483843a4,    0x48d647b1,    0x36ea33ba,    0x366b3676,
                    0x2992283d,    0x2943297f,    0x226021fe,    0x226f225a,
                    0x220021cd,    0x21bf221e,    0x28f627f2,    0x285c28ff,
                    0x35ff3423,    0x357f35eb,    0x47f94492,    0x48084802,
                    0x50394aed,    0x50f25030,    0x3f4a3b61,    0x3e8a3eda,
                    0x31402e6e,    0x30403130,    0x2a672924,    0x2a062a75,
                    0x2a1d2927,    0x29112a05,    0x30a12ed4,    0x2fb030cf,
                    0x3e4a3b65,    0x3d593e33,    0x51234ccc,    0x5085505e,
                    0x5b925568,    0x5d5b5bd5,    0x4d1947f3,    0x4c974c7a,
                    0x3feb3c77,    0x3fc73fe3,    0x38ac3635,    0x394238db,
                    0x37fc35d0,    0x38c0388e,    0x3eaa3bac,    0x3ef03ee5,
                    0x4b5446ec,    0x4b054add,    0x597e550c,    0x5988593c};
            memcpy((void *)m_rIspShadingLut.SensorGoldenCalTable.GainTable, (void*)debug_golden, sizeof(debug_golden));//mtk_lsc->CaptureTblSize);
#endif
        }

        MY_LOG("[%s:golden] XNUM, YNUM, WIDTH, HEIGHT, LWIDTH, LHEIGHT\n (0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)", __FUNCTION__,
                golden.ctl2.bits.SDBLK_XNUM     ,
                golden.ctl3.bits.SDBLK_YNUM     ,
                golden.ctl2.bits.SDBLK_WIDTH    ,
                golden.ctl3.bits.SDBLK_HEIGHT   ,
                golden.lblock.bits.SDBLK_lWIDTH ,
                golden.lblock.bits.SDBLK_lHEIGHT);

        {
            UINT32 *pUnitTbl = (UINT32*)mtk_lsc->CapTable;
            UINT32 *pGoldenTbl = (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable;
            UINT32 size = mtk_lsc->CaptureTblSize*4;
            MY_LOG("[%s:byte] Check GainUnitTbl [%d~%d] 0x%1x/0x%1x/0x%1x/0x%1x", __FUNCTION__,
                    size-4,size-1,
                    mtk_lsc->CapTable[size-4], mtk_lsc->CapTable[size-3],
                    mtk_lsc->CapTable[size-2], mtk_lsc->CapTable[size-1]);
            MY_LOG("[%s:unit] Check GainUnitTbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                    *(pUnitTbl), *(pUnitTbl+1), *(pUnitTbl+2), *(pUnitTbl+3));
            MY_LOG("[%s:golden] Check GainGoldenTbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                    *(pGoldenTbl), *(pGoldenTbl+1), *(pGoldenTbl+2), *(pGoldenTbl+3));

//            for (MUINT32 idx = 0; idx < mtk_lsc->CaptureTblSize; idx += 4) {
//                MY_LOG("[%s:golden] Check GainGoldenTbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                        *(pGoldenTbl+idx), *(pGoldenTbl+idx+1), *(pGoldenTbl+idx+2), *(pGoldenTbl+idx+3));
//            }

        }

        align_conf.cali.img_width   = m_SensorCrop[cali_scenario].u4CropW;//m_SensorCrop[cali_scenario].u4SrcW;
        align_conf.cali.img_height  = m_SensorCrop[cali_scenario].u4CropH;//m_SensorCrop[cali_scenario].u4SrcH;
        align_conf.cali.offset_x    = 0;//m_SensorCrop[cali_scenario].u4GrabX;
        align_conf.cali.offset_y    = 0;//m_SensorCrop[cali_scenario].u4GrabY;
        align_conf.cali.crop_width  = m_SensorCrop[cali_scenario].u4CropW;
        align_conf.cali.crop_height = m_SensorCrop[cali_scenario].u4CropH;
        align_conf.cali.bayer       = bayer;
        align_conf.cali.grid_x      = (int)eeprom.ctl2.bits.SDBLK_XNUM+2;
        align_conf.cali.grid_y      = (int)eeprom.ctl3.bits.SDBLK_YNUM+2;
        align_conf.cali.lwidth      = eeprom.lblock.bits.SDBLK_lWIDTH;
        align_conf.cali.lheight     = eeprom.lblock.bits.SDBLK_lHEIGHT;
        align_conf.cali.ratio_idx   = 0;
        align_conf.cali.grgb_same   = SHADING_GRGB_SAME_NO;
        align_conf.cali.table       = (UINT32*)mtk_lsc->CapTable;
        align_conf.cali.data_type   = SHADING_TYPE_GAIN;

        MY_LOG("[%s]  align_conf.cali \n"
                "img_width  = %d\n"
                "img_height = %d\n"
                "offset_x   = %d\n"
                "offset_y   = %d\n"
                "crop_width = %d\n"
                "crop_height= %d\n"
                "bayer      = %d\n"
                "grid_x     = %d\n"
                "grid_y     = %d\n"
                "lwidth     = %d\n"
                "lheight    = %d\n"
                "ratio_idx  = %d\n"
                "grgb_same  = %d\n"
                "table      = 0x%08x\n", __FUNCTION__,
                align_conf.cali.img_width   ,
                align_conf.cali.img_height  ,
                align_conf.cali.offset_x    ,
                align_conf.cali.offset_y    ,
                align_conf.cali.crop_width  ,
                align_conf.cali.crop_height ,
                align_conf.cali.bayer       ,
                align_conf.cali.grid_x      ,
                align_conf.cali.grid_y      ,
                align_conf.cali.lwidth      ,
                align_conf.cali.lheight     ,
                align_conf.cali.ratio_idx   ,
                align_conf.cali.grgb_same,
                align_conf.cali.table
        );

        align_conf.golden.img_width   = m_SensorCrop[cali_scenario].u4CropW;//m_SensorCrop[cali_scenario].u4SrcW;
        align_conf.golden.img_height  = m_SensorCrop[cali_scenario].u4CropH;//m_SensorCrop[cali_scenario].u4SrcH;
        align_conf.golden.offset_x    = 0;//m_SensorCrop[cali_scenario].u4GrabX;
        align_conf.golden.offset_y    = 0;//m_SensorCrop[cali_scenario].u4GrabY;
        align_conf.golden.crop_width  = m_SensorCrop[cali_scenario].u4CropW;
        align_conf.golden.crop_height = m_SensorCrop[cali_scenario].u4CropH;
        align_conf.golden.bayer       = BAYER_B;
        align_conf.golden.grid_x      = (int)golden.ctl2.bits.SDBLK_XNUM+2;
        align_conf.golden.grid_y      = (int)golden.ctl3.bits.SDBLK_YNUM+2;
        align_conf.golden.lwidth      = golden.lblock.bits.SDBLK_lWIDTH;
        align_conf.golden.lheight     = golden.lblock.bits.SDBLK_lHEIGHT;
        align_conf.golden.ratio_idx   = 0;
        align_conf.golden.grgb_same   = SHADING_GRGB_SAME_NO;
        align_conf.golden.table       = (UINT32*)m_rIspShadingLut.SensorGoldenCalTable.GainTable;
        align_conf.golden.data_type   = SHADING_TYPE_GAIN;


        MY_LOG("[%s]  align_conf.golden\n"
                "img_width  = %d\n"
                "img_height = %d\n"
                "offset_x   = %d\n"
                "offset_y   = %d\n"
                "crop_width = %d\n"
                "crop_height= %d\n"
                "bayer      = %d\n"
                "grid_x     = %d\n"
                "grid_y     = %d\n"
                "lwidth     = %d\n"
                "lheight    = %d\n"
                "ratio_idx  = %d\n"
                "grgb_same  = %d\n"
                "table      = 0x%08x\n", __FUNCTION__,
                align_conf.golden.img_width   ,
                align_conf.golden.img_height  ,
                align_conf.golden.offset_x    ,
                align_conf.golden.offset_y    ,
                align_conf.golden.crop_width  ,
                align_conf.golden.crop_height ,
                align_conf.golden.bayer       ,
                align_conf.golden.grid_x      ,
                align_conf.golden.grid_y      ,
                align_conf.golden.lwidth      ,
                align_conf.golden.lheight     ,
                align_conf.golden.ratio_idx   ,
                align_conf.golden.grgb_same,
                align_conf.golden.table
        );

#if 1   // One color temperature only
        //Step (1) process LSC_SCENARIO_04 (cap tile) first
        // chose the right LSC scenario to apply
        LSC_RESULT ret = S_LSC_CONVERT_OK;
        int lsc_scenario = LSC_SCENARIO_04;
        MUINT8 ct_idx = 0;

        MY_LOG("[%s] start shading_align_golden", __FUNCTION__);
        //for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM && ret == S_LSC_CONVERT_OK; ct_idx++)
        for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM; ct_idx++)
        {
            memcpy((void*)mtk_lsc->CapTable, (void*)tbl_buf, mtk_lsc->CaptureTblSize*4); // restore
//            UINT32 *pUnitTbl = (UINT32*)mtk_lsc->CapTable;
//            for (MUINT32 idx = 0; idx < mtk_lsc->CaptureTblSize; idx += 4) {
//                MY_LOG("[%s:unit] Check GainUnitTbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                        *(pUnitTbl+idx), *(pUnitTbl+idx+1), *(pUnitTbl+idx+2), *(pUnitTbl+idx+3));
//            }
//            {
//                UINT32 *pTbl = (MUINT32*)((MUINT32)getLut(LSC_SCENARIO_04) + (MUINT32)0 * getPerLutSize(LSC_SCENARIO_04));
//                for (MUINT32 idx = 0; idx < 16; idx += 4) {
//                    MY_LOG("[%s:ct0 orig tbl] Check ct0 tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                            *(pTbl+idx), *(pTbl+idx+1), *(pTbl+idx+2), *(pTbl+idx+3));
//                }
//            }

            fillTblInfoByLscScenarionCT(align_conf.input,
                    LSC_SCENARIO_04,
                    (ELscScenario_T)lsc_scenario, ct_idx, TRANS_INPUT);
            fillTblInfoByLscScenarionCT(align_conf.output,
                    LSC_SCENARIO_04,
                    (ELscScenario_T)lsc_scenario, ct_idx, TRANS_OUTPUT);

            ret = shading_align_golden(align_conf);

            if (S_LSC_CONVERT_OK != ret)
            {
                MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d\n",
                        __FUNCTION__,
                        lsc_scenario,
                        ct_idx);
            } else {
                MY_LOG("[%s] align ct_idx %d done!", __FUNCTION__, ct_idx);
//                {
//                    UINT32 *pTbl = (MUINT32*)align_conf.output.table;
//                    for (MUINT32 idx = 0; idx < 16; idx += 4) {
//                        MY_LOG("[%s:Output tbl] Check ct%d tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                ct_idx,
//                                *(pTbl+idx), *(pTbl+idx+1), *(pTbl+idx+2), *(pTbl+idx+3));
//                    }
//                }
//                {
//                    char filename[128];
//                    sprintf(filename, "/sdcard/lsc1to3data/Cap1to3_ct%d.bin", ct_idx);
//                    MY_LOG("[LscMgr:%s] DBG: Output Capture Table to %s", __FUNCTION__, filename);
//                    FILE* fpdebug = fopen(filename,"wb");
//                    if ( fpdebug == NULL )
//                    {
//                        MY_ERR("Can't open :%s\n",filename);
//                    } else {
//                        fwrite(align_conf.output.table,
//                                getPerLutSize((ELscScenario_T)lsc_scenario),
//                                1,fpdebug);
//                        fclose(fpdebug);
//                    }
//                }
            }
        }

        // Step (2), transform LSC_SCENARIO_04 to other scenario
        SHADIND_TRFM_CONF trfm;
        //******************* Transform Test **********************/
        //======= working buffer allocation ===============//
        trfm.working_buff_addr  = gWorkinBuffer;
        trfm.working_buff_size  = SHADIND_FUNC_WORKING_BUFFER_SIZE;
        trfm.afn = SHADING_AFN_R0D;
        // Process shading_transform
        MY_LOG("[%s] start shading_transform", __FUNCTION__);


        for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM; ct_idx++)
        {
            UINT32 *pTbl = (MUINT32*)(stRawLscInfo[LSC_SCENARIO_04].virtAddr + (MUINT32)ct_idx * getPerLutSize(LSC_SCENARIO_04));
            UINT32 TblSize = getPerLutSize(LSC_SCENARIO_04);
            UINT8 BackupTbl[TblSize];
            // backup input table
            memcpy((void*)BackupTbl, (void*)pTbl, TblSize);

            fillTblInfoByLscScenarionCT(trfm.input, LSC_SCENARIO_04, LSC_SCENARIO_04, ct_idx, TRANS_INPUT);

            for (lsc_scenario = 0; lsc_scenario < LSC_SCENARIO_30; lsc_scenario++)
            {
                if (lsc_scenario != LSC_SCENARIO_04) {
                    MY_LOG("[%s] transform from LSC_SCENARIO_04 to %d, ct_idx %d start", __FUNCTION__,
                            lsc_scenario, ct_idx);
                    fillTblInfoByLscScenarionCT(trfm.output, LSC_SCENARIO_04,
                            (ELscScenario_T)lsc_scenario, ct_idx, TRANS_OUTPUT);

                    ret = shading_transform(trfm);
                    // restore input table
                    memcpy((void*)trfm.input.table, (void*)BackupTbl, TblSize);
                    if (ret != S_LSC_CONVERT_OK)
                    {
                        MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d",
                                __FUNCTION__,
                                lsc_scenario,
                                ct_idx);
                    } else {
                        MY_LOG("[%s] transform from LSC_SCENARIO_04 to %d, ct_idx %d done", __FUNCTION__,
                                lsc_scenario, ct_idx);
//                        UINT32 *pOutTbl = (UINT32*)trfm.output.table;
//                        UINT32 *pInTbl = (UINT32*)trfm.input.table;
//                        for (MUINT32 idx = 0; idx < 64; idx += 4) {
//                            MY_LOG("[%s] Check In Tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                    *(pInTbl+idx), *(pInTbl+idx+1), *(pInTbl+idx+2), *(pInTbl+idx+3));
//                        }
//                        for (MUINT32 idx = 0; idx < 64; idx += 4) {
//                            MY_LOG("[%s] Check Out Tbl 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
//                                    *(pOutTbl+idx), *(pOutTbl+idx+1), *(pOutTbl+idx+2), *(pOutTbl+idx+3));
//                        }
//                        {
//                            char filename[128];
//                            sprintf(filename, "/sdcard/lsc1to3data/Trans_lsc%dct%d.bin", lsc_scenario, ct_idx);
//                            MY_LOG("[LscMgr:%s] DBG: Output  Table to %s", __FUNCTION__, filename);
//                            FILE* fpdebug = fopen(filename,"wb");
//                            if ( fpdebug == NULL )
//                            {
//                                MY_ERR("Can't open :%s\n",filename);
//                            } else {
//                                fwrite(trfm.output.table,
//                                        getPerLutSize((ELscScenario_T)lsc_scenario),
//                                        1,fpdebug);
//                                fclose(fpdebug);
//                            }
//                        }
                    }
                }
            }
        }
        MY_LOG("[%s] shading_transform DONE!!", __FUNCTION__);
#else
        // chose the right LSC scenario to apply
        LSC_RESULT ret = S_LSC_CONVERT_OK;
        int lsc_scenario = 0;
        MUINT8 ct_idx = 0;
        for (lsc_scenario = 0; lsc_scenario < LSC_SCENARIO_NUM && ret == S_LSC_CONVERT_OK; lsc_scenario++)
        {
            for (ct_idx = 0; ct_idx < SHADING_SUPPORT_CT_NUM && ret == S_LSC_CONVERT_OK; ct_idx++)
            {
                fillTblInfoByLscScenarionCT(align_conf.input, (ELscScenario_T)lsc_scenario, ct_idx);
                memcpy(&align_conf.output, &align_conf.input, sizeof(align_conf.input));
                ret = shading_align_golden(align_conf);

                if (S_LSC_CONVERT_OK != ret)
                {
                    MY_ERR("[%s] shading_align_golden error, lsc_scenario: %d, ct_idx: %d",
                            lsc_scenario,
                            ct_idx);
                }
            }
        }
#endif

        delete [] gWorkinBuffer;
        gWorkinBuffer = NULL;
    }
#endif
    RawLscTblFlushCurrTbl();
    m_bIsEEPROMImported = MTRUE;
    return MTRUE;
}


MBOOL
LscMgr::
ConfigUpdate() {
    MUINT8 idx = 0;
    ACDK_SCENARIO_ID_ENUM scenario;

    MY_LOG("[%s] !", __FUNCTION__);

    if (m_bIsLutLoaded == MFALSE)
    {
        MY_ERR("[%s] Lut not loaded yet!", __FUNCTION__);
        return MFALSE;
    }

#if USING_BUILTIN_LSC
    MY_LOG("[LscMgr] %s USING_BUILTIN_LSC", __FUNCTION__);


    // LSC scenario and Sensor scenario mapping
    for (idx = 0; idx < LSC_SCENARIO_NUM; idx++) {
        scenario = getSensorScenarioByLscScenario((ELscScenario_T)idx);
        getScenarioResolution(scenario);

        // debug
        m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN = 1;
        m_rIspLscCfg[idx].lsc_en.bits.LSC_EN = 1;
        m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR = stRawLscInfo[idx].phyAddr;
        m_rIspLscCfg[idx].xsize.bits.XSIZE  = 0;

        m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST = 0;
        m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST = 0;
        m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM = 15;
        m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM = 15;

        m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH =
                m_SensorCrop[scenario].u4CropW/(2*((MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM+1));
        m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT =
                m_SensorCrop[scenario].u4CropH/(2*((MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM+1));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH =
                (m_SensorCrop[scenario].u4CropW/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT =
                (m_SensorCrop[scenario].u4CropH/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT));

        m_rIspLscCfg[idx].ratio.val = 0x20202020;
        m_rIspLscCfg[idx].gain_th.val = 0;

        MY_LOG("[LscMgr:%s] LSCScenario %d, sensorOp %d DMA/EN/W/H/XNum/YNum/OffX/OffY/LW/LH"
                "(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", __FUNCTION__,
                idx, scenario,
                m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN,
                m_rIspLscCfg[idx].lsc_en.bits.LSC_EN,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT
        );
    }
#else
    MUINT32 round;
    for (idx = 0; idx < SHADING_SUPPORT_OP_NUM; idx++) {
        scenario = getSensorScenarioByLscScenario((ELscScenario_T)idx);
        getScenarioResolution(scenario);

        round = ((MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM+1);
        m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH =
                (m_SensorCrop[scenario].u4CropW+round)/(2*((MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM+1));

        round = ((MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM+1);
        m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT =
                (m_SensorCrop[scenario].u4CropH+round)/(2*((MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM+1));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH =
                (m_SensorCrop[scenario].u4CropW/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH));

        m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT =
                (m_SensorCrop[scenario].u4CropH/2 -
                        ((MUINT32)(m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM)*
                                (MUINT32)m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT));
        m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR = stRawLscInfo[idx].phyAddr;
        m_rIspLscCfg[idx].ratio.val = 0x20202020;
        m_rIspLscCfg[idx].gain_th.val = 0;

        MY_LOG("[LscMgr:%s] LSCScenario %d, sensorOp %d DMA/EN/W/H/XNum/YNum/OffX/OffY/LW/LH/Addr"
                "(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%08x)", __FUNCTION__,
                idx, scenario,
                m_rIspLscCfg[idx].lsci_en.bits.LSCI_EN,
                m_rIspLscCfg[idx].lsc_en.bits.LSC_EN,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_WIDTH,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_HEIGHT,
                m_rIspLscCfg[idx].ctl2.bits.SDBLK_XNUM,
                m_rIspLscCfg[idx].ctl3.bits.SDBLK_YNUM,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_XOFST,
                m_rIspLscCfg[idx].ctl1.bits.SDBLK_YOFST,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lWIDTH,
                m_rIspLscCfg[idx].lblock.bits.SDBLK_lHEIGHT,
                m_rIspLscCfg[idx].baseaddr.bits.BASE_ADDR
        );
    }
#endif
    return MTRUE;
}

NSIspTuning::LscMgr::LSCParameter &
LscMgr::
getLscNvram(void)
{
    //	        UINT8 u4Mode = 2;
    //	        for (int idx = 0; idx < 5; idx++) {
    //	            u4Mode = idx;
    //	        MY_LOG("[%s], mode = %d \n", __FUNCTION__, u4Mode);
    //	    MY_LOG("SHADING_EN:%d\n",           m_rIspLscCfg[u4Mode].lsc_en.bits.LSC_EN      );
    //	    MY_LOG("SHADINGBLK_XNUM:%d\n",      m_rIspLscCfg[u4Mode].ctl2.bits.SDBLK_XNUM    );
    //	    MY_LOG("SHADINGBLK_YNUM:%d\n",      m_rIspLscCfg[u4Mode].ctl3.bits.SDBLK_YNUM    );
    //	    MY_LOG("SHADINGBLK_WIDTH:%d\n",     m_rIspLscCfg[u4Mode].ctl2.bits.SDBLK_WIDTH   );
    //	    MY_LOG("SHADINGBLK_HEIGHT:%d\n",    m_rIspLscCfg[u4Mode].ctl3.bits.SDBLK_HEIGHT  );
    //	    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):0x%08x\n", m_rIspLscCfg[u4Mode].baseaddr.bits.BASE_ADDR);
    //	    MY_LOG("SD_LWIDTH:%d\n",            m_rIspLscCfg[u4Mode].lblock.bits.SDBLK_lWIDTH  );
    //	    MY_LOG("SD_LHEIGHT:%d\n",           m_rIspLscCfg[u4Mode].lblock.bits.SDBLK_lHEIGHT );
    //	    MY_LOG("SDBLK_RATIO00:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO00        );
    //	    MY_LOG("SDBLK_RATIO01:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO01        );
    //	    MY_LOG("SDBLK_RATIO10:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO10        );
    //	    MY_LOG("SDBLK_RATIO11:%d\n",        m_rIspLscCfg[u4Mode].ratio.bits.RATIO11        );
    //	        }
    return m_rIspLscCfg;
}


MUINT32*
LscMgr::
getLut(ELscScenario_T lsc_scenario) const {
    //    MY_LOG("[LscMgr] getLut m_eLscScenario %d m_u4CTIdx %d \n", m_eLscScenario, m_u4CTIdx);

#if USING_BUILTIN_LSC
    if (lsc_scenario == LSC_SCENARIO_04)
        return def_coef_cap;
    else
        return def_coef;
#else
    switch (lsc_scenario) {
        case LSC_SCENARIO_01:
            return &m_rIspShadingLut.PreviewFrmTable[0][0];
        case LSC_SCENARIO_03:
            return &m_rIspShadingLut.CaptureFrmTable[0][0];
        case LSC_SCENARIO_04:
            return &m_rIspShadingLut.CaptureTilTable[0][0];
        case LSC_SCENARIO_09_17:
            return &m_rIspShadingLut.VideoFrmTable[0][0];
        case LSC_SCENARIO_30:
            return &m_rIspShadingLut.N3DPvwTable[0][0];
        case LSC_SCENARIO_37:
            return &m_rIspShadingLut.N3DCapTable[0][0];
        default:
            MY_ERR("[LscMgr] "
                    "Wrong m_eLscScenario %d\n", lsc_scenario);
            break;
    }
    return NULL;
#endif
}


MUINT32 u4BufSizeU8[SHADING_SUPPORT_OP_NUM] = {
        MAX_SHADING_PvwFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_VdoFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_PvwTil_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM,
        MAX_SHADING_CapFrm_SIZE*sizeof(MUINT32)*SHADING_SUPPORT_CT_NUM
};

#if ENABLE_TSF
MUINT32 u4TSFBufSizeU8[] =
{
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // input
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // output
        MAX_SHADING_CapTil_SIZE*sizeof(MUINT32), // backup
        AWB_STAT_SIZE,                           // awb statistics
};
#endif

MUINT32
LscMgr::
getPerLutSize(ELscScenario_T eLscScenario) const {
    MUINT32 tableSize = 0;

#if USING_BUILTIN_LSC
    if (eLscScenario == LSC_SCENARIO_04) {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScenario < SHADING_SUPPORT_OP_NUM) {
        //        MY_LOG("[LscMgr] %s, PertableSize %d", __FUNCTION__,
        //                u4BufSizeU8[eLscScenario]/SHADING_SUPPORT_CT_NUM);
        return u4BufSizeU8[eLscScenario]/SHADING_SUPPORT_CT_NUM;
    } else {
        MY_ERR("[LscMgr] Wrong eLscScenario %d\n", eLscScenario);
        return 0;
    }

#endif
}

MUINT32
LscMgr::
getTotalLutSize(ELscScenario_T eLscScenario) const {
    //    MY_LOG("[LscMgr] "
    //            "getLut m_eLscScenario %d\n", eLscScenario);
    MUINT32 tableSize = 0;

#if USING_BUILTIN_LSC
    if (eLscScenario == LSC_SCENARIO_04) {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef_cap));
        return sizeof(def_coef_cap);
    } else {
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC size %d", __FUNCTION__, sizeof(def_coef));
        return sizeof(def_coef);
    }
#else

    if (eLscScenario < SHADING_SUPPORT_OP_NUM) {
        return u4BufSizeU8[eLscScenario];
    } else {
        MY_ERR("[LscMgr] Wrong eLscScenario %d\n", eLscScenario);
        return 0;
    }

#endif
}

MVOID
LscMgr::
loadLut()
{
    if (m_bIsLutLoaded == MTRUE)
    {
        MY_LOG("[LscMgr] m_bIsLutLoaded == MTRUE");
        return;
    }

    m_bIsLutLoaded = MTRUE;
    loadLutToSysram();
    MY_LOG("[LscMgr] loadLutToSysram Done!");
    ConfigUpdate();
    MY_LOG("[LscMgr] ConfigUpdate Done!");
    importEEPromData();
    MY_LOG("[LscMgr] importEEPromData Done!");
#if ENABLE_TSF
    loadTSFLut();
    MY_LOG("[LscMgr] loadTSFLut Done!");
#endif
}

MVOID
LscMgr::
loadLutToSysram()     //  VA <- LUT
{
    MUINT32 i;
    ELscScenario_T iScene;


#if 1
    for (i = 0; i < LSC_SCENARIO_NUM; i++)
    {
        iScene = static_cast<ELscScenario_T>(i);
#else   // debug
        iScene = m_eLscScenario;
#endif

        MY_LOG("[LscMgr] "
                "loadLutToSysram <<m_eLscScenario %d>>, m_pvVirTBA 0x%x \n", iScene,
                stRawLscInfo[iScene].virtAddr);
        if (stRawLscInfo[iScene].virtAddr == MNULL)
        {
            MY_ERR(
                    "[LscMgr] "
                    "Err :: load shading table to NULL address (m_pvVirTBA, 0x%x) \n",
                    stRawLscInfo[iScene].virtAddr);
            return;
        }

        MY_LOG("[LscMgr:%s] virAddr 0x%x, size %d, Lut 0x%x, LutSize %d", __FUNCTION__,
                stRawLscInfo[iScene].virtAddr,
                stRawLscInfo[iScene].size,
                getLut(iScene),
                getTotalLutSize(iScene));

        //        for (int j = 0; j < SHADING_SUPPORT_CT_NUM; j++) {
        //            for (int i = 0; i < 8; i++) {
        //                MY_LOG("[LscMgr:%s] NVRAM ct %d, idx %d, 0x%08x\n",
        //                        __FUNCTION__,
        //                        j,
        //                        i,
        //                        *(MUINT32*)(getLut(iScene)
        //                                + (stRawLscInfo[iScene].size/SHADING_SUPPORT_CT_NUM/4)*j
        //                                + i));
        //            }
        //        }

        if (stRawLscInfo[iScene].size < getTotalLutSize(iScene))
        {
            MY_ERR("[%s] stRawLscInfo[iScene].size %d, LutSize %d, Overflow!!", __FUNCTION__,
                    stRawLscInfo[iScene].size,
                    getTotalLutSize(iScene));
        }
        else
        {
            MY_LOG("[LscMgr:%s] virtAddr 0x%08x, src 0x%08x, size 0x%08x\n",
                    __FUNCTION__,
                    stRawLscInfo[iScene].virtAddr,
                    getLut(iScene),
                    getTotalLutSize(iScene));

            ::memcpy(reinterpret_cast<MVOID*>(stRawLscInfo[iScene].virtAddr), getLut(iScene), getTotalLutSize(iScene));
        }

        //	        for (int j = 0; j < SHADING_SUPPORT_CT_NUM; j++) {
        //	            for (int i = 0; i < 8; i++) {
        //	                MY_LOG("[LscMgr:%s] IMEM ct %d, idx %d, 0x%08x\n",
        //	                        __FUNCTION__,
        //	                        j,
        //	                        i,
        //	                        *(MUINT32*)((MUINT32*)stRawLscInfo[iScene].virtAddr
        //	                                + (stRawLscInfo[iScene].size/SHADING_SUPPORT_CT_NUM/4)*j
        //	                                + i));
        //	            }
        //	        }
        MY_LOG("[LscMgr:%s] Copy table done (start, end) = (0x%08x, 0x%08x)\n", __FUNCTION__,
                stRawLscInfo[iScene].virtAddr,
                stRawLscInfo[iScene].virtAddr + getTotalLutSize(iScene));
    }
    if (*((MUINT32*)stRawLscInfo[iScene].virtAddr+0) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+1) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+2) == 0 &&
            *((MUINT32*)stRawLscInfo[iScene].virtAddr+3) == 0) {
        MY_ERR("[LscMgr:%s] Default table is ZERO!!", __FUNCTION__);
    }

    RawLscTblFlushCurrTbl();
    return;
}


MUINT32
LscMgr::
getCTIdx() {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_ct = 0;
    property_get("debug.lsc_mgr.ct", value, "-1");
    dbg_ct = atoi(value);

    if (dbg_ct != -1) {
        MY_LOG("[LscMgr:%s] DEBUG set m_u4CTIdx to %d", __FUNCTION__,
                dbg_ct);
        m_u4CTIdx = dbg_ct;
    } else {
#if ENABLE_TSF
        if (isTSFEnable() == MTRUE && m_bMetaMode != MTRUE)
            m_u4CTIdx = getTSFD65Idx();
#endif
    }

    MY_LOG("[%s] Shading idx= %d\n", __FUNCTION__, m_u4CTIdx);
    return m_u4CTIdx;
}

MBOOL
LscMgr::
setCTIdx(MUINT32 const u4CTIdx) {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_ct = 0;
    property_get("debug.lsc_mgr.ct", value, "-1");
    dbg_ct = atoi(value);

    if (dbg_ct != -1) {
        MY_LOG("[LscMgr:%s] DEBUG set m_u4CTIdx to %d", __FUNCTION__,
                dbg_ct);
        m_u4CTIdx = dbg_ct;
    } else {
        if (SHADING_SUPPORT_CT_NUM <= u4CTIdx) {
            MY_LOG("!!! WRONG Shading idx= %d\n", u4CTIdx);
            return MFALSE;
        }

#if ENABLE_TSF
        if (isTSFEnable() == MTRUE && m_bMetaMode != MTRUE) {
            setIfChange(m_u4CTIdx, getTSFD65Idx());
            MY_LOG("[%s] TSF ading idx = %d\n", __FUNCTION__, m_u4CTIdx);
        } else {
            setIfChange(m_u4CTIdx, u4CTIdx);
            MY_LOG("[%s] Shading idx= %d\n", __FUNCTION__, u4CTIdx);
        }
#else
        setIfChange(m_u4CTIdx, u4CTIdx);
        MY_LOG("[%s] Shading idx= %d\n", __FUNCTION__, u4CTIdx);
#endif
    }
    return MTRUE;
}

MBOOL
LscMgr::
SetTBAToISP()     //  ISP <- PA
{
    MUINT32 virAddr = 0;

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized", __FUNCTION__);
        return MFALSE;
    }
    RawLscTblFlushCurrTbl();
#if ENABLE_TSF
    MY_LOG("[LscMgr:%s] TSF enable %d, state %d", __FUNCTION__,
            isTSFEnable(),
            mTSFState);
    if (isTSFEnable() == MTRUE && mTSFState != LSCMGR_TSF_STATE_IDLE)
    {
        virAddr = m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr;
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = m_TSFBuff[TSF_BUFIDX_INPUT].phyAddr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE = getPerLutSize(m_eLscScenario) - 1;

        MY_LOG("[LscMgr:%s] upate TSF phy addr 0x%08x, vir addr 0x%08x, size 0x%08x", __FUNCTION__,
                m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR,
                virAddr,
                m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE);

    }
    else
#endif  //ENABLE_TSF
    {
#if USING_BUILTIN_LSC
        MY_LOG("[LscMgr] %s USING_BUILTIN_LSC", __FUNCTION__);
        virAddr = stRawLscInfo[m_eLscScenario].virtAddr;
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = stRawLscInfo[m_eLscScenario].phyAddr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE =
                getTotalLutSize(static_cast<NSIspTuning::LscMgr::ELscScenario_T>(0)) - 1;

#else
        virAddr = stRawLscInfo[m_eLscScenario].virtAddr +
                getPerLutSize(m_eLscScenario) * getCTIdx();
        MUINT32 Addr =
                stRawLscInfo[m_eLscScenario].phyAddr
                + getPerLutSize(m_eLscScenario) * getCTIdx();
        m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR = Addr;
        m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE = getPerLutSize(m_eLscScenario) - 1;




        MY_LOG("[LscMgr] %s m_eLscScenario %d, TableSize %d, CCT 0x%x, phyAddr 0x%x, XSize 0x%x m_eSensorOp %d\n",
                __FUNCTION__,
                m_eLscScenario,
                getPerLutSize(m_eLscScenario),
                m_u4CTIdx,
                m_rIspLscCfg[m_eLscScenario].baseaddr.bits.BASE_ADDR,
                m_rIspLscCfg[m_eLscScenario].xsize.bits.XSIZE,
                m_eSensorOp);
#endif  // USING_BUILTIN_LSC
    }

//    {
//        char filename[128];
//        sprintf(filename, "/sdcard/lsc1to3data/Cap_ct%d.bin", m_u4CTIdx);
//        MY_LOG("[LscMgr:%s] DBG: Output Capture Table to %s", __FUNCTION__, filename);
//        FILE* fpdebug = fopen(filename,"wb");
//        if ( fpdebug == NULL )
//        {
//            MY_ERR("Can't open :%s\n",filename);
//        } else {
//            MUINT32 Addr =
//                    stRawLscInfo[m_eLscScenario].phyAddr
//                    + getPerLutSize(m_eLscScenario) * getCTIdx();
//            fwrite(Addr,
//                    getPerLutSize(m_eLscScenario)),
//                    1,fpdebug);
//            fclose(fpdebug);
//        }
//    }


    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        MINT32 dbg_tbl = 0;
        property_get("debug.lsc_mgr.dumptbl", value, "-1");
        dbg_tbl = atoi(value);

        if (virAddr != 0 && dbg_tbl != -1) {
            for (int i = 0; i < dbg_tbl; i+=4) {
                MY_LOG("[LscMgr] idx %d, 0x%08x 0x%08x 0x%08x 0x%08x\n",
                        i,
                        *(MUINT32*)((MUINT32*)virAddr + i + 0),
                        *(MUINT32*)((MUINT32*)virAddr + i + 1),
                        *(MUINT32*)((MUINT32*)virAddr + i + 2),
                        *(MUINT32*)((MUINT32*)virAddr + i + 3)
                );
            }
        }
    }
    return MTRUE;
}



MBOOL
LscMgr::
getScenarioResolution(ACDK_SCENARIO_ID_ENUM scenario)
{
    MUINT32 cmd;


    if (!m_pSensorHal)
    {
        MY_ERR("[LscMgr] %s, m_pSensorHal is NULL", __FUNCTION__);
        m_pSensorHal = SensorHal::createInstance();
        if (m_pSensorHal->init())
        {
            MY_ERR("m_pSensorHal re-instanate fail!!");
            m_pSensorHal->destroyInstance();
            m_pSensorHal = NULL;
            return MFALSE;
        }
    }

    m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (int)&m_SensorDev, 0, 0);
    m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_GRAB_INFO,
            (int)&m_SensorCrop[scenario].u4GrabX,
            (int)&m_SensorCrop[scenario].u4GrabY,
            scenario);

    switch(scenario)
    {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_PRV_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG, sensor SENSOR_CMD_GET_SENSOR_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_VIDEO_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_VIDEO_RANGE");
            break;
        case ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
            cmd = SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO, sensor SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_ZSD:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_ZSD, sensor SENSOR_CMD_GET_SENSOR_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW, sensor SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            cmd = SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE, sensor SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE");
            break;
        case ACDK_SCENARIO_ID_CAMERA_3D_VIDEO:
            cmd = SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE;
            MY_LOG("acdk ACDK_SCENARIO_ID_CAMERA_3D_VIDEO, sensor SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE");
            break;
        default:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE;
            break;
    }

    m_pSensorHal->sendCommand(m_SensorDev, cmd,
            (int)&m_SensorCrop[scenario].u4CropW,
            (int)&m_SensorCrop[scenario].u4CropH,
            0);
    m_SensorCrop[scenario].u4SrcW =
            m_SensorCrop[scenario].u4CropW+m_SensorCrop[scenario].u4GrabX;
    m_SensorCrop[scenario].u4SrcH =
            m_SensorCrop[scenario].u4CropH+m_SensorCrop[scenario].u4GrabY;

    MY_LOG("[%s] SensorOP %d GrabX GrabY SrcW SrcH CropW CropH DataFmt (%d, %d, %d, %d, %d, %d, %d)", __FUNCTION__,
            scenario,
            m_SensorCrop[scenario].u4GrabX,          // For input sensor width
            m_SensorCrop[scenario].u4GrabY,          // For input sensor height
            m_SensorCrop[scenario].u4SrcW,          // For input sensor width
            m_SensorCrop[scenario].u4SrcH,          // For input sensor height
            m_SensorCrop[scenario].u4CropW,        //TG crop width
            m_SensorCrop[scenario].u4CropH,        //TG crop height
            m_SensorCrop[scenario].DataFmt);

    return MTRUE;
}

MBOOL
LscMgr::
updateLscScenarioBySensorMode()
{
#if 0   // get sensor Op from sensor hal

    if (!m_pSensorHal)
    {
        MY_LOG("[%s] NULL m_pSensorHal!\n", __FUNCTION__);
        return MFALSE;
    }

    m_pSensorHal->sendCommand(SENSOR_DEV_NONE, SENSOR_CMD_GET_SENSOR_DEV, (int)&m_SensorDev, 0, 0);

    if (m_SensorDev == SENSOR_DEV_NONE)
    {
        MY_LOG("[%s] m_SensorDev is incorrect %d\n", __FUNCTION__,
                SENSOR_DEV_NONE);
        return MFALSE;
    }
    m_pSensorHal->sendCommand(m_SensorDev, SENSOR_CMD_GET_SENSOR_SCENARIO, (int)&m_eSensorOp, 0, 0);

    if (m_eSensorOp >= ACDK_SCENARIO_ID_MAX || m_SensorDev == SENSOR_DEV_NONE)
    {
        MY_LOG("[%s] m_eSensorOp output range %d, max %d\n", __FUNCTION__,
                m_eSensorOp,
                ACDK_SCENARIO_ID_MAX);
        return MFALSE;
    }
#else
    m_eSensorOp = getSensorScenarioByIspProfile(m_eIspProfile);
#endif
    m_eLscScenario = (ELscScenario_T)getLscScenarioBySensorScenario(m_eSensorOp);

    MY_LOG("[LscMgr] %s, Dev %d, SensorOp %d Lsc Scenario %d", __FUNCTION__,
            m_SensorDev,
            m_eSensorOp,
            m_eLscScenario);

    m_ePrevSensorOp = m_eSensorOp;
    return MTRUE;
}


MVOID
LscMgr::
updateLscScenarioByIspProfile(EIspProfile_T profile)
{

    switch(profile)
    {
        case EIspProfile_NormalPreview:
            m_eLscScenario = LSC_SCENARIO_01;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            break;
        case EIspProfile_ZsdPreview_CC:
        case EIspProfile_ZsdPreview_NCC:
        case EIspProfile_NormalCapture:
            m_eLscScenario = LSC_SCENARIO_04;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
            break;
        case EIspProfile_VideoPreview:
            m_eLscScenario = LSC_SCENARIO_09_17;
            m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
            break;
        case EIspProfile_VideoCapture:
            m_eLscScenario = LSC_SCENARIO_09_17;
            m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
            break;
        default:
            m_eLscScenario = LSC_SCENARIO_01;
            m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            break;
    }
    MY_LOG("[%s]  IspProfile %d, LscScenario %d", __FUNCTION__,
            m_eIspProfile,
            m_eLscScenario);
    return;
}


static ACDK_SCENARIO_ID_ENUM SensorScenarioIspProfileMapping[] =
{
        ACDK_SCENARIO_ID_CAMERA_PREVIEW,        //EIspProfile_NormalPreview
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_ZsdPreview_CC
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_ZsdPreview_NCC
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   //EIspProfile_NormalCapture
        ACDK_SCENARIO_ID_VIDEO_PREVIEW,         //EIspProfile_VideoPreview
        ACDK_SCENARIO_ID_VIDEO_PREVIEW,         //EIspProfile_VideoCapture
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_MFCapPass1
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_MFCapPass2
        ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,   // EIspProfile_ProcessedRAW
};

ACDK_SCENARIO_ID_ENUM
LscMgr::
getSensorScenarioByIspProfile(EIspProfile_T const eIspProfile)
{
    //MY_LOG("[LscMgr:%s], EIspProfile_T %d", __FUNCTION__, eIspProfile);

    if (eIspProfile >= EIspProfile_NUM)
    {
        MY_ERR("[LscMgr] %s, EIspProfile_T %d out of range!!", __FUNCTION__, eIspProfile);
        return ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
    return SensorScenarioIspProfileMapping[eIspProfile];
}

MBOOL
LscMgr::
setIspProfile(EIspProfile_T const eIspProfile)
{
    EIspProfile_T profile_bak;
    MBOOL bDirty = MFALSE, bSceneChange = MFALSE;
    MY_LOG("[LscMgr] %s, EIspProfile_T %d", __FUNCTION__, eIspProfile);

    if (eIspProfile >= EIspProfile_NUM)
        return MFALSE;
    {
        // debug
        //        ISP_NVRAM_LSC_T debug, debug2;
        //        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        //        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        //        MY_LOG("[LscMgr] %s Orig Shading param (0x%x, 0x%x, 0x%x, 0x%x), (0x%x, 0x%x ,0x%x ,0x%x) ,"
        //                "0x%x, 0x%x \n",
        //                __FUNCTION__,
        //                debug.lsci_en.val,
        //                debug.baseaddr.val,
        //                debug.xsize.val,
        //                debug.lsc_en.val,
        //                debug.ctl1.val, debug.ctl2.val,
        //                debug.ctl3.val, debug.lblock.val,
        //                debug.ratio.val,
        //                debug.gain_th.val);
        //        debug.lsci_en.bits.LSCI_EN = 1;
        //        debug.lsc_en.bits.LSC_EN = 1;
        //        debug.baseaddr.bits.BASE_ADDR = 0xdeadbeef;
        //        debug.xsize.bits.XSIZE  = 0x11bd;
        //        debug.ctl1.bits.SDBLK_XOFST = 0x1a;
        //        debug.ctl1.bits.SDBLK_YOFST = 0x1a;
        //        debug.ctl2.bits.SDBLK_WIDTH = 0x2dd;
        //        debug.ctl2.bits.SDBLK_XNUM = 0x1c;
        //        debug.ctl3.bits.SDBLK_HEIGHT = 0x4ee;
        //        debug.ctl3.bits.SDBLK_YNUM = 0x1b;
        //        debug.lblock.bits.SDBLK_lHEIGHT = 0xf5;
        //        debug.lblock.bits.SDBLK_lWIDTH = 0xfe;
        //
        //        MY_LOG("[LscMgr] %s Setting Shading param (0x%x, 0x%x, 0x%x, 0x%x), (0x%x, 0x%x ,0x%x ,0x%x) ,"
        //                "0x%x, 0x%x \n",
        //                __FUNCTION__,
        //                debug.lsci_en.val,
        //                debug.baseaddr.val,
        //                debug.xsize.val,
        //                debug.lsc_en.val,
        //                debug.ctl1.val, debug.ctl2.val,
        //                debug.ctl3.val, debug.lblock.val,
        //                debug.ratio.val,
        //                debug.gain_th.val);
        //
        //        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        //        ISP_MGR_LSC_T::getInstance(m_eActive).put(debug);
        //        ISP_MGR_LSC_T::getInstance(m_eActive).apply(EIspProfile_NormalPreview);
        //        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        //        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug2);
        //        MY_LOG("[LscMgr] %s Read back Shading param (0x%x, 0x%x, 0x%x, 0x%x), (0x%x, 0x%x ,0x%x ,0x%x) ,"
        //                "0x%x, 0x%x \n",
        //                __FUNCTION__,
        //                debug2.lsci_en.val,
        //                debug2.baseaddr.val,
        //                debug2.xsize.val,
        //                debug2.lsc_en.val,
        //                debug2.ctl1.val, debug.ctl2.val,
        //                debug2.ctl3.val, debug.lblock.val,
        //                debug2.ratio.val,
        //                debug2.gain_th.val);
    }

    m_bMetaMode = MFALSE;
    profile_bak = m_eIspProfile;
    bSceneChange = setIfChange(m_eIspProfile, eIspProfile);
    m_ePrevLscScenario = m_eLscScenario;
    bDirty = updateLscScenarioBySensorMode();

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[%s]  m_eLscScenario out of range %d, max %d",
                m_eLscScenario,
                LSC_SCENARIO_NUM);
        return MFALSE;
    }

    if (bSceneChange) {
        m_ePrevIspProfile = profile_bak;
    }

    if (m_pIMemDrv)
    {
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
        loadLut();
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

#if ENABLE_TSF
        if (bSceneChange == MTRUE)   // changed
        {
            if (mTSFState == LSCMGR_TSF_STATE_IDLE) {
                MY_LOG("[LscMgr] Init TSF table!");
                changeTSFState(LSCMGR_TSF_STATE_INIT);
            } else
                if (!(EIspProfile_VideoCapture == m_eIspProfile ||
                        EIspProfile_VideoCapture == m_ePrevIspProfile)) // to avoid table change
                    changeTSFState(LSCMGR_TSF_STATE_SCENECHANGE);

            if (m_bMetaMode == MTRUE) {
                //m_bTSF = MFALSE;
                MY_LOG("[LscMgr] m_bMetaMode!");
                m_bTSF = isTSFEnable();
            } else {
                MY_LOG("[LscMgr] Normal mode!");
                m_bTSF = isEnableTSF();
            }
        }
#endif
    }
    else
    {
        MY_LOG("[LscMgr] m_pIMemDrv 0x%x, bDirty %d", m_pIMemDrv, bDirty);
    }
    MY_LOG("[%s]  PrevLscScenario %d, LscScenario %d", __FUNCTION__,
            m_ePrevLscScenario,
            m_eLscScenario);

    return MTRUE;
}

EIspProfile_T
LscMgr::
getIspProfile(void)
{
    return m_eIspProfile;
}

EIspProfile_T
LscMgr::
getPrevIspProfile(void)
{
    return m_ePrevIspProfile;
}

MBOOL
LscMgr::
setMetaIspProfile(EIspProfile_T const eIspProfile, MUINT32 sensor_mode)
{
    EIspProfile_T profile_bak;
    MBOOL bDirty = MFALSE, bSceneChange = MFALSE;
    MY_LOG("[LscMgr:%s], eIspProfile %d, sensor_mode %d", __FUNCTION__,
            eIspProfile,
            sensor_mode);

    m_bMetaMode = MTRUE;
    m_SensorMode = sensor_mode;

    profile_bak = m_eIspProfile;
    bSceneChange = setIfChange(m_eIspProfile, eIspProfile);
    m_ePrevLscScenario = m_eLscScenario;
    bDirty = updateLscScenarioBySensorMode();

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[%s]  m_eLscScenario out of range %d, max %d",
                m_eLscScenario,
                LSC_SCENARIO_NUM);
        return MFALSE;
    }


    if (eIspProfile == EIspProfile_NormalCapture) {
        switch (m_SensorMode) {
            case ESensorMode_Preview:
                m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
                m_eLscScenario = LSC_SCENARIO_01;
                break;
            case ESensorMode_Video:
                m_eSensorOp = ACDK_SCENARIO_ID_VIDEO_PREVIEW;
                m_eLscScenario = LSC_SCENARIO_09_17;
                break;
            case ESensorMode_Capture:
                m_eSensorOp = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                m_eLscScenario = LSC_SCENARIO_04;
                break;
        }
        MY_LOG("[LscMgr: %s], final m_eLscScenario %d, m_eSensorOp %d", __FUNCTION__,
                m_eLscScenario,
                m_eSensorOp);
    }

    if (bSceneChange) {
        m_ePrevIspProfile = profile_bak;
    }

    if (m_pIMemDrv)
    {
        CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
        loadLut();
        CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

#if ENABLE_TSF
        if (bSceneChange == MTRUE)   // changed
        {
            if (mTSFState == LSCMGR_TSF_STATE_IDLE) {
                MY_LOG("[LscMgr] Init TSF table!");
                changeTSFState(LSCMGR_TSF_STATE_INIT);
            } else
                if (!(EIspProfile_VideoCapture == m_eIspProfile ||
                        EIspProfile_VideoCapture == m_ePrevIspProfile)) // to avoid table change
                    changeTSFState(LSCMGR_TSF_STATE_SCENECHANGE);

            if (m_bMetaMode == MTRUE) {
                //m_bTSF = MFALSE;
                MY_LOG("[LscMgr] m_bMetaMode!");
                m_bTSF = isTSFEnable();
            } else {
                MY_LOG("[LscMgr] Normal mode!");
                m_bTSF = isEnableTSF();
            }
        }
#endif
    }
    else
    {
        MY_LOG("[LscMgr] m_pIMemDrv 0x%x, bDirty %d", m_pIMemDrv, bDirty);
    }
    MY_LOG("[%s]  PrevLscScenario %d, LscScenario %d", __FUNCTION__,
            m_ePrevLscScenario,
            m_eLscScenario);
    return MTRUE;
}

MVOID
LscMgr::
setMetaLscScenario(ELscScenario_T lsc_scenario)
{
    m_eMetaLscScenario = lsc_scenario;
    m_bMetaMode = MTRUE;
}

MBOOL
LscMgr::
isEnable()
{

    {
        ISP_NVRAM_LSC_T debug;
        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        //        MY_LOG("[LscMgr] %s Shading param \n"
        //                "lsci, base, xsize, lsc, 0x%x, 0x%x, 0x%x, 0x%x, \n"
        //                "ctl1,2,3,lblock,ratio,gain_th 0x%x, 0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
        //                __FUNCTION__,
        //                debug.lsci_en.val,
        //                debug.baseaddr.val,
        //                debug.xsize.val,
        //                debug.lsc_en.val,
        //                debug.ctl1.val, debug.ctl2.val,
        //                debug.ctl3.val, debug.lblock.val,
        //                debug.ratio.val,
        //                debug.gain_th.val);
    }

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized");
        return MFALSE;
    }

    if (isBypass() == MTRUE) {
        return MFALSE;
    }
//    MY_LOG("[LscMgr] %s %x %d m_eLscScenario %d", __FUNCTION__,
//            m_rIspLscCfg,
//            m_rIspLscCfg[m_eLscScenario].lsc_en.bits.LSC_EN,
//            m_eLscScenario);
    return m_rIspLscCfg[m_eLscScenario].lsc_en.bits.LSC_EN;
}

MBOOL
LscMgr::
enableLscWoVariable(MBOOL const fgEnable)
{
    ISP_NVRAM_LSC_T tmp;
    MBOOL fgRet = MFALSE;
    MBOOL OrgShadingEn = MFALSE;
    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized");
        return MFALSE;
    }

    MY_LOG("[LscMgr] -enableLsc(enableLscWoVariable)"
            "  --> %d\n", fgEnable);
    // (1) get hw setting
    ISP_MGR_LSC_T::getInstance(m_eActive).get(tmp);

    //  (2) Change the state of hw data
    tmp.lsc_en.bits.LSC_EN = tmp.lsci_en.bits.LSCI_EN = (MUINT32)fgEnable;

    //  (3) Apply to ISP.
    ISP_MGR_LSC_T::getInstance(m_eActive).put(tmp); //put to ispmgr
    fgRet = ISP_MGR_LSC_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.

    MY_LOG("[LscMgr] enable %d", ISP_MGR_LSC_T::getInstance(m_eActive).isEnable());

    return fgRet;
}

MBOOL
LscMgr::
isBypass() {
    return m_bBypass;
}

MVOID
LscMgr::
enBypass(MBOOL enable) {
    m_bBypass = enable;
    enableLscWoVariable(enable);
    if (m_bBypass == MFALSE)
        enableLsc(isEnable());
}

MBOOL
LscMgr::
enableLsc(MBOOL const fgEnable)
{
    MBOOL fgRet = MFALSE;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_enable = 0;
    property_get("debug.lsc_mgr.enable", value, "-1");
    dbg_enable = atoi(value);

    if (m_eLscScenario >= LSC_SCENARIO_NUM)
    {
        MY_ERR("[LscMgr] %s m_eLscScenario not initialized");
        return MFALSE;
    }

    //  (2) Change the state of NVRAM data
    if (dbg_enable != -1) {
        MY_LOG("[LscMgr:%s] DEBUG set enable to %d", __FUNCTION__,
                dbg_enable);
        m_rIspLscCfg[m_eLscScenario].lsc_en.bits.LSC_EN =
                m_rIspLscCfg[m_eLscScenario].lsci_en.bits.LSCI_EN  = dbg_enable;
    } else {
        if (isBypass() == MTRUE) {
        } else {
            m_rIspLscCfg[m_eLscScenario].lsc_en.bits.LSC_EN =
                    m_rIspLscCfg[m_eLscScenario].lsci_en.bits.LSCI_EN = fgEnable;
        }
    }
    //  (3) Apply to ISP.
    fgRet = ISP_MGR_LSC_T::getInstance(m_eActive).reset();
    if (fgRet) {
        MY_LOG("[LscMgr:%s]  "
                "Meta %d, Shading param 0x%x, 0x%x, 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                m_bMetaMode,
                m_rIspLscCfg[m_eLscScenario].lsci_en.val,
                m_rIspLscCfg[m_eLscScenario].baseaddr.val,
                m_rIspLscCfg[m_eLscScenario].lsc_en.val,
                m_rIspLscCfg[m_eLscScenario].ctl1.val, m_rIspLscCfg[m_eLscScenario].ctl2.val,
                m_rIspLscCfg[m_eLscScenario].ctl3.val, m_rIspLscCfg[m_eLscScenario].lblock.val,
                m_rIspLscCfg[m_eLscScenario].ratio.val,
                m_rIspLscCfg[m_eLscScenario].gain_th.val);
        ISP_MGR_LSC_T::getInstance(m_eActive).put(m_rIspLscCfg[m_eLscScenario]); //put to ispmgr_mt6573
        fgRet = ISP_MGR_LSC_T::getInstance(m_eActive).apply(getIspProfile()); //Apply to ISP.
    }
    else
    {
        MY_LOG("[LscMgr] %s fail to read registers", __FUNCTION__);
    }

    if (isBypass() == MTRUE) {
        enableLscWoVariable(MFALSE);
        MY_LOG("[LscMgr] Bypassed", __FUNCTION__);
    }

    lbExit:
    return fgRet;
}

MINT32
LscMgr::
RawLscfreeMemory(IMEM_BUF_INFO& RawLscInfo)
{
    if (!m_pIMemDrv || RawLscInfo.virtAddr == 0)
    {
        MY_ERR("RawLsc Null m_pIMemDrv driver \n");
        return MFALSE;
    }
    MINT32 ret = MTRUE;

    if (!m_pIMemDrv->unmapPhyAddr(&RawLscInfo))
    {
        if (!m_pIMemDrv->freeVirtBuf(&RawLscInfo))
        {
            MY_LOG("RawLsc free VirtBuf/PhyBuf 0x%08x/0x%08x success\n",
                    RawLscInfo.virtAddr, RawLscInfo.phyAddr);
            RawLscInfo.virtAddr = 0;
            ret = MTRUE;
        }
        else
        {
            MY_ERR("RawLsc fVirtBuf/PhyBuf 0x%08x/0x%08x error\n",
                    RawLscInfo.virtAddr, RawLscInfo.phyAddr);
            ret = MFALSE;
        }
    }
    else
    {
        MY_ERR("RawLsc unmapPhyAddr error\n");
        ret = MFALSE;
    }

    return ret;
}

MBOOL
LscMgr::
RawLscTblMemInfoShow(IMEM_BUF_INFO& RawLscInfo)
{
    //MY_LOG("[LscMgr] RawLscTblMemInfoShow \n");
    MY_LOG("[LscMgr]RawLscInfo.virtAddr 0x%08x\n",
            RawLscInfo.virtAddr);
    MY_LOG("[LscMgr]RawLscInfo.phyAddr 0x%08x\n",
            RawLscInfo.phyAddr);
    MY_LOG("[LscMgr]RawLscInfo.size 0x%08x\n",
            RawLscInfo.size);
    return MTRUE;
}

MBOOL
LscMgr::
RawLscTblFlushCurrTbl(void) {
    m_pIMemDrv->cacheFlushAll();
    return MTRUE;
}

MBOOL
LscMgr::
RawLscTblSetPhyVirAddr(MUINT32 const u8LscIdx, MVOID* pPhyAddr,
        MVOID* pVirAddr) {
    MY_LOG("[LscMgr] %s not allowed!", __FUNCTION__);
    MBOOL ret = MFALSE;
    return ret;
}

MUINT32
LscMgr::
RawLscTblGetPhyAddr(MUINT32 const u8LscIdx) {
    return stRawLscInfo[u8LscIdx].phyAddr;
}

MUINT32
LscMgr::
RawLscTblGetVirAddr(MUINT32 const u8LscIdx) {
    return stRawLscInfo[u8LscIdx].virtAddr;
}

MBOOL
LscMgr::
RawLscTblAlloc(IMEM_BUF_INFO& RawLscInfo,
        MUINT32 const u8LscLutSize)
{
    MBOOL mbret = MFALSE;
    MUINT32 ret;

    if (!RawLscInfo.virtAddr)
    {
        MY_LOG("[%s] RawLscInfo.u4VirAddr 0x%08x, size %d\n", __FUNCTION__,
                RawLscInfo.virtAddr,
                u8LscLutSize);
        RawLscInfo.size = u8LscLutSize;

        if (!m_pIMemDrv->allocVirtBuf(&RawLscInfo))
        {
            if (m_pIMemDrv->mapPhyAddr(&RawLscInfo))
            {
                MY_LOG("mapPhyAddr error, size 0x%04x , virtAddr 0x%04x\n",
                        RawLscInfo.size,
                        RawLscInfo.virtAddr);
                mbret = MFALSE;
            }
            else
                mbret = MTRUE;
        }
        else
        {
            MY_LOG("allocVirtBuf error, size 0x%04x \n",
                    RawLscInfo.size);
            mbret = MFALSE;
        }
    }
    else
    {
        mbret = MTRUE;
        MY_LOG("already ! RawLscInfo.virtAddr 0x%8x, size%d\n", RawLscInfo.virtAddr, u8LscLutSize);
    }
    return mbret;
}

MBOOL
LscMgr::
RawLscTblInit() {
    MBOOL ret = MFALSE;

    UINT32 u8LscIdx = 0;


    ret = MTRUE;
    if (!m_pIMemDrv) {
        MY_LOG("new pIMemDrv()\n");
        MY_LOG("sizeof(stRawLscInfo) = %d\n", sizeof(stRawLscInfo));
        //::memset(&stRawLscInfo, 0x00, sizeof(stRawLscInfo));
        m_pIMemDrv = IMemDrv::createInstance();

        if (!m_pIMemDrv)
        {
            MY_LOG("m_pIMemDrv new fail.\n");
            ret = MFALSE;
        }
        else
        {
            MY_LOG("m_pIMemDrv createInstance success!\n");
            ret = m_pIMemDrv->init();
            if (ret == MTRUE)
            {
                MY_LOG("m_pIMemDrv init success!\n");

                for (u8LscIdx = 0; u8LscIdx < SHADING_SUPPORT_OP_NUM; u8LscIdx++)
                {
                    MY_LOG("[%s] -------------stRawLscInfo %d ---------------", __FUNCTION__, u8LscIdx);
                    if (!RawLscTblAlloc(stRawLscInfo[u8LscIdx], u4BufSizeU8[u8LscIdx]))
                    {
                        MY_LOG("RawLscTblAlloc(%d) FAILED\n", u8LscIdx);
                    }
                    else
                    {
                        RawLscTblMemInfoShow(stRawLscInfo[u8LscIdx]);
                    }
                }

#if ENABLE_TSF
                for (u8LscIdx = 0; u8LscIdx < sizeof(m_TSFBuff)/sizeof(IMEM_BUF_INFO); u8LscIdx++)
                {
                    MY_LOG("[%s] -------------m_TSFBuff %d ---------------", __FUNCTION__, u8LscIdx);
                    if (!RawLscTblAlloc(m_TSFBuff[u8LscIdx], u4TSFBufSizeU8[u8LscIdx]))
                    {
                        MY_LOG("m_TSFBuff(%d) FAILED\n", u8LscIdx);
                    }
                    else
                    {
                        RawLscTblMemInfoShow(m_TSFBuff[u8LscIdx]);
                    }
                }
#endif
                ret = MTRUE;
            }
            else
            {
                ret = MFALSE;
                MY_LOG("m_pIMemDrv init fail!\n");
            }
        }
    }
    else
    {
        MY_LOG("m_pIMemDrv = 0x%8x\n", (MUINT32) m_pIMemDrv);
    }
    return ret;
}

MBOOL
LscMgr::
RawLscTblUnInit()
{
    UINT32 lu32ErrCode = 0, ret = 0;
    UINT32 i = 0;

    for (i = 0; i < SHADING_SUPPORT_OP_NUM; i++)
    {
        MY_LOG("~ RawLscTblUnInit(%d)!!!\n", i);
        RawLscfreeMemory(stRawLscInfo[i]);
    }

//    ::memset(&stRawLscInfo, 0x00,
//            sizeof(stRawLscInfo));

#if ENABLE_TSF
    for (i = 0; i < sizeof(m_TSFBuff)/sizeof(IMEM_BUF_INFO); i++)
    {
        RawLscfreeMemory(m_TSFBuff[i]);
    }

//    ::memset(&m_TSFBuff, 0x00,
//            sizeof(m_TSFBuff));
#endif

    if (m_pIMemDrv)
    {
        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_SHAD_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_SHAD_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MRESULT LscMgr::
getDebugInfo(DEBUG_SHAD_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;
    ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION        ,(MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)m_eLscScenario);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)debug.lsci_en.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)debug.baseaddr.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)debug.lsc_en.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)debug.ratio.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)debug.gain_th.val);
    return 0;
}

MRESULT
LscMgr::
getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl)
{
    ::memset(&rShadingDbgTbl, 0, sizeof(rShadingDbgTbl));
    rShadingDbgTbl.hdr.u4KeyID = DEBUG_SHAD_ARRAY_KEYID;
    rShadingDbgTbl.hdr.u4ModuleCount = ModuleNum<1, 0>::val;
    rShadingDbgTbl.hdr.u4DbgSHADArrayOffset = sizeof(DEBUG_SHAD_ARRAY_INFO_S::Header);

    rShadingDbgTbl.rDbgSHADArray.ArrayHeight = 1200;
    rShadingDbgTbl.rDbgSHADArray.ArrayWidth = 1600;
    rShadingDbgTbl.rDbgSHADArray.Array[0] = 0xdeadbeef;
    return 0;
}

LscMgr::
LscMgr(ESensorDev_T eSensorDev,
        ISP_NVRAM_REGISTER_STRUCT& rIspNvram,
        ISP_SHADING_STRUCT& rShadingLut):
        m_u4CTIdx(0),
        m_eActive(eSensorDev),
        m_eSensorOp(ACDK_SCENARIO_ID_CAMERA_PREVIEW),
        m_ePrevSensorOp(ACDK_SCENARIO_ID_MAX),
        m_eIspProfile(EIspProfile_NUM),
        m_eLscScenario(LSC_SCENARIO_01),
        m_ePrevLscScenario(LSC_SCENARIO_NUM),
        m_u4SensorID(0),
        m_bIsEEPROMImported(MFALSE),
        m_bIsLutLoaded(MFALSE),
        m_bBypass(MFALSE),
        m_bMetaMode(MFALSE),
        m_rIspLscCfg(rIspNvram.LSC),
        m_rIspShadingLut(rShadingLut),
        m_pIMemDrv(NULL),
        //m_pIspMgr(NULL),
        //m_pIspDrv(NULL),
        m_pSensorHal(NULL)
#if ENABLE_TSF
,
m_bTSF(MFALSE)
#endif
{

    MY_LOG("[LscMgr] "
            "m_pIMemDrv == 0x%x\n"
            "m_rIspLscCfg 0x%x, m_rIspShadingLut 0x%x\n",
            (MUINT32)m_pIMemDrv,
            (MUINT32)&m_rIspLscCfg,
            (MUINT32)&m_rIspShadingLut);

    MY_LOG("[LscMgr] Shading\n Version = 0x%x\n SensorId =  0x%x\n PreviewSVDSize =  0x%x\n VideoSVDSize =  0x%x\n CaptureSVDSize =  0x%x\n",
            m_rIspShadingLut.Version,
            m_rIspShadingLut.SensorId,
            m_rIspShadingLut.PreviewSVDSize,
            m_rIspShadingLut.VideoSVDSize,
            m_rIspShadingLut.CaptureSVDSize
    );
    {
        ISP_NVRAM_LSC_T debug;
        ISP_MGR_LSC_T::getInstance(m_eActive).reset();
        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        MY_LOG("[LscMgr]  %s"
                "LscMgr() Shading param 0x%x, 0x%x, 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                debug.lsci_en.val,
                debug.baseaddr.val,
                debug.lsc_en.val,
                debug.ctl1.val, debug.ctl2.val,
                debug.ctl3.val, debug.lblock.val,
                debug.ratio.val,
                debug.gain_th.val);
    }

    MY_LOG("[LscMgr] ENTER LscMgr\n");
}

MBOOL
LscMgr::init() {
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    // reset LSC register for CQ0
    ISP_NVRAM_LSC_T lsc_off;
    ::memset(&lsc_off, 0, sizeof(ISP_NVRAM_LSC_T));
    ISP_MGR_LSC_T::getInstance(m_eActive).put(lsc_off);
    ISP_MGR_LSC_T::getInstance(m_eActive).apply(EIspProfile_NormalPreview);
    ISP_MGR_LSC_T::getInstance(m_eActive).apply(EIspProfile_NormalCapture);

    CPTLog(Event_Pipe_3A_ISP, CPTFlagStart);
    if (!RawLscTblInit())
    {
        MY_LOG("FATAL WRONG m_pIMemDrv new fail.\n");
    }
    CPTLog(Event_Pipe_3A_ISP, CPTFlagEnd);

#if ENABLE_TSF
    ::sem_init(&mTSFSem, 0, 0);
    mTSFState = LSCMGR_TSF_STATE_IDLE;
    ::pthread_create(&mTSFThread, NULL, (VPT)mThreadLoop, this);
    MY_LOG("[LscMgr] Create TSF thread 0x%x\n", (MUINT32) mTSFThread);
#endif
    m_pSensorHal = SensorHal::createInstance();
    if (m_pSensorHal->init())
    {
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    }

    MY_LOG("[LscMgr:%s] (m_pIMemDrv:0x%8x) (m_pSensorHal:0x%0x) \n", __FUNCTION__,
            (MUINT32) m_pIMemDrv, (MUINT32)m_pSensorHal);

    return MTRUE;
}

LscMgr::
~LscMgr()
{
    MY_LOG("[LscMgr] "
            "EXIT ~LscMgr(m_pIMemDrv - 0x%8x) >>\n", (MUINT32) m_pIMemDrv);
    enableLscWoVariable (MFALSE);
    {
        ISP_NVRAM_LSC_T debug;
        ISP_MGR_LSC_T::getInstance(m_eActive).get(debug);
        MY_LOG("[LscMgr]  %s"
                "LscMgr() Shading param 0x%x, 0x%x, 0x%x, 0x%x,0x%x ,0x%x ,0x%x ,0x%x, 0x%x \n",
                __FUNCTION__,
                debug.lsci_en.val,
                debug.baseaddr.val,
                debug.lsc_en.val,
                debug.ctl1.val, debug.ctl2.val,
                debug.ctl3.val, debug.lblock.val,
                debug.ratio.val,
                debug.gain_th.val);
    }

    MY_LOG("[LscMgr] "
            "EXIT ~LscMgr(m_pIMemDrv - 0x%8x) <<\n", (MUINT32) m_pIMemDrv);
}

MBOOL
LscMgr::uninit()
{
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    m_u4CTIdx = 0;
    m_eSensorOp=ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    m_ePrevSensorOp=ACDK_SCENARIO_ID_MAX;
    m_eIspProfile=EIspProfile_NUM;
    m_eLscScenario=LSC_SCENARIO_01;
    m_ePrevLscScenario=LSC_SCENARIO_NUM;
    m_bIsEEPROMImported=MFALSE;
    m_bIsLutLoaded=MFALSE;
    m_bMetaMode = MFALSE;

#if ENABLE_TSF
    m_bTSF = MFALSE;
    if (mTSFState != LSCMGR_TSF_STATE_EXIT)
    {
        ::pthread_mutex_lock(&mTSFMutex);
        mTSFState = LSCMGR_TSF_STATE_EXIT;
        ::pthread_mutex_unlock(&mTSFMutex);
        ::sem_post(&mTSFSem);
        ::pthread_join(mTSFThread, NULL);
    }
    m_bTSF = isEnableTSF();
#endif

    RawLscTblUnInit();

    if (m_pSensorHal)
    {
        m_pSensorHal->uninit();
        m_pSensorHal->destroyInstance();
        m_pSensorHal = NULL;
    }

    MY_LOG("[LscMgr:%s] (m_pIMemDrv:0x%8x) (m_pSensorHal:0x%0x) \n", __FUNCTION__,
            (MUINT32) m_pIMemDrv, (MUINT32)m_pSensorHal);

    return MTRUE;
}

#if ENABLE_TSF
MVOID
LscMgr::
fillTSFLscConfig(MTK_TSF_LSC_PARAM_STRUCT &config, EIspProfile_T profile)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario %d, %d", __FUNCTION__,
                m_eSensorOp,
                m_eLscScenario);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    config.raw_ht           = m_SensorCrop[sensor_scenario].u4CropH;//1902;
    config.raw_wd           = m_SensorCrop[sensor_scenario].u4CropW;//2532;
    config.x_offset         = m_rIspLscCfg[lsc_scenario].ctl1.bits.SDBLK_XOFST;//0;
    config.y_offset         = m_rIspLscCfg[lsc_scenario].ctl1.bits.SDBLK_YOFST;//0;
    config.block_wd         = m_rIspLscCfg[lsc_scenario].ctl2.bits.SDBLK_WIDTH;//79;   // half size
    config.block_ht         = m_rIspLscCfg[lsc_scenario].ctl3.bits.SDBLK_HEIGHT;//59;
    config.x_grid_num       = (MUINT16)m_rIspLscCfg[lsc_scenario].ctl2.bits.SDBLK_XNUM+2;//17;
    config.y_grid_num       = (MUINT16)m_rIspLscCfg[lsc_scenario].ctl3.bits.SDBLK_YNUM+2;//17;
    config.block_wd_last    = m_rIspLscCfg[lsc_scenario].lblock.bits.SDBLK_lWIDTH;//81;
    config.block_ht_last    = m_rIspLscCfg[lsc_scenario].lblock.bits.SDBLK_lHEIGHT;//66;

    MY_LOG("[%s] lsc, sensor %d, %d, \n"
            "(raw_wd, raw_ht, block_wd, block_ht, xgrid, ygrid, wd_last, ht_last) = \n"
            "(%d, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x) \n", __FUNCTION__,
            lsc_scenario,
            sensor_scenario,
            config.raw_wd,
            config.raw_ht,
            config.block_wd,
            config.block_ht,
            config.x_grid_num,
            config.y_grid_num,
            config.block_wd_last,
            config.block_ht_last);
}

MVOID
LscMgr::
fillTSFInitParams(MTK_TSF_ENV_INFO_STRUCT &params)
{
    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);

    // LSC table spec
    fillTSFLscConfig(*params.pLscConfig, m_eIspProfile);

    // General data
    params.ImgWidth         = AWB_WINDOW_NUM_X;
    params.ImgHeight        = AWB_WINDOW_NUM_Y;
    params.BayerOrder       = MTK_BAYER_B;
    // awb image statistics
    params.ImgAddr          = (MUINT8 *)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr;
    // shading table 16x16 or 32x32 coeff
    params.ShadingTbl       = (MINT32*)((MINT8*)(getLut(m_eLscScenario))+getTSFD65Idx()*getPerLutSize(m_eLscScenario));
    params.Para = (MUINT32*)getTSFTrainingData();
    params.pTuningPara = (MINT32*)getTSFTuningData();
    params.Raw16_9Mode      = 0;

    params.TS_TS               =  1;
    params.MA_NUM              =  5;

    MY_LOG("[%s]"
            "(ImgWidth, ImgHeight, BayerOrder, ImgAddr, ShadingTbl, Raw16_9Mode) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            params.ImgWidth,
            params.ImgHeight,
            params.BayerOrder,
            // awb image
            params.ImgAddr,
            // shading table
            params.ShadingTbl,
            params.Raw16_9Mode);

    MY_LOG("[%s] \n"
            "(raw_wd, raw_ht, block_wd, block_ht, xgrid, ygrid, wd_last, ht_last) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            params.pLscConfig->raw_wd,
            params.pLscConfig->raw_ht,
            params.pLscConfig->block_wd,
            params.pLscConfig->block_ht,
            params.pLscConfig->x_grid_num,
            params.pLscConfig->y_grid_num,
            params.pLscConfig->block_wd_last,
            params.pLscConfig->block_ht_last);
}

MVOID
LscMgr::
updateTSFParamByIspProfile(MTK_TSF_ENV_INFO_STRUCT &params, EIspProfile_T profile)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    MY_LOG("[LscMgr:%s] \n", __FUNCTION__);
    // LSC table spec
    fillTSFLscConfig(*params.pLscConfig, profile);

    // General data
    params.ImgWidth         = AWB_WINDOW_NUM_X;
    params.ImgHeight        = AWB_WINDOW_NUM_Y;

    switch(m_SensorCrop[sensor_scenario].DataFmt)
    {
        case 0:
            params.BayerOrder = MTK_BAYER_B;
            break;
        case 1:
            params.BayerOrder = MTK_BAYER_Gb;
            break;
        case 2:
            params.BayerOrder = MTK_BAYER_Gr;
            break;
        case 3:
            params.BayerOrder = MTK_BAYER_R;
            break;
    }

    // shading table 16x16 or 32x32 coeff
    params.ShadingTbl       = (MINT32*)((MINT8*)(getLut(m_eLscScenario))+getTSFD65Idx()*getPerLutSize(m_eLscScenario));;//(MINT32 *)stRawLscInfo[lsc_scenario].virtAddr;

    if (lsc_scenario == LSC_SCENARIO_09_17)
        params.Raw16_9Mode      = 1;
    else
        params.Raw16_9Mode      = 0;

    MY_LOG("[%s] lsc, sensor %d, %d, \n"
            "(ImgWidth, ImgHeight, BayerOrder, ImgAddr, ShadingTbl, Raw16_9Mode) = \n"
            "(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ) \n", __FUNCTION__,
            lsc_scenario,
            sensor_scenario,
            params.ImgWidth,
            params.ImgHeight,
            params.BayerOrder,
            // awb image
            params.ImgAddr,
            // shading table
            params.ShadingTbl,
            params.Raw16_9Mode);
}

MVOID
LscMgr::
updateTSFInputParam(MTK_TSF_SET_PROC_INFO_STRUCT &params)
{
    //MY_LOG("[LscMgr:%s] \n", __FUNCTION__);
    params.ParaL       = m_i4LV;
    params.ParaC       = m_u4CCT;
    params.ShadingTbl  = (MINT32*)getTSFInputAddr(m_eIspProfile);
}

MVOID
LscMgr::
prepareTSFInputBuffer(EIspProfile_T profile, LSCMGR_TSF_STATE state)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(profile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && profile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    MY_LOG("[LscMgr:%s] EIspProfile_T %d, sensor %d, lsc %d\n", __FUNCTION__,
            profile,
            sensor_scenario,
            lsc_scenario);

    if (profile == EIspProfile_NormalCapture && state == LSCMGR_TSF_STATE_GETNEWINPUT) {
        memcpy((void *)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                (void *)(stRawLscInfo[lsc_scenario].virtAddr+getCTIdx()*getPerLutSize(lsc_scenario)),
                getPerLutSize(lsc_scenario));
    } else {
        if (!(EIspProfile_NormalPreview == profile)) {
            MY_LOG("[LscMgr:%s] Need BAK from input vir 0x%08x to bak vir 0x%08x, size %d\n", __FUNCTION__,
                    m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                    m_TSFBuff[TSF_BUFIDX_BAK].virtAddr,
                    getPerLutSize(m_ePrevLscScenario));
            memcpy((void *)m_TSFBuff[TSF_BUFIDX_BAK].virtAddr, (void *)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                    getPerLutSize(m_ePrevLscScenario));

            MY_LOG("[LscMgr:%s] from table vir 0x%08x to input vir 0x%08x, size %d\n", __FUNCTION__,
                    stRawLscInfo[lsc_scenario].virtAddr+getCTIdx()*getPerLutSize(lsc_scenario),
                    m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                    getPerLutSize(lsc_scenario));
#if USING_BUILTIN_LSC
            memcpy((void *)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                    (void *)(getLut(lsc_scenario)),
                    getPerLutSize(lsc_scenario)); // D65?
#else
            memcpy((void *)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr,
                    (void *)(stRawLscInfo[lsc_scenario].virtAddr+getCTIdx()*getPerLutSize(lsc_scenario)),
                    getPerLutSize(lsc_scenario));
#endif
        } else {
            MY_LOG("[LscMgr:%s] EIspProfile_NormalPreview, from bak 0x%08x to input 0x%08x\n", __FUNCTION__,
                    m_TSFBuff[TSF_BUFIDX_BAK].virtAddr,
                    m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr);
            memcpy((void *)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr, (void *)m_TSFBuff[TSF_BUFIDX_BAK].virtAddr,
                    m_TSFBuff[TSF_BUFIDX_BAK].size);
        }
    }


    UINT32 last = getPerLutSize(lsc_scenario)/4 - 4;
    MY_LOG("[LscMgr:%s] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+0),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+1),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+2),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+3));
    MY_LOG("[LscMgr:%s] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+last+0),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+last+1),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+last+2),
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+last+3));
}

MUINT32
LscMgr::
getTSFInputAddr(EIspProfile_T profile)
{
    return m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr; // D65?
}

MUINT32
LscMgr::
getTSFOutputAddr(EIspProfile_T profile)
{
    return m_TSFBuff[TSF_BUFIDX_OUTPUT].virtAddr; // D65?
}


MBOOL
LscMgr::
checkAspectRatioChange(void)
{
    EIspProfile_T prev = m_ePrevIspProfile, cur = m_eIspProfile;
    ACDK_SCENARIO_ID_ENUM prev_idx = getSensorScenarioByIspProfile(prev);
    ACDK_SCENARIO_ID_ENUM cur_idx = getSensorScenarioByIspProfile(cur);
    MUINT32 prevH, prevW, curH, curW;
    prevH = m_SensorCrop[prev_idx].u4CropH;
    prevW = m_SensorCrop[prev_idx].u4CropW;
    curH = m_SensorCrop[cur_idx].u4CropH;
    curW = m_SensorCrop[cur_idx].u4CropW;

    MY_LOG("[LscMgr:%s]  prev H/W %d/%d, cur H/W %d/%d", __FUNCTION__,
            prevH, prevW,
            curH, curW);
    if (prevW * curH == prevH * curW) {
        return MFALSE;
    } else {
        return MTRUE;
    }
}

MBOOL
LscMgr::
isTSFEnable(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 manual_tsf = 0;
    property_get("debug.lsc_mgr.manual_tsf", value, "-1");
    manual_tsf = atoi(value);

    if (manual_tsf != -1)
        if (manual_tsf == 0)
            m_bTSF = MFALSE;
        else
            m_bTSF = MTRUE;

    return m_bTSF;
}

MVOID
LscMgr::
dumpTSFInput(void)
{
    ACDK_SCENARIO_ID_ENUM sensor_scenario = getSensorScenarioByIspProfile(m_eIspProfile);
    ELscScenario_T lsc_scenario =  getLscScenarioBySensorScenario(sensor_scenario);

    if (m_bMetaMode == MTRUE && m_eIspProfile == EIspProfile_NormalCapture) {
        MY_LOG("[LscMgr:%s] MetaMode reassign sensor/lsc scenario", __FUNCTION__);
        sensor_scenario = m_eSensorOp;
        lsc_scenario = m_eLscScenario;
    }

    UINT32 size = getPerLutSize(lsc_scenario)/4;
    for (UINT32 i = 0; i < 16; i+=4) {
        MY_LOG("[LscMgr:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+3));
    }

    for (UINT32 i = size-32; i < size; i+=4) {
        MY_LOG("[LscMgr:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+i+3));
    }
}

void CheckTable(MUINT32* input, MUINT32* output, MUINT32 U32length) {
    for (UINT32 i = 0; i < 4; i+=4) {
        MY_LOG("[input:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)input+i+0),
                *((MUINT32*)input+i+1),
                *((MUINT32*)input+i+2),
                *((MUINT32*)input+i+3));
    }

    for (UINT32 i = U32length-4; i < U32length; i+=4) {
        MY_LOG("[input:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)input+i+0),
                *((MUINT32*)input+i+1),
                *((MUINT32*)input+i+2),
                *((MUINT32*)input+i+3));
    }

    ///////////////////////////////////////
    for (UINT32 i = 0; i < 4; i+=4) {
        MY_LOG("[output:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)output+i+0),
                *((MUINT32*)output+i+1),
                *((MUINT32*)output+i+2),
                *((MUINT32*)output+i+3));
    }

    for (UINT32 i = U32length-4; i < U32length; i+=4) {
        MY_LOG("[output:%s: %d-%d] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                i,i+3,
                *((MUINT32*)output+i+0),
                *((MUINT32*)output+i+1),
                *((MUINT32*)output+i+2),
                *((MUINT32*)output+i+3));
    }
}
/////////////////////////////////
// TSF state machine
/////////////////////////////////
void *
LscMgr::
mThreadLoop(void *arg)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 dbg_tsf = 0;
    property_get("debug.lsc_mgr.dbg_tsf", value, "0");
    dbg_tsf = atoi(value);

    LscMgr *lsc = reinterpret_cast<LscMgr*>(arg);
    MY_LOG("[LscMgr:%s]  start state %d", __FUNCTION__, lsc->mTSFState);
    ::prctl(PR_SET_NAME,"Cam@3A-Lsc", 0, 0, 0);
    // set policy/priority
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_F858_THREAD;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
#endif 
    // init thread variables
    ::pthread_mutex_init(&lsc->mTSFMutex, NULL);
    ::pthread_mutex_init(&lsc->mTSFMutexSC, NULL);
    ::sem_init(&lsc->mTSFSem, 0, 0);
    ::sem_init(&lsc->mTSFSemSC, 0, 0);

    MRESULT ret = S_TSF_OK;
    ////////////////////////////////////////////////
    static MTK_TSF_ENV_INFO_STRUCT         TSFInit;
    static MTK_TSF_GET_ENV_INFO_STRUCT     TSFGetEnv;
    static MTK_TSF_SET_PROC_INFO_STRUCT    TSFInput;
    static MTK_TSF_RESULT_INFO_STRUCT      TSFOutput;
    static MTK_TSF_GET_PROC_INFO_STRUCT    TSFGetProc;
    static MTK_TSF_GET_LOG_INFO_STRUCT     TSFGetLog;
    static MTK_TSF_LSC_PARAM_STRUCT        LscConfig;
    static MTKTSF_STATE_ENUM               TSFProcState;
    static MTK_TSF_TBL_STRUCT              TSFUpdateInfo;
    static MTK_TSF_LSC_PARAM_STRUCT        UpdateLscConfig;

    MTKTsf* tsf = MTKTsf::createInstance();
    unsigned char* gWorkinBuffer = NULL;
    unsigned char* gDBGWorkinBuffer = NULL;

    // (1) create tsf instance
    if (!tsf)
    {
        MY_LOG("[LscMgr:%s]  NULL TSF instance", __FUNCTION__);
        ::pthread_exit(0);
        return NULL;
    }


    // (2) get/allocate tsf working buffer size
    MY_LOG("[%s] MTKTSF_FEATURE_GET_ENV_INFO ", __FUNCTION__);
    tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &TSFGetEnv);
    MY_LOG("[%s] Queried working buffer size : %d bytes\n", __FUNCTION__, TSFGetEnv.WorkingBuffSize);
    // new working buffer
    gWorkinBuffer   = new unsigned char[TSFGetEnv.WorkingBuffSize];
    memset(gWorkinBuffer,0, TSFGetEnv.WorkingBuffSize);

    // (3) construct data relationship
    TSFInit.WorkingBufAddr = (MUINT32 *)gWorkinBuffer;
    TSFInit.pLscConfig = &LscConfig;

    if (dbg_tsf == 1) {
        gDBGWorkinBuffer = new unsigned char[TSFGetEnv.DebugBuffSize];
        TSFInit.DebugAddr = (MUINT32*)gDBGWorkinBuffer;
        TSFInit.DebugFlag = 1;
    }

    while (lsc->mTSFState != LSCMGR_TSF_STATE_EXIT)
    {
        MBOOL isLscActive = MFALSE;

        ::sem_wait(&lsc->mTSFSem);
        ::pthread_mutex_lock(&lsc->mTSFMutex);
        isLscActive = (lsc->isBypass()==MTRUE)?MFALSE:MTRUE;

        if (isLscActive == MTRUE) {
            isLscActive = lsc->isEnable();
        }

        if (lsc->isTSFEnable() == MFALSE) {
            ::pthread_mutex_unlock(&lsc->mTSFMutex);
            ::sem_post(&lsc->mTSFSemSC);
            continue;
        }

        lsc->updateTSFInputParam(TSFInput);
        TSFOutput.ShadingTbl = (MUINT32*)lsc->getTSFOutputAddr(lsc->getIspProfile());

        switch(lsc->mTSFState)
        {
            case LSCMGR_TSF_STATE_IDLE: //////////////////////////////////////////////////
                MY_LOG("[%s] LSCMGR_TSF_STATE_IDLE ", __FUNCTION__);
                break; // do nothing
            case LSCMGR_TSF_STATE_INIT: //////////////////////////////////////////////////
                MY_LOG("[%s] LSCMGR_TSF_STATE_INIT ", __FUNCTION__);
                // (4) init algorithm params
                if (lsc->loadTSFLut() == MTRUE) {
                    tsf->TsfExit();
                    lsc->fillTSFInitParams(TSFInit);
                    ret = tsf->TsfInit(&TSFInit, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("TSF init error %x", ret);
                    } else {
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                        } else
                            lsc->mTSFState = LSCMGR_TSF_STATE_SCENECHANGE;
                    }
                } else {
                    ::sem_post(&lsc->mTSFSemSC);
                    MY_ERR("[%s] loadTSFLut fail", __FUNCTION__);
                }
                break;
            case LSCMGR_TSF_STATE_SCENECHANGE:  //////////////////////////////////////////////////
                // cur lsc scenario
                MY_LOG("[%s] LSCMGR_TSF_STATE_SCENECHANGE ", __FUNCTION__);
                lsc->prepareTSFInputBuffer(lsc->getIspProfile(), LSCMGR_TSF_STATE_SCENECHANGE);
                lsc->fillTSFLscConfig(UpdateLscConfig, lsc->getIspProfile());
                TSFUpdateInfo.pLscConfig = &UpdateLscConfig;
                TSFUpdateInfo.ShadingTbl = (MINT32*)lsc->getTSFInputAddr(lsc->getIspProfile());//D65���������������������������shading table (Capture size);

                if (EIspProfile_NormalPreview != lsc->getIspProfile()) {
                    MY_LOG("[%s] EIspProfile_NormalPreview != lsc->getIspProfile() ", __FUNCTION__);
                    if (lsc->checkAspectRatioChange() == MTRUE) {
                        MY_LOG("[LscMgr:%s] diff AspecRatio TsfExit", __FUNCTION__);
                        tsf->TsfExit();
                        lsc->fillTSFInitParams(TSFInit);
                        ret = tsf->TsfInit(&TSFInit, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("TSFInit error %x", ret);
                        } else {
                            ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                            if (ret != S_TSF_OK) {
                                MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                            } else
                                lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                        }
                    }
                    else  if (EIspProfile_NormalCapture == lsc->getIspProfile())
                    {
                        MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_SET_TBL_CHANGE\n", __FUNCTION__);
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                        }
                        MY_LOG("[LscMgr:%s] Normal Capture, MTKTSF_FEATURE_GEN_CAP_TBL\n", __FUNCTION__);
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                        } else {
                            MY_LOG("[%s] memcpy src 0x%0x, dst 0x%0x, size(U8) %d", __FUNCTION__,
                                    TSFOutput.ShadingTbl,
                                    TSFInput.ShadingTbl,
                                    lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                            ::memcpy((void*)TSFInput.ShadingTbl,
                                    (void*)TSFOutput.ShadingTbl,
                                    lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                            lsc->SetTBAToISP();
                            lsc->enableLsc(isLscActive);
                            MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!!\n", __FUNCTION__);
                        }

                        if (EIspProfile_VideoPreview == lsc->getPrevIspProfile() ||
                                EIspProfile_VideoCapture == lsc->getPrevIspProfile()) {
                            MY_LOG("[LscMgr:%s] prevIsp EIspProfile_VideoPreview||EIspProfile_VideoCapture", __FUNCTION__);
                            lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                        } else
                            lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                    } else
                        lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                } else {
                    MY_LOG("[LscMgr:%s] Apply previous table immediately", __FUNCTION__);
                    MY_LOG("[LscMgr:%s] Preview, MTKTSF_FEATURE_SET_TBL_CHANGE\n", __FUNCTION__);
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    lsc->SetTBAToISP();
                    lsc->enableLsc(isLscActive);
                    lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                }

//                {   // experiment on pass2 data flow
//                    ISP_NVRAM_OBC_T obc;
//                    //memset(&obc, 0, sizeof(ISP_NVRAM_OBC_T));
//                    ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).get(obc);
//                    obc.offst0.val = obc.offst1.val = obc.offst2.val = obc.offst3.val = 0;
//                    ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).put(obc);
//                    ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalCapture);
//                }
                break;
            case LSCMGR_TSF_STATE_GETNEWINPUT:  //////////////////////////////////////////////////
                // recalculate shading table according new input
                MY_LOG("[%s] LSCMGR_TSF_STATE_GETNEWINPUT ", __FUNCTION__);
                lsc->prepareTSFInputBuffer(lsc->getIspProfile(), LSCMGR_TSF_STATE_GETNEWINPUT);
                lsc->fillTSFLscConfig(UpdateLscConfig, lsc->getIspProfile());
                lsc->updateTSFInputParam(TSFInput);
                TSFUpdateInfo.pLscConfig = &UpdateLscConfig;
                TSFUpdateInfo.ShadingTbl = (MINT32*)lsc->getTSFInputAddr(lsc->getIspProfile());//D65���������������������������shading table (Capture size);
                TSFOutput.ShadingTbl = (MUINT32*)lsc->getTSFOutputAddr(lsc->getIspProfile());

                if (EIspProfile_NormalCapture == lsc->getIspProfile())
                {
#if 0
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                    } else {
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!!\n", __FUNCTION__);
                    }
#else
                    MY_LOG("[%s] MTKTSF_FEATURE_BATCH Intput/Output tbl 0x%08x/0x%08x", __FUNCTION__,
                            TSFInput.ShadingTbl,
                            TSFOutput.ShadingTbl);
                    //CheckTable((MUINT32*)TSFInput.ShadingTbl, (MUINT32*)TSFOutput.ShadingTbl, lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx())/4);

                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &TSFInput, &TSFOutput);
                    MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_BATCH complete!!\n", __FUNCTION__);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_BATCH error %x", ret);
                        lsc->dumpTSFInput();
                        tsf->TsfReset();
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    } else {
                        MY_LOG("[%s] memcpy src 0x%0x, dst 0x%0x, size %d", __FUNCTION__,
                                TSFOutput.ShadingTbl,
                                TSFInput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                        if (dbg_tsf == 1) {
                            char *filename = "/sdcard/tsfdata/TSFCap.bin";
                            MY_LOG("[LscMgr:%s] DBG: Output Capture Table to %s", __FUNCTION__, filename);
                            FILE* fpdebug = fopen(filename,"wb");
                            if ( fpdebug == NULL )
                            {
                                MY_ERR("Can't open :%s\n",filename);
                            } else {
                                fwrite((void*)TSFOutput.ShadingTbl,
                                        lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()),
                                        1,fpdebug);
                                fclose(fpdebug);
                            }
                        }
                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] Exit LSCMGR_TSF_STATE_GETNEWINPUT!!\n", __FUNCTION__);
                    }
#endif
//                    {   // experiment on pass2 data flow
//                        ISP_NVRAM_OBC_T obc;
//                        //memset(&obc, 0, sizeof(ISP_NVRAM_OBC_T));
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).get(obc);
//                        obc.offst0.val = obc.offst1.val = obc.offst2.val = obc.offst3.val = 0;
//                        obc.gain0.val = obc.gain1.val = obc.gain2.val = obc.gain3.val = 512;
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).put(obc);
//                        ISP_MGR_OBC_T::getInstance(NSIspTuning::ESensorDev_Main).apply(NSIspTuning::EIspProfile_NormalCapture);
//                    }

                    ::sem_post(&lsc->mTSFSemSC);
                    lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                }
                else
                {  // normal preview case
                    MY_LOG("[LscMgr:%s] Non Capture\n", __FUNCTION__);
                    MY_LOG("[%s] MTKTSF_FEATURE_SET_TBL_CHANGE ", __FUNCTION__);
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    } else {
                        MY_LOG("[%s] MTKTSF_FEATURE_SET_PROC_INFO ", __FUNCTION__);
                        ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                        if (ret != S_TSF_OK) {
                            MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                        } else {
                            lsc->mTSFState = LSCMGR_TSF_STATE_DO;
                        }
                    }
                }
                break;
            case LSCMGR_TSF_STATE_DO:   //////////////////////////////////////////////////
                if (EIspProfile_NormalCapture == lsc->getIspProfile())
                {
#if 1 // recalculate
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_TBL_CHANGE error %x", ret);
                    }
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &TSFInput, &TSFOutput);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_BATCH error %x", ret);
                        lsc->dumpTSFInput();
                        tsf->TsfReset();
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &TSFUpdateInfo, 0);
                    } else {
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));

                        lsc->SetTBAToISP();
                        lsc->enableLsc(isLscActive);
                        MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_BATCH complete!!\n", __FUNCTION__);
                    }
#else
                    // no smooth needed
                    MY_LOG("[LscMgr:%s] Continue Shot, MTKTSF_FEATURE_GEN_CAP_TBL\n", __FUNCTION__);
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &TSFUpdateInfo, &TSFOutput);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_GEN_CAP_TBL error %x", ret);
                    } else {
                        ::memcpy((void*)TSFInput.ShadingTbl,
                                (void*)TSFOutput.ShadingTbl,
                                lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                        MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GEN_CAP_TBL complete!!\n", __FUNCTION__);
                        lsc->SetTBAToISP();
                        lsc->enableLsc(MTRUE);
                    }
#endif // recalculate
                    ::sem_post(&lsc->mTSFSemSC); // for continue shot
                } else {
                    ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &TSFInput, 0);
                    if (ret != S_TSF_OK) {
                        MY_ERR("MTKTSF_FEATURE_SET_PROC_INFO error %x", ret);
                    } else {
                        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &TSFGetProc);
                        if (TSFGetProc.TsfState != MTKTSF_STATE_READY) {
                            ret = tsf->TsfMain();
                            if (ret != S_TSF_OK) {
                                MY_ERR("TsfMain error %x", ret);
                                lsc->dumpTSFInput();
                                tsf->TsfReset();
                                lsc->mTSFState = LSCMGR_TSF_STATE_GETNEWINPUT;
                            }
                        } else {
                            ret = tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, 0, &TSFOutput);
                            if (ret != S_TSF_OK) {
                                MY_ERR("MTKTSF_FEATURE_GET_RESULT error %x", ret);
                                tsf->TsfReset();
                            } else {
                                MY_LOG("[LscMgr:%s] MTKTSF_FEATURE_GET_RESULT \n", __FUNCTION__);
                                MY_LOG("[%s] Copy output 0x%08x back to input 0x%08x\n", __FUNCTION__,
                                        (MUINT32)TSFOutput.ShadingTbl,
                                        (MUINT32)TSFInput.ShadingTbl);

                                memcpy((void*)TSFInput.ShadingTbl,
                                        (void*)TSFOutput.ShadingTbl,
                                        lsc->getPerLutSize((NSIspTuning::LscMgr::ELscScenario_T)lsc->getRegIdx()));
                                lsc->SetTBAToISP();
                                lsc->enableLsc(isLscActive);
                                tsf->TsfReset();
                            }
                        }
                    }
                }
                break;
            default:    //////////////////////////////////////////////////
                MY_LOG("[LscMgr:%s] YOU SHOULD NOT SEE ME!!!", __FUNCTION__);
                break;
        }
        ::pthread_mutex_unlock(&lsc->mTSFMutex);
    };

    if (dbg_tsf == 1) {
        char *filename = "/sdcard/tsfdata/TSFDebug.bin";
        tsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_LOG, 0, &TSFGetLog);
        FILE* fpdebug = fopen(filename,"wb");
        if ( fpdebug == NULL )
        {
            printf("Can't open :%s\n",filename);
        } else {
            fwrite((void*)TSFGetLog.DebugBuffAddr, TSFGetLog.DebugBuffSize, 1,fpdebug);
            fclose(fpdebug);
        }
    }
    //============================== destory instance ================================//
    tsf->TsfExit();
    tsf->destroyInstance();
    //================================================================================//
    delete [] gWorkinBuffer;
    if (gDBGWorkinBuffer)
        delete [] gDBGWorkinBuffer;


    MY_LOG("[LscMgr:%s]  end", __FUNCTION__);
    ::pthread_exit(0);
    return NULL;
}

/////////////////////////////////////
// TSF state machine control
/////////////////////////////////////
static MBOOL TSFStateRule[NSIspTuning::LscMgr::LSCMGR_TSF_STATE_NUM][NSIspTuning::LscMgr::LSCMGR_TSF_STATE_NUM] =
{
        ////////////////IDLE    INIT,  SCENECHANGE, GETNEWINPUT, DO,       EXIT
        /*IDLE*/        {MTRUE, MTRUE, MFALSE,      MFALSE,      MFALSE,   MTRUE},
        /*INIT*/        {MTRUE, MTRUE, MFALSE,      MFALSE,      MFALSE,   MTRUE},
        /*SCENECHANGE*/ {MTRUE, MFALSE,MTRUE,       MFALSE,      MFALSE,   MTRUE},
        /*GETNEWINPUT*/ {MTRUE, MFALSE,MFALSE,      MTRUE,       MFALSE,   MTRUE},
        /*DO*/          {MTRUE, MFALSE,MTRUE,       MFALSE,      MTRUE,    MTRUE},
        /*EXIT*/        {MTRUE, MFALSE,MFALSE,      MFALSE,      MFALSE,   MTRUE},
};

MBOOL
LscMgr::
changeTSFState(LSCMGR_TSF_STATE state)
{
    MBOOL ret = MFALSE;
    MY_LOG("[LscMgr:%s]  old/new state %d/%d", __FUNCTION__, mTSFState, state);

    ::pthread_mutex_lock(&mTSFMutex);
    if (TSFStateRule[mTSFState][state] == MTRUE)
    {
        mTSFState = state;
        ret = MTRUE;
    }
    else {
        MY_ERR("[LscMgr:%s]  Invalid transition!", __FUNCTION__);
        ret = MFALSE;
    }
    ::sem_post(&mTSFSem);
    ::pthread_mutex_unlock(&mTSFMutex);

    //////////////////////////////// make sure scene change processed done
    if (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && m_bMetaMode == MTRUE) {
        MUINT32 check_cnt = 0;
        do {
            usleep(100);
            check_cnt++;
        } while (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && check_cnt < 100);
        MY_LOG("[LscMgr:%s] check_cnt %d", __FUNCTION__, check_cnt);
    }
    ////////////////////////////////
    return ret;
}

//////////////////////////////////////
// when TSF finished a cycle, call this
// function to change buffer
//////////////////////////////////////
MVOID
LscMgr::
updateTSFBuffIdx(void)
{

}

MVOID
LscMgr::
enableTSF(MBOOL enable) {
    MY_LOG("[LscMgr:%s] enable %d", __FUNCTION__,
            enable);
    m_bTSF = enable;
}

MBOOL
LscMgr::
loadTSFLut(void) {

#if USING_BUILTIN_LSC
    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr),
            reinterpret_cast<MVOID*>(getLut(m_eLscScenario)),
            getPerLutSize(m_eLscScenario));

    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_BAK].virtAddr),
            reinterpret_cast<MVOID*>(getLut(m_eLscScenario)),
            getPerLutSize(m_eLscScenario));
#else
    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr),
            reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_01].virtAddr + getTSFD65Idx()*getPerLutSize(LSC_SCENARIO_01)),
            getPerLutSize(LSC_SCENARIO_01));

    ::memcpy(reinterpret_cast<MVOID*>(m_TSFBuff[TSF_BUFIDX_BAK].virtAddr),
            reinterpret_cast<MVOID*>(stRawLscInfo[LSC_SCENARIO_01].virtAddr + getTSFD65Idx()*getPerLutSize(LSC_SCENARIO_01)),
            getPerLutSize(LSC_SCENARIO_01));
#endif

    if (*((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+0) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+1) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+2) == 0 &&
            *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+3) == 0) {
        MY_ERR("[LscMgr:%s] Default table is ZERO!!", __FUNCTION__);
        return MFALSE;
    } else {
        MY_LOG("[LscMgr:%s] 0x%08x/0x%08x/0x%08x/0x%08x", __FUNCTION__,
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+0),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+1),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+2),
                *((MUINT32*)m_TSFBuff[TSF_BUFIDX_INPUT].virtAddr+3));
        return MTRUE;
    }
}

#endif // ENABLE_TSF
//////////////////////////////////////
// update AWB statistics
//////////////////////////////////////
MBOOL
LscMgr::
updateTSFinput(LSCMGR_TSF_INPUT_SRC src, MINT32 lv, MUINT32 cct, MVOID *stat)
{
#if ENABLE_TSF
    //    MY_LOG("[LscMgr:%s] lv %d, addr 0x%08x", __FUNCTION__,lv, stat);
    static unsigned long FrameCnt = 0;

    if (isTSFEnable() == MFALSE)
        return MTRUE;

    ::pthread_mutex_lock(&mTSFMutex);
    FrameCnt++;
    // update parameters
    m_i4LV = lv;
    m_u4CCT = cct;
    AWB_STAT_T *pAWBStat = reinterpret_cast<AWB_STAT_T *>(stat);

    for (int y = 0; y < AWB_WINDOW_NUM_Y; y++) {
        memcpy((void*)(m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+(y*sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X)), (void*)&(pAWBStat->LINE[y]), sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X);
    }

//    for (int i = sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X; i < sizeof(AWB_WINDOW_T)*AWB_WINDOW_NUM_X+64; i+=4)
//        MY_LOG("[LscMgr:%s] 0x%02x 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+0),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+1),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+2),
//                *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+3));
    ::sem_post(&mTSFSem);
    ::pthread_mutex_unlock(&mTSFMutex);

    switch (src)
    {
        case TSF_INPUT_PV:
//            MY_LOG("[LscMgr:%s:%d]  state %d, TSF_INPUT_PV, lv %d, cct %d, addr 0x%08x", __FUNCTION__,
//                    FrameCnt,
//                    mTSFState,
//                    lv,
//                    cct,
//                    stat);
//        { // DEBUG
//            char *filename = "/sdcard/tsfdata/PVAWB.bin";
//
//            FILE* fpdebug = fopen(filename,"wb");
//            if ( fpdebug == NULL )
//            {
//                printf("Can't open :%s\n",filename);
//            } else {
//                fwrite((void*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr, m_TSFBuff[TSF_BUFIDX_AWB].size, 1,fpdebug);
//                fclose(fpdebug);
//            }
//        }
            break;
        case TSF_INPUT_CAP:
            MY_LOG("[LscMgr:%s:%d]  state %d, TSF_INPUT_CAP, lv %d, cct %d, addr 0x%08x", __FUNCTION__,
                    FrameCnt,
                    mTSFState,
                    lv,
                    cct,
                    stat);
            MY_LOG("[LscMgr:%s] Wait TSF thread", __FUNCTION__);
            //            for (int i = 0; i < 64; i+=4)
            //                MY_LOG("[LscMgr:%s] 0x%02x 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+0),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+1),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+2),
            //                        *((MUINT8*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr+i+3));
            ::sem_wait(&mTSFSemSC);
            //////////////////////////////// Double check
            if (mTSFState == LSCMGR_TSF_STATE_GETNEWINPUT && m_bMetaMode == MTRUE) {
                MY_LOG("[LscMgr:%s] Weird!!!", __FUNCTION__);
                MUINT32 check_cnt = 0;
                do {
                    usleep(100);
                    check_cnt++;
                } while (mTSFState == LSCMGR_TSF_STATE_SCENECHANGE && check_cnt < 100);
                MY_LOG("[LscMgr:%s] check_cnt %d", __FUNCTION__, check_cnt);
            }
            ////////////////////////////////

//            { // DEBUG
//                char *filename = "/sdcard/tsfdata/CapAWB.bin";
//
//                FILE* fpdebug = fopen(filename,"wb");
//                if ( fpdebug == NULL )
//                {
//                    printf("Can't open :%s\n",filename);
//                } else {
//                    fwrite((void*)m_TSFBuff[TSF_BUFIDX_AWB].virtAddr, m_TSFBuff[TSF_BUFIDX_AWB].size, 1,fpdebug);
//                    fclose(fpdebug);
//                }
//            }
            MY_LOG("[LscMgr:%s] Wait TSF complete", __FUNCTION__);
            break;
        case TSF_INPUT_VDO:
            //	        MY_LOG("[LscMgr:%s:%d]  state %d, TSF_INPUT_VDO, lv %d, cct %d, addr 0x%08x", __FUNCTION__,
            //	               FrameCnt,
            //	               mTSFState,
            //	               lv,
            //	               cct,
            //	               stat);
            break;
    }
#endif
    return MTRUE;
}

} /* namespace NSIspTuning */
