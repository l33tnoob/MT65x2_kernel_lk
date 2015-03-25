#ifndef __PMIC_WRAP_REGS_H__
#define __PMIC_WRAP_REGS_H__
#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>
#include <platform/mt_gpt.h>
#include <platform/mt_irq.h>
#include <sys/types.h>
#include <platform/sync_write.h>
#include <printf.h> 
#define PMIC_WRAP_DEBUG

#define PWRAPTAG                "[PWRAP] "
#ifdef PMIC_WRAP_DEBUG
  #define PWRAPDEB(fmt, arg...)   printf(PWRAPTAG fmt, ##arg)
  #define PWRAPFUC(fmt, arg...)   printf(PWRAPTAG "%s\n", __FUNCTION__)
#endif
#define PWRAPLOG(fmt, arg...)   printf(PWRAPTAG fmt,##arg)
#define PWRAPERR(fmt, arg...)   printf(PWRAPTAG "ERROR,line=%d " fmt, __LINE__, ##arg)
#define PWRAPREG(fmt, arg...)   printf(PWRAPTAG fmt,##arg)


//---start ---external API--------------------------------------------------
S32 pwrap_read( U32  adr, U32 *rdata );
S32 pwrap_write( U32  adr, U32  wdata );
S32 pwrap_wacs2( U32  write, U32  adr, U32  wdata, U32 *rdata );
//S32 pwrap_init ( void );
//#define MACH_FPGA
#ifdef MACH_FPGA
extern S32 pwrap_init_lk ( void );
int  pwrap_init_for_early_porting(void);
#endif
//---end ---external API----------------------------------------------------

#define SLV_6323
#define PMIC_WRAP_BASE        (PWRAP_BASE)//0x1000D000

#define MT_INFRA_SYS_CFG_BASE    (CONFIG_BASE)
#define CKSYS_BASE              (IO_PHYS)

#define PMIC_WRAP_REG_RANGE   85.5
#define PMIC_WRAP_REG_MAX     0x1000F154

//-------macro for timeout setting--------------------------------
/******************************************************************************
global variable and  sys interface
******************************************************************************/
#define TIMEOUT_RESET	    0xFF //us
#define TIMEOUT_READ	    0xFF //us
#define TIMEOUT_WAIT_IDLE	0xFF //us


//-------macro for spi clock config--------------------------------
#define CLK_CFG_4_CLR                       (CKSYS_BASE+0x088) //6582
//-----------------soft reset --------------------------------------------------------
#define INFRA_GLOBALCON_RST0               (MT_INFRA_SYS_CFG_BASE+0x030)

#define PWRAP_SOFT_RESET                   WRAP_SET_BIT(1<<7,INFRA_GLOBALCON_RST0)
#define PWRAP_CLEAR_SOFT_RESET_BIT         WRAP_CLR_BIT(1<<7,INFRA_GLOBALCON_RST0)


