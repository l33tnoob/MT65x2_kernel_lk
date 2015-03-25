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
#include "api.h"        //For invocation cache_clean_invalidate()
#include "cache_api.h"  //For invocation cache_clean_invalidate()
#endif

#if defined(MMC_MSDC_DRV_LK)
#include <arch/ops.h>
//For arch_clean_invalidate_cache_range() is defined in bootable/bootloader/include/arch/ops.h
#endif

static gpd_t msdc_gpd_pool[MSDC_MAX_NUM][MAX_GPD_POOL_SZ];
static bd_t  msdc_bd_pool[MSDC_MAX_NUM][MAX_BD_POOL_SZ];

#if defined(MMC_MSDC_DRV_CTP)
#if 0 //Light: turn if off before I verify it
extern u32 sg_autocmd_crc_tuning_blkno; //Temporarily declared as extern; To do: solve it
extern u32 sg_autocmd_crc_tuning_count; //Temporarily declared as extern; To do: solve it
#endif
#endif

#if defined(MSDC_USE_DMA_MODE)
void msdc_init_gpd_bd(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    gpd_t *gpd;
    bd_t *bd;
    int id=host->id;

    gpd  = &msdc_gpd_pool[id][0];
    bd	 = &msdc_bd_pool[id][0];
    memset(gpd, 0, sizeof(gpd_t) * MAX_GPD_POOL_SZ);
    memset(bd, 0, sizeof(bd_t) * MAX_BD_POOL_SZ);
    
    priv->bd_pool     = bd;
    priv->gpd_pool    = gpd;   
}

void msdc_flush_membuf(void *buf, u32 len)
{
    #if defined(MMC_MSDC_DRV_LK)
    arch_clean_invalidate_cache_range((addr_t)buf,len);

    #elif defined(MMC_MSDC_DRV_CTP)
    cache_clean_invalidate();

    #endif
}

u8 msdc_cal_checksum(u8 *buf, u32 len)
{
    u32 i, sum = 0;
    for (i = 0; i < len; i++) {
	    sum += buf[i];
    }
    return 0xFF - (u8)sum;
}

/* allocate gpd link-list from gpd_pool */
gpd_t *msdc_alloc_gpd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    gpd_t *gpd, *ptr, *prev;

    if (priv->alloc_gpd + num + 1 > MAX_GPD_POOL_SZ || num == 0)
	    return NULL;

    gpd = priv->gpd_pool + priv->alloc_gpd;
    priv->alloc_gpd += (num + 1); /* include null gpd */

    memset(gpd, 0, sizeof(gpd_t) * (num + 1));

    ptr = gpd + num - 1;
    ptr->next = (void*)(gpd + num); /* pointer to null gpd */

    /* create link-list */
    if (ptr != gpd) {
    	do {
    	    prev = ptr - 1;
    	    prev->next = ptr;
    	    ptr = prev;
    	} while (ptr != gpd);
    }

    return gpd;
}

/* allocate bd link-list from bd_pool */
bd_t *msdc_alloc_bd(struct mmc_host *host, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    bd_t *bd, *ptr, *prev;

    if (priv->alloc_bd + num > MAX_BD_POOL_SZ || num == 0)
	    return NULL;

    bd = priv->bd_pool + priv->alloc_bd;
    priv->alloc_bd += num;

    memset(bd, 0, sizeof(bd_t) * num);

    ptr = bd + num - 1;
    ptr->eol  = 1;
    ptr->next = 0;

    /* create link-list */
    if (ptr != bd) {
    	do {
    	    prev = ptr - 1;
    	    prev->next = ptr;
    	    prev->eol  = 0;
    	    ptr = prev;
    	} while (ptr != bd);
    }

    return bd;
}

/* queue bd link-list to one gpd */
void msdc_queue_bd(struct mmc_host *host, gpd_t *gpd, bd_t *bd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 1;
    gpd->ptr = (void*)bd;

    if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0)
	    return;

    /* calculate and fill bd checksum */
    while (bd) {
    	bd->chksum = msdc_cal_checksum((u8*)bd, 16);
    	bd = bd->next;
    }
}

