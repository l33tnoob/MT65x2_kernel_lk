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

#include "msdc_cfg.h" //Include msdc_cfg.h for defining MMC_MSDC_DRV_PRELOADER

#if defined(MMC_MSDC_DRV_PRELOADER)
#include "mtk_nand_core.h"
#endif

#include "msdc.h"
//There are definition of MSG() in mtk_name_core.h and msdc_utils.h (indirectly included by msdc.h).
//The later included definition will be used for expansion of MSG in this file.
//Therefore, msdc.h shall be included after mtk_nand_core.h

#define MMC_HOST_ID                 0

#define BUF_BLK_NUM                 4   /* 4 * 512bytes = 2KB */

/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/

/**************************************************************************
*  MACRO DEFINITION
**************************************************************************/

/**************************************************************************
*  EXTERNAL DECLARATION
**************************************************************************/
#ifdef FEATURE_MMC_ADDR_TRANS
#include "addr_trans.h"
static addr_trans_info_t g_emmc_addr_trans[EMMC_PART_NUM];
static addr_trans_tbl_t g_addr_trans_tbl;

#if defined(MMC_MSDC_DRV_PRELOADER)
extern struct nand_chip g_nand_chip;
static blkdev_t g_mmc_bdev;
u64 g_emmc_size = 0;
#endif

#if defined(MMC_MSDC_DRV_LK)
typedef phys_addr_t_addtrans phys_addr_t;
typedef virt_addr_t_addtrans virt_addr_t;
int g_user_virt_addr=0;
u64 g_emmc_size = 0;
#endif

static int mmc_switch_part(u32 part_id)
{
    int err = MMC_ERR_NONE;
    struct mmc_card *card;
    struct mmc_host *host;
    u8 part = (u8)part_id;
    u8 cfg;
    u8 *ext_csd;

    card = mmc_get_card(MMC_HOST_ID);
    host = mmc_get_host(MMC_HOST_ID);

    if (!card)
        return MMC_ERR_INVALID;

    ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {

        if (part_id == EMMC_PART_USER)
            part = EXT_CSD_PART_CFG_DEFT_PART;

        cfg = card->raw_ext_csd[EXT_CSD_PART_CFG];

        /* already set to specific partition */
        if (part == (cfg & 0x7))
            return MMC_ERR_NONE;

        cfg = (cfg & ~0x7) | part;

        err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CFG, cfg);

        if (err == MMC_ERR_NONE) {
            err = mmc_read_ext_csd(host, card);
            if (err == MMC_ERR_NONE) {
                ext_csd = &card->raw_ext_csd[0];
                if (ext_csd[EXT_CSD_PART_CFG] != cfg)
                    err = MMC_ERR_FAILED;
            }
        }

        MSG(OPS_MMC, "[SD%d] Switch to Partition %d, Result: %d\n", host->id, part_id, err);

    }
    return err;
}

static int mmc_virt_to_phys(u32 virt_blknr, u32 *phys_blknr, u32 *part_id)
{
    int ret;
    virt_addr_t virt;
    phys_addr_t phys;

    virt.addr = virt_blknr;

    ret = virt_to_phys_addr(&g_addr_trans_tbl, &virt, &phys);
    #if defined(MMC_MSDC_DRV_PRELOADER)
    if (phys.id == -1)
    #elif defined(MMC_MSDC_DRV_LK)
    if (phys.id == 0)
    #endif
        phys.id = EMMC_PART_USER;

    *phys_blknr = (ret == 0) ? phys.addr : virt_blknr; /* in 512B unit */
    *part_id    = (ret == 0) ? phys.id : EMMC_PART_USER;

    return ret;
}

static int mmc_phys_to_virt(u32 phys_blknr, u32 part_id, u32 *virt_blknr)
{
    int ret;
    virt_addr_t virt;
    phys_addr_t phys;

    phys.addr = phys_blknr;
    phys.id   = part_id;

    ret = phys_to_virt_addr(&g_addr_trans_tbl, &phys, &virt);

    *virt_blknr = (ret == 0) ? virt.addr : phys_blknr; /* in 512B unit */

    return ret;
}

