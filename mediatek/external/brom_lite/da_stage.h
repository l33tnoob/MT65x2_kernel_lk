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
*  da_stage.h
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains DA stage header
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
#ifndef _DA_STAGE_H
#define _DA_STAGE_H


#include "interface.h"
#include "_External/include/mtk_status.h"
#include "download_images.h"
#include "_External/include/DOWNLOAD.H"
#include "image.h"

#define PACKET_RE_TRANSMISSION_TIMES 3

typedef struct pt_resident
{
    unsigned char name[MAX_PARTITION_NAME_LEN];     /* partition name */
    U64 size;                          /* partition size */
    U64 offset;                        /* partition start */
    U64 mask_flags;                    /* partition flags */

} pt_resident; //store data by 64 bit

typedef struct pt_info
{
    int sequencenumber:8;
    int tool_or_sd_update:8;
    int mirror_pt_dl:4;   //mirror download OK
    int mirror_pt_has_space:4;
    int pt_changed:4;
    int pt_has_space:4;
} pt_info;

typedef struct _bmt_entry
{
    U16 bad_index;     // bad block index
    U16 mapped_index;  // mapping block index in the replace pool
}bmt_entry;


typedef enum{
    FEATURE_CHECK_WITH_ARM_BL = 0,
    FEATURE_CHECK_WITH_MAUI = 1,
    FEATURE_CHECK_WITH_EXT_BL = 2
}Feature_Check_Type;


typedef struct
{
    // NOR flash report
    STATUS_E            m_nor_ret;
    HW_ChipSelect_E     m_nor_chip_select[2];
    unsigned short      m_nor_flash_id;
    unsigned int        m_nor_flash_size;
    unsigned int        m_nor_flash_size_die1;
    unsigned short      m_nor_flash_dev_code_1;
    unsigned short      m_nor_flash_dev_code_2;
    unsigned short      m_nor_flash_dev_code_3;
    unsigned short      m_nor_flash_dev_code_4;
    STATUS_E            m_nor_flash_otp_status;
    unsigned int        m_nor_flash_otp_size;
    unsigned short      m_nor_flash_id_die2;
    unsigned int        m_nor_flash_size_die2;
    unsigned short      m_nor_flash_dev_code_1_die2;
    unsigned short      m_nor_flash_dev_code_2_die2;
    unsigned short      m_nor_flash_dev_code_3_die2;
    unsigned short      m_nor_flash_dev_code_4_die2;
    STATUS_E            m_nor_flash_otp_status_die2;
    unsigned int        m_nor_flash_otp_size_die2;


    // NAND flash report
    STATUS_E            m_nand_ret;
    HW_ChipSelect_E     m_nand_chip_select;
    unsigned short      m_nand_flash_id;
    unsigned int        m_nand_flash_size;
    unsigned short      m_nand_flash_dev_code_1;
    unsigned short      m_nand_flash_dev_code_2;
    unsigned short      m_nand_flash_dev_code_3;
    unsigned short      m_nand_flash_dev_code_4;
	unsigned short		m_nand_flash_dev_code_1_part2;
	unsigned short		m_nand_flash_dev_code_2_part2;
	unsigned short		m_nand_flash_dev_code_3_part2;
	unsigned short		m_nand_flash_dev_code_4_part2;
    unsigned short      m_nand_pagesize;
    unsigned short      m_nand_sparesize;
    unsigned short      m_nand_pages_per_block;
    unsigned char       m_nand_io_interface;
    unsigned char       m_nand_addr_cycle;

    // EMMC flash report
    STATUS_E            m_emmc_ret;
    unsigned char       m_emmc_manufacture_id;
    char                m_emmc_product_name[8];
    unsigned char       m_emmc_partitioned;
    unsigned int        m_emmc_boot1_size; // unit: 512 bytes
    unsigned int        m_emmc_boot2_size; // unit: 512 bytes
    unsigned int        m_emmc_rpmb_size;  // unit: 512 bytes
    unsigned int        m_emmc_gp1_size;   // unit: 512 bytes
    unsigned int        m_emmc_gp2_size;   // unit: 512 bytes
    unsigned int        m_emmc_gp3_size;   // unit: 512 bytes
    unsigned int        m_emmc_gp4_size;   // unit: 512 bytes
    unsigned int        m_emmc_ua_size;    // unit: 512 bytes

    // Internal RAM report
    STATUS_E            m_int_sram_ret;
    unsigned int        m_int_sram_size;

    // External RAM report
    STATUS_E            m_ext_ram_ret;
    HW_RAMType_E        m_ext_ram_type;
    HW_ChipSelect_E     m_ext_ram_chip_select;
    unsigned int        m_ext_ram_size;

} DA_REPORT_T;




