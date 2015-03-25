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

/*=======================================================================*/
/* HEADER FILES                              */
/*=======================================================================*/
#include "msdc.h"

#define NR_MMC              (MSDC_MAX_NUM)
#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))

struct mmc_host sd_host[NR_MMC];
struct mmc_card sd_card[NR_MMC];

const unsigned int tran_exp[] = {
    10000,  100000,     1000000,    10000000,
    0,      0,      0,      0
};

const unsigned char tran_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
    1,  10, 100,    1000,   10000,  100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
    const u32 __mask = (1 << (size)) - 1;
    const int __off = 3 - ((start) / 32);
    const int __shft = (start) & 31;
    u32 __res;

    __res = resp[__off] >> __shft;
    if ((size) + __shft >= 32)
        __res |= resp[__off-1] << (32 - __shft);
    return __res & __mask;
}

#ifdef USE_2WAY_ACMD41_CMD1
//Light: Shall be removed after EFUSE_Query_SD_Card_Init_Flow() have been implemented by BROM
bool EFUSE_Query_SD_Card_Init_Flow(void)
{
    return USE_2WAY_ACMD41_FAKE_EFUSE;
}
#endif


#if defined(FEATURE_MMC_CARD_DETECT)
int mmc_card_avail(struct mmc_host *host)
{
    return msdc_card_avail(host);
}
#endif

//Note: 1. Preloader use "if 0" to wrap mmc_card_protected
//      2. Neither preloader or LK call this function
#if defined(MMC_MSDC_DRV_CTP)
int mmc_card_protected(struct mmc_host *host)
{
    return msdc_card_protected(host);
}
#endif

struct mmc_host *mmc_get_host(int id)
{
    return &sd_host[id];
}

struct mmc_card *mmc_get_card(int id)
{
    return &sd_card[id];
}

int mmc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    int err;
    int retry = cmd->retries;

    //BEGIN Light 20121225
    if (cmd->opcode == MMC_CMD_APP_CMD ) {
        host->app_cmd = 1;
        host->app_cmd_arg = cmd->arg;
    } else {
        host->app_cmd = 0;
    }
    //END Light 20121225

    do {
        err = msdc_cmd(host, cmd);
        if (err == MMC_ERR_NONE)
            break;
    } while(retry--);

    return err;
}

static int mmc_app_cmd(struct mmc_host *host, struct mmc_command *cmd,
               u32 rca, int retries)
{
    int err = MMC_ERR_FAILED;
    struct mmc_command appcmd;

    appcmd.opcode  = MMC_CMD_APP_CMD;
    appcmd.arg     = rca << 16;
    appcmd.rsptyp  = RESP_R1;
    appcmd.retries = CMD_RETRIES;
    appcmd.timeout = CMD_TIMEOUT;

    do {
        err = mmc_cmd(host, &appcmd);

        if (err == MMC_ERR_NONE)
            err = mmc_cmd(host, cmd);
        if (err == MMC_ERR_NONE)
            break;
    } while (retries--);

    return err;
}

u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
    int bit;

    ocr &= host->ocr_avail;

    bit = uffs(ocr);
    if (bit) {
        bit -= 1;
        ocr &= 3 << bit;
    } else {
        ocr = 0;
    }
    return ocr;
}


////////////////////////////////////////////////////////////
//
// Functions for each SD/MMC command
//
////////////////////////////////////////////////////////////

int mmc_go_idle(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

#if defined(MMC_MSDC_DRV_CTP)
int mmc_go_irq_state(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_IO_MODE)) {
        printf("[SD%d] Card doesn't support I/O mode for IRQ state\n", host->id);
        return MMC_ERR_FAILED;
    }

    cmd.opcode  = MMC_CMD_GO_IRQ_STATE;
    cmd.rsptyp  = RESP_R5;
    cmd.arg = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_inactive(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_INACTIVE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_pre_idle(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg = 0xF0F0F0F0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_sleep_awake(struct mmc_host *host, struct mmc_card *card, int sleep)
{
    struct mmc_command cmd;
    u32 timeout;

    if (card->raw_ext_csd[EXT_CSD_S_A_TIMEOUT]) {
        timeout = ((1 << card->raw_ext_csd[EXT_CSD_S_A_TIMEOUT]) * 100) / 1000000;
    } else {
        timeout = CMD_TIMEOUT;
    }

    cmd.opcode  = MMC_CMD_SLEEP_AWAKE;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg = (card->rca << 16) | (sleep << 15);
    cmd.retries = CMD_RETRIES;
    cmd.timeout = timeout;

    return mmc_cmd(host, &cmd);
}
#endif

int mmc_send_status(struct mmc_host *host, struct mmc_card *card, u32 *status)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_STATUS;
    cmd.arg = card->rca << 16;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE) {
        *status = cmd.resp[0];
        #if MMC_DEBUG
        mmc_dump_card_status(*status);
        #endif
    }
    return err;
}

static int mmc_send_if_cond(struct mmc_host *host, u32 ocr)
{
    struct mmc_command cmd;
    int err;
    static const u8 test_pattern = 0xAA;
    u8 result_pattern;

    /*
     * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
     * before SD_APP_OP_COND. This command will harmlessly fail for
     * SD 1.0 cards.
     */

    cmd.opcode  = SD_CMD_SEND_IF_COND;
    cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        return err;

    result_pattern = cmd.resp[0] & 0xFF;

    if (result_pattern != test_pattern)
        return MMC_ERR_INVALID;

    return MMC_ERR_NONE;
}

static int mmc_send_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int i, err = 0;

    cmd.opcode  = MMC_CMD_SEND_OP_COND;
    cmd.arg = ocr;
    cmd.rsptyp  = RESP_R3;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    for (i = 100; i; i--) {
        err = mmc_cmd(host, &cmd);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        if (cmd.resp[0] & MMC_CARD_BUSY)
            break;

        err = MMC_ERR_TIMEOUT;

        mdelay(10); //Light To do: verify if mdelay(30) shall be used for SD30
    }

    if (!err && rocr)
        *rocr = cmd.resp[0];

    return err;
}

static int mmc_send_app_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int i, err = 0;

    cmd.opcode  = SD_ACMD_SEND_OP_COND;
    cmd.arg = ocr;
    cmd.rsptyp  = RESP_R3;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    for (i = 100; i; i--) {
        err = mmc_app_cmd(host, &cmd, 0, CMD_RETRIES);
        if (err != MMC_ERR_NONE)
            break;

        if (cmd.resp[0] & MMC_CARD_BUSY || ocr == 0)
            break;

        err = MMC_ERR_TIMEOUT;

        mdelay(10); //Light To do: verify if mdelay(30) shall be used for SD30
    }

    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

#if defined(FEATURE_MMC_SDIO)
int mmc_send_io_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int i, err = 0;

    BUG_ON(!host);

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = SD_IO_SEND_OP_COND;
    cmd.arg    = ocr;
    cmd.rsptyp = RESP_R4;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    for (i = 100; i; i--) {
        err = mmc_cmd(host, &cmd);
        if (err)
            break;

        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        if (cmd.resp[0] & MMC_CARD_BUSY)
            break;

        err = MMC_ERR_TIMEOUT;

        mdelay(10);
    }

    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}
#endif

static int mmc_all_send_cid(struct mmc_host *host, u32 *cid)
{
    int err;
    struct mmc_command cmd;

    /* send cid */
    cmd.opcode  = MMC_CMD_ALL_SEND_CID;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        return err;

    memcpy(cid, cmd.resp, sizeof(u32) * 4);

    return MMC_ERR_NONE;
}

/* code size add 1KB*/
static void mmc_decode_cid(struct mmc_card *card)
{
    //Attention!: The sequence of unstuff_bits are reordered to avoid License Issue
    u32 *resp = card->raw_cid;

    memset(&card->cid, 0, sizeof(struct mmc_cid));

    card->cid.prod_name[4]  = unstuff_bits(resp, 64, 8);
    card->cid.prod_name[3]  = unstuff_bits(resp, 72, 8);
    card->cid.prod_name[2]  = unstuff_bits(resp, 80, 8);
    card->cid.prod_name[1]  = unstuff_bits(resp, 88, 8);
    card->cid.prod_name[0]  = unstuff_bits(resp, 96, 8);

    if (mmc_card_mmc(card)) {
        /*
         * The selection of the format here is based upon published
         * specs from sandisk and from what people have reported.
         */
        card->cid.year          = unstuff_bits(resp, 8, 4) + 1997;
        card->cid.month         = unstuff_bits(resp, 12, 4);
        card->cid.prod_name[5]  = unstuff_bits(resp, 56, 8);

        switch (card->csd.mmca_vsn) {
            case 4: /* MMC v4 */
            case 3: /* MMC v3.1 - v3.3 */
            case 2: /* MMC v2.0 - v2.2 */
                card->cid.serial        = unstuff_bits(resp, 16, 32);
                card->cid.oemid         = unstuff_bits(resp, 104, 16);
                //card->cid.cbx           = unstuff_bits(resp, 112, 2);
                card->cid.manfid        = unstuff_bits(resp, 120, 8);
                break;

            case 1: /* MMC v1.4 */
            case 0: /* MMC v1.0 - v1.2 */
                card->cid.serial        = unstuff_bits(resp, 16, 24);
                card->cid.fwrev         = unstuff_bits(resp, 40, 4);
                card->cid.hwrev         = unstuff_bits(resp, 44, 4);
                card->cid.prod_name[6]  = unstuff_bits(resp, 48, 8);
                card->cid.manfid        = unstuff_bits(resp, 104, 24);
                break;

            default:
                printf("[SD%d] Unknown MMCA version %d\n",
                    mmc_card_id(card), card->csd.mmca_vsn);
                break;
        }
    } else {
        /*
         * SD doesn't currently have a version field so we will
         * have to assume we can parse this.
         */
        card->cid.month         = unstuff_bits(resp, 8, 4);
        card->cid.year          = unstuff_bits(resp, 12, 8);
        card->cid.serial        = unstuff_bits(resp, 24, 32);
        card->cid.fwrev         = unstuff_bits(resp, 56, 4);
        card->cid.hwrev         = unstuff_bits(resp, 60, 4);
        card->cid.oemid         = unstuff_bits(resp, 104, 16);
        card->cid.manfid        = unstuff_bits(resp, 120, 8);

        card->cid.year += 2000; /* SD cards year offset */
    }
}

