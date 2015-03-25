/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
/*******************************************************************************
* Filename:
* ---------
*  da_stage.c
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains the DA stage
*
* Author:
* -------
*  Kevin Lim (mtk60022)
*
*==============================================================================
*           HISTORY
* Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*------------------------------------------------------------------------------
* $Revision$
* $Modtime$
* $Log$
*
* 04 12 2013 monster.huang
* [STP100006902]  [FlashTool] v5.1312/v5.1314 maintenance
* [BROM LITE] Support combo memory and automatic generate eppParam.
* Fetch bmt address by acquiring linux partitions.
*
* 02 07 2013 stanley.song
* [STP100006748]  Update Brom Lite for SV5
* Update BROM Lite to support MT6255
 *
 * 10 01 2010 kevin.lim
 * [STP100004187]  Upload the BROM-lite source code on P4 server.
 * 10 01 2010 Kevin Lim
 * [STP100004187] BROM Lite v1.1037.2 release
*
*
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*==============================================================================
*******************************************************************************/
#include "da_stage.h"
#include "_External/include/DOWNLOAD.H"
#include "image.h"
#include "_External/include/mtk_mcu.h"
#include "_External/include/flash_dev_tbl.h"
#include "_External/include/nand_dev_tbl.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__GNUC__)
#include "GCC_Utility.h"
#endif



static unsigned short ComputeChecksum(const unsigned char *buf,
                                      unsigned int bufSize,
                                      unsigned short initialChecksum)
{
    unsigned int numWords;
    unsigned short *p;
    unsigned int wordIndex;
    unsigned short checksum;

    if(buf == NULL) return 0;
    if((bufSize % 2) != 0) return 0;

    numWords = bufSize / 2;
    p = (unsigned short *)buf;
    checksum = initialChecksum;

    for (wordIndex=0; wordIndex<numWords; ++wordIndex)
    {
        checksum ^= *(p + wordIndex);
    }


    return checksum;
}

static unsigned short GetImageChecksum(const unsigned char *p_temp,
                                       const unsigned int pkg_len)
{
    unsigned short init = 0;
    const unsigned char * first = p_temp;
    const unsigned char * last = p_temp + pkg_len;
    while ( first!=last )
    {
        init = init + *first++;
    }
    return init;
}

static unsigned short GetYaffsImageChecksum(const unsigned char *p_temp,
                                            const unsigned int pkg_len,
                                            const uint16 page_size,
                                            const uint16 spare_size)
{
    unsigned short check_sum = 0;
    unsigned int pagespare_len = page_size + spare_size;

    unsigned int j;
    for (j = 0; j < pkg_len; j += pagespare_len) {
        unsigned int i;
        for (i = 0; i < page_size; i++) {
            check_sum += *(p_temp+i);
        }
        p_temp += pagespare_len;
    }
    return check_sum;

}

static STATUS_E detect_flash(COM_HANDLE com_handle,
                             const struct image *flash_table,
                             unsigned int entry_size)
{
    const unsigned int table_size = flash_table->len / entry_size;
    unsigned char response = 0;
    unsigned int i;

    if (com_send_dword(com_handle, table_size) != COM_STATUS_DONE)
    {
        return 300;
    }

    if (com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
    {
        return 301;
    }

    if (response != ACK)
    {
        return 302;
    }

    for (i=0; i<table_size; ++i)
    {
        if (com_send_data(com_handle, &flash_table->buf[i*entry_size],
            entry_size) != COM_STATUS_DONE)
        {
            return 303;
        }

        if (com_recv_data_chk_len(com_handle, &response,1) != COM_STATUS_DONE)
        {
            return 304;
        }

        if (response == ACK)
        {
            break;
        }
    }

    return S_DONE;
}


STATUS_E da_connect(COM_HANDLE com_handle,
                    const struct image *nor_flash_table,
                    const struct image *nand_flash_table,
                    int isUSB,
                    unsigned int bmt_address)
{
    unsigned char chip_type = 0; int i=0;

    Sleep(1000);    // Wait for DA to initialize itself

    // Get SYNC_CHAR
    while (1)
    {
        COM_STATUS com_status;
        unsigned char response = 0;
        log_output("Get SYNC_CHAR ");
        com_status = com_recv_byte(com_handle, &response);
        log_output("%02X ", response);

        if (com_status == COM_STATUS_DONE)
        {
            if (response == SYNC_CHAR)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        else if (com_status == COM_STATUS_READ_TIMEOUT)
        {
            continue;
        }
        else
        {
            return S_FT_DA_INIT_SYNC_ERROR;
        }
    }

    // Get DA version
    {
        unsigned char version[2] = { 0 };

        if (com_recv_data(com_handle, version, 2) != COM_STATUS_DONE)
        {
            return S_FT_DA_INIT_SYNC_ERROR;
        }

        if ((version[0] != 0x03) || (version[1] != 0x02))
        {
            return S_FT_DA_VERSION_INCORRECT;
        }
    }

    // Get BB chip type
    {
        if (com_recv_byte(com_handle, &chip_type) != COM_STATUS_DONE)
        {
            return 601;
        }

        // This BROM Lite is restricted for MT6280
        if (chip_type != MT6280)
        {
            return S_UNKNOWN_TARGET_BBCHIP;
        }
    }

    // Trigger BAT_ON check for USBDL W/O Battery
    //send NAK
    if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
    {
        return 602;
    }

    // Send BootROM version and bootloader version
    {
        // send brom version
        //if (com_send_byte(com_handle, 0x05) != COM_STATUS_DONE)
        //TODO: 0x05 or 0xFF ??
        if (com_send_byte(com_handle, 0xFF) != COM_STATUS_DONE)
        {
            return 603;
        }
        // send BLOADER version
        if (com_send_byte(com_handle, 0xFE) != COM_STATUS_DONE)
        {
            return 604;
        }
    }

    // Send chip select
    {
        if (com_send_byte(com_handle, CS0) != COM_STATUS_DONE)
        {
            return 605;
        }

        if (com_send_byte(com_handle, CS_WITH_DECODER) != COM_STATUS_DONE)
        {
            return 606;
        }

        if (com_send_byte(com_handle, CS0) != COM_STATUS_DONE)
        {
            return 607;
        }
    }

    // Send NFI_ACCCON setting
    {
        if (com_send_dword(com_handle, 0x7007ffff) != COM_STATUS_DONE) //for MT6235B
        {
            return 608;
        }
    }

    // Send clock setting
    {
        if (com_send_byte(com_handle, EXT_26M) != COM_STATUS_DONE)
        {
            return 609;
        }
    }


    // Detect NOR flash
    {
        int ret = 0;
        const unsigned int entry_size = sizeof(NOR_Device_S);
        if ( (ret = detect_flash(com_handle, nor_flash_table, entry_size)) != S_DONE)
        {
            return ret;
        }
    }

    // Detect NAND flash
    {
        unsigned char response = 0;
        unsigned int bmt_result = 0;

        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            return 610;
        }

        if (response == ACK)
        {
            const unsigned int entry_size = sizeof(NAND_Device_S);
            // Send the begin address of BMT Pool for linux partition download
            if (com_send_dword(com_handle, bmt_address) != COM_STATUS_DONE)
            {
                return 611;
            }

            if (detect_flash(com_handle, nand_flash_table,
                entry_size) != S_DONE)
            {
                return 612;
            }

            //Read Init BMT result
            if (com_recv_dword(com_handle, &bmt_result) != COM_STATUS_DONE)
            {
                return 613;
            }


        }
    }


    if (isUSB)
    {
        Sleep(3000); // Wait for DA to detect external DRAM size
    }

    // Get HW detection report
    {
        DA_REPORT_T report = { 0 };
        unsigned char tmp[2] = { 0 };
        int i=0; //counter

        //
        // NOR flash report
        //
        if (com_recv_dword(com_handle, &report.m_nor_ret))
        {
            return 614;
        }

        if (com_recv_data(com_handle, tmp, 2))
        {
            return 615;
        }

        report.m_nor_chip_select[0] = tmp[0];
        report.m_nor_chip_select[1] = tmp[1];

        if (com_recv_word(com_handle,
            &report.m_nor_flash_id) != COM_STATUS_DONE)
        {
            return 616;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_size) != COM_STATUS_DONE)
        {
            return 617;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_size_die1) != COM_STATUS_DONE)
        {
            return 618;
        }



        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_1) != COM_STATUS_DONE)
        {
            return 619;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_2) != COM_STATUS_DONE)
        {
            return 620;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_3) != COM_STATUS_DONE)
        {
            return 621;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_4) != COM_STATUS_DONE)
        {
            return 622;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_otp_status) != COM_STATUS_DONE)
        {
            return 623;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_otp_size) != COM_STATUS_DONE)
        {
            return 624;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_id_die2) != COM_STATUS_DONE)
        {
            return 625;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_size_die2) != COM_STATUS_DONE)
        {
            return 626;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_1_die2) != COM_STATUS_DONE)
        {
            return 627;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_2_die2) != COM_STATUS_DONE)
        {
            return 628;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_3_die2) != COM_STATUS_DONE)
        {
            return 629;
        }

        if (com_recv_word(com_handle,
            &report.m_nor_flash_dev_code_4_die2) != COM_STATUS_DONE)
        {
            return 630;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_otp_status_die2) != COM_STATUS_DONE)
        {
            return 631;
        }

        if (com_recv_dword(com_handle,
            &report.m_nor_flash_otp_size_die2) != COM_STATUS_DONE)
        {
            return 632;
        }

        //
        // NAND flash report
        //
        if (com_recv_dword(com_handle, &report.m_nand_ret))
        {
            return 633;
        }

        if (com_recv_byte(com_handle, &tmp[0]))
        {
            return 634;
        }

        report.m_nand_chip_select = tmp[0];

        if (com_recv_word(com_handle,
            &report.m_nand_flash_id) != COM_STATUS_DONE)
        {
            return 635;
        }

        if (com_recv_dword(com_handle,
            &report.m_nand_flash_size) != COM_STATUS_DONE)
        {
            return 636;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_1) != COM_STATUS_DONE)
        {
            return 637;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_2) != COM_STATUS_DONE)
        {
            return 638;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_3) != COM_STATUS_DONE)
        {
            return 639;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_4) != COM_STATUS_DONE)
        {
            return 640;
        }

        //add second NAND info
        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_1_part2) != COM_STATUS_DONE)
        {
            return 641;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_2_part2) != COM_STATUS_DONE)
        {
            return 642;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_3_part2) != COM_STATUS_DONE)
        {
            return 643;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_flash_dev_code_4_part2) != COM_STATUS_DONE)
        {
            return 644;
        }


        if (com_recv_word(com_handle,
            &report.m_nand_pagesize) != COM_STATUS_DONE)
        {
            return 645;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_sparesize) != COM_STATUS_DONE)
        {
            return 646;
        }

        if (com_recv_word(com_handle,
            &report.m_nand_pages_per_block) != COM_STATUS_DONE)
        {
            return 647;
        }

        if (com_recv_data(com_handle, tmp, 2))
        {
            return 648;
        }

        report.m_nand_io_interface = tmp[0];
        report.m_nand_addr_cycle = tmp[1];

        //detect EMMC for SV5
        if (com_recv_dword(com_handle,
            &report.m_emmc_ret) != COM_STATUS_DONE)
        {
            return 649;
        }

        if (com_recv_byte(com_handle,
            &report.m_emmc_manufacture_id) != COM_STATUS_DONE)
        {
            return 650;
        }

        for(i = 0 ; i < 6 ; i ++)
        {
            if (com_recv_byte(com_handle,
                &report.m_emmc_product_name[i]) != COM_STATUS_DONE)
            {
                return 651;
            }

        }
        report.m_emmc_product_name[6] = '\0';

        if (com_recv_byte(com_handle,
            &report.m_emmc_partitioned) != COM_STATUS_DONE)
        {
            return 652;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_boot1_size) != COM_STATUS_DONE)
        {
            return 653;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_boot2_size) != COM_STATUS_DONE)
        {
            return 654;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_rpmb_size) != COM_STATUS_DONE)
        {
            return 655;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_gp1_size) != COM_STATUS_DONE)
        {
            return 656;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_gp2_size) != COM_STATUS_DONE)
        {
            return 657;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_gp3_size) != COM_STATUS_DONE)
        {
            return 658;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_gp4_size) != COM_STATUS_DONE)
        {
            return 659;
        }
        if (com_recv_dword(com_handle,
            &report.m_emmc_ua_size) != COM_STATUS_DONE)
        {
            return 660;
        }
        // SV3
        //report.m_emmc_ret = S_DA_EMMC_FLASH_NOT_FOUND; // for SV3, no EMMC support
        //report.m_emmc_partitioned = 0;

        //
        // Internal RAM report
        //
        if (com_recv_dword(com_handle,
            &report.m_int_sram_ret) != COM_STATUS_DONE)
        {
            return 661;
        }

        if (com_recv_dword(com_handle,
            &report.m_int_sram_size) != COM_STATUS_DONE)
        {
            return 662;
        }

        //
        // External RAM report
        //
        if (com_recv_dword(com_handle,
            &report.m_ext_ram_ret) != COM_STATUS_DONE)
        {
            return 663;
        }

        if (com_recv_data(com_handle, tmp, 2))
        {
            return 664;
        }

        report.m_ext_ram_type = tmp[0];
        report.m_ext_ram_chip_select = tmp[1];

        if (com_recv_dword(com_handle,
            &report.m_ext_ram_size) != COM_STATUS_DONE)
        {
            return 665;
        }

        //for SV5 read SF_DetectionTable
        for(i=0;i<12;i++)
        {
            if (com_recv_data(com_handle, tmp, 1))
            {
                return 666;
            }
        }

        // read ACK for check all data is sent
        if (com_recv_byte(com_handle, &tmp[0]) != COM_STATUS_DONE)
        {
            return 667;
        }
    }

    // Wait for DA to scan flash content
    {
        unsigned char response = 0;

        while (1)
        {
            unsigned char response = 0;

            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                continue;
            }

            if (response == ACK)
            {
                break;
            }
            else
            {
                return 668;
            }
        }
    }

    // Get download status
    {
        unsigned int download_status = 0;

        if (com_recv_dword(com_handle, &download_status) != COM_STATUS_DONE)
        {
            return 669;
        }
    }

    // Get boot style
    {
        unsigned int boot_style = 0;

        if (com_recv_dword(com_handle, &boot_style) != COM_STATUS_DONE)
        {
            return 670;
        }
    }

    // Get SOC check result
    {
        unsigned char soc_check_result = 0;

        if (com_recv_byte(com_handle, &soc_check_result) != COM_STATUS_DONE)
        {
            return 671;
        }
    }

    return S_DONE;
}


