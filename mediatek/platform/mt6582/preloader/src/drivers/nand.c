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
#include "nand.h"
#include "nand_core.h"
#include "bmt.h"
#include "part.h"
#include "partition_define.h"
#include "dram_buffer.h"

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE (2)
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

#define NFI_ISSUE_COMMAND(cmd, col_addr, row_addr, col_num, row_num) \
    do { \
    DRV_WriteReg(NFI_CMD_REG16,cmd);\
    while (DRV_Reg32(NFI_STA_REG32) & STA_CMD_STATE);\
    DRV_WriteReg32(NFI_COLADDR_REG32, col_addr);\
    DRV_WriteReg32(NFI_ROWADDR_REG32, row_addr);\
    DRV_WriteReg(NFI_ADDRNOB_REG16, col_num | (row_num<<ADDR_ROW_NOB_SHIFT));\
    while (DRV_Reg32(NFI_STA_REG32) & STA_ADDR_STATE);\
    }while(0);

u32 PAGE_SIZE;
u32 BLOCK_SIZE;

#define STORAGE_BUFFER_SIZE 0x10000
extern u8 storage_buffer[STORAGE_BUFFER_SIZE];
//u8 __DRAM__ nand_nfi_buf[NAND_NFI_BUFFER_SIZE];
#define nand_nfi_buf g_dram_buf->nand_nfi_buf

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
unsigned char g_nand_spare[128];

unsigned int nand_maf_id;
unsigned int nand_dev_id;
uint8 ext_id1, ext_id2, ext_id3;

static u32 g_u4ChipVer;
static u32 g_i4ErrNum;
static BOOL g_bInitDone;
BOOL g_bHwEcc = TRUE;
u32 PAGE_SIZE;
u32 BLOCK_SIZE;
u8 Bad_Block_Table[8192] = { 0 };

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
               80, 81, 82, 83, 84, 85, 86, 86,
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
static flashdev_info devinfo;
static char *nfi_buf;

bool get_device_info(u8*id, flashdev_info *devinfo);

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

#define NAND_SECTOR_SIZE 512

/**************************************************************************
*  reset descriptor
**************************************************************************/
void mtk_nand_reset_descriptor(void)
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

bool get_device_info(u8*id, flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1,target_id_len=-1;
    
    for (i = 0; i<CHIP_CNT; i++){
		mismatch=0;
		for(m=0;m<gen_FlashTable[i].id_length;m++){	
			if(id[m]!=gen_FlashTable[i].id[m]){
				mismatch=1;
				break;
			}	
		}
		if(mismatch == 0 && gen_FlashTable[i].id_length > target_id_len){
				target=i;
				target_id_len=gen_FlashTable[i].id_length;
		}	
    }

    if(target != -1){
		MSG(INIT, "Recognize NAND: ID [");
		for(n=0;n<gen_FlashTable[target].id_length;n++){
			devinfo->id[n] = gen_FlashTable[target].id[n];
			MSG(INIT, "%x ",devinfo->id[n]);
		}
		MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_FlashTable[target].devciename,gen_FlashTable[target].pagesize,gen_FlashTable[target].sparesize,gen_FlashTable[target].totalsize);
		devinfo->id_length=gen_FlashTable[i].id_length;
		devinfo->blocksize = gen_FlashTable[target].blocksize;
		devinfo->addr_cycle = gen_FlashTable[target].addr_cycle;
		devinfo->iowidth = gen_FlashTable[target].iowidth;
		devinfo->timmingsetting = gen_FlashTable[target].timmingsetting;
		devinfo->advancedmode = gen_FlashTable[target].advancedmode;
		devinfo->pagesize = gen_FlashTable[target].pagesize;
		devinfo->sparesize = gen_FlashTable[target].sparesize;
		devinfo->totalsize = gen_FlashTable[target].totalsize;
		memcpy(devinfo->devciename, gen_FlashTable[target].devciename, sizeof(devinfo->devciename));
    	return true;
	}else{
	    MSG(INIT, "Not Found NAND: ID [");
		for(n=0;n<NAND_MAX_ID;n++){
			MSG(INIT, "%x ",id[n]);
		}
		MSG(INIT, "]\n");
        return false;
	}
}