static int mmc_decode_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    unsigned int e, m, csd_struct;
    u32 *resp = card->raw_csd;

    /* common part; some part are updated later according to spec. */
    csd_struct = unstuff_bits(resp, 126, 2);
    csd->csd_struct = csd_struct;

    /* For MMC
     * We only understand CSD structure v1.1 and v1.2.
     * v1.2 has extra information in bits 15, 11 and 10.
     */
    if ( ( mmc_card_mmc(card) &&
           ( csd_struct != CSD_STRUCT_VER_1_0 && csd_struct != CSD_STRUCT_VER_1_1
            && csd_struct != CSD_STRUCT_VER_1_2 && csd_struct != CSD_STRUCT_EXT_CSD )
         ) ||
         ( mmc_card_sd(card) && ( csd_struct != 0 && csd_struct!=1 ) )
         )
    {
        printf("[SD%d] Unknown CSD ver %d\n", mmc_card_id(card), csd_struct);
        return MMC_ERR_INVALID;
    }

    m = unstuff_bits(resp, 99, 4);
    e = unstuff_bits(resp, 96, 3);
    csd->max_dtr      = tran_exp[e] * tran_mant[m];

    /* update later according to spec. */
    csd->read_blkbits = unstuff_bits(resp, 80, 4);

    #if defined(FEATURE_MMC_STRIPPED)
    csd->cmdclass     = unstuff_bits(resp, 84, 12);
    csd->read_partial = unstuff_bits(resp, 79, 1);
    csd->write_misalign = unstuff_bits(resp, 78, 1);
    csd->read_misalign = unstuff_bits(resp, 77, 1);
    csd->dsr = unstuff_bits(resp, 76, 1);
    csd->write_prot_grpsz = unstuff_bits(resp, 32, 7);
    csd->write_prot_grp = unstuff_bits(resp, 31, 1);
    csd->r2w_factor = unstuff_bits(resp, 26, 3);
    csd->write_blkbits = unstuff_bits(resp, 22, 4);
    csd->write_partial = unstuff_bits(resp, 21, 1);
    csd->copy = unstuff_bits(resp, 14, 1);
    csd->perm_wr_prot = unstuff_bits(resp, 13, 1);
    csd->tmp_wr_prot = unstuff_bits(resp, 12, 1);

    m = unstuff_bits(resp, 115, 4);
    e = unstuff_bits(resp, 112, 3);
    csd->tacc_ns     = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
    csd->tacc_clks   = unstuff_bits(resp, 104, 8) * 100;
    #endif

    e = unstuff_bits(resp, 47, 3);
    m = unstuff_bits(resp, 62, 12);
    csd->capacity     = (1 + m) << (e + 2);

    //Specific part
    if (mmc_card_sd(card)) {
        #if defined(FEATURE_MMC_STRIPPED)
        csd->erase_blk_en = unstuff_bits(resp, 46, 1);
        csd->erase_sctsz = unstuff_bits(resp, 39, 7) + 1;
        #endif

        switch (csd_struct) {
            case 0:
                break;
            case 1:
                /*
                 * This is a block-addressed SDHC card. Most
                 * interesting fields are unused and have fixed
                 * values. To avoid getting tripped by buggy cards,
                 * we assume those fixed values ourselves.
                 */
                mmc_card_set_blockaddr(card);

                m = unstuff_bits(resp, 48, 22);
                csd->capacity     = (1 + m) << 10;

                csd->read_blkbits = 9;

                #if defined(FEATURE_MMC_STRIPPED)
                csd->tacc_ns	 = 0; /* Unused */
                csd->tacc_clks	 = 0; /* Unused */
                csd->read_partial = 0;
                csd->write_misalign = 0;
                csd->read_misalign = 0;
                csd->r2w_factor = 4; /* Unused */
                csd->write_blkbits = 9;
                csd->write_partial = 0;
                #endif
                break;
        }
    } else {
        csd->mmca_vsn    = unstuff_bits(resp, 122, 4);
        #if defined(FEATURE_MMC_STRIPPED)
        csd->erase_sctsz = (unstuff_bits(resp, 42, 5) + 1) * (unstuff_bits(resp, 37, 5) + 1);
        #endif
    }

#if MMC_DEBUG
    mmc_dump_csd(card);
#endif

    return 0;
}

static void mmc_decode_ext_csd(struct mmc_card *card)
{
    u8 *ext_csd = &card->raw_ext_csd[0];

    card->ext_csd.sectors =
        ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
        ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
        ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
        ext_csd[EXT_CSD_SEC_CNT + 3] << 24;

    card->ext_csd.rev = ext_csd[EXT_CSD_REV];
#if defined(FEATURE_MMC_STRIPPED)
    card->ext_csd.hc_erase_grp_sz = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    card->ext_csd.hc_wp_grp_sz = ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512 * 1024;
    card->ext_csd.trim_tmo_ms = ext_csd[EXT_CSD_TRIM_MULT] * 300;
    card->ext_csd.boot_info   = ext_csd[EXT_CSD_BOOT_INFO];
    card->ext_csd.boot_part_sz = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
    card->ext_csd.access_sz = (ext_csd[EXT_CSD_ACC_SIZE] & 0xf) * 512;
    card->ext_csd.rpmb_sz = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
    card->ext_csd.erased_mem_cont = ext_csd[EXT_CSD_ERASED_MEM_CONT];
    card->ext_csd.part_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_PART_EN ? 1 : 0;
    card->ext_csd.enh_attr_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_ENH_ATTR_EN ? 1 : 0;
    card->ext_csd.enh_start_addr =
        (ext_csd[EXT_CSD_ENH_START_ADDR + 0] |
         ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24);
    card->ext_csd.enh_sz =
        (ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 * 1024 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
#endif

    if (card->ext_csd.sectors)
        mmc_card_set_blockaddr(card);

#ifdef FEATURE_MMC_UHS1
    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_2V) ||
        (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_HS200_1_8V)) {
        card->ext_csd.hs_max_dtr = 200000000;
    } else
#endif
    if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52_1_2V) ||
        (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_DDR_52)) {
        card->ext_csd.ddr_support = 1;
        card->ext_csd.hs_max_dtr = 52000000;
    } else if (ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_52) {
        card->ext_csd.hs_max_dtr = 52000000;
    } else if ((ext_csd[EXT_CSD_CARD_TYPE] & EXT_CSD_CARD_TYPE_26)) {
        card->ext_csd.hs_max_dtr = 26000000;
    } else {
        /* MMC v4 spec says this cannot happen */
        printf("[SD%d] MMCv4 but HS unsupported\n", card->host->id);
    }

#if MMC_DEBUG
    mmc_dump_ext_csd(card);
#endif
    return;
}

//Note: 1. Neither preloader or LK define this function
#if defined(MMC_MSDC_DRV_CTP)
int mmc_deselect_all_card(struct mmc_host *host)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_NONE;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    return err;
}
#endif

int mmc_select_card(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg = card->rca << 16;
    cmd.rsptyp  = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    return err;
}

int mmc_send_relative_addr(struct mmc_host *host, struct mmc_card *card, unsigned int *rca)
{
    int err;
    struct mmc_command cmd;

    memset(&cmd, 0, sizeof(struct mmc_command));

    if (mmc_card_mmc(card)) { /* set rca */
        cmd.opcode  = MMC_CMD_SET_RELATIVE_ADDR;
        cmd.arg     = *rca << 16;
        cmd.rsptyp  = RESP_R1;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
    } else {  /* send rca */
        cmd.opcode  = SD_CMD_SEND_RELATIVE_ADDR;
        cmd.arg     = 0;
        cmd.rsptyp  = RESP_R6;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;
    }
    err = mmc_cmd(host, &cmd);
    if ((err == MMC_ERR_NONE) && !mmc_card_mmc(card))
        *rca = cmd.resp[0] >> 16;

    return err;
}

