#include <stdio.h>  // for printf ...
#include <string.h> // for memcpy ...
#include <malloc.h> // for malloc ...
#include <config.h>
#include <platform/mt_typedefs.h>
#include <platform/mtk_nand.h>
#include <platform/mtk_snand_lk.h>
#include <mt_partition.h>
#include <platform/bmt.h>
#include "partition_define.h"
#include "cust_nand.h"
#include <arch/ops.h>
#include "snand_device_list.h"
#include <kernel/event.h>
#include <platform/mt_irq.h>
#include <mt_gpt.h>

//#define NAND_LK_TEST
#ifdef NAND_LK_TEST
#include "mt_partition.h"
#endif

#ifdef CONFIG_CMD_SNAND

#ifndef PART_SIZE_BMTPOOL
#define BMT_POOL_SIZE (80)
#else
#define BMT_POOL_SIZE (PART_SIZE_BMTPOOL)
#endif

#define PMT_POOL_SIZE	(2)

// Read Split related definitions and variables
#define SNAND_RS_BOUNDARY_BLOCK                     (2)
#define SNAND_RS_BOUNDARY_KB                        (1024)
#define SNAND_RS_SPARE_PER_SECTOR_FIRST_PART_VAL    (16)                                        // MT6572 shoud fix this as 16
#define SNAND_RS_SPARE_PER_SECTOR_FIRST_PART_NFI    (PAGEFMT_SPARE_16 << PAGEFMT_SPARE_SHIFT)   // MT6572 shoud fix this as 16
#define SNAND_RS_ECC_BIT_FIRST_PART                 (4)                                         // MT6572 shoud fix this as 4

u32 g_snand_rs_ecc_bit_second_part;
u32 g_snand_rs_spare_per_sector_second_part_nfi;
u32 g_snand_rs_num_page = 0;
u32 g_snand_rs_cur_part = 0xFFFFFFFF;

u32 g_snand_spare_per_sector = 0;   // because Read Split feature will change spare_per_sector in run-time, thus use a variable to indicate current spare_per_sector

// Buffer must be 64 byte cache line alignment, both start and end address.
__attribute__((aligned(64))) unsigned char g_snand_temp[NAND_SECTOR_SIZE + 64];

extern int mt_part_register_device(part_dev_t * dev);

int check_data_empty(void *data, unsigned size);
bool __nand_erase (u32 logical_addr);
bool mark_block_bad (u32 logical_addr);
bool mark_block_bad_hw(u32 offset);
bool nand_erase_hw (u32 offset);


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
    .oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7},
                {57, 6}}
};

static bmt_struct *g_bmt = NULL;
static struct nand_chip g_nand_chip;
static int en_interrupt = 0;
static event_t nand_int_event;

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

snand_flashdev_info devinfo;

#define CHIPVER_ECO_1 (0x8a00)
#define CHIPVER_ECO_2 (0x8a01)

struct NAND_CMD g_kCMD;
static u32 g_i4ErrNum;
static bool g_bInitDone;
u32 total_size;
u32 g_nand_size = 0;

static unsigned char g_data_buf[4096+128] __attribute__ ((aligned(32)));
static unsigned char g_spare_buf[256];
static u32 download_size = 0;

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

