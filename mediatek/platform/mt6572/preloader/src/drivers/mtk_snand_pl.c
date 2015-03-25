/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "typedefs.h"
#include "platform.h"
#include "blkdev.h"
#include "cust_nand.h"
#include "mtk_nand.h"
#include "mtk_nand_core.h"
#include "mtk_snand_pl.h"
#include "snand_device_list.h"
#include "bmt.h"
#include "part.h"
#include "partition_define.h"

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE (2)

// FIXME: find a better place for these definitions
#define NAND_SECTOR_SIZE        (512)
#define NAND_FDM_PER_SECTOR	    (8)

/******************************************************************************
*
* Macro definition
*
*******************************************************************************/

#define NFI_SET_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) | (value)))
#define NFI_SET_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) | (value)))
#define NFI_CLN_REG32(reg, value)   (DRV_WriteReg32(reg, DRV_Reg32(reg) & (~(value))))
#define NFI_CLN_REG16(reg, value)   (DRV_WriteReg16(reg, DRV_Reg16(reg) & (~(value))))

#define FIFO_PIO_READY(x)  (0x1 & x)
#define WAIT_NFI_PIO_READY(timeout) \
    do {\
    while( (!FIFO_PIO_READY(DRV_Reg(NFI_PIO_DIRDY_REG16))) && (--timeout) );\
    if(timeout == 0)\
   {\
   MSG(ERR, "Error: FIFO_PIO_READY timeout at line=%d, file =%s\n", __LINE__, __FILE__);\
   }\
    } while(0);

#define TIMEOUT_1   0x1fff
#define TIMEOUT_2   0x8ff
#define TIMEOUT_3   0xffff
#define TIMEOUT_4   5000        //PIO

#define STATUS_READY			(0x40)
#define STATUS_FAIL				(0x01)
#define STATUS_WR_ALLOW			(0x80)

u32 PAGE_SIZE;
u32 BLOCK_SIZE;

typedef enum
{
     SNAND_RB_DEFAULT    = 0
    ,SNAND_RB_READ_ID    = 1
    ,SNAND_RB_CMD_STATUS = 2
    ,SNAND_RB_PIO        = 3
} SNAND_Read_Byte_Mode;

SNAND_Read_Byte_Mode g_snand_read_byte_mode;

u8 g_snand_id_data[SNAND_MAX_ID + 1];
u8 g_snand_id_data_idx = 0;

/**************************************************************************
*  MACRO LIKE FUNCTION
**************************************************************************/

static inline u32 PAGE_NUM(u32 logical_size)
{
    return ((unsigned long)(logical_size) / PAGE_SIZE);
}

inline u32 LOGICAL_ADDR(u32 page_addr)
{
    return ((unsigned long)(page_addr) * PAGE_SIZE);
}

inline u32 BLOCK_ALIGN(u32 logical_addr)
{
    return (((u32) (logical_addr / BLOCK_SIZE)) * BLOCK_SIZE);
}

//---------------------------------------------------------------------------

//-------------------------------------------------------------------------
typedef U32(*STORGE_READ) (u8 * buf, u32 start, u32 img_size);

typedef struct
{
    u32 page_size;
    u32 pktsz;
} device_info_t;
//-------------------------------------------------------------------------

device_info_t gdevice_info;
boot_dev_t g_dev_vfunc;
static blkdev_t g_nand_bdev;
__attribute__((aligned(4))) unsigned char g_nand_spare[128];
u8 * g_snand_temp;

unsigned int nand_maf_id;
unsigned int nand_dev_id;
uint8 ext_id1, ext_id2, ext_id3;

static u32 g_i4ErrNum;
static BOOL g_bInitDone;
BOOL g_bHwEcc = TRUE;
u32 PAGE_SIZE;
u32 BLOCK_SIZE;
__attribute__((section(".bss.uninit"))) u8 Bad_Block_Table[8192] = { 0 }; // address will be at EMI and non-ZI.

struct nand_chip g_nand_chip;
struct nand_ecclayout *nand_oob = NULL;

static struct nand_ecclayout nand_oob_16 = {
    .eccbytes = 8,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
    .oobfree = {{1, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_64 = {
    .eccbytes = 32,
    .eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
               40, 41, 42, 43, 44, 45, 46, 47,
               48, 49, 50, 51, 52, 53, 54, 55,
               56, 57, 58, 59, 60, 61, 62, 63},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

struct nand_ecclayout nand_oob_128 = {
    .eccbytes = 64,
    .eccpos = {
               64, 65, 66, 67, 68, 69, 70, 71,
               72, 73, 74, 75, 76, 77, 78, 79,
               80, 81, 82, 83, 84, 85, 86, 87,
               88, 89, 90, 91, 92, 93, 94, 95,
               96, 97, 98, 99, 100, 101, 102, 103,
               104, 105, 106, 107, 108, 109, 110, 111,
               112, 113, 114, 115, 116, 117, 118, 119,
               120, 121, 122, 123, 124, 125, 126, 127},
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

struct NAND_CMD
{
    u32 u4ColAddr;
    u32 u4RowAddr;
    u32 u4OOBRowAddr;
    u8 au1OOB[64];
    u8 *pDataBuf;
};

static struct NAND_CMD g_kCMD;
static snand_flashdev_info devinfo;
static char *nfi_buf;

static bool     mtk_snand_get_device_info(u8*id, snand_flashdev_info *devinfo);
static u32      mtk_snand_gen_c1a3(const u32 cmd, const u32 address);
static void     mtk_snand_dev_enable_spiq(bool enable);

#ifdef CFG_SNAND_STT
void            stt_snand_main(void);
#endif

struct nand_manufacturers nand_manuf_ids[] = {
    {NAND_MANFR_TOSHIBA, "Toshiba"},
    {NAND_MANFR_SAMSUNG, "Samsung"},
    {NAND_MANFR_FUJITSU, "Fujitsu"},
    {NAND_MANFR_NATIONAL, "National"},
    {NAND_MANFR_RENESAS, "Renesas"},
    {NAND_MANFR_STMICRO, "ST Micro"},
    {NAND_MANFR_HYNIX, "Hynix"},
    {NAND_MANFR_MICRON, "Micron"},
    {NAND_MANFR_AMD, "AMD"},
    {0x0, "Unknown"}
};

static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff))
    {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff))
    {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf))
    {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3))
    {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1))
    {
        x >>= 1;
        r += 1;
    }
    return r;
}

/**************************************************************************
*  reset descriptor
**************************************************************************/
void mtk_snand_reset_descriptor(void)
{

    g_nand_chip.page_shift = 0;
    g_nand_chip.page_size = 0;
    g_nand_chip.ChipID = 0;     /* Type of DiskOnChip */
    g_nand_chip.chips_name = 0;
    g_nand_chip.chipsize = 0;
    g_nand_chip.erasesize = 0;
    g_nand_chip.mfr = 0;        /* Flash IDs - only one type of flash per device */
    g_nand_chip.id = 0;
    g_nand_chip.name = 0;
    g_nand_chip.numchips = 0;
    g_nand_chip.oobblock = 0;   /* Size of OOB blocks (e.g. 512) */
    g_nand_chip.oobsize = 0;    /* Amount of OOB data per block (e.g. 16) */
    g_nand_chip.eccsize = 0;
    g_nand_chip.bus16 = 0;
    g_nand_chip.nand_ecc_mode = 0;

}

static bool mtk_snand_get_device_info(u8*id, snand_flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1,target_id_len=-1;

    for (i = 0; i<SNAND_CHIP_CNT; i++){
		mismatch=0;
		for(m=0;m<gen_snand_FlashTable[i].id_length;m++){
			if(id[m]!=gen_snand_FlashTable[i].id[m]){
				mismatch=1;
				break;
			}
		}
		if(mismatch == 0 && gen_snand_FlashTable[i].id_length > target_id_len){
				target=i;
				target_id_len=gen_snand_FlashTable[i].id_length;
		}
    }

    if(target != -1){
		MSG(INIT, "Recognize NAND: ID [");
		for(n=0;n<gen_snand_FlashTable[target].id_length;n++){
			devinfo->id[n] = gen_snand_FlashTable[target].id[n];
			MSG(INIT, "%x ",devinfo->id[n]);
		}
		MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_snand_FlashTable[target].devicename,gen_snand_FlashTable[target].pagesize,gen_snand_FlashTable[target].sparesize,gen_snand_FlashTable[target].totalsize);
		devinfo->id_length=gen_snand_FlashTable[i].id_length;
		devinfo->blocksize = gen_snand_FlashTable[target].blocksize;
		devinfo->advancedmode = gen_snand_FlashTable[target].advancedmode;

        // SW workaround for SNAND_ADV_READ_SPLIT
        if (0xC8 == devinfo->id[0] && 0xF4 == devinfo->id[1])
        {
            devinfo->advancedmode |= (SNAND_ADV_READ_SPLIT | SNAND_ADV_VENDOR_RESERVED_BLOCKS);
        }

		devinfo->pagesize = gen_snand_FlashTable[target].pagesize;
		devinfo->sparesize = gen_snand_FlashTable[target].sparesize;
		devinfo->totalsize = gen_snand_FlashTable[target].totalsize;
		devinfo->SNF_DLY_CTL1 = gen_snand_FlashTable[target].SNF_DLY_CTL1;
		devinfo->SNF_DLY_CTL2 = gen_snand_FlashTable[target].SNF_DLY_CTL2;
		devinfo->SNF_DLY_CTL3 = gen_snand_FlashTable[target].SNF_DLY_CTL3;
		devinfo->SNF_DLY_CTL4 = gen_snand_FlashTable[target].SNF_DLY_CTL4;
		devinfo->SNF_MISC_CTL = gen_snand_FlashTable[target].SNF_MISC_CTL;
		devinfo->SNF_DRIVING = gen_snand_FlashTable[target].SNF_DRIVING;
		memcpy(devinfo->devicename, gen_snand_FlashTable[target].devicename, sizeof(devinfo->devicename));
    	return true;
	}else{
	    MSG(INIT, "Not Found NAND: ID [");
		for(n=0;n<SNAND_MAX_ID;n++){
			MSG(INIT, "%x ",id[n]);
		}
		MSG(INIT, "]\n");
        return false;
	}
}

//---------------------------------------------------------------------------
static bool mtk_snand_check_RW_count(u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;

    while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
static bool mtk_snand_status_ready(u32 u4Status)
{
    u32 timeout = 0xFFFF;

	u4Status &= ~STA_NAND_BUSY;

    while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;

        if (0 == timeout)
        {
            return FALSE;
        }
    }

    return TRUE;
}

//---------------------------------------------------------------------------
static void mtk_snand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

//---------------------------------------------------------------------------
static void mtk_snand_ecc_decode_start(void)
{
    u32 reg;

    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;

    reg = DRV_Reg32(ECC_DECCNFG_REG32);
    reg &= ~DEC_CNFG_DEC_MODE_MASK;
    reg |= DEC_CNFG_NFI;
    DRV_WriteReg32(ECC_DECCNFG_REG32, reg);

    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

//---------------------------------------------------------------------------
static void mtk_snand_ecc_decode_end(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

//---------------------------------------------------------------------------
static void mtk_snand_ecc_encode_start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

//---------------------------------------------------------------------------
static void mtk_snand_ecc_encode_end(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

//---------------------------------------------------------------------------
static void mtk_snand_ecc_config(u32 ecc_bit)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;

    u32 ecc_bit_cfg = ECC_CNFG_ECC4;

    switch (ecc_bit)
    {
      case 4:
          ecc_bit_cfg = ECC_CNFG_ECC4;
          break;
      case 8:
          ecc_bit_cfg = ECC_CNFG_ECC8;
          break;
      case 10:
          ecc_bit_cfg = ECC_CNFG_ECC10;
          break;
      case 12:
          ecc_bit_cfg = ECC_CNFG_ECC12;
          break;
      default:
          break;
    }

    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    do
    {;
    }
    while (!DRV_Reg16(ECC_DECIDLE_REG16));

    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
    do
    {;
    }
    while (!DRV_Reg32(ECC_ENCIDLE_REG32));

    /* setup FDM register base */
    DRV_WriteReg32(ECC_FDMADDR_REG32, NFI_FDM0L_REG32);

    u4ENCODESize = (NAND_SECTOR_SIZE + 8) << 3;
    u4DECODESize = ((NAND_SECTOR_SIZE + 8) << 3) + ecc_bit * 13;

    /* configure ECC decoder && encoder */
    DRV_WriteReg32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));

    DRV_WriteReg32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));