#define PMIC_WRAP_MUX_SEL               (PMIC_WRAP_BASE+0x0)
#define PMIC_WRAP_WRAP_EN               (PMIC_WRAP_BASE+0x4)
#define PMIC_WRAP_DIO_EN                (PMIC_WRAP_BASE+0x8)
#define PMIC_WRAP_SIDLY                 (PMIC_WRAP_BASE+0xC)
#define PMIC_WRAP_OP_TYPE               (PMIC_WRAP_BASE+0x10)
#define PMIC_WRAP_MSB_FIRST             (PMIC_WRAP_BASE+0x14)
#define PMIC_WRAP_RDDMY                 (PMIC_WRAP_BASE+0x18)
#define PMIC_WRAP_SI_CK_CON             (PMIC_WRAP_BASE+0x1C)
#define PMIC_WRAP_CSHEXT_WRITE          (PMIC_WRAP_BASE+0x20)
#define PMIC_WRAP_CSHEXT_READ           (PMIC_WRAP_BASE+0x24)
#define PMIC_WRAP_CSLEXT_START          (PMIC_WRAP_BASE+0x28)
#define PMIC_WRAP_CSLEXT_END            (PMIC_WRAP_BASE+0x2C)
#define PMIC_WRAP_STAUPD_PRD            (PMIC_WRAP_BASE+0x30)
#define PMIC_WRAP_STAUPD_GRPEN          (PMIC_WRAP_BASE+0x34)
#define PMIC_WRAP_STAUPD_MAN_TRIG       (PMIC_WRAP_BASE+0x38)
#define PMIC_WRAP_STAUPD_STA            (PMIC_WRAP_BASE+0x3C)
#define PMIC_WRAP_GPS_STA               (PMIC_WRAP_BASE+0x40)
#define PMIC_WRAP_WRAP_STA              (PMIC_WRAP_BASE+0x44)
#define PMIC_WRAP_HARB_INIT             (PMIC_WRAP_BASE+0x48)
#define PMIC_WRAP_HARB_HPRIO            (PMIC_WRAP_BASE+0x4C)
#define PMIC_WRAP_HIPRIO_ARB_EN         (PMIC_WRAP_BASE+0x50)
#define PMIC_WRAP_HARB_STA0             (PMIC_WRAP_BASE+0x54)
#define PMIC_WRAP_HARB_STA1             (PMIC_WRAP_BASE+0x58)
#define PMIC_WRAP_MAN_EN                (PMIC_WRAP_BASE+0x5C)
#define PMIC_WRAP_MAN_CMD               (PMIC_WRAP_BASE+0x60)
#define PMIC_WRAP_MAN_RDATA             (PMIC_WRAP_BASE+0x64)
#define PMIC_WRAP_MAN_VLDCLR            (PMIC_WRAP_BASE+0x68)
#define PMIC_WRAP_WACS0_EN              (PMIC_WRAP_BASE+0x6C)
#define PMIC_WRAP_INIT_DONE0            (PMIC_WRAP_BASE+0x70)
#define PMIC_WRAP_WACS0_CMD             (PMIC_WRAP_BASE+0x74)
#define PMIC_WRAP_WACS0_RDATA           (PMIC_WRAP_BASE+0x78)
#define PMIC_WRAP_WACS0_VLDCLR          (PMIC_WRAP_BASE+0x7C)
#define PMIC_WRAP_WACS1_EN              (PMIC_WRAP_BASE+0x80)
#define PMIC_WRAP_INIT_DONE1            (PMIC_WRAP_BASE+0x84)
#define PMIC_WRAP_WACS1_CMD             (PMIC_WRAP_BASE+0x88)
#define PMIC_WRAP_WACS1_RDATA           (PMIC_WRAP_BASE+0x8C)
#define PMIC_WRAP_WACS1_VLDCLR          (PMIC_WRAP_BASE+0x90)
#define PMIC_WRAP_WACS2_EN              (PMIC_WRAP_BASE+0x94)
#define PMIC_WRAP_INIT_DONE2            (PMIC_WRAP_BASE+0x98)
#define PMIC_WRAP_WACS2_CMD             (PMIC_WRAP_BASE+0x9C)
#define PMIC_WRAP_WACS2_RDATA           (PMIC_WRAP_BASE+0xA0)
#define PMIC_WRAP_WACS2_VLDCLR          (PMIC_WRAP_BASE+0xA4)
#define PMIC_WRAP_INT_EN                (PMIC_WRAP_BASE+0xA8)
#define PMIC_WRAP_INT_FLG_RAW           (PMIC_WRAP_BASE+0xAC)
#define PMIC_WRAP_INT_FLG               (PMIC_WRAP_BASE+0xB0)
#define PMIC_WRAP_INT_CLR               (PMIC_WRAP_BASE+0xB4)
#define PMIC_WRAP_SIG_ADR               (PMIC_WRAP_BASE+0xB8)
#define PMIC_WRAP_SIG_MODE              (PMIC_WRAP_BASE+0xBC)
#define PMIC_WRAP_SIG_VALUE             (PMIC_WRAP_BASE+0xC0)
#define PMIC_WRAP_SIG_ERRVAL            (PMIC_WRAP_BASE+0xC4)
#define PMIC_WRAP_CRC_EN                (PMIC_WRAP_BASE+0xC8)
#define PMIC_WRAP_TIMER_EN              (PMIC_WRAP_BASE+0xCC)
#define PMIC_WRAP_TIMER_STA             (PMIC_WRAP_BASE+0xD0)
#define PMIC_WRAP_WDT_UNIT              (PMIC_WRAP_BASE+0xD4)
#define PMIC_WRAP_WDT_SRC_EN            (PMIC_WRAP_BASE+0xD8)
#define PMIC_WRAP_WDT_FLG               (PMIC_WRAP_BASE+0xDC)
#define PMIC_WRAP_DEBUG_INT_SEL         (PMIC_WRAP_BASE+0xE0)
#define PMIC_WRAP_DVFS_ADR0             (PMIC_WRAP_BASE+0xE4)
#define PMIC_WRAP_DVFS_WDATA0           (PMIC_WRAP_BASE+0xE8)
#define PMIC_WRAP_DVFS_ADR1             (PMIC_WRAP_BASE+0xEC)
#define PMIC_WRAP_DVFS_WDATA1           (PMIC_WRAP_BASE+0xF0)
#define PMIC_WRAP_DVFS_ADR2             (PMIC_WRAP_BASE+0xF4)
#define PMIC_WRAP_DVFS_WDATA2           (PMIC_WRAP_BASE+0xF8)
#define PMIC_WRAP_DVFS_ADR3             (PMIC_WRAP_BASE+0xFC)
#define PMIC_WRAP_DVFS_WDATA3           (PMIC_WRAP_BASE+0x100)
#define PMIC_WRAP_DVFS_ADR4             (PMIC_WRAP_BASE+0x104)
#define PMIC_WRAP_DVFS_WDATA4           (PMIC_WRAP_BASE+0x108)
#define PMIC_WRAP_DVFS_ADR5             (PMIC_WRAP_BASE+0x10C)
#define PMIC_WRAP_DVFS_WDATA5           (PMIC_WRAP_BASE+0x110)
#define PMIC_WRAP_DVFS_ADR6             (PMIC_WRAP_BASE+0x114)
#define PMIC_WRAP_DVFS_WDATA6           (PMIC_WRAP_BASE+0x118)
#define PMIC_WRAP_DVFS_ADR7             (PMIC_WRAP_BASE+0x11C)
#define PMIC_WRAP_DVFS_WDATA7           (PMIC_WRAP_BASE+0x120)
#define PMIC_WRAP_CIPHER_KEY_SEL        (PMIC_WRAP_BASE+0x124)
#define PMIC_WRAP_CIPHER_IV_SEL         (PMIC_WRAP_BASE+0x128)
#define PMIC_WRAP_CIPHER_EN             (PMIC_WRAP_BASE+0x12C)
#define PMIC_WRAP_CIPHER_RDY            (PMIC_WRAP_BASE+0x130)
#define PMIC_WRAP_CIPHER_MODE           (PMIC_WRAP_BASE+0x134)
#define PMIC_WRAP_CIPHER_SWRST          (PMIC_WRAP_BASE+0x138)
#define PMIC_WRAP_DCM_EN                (PMIC_WRAP_BASE+0x13C)
#define PMIC_WRAP_DCM_DBC_PRD           (PMIC_WRAP_BASE+0x140)
#define PMIC_WRAP_ADC_CMD_ADDR          (PMIC_WRAP_BASE+0x144)
#define PMIC_WRAP_PWRAP_ADC_CMD         (PMIC_WRAP_BASE+0x148)
#define PMIC_WRAP_ADC_RDY_ADDR          (PMIC_WRAP_BASE+0x14C)
#define PMIC_WRAP_ADC_RDATA_ADDR1       (PMIC_WRAP_BASE+0x150)
#define PMIC_WRAP_ADC_RDATA_ADDR2       (PMIC_WRAP_BASE+0x154)