void dump_nfi(void)
{
    printf("~~~~Dump NFI Register in LK~~~~\n");
    printf("NFI_CNFG_REG16: 0x%x\n", DRV_Reg16(NFI_CNFG_REG16));
    printf("NFI_PAGEFMT_REG16: 0x%x\n", DRV_Reg16(NFI_PAGEFMT_REG16));
    printf("NFI_CON_REG16: 0x%x\n", DRV_Reg16(NFI_CON_REG16));
    printf("NFI_ACCCON_REG32: 0x%x\n", DRV_Reg32(NFI_ACCCON_REG32));
    printf("NFI_INTR_EN_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_EN_REG16));
    printf("NFI_INTR_REG16: 0x%x\n", DRV_Reg16(NFI_INTR_REG16));
    printf("NFI_CMD_REG16: 0x%x\n", DRV_Reg16(NFI_CMD_REG16));
    printf("NFI_ADDRNOB_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRNOB_REG16));
    printf("NFI_COLADDR_REG32: 0x%x\n", DRV_Reg32(NFI_COLADDR_REG32));
    printf("NFI_ROWADDR_REG32: 0x%x\n", DRV_Reg32(NFI_ROWADDR_REG32));
    printf("NFI_STRDATA_REG16: 0x%x\n", DRV_Reg16(NFI_STRDATA_REG16));
    printf("NFI_DATAW_REG32: 0x%x\n", DRV_Reg32(NFI_DATAW_REG32));
    printf("NFI_DATAR_REG32: 0x%x\n", DRV_Reg32(NFI_DATAR_REG32));
    printf("NFI_PIO_DIRDY_REG16: 0x%x\n", DRV_Reg16(NFI_PIO_DIRDY_REG16));
    printf("NFI_STA_REG32: 0x%x\n", DRV_Reg32(NFI_STA_REG32));
    printf("NFI_FIFOSTA_REG16: 0x%x\n", DRV_Reg16(NFI_FIFOSTA_REG16));
    printf("NFI_LOCKSTA_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKSTA_REG16));
    printf("NFI_ADDRCNTR_REG16: 0x%x\n", DRV_Reg16(NFI_ADDRCNTR_REG16));
    printf("NFI_STRADDR_REG32: 0x%x\n", DRV_Reg32(NFI_STRADDR_REG32));
    printf("NFI_BYTELEN_REG16: 0x%x\n", DRV_Reg16(NFI_BYTELEN_REG16));
    printf("NFI_CSEL_REG16: 0x%x\n", DRV_Reg16(NFI_CSEL_REG16));
    printf("NFI_IOCON_REG16: 0x%x\n", DRV_Reg16(NFI_IOCON_REG16));
    printf("NFI_FDM0L_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0L_REG32));
    printf("NFI_FDM0M_REG32: 0x%x\n", DRV_Reg32(NFI_FDM0M_REG32));
    printf("NFI_LOCK_REG16: 0x%x\n", DRV_Reg16(NFI_LOCK_REG16));
    printf("NFI_LOCKCON_REG32: 0x%x\n", DRV_Reg32(NFI_LOCKCON_REG32));
    printf("NFI_LOCKANOB_REG16: 0x%x\n", DRV_Reg16(NFI_LOCKANOB_REG16));
    printf("NFI_FIFODATA0_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA0_REG32));
    printf("NFI_FIFODATA1_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA1_REG32));
    printf("NFI_FIFODATA2_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA2_REG32));
    printf("NFI_FIFODATA3_REG32: 0x%x\n", DRV_Reg32(NFI_FIFODATA3_REG32));
    printf("NFI_MASTERSTA_REG16: 0x%x\n", DRV_Reg16(NFI_MASTERSTA_REG16));
    printf("NFI clock register: 0x%x: %s\n",(PERI_CON_BASE+0x18), (DRV_Reg32((volatile u32 *)(PERI_CON_BASE+0x18)) & (0x1)) ? "Clock Disabled" : "Clock Enabled");
    printf("NFI clock SEL (MT6572):0x%x: %s\n",(PERI_CON_BASE+0x5C), (DRV_Reg32((volatile u32 *)(PERI_CON_BASE+0x5C)) & (0x1)) ? "Half clock" : "Quarter clock");

    printf("RW_SNAND_MAC_CTL: 0x%x\n", DRV_Reg32(RW_SNAND_MAC_CTL));
	printf("RW_SNAND_MAC_OUTL: 0x%x\n", DRV_Reg32(RW_SNAND_MAC_OUTL));
	printf("RW_SNAND_MAC_INL: 0x%x\n", DRV_Reg32(RW_SNAND_MAC_INL));

	printf("RW_SNAND_RD_CTL1: 0x%x\n", DRV_Reg32(RW_SNAND_RD_CTL1));
	printf("RW_SNAND_RD_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_RD_CTL2));
	printf("RW_SNAND_RD_CTL3: 0x%x\n", DRV_Reg32(RW_SNAND_RD_CTL3));

	printf("RW_SNAND_GF_CTL1: 0x%x\n", DRV_Reg32(RW_SNAND_GF_CTL1));
	printf("RW_SNAND_GF_CTL3: 0x%x\n", DRV_Reg32(RW_SNAND_GF_CTL3));

	printf("RW_SNAND_PG_CTL1: 0x%x\n", DRV_Reg32(RW_SNAND_PG_CTL1));
	printf("RW_SNAND_PG_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_PG_CTL2));
	printf("RW_SNAND_PG_CTL3: 0x%x\n", DRV_Reg32(RW_SNAND_PG_CTL3));

	printf("RW_SNAND_ER_CTL: 0x%x\n", DRV_Reg32(RW_SNAND_ER_CTL));
	printf("RW_SNAND_ER_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_ER_CTL2));

	printf("RW_SNAND_MISC_CTL: 0x%x\n", DRV_Reg32(RW_SNAND_MISC_CTL));
	printf("RW_SNAND_MISC_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_MISC_CTL2));

	printf("RW_SNAND_DLY_CTL1: 0x%x\n", DRV_Reg32(RW_SNAND_DLY_CTL1));
	printf("RW_SNAND_DLY_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_DLY_CTL2));
	printf("RW_SNAND_DLY_CTL3: 0x%x\n", DRV_Reg32(RW_SNAND_DLY_CTL3));
	printf("RW_SNAND_DLY_CTL4: 0x%x\n", DRV_Reg32(RW_SNAND_DLY_CTL4));

	printf("RW_SNAND_STA_CTL1: 0x%x\n", DRV_Reg32(RW_SNAND_STA_CTL1));
	printf("RW_SNAND_STA_CTL2: 0x%x\n", DRV_Reg32(RW_SNAND_STA_CTL2));
	printf("RW_SNAND_STA_CTL3: 0x%x\n", DRV_Reg32(RW_SNAND_STA_CTL3));

	printf("RW_SNAND_CNFG: 0x%x\n", DRV_Reg32(RW_SNAND_CNFG));

	printf("ECC_DECCNFG_REG32: 0x%X\n", DRV_Reg32(ECC_DECCNFG_REG32));
	printf("ECC_DECCON_REG16: 0x%X\n", DRV_Reg32(ECC_DECCON_REG16));
	printf("ECC_DECDIADDR_REG32: 0x%X\n", DRV_Reg32(ECC_DECDIADDR_REG32));
	printf("ECC_DECDONE_REG16: 0x%X\n", DRV_Reg16(ECC_DECDONE_REG16));
}

bool snand_get_device_info(u8*id, snand_flashdev_info *devinfo)
{
    u32 i,m,n,mismatch;
    int target=-1;
    u8 target_id_len=0;

    for (i = 0; i < SNAND_CHIP_CNT; i++)
    {
		mismatch=0;

		for (m=0;m<gen_snand_FlashTable[i].id_length;m++)
		{
			if(id[m]!=gen_snand_FlashTable[i].id[m])
			{
				mismatch=1;
				break;
			}
		}

		if (mismatch == 0 && gen_snand_FlashTable[i].id_length > target_id_len)
		{
				target=i;
				target_id_len=gen_snand_FlashTable[i].id_length;
		}
    }

    if (target != -1)
    {
		MSG(INIT, "Recognize NAND: ID [");

		for (n=0;n<gen_snand_FlashTable[target].id_length;n++)
		{
			devinfo->id[n] = gen_snand_FlashTable[target].id[n];
			MSG(INIT, "%x ",devinfo->id[n]);
		}

		MSG(INIT, "], Device Name [%s], Page Size [%d]B Spare Size [%d]B Total Size [%d]MB\n",gen_snand_FlashTable[target].devicename,gen_snand_FlashTable[target].pagesize,gen_snand_FlashTable[target].sparesize,gen_snand_FlashTable[target].totalsize);
		devinfo->id_length=gen_snand_FlashTable[target].id_length;
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
		memcpy(devinfo->devicename, gen_snand_FlashTable[target].devicename, sizeof(devinfo->devicename));
		devinfo->SNF_DLY_CTL1 = gen_snand_FlashTable[target].SNF_DLY_CTL1;
		devinfo->SNF_DLY_CTL2 = gen_snand_FlashTable[target].SNF_DLY_CTL2;
		devinfo->SNF_DLY_CTL3 = gen_snand_FlashTable[target].SNF_DLY_CTL3;
		devinfo->SNF_DLY_CTL4 = gen_snand_FlashTable[target].SNF_DLY_CTL4;
		devinfo->SNF_MISC_CTL = gen_snand_FlashTable[target].SNF_MISC_CTL;
		devinfo->SNF_DRIVING = gen_snand_FlashTable[target].SNF_DRIVING;

    	return true;
	}
	else
	{
	    MSG(INIT, "Not Found NAND: ID [");

		for(n=0;n<SNAND_MAX_ID;n++)
		{
			MSG(INIT, "%x ",id[n]);
		}

		MSG(INIT, "]\n");

        return false;
	}
}

// Read Split related APIs
static bool snand_rs_if_require_split() // must be executed after snand_rs_reconfig_nfiecc()
{
    if (devinfo.advancedmode & SNAND_ADV_READ_SPLIT)
    {
        if (g_snand_rs_cur_part != 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

static void snand_rs_reconfig_nfiecc(u32 row_addr)
{
    // 1. only decode part should be re-configured
    // 2. only re-configure essential part (fixed register will not be re-configured)

    u16 reg16;
    u32 ecc_bit;
    u32 ecc_bit_cfg = ECC_CNFG_ECC4;
    u32 u4DECODESize;

    if (0 == (devinfo.advancedmode & SNAND_ADV_READ_SPLIT))
    {
        return;
    }

    if (row_addr < g_snand_rs_num_page)
    {
        if (g_snand_rs_cur_part != 0)
        {
            ecc_bit = SNAND_RS_ECC_BIT_FIRST_PART;

            reg16 = DRV_Reg(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= SNAND_RS_SPARE_PER_SECTOR_FIRST_PART_NFI;
            DRV_WriteReg16(NFI_PAGEFMT_REG16, reg16);

            g_snand_spare_per_sector = SNAND_RS_SPARE_PER_SECTOR_FIRST_PART_VAL;

            g_snand_rs_cur_part = 0;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (g_snand_rs_cur_part != 1)
        {
            ecc_bit = g_snand_rs_ecc_bit_second_part;

            reg16 = DRV_Reg(NFI_PAGEFMT_REG16);
            reg16 &= ~PAGEFMT_SPARE_MASK;
            reg16 |= g_snand_rs_spare_per_sector_second_part_nfi;
            DRV_WriteReg16(NFI_PAGEFMT_REG16, reg16);

            g_snand_spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

            g_snand_rs_cur_part = 1;
        }
        else
        {
            return;
        }
    }

    switch(ecc_bit)
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

    u4DECODESize = ((NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3) + ecc_bit * 13;

    /* configure ECC decoder && encoder */
    DRV_WriteReg32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT | ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
}

static void snand_ecc_config(nand_ecc_level ecc_level)
{
    u32 u4ENCODESize;
    u32 u4DECODESize;
    u32 ecc_bit_cfg = ECC_CNFG_ECC4;

    switch(ecc_level)
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

    u4ENCODESize = (NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3;
    u4DECODESize = ((NAND_SECTOR_SIZE + NAND_FDM_PER_SECTOR) << 3) + ecc_level * 13;

    /* configure ECC decoder && encoder */
    DRV_WriteReg32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (u4DECODESize << DEC_CNFG_CODE_SHIFT));
    DRV_WriteReg32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (u4ENCODESize << ENC_CNFG_MSG_SHIFT));
#ifndef MANUAL_CORRECT
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_CORRECT);
#else
    NFI_SET_REG32(ECC_DECCNFG_REG32, DEC_CNFG_EL);
#endif
}


static void snand_ecc_decode_start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_EN);
}


static void snand_ecc_decode_end(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg16(ECC_DECIDLE_REG16) & DEC_IDLE)) ;
    DRV_WriteReg16(ECC_DECCON_REG16, DEC_DE);
}

//-------------------------------------------------------------------------------
static void snand_ecc_encode_start(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_EN);
}

//-------------------------------------------------------------------------------
static void snand_ecc_encode_end(void)
{
    /* wait for device returning idle */
    while (!(DRV_Reg32(ECC_ENCIDLE_REG32) & ENC_IDLE)) ;
    DRV_WriteReg16(ECC_ENCCON_REG16, ENC_DE);
}

//-------------------------------------------------------------------------------
static bool snand_check_bch_error(u8 * pDataBuf, u32 u4SecIndex, u32 u4PageAddr)
{
    bool bRet = true;
    u16 u2SectorDoneMask = 1 << u4SecIndex;
    u32 u4ErrorNumDebug0, u4ErrorNumDebug1,i, u4ErrNum;
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
            return false;
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
                MSG(ERR, "In LittleKernel UnCorrectable at PageAddr=%d, Sector=%d\n", u4PageAddr, i);
                bRet = false;
            } else
            {
		if (u4ErrNum)
                {
                    MSG(ERR, " In LittleKernel Correct %d at PageAddr=%d, Sector=%d\n", u4ErrNum, u4PageAddr, i);
		}
            }
        }
    }
#else
    memset(au4ErrBitLoc, 0x0, sizeof(au4ErrBitLoc));
    u4ErrorNumDebug0 = DRV_Reg32(ECC_DECENUM_REG32);
    u4ErrNum = DRV_Reg32(ECC_DECENUM_REG32) >> (u4SecIndex << 2);
    u4ErrNum &= 0xF;
    if (u4ErrNum)
    {
        if (0xF == u4ErrNum)
        {
            MSG(ERR, "UnCorrectable at PageAddr=%d\n", u4PageAddr);
            bRet = false;
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
                } else
                {
                    MSG(ERR, "UnCorrectable ErrLoc=%d\n", au4ErrBitLoc[i]);
                }

                u4ErrBitLoc2nd = (au4ErrBitLoc[i] >> 16) & 0x1FFF;
                if (0 != u4ErrBitLoc2nd)
                {
                    if (u4ErrBitLoc2nd < 0x1000)
                    {
                        u4ErrByteLoc = u4ErrBitLoc2nd / 8;
                        u4BitOffset = u4ErrBitLoc2nd % 8;
                        pDataBuf[u4ErrByteLoc] = pDataBuf[u4ErrByteLoc] ^ (1 << u4BitOffset);
                    } else
                    {
                        MSG(ERR, "UnCorrectable High ErrLoc=%d\n", au4ErrBitLoc[i]);
                    }
                }
            }
            bRet = true;
        }

        if (0 == (DRV_Reg16(ECC_DECFER_REG16) & (1 << u4SecIndex)))
        {
            bRet = false;
        }
    }
#endif

    return bRet;
}

static bool snand_RFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) < u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_RD_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("snand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}

//-------------------------------------------------------------------------------
static bool snand_WFIFOValidSize(u16 u2Size)
{
    u32 timeout = 0xFFFF;
    while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)) > u2Size)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    if (u2Size == 0)
    {
        while (FIFO_WR_REMAIN(DRV_Reg16(NFI_FIFOSTA_REG16)))
        {
            timeout--;
            if (0 == timeout)
            {
                printf("snand_RFIFOValidSize failed: 0x%x\n", u2Size);
                return false;
            }
        }
    }

    return true;
}

static bool snand_status_ready(u32 u4Status)
{
    #if 0
    u32 timeout = 0xFFFF;
    while ((DRV_Reg32(NFI_STA_REG32) & u4Status) != 0)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    #endif

    return true;
}

static void snand_wait_us(u32 us)
{
    gpt_busy_wait_us(us);
    //udelay(us);
}

static void snand_dev_mac_enable(SNAND_Mode mode)
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
 * @remarks: !NOTE! This function must be used with snand_dev_mac_enable in pair!
 */
static void snand_dev_mac_trigger(void)
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
 * @remarks: !NOTE! This function must be used after snand_dev_mac_trigger
 */
static void snand_dev_mac_leave(void)
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

static void snand_dev_mac_op(SNAND_Mode mode)
{
    snand_dev_mac_enable(mode);
    snand_dev_mac_trigger();
    snand_dev_mac_leave();
}

static void snand_dev_command_ext(SNAND_Mode mode, const U8 cmd[], U8 data[], const u32 outl, const u32 inl)
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
    snand_dev_mac_op(mode);

    // for NULL data, this loop will be skipped
    for (i = 0, p_data = ((P_U8)RW_SNAND_GPRAM_DATA + outl); i < inl; ++i, ++data, ++p_data)
    {
        *data = DRV_Reg8(p_data);
    }

    return;
}

static void snand_dev_command(const u32 cmd, u8 outlen)
{
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, outlen);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 0);
    snand_dev_mac_op(SPI);

    return;
}