//Note: 1. Neither preloader or LK call this function
#if defined(MMC_MSDC_DRV_CTP)
int mmc_lock_unlock(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_LOCK_UNLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = 0;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}
#endif

int mmc_switch(struct mmc_host *host, struct mmc_card *card,
            u8 set, u8 index, u8 value)
{
    int err;
    u32 status;
    struct mmc_command cmd;

    cmd.opcode = MMC_CMD_SWITCH;
    cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
        (index << 16) | (value << 8) | set;
    cmd.rsptyp = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        return err;

    do {
        err = mmc_send_status(host, card, &status);
        if (err) {
            printf("[SD%d] Fail to send status %d\n", host->id, err);
            break;
        }
        if (status & R1_SWITCH_ERROR) {
            printf("[SD%d] switch error. arg(0x%x)\n", host->id, cmd.arg);
            return MMC_ERR_FAILED;
        }
    } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));

    return err;
}

static int mmc_sd_switch(struct mmc_host *host,
            struct mmc_card *card,
            int mode, int group, u8 value, mmc_switch_t *resp)
{
    int err = MMC_ERR_FAILED;
    int result = 0;
    struct mmc_command cmd;
    u32 *sts = (u32 *)resp;

    mode = !!mode;
    value &= 0xF;

    /* argument: mode[31]= 0 (for check func.) and 1 (for switch func) */
    cmd.opcode = SD_CMD_SWITCH;
    cmd.arg = mode << 31 | 0x00FFFFFF;
    cmd.arg &= ~(0xF << (group * 4));
    cmd.arg |= value << (group * 4);
    cmd.rsptyp = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = 100;  /* 100ms */

#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do{
#endif
        msdc_set_blknum(host, 1);
        msdc_set_blklen(host, 64);
        msdc_set_timeout(host, 100000000, 0);
        err = mmc_cmd(host, &cmd);

        if (err != MMC_ERR_NONE)
            goto out;

        /* 512 bits = 64 bytes = 16 words */
        err = msdc_pio_read(host, sts, 64);
        if (err != MMC_ERR_NONE){
            msdc_abort_handler(host, 1);
            #if defined(FEATURE_MMC_RD_TUNING)
            result = msdc_tune_read(host);
            #else
            goto out;
            #endif
        }
#if defined(FEATURE_MMC_RD_TUNING)
    }while(err && result != MMC_ERR_READTUNEFAIL);
    msdc_reset_tune_counter(host);
#endif

#if MMC_DEBUG
    {
        int i;
        u8 *byte = (u8*)&sts[0];

        /* Status:   B0      B1    ...
         * Bits  : 511-504 503-495 ...
         */

        for (i = 0; i < 4; i++) {
            MSG(RSP, "  [%d-%d] %xh %xh %xh %xh\n",
                ((3 - i + 1) << 7) - 1, (3 - i) << 7,
                sts[(i << 2) + 0], sts[(i << 2) + 1],
                sts[(i << 2) + 2], sts[(i << 2) + 3]);
        }
        for (i = 0; i < 8; i++) {
            MSG(RSP, "  [%d-%d] %xh %xh %xh %xh %xh %xh %xh %xh\n",
                ((8 - i) << 6) - 1, (8 - i - 1) << 6,
                byte[(i << 3) + 0], byte[(i << 3) + 1],
                byte[(i << 3) + 2], byte[(i << 3) + 3],
                byte[(i << 3) + 4], byte[(i << 3) + 5],
                byte[(i << 3) + 6], byte[(i << 3) + 7]);
        }
    }
#endif

out:
    return err;
}

#ifdef FEATURE_MMC_UHS1
int mmc_ctrl_speed_class(struct mmc_host *host, u32 scc)
{
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SPEED_CLASS_CTRL;
    cmd.arg = scc << 28;
    cmd.rsptyp  = RESP_R1B;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_switch_volt(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_VOL_SWITCH;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE)
        err = msdc_switch_volt(host, MMC_VDD_165_195);

    return err;
}

int mmc_switch_uhs1(struct mmc_host *host, struct mmc_card *card, unsigned int mode)
{
    int err;
    u8  status[64];
    int val;
    const char *smode[] = { "SDR12", "SDR25", "SDR50", "SDR104", "DDR50" };

    err = mmc_sd_switch(host, card, 1, 0, mode, (mmc_switch_t*)&status[0]);

    if (err != MMC_ERR_NONE)
        goto out;

    if ((status[16] & 0xF) != mode) {
        printf("[SD%d] UHS-1 %s mode not supported!\n", host->id, smode[mode]);
        err = MMC_ERR_FAILED;
    } else {
        card->uhs_mode = mode;
        mmc_card_set_uhs1(card);
        printf("[SD%d] Switch to UHS-1 %s mode\n", host->id, smode[mode]);
        if (mode == MMC_SWITCH_MODE_DDR50) {
            mmc_card_set_ddr(card);
        }
    }

out:
    return err;
}

int mmc_switch_drv_type(struct mmc_host *host, struct mmc_card *card, int val)
{
    int err;
    u8  status[64];
    const char *type[] = { "TYPE-B", "TYPE-A", "TYPE-C", "TYPE-D" };

    err = mmc_sd_switch(host, card, 1, 2, val, (mmc_switch_t*)&status[0]);

    if (err != MMC_ERR_NONE)
        goto out;

    if ((status[15] & 0xF) != val) {
        printf("[SD%d] UHS-1 %s drv not supported!\n", host->id, type[val]);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to UHS-1 %s drv!\n", host->id, type[val]);
    }

out:
    return err;
}

int mmc_switch_max_cur(struct mmc_host *host, struct mmc_card *card, int val)
{
    int err;
    u8  status[64];
    const char *curr[] = { "200mA", "400mA", "600mA", "800mA" };

    err = mmc_sd_switch(host, card, 1, 3, val, (mmc_switch_t*)&status[0]);

    if (err != MMC_ERR_NONE)
        goto out;

    if (((status[15] >> 4) & 0xF) != val) {
        printf("[SD%d] UHS-1 %s max. current not supported!\n", host->id, curr[val]);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to UHS-1 %s max. current!\n", host->id, curr[val]);
    }

out:
    return err;
}

//Note: 1. Neither preloader or LK define this function
int mmc_send_tuning_blk(struct mmc_host *host, struct mmc_card *card, u32 *buf)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 64);
    msdc_set_timeout(host, 100000000, 0);
    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        goto out;

    err = msdc_pio_read(host, buf, 64);
    if (err != MMC_ERR_NONE)
        goto out;

    #if MMC_DEBUG
    mmc_dump_tuning_blk((u8*)buf);
    #endif

out:
    return err;
}

int mmc_tune_timing(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE;

    if (mmc_card_sd(card) && mmc_card_uhs1(card) && !mmc_card_ddr(card)) {
        err = msdc_tune_uhs1(host, card);
    }
    return err;
}
#endif

#ifdef FEATURE_MMC_HS
int mmc_switch_hs(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    u8  status[64];
    int val = MMC_SWITCH_MODE_SDR25;

    err = mmc_sd_switch(host, card, 1, 0, val, (mmc_switch_t*)&status[0]);

    if (err != MMC_ERR_NONE)
        goto out;

    if ((status[16] & 0xF) != 1) {
        printf("[SD%d] HS mode not supported!\n", host->id);
        err = MMC_ERR_FAILED;
    } else {
        printf("[SD%d] Switch to HS mode\n", host->id);
        mmc_card_set_highspeed(card);
    }

out:
    return err;
}
#endif

#if defined(FEATURE_MMC_STRIPPED)
static int mmc_read_scrs(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE;
    int result;
    struct mmc_command cmd;
    struct sd_scr *scr = &card->scr;
    u32 resp[4];
    u32 tmp;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 8);
    msdc_set_timeout(host, 100000000, 0);

    cmd.opcode  = SD_ACMD_SEND_SCR;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;
#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do{
#endif
        mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
        if ((err != MMC_ERR_NONE) || !(cmd.resp[0] & R1_APP_CMD))
            return MMC_ERR_FAILED;

        /* 8 bytes = 2 words */
        err = msdc_pio_read(host, card->raw_scr, 8);
        if (err != MMC_ERR_NONE){
            msdc_abort_handler(host, 1);
            #if defined(FEATURE_MMC_RD_TUNING)
            result = msdc_tune_read(host);
            #else
            return err;
            #endif
        }
#if defined(FEATURE_MMC_RD_TUNING)
    } while(err && result != MMC_ERR_READTUNEFAIL);
    msdc_reset_tune_counter(host);