#ifndef MANUAL_CORRECT
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif

}

/******************************************************************************
* mtk_snand_check_bch_error
*
* DESCRIPTION:
*   Check BCH error or not !
*
* PARAMETERS:
*   struct mtd_info *mtd
*    u8* pDataBuf
*    u32 u4SecIndex
*    u32 u4PageAddr
*
* RETURNS:
*   None
*
* NOTES:
*   None
*
******************************************************************************/
static bool mtk_snand_check_bch_error(u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = TRUE;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0, u4ErrorNumDebug1, i, u4ErrNum;
    u32 timeout = 0xFFFF;

#ifdef MANUAL_CORRECT
    u32 au4ErrBitLoc[6];
    u32 u4ErrByteLoc, u4BitOffset;
    u32 u4ErrBitLoc1th, u4ErrBitLoc2nd;
#endif

    while (0 == (u2SectorDoneMask & DRV_Reg16(ECC_DECDONE_REG16)))
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
#ifndef MANUAL_CORRECT
    u4ErrorNumDebug0 = DRV_Reg32(ECC_DECENUM0_REG32);
    u4ErrorNumDebug1 = DRV_Reg32(ECC_DECENUM1_REG32);

    if (0 != (u4ErrorNumDebug0 & 0xFFFFF) || 0 != (u4ErrorNumDebug1 & 0xFFFFF))
    {
        for (i = 0; i <= u4SecIndex; ++i)
        {
            if (i < 4)
            {
                u4ErrNum = DRV_Reg32(ECC_DECENUM0_REG32) >> (i * 5);
            }
            else
            {
                u4ErrNum = DRV_Reg32(ECC_DECENUM1_REG32) >> ((i - 4) * 5);
            }

            u4ErrNum &= 0x1F;

            if (0x1F == u4ErrNum)
            {
                MSG(ERR, "In Preloader UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
                bRet = false;
            }
            else
            {
            	if (u4ErrNum)
                {
            	    MSG(ERR, " In Preloader Correct %d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
            	}
            }
        }
    }
#else
/* We will manually correct the error bits in the last sector, not all the sectors of the page!*/
    //memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug = DRV_Reg32(ECC_DECENUM_REG32);
    u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
    u4ErrNum &= 0xF;
    if (u4ErrNum)
    {
        if (0xF == u4ErrNum)
        {
            //mtd->ecc_stats.failed++;
            bRet = FALSE;
        } else
        {
            for (i = 0; i < ((u4ErrNum + 1) >> 1); ++i)
            {
                au4ErrBitLoc[i] = DRV_Reg32(ECC_DECEL0_REG32 + i);
                u4ErrBitLoc1th = au4ErrBitLoc[i] & 0x1FFF;
                if (u4ErrBitLoc1th < 0x1000)
                {
                    u4ErrByteLoc = u4ErrBitLoc1th / 8;
                    u4BitOffset = u4ErrBitLoc1th % 8;
                    pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    //mtd->ecc_stats.corrected++;
                } else
                {
                    //mtd->ecc_stats.failed++;
                    MSG(INIT, "UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }
                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                        //mtd->ecc_stats.corrected++;
                    } else
                    {
                        //mtd->ecc_stats.failed++;
                        MSG(INIT, "UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
        }
        if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = FALSE;
        }
    }
#endif
    return bRet;
}

//---------------------------------------------------------------------------
static bool mtk_snand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
static bool mtk_snand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return FALSE;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
bool mtk_snand_reset_con(void)
{
    int timeout = 0xFFFF;

    // part 1. SNF

    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL | SNAND_SW_RST;
    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL & ~SNAND_SW_RST;

    // part 2. NFI

    if (DRV_Reg16(NFI_MASTERSTA_REG16)) // master is busy
    {
        DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

        while (DRV_Reg16(NFI_MASTERSTA_REG16))
        {
            timeout--;

            if (!timeout)
            {
                MSG(INIT, "MASTERSTA timeout\n");
            }
        }
    }

    /* issue reset operation */
    DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    //return mtk_snand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_snand_RFIFOValidSize(0) && mtk_snand_WFIFOValidSize(0);
    return mtk_snand_status_ready(STA_NFI_FSM_MASK | STA_NAND_FSM_MASK) && mtk_snand_RFIFOValidSize(0) && mtk_snand_WFIFOValidSize(0);
}

static bool mtk_snand_read_status(void)
{
    return TRUE;

    #if 0
    int status, i;
    mtk_snand_reset_con();
    unsigned int timeout;

    mtk_snand_reset_con();

    /* Disable HW ECC */
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    /* Disable 16-bit I/O */
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_OP_SRD | CNFG_READ_EN | CNFG_BYTE_RW);

    DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NOB_SHIFT));

    DRV_WriteReg16(NFI_CON_REG16, 0x3);
    mtk_snand_set_mode(CNFG_OP_SRD);
    DRV_WriteReg16(NFI_CNFG_REG16, 0x2042);
    mtk_snand_set_command(NAND_CMD_STATUS);
    DRV_WriteReg16(NFI_CON_REG16, 0x90);

    timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);

    if (timeout)
    {
        status = (DRV_Reg16(NFI_DATAR_REG32));
    }
    //~  clear NOB
    DRV_WriteReg16(NFI_CON_REG16, 0);

    if (g_nand_chip.bus16 == NAND_BUS_WIDTH_16)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    // check READY/BUSY status first
    if (!(STATUS_READY & status))
    {
        MSG(ERR, "status is not ready\n");
    }
    // flash is ready now, check status code
    if (STATUS_FAIL & status)
    {
        if (!(STATUS_WR_ALLOW & status))
        {
            MSG(INIT, "status locked\n");
            return FALSE;
        } else
        {
            MSG(INIT, "status unknown\n");
            return FALSE;
        }
    } else
    {
        return TRUE;
    }
    #endif
}

//---------------------------------------------------------------------------

static void mtk_snand_configure_fdm(u16 u2FDMSize)
{
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//---------------------------------------------------------------------------
static void mtk_snand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

static void mtk_snand_wait_us(u32 us)
{
    udelay(us);
}

static void mtk_snand_dev_mac_enable(SNAND_Mode mode)
{
    u32 mac;

    mac = DRV_Reg32(RW_SNAND_MAC_CTL);

    // SPI
    if (mode == SPI)
    {
        mac &= ~SNAND_MAC_SIO_SEL;   // Clear SIO_SEL to send command in SPI style
        mac |= SNAND_MAC_EN;         // Enable Macro Mode
    }
    // QPI
    else
    {
        /*
         * SFI V2: QPI_EN only effects direct read mode, and it is moved into DIRECT_CTL in V2
         *         There's no need to clear the bit again.
         */
        mac |= (SNAND_MAC_SIO_SEL | SNAND_MAC_EN);  // Set SIO_SEL to send command in QPI style, and enable Macro Mode
    }

    DRV_WriteReg32(RW_SNAND_MAC_CTL, mac);
}

/**
 * @brief This funciton triggers SFI to issue command to serial flash, wait SFI until ready.
 *
 * @remarks: !NOTE! This function must be used with mtk_snand_dev_mac_enable in pair!
 */
static void mtk_snand_dev_mac_trigger(void)
{
    u32 mac;

    mac = DRV_Reg32(RW_SNAND_MAC_CTL);

    // Trigger SFI: Set TRIG and enable Macro mode
    mac |= (SNAND_TRIG | SNAND_MAC_EN);
    DRV_WriteReg32(RW_SNAND_MAC_CTL, mac);

    /*
     * Wait for SFI ready
     * Step 1. Wait for WIP_READY becoming 1 (WIP register is ready)
     */
    while (!(DRV_Reg32(RW_SNAND_MAC_CTL) & SNAND_WIP_READY));

    /*
     * Step 2. Wait for WIP becoming 0 (Controller finishes command write process)
     */
    while ((DRV_Reg32(RW_SNAND_MAC_CTL) & SNAND_WIP));


}

/**
 * @brief This funciton leaves Macro mode and enters Direct Read mode
 *
 * @remarks: !NOTE! This function must be used after mtk_snand_dev_mac_trigger
 */
static void mtk_snand_dev_mac_leave(void)
{
    u32 mac;

    // clear SF_TRIG and leave mac mode
    mac = DRV_Reg32(RW_SNAND_MAC_CTL);

    /*
     * Clear following bits
     * SF_TRIG: Confirm the macro command sequence is completed
     * SNAND_MAC_EN: Leaves macro mode, and enters direct read mode
     * SNAND_MAC_SIO_SEL: Always reset quad macro control bit at the end
     */
    mac &= ~(SNAND_TRIG | SNAND_MAC_EN | SNAND_MAC_SIO_SEL);
    DRV_WriteReg32(RW_SNAND_MAC_CTL, mac);
}

static void mtk_snand_dev_mac_op(SNAND_Mode mode)
{
    mtk_snand_dev_mac_enable(mode);
    mtk_snand_dev_mac_trigger();
    mtk_snand_dev_mac_leave();
}

static void mtk_snand_dev_command_ext(SNAND_Mode mode, const U8 cmd[], U8 data[], const u32 outl, const u32 inl)
{
    u32   tmp;
    u32   i, j;
    P_U8  p_data, p_tmp;

    p_tmp = (P_U8)(&tmp);

    // Moving commands into SFI GPRAM
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA); i < outl; p_data += 4)
    {
        // Using 4 bytes aligned copy, by moving the data into the temp buffer and then write to GPRAM
        for (j = 0, tmp = 0; i < outl && j < 4; i++, j++)
        {
            p_tmp[j] = cmd[i];
        }

        DRV_WriteReg32(p_data, tmp);
    }

    DRV_WriteReg32(RW_SNAND_MAC_OUTL, outl);
    DRV_WriteReg32(RW_SNAND_MAC_INL, inl);
    mtk_snand_dev_mac_op(mode);

    // for NULL data, this loop will be skipped
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data)
    {
        *data = DRV_Reg8(p_data);
    }

    return;
}

static void mtk_snand_dev_command(const u32 cmd, u8 outlen)
{
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, outlen);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 0);
    mtk_snand_dev_mac_op(SPI);

    return;
}

static void mtk_snand_reset_dev()
{
    u8 cmd = SNAND_CMD_SW_RESET;

    // issue SW RESET command to device
    mtk_snand_dev_command_ext(SPI, &cmd, NULL, 1, 0);

    // wait for awhile, then polling status register (required by spec)
    mtk_snand_wait_us(SNAND_DEV_RESET_LATENCY_US);

    *RW_SNAND_GPRAM_DATA = (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8));
    *RW_SNAND_MAC_OUTL = 2;
    *RW_SNAND_MAC_INL = 1;

    // polling status register

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (cmd & SNAND_STATUS_OIP))
        {
            break;
        }
    }
}

static void mtk_snand_dev_read_id(u8 id[])
{
    u8 cmd = SNAND_CMD_READ_ID;

    mtk_snand_dev_command_ext(SPI, &cmd, id, 1, SNAND_MAX_ID + 1);
}

//---------------------------------------------------------------------------
static void mtk_snand_command_bp(unsigned command)
{
    switch (command)
    {
        case NAND_CMD_READID:

            /* Issue NAND chip reset command */
            mtk_snand_reset_dev();
            mtk_snand_reset_con();

            mtk_snand_dev_read_id(g_snand_id_data);
            g_snand_read_byte_mode = SNAND_RB_READ_ID;
            g_snand_id_data_idx = 1;    // skip one dummy byte (0) for SPI-NAND

            break;

        default:
            break;
    }
}