/* unit-test */
#if 0
void mmc_addr_trans_test(void)
{
    u32 i, virt_blknr, phys_blknr, part_id;

    virt_blknr = 0;
    for (i = 0; i < EMMC_PART_NUM; i++) {
        mmc_virt_to_phys(virt_blknr - 1, &phys_blknr, &part_id);
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr - 1, phys_blknr, part_id);
        mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr, phys_blknr, part_id);
        mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);
        printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
            virt_blknr + 1, phys_blknr, part_id);
        virt_blknr += g_emmc_addr_trans[i].len;
    }
    mmc_virt_to_phys(virt_blknr - 1, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr - 1, phys_blknr, part_id);
    mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr, phys_blknr, part_id);
    mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);
    printf("[EMMC] Virt: 0x%x --> Phys: 0x%x Part: %d\n",
        virt_blknr + 1, phys_blknr, part_id);

    phys_blknr = 0;
    for (i = 0; i < EMMC_PART_NUM; i++) {
        mmc_phys_to_virt(phys_blknr, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + 1, i, &virt_blknr);

        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len + 1, i, &virt_blknr);
    }
}
#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
static int mmc_addr_trans_tbl_init(struct mmc_card *card, blkdev_t *bdev)
#elif defined(MMC_MSDC_DRV_LK)
static int mmc_addr_trans_tbl_init(struct mmc_card *card)
#endif
{
    u32 wpg_sz;
    u8 *ext_csd;

    memset(&g_addr_trans_tbl, 0, sizeof(addr_trans_tbl_t));

    ext_csd = &card->raw_ext_csd[0];

    #if defined(MMC_MSDC_DRV_PRELOADER)
    bdev->offset = 0;
    #endif

    if (mmc_card_mmc(card) && ext_csd[EXT_CSD_REV] >= 3) {
        u64 size[EMMC_PART_NUM];
        u32 i;

        if ((ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN)
            && (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] > 0)) {
            wpg_sz = 512 * 1024 * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
                ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
        } else {
            wpg_sz = card->csd.write_prot_grpsz;
        }

        size[EMMC_PART_BOOT1] = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_BOOT2] = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_RPMB]  = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
        size[EMMC_PART_GP1]   = ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP1_SIZE_MULT + 0];
        size[EMMC_PART_GP2]   = ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP2_SIZE_MULT + 0];
        size[EMMC_PART_GP3]   = ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP3_SIZE_MULT + 0];
        size[EMMC_PART_GP4]   = ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] * 256 * 256 +
                                ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] * 256 +
                                ext_csd[EXT_CSD_GP4_SIZE_MULT + 0];
        size[EMMC_PART_USER]  = (u64)card->blklen * card->nblks;

        size[EMMC_PART_GP1] *= wpg_sz;
        size[EMMC_PART_GP2] *= wpg_sz;
        size[EMMC_PART_GP3] *= wpg_sz;
        size[EMMC_PART_GP4] *= wpg_sz;

        for (i = EMMC_PART_BOOT1; i < EMMC_PART_NUM; i++) {
            g_emmc_addr_trans[i].id  = i;
            g_emmc_addr_trans[i].len = size[i] / 512; /* in 512B unit */
            #if defined(MMC_MSDC_DRV_LK)
            if (i<EMMC_PART_USER)
                g_user_virt_addr += size[i];
            #endif
            g_emmc_size += size[i];
        }

        /* determine user area offset */
        #if defined(MMC_MSDC_DRV_PRELOADER)
        for (i = EMMC_PART_BOOT1; i < EMMC_PART_USER; i++) {
            bdev->offset += size[i];
        }
        bdev->offset /= bdev->blksz; /* in blksz unit */
        #endif

        g_addr_trans_tbl.num  = EMMC_PART_NUM;
        g_addr_trans_tbl.info = &g_emmc_addr_trans[0];
    } else {
        g_addr_trans_tbl.num  = 0;
        g_addr_trans_tbl.info = NULL;
    }

    return 0;
}
#define mmc_virt_switch(vbn,pbn)    \
do{ u32 pid; \
    if (mmc_virt_to_phys(vbn, pbn, &pid) == MMC_ERR_NONE) \
        mmc_switch_part(pid); \
}while(0)

