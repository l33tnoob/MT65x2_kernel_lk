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
*  download_images.c
*
* Project:
* --------
*   Standalone Flash downloader sample code
*
* Description:
* ------------
*   This module contains download procedure.
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
#include "bootrom_stage.h"
#include "da_stage.h"
#include "download_images.h"
#include "interface.h"
#include "_External/include/DOWNLOAD.H"
#if defined(__GNUC__)
#include "GCC_Utility.h"
#endif

static STATUS_E bootrom_stage(COM_HANDLE com_handle,
                                const struct image *download_agent,
                                const struct image *download_agent_TCM,
                                const struct image *download_EPP,
                                const ExternalMemorySetting *externalMemorySetting,
                                int isUSB, int isNFB)
{
    STATUS_E status;
    unsigned int response;
    unsigned int size = 0;

    log_output("Connecting to BootROM... com_handle(%d)\n", com_handle);
    status = bootrom_connect(com_handle);


    if (status != S_DONE)
    {
        log_output("FAIL:Connecting to BoortRom... handle port (%d)\n",com_handle);
        log_feedback("FAIL:Connecting to BoortRom... handle port (%d)\n",com_handle);    
        return status;
    }
    log_output("SUCCESS:Connecting to BoortRom... handle port (%d)\n",com_handle);
    log_feedback("SUCCESS:Connecting to BoortRom... handle port (%d)\n",com_handle);    

    //Todo: detect bootROM connection or bootloader connection
    //Use BROM USB below
#if defined(_MSC_VER)
    if (com_change_timeout(com_handle, 500, 5000))
    {
        return S_COM_PORT_SET_TIMEOUT_FAIL;
    }
#endif


    log_output("Disabling watchdog..\n");

    status = bootrom_disable_watchdog(com_handle);
    if (status != S_DONE)
    {
        if (com_recv_dword(com_handle, &response) != COM_STATUS_DONE)
        {
            log_output("FAIL:Disabling watchdog..\n");
            log_feedback("FAIL:Disabling watchdog..\n");    
            return status;
        }
    }
    log_output("SUCCESS:Disabling watchdog..\n");
    log_feedback("SUCCESS:Disabling watchdog..\n");    


    log_output("Latching powerkey...\n");
    status = bootrom_latch_powerkey(com_handle);
    if (status != S_DONE)
    {
        log_output("FAIL:Latching powerkey...\n");
        log_feedback("FAIL:Latching powerkey...\n");    
        return status;
    }
    log_output("SUCCESS:Latching powerkey...\n");
    log_feedback("SUCCESS:Latching powerkey...\n");    

    //SetRemap for BROM
    log_output("Set Remap...\n");
    status = bootrom_SetRemap(com_handle);
    if (status != S_DONE)
    {
        log_output("FAIL:Set Remap...\n");
        log_feedback("FAIL:Set Remap...\n");    
        return status;
    }
    log_output("SUCCESS:Set Remap...\n");
    log_feedback("SUCCESS:Set Remap...\n");    

    log_output("Send EPP...\n");
    status = bootrom_SendEPP(com_handle, download_EPP, externalMemorySetting);
    if (status != S_DONE)
    {
        log_output("FAIL:Send EPP...\n");
        log_feedback("FAIL:Send EPP...\n");    
        return status;
    }
    log_output("SUCCESS:Send EPP...\n");
    log_feedback("SUCCESS:Send EPP...\n");

    log_output("Sending DA1...\n");
    status = bootrom_send_download_agent(com_handle, download_agent, isUSB);
    if (status != S_DONE)
    {
        log_output("FAIL:Sending DA1...\n");
        log_feedback("FAIL:Sending DA1...\n");    
        return status;
    }
    log_output("SUCCESS:Sending DA1...\n");
    log_feedback("SUCCESS:Sending DA1...\n");    

    if(download_agent_TCM->buf != NULL)
    {
        log_output("Sending DA2...\n");
        status = bootrom_send_download_agent(com_handle, download_agent_TCM, isUSB);
        if (status != S_DONE)
        {
            log_output("FAIL:Sending DA2...\n");
            log_feedback("FAIL:Sending DA2...\n");    
            return status;
        }
        log_output("SUCCESS:Sending DA2...\n");
        log_feedback("SUCCESS:Sending DA2...\n");    
    }

    log_output("Transferring control to DA...\n");
    status = bootrom_jump_to_download_agent(com_handle, download_agent);
    if (status != S_DONE)
    {
        log_output("FAIL:Transferring control to DA...\n");
        log_feedback("FAIL:Transferring control to DA...\n");    
        return status;
    }
    log_output("SUCCESS:Transferring control to DA...\n");
    log_feedback("SUCCESS:Transferring control to DA...\n");    

    return S_DONE;
}