//-----------------------------------------------------------------------------
static u8 mtk_snand_read_byte(void)
{
    /* Check the PIO bit is ready or not */
    u32 timeout = TIMEOUT_4;
    u8  reg8;

    if (SNAND_RB_READ_ID == g_snand_read_byte_mode) // read ID
    {
        if (g_snand_id_data_idx > SNAND_MAX_ID)	// note: g_snand_id_data_idx is 1 by default!
        {
            return 0;
        }
        else
        {
            return g_snand_id_data[g_snand_id_data_idx++];
        }
    }
    else
    {
        WAIT_NFI_PIO_READY(timeout);    // FIXME

        return DRV_Reg8(NFI_DATAR_REG32);
    }
}

bool mtk_snand_get_flash_id(u8 * nand_id, int longest_id_number)
{
    u8 maf_id = 0;
    u8 dev_id = 0;
    int i = 0;
    u8 *id = nand_id;

    //DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    mtk_snand_command_bp(NAND_CMD_READID);

    maf_id = mtk_snand_read_byte();
    dev_id = mtk_snand_read_byte();

    if (maf_id == 0 || dev_id == 0)
    {
        return FALSE;
    }
    //*id= (dev_id<<8)|maf_id;
    //    *id= (maf_id<<8)|dev_id;
    id[0] = maf_id;
    id[1] = dev_id;

    for (i = 2; i < longest_id_number; i++)
        id[i] = mtk_snand_read_byte();

    return TRUE;
}

void mtk_snand_dev_ecc_control(u8 enable)
{
    u32 cmd;
    u8  otp;
    u8  otp_new;

    // read original otp settings

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    otp = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    if (enable == TRUE)
    {
        otp_new = otp | SNAND_OTP_ECC_ENABLE;
    }
    else
    {
        otp_new = otp & ~SNAND_OTP_ECC_ENABLE;
    }

    if (otp != otp_new)
    {
        // write enable

        mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);


        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (otp_new << 16);

        mtk_snand_dev_command(cmd, 3);
    }
}

void mtk_snand_dev_turn_off_bbi()
{
    u32 cmd;
    u8  reg;
    u8  reg_new;

    // read original block lock settings
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    reg = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    reg_new = reg & ~SNAND_OTP_BBI;

    if (reg != reg_new)
    {
        // write enable
        mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (reg_new << 16);
        mtk_snand_dev_command(cmd, 3);
    }
}

void mtk_snand_dev_unlock_all_blocks()
{
    u32 cmd;
    u8  lock;
    u8  lock_new;

    // read original block lock settings
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_BLOCK_LOCK << 8);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    lock = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

    lock_new = lock & ~SNAND_BLOCK_LOCK_BITS;

    if (lock != lock_new)
    {
        // write enable
        mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

        // set features
        cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_BLOCK_LOCK << 8) | (lock_new << 16);
        mtk_snand_dev_command(cmd, 3);
    }
}

/*******************************************************************************
 * GPIO(PinMux) register definition
 *******************************************************************************/
#define RW_GPIO_MODE6_SNAND         ((P_U32)(GPIO_BASE+0x0360)) // SFCK
#define RW_GPIO_MODE7_SNAND         ((P_U32)(GPIO_BASE+0x0370)) // SFWP, SFOUT, SFHOLD, SFIN, SFCS1
#define RW_GPIO_DRV0                ((P_U32)(0x1020B060))

void mtk_snand_gpio_init(void)
{
    u32 reg_value;

    // Switch SFIN/SFHOLD/SFWP
    reg_value = *RW_GPIO_MODE7_SNAND;
    reg_value &= ~(((0x0F)<<4)|
                   ((0x0F)<<8)|
                   ((0x0F)<<16));

    *RW_GPIO_MODE7_SNAND = reg_value |
                        ((0x03)<<4)|
                        ((0x03)<<8)|
                        ((0x03)<<16);

    // Switch SCLK
    reg_value = *RW_GPIO_MODE6_SNAND;
    reg_value &= ~(((0x0F)<<28));
    *RW_GPIO_MODE6_SNAND = reg_value |
                      ((0x03)<<28);
    // Switch SFOUT
    reg_value = *RW_GPIO_MODE7_SNAND;
    reg_value &= ~(((0x0F)<<12));
    *RW_GPIO_MODE7_SNAND = reg_value |
                        ((0x03)<<12);

    // Swith SFCS
    reg_value = *RW_GPIO_MODE7_SNAND;
    reg_value &= 0xFFFFFFF0;
    *RW_GPIO_MODE7_SNAND = reg_value |
                      ((0x03)<<0) ;

    // Disable Pull up
    *(volatile unsigned int *) 0x1020B040 &= ~(0x000001F);

    // pad macro
    DRV_WriteReg32(RW_SNAND_CNFG, 1);
}
static int mtk_snand_init(void)
{
    int i, j, busw;
     u8 id[SNAND_MAX_ID];
    u16 spare_bit = 0;

    u16 spare_per_sector = 16;
    u32 ecc_bit = 4;

    // Config pin mux for NAND device
    mtk_snand_gpio_init();

    nfi_buf = (unsigned char *)NAND_NFI_BUFFER;

    memset(&devinfo, 0, sizeof(devinfo));

    /* Dynamic Control */
    g_bInitDone = FALSE;
    g_kCMD.u4OOBRowAddr = (u32) - 1;

#if defined(CONFIG_EARLY_LINUX_PORTING)		// FPGA NAND is placed at CS1
    DRV_WriteReg16(NFI_CSEL_REG16, 1);
#else
    DRV_WriteReg16(NFI_CSEL_REG16, NFI_DEFAULT_CS);
#endif

    /* Set default NFI access timing control */
    //DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    mtk_snand_reset_con();

    g_nand_chip.nand_ecc_mode = NAND_ECC_HW;

    mtk_snand_command_bp(NAND_CMD_READID);

    for(i = 0; i < SNAND_MAX_ID; i++)
    {
		id[i] = mtk_snand_read_byte();
	}

	nand_maf_id = id[0];
    nand_dev_id = id[1];
    memset(&devinfo, 0, sizeof(devinfo));

    if (!mtk_snand_get_device_info(id, &devinfo))
    {
        MSG(INIT, "NAND unsupport\n");
        ASSERT(0);
    }

    g_nand_chip.name = devinfo.devicename;
    g_nand_chip.chipsize = devinfo.totalsize << 20;
    g_nand_chip.page_size = devinfo.pagesize;
    g_nand_chip.page_shift = uffs(g_nand_chip.page_size) - 1;
    g_nand_chip.oobblock = g_nand_chip.page_size;
    g_nand_chip.erasesize = devinfo.blocksize << 10;
    g_nand_chip.bus16 = 4;

    // configure SNF timing
    *RW_SNAND_DLY_CTL1 = devinfo.SNF_DLY_CTL1;
    *RW_SNAND_DLY_CTL2 = devinfo.SNF_DLY_CTL2;
    *RW_SNAND_DLY_CTL3 = devinfo.SNF_DLY_CTL3;
    *RW_SNAND_DLY_CTL4 = devinfo.SNF_DLY_CTL4;
    *RW_SNAND_MISC_CTL = devinfo.SNF_MISC_CTL;

    // set inverse clk & latch latency
    *RW_SNAND_MISC_CTL &= ~SNAND_CLK_INVERSE;       // disable inverse clock and 1 T delay
    *RW_SNAND_MISC_CTL &= ~SNAND_LATCH_LAT_MASK;    // set latency to 0T delay
    *RW_SNAND_MISC_CTL |= SNAND_SAMPLE_CLK_INVERSE; // enable sample clock inverse
    *RW_SNAND_MISC_CTL |= SNAND_4FIFO_EN;

    // configure driving
    *RW_GPIO_DRV0 = *RW_GPIO_DRV0 & ~(0x0007);          // Clear Driving
    *RW_GPIO_DRV0 = *RW_GPIO_DRV0 | devinfo.SNF_DRIVING;

    #ifndef CFG_SNAND_STT   // STT load does not raise SNF speed here

    // raise SPINFI to 104 MHz
    *((P_U32)(0x10000000)) = (*((P_U32)(0x10000000)) & 0xBFFE3FFF) | 0x40010000;    // (preloader does not have such API, thus modify register directly)

    #endif

    g_nand_chip.oobsize = devinfo.sparesize;

    spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

    if (spare_per_sector >= 28)
    {
        spare_bit = PAGEFMT_SPARE_28;
        ecc_bit = 12;
        spare_per_sector = 28;
    } else if (spare_per_sector >= 27)
    {
        spare_bit = PAGEFMT_SPARE_27;
        ecc_bit = 8;
        spare_per_sector = 27;
    } else if (spare_per_sector >= 26)
    {
        spare_bit = PAGEFMT_SPARE_26;
        ecc_bit = 8;
        spare_per_sector = 26;
    } else if (spare_per_sector >= 16)
    {
        spare_bit = PAGEFMT_SPARE_16;
        ecc_bit = 4;
        spare_per_sector = 16;
    } else
    {
        MSG(INIT, "[NAND]: NFI not support oobsize: %x\n", spare_per_sector);
        ASSERT(0);
    }

    g_nand_chip.oobsize = spare_per_sector * (g_nand_chip.page_size / NAND_SECTOR_SIZE);
    MSG(INIT, "[NAND]: oobsize: %d\n", g_nand_chip.oobsize);
    g_nand_chip.chipsize -= g_nand_chip.erasesize * (BMT_POOL_SIZE);

    if (g_nand_chip.oobblock == 4096)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
        nand_oob = &nand_oob_128;
    } else if (g_nand_chip.oobblock == 2048)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
        nand_oob = &nand_oob_64;
    } else if (g_nand_chip.oobblock == 512)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_512);
        nand_oob = &nand_oob_16;
    }

    if (g_nand_chip.nand_ecc_mode == NAND_ECC_HW)
    {
        // MSG (INIT, "Use HW ECC\n");
        NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        mtk_snand_ecc_config(ecc_bit);
        mtk_snand_configure_fdm(8);
    }

    /* Initilize interrupt. Clear interrupt, read clear. */
    DRV_Reg16(NFI_INTR_REG16);

    /* Interrupt arise when read data or program data to/from AHB is done. */
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);

    #ifndef CFG_SNAND_STT
    if (!(init_bmt(&g_nand_chip, BMT_POOL_SIZE)))
    {
        MSG(INIT, "Error: init bmt failed, quit!\n");
        ASSERT(0);
        return 0;
    }
    #endif

    g_nand_chip.chipsize -= g_nand_chip.erasesize * (PMT_POOL_SIZE);

	mtk_snand_dev_unlock_all_blocks();

	mtk_snand_dev_turn_off_bbi();

	mtk_snand_dev_ecc_control(FALSE);

    return 0;
}

