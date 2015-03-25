#ifndef __M4U_LIB_PRIV_H__
#define __M4U_LIB_PRIV_H__

#define __PMEM_WRAP_LAYER_EN__


//====================================
// about portid
//====================================
#define M4U_LARB0_PORTn(n)      ((n)+0)
#define M4U_LARB1_PORTn(n)      ((n)+9)
#define M4U_LARB2_PORTn(n)      ((n)+16)
#define M4U_LARB3_PORTn(n)      ((n)+33)
#define M4U_LARB4_PORTn(n)      ((n)+33)
#define M4U_LARB5_PORTn(n)      ((n)+33)

typedef enum
{
    DISP_OVL_0               =  M4U_LARB0_PORTn(0)    ,
    DISP_RDMA                =  M4U_LARB0_PORTn(1)    ,
    DISP_WDMA                =  M4U_LARB0_PORTn(2)    ,
    MM_CMDQ                  =  M4U_LARB0_PORTn(3)    ,
    MDP_RDMA                 =  M4U_LARB0_PORTn(4)    ,
    MDP_WDMA                 =  M4U_LARB0_PORTn(5)    ,
    MDP_ROTO                 =  M4U_LARB0_PORTn(6)    ,
    MDP_ROTCO                =  M4U_LARB0_PORTn(7)    ,
    MDP_ROTVO                =  M4U_LARB0_PORTn(8)    ,
  
    VDEC_MC_EXT              =  M4U_LARB1_PORTn(0)    ,
    VDEC_PP_EXT              =  M4U_LARB1_PORTn(1)    ,
    VDEC_AVC_MV_EXT          =  M4U_LARB1_PORTn(2)    ,
    VDEC_PRED_RD_EXT         =  M4U_LARB1_PORTn(3)    ,
    VDEC_PRED_WR_EXT         =  M4U_LARB1_PORTn(4)    ,
    VDEC_VLD_EXT             =  M4U_LARB1_PORTn(5)    ,
    VDEC_PP_INT              =  M4U_LARB1_PORTn(6)    ,
  
    CAM_IMGO                 =  M4U_LARB2_PORTn(0)    ,
    CAM_IMG2O                =  M4U_LARB2_PORTn(1)    ,
    CAM_LSCI                 =  M4U_LARB2_PORTn(2)    ,
    CAM_IMGI                 =  M4U_LARB2_PORTn(3)    ,
    CAM_ESFKO                =  M4U_LARB2_PORTn(4)    ,
    CAM_AAO                  =  M4U_LARB2_PORTn(5)    ,
    JPGENC_RDMA              =  M4U_LARB2_PORTn(6)    ,
    JPGENC_BSDMA             =  M4U_LARB2_PORTn(7)    ,
    VENC_RD_COMV             =  M4U_LARB2_PORTn(8)    ,
    VENC_SV_COMV             =  M4U_LARB2_PORTn(9)    ,
    VENC_RCPU                =  M4U_LARB2_PORTn(10)    ,
    VENC_REC_FRM             =  M4U_LARB2_PORTn(11)    ,
    VENC_REF_LUMA            =  M4U_LARB2_PORTn(12)    ,
    VENC_REF_CHROMA          =  M4U_LARB2_PORTn(13)    ,
    VENC_BSDMA               =  M4U_LARB2_PORTn(14)    ,
    VENC_CUR_LUMA            =  M4U_LARB2_PORTn(15)    ,
    VENC_CUR_CHROMA          =  M4U_LARB2_PORTn(16)    ,

    M4U_CLNTMOD_LCDC_UI     =  M4U_LARB3_PORTn(8)   ,

    M4U_PORT_UNKNOWN,

};
#define M4U_CLNTMOD_MAX M4U_PORT_UNKNOWN


//IOCTL commnad
#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_POWER_ON            _IOW(MTK_M4U_MAGICNO, 0, int)
#define MTK_M4U_T_POWER_OFF           _IOW(MTK_M4U_MAGICNO, 1, int)
#define MTK_M4U_T_DUMP_REG            _IOW(MTK_M4U_MAGICNO, 2, int)
#define MTK_M4U_T_DUMP_INFO           _IOW(MTK_M4U_MAGICNO, 3, int)
#define MTK_M4U_T_ALLOC_MVA           _IOWR(MTK_M4U_MAGICNO,4, int)
#define MTK_M4U_T_DEALLOC_MVA         _IOW(MTK_M4U_MAGICNO, 5, int)
#define MTK_M4U_T_INSERT_TLB_RANGE    _IOW(MTK_M4U_MAGICNO, 6, int)
#define MTK_M4U_T_INVALID_TLB_RANGE   _IOW(MTK_M4U_MAGICNO, 7, int)
#define MTK_M4U_T_INVALID_TLB_ALL     _IOW(MTK_M4U_MAGICNO, 8, int)
#define MTK_M4U_T_MANUAL_INSERT_ENTRY _IOW(MTK_M4U_MAGICNO, 9, int)
#define MTK_M4U_T_CACHE_SYNC          _IOW(MTK_M4U_MAGICNO, 10, int)
#define MTK_M4U_T_CONFIG_PORT         _IOW(MTK_M4U_MAGICNO, 11, int)
#define MTK_M4U_T_CONFIG_ASSERT       _IOW(MTK_M4U_MAGICNO, 12, int)
#define MTK_M4U_T_INSERT_WRAP_RANGE   _IOW(MTK_M4U_MAGICNO, 13, int)
#define MTK_M4U_T_MONITOR_START       _IOW(MTK_M4U_MAGICNO, 14, int)
#define MTK_M4U_T_MONITOR_STOP        _IOW(MTK_M4U_MAGICNO, 15, int)
#define MTK_M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MTK_M4U_MAGICNO, 16, int)
#define MTK_M4U_T_CONFIG_PORT_ROTATOR _IOW(MTK_M4U_MAGICNO, 17, int)
#define MTK_M4U_T_QUERY_MVA           _IOW(MTK_M4U_MAGICNO, 18, int)
#define MTK_M4U_T_M4UDrv_CONSTRUCT    _IOW(MTK_M4U_MAGICNO, 19, int)
#define MTK_M4U_T_M4UDrv_DECONSTRUCT  _IOW(MTK_M4U_MAGICNO, 20, int)
#define MTK_M4U_T_DUMP_PAGETABLE      _IOW(MTK_M4U_MAGICNO, 21, int)
#define MTK_M4U_T_REGISTER_BUFFER     _IOW(MTK_M4U_MAGICNO, 22, int)
#define MTK_M4U_T_CACHE_FLUSH_ALL     _IOW(MTK_M4U_MAGICNO, 23, int)
#define MTK_M4U_T_REG_SET             _IOW(MTK_M4U_MAGICNO, 24, int)
#define MTK_M4U_T_REG_GET             _IOW(MTK_M4U_MAGICNO, 25, int)


#endif