#endif

    MSG(INF, "[SD%d] SCR: %x %x (raw)\n", host->id, card->raw_scr[0], card->raw_scr[1]);

    tmp = ntohl(card->raw_scr[0]);
    card->raw_scr[0] = ntohl(card->raw_scr[1]);
    card->raw_scr[1] = tmp;

    MSG(INF, "[SD%d] SCR: %x %x (ntohl)\n", host->id, card->raw_scr[0], card->raw_scr[1]);

    resp[2] = card->raw_scr[1];
    resp[3] = card->raw_scr[0];

    if (unstuff_bits(resp, 60, 4) != 0) {
        printf("[SD%d] Unknown SCR ver %d\n", mmc_card_id(card), unstuff_bits(resp, 60, 4));
        return MMC_ERR_INVALID;
    }

    scr->scr_struct = unstuff_bits(resp, 60, 4);
    scr->sda_vsn = unstuff_bits(resp, 56, 4);
    scr->data_bit_after_erase = unstuff_bits(resp, 55, 1);
    scr->security = unstuff_bits(resp, 52, 3);
    scr->bus_widths = unstuff_bits(resp, 48, 4);
    scr->sda_vsn3 = unstuff_bits(resp, 47, 1);
    scr->ex_security = unstuff_bits(resp, 43, 4);
    scr->cmd_support = unstuff_bits(resp, 32, 2);
    printf("[SD%d] SD_SPEC(%d) SD_SPEC3(%d) SD_BUS_WIDTH=%d\n",
        mmc_card_id(card), scr->sda_vsn, scr->sda_vsn3, scr->bus_widths);
    printf("[SD%d] SD_SECU(%d) EX_SECU(%d), CMD_SUPP(%d): CMD23(%d), CMD20(%d)\n",
        mmc_card_id(card), scr->security, scr->ex_security, scr->cmd_support,
        (scr->cmd_support >> 1) & 0x1, scr->cmd_support & 0x1);
    return err;
}
#endif

static int mmc_read_csds(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_CSD;
    cmd.arg = card->rca << 16;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT * 100;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE)
        memcpy(&card->raw_csd, &cmd.resp[0], sizeof(u32) * 4);
    return err;
}


/* Read and decode extended CSD. */
int mmc_read_ext_csd(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE;
    u32 *ptr;
    int result = MMC_ERR_NONE;
    struct mmc_command cmd;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4) {
        printf("[SD%d] MMCA_VSN: %d. Skip EXT_CSD\n", host->id, card->csd.mmca_vsn);
        return MMC_ERR_NONE;
    }

    /*
     * As the ext_csd is so large and mostly unused, we don't store the
     * raw block in mmc_card.
     */
    memset(&card->raw_ext_csd[0], 0, 512);
    ptr = (u32*)&card->raw_ext_csd[0];

    cmd.opcode  = MMC_CMD_SEND_EXT_CSD;
    cmd.arg = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

#if defined(FEATURE_MMC_RD_TUNING)
    msdc_reset_tune_counter(host);
    do{
#endif
        msdc_set_blknum(host, 1);
        msdc_set_blklen(host, 512);
        msdc_set_timeout(host, 100000000, 0);
        err = mmc_cmd(host, &cmd);
        if (err != MMC_ERR_NONE)
            goto out;

        err = msdc_pio_read(host, ptr, 512);
        if (err != MMC_ERR_NONE){
            msdc_abort_handler(host, 1);
            #if defined(EATURE_MMC_RD_TUNING)
            result = msdc_tune_read(host);
            #else
            goto out;
            #endif
        }

#if defined(FEATURE_MMC_RD_TUNING)
    } while ( err && result != MMC_ERR_READTUNEFAIL );
    msdc_reset_tune_counter(host);
#endif
    mmc_decode_ext_csd(card);

out:
    return err;
}

int mmc_set_ext_csd(struct mmc_card *card, uint8 addr, uint8 value)
{
    int err;
    u8 *ext_csd;

    /* can't write */
    if (192 <= addr || !card || !mmc_card_mmc(card))
        return MMC_ERR_INVALID;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, addr, value);

    //Readback to verify
    if (err == MMC_ERR_NONE) {
        err = mmc_read_ext_csd(card->host, card);
        if (err == MMC_ERR_NONE) {
            ext_csd = &card->raw_ext_csd[0];
            if (ext_csd[addr] != value)
                err = MMC_ERR_FAILED;
        }
    }

    return err;
}

/* Fetches and decodes switch information */
static int mmc_read_switch(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    u8  status[64];

    err = mmc_sd_switch(host, card, 0, 0, 1, (mmc_switch_t*)&status[0]);
    if (err != MMC_ERR_NONE) {
        /* Card not supporting high-speed will ignore the command. */
        err = MMC_ERR_NONE;
        goto out;
    }

    /* bit 511:480 in status[0]. bit 415:400 in status[13] */
    if (status[13] & 0x01) {
        printf("[SD%d] Support: Default/SDR12\n", host->id);
        card->sw_caps.hs_max_dtr = 25000000;  /* default or sdr12 */
    }
    if (status[13] & 0x02) {
        printf("[SD%d] Support: HS/SDR25\n", host->id);
        card->sw_caps.hs_max_dtr = 50000000;  /* high-speed or sdr25 */
    }

#ifdef FEATURE_MMC_UHS1
    if (status[13] & 0x04) {
        printf("[SD%d] Support: SDR50\n", host->id);
        card->sw_caps.hs_max_dtr = 100000000; /* sdr50 */
    }
    if (status[13] & 0x08) {
        printf("[SD%d] Support: SDR104\n", host->id);
        card->sw_caps.hs_max_dtr = 208000000; /* sdr104 */
    }
    if (status[13] & 0x10) {
        printf("[SD%d] Support: DDR50\n", host->id);
        card->sw_caps.hs_max_dtr = 50000000;  /* ddr50 */
        card->sw_caps.ddr = 1;
    }

    if (status[9] & 0x01) {
        printf("[SD%d] Support: Type-B Drv\n", host->id);
    }
    if (status[9] & 0x02) {
        printf("[SD%d] Support: Type-A Drv\n", host->id);
    }
    if (status[9] & 0x04) {
        printf("[SD%d] Support: Type-C Drv\n", host->id);
    }
    if (status[9] & 0x08) {
        printf("[SD%d] Support: Type-D Drv\n", host->id);
    }
    if (status[7] & 0x01) {
        printf("[SD%d] Support: 200mA current limit\n", host->id);
    }
    if (status[7] & 0x02) {
        printf("[SD%d] Support: 400mA current limit\n", host->id);
    }
    if (status[7] & 0x04) {
        printf("[SD%d] Support: 600mA current limit\n", host->id);
    }
    if (status[7] & 0x08) {
        printf("[SD%d] Support: 800mA current limit\n", host->id);
    }
#endif

out:
    return err;
}

int mmc_set_card_detect(struct mmc_host *host, struct mmc_card *card, int connect)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_ACMD_SET_CLR_CD;
    cmd.arg = connect;
    cmd.rsptyp  = RESP_R1; /* CHECKME */
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
    return err;
}

int mmc_set_blk_length(struct mmc_host *host, u32 blklen)
{
    int err;
    struct mmc_command cmd;

    /* set block len */
    cmd.opcode  = MMC_CMD_SET_BLOCKLEN;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = blklen;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;
    err = mmc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE)
        msdc_set_blklen(host, blklen);

    return err;
}

#if defined(MMC_MSDC_DRV_CTP)
int mmc_set_blk_count(struct mmc_host *host, u32 blkcnt)
{
    int err;
    struct mmc_command cmd;

    /* set block count */
    cmd.opcode  = MMC_CMD_SET_BLOCK_COUNT;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = blkcnt; /* bit31 is for reliable write request */
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;
    err = mmc_cmd(host, &cmd);

    return err;
}
#endif

int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width)
{
    int err = MMC_ERR_NONE;
    u32 arg = SD_BUS_WIDTH_4;
    struct mmc_command cmd;

    if (mmc_card_sd(card)) {
        if (width == HOST_BUS_WIDTH_8) {
            WARN_ON(width == HOST_BUS_WIDTH_8);
            width = HOST_BUS_WIDTH_4;
        }

        if ((width == HOST_BUS_WIDTH_4) && (host->caps & MMC_CAP_4_BIT_DATA)) {
            arg = SD_BUS_WIDTH_4;
        } else {
            arg = SD_BUS_WIDTH_1;
            width = HOST_BUS_WIDTH_1;
        }

        cmd.opcode  = SD_ACMD_SET_BUSWIDTH;
        cmd.arg     = arg;
        cmd.rsptyp  = RESP_R1;
        cmd.retries = CMD_RETRIES;
        cmd.timeout = CMD_TIMEOUT;

        err = mmc_app_cmd(host, &cmd, card->rca, 0);
        if (err != MMC_ERR_NONE)
            goto out;

        msdc_config_bus(host, width);

    } else if (mmc_card_mmc(card)) {
        if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
            goto out;

        if (width == HOST_BUS_WIDTH_8) {
            if (host->caps & MMC_CAP_8_BIT_DATA) {
                arg = ((host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support) ?
                    EXT_CSD_BUS_WIDTH_8_DDR : EXT_CSD_BUS_WIDTH_8;
            } else {
                width = HOST_BUS_WIDTH_4;
            }
        }

        if (width == HOST_BUS_WIDTH_4) {
            if (host->caps & MMC_CAP_4_BIT_DATA) {
                arg = ((host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support) ?
                    EXT_CSD_BUS_WIDTH_4_DDR : EXT_CSD_BUS_WIDTH_4;
            } else {
                width = HOST_BUS_WIDTH_1;
            }
        }

        if (width == HOST_BUS_WIDTH_1)
            arg = EXT_CSD_BUS_WIDTH_1;

        err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH, arg);
        if (err != MMC_ERR_NONE)
            goto out;

        if (arg == EXT_CSD_BUS_WIDTH_8_DDR || arg == EXT_CSD_BUS_WIDTH_4_DDR) {
            mmc_card_set_ddr(card);
        } else {
            card->state &= ~MMC_STATE_DDR;
        }
        msdc_config_clock(host, mmc_card_ddr(card), host->sclk);

        msdc_config_bus(host, width);

    } else {
        BUG_ON(1); /* card is not recognized */

    }

out:
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Switch to bus width(%d) failed\n", host->id, arg);
    }
    return err;
}