static void snand_reset_dev()
{
    u8 cmd = SNAND_CMD_SW_RESET;

    // issue SW RESET command to device
    snand_dev_command_ext(SPI, &cmd, NULL, 1, 0);

    // wait for awhile, then polling status register (required by spec)
    snand_wait_us(SNAND_DEV_RESET_LATENCY_US);

    *RW_SNAND_GPRAM_DATA = (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8));
    *RW_SNAND_MAC_OUTL = 2;
    *RW_SNAND_MAC_INL = 1;

    // polling status register

    for (;;)
    {
        snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (cmd & SNAND_STATUS_OIP))
        {
            break;
        }
    }
}

static bool snand_reset_con(void)
{
    int timeout = 0xFFFF;

    // part 1. SNF

    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL | SNAND_SW_RST;
    *RW_SNAND_MISC_CTL = *RW_SNAND_MISC_CTL &= ~SNAND_SW_RST;

    // part 2. NFI

    if (DRV_Reg16(NFI_MASTERSTA_REG16)) // master is busy
    {
        DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

        while (DRV_Reg16(NFI_MASTERSTA_REG16))
        {
            timeout--;

            if (!timeout)
            {
                MSG(FUC, "Wait for NFI_MASTERSTA timeout\n");
            }
        }
    }
    /* issue reset operation */
    DRV_WriteReg16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

    return snand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) && snand_RFIFOValidSize(0) && snand_WFIFOValidSize(0);
}

//-------------------------------------------------------------------------------
static void snand_set_mode(u16 u2OpMode)
{
    u16 u2Mode = DRV_Reg16(NFI_CNFG_REG16);
    u2Mode &= ~CNFG_OP_MODE_MASK;
    u2Mode |= u2OpMode;
    DRV_WriteReg16(NFI_CNFG_REG16, u2Mode);
}

//-------------------------------------------------------------------------------
static void snand_set_autoformat(bool bEnable)
{
    if (bEnable)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    } else
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
    }
}

//-------------------------------------------------------------------------------
static void snand_configure_fdm(u16 u2FDMSize)
{
    NFI_CLN_REG16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_SHIFT);
    NFI_SET_REG16(NFI_PAGEFMT_REG16, u2FDMSize << PAGEFMT_FDM_ECC_SHIFT);
}

//-------------------------------------------------------------------------------
static bool snand_check_RW_count(u16 u2WriteSize)
{
    u32 timeout = 0xFFFF;
    u16 u2SecNum = u2WriteSize >> 9;

    while (ADDRCNTR_CNTR(DRV_Reg16(NFI_ADDRCNTR_REG16)) < u2SecNum)
    {
        timeout--;
        if (0 == timeout)
        {
            return false;
        }
    }
    return true;
}

static u32 snand_reverse_byte_order(u32 num)
{
   u32 ret = 0;

   ret |= ((num >> 24) & 0x000000FF);
   ret |= ((num >> 8)  & 0x0000FF00);
   ret |= ((num << 8)  & 0x00FF0000);
   ret |= ((num << 24) & 0xFF000000);

   return ret;
}

static u32 snand_gen_c1a3(const u32 cmd, const u32 address)
{
    return ((snand_reverse_byte_order(address) & 0xFFFFFF00) | (cmd & 0xFF));
}

static void snand_dev_enable_spiq(bool enable)
{
    u8   regval;
    u32  cmd;

    // read QE in status register
    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_OTP << 8);
    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    snand_dev_mac_op(SPI);

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

    snand_dev_mac_op(SPI);
}

//-------------------------------------------------------------------------------
static bool snand_ready_for_read(struct nand_chip *nand, u32 u4RowAddr, u32 u4ColAddr, u32 u4SecNum, bool bFull, u8 * buf)
{
    /* Reset NFI HW internal state machine and flush NFI in/out FIFO */
    bool        bRet = false;
    u32         cmd;
    u32         reg;
    SNAND_Mode  mode = SPIQ;

    if (!snand_reset_con())
    {
        goto cleanup;
    }

    // 1. Read page to cache

    cmd = snand_gen_c1a3(SNAND_CMD_PAGE_READ, u4RowAddr); // PAGE_READ command + 3-byte address

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 1 + 3);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 0);

    snand_dev_mac_op(SPI);

    // 2. Get features (status polling)

    cmd = SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8);

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, cmd);
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    for (;;)
    {
        snand_dev_mac_op(SPI);

        cmd = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if ((cmd & SNAND_STATUS_OIP) == 0)
        {
            if (SNAND_STATUS_TOO_MANY_ERROR_BITS == (cmd & SNAND_STATUS_ECC_STATUS_MASK))
            {
                bRet = FALSE;
            }

            break;
        }
    }

    //------ SNF Part ------

    // set PAGE READ command & address
    reg = (SNAND_CMD_PAGE_READ << SNAND_PAGE_READ_CMD_OFFSET) | (u4RowAddr & SNAND_PAGE_READ_ADDRESS_MASK);
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
        snand_dev_enable_spiq(TRUE);

        reg = DRV_Reg32(RW_SNAND_RD_CTL2);
        reg &= ~SNAND_DATA_READ_CMD_MASK;
        reg |= SNAND_CMD_RANDOM_READ_SPIQ & SNAND_DATA_READ_CMD_MASK;
        DRV_WriteReg32(RW_SNAND_RD_CTL2, reg);
    }

    // set DATA READ address
    DRV_WriteReg32(RW_SNAND_RD_CTL3, (u4ColAddr & SNAND_DATA_READ_ADDRESS_MASK));

    // set SNF data length (set in snand_xxx_read_data)

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

    reg = u4SecNum * (NAND_SECTOR_SIZE + g_snand_spare_per_sector);

    DRV_WriteReg32(RW_SNAND_MISC_CTL2, (reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)reg);

    //------ NFI Part ------

    snand_set_mode(CNFG_OP_CUST);
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, u4SecNum << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_SPIDMA_REG32, 0);

    if (bFull)  // read AUTO_FMT data
    {
#if USE_AHB_MODE
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
#else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif
        DRV_WriteReg32(NFI_STRADDR_REG32, buf);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }
    else  // read raw data by MCU
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
    }

    snand_set_autoformat(bFull);

    if (bFull)
    {
        snand_ecc_decode_start();
    }

    bRet = true;

  cleanup:
    return bRet;
}

//-----------------------------------------------------------------------------
static bool snand_ready_for_write(struct nand_chip *nand, u32 u4RowAddr, u8 * buf)
{
    bool        bRet = false;
    u16         sec_num = 1 << (nand->page_shift - 9);
    u32         reg;
    SNAND_Mode  mode = SPIQ;

    if (!snand_reset_con())
    {
        return false;
    }

    // 1. Write Enable
    snand_dev_command(SNAND_CMD_WRITE_ENABLE, 1);

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
        snand_dev_enable_spiq(TRUE);
    }

    // set program load address
    DRV_WriteReg32(RW_SNAND_PG_CTL2, 0 & SNAND_PG_LOAD_ADDR_MASK);  // col_addr = 0

    // set program execution address
    DRV_WriteReg32(RW_SNAND_PG_CTL3, u4RowAddr);

    // set SNF data length  (set in snand_xxx_write_data)

    // set SNF timing
    reg = DRV_Reg32(RW_SNAND_MISC_CTL);

    reg |= SNAND_PG_LOAD_CUSTOM_EN;

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

    reg = sec_num * (NAND_SECTOR_SIZE + g_snand_spare_per_sector);

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)reg);

    // set SNF data length
    DRV_WriteReg32(RW_SNAND_MISC_CTL2, reg | (reg << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET));

    //------ NFI Part ------

    // reset NFI
    snand_reset_con();

    snand_set_mode(CNFG_OP_PRGM);

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
    DRV_WriteReg16(NFI_CON_REG16, sec_num << CON_NFI_SEC_SHIFT);

    DRV_WriteReg32(NFI_SPIDMA_REG32, 0);