/* queue data buf to one gpd */
void msdc_queue_buf(struct mmc_host *host, gpd_t *gpd, u8 *buf)
{
    BUG_ON(gpd->ptr);

    gpd->hwo = 1;
    gpd->bdp = 0;
    gpd->ptr = (void*)buf;
}

/* add gpd link-list to active list */
void msdc_add_gpd(struct mmc_host *host, gpd_t *gpd, int num)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    if (num > 0) {
    	if (!priv->active_head) {
    	    priv->active_head = gpd;
    	} else {
    	    priv->active_tail->next = gpd;
    	}
    	priv->active_tail = gpd + num - 1;
    
    	if ((priv->cfg.flags & DMA_FLAG_EN_CHKSUM) == 0)
    	    return;
    
    	/* calculate and fill gpd checksum */
    	while (gpd) {
    	    gpd->chksum = msdc_cal_checksum((u8 *)gpd, 16);
    	    gpd = gpd->next;
    	}
    }
}

void msdc_reset_gpd(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;

    priv->alloc_bd  = 0;
    priv->alloc_gpd = 0;
    priv->active_head = NULL;
    priv->active_tail = NULL;
}

void msdc_set_dma(struct mmc_host *host, u8 burstsz, u32 flags)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;

    cfg->burstsz = burstsz;
    cfg->flags	 = flags;
}

int msdc_sg_init(struct scatterlist *sg, void *buf, u32 buflen)
{
    int i = MAX_SG_POOL_SZ;
    char *ptr = (char *)buf;

    BUG_ON(buflen > MAX_SG_POOL_SZ * MAX_SG_BUF_SZ);
    msdc_flush_membuf(buf, buflen);
    while (i > 0) {
        if (buflen > MAX_SG_BUF_SZ) {
            sg->addr = (u32)ptr;
            sg->len  = MAX_SG_BUF_SZ;
            buflen  -= MAX_SG_BUF_SZ;
            ptr     += MAX_SG_BUF_SZ;
            sg++; i--;
        } else {
            sg->addr = (u32)ptr;
            sg->len  = buflen;
            i--;
            break;
        }
    }

    return MAX_SG_POOL_SZ - i;
}

void msdc_dma_init(struct mmc_host *host, struct dma_config *cfg, void *buf, u32 buflen)
{
    u32 base = host->base;

    cfg->xfersz = buflen;

    if (cfg->mode == MSDC_MODE_DMA_BASIC) {
        cfg->sglen = 1;
        cfg->sg[0].addr = (u32)buf;
        cfg->sg[0].len = buflen;
        msdc_flush_membuf(buf, buflen);
    } else {
        cfg->sglen = msdc_sg_init(cfg->sg, buf, buflen);
    }

    MSDC_CLR_FIFO();
    MSDC_DMA_ON();
}

int msdc_dma_cmd(struct mmc_host *host, struct dma_config *cfg, struct mmc_command *cmd)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    u32 opcode = cmd->opcode;
    u32 rsptyp = cmd->rsptyp;
    u32 rawcmd;

    rawcmd = (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)) |
            rsptyp << 7 | host->blklen << 16;

    if (opcode == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
        rawcmd |= ((2 << 11) | (1 << 13));
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_WRITE_BLOCK) {
        rawcmd |= ((1 << 11) | (1 << 13));
    } else if (opcode == MMC_CMD_READ_MULTIPLE_BLOCK) {
        rawcmd |= (2 << 11);
        if (priv->autocmd & MSDC_AUTOCMD12)
            rawcmd |= (1 << 28);
        else if (priv->autocmd & MSDC_AUTOCMD23)
            rawcmd |= (2 << 28);
    } else if (opcode == MMC_CMD_READ_SINGLE_BLOCK) {
        rawcmd |= (1 << 11);
    #if defined(FEATURE_MMC_SDIO)
    } else if (opcode == SD_IO_RW_EXTENDED) {
        if (cmd->arg & 0x80000000)  /* R/W flag */
            rawcmd |= (1 << 13);
        if ((cmd->arg & 0x08000000) && ((cmd->arg & 0x1FF) > 1))
            rawcmd |= (2 << 11); /* multiple block mode */
        else
            rawcmd |= (1 << 11);
    } else if (opcode == SD_IO_RW_DIRECT) {
        if ((cmd->arg & 0x80000000) && ((cmd->arg >> 9) & 0x1FFFF))/* I/O abt */
            rawcmd |= (1 << 14);
    #endif
    } else {
        return -1;
    }

    MSG(DMA, "[SD%d] DMA CMD(%d), AUTOCMD12(%d), AUTOCMD23(%d)\n",
        host->id, (opcode & ~(SD_CMD_BIT | SD_CMD_APP_BIT)),
        (priv->autocmd & MSDC_AUTOCMD12) ? 1 : 0,
        (priv->autocmd & MSDC_AUTOCMD23) ? 1 : 0);

    cfg->cmd = rawcmd;
    cfg->arg = cmd->arg;

    return 0;
}