////////////////////////////////////////////////////////////
//
// Functions for write protection commands
//
////////////////////////////////////////////////////////////

//Note: 1. LK undefine this function
//SDHC and SDXC does not support this command
int mmc_set_write_prot(struct mmc_card *card, u32 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_WRITE_PROT))
        return MMC_ERR_INVALID;

    cmd.opcode  = MMC_CMD_SET_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

//Note: 1. LK undefine this function
//SDHC and SDXC does not support this command
int mmc_clr_write_prot(struct mmc_host *host, u32 addr)
{
    struct mmc_command cmd;

    if (!(host->card->csd.cmdclass & CCC_WRITE_PROT))
        return MMC_ERR_INVALID;

    cmd.opcode  = MMC_CMD_CLR_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

//Note: 1. LK undefine this function
//SDHC and SDXC does not support this command
int mmc_send_write_prot(struct mmc_card *card, u32 wp_addr, u32 *wp_status)
{
    int err;
    struct mmc_command cmd;
    struct mmc_host *host = card->host;
    u8 *buf = (u8*)wp_status;

    if (!(card->csd.cmdclass & CCC_WRITE_PROT))
        return MMC_ERR_INVALID;

    cmd.opcode  = MMC_CMD_SEND_WRITE_PROT;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = wp_addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 4);
    msdc_set_timeout(host, 100000000, 0);
    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        goto out;

    //To do: if tuning is necessary?
    err = msdc_pio_read(host, (u32 *)buf, 4);
    if (err != MMC_ERR_NONE)
        goto out;

out:
    return err;
}

//Note: 1. LK undefine this function
//CMD31, For MMC only
int mmc_send_write_prot_type(struct mmc_card *card, u32 wp_addr, u32 *wp_type)
{
    int err;
    struct mmc_command cmd;
    struct mmc_host *host = card->host;
    u8 *buf = (u8*)wp_type;

    if (!(card->csd.cmdclass & CCC_WRITE_PROT))
        return MMC_ERR_INVALID;

    cmd.opcode  = MMC_CMD_SEND_WRITE_PROT_TYPE;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = wp_addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 8);
    msdc_set_timeout(host, 100000000, 0);
    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        goto out;

    //To do: if tuning is necessary?
    err = msdc_pio_read(host, (u32 *)buf, 8);
    if (err != MMC_ERR_NONE)
        goto out;

out:
    return err;
}

u32 mmc_get_wpg_size(struct mmc_card *card)
{
    u32 size;
    u8 *ext_csd;

    if (mmc_card_mmc(card)) {
        ext_csd = &card->raw_ext_csd[0];

        if ((ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN)
            && (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] > 0))
        {
            size = 512 * 1024 * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] *
            ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
        } else {
            size = card->csd.write_prot_grpsz;
        }
    } else {
        if (card->csd.write_prot_grp) {
            /* SDSC could support write protect group */
            size = (card->csd.write_prot_grpsz + 1) * (1 << card->csd.write_blkbits);
        } else {
            /* SDHC and SDXC don't support write protect group */
            size = 0;
        }
    }

    return size;
}

#if defined(MMC_MSDC_DRV_CTP)
int mmc_set_user_wp(struct mmc_card *card, u8 wp)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
        goto out;

    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL,
        EXT_CSD_USR_WP, wp);

out:
    return err;
}
#endif


////////////////////////////////////////////////////////////
//
// Functions for erase commands
//
////////////////////////////////////////////////////////////
//#if defined(MMC_MSDC_DRV_CTP)
int mmc_erase_start(struct mmc_card *card, u32 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        printf("[SD%d] Card doesn't support Erase commands\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (mmc_card_highcaps(card))
        addr /= MMC_BLOCK_SIZE; /* in sector unit */

    if (mmc_card_mmc(card)) {
        cmd.opcode = MMC_CMD_ERASE_GROUP_START;
    } else {
        cmd.opcode = MMC_CMD_ERASE_WR_BLK_START;
    }

    cmd.rsptyp  = RESP_R1;
    cmd.arg = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase_end(struct mmc_card *card, u32 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        printf("[SD%d] Erase isn't supported\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (mmc_card_highcaps(card))
        addr /= MMC_BLOCK_SIZE; /* in sector unit */

    if (mmc_card_mmc(card)) {
        cmd.opcode = MMC_CMD_ERASE_GROUP_END;
    } else {
        cmd.opcode = MMC_CMD_ERASE_WR_BLK_END;
    }

    cmd.rsptyp  = RESP_R1;
    cmd.arg = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase(struct mmc_card *card, u32 arg)
{
    int err;
    u32 status;
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & CCC_ERASE)) {
        printf("[SD%d] Erase isn't supported\n", card->host->id);
        return MMC_ERR_INVALID;
    }

    if (arg & MMC_ERASE_SECURE_REQ) {
        if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
            EXT_CSD_SEC_FEATURE_ER_EN)) {
            return MMC_ERR_INVALID;
        }
    }
    if ((arg & MMC_ERASE_GC_REQ) || (arg & MMC_ERASE_TRIM)) {
        if (!(card->raw_ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT] &
            EXT_CSD_SEC_FEATURE_GB_CL_EN)) {
            return MMC_ERR_INVALID;
        }
    }

    cmd.opcode  = MMC_CMD_ERASE;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg = arg;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(card->host, &cmd);

    if (!err) {
        do {
            err = mmc_send_status(card->host, card, &status);
            if (err) break;
            #if MMC_DEBUG
            mmc_dump_card_status(status);
            #endif
            if (R1_STATUS(status) != 0) break;
        } while (R1_CURRENT_STATE(status) == 7);
    }
    return err;
}

void mmc_set_clock(struct mmc_host *host, int ddr, u32 hz)
{
    msdc_config_clock(host, ddr, hz);
}

#if 0
void mmc_hard_reset(void)
{
    msdc_hard_reset();
}
#endif

#if defined(MMC_MSDC_DRV_CTP)
int mmc_polling_CD_INT(struct mmc_host * host)
{
    return msdc_polling_CD_interrupt(host);
}
#endif

////////////////////////////////////////////////////////////
//
// Read, Write
//
////////////////////////////////////////////////////////////
int mmc_dev_bread(struct mmc_card *card, unsigned long blknr, u32 blkcnt, u8 *dst)
{
    struct mmc_host *host = card->host;
    u32 blksz = host->blklen;
    int tune = 0;
    #if defined(FEATURE_MMC_RD_TUNING)
    int retry = 1;
    #else
    int retry = 3;
    #endif
    int err;
    unsigned long src;

    src = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    MSG(OPS_MMC, "[SD%d] Block Read Cnt: %d, Physical Addr: %xh\n", host->id, blkcnt, (unsigned int)blknr);

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_read(host, (uchar *)dst, src, blkcnt);
        } else {
            #if defined(FEATURE_MMC_RD_TUNING)
                err = msdc_tune_bread(host, (uchar *)dst, src, blkcnt);
                //Introduced by 82 FPGA
                //Light To do: Use this sector or not?
                //       If msdc_tune_bread has been modified to contain reducing clock frequency,
                //       the clause in MSDC_USE_CLKDIV_IN_DATCRC can be removed
            #elif MSDC_USE_CLKDIV_IN_DATCRC
                if (err && (host->sclk > (host->f_max >> 4))) {
                    msdc_config_clock(host, mmc_card_ddr(card), host->sclk >> 1);
                    err = host->blk_read(host, (uchar *)dst, src, blkcnt);
                }
            #endif
        }
        mmc_prof_stop();
        if (err == MMC_ERR_NONE) {
            mmc_prof_update(mmc_prof_read, blkcnt, mmc_prof_handle(host->id));
            break;
        }

        #if defined(FEATURE_MMC_CM_TUNING) || defined(FEATURE_MMC_RD_TUNING)
        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC){
            //Light To do: Verify if it can be reached via "err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC"
            //       since blk_read and msdc_tune_bread will perform msdc_tune_cmdrsp() on these 2 error
            #if 0
            tune = 1;
            retry++;

            #else
            if ( tune ) break;
            tune = 1;

            #endif
        }
        else if(err == MMC_ERR_READTUNEFAIL || err == MMC_ERR_CMDTUNEFAIL){
            //Light the condition: (err == MMC_ERR_READTUNEFAIL) seems won't occurs forever
            printf("[SD%d] Fail to tuning,%s",host->id,(err == MMC_ERR_CMDTUNEFAIL)?"cmd tune failed!\n":"read tune failed!\n");
            break;
        }
        #endif
        if(err == MMC_ERR_TIMEOUT ) {
            printf("[SD%d] mmc_dev_bwrite TIMEOUT\n", host->id);
            break;
        }

        //retry will decrement when timeout error occurs
    } while (retry--);

    return err;
}