//-----------------------------------------------------------------------------
static void mtk_snand_stop_read(void)
{
    //------ NFI Part

    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    DRV_WriteReg32(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
    DRV_WriteReg32(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    NFI_CLN_REG32(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);

    if (g_bHwEcc)
    {
        mtk_snand_ecc_decode_end();
    }
}

//-----------------------------------------------------------------------------
static void mtk_snand_stop_write(void)
{
    //------ NFI Part

    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    DRV_WriteReg32(RW_SNAND_STA_CTL1, SNAND_CUSTOM_PROGRAM);
    DRV_WriteReg32(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    NFI_CLN_REG32(RW_SNAND_MISC_CTL, SNAND_PG_LOAD_CUSTOM_EN);

    mtk_snand_dev_enable_spiq(FALSE);

    if (g_bHwEcc)
    {
        mtk_snand_ecc_encode_end();
    }
}

//-----------------------------------------------------------------------------
static bool mtk_snand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;

    while ((dec_mask != DRV_Reg16(ECC_DECDONE_REG16)) && timeout > 0) // all sectors are decoded done!
    {
        timeout--;

        if (timeout == 0)
        {
            MSG(ERR, "ECC_DECDONE: timeout\n");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
static bool mtk_snand_read_page_data(u32 * buf, u32 num_sec)
{
    u32 timeout = 0xFFFF;
    u32 u4Size;
    u32 i;
    u32 *pBuf32;

    u4Size = num_sec * NAND_SECTOR_SIZE;

#if (USE_AHB_MODE)

    pBuf32 = (u32 *) buf;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    DRV_Reg16(NFI_INTR_REG16);  // read clear
    DRV_WriteReg16(NFI_INTR_EN_REG16, AHB_DONE_EN);
    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))    // for custom read, wait NFI's INTR_AHB_DONE done to ensure all data are transferred to buffer
    {
        timeout--;

        if (0 == timeout)
        {
            return FALSE;
        }
    }

    timeout = 0xFFFF;

    while ((u4Size >> 9) > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
    {
        timeout--;

        if (0 == timeout)
        {
            return FALSE;
        }
    }

#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);
    pBuf32 = (u32 *) buf;

    for (i = 0; (i < (u4Size >> 2)) && (timeout > 0);)
    {
        if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
        {
            *pBuf32++ = DRV_Reg32(NFI_DATAR_REG32);
            i++;
        } else
        {
            timeout--;
        }
        if (0 == timeout)
        {
            return FALSE;
        }
    }
#endif
    return TRUE;
}

//-----------------------------------------------------------------------------
static bool mtk_snand_write_page_data(u32 * buf)
{
    u32 timeout = 0xFFFF;
    u32 u4Size = g_nand_chip.oobblock;

#if (USE_AHB_MODE)
    u32 *pBuf32;
    pBuf32 = (u32 *) buf;

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    DRV_Reg16(NFI_INTR_REG16);  // read clear
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_CUSTOM_PROG_DONE_INTR_EN);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);

    while (!(DRV_Reg32(RW_SNAND_STA_CTL1) & SNAND_CUSTOM_PROGRAM))  // for custom program, wait RW_SNAND_STA_CTL1's SNAND_CUSTOM_PROGRAM done to ensure all data are loaded to device buffer
    {
        timeout--;

        if (0 == timeout)
        {
            return FALSE;
        }
    }

#else
    u32 i;
    u32 *pBuf32;
    pBuf32 = (u32 *) buf;

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);

    for (i = 0; (i < (u4Size >> 2)) && (timeout > 0);)
    {
        if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
        {
            DRV_WriteReg32(NFI_DATAW_REG32, *pBuf32++);
            i++;
        } else
        {
            timeout--;
        }

        if (0 == timeout)
        {
            return FALSE;
        }
    }
#endif
    return TRUE;
}

//-----------------------------------------------------------------------------
static void mtk_snand_read_fdm_data(u32 u4SecNum, u8 * spare_buf)
{
    u32 i;
    u32 *pBuf32 = (u32 *) spare_buf;

    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 3));
        *pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 3));
    }
}

//-----------------------------------------------------------------------------
static void mtk_snand_write_fdm_data(u32 u4SecNum, u8 * oob)
{
    u32 i;
    u32 *pBuf32 = (u32 *) oob;

    for (i = 0; i < u4SecNum; ++i)
    {
        DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 3), *pBuf32++);
        DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 3), *pBuf32++);
    }
}

//---------------------------------------------------------------------------
static bool mtk_snand_ready_for_read(u32 row_addr, u32 sec_num, u8 * buf)
{
    u32     colnob = 2;
    bool    bRet = FALSE;
    u32     cmd;
    u32     reg;
    SNAND_Mode mode = SPIQ;

    if (!mtk_snand_reset_con())
    {
        goto cleanup;
    }

    // 1. Read page to cache

    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PAGE_READ, row_addr); // PAGE_READ command + 3-byte address

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 1 + 3);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            //if (SNAND_STATUS_TOO_MANY_ERROR_BITS == (cmd & SNAND_STATUS_ECC_STATUS_MASK) )
            //{
            //    bRet = FALSE;
            //}

            break;
        }
    }

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (row_addr & SNAND_PAGE_READ_ADDRESS_MASK);
    DRV_WriteReg32(RW_SNAND_RD_CTL1, reg);

    // set DATA READ dummy cycle and command (use default value, ignored)
    if (mode == SPI)
    {
        reg = DRV_Reg32(RW_SNAND_RD_CTL2);
        reg &= ~SNAND_DATA_READ_CMD_MASK;
        reg |= SNAND_CMD_RANDOM_READ & SNAND_DATA_READ_CMD_MASK;
        DRV_WriteReg32(RW_SNAND_RD_CTL2, reg);

    }
    else if (mode == SPIQ)
    {
        mtk_snand_dev_enable_spiq(TRUE);

        reg = DRV_Reg32(RW_SNAND_RD_CTL2);
        reg &= ~SNAND_DATA_READ_CMD_MASK;
        reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
        DRV_WriteReg32(RW_SNAND_RD_CTL2, reg);
    }

    // set DATA READ address
    DRV_WriteReg32(RW_SNAND_RD_CTL3, (0 & SNAND_DATA_READ_ADDRESS_MASK));

    // set SNF data length
    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        cmd = sec_num * (NAND_SECTOR_SIZE + (g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE)));
        reg = cmd | (cmd << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);
    }
    else
    {
        reg = (g_nand_chip.page_size + g_nand_chip.oobsize) |
            ((g_nand_chip.page_size + g_nand_chip.oobsize) << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);
    }

    DRV_WriteReg32(RW_SNAND_MISC_CTL2, reg);

    // set SNF timing
    reg = DRV_Reg32(RW_SNAND_MISC_CTL);

    reg |= SNAND_DATARD_CUSTOM_EN;

    if (mode == SPI)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
    }
    else if (mode == SPIQ)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
    }

    DRV_WriteReg32(RW_SNAND_MISC_CTL, reg);

    //------ NFI Part ------

    mtk_snand_reset_con();

    mtk_snand_set_mode(CNFG_OP_CUST);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_SPIDMA_REG32, 0);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    //NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB_BURST_EN);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    DRV_WriteReg32(NFI_STRADDR_REG32, buf);

    if (g_bHwEcc)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_snand_set_autoformat(TRUE);

    if (g_bHwEcc)
    {
        mtk_snand_ecc_decode_start();
    }

    bRet = TRUE;

  cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool mtk_snand_ready_for_write(u32 page_addr, u32 sec_num, u8 * buf)
{
    bool        bRet = FALSE;
    u32         reg;
    u32         len;
    SNAND_Mode  mode = SPIQ;

    if (!mtk_snand_reset_con())
    {
        return FALSE;
    }

    // 1. Write Enable
    mtk_snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

    //------ SNF Part ------

    // set SPI-NAND command
    if (SPI == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD << SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        DRV_WriteReg32(RW_SNAND_PG_CTL1, reg);
    }
    else if (SPIQ == mode)
    {
        reg = SNAND_CMD_WRITE_ENABLE | (SNAND_CMD_PROGRAM_LOAD_X4<< SNAND_PG_LOAD_CMD_OFFSET) | (SNAND_CMD_PROGRAM_EXECUTE << SNAND_PG_EXE_CMD_OFFSET);
        DRV_WriteReg32(RW_SNAND_PG_CTL1, reg);
        mtk_snand_dev_enable_spiq(TRUE);
    }

    // set program load address
    DRV_WriteReg32(RW_SNAND_PG_CTL2, 0 & SNAND_PG_LOAD_ADDR_MASK);  // col_addr = 0

    // set program execution address
    DRV_WriteReg32(RW_SNAND_PG_CTL3, page_addr);

    // set SNF data length
    reg = (g_nand_chip.page_size + g_nand_chip.oobsize) |
          ((g_nand_chip.page_size + g_nand_chip.oobsize) << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);
    DRV_WriteReg32(RW_SNAND_MISC_CTL2, reg);

    // set SNF timing
    reg = DRV_Reg32(RW_SNAND_MISC_CTL);

    reg |= SNAND_PG_LOAD_CUSTOM_EN; // custom mode

    if (SPI == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X1 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg &=~ SNAND_PG_LOAD_X4_EN;
    }
    else if (SPIQ == mode)
    {
        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);
        reg |= SNAND_PG_LOAD_X4_EN;
    }

    DRV_WriteReg32(RW_SNAND_MISC_CTL, reg);

    //------ NFI Part ------

    // reset NFI
    mtk_snand_reset_con();

    mtk_snand_set_mode(CNFG_OP_PRGM);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    //NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB_BURST_EN);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    if (g_bHwEcc)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_snand_set_autoformat(TRUE);

    if (g_bHwEcc)
    {
        mtk_snand_ecc_encode_start();
    }

    if (!mtk_snand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = TRUE;
  cleanup:

    return bRet;
}

//#############################################################################
//# NAND Driver : Page Read
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################
int mtk_nand_read_page_hwecc(unsigned int logical_addr, char *buf)
{
    int i, start, len, offset = 0;
    int block = logical_addr / g_nand_chip.erasesize;
    int page_in_block = PAGE_NUM(logical_addr) % NAND_BLOCK_BLKS;
    int mapped_block;
    u8 *oob = buf + g_nand_chip.page_size;

    mapped_block = get_mapping_block_index(block);

    if (!mtk_nand_read_page_hw(page_in_block + mapped_block * NAND_BLOCK_BLKS, buf, g_nand_spare))  // g_nand_spare
        return FALSE;

    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        memcpy(oob + offset, g_nand_spare + start, len);
        offset += len;
    }

    return true;
}

static u32 mtk_snand_reverse_byte_order(u32 num)
{
   u32 ret = 0;

   ret |= ((num >> 24) & 0x000000FF);
   ret |= ((num >> 8)  & 0x0000FF00);
   ret |= ((num << 8)  & 0x00FF0000);
   ret |= ((num << 24) & 0xFF000000);

   return ret;
}

static u32 mtk_snand_gen_c1a3(const u32 cmd, const u32 address)
{
    return ((mtk_snand_reverse_byte_order(address) & 0xFFFFFF00) | (cmd & 0xFF));
}

static void mtk_snand_dev_enable_spiq(bool enable)
{
    u8   regval;
    u32  cmd;

    // read QE in status register
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    mtk_snand_dev_mac_op(SPI);

    regval = DRV_Reg8(((volatile u8 *)RW_SNAND_GPRAM_DATA + 2));

    if (FALSE == enable)    // disable x4
    {
        if ((regval & SNAND_OTP_QE) == 0)
        {
            return;
        }
        else
        {
            regval = regval & ~SNAND_OTP_QE;
        }
    }
    else    // enable x4
    {
        if ((regval & SNAND_OTP_QE) == 1)
        {
            return;
        }
        else
        {
            regval = regval | SNAND_OTP_QE;
        }
    }

    // if goes here, it means QE needs to be set as new different value

    // write status register
    cmd = SNAND_CMD_SET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8) | (regval << 16);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 3);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 0);

    mtk_snand_dev_mac_op(SPI);
}