STATUS_E SV5_CMD_CheckBootLoaderFeature_CheckLoadType(COM_HANDLE com_handle,
                                        const struct image *img,
                                        Feature_Check_Type checkType)
{
    unsigned int status;
    unsigned int target_feature_combination;
    unsigned int target_bl_maui_paired_ver;

    // 1. Send command
    if (com_send_byte(com_handle, DA_BL_FEATURE_CHECK_CMD) != COM_STATUS_DONE)
    {
        return 950;
    }
    // 2. send checking method: get feature from target's BL or MAUI
    if (com_send_dword(com_handle, checkType) != COM_STATUS_DONE)
    {
        return 951;
    }
    //
    //  3. Wait for response
    //
    if (com_recv_dword(com_handle, &status) != COM_STATUS_DONE)
    {
        return 952;
    }
    if (status != S_DONE)
    {
        //Feature info not found %s, target flash is empty
        log_output("Feature info(%d) not found, target flash is empty \n",checkType);
        return S_DONE;
    }
    // 4. Get feature from target


    if (com_recv_dword(com_handle, &target_feature_combination) != COM_STATUS_DONE)
    {
        return 953;
    }

    if (com_recv_dword(com_handle, &target_bl_maui_paired_ver) != COM_STATUS_DONE)
    {
        return 954;
    }

    //Skip to get feature info from PC's ARM_BL
    //No check features between PC's and target's ARM_BL

    return S_DONE;
}

STATUS_E da_write_boot_loader(COM_HANDLE com_handle,
                              const struct image *bootLoader,
                              const struct image *extBootLoader,
                              const struct image *dspBootLoader)
{
    const unsigned int packet_len = 4096;
    unsigned char response = 0;
    unsigned int da_ret = 0;
    unsigned int bl_status = 0;
    unsigned int i;
    unsigned int num_bytes_sent;
    unsigned int num_of_bl = 0;
    unsigned int num_of_download_bl = 0;
    unsigned int offset;

    GFH_FILE_INFO_v1 bootLoader_gfh_file_info[3];
    GFH_BL_INFO_v1    bootLoader_gfh_bl_info[3];

    if(bootLoader != NULL)
    {
        offset =0;
        memcpy(&bootLoader_gfh_file_info[0], bootLoader->buf, sizeof(GFH_FILE_INFO_v1));
        //Get GFH_BL_INFO_v1
        for(i=0;i<10;i++)
        {
            memcpy(&bootLoader_gfh_bl_info[0], bootLoader->buf+offset, sizeof(GFH_BL_INFO_v1));

            if(bootLoader_gfh_bl_info[0].m_gfh_hdr.m_type == GFH_BL_INFO)
            {
                break;
            }else{
                offset += bootLoader_gfh_bl_info[0].m_gfh_hdr.m_size;
                if(i==9)
                {
                    return 800;
                }
            }
        }
        num_of_bl = 1;
        num_of_download_bl ++;
    }
    if(extBootLoader != NULL)
    {
        offset =0;
        memcpy(&bootLoader_gfh_file_info[1], extBootLoader->buf, sizeof(GFH_FILE_INFO_v1));
        for(i=0;i<10;i++)
        {
            memcpy(&bootLoader_gfh_bl_info[1], extBootLoader->buf+offset, sizeof(GFH_BL_INFO_v1));

            if(bootLoader_gfh_bl_info[1].m_gfh_hdr.m_type == GFH_BL_INFO)
            {
                break;
            }else{
                offset += bootLoader_gfh_bl_info[1].m_gfh_hdr.m_size;
                if(i==9)
                {
                    return 801;
                }
            }
        }
        num_of_bl = 2;
        num_of_download_bl ++;
    }
    if(dspBootLoader != NULL)
    {
        offset =0;
        memcpy(&bootLoader_gfh_file_info[2], dspBootLoader->buf, sizeof(GFH_FILE_INFO_v1));
        for(i=0;i<10;i++)
        {
            memcpy(&bootLoader_gfh_bl_info[2], dspBootLoader->buf+offset, sizeof(GFH_BL_INFO_v1));

            if(bootLoader_gfh_bl_info[2].m_gfh_hdr.m_type == GFH_BL_INFO)
            {
                break;
            }else{
                offset += bootLoader_gfh_bl_info[2].m_gfh_hdr.m_size;
                if(i==9)
                {
                    return S_FT_NFB_DOWNLOAD_BOOTLOADER_FAIL;
                }
            }
        }
        num_of_bl = 3;
        num_of_download_bl ++;
    }


    // send Format HB command
    if (com_send_byte(com_handle, DA_FORMAT_HB) != COM_STATUS_DONE)
    {
        return 802;
    }
    // send Flash Dev
    if (com_send_byte(com_handle, 0x02) != COM_STATUS_DONE)
    {
        return 803;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 804;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 805;
        }
        log_output("[WARNING]Format HB error!!! err(%d)\n", da_ret);
    }


    // send DA_DOWNLOAD_BLOADER_CMD
    if (com_send_byte(com_handle, DA_DOWNLOAD_BLOADER_CMD) != COM_STATUS_DONE)
    {
        return 806;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 807;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 808;
        }
        return da_ret;
    }

    // send packet length
    if (com_send_dword(com_handle, packet_len) != COM_STATUS_DONE)
    {
        return 809;
    }

    //- 1.2. send flash_dev type
    if (com_send_byte(com_handle, F_NAND_SEQUENTIAL) != COM_STATUS_DONE)
    {
        return 810;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 811;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 812;
        }
        return da_ret;
    }
    //check Header Block...
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 813;
    }
    if (response == ACK)
    {
        //TRACE("DA_cmd::CMD_DownloadBootLoader(): 1st download ... \n");
    }else if (response == CONT_CHAR)
    {
        //TRACE("DA_cmd::CMD_DownloadBootLoader(): updating ... \n");
    }else
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 814;
        }
        return da_ret;
    }

    //2. Send num of BLs
    if (com_send_dword(com_handle, num_of_download_bl) != COM_STATUS_DONE)
    {
        return 815;
    }

    Sleep(1000);

    //4. Get BL Profiles
    for(i=0; i<num_of_bl; i++)
    {
        const struct image *blImage;
        switch(i)
        {
        case 0:
            blImage = bootLoader;
            break;
        case 1:
            blImage = extBootLoader;
            break;
        case 2:
            blImage = dspBootLoader;
            break;
        }
        if(blImage == NULL)
        {
            continue;
        }
        // 4.1 BL_EXIST_MAGIC
        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return 816;
        }

        // 4.2 BL_DEV
        if (com_send_byte(com_handle, F_NAND_SEQUENTIAL) != COM_STATUS_DONE)
        {
            return 817;
        }
        // 4.3 BL_TYPE
        if (com_send_word(com_handle, bootLoader_gfh_file_info[i].m_file_type) != COM_STATUS_DONE)
        {
            return 818;
        }
        // 4.4 BL_BEGIN_DEV_ADDR
        if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_load_addr) != COM_STATUS_DONE)
        {
            return 819;
        }

        if(bootLoader_gfh_file_info[i].m_max_size == 0xFFFFFFFF)
        {
            if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_load_addr+bootLoader_gfh_file_info[i].m_file_len) != COM_STATUS_DONE)
            {
                return 820;
            }
        }else{
            if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_load_addr+bootLoader_gfh_file_info[i].m_max_size) != COM_STATUS_DONE)
            {
                return 821;
            }
        }

        // 4.6 BL_ATTRIBUTE
        if (com_send_dword(com_handle, bootLoader_gfh_bl_info[i].m_bl_attr) != COM_STATUS_DONE)
        {
            return 822;
        }

        // 4.7 BL_MAX_SIZE
        if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_max_size) != COM_STATUS_DONE)
        {
            return 823;
        }

        // 4.8 AC_C_Enable
        if (com_send_byte(com_handle, 0) != COM_STATUS_DONE)
        {
            return 824;
        }
        // 4.9 AC Offset
        if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_content_offset + 0) != COM_STATUS_DONE)
        {
            return 825;
        }
        // 4.10 AC Length
        if (com_send_dword(com_handle, 0) != COM_STATUS_DONE)
        {
            return 826;
        }

        //----- EXT INFO ----
        // 4.11 BL_SIZE
        if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_file_len) != COM_STATUS_DONE)
        {
            return 827;
        }
        // 4.12 BL_RESERVED_BLOCK
        if (com_send_byte(com_handle, 0) != COM_STATUS_DONE)
        {
            return 828;
        }
        // 4.13 BL_ALIGN_TYPE
        if (com_send_byte(com_handle, 0) != COM_STATUS_DONE)
        {
            return 829;
        }
        // 4.14 BL_FILEINFO_ATTR
        if (com_send_dword(com_handle, bootLoader_gfh_file_info[i].m_attr) != COM_STATUS_DONE)
        {
            return 830;
        }

        //---- Check result ----
        // read ack
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            return 831;
        }
        if (response != ACK)
        {
            if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
            {
                return 832;
            }
            return da_ret;

        }
    }

    //5. Set BL Profiles
    // read ack
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 833;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 834;
        }
        return da_ret;

    }
    // 5.5 wait for BL Self update
    // read ack
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 835;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 836;
        }
        return da_ret;
    }
    //6. Send BL
    //send boot loader
    for(i=0; i<num_of_bl; i++)
    {

        const struct image *blImage;
        switch(i)
        {
        case 0:
            blImage = bootLoader;
            break;
        case 1:
            blImage = extBootLoader;
            break;
        case 2:
            blImage = dspBootLoader;
            break;
        }
        if(blImage == NULL)
        {
            continue;
        }

        num_bytes_sent = 0;
        while (num_bytes_sent < blImage->len)
        {
            unsigned int j;
            unsigned short checksum;
            unsigned int num_bytes_to_send;
            unsigned short dummy_checksum = 0;

            checksum = 0;

            if (packet_len > (blImage->len - num_bytes_sent))
            {
                num_bytes_to_send = blImage->len - num_bytes_sent;
            }
            else
            {
                num_bytes_to_send = packet_len;
            }

            if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
            {
                return 837;
            }

            if (com_send_data(com_handle, blImage->buf + num_bytes_sent,
                num_bytes_to_send) != COM_STATUS_DONE)
            {
                return 838;
            }

            // calculate checksum
            for(j=0; j<num_bytes_to_send; j++) {
                // WARNING: MUST make sure it unsigned value to do checksum
                checksum += blImage->buf[num_bytes_sent+j];
                //log_output("%x ", blImage->buf[num_bytes_sent+j]);
            }

            if (com_send_word(com_handle, checksum) != COM_STATUS_DONE)
            {
                return 839;
            }

            while (1)
            {
                // TODO: error handling
                if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
                {
                    continue;
                }

                if (response == CONT_CHAR)
                {
                    break;
                }
                else if((response == NACK)||(response == STOP_CHAR))
                {
                    return 840;
                }
            }

            num_bytes_sent += num_bytes_to_send;
        }
    }

    //7. download finish
    // read ack

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 841;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 842;
        }
        return da_ret;

    }
    else
    {
        if (com_recv_dword(com_handle, &bl_status) != COM_STATUS_DONE)
        {
            return 843;
        }

    }
    return S_DONE;
}