#else
#if defined(MMC_MSDC_DRV_LK)
int g_user_virt_addr=0; //For building init.c. Check if it is used when FEATURE_MMC_ADDR_TRANS is not defined
u64 g_emmc_size = 0;    //For building init.c. Check if it is used when FEATURE_MMC_ADDR_TRANS is not defined
#endif

#define mmc_virt_to_phys(vbn,pbn,pid)       do{}while(0)
#define mmc_phys_to_virt(pbn,pid,vbn)       do{}while(0)
#define mmc_virt_switch(vbn,pbn)            do{}while(0)

#if defined(MMC_MSDC_DRV_PRELOADER)
#define mmc_addr_trans_tbl_init(card,bdev)  do{}while(0)
#elif defined(MMC_MSDC_DRV_LK)
#define mmc_addr_trans_tbl_init(card)  do{}while(0)
#endif

#endif

#if defined(MMC_MSDC_DRV_PRELOADER)
static int mmc_bread(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    struct mmc_host *host = (struct mmc_host*)bdev->priv;

    mmc_virt_switch(blknr, (u32*)&blknr);

    return mmc_block_read(host->id, (unsigned long)blknr, blks, (unsigned long*)buf);
}

static int mmc_bwrite(blkdev_t *bdev, u32 blknr, u32 blks, u8 *buf)
{
    struct mmc_host *host = (struct mmc_host*)bdev->priv;

    mmc_virt_switch(blknr, (u32*)&blknr);

    return mmc_block_write(host->id, (unsigned long)blknr, blks, (unsigned long*)buf);
}

// ==========================================================
// MMC Common Interface - Init
// ==========================================================
u32 mmc_init_device(void)
{
    int ret;
    struct mmc_card *card;

    if (!blkdev_get(BOOTDEV_SDMMC)) {

        ret = mmc_init(MMC_HOST_ID, MSDC_MODE_DEFAULT);
        if (ret != 0) {
            printf("[SD0] init card failed\n");
            return ret;
        }

        memset(&g_mmc_bdev, 0, sizeof(blkdev_t));

        card = mmc_get_card(MMC_HOST_ID);

        g_mmc_bdev.blksz   = MMC_BLOCK_SIZE;
        g_mmc_bdev.erasesz = MMC_BLOCK_SIZE;
        g_mmc_bdev.blks    = card->nblks;
        g_mmc_bdev.bread   = mmc_bread;
        g_mmc_bdev.bwrite  = mmc_bwrite;
        g_mmc_bdev.type    = BOOTDEV_SDMMC;
        g_mmc_bdev.blkbuf  = (u8*)STORAGE_BUFFER_ADDR;
        g_mmc_bdev.priv    = (void*)mmc_get_host(MMC_HOST_ID);

#ifdef FEATURE_MMC_ADDR_TRANS
        mmc_addr_trans_tbl_init(card, &g_mmc_bdev);
#endif

        blkdev_register(&g_mmc_bdev);
    }
    return 0;
}

u32 mmc_get_device_id(u8 *id, u32 len,u32 *fw_len)
{
    u8 buf[16]; /* CID = 128 bits */
    struct mmc_card *card;
	u8 buf_sanid[512];
    u32 i=0;
	u8 *bptr=buf;

    if (0 != mmc_init_device())
        return -1;

    card = mmc_get_card(MMC_HOST_ID);

    do {
        *bptr = (card->raw_cid[i] >> 24) & 0xFF; bptr++;
        *bptr = (card->raw_cid[i] >> 16) & 0xFF; bptr++;
        *bptr = (card->raw_cid[i] >> 8) & 0xFF; bptr++;
        *bptr = (card->raw_cid[i] >> 0) & 0xFF; bptr++;
        i++;
    } while(i<4 );

	*fw_len = 1;
	if( (buf[0] == 0x45) && (card->raw_ext_csd[EXT_CSD_REV]<=5) ) {
		if (0 == mmc_get_sandisk_fwid(MMC_HOST_ID,buf_sanid)){
			*fw_len = 6;
		}
	}
	len = len > 16 ? 16 : len;
    memcpy(id, buf, len);
	if(*fw_len == 6)
		memcpy(id+len,buf_sanid+32,6);
	else
		*(id+len) = buf[9];
    return 0;
}

