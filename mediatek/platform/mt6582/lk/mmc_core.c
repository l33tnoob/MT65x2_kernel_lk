/*=======================================================================*/
/* HEADER FILES                                                          */
/*=======================================================================*/
#include <stdio.h>
#include <string.h>

#include <config.h>
#include <platform/msdc.h>
#include <platform/addr_trans.h>
#include <platform/msdc_utils.h>
#include <platform/mmc_core.h>
#include <platform/mmc_test.h>
#include <platform/mmc_hal.h>
#include <mt_partition.h>
#include <platform/mt_gpt.h>

//#include "pmt.h"
//#include "../kernel/drivers/mmc-host/emmc_device_list.h"

//#ifdef CONFIG_MMC

#define NR_MMC             (MSDC_MAX_NUM)
#define CMD_RETRIES        (5)
#define CMD_TIMEOUT        (100)    /* 100ms */
//#define ARRAY_SIZE(x)      (sizeof(x) / sizeof((x)[0]))

static struct mmc_host sd_host[NR_MMC];
static struct mmc_card sd_card[NR_MMC];
static block_dev_desc_t sd_dev[NR_MMC];
static int boot_dev_found = 0;
static part_dev_t boot_dev;
int g_user_virt_addr=0;
u64 g_emmc_size = 0;
#if 0
typedef struct{
	u32 image_index;
	u32 pc_checksum;
	u32 da_checksum;
	char checksum_status[8];
}CHECKSUM_INFO_EMMC;

typedef struct{
	char magic_num[32];
	CHECKSUM_INFO_EMMC part_info[PART_MAX_COUNT];
	char ram_checksum[16];
	char download_status[16];
}DL_STATUS_EMMC;

#define DL_INFO_SIZE ((u64)2048)
#define DL_NOT_FOUND 2
#define DL_PASS 0
#define DL_FAIL 1

char dl_buf[DL_INFO_SIZE];
#endif
static const unsigned int tran_exp[] = {
    10000,      100000,     1000000,    10000000,
    0,      0,      0,      0
};