#ifdef MT6572
#define PMIC_WRAP_SWRST                 (PMIC_WRAP_BASE+0x180)
#define PMIC_WRAP_DEBUG_SEL             (PMIC_WRAP_BASE+0x190)
#endif



//-----macro for wrapper  regsister--------------------------------------------------------
#define GET_STAUPD_DLE_CNT(x)        ((x>>0)  & 0x00000007)
#define GET_STAUPD_ALE_CNT(x)        ((x>>3)  & 0x00000007)
#define GET_STAUPD_FSM(x)            ((x>>6)  & 0x00000007)
#define GET_PWRAP_GPS_ACK(x)         ((x>>0)  & 0x00000001)
#define GET_GPS_PWRAP_REQ(x)         ((x>>1)  & 0x00000001)
#define GET_GPSINF_DLE_CNT(x)        ((x>>4)  & 0x00000003)
#define GET_GPSINF_ALE_CNT(x)        ((x>>6)  & 0x00000003)
#define GET_GPS_INF_FSM(x)           ((x>>8)  & 0x00000007)
#define GET_PWRAP_GPS_WDATA(x)       ((x>>15) & 0x0001ffff)
#define GET_WRAP_CH_DLE_RESTCNT(x)   ((x>>0)  & 0x00000007)
#define GET_WRAP_CH_ALE_RESTCNT(x)   ((x>>3)  & 0x00000003)
#define GET_WRAP_AG_DLE_RESTCNT(x)   ((x>>5)  & 0x00000003)
#define GET_WRAP_CH_W(x)             ((x>>7)  & 0x00000001)
#define GET_WRAP_CH_REQ(x)           ((x>>8)  & 0x00000001)
#define GET_AG_WRAP_W(x)             ((x>>9)  & 0x00000001)
#define GET_AG_WRAP_REQ(x)           ((x>>10) & 0x00000001)
#define GET_WRAP_FSM(x)              ((x>>11) & 0x0000000f)
#define GET_HARB_WRAP_WDATA(x)       ((x>>0)  & 0x0000ffff)
#define GET_HARB_WRAP_ADR(x)         ((x>>16) & 0x00007fff)
#define GET_HARB_WRAP_REQ(x)         ((x>>31) & 0x00000001)
#define GET_HARB_DLE_EMPTY(x)        ((x>>0)  & 0x00000001)
#define GET_HARB_DLE_FULL(x)         ((x>>1)  & 0x00000001)
#define GET_HARB_VLD(x)              ((x>>2)  & 0x00000001)
#define GET_HARB_DLE_OWN(x)          ((x>>3)  & 0x0000000f)
#define GET_HARB_OWN(x)              ((x>>7)  & 0x0000000f)
#define GET_HARB_DLE_RESTCNT(x)      ((x>>11) & 0x0000000f)
#define GET_AG_HARB_REQ(x)           ((x>>15) & 0x0000007f)
#define GET_HARB_WRAP_W(x)           ((x>>22) & 0x00000001)
#define GET_HARB_WRAP_REQ0(x)        ((x>>23) & 0x00000001)
#define GET_SPI_WDATA(x)             ((x>>0)  & 0x000000ff)
#define GET_SPI_OP(x)                ((x>>8)  & 0x0000001f)
#define GET_SPI_W(x)                 ((x>>13) & 0x00000001)
#define GET_MAN_RDATA(x)             ((x>>0)  & 0x000000ff)
#define GET_MAN_FSM(x)               ((x>>8)  & 0x00000007)
#define GET_MAN_REQ(x)               ((x>>11) & 0x00000001)
#define GET_WACS0_WDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS0_ADR(x)             ((x>>16) & 0x00007fff)
#define GET_WACS0_WRITE(x)           ((x>>31) & 0x00000001)
#define GET_WACS0_RDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS0_FSM(x)             ((x>>16) & 0x00000007)
#define GET_WACS0_REQ(x)             ((x>>19) & 0x00000001)
#define GET_SYNC_IDLE0(x)            ((x>>20) & 0x00000001)
#define GET_INIT_DONE0(x)            ((x>>21) & 0x00000001)
#define GET_WACS1_WDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS1_ADR(x)             ((x>>16) & 0x00007fff)
#define GET_WACS1_WRITE(x)           ((x>>31) & 0x00000001)
#define GET_WACS1_RDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS1_FSM(x)             ((x>>16) & 0x00000007)
#define GET_WACS1_REQ(x)             ((x>>19) & 0x00000001)
#define GET_SYNC_IDLE1(x)            ((x>>20) & 0x00000001)
#define GET_INIT_DONE1(x)            ((x>>21) & 0x00000001)
#define GET_WACS2_WDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS2_ADR(x)             ((x>>16) & 0x00007fff)
#define GET_WACS2_WRITE(x)           ((x>>31) & 0x00000001)
#define GET_WACS2_RDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS2_FSM(x)             ((x>>16) & 0x00000007)
#define GET_WACS2_REQ(x)             ((x>>19) & 0x00000001)
#define GET_SYNC_IDLE2(x)            ((x>>20) & 0x00000001)
#define GET_INIT_DONE2(x)            ((x>>21) & 0x00000001)