#if USE_AHB_MODE
    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
    DRV_WriteReg32(NFI_STRADDR_REG32, buf);
#else
    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_AHB);
#endif

    NFI_SET_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);

    snand_set_autoformat(true);

    snand_ecc_encode_start();

    bRet = true;

    return bRet;
}

//-----------------------------------------------------------------------------
static bool snand_dma_read_data(u8 * pDataBuf, u32 num_sec)
{
    u32 timeout = 0xFFFF;

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_AHB_DONE_EN);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

    if (en_interrupt)
    {
    	if(event_wait_timeout(&nand_int_event,100))
    	{
    		printf("[snand_dma_read_data]wait for AHB done timeout\n");
    		dump_nfi();
    		return false;
    	}

    	timeout = 0xFFFF;

	    while (num_sec > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
	    {
    		timeout--;

    		if (0 == timeout)
    		{
    		    return false;       //4
    		}
	    }
    }
    else
    {
        while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
        {
            timeout--;

            if (0 == timeout)
            {
                return false;
            }
        }

        timeout = 0xFFFF;

        while (num_sec > ((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12))
        {
            timeout--;

            if (0 == timeout)
            {
                return false;       //4
            }
        }
    }
    return true;
}

static bool snand_mcu_read_data(u8 * pDataBuf, u32 length, bool full)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32;
    u32 snf_len;

    // set SNF data length
	if (full)
	{
		snf_len = length + g_nand_chip.oobsize;
	}
	else
	{
		snf_len = length;
	}

    DRV_WriteReg32(RW_SNAND_MISC_CTL2, (snf_len | (snf_len << SNAND_PROGRAM_LOAD_BYTE_LEN_OFFSET)));

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYREAD);

    if (length % 4)
    {
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }
    else
    {
	    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    }

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BRD);

    pBuf32 = (u32 *) pDataBuf;

    if (length % 4)
    {
        for (i = 0; (i < length) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pDataBuf++ = DRV_Reg8(NFI_DATAR_REG32);
            i++;

        }
    } else
    {
        WAIT_NFI_PIO_READY(timeout);
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
        {
            WAIT_NFI_PIO_READY(timeout);
            *pBuf32++ = DRV_Reg32(NFI_DATAR_REG32);
            i++;
        }
    }
    return true;
}

static bool snand_read_page_data(u8 * buf, u32 num_sec)
{
#if USE_AHB_MODE
    return snand_dma_read_data(buf, num_sec);
#else
    return snand_mcu_read_data(buf, (num_sec * (NAND_SECTOR_SIZE + (g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE)))), true);
#endif
}

static bool snand_dma_write_data(u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, INTR_CUSTOM_PROG_DONE_INTR_EN);

    if ((unsigned int)buf % 16)
    {
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
    }
    else
    {
        //NFI_SET_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_DMA_BURST_EN); // Stanley Chu
    }

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);

    if (en_interrupt)
    {
    	if (event_wait_timeout(&nand_int_event,100))
    	{
    		printf("[snand_dma_write_data]wait for AHB done timeout\n");
    		dump_nfi();

            return false;
    	}
    }
    else
    {
        //while (!(DRV_Reg16(NFI_INTR_REG16) & INTR_AHB_DONE))
        while (!(DRV_Reg32(RW_SNAND_STA_CTL1) & SNAND_CUSTOM_PROGRAM))  // for custom program, wait RW_SNAND_STA_CTL1's SNAND_CUSTOM_PROGRAM done to ensure all data are loaded to device buffer
        {
            timeout--;

            if (0 == timeout)
            {
                printf("wait write AHB done timeout\n");
                dump_nfi();

    	        return FALSE;
    		}
        }
    }

    return true;
}

static bool snand_mcu_write_data(const u8 * buf, u32 length)
{
    u32 timeout = 0xFFFF;
    u32 i;
    u32 *pBuf32 = (u32 *) buf;

    NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    // set dummy command to trigger NFI enter custom mode
    DRV_WriteReg16(NFI_CMD_REG16, NAND_CMD_DUMMYPROG);

    NFI_SET_REG16(NFI_CON_REG16, CON_NFI_BWR);

    if ((u32) buf % 4 || length % 4)
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);
    else
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_BYTE_RW);

    if ((u32) buf % 4 || length % 4)
    {
        for (i = 0; (i < (length)) && (timeout > 0);)
        {
            if (DRV_Reg16(NFI_PIO_DIRDY_REG16) & 1)
            {
                DRV_WriteReg32(NFI_DATAW_REG32, *buf++);
                i++;
            } else
            {
                timeout--;
            }
            if (0 == timeout)
            {
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    } else
    {
        for (i = 0; (i < (length >> 2)) && (timeout > 0);)
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
                printf("[%s] nand mcu write timeout\n", __FUNCTION__);
                dump_nfi();
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
static bool snand_write_page_data(u8 * buf, u32 length)
{
#if USE_AHB_MODE
    return snand_dma_write_data(buf, length);
#else
    return snand_mcu_write_data(buf, length);
#endif
}

static void snand_read_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        *pBuf32++ = DRV_Reg32(NFI_FDM0L_REG32 + (i << 1));
        *pBuf32++ = DRV_Reg32(NFI_FDM0M_REG32 + (i << 1));
    }
}

static void snand_write_fdm_data(u8 * pDataBuf, u32 u4SecNum)
{
    u32 i;
    u32 *pBuf32 = (u32 *) pDataBuf;
    for (i = 0; i < u4SecNum; ++i)
    {
        DRV_WriteReg32(NFI_FDM0L_REG32 + (i << 1), *pBuf32++);
        DRV_WriteReg32(NFI_FDM0M_REG32 + (i << 1), *pBuf32++);
    }
}

static void snand_stop_read(void)
{
    //------ NFI Part

    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BRD);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    DRV_WriteReg32(RW_SNAND_STA_CTL1, SNAND_CUSTOM_READ);
    DRV_WriteReg32(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    NFI_CLN_REG32(RW_SNAND_MISC_CTL, SNAND_DATARD_CUSTOM_EN);

    snand_ecc_decode_end();
}

static void snand_stop_write(void)
{
    //------ NFI Part

    NFI_CLN_REG16(NFI_CON_REG16, CON_NFI_BWR);

    //------ SNF Part

    // set 1 then set 0 to clear done flag
    DRV_WriteReg32(RW_SNAND_STA_CTL1, SNAND_CUSTOM_PROGRAM);
    DRV_WriteReg32(RW_SNAND_STA_CTL1, 0);

    // clear essential SNF setting
    NFI_CLN_REG32(RW_SNAND_MISC_CTL, SNAND_PG_LOAD_CUSTOM_EN);

    snand_dev_enable_spiq(FALSE);

    snand_ecc_encode_end();
}

static bool snand_check_dececc_done(u32 u4SecNum)
{
    u32 timeout, dec_mask;
    timeout = 0xffff;
    dec_mask = (1 << u4SecNum) - 1;

    while ((dec_mask != DRV_Reg(ECC_DECDONE_REG16)) && timeout > 0)
    {
        timeout--;

        if (timeout == 0)
        {
            MSG(ERR, "ECC_DECDONE: timeout\n");
            dump_nfi();

            return false;
        }
    }

    return true;
}

static bool snand_read_page_part2(u32 row_addr, u32 num_sec, u8 * buf)
{
    bool    bRet = true;
    u32     reg;
    u32     col_part2, i, len;
    u32     spare_per_sector;
    P_U8    buf_part2;
    u32     timeout = 0xFFFF;
    u32     old_dec_mode = 0;

    spare_per_sector = g_nand_chip.oobsize / (g_nand_chip.page_size / NAND_SECTOR_SIZE);

    arch_clean_invalidate_cache_range((addr_t)buf, (size_t)(NAND_SECTOR_SIZE + spare_per_sector));

    for (i = 0; i < 2 ; i++)
    {
        snand_reset_con();

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

        snand_set_mode(CNFG_OP_CUST);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_READ_EN);
        NFI_SET_REG16(NFI_CNFG_REG16, CNFG_AHB);
        NFI_CLN_REG16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        snand_set_autoformat(FALSE);

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
                return FALSE;
            }
        }

        timeout = 0xFFFF;

        while (((DRV_Reg16(NFI_BYTELEN_REG16) & 0xf000) >> 12) != 1)
        {
            timeout--;

            if (0 == timeout)
            {
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

    return bRet;
}


int nand_exec_read_page_hw(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
    u32 u4SecNum = u4PageSize >> 9;
    u32 i;
    u8  * p_buf_local;

    snand_rs_reconfig_nfiecc(u4RowAddr);

    if (snand_rs_if_require_split())
    {
        u4SecNum--;
    }

    //MSG(INFO, "[nand_exec_read_page_hw] r:%d,b:%d,s:%d,part:%d,spare:%d\n", u4RowAddr, u4RowAddr / 64, u4SecNum, g_snand_rs_cur_part, g_snand_spare_per_sector);

    if (snand_ready_for_read(nand, u4RowAddr, 0, u4SecNum, true, pPageBuf))
    {
        if (!snand_read_page_data(pPageBuf, u4SecNum))
        {
            bRet = false;
        }
        if (!snand_status_ready(STA_NAND_BUSY))
        {
            bRet = false;
        }

        if (!snand_check_dececc_done(u4SecNum))
        {
            bRet = false;
        }

        snand_read_fdm_data(pFDMBuf, u4SecNum);

        if (!snand_check_bch_error(pPageBuf, u4SecNum - 1, u4RowAddr))
        {
            g_i4ErrNum++;
        }

        snand_stop_read();
    }

    if (snand_rs_if_require_split())
    {
        // read part II

        u4SecNum++;

        // note: use local temp buffer to read part 2
        if (!snand_read_page_part2(u4RowAddr, u4SecNum, g_snand_temp))
        {
            bRet = false;
        }

        // copy data

        p_buf_local = pPageBuf + NAND_SECTOR_SIZE * (u4SecNum - 1);

        for (i = 0; i < NAND_SECTOR_SIZE / sizeof(u32); i++)
        {
            ((u32 *)p_buf_local)[i] = ((u32 *)g_snand_temp)[i];
        }

        // copy FDM data

        p_buf_local = pFDMBuf + NAND_FDM_PER_SECTOR * (u4SecNum - 1);

        for (i = 0; i < NAND_FDM_PER_SECTOR / sizeof(u32); i++)
        {
            ((u32 *)p_buf_local)[i] = ((u32 *)g_snand_temp)[i + (NAND_SECTOR_SIZE / sizeof(u32))];
        }
    }

    snand_dev_enable_spiq(FALSE);

    return bRet;
}

static bool snand_exec_read_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = u4RowAddr / page_per_block;
    int page_in_block = u4RowAddr % page_per_block;
    int mapped_block;
    int i, start, len, offset;
    struct nand_oobfree *free;
    u8 oob[0x80];

    mapped_block = get_mapping_block_index(block);

    if (!nand_exec_read_page_hw(nand, (mapped_block * page_per_block + page_in_block), u4PageSize, pPageBuf, oob))
        return false;

    offset = 0;
    free = nand->ecclayout->oobfree;

    for (i = 0;  i < MTD_MAX_OOBFREE_ENTRIES&&free[i].length; i++)
    {
        start = free[i].offset;
        len = free[i].length;
        memcpy(pFDMBuf + offset, oob + start, len);
        offset += len;
    }

    return false;
}

