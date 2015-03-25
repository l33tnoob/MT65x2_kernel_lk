/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2010
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include "msdc.h"

#if defined(MMC_MSDC_DRV_CTP)
#include <common.h>
#endif
//#include <string.h> //need by CTP?


#if MMC_DEBUG
void mmc_dump_card_status(u32 card_status)
{
    static char *state[] = {
    	"Idle", 	/* 0 */
    	"Ready",	/* 1 */
    	"Ident",	/* 2 */
    	"Stby", 	/* 3 */
    	"Tran", 	/* 4 */
    	"Data", 	/* 5 */
    	"Rcv",		/* 6 */
    	"Prg",		/* 7 */
    	"Dis",		/* 8 */
    	"Reserved",	/* 9 */
    	"Reserved",	/* 10 */
    	"Reserved",	/* 11 */
    	"Reserved",	/* 12 */
    	"Reserved",	/* 13 */
    	"Reserved",	/* 14 */
    	"I/O mode",	/* 15 */
    };
    if (card_status & R1_OUT_OF_RANGE)
	    printf("\t[CARD_STATUS] Out of Range\n");
    if (card_status & R1_ADDRESS_ERROR)
	    printf("\t[CARD_STATUS] Address Error\n");
    if (card_status & R1_BLOCK_LEN_ERROR)
	    printf("\t[CARD_STATUS] Block Len Error\n");
    if (card_status & R1_ERASE_SEQ_ERROR)
	    printf("\t[CARD_STATUS] Erase Seq Error\n");
    if (card_status & R1_ERASE_PARAM)
	    printf("\t[CARD_STATUS] Erase Param\n");
    if (card_status & R1_WP_VIOLATION)
	    printf("\t[CARD_STATUS] WP Violation\n");
    if (card_status & R1_CARD_IS_LOCKED)
	    printf("\t[CARD_STATUS] Card is Locked\n");
    if (card_status & R1_LOCK_UNLOCK_FAILED)
	    printf("\t[CARD_STATUS] Lock/Unlock Failed\n");
    if (card_status & R1_COM_CRC_ERROR)
	    printf("\t[CARD_STATUS] Command CRC Error\n");
    if (card_status & R1_ILLEGAL_COMMAND)
	    printf("\t[CARD_STATUS] Illegal Command\n");
    if (card_status & R1_CARD_ECC_FAILED)
	    printf("\t[CARD_STATUS] Card ECC Failed\n");
    if (card_status & R1_CC_ERROR)
	    printf("\t[CARD_STATUS] CC Error\n");
    if (card_status & R1_ERROR)
	    printf("\t[CARD_STATUS] Error\n");
    if (card_status & R1_UNDERRUN)
	    printf("\t[CARD_STATUS] Underrun\n");
    if (card_status & R1_OVERRUN)
	    printf("\t[CARD_STATUS] Overrun\n");
    if (card_status & R1_CID_CSD_OVERWRITE)
	    printf("\t[CARD_STATUS] CID/CSD Overwrite\n");
    if (card_status & R1_WP_ERASE_SKIP)
	    printf("\t[CARD_STATUS] WP Eraser Skip\n");
    if (card_status & R1_CARD_ECC_DISABLED)
	    printf("\t[CARD_STATUS] Card ECC Disabled\n");
    if (card_status & R1_ERASE_RESET)
	    printf("\t[CARD_STATUS] Erase Reset\n");
    if (card_status & R1_READY_FOR_DATA)
	    printf("\t[CARD_STATUS] Ready for Data\n");
    if (card_status & R1_SWITCH_ERROR)
	    printf("\t[CARD_STATUS] Switch error\n");
    if (card_status & R1_URGENT_BKOPS)
	    printf("\t[CARD_STATUS] Urgent background operations\n");
    if (card_status & R1_APP_CMD)
	    printf("\t[CARD_STATUS] App Command\n");

    printf("\t[CARD_STATUS] '%s' State\n",
    state[R1_CURRENT_STATE(card_status)]);
}

