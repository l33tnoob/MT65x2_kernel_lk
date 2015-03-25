#ifndef __seninf_reg_h__
#define __seninf_reg_h__

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
#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)          	\
    do {                                                                \
        (RegBase->RegName.Bits.FieldName) = (Value);                    \        
    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                      	\
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)



#define SENINF_BASE_HW     0x14014000
#define SENINF_BASE_RANGE  0x1000 
#define PACKING volatile
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6572_SENINF_TOP_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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

/* end MT6572_SENINF_TOP_CODA.xml*/

/* start MT6572_SENINF_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_4;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_5;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_6;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DEBUG_7;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD SENINF_DATA0              : 12;
        FIELD rsv_12                    : 4;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_DATA;

/* end MT6572_SENINF_CODA.xml*/

/* start MT6572_SENINF_CSI2_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DELAY;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_ECCDBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CRCDBG;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VER;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_SHORT_INFO;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNMUX;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_HSYNC_CNT;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VS;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_BIST;

/* end MT6572_SENINF_CSI2_CODA.xml*/

/* start MT6572_SENINF_SCAM_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD Enable                    : 1;
        FIELD rsv_1                     : 15;
        FIELD Reset                     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CON;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD WIDTH                     : 12;
        FIELD rsv_12                    : 4;
        FIELD HEIGHT                    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DIS_GATED_CLK             : 1;
        FIELD Reserved                  : 31;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_CFG2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LINE_ID                   : 16;
        FIELD PACKET_SIZE               : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD Reserved                  : 8;
        FIELD DATA_ID                   : 6;
        FIELD CRC_ON                    : 1;
        FIELD ACTIVE                    : 1;
        FIELD DATA_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SCAM1_INFO1;

typedef PACKING union
{
    PACKING struct
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

/* end MT6572_SENINF_SCAM_CODA.xml*/

/* start MT6572_SENINF_TG_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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
} REG_TG1_PH_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_TG1_SEN_CK;

typedef PACKING union
{
    PACKING struct
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
} REG_TG1_TM_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_TG1_TM_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_TG1_TM_CLK;
#if 0
typedef PACKING union
{
    PACKING struct
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
} REG_TG2_PH_CNT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_TG2_SEN_CK;

typedef PACKING union
{
    PACKING struct
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
} REG_TG2_TM_CTL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_TG2_TM_SIZE;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_TG2_TM_CLK;
#endif
/* end MT6572_SENINF_TG_CODA.xml*/

/* start MT6572_SENINF_CCIR656_CODA.xml*/
typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_HS_START          : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_HS_END            : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_H;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_HTOTAL         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HACTIVE        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_HWIDTH         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HSTART         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_VTOTAL         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VACTIVE        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_VWIDTH         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VSTART         : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_TYPE           : 8;
        FIELD rsv_8                     : 8;
        FIELD CCIR656_PT_COLOR_BAR_TH   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL1;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_Y              : 8;
        FIELD CCIR656_PT_CB             : 8;
        FIELD CCIR656_PT_CR             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL2;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_PT_BD_Y           : 8;
        FIELD CCIR656_PT_BD_CB          : 8;
        FIELD CCIR656_PT_BD_CR          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL3;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CCIR656_IN_FIELD          : 1;
        FIELD CCIR656_IN_VS             : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_STATUS;

/* end MT6572_SENINF_CCIR656_CODA.xml*/

/* start MT6572_ncsi2.xml*/
typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_TIMING;