static bool snand_dev_program_execute(u32 page)
{
    u32 cmd;
    u8  reg8;
    bool bRet = TRUE;

    // 3. Program Execute

    cmd = snand_gen_c1a3(SNAND_CMD_PROGRAM_EXECUTE, page);

    snand_dev_command(cmd, 4);

    DRV_WriteReg32(RW_SNAND_GPRAM_DATA, (SNAND_CMD_GET_FEATURES | (SNAND_CMD_FEATURES_STATUS << 8)));
    DRV_WriteReg32(RW_SNAND_MAC_OUTL, 2);
    DRV_WriteReg32(RW_SNAND_MAC_INL , 1);

    while (1)
    {
        snand_dev_mac_op(SPI);

        reg8 = DRV_Reg8(((P_U8)RW_SNAND_GPRAM_DATA + 2));

        if (0 == (reg8 & SNAND_STATUS_OIP)) // ready
        {
            if (0 != (reg8 & SNAND_STATUS_PROGRAM_FAIL)) // ready but having fail report from device
            {
            	printf("[snand] snand_dev_program_execute: prog failed\n");	// Stanley Chu

                bRet = FALSE;
            }

            break;
        }
    }

    return bRet;
}

bool snand_is_vendor_reserved_blocks(u32 row_addr)
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

static bool snand_exec_write_page(struct nand_chip *nand, u32 u4RowAddr, u32 u4PageSize, u8 * pPageBuf, u8 * pFDMBuf)
{
    bool bRet = true;
    u32 u4SecNum = u4PageSize >> 9;

    if (TRUE == snand_is_vendor_reserved_blocks(u4RowAddr))
    {
        return FALSE;
    }

    snand_rs_reconfig_nfiecc(u4RowAddr);

    if (snand_ready_for_write(nand, u4RowAddr, pPageBuf))
    {
        snand_write_fdm_data(pFDMBuf, u4SecNum);

        if (!snand_write_page_data(pPageBuf, u4PageSize))
        {
            bRet = false;
        }

        if (!snand_check_RW_count(u4PageSize))
        {
            bRet = false;
        }

        snand_stop_write();

        snand_dev_program_execute(u4RowAddr);
    }
    return bRet;
}

static bool snand_read_oob_raw(struct nand_chip *chip, u32 page_addr, u32 length, u8 * buf)
{
    bool bRet = true;

    // this API is called by nand_block_bad_hw() only! nand_block_bad_hw() will only check spare[0], thus we only need to read the 1st sector

    if (length > 32 || length % OOB_AVAIL_PER_SECTOR || !buf)
    {
        printf("[%s] invalid parameter, length: %d, buf: %p\n", __FUNCTION__, length, buf);
        return false;
    }

    snand_rs_reconfig_nfiecc(page_addr);

    //MSG(INFO, "[snand_read_oob_raw] r:%d,b:%d,s:1,part:%d,spare:%d\n", page_addr, page_addr / 64, g_snand_rs_cur_part, g_snand_spare_per_sector);

    // read the 1st sector (including its spare area) with MTK ECC enabled
    if (snand_ready_for_read(chip, page_addr, 0, 1, true, g_snand_temp))
    {
        if (!snand_read_page_data(g_snand_temp, 1))  // only read 1 sector
        {
            bRet = false;
        }
        if (!snand_status_ready(STA_NAND_BUSY))
        {
            bRet = false;
        }

        if (!snand_check_dececc_done(1))
        {
            bRet = false;
        }

        snand_read_fdm_data(g_snand_temp + NAND_SECTOR_SIZE, 1);

        if (!snand_check_bch_error(g_snand_temp, 1 - 1, page_addr))
        {
            g_i4ErrNum++;
        }

        snand_stop_read();
    }

    // copy spare[0] to oob buffer
    buf[0] = g_snand_temp[NAND_SECTOR_SIZE];

    return bRet;
}

bool nand_block_bad_hw(struct nand_chip * nand, u32 offset)
{
    u32 page_per_block = nand->erasesize / nand->page_size;
    u32 page_addr = offset >> nand->page_shift;
    u8 oob_buf[OOB_AVAIL_PER_SECTOR];

    if (TRUE == snand_is_vendor_reserved_blocks(page_addr))
    {
        return 1;   // return bad block for reserved blocks
    }

    memset(oob_buf, 0, OOB_AVAIL_PER_SECTOR);

    page_addr &= ~(page_per_block - 1);

    if (!snand_read_oob_raw(nand, page_addr, OOB_AVAIL_PER_SECTOR, oob_buf))
    {
        printf("snand_read_oob_raw return fail\n");
    }

    if (oob_buf[0] != 0xff)
    {
        printf("Bad block detect at block 0x%x, oob_buf[0] is %x\n", page_addr / page_per_block, oob_buf[0]);
        return true;
    }

    return false;
}

static bool snand_block_bad(struct nand_chip *nand, u32 page_addr)
{
    u32 page_per_block = 1 << (nand->phys_erase_shift - nand->page_shift);
    int block = page_addr / page_per_block;
    int mapped_block = get_mapping_block_index(block);

    return nand_block_bad_hw(nand, mapped_block << nand->phys_erase_shift);
}

static bool snand_erase_hw_rowaddr(u32 row_addr)
{
    bool bRet = TRUE;
    u32  reg;
    u32  polling_times;

    if (TRUE == snand_is_vendor_reserved_blocks(row_addr))
    {
        return FALSE;
    }

    snand_reset_con();

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

//not support un-block-aligned write
static int snand_part_write(part_dev_t * dev, uchar * src, ulong dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    u8 res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift) << 1;
    u32 u4BlkAddr = (dst >> nand->phys_erase_shift) << 1;
    u32 u4ColAddr = dst & (u4PageSize - 1);
    u32 u4RowAddr = dst >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4WriteLen = 0;
    int i4Len;
    u32 k = 0;

    for (k = 0; k < sizeof(g_kCMD.au1OOB); k++)
        *(g_kCMD.au1OOB + k) = 0xFF;

    while (((u32)size > u4WriteLen) && (u4BlkAddr < u4BlkEnd))
    {
        if (!u4ColAddr)
        {
            MSG(OPS, "Erase the block of 0x%08x\n", u4BlkAddr);

            snand_erase_hw_rowaddr(u4RowAddr);
        }

        res = snand_block_bad(nand, ((u4BlkAddr >> 1) * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min((u32)size - u4WriteLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && ((u32)i4Len == u4PageSize))
                {
                    snand_exec_write_page(nand, u4RowAddr, u4PageSize, src + u4WriteLen, g_kCMD.au1OOB);
                } else
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(nand->buffers->databuf + u4ColAddr, src + u4WriteLen, i4Len);
                    snand_exec_write_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                }
                u4WriteLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr++;
    }

    return (int)u4WriteLen;

}

static int snand_part_read(part_dev_t * dev, ulong source, uchar * dst, int size)
{
    struct nand_chip *nand = (struct nand_chip *)dev->blkdev;
    uint8_t res;
    u32 u4PageSize = 1 << nand->page_shift;
    u32 u4PageNumPerBlock = 1 << (nand->phys_erase_shift - nand->page_shift);
    u32 u4BlkEnd = (nand->chipsize >> nand->phys_erase_shift);
    u32 u4BlkAddr = (source >> nand->phys_erase_shift);
    u32 u4ColAddr = source & (u4PageSize - 1);
    u32 u4RowAddr = source >> nand->page_shift;
    u32 u4RowEnd;
    u32 u4ReadLen = 0;
    int i4Len;

    while (((u32)size > u4ReadLen) && (u4BlkAddr < u4BlkEnd))
    {
        res = snand_block_bad(nand, (u4BlkAddr * u4PageNumPerBlock));

        if (!res)
        {
            u4RowEnd = (u4RowAddr + u4PageNumPerBlock) & (~u4PageNumPerBlock + 1);
            for (; u4RowAddr < u4RowEnd; u4RowAddr++)
            {
                i4Len = min(size - u4ReadLen, u4PageSize - u4ColAddr);
                if (0 >= i4Len)
                {
                    break;
                }
                if ((u4ColAddr == 0) && ((u32)i4Len == u4PageSize))
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, dst + u4ReadLen, g_kCMD.au1OOB);
                } else
                {
                    snand_exec_read_page(nand, u4RowAddr, u4PageSize, nand->buffers->databuf, g_kCMD.au1OOB);
                    memcpy(dst + u4ReadLen, nand->buffers->databuf + u4ColAddr, i4Len);
                }
                u4ReadLen += i4Len;
                u4ColAddr = (u4ColAddr + i4Len) & (u4PageSize - 1);
            }
        } else
        {
            printf("Detect bad block at block 0x%x\n", u4BlkAddr);
            u4RowAddr += u4PageNumPerBlock;
        }
        u4BlkAddr++;
    }
    return (int)u4ReadLen;
}