static int mmc_dev_bwrite(struct mmc_card *card, unsigned long blknr, u32 blkcnt, u8 *src)
{
    struct mmc_host *host = card->host;
    u32 blksz = host->blklen;
    u32 status;
    int tune = 0;
    #if defined(FEATURE_MMC_WR_TUNING)
    int retry = 1;
    #else
    int retry = 3;
    #endif
    int err= MMC_ERR_NONE;
    unsigned long dst;

    dst = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    #if 0
    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_write(host, dst, (uchar *)src, blkcnt);
        } else {
            #if defined(FEATURE_MMC_WR_TUNING)
                err = msdc_tune_bwrite(host, dst, (uchar *)src, blkcnt);
                //Introduced by 82 FPGA
                //To do: Use this sector or not?
                //       If msdc_tune_bread has been modified to contain reducing clock frequency,
                //       the clause in MSDC_USE_CLKDIV_IN_DATCRC can be removed
            #elif MSDC_USE_CLKDIV_IN_DATCRC
                if (err && (host->sclk > (host->f_max >> 4))) {
                    msdc_config_clock(host, mmc_card_ddr(card), host->sclk >> 1);
                    err = host->blk_write(host, dst, (uchar *)src, blkcnt);
                }
            #endif
        }
        if (err == MMC_ERR_NONE) {
            do {
                err = mmc_send_status(host, card, &status);
                if (err) {
                    printf("[SD%d] Fail to send status %d\n", host->id, err);
                    break;
                }
            } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));
            mmc_prof_stop();
            mmc_prof_update(mmc_prof_write, blkcnt, mmc_prof_handle(host->id));
            MSG(OPS, "[SD%d] Write %d bytes (DONE)\n",
                host->id, blkcnt * blksz);
            break;
        }

        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC){
            tune = 1;
            retry++;
        }
        else if(err == MMC_ERR_WRITETUNEFAIL || err == MMC_ERR_CMDTUNEFAIL){
            printf("[SD%d] Fail to tuning,%s",host->id,(err == MMC_ERR_CMDTUNEFAIL)?"cmd tune failed!\n":"write tune failed!\n");
            break;
        }
    } while (retry--);

    #else

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_write(host, dst, (uchar *)src, blkcnt);
        } else {
            #if defined(FEATURE_MMC_WR_TUNING)
                err = msdc_tune_bwrite(host, dst, (uchar *)src, blkcnt);
            #endif
        }
        if (err == MMC_ERR_NONE) {
            do {
                err = mmc_send_status(host, card, &status);
                if (err) {
                    printf("[SD%d] Fail to send status %d\n", host->id, err);
                    break;
                }
            } while (!(status & R1_READY_FOR_DATA) || (R1_CURRENT_STATE(status) == 7));
            mmc_prof_stop();
            mmc_prof_update(mmc_prof_write, blkcnt, mmc_prof_handle(host->id));
            MSG(OPS, "[SD%d] Write %d bytes (DONE)\n", host->id, blkcnt * blksz);
            break;
        }

        #if defined(FEATURE_MMC_WR_TUNING)
        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC){
            if ( tune ) break;
            tune = 1;
        }
        #endif
        if(err == MMC_ERR_TIMEOUT ) {
            printf("[SD%d] mmc_dev_bwrite TIMEOUT\n", host->id);
            break;
        }

    } while (retry);

    #endif

    return err;
}