static bool mtk_snand_read_page_part2(u32 row_addr, u32 num_sec, u8 * buf)
{
    bool    bRet = true;
    u32     reg;
    SNAND_Mode mode = SPIQ;
    u32     col_part2, i, len;
    u32     spare_per_sector;
    P_U8    buf_part2;
    u32     timeout = 0xFFFF;
    u32     old_dec_mode = 0;

    spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

    for (i = 0; i < 2 ; i++)
    {
        mtk_snand_reset_con();

        if (0 == i)
        {
            col_part2 = (NAND_SECTOR_SIZE + spare_per_sector) * (num_sec - 1);

            buf_part2 = buf;

            len = 2112 - col_part2;
        }
        else
        {
            col_part2 = 2112;

            buf_part2 += len;   // append to first round

            len = (num_sec * (NAND_SECTOR_SIZE + spare_per_sector)) - 2112;
        }

        //------ SNF Part ------

        // set DATA READ address
        DRV_WriteReg32(RW_SNAND_RD_CTL3, (col_part2 & SNAND_DATA_READ_ADDRESS_MASK));

        // set RW_SNAND_MISC_CTL
        reg = DRV_Reg32(RW_SNAND_MISC_CTL);

        reg |= SNAND_DATARD_CUSTOM_EN;

        reg &= ~SNAND_DATA_READ_MODE_MASK;
        reg |= ((SNAND_DATA_READ_MODE_X4 << SNAND_DATA_READ_MODE_OFFSET) & SNAND_DATA_READ_MODE_MASK);

        DRV_WriteReg32(RW_SNAND_MISC_CTL, reg);

        // set SNF data length
        reg = len | (len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET);

        DRV_WriteReg32(RW_SNAND_MISC_CTL2, reg);

        //------ NFI Part ------

        mtk_snand_set_mode(CNFG_OP_CUST);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        mtk_snand_set_autoformat(FALSE);

        DRV_WriteReg16(NFI_CON_REG16, 1 << CON_NFI_SEC_SHIFT);  // fixed to sector number 1

        DRV_WriteReg32(NFI_STRADDR_REG32, buf_part2);

        DRV_WriteReg32(NFI_SPIDMA_REG32, SPIDMA_SEC_EN | (len & SPIDMA_SEC_SIZE_MASK));


        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

        // set dummy command to trigger NFI enter custom mode
        DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

        DRV_Reg16(NFI_INTR_REG16);  // read clear
        DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

        NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

        timeout = 0xFFFF;

        while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))    // for custom read, wait NFI's INTR_AHB_DONE done to ensure all data are transferred to buffer
        {
            timeout--;

            if (0 == timeout)
            {
                //MSG(INIT, "[snand][mtk_snand_read_page_part2] i:%d NFI_INTR_REG16 timeout!\n", i);
                return FALSE;
            }
        }

        timeout = 0xFFFF;

        while (((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) != 1)
        {
            timeout--;

            if (0 == timeout)
            {
                //MSG(INIT, "[snand][mtk_snand_read_page_part2] i:%d NFI_BYTELEN_REG16 timeout!\n", i);
                return FALSE;
            }
        }

        //------ NFI Part

        NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);

        //------ SNF Part

        // set 1 then set 0 to clear done flag
        DRV_WriteReg32(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
        DRV_WriteReg32(RW_SNAND_STA_CTL1, 0);

        // clear essential SNF setting
        NFI_CLN_REG32(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);
    }

    /* configure ECC decoder && encoder */
    reg = DRV_Reg32(ECC_DECCNFG_REG32);
    old_dec_mode = reg & DEC_CNFG_DEC_MODE_MASK;
    reg &= ~DEC_CNFG_DEC_MODE_MASK;
    reg |= DEC_CNFG_AHB;
    DRV_WriteReg32(ECC_DECCNFG_REG32, reg);

    DRV_WriteReg32(ECC_DECDIADDR_REG32, (u32)buf);

    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);

    while(!((DRV_Reg32(ECC_DECDONE_REG16)) & (1 << 0)));

    reg = DRV_Reg32(ECC_DECENUM0_REG32);

    if (0 != reg)
    {
        reg &= 0x1F;

        if (0x1F == reg)
        {
            bRet = false;   // ECC-U
        }
    }

    // restore essential NFI and ECC registers
    DRV_WriteReg32(NFI_SPIDMA_REG32, 0);
    reg = DRV_Reg32(ECC_DECCNFG_REG32);
    reg &= ~DEC_CNFG_DEC_MODE_MASK;
    reg |= old_dec_mode;
    DRV_WriteReg32(ECC_DECCNFG_REG32, reg);
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
    DRV_WriteReg32(ECC_DECDIADDR_REG32, 0);

  cleanup:
    return bRet;
}


int mtk_nand_read_page_hw(u32 page, u8 * dat, u8 * oob)
{
    bool bRet = TRUE;
    u8 *pPageBuf;
    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    u32 i;
    pPageBuf = (u8 *) dat;

    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        u4SecNum--;
    }

    if (mtk_snand_ready_for_read(page, u4SecNum, pPageBuf))
    {
        if (!mtk_snand_read_page_data((u32 *) pPageBuf, u4SecNum))
        {
            bRet = FALSE;
        }

        if (!mtk_snand_status_ready(STA_NAND_BUSY))
        {
            bRet = FALSE;
        }

        if (g_bHwEcc)
        {
            if (!mtk_snand_check_dececc_done(u4SecNum))
            {
                bRet = FALSE;
            }
        }

        mtk_snand_read_fdm_data(u4SecNum, oob);

        if (g_bHwEcc)
        {
            if (!mtk_snand_check_bch_error(pPageBuf, u4SecNum - 1, page))
            {
                MSG(ERASE, "check bch error !\n");
                bRet = FALSE;
            }
        }

        mtk_snand_stop_read();
    }

    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        g_snand_temp = (unsigned char *)(NAND_NFI_BUFFER + NAND_NFI_BUFFER_SIZE - (MAX_MAIN_SIZE + MAX_SPAR_SIZE));

        // read part II

        u4SecNum++;

        // note: use local temp buffer to read part 2
        mtk_snand_read_page_part2(page, u4SecNum, g_snand_temp);

        // g_snand_temp now is formatted as PAGE_DATA | FDM DATA

        // copy data

        pPageBuf = dat + NAND_SECTOR_SIZE * (u4SecNum - 1);

        for (i = 0; i < NAND_SECTOR_SIZE / sizeof(u32); i++)
        {
            ((u32 *)pPageBuf)[i] = ((u32 *)g_snand_temp)[i];
        }

        // copy FDM data

        pPageBuf = oob + NAND_FDM_PER_SECTOR * (u4SecNum - 1);

        for (i = 0; i < NAND_FDM_PER_SECTOR / sizeof(u32); i++)
        {
            ((u32 *)pPageBuf)[i] = ((u32 *)g_snand_temp)[i + (NAND_SECTOR_SIZE / sizeof(u32))];
        }
    }

    mtk_snand_dev_enable_spiq(FALSE);

    return bRet;
}

//#############################################################################
//# NAND Driver : Page Write
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################

int mtk_nand_write_page_hwecc(unsigned int logical_addr, char *buf)
{
    u16 block = logical_addr / g_nand_chip.erasesize;
    u16 mapped_block = get_mapping_block_index(block);
    u16 page_in_block = PAGE_NUM(logical_addr) % NAND_BLOCK_BLKS;
    u8 *oob = buf + g_nand_chip.oobblock;
    int i;
    int start, len, offset;

    for (i = 0; i < sizeof(g_nand_spare); i++)
        *(g_nand_spare + i) = 0xFF;

    offset = 0;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        memcpy((g_nand_spare + start), (oob + offset), len);
        offset += len;
    }

    // write bad index into oob
    if (mapped_block != block)
    {
        set_bad_index_to_oob(g_nand_spare, block);
    } else
    {
        set_bad_index_to_oob(g_nand_spare, FAKE_INDEX);
    }

    if (!mtk_nand_write_page_hw(page_in_block + mapped_block * NAND_BLOCK_BLKS, buf, g_nand_spare))
    {
        MSG(INIT, "write fail happened @ block 0x%x, page 0x%x\n", mapped_block, page_in_block);
        return update_bmt((page_in_block + mapped_block * NAND_BLOCK_BLKS) * g_nand_chip.oobblock, UPDATE_WRITE_FAIL, buf, g_nand_spare);
    }

    return TRUE;
}

static bool mtk_snand_dev_program_execute(u32 page)
{
    u32 cmd;
    bool bRet = TRUE;

    // 3. Program Execute
    cmd = mtk_snand_gen_c1a3(SNAND_CMD_PROGRAM_EXECUTE, page);

    mtk_snand_dev_command(cmd, 4);

    // 4. Status Polling
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    while (1)
    {
        mtk_snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_PROGRAM_FAIL) != 0)
        {
            bRet = FALSE;
        }

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            break;
        }
    }

    return bRet;
}

bool mtk_snand_is_vendor_reserved_blocks(u32 row_addr)
{
    u32 page_per_block = g_nand_chip.erasesize / g_nand_chip.page_size;
    u32 target_block = row_addr / page_per_block;

    if (devinfo.advancedmode & SNAND_ADV_VENDOR_RESERVED_BLOCKS)
    {
        if (target_block >= 2045 && target_block <= 2048)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int mtk_nand_write_page_hw(u32 page, u8 * dat, u8 * oob)
{
    bool bRet = TRUE;
    u32 pagesz = g_nand_chip.oobblock;
    u32 u4SecNum = pagesz >> NAND_PAGE_SHIFT;
    u32 cmd;
    int i, j, start, len;
    bool empty = TRUE;
    u8 oob_checksum = 0;

    if (TRUE == mtk_snand_is_vendor_reserved_blocks(page))
    {
        return FALSE;
    }

    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && nand_oob->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = nand_oob->oobfree[i].offset;
        len = nand_oob->oobfree[i].length;
        for (j = 0; j < len; j++)
        {
            oob_checksum ^= oob[start + j];
            if (oob[start + j] != 0xFF)
                empty = FALSE;
        }
    }

    if (!empty)
    {
        oob[nand_oob->oobfree[i - 1].offset + nand_oob->oobfree[i - 1].length] = oob_checksum;
    }

    if (mtk_snand_ready_for_write(page, u4SecNum, dat))
    {
        mtk_snand_write_fdm_data(u4SecNum, oob);

        if (!mtk_snand_write_page_data((u32 *) dat))
        {
            bRet = FALSE;
        }

        if (!mtk_snand_check_RW_count(g_nand_chip.oobblock))
        {
            bRet = FALSE;
        }

        mtk_snand_stop_write();

        return mtk_snand_dev_program_execute(page);
    }
    else
    {
        return FALSE;
    }

    return bRet;
}

unsigned int nand_block_bad(unsigned int logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (nand_block_bad_hw(mapped_block * g_nand_chip.erasesize))
    {
        if (update_bmt(mapped_block * g_nand_chip.erasesize, UPDATE_UNMAPPED_BLOCK, NULL, NULL))
        {
            return logical_addr;    // return logical address
        }
        return logical_addr + g_nand_chip.erasesize;
    }

    return logical_addr;
}

bool nand_block_bad_hw(u32 logical_addr)
{
    bool bRet = FALSE;
    u32 page = logical_addr / g_nand_chip.oobblock;

    int i, page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);
    u8 * pspare;
    char *tmp = (char *)nfi_buf;
    u8 * pbuf8;
    memset(tmp, 0x0, g_nand_chip.oobblock + g_nand_chip.oobsize);

    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;

    page &= ~(page_num - 1);

    if (TRUE == mtk_snand_is_vendor_reserved_blocks(page))
    {
        return TRUE;   // return bad block for reserved blocks
    }

    pspare = g_nand_spare;

    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        u4SecNum--;
    }

    if (mtk_snand_ready_for_read(page, u4SecNum, tmp))  // read a whole page anyway (easier deriver implementation)
    {
        if (!mtk_snand_read_page_data((u32 *) tmp, u4SecNum))
        {
            bRet = FALSE;
        }

        if (!mtk_snand_status_ready(STA_NAND_BUSY))
        {
            bRet = FALSE;
        }

        if (!mtk_snand_check_dececc_done(u4SecNum))
        {
            bRet = FALSE;
        }

        mtk_snand_read_fdm_data(u4SecNum, g_nand_spare);

        if (!mtk_snand_check_bch_error(tmp, u4SecNum - 1, page))
        {
            MSG(ERASE, "check bch error !\n");
            bRet = FALSE;
        }

        mtk_snand_stop_read();
    }

    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        // read part II

        g_snand_temp = (unsigned char *)(NAND_NFI_BUFFER + NAND_NFI_BUFFER_SIZE - (MAX_MAIN_SIZE + MAX_SPAR_SIZE));

        u4SecNum++;

        // note: use local temp buffer to read part 2
        mtk_snand_read_page_part2(page, u4SecNum, g_snand_temp);

        // g_snand_temp now is formatted as PAGE_DATA | FDM DATA

        // copy data

        pbuf8 = tmp + NAND_SECTOR_SIZE * (u4SecNum - 1);

        for (i = 0; i < NAND_SECTOR_SIZE / sizeof(u32); i++)
        {
            ((u32 *)pbuf8)[i] = ((u32 *)g_snand_temp)[i];
        }

        // copy FDM data

        pbuf8 = pspare + NAND_FDM_PER_SECTOR * (u4SecNum - 1);

        for (i = 0; i < NAND_FDM_PER_SECTOR / sizeof(u32); i++)
        {
            ((u32 *)pbuf8)[i] = ((u32 *)g_snand_temp)[i + (NAND_SECTOR_SIZE / sizeof(u32))];
        }
    }

    // check bad block mark

    if (pspare[0] != 0xFF || pspare[8] != 0xFF || pspare[16] != 0xFF || pspare[24] != 0xFF)
    {
        bRet = TRUE;
    }

    return bRet;
}

