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

#ifndef _MSDC_DMA_H_
#define _MSDC_DMA_H_

#include "mmc_core.h"

/*--------------------------------------------------------------------------*/
/* Descriptor Structure                             */
/*--------------------------------------------------------------------------*/
typedef struct {
    uint32  hwo:1; /* could be changed by hw */
    uint32  bdp:1;
    uint32  rsv0:6;
    uint32  chksum:8;
    uint32  intr:1;
    uint32  rsv1:15;
    void   *next;
    void   *ptr;
    uint32  buflen:16;
    uint32  extlen:8;
    uint32  rsv2:8;
    uint32  arg;
    uint32  blknum;
    uint32  cmd;
} gpd_t;

typedef struct {
    uint32  eol:1;
    uint32  rsv0:7;
    uint32  chksum:8;
    uint32  rsv1:1;
    uint32  blkpad:1;
    uint32  dwpad:1;
    uint32  rsv2:13;
    void   *next;
    void   *ptr;
    uint32  buflen:16;
    uint32  rsv3:16;
} bd_t;

struct scatterlist {
    u32 addr;
    u32 len;
};

struct scatterlist_ex {
    u32 cmd;
    u32 arg;
    u32 sglen;
    struct scatterlist *sg;
};

struct dma_config {
    u32 flags;           /* flags */
    u32 xfersz;          /* xfer size in bytes */
    u32 sglen;           /* size of scatter list */
    u32 blklen;          /* block size */
    struct scatterlist *sg;  /* I/O scatter list */
    struct scatterlist_ex *esg;  /* extended I/O scatter list */
    u8  mode;            /* dma mode        */
    u8  burstsz;         /* burst size      */
    u8  intr;            /* dma done interrupt */
    u8  padding;         /* padding */
    u32 cmd;             /* enhanced mode command */
    u32 arg;             /* enhanced mode arg */
    u32 rsp;             /* enhanced mode command response */
    u32 autorsp;         /* auto command response */
    u8  inboot;          /* this flag is for Basic DMA in eMMC boot mode*/
};


void msdc_init_gpd_bd(struct mmc_host *host);
void msdc_flush_membuf(void *buf, u32 len);
u8 msdc_cal_checksum(u8 *buf, u32 len);
gpd_t *msdc_alloc_gpd(struct mmc_host *host, int num);
bd_t *msdc_alloc_bd(struct mmc_host *host, int num);
void msdc_queue_bd(struct mmc_host *host, gpd_t *gpd, bd_t *bd);
void msdc_queue_buf(struct mmc_host *host, gpd_t *gpd, u8 *buf);
void msdc_add_gpd(struct mmc_host *host, gpd_t *gpd, int num);
void msdc_reset_gpd(struct mmc_host *host);
void msdc_set_dma(struct mmc_host *host, u8 burstsz, u32 flags);
int msdc_sg_init(struct scatterlist *sg, void *buf, u32 buflen);
void msdc_dma_init(struct mmc_host *host, struct dma_config *cfg, void *buf, u32 buflen);
int msdc_dma_cmd(struct mmc_host *host, struct dma_config *cfg, struct mmc_command *cmd);
int msdc_dma_config(struct mmc_host *host, struct dma_config *cfg);
void msdc_dma_resume(struct mmc_host *host);
void msdc_dma_start(struct mmc_host *host);
void msdc_dma_stop(struct mmc_host *host);
int msdc_dma_wait_done(struct mmc_host *host, u32 timeout);
#if 0
/*
int msdc_dma_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
*/
#else
int msdc_dma_iorw(struct mmc_card *card, int write,
    u8 *buf, unsigned blocks, unsigned blksz);
#endif
//int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd, uchar *buf, ulong nblks);
int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd, struct mmc_data *data);
int msdc_dma_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
int msdc_dma_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);

#endif //ifndef _MSDC_DMA_H_