//Read desired data blocks in an invocation.
//Internally, divide read operation to HW in unit of HW/SW limitation
int mmc_block_read(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz    = host->blklen;
    u32 maxblks  = host->max_phys_segs;
    //u32 xfercnt  = blkcnt / maxblks;
    //u32 leftblks = blkcnt % maxblks;
    u32 leftblks;
    u8 *buf = (u8*)dst;
    int ret;

    if (!blkcnt)
        return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        printf("[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n",
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    do {
        leftblks=((blkcnt> maxblks) ? maxblks : blkcnt);
        ret = mmc_dev_bread(card, (unsigned long)blknr, leftblks, buf);
        if (ret)
            return ret;
        blknr += leftblks;
        buf   += maxblks * blksz;
        blkcnt -= leftblks;
    } while ( blkcnt );

    return ret;
}

int mmc_block_write(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz    = host->blklen;
    u32 maxblks  = host->max_phys_segs;
    //u32 xfercnt  = blkcnt / maxblks;
    //u32 leftblks = blkcnt % maxblks;
    u32 leftblks;
    u8 *buf = (u8*)src;
    int ret;

    if (!blkcnt)
        return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        printf("[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n",
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    do {
        leftblks=((blkcnt> maxblks) ? maxblks : blkcnt);
        ret = mmc_dev_bwrite(card, (unsigned long)blknr, leftblks, buf);
        if (ret)
            return ret;
        blknr += leftblks;
        buf   += maxblks * blksz;
        blkcnt -= leftblks;
    } while ( blkcnt );

    return ret;
}

////////////////////////////////////////////////////////////
//
// Initialization
//
////////////////////////////////////////////////////////////

int mmc_init_mem_card(struct mmc_host *host, struct mmc_card *card, u32 ocr)
{
    int err, id = host->id;
#ifdef FEATURE_MMC_UHS1
    int s18a = 0;
#endif

    #ifdef USE_2WAY_ACMD41_CMD1
    bool bSP_Way_ACMD41_CMD1=EFUSE_Query_SD_Card_Init_Flow();

    if ( !bSP_Way_ACMD41_CMD1 ) {
        mmc_card_set_sd(card); //Try to use as SD first
        goto alternative_mmc_init_mem_card_flow;
    }
    #endif
    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F) {
        printf("Ignore voltage below the defined range but claimed by card\n");
        ocr &= ~0x7F;
    }

    ocr = host->ocr = mmc_select_voltage(host, ocr);

    /*
     * Can we support the voltage(s) of the card(s)?
     */
    if (!host->ocr) {
        err = MMC_ERR_FAILED;
        goto out;
    }

    mmc_go_idle(host);

    /* send interface condition */
    if (mmc_card_sd(card))
        err = mmc_send_if_cond(host, ocr);

    /* host support HCS[30] */
    ocr |= (1 << 30);

#ifdef FEATURE_MMC_UHS1
    if (!err) {
        /* host support S18A[24] and XPC[28]=1 to support speed class */
        if (host->caps & MMC_CAP_SD_UHS1)
            ocr |= ((1 << 28) | (1 << 24));
    }
#endif

    /* send operation condition */
#ifdef USE_2WAY_ACMD41_CMD1
alternative_mmc_init_mem_card_flow:
#endif
    if (mmc_card_sd(card)) {
        err = mmc_send_app_op_cond(host, ocr, &card->ocr);
    } else {
    /* The extra bit indicates that we support high capacity */
        err = mmc_send_op_cond(host, ocr, &card->ocr);
    }

    if (err != MMC_ERR_NONE) {
        #ifdef USE_2WAY_ACMD41_CMD1
        if ( !bSP_Way_ACMD41_CMD1 && mmc_card_sd(card) ) {
            ocr=MMC_VDD_27_36|MMC_CARD_HCS;
            mmc_card_set_unknown(card);
            mmc_card_set_mmc(card);
            goto alternative_mmc_init_mem_card_flow;
        }
        #endif

        printf("[SD%d] Fail in SEND_OP_COND cmd\n", id);
        goto out;
    }

    /* set hcs bit if a high-capacity card */
    card->state |= ((card->ocr >> 30) & 0x1) ? MMC_STATE_HIGHCAPS : 0;
#ifdef FEATURE_MMC_UHS1
    s18a = (card->ocr >> 24) & 0x1;

    /* S18A support by card. switch to 1.8V signal */
    if (s18a) {
        err = mmc_switch_volt(host, card);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in SWITCH_VOLT cmd\n", id);
            goto out;
        }
    }
#endif
    /* send cid */
    err = mmc_all_send_cid(host, card->raw_cid);

    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in SEND_CID cmd\n", id);
        goto out;
    }
    //Move here by 82 FPGA
    mmc_decode_cid(card);

    if (mmc_card_mmc(card))
        card->rca = 0x1; /* assign a rca */

    /* set/send rca */
    err = mmc_send_relative_addr(host, card, &card->rca);
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in SEND_RCA cmd\n", id);
        goto out;
    }

    /* send csd */
    err = mmc_read_csds(host, card);
    if (err != MMC_ERR_NONE) {
        goto out;
    }

    /* decode csd */
    err = mmc_decode_csd(card);
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in decode csd\n", id);
        goto out;
    }
    /* 2012-02-25: bug, should decode csd first */
    //Comment out by moving ahead
    //mmc_decode_cid(card);

    /* select this card */
    err = mmc_select_card(host, card);
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in select card cmd\n", id);
        goto out;
    }

    if (mmc_card_sd(card)) {
        #if defined(FEATURE_MMC_STRIPPED)
        /* send scr */
        err = mmc_read_scrs(host, card);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in SEND_SCR cmd\n", id);
            goto out;
        }
        #endif

        if ((card->csd.cmdclass & CCC_SWITCH) &&
            (mmc_read_switch(host, card) == MMC_ERR_NONE)) {
            do {
                #ifdef FEATURE_MMC_UHS1
                if (s18a && (host->caps & MMC_CAP_SD_UHS1)) {
                    /* TODO: Switch driver strength first then current limit
                     *       and access mode */
                    unsigned int freq, uhs_mode, drv_type, max_curr;
                    freq = min(host->f_max, card->sw_caps.hs_max_dtr);

                    if (freq > 100000000) {
                        uhs_mode = MMC_SWITCH_MODE_SDR104;
                    } else if (freq <= 100000000 && freq > 50000000) {
                        if (card->sw_caps.ddr && host->caps & MMC_CAP_DDR) {
                            uhs_mode = MMC_SWITCH_MODE_DDR50;
                        } else {
                            uhs_mode = MMC_SWITCH_MODE_SDR50;
                        }
                    } else if (freq <= 50000000 && freq > 25000000) {
                        uhs_mode = MMC_SWITCH_MODE_SDR25;
                    } else {
                        uhs_mode = MMC_SWITCH_MODE_SDR12;
                    }
                    drv_type = MMC_SWITCH_MODE_DRV_TYPE_B;
                    max_curr = MMC_SWITCH_MODE_CL_200MA;

                    if (mmc_switch_drv_type(host, card, drv_type) == MMC_ERR_NONE &&
                        mmc_switch_max_cur(host, card, max_curr) == MMC_ERR_NONE &&
                        mmc_switch_uhs1(host, card, uhs_mode) == MMC_ERR_NONE) {
                        break;
                    } else {
                        mmc_switch_drv_type(host, card, MMC_SWITCH_MODE_DRV_TYPE_B);
                        mmc_switch_max_cur(host, card, MMC_SWITCH_MODE_CL_200MA);
                    }
                }
                #endif
                #ifdef FEATURE_MMC_HS
                if (host->caps & MMC_CAP_SD_HIGHSPEED) {
                    mmc_switch_hs(host, card);
                    break;
                }
                #endif
            } while(0);
        }

        /* set bus width */
        mmc_set_bus_width(host, card, HOST_BUS_WIDTH_4);

        /* compute bus speed. */
        card->maxhz = (unsigned int)-1;

        if (mmc_card_highspeed(card) || mmc_card_uhs1(card))
        {
            if (card->maxhz > card->sw_caps.hs_max_dtr)
                card->maxhz = card->sw_caps.hs_max_dtr;
        } else if (card->maxhz > card->csd.max_dtr) {
            card->maxhz = card->csd.max_dtr;
        }

    } else {
        if(!(host->ocr_avail & MMC_VDD_165_195)){ //Use definition reference from linux driver
            host->caps  = host->caps &(~MMC_CAP_EMMC_HS200);
            MSG(INF, "[SD%d] can not switch to HS200:Host voltage not support!\n",id);
        }

        /* send ext csd */
        err = mmc_read_ext_csd(host, card);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in SEND_EXT_CSD cmd\n", id);
            goto out;
        }

        #ifdef FEATURE_MMC_HS
        /* activate high speed (if supported) */
        if ((card->ext_csd.hs_max_dtr > 52000000) && (host->caps & MMC_CAP_EMMC_HS200)){
            //For HS200 timing, bus width shall be set to 4/8 bit first
            mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);

            err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 2);
            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to HS200 mode\n", host->id);
                mmc_card_set_hs200(card);
            }
        }
        else if ((card->ext_csd.hs_max_dtr != 0) && (host->caps & MMC_CAP_MMC_HIGHSPEED)) {
            err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to HS mode\n", host->id);
                mmc_card_set_highspeed(card);
            }

            //For High speed, bus width shall be schanged after bus timing
            mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8); //Set to 8bit and DDR
        }
        #endif

        /* compute bus speed. */
        card->maxhz = (unsigned int)-1;

        if (mmc_card_highspeed(card)) {
            card->maxhz = 52000000;
        } else if ( mmc_card_hs200(card)) {
            if (card->maxhz > card->ext_csd.hs_max_dtr)
                card->maxhz = card->ext_csd.hs_max_dtr;
        } else if (card->maxhz > card->csd.max_dtr) {
            card->maxhz = card->csd.max_dtr;
        }
    }

    /* set block len. note that cmd16 is illegal while mmc card is in ddr mode */
    if (!(mmc_card_mmc(card) && mmc_card_ddr(card))) {
        err = mmc_set_blk_length(host, MMC_BLOCK_SIZE);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in set blklen cmd\n", id);
            goto out;
        }
    }

    /* set clear card detect */
    if (mmc_card_sd(card))
        mmc_set_card_detect(host, card, 0);

    if (!mmc_card_sd(card) && mmc_card_blockaddr(card)) {
        /* The EXT_CSD sector count is in number or 512 byte sectors. */
        card->blklen = MMC_BLOCK_SIZE;
        card->nblks  = card->ext_csd.sectors;
    } else {
        /* The CSD capacity field is in units of read_blkbits.
         * set_capacity takes units of 512 bytes.
         */
        card->blklen = MMC_BLOCK_SIZE;
        card->nblks  = card->csd.capacity << (card->csd.read_blkbits - 9);
    }

    printf("[SD%d] Size: %d MB, Max.Speed: %d kHz, blklen(%d), nblks(%d), ro(%d)\n",
        id, ((card->nblks / 1024) * card->blklen) / 1024 , card->maxhz / 1000,
        card->blklen, card->nblks, mmc_card_readonly(card));

    card->ready = 1;

    printf("[SD%d] Initialized\n", id);

out:
    return err;
}


int mmc_init_card(struct mmc_host *host, struct mmc_card *card)
{
    int err, id = host->id;
    u32 ocr;

    memset(card, 0, sizeof(struct mmc_card));
    mmc_prof_init(id, host, card);
    mmc_prof_start();

#ifdef FEATURE_MMC_CARD_DETECT
    if (!msdc_card_avail(host)) {
        err = MMC_ERR_INVALID;
        goto out;
    }
#endif

#if 0
    //Neither Preloader or LK enable this section
    if (msdc_card_protected(host))
        mmc_card_set_readonly(card);
#endif

    mmc_card_set_present(card);
    mmc_card_set_host(card, host);
    mmc_card_set_unknown(card);

    mmc_go_idle(host);

    /* send interface condition */ //CMD8
    err=mmc_send_if_cond(host, host->ocr_avail);
    #ifdef USE_2WAY_ACMD41_CMD1
    if ( err==MMC_ERR_NONE ) {
        //High density or Extended density
        ocr=MMC_VDD_27_36|MMC_CARD_HCS;
    } else {
        //Standard density
        ocr=MMC_VDD_27_36;
    }
    #endif

#ifdef FEATURE_MMC_SDIO
    if (mmc_send_io_op_cond(host, 0, &ocr) == MMC_ERR_NONE) {
        mmc_card_set_sdio(card);
        err = mmc_init_sdio_card(host, card, ocr);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in init sdio card\n", id);
            goto out;
        }
        /* no memory present */
        if ((ocr & 0x08000000) == 0) {
            goto out;
        }
    }
#endif

    #ifdef USE_2WAY_ACMD41_CMD1
    if ( EFUSE_Query_SD_Card_Init_Flow() )
    {
    #endif
        //SP Way
        /* query operation condition */
        err = mmc_send_app_op_cond(host, 0, &ocr);
        if (err != MMC_ERR_NONE) {
            err = mmc_send_op_cond(host, 0, &ocr);
            if (err != MMC_ERR_NONE) {
                printf("[SD%d] Fail in SEND_IF_COND cmd\n", id);
                goto out;
            }
            mmc_card_set_mmc(card);
        } else {
            mmc_card_set_sd(card);
        }
    #ifdef USE_2WAY_ACMD41_CMD1
    }
    #endif

    err = mmc_init_mem_card(host, card, ocr);

    if (err)
        goto out;

    /* change clock */
    if ( (host->caps & MMC_CAP_DDR) && card->ext_csd.ddr_support )
        msdc_config_clock(host, 1, card->maxhz);
    else
        msdc_config_clock(host, 0, card->maxhz);

    /* 2012-02-25 lookup the ETT table */
    #if 0 //<--Yuchi said that LK/Preloader do not use it
    msdc_ett_offline(host, card);
    #endif

    #ifdef FEATURE_MMC_UHS1
    /* tune timing */
    mmc_tune_timing(host, card);
    #endif