bool mark_block_bad(u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    return mark_block_bad_hw(mapped_block * g_nand_chip.erasesize);
}

bool mark_block_bad_hw(u32 offset)
{
    bool bRet = FALSE;
    u32 index;
    u32 page_addr = offset / g_nand_chip.oobblock;
    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    unsigned char *pspare;
    int i, page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);
    unsigned char buf[2048];

    for (index = 0; index < 64; index++)
        *(g_nand_spare + index) = 0xFF;

    pspare = g_nand_spare;

    for (index = 8, i = 0; i < 4; i++)
        pspare[i * index] = 0x0;

    page_addr &= ~(page_num - 1);
    MSG(BAD, "Mark bad block at 0x%x\n", page_addr);

    if (TRUE == mtk_snand_is_vendor_reserved_blocks(page_addr))
    {
        return FALSE;
    }

    if (mtk_snand_ready_for_write(page_addr, u4SecNum, buf))
    {
        mtk_snand_write_fdm_data(u4SecNum, g_nand_spare);

        if (!mtk_snand_write_page_data((u32 *) & buf))
        {
            bRet = FALSE;
        }

        if (!mtk_snand_check_RW_count(g_nand_chip.oobblock))
        {
            bRet = FALSE;
        }

        mtk_snand_stop_write();

        bRet = mtk_snand_dev_program_execute(page_addr);
    }
    else
    {
        return FALSE;
    }

    for (index = 0; index < 64; index++)    // FIXME: reset spare area ??
    {
        *(g_nand_spare + index) = 0xFF;
    }

    return bRet;
}

//#############################################################################
//# NAND Driver : Page Write
//#
//# NAND Page Format (Large Page 2KB)
//#  |------ Page:2048 Bytes ----->>||---- Spare:64 Bytes -->>|
//#
//# Parameter Description:
//#     page_addr               : specify the starting page in NAND flash
//#
//#############################################################################
bool mtk_nand_erase_hw(u32 offset)
{
    bool bRet = TRUE;
    u32  row_addr = offset / g_nand_chip.oobblock; // oobblock = page size (bytes)
    u32  reg;
    u32  polling_times;

    if (nand_block_bad_hw(offset))
    {
        return FALSE;
    }

    if (TRUE == mtk_snand_is_vendor_reserved_blocks(row_addr))
    {
        return FALSE;
    }

    mtk_snand_reset_con();

    // erase address
    DRV_WriteReg32(RW_SNAND_ER_CTL2, row_addr);

    // set loop limit and polling cycles
    reg = (SNAND_LOOP_LIMIT_NO_LIMIT << SNAND_LOOP_LIMIT_OFFSET) | 0x20;
    DRV_WriteReg32(RW_SNAND_GF_CTL3, reg);

    // set latch latency & CS de-select latency (ignored)

    // set erase command
    reg = SNAND_CMD_BLOCK_ERASE << SNAND_ER_CMD_OFFSET;
    DRV_WriteReg32(RW_SNAND_ER_CTL, reg);

    // trigger interrupt waiting
    reg = DRV_Reg16(NFI_INTR_EN_REG16);
    reg = INTR_AUTO_BLKER_INTR_EN;
    DRV_WriteReg16(NFI_INTR_EN_REG16, reg);

    // trigger auto erase
    reg = DRV_Reg32(RW_SNAND_ER_CTL);
    reg |= SNAND_AUTO_ERASE_TRIGGER;
    DRV_WriteReg32(RW_SNAND_ER_CTL, reg);

    // wait for auto erase finish
    for (polling_times = 1;;polling_times++)
    {
        reg = DRV_Reg32(RW_SNAND_STA_CTL1);

        if ((reg & SNAND_AUTO_BLKER) == 0)
        {
            reg = DRV_Reg32(RW_SNAND_GF_CTL1);
            reg &= SNAND_GF_STATUS_MASK;

            continue;
        }
        else
        {
            // set 1 then set 0 to clear done flag
            DRV_WriteReg32(RW_SNAND_STA_CTL1, reg);
            reg = reg & ~SNAND_AUTO_BLKER;
            DRV_WriteReg32(RW_SNAND_STA_CTL1, reg);

            // clear trigger bit
            reg = DRV_Reg32(RW_SNAND_ER_CTL);
            reg &= ~SNAND_AUTO_ERASE_TRIGGER;
            DRV_WriteReg32(RW_SNAND_ER_CTL, reg);

            reg = DRV_Reg32(RW_SNAND_GF_CTL1);
            reg &= SNAND_GF_STATUS_MASK;

            break;
        }
    }


    // check get feature status
    reg = *RW_SNAND_GF_CTL1 & SNAND_GF_STATUS_MASK;

    if (0 != (reg & SNAND_STATUS_ERASE_FAIL))
    {
        bRet = FALSE;
    }

    return bRet;
}

int mtk_nand_erase(u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (!mtk_nand_erase_hw(mapped_block * g_nand_chip.erasesize))
    {
        MSG(INIT, "erase block 0x%x failed\n", mapped_block);
        return update_bmt(mapped_block * g_nand_chip.erasesize, UPDATE_ERASE_FAIL, NULL, NULL);
    }

    return TRUE;
}

bool mtk_nand_wait_for_finish(void)
{
    bool    bTimeout;
    u32     reg;

    for (bTimeout = FALSE; bTimeout != TRUE;)
    {
        reg = DRV_Reg16(NFI_MASTERSTA);

        if (0 == (reg & MASTERSTA_MASK))
        {
            break;
        }
    }
}

/**************************************************************************
*  MACRO LIKE FUNCTION
**************************************************************************/
static int mtk_snand_read_block(blkdev_t * bdev, u32 blknr, u32 blks, u8 * buf)
{
    u32 i;
    u32 offset = blknr * bdev->blksz;

    for (i = 0; i < blks; i++)
    {
        offset = nand_read_data(buf, offset);
        offset += bdev->blksz;
        buf += bdev->blksz;
    }
    return 0;
}

static int mtk_snand_write_block(blkdev_t * bdev, u32 blknr, u32 blks, u8 * buf)
{
    u32 i;
    u32 offset = blknr * bdev->blksz;

    for (i = 0; i < blks; i++)
    {
        offset = nand_write_data(buf, offset);
        offset += bdev->blksz;
        buf += bdev->blksz;
    }
    return 0;
}

// ==========================================================
// NAND Common Interface - Init
// ==========================================================

u32 nand_init_device(void)
{
    if (!blkdev_get(BOOTDEV_NAND))
    {
        mtk_snand_reset_descriptor();
        mtk_snand_init();

        PAGE_SIZE = (u32) g_nand_chip.page_size;
        BLOCK_SIZE = (u32) g_nand_chip.erasesize;

        memset(&g_nand_bdev, 0, sizeof(blkdev_t));
        g_nand_bdev.blksz = g_nand_chip.page_size;
        g_nand_bdev.erasesz = g_nand_chip.erasesize;
        g_nand_bdev.blks = g_nand_chip.chipsize;
        g_nand_bdev.bread = mtk_snand_read_block;
        g_nand_bdev.bwrite = mtk_snand_write_block;
        g_nand_bdev.blkbuf = (u8 *) STORAGE_BUFFER_ADDR;
        g_nand_bdev.type = BOOTDEV_NAND;
        blkdev_register(&g_nand_bdev);
    }

    #ifdef CFG_SNAND_STT
	stt_snand_main();   // STT load only do STT engine, and stop after STT engine is finished
	#endif

    return 0;
}

void Invert_Bits(u8 * buff_ptr, u32 bit_pos)
{
    u32 byte_pos = 0;
    u8 byte_val = 0;
    u8 temp_val = 0;
    u32 invert_bit = 0;

    byte_pos = bit_pos >> 3;
    invert_bit = bit_pos & ((1 << 3) - 1);
    byte_val = buff_ptr[byte_pos];
    temp_val = byte_val & (1 << invert_bit);

    if (temp_val > 0)
        byte_val &= ~temp_val;
    else
        byte_val |= (1 << invert_bit);
    buff_ptr[byte_pos] = byte_val;
}

void compare_page(u8 * testbuff, u8 * sourcebuff, u32 length, char *s)
{
    u32 errnum = 0;
    u32 ii = 0;
    u32 index;
    printf("%s", s);
    for (index = 0; index < length; index++)
    {
        if (testbuff[index] != sourcebuff[index])
        {
            u8 t = sourcebuff[index] ^ testbuff[index];
            for (ii = 0; ii < 8; ii++)
            {
                if ((t >> ii) & 0x1 == 1)
                {
                    errnum++;
                }
            }
            printf(" ([%d]=%x) != ([%d]=%x )", index, sourcebuff[index], index, testbuff[index]);
        }

    }
    if (errnum > 0)
    {
        printf(": page have %d mismatch bits\n", errnum);
    } else
    {
        printf(" :the two buffers are same!\n");
    }
}

u8 empty_page(u8 * sourcebuff, u32 length)
{
    u32 index = 0;
    for (index = 0; index < length; index++)
    {
        if (sourcebuff[index] != 0xFF)
        {
            return 0;
        }
    }
    return 1;
}

u32 __nand_ecc_test(u32 offset, u32 max_ecc_capable)
{

    int ecc_level = max_ecc_capable;
    int sec_num = g_nand_chip.page_size >> 9;
    u32 sec_size = g_nand_chip.page_size / sec_num;
    u32 NAND_MAX_PAGE_LENGTH = g_nand_chip.page_size + 8 * sec_num;
    u32 chk_bit_len = 64 * 4;
    u32 page_per_blk = g_nand_chip.erasesize / g_nand_chip.page_size;
    u32 sec_index, curr_error_bit, err_bits_per_sec, page_idx, errbits, err;

    u8 *testbuff = malloc(NAND_MAX_PAGE_LENGTH);
    u8 *sourcebuff = malloc(NAND_MAX_PAGE_LENGTH);
    u8 empty;

    for (err_bits_per_sec = 1; err_bits_per_sec <= ecc_level; err_bits_per_sec++)
    {
        printf("~~~start test ecc correct in ");
#if USE_AHB_MODE
        printf(" AHB mode");
#else
        printf(" MCU mode");
#endif
        printf(", every sector have %d bit error~~~\n", err_bits_per_sec);
        for (curr_error_bit = 0; curr_error_bit < chk_bit_len && offset < g_nand_chip.chipsize; offset += g_nand_chip.page_size)
        {
            memset(testbuff, 0x0a, NAND_MAX_PAGE_LENGTH);
            memset(sourcebuff, 0x0b, NAND_MAX_PAGE_LENGTH);
            g_bHwEcc = TRUE;
            nand_read_data(sourcebuff, offset);
            empty = empty_page(sourcebuff, g_nand_chip.page_size);
            if (empty)
            {
                printf("page %d is empty\n", offset / g_nand_chip.page_size);
                memset(sourcebuff, 0x0c, NAND_MAX_PAGE_LENGTH);
                nand_write_data(sourcebuff, offset);
                nand_read_data(sourcebuff, offset);
            }
            if (0 != (DRV_Reg32(ECC_DECENUM0_REG32) & 0xFFFFF) ||0 != (DRV_Reg32(ECC_DECENUM1_REG32) & 0xFFFFF) )
            {
                printf("skip the page %d, because it is empty ( %d )or already have error bits (%x)!\n", offset / g_nand_chip.page_size, empty, err);
            } else
            {
                printf("~~~start test ecc correct in Page 0x%x ~~~\n", offset / g_nand_chip.page_size);
                memcpy(testbuff, sourcebuff, NAND_MAX_PAGE_LENGTH);
                for (sec_index = 0; sec_index < sec_num; sec_index++)
                {
                    //printf("insert err bit @ page %d:sector %d : bit ",page_idx+offset/g_nand_chip.page_size,sec_index);
                    for (errbits = 0; errbits < err_bits_per_sec; errbits++)
                    {
                        Invert_Bits(((u8 *) testbuff) + sec_index * sec_size, curr_error_bit);
                        //printf("%d, ",curr_error_bit);
                        curr_error_bit++;
                    }
                    //printf("\n");
                }
                g_bHwEcc = FALSE;
                nand_write_data(testbuff, offset);
                compare_page(testbuff, sourcebuff, NAND_MAX_PAGE_LENGTH, "source and test buff check ");
                g_bHwEcc = TRUE;
                nand_read_data(testbuff, offset);
                compare_page(testbuff, sourcebuff, NAND_MAX_PAGE_LENGTH, "read back check ");
            }
        }
    }

    free(testbuff);
    free(sourcebuff);

}