#if CFG_LEGACY_USB_DOWNLOAD
// ==========================================================
// MMC Common Interface - Correct R/W Address
// ==========================================================
u32 mmc_find_safe_block (u32 offset)
{
    return offset;
}


// ==========================================================
// MMC Common Interface - Read Function
// ==========================================================
u32 mmc_read_data (u8 * buf, u32 offset)
{
    unsigned long blknr;
    u32 blks;
    int ret;

    blknr = offset / 512;
    blks = BUF_BLK_NUM;
    mmc_virt_switch(blknr, (u32*)&blknr);
    ret = mmc_block_read(MMC_HOST_ID, blknr, blks, (unsigned long *)buf);
    if (ret != MMC_ERR_NONE)
    {
        printf("[SD0] block read 0x%x failed\n", offset);
    }

    return offset;
}

// ==========================================================
// MMC Common Interface - Write Function
// ==========================================================
u32 mmc_write_data (u8 * buf, u32 offset)
{
    unsigned long blknr;
    u32 blks;
    int ret;

    blknr = offset / 512;
    blks = BUF_BLK_NUM; /* 2K bytes */
    mmc_virt_switch(blknr, (u32*)&blknr);
    ret = mmc_block_write(MMC_HOST_ID, blknr, blks, (unsigned long *)buf);
    if (ret != MMC_ERR_NONE)
    {
        printf("[SD0] block write 0x%x failed\n", offset);
    }
    return offset;
}

// ==========================================================
// MMC Common Interface - Erase Function
// ==========================================================
bool mmc_erase_data (u32 offset, u32 offset_limit, u32 size)
{
    /* Notice that the block size is different with different emmc.
    * Thus, it could overwrite other partitions while erasing data.
    * Don't implement it if you don't know the block size of emmc.
    */
    return TRUE;
}