STATUS_E da_connect(COM_HANDLE com_handle,
                    const struct image *nor_flash_table,
                    const struct image *nand_flash_table,
                    int isUSB,
                    unsigned int bmt_address);

STATUS_E da_disconnect(COM_HANDLE com_handle);

STATUS_E da_EnableWatchDog(COM_HANDLE com_handle, unsigned short  ms_timeout_interval);

STATUS_E da_NAND_ReadBlock(COM_HANDLE com_handle,
						unsigned int startAddr,
						unsigned int blockSize,
						NUTL_ReadFlag_E flag,
						unsigned char *Buffer,
						unsigned int *buf_len);

STATUS_E da_FormatFAT(COM_HANDLE com_handle,
						NUTL_EraseFlag_E  flag,
						unsigned int  bValidation,
                         Range *nor_fat_range,
                         Range *nand_fat_range);

STATUS_E da_FormatFlash(COM_HANDLE com_handle,
						HW_StorageType_E  type,
						NUTL_EraseFlag_E  flag,
						unsigned int  bValidation,
						unsigned int address,
						unsigned int length);


STATUS_E da_write_boot_loader(COM_HANDLE com_handle,
                         const struct image *bootLoader,
						 const struct image *dspBootLoader,
						 const struct image *extBootLoader);

STATUS_E da_write_NFB_images(COM_HANDLE com_handle,
                        const struct image *rom,
						const struct image *secondRom,
						const struct image *dspRom,
						const struct image *demand_paging_rom);

STATUS_E da_write_linux_images(COM_HANDLE com_handle,
							 struct image *linux_images,
							 const unsigned int num_linux_images,
							 const int eraseHB);


STATUS_E SV5_CMD_CheckBootLoaderFeature_CheckLoadType(COM_HANDLE com_handle,
                         const struct image *img, Feature_Check_Type checkType);
STATUS_E SV5_CMD_GetSettingOfCBR(COM_HANDLE com_handle, unsigned int *startBlockOfCBR,
                            unsigned int *numNormalBlock, unsigned int *numFastlBlock,
                            unsigned int *numSpareBlock, unsigned int *numMaxRecordInBlock,
                            unsigned int *numTotalBlock, unsigned char *bReadOnlyMode,
                            unsigned int *cbr_version);

STATUS_E SV5_CMD_FormatCBR(COM_HANDLE com_handle);
STATUS_E SV5_CMD_CreateCBR(COM_HANDLE com_handle, unsigned int numNormalBlock,
                            unsigned int numFastlBlock, unsigned int numMinSpareBlock,
                            unsigned int numMaxSpareBlock, unsigned int numMaxRecordInBlock,
                            unsigned char bReadOnlyMode,  unsigned int cbr_version);

STATUS_E SV5_CMD_BACKUP_AND_ERASE_HB(COM_HANDLE com_handle);
STATUS_E SV5_CMD_RESTORE_HB(COM_HANDLE com_handle);
STATUS_E SV5_CMD_ReadBMT(COM_HANDLE com_handle, DA_REPORT_T *p_da_report);
STATUS_E SV5_CMD_NAND_DL_Partition_Data(COM_HANDLE com_handle,
                                const struct image *linux_images,
                                const unsigned int num_linux_images,
                                unsigned int packetLength);
STATUS_E SV5_CMD_NAND_DL_Encrypt_Partition_Data(COM_HANDLE com_handle,
                                const struct image *sec_img);

STATUS_E SV5_CMD_ReadPartitionInfo(COM_HANDLE com_handle,
                                 pt_resident* part,
                                 unsigned int * part_num,
                                 const unsigned int max_num);
STATUS_E SV5_CMD_WritePartitionInfo(COM_HANDLE com_handle,
                                 const struct image *linux_images,
                                 const unsigned int num_linux_images,
                                 pt_resident* new_part,
                                 const unsigned int max_num,
                                 int bIsUpdated);

STATUS_E SV5_FlashTool_ReadPartitionCount(COM_HANDLE com_handle, unsigned int* count);
STATUS_E SV5_FlashTool_CheckPartitionTable(COM_HANDLE com_handle,
                                    const struct image *linux_images,
                                    const unsigned int num_linux_images,
                                    pt_resident* new_part,
                                    pt_info* pi);




#endif /* _DA_STAGE_H */