int msdc_dma_config(struct mmc_host *host, struct dma_config *cfg)
{
    u32 base = host->base;
    u32 sglen = cfg->sglen;
    //u32 i;
    u32 j;
#ifdef FEATURE_MSDC_ENH_DMA_MODE
    u32 num, bdlen, arg, xfersz;
#endif
    u8  blkpad, dwpad, chksum;
    struct scatterlist *sg = cfg->sg;
    gpd_t *gpd;
    bd_t *bd;

    switch (cfg->mode) {
        case MSDC_MODE_DMA_BASIC:
            BUG_ON(cfg->xfersz > MAX_DMA_CNT);
            BUG_ON(cfg->sglen != 1);
            MSDC_WRITE32(MSDC_DMA_SA, sg->addr);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_LASTBUF, 1);
            MSDC_WRITE32(MSDC_DMA_LEN, sg->len);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 0);
            break;

        case MSDC_MODE_DMA_DESC:
            blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
            dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
            chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

#if 0 /* YD: current design doesn't support multiple GPD in descriptor dma mode */
            /* calculate the required number of gpd */
            num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;
            gpd = msdc_alloc_gpd(host, num);
            for (i = 0; i < num; i++) {
                gpd[i].intr = 0;
                if (sglen > MAX_BD_PER_GPD) {
                  bdlen  = MAX_BD_PER_GPD;
                  sglen -= MAX_BD_PER_GPD;
                } else {
                  bdlen = sglen;
                  sglen = 0;
                }

                bd = msdc_alloc_bd(host, bdlen);
                for (j = 0; j < bdlen; j++) {
                  MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                  sg++;
                }
                msdc_queue_bd(host, &gpd[i], bd);
                msdc_flush_membuf(bd, bdlen * sizeof(bd_t));
            }
            msdc_add_gpd(host, gpd, num);
            #if MSDC_DEBUG
            msdc_dump_dma_desc(host);
            #endif
            msdc_flush_membuf(gpd, num * sizeof(gpd_t));
            MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
            MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
#else
            /* calculate the required number of gpd */
            BUG_ON(sglen > MAX_BD_POOL_SZ);

            gpd = msdc_alloc_gpd(host, 1);
            gpd->intr = 0;
            //printf("sglen(%d),sg->len(%d)\n",sglen,sg->len);
            bd = msdc_alloc_bd(host, sglen);
            for (j = 0; j < sglen; j++) {
                MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                sg++;
            }
            msdc_queue_bd(host, &gpd[0], bd);
            msdc_flush_membuf(bd, sglen * sizeof(bd_t));

            msdc_add_gpd(host, gpd, 1);
            #if MSDC_DEBUG
            msdc_dump_dma_desc(host);
            #endif
            msdc_flush_membuf(gpd, (1 + 1) * sizeof(gpd_t)); /* include null gpd */
            MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
            MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);

#endif
            break;