static const unsigned char tran_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned char mmc_tran_mant[] = {
    0,  10, 12, 13, 15, 20, 26, 30,
    35, 40, 45, 52, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
    1,  10, 100,    1000,   10000,  100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static u32 unstuff_bits(u32 *resp, u32 start, u32 size)
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

#define MSDC_DECODE_BITS(r,s,sz)    unstuff_bits(r,s,sz)

#ifdef MMC_PROFILING
static void mmc_prof_card_init(void *data, u32 id, u32 counts)
{
    int err = (int)data;    
    if (!err) {
        printf("[SD%d] Init Card, %d counts, %d us\n", 
            id, counts, counts * 30 + counts * 16960 / 32768);        
    }
}

static void mmc_prof_read(void *data, u32 id, u32 counts)
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

static void mmc_prof_write(void *data, u32 id, u32 counts)
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

#if MMC_DEBUG
void mmc_dump_card_status(u32 card_status)
{
    msdc_dump_card_status(card_status);
}

static void mmc_dump_ocr_reg(u32 resp)
{
    msdc_dump_ocr_reg(resp);
}

static void mmc_dump_rca_resp(u32 resp)
{
    msdc_dump_rca_resp(resp);
}

static void mmc_dump_tuning_blk(u8 *buf)
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

static void mmc_dump_csd(struct mmc_card *card)
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
        csd_struct = MSDC_DECODE_BITS(resp, 126, 2);
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
        csd_struct = MSDC_DECODE_BITS(resp, 126, 2);
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
    printf("[EXT_CSD] EXT_CSD rev.              : v1.%d (MMCv%s)\n", 
        ext_csd[EXT_CSD_REV], rev[ext_csd[EXT_CSD_REV]]);
    printf("[EXT_CSD] CSD struct rev.           : v1.%d\n", ext_csd[EXT_CSD_STRUCT]);
    printf("[EXT_CSD] Supported command sets    : %xh\n", ext_csd[EXT_CSD_S_CMD_SET]);
    printf("[EXT_CSD] HPI features              : %xh\n", ext_csd[EXT_CSD_HPI_FEATURE]);
    printf("[EXT_CSD] BG operations support     : %xh\n", ext_csd[EXT_CSD_BKOPS_SUPP]);
    printf("[EXT_CSD] BG operations status      : %xh\n", ext_csd[EXT_CSD_BKOPS_STATUS]);
    memcpy(&tmp, &ext_csd[EXT_CSD_CORRECT_PRG_SECTS_NUM], 4);
    printf("[EXT_CSD] Correct prg. sectors      : %xh\n", tmp);
    printf("[EXT_CSD] 1st init time after part. : %d ms\n", ext_csd[EXT_CSD_INI_TIMEOUT_AP] * 100);
    printf("[EXT_CSD] Min. write perf.(DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_W_8_52]);
    printf("[EXT_CSD] Min. read perf. (DDR,52MH,8b): %xh\n", ext_csd[EXT_CSD_MIN_PERF_DDR_R_8_52]);
    printf("[EXT_CSD] TRIM timeout: %d ms\n", ext_csd[EXT_CSD_TRIM_MULT] & 0xFF * 300);
    printf("[EXT_CSD] Secure feature support: %xh\n", ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT]);
    printf("[EXT_CSD] Secure erase timeout  : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_ERASE_MULT]);
    printf("[EXT_CSD] Secure trim timeout   : %d ms\n", 300 *
        ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * ext_csd[EXT_CSD_SEC_TRIM_MULT]);
    printf("[EXT_CSD] Access size           : %d bytes\n", ext_csd[EXT_CSD_ACC_SIZE] * 512);
    printf("[EXT_CSD] HC erase unit size    : %d kbytes\n", ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512);
    printf("[EXT_CSD] HC erase timeout      : %d ms\n", ext_csd[EXT_CSD_ERASE_TIMEOUT_MULT] * 300);
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
    printf("[EXT_CSD] Power class(26MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_360]);
    printf("[EXT_CSD] Power class(52MH,3.6V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_360]);    
    printf("[EXT_CSD] Power class(26MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_26_195]);
    printf("[EXT_CSD] Power class(52MH,1.9V)    : %xh\n", ext_csd[EXT_CSD_PWR_CL_52_195]);
    printf("[EXT_CSD] Part. switch timing    : %xh\n", ext_csd[EXT_CSD_PART_SWITCH_TIME]);
    printf("[EXT_CSD] Out-of-INTR busy timing: %xh\n", ext_csd[EXT_CSD_OUT_OF_INTR_TIME]);  
    printf("[EXT_CSD] Card type       : %xh\n", ext_csd[EXT_CSD_CARD_TYPE]);
    printf("[EXT_CSD] Command set     : %xh\n", ext_csd[EXT_CSD_CMD_SET]);
    printf("[EXT_CSD] Command set rev.: %xh\n", ext_csd[EXT_CSD_CMD_SET_REV]);
    printf("[EXT_CSD] HS timing       : %xh\n", ext_csd[EXT_CSD_HS_TIMING]);
    printf("[EXT_CSD] Bus width       : %xh\n", ext_csd[EXT_CSD_BUS_WIDTH]);
    printf("[EXT_CSD] Erase memory content : %xh\n", ext_csd[EXT_CSD_ERASED_MEM_CONT]);
    printf("[EXT_CSD] Partition config      : %xh\n", ext_csd[EXT_CSD_PART_CFG]);
    printf("[EXT_CSD] Boot partition size   : %d kbytes\n", ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128);    
    printf("[EXT_CSD] Boot information      : %xh\n", ext_csd[EXT_CSD_BOOT_INFO]);    
    printf("[EXT_CSD] Boot config protection: %xh\n", ext_csd[EXT_CSD_BOOT_CONFIG_PROT]);
    printf("[EXT_CSD] Boot bus width        : %xh\n", ext_csd[EXT_CSD_BOOT_BUS_WIDTH]);
    printf("[EXT_CSD] Boot area write prot  : %xh\n", ext_csd[EXT_CSD_BOOT_WP]);
    printf("[EXT_CSD] User area write prot  : %xh\n", ext_csd[EXT_CSD_USR_WP]);
    printf("[EXT_CSD] FW configuration      : %xh\n", ext_csd[EXT_CSD_FW_CONFIG]);
    printf("[EXT_CSD] RPMB size : %d kbytes\n", ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128);
    printf("[EXT_CSD] Write rel. setting  : %xh\n", ext_csd[EXT_CSD_WR_REL_SET]);
    printf("[EXT_CSD] Write rel. parameter: %xh\n", ext_csd[EXT_CSD_WR_REL_PARAM]);
    printf("[EXT_CSD] Start background ops : %xh\n", ext_csd[EXT_CSD_BKOPS_START]);
    printf("[EXT_CSD] Enable background ops: %xh\n", ext_csd[EXT_CSD_BKOPS_EN]);
    printf("[EXT_CSD] H/W reset function   : %xh\n", ext_csd[EXT_CSD_RST_N_FUNC]);
    printf("[EXT_CSD] HPI management       : %xh\n", ext_csd[EXT_CSD_HPI_MGMT]);
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
#define FEATURE_MMC_ADDR_TRANS
#ifdef FEATURE_MMC_ADDR_TRANS
typedef enum {
	 EMMC_PART_UNKNOWN
    ,EMMC_PART_BOOT1
    ,EMMC_PART_BOOT2
    ,EMMC_PART_RPMB
    ,EMMC_PART_GP1
    ,EMMC_PART_GP2
    ,EMMC_PART_GP3
    ,EMMC_PART_GP4
    ,EMMC_PART_USER
    ,EMMC_PART_NUM
} emmc_phy_part_t;

static addr_trans_info_t g_emmc_addr_trans[EMMC_PART_NUM];
static addr_trans_tbl_t g_addr_trans_tbl;

static int mmc_switch_part(u32 part_id)
{
    int err = MMC_ERR_NONE;
    struct mmc_card *card;
    struct mmc_host *host;    
    u8 part = (u8)part_id;
    u8 cfg;
    u8 *ext_csd;

    card = mmc_get_card(0);
    host = mmc_get_host(0);
    
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
    }
    return err;    
}

static int mmc_virt_to_phys(u32 virt_blknr, u32 *phys_blknr, u32 *part_id)
{
    int ret;
    virt_addr_t_addtrans virt;
    phys_addr_t_addtrans phys;

    virt.addr = virt_blknr;

    ret = virt_to_phys_addr(&g_addr_trans_tbl, &virt, &phys);
    if (phys.id == 0)
        phys.id = EMMC_PART_USER;

    *phys_blknr = (ret == 0) ? phys.addr : virt_blknr; /* in 512B unit */
    *part_id    = (ret == 0) ? phys.id : EMMC_PART_USER;

    return ret;
}

static int mmc_phys_to_virt(u32 phys_blknr, u32 part_id, u32 *virt_blknr)
{
    int ret;
    virt_addr_t_addtrans virt;
    phys_addr_t_addtrans phys;

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
        mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
        mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);
        virt_blknr += g_emmc_addr_trans[i].len;
    }
    mmc_virt_to_phys(virt_blknr - 1, &phys_blknr, &part_id);    
    mmc_virt_to_phys(virt_blknr, &phys_blknr, &part_id);
    mmc_virt_to_phys(virt_blknr + 1, &phys_blknr, &part_id);

    phys_blknr = 0;
    for (i = 0; i < EMMC_PART_NUM; i++) {
        mmc_phys_to_virt(phys_blknr, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + 1, i, &virt_blknr);

        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len, i, &virt_blknr);
        mmc_phys_to_virt(phys_blknr + g_emmc_addr_trans[i].len + 1, i, &virt_blknr);
    }
}
#endif

int mmc_addr_trans_tbl_init(void)
{
    u32 wpg_sz;
    u8 *ext_csd;
    struct mmc_card *card;

    card = mmc_get_card(0);

    if (!card)
        return -1;

    memset(&g_addr_trans_tbl, 0, sizeof(addr_trans_tbl_t));
	

    ext_csd = &card->raw_ext_csd[0];

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
        size[EMMC_PART_UNKNOWN] = 0;    
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

        for (i = EMMC_PART_UNKNOWN; i < EMMC_PART_NUM; i++) {
            g_emmc_addr_trans[i].id  = i;
            g_emmc_addr_trans[i].len = size[i] / 512; /* in 512B unit */
			if(i<EMMC_PART_USER)
				g_user_virt_addr += size[i];
			g_emmc_size += size[i];
        }
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
#define mmc_virt_to_phys(vbn,pbn,pid)       do{}while(0)
#define mmc_phys_to_virt(pbn,pid,vbn)       do{}while(0)
#define mmc_virt_switch(vbn,pbn)            do{}while(0)
#define mmc_addr_trans_tbl_init()           do{}while(0)
#endif

int mmc_card_avail(struct mmc_host *host)
{
    return msdc_card_avail(host);
}

int mmc_card_protected(struct mmc_host *host)
{
    return msdc_card_protected(host);
}

struct mmc_host *mmc_get_host(int id)
{
    return &sd_host[id];
}

struct mmc_card *mmc_get_card(int id)
{
    return &sd_card[id];
}

static int mmc_cmd(struct mmc_host *host, struct mmc_command *cmd)
{
    int err;
    int retry = cmd->retries;

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

static u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
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

int mmc_go_idle(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd); 
}

#if 0
int mmc_go_irq_state(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & MMC_CCC_IO_MODE)) {
        printf("[SD%d] Card doesn't support I/O mode for IRQ state\n", host->id);
        return MMC_ERR_FAILED;
    }

    cmd.opcode  = MMC_CMD_GO_IRQ_STATE;
    cmd.rsptyp  = RESP_R5;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_inactive(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_INACTIVE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

static int mmc_go_pre_idle(struct mmc_host *host, struct mmc_card *card)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_GO_IDLE_STATE;
    cmd.rsptyp  = RESP_NONE;
    cmd.arg     = 0xF0F0F0F0;
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
    cmd.arg     = (card->rca << 16) | (sleep << 15);
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
    cmd.arg     = card->rca << 16;
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
    cmd.arg     = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
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
    cmd.arg     = ocr;
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

        mdelay(10);
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
    cmd.arg     = ocr;
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

        mdelay(10);
    }

    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

static int mmc_all_send_cid(struct mmc_host *host, u32 *cid)
{
    int err;
    struct mmc_command cmd;

    /* send cid */
    cmd.opcode  = MMC_CMD_ALL_SEND_CID;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        return err;

    memcpy(cid, cmd.resp, sizeof(u32) * 4);

    return MMC_ERR_NONE;
}

static void mmc_decode_cid(struct mmc_card *card)
{
    u32 *resp = card->raw_cid;

    memset(&card->cid, 0, sizeof(struct mmc_cid));

    if (mmc_card_sd(card)) {
        /*
        * SD doesn't currently have a version field so we will
        * have to assume we can parse this.
        */
        card->cid.month			= MSDC_DECODE_BITS(resp, 8, 4);
        card->cid.year			= MSDC_DECODE_BITS(resp, 12, 8);
        card->cid.serial		= MSDC_DECODE_BITS(resp, 24, 32);
        card->cid.fwrev			= MSDC_DECODE_BITS(resp, 56, 4);
        card->cid.hwrev			= MSDC_DECODE_BITS(resp, 60, 4);
        card->cid.prod_name[4]	= MSDC_DECODE_BITS(resp, 64, 8);
        card->cid.prod_name[3]	= MSDC_DECODE_BITS(resp, 72, 8);
        card->cid.prod_name[2]	= MSDC_DECODE_BITS(resp, 80, 8);
        card->cid.prod_name[1]	= MSDC_DECODE_BITS(resp, 88, 8);
        card->cid.prod_name[0]	= MSDC_DECODE_BITS(resp, 96, 8);
        card->cid.oemid			= MSDC_DECODE_BITS(resp, 104, 16);
        card->cid.manfid		= MSDC_DECODE_BITS(resp, 120, 8);

    	card->cid.year += 2000; /* SD cards year offset */
    } else {
        /*
         * The selection of the format here is based upon published
         * specs from sandisk and from what people have reported.
         */
        switch (card->csd.mmca_vsn) {
        case 0: /* MMC v1.0 - v1.2 */
        case 1: /* MMC v1.4 */
            card->cid.year          = MSDC_DECODE_BITS(resp, 8, 4) + 1997;
            card->cid.month         = MSDC_DECODE_BITS(resp, 12, 4);
            card->cid.serial        = MSDC_DECODE_BITS(resp, 16, 24);
            card->cid.fwrev         = MSDC_DECODE_BITS(resp, 40, 4);
            card->cid.hwrev         = MSDC_DECODE_BITS(resp, 44, 4);
            card->cid.prod_name[6]  = MSDC_DECODE_BITS(resp, 48, 8);
            card->cid.prod_name[5]  = MSDC_DECODE_BITS(resp, 56, 8);
            card->cid.prod_name[4]  = MSDC_DECODE_BITS(resp, 64, 8);
            card->cid.prod_name[3]  = MSDC_DECODE_BITS(resp, 72, 8);
            card->cid.prod_name[2]  = MSDC_DECODE_BITS(resp, 80, 8);
            card->cid.prod_name[1]  = MSDC_DECODE_BITS(resp, 88, 8);
            card->cid.prod_name[0]  = MSDC_DECODE_BITS(resp, 96, 8);
            card->cid.manfid        = MSDC_DECODE_BITS(resp, 104, 24);
            break;

        case 2: /* MMC v2.0 - v2.2 */
        case 3: /* MMC v3.1 - v3.3 */
        case 4: /* MMC v4 */
            card->cid.year          = MSDC_DECODE_BITS(resp, 8, 4) + 1997;
            card->cid.month         = MSDC_DECODE_BITS(resp, 12, 4);
            card->cid.serial        = MSDC_DECODE_BITS(resp, 16, 32);
            card->cid.prod_name[5]  = MSDC_DECODE_BITS(resp, 56, 8);
            card->cid.prod_name[4]  = MSDC_DECODE_BITS(resp, 64, 8);
            card->cid.prod_name[3]  = MSDC_DECODE_BITS(resp, 72, 8);
            card->cid.prod_name[2]  = MSDC_DECODE_BITS(resp, 80, 8);
            card->cid.prod_name[1]  = MSDC_DECODE_BITS(resp, 88, 8);
            card->cid.prod_name[0]  = MSDC_DECODE_BITS(resp, 96, 8);
            card->cid.oemid         = MSDC_DECODE_BITS(resp, 104, 16);
            //card->cid.cbx           = MSDC_DECODE_BITS(resp, 112, 2);
            card->cid.manfid        = MSDC_DECODE_BITS(resp, 120, 8);
            break;

        default:
            printf("[SD%d] Unknown MMCA version %d\n",
                mmc_card_id(card), card->csd.mmca_vsn);
            break;
        }
    }
}

static int mmc_decode_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->csd;
    unsigned int e, m, csd_struct;
    u32 *resp = card->raw_csd;

    if (mmc_card_sd(card)) {
        csd_struct = MSDC_DECODE_BITS(resp, 126, 2);
        csd->csd_struct = csd_struct;

        switch (csd_struct) {
        case 0:
            csd->tmp_wr_prot = MSDC_DECODE_BITS(resp, 12, 1);
            csd->perm_wr_prot = MSDC_DECODE_BITS(resp, 13, 1);
            csd->copy = MSDC_DECODE_BITS(resp, 14, 1);
            csd->write_partial = MSDC_DECODE_BITS(resp, 21, 1);
            csd->write_blkbits = MSDC_DECODE_BITS(resp, 22, 4);
            csd->r2w_factor = MSDC_DECODE_BITS(resp, 26, 3);
            csd->write_prot_grp = MSDC_DECODE_BITS(resp, 31, 1);
            csd->write_prot_grpsz = MSDC_DECODE_BITS(resp, 32, 7);
            csd->erase_sctsz = MSDC_DECODE_BITS(resp, 39, 7);
            csd->dsr = MSDC_DECODE_BITS(resp, 76, 1);
            csd->read_misalign = MSDC_DECODE_BITS(resp, 77, 1);
            csd->write_misalign = MSDC_DECODE_BITS(resp, 78, 1);
            csd->read_partial = MSDC_DECODE_BITS(resp, 79, 1);
            csd->read_blkbits = MSDC_DECODE_BITS(resp, 80, 4);
            csd->cmdclass	  = MSDC_DECODE_BITS(resp, 84, 12);
            csd->tacc_clks	 = MSDC_DECODE_BITS(resp, 104, 8) * 100;

            e = MSDC_DECODE_BITS(resp, 47, 3);
            m = MSDC_DECODE_BITS(resp, 62, 12);
            csd->capacity	  = (1 + m) << (e + 2);
            
            e = MSDC_DECODE_BITS(resp, 96, 3);
            m = MSDC_DECODE_BITS(resp, 99, 4);
            csd->max_dtr	  = tran_exp[e] * tran_mant[m];

            e = MSDC_DECODE_BITS(resp, 112, 3);
            m = MSDC_DECODE_BITS(resp, 115, 4);
            csd->tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
            break;
        case 1:
            /*
             * This is a block-addressed SDHC card. Most
             * interesting fields are unused and have fixed
             * values. To avoid getting tripped by buggy cards,
             * we assume those fixed values ourselves.
             */
            mmc_card_set_blockaddr(card);

            csd->tacc_ns	 = 0; /* Unused */
            csd->tacc_clks	 = 0; /* Unused */

            csd->tmp_wr_prot = MSDC_DECODE_BITS(resp, 12, 1);
            csd->perm_wr_prot = MSDC_DECODE_BITS(resp, 13, 1);
            csd->copy = MSDC_DECODE_BITS(resp, 14, 1);
            csd->write_prot_grp = MSDC_DECODE_BITS(resp, 31, 1);
            csd->write_prot_grpsz = MSDC_DECODE_BITS(resp, 32, 7);
            csd->erase_sctsz = MSDC_DECODE_BITS(resp, 39, 7);
            csd->dsr = MSDC_DECODE_BITS(resp, 76, 1);
            csd->cmdclass	  = MSDC_DECODE_BITS(resp, 84, 12);

            m = MSDC_DECODE_BITS(resp, 48, 22);
            csd->capacity     = (1 + m) << 10;
            
            m = MSDC_DECODE_BITS(resp, 99, 4);
            e = MSDC_DECODE_BITS(resp, 96, 3);
            csd->max_dtr	  = tran_exp[e] * tran_mant[m];

            csd->read_partial = 0;
            csd->read_blkbits = 9;
            csd->write_partial = 0;
            csd->write_blkbits = 9;
            csd->read_misalign = 0;
            csd->write_misalign = 0;
            csd->r2w_factor = 4; /* Unused */
            
            break;
        default:
            printf("[SD%d] Unknown CSD ver %d\n", mmc_card_id(card), csd_struct);
            return MMC_ERR_INVALID;
        }
    } else {
        /*
         * We only understand CSD structure v1.1 and v1.2.
         * v1.2 has extra information in bits 15, 11 and 10.
         */
        csd_struct = MSDC_DECODE_BITS(resp, 126, 2);

        if (csd_struct != CSD_STRUCT_VER_1_0 && csd_struct != CSD_STRUCT_VER_1_1 
            && csd_struct != CSD_STRUCT_VER_1_2 && csd_struct != CSD_STRUCT_EXT_CSD) {
            printf("[SD%d] Unknown CSD ver %d\n", mmc_card_id(card), csd_struct);
            return MMC_ERR_INVALID;
        }

        csd->csd_struct = csd_struct;
        csd->tmp_wr_prot = MSDC_DECODE_BITS(resp, 12, 1);
        csd->perm_wr_prot = MSDC_DECODE_BITS(resp, 13, 1);
        csd->copy = MSDC_DECODE_BITS(resp, 14, 1);
        csd->write_partial = MSDC_DECODE_BITS(resp, 21, 1);
        csd->write_blkbits = MSDC_DECODE_BITS(resp, 22, 4);
        csd->r2w_factor = MSDC_DECODE_BITS(resp, 26, 3);
        csd->write_prot_grp = MSDC_DECODE_BITS(resp, 31, 1);
        csd->write_prot_grpsz = MSDC_DECODE_BITS(resp, 32, 7);
        csd->erase_sctsz = (MSDC_DECODE_BITS(resp, 42, 5) + 1) * (MSDC_DECODE_BITS(resp, 37, 5) + 1);
        csd->dsr = MSDC_DECODE_BITS(resp, 76, 1);
        csd->read_misalign = MSDC_DECODE_BITS(resp, 77, 1);
        csd->write_misalign = MSDC_DECODE_BITS(resp, 78, 1);
        csd->read_partial = MSDC_DECODE_BITS(resp, 79, 1);
        csd->read_blkbits = MSDC_DECODE_BITS(resp, 80, 4);
        csd->cmdclass     = MSDC_DECODE_BITS(resp, 84, 12);
        csd->tacc_clks   = MSDC_DECODE_BITS(resp, 104, 8) * 100;
        csd->mmca_vsn    = MSDC_DECODE_BITS(resp, 122, 4);

        e = MSDC_DECODE_BITS(resp, 47, 3);
        m = MSDC_DECODE_BITS(resp, 62, 12);
        csd->capacity     = (1 + m) << (e + 2);
        
        e = MSDC_DECODE_BITS(resp, 96, 3);
        m = MSDC_DECODE_BITS(resp, 99, 4);
        csd->max_dtr      = tran_exp[e] * mmc_tran_mant[m];
        
        e = MSDC_DECODE_BITS(resp, 112, 3);
        m = MSDC_DECODE_BITS(resp, 115, 4);
        csd->tacc_ns     = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
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
    card->ext_csd.hc_erase_grp_sz = ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 512;
    card->ext_csd.hc_wp_grp_sz = ext_csd[EXT_CSD_HC_WP_GPR_SIZE];
    card->ext_csd.trim_tmo_ms = ext_csd[EXT_CSD_TRIM_MULT] * 300;
    card->ext_csd.boot_info   = ext_csd[EXT_CSD_BOOT_INFO];
    card->ext_csd.boot_part_sz = ext_csd[EXT_CSD_BOOT_SIZE_MULT] * 128 * 1024;
    card->ext_csd.access_sz = (ext_csd[EXT_CSD_ACC_SIZE] & 0xf) * 512;
    card->ext_csd.rpmb_sz = ext_csd[EXT_CSD_RPMB_SIZE_MULT] * 128 * 1024;
    card->ext_csd.erased_mem_cont = ext_csd[EXT_CSD_ERASED_MEM_CONT];
    card->ext_csd.part_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_PART_EN;
    card->ext_csd.enh_attr_en = ext_csd[EXT_CSD_PART_SUPPORT] & EXT_CSD_PART_SUPPORT_ENH_ATTR_EN;
    card->ext_csd.enh_start_addr = 
        (ext_csd[EXT_CSD_ENH_START_ADDR] |
         ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16 |
         ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24);
    card->ext_csd.enh_sz = 
        (ext_csd[EXT_CSD_ENH_SIZE_MULT] | 
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8 | 
         ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) * 512 * 1024 *
         ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];

    if (card->ext_csd.sectors)
       	mmc_card_set_blockaddr(card);

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

#if 0
int mmc_deselect_all_card(struct mmc_host *host)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_NONE; /* CHECKME. R1/R1B will timeout */
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
    cmd.arg     = card->rca << 16;
    cmd.rsptyp  = RESP_R1B;     /* CHECKME */
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    return err;
}