STATUS_E SV5_CMD_GetSettingOfCBR(COM_HANDLE com_handle, unsigned int *startBlockOfCBR,
                                    unsigned int *numNormalBlock, unsigned int *numFastlBlock,
                                    unsigned int *numSpareBlock, unsigned int *numMaxRecordInBlock,
                                            unsigned int *numTotalBlock, unsigned char *bReadOnlyMode,
                                            unsigned int *cbr_version)
{
    unsigned char response = 0;
    unsigned int errorCode;

    if (com_send_byte(com_handle, DA_GET_CBR_SETTTING_CMD) != COM_STATUS_DONE)
    {
        return 940;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 941;
    }
    if (response != ACK)
    {

        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
        }
        return errorCode;
    }

    if (com_recv_dword(com_handle, startBlockOfCBR) != COM_STATUS_DONE)
    {
        return 942;
    }
    if (com_recv_dword(com_handle, numNormalBlock) != COM_STATUS_DONE)
    {
        return 943;
    }
    if (com_recv_dword(com_handle, numFastlBlock) != COM_STATUS_DONE)
    {
        return 944;
    }
    if (com_recv_dword(com_handle, numSpareBlock) != COM_STATUS_DONE)
    {
        return 945;
    }
    if (com_recv_dword(com_handle, numMaxRecordInBlock) != COM_STATUS_DONE)
    {
        return 946;
    }
    if (com_recv_dword(com_handle, numTotalBlock) != COM_STATUS_DONE)
    {
        return 947;
    }
    if (com_recv_byte(com_handle, bReadOnlyMode) != COM_STATUS_DONE)
    {
        return 948;
    }
    if (com_recv_dword(com_handle, cbr_version) != COM_STATUS_DONE)
    {
        return 949;
    }

    return 0;
}

STATUS_E SV5_CMD_CreateCBR(COM_HANDLE com_handle, unsigned int numNormalBlock,
                                    unsigned int numFastlBlock, unsigned int numMinSpareBlock,
                                    unsigned int numMaxSpareBlock, unsigned int numMaxRecordInBlock,
                                            unsigned char bReadOnlyMode,  unsigned int cbr_version)
{
    unsigned char response = 0;
    unsigned int errorCode;

    if (com_send_byte(com_handle, DA_CREATE_CBR_CMD) != COM_STATUS_DONE)
    {
        return 1;
    }

    if (com_send_dword(com_handle, numNormalBlock) != COM_STATUS_DONE)
    {
        return 4;
    }
    if (com_send_dword(com_handle, numFastlBlock) != COM_STATUS_DONE)
    {
        return 5;
    }
    if (com_send_dword(com_handle, numMinSpareBlock) != COM_STATUS_DONE)
    {
        return 6;
    }
    if (com_send_dword(com_handle, numMaxSpareBlock) != COM_STATUS_DONE)
    {
        return 7;
    }
    if (com_send_dword(com_handle, numMaxRecordInBlock) != COM_STATUS_DONE)
    {
        return 8;
    }
    if (com_send_byte(com_handle, bReadOnlyMode) != COM_STATUS_DONE)
    {
        return 10;
    }
    if (com_send_dword(com_handle, cbr_version) != COM_STATUS_DONE)
    {
        return 11;
    }

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 12;
    }
    if (response != ACK)
    {
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 13;
        }
    }

    return 0;
}
#define NUM_OF_ROM 4
STATUS_E da_write_NFB_images(COM_HANDLE com_handle,
                             const struct image *rom,
                             const struct image *secondRom,
                             const struct image *dspRom,
                             const struct image *demand_paging_rom)
{
    const unsigned int packet_len = 4096;
    unsigned char response = 0;
    unsigned int i,j;
    unsigned int m_chksum;
    unsigned int num_bytes_sent;
    int ret = S_DONE;
    unsigned int errorCode = 0;
    unsigned int offset = 0;
    unsigned int num_of_rom = 0;
    unsigned int num_of_download_rom = 0;

    //CBR information
    unsigned int startBlockOfCBR = 0;
    unsigned int numNormalBlock = 0;
    unsigned int numFaskBlock = 0;
    unsigned int numSpareBlock = 0;
    unsigned int numMaxRecordInBlock = 0;
    unsigned int numTotalBlock = 0;
    unsigned char bReadOnlyMode = 1;
    unsigned int target_cbr_version=0;

    //Flash Layout
    unsigned int regionCount = 0;
    unsigned int binaryType = 0;
    unsigned int startPage = 0;
    unsigned int boundPage = 0;


    GFH_FILE_INFO_v1 bootLoader_gfh_file_info[NUM_OF_ROM] = {0};
    GFH_CBR_INFO_v1 cbrVersionInfo[NUM_OF_ROM] = {0};

    if(rom != NULL && rom->buf)
    {
        offset =0;
        //Get GFH file info
        memcpy(&bootLoader_gfh_file_info[0], rom->buf, sizeof(GFH_FILE_INFO_v1));
        num_of_rom = 1;
        num_of_download_rom ++;

        //Get CBR version info from main ROM bin.
        for(i=0;i<10;i++)
        {
            memcpy(&cbrVersionInfo[0], rom->buf+offset, sizeof(GFH_CBR_INFO_v1));

            if(cbrVersionInfo[0].m_gfh_hdr.m_type == GFH_CBR_INFO)
            {
                break;
            }else{
                offset += cbrVersionInfo[0].m_gfh_hdr.m_size;
                if(i==9)
                {
                    return 900;
                }
            }
        }
    }
    if(secondRom != NULL && secondRom->buf)
    {
        //Get GFH file info
        memcpy(&bootLoader_gfh_file_info[1], secondRom->buf, sizeof(GFH_FILE_INFO_v1));
        num_of_rom = 2;
        num_of_download_rom ++;
    }
    if(dspRom != NULL && dspRom->buf)
    {
        //Get GFH file info
        memcpy(&bootLoader_gfh_file_info[2], dspRom->buf, sizeof(GFH_FILE_INFO_v1));
        num_of_rom = 3;
        num_of_download_rom ++;
    }
    if(demand_paging_rom != NULL && demand_paging_rom->buf)
    {
        //Get GFH file info
        memcpy(&bootLoader_gfh_file_info[3], demand_paging_rom->buf, sizeof(GFH_FILE_INFO_v1));
        num_of_rom = 4;
        num_of_download_rom ++;
    }


    //1. Format CBR Record( done at beginning)

    // 2. Download MAUI

    //Get Setting of CBR
    if( 0 != (ret = SV5_CMD_GetSettingOfCBR(com_handle, &startBlockOfCBR,&numNormalBlock, &numFaskBlock, &numSpareBlock,
        &numMaxRecordInBlock, &numTotalBlock, (unsigned char *)&bReadOnlyMode, &target_cbr_version)))
    {
        const unsigned int numMinSpareBlock = cbrVersionInfo[0].m_cbr_spare_block_num;
        const unsigned int numMaxSpareBlock = cbrVersionInfo[0].m_cbr_spare_block_num;
        unsigned int load_cbr_version = (cbrVersionInfo[0].m_gfh_hdr.m_magic_ver>>24);
        numNormalBlock = cbrVersionInfo[0].m_cbr_normal_block_num;
        numFaskBlock = 0;
        numMaxRecordInBlock = 16;
        bReadOnlyMode = 0;

        if(ret != S_DA_CBR_NOT_FOUND)
        {
            return ret;
        }


        if(0 == numNormalBlock)
        {
            // ex: MT6251 may not have CBR
            //LOG_WARNING("No CBR is created.");
        }
        else
        {
            if( 0 != (ret = SV5_CMD_CreateCBR(com_handle, numNormalBlock,
                            numFaskBlock, numMinSpareBlock,
                            numMaxSpareBlock, numMaxRecordInBlock,
                            bReadOnlyMode, load_cbr_version) ) )
            {
                ret = STATUS_CODE(ret, S_FT_DOWNLOAD_FAIL);
                return ret;
            }
        }

    }else{
        // CBR exists. Therefore, we should compare the setting
        // between the flash and Primary MAUI image.
        if((cbrVersionInfo[0].m_gfh_hdr.m_magic_ver>>24) != target_cbr_version)
        {
            return S_DA_CBR_VERSION_NOT_MATCHED;
        }

        if ((cbrVersionInfo[0].m_cbr_normal_block_num != numNormalBlock) ||
                        (cbrVersionInfo[0].m_cbr_spare_block_num != numSpareBlock))
        {
            return S_DA_CBR_COMPARE_FAILED;
        }
    }


    // 2. set memory block
    // send mem block command
    if (com_send_byte(com_handle, DA_NAND_IMAGE_LIST_CMD) != COM_STATUS_DONE)
    {
        return 902;
    }
    // send enable MBA feature
    if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
    {
        return 903;
    }
    if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
    {
        return 904;
    }

    // send enable FOTA feature
    if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
    {
        return 905;
    }


    //IMAGE_LIST_IMAGE_COUNT
    if (com_send_byte(com_handle, (unsigned char)num_of_download_rom) != COM_STATUS_DONE)
    {
        return 906;
    }
    //IMAGE_LIST_ENABLE_COUNT
    if (com_send_byte(com_handle, (unsigned char)num_of_download_rom) != COM_STATUS_DONE)
    {
        return 907;
    }

    for(i=0; i<num_of_rom; i++)
    {
        const unsigned int beginAddr = bootLoader_gfh_file_info[i].m_load_addr;
        const unsigned int endAddr = beginAddr + bootLoader_gfh_file_info[i].m_file_len - 1;
        const unsigned int binType = bootLoader_gfh_file_info[i].m_file_type;
        const unsigned int maxSize = bootLoader_gfh_file_info[i].m_max_size;
        const unsigned char enable = 1;
        const struct image *RomImage;
        switch(i)
        {
        case 0:
            RomImage = rom;
            break;
        case 1:
            RomImage = secondRom;
            break;
        case 2:
            RomImage = dspRom;
            break;
        case 3:
            RomImage = demand_paging_rom;
            break;
        }
        if(RomImage == NULL)
        {
            continue;
        }

        // send enable flag
        if (com_send_byte(com_handle, enable) != COM_STATUS_DONE)
        {
            return 908;
        }
        // send begin addr, high byte first
        if (com_send_dword(com_handle, beginAddr) != COM_STATUS_DONE)
        {
            return 909;
        }
        // send end addr, high byte first
        if (com_send_dword(com_handle, endAddr) != COM_STATUS_DONE)
        {
            return 910;
        }
        // send max size
        if (com_send_dword(com_handle, maxSize) != COM_STATUS_DONE)
        {
            return 911;
        }
        // send Type
        if (com_send_dword(com_handle, binType) != COM_STATUS_DONE)
        {
            return 912;
        }
        //read ack
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            return 913;
        }
        if (response != ACK)
        {
            return 914;
        }
    }

    // 3. download image: write to nand flash by memory block
    if (com_send_byte(com_handle, DA_NFB_WRITE_IMAGE_CMD) != COM_STATUS_DONE)
    {
        return 915;
    }

    // wait for command allowance check
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 916;
        }
    }

    // send packet length
    if (com_send_dword(com_handle, packet_len) != COM_STATUS_DONE)
    {
        return 917;
    }

    // if accuracy is ACCURACY_AUTO, auto calculate accuracy by baudrate
    //??

    // S1. wait for DA to read boot loader header
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 918;
        }
    }


    // S2. send all rom files

    //send ROM image
    for(i=0; i<num_of_rom;i++)
    {

        const struct image *RomImage;
        switch(i)
        {
        case 0:
            RomImage = rom;
            break;
        case 1:
            RomImage = secondRom;
            break;
        case 2:
            RomImage = dspRom;
            break;
        case 3:
            RomImage = demand_paging_rom;
            break;
        }
        if(RomImage == NULL)
        {
            continue;
        }

        num_bytes_sent = 0;
        while (num_bytes_sent < RomImage->len)
        {
            unsigned int j;
            unsigned short checksum;
            unsigned int num_bytes_to_send;
            unsigned short dummy_checksum = 0;

            checksum = 0;

            log_output(".");

            if (packet_len > (RomImage->len - num_bytes_sent))
            {
                num_bytes_to_send = RomImage->len - num_bytes_sent;
            }
            else
            {
                num_bytes_to_send = packet_len;
            }

            if (com_send_data(com_handle, RomImage->buf + num_bytes_sent,
                num_bytes_to_send) != COM_STATUS_DONE)
            {
                return 919;
            }

            // calculate checksum
            for(j=0; j<num_bytes_to_send; j++) {
                // WARNING: MUST make sure it unsigned value to do checksum
                checksum += RomImage->buf[num_bytes_sent+j];
            }

            if (com_send_word(com_handle, checksum) != COM_STATUS_DONE)
            {
                return 920;
            }

            while (1)
            {
                // TODO: error handling
                if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
                {
                    continue;
                }

                if (response == CONT_CHAR)
                {
                    break;
                }
                else if((response == NACK)||(response == STOP_CHAR))
                {
                    return 921;
                }
            }

            num_bytes_sent += num_bytes_to_send;
        }
    }
    log_output("\n");
    /* Get Flash Layout */
    if (com_recv_dword(com_handle, &regionCount) != COM_STATUS_DONE)
    {
        return 922;
    }
    for(i = 0; i< regionCount; i++)
    {
        if (com_recv_dword(com_handle, &binaryType) != COM_STATUS_DONE)
        {
            return 923;
        }
        if (com_recv_dword(com_handle, &startPage) != COM_STATUS_DONE)
        {
            return 924;
        }
        if (com_recv_dword(com_handle, &boundPage) != COM_STATUS_DONE)
        {
            return 925;
        }
        log_output("Region[%d] {Binary Type: 0x%X, Start Page: 0x%X, Bound Page: 0x%X}\n",
                        i, binaryType, startPage, boundPage);

    }

    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 926;
        }
    }

    for(i=0; i<num_of_rom;i++)
    {

        const struct image *RomImage;
        switch(i)
        {
        case 0:
            RomImage = rom;
            break;
        case 1:
            //RomImage = dspRom;
            RomImage = secondRom;
            break;
        case 2:
            RomImage = demand_paging_rom;
            break;
        }

        //calculate checksum
        m_chksum = 0;
        for(j=0; j<RomImage->len; j++) {
            m_chksum += RomImage->buf[j];
        }
        if (com_send_word(com_handle, (unsigned short)m_chksum) != COM_STATUS_DONE)
        {
            return 927;
        }
        while (1)
        {
            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                continue;
            }

            if (response == ACK)
            {
                break;
            }
            else
            {
                return 928;
            }
        }
    }
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 929;
        }
    }
    return S_DONE;
}

