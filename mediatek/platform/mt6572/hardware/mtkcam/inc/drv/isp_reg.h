#ifndef __isp_reg_h__
#define __isp_reg_h__

// ----------------- cam Bit Field Definitions -------------------

#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)
#define ISP_BASE_HW     0x14013000
#define ISP_BASE_RANGE  0x1000 
//#define PACKING volatile
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6572_cam_csr.xml*/
typedef volatile union
{
    volatile struct
    {
        FIELD TG1_EN                    : 1;
        FIELD CDRZ_EN                   : 1;
        FIELD IMGO_EN                   : 1;
        FIELD DB_EN                     : 1;
        FIELD DB_LOCK                   : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_MODULE_EN;

typedef volatile union
{
    volatile struct
    {
        FIELD TG1_FMT                   : 3;
        FIELD rsv_3                     : 1;
        FIELD TG1_SW                    : 2;
        FIELD rsv_6                     : 10;
        FIELD IMGO_FORMAT               : 2;
        FIELD rsv_18                    : 6;
        FIELD IMGO_BUS_SIZE             : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_FMT_SEL;

typedef volatile union
{
    volatile struct
    {
        FIELD VS1_INT_EN                : 1;
        FIELD TG1_INT1_EN               : 1;
        FIELD TG1_INT2_EN               : 1;
        FIELD EXPDON1_INT_EN            : 1;
        FIELD TG1_ERR_INT_EN            : 1;
        FIELD TG1_GBERR_INT_EN          : 1;
        FIELD TG1_DROP_INT_EN           : 1;
        FIELD TG1_SOF_INT_EN            : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_DON_INT_EN          : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_INT_EN           : 1;
        FIELD IMGO_OVERR_INT_EN         : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_INT_EN          : 1;
        FIELD rsv_20                    : 11;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_INT_EN;

typedef volatile union
{
    volatile struct
    {
        FIELD VS1_ST                    : 1;
        FIELD TG1_ST1                   : 1;
        FIELD TG1_ST2                   : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD TG1_GBERR_ST              : 1;
        FIELD TG1_DROP_INT_ST           : 1;
        FIELD TG1_SOF1_INT_ST           : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_TG1_DON_ST          : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_ST               : 1;
        FIELD IMGO_OVERR_ST             : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_ST              : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_INT_STATUS;

typedef volatile union
{
    volatile struct
    {
        FIELD IMGO_RST_TRIG             : 1;
        FIELD IMGO_RST_ST               : 1;
        FIELD SW_RST                    : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_SW_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_SPARE0;

typedef volatile union
{
    volatile struct
    {
        FIELD CTL_SPARE1                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_SPARE1;

typedef volatile union
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_IMGO_FBC;

typedef volatile union
{
    volatile struct
    {
        FIELD RAW_DP_CK_EN              : 1;
        FIELD rsv_1                     : 1;
        FIELD DIP_DP_CK_EN              : 1;
        FIELD rsv_3                     : 12;
        FIELD DMA_DP_CK_EN              : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_CLK_EN;

typedef volatile union
{
    volatile struct
    {
        FIELD DEBUG_MOD_SEL             : 4;
        FIELD rsv_4                     : 4;
        FIELD DEBUG_SEL                 : 12;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_DBG_SET;

typedef volatile union
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_DBG_PORT;

typedef volatile union
{
    volatile struct
    {
        FIELD CTL_DATE_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_DATE_CODE;

typedef volatile union
{
    volatile struct
    {
        FIELD CTL_PROJ_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_PROJ_CODE;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_DCM_DIS              : 1;
        FIELD IMGO_DCM_DIS              : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_DCM_DIS;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_DCM_ST               : 1;
        FIELD IMGO_DCM_ST               : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_CAM_DCM_STATUS;

/* end MT6572_CAM_csr.xml*/

/* start MT6572_100_dma.xml*/
typedef volatile union
{
    volatile struct
    {
        FIELD rsv_0                     : 16;
        FIELD IMGO_SOFT_RST_STAT        : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_SOFT_RSTSTAT;

typedef volatile union
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_BASE_ADDR;

typedef volatile union
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_OFST_ADDR;

typedef volatile union
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_XSIZE;

typedef volatile union
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_YSIZE;

typedef volatile union
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_STRIDE;

typedef volatile union
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 2;
        FIELD LAST_ULTRA_EN             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON;

typedef volatile union
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON2;

typedef volatile union
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CROP;

typedef volatile union
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD IMGO_ERR                  : 1;
        FIELD rsv_10                    : 21;
        FIELD ERR_CLR_MD                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_ERR_CTRL;

typedef volatile union
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_ERR_STAT;

typedef volatile union
{
    volatile struct
    {
        FIELD DEBUG_ADDR                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_DEBUG_ADDR;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV1;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV2;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV3;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV4;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV5;

typedef volatile union
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV6;

/* end MT6572_100_dma.xml*/

/* start MT6572_201_raw_tg.xml*/
typedef volatile union
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_MODE;

typedef volatile union
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_VF_CON;

typedef volatile union
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_PXL;

typedef volatile union
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_LIN;

typedef volatile union
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_PATH_CFG;

typedef volatile union
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MEMIN_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT1;

typedef volatile union
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT2;

typedef volatile union
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOF_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOT_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_EOT_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_ERR_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_DAT_NO;

typedef volatile union
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRM_CNT_ST;

typedef volatile union
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRMSIZE_ST;

typedef volatile union
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INTER_ST;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHA_EN                 : 1;
        FIELD FLASH_EN                  : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHA_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHA_END_FRM            : 3;
        FIELD rsv_11                    : 1;
        FIELD FLASH_POL                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHA_LUNIT_NO           : 20;
        FIELD FLASHA_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_LINE_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHA_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHA_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_POS;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHB_EN                 : 1;
        FIELD FLASHB_TRIG_SRC           : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHB_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHB_START_FRM          : 4;
        FIELD FLASHB_CONT_FRM           : 3;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHB_LUNIT_NO           : 20;
        FIELD FLASHB_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_LINE_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHB_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHB_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS;

typedef volatile union
{
    volatile struct
    {
        FIELD FLASHB_CYC_CNT            : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS1;

typedef volatile union
{
    volatile struct
    {
        FIELD GSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD GSCTRL_POL                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD GS_EPTIME                 : 23;
        FIELD rsv_23                    : 1;
        FIELD GSMS_TIMEU                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_TIME;

typedef volatile union
{
    volatile struct
    {
        FIELD MSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD MSCTRL_VSPOL              : 1;
        FIELD MSCTRL_OPEN_TRSRC         : 1;
        FIELD rsv_6                     : 2;
        FIELD MSCTRL_TRSRC              : 2;
        FIELD rsv_10                    : 6;
        FIELD MSCP1_PH0                 : 1;
        FIELD MSCP1_PH1                 : 1;
        FIELD MSCP1_PH2                 : 1;
        FIELD rsv_19                    : 1;
        FIELD MSOP1_PH0                 : 1;
        FIELD MSOP1_PH1                 : 1;
        FIELD rsv_22                    : 1;
        FIELD MSP1_RST                  : 1;
        FIELD MSCP2_PH0                 : 1;
        FIELD MSCP2_PH1                 : 1;
        FIELD MSCP2_PH2                 : 1;
        FIELD rsv_27                    : 1;
        FIELD MSOP2_PH0                 : 1;
        FIELD MSOP2_PH1                 : 1;
        FIELD rsv_30                    : 1;
        FIELD MSP2_RST                  : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_PHASE;

typedef volatile union
{
    volatile struct
    {
        FIELD MS_TCLOSE                 : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CL_TIME;

typedef volatile union
{
    volatile struct
    {
        FIELD MS_TOPEN                  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OP_TIME;

typedef volatile union
{
    volatile struct
    {
        FIELD MS_CL_T1                  : 16;
        FIELD MS_CL_T2                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CLPH_TIME;

typedef volatile union
{
    volatile struct
    {
        FIELD MS_OP_T3                  : 16;
        FIELD MS_OP_T4                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OPPH_TIME;
#if 0
typedef volatile union
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_MODE;

typedef volatile union
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_VF_CON;

typedef volatile union
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_PXL;

typedef volatile union
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_LIN;

typedef volatile union
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_PATH_CFG;

typedef volatile union
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_MEMIN_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT1;

typedef volatile union
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT2;

typedef volatile union
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOF_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOT_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_EOT_CNT;

typedef volatile union
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_ERR_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_DAT_NO;

typedef volatile union
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRM_CNT_ST;

typedef volatile union
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRMSIZE_ST;

typedef volatile union
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INTER_ST;
#endif
/* end MT6572_201_raw_tg.xml*/

/* start MT6572_500_cdp_cdrz.xml*/
typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_HORIZONTAL_EN        : 1;
        FIELD CDRZ_Vertical_EN          : 1;
        FIELD rsv_2                     : 2;
        FIELD CDRZ_Vertical_First       : 1;
        FIELD CDRZ_Horizontal_Algorithm : 2;
        FIELD CDRZ_Vertical_Algorithm   : 2;
        FIELD CDRZ_Dering_en            : 1;
        FIELD CDRZ_Truncation_Bit_H     : 3;
        FIELD CDRZ_Truncation_Bit_V     : 3;
        FIELD CDRZ_Horizontal_Table_Select : 5;
        FIELD CDRZ_Vertical_Table_Select : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CONTROL;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Input_Image_W        : 13;
        FIELD rsv_13                    : 3;
        FIELD CDRZ_Input_Image_H        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_INPUT_IMAGE;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Output_Image_W       : 13;
        FIELD rsv_13                    : 3;
        FIELD CDRZ_Output_Image_H       : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_OUTPUT_IMAGE;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Horizontal_Coeff_Step : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_HORIZONTAL_COEFF_STEP;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Vertical_Coeff_Step  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_VERTICAL_COEFF_STEP;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Luma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Luma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Luma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Luma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Horizontal_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Horizontal_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Vertical_Integer_Offset : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Chroma_Vertical_Subpixel_Offset : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Dering_Threshold_1V  : 4;
        FIELD rsv_4                     : 12;
        FIELD CDRZ_Dering_Threshold_1H  : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_DERING_1;

typedef volatile union
{
    volatile struct
    {
        FIELD CDRZ_Dering_Threshold_2V  : 9;
        FIELD rsv_9                     : 7;
        FIELD CDRZ_Dering_Threshold_2H  : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CDRZ_DERING_2;

/* end MT6572_500_cdp_cdrz.xml*/

// ----------------- cam  Grouping Definitions -------------------
// ----------------- cam Register Definition -------------------
typedef volatile struct _isp_reg_
{
    //UINT32                          rsv_0000[3072];           // 0000..2FFC
    //ISP_BASE_HW is 0x14013000
    CAM_REG_CAM_MODULE_EN           CAM_CTL_MODULE_EN;        // 3000 (start MT6572_CAM_csr.xml)
    CAM_REG_CAM_FMT_SEL             CAM_CTL_FMT_SEL;          // 3004
    UINT32                          rsv_3008[2];              // 3008..300C
    CAM_REG_CAM_INT_EN              CAM_CTL_INT_EN;           // 3010
    CAM_REG_CAM_INT_STATUS          CAM_CTL_INT_STATUS;       // 3014
    UINT32                          rsv_3018[2];              // 3018..301C
    CAM_REG_CAM_SW_CTL              CAM_CTL_SW_CTL;           // 3020
    UINT32                          rsv_3024[3];              // 3024..302C
    CAM_REG_CAM_SPARE0              CAM_CTL_SPARE0;           // 3030
    CAM_REG_CAM_SPARE1              CAM_CTL_SPARE1;           // 3034
    UINT32                          rsv_3038[2];              // 3038..303C
    CAM_REG_CAM_IMGO_FBC            CAM_CTL_IMGO_FBC;         // 3040
    UINT32                          rsv_3044[3];              // 3044..304C
    CAM_REG_CAM_CLK_EN              CAM_CTL_CLK_EN;           // 3050
    UINT32                          rsv_3054[3];              // 3054..305C
    CAM_REG_CAM_DBG_SET             CAM_CTL_DBG_SET;          // 3060
    CAM_REG_CAM_DBG_PORT            CAM_CTL_DBG_PORT;         // 3064
    UINT32                          rsv_3068[2];              // 3068..306C
    CAM_REG_CAM_DATE_CODE           CAM_CTL_DATE_CODE;        // 3070
    CAM_REG_CAM_PROJ_CODE           CAM_CTL_PROJ_CODE;        // 3074
    UINT32                          rsv_3078[2];              // 3078..307C
    CAM_REG_CAM_DCM_DIS             CAM_CTL_DCM_DIS;          // 3080
    CAM_REG_CAM_DCM_STATUS          CAM_CTL_DCM_STATUS;       // 3084
    UINT32                          rsv_3088[94];             // 3088..31FC
    CAM_REG_DMA_SOFT_RSTSTAT        CAM_DMA_SOFT_RSTSTAT;     // 3200 (start MT6572_100_dma.xml)
    CAM_REG_IMGO_BASE_ADDR          CAM_IMGO_BASE_ADDR;       // 3204
    CAM_REG_IMGO_OFST_ADDR          CAM_IMGO_OFST_ADDR;       // 3208
    CAM_REG_IMGO_XSIZE              CAM_IMGO_XSIZE;           // 320C
    CAM_REG_IMGO_YSIZE              CAM_IMGO_YSIZE;           // 3210
    CAM_REG_IMGO_STRIDE             CAM_IMGO_STRIDE;          // 3214
    CAM_REG_IMGO_CON                CAM_IMGO_CON;             // 3218
    CAM_REG_IMGO_CON2               CAM_IMGO_CON2;            // 321C
    CAM_REG_IMGO_CROP               CAM_IMGO_CROP;            // 3220
    CAM_REG_DMA_ERR_CTRL            CAM_DMA_ERR_CTRL;         // 3224
    CAM_REG_IMGO_ERR_STAT           CAM_IMGO_ERR_STAT;        // 3228
    CAM_REG_DMA_DEBUG_ADDR          CAM_DMA_DEBUG_ADDR;       // 322C
    CAM_REG_DMA_RSV1                CAM_DMA_RSV1;             // 3230
    CAM_REG_DMA_RSV2                CAM_DMA_RSV2;             // 3234
    CAM_REG_DMA_RSV3                CAM_DMA_RSV3;             // 3238
    CAM_REG_DMA_RSV4                CAM_DMA_RSV4;             // 323C
    CAM_REG_DMA_RSV5                CAM_DMA_RSV5;             // 3240
    CAM_REG_DMA_RSV6                CAM_DMA_RSV6;             // 3244
    UINT32                          rsv_3248[114];            // 3248..340C
    CAM_REG_TG_SEN_MODE             CAM_TG_SEN_MODE;          // 3410 (start MT6572_201_raw_tg.xml)
    CAM_REG_TG_VF_CON               CAM_TG_VF_CON;            // 3414
    CAM_REG_TG_SEN_GRAB_PXL         CAM_TG_SEN_GRAB_PXL;      // 3418
    CAM_REG_TG_SEN_GRAB_LIN         CAM_TG_SEN_GRAB_LIN;      // 341C
    CAM_REG_TG_PATH_CFG             CAM_TG_PATH_CFG;          // 3420
    CAM_REG_TG_MEMIN_CTL            CAM_TG_MEMIN_CTL;         // 3424
    CAM_REG_TG_INT1                 CAM_TG_INT1;              // 3428
    CAM_REG_TG_INT2                 CAM_TG_INT2;              // 342C
    CAM_REG_TG_SOF_CNT              CAM_TG_SOF_CNT;           // 3430
    CAM_REG_TG_SOT_CNT              CAM_TG_SOT_CNT;           // 3434
    CAM_REG_TG_EOT_CNT              CAM_TG_EOT_CNT;           // 3438
    CAM_REG_TG_ERR_CTL              CAM_TG_ERR_CTL;           // 343C
    CAM_REG_TG_DAT_NO               CAM_TG_DAT_NO;            // 3440
    CAM_REG_TG_FRM_CNT_ST           CAM_TG_FRM_CNT_ST;        // 3444
    CAM_REG_TG_FRMSIZE_ST           CAM_TG_FRMSIZE_ST;        // 3448
    CAM_REG_TG_INTER_ST             CAM_TG_INTER_ST;          // 344C
    UINT32                          rsv_3450[4];              // 3450..345C
    CAM_REG_TG_FLASHA_CTL           CAM_TG_FLASHA_CTL;        // 3460
    CAM_REG_TG_FLASHA_LINE_CNT      CAM_TG_FLASHA_LINE_CNT;   // 3464
    CAM_REG_TG_FLASHA_POS           CAM_TG_FLASHA_POS;        // 3468
    CAM_REG_TG_FLASHB_CTL           CAM_TG_FLASHB_CTL;        // 346C
    CAM_REG_TG_FLASHB_LINE_CNT      CAM_TG_FLASHB_LINE_CNT;   // 3470
    CAM_REG_TG_FLASHB_POS           CAM_TG_FLASHB_POS;        // 3474
    CAM_REG_TG_FLASHB_POS1          CAM_TG_FLASHB_POS1;       // 3478
    CAM_REG_TG_GSCTRL_CTL           CAM_TG_GSCTRL_CTL;        // 347C
    CAM_REG_TG_GSCTRL_TIME          CAM_TG_GSCTRL_TIME;       // 3480
    CAM_REG_TG_MS_PHASE             CAM_TG_MS_PHASE;          // 3484
    CAM_REG_TG_MS_CL_TIME           CAM_TG_MS_CL_TIME;        // 3488
    CAM_REG_TG_MS_OP_TIME           CAM_TG_MS_OP_TIME;        // 348C
    CAM_REG_TG_MS_CLPH_TIME         CAM_TG_MS_CLPH_TIME;      // 3490
    CAM_REG_TG_MS_OPPH_TIME         CAM_TG_MS_OPPH_TIME;      // 3494
    UINT32                          rsv_3498[6];              // 3498..34AC
#if 0 
    CAM_REG_TG2_SEN_MODE            CAM_TG2_SEN_MODE;         // 34B0
    CAM_REG_TG2_VF_CON              CAM_TG2_VF_CON;           // 34B4
    CAM_REG_TG2_SEN_GRAB_PXL        CAM_TG2_SEN_GRAB_PXL;     // 34B8
    CAM_REG_TG2_SEN_GRAB_LIN        CAM_TG2_SEN_GRAB_LIN;     // 34BC
    CAM_REG_TG2_PATH_CFG            CAM_TG2_PATH_CFG;         // 34C0
    CAM_REG_TG2_MEMIN_CTL           CAM_TG2_MEMIN_CTL;        // 34C4
    CAM_REG_TG2_INT1                CAM_TG2_INT1;             // 34C8
    CAM_REG_TG2_INT2                CAM_TG2_INT2;             // 34CC
    CAM_REG_TG2_SOF_CNT             CAM_TG2_SOF_CNT;          // 34D0
    CAM_REG_TG2_SOT_CNT             CAM_TG2_SOT_CNT;          // 34D4
    CAM_REG_TG2_EOT_CNT             CAM_TG2_EOT_CNT;          // 34D8
    CAM_REG_TG2_ERR_CTL             CAM_TG2_ERR_CTL;          // 34DC
    CAM_REG_TG2_DAT_NO              CAM_TG2_DAT_NO;           // 34E0
    CAM_REG_TG2_FRM_CNT_ST          CAM_TG2_FRM_CNT_ST;       // 34E4
    CAM_REG_TG2_FRMSIZE_ST          CAM_TG2_FRMSIZE_ST;       // 34E8
    CAM_REG_TG2_INTER_ST            CAM_TG2_INTER_ST;         // 34EC
#else
    UINT32                          rsv_34B0[16];             // 34B0..34EC
#endif
    UINT32                          rsv_34F0[388];            // 34F0..3AFC
    CAM_REG_CDRZ_CONTROL            CAM_CDRZ_CONTROL;         // 3B00 (start MT6572_500_cdp_cdrz.xml)
    CAM_REG_CDRZ_INPUT_IMAGE        CAM_CDRZ_INPUT_IMAGE;     // 3B04
    CAM_REG_CDRZ_OUTPUT_IMAGE       CAM_CDRZ_OUTPUT_IMAGE;    // 3B08
    CAM_REG_CDRZ_HORIZONTAL_COEFF_STEP CAM_CDRZ_HORIZONTAL_COEFF_STEP; // 3B0C
    CAM_REG_CDRZ_VERTICAL_COEFF_STEP CAM_CDRZ_VERTICAL_COEFF_STEP; // 3B10
    CAM_REG_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET; // 3B14
    CAM_REG_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET; // 3B18
    CAM_REG_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET; // 3B1C
    CAM_REG_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET; // 3B20
    CAM_REG_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET; // 3B24
    CAM_REG_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET; // 3B28
    CAM_REG_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET; // 3B2C
    CAM_REG_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET; // 3B30
    CAM_REG_CDRZ_DERING_1           CAM_CDRZ_DERING_1;        // 3B34
    CAM_REG_CDRZ_DERING_2           CAM_CDRZ_DERING_2;        // 3B38
}isp_reg_t;

#undef volatile
#endif // __isp_reg_h__