static int mmc_send_relative_addr(struct mmc_host *host, struct mmc_card *card, unsigned int *rca)
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

#if 0
int mmc_send_tuning_blk(struct mmc_host *host, struct mmc_card *card, u32 *buf)
{
    int err;
	struct mmc_command cmd;

    cmd.opcode  = SD_CMD_SEND_TUNING_BLOCK;
    cmd.arg     = 0;
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
        if (status & STA_SWITCH_ERROR) {
            printf("[SD%d] switch error. arg(0x%x)\n", host->id, cmd.arg);
            return MMC_ERR_FAILED;
        }
    } while (!(status & STA_READY_FOR_DATA) || (STA_CURRENT_STATE(status) == 7));

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
    //u32 retries;

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
	msdc_reset_tune_counter(host);
	do{
    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 64);
    msdc_set_timeout(host, 100000000, 0);
    err = mmc_cmd(host, &cmd);

    if (err != MMC_ERR_NONE)
        goto out;

    //retries = 50000;

    /* 512 bits = 64 bytes = 16 words */
    err = msdc_pio_read(host, sts, 64);
    if (err != MMC_ERR_NONE){
			if(msdc_abort_handler(host, 1))
				printf("[SD%d] data abort failed\n",host->id);
			result = msdc_tune_read(host);
		}
	}while(err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);