#ifdef FEATURE_MSDC_ENH_DMA_MODE
        case MSDC_MODE_DMA_ENHANCED:
            arg = cfg->arg;
            blkpad = (cfg->flags & DMA_FLAG_PAD_BLOCK) ? 1 : 0;
            dwpad  = (cfg->flags & DMA_FLAG_PAD_DWORD) ? 1 : 0;
            chksum = (cfg->flags & DMA_FLAG_EN_CHKSUM) ? 1 : 0;

            /* calculate the required number of gpd */
            num = (sglen + MAX_BD_PER_GPD - 1) / MAX_BD_PER_GPD;
            gpd = msdc_alloc_gpd(host, num);
            for (i = 0; i < num; i++) {
                xfersz = 0;
                if (sglen > MAX_BD_PER_GPD) {
                  bdlen  = MAX_BD_PER_GPD;
                  sglen -= MAX_BD_PER_GPD;
                } else {
                  bdlen = sglen;
                  sglen = 0;
                }

                bd = msdc_alloc_bd(host, bdlen);
                for (j = 0; j < bdlen; j++) {
                  xfersz += sg->len;
                  MSDC_INIT_BD(&bd[j], blkpad, dwpad, sg->addr, sg->len);
                  sg++;
                }
                /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
                 * is done. For multiple GPDs, multiple XFER_COMP interrupts will be
                 * triggerred. In such situation, it's not easy to know which
                 * interrupt indicates the transaction is done. So, we use the
                 * latest one GPD's INT as the transaction done interrupt.
                 */
                //gpd[i].intr = cfg->intr;
                gpd[i].intr = (i == num - 1) ? 0 : 1;
                gpd[i].cmd  = cfg->cmd;
                gpd[i].blknum = xfersz / cfg->blklen;
                gpd[i].arg  = arg;
                gpd[i].extlen = 0xC;

                arg += xfersz;

                msdc_queue_bd(host, &gpd[i], bd);
                msdc_flush_membuf(bd, bdlen * sizeof(bd_t));
            }

            msdc_add_gpd(host, gpd, num);

            #if MSDC_DEBUG
            msdc_dump_dma_desc(host);
            #endif

            msdc_flush_membuf(gpd, (num + 1) * sizeof(gpd_t)); /* include null gpd */
            MSDC_WRITE32(MSDC_DMA_SA, (u32)&gpd[0]);
            MSDC_SET_FIELD(MSDC_DMA_CFG, MSDC_DMA_CFG_DECSEN, chksum);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_BRUSTSZ, cfg->burstsz);
            MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_MODE, 1);
            break;
#endif

        default:
            break;
    }
    MSG(DMA, "[SD%d] DMA_SA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_SA));
    MSG(DMA, "[SD%d] DMA_CA   = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CA));
    MSG(DMA, "[SD%d] DMA_CTRL = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CTRL));
    MSG(DMA, "[SD%d] DMA_CFG  = 0x%x\n", host->id, MSDC_READ32(MSDC_DMA_CFG));

    return 0;
}

void msdc_dma_resume(struct mmc_host *host)
{
    u32 base = host->base;

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_RESUME, 1);

    MSG(DMA, "[SD%d] DMA resume\n", host->id);
}

void msdc_dma_start(struct mmc_host *host)
{
    u32 base = host->base;
#if defined(MMC_MSDC_DRV_LK)
	u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR;
	msdc_intr_unmask(host,wints);
#endif

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_START, 1);

    MSG(DMA, "[SD%d] DMA start\n", host->id);
}

void msdc_dma_stop(struct mmc_host *host)
{
    u32 base = host->base;
#if defined(MMC_MSDC_DRV_LK)
    u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR;
#endif

    MSDC_SET_FIELD(MSDC_DMA_CTRL, MSDC_DMA_CTRL_STOP, 1);
    while ((MSDC_READ32(MSDC_DMA_CFG) & MSDC_DMA_CFG_STS) != 0);
    MSDC_DMA_OFF();

    MSG(DMA, "[SD%d] DMA Stopped\n", host->id);
#if defined(MMC_MSDC_DRV_LK)
    msdc_intr_mask(host,wints);
#endif
    msdc_reset_gpd(host);
}

