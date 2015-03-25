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
#include "download.h"
#include "part.h"

#if CFG_LEGACY_USB_DOWNLOAD

#include "mtk_nand_core.h"
#include "mtk_nand.h"
#include "boot_device.h"
#include "mt_usbtty.h"
#include "mtk_wdt.h"
#include "pmt.h"

#define MOD                 "<DM>"

/**************************************************************************
 *  MACRO DEFINITION
 **************************************************************************/
#if DM_DBG_LOG
#define DM_ASSERT(expr)      {  if ((expr)==FALSE){  \
    print("%s : [ASSERT] at %s #%d %s\n       %s\n       above expression is not TRUE\n", MOD, __FILE__, __LINE__, __FUNCTION__, #expr); \
    while(1){};  }                    \
                             }
#define DM_STATE_LOG(state)  print("%s : state %s\n", MOD, (state))
#define DM_ENTRY_LOG()       print("%s : enter %s\n", MOD, __FUNCTION__)
#define DM_LOG               print
#else
#define DM_ASSERT(expr)
#define DM_STATE_LOG(state)
#define DM_ENTRY_LOG()
#define DM_LOG
#endif

/**************************************************************************
 *  LOCAL VARIABLE DECLARATION
 **************************************************************************/
DM_CONTEXT dm_ctx = { 0 };

/**************************************************************************
 *  GLOBAL VARIABLE DECLARATION
 **************************************************************************/
u32 g_dl_safe_start_addr;
char dm_rx_buf[DM_BUF_MAX_SIZE];
BOOL g_cust_key_init;
BOOL g_end_user_flash_tool;
BOOL g_sec_img_patch_enable;
BOOL g_sec_img_patch_valid;

DM_PARTITION_INFO_PACKET g_img_dl_pt_info = {0};

/**************************************************************************
 *  EXTERNAL DECLARATION
 **************************************************************************/
extern int nand_curr_device;
extern u32 nand_maf_id;
extern u32 nand_dev_id;
extern struct nand_chip g_nand_chip;
extern int usbdl_init (void);

#if DM_TIME_ANALYSIS
u32 g_DM_TIME_Tmp;
u32 g_DM_TIME_ReadData;
u32 g_DM_TIME_FlashData;
u32 g_DM_TIME_Erase;
u32 g_DM_TIME_USBGet;
u32 g_DM_TIME_SkipBadBlock;
u32 g_DM_TIME_Checksum;
u32 g_DM_TIME_Total_Begin;
u32 g_DM_TIME_Total;

#define DM_TIME_BEGIN \
    g_DM_TIME_Tmp = get_timer(0);

#define DM_TIME_END_NAND_READ \
    g_DM_TIME_ReadData += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_WRITE \
    g_DM_TIME_FlashData += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_ERASE \
    g_DM_TIME_Erase += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_NAND_BAD_BLOCK \
    g_DM_TIME_SkipBadBlock += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_USB_READ \
    g_DM_TIME_USBGet += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_END_CHECKSM \
    g_DM_TIME_Checksum += get_timer(g_DM_TIME_Tmp);\

#define DM_TIME_TOTAL_BEGIN \
    g_DM_TIME_Total_Begin = get_timer(0);

#define DM_TIME_TOTAL_END \
    g_DM_TIME_Total = get_timer(g_DM_TIME_Total_Begin);

void dump_time_analysis (void)
{
    print ("\n%s : --------------------------------------\n", MOD);
    print ("%s : [TIME] NAND erase takes %d ms \n", MOD, g_DM_TIME_Erase);
    print ("%s : [TIME] NAND flash takes %d ms \n", MOD,g_DM_TIME_FlashData);
    print ("%s : [TIME] NAND skip bad block takes %d ms \n", MOD,g_DM_TIME_SkipBadBlock);
    print ("%s : [TIME] USB read takes %d ms \n", MOD, g_DM_TIME_USBGet);
    print ("%s : [TIME] DM check sum takes %d ms \n", MOD,g_DM_TIME_Checksum);
    print ("%s : --------------------------------------\n", MOD);
    print ("%s : [TIME] DM total takes %d ms \n", MOD, g_DM_TIME_Total);
    print ("%s : --------------------------------------\n\n", MOD);
}
#else
#define DM_TIME_BEGIN
#define DM_TIME_END_NAND_READ
#define DM_TIME_END_NAND_WRITE
#define DM_TIME_END_NAND_ERASE
#define DM_TIME_END_NAND_BAD_BLOCK
#define DM_TIME_END_USB_READ
#define DM_TIME_END_CHECKSM
#define DM_TIME_TOTAL_BEGIN
#define DM_TIME_TOTAL_END
#endif