typedef PACKING union
{
    PACKING struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_TIMING;

typedef PACKING union
{
    PACKING struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DPCM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD VC                        : 2;
        FIELD DT                        : 6;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_VC;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DGB_SEL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DBG_PORT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_FSM;

typedef PACKING union
{
    PACKING struct
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

typedef PACKING union
{
    PACKING struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_FRAME_LINE_NUM;

typedef PACKING union
{
    PACKING struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_GENERIC_SHORT;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE0;

typedef PACKING union
{
    PACKING struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE1;

/* end MT6572_ncsi2.xml*/

// ----------------- seninf_top  Grouping Definitions -------------------
// ----------------- seninf_top Register Definition -------------------
typedef volatile struct _seninf_reg_
{
    //UINT32                          rsv_0000[4096];           // 0000..3FFC
    // SENINF_BASE_HW is 0x14014000
    REG_SENINF_TOP_CTRL             SENINF_TOP_CTRL;          // 4000 (start MT6572_SENINF_TOP_CODA.xml)
    UINT32                          rsv_4004[3];              // 4004..400C
    REG_SENINF1_CTRL                SENINF1_CTRL;             // 4010 (start MT6572_SENINF_CODA.xml)
    REG_SENINF1_INTEN               SENINF1_INTEN;            // 4014
    REG_SENINF1_INTSTA              SENINF1_INTSTA;           // 4018
    REG_SENINF1_SIZE                SENINF1_SIZE;             // 401C
    REG_SENINF1_DEBUG_1             SENINF1_DEBUG_1;          // 4020
    REG_SENINF1_DEBUG_2             SENINF1_DEBUG_2;          // 4024
    REG_SENINF1_DEBUG_3             SENINF1_DEBUG_3;          // 4028
    REG_SENINF1_DEBUG_4             SENINF1_DEBUG_4;          // 402C
    REG_SENINF1_DEBUG_5             SENINF1_DEBUG_5;          // 4030
    REG_SENINF1_DEBUG_6             SENINF1_DEBUG_6;          // 4034
    REG_SENINF1_DEBUG_7             SENINF1_DEBUG_7;          // 4038
    REG_SENINF1_SPARE               SENINF1_SPARE;            // 403C
    REG_SENINF1_DATA                SENINF1_DATA;             // 4040
    UINT32                          rsv_4044[47];             // 4044..40FC
    REG_SENINF1_CSI2_CTRL           SENINF1_CSI2_CTRL;        // 4100 (start MT6572_SENINF_CSI2_CODA.xml)
    REG_SENINF1_CSI2_DELAY          SENINF1_CSI2_DELAY;       // 4104
    REG_SENINF1_CSI2_INTEN          SENINF1_CSI2_INTEN;       // 4108
    REG_SENINF1_CSI2_INTSTA         SENINF1_CSI2_INTSTA;      // 410C
    REG_SENINF1_CSI2_ECCDBG         SENINF1_CSI2_ECCDBG;      // 4110
    REG_SENINF1_CSI2_CRCDBG         SENINF1_CSI2_CRCDBG;      // 4114
    REG_SENINF1_CSI2_DBG            SENINF1_CSI2_DBG;         // 4118
    REG_SENINF1_CSI2_VER            SENINF1_CSI2_VER;         // 411C
    REG_SENINF1_CSI2_SHORT_INFO     SENINF1_CSI2_SHORT_INFO;  // 4120
    REG_SENINF1_CSI2_LNFSM          SENINF1_CSI2_LNFSM;       // 4124
    REG_SENINF1_CSI2_LNMUX          SENINF1_CSI2_LNMUX;       // 4128
    REG_SENINF1_CSI2_HSYNC_CNT      SENINF1_CSI2_HSYNC_CNT;   // 412C
    REG_SENINF1_CSI2_CAL            SENINF1_CSI2_CAL;         // 4130
    REG_SENINF1_CSI2_DS             SENINF1_CSI2_DS;          // 4134
    REG_SENINF1_CSI2_VS             SENINF1_CSI2_VS;          // 4138
    REG_SENINF1_CSI2_BIST           SENINF1_CSI2_BIST;        // 413C
    UINT32                          rsv_4140[48];             // 4140..41FC
    REG_SCAM1_CFG                   SCAM1_CFG;                // 4200 (start MT6572_SENINF_SCAM_CODA.xml)
    REG_SCAM1_CON                   SCAM1_CON;                // 4204
    UINT32                          rsv_4208;                 // 4208
    REG_SCAM1_INT                   SCAM1_INT;                // 420C
    REG_SCAM1_SIZE                  SCAM1_SIZE;               // 4210
    UINT32                          rsv_4214[3];              // 4214..421C
    REG_SCAM1_CFG2                  SCAM1_CFG2;               // 4220
    UINT32                          rsv_4224[3];              // 4224..422C
    REG_SCAM1_INFO0                 SCAM1_INFO0;              // 4230
    REG_SCAM1_INFO1                 SCAM1_INFO1;              // 4234
    UINT32                          rsv_4238[2];              // 4238..423C
    REG_SCAM1_STA                   SCAM1_STA;                // 4240
    UINT32                          rsv_4244[47];             // 4244..42FC
    REG_TG1_PH_CNT                  SENINF_TG1_PH_CNT;        // 4300 (start MT6572_SENINF_TG_CODA.xml)
    REG_TG1_SEN_CK                  SENINF_TG1_SEN_CK;        // 4304
    REG_TG1_TM_CTL                  SENINF_TG1_TM_CTL;        // 4308
    REG_TG1_TM_SIZE                 SENINF_TG1_TM_SIZE;       // 430C
    REG_TG1_TM_CLK                  SENINF_TG1_TM_CLK;        // 4310
    UINT32                          rsv_4314[35];             // 4314..439C
#if 0 
    REG_TG2_PH_CNT                  TG2_PH_CNT;               // 43A0
    REG_TG2_SEN_CK                  TG2_SEN_CK;               // 43A4
    REG_TG2_TM_CTL                  TG2_TM_CTL;               // 43A8
    REG_TG2_TM_SIZE                 TG2_TM_SIZE;              // 43AC
    REG_TG2_TM_CLK                  TG2_TM_CLK;               // 43B0
#else
    UINT32                          rsv_43A0[5];              // 43A0..43B0
#endif
    UINT32                          rsv_43B4[19];             // 43B4..43FC
    REG_CCIR656_CTL                 CCIR656_CTL;              // 4400 (start MT6572_SENINF_CCIR656_CODA.xml)
    REG_CCIR656_H                   CCIR656_H;                // 4404
    REG_CCIR656_PTGEN_H_1           CCIR656_PTGEN_H_1;        // 4408
    REG_CCIR656_PTGEN_H_2           CCIR656_PTGEN_H_2;        // 440C
    REG_CCIR656_PTGEN_V_1           CCIR656_PTGEN_V_1;        // 4410
    REG_CCIR656_PTGEN_V_2           CCIR656_PTGEN_V_2;        // 4414
    REG_CCIR656_PTGEN_CTL1          CCIR656_PTGEN_CTL1;       // 4418
    REG_CCIR656_PTGEN_CTL2          CCIR656_PTGEN_CTL2;       // 441C
    REG_CCIR656_PTGEN_CTL3          CCIR656_PTGEN_CTL3;       // 4420
    REG_CCIR656_STATUS              CCIR656_STATUS;           // 4424
    UINT32                          rsv_4428[118];            // 4428..45FC
    REG_SENINF1_NCSI2_CTL           SENINF1_NCSI2_CTL;        // 4600
    REG_SENINF1_NCSI2_LNRC_TIMING   SENINF1_NCSI2_LNRC_TIMING; // 4604
    REG_SENINF1_NCSI2_LNRD_TIMING   SENINF1_NCSI2_LNRD_TIMING; // 4608
    REG_SENINF1_NCSI2_DPCM          SENINF1_NCSI2_DPCM;       // 460C
    REG_SENINF1_NCSI2_VC            SENINF1_NCSI2_VC;         // 4610
    REG_SENINF1_NCSI2_INT_EN        SENINF1_NCSI2_INT_EN;     // 4614
    REG_SENINF1_NCSI2_INT_STATUS    SENINF1_NCSI2_INT_STATUS; // 4618
    REG_SENINF1_NCSI2_DGB_SEL       SENINF1_NCSI2_DGB_SEL;    // 461C
    REG_SENINF1_NCSI2_DBG_PORT      SENINF1_NCSI2_DBG_PORT;   // 4620
    REG_SENINF1_NCSI2_LNRC_FSM      SENINF1_NCSI2_LNRC_FSM;   // 4624
    REG_SENINF1_NCSI2_LNRD_FSM      SENINF1_NCSI2_LNRD_FSM;   // 4628
    REG_SENINF1_NCSI2_FRAME_LINE_NUM SENINF1_NCSI2_FRAME_LINE_NUM; // 462C
    REG_SENINF1_NCSI2_GENERIC_SHORT SENINF1_NCSI2_GENERIC_SHORT; // 4630
    UINT32                          rsv_4634[3];              // 4634..463C
    REG_SENINF1_NCSI2_SPARE0        SENINF1_NCSI2_SPARE0;     // 4640
    REG_SENINF1_NCSI2_SPARE1        SENINF1_NCSI2_SPARE1;     // 4644
}seninf_reg_t;

#undef PACKING
#endif // __seninf_reg_h__