static void snand_dev_read_id(u8 id[])
{
    u8 cmd = SNAND_CMD_READ_ID;

    snand_dev_command_ext(SPI, &cmd, id, 1, SNAND_MAX_ID + 1);
}

static void snand_command_bp(struct nand_chip *nand_chip, unsigned command, int column, int page_addr)
{
    switch (command)
    {
        case NAND_CMD_RESET:

            snand_reset_con();
            break;

        case NAND_CMD_READ_ID:

            snand_reset_con();
            snand_reset_dev();

            snand_dev_read_id(g_snand_id_data);
            g_snand_read_byte_mode = SNAND_RB_READ_ID;
            g_snand_id_data_idx = 1;

			printf("[SNAND] snand_command_bp(NAND_CMD_READID), ID:%x,%x\n", g_snand_id_data[1], g_snand_id_data[2]);

            break;

        default:
            printf("[SNAND] ERROR! snand_command_bp : unknow command %d\n", command);
            break;
    }
}

static u_char snand_read_byte(void)
{
    /* Check the PIO bit is ready or not */
    unsigned int timeout = TIMEOUT_4;

    if (SNAND_RB_READ_ID == g_snand_read_byte_mode)
    {
        if (g_snand_id_data_idx > SNAND_MAX_ID)
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
        WAIT_NFI_PIO_READY(timeout);

        return DRV_Reg8(NFI_DATAR_REG32);
    }
}

void lk_nand_irq_handler(unsigned int irq)
{
	u32 inte,sts;

	mt_irq_ack(irq);
	inte = DRV_Reg16(NFI_INTR_EN_REG16);
	sts = DRV_Reg16(NFI_INTR_REG16);
	//MSG(INT, "[lk_nand_irq_handler]irq %x enable:%x %x\n",irq,inte,sts);
	if(sts & inte){
	//	printf("[lk_nand_irq_handler]send event,\n");
		DRV_WriteReg16(NFI_INTR_EN_REG16, 0);
		DRV_WriteReg16(NFI_INTR_REG16,sts);
		event_signal(&nand_int_event,0);
	}
	return;
}

#define RW_GPIO_MODE6_SNAND         ((P_U32)(GPIO_BASE+0x0360)) // SFCK
#define RW_GPIO_MODE7_SNAND         ((P_U32)(GPIO_BASE+0x0370)) // SFWP, SFOUT, SFHOLD, SFIN, SFCS1
#define RW_GPIO_DRV0                ((P_U32)(0x1020B060))

static void snand_gpio_init(void)
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

int nand_init_device(struct nand_chip *nand)
{
    int index;
    u8 id[SNAND_MAX_ID] = {0};
    u32 spare_bit;
    u32 spare_per_sec;
    u32 ecc_bit;

    memset(&devinfo, 0, sizeof(devinfo));
    g_bInitDone = FALSE;
    g_kCMD.u4OOBRowAddr = (u32) - 1;

	snand_gpio_init();

#if defined(CONFIG_EARLY_LINUX_PORTING)		// FPGA NAND is placed at CS1
	DRV_WriteReg16(NFI_CSEL_REG16, 1);
#else
    DRV_WriteReg16(NFI_CSEL_REG16, NFI_DEFAULT_CS);
#endif

    //DRV_WriteReg32(NFI_ACCCON_REG32, NFI_DEFAULT_ACCESS_TIMING);
    DRV_WriteReg16(NFI_CNFG_REG16, 0);
    DRV_WriteReg16(NFI_PAGEFMT_REG16, 0);
    snand_reset_con();

    nand->nand_ecc_mode = NAND_ECC_HW;
    snand_command_bp(&g_nand_chip, NAND_CMD_READ_ID, 0, 0);

    MSG(INFO, "NAND ID: ");

    for (index = 0; index < SNAND_MAX_ID; index++)
    {
        id[index] = snand_read_byte();
        MSG(INFO, " %x", id[index]);
    }

    MSG(INFO, "\n ");

    if (!snand_get_device_info(id, &devinfo))
    {
        MSG(ERR, "NAND unsupport\n");
        return -1;
    }

    nand->name = devinfo.devicename;
    nand->chipsize = devinfo.totalsize << 20;
    nand->erasesize = devinfo.blocksize << 10;
    nand->phys_erase_shift = uffs(nand->erasesize) - 1;
    nand->page_size = devinfo.pagesize;
    nand->writesize = devinfo.pagesize;
    nand->page_shift = uffs(nand->page_size) - 1;
    nand->oobblock = nand->page_size;
    nand->bus16 = IO_WIDTH_4;
    nand->id_length = devinfo.id_length;

    for (index = 0; index < devinfo.id_length; index++)
    {
        nand->id[index] = id[index];
    }

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

	spare_per_sec = devinfo.sparesize >> (nand->page_shift - 9);

	//MSG(INFO, "devinfo.sparesize:%d, nand->page_shift:%d, spare_per_sec:%d", devinfo.sparesize, nand->page_shift, spare_per_sec);	// Stanley Chu

	if(spare_per_sec>=28){
		spare_bit = PAGEFMT_SPARE_28;
		ecc_bit = 12;
		spare_per_sec = 28;
	}else if(spare_per_sec>=27)
	{
		spare_bit = PAGEFMT_SPARE_27;
		ecc_bit = 8;
		spare_per_sec = 27;
	}else if(spare_per_sec>=26){
		spare_bit = PAGEFMT_SPARE_26;
		ecc_bit = 8;
		spare_per_sec = 26;
	}else if(spare_per_sec>=16){
		spare_bit = PAGEFMT_SPARE_16;
		ecc_bit = 4;
		spare_per_sec = 16;
	}else{
		printf("[NAND]: NFI not support oobsize: %x\n", spare_per_sec);
		while(1);
		return -1;
	}

    g_snand_spare_per_sector = spare_per_sec;
	g_snand_rs_ecc_bit_second_part = ecc_bit;

	devinfo.sparesize = spare_per_sec<<(nand->page_shift-9);
	MSG(INFO, "[NAND]nand eccbit %d , sparesize %d\n",ecc_bit,devinfo.sparesize);

    nand->oobsize = devinfo.sparesize;

    nand->buffers = malloc(sizeof(struct nand_buffers));

    if (nand->oobblock == 4096)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
        nand->ecclayout = &nand_oob_128;
    } else if (nand->oobblock == 2048)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
        nand->ecclayout = &nand_oob_64;
    } else if (nand->oobblock == 512)
    {
        NFI_SET_REG16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_512);
        nand->ecclayout = &nand_oob_16;
    }

    g_snand_rs_spare_per_sector_second_part_nfi = (spare_bit << PAGEFMT_SPARE_SHIFT);

    if (nand->nand_ecc_mode == NAND_ECC_HW)
    {
        NFI_SET_REG32(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
        snand_ecc_config(ecc_bit);
        snand_configure_fdm(NAND_FDM_PER_SECTOR);
    }

    DRV_Reg16(NFI_INTR_REG16);
    DRV_WriteReg16(NFI_INTR_EN_REG16, 0);

	if (en_interrupt)
	{
		event_init(&nand_int_event,false,EVENT_FLAG_AUTOUNSIGNAL);
		mt_irq_set_sens(MT_NFI_IRQ_ID, MT65xx_EDGE_SENSITIVE);
		mt_irq_set_polarity(MT_NFI_IRQ_ID, MT65xx_POLARITY_LOW);
		mt_irq_unmask(MT_NFI_IRQ_ID);
	}

    g_nand_size = nand->chipsize;
    nand->chipsize -= nand->erasesize * (BMT_POOL_SIZE);

    // init read split boundary
    //g_snand_rs_num_page = SNAND_RS_BOUNDARY_BLOCK * (nand->erasesize / nand->page_size);
    g_snand_rs_num_page = SNAND_RS_BOUNDARY_KB * 1024 / nand->page_size;

    g_bInitDone = true;

    if (!g_bmt)
    {
        if (!(g_bmt = init_bmt(nand, BMT_POOL_SIZE)))
        {
            MSG(INIT, "Error: init bmt failed\n");
            return -1;
        }
    }

    return 0;

}