#if MMC_DEBUG
    {
        int i;
        u8 *byte = (u8*)&sts[0];

        /* Status:   B0      B1    ...
         * Bits  : 511-504 503-495 ...
         */

        for (i = 0; i < 4; i++) {
            MSG(RSP, "  [%.3d-%.3d] %.8x %.8x %.8x %.8x\n", 
                ((3 - i + 1) << 7) - 1, (3 - i) << 7, 
                sts[(i << 2) + 0], sts[(i << 2) + 1],
                sts[(i << 2) + 2], sts[(i << 2) + 3]);
        }
        for (i = 0; i < 8; i++) {
            MSG(RSP, "  [%.3d-%.3d] %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n",
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
    cmd.arg     = scc << 28;
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
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    err = mmc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE)
        err = msdc_switch_volt(host, MSDC_VDD_18_19);

    return err;
}
#endif

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
        printf("[SD%d] Switch to HS mode!\n", host->id);
        mmc_card_set_highspeed(card);
    }

out:
    return err;
}

#ifdef FEATURE_MMC_UHS1
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
        printf("[SD%d] Switch to UHS-1 %s mode!\n", host->id, smode[mode]);
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
#endif

static int mmc_read_csds(struct mmc_host *host, struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SEND_CSD;
    cmd.arg     = card->rca << 16;
    cmd.rsptyp  = RESP_R2;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT * 100;

    err = mmc_cmd(host, &cmd);
    if (err == MMC_ERR_NONE)
        memcpy(&card->raw_csd, &cmd.resp[0], sizeof(u32) * 4);
    return err;
}