int msdc_dma_wait_done(struct mmc_host *host, u32 timeout)
{
    u32 base = host->base;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    u32 status;
    u32 error = MMC_ERR_NONE;
    u32 wints = MSDC_INT_XFER_COMPL | MSDC_INT_DATTMO | MSDC_INT_DATCRCERR |
                MSDC_INT_DXFER_DONE | MSDC_INT_DMAQ_EMPTY |
                MSDC_INT_ACMDRDY | MSDC_INT_ACMDTMO | MSDC_INT_ACMDCRCERR |
                MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;

    do {
        MSG(DMA, "[SD%d] DMA Curr Addr: 0x%x, Active: %d\n", host->id,
            MSDC_READ32(MSDC_DMA_CA), MSDC_READ32(MSDC_DMA_CFG) & 0x1);

        #if defined(MMC_MSDC_DRV_LK)
        status = msdc_lk_intr_wait(host, wints);
        #else
        status = msdc_intr_wait(host, wints);
        #endif

        if (status == 0 || status & MSDC_INT_DATTMO) {
            printf("[SD%d] DMA DAT timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;
        } else if (status & MSDC_INT_DATCRCERR) {
            printf("[SD%d] DMA DAT CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        } else if (status & MSDC_INT_CMDTMO) {
            printf("[SD%d] DMA CMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;
        } else if (status & MSDC_INT_RSPCRCERR) {
            printf("[SD%d] DMA CMD CRC error(%xh)\n", host->id, status);
            error = MMC_ERR_BADCRC;
            goto end;
        } else if (status & MSDC_INT_ACMDTMO) {
            printf("[SD%d] DMA ACMD timeout(%xh)\n", host->id, status);
            error = MMC_ERR_TIMEOUT;
            goto end;
        } else if (status & MSDC_INT_ACMDCRCERR) {
            printf("[SD%d] DMA ACMD CRC error(%xh)\n", host->id, status);
            //Light: 6583 SLT changes to return MSDC_INT_ACMDCRCERR.
            //       However, Return MSDC_INT_ACMDCRCERR trigger tuning of cmd, but this tuning fail.
            //       Therefore reserve returning of MMC_ERR_BADCRC is correct
            //error = MMC_ERR_ACMD_RSPCRC; 
            error = MMC_ERR_BADCRC; 
            goto end;
        }

    #ifdef FEATURE_MSDC_ENH_DMA_MODE
        if ((cfg->mode == MSDC_MODE_DMA_ENHANCED) && (status & MSDC_INT_CMDRDY)) {
            cfg->rsp = MSDC_READ32(SDC_RESP0);
            MSG(DMA, "[SD%d] DMA ENH CMD Rdy, Resp(%xh)\n", host->id, cfg->rsp);
            #if MMC_DEBUG
            mmc_dump_card_status(cfg->rsp);
            #endif
        }
    #endif

        if (status & MSDC_INT_ACMDRDY) {
            cfg->autorsp = MSDC_READ32(SDC_ACMD_RESP);
            MSG(DMA, "[SD%d] DMA AUTO CMD Rdy, Resp(%xh)\n", host->id, cfg->autorsp);
            #if MMC_DEBUG
            mmc_dump_card_status(cfg->autorsp);
            #endif
        }

    #ifdef FEATURE_MSDC_ENH_DMA_MODE
        if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
            /* YD: 1 XFER_COMP interrupt will be triggerred by each GPD when it
             * is done. For multiple GPDs, multiple XFER_COMP interrupts will be
             * triggerred. In such situation, it's not easy to know which
             * interrupt indicates the transaction is done. So, we use the
             * latest one GPD's INT as the transaction done interrupt.
             */
            if (status & MSDC_INT_DXFER_DONE)
              break;
        } else
    #endif
        {
            if (cfg->inboot && cfg->mode == MSDC_MODE_DMA_BASIC){
                //printf("Polling DMA tranfer done in eMMC boot mode\n");
                if (status & MSDC_INT_DXFER_DONE)
                    break;
            }

            if (status & MSDC_INT_XFER_COMPL)
                break;
        }
    } while (1);

    /* check dma status */
    do {
        status = MSDC_READ32(MSDC_DMA_CFG);
        if (status & MSDC_DMA_CFG_GPDCSERR) {
            MSG(DMA, "[SD%d] GPD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if (status & MSDC_DMA_CFG_BDCSERR) {
            MSG(DMA, "[SD%d] BD checksum error\n", host->id);
            error = MMC_ERR_BADCRC;
            break;
        } else if ((status & MSDC_DMA_CFG_STS) == 0) {
            break;
        }
    } while (1);
end:
    return error;
}

#if defined(FEATURE_MMC_SDIO)
#if 0
/*
int msdc_dma_iorw(struct mmc_card *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
*/
#else
int msdc_dma_iorw(struct mmc_card *card, int write,
    u8 *buf, unsigned blocks, unsigned blksz)
#endif

{
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE;
    struct mmc_host *host = card->host;
    struct mmc_command cmd;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    u32 nblks = (u32)blocks;

    /* Comment out by moving to mmc_io_rw_extended()
    #if 0
    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = SD_IO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= fn << 28;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= addr << 9;
    if (blocks == 1 && blksz <= 512) {
      cmd.arg |= (blksz == 512) ? 0 : blksz;    // byte mode
    } else {
      cmd.arg |= 0x08000000 | blocks;           // block mode
    }
    cmd.rsptyp  = RESP_R5;
    cmd.retries = CMD_RETRIES;
    cmd.timeout = CMD_TIMEOUT;
    #endif
    */

    if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
        /* NOTICE: SDIO can't not issue multiple commands for one transcation data
         * so can't use multiple GPDs for that. But multiple transactions can
         * use multiple GPDs.
         * If BUG_ON is triggerred, please increase MAX_BD_PER_GPD number.
         */
        BUG_ON((blocks * blksz / MAX_SG_BUF_SZ) > MAX_BD_PER_GPD);
        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);
        msdc_dma_cmd(host, cfg, &cmd);
        msdc_dma_init(host, cfg, (void*)buf, nblks * blksz);
        msdc_dma_config(host, cfg);
        msdc_dma_start(host);
        err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
        msdc_dma_stop(host);
        /* SDIO workaround for CMD53 multiple block transfer */
        #if 1
        if (!err && nblks > 1) {
            err=msdc_cmd_io_abort(host);
        }
        #endif
    } else {
        u32 left_sz, xfer_sz;

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, 100000000, 0);

        left_sz = nblks * blksz;

        if (cfg->mode == MSDC_MODE_DMA_BASIC) {
            /* NOTICE: SDIO can't not issue multiple commands for one transcation
             * data. If BUG_ON is triggerred, please decrease transaction data size.
             */
            BUG_ON(left_sz > MAX_DMA_CNT);
            xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
            nblks   = xfer_sz / blksz;
        } else {
            xfer_sz = left_sz;
        }

        while (left_sz) {

            msdc_set_blknum(host, nblks);
            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);
            msdc_dma_config(host, cfg);

            err = msdc_cmd(host, &cmd);

            if (err != MMC_ERR_NONE) {
                msdc_reset_gpd(host);
                goto done;
            }

            msdc_dma_start(host);
            err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);

            /* SDIO workaround for CMD53 multiple block transfer */
            #if 1
            if (!err && nblks > 1) {
                err=msdc_cmd_io_abort(host);
            }
            #endif
            if (err != MMC_ERR_NONE)
                goto done;
            buf     += xfer_sz;
            left_sz -= xfer_sz;

            if (left_sz) {
                xfer_sz  = (xfer_sz > left_sz) ? left_sz : xfer_sz;
                nblks  = (left_sz > xfer_sz) ? nblks : left_sz / blksz;
            }
        }
    }

