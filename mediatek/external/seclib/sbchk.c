/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2011
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

/******************************************************************************
 *  MODULE NAME
 ******************************************************************************/
#define MOD                         "ASP"

/******************************************************************************
 *  INTERNAL DEFINITION
 ******************************************************************************/
#define SEC_OK                      0x0000
#define SEC_SBOOT_NOT_ENABLED       0x9007
#define SEC_SUSBDL_NOT_ENABLED      0x9009


/******************************************************************************
 *  SEC FILE LIST
 ******************************************************************************/
/* the integrity of all the files listed in the following ini will be protected */
#define ANDRO_SEC_FILE_LIST         "/system/etc/firmware/S_ANDRO_SFL.ini"
#define SECRO_SEC_FILE_LIST         "/system/secro/S_SECRO_SFL.ini"

/******************************************************************************
 *  MD FILE
 ******************************************************************************/
#define MODEM_BIN                   "/system/etc/firmware/modem.img"

/******************************************************************************
 *  EXTERNAL FUNCTION
 ******************************************************************************/
extern int asp_main(int argc, char *argv[], bool bPunishCtrl, char *Android_SecFL, char *SecRO_SecFL, char *MD, unsigned int delay_check_s);

/******************************************************************************
 *  MAIN FLOW
 ******************************************************************************/

int main(int argc, char *argv[])
{
    int ret = SEC_OK;
   
    /* =================================== */
    /* do check                            */
    /* =================================== */
    /* @ asp_main parameter */
    /*   1st : application input argument */
    /*   2nd : application input argument */
    /*   3rd : do sbchk punishment 'kernel assert' or not if image is verified fail */    
    /*   4th : secure file list */
    /*   5th : secure file list */
    /*   6th : modem file */    
    /*   7th : delay 's' then do check */    
    ret = asp_main(argc,argv,true,ANDRO_SEC_FILE_LIST,SECRO_SEC_FILE_LIST,MODEM_BIN,5);

    if((ret == SEC_SBOOT_NOT_ENABLED) || (ret == SEC_SUSBDL_NOT_ENABLED))
    {
        printf("[%s] no check. ret 0x%x\n",MOD,ret);
        goto _end;
    }
    
    if(SEC_OK != ret)
    {   
        printf("[%s] check fail. ret '0x%x'\n",MOD,ret);
        assert(0);
    }

_end:
    return ret;    

}