//---------------------------------------------------------------------------
static bool mtk_nand_check_RW_count(u16 u2WriteSize)
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
static bool mtk_nand_status_ready(u32 u4Status)
{
    u32 timeout = 0xFFFF;
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
static void mtk_nand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

//---------------------------------------------------------------------------
static bool mtk_nand_set_command(u16 command)
{
    /* Write command to device */
    DRV_WriteReg16(NFI_CMD_REG16, command);
    return mtk_nand_status_ready(STA_CMD_STATE);
}

//---------------------------------------------------------------------------
static bool mtk_nand_set_address(u32 u4ColAddr, u32 u4RowAddr, u16 u2ColNOB, u16 u2RowNOB)
{
    /* fill cycle addr */
    DRV_WriteReg32(NFI_COLADDR_REG32, u4ColAddr);
    DRV_WriteReg32(NFI_ROWADDR_REG32, u4RowAddr);
    DRV_WriteReg16(NFI_ADDRNOB_REG16, u2ColNOB | (u2RowNOB << ADDR_ROW_NOB_SHIFT));
    return mtk_nand_status_ready(STA_ADDR_STATE);
}

//---------------------------------------------------------------------------
static void ECC_Decode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}

//---------------------------------------------------------------------------
static void ECC_Decode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

//---------------------------------------------------------------------------
static void ECC_Encode_Start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

//---------------------------------------------------------------------------
static void ECC_Encode_End(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

//---------------------------------------------------------------------------
static void ECC_Config(u32 ecc_bit)
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
* mtk_nand_check_bch_error
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
static bool mtk_nand_check_bch_error(u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
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
            } else
            {
                u4ErrNum = DRV_Reg32(ECC_DECENUM1_REG32) >> ((i - 4) * 5);
            }
            u4ErrNum &= 0x1F;
            if (0x1F == u4ErrNum)
            {
                MSG(ERR, "In Preloader UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
                bRet = false;
            } else
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
static bool mtk_nand_RFIFOValidSize(u16 u2Size)
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
static bool mtk_nand_WFIFOValidSize(u16 u2Size)
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
bool mtk_nand_reset(void)
{
    int timeout = 0xFFFF;
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

    return mtk_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && mtk_nand_RFIFOValidSize(0) && mtk_nand_WFIFOValidSize(0);
}

static bool mtk_nand_read_status(void)
{
    int status, i;
    mtk_nand_reset();
    unsigned int timeout;

    mtk_nand_reset();

    /* Disable HW ECC */
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    /* Disable 16-bit I/O */
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_OP_SRD | CNFG_READ_EN | CNFG_BYTE_RW);

    DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NOB_SHIFT));

    DRV_WriteReg16(NFI_CON_REG16, 0x3);
    mtk_nand_set_mode(CNFG_OP_SRD);
    DRV_WriteReg16(NFI_CNFG_REG16, 0x2042);
    mtk_nand_set_command(NAND_CMD_STATUS);
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
}

//---------------------------------------------------------------------------