void handle_pt_cmd (void);

/**************************************************************************
 * Fill Internal Imgp Data
 **************************************************************************/
void fill_internal_imgp (DM_IMAGE_INFO_PACKET *imgp, DM_EXT_IMAGE_INFO_PACKET *ext_imgp)
{
    imgp->pattern = ext_imgp->pattern;
    imgp->img_info.img_format = ext_imgp->img_info.img_format;
    imgp->img_info.img_type = ext_imgp->img_info.img_type;    
    imgp->img_info.img_size = ext_imgp->img_info.img_size;       
    imgp->img_info.addr_off = ext_imgp->img_info.addr_off;  

#ifdef FEATURE_DOWNLOAD_BOUNDARY_CHECK
    imgp->img_info.addr_boundary = ext_imgp->img_info.addr_boundary;       
    //print ("get image boundary (0x%x) from tool\n",imgp->img_info.addr_boundary);    
#else    
    imgp->img_info.addr_boundary = imgp->img_info.addr_off + get_part_range (imgp->img_info.img_type);  
    print ("get image boundary (0x%x) from partition table\n",imgp->img_info.addr_boundary);    
#endif    

    if (imgp->img_info.img_type == DM_IMG_TYPE_USRDATA)
    {   imgp->img_info.addr_boundary = g_nand_chip.chipsize;
    }

    imgp->img_info.pkt_size = ext_imgp->img_info.pkt_size;      

}

/**************************************************************************
 * Handle Nand Flash Erase
 **************************************************************************/
u32 force_erase (DM_IMG_INFO * img_info, u32 pktsz)
{

    DM_TIME_BEGIN;
    // erase whole partition to keep data consistent
    {
        u32 erase_limit = 0;
        if (img_info->img_type == DM_IMG_TYPE_USRDATA)
        {
            erase_limit = g_nand_chip.chipsize;
            //print ("%s : erase limit address (0x%x)\n", MOD,erase_limit);
            //print ("%s : user data partition ! erase 0x%x ~ 0x%x\n",  MOD, img_info->addr_off, g_nand_chip.chipsize);
            if (FALSE ==  g_boot_dev.dev_erase_data (img_info->addr_off, g_nand_chip.chipsize, g_nand_chip.chipsize - img_info->addr_off))
            {
                DM_ASSERT (0);
                dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
                dm_ctx.dm_err = DM_ERR_NAND_ER_FAIL;
            }
        }
        else
        {
            erase_limit = dm_ctx.img_info.addr_off + dm_ctx.part_range;
            //print ("%s : erase limit address (0x%x)\n", MOD, erase_limit);
            if (FALSE == g_boot_dev.dev_erase_data (img_info->addr_off, erase_limit, dm_ctx.part_range))
            {
                DM_ASSERT (0);
                dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
                dm_ctx.dm_err = DM_ERR_NAND_ER_FAIL;
            }
        }
    }
    DM_TIME_END_NAND_ERASE;
}


/**************************************************************************
 *  Handle Nand Flash Erase
 **************************************************************************/
u32 handle_erase (DM_IMG_INFO * img_info, u32 pktsz)
{
    if ((FALSE == g_end_user_flash_tool) || (FALSE == g_sec_img_patch_enable))
    {
        return;
    }

    force_erase (img_info, pktsz);
}

/**************************************************************************
 * Handle Data
 **************************************************************************/
