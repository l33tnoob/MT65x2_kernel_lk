#ifndef _SENINF_REG_H_
#define _SENINF_REG_H_


// ----------------- seninf_top Bit Field Definitions -------------------

//#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
//#define SENINF_REG(RegBase, RegName) (RegBase->RegName.Raw)

// New macro for read ISP registers.
#define SENINF_READ_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define SENINF_READ_REG(RegBase, RegName)              (RegBase->RegName.Raw)

// New macro for write ISP registers except CAM_CTL_EN1/CAM_CTL_EN2/CAM_DMA_EN/CAM_CTL_EN1_SET/
// CAM_CTL_EN2_SET/CAM_CTL_DMA_EN_SET/CAM_CTL_EN1_CLR/CAM_CTL_EN2_CLR/CAM_CTL_DMA_EN_CLR
// For CAM_CTL_EN1/CAM_CTL_EN2/CAM_DMA_EN/CAM_CTL_EN1_SET/CAM_CTL_EN2_SET/CAM_CTL_DMA_EN_SET/CAM_CTL_EN1_CLR/
// CAM_CTL_EN2_CLR/CAM_CTL_DMA_EN_CLR, use ISP_WRITE_ENABLE_BITS()/ISP_WRITE_ENABLE_REG() instead.


#if 0
#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)              \
    do {                                                                \
        (RegBase->RegName.Bits.FieldName) = (Value);                    \
        dsb();                                                          \
    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
        dsb();                                                          \
    } while (0)
#else
#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)              \
    do {                                                                \
        (RegBase->RegName.Bits.FieldName) = (Value);                    \
    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)
#endif


#define mt65xx_reg_writel(v, a) \
        do {    \
            *(volatile unsigned int *)(a) = (v);    \
        } while (0)


//#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_HW     0x15000000
#define SENINF_BASE_RANGE  0x10000 
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6582_SENINF_TOP_CODA.xml*/
typedef volatile union _REG_SENINF_TOP_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD SENINF1_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_EN           : 1;
        FIELD SENINF1_PCLK_EN           : 1;
        FIELD rsv_12                    : 4;
        FIELD SENINF_TOP_N3D_SW_RST     : 1;
        FIELD rsv_17                    : 14;
        FIELD SENINF_TOP_DBG_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CTRL;

/* end MT6582_SENINF_TOP_CODA.xml*/

/* start MT6582_SENINF_CODA.xml*/
typedef volatile union _REG_SENINF1_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CTRL;

typedef volatile union _REG_SENINF1_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_INTEN;

typedef volatile union _REG_SENINF1_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_INTSTA;

typedef volatile union _REG_SENINF1_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_SIZE;

typedef volatile union _REG_SENINF1_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_1;

typedef volatile union _REG_SENINF1_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_2;

typedef volatile union _REG_SENINF1_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_3;

typedef volatile union _REG_SENINF1_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_4;

typedef volatile union _REG_SENINF1_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_5;

typedef volatile union _REG_SENINF1_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_6;

typedef volatile union _REG_SENINF1_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_7;

