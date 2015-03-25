#ifndef _ISP_TUNING_CUSTOM_H_
#define _ISP_TUNING_CUSTOM_H_

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
class IspTuningCustom
{
protected:  ////    Ctor/Dtor.
    IspTuningCustom() {}
    virtual ~IspTuningCustom() {}

public:
    static IspTuningCustom* createInstance(ESensorDev_T const eSensorDev, MUINT32 const u4SensorID);
    virtual void destroyInstance() = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorDev_T   getSensorDev() const = 0;
    virtual MUINT32   getSensorID() const = 0;
    virtual INDEX_T const*  getDefaultIndex(EIspProfile_T const eIspProfile, EIndex_Scene_T const eIdx_Scene, EIndex_ISO_T const eIdx_ISO) const = 0;
    virtual MVOID evaluate_nvram_index(RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr);

public:     ////    Operations.

    template <class T>
	T LIMIT(T const value, T const low_bound, T const upper_bound)
    {
        if (value < low_bound)
        {
            return (low_bound);
        }
        else if (value > upper_bound)
		{
            return (upper_bound);
		}
		else
		{
		    return (value);
		}
    }

    virtual
    MVOID
    refine_OBC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_OBC_T& rOBC
    );

    virtual
    MVOID
    refine_BPC(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_BPC_T& rBPC
    );

    virtual
    MVOID
    refine_NR1(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_NR1_T& rNR1
    );

    virtual
    MVOID
    refine_SL2(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_SL2_T& rSL2
    );

    virtual
    MVOID
    refine_PGN(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_PGN_T& rPGN
    );

    virtual
    MVOID
    refine_CFA(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CFA_T& rCFA
    );

    virtual
    MVOID
    refine_CCM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CCM_T& rCCM
    );

    virtual
    MVOID
    refine_GGM(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_GGM_T& rGGM
    );

    virtual
    MVOID
    refine_ANR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_ANR_T& rANR
    );

    virtual
    MVOID
    refine_CCR(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_CCR_T& rCCR
    );

    virtual
    MVOID
    refine_EE(
        RAWIspCamInfo const& rCamInfo, IspNvramRegMgr const& rIspRegMgr, ISP_NVRAM_EE_T& rEE
    );

    virtual
    EIndex_CCM_T
    evaluate_CCM_index (
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MBOOL
    is_to_invoke_dynamic_ccm(
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    EIndex_PCA_LUT_T
    evaluate_PCA_LUT_index  (
        RAWIspCamInfo const& rCamInfo
    );

    virtual
    MVOID
    userSetting_EE(
        EIndex_Isp_Edge_T eIdx_Edge, ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    userSetting_EFFECT(
        RAWIspCamInfo const& rCamInfo, EIndex_Effect_T const& eIdx_Effect, IspUsrSelectLevel_T const& rIspUsrSelectLevel, ISP_NVRAM_G2C_T& rG2C, ISP_NVRAM_SE_T& rSE
    );

    virtual
    EIndex_ISO_T
    map_ISO_value_to_index(
        MUINT32 const u4Iso
    ) const;

    virtual
    MVOID
    refineLightSourceAWBGainforMultiCCM(
        AWB_GAIN_T& rD65, AWB_GAIN_T& rTL84, AWB_GAIN_T& rCWF, AWB_GAIN_T& rA
    );


    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (
            RAWIspCamInfo const& rCamInfo
    ) const;

};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