out:
    mmc_prof_stop();
    mmc_prof_update(mmc_prof_card_init, (ulong)id, (void*)err);
    if (err) {
        //To Do: 82 FPGA enable the following line. Check if it is necessary
        //msdc_power(host, MMC_POWER_OFF);
        return err;
    }
    host->card = card;
    return 0;
}

int mmc_init_host(struct mmc_host *host, int id, int clksrc, u8 mode)
{
    memset(host, 0, sizeof(struct mmc_host));

    return msdc_init(id, host, clksrc, mode);
}

int mmc_init(int id, u32 trans_mode)
{
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;

    BUG_ON(id >= NR_MMC);

    host = &sd_host[id];
    card = &sd_card[id];
    err = mmc_init_host(host, id, -1, trans_mode);

    if (err == MMC_ERR_NONE)
        err = mmc_init_card(host, card);

#ifdef MTK_EMMC_SUPPORT_OTP
    printf("=========use hc erase size\n");
    mmc_set_erase_grp_def(card, 1);
#endif

#ifdef MMC_TEST
    //mmc_test(0, NULL);
#endif

    return err;
}

#if defined(MSDC_WITH_DEINIT)
int mmc_deinit(int id)
{
    int err = MMC_ERR_NONE;

    BUG_ON(id >= NR_MMC);

    err = msdc_deinit(&sd_host[id]);

    return err;
}
#endif

//#endif

int mmc_set_erase_grp_def(struct mmc_card *card, int enable)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card) || !mmc_card_highcaps(card))
    goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
    goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_ERASE_GRP_DEF,
        EXT_CSD_ERASE_GRP_DEF_EN & enable);

out:
    return err;
}

////////////////////////////////////////////////////////////
//
// Functions related to paration operation
//
////////////////////////////////////////////////////////////

int mmc_set_gp_size(struct mmc_card *card, u8 id, u32 size)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8 gp[] = { EXT_CSD_GP1_SIZE_MULT, EXT_CSD_GP2_SIZE_MULT,
        EXT_CSD_GP3_SIZE_MULT, EXT_CSD_GP4_SIZE_MULT };
    u8 arg;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || !mmc_card_highcaps(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    id--;
    size /= 512 * 1024;
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    /* 143-144: GP_SIZE_MULT_X_0-GP_SIZE_MULT_X_2 */
    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err = mmc_set_ext_csd(card, gp[id] + i, arg);
        if (err)
            goto out;
    }

out:
    return err;
}

int mmc_set_part_config(struct mmc_card *card, u8 cfg)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_PART_CFG, cfg);

out:
    return err;
}

int mmc_set_part_attr(struct mmc_card *card, u8 attr)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
    goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
    goto out;

    if (!card->ext_csd.enh_attr_en) {
        err = MMC_ERR_INVALID;
        goto out;
    }

    attr &= 0x1F;
    attr |= (card->raw_ext_csd[EXT_CSD_PART_ATTR] & 0x1F);
    err = mmc_set_ext_csd(card, EXT_CSD_PART_ATTR, attr);

out:
    return err;
}

int mmc_set_part_compl(struct mmc_card *card)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_set_ext_csd(card, EXT_CSD_PART_SET_COMPL, EXT_CSD_PART_SET_COMPL_BIT);

out:
    return err;
}

#if defined(FEATURE_MMC_BOOT_MODE)
#if defined(MMC_MSDC_DRV_CTP)
int mmc_part_read(struct mmc_card *card, u8 partno, unsigned long blknr, u32 blkcnt, unsigned long *dst)
{
    int err = MMC_ERR_FAILED;
    u8 val;
    u8 *ext_csd = &card->raw_ext_csd[0];
    struct mmc_host *host = card->host;

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    /* configure to specified partition */
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | (partno & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE)
        goto out;

    /* write block to this partition */
    err = mmc_block_read(host->id, blknr, blkcnt, dst);

out:
    /* configure to user partition */
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
    mmc_set_part_config(card, val);

    return err;
}

int mmc_part_write(struct mmc_card *card, u8 partno, unsigned long blknr, u32 blkcnt, unsigned long *src)
{
    int err = MMC_ERR_FAILED;
    u8 val;
    u8 *ext_csd = &card->raw_ext_csd[0];
    struct mmc_host *host = card->host;

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 ||
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    /* configure to specified partition */
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | (partno & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE)
        goto out;

    /* write block to this partition */
    err = mmc_block_write(host->id, blknr, blkcnt, src);

out:
    /* configure to user partition */
    val = (ext_csd[EXT_CSD_PART_CFG] & ~0x7) | EXT_CSD_PART_CFG_DEFT_PART;
    mmc_set_part_config(card, val);

    return err;
}
#endif
#endif


////////////////////////////////////////////////////////////
//
// Functions related to enhanced area
//
////////////////////////////////////////////////////////////

int mmc_set_enh_size(struct mmc_card *card, u32 size)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8 arg;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || !mmc_card_highcaps(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    /* need to set ERASE_GRP_DEF first?? */
    if (0 == (card->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN))
        goto out;

    size /= (512 * 1024);
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    /* 140-142: ENH_SIZE_MULT0-ENH_SIZE_MULT2 */
    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err = mmc_set_ext_csd(card, EXT_CSD_ENH_SIZE_MULT + i, arg);
        if (err)
            goto out;
    }

out:
    return err;
}

int mmc_set_enh_start_addr(struct mmc_card *card, u32 addr)
{
    int i;
    int err = MMC_ERR_FAILED;
    u8 arg;

    if (mmc_card_sd(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    /* need to set ERASE_GRP_DEF first?? */
    if (0 == (card->raw_ext_csd[EXT_CSD_ERASE_GRP_DEF] & EXT_CSD_ERASE_GRP_DEF_EN))
        goto out;

    /* start address would be round to protect group aligned. */
    if (mmc_card_highcaps(card))
        addr = addr / 512; /* in sector unit. otherwise in byte unit */

    /* 136-139: ENH_START_ADDR0-ENH_START_ADDR3 */
    for (i = 0; i < 4; i++) {
        arg  = (u8)(addr & 0xFF);
        addr = addr >> 8;
        err = mmc_set_ext_csd(card, EXT_CSD_ENH_START_ADDR + i, arg);
        if (err)
            goto out;
    }

out:
    return err;
}

#if defined(MMC_MSDC_DRV_PRELOADER)
void mmc_stuff_buff(u8* buf)
{
	memset(buf,0,512);
	buf[0] = 0x10;
	buf[1] = 0x06;
	buf[2] = 0x01;
	buf[3] = 0xF0;
	buf[11]= 0xAA;
	buf[12]= 0xA9;
	buf[13]= 0x87;
	buf[14]= 0x74;
	buf[15]= 0x3C;
	buf[16]= 0x71;
	buf[17]= 0xFB;
	buf[18]= 0xD4;
}

int mmc_get_sandisk_fwid(int id, u8* buf)
{
	struct mmc_host *host;
	struct mmc_card *card;
	struct mmc_command stop;
	int err = MMC_ERR_NONE;
	u32 status;
	u32 state = 0;
	host = &sd_host[id];
    card = &sd_card[id];
	while (state != 4) {
        err = mmc_send_status(host, card, &status);
        if (err) {
            printf("[SD%d] Fail to send status %d\n", host->id, err);
            return err;
        }
        state = R1_CURRENT_STATE(status);
            printf("check card state<%d>\n", state);
            if (state == 5 || state == 6) {
                printf("state<%d> need cmd12 to stop\n", state);
                stop.opcode  = MMC_CMD_STOP_TRANSMISSION;
                stop.rsptyp  = RESP_R1B;
                stop.arg     = 0;
                stop.retries = CMD_RETRIES;
                stop.timeout = CMD_TIMEOUT;
                msdc_send_cmd(host, &stop);
                msdc_wait_rsp(host, &stop); // don't tuning
            } else if (state == 7) {  // busy in programing
                printf("state<%d> card is busy\n", state);
                mdelay(100);
            } else if (state != 4) {
                printf("state<%d> ??? \n", state);
                return MMC_ERR_INVALID;
            }
        }
	mmc_stuff_buff(buf);
#if defined(MSDC_USE_DMA_MODE)
    err = msdc_dma_send_sandisk_fwid(host, buf,MMC_CMD50,1);
    if (err) {
        printf("[SD%d] Fail to %s sandisk fwid %d\n", host->id, "send(CMD50)", err);
        return err;
    }

    err = msdc_dma_send_sandisk_fwid(host, buf,MMC_CMD21,1);
    if (err) {
        printf("[SD%d] Fail to %s sandisk fwid %d\n", host->id, "get(CMD21)", err);
        return err;
    }

#else
    err = msdc_pio_send_sandisk_fwid(host, buf, MMC_CMD50);
    if (err) {
        printf("[SD%d] Fail to %s sandisk fwid %d\n", host->id, "send(CMD50)", err);
        return err;
    }

    err = msdc_pio_send_sandisk_fwid(host, buf, MMC_CMD21);
    if (err) {
        printf("[SD%d] Fail to %s sandisk fwid %d\n", host->id, "get(CMD21)", err);
        return err;
    }
#endif
	return err;
}
#endif