void handle_data (u32 pktsz, u8 * buf)
{
    bool res = TRUE;
    unsigned int i = 0;
    u32 starting_block = 0;
    u32 spare_start, spare_offset, spare_len;
    bool first_page = TRUE;
    bool need_erase_nand = TRUE;
    bool invalid_addr = FALSE;
    blkdev_t *blkdev;

    blkdev = blkdev_get(CFG_BOOT_DEV);

    DM_ENTRY_LOG ();

    if (dm_ctx.dm_status == DM_STATUS_ERR_ONGOING)
    {
        while (dm_ctx.curr_cnt <= dm_ctx.pkt_cnt)
        {
            mt_usbtty_getcn (pktsz, buf);
            dm_ctx.curr_cnt++;
        };
        dm_ctx.dm_status = DM_STATUS_ERR_FINISHED;
        return;
    }

    /* make sure the starting block is good */
    starting_block = g_boot_dev.dev_find_safe_block (dm_ctx.page_off);
    if (dm_ctx.page_off != starting_block)
    {
        dm_ctx.page_off = starting_block;
    }

    do
    {
        /* fill USB buffer */
        DM_TIME_BEGIN;
        mt_usbtty_getcn (pktsz, buf);
        DM_TIME_END_USB_READ;

        /* calculate check sum of received buffer */
#if (DM_CAL_CKSM_FROM_USB_BUFFER || DM_DBG_LOG)
#if DM_CAL_CKSM_FROM_USB_BUFFER
        DM_TIME_BEGIN;
        cal_chksum_per_pkt (buf, pktsz);
        DM_TIME_END_CHECKSM;
#endif
#endif

        /* check image boundary 
           always check image boundary at begining to ensure that
           "won't write any data to next partition" */
        if (dm_ctx.page_off >= dm_ctx.img_info.addr_boundary)
        {   
            //print ("current page_off (0%x) >= addr_boundary (0x%x)\n", dm_ctx.page_off, dm_ctx.img_info.addr_boundary);            
            invalid_addr = TRUE;            

        }        

        /* if addr is invalid, skip the nand writing process */
        if (invalid_addr == TRUE)
        {   goto _next;
        }

        if (TRUE == need_erase_nand)
        {
            /* erase nand flash */
            handle_erase (&dm_ctx.img_info, pktsz);
            need_erase_nand = FALSE;
        }

        /* when the address is block alignment, check if this block is good */
        DM_TIME_BEGIN;
        if (dm_ctx.page_off % blkdev->erasesz == 0)
        {   
            dm_ctx.page_off = g_boot_dev.dev_find_safe_block (dm_ctx.page_off);
        }
        DM_TIME_END_NAND_BAD_BLOCK;

        /* write nand flash */
        DM_TIME_BEGIN;
        g_boot_dev.dev_write_data (buf, dm_ctx.page_off) ;
        DM_TIME_END_NAND_WRITE;

_next:

        /* update the latest safe nand addr */
        g_dl_safe_start_addr = dm_ctx.page_off;

        /* increase must after flash data */
        dm_ctx.curr_cnt++;
        dm_ctx.curr_off += pktsz;
        dm_ctx.page_off += dm_ctx.page_size;

        //delay (1000);

    }
    while ((dm_ctx.curr_cnt <= dm_ctx.pkt_cnt) && (dm_ctx.dm_status == DM_STATUS_SECT_ONGING));
    dm_ctx.dm_status = DM_STATUS_SECT_FINISHED;

    return;
}


/**************************************************************************
 *  Calculate Checksum
 **************************************************************************/