done:
    if (derr != MMC_ERR_NONE) {
        printf("[SD%d] <CMD%d> IO DMA data error (%d)\n", host->id, cmd.opcode & ~SD_CMD_BIT, derr);
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
    }

    return err;
}
#endif

int msdc_dma_transfer(struct mmc_host *host, struct mmc_command *cmd, struct mmc_data *data)
{
    int err = MMC_ERR_NONE, derr = MMC_ERR_NONE, cmderr= MMC_ERR_NONE;
    int multi;
    u32 blksz = host->blklen;
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    struct dma_config *cfg = &priv->cfg;
    uchar *buf = data->buf;
    ulong nblks = data->blks;

    BUG_ON(nblks * blksz > MAX_DMA_TRAN_SIZE);
    
    /* used for some debug func */
    host->cmd = cmd;

    multi = nblks > 1 ? 1 : 0;

    if (cfg->mode == MSDC_MODE_DMA_ENHANCED) {
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 1);

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, data->timeout * 1000000, 0);
        msdc_dma_cmd(host, cfg, cmd);
        msdc_dma_init(host, cfg, (void*)buf, nblks * blksz);
        msdc_dma_config(host, cfg);
        msdc_dma_start(host);
        err = derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
        msdc_dma_stop(host);
        msdc_flush_membuf(buf,nblks * blksz);
        if (multi && (priv->autocmd == 0))
            msdc_set_autocmd(host, MSDC_AUTOCMD12, 0);

    } else {
        u32 left_sz, xfer_sz;

        msdc_set_blklen(host, blksz);
        msdc_set_timeout(host, data->timeout * 1000000, 0);

        left_sz = nblks * blksz;

        if (cfg->mode == MSDC_MODE_DMA_BASIC) {
            xfer_sz = left_sz > MAX_DMA_CNT ? MAX_DMA_CNT : left_sz;
            nblks   = xfer_sz / blksz;
            //printf("xfer_sz(%d),left_sz(%d),nblks(%d)\n",xfer_sz,left_sz,nblks);
        } else {
            xfer_sz = left_sz;
        }

        while (left_sz) {
            derr = MMC_ERR_NONE;
            msdc_set_blknum(host, nblks);

            msdc_dma_init(host, cfg, (void*)buf, xfer_sz);
            //printf("nblks(%d),xfer_sz(%d),left_sz(%d)\n",nblks,xfer_sz,left_sz);

            err = msdc_send_cmd(host, cmd);
            msdc_dma_config(host, cfg);

            //The only err can be returned by msdc_send_cmd() is MMC_ERR_TIMEOUT
            if (err != MMC_ERR_NONE) {
                //msdc_dma_stop(host); //TBD:
                                       //original preloader/LK in JB2 comment out this line,
                                       //but CTP reserve this line
                msdc_reset_gpd(host);
                goto done;
            }

            err = msdc_wait_rsp(host, cmd);
            
            if (err == MMC_ERR_BADCRC) {
                //CMD_RSPCRC and ACMD_RSPCRC are both returned as MMC_ERR_BADCRC in msdc_wait_rsp()
                u32 base = host->base;
                u32 tmp = MSDC_READ32(SDC_CMD);

                /* check if data is used by the command or not */
                if (tmp & SDC_CMD_DTYP) {
                    msdc_abort_handler(host, 1);
                }
                #ifdef FEATURE_MMC_CM_TUNING
                if ( host->app_cmd!=2 ) //Light 20130203, to prevent recursive call path: msdc_tune_cmdrsp->msdc_app_cmd->msdc_cmd->msdc_tune_cmdrsp

                {
                    err = msdc_tune_cmdrsp(host, cmd);
                    if (err != MMC_ERR_NONE){   //20130121 Light Added
                        msdc_reset_gpd(host);
                        //when fail, err will be assigned as MMC_ERR_CMDTUNEFAIL in msdc_tune_cmddsp
                        goto done;              //20130121 Light Added
                    }
                }
                #endif
            }

            if (err != MMC_ERR_NONE)
                goto done;

            //printf("resp come\n");
            msdc_dma_start(host);
            derr = msdc_dma_wait_done(host, 0xFFFFFFFF);
            msdc_dma_stop(host);
            msdc_flush_membuf(buf, nblks * blksz);

            if (derr != MMC_ERR_NONE)
                goto done;

            if (multi && (priv->autocmd == 0)) {
                cmderr = msdc_cmd_stop(host, cmd);
             }

             if (cmderr != MMC_ERR_NONE)
                 goto done;
             buf     += xfer_sz;
             left_sz -= xfer_sz;

             /* left_sz > 0 only when in basic dma mode */
            if (left_sz) {
                cmd->arg += nblks; /* update to next start address */
                xfer_sz  = (xfer_sz > left_sz) ? left_sz : xfer_sz;
                nblks  = (left_sz > xfer_sz) ? nblks : left_sz / blksz;
            }
        }
    }