static void mtk_nand_configure_lock(void)
{
    u32 u4WriteColNOB = 2;
    u32 u4WriteRowNOB = 3;
    u32 u4EraseColNOB = 0;
    u32 u4EraseRowNOB = 3;
    DRV_WriteReg16(NFI_LOCKANOB_REG16,
                   (u4WriteColNOB << PROG_CADD_NOB_SHIFT) | (u4WriteRowNOB << PROG_RADD_NOB_SHIFT) | (u4EraseColNOB << ERASE_CADD_NOB_SHIFT) | (u4EraseRowNOB << ERASE_RADD_NOB_SHIFT));

    // Workaround method for ECO1 mt6577
    if (CHIPVER_ECO_1 == g_u4ChipVer)
    {
        int i;
        for (i = 0; i < 16; ++i)
        {
            DRV_WriteReg32(NFI_LOCK00ADD_REG32 + (i << 1), 0xFFFFFFFF);
            DRV_WriteReg32(NFI_LOCK00FMT_REG32 + (i << 1), 0xFFFFFFFF);
        }
        //DRV_WriteReg16(NFI_LOCKANOB_REG16, 0x0);
        DRV_WriteReg32(NFI_LOCKCON_REG32, 0xFFFFFFFF);
        DRV_WriteReg16(NFI_LOCK_REG16, NFI_LOCK_ON);
    }
}

//---------------------------------------------------------------------------

static void mtk_nand_configure_fdm(u16 u2FDMSize)
{
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//---------------------------------------------------------------------------
static void mtk_nand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//---------------------------------------------------------------------------
static void mtk_nand_command_bp(unsigned command)
{
    u32 timeout;

    switch (command)
    {
      case NAND_CMD_READID:
          /* Issue NAND chip reset command */
          NFI_ISSUE_COMMAND(NAND_CMD_RESET, 0, 0, 0, 0);

          timeout = TIMEOUT_4;

          while (timeout)
              timeout--;

          mtk_nand_reset();

          /* Disable HW ECC */
          NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

          /* Disable 16-bit I/O */
          NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
          NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
          mtk_nand_reset();
          mtk_nand_set_mode(CNFG_OP_SRD);
          mtk_nand_set_command(NAND_CMD_READID);
          mtk_nand_set_address(0, 0, 1, 0);
          DRV_WriteReg16(NFI_CON_REG16, CON_NFI_SRD);
          while (DRV_Reg32(NFI_STA_REG32) & STA_DATAR_STATE) ;
          break;

      default:
          break;
    }
}

//-----------------------------------------------------------------------------
static u8 mtk_nand_read_byte(void)
{
    /* Check the PIO bit is ready or not */
    u32 timeout = TIMEOUT_4;
    WAIT_NFI_PIO_READY(timeout);
    return DRV_Reg8(NFI_DATAR_REG32);
}

bool getflashid(u8 * nand_id, int longest_id_number)
{
    u8 maf_id = 0;
    u8 dev_id = 0;
    int i = 0;
    u8 *id = nand_id;
    //PDN_Power_CONA_DOWN (PDN_PERI_NFI, FALSE);

    DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    mtk_nand_command_bp(NAND_CMD_READID);

    maf_id = mtk_nand_read_byte();
    dev_id = mtk_nand_read_byte();

    if (maf_id == 0 || dev_id == 0)
    {
        return FALSE;
    }
    //*id= (dev_id<<8)|maf_id;
    //    *id= (maf_id<<8)|dev_id;
    id[0] = maf_id;
    id[1] = dev_id;

    for (i = 2; i < longest_id_number; i++)
        id[i] = mtk_nand_read_byte();

    return TRUE;
}

int mtk_nand_init(void)
{
    int i, j, busw;
     u8 id[NAND_MAX_ID];
    u16 spare_bit = 0;

    u16 spare_per_sector = 16;
    u32 ecc_bit = 4;
    nfi_buf = (unsigned char *)NAND_NFI_BUFFER;

    memset(&devinfo, 0, sizeof(devinfo));

    /* Dynamic Control */
    g_bInitDone = FALSE;
    g_u4ChipVer = DRV_Reg32(CONFIG_BASE);   /*HW_VER */
    g_kCMD.u4OOBRowAddr = (u32) - 1;

    DRV_WriteReg16(NFI_CSEL_REG16, NFI_DEFAULT_CS);


    /* Set default NFI access timing control */
    DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);

    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);

    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    mtk_nand_reset();

    /* Read the first 4 byte to identify the NAND device */

    g_nand_chip.page_shift = NAND_LARGE_PAGE;
    g_nand_chip.page_size = 1 << g_nand_chip.page_shift;
    g_nand_chip.oobblock = NAND_PAGE_SIZE;
    g_nand_chip.oobsize = NAND_BLOCK_BLKS;

    g_nand_chip.nand_ecc_mode = NAND_ECC_HW;

    mtk_nand_command_bp(NAND_CMD_READID);
    
    for(i=0;i<NAND_MAX_ID;i++){
		id[i]=mtk_nand_read_byte ();
	}
	nand_maf_id = id[0];
    	nand_dev_id = id[1];
     memset(&devinfo, 0, sizeof(devinfo));

    if (!get_device_info(id, &devinfo))
    {
        MSG(INIT, "NAND unsupport\n");
        ASSERT(0);
    }

    g_nand_chip.name = devinfo.devciename;
    g_nand_chip.chipsize = devinfo.totalsize << 20;
    g_nand_chip.page_size = devinfo.pagesize;
    g_nand_chip.page_shift = uffs(g_nand_chip.page_size) - 1;
    g_nand_chip.oobblock = g_nand_chip.page_size;
    g_nand_chip.erasesize = devinfo.blocksize << 10;

    g_nand_chip.bus16 = devinfo.iowidth;
    DRV_WriteReg32(NFI_ACCCON_REG32, devinfo.timmingsetting);

    if (!devinfo.sparesize)
        g_nand_chip.oobsize = (8 << ((ext_id2 >> 2) & 0x01)) * (g_nand_chip.oobblock / 512);
    else
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
    MSG(INIT, "[NAND]: oobsize: %x\n", g_nand_chip.oobsize);
    g_nand_chip.chipsize -= g_nand_chip.erasesize * (BMT_POOL_SIZE);
    if (g_nand_chip.bus16 == NAND_BUS_WIDTH_16)
    {
#ifdef  DBG_PRELOADER
        MSG(INIT, "USE 16 IO\n");
#endif
        NFI_SET_REG16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);
    }

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
        ECC_Config(ecc_bit);
        mtk_nand_configure_fdm(8);
        mtk_nand_configure_lock();
    }

    /* Initilize interrupt. Clear interrupt, read clear. */
    DRV_Reg16(NFI_INTR_REG16);

    /* Interrupt arise when read data or program data to/from AHB is done. */
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);

    if (!(init_bmt(&g_nand_chip, BMT_POOL_SIZE)))
    {
        MSG(INIT, "Error: init bmt failed, quit!\n");
        ASSERT(0);
        return 0;
    }

    g_nand_chip.chipsize -= g_nand_chip.erasesize * (PMT_POOL_SIZE);
    return 0;
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_read(void)
{
    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);
    if (g_bHwEcc)
    {
        ECC_Decode_End();
    }
}