void handle_cksm (void)
{
    u32 u4cksum = 0;
    DM_CHKSUM_PACKET cksm = { DM_CHKSUM_PKT_PATN, 0 };

    DM_ENTRY_LOG ();

#if DM_CAL_CKSM_FROM_USB_BUFFER
    print ("<CHKSUM> : USB buffer .. ");
    cksm.chk_sum = dm_ctx.chk_sum;
    print ("0x%x (=%d)\n", cksm.chk_sum, cksm.chk_sum);
#endif

#if DM_CAL_CKSM_FROM_NAND_FLASH
    print ("<CHKSUM> : NAND flash .. ");
    u4cksum = g_boot_dev.dev_chksum_per_file (dm_ctx.img_info.addr_off, dm_ctx.img_info.img_size);
    print ("0x%x (=%d)\n", u4cksum, u4cksum);
#if DM_CAL_CKSM_FROM_USB_BUFFER
    if (u4cksum != cksm.chk_sum)
    {
        print ("ERROR : USB buffer != NAND flash content\n");
        ASSERT (0);
    }
#endif
    cksm.chk_sum = u4cksum;
#endif

    mt_usbtty_putcn (DM_SZ_CHK_SUM_PKT, (u8 *) & cksm, TRUE);
    dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT;

    return;
}

/**************************************************************************
 *  Check Image Information
 **************************************************************************/
u32 check_imgp (DM_IMAGE_INFO_PACKET * imgp, u32 * pktsz)
{
    print ("\n%s : ------------------ IMG ---------------\n", MOD);
    print ("%s : IMG fmt    = %s   \t type = %s\n", MOD, get_img_fmt (imgp->img_info.img_format),  get_img_type (imgp->img_info.img_type));
    print ("%s : IMG sz     = 0x%x \t addr = 0x%x\n", MOD,(imgp->img_info.img_size), (imgp->img_info.addr_off));
    print ("%s : addr       = 0x%x \t boundary = 0x%x\n", MOD, (imgp->img_info.addr_off), (imgp->img_info.addr_boundary));
    print ("%s : pkt sz     = 0x%x \n", MOD, (imgp->img_info.pkt_size));
    //print ("%s : --------------------------------------\n", MOD);
    print ("%s : IMG range  = 0x%x ~ 0x%x\n", MOD, (imgp->img_info.addr_off), (imgp->img_info.addr_off) + (imgp->img_info.img_size));
    //print ("%s : --------------------------------------\n", MOD);

    // image type is partition table inform
    if (DM_IMG_TYPE_PT_TABLE_INFORM == imgp->img_info.img_type)
    {
        print ("partition table inform\n");
        mt_usbtty_puts (DM_STR_START_REQ);
        handle_pt_cmd ();
        return;
    }
    
    // check image boundary  
    if (g_dl_safe_start_addr > imgp->img_info.addr_off)
    {
        print ("can't flash this image !\n");
        print ("current safe starting address for flashing (0x%x) > current image address (0x%x)\n", g_dl_safe_start_addr, imgp->img_info.addr_off);
        ASSERT (0);
    }

    DM_LOG ("%s : cur safe start addr for flashing (0x%x)\n", MOD, g_dl_safe_start_addr);
    g_dl_safe_start_addr = imgp->img_info.addr_off;

    if (imgp->pattern != DM_IMAGE_INFO_PKT_PATN)
    {
        DM_ASSERT (imgp->pattern == DM_IMAGE_INFO_PKT_PATN);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_SEQ;
    }
    // start address offset must be block alignment
    else if (imgp->img_info.addr_off % dm_ctx.block_size)
    {
        DM_ASSERT (imgp->img_info.addr_off % dm_ctx.block_size == 0);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_ADDR;
        *pktsz = save_imgp (imgp);
    }
    // packet size must be page size + spare size
    else if (imgp->img_info.pkt_size != dm_ctx.page_size + dm_ctx.spare_size)
    {
        DM_ASSERT (imgp->img_info.pkt_size == dm_ctx.page_size + dm_ctx.spare_size);
        dm_ctx.dm_status = DM_STATUS_ERR_ONGOING;
        dm_ctx.dm_err = DM_ERR_WRONG_PKT_SZ;
        *pktsz = save_imgp (imgp);
    }
    else
    {
        *pktsz = save_imgp (imgp);
         dm_ctx.dm_status = DM_STATUS_SECT_ONGING;
         DM_TIME_BEGIN;
         // erase whole partition to keep data consistent
         force_erase (&imgp->img_info, *pktsz);
         DM_TIME_END_NAND_ERASE;
    }

    return *pktsz;
}