static int mmc_read_scrs(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE; 
	int result; 
    //int retries;
    struct mmc_command cmd;
    struct sd_scr *scr = &card->scr;
    u32 resp[4];
    u32 tmp;

    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 8);
    msdc_set_timeout(host, 100000000, 0);

    cmd.opcode  = SD_ACMD_SEND_SCR;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;
	msdc_reset_tune_counter(host);
	do{
    mmc_app_cmd(host, &cmd, card->rca, CMD_RETRIES);
    if ((err != MMC_ERR_NONE) || !(cmd.resp[0] & STA_APP_CMD))
        return MMC_ERR_FAILED;

    //retries = 50000;

    /* 8 bytes = 2 words */
    err = msdc_pio_read(host, card->raw_scr, 8);
    if (err != MMC_ERR_NONE){
			if(msdc_abort_handler(host, 1))
				printf("[SD%d] data abort failed\n",host->id);
			result = msdc_tune_read(host);
		}
	}while(err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);
    MSG(INF, "[SD%d] SCR: %x %x (raw)\n", host->id, card->raw_scr[0], card->raw_scr[1]);

    tmp = ntohl(card->raw_scr[0]);
    card->raw_scr[0] = ntohl(card->raw_scr[1]);
    card->raw_scr[1] = tmp;   

    MSG(INF, "[SD%d] SCR: %x %x (ntohl)\n", host->id, card->raw_scr[0], card->raw_scr[1]);

    resp[2] = card->raw_scr[1];
    resp[3] = card->raw_scr[0];

    if (MSDC_DECODE_BITS(resp, 60, 4) != 0) {
        printf("[SD%d] Unknown SCR ver %d\n",
            mmc_card_id(card), MSDC_DECODE_BITS(resp, 60, 4));
        return MMC_ERR_INVALID;
    }

    scr->scr_struct = MSDC_DECODE_BITS(resp, 60, 4);
    scr->sda_vsn = MSDC_DECODE_BITS(resp, 56, 4);
    scr->data_bit_after_erase = MSDC_DECODE_BITS(resp, 55, 1);
    scr->security = MSDC_DECODE_BITS(resp, 52, 3);
    scr->bus_widths = MSDC_DECODE_BITS(resp, 48, 4);
    scr->sda_vsn3 = MSDC_DECODE_BITS(resp, 47, 1);
    scr->ex_security = MSDC_DECODE_BITS(resp, 43, 4);
    scr->cmd_support = MSDC_DECODE_BITS(resp, 32, 2);
    printf("[SD%d] SD_SPEC(%d) SD_SPEC3(%d) SD_BUS_WIDTH=%d\n", 
        mmc_card_id(card), scr->sda_vsn, scr->sda_vsn3, scr->bus_widths);        
    printf("[SD%d] SD_SECU(%d) EX_SECU(%d), CMD_SUPP(%d): CMD23(%d), CMD20(%d)\n", 
        mmc_card_id(card), scr->security, scr->ex_security, scr->cmd_support,
        (scr->cmd_support >> 1) & 0x1, scr->cmd_support & 0x1);
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
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_R1;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;
	msdc_reset_tune_counter(host);
do{
    msdc_set_blknum(host, 1);
    msdc_set_blklen(host, 512);
    msdc_set_timeout(host, 100000000, 0);
    err = mmc_cmd(host, &cmd);
    if (err != MMC_ERR_NONE)
        goto out;

    err = msdc_pio_read(host, ptr, 512);
    if (err != MMC_ERR_NONE){
       	if(msdc_abort_handler(host, 1))
			printf("[SD%d] data abort failed\n",host->id);
		result = msdc_tune_read(host);
	}
}while(err && result != MMC_ERR_READTUNEFAIL);
	msdc_reset_tune_counter(host);
    mmc_decode_ext_csd(card);

out:
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
    if (status[13] & 0x10) {
        printf("[SD%d] Support: DDR50\n", host->id);
        card->sw_caps.hs_max_dtr = 50000000;  /* ddr50 */
        card->sw_caps.ddr = 1;
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

#if 0
static int mmc_deselect_cards(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SELECT_CARD;
    cmd.arg     = 0;
    cmd.rsptyp  = RESP_NONE;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_lock_unlock(struct mmc_host *host)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_LOCK_UNLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = 0;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_set_write_prot(struct mmc_host *host, u32 addr)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_SET_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}

int mmc_clr_write_prot(struct mmc_host *host, u32 addr)
{
    struct mmc_command cmd;

    cmd.opcode  = MMC_CMD_CLR_WRITE_PROT;
    cmd.rsptyp  = RESP_R1B;
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(host, &cmd);
}
#endif

int mmc_erase_start(struct mmc_card *card, u64 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & MMC_CCC_ERASE)) {
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
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase_end(struct mmc_card *card, u64 addr)
{
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & MMC_CCC_ERASE)) {
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
    cmd.arg     = addr;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;

    return mmc_cmd(card->host, &cmd);
}

int mmc_erase(struct mmc_card *card, u32 arg)
{
    int err;
    u32 status;
    struct mmc_command cmd;

    if (!(card->csd.cmdclass & MMC_CCC_ERASE)) {
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
    cmd.arg     = arg;
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
            if (STA_STATUS(status) != 0) break;
        } while (STA_CURRENT_STATE(status) == 7);        
    }    
    return err;
}
static int __mmc_do_erase(struct mmc_host *host, struct mmc_card *card, u64 start_addr,u64 len)
{
	int err = MMC_ERR_NONE;
	u64 end_addr =((start_addr + len)/card->blklen - 1) * card->blklen;
	if(end_addr/card->blklen > card->nblks){
			printf("[MSDC%d]Erase address out of range! start<0x%llx>,len<0x%llx>,card_nblks<0x%x>\n",host->id,start_addr,len,card->nblks);
			return MMC_ERR_INVALID;
		}
	if ((err = mmc_erase_start(card, start_addr))) {
			printf("[MSDC%d]Set erase start addrees 0x%llx failed,Err<%d>\n",host->id,start_addr,err);
            goto out;
        }
        if((err = mmc_erase_end(card, end_addr))){
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
	if((!card) || (!host) ){
		printf("[mmc_do_erase] card<0x%x> host<0x%x>  mmc_do_erase\n",card,host);
		return MMC_ERR_INVALID;
	}
	if(!len ){
			printf("[MSDC%d] invalid erase size! len<0x%llx>\n",host->id,len);
			return MMC_ERR_INVALID;
	}
	if((start_addr % card->blklen) || (len % card->blklen)){
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
	
	for(; s_pid < e_pid; s_pid++){
		erase_part[s_pid].id = s_pid;
		erase_part[s_pid].start_blk = 0;
		erase_part[s_pid].blkcnt = g_emmc_addr_trans[s_pid].len;
		}
	if(s_pid_o != e_pid){
			erase_part[s_pid].id = s_pid;
			erase_part[s_pid].start_blk = 0;
			erase_part[s_pid].blkcnt = e_blknr + 1;
		}
	for(s_pid = s_pid_o;s_pid <= e_pid;s_pid++){
		if(erase_part[s_pid].blkcnt == 0 || s_pid == EMMC_PART_RPMB)
			continue;
			if((err = mmc_switch_part(s_pid))){
					printf("[MSDC%d] mmc swtich failed.part<%d> error <%d> \n",host->id,s_pid,err);
					return err;
				}
			
            start_addr  = (u64)(erase_part[s_pid].start_blk * card->blklen);
            len    = (u64)((erase_part[s_pid].blkcnt) * card->blklen);
			printf("[MSDC%d] mmc erase part<%d> <0x%llx - 0x%llx>start....\n",host->id,s_pid, start_addr, start_addr + len);
            err =  __mmc_do_erase(host, card, start_addr, len);
            if (err){
				printf("[MSDC%d] mmc erase failed. part<%d> error <%d> \n",host->id,s_pid,err);
				return err;
			}
			printf("[MSDC%d] mmc erase part<%d> <0x%llx - 0x%llx> done.\n",host->id,s_pid, start_addr, start_addr + len);
		}
#else
	if(err =  __mmc_do_erase(host,card,start_addr,len){
				printf("[MSDC%d] mmc erase failed.error <%d> \n",host->id,err);
				return err;
			}
#endif
	return err;
	
}
int mmc_tune_timing(struct mmc_host *host, struct mmc_card *card)
{
    int err = MMC_ERR_NONE;
#ifdef FEATURE_MMC_UHS1
    if (mmc_card_sd(card) && (mmc_card_uhs1(card))) {
        err = msdc_tune_uhs1(host, card);
    }
#endif
    return err;
}

void mmc_set_clock(struct mmc_host *host, int ddr, unsigned int hz)
{
    if (hz >= host->f_max) {
        hz = host->f_max;
    } else if (hz < host->f_min) {
        hz = host->f_min;
    }
    msdc_config_clock(host, ddr > 0 ? 1 : 0, hz);
}

int mmc_set_card_detect(struct mmc_host *host, struct mmc_card *card, int connect)
{
    int err;
    struct mmc_command cmd;

    cmd.opcode  = SD_ACMD_SET_CLR_CD;
    cmd.arg     = connect;
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
    cmd.arg     = blklen;
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;
    err = mmc_cmd(host, &cmd);

    if (err == MMC_ERR_NONE)
        msdc_set_blklen(host, blklen);

    return err;
}

int mmc_set_blk_count(struct mmc_host *host, u32 blkcnt)
{
    int err;
    struct mmc_command cmd;

    /* set block count */
    cmd.opcode  = MMC_CMD_SET_BLOCK_COUNT;
    cmd.rsptyp  = RESP_R1;
    cmd.arg     = blkcnt; /* bit31 is for reliable write request */
    cmd.retries = 3;
    cmd.timeout = CMD_TIMEOUT;
    err = mmc_cmd(host, &cmd);

    return err;
}

int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width)
{
    int err = MMC_ERR_NONE;
    u32 arg = EXT_CSD_BUS_WIDTH_1;
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
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Switch to bus width(%d) failed\n", host->id, arg);
            goto out;
        }
        if (arg == EXT_CSD_BUS_WIDTH_8_DDR || arg == EXT_CSD_BUS_WIDTH_4_DDR) {
            mmc_card_set_ddr(card);
        } else {
            card->state &= ~MMC_STATE_DDR;
        }
        mmc_set_clock(host, mmc_card_ddr(card), host->sclk);

        msdc_config_bus(host, width);
    } else {
        BUG_ON(1); /* card is not recognized */
    }
out:    
    return err;
}