STATUS_E da_write_linux_images(COM_HANDLE com_handle,
                             struct image *linux_images,
                             const unsigned int num_linux_images,
                             const int eraseHB)
{
    // only support flash device: F_NAND_SEQUENTIAL, F_NAND_TTBL, F_NAND_FDM50
    STATUS_E ret = 0;
    DA_REPORT_T da_report;
    memset(&da_report, 0x0, sizeof(da_report));


    // RNDIS dongle use web-download to upate AP images via Linux update mechanisms.
    // But Hosted dongle can't use web-download.
    // We could treat hoted dongle behavior as traditonal dongle.
    // So, tool won't erase header block if user doesn't download bootloaders.
    if (eraseHB)
    {
        ret = SV5_CMD_BACKUP_AND_ERASE_HB(com_handle);
        if(ret)
        {
            log_output("Backup Header block failed!\n");
            return ret;
        }
    }


    {
        pt_resident new_part[PART_MAX_COUNT];
        pt_info pi;
        unsigned int new_nvram_start_address = 0x0;
        unsigned int new_nvram_length = 0x0;
        unsigned int old_nvram_start_address = 0x0; // old nvram start address
        unsigned int old_nvram_length = 0x0;        // old nvram length
        int i =0;
        memset(new_part, 0x0, sizeof(new_part));
        //Check Partitions and create partition info to new_part
        ret = SV5_FlashTool_CheckPartitionTable(com_handle,
                                                linux_images,
                                                num_linux_images,
                                                new_part,
                                                &pi);

        //-----------------
        // NVRAM Backup
        //-----------------
        for(i = 0; i < sizeof(new_part)/sizeof(new_part[0]); i++)
        {
            if(!strcmp((const char *)&(new_part[i].name[0]),"NVRAM"))
            {
                new_nvram_start_address = (unsigned int)new_part[i].offset;
                new_nvram_length = (unsigned int)new_part[i].size;
            }
        }

        if (S_DONE == ret)
        {
        }
        else if (S_PART_NO_VALID_TABLE == ret)
        {
            ret = SV5_CMD_WritePartitionInfo(com_handle,
                                            linux_images,
                                            num_linux_images,
                                            new_part,
                                            PART_MAX_COUNT,
                                            0);
            if(ret)
            {
                // Write partition info Failed.
                return ret;
            }
        }
        else if(S_FT_PMT_MISMATCH == ret) // Rewrite PartitionInfo
        {
            //Re-writeback the new partition table.
            pt_resident original_part[PART_MAX_COUNT];
            unsigned int original_part_num;
            unsigned int nvram_start_address = 0;
            unsigned int nvram_start_length = 0;
            unsigned int i =0;
            int found = 0;
            ret = SV5_CMD_ReadPartitionInfo(com_handle, original_part, &original_part_num, PART_MAX_COUNT);
            if(ret)
            {
                //Read partition info Failed.
                return ret;
            }

            // search NVRAM partition from target
            for(i = 0; i < original_part_num; i++)
            {
                if(!strcmp((const char *)&(original_part[i].name[0]),"NVRAM"))
                {
                    found = 1;
                    old_nvram_start_address = (unsigned int)original_part[i].offset;
                    old_nvram_length= (unsigned int)original_part[i].size;
                    break;
                }
            }
            if(!found)
            {
                //Cannot find \"NVRAM\" Partition on target, skip NVRAM backup
            }
            else
            {

                //Check if the NVRAM backup partition size matches the new configuarion
                if(old_nvram_length != new_nvram_length)
                {
                    //The size of new NVRAM partition (0x%x) doesn't match that of target (0x%x).
                    return S_FT_BACKUP_OF_NVRAM_PARTITION_MISMATCH;
                }

                // readback NVRAM for backup
                for(i = 0; i < num_linux_images; i++)
                {
                    if(!strcmp(linux_images[i].name,"NVRAM"))
                    {
                        linux_images[i].buf = malloc(old_nvram_length);
                        ret = da_NAND_ReadBlock(com_handle,
                                                old_nvram_start_address,
                                                old_nvram_length,
                                                NUTL_LOGICAL_BMT_READ_PAGE_ONLY,
                                                linux_images[i].buf,
                                                &linux_images[i].len);
                        linux_images[i].type = RAW;
                        break;
                    }
                }
                // the NVRAM buf will be released by release_images function
                log_output("Readback finished\n", ret);
                if(ret)
                {
                    // Write partition info Failed.
                    return ret;
                }

            }

            ret = SV5_CMD_WritePartitionInfo(com_handle,
                                            linux_images,
                                            num_linux_images,
                                            new_part,
                                            PART_MAX_COUNT,
                                            0);
            if(ret)
            {
                // Write partition info Failed.
                return ret;
            }
        }
        else
        {
            //Read partition info Failed.
            return ret;
        }


        if (0 != (ret = SV5_CMD_ReadBMT(com_handle, &da_report)))
        {
            // ReadBMT fail. Continue next step
        }
        // download partition image: write to nand flash by memory block
        // set raw partition packet length : 4096
        if(0 != (ret=SV5_CMD_NAND_DL_Partition_Data(com_handle,
                                                 linux_images,
                                                 num_linux_images,
                                                 4096))
          )
        {
            //Download nand partition data failed
            return ret;
        }
        // Write partition table again
        if (0 != (ret = SV5_CMD_WritePartitionInfo(com_handle,
                                            linux_images,
                                            num_linux_images,
                                            new_part,
                                            PART_MAX_COUNT,
                                            1)))
        {
            // Write partition info Failed.
            return ret;
        }
        if (0 != (ret = SV5_CMD_ReadBMT(com_handle, &da_report)))
        {
            // ReadBMT fail. Continue next step
        }
    }

// RNDIS dongle use web-download to upate AP images via Linux update mechanisms.
// But Hosted dongle can't use web-download.
// We could treat hoted dongle behavior as traditonal dongle.
// So, tool won't erase header block if user doesn't download bootloaders.
    if (eraseHB)
    {
        ret = SV5_CMD_RESTORE_HB(com_handle);
        if(ret)
        {
            log_output("Restore Header block failed!\n");
            return ret;
        }
    }//end of download Bootloader if


    {
        unsigned int i = 0;
        for(i = 0; i < num_linux_images; i++)
        {
            if(!strcmp(linux_images[i].name,"SecurePartition"))
            {
                ret = SV5_CMD_NAND_DL_Encrypt_Partition_Data(com_handle,
                                        &linux_images[i]);
                if(ret)
                {
                    log_output("Download encrypted data failed!\n");
                    return ret;
                }
                break;
            }
        }
    }


    return ret;
}