//======================================================================
//  check partition table info command
//======================================================================
void check_pt_cmd (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
    DM_PKT_TYPE pkt_type;
    char buffer[DM_CMD_MAX_SIZE];
    int i;
#ifdef DEBUG
    print ("DM_PARTITION_INFO_PACKET\n");
    print (": pattern %x\n", g_img_dl_pt_info.pattern);
    print (": part_num = %d\n", g_img_dl_pt_info.part_num);
    for (i = 0; i < PART_MAX_COUNT; i++)
    {
        print (": part_info[%d].part_name = %s\n", i, g_img_dl_pt_info.part_info[i].part_name);
        print (": part_info[%d].start_addr = %x\n", i, g_img_dl_pt_info.part_info[i].start_addr);
        print (": part_info[%d].part_len = %x\n", i, g_img_dl_pt_info.part_info[i].part_len);
        print (": part_info[%d].part_visibility = %x\n", i, g_img_dl_pt_info.part_info[i].part_visibility);
        print (": part_info[%d].dl_selected = %x\n", i, g_img_dl_pt_info.part_info[i].dl_selected);
        print ("\n");
    }
#endif    
    //==================================================
    // check if specified partitions can be downloaded
    //==================================================
#if CFG_PMT_SUPPORT
    errp.err_code = new_part_tab ((u8 *) &g_img_dl_pt_info);
#else
    print("%s : PMT is not supported\n", MOD);
    errp.err_code = DM_ERR_OK;
#endif
    mt_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);

    if (errp.err_code != DM_ERR_OK)
    {

        print ("\n%s : the specified partitions can not be downloaded, err_code = %d\n", MOD, errp.err_code);
       
        //================================
        // receive REBOOT packet
        //================================
        reset_dm_descriptor ();
        mt_usbtty_getcn (DM_SZ_REBOOT_STR, buffer);
        pkt_type = judge_pkt_type ((const void *) buffer);
        
        if (pkt_type == DM_PKT_REBT)
        {
            //print ("%s : received REBOOT packet\n", MOD);
            dm_ctx.dm_status = DM_STATUS_REBOOT;
        } 
        else   
        {
            //print ("%s : do not received REBOOT packet\n", MOD);
            dm_ctx.dm_status = DM_STATUS_START;
        }
    }
    else
    {
        dm_ctx.dm_status = DM_STATUS_START;
    }
}

//======================================================================
//  handle partition table info command
//======================================================================
void  handle_pt_cmd (void)
{
    DM_ENTRY_LOG ();
    mt_usbtty_getcn (DM_SZ_PT_INFO_CMD_PKT, (u8 *) & g_img_dl_pt_info);
    check_pt_cmd ();
}

/**************************************************************************
 *  Handle Image 
 **************************************************************************/
u32 handle_imgp (u32 * pktsz)
{
    DM_EXT_IMAGE_INFO_PACKET ext_imgp;
    DM_IMAGE_INFO_PACKET imgp;

    memset(&ext_imgp, 0, sizeof(DM_EXT_IMAGE_INFO_PACKET));
    memset(&imgp, 0, sizeof(DM_IMAGE_INFO_PACKET));

    DM_ENTRY_LOG ();
    mt_usbtty_getcn (DM_SZ_EXT_IMG_INFO_PKT, (u8 *) & ext_imgp);
    fill_internal_imgp(&imgp , &ext_imgp);

    check_imgp (&imgp, pktsz);
    mt_usbtty_puts (DM_STR_START_REQ);
    return *pktsz;
}

/**************************************************************************
 *  Return Packet Size Per Transmission
 **************************************************************************/