u32 nand_ecc_test(void)
{
    part_t *part = part_get(PART_UBOOT);
    u32 offset = (part->startblk) * g_nand_chip.page_size;
    __nand_ecc_test(offset, 4);

    part_t *part2 = part_get(PART_BOOTIMG);
    offset = (part2->startblk) * g_nand_chip.page_size;
    __nand_ecc_test(offset, 4);
    return 0;
}

u32 nand_get_device_id(u8 * id, u32 len)
{
    u8 buf[16];

    // Config pin mux for NAND device, since EMI init will be called before NAND init done/
    mtk_snand_gpio_init();

    if (TRUE != mtk_snand_get_flash_id(buf, len))
        return -1;

    len = len > 16 ? 16 : len;

    memcpy(id, buf, len);

    return 0;
}

/* LEGACY - TO BE REMOVED { */
// ==========================================================
// NAND Common Interface - Correct R/W Address
// ==========================================================
u32 nand_find_safe_block(u32 offset)
{

    u32 original_offset = offset;
    u32 new_offset = 0;
    unsigned int blk_index = 0;
    static BOOL Bad_Block_Table_init = FALSE;

    if (Bad_Block_Table_init == FALSE)
    {
        Bad_Block_Table_init = TRUE;
        memset(Bad_Block_Table, 0, sizeof(Bad_Block_Table));
        print("Bad_Block_Table init, sizeof(Bad_Block_Table)= %d \n", sizeof(Bad_Block_Table));
    }

    blk_index = BLOCK_ALIGN(offset) / BLOCK_SIZE;
    if (Bad_Block_Table[blk_index] == 1)
    {
        return offset;
    }
    // new_offset is block alignment
    new_offset = nand_block_bad(BLOCK_ALIGN(offset));

    // find next block until the block is good
    while (new_offset != BLOCK_ALIGN(offset))
    {
        offset = new_offset;
        new_offset = nand_block_bad(BLOCK_ALIGN(offset));
    }

    if (original_offset != offset)
    {
        Bad_Block_Table[(original_offset / BLOCK_SIZE)] = 2;
        print("offset (0x%x) is bad block. next safe block is (0x%x)\n", original_offset, offset);
    }

    Bad_Block_Table[(BLOCK_ALIGN(offset) / BLOCK_SIZE)] = 1;

    return offset;
}

/* LEGACY - TO BE REMOVED } */

// ==========================================================
// NAND Common Interface - Read Function
// ==========================================================
u32 nand_read_data(u8 * buf, u32 offset)
{

    // make sure the block is safe to flash
    offset = nand_find_safe_block(offset);

    if (mtk_nand_read_page_hwecc(offset, buf) == FALSE)
    {
        print("nand_read_data fail\n");
        return -1;
    }

    return offset;
}

// ==========================================================
// NAND Common Interface - Write Function
// ==========================================================
u32 nand_write_data(u8 * buf, u32 offset)
{
    // make sure the block is safe to flash
    offset = nand_find_safe_block(offset);

    if (mtk_nand_write_page_hwecc(offset, buf) == FALSE)
    {
        print("nand_write_data fail\n");
        ASSERT(0);
    }

    return offset;
}

// ==========================================================
// NAND Common Interface - Erase Function
// ==========================================================
bool nand_erase_data(u32 offset, u32 offset_limit, u32 size)
{

    u32 img_size = size;
    u32 tpgsz;
    u32 tblksz;
    u32 cur_offset;
    u32 i = 0;

    // do block alignment check
    if (offset % BLOCK_SIZE != 0)
    {
        print("offset must be block alignment (0x%x)\n", BLOCK_SIZE);
        ASSERT(0);
    }
    // calculate block number of this image
    if ((img_size % BLOCK_SIZE) == 0)
    {
        tblksz = img_size / BLOCK_SIZE;
    } else
    {
        tblksz = (img_size / BLOCK_SIZE) + 1;
    }

    print("[ERASE] image size = 0x%x\n", img_size);
    print("[ERASE] the number of nand block of this image = %d\n", tblksz);

    // erase nand block
    cur_offset = offset;
    while (tblksz != 0)
    {
        if (mtk_nand_erase(cur_offset) == FALSE)
        {
            print("[ERASE] erase fail\n");
            mark_block_bad(cur_offset);
            //ASSERT (0);
        }
        cur_offset += BLOCK_SIZE;

        tblksz--;

        if (tblksz != 0 && cur_offset >= offset_limit)
        {
            print("[ERASE] cur offset (0x%x) exceeds erase limit address (0x%x)\n", cur_offset, offset_limit);
            return TRUE;
        }
    }

    return TRUE;
}

#if CFG_LEGACY_USB_DOWNLOAD

// ==========================================================
// NAND Common Interface - Check If Device Is Ready To Use
// ==========================================================
void nand_wait_ready(void)
{
    // wait for NAND flashing complete
    mtk_nand_wait_for_finish();
}

// ==========================================================
// NAND Common Interface - Checksum Calculation Body (skip HW ECC area)
// ==========================================================
u32 nand_chksum_body(u32 chksm, char *buf, u32 pktsz)
{
    u32 i, spare_start, spare_offset = 0, spare_len;
    struct nand_oobfree *oobfree = &nand_oob->oobfree[0];

    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && oobfree->length; i++, oobfree++)
    {
        /* Set the reserved bytes to 0xff */
        spare_start = oobfree->offset;
        spare_len = oobfree->length;
        memcpy((buf + g_nand_chip.page_size + spare_offset), (g_nand_spare + spare_start), spare_len);
        spare_offset += spare_len;
    }

    /* checksum algorithm core, simply exclusive or */
    // skip spare because FAT format image doesn't have any spare region
    for (i = 0; i < pktsz - g_nand_chip.oobsize; i++)
    {
        chksm ^= buf[i];
    }

    return chksm;
}

// ==========================================================
// NAND Common Interface - Checksum Calculation
// ==========================================================
u32 nand_chksum_per_file(u32 nand_offset, u32 img_size)
{
    u32 now = 0, i = 0, chksm = 0, start_block = 0, total = 0;
    INT32 cnt;
    bool ret = TRUE;

    u32 start = nand_offset;
    u32 pagesz = g_nand_chip.page_size;
    u32 pktsz = pagesz + g_nand_chip.oobsize;
    u8 *buf = (u8 *) STORAGE_BUFFER_ADDR;

    // clean the buffer
    memset(buf, 0x0, pktsz);

    // calculate the number of page
    total = img_size / pagesz;
    if (img_size % pagesz != 0)
    {
        total++;
    }
    // check the starting block is safe
    start_block = nand_find_safe_block(start);
    if (start_block != start)
    {
        start = start_block;
    }
    // copy data from NAND to MEM
    for (cnt = total, now = start; cnt >= 0; cnt--, now += pagesz)
    {

        // when the address is block alignment, check if this block is good
        if (now % BLOCK_SIZE == 0)
        {
            now = nand_find_safe_block(now);
        }

        /* read a packet */
        nand_read_data(buf, now);

        /* cal chksm */
        chksm = nand_chksum_body(chksm, buf, pktsz);

    }

    return chksm;
}
#endif

//----------------------------------------------
// SPI-NAND HQA STT
//----------------------------------------------
#ifdef CFG_SNAND_STT

#include "mtk_wdt.h"

#define SNAND_STT_PATTERN_UNIT_SIZE     (256)
#define SNAND_STT_PATTERN_TOTAL_SIZE    (0x4000)
#define SNAND_STT_SEQ_READ_LEN          (0x4000) // 16 KB

u32     g_stt_snand_clock = 104;    // 72 use 104 MHz to do STT test
u32     g_stt_snand_inv_clk = 5;        // : 5: Inverse clock  0: 0T delay
u32     g_stt_snand_golden_block;
u8    * g_stt_snand_buf_read;
u8    * g_stt_snand_buf_spare;
u8    * g_stt_snand_buf_golden_pattern;
u8      g_stt_debug = 1;