void nand_init(void)
{
    static part_dev_t dev;

    if (!nand_init_device(&g_nand_chip))
    {
        struct nand_chip *t_nand = &g_nand_chip;
        printf("NAND init done in LK\n");
	    total_size = t_nand->chipsize - t_nand->erasesize * (PMT_POOL_SIZE);
        dev.id = 0;
        dev.init = 1;
        dev.blkdev = (block_dev_desc_t *) t_nand;
        dev.read = snand_part_read;
        dev.write = snand_part_write;
        mt_part_register_device(&dev);
        printf("NAND register done in LK\n");
        return;
    } else
    {
        printf("NAND init fail in LK\n");
    }

}

void nand_driver_test(void){
#ifdef NAND_LK_TEST
    u32 test_len=4096;
    long len;
    int fail=0;
    u32 index = 0;
    part_dev_t *dev = mt_part_get_device();
    part_t *part = mt_part_get_partition(PART_LOGO);
    unsigned long start_addr = part->startblk * BLK_SIZE;
    u8 *original = malloc(test_len);
    u8 *source = malloc(test_len);
    u8 *readback = malloc(test_len);

    for (index = 0; index < test_len; index++)
    {
        source[index] = index % 16;
    }
    memset(original, 0x0a, test_len);
    memset(readback, 0x0b, test_len);
    printf("~~~~~~~~~nand driver test in lk~~~~~~~~~~~~~~\n");
    len = dev->read(dev, start_addr, (uchar *) original, test_len);
    if (len != test_len)
    {
        printf("read original fail %d\n", len);
    }
    printf("oringinal data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", original[index]);
    }
    printf("\n");
    len = dev->write(dev, (uchar *) source, start_addr, test_len);
    if (len != test_len)
    {
        printf("write source fail %d\n", len);
    }
    len = dev->read(dev, start_addr, (uchar *) readback,test_len);
    if (len != test_len)
    {
        printf("read back fail %d\n", len);
    }
    printf("readback data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", readback[index]);
    }
    printf("\n");
    for (index = 0; index < test_len; index++)
    {
        if (source[index] != readback[index])
        {
            printf("compare fail %d\n", index);
		fail=1;
            break;
        }
    }
    if(fail==0){
	printf("compare success!\n");
    }
    len = dev->write(dev, (uchar *) original, start_addr, test_len);
    if (len != test_len)
    {
        printf("write back fail %d\n", len);
    } else
    {
        printf("recovery success\n");
    }
	memset(original,0xd,test_len);
	len = dev->read(dev, start_addr, (uchar *) original, test_len);
    if (len != test_len)
    {
        printf("read original fail %d\n", len);
    }
    printf("read back oringinal data:");
    for (index = 0; index < 300; index++)
    {
        printf(" %x", original[index]);
    }
    printf("\n");
    printf("~~~~~~~~~nand driver test in lk~~~~~~~~~~~~~~\n");
    free(original);
    free(source);
    free(readback);
#endif
}

/******************** ***/
/*    support for fast boot    */
/***********************/
int nand_erase(u64 offset, u64 size)
{
    u32 img_size = (u32)size;
    u32 tblksz;
    u32 cur_offset;
	u32 block_size = g_nand_chip.erasesize;

    // do block alignment check
    if ((u32)offset % block_size != 0)
    {
        printf("offset must be block alignment (0x%x)\n", block_size);
        return -1;
    }

    // calculate block number of this image
    if ((img_size % block_size) == 0)
    {
        tblksz = img_size / block_size;
    }
    else
    {
        tblksz = (img_size / block_size) + 1;
    }

    printf ("[ERASE] image size = 0x%x\n", img_size);
    printf ("[ERASE] the number of nand block of this image = %d\n", tblksz);

    // erase nand block
    cur_offset = (u32)offset;
    while (tblksz != 0)
    {
        if (__nand_erase(cur_offset) == FALSE)
        {
            printf("[ERASE] erase 0x%x fail\n",cur_offset);
            mark_block_bad (cur_offset);

        }
        cur_offset += block_size;

        tblksz--;

        if (tblksz != 0 && cur_offset >= total_size)
        {
            printf("[ERASE] cur offset (0x%x) exceeds erase limit address (0x%x)\n", cur_offset, total_size);
            return 0;
        }
    }


    return 0;

}
bool __nand_erase (u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    if (!nand_erase_hw(mapped_block * g_nand_chip.erasesize))
    {
        printf("erase block 0x%x failed\n", mapped_block);
        if(update_bmt(mapped_block * g_nand_chip.erasesize, UPDATE_ERASE_FAIL, NULL, NULL)){
			printf("erase block fail and update bmt sucess\n");
			return TRUE;
		}else{
			printf("erase block 0x%x failed but update bmt fail\n",mapped_block);
			return FALSE;
		}
    }

    return TRUE;
}
static int erase_fail_test = 0;
bool nand_erase_hw (u32 offset)
{
    bool bRet = TRUE;
    u32 page_addr = offset / g_nand_chip.oobblock;

    if (nand_block_bad_hw(&g_nand_chip,offset))
    {
        return FALSE;
    }

	if (erase_fail_test)
	{
		erase_fail_test = 0;
		return FALSE;
	}

    bRet = snand_erase_hw_rowaddr(page_addr);

    return bRet;
}

bool mark_block_bad (u32 logical_addr)
{
    int block = logical_addr / g_nand_chip.erasesize;
    int mapped_block = get_mapping_block_index(block);

    return mark_block_bad_hw(mapped_block * g_nand_chip.erasesize);
}

bool mark_block_bad_hw(u32 offset)
{
    u32 index;
	unsigned char buf[4096];
	unsigned char spare_buf[64];
    u32 page_addr = offset / g_nand_chip.oobblock;
    u32 u4SecNum = g_nand_chip.oobblock >> 9;
    int i, page_num = (g_nand_chip.erasesize / g_nand_chip.oobblock);

    memset(buf,0xFF,4096);

    for (index = 0; index < 64; index++)
        *(spare_buf + index) = 0xFF;

    for (index = 8, i = 0; (u32)i < u4SecNum; i++)
        spare_buf[i * index] = 0x0;

    page_addr &= ~(page_num - 1);
    MSG (INFO, "Mark bad block at 0x%x\n", page_addr);

	return snand_exec_write_page(&g_nand_chip, page_addr, g_nand_chip.oobblock, (u8 *)buf,(u8 *)spare_buf);
}
int nand_write_page_hw(u32 page, u8 *dat, u8 *oob)
{
    int i, j, start, len;
    bool empty = TRUE;
    u8 oob_checksum = 0;
    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && g_nand_chip.ecclayout->oobfree[i].length; i++)
    {
        /* Set the reserved bytes to 0xff */
        start = g_nand_chip.ecclayout->oobfree[i].offset;
        len = g_nand_chip.ecclayout->oobfree[i].length;
        for (j = 0; j < len; j++)
        {
            oob_checksum ^= oob[start + j];
            if (oob[start + j] != 0xFF)
                empty = FALSE;
        }
    }

    if (!empty)
    {
        oob[g_nand_chip.ecclayout->oobfree[i-1].offset + g_nand_chip.ecclayout->oobfree[i-1].length] = oob_checksum;
    }

	return snand_exec_write_page(&g_nand_chip, page, g_nand_chip.oobblock, (u8 *)dat,(u8 *)oob);

  }
int nand_write_page_hwecc (unsigned int logical_addr, char *buf, char *oob_buf)
{
	u32 page_size = g_nand_chip.oobblock;
	u32 block_size = g_nand_chip.erasesize;
	u32 block = logical_addr / block_size;
    u32 mapped_block = get_mapping_block_index(block);
	u32 pages_per_blk = (block_size/page_size);
    u32 page_in_block = (logical_addr/page_size)%pages_per_blk;

    int i;
    int start, len, offset;
    for (i = 0; (u32)i < sizeof(g_spare_buf); i++)
        *(g_spare_buf + i) = 0xFF;

    offset = 0;

	if(oob_buf != NULL){
	    for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && g_nand_chip.ecclayout->oobfree[i].length; i++)
	    {
	        /* Set the reserved bytes to 0xff */
	        start = g_nand_chip.ecclayout->oobfree[i].offset;
	        len = g_nand_chip.ecclayout->oobfree[i].length;
	        memcpy ((g_spare_buf + start), (oob_buf + offset), len);
	        offset += len;
	    }
	}

    // write bad index into oob
    if (mapped_block != block)
    {
        // MSG(INIT, "page: 0x%x\n", page_in_block);
        set_bad_index_to_oob(g_spare_buf, block);
    }
    else
    {
        set_bad_index_to_oob(g_spare_buf, FAKE_INDEX);
    }

    if (!nand_write_page_hw(page_in_block + mapped_block * pages_per_blk,
            (u8 *)buf, g_spare_buf))
    {
        MSG(INIT, "write fail happened @ block 0x%x, page 0x%x\n", mapped_block, page_in_block);
        return update_bmt( (page_in_block + mapped_block * pages_per_blk) * g_nand_chip.oobblock,
                UPDATE_WRITE_FAIL, (u8 *)buf, g_spare_buf);
    }

    return TRUE;
}