u32 save_imgp (DM_IMAGE_INFO_PACKET * imgp)
{
    u32 cnt;
    u32 extra_sz_for_patch_sig = 0;
    u32 extra_pk_cnt_for_patch_sig = 0;
    
    DM_ENTRY_LOG ();
    /* save image information to main descriptor */
    dm_ctx.img_info.img_format = imgp->img_info.img_format;
    dm_ctx.img_info.img_type = imgp->img_info.img_type;
    dm_ctx.img_info.img_size = imgp->img_info.img_size;
    dm_ctx.img_info.addr_off = imgp->img_info.addr_off;
    dm_ctx.img_info.addr_boundary = imgp->img_info.addr_boundary;
    dm_ctx.img_info.pkt_size = dm_ctx.page_size + dm_ctx.spare_size;

    dm_ctx.curr_cnt = 0;
    dm_ctx.curr_off = dm_ctx.img_info.addr_off;
    dm_ctx.page_off = dm_ctx.img_info.addr_off;


#ifdef FEATURE_DOWNLOAD_BOUNDARY_CHECK    
    dm_ctx.part_range = dm_ctx.img_info.addr_boundary - dm_ctx.img_info.addr_off;
#else
    dm_ctx.part_range = get_part_range (dm_ctx.img_info.img_type);
#endif

    if (dm_ctx.img_info.img_format == DM_IMG_FORMAT_YAFFS2)
    {
        cnt = (dm_ctx.img_info.img_size % dm_ctx.img_info.pkt_size) ?
            (dm_ctx.img_info.img_size / dm_ctx.img_info.pkt_size + 1) : (dm_ctx.img_info.img_size / dm_ctx.img_info.pkt_size);
        dm_ctx.pkt_cnt = cnt - 1;   /* because curr_cnt count from 0 */
    }
    else                        
    {
        cnt = (dm_ctx.img_info.img_size % dm_ctx.page_size) ?
            (dm_ctx.img_info.img_size / dm_ctx.page_size + 1) : (dm_ctx.img_info.img_size / dm_ctx.page_size);
        dm_ctx.pkt_cnt = cnt - 1;   /* because curr_cnt count from 0 */
    }

#if DM_DBG_LOG
    dump_dm_descriptor ();
#endif

    return dm_ctx.img_info.pkt_size;
}

/**************************************************************************
 *  Handle Flash Information Sent From Flash Tool
 **************************************************************************/
void handle_pl_info (void)
{
    DM_PL_INFO_PACKET plip = {0};
    DM_ENTRY_LOG ();
    plip.pattern = DM_PL_INFO_PKT_PATN;
    plip.chip = DRV_Reg16 (APHW_CODE);

    // fill _DM_FLASH_INFO according to NAND exported info.         
    plip.flash_info.man_code = nand_maf_id;
    plip.flash_info.dev_id = g_nand_chip.id;
    plip.flash_info.dev_code = nand_dev_id;
    plip.flash_info.dev_size = g_nand_chip.chipsize;
    plip.flash_info.page_size = g_nand_chip.page_size;
    plip.flash_info.spare_size = g_nand_chip.oobsize;

    // fill dm descriptor according to NAND exported info.          
    dm_ctx.block_size = g_nand_chip.erasesize;
    dm_ctx.page_size = g_nand_chip.page_size;
    dm_ctx.spare_size = g_nand_chip.oobsize;

    DM_LOG ("----------------------------------------\n");
    DM_LOG ("g_nand_chip.page_shift    = %x\n", g_nand_chip.page_shift);
    DM_LOG ("g_nand_chip.page_size     = %x\n", g_nand_chip.page_size);
    DM_LOG ("g_nand_chip.ChipID        = %x\n", g_nand_chip.ChipID);
    DM_LOG ("g_nand_chip.chips_name    = %x\n", g_nand_chip.chips_name);
    DM_LOG ("g_nand_chip.chipsize      = %x\n", g_nand_chip.chipsize);
    DM_LOG ("g_nand_chip.erasesize     = %x\n", g_nand_chip.erasesize);
    DM_LOG ("g_nand_chip.mfr           = %x\n", g_nand_chip.mfr);
    DM_LOG ("g_nand_chip.id            = %x\n", g_nand_chip.id);
    DM_LOG ("g_nand_chip.name          = %x\n", g_nand_chip.name);
    DM_LOG ("g_nand_chip.numchips      = %x\n", g_nand_chip.numchips);
    DM_LOG ("g_nand_chip.oobblock      = %x\n", g_nand_chip.oobblock);
    DM_LOG ("g_nand_chip.oobsize       = %x\n", g_nand_chip.oobsize);
    DM_LOG ("g_nand_chip.eccsize       = %x\n", g_nand_chip.eccsize);
    DM_LOG ("g_nand_chip.bus16         = %x\n", g_nand_chip.bus16);
    DM_LOG ("g_nand_chip.nand_ecc_mode = %x\n", g_nand_chip.nand_ecc_mode);
    DM_LOG ("----------------------------------------\n");

    dm_ctx.img_info.pkt_size = dm_ctx.page_size + dm_ctx.spare_size;
    mt_usbtty_putcn (DM_SZ_PL_INFO_PKT, (u8 *) & plip, TRUE);
}