//macro for staupd sta fsm
#define STAUPD_FSM_IDLE               (0x00)
#define STAUPD_FSM_REQ                (0x02)
#define STAUPD_FSM_WFDLE              (0x04) //wait for dle,wait for read data done,

//macro for WRAP_STA  FSM
//#define WRAP_STA_FSM_IDLE               (0x00)
//#define WRAP_STA_IDLE               (0x00)

//macro for MAN_RDATA  FSM
#define MAN_FSM_NO_REQ             (0x00)
#define MAN_FSM_IDLE               (0x00)
#define MAN_FSM_REQ                (0x02)
#define MAN_FSM_WFDLE              (0x04) //wait for dle,wait for read data done,
#define MAN_FSM_WFVLDCLR           (0x06)

//macro for WACS_FSM
#define WACS_FSM_IDLE               (0x00)
#define WACS_FSM_REQ                (0x02)
#define WACS_FSM_WFDLE              (0x04) //wait for dle,wait for read data done,
#define WACS_FSM_WFVLDCLR           (0x06) //finish read data , wait for valid flag clearing
#define WACS_INIT_DONE              (0x01)
#define WACS_SYNC_IDLE              (0x01)
#define WACS_SYNC_BUSY              (0x00)



//-----macro for  regsister@PMIC -------------------------------------------------
#define PMIC_REG_BASE             (0x0000)
#define DEW_BASE              (0x018A)