static STATUS_E da_stage(COM_HANDLE com_handle,
                         const struct image *nor_flash_table,
                         const struct image *nand_flash_table,
                         const struct image *bootLoader,
             const struct image *extBootLoader,
             const struct image *dspBootLoader,
             const struct image *rom,
                         const struct image *secondRom,
             const struct image *dspRom,
             const struct image *demand_paging_rom,
             struct image *linux_images,
             unsigned int num_linux_images,
             int isUSB,
             int isNFB,
             unsigned int bmt_address)
{
    STATUS_E status;

    // To enable hardware flow contrl, you have to
    // (1) properly configure CTS/RTS of the target hardware,
    // (2) use hardware-flow-control-enabled Download Agent, and
    // (3) uncomment the following lines

    //if (com_enable_hardware_flow_control(com_handle) != COM_STATUS_DONE)
    //{
    //    log_output("Failed to enable HW flow control "
    //               "on the communication port\n");
    //    return S_UNDEFINED_ERROR;
    //}

    unsigned char response = 0;
    unsigned int errorCode =0;
    int eraseHB = 0;

    log_output("Connecting to DA...\n");
    status = da_connect(com_handle, nor_flash_table, nand_flash_table, isUSB, bmt_address);

    if (status != S_DONE)
    {
        log_output("FAIL:Connecting to DA...\n");
        log_feedback("FAIL:Connecting to DA...\n");    
        return status;
    }
    log_output("SUCCESS:Connecting to DA...\n");
    log_feedback("SUCCESS:Connecting to DA...\n");    

    //CMD_GetFATRanges

    if(isNFB)
    {
        //Format CBR before writing
        log_output("Formatting CBR...\n");
        status = SV5_CMD_FormatCBR(com_handle);
        if(status != 0)
        {
            log_output("FAIL:Formatting CBR...\n");
            log_feedback("FAIL:Formatting CBR...\n");    
            return status;
        }
        log_output("SUCCESS:Formatting CBR...\n");
        log_feedback("SUCCESS:Formatting CBR...\n");    
    
        //Write Boot loader
        //Assume bootloader full download if bootLoader != NULL
        //or no bootloader download
        if(bootLoader != NULL)
        {
            log_output("Write Boot loader...\n");
            status = da_write_boot_loader(com_handle, bootLoader, extBootLoader, dspBootLoader);
            if (status != S_DONE)
            {
                log_output("FAIL:Write Boot loader...\n");
                log_feedback("FAIL:Write Boot loader...\n");
                return status;
            }
            log_output("SUCCESS:Write Boot loader...\n");
            log_feedback("SUCCESS:Write Boot loader...\n");
        }
        else
        {
            // Bypass check boot loader feature
            // only check target is not empty
            log_output("Get boot loader feature...\n");
            status = SV5_CMD_CheckBootLoaderFeature_CheckLoadType(com_handle, bootLoader, FEATURE_CHECK_WITH_ARM_BL);
            if (status != S_DONE)
            {
                log_output("FAIL:Get boot loader feature...\n");
                log_feedback("FAIL:Get boot loader feature...\n");
                return status;
            }
            log_output("SUCCESS:Get boot loader feature...\n");
            log_feedback("SUCCESS:Get boot loader feature...\n");
        }

//======================================================
        //Send NFB write image
        if( (rom != NULL && rom->buf) ||
            (secondRom != NULL && secondRom->buf) ||
            (dspRom != NULL && dspRom->buf) ||
            (demand_paging_rom != NULL && demand_paging_rom->buf))
        {
            log_output("Write NFB images...\n");
                status = da_write_NFB_images(com_handle,
                                rom,
                                secondRom,
                                dspRom,
                                demand_paging_rom);
                if (status != S_DONE)
                {
                log_output("FAIL:Write NFB images...\n");
                log_feedback("FAIL:Write NFB images...\n");
                    return status;
                }
            log_output("SUCCESS:Write NFB images...\n");
            log_feedback("SUCCESS:Write NFB images...\n");
        }

/*
        status = da_FormatFAT(com_handle,
                                HW_STORAGE_NAND,
                                0,
                                NULL,
                                NULL);
        if (status != S_DONE)
        {
            return status;
        }
*/
//======================================================

//======================================================
        if(num_linux_images > 0)
        {
            // if both the ARM BL and EXT BL are downloaded, erase the HB
            if(bootLoader->buf != NULL && extBootLoader->buf != NULL)
            {
                eraseHB = 1;
            }

                //Send NFB write linux partition image
                log_output("Write linux partition images...\n");
                status = da_write_linux_images( com_handle,
                                            linux_images ,
                                            num_linux_images,
                                            eraseHB); // 1 for eraseHB
                if (status != S_DONE)
                {
                log_output("FAIL:Write linux partition images...\n");
                log_feedback("FAIL:Write linux partition images...\n");
                    return status;
                }
            log_output("SUCCESS:Write linux partition images...\n");
            log_feedback("SUCCESS:Write linux partition images...\n");
        }
//======================================================


/*
//======================================================
        //Send total format
        status = da_FormatFlash(com_handle,
                                HW_STORAGE_NAND,
                                NUTL_ERASE,
                                0,
                                0x00000000,
                                0x08000000);
//======================================================
*/
    }
    else
    {
    }

    log_output("Enable Watch Dog...\n");
    da_EnableWatchDog(com_handle, (unsigned short)(1000/15.6));

#if 0
    /* doesn't care the return code of da_EnableWatchDog ? */
    if (status != S_DONE)
    {
        log_output("FAIL:Enable Watch Dog...\n");
        log_feedback("FAIL:Enable Watch Dog...\n");
        return status;
    }
#endif
    log_output("SUCCESS:Enable Watch Dog...\n");
    log_feedback("SUCCESS:Enable Watch Dog...\n");

    return S_DONE;
}
STATUS_E download_images(const struct image *download_agent,
                         const struct image *download_agent_TCM,
                         const struct image *nor_flash_table,
                         const struct image *nand_flash_table,
                         const struct image *download_EPP,
                         const struct image *bootLoader,
                         const struct image *extBootLoader,
                         const struct image *dspBootLoader,
                         const struct image *rom,
                         const struct image *secondRom,
                         const struct image *dspRom,
                         const struct image *demand_paging_rom,
                         struct image *linux_images,
                         const struct ExternalMemorySetting *externalMemorySetting,
                         unsigned int num_linux_images,
                         unsigned int bmt_address,
                         int isUSB,
                         int isNFB)
{

    COM_HANDLE com_handle = INVALID_COM_HANDLE;
    STATUS_E status;

    if(isUSB){
        int retry = 0;

        while(1)
        {
            Sleep(100);
            //if (com_open(&com_handle, 19200) == COM_STATUS_DONE)
            if (com_open(&com_handle, 115200) == COM_STATUS_DONE)
            {
                log_output("handle (%d)\n", com_handle);
                log_feedback("SUCCESS:handle port (%d)\n",com_handle);
                break;
            }else{
                log_output("faile handle (%d)\n", com_handle);
                log_feedback("FAIL:handle port (%d)\n",com_handle);
                com_close(&com_handle);
                if (++retry > 10)
                    return S_COM_PORT_OPEN_FAIL;
            }
        }

    }else{
        if (com_open(&com_handle, 115200) != COM_STATUS_DONE)
        {
            log_output("Failed to open the communication port\n");
            return S_COM_PORT_OPEN_FAIL;
        }
    }


    status = bootrom_stage(com_handle, download_agent,download_agent_TCM, download_EPP, externalMemorySetting, isUSB, isNFB);

    if (status != S_DONE)
    {
            log_output("Download failed in BootROM stage: error=%u\n", status);
            //com_close(&com_handle);
            return status;
    }

    status = da_stage(com_handle, nor_flash_table, nand_flash_table,
        bootLoader,extBootLoader,dspBootLoader, rom, secondRom, dspRom, demand_paging_rom,
        linux_images,
        num_linux_images,
        isUSB,
        isNFB,
        bmt_address);

    if (status != S_DONE)
    {
        log_output("Download failed in DA stage: error=%u\n", status);
        //com_close(&com_handle);
        //Sleep(5000);
        return status;
    }
    
    log_feedback("FINISHED UPDATE");
    log_output("FINISHED UPDATAE!\n");

    if (com_close(&com_handle) != COM_STATUS_DONE)
    {
        log_output("Failed to close the communication port\n");
        //Sleep(5000);
    }
    //Sleep(5000);

    return S_DONE;
}