/**************************************************************************
 *  Handle Error
 **************************************************************************/
void handle_errp (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
    DM_ENTRY_LOG ();
    errp.err_code = dm_ctx.dm_err;
    mt_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
    dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT; //DM_STATUS_REBOOT;
    return;
}

//======================================================================
//  update partition table 
//======================================================================
void  handle_update (void)
{
    DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };

    //==================================================
    // check if specified partitions can be updated
    //==================================================
#if CFG_PMT_SUPPORT    
    errp.err_code = update_part_tab ();
#else
    print("%s : PMT is not supported\n", MOD);
    errp.err_code = DM_ERR_OK;
#endif
    mt_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
    dm_ctx.dm_status = DM_STATUS_SECT_WAIT_NXT;
}    

/**************************************************************************
*  Reboot State
**************************************************************************/
void handle_reboot (void)
{
    // check if device Is ready to use
    g_boot_dev.dev_wait_ready ();
    DM_TIME_TOTAL_END;
#if DM_TIME_ANALYSIS
    dump_time_analysis ();
#endif
    mt_usb_disconnect_internal();
    do_reboot (0);
}

/**************************************************************************
 *  Auto Boot State
 **************************************************************************/
void handle_autoboot (void)
{
    /* check if device Is ready to use */
    g_boot_dev.dev_wait_ready ();

    DM_TIME_TOTAL_END;
#if DM_TIME_ANALYSIS
    dump_time_analysis ();
#endif

    mt_usb_disconnect_internal();
    do_reboot (1);
}


/**************************************************************************
 *  Middle State
 **************************************************************************/
void handle_midle_state (u8 * buf)
{
    DM_PKT_TYPE pkt_type;
    DM_EXT_IMAGE_INFO_PACKET *ext_imgp;
    DM_PATCH_CMD_PACKET *patch_cmd;
    DM_IMAGE_INFO_PACKET imgp;

    u32 pktsz = 0;
    part_t *part;
    u8 *name = NULL;
    
    DM_ENTRY_LOG ();

    memset(&imgp, 0, sizeof(DM_IMAGE_INFO_PACKET));
    reset_dm_descriptor ();

    /* receive a little bit data, check if it is REBOOT */
    mt_usbtty_getcn (DM_SZ_REBOOT_STR, buf);
    pkt_type = judge_pkt_type ((const void *) buf);

    if (pkt_type == DM_PKT_REBT)
    {
        dm_ctx.dm_status = DM_STATUS_REBOOT;
        return;
    }

    if (pkt_type == DM_PKT_AUTO)
    {
        print ("autoboot mode !!!\n");
        dm_ctx.dm_status = DM_STATUS_ATBOOT;
        return;
    }

    if (pkt_type == DM_PKT_UPDT)
    {
        print ("update partition table\n");
        dm_ctx.dm_status = DM_STATUS_UPDATE;
        return;
    }
    
    /* pcaket type is IMGP */
    if (pkt_type == DM_PKT_IMGP)
    {
        //print ("\nCASE 2 : pcaket type is DM_PKT_IMGP\n");
        mt_usbtty_getcn (DM_SZ_EXT_IMG_INFO_PKT - DM_SZ_REBOOT_STR, (u8 *) (buf + DM_SZ_REBOOT_STR));
        ext_imgp = (DM_EXT_IMAGE_INFO_PACKET *) buf;
        fill_internal_imgp (&imgp , &(*ext_imgp));
        check_imgp (&imgp, &pktsz);
        mt_usbtty_puts (DM_STR_START_REQ);
    }
    else
    {
        print ("\nCASE 3 : others\n");
        if (pkt_type != DM_PKT_IMGP)
        {
            print ("pkt_type = 0x%x\n", pkt_type);
        }
        ASSERT (0);
    }
 
    return;
}