#define WRAP_DEW_DIO_EN         (PMIC_REG_BASE+0x018A)
#define WRAP_DEW_READ_TEST      (PMIC_REG_BASE+0x018C)
#define WRAP_DEW_WRITE_TEST     (PMIC_REG_BASE+0x018E)
#define WRAP_DEW_CRC_SWRST      (PMIC_REG_BASE+0x0190)
#define WRAP_DEW_CRC_EN         (PMIC_REG_BASE+0x0192)
#define WRAP_DEW_CRC_VAL        (PMIC_REG_BASE+0x0194)
#define WRAP_DEW_DBG_MON_SEL    (PMIC_REG_BASE+0x0196)
#define WRAP_DEW_CIPHER_KEY_SEL (PMIC_REG_BASE+0x0198)
#define WRAP_DEW_CIPHER_IV_SEL  (PMIC_REG_BASE+0x019A)
#define WRAP_DEW_CIPHER_EN      (PMIC_REG_BASE+0x019C)
#define WRAP_DEW_CIPHER_RDY     (PMIC_REG_BASE+0x019E)
#define WRAP_DEW_CIPHER_MODE    (PMIC_REG_BASE+0x01A0)
#define WRAP_DEW_CIPHER_SWRST   (PMIC_REG_BASE+0x01A2)
#define WRAP_DEW_RDDMY_NO       (PMIC_REG_BASE+0x01A4)
#define WRAP_DEW_RDATA_DLY_SEL  (PMIC_REG_BASE+0x01A6)

#define WRAP_AUXADC_CON21       (PMIC_REG_BASE+0x076C)
#define WRAP_AUXADC_ADC12       (PMIC_REG_BASE+0x072C)
#define WRAP_AUXADC_ADC13       (PMIC_REG_BASE+0x072E)
#define WRAP_AUXADC_ADC14       (PMIC_REG_BASE+0x0730)