STATUS_E SendDataWithRetransmission(COM_HANDLE com_handle,
                                    const char *data,
                                    unsigned int dataLength,
                                    unsigned int packetLength,
                                    unsigned int sendChecksum)
{
    const unsigned int numPackets = ((dataLength - 1) / packetLength) + 1;
    unsigned int numPacketsSent = 0;

    while (numPacketsSent < numPackets)
    {
        int isLastPacket;
        const unsigned int startOffset = numPacketsSent * packetLength;
        unsigned int currentPacketLength;
        unsigned short checksum = 0;
        unsigned int i;
        unsigned char response;

        if(numPacketsSent == (numPackets - 1))
        {
            isLastPacket = 1;
        }else{
            isLastPacket = 0;
        }

        if(isLastPacket){
            currentPacketLength = dataLength - startOffset;
        }else{
            currentPacketLength = packetLength;
        }

        if (sendChecksum)
        {
            for (i=0; i<currentPacketLength; ++i)
            {
                checksum += (unsigned char) data[startOffset+i];
            }
        }

        while (1)
        {
            if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
            {
                return S_FT_DOWNLOAD_FAIL;
            }

            if (com_send_data(com_handle, data + startOffset,
                currentPacketLength) != COM_STATUS_DONE)
            {
                return S_FT_DOWNLOAD_FAIL;
            }

            if (isLastPacket)
            {
                // Send padding
                for (i=0; i<packetLength-currentPacketLength; ++i)
                {
                    if (com_send_byte(com_handle, 0) != 0)
                    {
                        return 3;
                    }
                }
            }

            if (sendChecksum)
            {
                if (com_send_word(com_handle, checksum) != 0)
                {
                    return 4;
                }
            }

            //
            // FIXME (This is only a workaround)
            //
            //   The first attempt to read the CONT_CHAR may not succeed,
            //   so we set a larger timeout value to let ReadData8() keep
            //   trying.
            //
            while (1)
            {
                // TODO: error handling
                if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
                {
                    continue;
                }else{
                    break;
                }
            }

            if (response == CONT_CHAR)
            {
                break;
            }

            if (response == NACK)
            {
            /*unsigned int errorCode;

              if (ReadData32(com.GetHandle(), errorCode) != 0)
              {
              return 6;
              }

                MTRACE_ERR(g_hBROM_DEBUG,
                "DA_cmd::SendDataWithRetransmission(): \"%s\"",
                StatusToString(errorCode));

                  if (errorCode == S_DA_NAND_EXCEED_CONTAINER_LIMIT)
                  {
                  return errorCode;
                  }
                  else if (errorCode == S_DA_NAND_REACH_END_OF_FLASH)
                  {
                  return errorCode;
            }*/
                return 0;
            }

            //!! target forcely stop
            if (response == STOP_CHAR)
            {
                //            MTRACE_WARN(g_hBROM_DEBUG, "DA_cmd::SendDataWithRetransmission(): STOP!");

                return 0;
            }
        }

        ++numPacketsSent;
    }

    return S_DONE;
}

STATUS_E da_NAND_WriteBlock(COM_HANDLE com_handle,
                            unsigned int blockIndex,
                            unsigned int pageSize,
                            unsigned int spareSize,
                            unsigned int numPagesPerBlock,
                            const unsigned char *pageBuffer,
                            const unsigned char *spareBuffer,
                            unsigned short checksum)
{
    unsigned char response = 0;

    //unsigned int packetLength = pageSize + spareSize;
    //unsigned int numPagesPerPacket = 1;
    const unsigned int packetLength = 16896;
    unsigned int numPagesPerPacket;

    unsigned char *packetBuffer;
    unsigned int i, j;
    STATUS_E status;

    unsigned short checksumFromDA = 0;

    log_output("Write Block num = %u\n", blockIndex);

    if (com_send_byte(com_handle, DA_NAND_WRITE_BLOCK) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, blockIndex) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, pageSize) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, spareSize) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, numPagesPerBlock) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {

            break;
        }
        else
        {
            unsigned int errorCode;
            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
            }
            return errorCode;
        }
    }

    //
    // Send page data and spare data
    //
    if (pageSize == 512)
    {
        //numPagesPerPacket = 8;
        //packetLength *= numPagesPerPacket;

        // (512 + 16) * 32 == 16896
        numPagesPerPacket = 32;
    }
    else if (pageSize == 2048)
    {
        //numPagesPerPacket = 2;
        //packetLength *= numPagesPerPacket;

        // (2048 + 64) * 8 == 16896
        numPagesPerPacket = 8;
    }else
    {
        //MTRACE_ERR(g_hBROM_DEBUG, "DA_cmd::CMD_NAND_WriteBlock(): "
        //           "Unsupported page size: %u", pageSize);
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    packetBuffer = malloc(packetLength);
    for (i=0; i<numPagesPerBlock; i+=numPagesPerPacket)
    {
        for (j=0; j<numPagesPerPacket; ++j)
        {
            memcpy(packetBuffer+(j*(pageSize+spareSize)),
                pageBuffer+(i * pageSize + j * pageSize),
                pageSize
                );
            memcpy(packetBuffer+(pageSize + j*(pageSize+spareSize)),
                spareBuffer+(i * spareSize + j * spareSize),
                spareSize
                );
        }

        status = SendDataWithRetransmission(
            com_handle, packetBuffer,
            packetLength, packetLength,0);

        if (status == S_DONE)
        {
            continue;
        }

        if (status == S_DA_NAND_BAD_BLOCK)
        {
            free(packetBuffer);
            return status;
        }

        free(packetBuffer);
        return 9;
    }

    //
    // Compare checksum for page data
    // (TODO: spare data)
    //
    while (1)
    {
        // TODO: error handling
        if (com_recv_word(com_handle, &checksumFromDA) != COM_STATUS_DONE)
        {
            continue;
        }else{
            break;
        }
    }

    if (checksum != checksumFromDA)
    {
        //MTRACE_ERR(g_hBROM_DEBUG, "DA_cmd::CMD_NAND_WriteBlock(): "
        // "Checksum mismatch => PC(0x%04X) DA(0x%04X)",
        //          checksum, checksumFromDA);
        return 12;
    }

    free(packetBuffer);
    return S_DONE;
}

STATUS_E da_NAND_WritePagesWithinBlock(COM_HANDLE com_handle,
                                       unsigned int blockIndex,
                                       unsigned int nonemptyPageIndicesSize,
                                       unsigned int *nonemptyPageIndicesPageIndex,
                                       unsigned int pageSize,
                                       unsigned int spareSize,
                                       unsigned int numPagesPerBlock,
                                       const unsigned char *pageBuffer,
                                       const unsigned char *spareBuffer,
                                       unsigned short checksum)
{
    unsigned char response = 0;

    unsigned int packetLength = 16896;
    unsigned int numPagesPerPacket;

    unsigned char *packetBuffer;
    unsigned int i;
    STATUS_E status;

    unsigned short checksumFromDA = 0;

    log_output("Write Block num = %u (WritePagesWithinBlock)\n", blockIndex);

    if (com_send_byte(com_handle, DA_NAND_WRITE_PAGES_WITHIN_BLOCK) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, blockIndex) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, nonemptyPageIndicesSize) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, pageSize) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, spareSize) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    if (com_send_dword(com_handle, numPagesPerBlock) != COM_STATUS_DONE)
    {
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {

            break;
        }
        else
        {
            unsigned int errorCode;
            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
            }
            return errorCode;
        }
    }

    //
    // Send page data and spare data
    //
    if (pageSize == 512)
    {
        //numPagesPerPacket = 8;
        //packetLength *= numPagesPerPacket;

        // (512 + 16) * 32 == 16896
        numPagesPerPacket = 32;
    }
    else if (pageSize == 2048)
    {
        //numPagesPerPacket = 2;
        //packetLength *= numPagesPerPacket;

        // (2048 + 64) * 8 == 16896
        numPagesPerPacket = 8;
    }else
    {
        //MTRACE_ERR(g_hBROM_DEBUG, "DA_cmd::CMD_NAND_WriteBlock(): "
        //           "Unsupported page size: %u", pageSize);
        return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
    }

    packetLength = pageSize + spareSize;
    numPagesPerPacket = 1;

    packetBuffer = malloc(packetLength);
    //for (i=0; i<numPagesPerBlock; i+=numPagesPerPacket)
    for (i=0; i<nonemptyPageIndicesSize; i++)
    {
        if (com_send_dword(com_handle, nonemptyPageIndicesPageIndex[i]) != COM_STATUS_DONE)
        {
            return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
        }

        //for (j=0; j<numPagesPerPacket; ++j)
        {
            memcpy(packetBuffer,
                pageBuffer+(nonemptyPageIndicesPageIndex[i] * pageSize),
                pageSize
                );
            memcpy(packetBuffer+pageSize,
                spareBuffer+(nonemptyPageIndicesPageIndex[i] * spareSize),
                spareSize
                );
        }

        status = SendDataWithRetransmission(
            com_handle, packetBuffer,
            packetLength, packetLength,0);

        if (status == S_DA_NAND_BAD_BLOCK)
        {
            free(packetBuffer);
            return status;
        }

        //
        // Compare checksum for page data
        // (TODO: spare data)
        //
        while (1)
        {
            // TODO: error handling
            if (com_recv_word(com_handle, &checksumFromDA) != COM_STATUS_DONE)
            {
                continue;
            }else{
                break;
            }
        }

        /*if (checksum != checksumFromDA)
        {
        //MTRACE_ERR(g_hBROM_DEBUG, "DA_cmd::CMD_NAND_WriteBlock(): "
        // "Checksum mismatch => PC(0x%04X) DA(0x%04X)",
        //          checksum, checksumFromDA);
        return 12;
    }*/

        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return S_FT_SET_DOWNLOAD_BLOCK_FAIL;
        }

        if (status == S_DONE)
        {
            continue;
        }


        free(packetBuffer);
        return 9;
    }


    free(packetBuffer);
    return S_DONE;
}