void mmc_dump_ocr_reg(u32 resp)
{
    if (resp & (1 << 7))
	    printf("\t[OCR] Low Voltage Range\n");
    if (resp & (1 << 15))
	    printf("\t[OCR] 2.7-2.8 volt\n");
    if (resp & (1 << 16))
	    printf("\t[OCR] 2.8-2.9 volt\n");
    if (resp & (1 << 17))
	    printf("\t[OCR] 2.9-3.0 volt\n");
    if (resp & (1 << 18))
	    printf("\t[OCR] 3.0-3.1 volt\n");
    if (resp & (1 << 19))
	    printf("\t[OCR] 3.1-3.2 volt\n");
    if (resp & (1 << 20))
	    printf("\t[OCR] 3.2-3.3 volt\n");
    if (resp & (1 << 21))
	    printf("\t[OCR] 3.3-3.4 volt\n");
    if (resp & (1 << 22))
	    printf("\t[OCR] 3.4-3.5 volt\n");
    if (resp & (1 << 23))
	    printf("\t[OCR] 3.5-3.6 volt\n");
    if (resp & (1 << 24))
	    printf("\t[OCR] Switching to 1.8V Accepted (S18A)\n");
    if (resp & (1 << 30))
	    printf("\t[OCR] Card Capacity Status (CCS)\n");
    if (resp & (1UL << 31))
	    printf("\t[OCR] Card Power Up Status (Idle)\n");
    else
	    printf("\t[OCR] Card Power Up Status (Busy)\n");
}

void mmc_dump_rca_resp(u32 resp)
{
    u32 card_status = (((resp >> 15) & 0x1) << 23) |
		      (((resp >> 14) & 0x1) << 22) |
		      (((resp >> 13) & 0x1) << 19) |
			(resp & 0x1fff);

    printf("\t[RCA] 0x%x\n", resp >> 16);
    mmc_dump_card_status(card_status);
}

void mmc_dump_io_resp(u32 resp)
{
    u32 flags = (resp >> 8) & 0xFF;
    char *state[] = {"DIS", "CMD", "TRN", "RFU"};

    if (flags & (1 << 7))
	    printf("\t[IO] COM_CRC_ERR\n");
    if (flags & (1 << 6))
	    printf("\t[IO] Illgal command\n");
    if (flags & (1 << 3))
	    printf("\t[IO] Error\n");
    if (flags & (1 << 2))
	    printf("\t[IO] RFU\n");
    if (flags & (1 << 1))
	    printf("\t[IO] Function number error\n");
    if (flags & (1 << 0))
	    printf("\t[IO] Out of range\n");

    printf("[IO] State: %s, Data:0x%x\n", state[(resp >> 12) & 0x3], resp & 0xFF);
}

void mmc_dump_tuning_blk(u8 *buf)
{
    int i;
    for (i = 0; i < 16; i++) {
    	printf("[TBLK%d] %x%x%x%x%x%x%x%x\n", i,
    	    (buf[(i<<2)] >> 4) & 0xF, buf[(i<<2)] & 0xF,
    	    (buf[(i<<2)+1] >> 4) & 0xF, buf[(i<<2)+1] & 0xF,
    	    (buf[(i<<2)+2] >> 4) & 0xF, buf[(i<<2)+2] & 0xF,
    	    (buf[(i<<2)+3] >> 4) & 0xF, buf[(i<<2)+3] & 0xF);
    }
}

void mmc_dump_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    u32 *resp = card->raw_csd;
    int i;
    unsigned int csd_struct;
    static char *sd_csd_ver[] = {"v1.0", "v2.0"};
    static char *mmc_csd_ver[] = {"v1.0", "v1.1", "v1.2", "Ver. in EXT_CSD"};
    static char *mmc_cmd_cls[] = {"basic", "stream read", "block read",
	    "stream write", "block write", "erase", "write prot", "lock card",
	    "app-spec", "I/O", "rsv.", "rsv."};
    static char *sd_cmd_cls[] = {"basic", "rsv.", "block read",
	    "rsv.", "block write", "erase", "write prot", "lock card",
	    "app-spec", "I/O", "switch", "rsv."};

    if (mmc_card_sd(card)) {
    	csd_struct = unstuff_bits(resp, 126, 2);
    	printf("[CSD] CSD %s\n", sd_csd_ver[csd_struct]);
    	printf("[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
    	if (csd_struct == 1) {
    	    printf("[CSD] Read/Write Blk Len = 512bytes\n");
    	} else {
    	    printf("[CSD] Read Blk Len = %d, Write Blk Len = %d\n",
    		    1 << csd->read_blkbits, 1 << csd->write_blkbits);
    	}
    	printf("[CSD] CMD Class:");
    	for (i = 0; i < 12; i++) {
    	    if ((csd->cmdclass >> i) & 0x1)
    		    printf("'%s' ", sd_cmd_cls[i]);
    	}
    	printf("\n");
    } else {
    	csd_struct = unstuff_bits(resp, 126, 2);
    	printf("[CSD] CSD %s\n", mmc_csd_ver[csd_struct]);
    	printf("[CSD] MMCA Spec v%d\n", csd->mmca_vsn);
    	printf("[CSD] TACC_NS: %d ns, TACC_CLKS: %d clks\n", csd->tacc_ns, csd->tacc_clks);
    	printf("[CSD] Read Blk Len = %d, Write Blk Len = %d\n",
    	    1 << csd->read_blkbits, 1 << csd->write_blkbits);
    	printf("[CSD] CMD Class:");
    	for (i = 0; i < 12; i++) {
    	    if ((csd->cmdclass >> i) & 0x1)
    		    printf("'%s' ", mmc_cmd_cls[i]);
    	}
    	printf("\n");
    }
}