#ifdef SLV_6320
  #define PMIC_TOP_CKCON2           (PMIC_BASE+0x012A)
#elif defined SLV_6323
  #define WRAP_TOP_CKCON1_CLR     (PMIC_REG_BASE+0x012A)
  #define WRAP_TOP_CKCON1         (PMIC_REG_BASE+0x0126)
#endif




//-----macro for dewrapper defaule value-------------------------------------------------------
#define DEFAULT_VALUE_READ_TEST      0x5aa5
#define WRITE_TEST_VALUE             0xa55a

//-----macro for manual commnd --------------------------------------------------------
#define OP_WR    (0x1)
#define OP_RD    (0x0)
#define OP_CSH   (0x0)
#define OP_CSL   (0x1)
#define OP_CK    (0x2)
#define OP_OUTS  (0x8)
#define OP_OUTD  (0x9)
#define OP_OUTQ  (0xA)
#define OP_INS   (0xC)
#define OP_INS0  (0xD)
#define OP_IND   (0xE)
#define OP_INQ   (0xF)

#define OP_OS2IS (0x10)
#define OP_OS2ID (0x11)
#define OP_OS2IQ (0x12)
#define OP_OD2IS (0x13)
#define OP_OD2ID (0x14)
#define OP_OD2IQ (0x15)
#define OP_OQ2IS (0x16)
#define OP_OQ2ID (0x17)
#define OP_OQ2IQ (0x18)

#define OP_OSNIS (0x19)
#define OP_ODNID (0x1A)


//-----macro for read/write register --------------------------------------------------------

//#define WRAP_RD32(addr)            (*(volatile U32 *)(addr))
//#define WRAP_WR32(addr,data)       ((*(volatile U32 *)(addr)) = (U32)data)

//#define WRAP_SET_BIT(BS,REG)       ((*(volatile U32*)(REG)) |= (U32)(BS))
//#define WRAP_CLR_BIT(BS,REG)       ((*(volatile U32*)(REG)) &= ~((U32)(BS)))
#define WRAP_RD32(addr)            DRV_Reg32(addr)
#define WRAP_WR32(addr,val)        mt65xx_reg_sync_writel((val), (addr))

#define WRAP_SET_BIT(BS,REG)       mt65xx_reg_sync_writel((DRV_Reg32(REG) | (U32)(BS)), (REG))
#define WRAP_CLR_BIT(BS,REG)       mt65xx_reg_sync_writel((DRV_Reg32(REG) & (~(U32)(BS))), (REG))


#define E_PWR_INVALID_ARG               1
#define E_PWR_INVALID_RW                2
#define E_PWR_INVALID_ADDR              3
#define E_PWR_INVALID_WDAT              4
#define E_PWR_INVALID_OP_MANUAL         5
#define E_PWR_NOT_IDLE_STATE            6
#define E_PWR_NOT_INIT_DONE             7
#define E_PWR_NOT_INIT_DONE_READ        8
#define E_PWR_WAIT_IDLE_TIMEOUT         9
#define E_PWR_WAIT_IDLE_TIMEOUT_READ    10
#define E_PWR_INIT_SIDLY_FAIL           11
#define E_PWR_RESET_TIMEOUT             12
#define E_PWR_TIMEOUT                   13


#define E_PWR_INIT_RESET_SPI            20
#define E_PWR_INIT_SIDLY                21
#define E_PWR_INIT_REG_CLOCK            22
#define E_PWR_INIT_ENABLE_PMIC          23
#define E_PWR_INIT_DIO                  24
#define E_PWR_INIT_CIPHER               25
#define E_PWR_INIT_WRITE_TEST           26
#define E_PWR_INIT_ENABLE_CRC           27
#define E_PWR_INIT_ENABLE_DEWRAP        28

#define E_PWR_READ_TEST_FAIL            30
#define E_PWR_WRITE_TEST_FAIL           31
#define E_PWR_SWITCH_DIO                32


#endif //__PMIC_WRAP_REGS_H__