//-----------------------------------------------------------------------------
static void mtk_nand_stop_write(void)
{
    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);
    if (g_bHwEcc)
    {
        ECC_Encode_End();
    }
}

//-----------------------------------------------------------------------------
static bool mtk_nand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;
    while ((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout > 0)
        timeout--;
    if (timeout == 0)
    {
        MSG(ERR, "ECC_DECDONE: timeout\n");
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_read_page_data(u32 * buf)
{
    u32 timeout = 0xFFFF;
    u32 u4Size = g_nand_chip.oobblock;
    u32 i;
    u32 *pBuf32;

#if (USE_AHB_MODE)
    pBuf32 = (u32 *) buf;
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
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
static bool mtk_nand_write_page_data(u32 * buf)
{
    u32 timeout = 0xFFFF;
    u32 u4Size = g_nand_chip.oobblock;

#if (USE_AHB_MODE)
    u32 *pBuf32;
    pBuf32 = (u32 *) buf;

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);
    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);
    while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
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
static void mtk_nand_read_fdm_data(u32 u4SecNum, u8 * spare_buf)
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
static void mtk_nand_write_fdm_data(u32 u4SecNum, u8 * oob)
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
static bool mtk_nand_ready_for_read(u32 page_addr, u32 sec_num, u8 * buf)
{
    u32 u4RowAddr = page_addr;
    u32 colnob = 2;
    u32 rownob = devinfo.addr_cycle - colnob;
    bool bRet = FALSE;

    if (!mtk_nand_reset())
    {
        goto cleanup;
    }

    /* Enable HW ECC */
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    mtk_nand_set_mode(CNFG_OP_READ);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
    if (g_bHwEcc)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    mtk_nand_set_autoformat(TRUE);
    if (g_bHwEcc)
    {
        ECC_Decode_Start();
    }
    if (!mtk_nand_set_command(NAND_CMD_READ0))
    {
        goto cleanup;
    }
    if (!mtk_nand_set_address(0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_command(NAND_CMD_READSTART))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready(STA_NAND_BUSY))
    {
        goto cleanup;
    }

    bRet = TRUE;

  cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool mtk_nand_ready_for_write(u32 page_addr, u32 sec_num, u8 * buf)
{
    bool bRet = FALSE;
    u32 u4RowAddr = page_addr;
    u32 colnob = 2;
    u32 rownob = devinfo.addr_cycle - colnob;

    if (!mtk_nand_reset())
    {
        return FALSE;
    }

    mtk_nand_set_mode(CNFG_OP_PRGM);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);

    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    if (g_bHwEcc)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    mtk_nand_set_autoformat(TRUE);
    if (g_bHwEcc)
    {
        ECC_Encode_Start();
    }

    if (!mtk_nand_set_command(NAND_CMD_SEQIN))
    {
        goto cleanup;
    }

    if (!mtk_nand_set_address(0, u4RowAddr, colnob, rownob))
    {
        goto cleanup;
    }

    if (!mtk_nand_status_ready(STA_NAND_BUSY))
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

int mtk_nand_read_page_hw(u32 page, u8 * dat, u8 * oob)
{
    bool bRet = TRUE;
    u8 *pPageBuf;
    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    pPageBuf = (u8 *) dat;

    if (mtk_nand_ready_for_read(page, u4SecNum, pPageBuf))
    {
        if (!mtk_nand_read_page_data((u32 *) pPageBuf))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_status_ready(STA_NAND_BUSY))
        {
            bRet = FALSE;
        }
        if (g_bHwEcc)
        {
            if (!mtk_nand_check_dececc_done(u4SecNum))
            {
                bRet = FALSE;
            }
        }
        mtk_nand_read_fdm_data(u4SecNum, oob);
        if (g_bHwEcc)
        {
            if (!mtk_nand_check_bch_error(pPageBuf, u4SecNum - 1, page))
            {
                MSG(ERASE, "check bch error !\n");
                bRet = FALSE;
            }
        }
        mtk_nand_stop_read();
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

int mtk_nand_write_page_hw(u32 page, u8 * dat, u8 * oob)
{
    bool bRet = TRUE;
    u32 pagesz = g_nand_chip.oobblock;
    u32 timeout, u4SecNum = pagesz >> NAND_PAGE_SHIFT;

    int i, j, start, len;
    bool empty = TRUE;
    u8 oob_checksum = 0;

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

    while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;

    if (mtk_nand_ready_for_write(page, u4SecNum, dat))
    {
        mtk_nand_write_fdm_data(u4SecNum, oob);
        if (!mtk_nand_write_page_data((u32 *) dat))
        {
            bRet = FALSE;
        }
        if (!mtk_nand_check_RW_count(g_nand_chip.oobblock))
        {
            bRet = FALSE;
        }
        mtk_nand_stop_write();
        mtk_nand_set_command(NAND_CMD_PAGEPROG);
        mtk_nand_status_ready(STA_NAND_BUSY);
        return mtk_nand_read_status();
    } else
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
    unsigned char *pspare;
    char *tmp = (char *)nfi_buf;
    memset(tmp, 0x0, g_nand_chip.oobblock + g_nand_chip.oobsize);

    u32 u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    page &= ~(page_num - 1);

    if (mtk_nand_ready_for_read(page, u4SecNum, tmp))
    {
        if (!mtk_nand_read_page_data((u32 *) tmp))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_status_ready(STA_NAND_BUSY))
        {
            bRet = FALSE;
        }

        if (!mtk_nand_check_dececc_done(u4SecNum))
        {
            bRet = FALSE;
        }

        mtk_nand_read_fdm_data(u4SecNum, g_nand_spare);

        if (!mtk_nand_check_bch_error(tmp, u4SecNum - 1, page))
        {
            MSG(ERASE, "check bch error !\n");
            bRet = FALSE;
        }

        mtk_nand_stop_read();
    }

    pspare = g_nand_spare;

    if (pspare[0] != 0xFF || pspare[8] != 0xFF || pspare[16] != 0xFF || pspare[24] != 0xFF)
    {
        bRet = TRUE;
        // break;
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
    while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;

    if (mtk_nand_ready_for_write(page_addr, u4SecNum, buf))
    {
        mtk_nand_write_fdm_data(u4SecNum, g_nand_spare);
        if (!mtk_nand_write_page_data((u32 *) & buf))
        {
            bRet = FALSE;
        }
        if (!mtk_nand_check_RW_count(g_nand_chip.oobblock))
        {
            bRet = FALSE;
        }
        mtk_nand_stop_write();
        mtk_nand_set_command(NAND_CMD_PAGEPROG);
        mtk_nand_status_ready(STA_NAND_BUSY);
    } else
    {
        return FALSE;
    }

    for (index = 0; index < 64; index++)
        *(g_nand_spare + index) = 0xFF;
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
    u32 timeout, u4SecNum = g_nand_chip.oobblock >> NAND_PAGE_SHIFT;
    u32 rownob = devinfo.addr_cycle - 2;
    u32 page_addr = offset / g_nand_chip.oobblock;

    if (nand_block_bad_hw(offset))
    {
        return FALSE;
    }

    mtk_nand_reset();
    mtk_nand_set_mode(CNFG_OP_ERASE);
    mtk_nand_set_command(NAND_CMD_ERASE1);
    mtk_nand_set_address(0, page_addr, 0, rownob);

    mtk_nand_set_command(NAND_CMD_ERASE2);
    if (!mtk_nand_status_ready(STA_NAND_BUSY))
    {
        return FALSE;
    }

    if (!mtk_nand_read_status())
    {
        return FALSE;
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
    while (DRV_Reg32(NFI_STA_REG32) & STA_NAND_BUSY) ;
    return TRUE;
}

/**************************************************************************
*  MACRO LIKE FUNCTION
**************************************************************************/
static int nand_bread(blkdev_t * bdev, u32 blknr, u32 blks, u8 * buf)
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

static int nand_bwrite(blkdev_t * bdev, u32 blknr, u32 blks, u8 * buf)
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
        mtk_nand_reset_descriptor();
        mtk_nand_init();

        PAGE_SIZE = (u32) g_nand_chip.page_size;
        BLOCK_SIZE = (u32) g_nand_chip.erasesize;

        memset(&g_nand_bdev, 0, sizeof(blkdev_t));
        g_nand_bdev.blksz = g_nand_chip.page_size;
        g_nand_bdev.erasesz = g_nand_chip.erasesize;
        g_nand_bdev.blks = g_nand_chip.chipsize;
        g_nand_bdev.bread = nand_bread;
        g_nand_bdev.bwrite = nand_bwrite;
        g_nand_bdev.blkbuf = (u8 *) storage_buffer;
        g_nand_bdev.type = BOOTDEV_NAND;
        blkdev_register(&g_nand_bdev);
    }

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

    if (TRUE != getflashid(buf, len))
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