int mmc_set_erase_grp_def(struct mmc_card *card, int enable)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card) || !mmc_card_highcaps(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_ERASE_GRP_DEF, EXT_CSD_ERASE_GRP_DEF_EN & enable);

out:
    return err;
}

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
    size /= 512;
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    /* 143-144: GP_SIZE_MULT_X_0-GP_SIZE_MULT_X_2 */
    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
            gp[id] + i, arg);
        if (err)
            goto out;
    }

out:
    return err;
}

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

    size /= 512;
    size /= (ext_csd[EXT_CSD_HC_WP_GPR_SIZE] * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);

    /* 140-142: ENH_SIZE_MULT0-ENH_SIZE_MULT2 */
    for (i = 0; i < 3; i++) {
        arg  = (u8)(size & 0xFF);
        size = size >> 8;
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
            EXT_CSD_ENH_SIZE_MULT + i, arg);
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

    if (mmc_card_highcaps(card))
        addr = addr / 512; /* in sector unit */

    /* 136-139: ENH_START_ADDR0-ENH_START_ADDR3 */
    for (i = 0; i < 4; i++) {
        arg  = (u8)(addr & 0xFF);
        addr = addr >> 8;
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
            EXT_CSD_ENH_START_ADDR + i, arg);
        if (err)
            goto out;
    }

out:
    return err;
}

int mmc_set_boot_bus(struct mmc_card *card, u8 rst_bwidth, u8 mode, u8 bwidth)
{
    int err = MMC_ERR_FAILED;
    u8 arg;

    if (mmc_card_sd(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    arg = mode | rst_bwidth | bwidth;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_BOOT_BUS_WIDTH, arg);

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

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_PART_CFG, cfg);

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

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_PART_ATTR, attr);

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

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_PART_SET_COMPL, EXT_CSD_PART_SET_COMPL_BIT);

out:
    return err;
}

int mmc_set_reset_func(struct mmc_card *card, u8 enable)
{
    int err = MMC_ERR_FAILED;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card))
        goto out;

    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    if (ext_csd[EXT_CSD_RST_N_FUNC] == 0) {
        err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
            EXT_CSD_RST_N_FUNC, enable);
    }
out:
    return err;
}

int mmc_boot_config(struct mmc_card *card, u8 acken, u8 enpart, u8 buswidth, u8 busmode)
{
    int err = MMC_ERR_FAILED;
    u8 val;
    u8 rst_bwidth = 0;
    u8 *ext_csd = &card->raw_ext_csd[0];

    if (mmc_card_sd(card) || card->csd.mmca_vsn < CSD_SPEC_VER_4 || 
        !card->ext_csd.boot_info || card->ext_csd.rev < 3)
        goto out;

    if (card->ext_csd.rev > 3 && !card->ext_csd.part_en)
        goto out;

    /* configure boot partition */
    val = acken | enpart | (ext_csd[EXT_CSD_PART_CFG] & 0x7);
    err = mmc_set_part_config(card, val);
    if (err != MMC_ERR_NONE)
        goto out;

    /* update ext_csd information */
    ext_csd[EXT_CSD_PART_CFG] = val;

    /* configure boot bus mode and width */
    rst_bwidth = buswidth != EXT_CSD_BOOT_BUS_WIDTH_1 ? 1 : 0;

    err = mmc_set_boot_bus(card, rst_bwidth, busmode, buswidth);       

out:

    return err;
}

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

int mmc_set_boot_prot(struct mmc_card *card, u8 prot)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
        goto out;

    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_BOOT_CONFIG_PROT, prot);

out:
    return err;
}

int mmc_set_boot_wp(struct mmc_card *card, u8 wp)
{
    int err = MMC_ERR_FAILED;

    if (mmc_card_sd(card))
        goto out;

    WARN_ON(card->csd.mmca_vsn < CSD_SPEC_VER_4);
    if (card->csd.mmca_vsn < CSD_SPEC_VER_4)
        goto out;

    err = mmc_switch(card->host, card, EXT_CSD_CMD_SET_NORMAL, 
        EXT_CSD_BOOT_WP, wp);

out:
    return err;
}

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

int mmc_dev_bread(struct mmc_card *card, unsigned long blknr, u32 blkcnt, u8 *dst)
{
    struct mmc_host *host = card->host;
    u32 blksz = host->blklen;
    int tune = 0;
    int retry = 3;
    int err;
    unsigned long src;

    src = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_read(host, (u8 *)dst, src, blkcnt);
        } else {
            #ifdef FEATURE_MMC_RD_TUNING
            err = msdc_tune_bread(host, (u8 *)dst, src, blkcnt);
            #endif
            if (err && (host->sclk > (host->f_max >> 4)))
                mmc_set_clock(host, mmc_card_ddr(card), host->sclk >> 1);            
        }
        mmc_prof_stop();
        if (err == MMC_ERR_NONE) {
            mmc_prof_update(mmc_prof_read, blkcnt, mmc_prof_handle(host->id));
            break;
        }
        
        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC){
            tune = 1;retry++;}
		else if(err == MMC_ERR_READTUNEFAIL || err == MMC_ERR_CMDTUNEFAIL){
			printf("[SD%d] Fail to tuning,%s",host->id,(err == MMC_ERR_CMDTUNEFAIL)?"cmd tune failed!\n":"read tune failed!\n");
			break;
			}
    } while (retry--);

    return err;
}