u8 driving_tbl[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
                    //   4        8      12       16      20       24       28       32 mA
u8 g_snand_io_input_dly_tbl[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
};

u8 g_snand_sam_clk_dly_tbl[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,  0x2F ,0x2F ,0x2F
};

bool stt_snand_read_bulk_data_test(u32 block, s32 read_round)
{
    u32 data_err = 0;
    s32 i, j, k;
    u8  rdata, gdata;

    for (; read_round > 0; read_round--)
    {
        for (i = 0; i < SNAND_STT_SEQ_READ_LEN / g_nand_chip.oobblock; i++)   // read 2 KB in a loop, totally 16 KB
        {
            memset(g_stt_snand_buf_read, 0xCC, g_nand_chip.oobblock);
            memset(g_stt_snand_buf_spare, 0xDD, g_nand_chip.oobsize);

            mtk_nand_read_page_hw(i + g_stt_snand_golden_block * (g_nand_chip.erasesize / g_nand_chip.oobblock), g_stt_snand_buf_read, g_stt_snand_buf_spare);

            for (j = 0; j < g_nand_chip.oobblock; j += SNAND_STT_PATTERN_UNIT_SIZE)  // compare 256 bytes in a one time
            {
                for (k = 0; k < SNAND_STT_PATTERN_UNIT_SIZE; k += 1)
                {
                    // get read data
                    rdata = g_stt_snand_buf_read[j + k];

                    // get golden data
                    gdata = g_stt_snand_buf_golden_pattern[k];

                    if (rdata != gdata)
                    {
                        data_err = 1;
                        break;
                    }
                }

                if (1 == data_err)
                {
                    break;
                }
            }

            if (1 == data_err)
            {
                break;
            }
        }

        if (1 == data_err)
        {
            break;
        }
    }

    if (1 == data_err)
    {
        return false;
    }
    else
    {
        return true;
    }

}

void stt_snand_test_engine(void)
{
    s32 i, j, k, m, tmp_val;
    s32 test_round =1;
    u16 driving;

    static u8 result1[48][48], result2[48][48];

    if (104 == g_stt_snand_clock)
    {
        *((P_U32)(0x10000000)) = (*((P_U32)(0x10000000)) & 0xBFFE3FFF) | 0x40010000;    // (preloader does not have such API, thus modify register directly)
    }

    printf("\n");

    for(j = 0 ; j < 48; j++)
    {
        for(m = 0; m <48; m++)
        {
            result1[j][m] = 0x0;
            result2[j][m] = 0x0;
        }
    }

    #if !defined(__SNAND_STT_DRIVING_FIXED__)
    for(i = 1 ; i < 8; i++)
    #endif
    {
        #if defined(__SNAND_STT_DRIVING_FIXED__)
        i = 1;
        #endif

        // set device driving
        driving = *RW_GPIO_DRV0 & ~(0x0007); // Clear Driving
        *RW_GPIO_DRV0 = driving |(driving_tbl[i] << 0);

        printf("\nSet driving to %x, testing ...\n", driving_tbl[i]);

        // set inverse clk & latch latency
        *RW_SNAND_MISC_CTL &= ~SNAND_CLK_INVERSE;       // disable inverse clock and 1 T delay
        *RW_SNAND_MISC_CTL &= ~SNAND_LATCH_LAT_MASK;    // set latency to 0T delay

        if (g_stt_snand_inv_clk == 5)
        {
           *RW_SNAND_MISC_CTL |= SNAND_SAMPLE_CLK_INVERSE; // enable sample clock inverse
        }

        *RW_SNAND_MISC_CTL |= SNAND_4FIFO_EN;

        // reset delay

        // 1. sample delay
        *RW_SNAND_DLY_CTL3 &= ~(SNAND_SFCK_SAM_DLY_MASK | SNAND_SFIFO_WR_EN_DLY_SEL_MASK); // set sample CLK delay / SFIFO WR EN delay to 0;

        // 2. IO input delay
        *RW_SNAND_DLY_CTL2 &= ~(SNAND_SFIO0_IN_DLY_MASK | SNAND_SFIO1_IN_DLY_MASK | SNAND_SFIO2_IN_DLY_MASK | SNAND_SFIO3_IN_DLY_MASK); // set input INPUT delay to 0;

        for(k = 0 ; k < 10; k++);       // delay awhile

        // OK! it's time to do STT table!

        for (j = 0; j < 48; j++)  // j => switch IO input delay
        {
            *RW_SNAND_DLY_CTL2 = g_snand_io_input_dly_tbl[j] | (g_snand_io_input_dly_tbl[j] << 8) | (g_snand_io_input_dly_tbl[j] << 16) | (g_snand_io_input_dly_tbl[j] << 24);

            for (k = 0 ; k < 10; k++);   // delay awhile

            for (m = 0; m < 48; m++) // m => switch sample clock delay
            {
                tmp_val = *RW_SNAND_DLY_CTL3 & ~(SNAND_SFCK_SAM_DLY_MASK | SNAND_SFIFO_WR_EN_DLY_SEL_MASK);

                *RW_SNAND_DLY_CTL3 = tmp_val | (g_snand_sam_clk_dly_tbl[m]) | (g_snand_sam_clk_dly_tbl[m] << 24);

                for(k = 0 ; k < 10; k++);   // delay awhile

                if (true == stt_snand_read_bulk_data_test(g_stt_snand_golden_block, test_round))
                {
                    result1[j][m] = 0x1;
                }
                else
                {
                    result1[j][m] = 0x0;
                }
            }
        }

        printf("\n......................SNAND Tuning Tool.........................\n" );
        printf("\n......................SFC: %dMhz .........................\n ", g_stt_snand_clock);
        printf("\nSNAND Driving Setting %x : ", driving_tbl[i]);

        if (g_stt_snand_inv_clk > 0)
        {
            printf("\nSample Clock Inverse: Enabled (0.%dT delay)      \n", g_stt_snand_inv_clk);
        }
        else
        {
            printf("\nSample Clock Inverse: Disabled (0T delay)      \n", g_stt_snand_inv_clk);
        }

        printf(">>"); // Special mark For parsing tool to find a grid

        for (j = 0; j < 48; j++)
        {
            printf("\n" );

            for (m = 0; m < 48; m++)
            {
                printf("%d ",result1[j][m]);
            }

            printf("  " );
        }

        printf("\n<<\n"); // Special mark For parsing tool to find a grid

        printf("\n");

        #if defined(__SNAND_STT_PARSER_ENABLED__)
        {
            #define SNAND_STT_PARSER_RULE_MIN_ONE_CNT   (15)

            u32 x, y, one_count, one_start;

            if (0 == timing_found)
            {
                for (x = 0; x < 48; x++)
                {
                    one_count = 0;
                    one_start = 0xFFFFFFFF;

                    for (y = 0; y < 49; y++)
                    {
                        if (1 == result1[x][y] && y < 48)
                        {
                            one_count++;

                            if (0xFFFFFFFF == one_start)
                            {
                                one_start = y;
                            }
                        }
                        else    // 0 || y == 49 (all 1 in this row)
                        {
                            if (one_count >= SNAND_STT_PARSER_RULE_MIN_ONE_CNT)
                            {
                                sel_io_input_dly_idx = x;
                                sel_sample_clk_dly_idx = (one_start + y) / 2;

                                timing_found = 1;

                                break;
                            }
                            else    // not discover 1 yet OR one_count is not long enough
                            {
                                if (0xFFFFFFFF == one_start)    // not discover 1 yet
                                {
                                    continue;
                                }
                                else    // continuous 1 is not enough
                                {
                                    break;  // finish this line search
                                }
                            }
                        }
                    }

                    if (timing_found)
                    {
                        sel_io_input_dly_idx = g_snand_io_input_dly_tbl[sel_io_input_dly_idx];
                        sel_sample_clk_dly_idx = g_snand_sam_clk_dly_tbl[sel_sample_clk_dly_idx];
                        sel_driving = *RW_GPIO_DRV0;

                        break;
                    }
                }
            }
        }
        #endif
    }

    #if defined(__SNAND_STT_PARSER_ENABLED__)
    if (timing_found)
    {
        printf("\n\r ====== SPI-NAND Timing for %d Mhz ======\n\r", g_stt_snand_clock);

        *RW_SNAND_DLY_CTL2 = sel_io_input_dly_idx | (sel_io_input_dly_idx << 8) | (sel_io_input_dly_idx << 16) | (sel_io_input_dly_idx << 24);

        printf("\n SNAND_DLY_CTL2(0x%x)   = 0x%x", RW_SNAND_DLY_CTL2, *RW_SNAND_DLY_CTL2);

        i = *RW_SNAND_DLY_CTL3 & ~(SNAND_SFCK_SAM_DLY_MASK | SNAND_SFIFO_WR_EN_DLY_SEL_MASK);

        *RW_SNAND_DLY_CTL3 = i | (sel_sample_clk_dly_idx) | (sel_sample_clk_dly_idx << 24);

        printf("\n SNAND_DLY_CTL3(0x%x)   = 0x%x", RW_SNAND_DLY_CTL3, *RW_SNAND_DLY_CTL3);

        printf("\n SNAND_MISC_CTL(0x%x)   = 0x%x", RW_SNAND_MISC_CTL, *RW_SNAND_MISC_CTL);

        *RW_GPIO_DRV0 = sel_driving;

        printf("\n GPIO_DRV0(0x%x)   = 0x%x", RW_GPIO_DRV0, sel_driving);
    }
    else
    {
        printf("\n\r [STT ERROR] Window is not found!\n\r");
    }
    #endif

    printf("\n ......................SNAND Tuning Tool Finished ...................\n\n" );

    while(1);
}


void stt_snand_init_pattern(void)
{
    #define SNAND_STT_INIT_PATTERN_BLOCK_SEARCH_BEGIN   (100)
    #define SNAND_STT_INIT_PATTERN_BLOCK_SEARCH_END     (120)

    u32 i, j;
    u32 len_left, row_addr;

    // 128 bytes
    for (i = 0; i < SNAND_STT_PATTERN_UNIT_SIZE / 2; i += 4)
    {
        *(u32 *)(&g_stt_snand_buf_golden_pattern[i]) = 0xF0F0F0F0; //g_stt_snand_buf_golden_pattern used size 256 byte, 128 byte fill 0xF0F0F0F0 another 128 byte fill 0x5A5AA5A5
    }

    // 128 bytes
    for (i = SNAND_STT_PATTERN_UNIT_SIZE / 2; i < SNAND_STT_PATTERN_UNIT_SIZE; i += 4)
    {
        *(u32 *)(&g_stt_snand_buf_golden_pattern[i]) = 0x5A5AA5A5;
    }

    g_stt_snand_golden_block = SNAND_STT_INIT_PATTERN_BLOCK_SEARCH_BEGIN;

STT_SNAND_INIT_PATTERN_BEGIN:

    // step 1. find a good block

    for (; g_stt_snand_golden_block < SNAND_STT_INIT_PATTERN_BLOCK_SEARCH_END; g_stt_snand_golden_block++)
    {
        if (mtk_nand_erase_hw(g_stt_snand_golden_block * g_nand_chip.erasesize))
        {
            break;
        }
    }

    if (SNAND_STT_INIT_PATTERN_BLOCK_SEARCH_END == g_stt_snand_golden_block)
    {
        printf("[stt] error: can't find a good block for STT!");

        while (1);
    }

    // 256 bytes each block
    // 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0,
    // 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0,
    // 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0,
    // 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0, 0xF0F0F0F0,
    // 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5,
    // 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5,
    // 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5,
    // 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5, 0x5A5AA5A5,

    // step 1. write golden pattern

    // copy 2KB golden pattern to write buffer

    for (i = 0; i < g_nand_chip.oobblock / SNAND_STT_PATTERN_UNIT_SIZE; i++)
    {
        memcpy((u8 *)(((u32)g_stt_snand_buf_read) + (SNAND_STT_PATTERN_UNIT_SIZE * i)), g_stt_snand_buf_golden_pattern, SNAND_STT_PATTERN_UNIT_SIZE);

        //if (g_stt_debug) printf("memcpy - g_stt_snand_buf_read: 0x%X\n", (u32)(((u32)g_stt_snand_buf_read) + (SNAND_STT_PATTERN_UNIT_SIZE * i)));
    }

    // write golden pattern to SPI-NAND (2KB data per page)

    for (i = 0; i < SNAND_STT_PATTERN_TOTAL_SIZE / g_nand_chip.oobblock; i++)
    {
        memset(g_stt_snand_buf_spare, 0xFF, g_nand_chip.oobsize);

        if (!mtk_nand_write_page_hw(i + g_stt_snand_golden_block * (g_nand_chip.erasesize / g_nand_chip.oobblock), g_stt_snand_buf_read, g_stt_snand_buf_spare))
        {
            g_stt_snand_golden_block++;

            goto STT_SNAND_INIT_PATTERN_BEGIN;
        }
    }
}

/*
 * NOTE. This API must be called after mtk_wdt_init() is called (in platform_init())
 */
void stt_snand_disable_wdt_reset(void)
{
    u32 tmp;

    tmp = DRV_Reg32(MTK_WDT_MODE);
    tmp &= ~MTK_WDT_MODE_ENABLE;       /* disable watchdog */
    tmp |= (MTK_WDT_MODE_KEY);         /* need key then write is allowed */
    DRV_WriteReg32(MTK_WDT_MODE,tmp);
}

void stt_snand_main(void)
{
    printf("========================== SPI-NAND STT ==========================\n");

    g_stt_snand_buf_read = (u8 *)NAND_NFI_BUFFER;

    if ((u32)g_stt_snand_buf_read % 32)
    {
        g_stt_snand_buf_read = g_stt_snand_buf_read + (32 - ((u32)g_stt_snand_buf_read % 32));
    }

    g_stt_snand_buf_spare = g_stt_snand_buf_read + g_nand_chip.oobblock + g_nand_chip.oobsize;

    g_stt_snand_buf_golden_pattern = g_stt_snand_buf_spare + g_nand_chip.oobsize;

    printf("[env] read buf: 0x%X\n", g_stt_snand_buf_read);

    printf("\n{{");
    printf("\nMT6572 SNAND");
    printf("\n}}\n");
    printf("\n((");
    printf("\n%dMhz", g_stt_snand_clock);
    printf("\n))");

    //Disable WDT reset because STT test will do for a long time
    stt_snand_disable_wdt_reset();

    //---------------------------------------------------------------
    // Step 1: Program Pattern
    //---------------------------------------------------------------

    stt_snand_init_pattern();                      // initilaize pattern

    //---------------------------------------------------------------
    // Step 2: PLL init
    //---------------------------------------------------------------

    if(104 == g_stt_snand_clock)
    {
        *((P_U32)(0x10000000)) = (*((P_U32)(0x10000000)) & 0xBFFE3FFF) | 0x40010000;    // (preloader does not have such API, thus modify register directly)
    }
    else if(26 == g_stt_snand_clock)
    {
        // do nothing because we already use 26 MHz now
    }

    //---------------------------------------------------------------
    // Step 3: Run STT test
    //---------------------------------------------------------------
    stt_snand_test_engine();
}

#endif