// ==========================================================
// MMC Common Interface - Check If Device Is Ready To Use
// ==========================================================
void mmc_wait_ready (void)
{
    return;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation Body
// ==========================================================
u32 mmc_chksum_body (u32 chksm, char *buf, u32 pktsz)
{
    u32 i;

    // TODO : Correct It
    /* checksum algorithm body, simply exclusive or */
    for (i = 0; i < pktsz; i++)
        chksm ^= buf[i];

    return chksm;
}

// ==========================================================
// MMC Common Interface - Checksum Calculation
// ==========================================================
u32 mmc_chksum_per_file (u32 mmc_offset, u32 img_size)
{
    u32 now = 0, i = 0, chksm = 0, start_block = 0, total = 0;
    INT32 cnt;
    bool ret = TRUE;

    // TODO : Correct it. Don't use nand page size
    u32 start = mmc_offset;
    u32 pagesz = g_nand_chip.page_size;
    u32 pktsz = pagesz + g_nand_chip.oobsize;
    u8 *buf = (u8 *)STORAGE_BUFFER_ADDR;

    // clean the buffer
    memset (buf, 0x0, STORAGE_BUFFER_ADDR);

    // calculate the number of page
    total = img_size / pagesz;
    if (img_size % pagesz != 0)
    {
        total++;
    }

    // check the starting block is safe
    start_block = mmc_find_safe_block (start);
    if (start_block != start)
    {
        start = start_block;
    }

    // copy data from msdc to memory
    for (cnt = total, now = start; cnt >= 0; cnt--, now += pagesz)
    {
        /* read a packet */
        mmc_read_data (buf, now);
        chksm = mmc_chksum_body (chksm, buf, pktsz);
    }

    return chksm;
}
#endif
#endif


// ==========================================================
// MMC Common Interface - Init for LK
// ==========================================================
#if defined(MMC_MSDC_DRV_LK)
#include "part.h"
#include <mt_partition.h>
static block_dev_desc_t sd_dev[MSDC_MAX_NUM];
static int boot_dev_found = 0;
static part_dev_t boot_dev;

unsigned long mmc_wrap_bread(int dev_num, unsigned long blknr, lbaint_t blkcnt, void *dst)
{
    MSG(OPS_MMC, "[SD%d] Block Read Virtual Addr: %xh\n", dev_num, (unsigned int)blknr);

    #if defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    if (dev_num==0)
    #endif
    {
        mmc_virt_switch(blknr, (u32 *)&blknr);
    }

    return mmc_block_read(dev_num, blknr, blkcnt, (unsigned long *)dst) == MMC_ERR_NONE ? blkcnt : (unsigned long) -1;
}

unsigned long mmc_wrap_bwrite(int dev_num, unsigned long blknr, lbaint_t blkcnt, const void *src)
{
    MSG(OPS_MMC, "[SD%d] Block Write Virtual Addr: %xh\n", dev_num, (unsigned int)blknr);

    #if defined(FEATURE_MMC_MEM_PRESERVE_MODE)
    if (dev_num==0)
    #endif
    {
        mmc_virt_switch(blknr, (u32 *)&blknr);
    }

    return mmc_block_write(dev_num, blknr, blkcnt, (unsigned long *)src) == MMC_ERR_NONE ? blkcnt : (unsigned long) -1;
}

int mmc_legacy_init(int verbose)
{
    int id = verbose - 1;
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;
    block_dev_desc_t *bdev;

    bdev = &sd_dev[id];

    //msdc_hard_reset();

    err=mmc_init(id, MSDC_MODE_DEFAULT);

    if (err == MMC_ERR_NONE && !boot_dev_found) {
        /* fill in device description */
        card=mmc_get_card(id);
        host=mmc_get_host(id);
        #if defined(FEATURE_MMC_ADDR_TRANS)
        mmc_addr_trans_tbl_init(card);
        #endif

        bdev->dev         = id;
        bdev->blksz       = MMC_BLOCK_SIZE;
        bdev->lba         = card->nblks * card->blklen / MMC_BLOCK_SIZE;
        bdev->block_read  = mmc_wrap_bread;
        bdev->block_write = mmc_wrap_bwrite;

        #if defined(FEATURE_MMC_MEM_PRESERVE_MODE)
        if ( id==1 ) {
            host->boot_type = NON_BOOTABLE;
            return err;
        }
        #endif

        host->boot_type   = RAW_BOOT;

        /* FIXME. only one RAW_BOOT dev */
        if (host->boot_type == RAW_BOOT) {
            boot_dev.id = id;
            boot_dev.init = 1;
            boot_dev.blkdev = bdev;
            mt_part_register_device(&boot_dev);
            boot_dev_found = 1;
            printf("[SD%d] boot device found\n", id);
        } else if (host->boot_type == FAT_BOOT) {
            #if (CONFIG_COMMANDS & CFG_CMD_FAT)
            if (0 == fat_register_device(bdev, 1)) {
                boot_dev_found = 1;
                printf("[SD%d] FAT partition found\n", id);
            }
            #endif
        }
    }

    return err;
}
#endif


// ==========================================================
// MMC Common Interface - Erase
// ==========================================================
#if defined(MMC_MSDC_DRV_LK) || defined(MMC_MSDC_DRV_CTP)
static int __mmc_do_erase(struct mmc_host *host,struct mmc_card *card,u64 start_addr,u64 len)
{
	int err = MMC_ERR_NONE;
	u64 end_addr =((start_addr + len)/card->blklen - 1) * card->blklen;
	if (end_addr/card->blklen > card->nblks){
		printf("[MSDC%d]Erase address out of range! start<0x%llx>,len<0x%llx>,card_nblks<0x%x>\n",host->id,start_addr,len,card->nblks);
		return MMC_ERR_INVALID;
    }

	if ((err = mmc_erase_start(card, start_addr))) {
		printf("[MSDC%d]Set erase start addrees 0x%llx failed,Err<%d>\n",host->id,start_addr,err);
        goto out;
    }
    if ((err = mmc_erase_end(card, end_addr))){
		printf("[MSDC%d]Set erase end addrees 0x%llx + 0x%llx failed,Err<%d>\n",host->id,start_addr,len,err);
        goto out;
    }
    if ((err = mmc_erase(card, MMC_ERASE_TRIM))){
        printf("[MSDC%d]Set erase <0x%llx - 0x%llx> failed,Err<%d>\n",host->id,start_addr,start_addr + len,err);
        goto out;
    }
    printf("[MSDC%d]0x%llx - 0x%llx Erased\n", host->id,start_addr,start_addr + len);
out:
	return err;
}

int mmc_do_erase(int dev_num,u64 start_addr,u64 len)
{
	struct mmc_host *host = mmc_get_host(dev_num);
	struct mmc_card *card = mmc_get_card(dev_num);
	struct mmc_erase_part erase_part[EMMC_PART_NUM];
	u32 s_blknr = 0;
	u32 e_blknr = 0;
	u32 s_pid,s_pid_o,e_pid;
	u32 err;
	if ( (!card) || (!host) ){
		printf("[mmc_do_erase] card<0x%x> host<0x%x>  mmc_do_erase\n", (unsigned int)card, (unsigned int)host);
		return MMC_ERR_INVALID;
	}
	if (!len ){
		printf("[MSDC%d] invalid erase size! len<0x%llx>\n",host->id,len);
		return MMC_ERR_INVALID;
	}
	if ((start_addr % card->blklen) || (len % card->blklen)){
		printf("[MSDC%d] non-alignment erase address! start<0x%llx>,len<0x%llx>,card_nblks<0x%x>\n",host->id,start_addr,len,card->nblks);
		return MMC_ERR_INVALID;
	}

#ifdef FEATURE_MMC_ADDR_TRANS
	s_blknr = start_addr/card->blklen;
	e_blknr = (start_addr + len)/card->blklen -1;
    if (mmc_virt_to_phys(s_blknr, &s_blknr, &s_pid) != MMC_ERR_NONE){
		printf("[MSDC%d] s_addr trans error blknr<0x%x>\n",host->id,s_blknr);
		return MMC_ERR_INVALID;
	}
	if (mmc_virt_to_phys(e_blknr, &e_blknr, &e_pid) != MMC_ERR_NONE){
		printf("[MSDC%d] e_addr trans error blknr<0x%x>\n",host->id,e_blknr);
		return MMC_ERR_INVALID;
    }
	s_pid_o = s_pid;
	erase_part[s_pid].id = s_pid;
	erase_part[s_pid].start_blk = s_blknr;
	erase_part[s_pid].blkcnt = (s_pid == e_pid) ? (len/card->blklen):(g_emmc_addr_trans[s_pid].len - s_blknr);
	s_pid++;

	for( ; s_pid < e_pid; s_pid++){
		erase_part[s_pid].id = s_pid;
		erase_part[s_pid].start_blk = 0;
		erase_part[s_pid].blkcnt = g_emmc_addr_trans[s_pid].len;
	}
	if(s_pid_o != e_pid){
		erase_part[s_pid].id = s_pid;
		erase_part[s_pid].start_blk = 0;
		erase_part[s_pid].blkcnt = e_blknr + 1;
	}
	for(s_pid = s_pid_o; s_pid <= e_pid; s_pid++){
		if (erase_part[s_pid].blkcnt == 0 || s_pid == EMMC_PART_RPMB)
			continue;
		if ((err = mmc_switch_part(s_pid))){
			printf("[MSDC%d] mmc swtich failed.part<%d> error <%d> \n",host->id,s_pid,err);
			return err;
		}
		printf("[MSDC%d] mmc erase part<%d> <0x%llx - 0x%llx>start....\n",host->id,s_pid,(u64)(erase_part[s_pid].start_blk * card->blklen),(u64)(erase_part[s_pid].start_blk + erase_part[s_pid].blkcnt) * card->blklen);
		if ( (err =  __mmc_do_erase(host,card,(u64)(erase_part[s_pid].start_blk * card->blklen),(u64)((erase_part[s_pid].blkcnt) * card->blklen )))!=0 ) {
			printf("[MSDC%d] mmc erase failed. part<%d> error <%d> \n",host->id,s_pid,err);
			return err;
		}
		printf("[MSDC%d] mmc erase part<%d> <0x%llx - 0x%llx> done.\n",host->id,s_pid,(u64)(erase_part[s_pid].start_blk * card->blklen),(u64)(erase_part[s_pid].start_blk + erase_part[s_pid].blkcnt) * card->blklen);
	}
#else
	if (err =  __mmc_do_erase(host,card,start_addr,len) ) {
		printf("[MSDC%d] mmc erase failed.error <%d> \n",host->id,err);
		return err;
	}
#endif
	return err;

}
#endif