static int mmc_dev_bwrite(struct mmc_card *card, unsigned long blknr, u32 blkcnt, u8 *src)
{
    struct mmc_host *host = card->host;
    u32 blksz = host->blklen;
    u32 status;
    int tune = 0;
    int retry = 3;
    int err;
    unsigned long dst;

    dst = mmc_card_highcaps(card) ? blknr : blknr * blksz;

    do {
        mmc_prof_start();
        if (!tune) {
            err = host->blk_write(host, dst, (u8 *)src, blkcnt);
        } else {
            #ifdef FEATURE_MMC_WR_TUNING        
            err = msdc_tune_bwrite(host, dst, (u8 *)src, blkcnt);
            #endif
            if (err && (host->sclk > (host->f_max >> 4)))
                mmc_set_clock(host, mmc_card_ddr(card), host->sclk >> 1);            
        }
        if (err == MMC_ERR_NONE) {
            do {
                err = mmc_send_status(host, card, &status);
                if (err) {
                    printf("[SD%d] Fail to send status %d\n", host->id, err);
                    break;
                }
            } while (!(status & STA_READY_FOR_DATA) ||
                (STA_CURRENT_STATE(status) == 7));
            mmc_prof_stop();
            mmc_prof_update(mmc_prof_write, blkcnt, mmc_prof_handle(host->id));
            MSG(OPS, "[SD%d] Write %d bytes (DONE)\n", 
                host->id, blkcnt * blksz);
            break;
        }

        if (err == MMC_ERR_BADCRC || err == MMC_ERR_ACMD_RSPCRC || err == MMC_ERR_CMD_RSPCRC){
            tune = 1;retry++;}
		else if(err == MMC_ERR_WRITETUNEFAIL || err == MMC_ERR_CMDTUNEFAIL){
			printf("[SD%d] Fail to tuning,%s",host->id,(err == MMC_ERR_CMDTUNEFAIL)?"cmd tune failed!\n":"write tune failed!\n");
			break;
			}
    } while (retry--);

    return err;
}

int mmc_block_read(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz    = host->blklen;
    u32 maxblks  = host->max_phys_segs;
    u32 xfercnt  = blkcnt / maxblks;
    u32 leftblks = blkcnt % maxblks;
    u32 i;
    u8 *buf = (u8*)dst;
    int ret;

	//printf("[SD%d] virt addr:%ld\n",dev_num,blknr);
	mmc_virt_switch((unsigned int)blknr, (unsigned int*)&blknr);
	//printf("[SD%d] phys addr:%ld\n",dev_num,blknr);
    if (!blkcnt)
        return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        printf("[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n", 
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    for (i = xfercnt; i > 0; i--) {
        ret = mmc_dev_bread(card, (unsigned long)blknr, maxblks, buf);
        if (ret)
            return ret;
        blknr += maxblks;
        buf   += maxblks * blksz;
    }
    if (leftblks)
        ret = mmc_dev_bread(card, (unsigned long)blknr, leftblks, buf);
    
    return ret;
}

int mmc_block_write(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src)
{
    struct mmc_host *host = mmc_get_host(dev_num);
    struct mmc_card *card = mmc_get_card(dev_num);
    u32 blksz    = host->blklen;
    u32 maxblks  = host->max_phys_segs;
    u32 xfercnt  = blkcnt / maxblks;
    u32 leftblks = blkcnt % maxblks;
    u32 i;
    u8 *buf = (u8*)src;    
    int ret;

	//printf("virt addr:%ld\n",dev_num,blknr);
	mmc_virt_switch((unsigned int)blknr, (unsigned int*)&blknr);
	//printf("phys addr:%ld\n",dev_num,blknr);
    if (!blkcnt)
        return MMC_ERR_NONE;

    if (blknr * (blksz / MMC_BLOCK_SIZE) > card->nblks) {
        printf("[SD%d] Out of block range: blknr(%d) > sd_blknr(%d)\n", 
            host->id, blknr, card->nblks);
        return MMC_ERR_INVALID;
    }

    for (i = xfercnt; i > 0; i--) {
        ret = mmc_dev_bwrite(card, (unsigned long)blknr, maxblks, buf);
        if (ret)
            return ret;
        blknr += maxblks;
        buf   += maxblks * blksz;
    }
    if (leftblks)
        ret = mmc_dev_bwrite(card, (unsigned long)blknr, leftblks, buf);
    
    return ret;
}

#ifdef FEATURE_MMC_BOOT_MODE
void mmc_boot_reset(struct mmc_host *host, int reset)
{
    msdc_emmc_boot_reset(host, reset);
}

int mmc_boot_up(struct mmc_host *host, int mode, int ackdis, u32 *to, u32 size)
{
    int err;

    ERR_EXIT(msdc_emmc_boot_start(host, host->sclk, 0, mode, ackdis), err, MMC_ERR_NONE);
    ERR_EXIT(msdc_emmc_boot_read(host, size, to), err, MMC_ERR_NONE);

exit:
    msdc_emmc_boot_stop(host, mode);

    return err;
}
#endif

int mmc_init_mem_card(struct mmc_host *host, struct mmc_card *card, u32 ocr)
{
    int err, id = host->id;

    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F) {
        printf("card claims to support voltages "
            "below the defined range. These will be ignored.\n");
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
    if (mmc_card_sd(card)) {
        err = mmc_send_app_op_cond(host, ocr, &card->ocr);
    } else {
        /* The extra bit indicates that we support high capacity */
        err = mmc_send_op_cond(host, ocr, &card->ocr);
    }

    if (err != MMC_ERR_NONE) {
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
        printf("[SD%d] Fail in SEND_CSD cmd\n", id);
        goto out;        
    }

    /* decode csd */
    err = mmc_decode_csd(card);
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in decode csd\n", id);
        goto out;        
    }
    /* 2012-02-25: bug, should decode csd first */
    mmc_decode_cid(card);
    
    /* select this card */
    err = mmc_select_card(host, card);
    if (err != MMC_ERR_NONE) {
        printf("[SD%d] Fail in select card cmd\n", id);
        goto out;
    }

    if (mmc_card_sd(card)) {
        /* send scr */    
        err = mmc_read_scrs(host, card);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in SEND_SCR cmd\n", id);
            goto out;
        }

        if ((card->csd.cmdclass & MMC_CCC_SWITCH) && 
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
                if (host->caps & MMC_CAP_SD_HIGHSPEED) {
                    mmc_switch_hs(host, card);
                    break;
                }
            } while(0);
        }

        /* set bus width */
        mmc_set_bus_width(host, card, HOST_BUS_WIDTH_4); 

        /* compute bus speed. */
        card->maxhz = (unsigned int)-1;

        if (mmc_card_highspeed(card) || mmc_card_uhs1(card)) {
            if (card->maxhz > card->sw_caps.hs_max_dtr)
                card->maxhz = card->sw_caps.hs_max_dtr;
        } else if (card->maxhz > card->csd.max_dtr) {
            card->maxhz = card->csd.max_dtr;
        }
    } else {
        /* send ext csd */
        err = mmc_read_ext_csd(host, card);
        if (err != MMC_ERR_NONE) {
            printf("[SD%d] Fail in SEND_EXT_CSD cmd\n", id);
            goto out;        
        }

        /* activate high speed (if supported) */
        if ((card->ext_csd.hs_max_dtr != 0) && (host->caps & MMC_CAP_MMC_HIGHSPEED)) {
            err = mmc_switch(host, card, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

            if (err == MMC_ERR_NONE) {
                printf("[SD%d] Switch to High-Speed mode!\n", host->id);
                mmc_card_set_highspeed(card);
            }
        }

        /* set bus width */
        mmc_set_bus_width(host, card, HOST_BUS_WIDTH_8);

        /* compute bus speed. */
        card->maxhz = (unsigned int)-1;

        if (mmc_card_highspeed(card)) {
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

// Apply ett tool result
#if 0
static int msdc_ett_offline_to_ub(struct mmc_host *host, struct mmc_card *card)
{
    int ret = 1;
    int size = sizeof(g_mmcTable) / sizeof(mmcdev_info);
    int i, temp; 
    u32 base = host->base;
    u8  m_id = card->cid.manfid; 
	char * pro_name = card->cid.prod_name; 
    
    printf("msdc_ett_offline_to_ub: size<%d> m_id<0x%x>\n", size, m_id);

    for (i = 0; i < size; i++) {
        printf("msdc <%d> <%s> <%s>\n", i, g_mmcTable[i].pro_name, pro_name); 
                    	
        if ((g_mmcTable[i].m_id == m_id) && (!strncmp(g_mmcTable[i].pro_name, pro_name, 6))) {
            printf("msdc ett index<%d>: <%d> <%d> <0x%x> <0x%x> <0x%x>\n", i, 
                g_mmcTable[i].r_smpl, g_mmcTable[i].d_smpl, 
                g_mmcTable[i].cmd_rxdly, g_mmcTable[i].rd_rxdly, g_mmcTable[i].wr_rxdly);  

            // set to msdc0 
            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_RSPL, g_mmcTable[i].r_smpl); 
            MSDC_SET_FIELD(MSDC_IOCON, MSDC_IOCON_DSPL, g_mmcTable[i].d_smpl);
          
            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_CMDRRDLY, g_mmcTable[i].cmd_rxdly);
            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATRRDLY, g_mmcTable[i].rd_rxdly); 
            MSDC_SET_FIELD(MSDC_PAD_TUNE, MSDC_PAD_TUNE_DATWRDLY, g_mmcTable[i].wr_rxdly); 

            temp = g_mmcTable[i].rd_rxdly; temp &= 0x1F;             
            MSDC_WRITE32(MSDC_DAT_RDDLY0, (temp<<0 | temp<<8 | temp<<16 | temp<<24)); 
            MSDC_WRITE32(MSDC_DAT_RDDLY1, (temp<<0 | temp<<8 | temp<<16 | temp<<24));
                                 
            ret = 0;
            break;
        }
    }
    
    if (ret) printf("msdc failed to find\n");
    return ret;        	
}
#endif
int mmc_init_card(struct mmc_host *host, struct mmc_card *card)
{
    int err, id = host->id;
    u32 ocr;

    printf("[%s]: start\n", __func__);
    memset(card, 0, sizeof(struct mmc_card));
    mmc_prof_init(id, host, card);
    mmc_prof_start();

    if (!msdc_card_avail(host)) {
        err = MMC_ERR_INVALID;
        goto out;
    }

#if 0
    if (msdc_card_protected(host))
        mmc_card_set_readonly(card);
#endif

    mmc_card_set_present(card);
    mmc_card_set_host(card, host);
    mmc_card_set_unknown(card);

    mmc_go_idle(host);

    /* send interface condition */
    mmc_send_if_cond(host, host->ocr_avail);

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

    err = mmc_init_mem_card(host, card, ocr);

    if (err)
        goto out;

    /* change clock */
    mmc_set_clock(host, mmc_card_ddr(card), card->maxhz);
    /* 2012-02-25 lookup the ETT table */
    //msdc_ett_offline_to_ub(host, card);
    
    #ifdef FEATURE_MMC_UHS1
    /* tune timing */    
    mmc_tune_timing(host, card);
    #endif

out:
    mmc_prof_stop();
    mmc_prof_update(mmc_prof_card_init, (u32)id, (void*)err);
    if (err) {
        //msdc_power(host, MMC_POWER_OFF);
        printf("[%s]: failed, err=%d\n", __func__, err);
        return err;
    }
    host->card = card;    
    printf("[%s]: finish successfully\n", __func__);
    return 0;
}