STATUS_E da_NAND_ReadBlock(COM_HANDLE com_handle,
                           unsigned int startAddr,
                           unsigned int blockSize,
                           NUTL_ReadFlag_E flag,
                           unsigned char *Buffer,
                           unsigned int *buf_len)
{
    unsigned char val;
    unsigned int total_read_bytes;
    unsigned int current_read_bytes;
    const unsigned int    max_pagesize=2048;
    const unsigned int    max_pagesparesize=2112;
    unsigned int page_size, spare_size;
    unsigned int    packet_length=0;
    unsigned int unit_of_page_read_bytes;
    const unsigned int m_packet_length = 4096;

    //unsigned char *buf;
    unsigned char response;

    unsigned int    pages_of_packet;
    unsigned int    retry_count;

    // limit packet_length to max_pagesparesize as minmimum value
    if(m_packet_length < max_pagesparesize ) {
        packet_length = max_pagesparesize;        // max page+spare size
    }
    else {
        packet_length = max_pagesparesize*(m_packet_length/max_pagesize);    // page+spare alignment
    }

    // check if read addr is pagesize alignment
    if( 0 != (startAddr%max_pagesize) ) {
        return S_FT_NAND_READADDR_NOT_PAGE_ALIGNMENT;
    }

    // check if read length is pagesize alignment
    if( 0 != (blockSize%max_pagesize) ) {
        return S_FT_NAND_READLEN_NOT_PAGE_ALIGNMENT;
    }

    // send NAND read page commnad and readflag
    if (com_send_byte(com_handle, DA_NAND_READPAGE_CMD) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }
    // send readback FAT flag
    val = flag;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }

    // send readback address
    if (com_send_dword(com_handle, startAddr) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }
    // send readback length
    if (com_send_dword(com_handle, blockSize) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }
    // Send container length
    if (com_send_dword(com_handle, 0) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }

    // wait for addr and len check
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return S_DA_NAND_PAGE_READ_FAILED;
        }
    }

    // get Unit of Page Read
    if (com_recv_dword(com_handle, &page_size))
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }
    if (com_recv_dword(com_handle, &spare_size))
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }

    // get Unit of Page Read
    if (com_recv_dword(com_handle, &unit_of_page_read_bytes))
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }

    // send pages_per_packet
    pages_of_packet = packet_length/(page_size+spare_size);
    if (com_send_dword(com_handle, pages_of_packet) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }

    // get total read back length
    if (com_recv_dword(com_handle, &total_read_bytes) != COM_STATUS_DONE)
    {
        return S_DA_NAND_PAGE_READ_FAILED;
    }
    *buf_len = total_read_bytes;
    current_read_bytes = 0;
    retry_count = 10;
    while( current_read_bytes < total_read_bytes ) {
        unsigned short    buf_checksum;
        unsigned short    packet_checksum;
        unsigned int    j;
        unsigned int    packet_bytes = 0;
        unsigned int    re_transmission = 0;
        //unsigned int result;

        // read data
        for(j=0; j<1000;j++)
        {
            //if (com_recv_data(com_handle, Buffer+(current_read_bytes), pages_of_packet * unit_of_page_read_bytes) == COM_STATUS_DONE)
            if (com_recv_data_chk_len(com_handle, Buffer+(current_read_bytes), pages_of_packet * unit_of_page_read_bytes) == COM_STATUS_DONE)
            {
                break;
            }else{

            }
        }
        //result = com_recv_data(com_handle, Buffer+(current_read_bytes), pages_of_packet * unit_of_page_read_bytes);

        if(re_transmission==1){
            if(retry_count==0)
            {
                com_send_byte(com_handle, NACK);
                return S_DA_NAND_PAGE_READ_FAILED;
            }else{
                if (com_send_byte(com_handle, CONT_CHAR) != COM_STATUS_DONE)
                {
                    return S_DA_NAND_PAGE_READ_FAILED;
                }
                retry_count--;
            }
            continue;
        }


        packet_bytes += (pages_of_packet * unit_of_page_read_bytes);

        // read chksum
        if(com_recv_word(com_handle, &packet_checksum) != COM_STATUS_DONE)
        {
            return S_DA_NAND_PAGE_READ_FAILED;
        }

        // if empty packet, jump to sending ack step
        if( 0 == packet_bytes ) {
            // send ACK
            if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
            {
                return S_DA_NAND_PAGE_READ_FAILED;
            }
            continue;
        }

        // calculate buffer checksum
        buf_checksum = 0;
        for(j=0; j<packet_bytes; j++) {
            // WARNING: MUST make sure it unsigned value to do checksum
            buf_checksum += Buffer[current_read_bytes+j];
        }

        // compare checksum
        if( buf_checksum != packet_checksum ) {
            // send ACK
            if(retry_count==0)
            {
                com_send_byte(com_handle, NACK);
                return S_DA_NAND_PAGE_READ_FAILED;
            }else{
                if (com_send_byte(com_handle, CONT_CHAR) != COM_STATUS_DONE)
                {
                    return S_DA_NAND_PAGE_READ_FAILED;
                }
                retry_count--;
                continue;
            }
            return S_DA_NAND_PAGE_READ_FAILED;
        }
        current_read_bytes += packet_bytes;

        // send ACK
        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return S_DA_NAND_PAGE_READ_FAILED;
        }

    }

    return S_DONE;
}

STATUS_E da_FormatFAT(COM_HANDLE com_handle,
                        NUTL_EraseFlag_E  flag,
                        unsigned int  bValidation,
                         Range *nor_fat_range,
                         Range *nand_fat_range)
{
    unsigned char response = 0;
    unsigned char val;
    unsigned int status;
    unsigned int format_addr,format_length;
    Range norFATRange, nandFATRange, emmcFATRange;

    if (com_send_byte(com_handle, DA_FORMAT_FAT_CMD) != COM_STATUS_DONE)
    {
        return 390;
    }
    val = (unsigned char)flag;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return 391;
    }
    val = (unsigned char)bValidation;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return 392;
    }

    //
    // Wait for response
    //
    if (com_recv_dword(com_handle, &status) != COM_STATUS_DONE)
    {
        return 393;
    }
    if (status != S_DONE)
    {
        return status;
    }

    if (com_recv_dword(com_handle, &norFATRange.m_start_address) != COM_STATUS_DONE)
    {
        return 394;
    }
    if (com_recv_dword(com_handle, &norFATRange.m_length) != COM_STATUS_DONE)
    {
        return 395;
    }
    if (com_recv_dword(com_handle, &nandFATRange.m_start_address) != COM_STATUS_DONE)
    {
        return 396;
    }
    if (com_recv_dword(com_handle, &nandFATRange.m_length) != COM_STATUS_DONE)
    {
        return 397;
    }
    if (com_recv_dword(com_handle, &emmcFATRange.m_start_address) != COM_STATUS_DONE)
    {
        return 398;
    }
    if (com_recv_dword(com_handle, &emmcFATRange.m_length) != COM_STATUS_DONE)
    {
        return 399;
    }

    if (norFATRange.m_length != 0)
    {
        //type = HW_STORAGE_NOR;
        format_addr            = norFATRange.m_start_address;
        format_length       = norFATRange.m_length;

    }

    if(nandFATRange.m_length != 0)
    {
        //type = HW_STORAGE_NAND;
        format_addr            = nandFATRange.m_start_address;
        format_length       = nandFATRange.m_length;
    }

    if(emmcFATRange.m_length != 0)
    {
        //type = HW_STORAGE_NAND;
        format_addr            = emmcFATRange.m_start_address;
        format_length       = emmcFATRange.m_length;
    }


    // wait for command allowance check
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 400;
        }
    }
    // range checking
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 401;
        }
    }
    // wait til format is done
    while (1)
    {
        STATUS_E da_ret;
        unsigned char return_progress = 0;
        unsigned int addr32;

        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return 402;
        }

        // check report
        if (da_ret == S_DA_IN_PROGRESS || S_DONE==da_ret)
        {
            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                return 403;
            }
        }else{
            // error, read fail address
            if (com_recv_dword(com_handle, &addr32) != COM_STATUS_DONE)
            {
                return 404;
            }
            // get continue flag
            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                return 405;
            }
            if (CONT_CHAR == response)
            {
                continue;
            }else{
                return 406;
            }
        }

        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return 407;
        }

        // 100%, format is done
        if (S_DONE == da_ret)
        {
            break;
        }
    }

    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            return 408;
        }
    }

    return S_DONE;
}