int nand_write_img(u32 addr, void *data, u32 img_sz,u32 partition_size,int img_type)
{
	unsigned int page_size = g_nand_chip.oobblock;
	unsigned int img_spare_size = 0,write_size;
	unsigned int block_size = g_nand_chip.erasesize;
	unsigned int partition_end = addr + partition_size;
	bool ret;
	unsigned int b_lastpage = 0;
	printf("[nand_wite_img]write to addr %x,img size %x\n",addr,img_sz);
	if(addr % block_size || partition_size % block_size){
		printf("[nand_write_img]partition address or partition size is not block size alignment\n");
		return -1;
	}
	if(img_sz > partition_size){
		printf("[nand_write_img]img size %x exceed partition size\n",img_sz);
		return -1;
	}
	if(page_size == 4096){
		img_spare_size = 128;
	}else if(page_size == 2048){
		img_spare_size = 64;
	}

	if(img_type == YFFS2_IMG){
		write_size = page_size + img_spare_size;

		if(img_sz % write_size){
			printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
			return -1;
		}
	}else{
		write_size = page_size;
	/*	if(img_sz % write_size){
			printf("[nand_write_img]img size is not w_size %d alignment\n",write_size);
			return -1;
		}*/
	}

	while(img_sz>0){

		if((addr+img_sz)>partition_end){
			printf("[nand_wite_img]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,img_sz);
			return -1;
		}

		/*1. need to erase before write*/
		if((addr % block_size)==0){
			if (__nand_erase(addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad (addr);
           		addr += block_size;
				continue;  //erase fail, skip this block
       	 	}
		}
		/*2. write page*/
		if((img_sz < write_size)){
			b_lastpage = 1;
			memset(g_data_buf,0xff,write_size);
			memcpy(g_data_buf,data,img_sz);
			if(img_type == YFFS2_IMG){
				ret = nand_write_page_hwecc(addr, (char *)g_data_buf, (char *)g_data_buf + page_size);
			}else{
				if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size))){
					printf("[nand_write_img]skip empty page\n");
					ret = true;
				}else{
				ret = nand_write_page_hwecc(addr,(char *)g_data_buf,NULL);
			}
			}
		}else{
			if(img_type == YFFS2_IMG){
				ret = nand_write_page_hwecc(addr,data,data+page_size);
			}else{
				if((img_type == UBIFS_IMG)&& (check_data_empty((void *)data,page_size))){
					printf("[nand_write_img]skip empty page\n");
					ret = true;
				}else{
				ret = nand_write_page_hwecc(addr,data,NULL);
			}
		}
		}
		if(ret == FALSE){
			printf("[nand_write_img]write fail at % 0x%x\n",addr);
			if (__nand_erase(addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad (addr);
       	 	}
			data -= ((addr%block_size)/page_size)*write_size;
			img_sz += ((addr%block_size)/page_size)*write_size;
			addr += block_size;
			continue;  // write fail, try  to write the next block
		}
		if(b_lastpage){
			data += img_sz;
			img_sz = 0 ;
			addr += page_size;
		}else{
			data += write_size;
			img_sz -= write_size;
			addr += page_size;
		}
	}
	/*3. erase any block remained in partition*/
	addr = ((addr+block_size-1)/block_size)*block_size;

	nand_erase((u64)addr,(u64)(partition_end - addr));

	return 0;
}

int nand_write_img_ex(u32 addr, void *data, u32 length,u32 total_size, u32 *next_offset, u32 partition_start,u32 partition_size, int img_type)
{
	unsigned int page_size = g_nand_chip.oobblock;
	unsigned int img_spare_size = 0,write_size;
	unsigned int block_size = g_nand_chip.erasesize;
	unsigned int partition_end = partition_start + partition_size;
	//unsigned int first_chunk = 0;
	unsigned int last_chunk = 0;
	unsigned int left_size = 0;
	bool ret;
	u32 last_addr = addr;
	u32 dst_block = 0;
	printf("[nand_write_img_ex]write to addr %x,img size %x, img_type %d\n",addr,length,img_type);
	if(partition_start % block_size || partition_size % block_size){
		printf("[nand_write_img_ex]partition address or partition size is not block size alignment\n");
		return -1;
	}
	if(length > partition_size){
		printf("[nand_write_img_ex]img size %x exceed partition size\n",length);
		return -1;
	}


	if(page_size == 4096){
		img_spare_size = 128;
	}else if(page_size == 2048){
		img_spare_size = 64;
	}
    if(last_addr % page_size){
		printf("[nand_write_img_ex]write addr is not page_size %d alignment\n",page_size);
		return -1;
	}
	if(img_type == YFFS2_IMG){
		write_size = page_size + img_spare_size;
		if(total_size % write_size){
			printf("[nand_write_img_ex]total image size %d is not w_size %d alignment\n",total_size,write_size);
			return -1;
		}
	}else{
		write_size = page_size;
	}
	if(addr == partition_start){
		printf("[nand_write_img_ex]first chunk\n");
		//first_chunk = 1;
		download_size = 0;
		memset(g_data_buf,0xff,write_size);
	}
	if((length + download_size) >= total_size){
		printf("[nand_write_img_ex]last chunk\n");
		last_chunk = 1;
	}

	left_size = (download_size % write_size);

	while(length>0){

		if((addr+length)>partition_end){
			printf("[nand_write_img_ex]write to addr %x,img size %x exceed parition size,may be so many bad blocks\n",addr,length);
			return -1;
		}

		/*1. need to erase before write*/
		if((addr % block_size)==0){
			if (__nand_erase(addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad (addr);
           		addr += block_size;
				continue;  //erase fail, skip this block
       	 	}
		}
		if((length < write_size)&&(!left_size)){
			memset(g_data_buf,0xff,write_size);
			memcpy(g_data_buf,data,length);

			if(!last_chunk){
				download_size += length;
				break;
			}
		}else if(left_size){
			memcpy(&g_data_buf[left_size],data,write_size-left_size);

		}else{
			memcpy(g_data_buf,data,write_size);
		}

		/*2. write page*/

		if(img_type == YFFS2_IMG){
			ret = nand_write_page_hwecc(addr,(char *)g_data_buf,(char *)g_data_buf + page_size);
		}else{
			if((img_type == UBIFS_IMG)&& (check_data_empty((void *)g_data_buf,page_size))){
				printf("[nand_write_img_ex]skip empty page\n");
				ret = true;
			}
			else{
					ret = nand_write_page_hwecc(addr,(char *)g_data_buf,NULL);
			}
		}
		/*need to check?*/
		if(ret == FALSE){
			printf("[nand_write_img_ex]write fail at % 0x%x\n",addr);
			while(1){
				dst_block = find_next_good_block(addr/block_size);
				if(dst_block == 0)
				{
					printf("[nand_write_img_ex]find next good block fail\n");
					return -1;
				}
				ret = block_replace(addr/block_size,dst_block,addr/page_size);
				if(ret == FALSE){
					printf("[nand_write_img_ex]block replace fail,continue\n");
					continue;
				}else{
					printf("[nand_write_img_ex]block replace sucess %x--> %x\n",addr/block_size,dst_block);
					break;
				}

			}
			addr = (addr%block_size) + (dst_block*block_size);
		/*	if (__nand_erase(addr) == FALSE)
     		{
	            printf("[ERASE] erase 0x%x fail\n",addr);
	            mark_block_bad (addr);
       	 	}
			data -= ((addr%block_size)/page_size)*write_size;
			length += ((addr%block_size)/page_size)*write_size;
			addr += block_size;*/
			continue;  // write fail, try  to write the next block
		}
		if(left_size)
		{
			data += (write_size - left_size);
			length -= (write_size - left_size);
			addr += page_size;
			download_size += (write_size - left_size);
			left_size = 0;
		}
		else{
			data += write_size;
			length -= write_size;
			addr += page_size;
			download_size += write_size;
		}
	}
	*next_offset = addr - last_addr;
	if(last_chunk){
		/*3. erase any block remained in partition*/
		addr = ((addr+block_size-1)/block_size)*block_size;

		nand_erase((u64)addr,(u64)(partition_end - addr));
	}
	return 0;
}

int check_data_empty(void *data, unsigned size)
{
		int i;
		u32 *tp = (u32 *)data;

		for(i = 0; (u32)i < size / 4; i++){
			if(*(tp+i) != 0xffffffff){
				return 0;
			}
		}
		return 1;
}

static u32 find_next_good_block(u32 start_block)
{
	u32 i;
	u32 dst_block = 0;
	for(i=start_block;i<(total_size/g_nand_chip.erasesize);i++)
	{
		if(!snand_block_bad(&g_nand_chip,i*(g_nand_chip.erasesize/g_nand_chip.page_size))){
			dst_block = i;
			break;
		}
	}
	return dst_block;
}

static bool block_replace(u32 src_block, u32 dst_block, u32 error_page)
{
	bool ret;
	u32 block_size = g_nand_chip.erasesize;
	u32 page_size = g_nand_chip.page_size;
	int i;
	u8 *data_buf;
	u8 *spare_buf;
	ret = __nand_erase(dst_block*block_size);
	if(ret == FALSE){
		printf("[block_replace]%x-->%x erase fail\n",src_block,dst_block);
		mark_block_bad(src_block*block_size);
		return ret;
	}
	data_buf = malloc(4096);
	spare_buf = malloc(256);
	if(!data_buf || !spare_buf){
		printf("[block_replace]malloc mem fail\n");
		return -1;
	}

	memset(data_buf,0xff,4096);
	memset(spare_buf,0xff,256);
	for(i=0;(u32)i<error_page;i++)
	{
		snand_exec_read_page(&g_nand_chip,src_block*(block_size/page_size) + i,page_size,data_buf,spare_buf);
		ret = nand_write_page_hwecc(dst_block*block_size + i*page_size,(char *)data_buf,(char *)spare_buf);
		if(ret == FALSE)
			mark_block_bad(dst_block*block_size);
	}

	mark_block_bad(src_block*block_size);
	free(data_buf);
	free(spare_buf);
	return ret;

}

#endif