int mmc_init_host(struct mmc_host *host, int id)
{
    memset(host, 0, sizeof(struct mmc_host));

    return msdc_init(host, id);
}

void mmc_hard_reset(void)
{
    msdc_hard_reset();
}

#if 0
int mmc_init(int id)
{   
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;

    BUG_ON(id >= NR_MMC);

    host = &sd_host[id];
    card = &sd_card[id];
    err = mmc_init_host(host, id);
    if (err == MMC_ERR_NONE)
        err = mmc_init_card(host, card);

#ifdef MMC_TEST
    mmc_test(0, NULL);
#endif

    return err;
}
#else
unsigned long mmc_wrap_bread(int dev_num, unsigned long blknr, lbaint_t blkcnt, void *dst)
{
    return mmc_block_read(dev_num, blknr, (u32)blkcnt, (unsigned long*)dst) == MMC_ERR_NONE ? blkcnt : (unsigned long) -1;
}

unsigned long mmc_wrap_bwrite(int dev_num, unsigned long blknr, lbaint_t blkcnt, const void *src)
{
    return mmc_block_write(dev_num, blknr, (u32)blkcnt, (unsigned long*)src) == MMC_ERR_NONE ? blkcnt : (unsigned long) -1;
}
#if 0
int mmc_get_dl_info(void)
{
	
	DL_STATUS_EMMC download_info;
	u64 dl_addr = g_emmc_size - DL_INFO_SIZE;
	part_dev_t *dev = mt6575_part_get_device();
	int i;

	printf("get dl info from 0x%x\n",dl_addr);
	mmc_wrap_bread(dev->id,dl_addr/512,DL_INFO_SIZE/512,(u8 *)dl_buf);
	memcpy(&download_info,dl_buf,sizeof(download_info));
	if(memcmp(download_info.magic_num,"DOWNLOAD INFORMATION!!",22)){
		printf("DL INFO NOT FOUND\n");
		return DL_NOT_FOUND;
	}

	if(!memcmp(download_info.download_status,"DL_DONE",7)||!memcmp(download_info.download_status,"DL_CK_DONE",10))
	{
		printf("dl done. status = %s\n",download_info.download_status);	
		printf("dram checksum : %s\n",download_info.ram_checksum);
		for(i=0;i<PART_MAX_COUNT;i++)
		{
			if(download_info.part_info[i].image_index!=0){
				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
			}
		}
		return DL_PASS;
	}else
	{
		printf("dl error. status = %s\n",download_info.download_status);
		printf("dram checksum : %s\n",download_info.ram_checksum);
		for(i=0;i<PART_MAX_COUNT;i++)
		{
			if(download_info.part_info[i].image_index!=0){
				printf("image_index:%d, checksum: %s\n",download_info.part_info[i].image_index,download_info.part_info[i].checksum_status);
			}
		}
		return DL_FAIL;
	}
}
#endif
int mmc_legacy_init(int verbose)
{
    int id = verbose - 1;
    int err = MMC_ERR_NONE;
    struct mmc_host *host;
    struct mmc_card *card;
    block_dev_desc_t *bdev;

    printf("[%s]: start\n", __func__);
    BUG_ON(id >= NR_MMC);

    msdc_hard_reset();

    host = &sd_host[id];
    card = &sd_card[id];
    bdev = &sd_dev[id];
    printf("[%s]: start mmc_init_host\n", __func__);
    err = mmc_init_host(host, id);

    if (err == MMC_ERR_NONE){
        printf("[%s]: start mmc_init_card\n", __func__);
        err = mmc_init_card(host, card);
    }

    if (err == MMC_ERR_NONE && !boot_dev_found) {
		#ifdef MMC_TEST
    		mmc_test(0, NULL);
		#endif
		printf("addr_trans_init\n");
		mmc_addr_trans_tbl_init();
		printf("addr_trans_init done\n");
        /* fill in device description */
        bdev->dev         = id;
        bdev->blksz       = MMC_BLOCK_SIZE;
        bdev->lba         = card->nblks * card->blklen / MMC_BLOCK_SIZE;
        bdev->block_read  = mmc_wrap_bread;
        bdev->block_write = mmc_wrap_bwrite;

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

#ifdef MTK_EMMC_SUPPORT_OTP 
    printf("=========use hc erase size\n");
    mmc_set_erase_grp_def(card, 1);
#endif



    return err;
}
#endif

//#endif	/* CONFIG_MMC */