void mmc_dump_ext_csd(struct mmc_card *card)
{
    u8 *ext_csd = &card->raw_ext_csd[0];
    u32 tmp;
    char *rev[] = { "4.0", "4.1", "4.2", "4.3", "Obsolete", "4.41" };

    printf("===========================================================\n");
    printf("[EXT_CSD] EXT_CSD rev.		: v1.%d (MMCv%s)\n",
	    ext_csd[EXT_CSD_REV], rev[ext_csd[EXT_CSD_REV]]);
    printf("[EXT_CSD] CSD struct rev.		: v1.%d\n", ext_csd[EXT_CSD_STRUCT]);
    printf("[EXT_CSD] Supported command sets	: %xh\n", ext_csd[EXT_CSD_S_CMD_SET]);
    printf("[EXT_CSD] HPI features		: %xh\n", ext_csd[EXT_CSD_HPI_FEATURE]);
    printf("[EXT_CSD] BG operations support	: %xh\n", ext_csd[EXT_CSD_BKOPS_SUPP]);
    printf("[EXT_CSD] BG operations status	: %xh\n", ext_csd[EXT_CSD_BKOPS_STATUS]);
        memcpy(&tmp, &ext_csd[EXT_CSD_CORRECT_PRG_SECTS_NUM], 4);
    printf("[EXT_CSD] Correct prg. sectors	: %xh\n", tmp);
    printf("[EXT_CSD] 1st init time after part. : %d ms\n", ext_csd[EXT_CSD_INI_TIMEOUT_AP] * 100);
    printf("[EXT_CSD] Min. write perf.(DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_W_8_52]);
    printf("[EXT_CSD] Min. read perf. (DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_R_8_52]);
    printf("[EXT_CSD] TRIM timeout: %d ms\n", ext_csd[EXT_CSD_TRIM_MULT] & 0xFF * 300);
    printf("[EXT_CSD] Secure feature support: %xh\n", ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT]);
    printf("[EXT_CSD] Secure erase timeout  : %d ms\n", 300 *
	    ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_ERASE_MULT]);
    printf("[EXT_CSD] Secure trim timeout   : %d ms\n", 300 *
	    ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_TRIM_MULT]);
    printf("[EXT_CSD] Access size	    : %d bytes\n", ext_csd[EXT_CSD_ACC_SIZE] * 512);
    printf("[EXT_CSD] HC erase unit size    : %d kbytes\n", ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512);
    printf("[EXT_CSD] HC erase timeout	    : %d ms\n", ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * 300);
    printf("[EXT_CSD] HC write prot grp size: %d kbytes\n", 512 *
	    ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * ext_csd[EXT_CSD_HC_WP_GPR_SIZE]);
    printf("[EXT_CSD] HC erase grp def.     : %xh\n", ext_csd[EXT_CSD_ERASE_GRP_DEF]);
    printf("[EXT_CSD] Reliable write sect count: %xh\n", ext_csd[EXT_CSD_REL_WR_SEC_C]);
    printf("[EXT_CSD] Sleep current (VCC) : %xh\n", ext_csd[EXT_CSD_S_C_VCC]);
    printf("[EXT_CSD] Sleep current (VCCQ): %xh\n", ext_csd[EXT_CSD_S_C_VCCQ]);
    printf("[EXT_CSD] Sleep/awake timeout : %d ns\n",
	    100 * (2 << ext_csd[EXT_CSD_S_A_TIMEOUT]));
    memcpy(&tmp, &ext_csd[EXT_CSD_SEC_CNT], 4);
    printf("[EXT_CSD] Sector count : %xh\n", tmp);
    printf("[EXT_CSD] Min. WR Perf.  (52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_8_52]);
    printf("[EXT_CSD] Min. Read Perf.(52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_8_52]);
    printf("[EXT_CSD] Min. WR Perf.  (26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_8_26_4_25]);
    printf("[EXT_CSD] Min. Read Perf.(26MH,8b,52MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_8_26_4_25]);
    printf("[EXT_CSD] Min. WR Perf.  (26MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_W_4_26]);
    printf("[EXT_CSD] Min. Read Perf.(26MH,4b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_R_4_26]);
    printf("[EXT_CSD] Power class: %x\n", ext_csd[EXT_CSD_PWR_CLASS]);
    printf("[EXT_CSD] Power class(DDR,52MH,3.6V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_360]);
    printf("[EXT_CSD] Power class(DDR,52MH,1.9V): %xh\n", ext_csd[EXT_CSD_PWR_CL_DDR_52_195]);
    printf("[EXT_CSD] Power class(26MH,3.6V)	: %xh\n", ext_csd[EXT_CSD_PWR_CL_26_360]);
    printf("[EXT_CSD] Power class(52MH,3.6V)	: %xh\n", ext_csd[EXT_CSD_PWR_CL_52_360]);
    printf("[EXT_CSD] Power class(26MH,1.9V)	: %xh\n", ext_csd[EXT_CSD_PWR_CL_26_195]);
    printf("[EXT_CSD] Power class(52MH,1.9V)	: %xh\n", ext_csd[EXT_CSD_PWR_CL_52_195]);
    printf("[EXT_CSD] Part. switch timing    : %xh\n", ext_csd[EXT_CSD_PART_SWITCH_TIME]);
    printf("[EXT_CSD] Out-of-INTR busy timing: %xh\n", ext_csd[EXT_CSD_OUT_OF_INTR_TIME]);
    printf("[EXT_CSD] Card type       : %xh\n", ext_csd[EXT_CSD_CARD_TYPE]);
    printf("[EXT_CSD] Command set     : %xh\n", ext_csd[EXT_CSD_CMD_SET]);
    printf("[EXT_CSD] Command set rev.: %xh\n", ext_csd[EXT_CSD_CMD_SET_REV]);
    printf("[EXT_CSD] HS timing       : %xh\n", ext_csd[EXT_CSD_HS_TIMING]);
    printf("[EXT_CSD] Bus width       : %xh\n", ext_csd[EXT_CSD_BUS_WIDTH]);
    printf("[EXT_CSD] Erase memory content : %xh\n", ext_csd[EXT_CSD_ERASED_MEM_CONT]);
    printf("[EXT_CSD] Partition config	    : %xh\n", ext_csd[EXT_CSD_PART_CFG]);
    printf("[EXT_CSD] Boot partition size   : %d kbytes\n", ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128);
    printf("[EXT_CSD] Boot information	    : %xh\n", ext_csd[EXT_CSD_BOOT_INFO]);
    printf("[EXT_CSD] Boot config protection: %xh\n", ext_csd[EXT_CSD_BOOT_CONFIG_PROT]);
    printf("[EXT_CSD] Boot bus width	    : %xh\n", ext_csd[EXT_CSD_BOOT_BUS_WIDTH]);
    printf("[EXT_CSD] Boot area write prot  : %xh\n", ext_csd[EXT_CSD_BOOT_WP]);
    printf("[EXT_CSD] User area write prot  : %xh\n", ext_csd[EXT_CSD_USR_WP]);
    printf("[EXT_CSD] FW configuration	    : %xh\n", ext_csd[EXT_CSD_FW_CONFIG]);
    printf("[EXT_CSD] RPMB size : %d kbytes\n", ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128);
    printf("[EXT_CSD] Write rel. setting  : %xh\n", ext_csd[EXT_CSD_WR_REL_SET]);
    printf("[EXT_CSD] Write rel. parameter: %xh\n", ext_csd[EXT_CSD_WR_REL_PARAM]);
    printf("[EXT_CSD] Start background ops : %xh\n", ext_csd[EXT_CSD_BKOPS_START]);
    printf("[EXT_CSD] Enable background ops: %xh\n", ext_csd[EXT_CSD_BKOPS_EN]);
    printf("[EXT_CSD] H/W reset function   : %xh\n", ext_csd[EXT_CSD_RST_N_FUNC]);
    printf("[EXT_CSD] HPI management	   : %xh\n", ext_csd[EXT_CSD_HPI_MGMT]);
        memcpy(&tmp, &ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT], 4);
    printf("[EXT_CSD] Max. enhanced area size : %xh (%d kbytes)\n",
	    tmp & 0x00FFFFFF, (tmp & 0x00FFFFFF) * 512 *
	    ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Part. support  : %xh\n", ext_csd[EXT_CSD_PART_SUPPORT]);
    printf("[EXT_CSD] Part. attribute: %xh\n", ext_csd[EXT_CSD_PART_ATTR]);
    printf("[EXT_CSD] Part. setting  : %xh\n", ext_csd[EXT_CSD_PART_SET_COMPL]);
    printf("[EXT_CSD] General purpose 1 size : %xh (%d kbytes)\n",
    	(ext_csd[EXT_CSD_GP1_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] << 16),
    	(ext_csd[EXT_CSD_GP1_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP1_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP1_SIZE_MULT + 2] << 16) * 512 *
    	 ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
    	 ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 2 size : %xh (%d kbytes)\n",
    	(ext_csd[EXT_CSD_GP2_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] << 16),
    	(ext_csd[EXT_CSD_GP2_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP2_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP2_SIZE_MULT + 2] << 16) * 512 *
    	 ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
    	 ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 3 size : %xh (%d kbytes)\n",
    	(ext_csd[EXT_CSD_GP3_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] << 16),
    	(ext_csd[EXT_CSD_GP3_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP3_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP3_SIZE_MULT + 2] << 16) * 512 *
    	 ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
    	 ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] General purpose 4 size : %xh (%d kbytes)\n",
    	(ext_csd[EXT_CSD_GP4_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] << 16),
    	(ext_csd[EXT_CSD_GP4_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_GP4_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_GP4_SIZE_MULT + 2] << 16) * 512 *
    	 ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
    	 ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Enh. user area size : %xh (%d kbytes)\n",
    	(ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16),
    	(ext_csd[EXT_CSD_ENH_SIZE_MULT + 0] |
    	 ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 |
    	 ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 *
    	 ext_csd[EXT_CSD_HC_WP_GPR_SIZE] *
    	 ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
    printf("[EXT_CSD] Enh. user area start: %xh\n",
    	(ext_csd[EXT_CSD_ENH_START_ADDR + 0] |
    	 ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
    	 ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
    	 ext_csd[EXT_CSD_ENH_START_ADDR + 3]) << 24);
    printf("[EXT_CSD] Bad block mgmt mode: %xh\n", ext_csd[EXT_CSD_BADBLK_MGMT]);
    printf("===========================================================\n");
}
#endif

#ifdef MMC_PROFILING
void mmc_prof_card_init(void *data, ulong id, ulong counts)
{
    int err = (int)data;
    if (!err) {
	    printf("[SD%d] Init Card, %d counts, %d us\n",
	        id, counts, counts * 30 + counts * 16960 / 32768);
    }
}

//To do: combine mmc_prof_read() and mmc_prof_write()
void mmc_prof_read(void *data, ulong id, ulong counts)
{
    struct mmc_op_perf *perf = (struct mmc_op_perf *)data;
    struct mmc_op_report *rpt;
    u32 blksz = perf->host->blklen;
    u32 blkcnt = (u32)id;

    if (blkcnt > 1)
	    rpt = &perf->multi_blks_read;
    else
	    rpt = &perf->single_blk_read;

    rpt->count++;
    rpt->total_size += blkcnt * blksz;
    rpt->total_time += counts;
    if ((counts < rpt->min_time) || (rpt->min_time == 0))
	    rpt->min_time = counts;
    if ((counts > rpt->max_time) || (rpt->max_time == 0))
	    rpt->max_time = counts;

    printf("[SD%d] Read %d bytes, %d counts, %d us, %d KB/s, Avg: %d KB/s\n",
	    perf->host->id, blkcnt * blksz, counts,
	    counts * 30 + counts * 16960 / 32768,
	    blkcnt * blksz * 32 / (counts ? counts : 1),
	((rpt->total_size / 1024) * 32768) / rpt->total_time);
}

void mmc_prof_write(void *data, ulong id, ulong counts)
{
    struct mmc_op_perf *perf = (struct mmc_op_perf *)data;
    struct mmc_op_report *rpt;
    u32 blksz = perf->host->blklen;
    u32 blkcnt = (u32)id;

    if (blkcnt > 1)
	    rpt = &perf->multi_blks_write;
    else
	    rpt = &perf->single_blk_write;

    rpt->count++;
    rpt->total_size += blkcnt * blksz;
    rpt->total_time += counts;
    if ((counts < rpt->min_time) || (rpt->min_time == 0))
	    rpt->min_time = counts;
    if ((counts > rpt->max_time) || (rpt->max_time == 0))
	    rpt->max_time = counts;

    printf("[SD%d] Write %d bytes, %d counts, %d us, %d KB/s, Avg: %d KB/s\n",
	    perf->host->id, blkcnt * blksz, counts,
	    counts * 30 + counts * 16960 / 32768,
	    blkcnt * blksz * 32 / (counts ? counts : 1),
	    ((rpt->total_size / 1024) * 32768) / rpt->total_time);
}
#endif