done:
    
    if (err != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout. 
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        if ( derr==MMC_ERR_NONE )           //For Enahnced mode
            return err;                     // high level will retry
    }

    if (derr != MMC_ERR_NONE){
        /* crc error find in data transfer. need reset host & send cmd12 */
        /* if autocmd crc occur, will enter here too */
        printf("[SD%d] <CMD%d> DMA data error (%d)\n", host->id, cmd->opcode, derr);
        msdc_abort_handler(host, 1);

        #if defined(MMC_MSDC_DRV_CTP)
        #if 0 //Light: turn if off before I verify it
        if ((sg_autocmd_crc_tuning_blkno == cmd->arg) && (derr == MMC_ERR_ACMD_RSPCRC)){
            /* update cmd tuning parameter */
            sg_autocmd_crc_tuning_count++;
            msdc_tune_update_cmdrsp(host, sg_autocmd_crc_tuning_count);
        } else if(derr == MMC_ERR_ACMD_RSPCRC) {
            /* just retry r/w cmd with the first ACMD CRC */
            sg_autocmd_crc_tuning_blkno = cmd->arg;
            sg_autocmd_crc_tuning_count = 0;
        }
        #endif
        #endif

        return derr;
    }

    if (cmderr != MMC_ERR_NONE){
        /* msdc_cmd will do cmd tuning flow, so if enter here, cmd maybe timeout
         * need reset host */
        //Light: msdc_abort_handler() combined from preloader/LK and CTP can not meet this purpose,
        //       so call msdc_abort() directly
        //msdc_abort_handler(host, 0);
        msdc_abort(host);
        return MMC_ERR_FAILED;  // high level will retry 
    }

    return err;
}