STATUS_E da_FormatFlash(COM_HANDLE com_handle,
                        HW_StorageType_E  type,
                        NUTL_EraseFlag_E  flag,
                        unsigned int  bValidation,
                        unsigned int address,
                        unsigned int length)
{
    unsigned char response = 0;
    unsigned char val;

    if (com_send_byte(com_handle, DA_FORMAT_CMD) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }
    val = type;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }
    val = flag;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }
    val = bValidation;
    if (com_send_byte(com_handle, val) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }

    // send erase address
    if (com_send_dword(com_handle, address) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }
    // send erase length
    if (com_send_dword(com_handle, length) != COM_STATUS_DONE)
    {
        return S_DA_NAND_ERASE_FAILED;
    }

    // wait for command allowance check
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            unsigned int errorCode;
            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            return errorCode;
        }
    }
    // range checking
    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            unsigned int errorCode;
            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            return errorCode;
        }
    }

    // wait til format is done
    // read report from DA
    while (1)
    {
        STATUS_E da_ret;
        unsigned char return_progress = 0;
        unsigned int addr32;

        if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
        {
            return S_DA_NAND_ERASE_FAILED;
        }

        // check report
        if (da_ret == S_DA_IN_PROGRESS || S_DONE==da_ret)
        {
            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
        }else{
            // error, read fail address
            if (com_recv_dword(com_handle, &addr32) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            // get continue flag
            if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            if (CONT_CHAR == response)
            {
                continue;
            }else{
                return S_DA_NAND_ERASE_FAILED;
            }
        }

        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return S_DA_NAND_ERASE_FAILED;
        }

        // 100%, format is done
        if (S_DONE == da_ret)
        {
            break;
        }
    }

    while (1)
    {
        if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        {
            continue;
        }

        if (response == ACK)
        {
            break;
        }
        else
        {
            unsigned int fmt_begin_addr = 0;
            unsigned int fmt_length = 0;
            unsigned int total_blocks = 0;
            unsigned int bad_blocks = 0;
            unsigned int err_blocks = 0;
            if (com_recv_dword(com_handle, &fmt_begin_addr) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            if (com_recv_dword(com_handle, &fmt_length) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            if (com_recv_dword(com_handle, &total_blocks) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            if (com_recv_dword(com_handle, &bad_blocks) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            if (com_recv_dword(com_handle, &err_blocks) != COM_STATUS_DONE)
            {
                return S_DA_NAND_ERASE_FAILED;
            }
            log_output("FORMAT: begin address(0x%X) length(0x%X) total blocks(%d) bad blocks(%d) err blocks(%d)\n",
                        fmt_begin_addr,
                        fmt_length,
                        total_blocks,
                        bad_blocks,
                        err_blocks);
            break;
        }
    }

    return S_DONE;
}
STATUS_E SV5_CMD_FormatCBR(COM_HANDLE com_handle)
{
    unsigned char response;

    if (com_send_byte(com_handle, DA_FORMAT_CBR) != COM_STATUS_DONE)
    {
        return 380;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 381;
    }
    if (response != ACK)
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 382;
        }
        //Format CBR error!!!
        return errorCode;
    }
    return S_DONE;
}

STATUS_E SV5_CMD_BACKUP_AND_ERASE_HB(COM_HANDLE com_handle)
{
    unsigned char status;

    // Send command
    if (com_send_byte(com_handle, DA_BACKUP_AND_ERASE_HB) != COM_STATUS_DONE)
    {
        return 960;
    }

    if (com_recv_byte(com_handle, &status) != COM_STATUS_DONE)
    {
        return 961;
    }
    if (status != ACK)
    {
        return 962;
    }
    // Start to backup and erase header block

    if (com_recv_byte(com_handle, &status) != COM_STATUS_DONE)
    {
        return 963;
    }
    if (status != ACK)
    {
        return 964;
    }
    // Done to backup and erase header block

    return S_DONE;
}

STATUS_E SV5_CMD_RESTORE_HB(COM_HANDLE com_handle)
{
    unsigned char status;

    // Send command
    if (com_send_byte(com_handle, DA_RESTORE_HB) != COM_STATUS_DONE)
    {
        return 970;
    }

    if (com_recv_byte(com_handle, &status) != COM_STATUS_DONE)
    {
        return 971;
    }
    if (status != ACK)
    {
        return 972;
    }
    // Start to restore header block

    if (com_recv_byte(com_handle, &status) != COM_STATUS_DONE)
    {
        return 973;
    }
    if (status != ACK)
    {
        return 974;
    }
    // Done to restore header block

    return S_DONE;
}


STATUS_E SV5_CMD_ReadBMT(COM_HANDLE com_handle, DA_REPORT_T *p_da_report)
{
    bmt_entry* p_bmt_entry = NULL;
    unsigned char bmt_entry_num = 0;
    unsigned char response = 0;
    int idx = 0;
    unsigned int block_size = p_da_report->m_nand_pagesize * p_da_report->m_nand_pages_per_block;

    // send DA_READ_BMT
    if (com_send_byte(com_handle, DA_READ_BMT) != COM_STATUS_DONE)
        return 320;

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
        return 321;

    if (ACK != response)
        return 322;
    if (com_recv_byte(com_handle, &bmt_entry_num) != COM_STATUS_DONE)
        return 323;

    p_bmt_entry = (bmt_entry*)malloc(sizeof(bmt_entry)*bmt_entry_num);
    memset(p_bmt_entry, 0, sizeof(bmt_entry)*bmt_entry_num);

    for (idx = 0; idx < bmt_entry_num; idx++)
    {
        if (com_recv_word(com_handle, &p_bmt_entry[idx].bad_index) != COM_STATUS_DONE)
        {
            free(p_bmt_entry);
            return 324;
        }
        if (com_recv_word(com_handle, &p_bmt_entry[idx].mapped_index) != COM_STATUS_DONE)
        {
            free(p_bmt_entry);
            return 325;
        }
    }
    if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
    {
        free(p_bmt_entry);
        return 326;
    }
    log_output("Read BMT: %02d entries.\n", bmt_entry_num);
    for (idx = 0; idx < bmt_entry_num ;idx++)
    {
        log_output("Read BMT(%02d): %02X -> %02X, %08X -> %08X\n",
                    idx,
                    p_bmt_entry[idx].bad_index,
                    p_bmt_entry[idx].mapped_index,
                    p_bmt_entry[idx].bad_index*block_size,
                    p_bmt_entry[idx].mapped_index*block_size);
    }
    return 0;
}

STATUS_E SV5_CMD_NAND_DL_Partition_Data(COM_HANDLE com_handle,
                                    const struct image *linux_images,
                                    const unsigned int num_linux_images,
                                    unsigned int packetLength)
{
    // the load and target's flash must support 2K packet size
    const unsigned int yaffs_packetLength = 2048+64;
    unsigned int index = 0;
    unsigned int total_bytes = 0;
    unsigned char response = 0;
    log_output("packetlength=%u , yaffs_packetLength=%u\n",
            packetLength,
            yaffs_packetLength);

    //Calculate the total size of all the image
    for(index = 0; index < num_linux_images; index++)
    {
        if(linux_images[index].type == RESERVED ||
            (strcmp(linux_images[index].name, "SecurePartition") == 0))
        {
            continue;
        }
        else
        {
            total_bytes += linux_images[index].len;
        }
    }

    //DA_DL_LINUX_RAW_DATA
    for(index = 0; index < num_linux_images; index++)
    {
        unsigned int partition_size = 0;
        unsigned int send_packet = packetLength; // packetLength is just used to store respected packet length

        //1 Send DA command by partition type
        if(linux_images[index].type == RESERVED )
        {
            log_output("Pass DL Partition %s type(0x%x)\n", linux_images[index].name, linux_images[index].type);
            continue;
        }
        else if(linux_images[index].type == RAW)
        {
            if( strcmp(linux_images[index].name, "SecurePartition") == 0)
            {
                // SV5_CMD_NAND_DL_Encrypt_Partition_Data(hCOM, rom);
                // Do nothing
                continue;
            }

            if (com_send_byte(com_handle, DA_DL_LINUX_RAW_DATA) != COM_STATUS_DONE)
            {
                return 330;
            }
        }
        else if(linux_images[index].type == YAFFS)
        {
            send_packet = yaffs_packetLength;

            if (com_send_byte(com_handle, DA_DL_LINUX_YAFFS_DATA) != COM_STATUS_DONE)
            {
                return 331;
            }
        }
        else
        {
            return 332;
            //Return error of using unsupported type
        }

        if(index < num_linux_images-1)
        {
            //the partition size of the last one will be 0
            partition_size = linux_images[index+1].load_addr - linux_images[index].load_addr;
        }

        log_output("---Write Partition [%s] type(0x%x)---\n",linux_images[index].name, linux_images[index].type);
        log_output("Partition Address 0x%X\n",linux_images[index].load_addr);
        log_output("Partition Size 0x%X\n",partition_size);
        log_output("Image Size 0x%X(%d)\n",linux_images[index].len,linux_images[index].len);
        log_output("Packet Length 0x%X(%d)\n",send_packet,send_packet);

        // Apply same protocol to DA_DL_LINUX_RAW_DATA and DA_DL_LINUX_YAFFS_DATA

        if (com_send_dword(com_handle, linux_images[index].load_addr) != COM_STATUS_DONE)
        {
            return 333;
        }

        if (com_send_dword(com_handle, partition_size) != COM_STATUS_DONE)
        {
            return 334;
        }

        if (com_send_dword(com_handle, linux_images[index].len) != COM_STATUS_DONE)
        {
            return 335;
        }

        // for yaffs: 2048+64 per packet

        if (com_send_dword(com_handle, send_packet) != COM_STATUS_DONE)
        {
            return 336;
        }

        if (com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
        {
            return 337;
        }

        if (response != ACK)
        {
            unsigned int errorCode;

            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return 338;
            }
            return errorCode;
        }
        //2 send image to DA
        {
            const unsigned char *imageBuf = linux_images[index].buf;
            const unsigned int imageSize = linux_images[index].len;
            unsigned int sent_bytes = 0;
            unsigned int retry_count = 0;

            unsigned char buf[5];
            unsigned int packet_index = 0;
            unsigned int target_bytes = imageSize;
            unsigned int target_sent_bytes = 0;

            STATUS_E    da_ret;
            int ret;
           // send each rom file
            sent_bytes = 0;
            retry_count = 0;
            packet_index = 0;
            log_output("[%s]:%lu bytes, target_sent_bytes=%lu/%lu.\n",
                linux_images[index].name,
                imageSize,
                target_sent_bytes,
                target_bytes);
            while( sent_bytes < imageSize )
            {
                unsigned int j;
                unsigned short checksum;
                unsigned int frame_bytes;
re_transmission:
                // reset the frame checksum
                checksum = 0;
                // if the last frame is less then PACKET_LENGTH bytes
                if( send_packet > (imageSize-sent_bytes) )
                {
                    frame_bytes = imageSize - sent_bytes;
                }
                else
                {
                    // the normal frame
                    frame_bytes = send_packet;
                }
                // send frame
                if(com_send_data(com_handle, imageBuf+sent_bytes, frame_bytes) != COM_STATUS_DONE)
                {
                    goto read_cont_char;
                }
                // calculate checksum
                for(j=0; j<frame_bytes; j++)
                {
                    // WARNING: MUST make sure it unsigned value to do checksum
                    checksum += imageBuf[sent_bytes+j];
                }
                // send 2 bytes checksum, high byte first
                buf[0] = (unsigned char)((checksum>> 8)&0x000000FF);
                buf[1] = (unsigned char)((checksum)    &0x000000FF);
                  if(com_send_data(com_handle, buf, 2) != COM_STATUS_DONE)
                {
                    goto read_cont_char;
                }
read_cont_char:

                // read CONT_CHAR
                   if(com_recv_data_chk_len(com_handle, &buf[0], 1) != COM_STATUS_DONE)
                {
                    return 339;
                }

                if( CONT_CHAR == buf[0] )
                {
                    // sent ok!, reset retry_count
                    retry_count = 0;
                }
                else
                {
                    // get error code
                    if (com_recv_dword(com_handle, &da_ret) != COM_STATUS_DONE)
                    {
                        return 340;
                    }
                    switch(da_ret)
                    {
                    case S_DA_UART_RX_BUF_FULL:
                        // target RX buffer is full, add delay to wait for flash erase done
                        //Sleep(DA_FLASH_ERASE_WAITING_TIME);
                    case S_DA_UART_DATA_CKSUM_ERROR:
                    case S_DA_UART_GET_DATA_TIMEOUT:
                    case S_DA_UART_GET_CHKSUM_LSB_TIMEOUT:
                    case S_DA_UART_GET_CHKSUM_MSB_TIMEOUT:
                        // check retry times
                        if( PACKET_RE_TRANSMISSION_TIMES > retry_count ) {
                            retry_count++;
                            log_output("PKT[%lu]:Retry(%u):(%d) received, start to re-transmit.\n",
                                    packet_index, retry_count, da_ret);
                        }
                        else {
                            // fail to re-transmission
                            log_output("PKT[%lu]: Retry(%u): stop to re-transmit! retry %u times fail!\n",
                                        packet_index, retry_count, retry_count);
                            // send NACK to wakeup DA to stop
                               if(com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
                            {
                                return 341;
                            }
                            return 342;
                        }

                        // wait for DA clean RX buffer
                        log_output("PKT[%lu]: Retry(%u): wait for DA clean it's RX buffer.\n", packet_index, retry_count);
                        if( COM_STATUS_DONE != (ret=com_recv_data_chk_len(com_handle, buf, 1)) )
                        {
                            log_output("PKT[%lu]: ReadData(): fail, Err(%d)\n", packet_index, ret);
                            return 343;
                        }
                        if( ACK != buf[0] )
                        {
                            log_output("PKT[%lu]: Retry(%u): wrong ack(0x%02X) return!\n", packet_index, retry_count, buf[0]);
                            return 344;
                        }

                        // send CONT_CHAR to wakeup DA to start recieving again
                        log_output("PKT[%lu]: Retry(%u): send CONT_CHAR to wakeup DA to start recieving again.\n", packet_index, retry_count);
                        if(com_send_byte(com_handle, CONT_CHAR) != COM_STATUS_DONE)
                        {
                            return 345;
                        }

                        // re-transmission this frame
                        log_output("PKT[%lu]: Retry(%u): re-transmission this frame, offset(%lu).\n",
                            packet_index,
                            retry_count,
                            sent_bytes);
                        goto re_transmission;
                        break;

                    default:
                        // flash erase timeout abort transmission
                        log_output("PKT[%lu]: (%d), abort transmission!\n",
                            packet_index,
                            da_ret);
                        return da_ret;
                    }
                }

                // update progress state
                sent_bytes += frame_bytes;
                target_sent_bytes += frame_bytes;
                packet_index++;
            }
        }
        //3 checksum
        {
            uint8 data;
            uint16 page_size;
            uint16 spare_size;
            //Use different checksum for YAFFS or RAW
            unsigned short pc_checksum = 0;
            unsigned short target_checksum = 0;

            if (com_recv_data_chk_len(com_handle, &data, 1) != COM_STATUS_DONE)
            {
                return 346;
            }
            if(data != S_DONE)
            {
                return 347;
            }

            // read page size and spare size from target
            if (com_recv_word(com_handle, &page_size) != COM_STATUS_DONE)
            {
                return 348;
            }
            if (com_recv_word(com_handle, &spare_size) != COM_STATUS_DONE)
            {
                return 349;
            }
            log_output("Get target flash page_size: 0x%x, spare_size: 0x%x\n", page_size, spare_size);

            if(linux_images[index].type == YAFFS)
            {
                log_output("Calculate YAFFS checksum\n");
                pc_checksum = GetYaffsImageChecksum(linux_images[index].buf, linux_images[index].len, page_size, spare_size);
            }
            else // for RAW partition
            {
                pc_checksum = GetImageChecksum(linux_images[index].buf, linux_images[index].len);
            }
            log_output("Sending PC-side ckecksum (0x%04X)\n", pc_checksum);
            if (com_send_word(com_handle, pc_checksum) != COM_STATUS_DONE)
            {
                return 350;
            }
            // Read checksum response < 4mins
            if( com_recv_data_chk_len(com_handle, &data, 1) != COM_STATUS_DONE )
            {
                return 351;
            }

            if (com_recv_word(com_handle, &target_checksum) != COM_STATUS_DONE)
            {
                return 352;
            }
            log_output("Getting target image ckecksum (0x%04X)\n", target_checksum);

            if(data != ACK)
            {
                log_output("Checksum Compare Fail\n");
                return 353;
            }

        }
    }


    return S_DONE;

}
STATUS_E SV5_CMD_NAND_DL_Encrypt_Partition_Data(COM_HANDLE com_handle,
                                const struct image *sec_img)
{
    unsigned char response = 0;
    unsigned short checksum =0;
    unsigned short checksumPC=0;
    unsigned int i=0;


    if (com_send_byte(com_handle, DA_DL_AND_ENCRYPT_LINUX_RAW_DATA) != COM_STATUS_DONE)
    {
        return 360;
    }

    if (com_send_dword(com_handle, sec_img->load_addr) != COM_STATUS_DONE)
    {
        return 361;
    }

    if (com_send_dword(com_handle, sec_img->len) != COM_STATUS_DONE)
    {
        return 362;
    }

    // environment checking
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 363;
    }
    if (response != ACK)
    {
        if (response == NACK)
        {
            unsigned int errorCode;
            if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
            {
                return 364;
            }
            return errorCode;
        }
        return 365;
    }
    // send data
/*    if(com_send_data(com_handle, sec_img->buf, sec_img->len) != COM_STATUS_DONE)
    {
        return 366;
    }*/
    {
        unsigned int sent_bytes = 0;
        unsigned int frame_bytes = 0;
        while( sent_bytes < sec_img->len )
        {
            if( 4096 > (sec_img->len-sent_bytes) )
            {
                frame_bytes = sec_img->len - sent_bytes;
            }
            else
            {
                // the normal frame
                frame_bytes = 4096;
            }
            // send frame
            if(com_send_data(com_handle, sec_img->buf+sent_bytes, frame_bytes) != COM_STATUS_DONE)
            {
                return 366;
            }
            sent_bytes += frame_bytes;
        }
    }

    // send data done, calculate checksum
    for(i = 0; i < sec_img->len; i++)
    {
        checksumPC += sec_img->buf[i];
    }

    if (com_send_word(com_handle, checksumPC) != COM_STATUS_DONE)
    {
        return 367;
    }
    // receive response for checksum
    if(com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
    {
        return 368;
    }
    if (response != ACK)
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 369;
        }
        return errorCode;
    }

    // receive response for program
    if(com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
    {
        return 370;
    }
    if (response != ACK)
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 371;
        }
        return errorCode;
    }
    log_output("Download encrypted data success\n");
    return S_DONE;


}