typedef volatile union _REG_SENINF1_SPARE_
{
    volatile struct
    {
        FIELD SENINF_FORMAT             : 4;
        FIELD SENINF_EN                 : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_SPARE;

typedef volatile union _REG_SENINF1_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 12;
        FIELD rsv_12                    : 4;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DATA;

/* end MT6582_SENINF_CODA.xml*/

/* start MT6582_SENINF_CSI2_CODA.xml*/
typedef volatile union _REG_SENINF1_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CTRL;

typedef volatile union _REG_SENINF1_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DELAY;

typedef volatile union _REG_SENINF1_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTEN;

typedef volatile union _REG_SENINF1_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTSTA;

typedef volatile union _REG_SENINF1_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_ECCDBG;

typedef volatile union _REG_SENINF1_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CRCDBG;

typedef volatile union _REG_SENINF1_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DBG;

typedef volatile union _REG_SENINF1_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VER;

typedef volatile union _REG_SENINF1_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF1_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNFSM;

typedef volatile union _REG_SENINF1_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNMUX;

typedef volatile union _REG_SENINF1_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF1_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CAL;

typedef volatile union _REG_SENINF1_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DS;

typedef volatile union _REG_SENINF1_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VS;

typedef volatile union _REG_SENINF1_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_BIST;

/* end MT6582_SENINF_CSI2_CODA.xml*/

/* start MT6582_SENINF_SCAM_CODA.xml*/
typedef volatile union _REG_SCAM1_CFG_
{
    volatile struct
    {
        FIELD INTEN0                    : 1;
        FIELD INTEN1                    : 1;
        FIELD INTEN2                    : 1;
        FIELD INTEN3                    : 1;
        FIELD INTEN4                    : 1;
        FIELD INTEN5                    : 1;
        FIELD INTEN6                    : 1;
        FIELD rsv_7                     : 1;
        FIELD Cycle                     : 3;
        FIELD rsv_11                    : 1;
        FIELD Clock_inverse             : 1;
        FIELD rsv_13                    : 4;
        FIELD Continuous_mode           : 1;
        FIELD rsv_18                    : 2;
        FIELD Debug_mode                : 1;
        FIELD rsv_21                    : 3;
        FIELD CSD_NUM                   : 2;
        FIELD rsv_26                    : 2;
        FIELD Warning_mask              : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CFG;

typedef volatile union _REG_SCAM1_CON_
{
    volatile struct
    {
        FIELD Enable                    : 1;
        FIELD rsv_1                     : 15;
        FIELD Reset                     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CON;

typedef volatile union _REG_SCAM1_INT_
{
    volatile struct
    {
        FIELD INT0                      : 1;
        FIELD INT1                      : 1;
        FIELD INT2                      : 1;
        FIELD INT3                      : 1;
        FIELD INT4                      : 1;
        FIELD INT5                      : 1;
        FIELD INT6                      : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INT;

typedef volatile union _REG_SCAM1_SIZE_
{
    volatile struct
    {
        FIELD WIDTH                     : 12;
        FIELD rsv_12                    : 4;
        FIELD HEIGHT                    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_SIZE;

typedef volatile union _REG_SCAM1_CFG2_
{
    volatile struct
    {
        FIELD DIS_GATED_CLK             : 1;
        FIELD Reserved                  : 31;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CFG2;

typedef volatile union _REG_SCAM1_INFO0_
{
    volatile struct
    {
        FIELD LINE_ID                   : 16;
        FIELD PACKET_SIZE               : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO0;

typedef volatile union _REG_SCAM1_INFO1_
{
    volatile struct
    {
        FIELD Reserved                  : 8;
        FIELD DATA_ID                   : 6;
        FIELD CRC_ON                    : 1;
        FIELD ACTIVE                    : 1;
        FIELD DATA_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO1;

typedef volatile union _REG_SCAM1_STA_
{
    volatile struct
    {
        FIELD FEND_CNT                  : 4;
        FIELD W_CRC_CNT                 : 4;
        FIELD W_SYNC_CNT                : 4;
        FIELD W_PID_CNT                 : 4;
        FIELD W_LID_CNT                 : 4;
        FIELD W_DID_CNT                 : 4;
        FIELD W_SIZE_CNT                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_STA;

/* end MT6582_SENINF_SCAM_CODA.xml*/

/* start MT6582_SENINF_TG_CODA.xml*/
typedef volatile union _SENINF_REG_TG1_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_PH_CNT;

typedef volatile union _SENINF_REG_TG1_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_SEN_CK;

typedef volatile union _SENINF_REG_TG1_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CTL;

typedef volatile union _SENINF_REG_TG1_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_SIZE;

typedef volatile union _SENINF_REG_TG1_TM_CLK_
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CLK;

/* end MT6582_SENINF_TG_CODA.xml*/

/* start MT6582_SENINF_CCIR656_CODA.xml*/
typedef volatile union _REG_CCIR656_CTL_
{
    volatile struct
    {
        FIELD CCIR656_REV_0             : 1;
        FIELD CCIR656_REV_1             : 1;
        FIELD CCIR656_HS_POL            : 1;
        FIELD CCIR656_VS_POL            : 1;
        FIELD CCIR656_PT_EN             : 1;
        FIELD CCIR656_EN                : 1;
        FIELD rsv_6                     : 2;
        FIELD CCIR656_DBG_SEL           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_CTL;

typedef volatile union _REG_CCIR656_H_
{
    volatile struct
    {
        FIELD CCIR656_HS_START          : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_HS_END            : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_H;

typedef volatile union _REG_CCIR656_PTGEN_H_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_HTOTAL         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HACTIVE        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_1;

typedef volatile union _REG_CCIR656_PTGEN_H_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_HWIDTH         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HSTART         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_2;

typedef volatile union _REG_CCIR656_PTGEN_V_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_VTOTAL         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VACTIVE        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_1;

typedef volatile union _REG_CCIR656_PTGEN_V_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_VWIDTH         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VSTART         : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_2;

typedef volatile union _REG_CCIR656_PTGEN_CTL1_
{
    volatile struct
    {
        FIELD CCIR656_PT_TYPE           : 8;
        FIELD rsv_8                     : 8;
        FIELD CCIR656_PT_COLOR_BAR_TH   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL1;

typedef volatile union _REG_CCIR656_PTGEN_CTL2_
{
    volatile struct
    {
        FIELD CCIR656_PT_Y              : 8;
        FIELD CCIR656_PT_CB             : 8;
        FIELD CCIR656_PT_CR             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL2;

typedef volatile union _REG_CCIR656_PTGEN_CTL3_
{
    volatile struct
    {
        FIELD CCIR656_PT_BD_Y           : 8;
        FIELD CCIR656_PT_BD_CB          : 8;
        FIELD CCIR656_PT_BD_CR          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL3;

typedef volatile union _REG_CCIR656_STATUS_
{
    volatile struct
    {
        FIELD CCIR656_IN_FIELD          : 1;
        FIELD CCIR656_IN_VS             : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_STATUS;

/* end MT6582_SENINF_CCIR656_CODA.xml*/

/* start MT6582_ncsi2.xml*/
typedef volatile union _REG_SENINF1_NCSI2_CTL_
{
    volatile struct
    {
        FIELD DATA_LANE0_EN             : 1;
        FIELD DATA_LANE1_EN             : 1;
        FIELD DATA_LANE2_EN             : 1;
        FIELD DATA_LANE3_EN             : 1;
        FIELD CLOCK_LANE_EN             : 1;
        FIELD ECC_EN                    : 1;
        FIELD CRC_EN                    : 1;
        FIELD DPCM_EN                   : 1;
        FIELD HSRX_DET_EN               : 1;
        FIELD HS_PRPR_EN                : 1;
        FIELD DT_INTERLEAVING           : 1;
        FIELD VC_INTERLEAVING           : 1;
        FIELD GENERIC_LONG_PACKET_EN    : 1;
        FIELD IMAGE_PACKET_EN           : 1;
        FIELD BYTE2PIXEL_EN             : 1;
        FIELD VS_TYPE                   : 1;
        FIELD ED_SEL                    : 1;
        FIELD rsv_17                    : 1;
        FIELD FLUSH_MODE                : 2;
        FIELD SYNC_DET_SCHEME           : 1;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_CTL;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DPCM;

typedef volatile union _REG_SENINF1_NCSI2_VC_
{
    volatile struct
    {
        FIELD VC                        : 2;
        FIELD DT                        : 6;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_VC;

typedef volatile union _REG_SENINF1_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_EN;

typedef volatile union _REG_SENINF1_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF1_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF1_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 6;
        FIELD rsv_6                     : 2;
        FIELD LNRD1_RX_FSM              : 6;
        FIELD rsv_14                    : 2;
        FIELD LNRD2_RX_FSM              : 6;
        FIELD rsv_22                    : 2;
        FIELD LNRD3_RX_FSM              : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF1_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF1_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF1_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE0;

typedef volatile union _REG_SENINF1_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE1;

/* end MT6582_ncsi2.xml*/

// ----------------- seninf_top  Grouping Definitions -------------------
// ----------------- seninf_top Register Definition -------------------
typedef volatile struct _seninf_reg_t_
{
    UINT32                          rsv_0000[8192];           // 0000..7FFC
    REG_SENINF_TOP_CTRL             SENINF_TOP_CTRL;          // 8000 (start MT6582_SENINF_TOP_CODA.xml)
    UINT32                          rsv_8004[3];              // 8004..800C
    REG_SENINF1_CTRL                SENINF1_CTRL;             // 8010 (start MT6582_SENINF_CODA.xml)
    REG_SENINF1_INTEN               SENINF1_INTEN;            // 8014
    REG_SENINF1_INTSTA              SENINF1_INTSTA;           // 8018
    REG_SENINF1_SIZE                SENINF1_SIZE;             // 801C
    REG_SENINF1_DEBUG_1             SENINF1_DEBUG_1;          // 8020
    REG_SENINF1_DEBUG_2             SENINF1_DEBUG_2;          // 8024
    REG_SENINF1_DEBUG_3             SENINF1_DEBUG_3;          // 8028
    REG_SENINF1_DEBUG_4             SENINF1_DEBUG_4;          // 802C
    REG_SENINF1_DEBUG_5             SENINF1_DEBUG_5;          // 8030
    REG_SENINF1_DEBUG_6             SENINF1_DEBUG_6;          // 8034
    REG_SENINF1_DEBUG_7             SENINF1_DEBUG_7;          // 8038
    REG_SENINF1_SPARE               SENINF1_SPARE;            // 803C
    REG_SENINF1_DATA                SENINF1_DATA;             // 8040
    UINT32                          rsv_8044[47];             // 8044..80FC
    REG_SENINF1_CSI2_CTRL           SENINF1_CSI2_CTRL;        // 8100 (start MT6582_SENINF_CSI2_CODA.xml)
    REG_SENINF1_CSI2_DELAY          SENINF1_CSI2_DELAY;       // 8104
    REG_SENINF1_CSI2_INTEN          SENINF1_CSI2_INTEN;       // 8108
    REG_SENINF1_CSI2_INTSTA         SENINF1_CSI2_INTSTA;      // 810C
    REG_SENINF1_CSI2_ECCDBG         SENINF1_CSI2_ECCDBG;      // 8110
    REG_SENINF1_CSI2_CRCDBG         SENINF1_CSI2_CRCDBG;      // 8114
    REG_SENINF1_CSI2_DBG            SENINF1_CSI2_DBG;         // 8118
    REG_SENINF1_CSI2_VER            SENINF1_CSI2_VER;         // 811C
    REG_SENINF1_CSI2_SHORT_INFO     SENINF1_CSI2_SHORT_INFO;  // 8120
    REG_SENINF1_CSI2_LNFSM          SENINF1_CSI2_LNFSM;       // 8124
    REG_SENINF1_CSI2_LNMUX          SENINF1_CSI2_LNMUX;       // 8128
    REG_SENINF1_CSI2_HSYNC_CNT      SENINF1_CSI2_HSYNC_CNT;   // 812C
    REG_SENINF1_CSI2_CAL            SENINF1_CSI2_CAL;         // 8130
    REG_SENINF1_CSI2_DS             SENINF1_CSI2_DS;          // 8134
    REG_SENINF1_CSI2_VS             SENINF1_CSI2_VS;          // 8138
    REG_SENINF1_CSI2_BIST           SENINF1_CSI2_BIST;        // 813C
    UINT32                          rsv_8140[48];             // 8140..81FC
    REG_SCAM1_CFG                   SCAM1_CFG;                // 8200 (start MT6582_SENINF_SCAM_CODA.xml)
    REG_SCAM1_CON                   SCAM1_CON;                // 8204
    UINT32                          rsv_8208;                 // 8208
    REG_SCAM1_INT                   SCAM1_INT;                // 820C
    REG_SCAM1_SIZE                  SCAM1_SIZE;               // 8210
    UINT32                          rsv_8214[3];              // 8214..821C
    REG_SCAM1_CFG2                  SCAM1_CFG2;               // 8220
    UINT32                          rsv_8224[3];              // 8224..822C
    REG_SCAM1_INFO0                 SCAM1_INFO0;              // 8230
    REG_SCAM1_INFO1                 SCAM1_INFO1;              // 8234
    UINT32                          rsv_8238[2];              // 8238..823C
    REG_SCAM1_STA                   SCAM1_STA;                // 8240
    UINT32                          rsv_8244[47];             // 8244..82FC
    SENINF_REG_TG1_PH_CNT           SENINF_TG1_PH_CNT;        // 8300 (start MT6582_SENINF_TG_CODA.xml)
    SENINF_REG_TG1_SEN_CK           SENINF_TG1_SEN_CK;        // 8304
    SENINF_REG_TG1_TM_CTL           SENINF_TG1_TM_CTL;        // 8308
    SENINF_REG_TG1_TM_SIZE          SENINF_TG1_TM_SIZE;       // 830C
    SENINF_REG_TG1_TM_CLK           SENINF_TG1_TM_CLK;        // 8310
    UINT32                          rsv_8314[59];             // 8314..83FC
    REG_CCIR656_CTL                 CCIR656_CTL;              // 8400 (start MT6582_SENINF_CCIR656_CODA.xml)
    REG_CCIR656_H                   CCIR656_H;                // 8404
    REG_CCIR656_PTGEN_H_1           CCIR656_PTGEN_H_1;        // 8408
    REG_CCIR656_PTGEN_H_2           CCIR656_PTGEN_H_2;        // 840C
    REG_CCIR656_PTGEN_V_1           CCIR656_PTGEN_V_1;        // 8410
    REG_CCIR656_PTGEN_V_2           CCIR656_PTGEN_V_2;        // 8414
    REG_CCIR656_PTGEN_CTL1          CCIR656_PTGEN_CTL1;       // 8418
    REG_CCIR656_PTGEN_CTL2          CCIR656_PTGEN_CTL2;       // 841C
    REG_CCIR656_PTGEN_CTL3          CCIR656_PTGEN_CTL3;       // 8420
    REG_CCIR656_STATUS              CCIR656_STATUS;           // 8424
    UINT32                          rsv_8428[118];            // 8428..85FC
    REG_SENINF1_NCSI2_CTL           SENINF1_NCSI2_CTL;        // 8600
    REG_SENINF1_NCSI2_LNRC_TIMING   SENINF1_NCSI2_LNRC_TIMING; // 8604
    REG_SENINF1_NCSI2_LNRD_TIMING   SENINF1_NCSI2_LNRD_TIMING; // 8608
    REG_SENINF1_NCSI2_DPCM          SENINF1_NCSI2_DPCM;       // 860C
    REG_SENINF1_NCSI2_VC            SENINF1_NCSI2_VC;         // 8610
    REG_SENINF1_NCSI2_INT_EN        SENINF1_NCSI2_INT_EN;     // 8614
    REG_SENINF1_NCSI2_INT_STATUS    SENINF1_NCSI2_INT_STATUS; // 8618
    REG_SENINF1_NCSI2_DGB_SEL       SENINF1_NCSI2_DGB_SEL;    // 861C
    REG_SENINF1_NCSI2_DBG_PORT      SENINF1_NCSI2_DBG_PORT;   // 8620
    REG_SENINF1_NCSI2_LNRC_FSM      SENINF1_NCSI2_LNRC_FSM;   // 8624
    REG_SENINF1_NCSI2_LNRD_FSM      SENINF1_NCSI2_LNRD_FSM;   // 8628
    REG_SENINF1_NCSI2_FRAME_LINE_NUM SENINF1_NCSI2_FRAME_LINE_NUM; // 862C
    REG_SENINF1_NCSI2_GENERIC_SHORT SENINF1_NCSI2_GENERIC_SHORT; // 8630
    UINT32                          rsv_8634[3];              // 8634..863C
    REG_SENINF1_NCSI2_SPARE0        SENINF1_NCSI2_SPARE0;     // 8640
    REG_SENINF1_NCSI2_SPARE1        SENINF1_NCSI2_SPARE1;     // 8644
}seninf_reg_t;

#endif // _SENINF_REG_H_