/**************************************************************************
 *  Allocate Buffer
 **************************************************************************/
u8 * prepare_data_buf (void)
{
#if 1 //CFG_FPGA_PLATFORM
    dm_ctx.data_buf = (u8 *) 0xF00000;
#else
    dm_ctx.data_buf = (u8 *) dm_rx_buf;
#endif 
    if (dm_ctx.data_buf)
        return dm_ctx.data_buf;

    DM_ASSERT (dm_ctx.data_buf);

    {
        DM_ERRCODE_PACKET errp = { DM_ERROR_PKT_PATN, 0 };
        errp.err_code = DM_ERR_NOMEM;   //TODO
        mt_usbtty_putcn (DM_SZ_ERR_CODE_PKT, (u8 *) & errp, TRUE);
        return NULL;
    }
}

/**************************************************************************
 *  Download Main Function
 **************************************************************************/
void download_handler(void)
{
    u32 pktsz = 0;
    u8 *buf = NULL;

    handle_pl_info ();
    buf = prepare_data_buf ();

    /* init global vars */
    g_dl_safe_start_addr = 0;
    g_cust_key_init = FALSE;
    g_end_user_flash_tool = FALSE;
    g_sec_img_patch_enable = FALSE;
    g_sec_img_patch_valid = FALSE;

#if DM_CAL_CKSM_FROM_USB_BUFFER
    dm_ctx.chk_sum = 0;
#endif
#if DM_TIME_ANALYSIS
    g_DM_TIME_Tmp = 0;
    g_DM_TIME_ReadData = 0;
    g_DM_TIME_FlashData = 0;
    g_DM_TIME_Erase = 0;
    g_DM_TIME_USBGet = 0;
    g_DM_TIME_SkipBadBlock = 0;
    g_DM_TIME_Checksum = 0;
    g_DM_TIME_Total_Begin = 0;
    g_DM_TIME_Total = 0;
#endif
    reset_dm_descriptor ();

    DM_TIME_TOTAL_BEGIN;

    if (!buf)
        return;

    dm_ctx.dm_status = DM_STATUS_START;

    while (1)
    {
        if (dm_ctx.dm_status == DM_STATUS_SECT_ONGING)
        {
            DM_STATE_LOG ("SECT_ONGOING");
            handle_data (pktsz, buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_START)
        {
            DM_STATE_LOG ("START");
            handle_imgp (&pktsz);
        }
        else if (dm_ctx.dm_status == DM_STATUS_SECT_FINISHED)
        {
            DM_STATE_LOG ("SECT_FINISHED");
            handle_cksm ();
        }
        else if (dm_ctx.dm_status == DM_STATUS_SECT_WAIT_NXT)
        {
            DM_STATE_LOG ("WAIT_NEXT");
            handle_midle_state (buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_REBOOT)
        {
            DM_STATE_LOG ("REBOOT");
            handle_reboot ();
            break;
        }
        else if (dm_ctx.dm_status == DM_STATUS_ATBOOT)
        {
            DM_STATE_LOG ("ATBOOT");
            handle_autoboot ();
            break;
        }
        else if (dm_ctx.dm_status == DM_STATUS_UPDATE)
        {
            DM_STATE_LOG ("UPDATE");
            handle_update ();
        }
        else if (dm_ctx.dm_status == DM_STATUS_ERR_ONGOING)
        {
            DM_STATE_LOG ("ERR_ONGOING");
            handle_data (pktsz, buf);
        }
        else if (dm_ctx.dm_status == DM_STATUS_ERR_FINISHED)
        {
            DM_STATE_LOG ("ERR_FINISHED");
            handle_errp ();
        }
    }
    return;
}
#endif /* CFG_LEGACY_USB_DOWNLOAD */