STATUS_E SV5_FlashTool_CheckPartitionTable(COM_HANDLE com_handle,
                                    const struct image *linux_images,
                                    const unsigned int num_linux_images,
                                    pt_resident* new_part,
                                    pt_info* pi)
{
    unsigned int part_num = 0;
    unsigned int tmp_addr = 0;
    unsigned int index = 0;
    unsigned int change_index = 0;
    unsigned int ret = 0;
    unsigned int count = 0;
    pt_resident original_part[PART_MAX_COUNT];
    int pt_changed = 0;

    for(index = 0;
        index < num_linux_images;
        ++index)
    {
        memcpy(new_part[index].name, linux_images[index].name, MAX_PARTITION_NAME_LEN);
        new_part[index].offset = linux_images[index].load_addr;//dm_part->part_info[part_num].start_addr;
        new_part[index].size = 0;
        new_part[index].mask_flags = 0;
        if (index >0)
        {
            new_part[index-1].size = (linux_images[index].load_addr - new_part[index-1].offset);
        }
    }



    // Start to query the partition info
    // get cout
    // get part info
    ret = SV5_CMD_ReadPartitionInfo(com_handle, original_part, &count, PART_MAX_COUNT);
    if ( S_DONE != ret )
    {
        // If this is the first time to dl, return S_PART_NO_VALID_TABLE
        return ret;
    }

    for (index = 0; index < num_linux_images; index++)
    {
        log_output("ON Host PART[%-2d]: [%-14s], (0x%016X), (0x%016X)\n", index, new_part[index].name, new_part[index].offset, new_part[index].size);
    }

    for(change_index = 0; change_index < count; change_index++)
    {
        if( (new_part[change_index].size != original_part[change_index].size)||
            (new_part[change_index].offset != original_part[change_index].offset))
        {
#if defined(_MSC_VER)
            log_output("Partition %d (%-14s) size changed from 0x%016I64X to 0x%016I64X\n\
            offset changed from 0x%016I64X to 0x%016I64X\n",
#else
            log_output("Partition %d (%-14s) size changed from 0x%016llX to 0x%016llX\n\
            offset changed from 0x%016llX to 0x%016llX\n",
#endif
            change_index,
            original_part[change_index].name,
            original_part[change_index].size,
            new_part[change_index].size,
            original_part[change_index].offset,
            new_part[change_index].offset);
            pt_changed = 1;
            break;
        }
    }

    if(pt_changed == 1)
    {
        //full linux binaries download
        // the download partition info is different with target's.
        ret = S_FT_PMT_MISMATCH;
    }
    else
    {
        ret = S_DONE;
    }

    return ret;
}

STATUS_E SV5_FlashTool_ReadPartitionCount(COM_HANDLE com_handle, unsigned int* count)
{
    STATUS_E ret = S_DONE;
    pt_resident temp[PART_MAX_COUNT];
    ret = SV5_CMD_ReadPartitionInfo(com_handle, temp, count, PART_MAX_COUNT);
    return ret;
}

STATUS_E SV5_CMD_ReadPartitionInfo(COM_HANDLE com_handle,
                                 pt_resident* part,
                                 unsigned int * part_num,
                                 const unsigned int max_num)
{
    unsigned char response = 0;
    unsigned int index = 0;
    unsigned int total_len, table_size;

    if (com_send_byte(com_handle, DA_READ_PARTITION_TBL_CMD) != COM_STATUS_DONE)
    {
        return 980;
    }

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 981;
    }

    if (response != ACK)
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 982;
        }
        return errorCode;//if this is the first time to dl, DA will return S_PART_NO_VALID_TABLE
    }
    //command is allowed
    if (com_recv_dword(com_handle, &total_len) != COM_STATUS_DONE)
    {
        return 983;
    }

    if (0 != (total_len % sizeof(pt_resident)))
    {
        log_output("PartitionInfo not match: total_len(%d), entry_len(%d).", total_len, sizeof(pt_resident));
        if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
        {
            return 984;
        }
        else
        {
            return 985;
        }
    }
    else if (max_num < (total_len/sizeof(pt_resident)))
    {
        log_output("insufficient memory: total_len(%d), entry_len(%d), maxnum(%d)", total_len, sizeof(pt_resident), max_num);
        if (com_send_byte(com_handle, NACK) != COM_STATUS_DONE)
        {
            return 986;
        }
        else
        {
            return 987;
        }

    }
    else
    {
        if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
        {
            return 988;
        }
    }

    table_size = total_len/sizeof(pt_resident);
    if (com_recv_data_chk_len(com_handle, (unsigned char*)part, total_len) != COM_STATUS_DONE)
    {
        return 989;
    }

    if (com_send_byte(com_handle, ACK) != COM_STATUS_DONE)
    {
        return 990;
    }
    // dump partitions
    for(index = 0; index < table_size; ++index)
    {
#if defined(_MSC_VER)
        log_output("ON Target PART[%-2d](%-14s) - offset (0x%016I64X) - size (0x%016I64X) - mask (0x%016I64X)\n",
#else
        log_output("ON Target PART[%-2d](%-14s) - offset (0x%016llX) - size (0x%016llX) - mask (0x%016llX)\n",
#endif
            index,
            part[index].name,
            part[index].offset,
            part[index].size,
            part[index].mask_flags);
    }
    *part_num = table_size;

    {
        unsigned int part_size = 0xffff;
        //There must be one of partitions whose size is 0
        for (index = 0; index < table_size; index++)
        {
            if(part[index].size ==0)
            {
                part_size = index+1;
                break;
            }
        }
        if (part_size == 0xffff)
        {
            log_output("The table may be corrupted!");
            return S_FT_LAST_PARTITION_NOT_FOUND;
        }
    }
    return S_DONE;
}
STATUS_E SV5_CMD_WritePartitionInfo(COM_HANDLE com_handle,
                                         const struct image *linux_images,
                                         const unsigned int num_linux_images,
                                         pt_resident* new_part,
                                         const unsigned int max_num,
                                         int bIsUpdated)
{
    unsigned char response = 0;
    unsigned int index = 0;
    const int pt_size = sizeof(pt_resident);
    const int pi_size = sizeof(pt_info);

    if (com_send_byte(com_handle, DA_WRITE_PARTITION_TBL_CMD) != COM_STATUS_DONE)
    {
        return 300;
    }

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 301;
    }

    if (response != ACK)
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 302;
        }
        return errorCode;
    }
    //command is allowed
    if (com_send_dword(com_handle, num_linux_images) != COM_STATUS_DONE)
    {
        return 303;
    }
    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 304;
    }
    if (response != ACK)
    {
        return 305;
    }

    // target table size is received.

    for (index = 0; index < num_linux_images; ++index)
    {
        // write
        if(com_send_data(com_handle, (unsigned char *)&new_part[index], pt_size) != COM_STATUS_DONE)
        {
            return 306;
        }
        // receive response
        if(com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
        {
            return 307;
        }
        // found
        if( response == ACK )
        {
            break;
        }
        // not found
        else if( response == CONT_CHAR )
        {
            continue;
        }
        // something wrong
        else
        {
            return 308;
        }
    }

    // Send the table update flag
    if (com_send_dword(com_handle, bIsUpdated))
    {
        return 309;
    }

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return 310;
    }

    if (com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
    {
        return 311;
    }
    if( response != ACK )
    {
        unsigned int errorCode;
        if (com_recv_dword(com_handle, &errorCode) != COM_STATUS_DONE)
        {
            return 312;
        }
        return errorCode;

    }
    log_output("(%s) Partition table Done ...", bIsUpdated?"UPDATE":"NEW");
    return S_DONE;
}


STATUS_E da_disconnect(COM_HANDLE com_handle)
{
    unsigned char response = 0;

    if (com_send_byte(com_handle, DA_FINISH_CMD) != COM_STATUS_DONE)
    {
        return S_FT_FINISH_CMD_FAIL;
    }

    if (com_recv_byte(com_handle, &response) != COM_STATUS_DONE)
    {
        return S_FT_FINISH_CMD_FAIL;
    }

    if (response != ACK)
    {
        return S_FT_FINISH_CMD_FAIL;
    }

    return S_DONE;
}


STATUS_E da_EnableWatchDog(COM_HANDLE com_handle, unsigned short  ms_timeout_interval)
{
    unsigned char response = 0;
    //jian.dong add from Huifen.Wang
    unsigned int resetFlag = 0x0; // 0x0 for normal mode, 0x1 for download mode

    if (com_send_byte(com_handle, DA_ENABLE_WATCHDOG_CMD) != COM_STATUS_DONE)
    {
        return S_FT_FINISH_CMD_FAIL;
    }

    if (com_send_word(com_handle, ms_timeout_interval) != COM_STATUS_DONE)
    {
        return S_FT_FINISH_CMD_FAIL;
    }
    //jian.dong add from Huifen.Wang
    if (com_send_dword(com_handle, resetFlag) != COM_STATUS_DONE) {
        return S_FT_FINISH_CMD_FAIL;
    }
    log_output("da_EnableWatchDog resetFlag (0x%u)\n", resetFlag);
    if (com_recv_data_chk_len(com_handle, &response, 1) != COM_STATUS_DONE)
    {
        return S_FT_FINISH_CMD_FAIL;
    }
    if (response != ACK)
    {
        return S_FT_FINISH_CMD_FAIL;
    }

    return S_DONE;
}