int msdc_dma_bread(struct mmc_host *host, uchar *dst, ulong src, ulong nblks)
{
    int multi;
    struct mmc_command cmd;
    struct mmc_data data;

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Read DMA data %u blks from 0x%x\n", host->id, (unsigned int)nblks, (unsigned int)src);

    multi = nblks > 1 ? 1 : 0;

    /* send read command */
    cmd.opcode  = multi ? MMC_CMD_READ_MULTIPLE_BLOCK : MMC_CMD_READ_SINGLE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = src;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    data.blks  = nblks;
    data.buf   = (u8*)dst;
    data.timeout = 100; /* 100ms */

    return msdc_dma_transfer(host, &cmd, &data);
}

int msdc_dma_bwrite(struct mmc_host *host, ulong dst, uchar *src, ulong nblks)
{
    int multi;
    struct mmc_command cmd;
    struct mmc_data data;

    BUG_ON(nblks > host->max_phys_segs);

    MSG(OPS, "[SD%d] Write data %d blks to 0x%x\n", host->id, (unsigned int)nblks, (unsigned int)dst);

    multi = nblks > 1 ? 1 : 0;

    /* send write command */
    cmd.opcode  = multi ? MMC_CMD_WRITE_MULTIPLE_BLOCK : MMC_CMD_WRITE_BLOCK;
    cmd.rsptyp  = RESP_R1;
    cmd.arg = dst;
    cmd.retries = 0;
    cmd.timeout = CMD_TIMEOUT;

    data.blks  = nblks;
    data.buf   = (u8*)src;
    data.timeout = 250; /* 250ms */

    return msdc_dma_transfer(host, &cmd, &data);
}

#if MSDC_DEBUG
void msdc_dump_dma_desc(struct mmc_host *host)
{
    msdc_priv_t *priv = (msdc_priv_t*)host->priv;
    int i;
    u32 *ptr;

    if (MSG_EVT_MASK & MSG_EVT_DMA) {
	for (i = 0; i < priv->alloc_gpd; i++) {
	    ptr = (u32*)&priv->gpd_pool[i];
	    printf("[SD%d] GD[%d](0x%xh): %xh %xh %xh %xh %xh %xh %xh\n",
    		host->id, i, (u32)ptr, *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4),
    		*(ptr+5), *(ptr+6));
	}

	for (i = 0; i < priv->alloc_bd; i++) {
	    ptr = (u32*)&priv->bd_pool[i];
	    printf("[SD%d] BD[%d](0x%xh): %xh %xh %xh %xh\n",
		    host->id, i, (u32)ptr, *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
	}
    }
}
#endif

#endif