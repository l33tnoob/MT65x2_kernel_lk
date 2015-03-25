/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_nfc.c
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   NFC META implementation.
 *
 * Author:
 * -------
 *  
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 08 26 2011 xiao.liu
 * [ALPS00070195] [Need Patch] [Volunteer Patch][EM] add deinit in NFC
 * .
 *
 * 08 17 2011 xiao.liu
 * [ALPS00067797] [Blocking][NFC] Enter EM to select NFC , then Phone always show JE
 * .
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <ctype.h>
#include <dirent.h>
#include "meta_nfc_para.h"
#include "WM2Linux.h"


static NFC_CNF nfc_cnf;
static int nfc_service_sockfd = -1;
static pthread_t  read_cnf_thread_handle;
static unsigned char bStop_ReadThread = 0;

#ifdef META_NFC_SELF_TEST_EN
static NFC_CNF_CB meta_nfc_cnf_cb = NULL;
#endif
extern BOOL WriteToEM(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);

#ifdef META_NFC_SELF_TEST_EN
void META_NFC_Register(NFC_CNF_CB callback)
{
    meta_nfc_cnf_cb = callback;
}
#endif
static void* META_NFC_read_cnf(void *arg)
{
    int rec_bytes = 0;
    // Read resonse
    META_LOG("META_NFC_CMD:NFC read thread start");
    bStop_ReadThread = 0;
    while(bStop_ReadThread == 0)
    {    
        ilm_struct nfc_ilm_rec;
        nfc_msg_struct nfc_msg;
        unsigned char nfc_msg_length;
        unsigned char fgSupport = 1;
        //clean struct buffer
        memset(&nfc_ilm_rec, 0, sizeof(ilm_struct));
        //read fd
        //if get response break
        rec_bytes = read(nfc_service_sockfd,(char*)&nfc_ilm_rec, sizeof(ilm_struct));             
        if (rec_bytes > 0)
        {
            // check msg id
            
            META_LOG("META_NFC_CMD:NFC read (msg_id,dest_mod_id) = (%d,%d)",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);
            
            if ((nfc_ilm_rec.msg_id == MSG_ID_NFC_TEST_RSP) && (nfc_ilm_rec.dest_mod_id == MOD_NFC_APP))
            {
                nfc_msg_length = sizeof(nfc_msg_struct);
                memcpy( &nfc_msg, (nfc_msg_struct*)nfc_ilm_rec.local_para_ptr, nfc_msg_length);
                META_LOG("META_NFC_CMD:NFC read msg_type=%d,length=%d", nfc_msg.msg_type,nfc_msg_length);
                switch (nfc_msg.msg_type)
                {
                	
                	  case MSG_ID_NFC_STOP_TEST_RSP:
                	  {
                	  		nfc_cnf.op = NFC_OP_DEINIT;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC MSG_ID_NFC_STOP_TEST_RSP =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                           
                        break;
                	  }
                    case MSG_ID_NFC_SETTING_RSP:
                    { 
                        nfc_cnf.op = NFC_OP_SETTING;
                        memcpy(&nfc_cnf.result.m_setting_cnf, (nfc_setting_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_setting_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SETTING =%d/%d/%d/%d/%d/%d/%d/%d/%d/", 
                            nfc_cnf.result.m_setting_cnf.status,
                            nfc_cnf.result.m_setting_cnf.debug_enable,
                            nfc_cnf.result.m_setting_cnf.fw_ver,
                            nfc_cnf.result.m_setting_cnf.get_capabilities,
                            nfc_cnf.result.m_setting_cnf.sw_ver,
                            nfc_cnf.result.m_setting_cnf.hw_ver,
                            nfc_cnf.result.m_setting_cnf.fw_ver,
                            nfc_cnf.result.m_setting_cnf.reader_mode,
                            nfc_cnf.result.m_setting_cnf.card_mode);                        
                        break;
                    }
                    case MSG_ID_NFC_NOTIFICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_REG_NOTIFY;
                        memcpy(&nfc_cnf.result.m_reg_notify_cnf, (nfc_reg_notif_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length),sizeof(nfc_reg_notif_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/", 
                            nfc_cnf.result.m_reg_notify_cnf.status);                              
                        break;
                    }
                    case MSG_ID_NFC_SE_SET_RSP:
                    {
                        nfc_cnf.op = NFC_OP_SECURE_ELEMENT;
                        memcpy(&nfc_cnf.result.m_se_set_cnf, (nfc_se_set_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_se_set_response));
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SECURE_ELEMENT =%d/", 
                            nfc_cnf.result.m_se_set_cnf.status);                             
                        break;
                    }
                    case MSG_ID_NFC_DISCOVERY_RSP:
                    {
                        nfc_cnf.op = NFC_OP_DISCOVERY;
                        memcpy(&nfc_cnf.result.m_dis_notify_cnf, (nfc_dis_notif_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_dis_notif_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/%d/", 
                            nfc_cnf.result.m_dis_notify_cnf.status,
                            nfc_cnf.result.m_dis_notify_cnf.type);                        
                        break;
                    }
                    case MSG_ID_NFC_TAG_READ_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_READ;
                        memcpy(&nfc_cnf.result.m_tag_read_cnf, (nfc_tag_read_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_read_response)); 
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_READ =%d/%d/", 
                            nfc_cnf.result.m_tag_read_cnf.status,
                            nfc_cnf.result.m_tag_read_cnf.type);                            
                        break;
                    }
                    case MSG_ID_NFC_TAG_WRITE_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_WRITE;
                        memcpy(&nfc_cnf.result.m_tag_write_cnf, (nfc_tag_write_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_write_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_WRITE =%d/%d/", 
                            nfc_cnf.result.m_tag_write_cnf.status,
                            nfc_cnf.result.m_tag_write_cnf.type);                         
                        break;
                    }                
                    case MSG_ID_NFC_TAG_DISCONN_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_DISCONN;
                        memcpy(&nfc_cnf.result.m_tag_discon_cnf, (nfc_tag_disconnect_request*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_disconnect_request));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_DISCONN =%d/", 
                            nfc_cnf.result.m_tag_discon_cnf.status);                           
                        break;
                    } 
                    case MSG_ID_NFC_TAG_F2NDEF_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_FORMAT_NDEF;
                        memcpy(&nfc_cnf.result.m_tag_fromat2Ndef_cnf, (nfc_tag_fromat2Ndef_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_fromat2Ndef_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_FORMAT_NDEF =%d/", 
                            nfc_cnf.result.m_tag_fromat2Ndef_cnf.status);                          
                        break;
                    } 
                    case MSG_ID_NFC_TAG_RAWCOM_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_RAW_COMM;
                        memcpy(&nfc_cnf.result.m_tag_raw_com_cnf, (nfc_tag_raw_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_raw_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_RAW_COMM =%d/%d/", 
                            nfc_cnf.result.m_tag_raw_com_cnf.status, 
                            nfc_cnf.result.m_tag_raw_com_cnf.type);                        
                        break;
                    }    
                    case MSG_ID_NFC_P2P_COMMUNICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_P2P_COMM;
                        memcpy(&nfc_cnf.result.m_p2p_com_cnf, (nfc_p2p_com_response*)nfc_ilm_rec.local_para_ptr, sizeof(nfc_p2p_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_P2P_COMM =%d/%d/", 
                            nfc_cnf.result.m_p2p_com_cnf.status, 
                            nfc_cnf.result.m_p2p_com_cnf.length);    
                        
                        break;
                    }  
                    case MSG_ID_NFC_RD_COMMUNICATION_RSP:
                    {
                        nfc_cnf.op = NFC_OP_RD_COMM;
                        memcpy(&nfc_cnf.result.m_rd_com_cnf, (nfc_rd_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_rd_com_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_RD_COMM =%d/%d/", 
                            nfc_cnf.result.m_rd_com_cnf.status, 
                            nfc_cnf.result.m_rd_com_cnf.length);                                   
                        break;
                    }
                    case MSG_ID_NFC_TX_ALWAYSON_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TX_ALWAYSON_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                           
                        break;
                    }   
                    case MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TX_ALWAYSON_WO_ACK_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_WO_ACK_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }
                    case MSG_ID_NFC_CARD_EMULATION_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_CARD_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_CARD_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                        
                        break;
                    }      
                    case MSG_ID_NFC_READER_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_READER_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_READER_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }          
                    case MSG_ID_NFC_P2P_MODE_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_P2P_MODE_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_P2P_MODE_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }    
                    case MSG_ID_NFC_SWP_SELF_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_SWP_SELF_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_SWP_SELF_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }       
                    case MSG_ID_NFC_ANTENNA_SELF_TEST_RSP:
                    {
                        nfc_cnf.op = NFC_OP_ANTENNA_SELF_TEST;
                        memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_ANTENNA_SELF_TEST =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                           
                        break;
                    }                     
                    case MSG_ID_NFC_TAG_UID_RW_RSP:
                    {
                        nfc_cnf.op = NFC_OP_TAG_UID_RW;
                        memcpy(&nfc_cnf.result.m_script_uid_cnf, (nfc_script_uid_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_uid_response));                            
                        META_LOG("META_NFC_CMD:NFC NFC_OP_TAG_UID_RW =%d/", 
                            nfc_cnf.result.m_script_cnf.result);                         
                        break;
                    }
                    case MSG_ID_NFC_CARD_MODE_TEST_RSP:
                    //case MSG_ID_NFC_STOP_TEST_RSP:
                    default:
                    {
                        fgSupport = 0;
                        META_LOG("META_NFC_CMD:Don't support CNF CMD %d",nfc_msg.msg_type);
                        break;
                    }
                }
                if (fgSupport == 1)
                {
                    META_LOG("META_NFC_CMD:NFC read nfc_cnf.op=%d,nfc_msg.msg_type=%d", nfc_cnf.op,nfc_msg.msg_type);
                    nfc_cnf.status = META_SUCCESS;
                    #ifdef META_NFC_SELF_TEST_EN
                    if (meta_nfc_cnf_cb)
                    {
                        meta_nfc_cnf_cb(&nfc_cnf, NULL, 0);
                    }
                    else
                    #endif
                    {
                        WriteToEM(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
                    }
                }
                else 
                {

                    META_LOG("META_NFC_CMD:Don't Write to PC MSGID,%d,",nfc_msg.msg_type);
                }
            }
            else 
            {
                META_LOG("META_NFC_CMD:Don't support MSGID,%d,DestID,%d",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);
            }
        }        
        else
        {
            usleep(100000); // wake up every 0.1sec     
        }
    }
    bStop_ReadThread = 1;
    META_LOG("META_NFC_CMD:NFC read thread stop");
    pthread_exit(NULL);
   return NULL;

}
/********************************************************************************
//FUNCTION:
//		META_NFC_init
//DESCRIPTION:
//		NFC Init for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
int META_NFC_init(void)
{
    
    pid_t pid;
    //int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;	
    int iResult;
    unsigned long iMode = 1;

    // Run nfc service process
    if ((pid = fork()) < 0) 
    {
        META_LOG("META_NFC_init: fork fails: %d (%s)\n", errno, strerror(errno));
        return (-2);
    } 
    else if (pid == 0)  /*child process*/
    {
        int err;
    
        META_LOG("nfc_open: execute: %s\n", "/system/xbin/nfcservice");
        err = execl("/system/xbin/nfcservice", "nfcservice", NULL);
        if (err == -1)
        {
            META_LOG("META_NFC_init: execl error: %s\n", strerror(errno));
            return (-3);
        }
        return 0;
    } 
    else  /*parent process*/
    {
        META_LOG("META_NFC_init: pid = %d\n", pid);
    }

    // Create socket

    nfc_service_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfc_service_sockfd < 0) 
    {
        META_LOG("META_NFC_init: ERROR opening socket");
        return (-4);
    }
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        META_LOG("META_NFC_init: ERROR, no such host\n");
        return (-5);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(SOCKET_NFC_PORT);
    
    
    
    



    sleep(3);  // sleep 5sec for nfcservice to finish initialization
    
    /* Now connect to the server */
    if (connect(nfc_service_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
         META_LOG("META_NFC_init: ERROR connecting");
         return (-6);
    }
    META_LOG("META_NFC_init: create read command thread\n"); 
    if(pthread_create(&read_cnf_thread_handle, NULL, META_NFC_read_cnf,
          NULL) != 0)
    {
       META_LOG("META_NFC_init:Fail to create read command thread");
       return (-7);
    }   

    
    META_LOG("META_NFC_init: done\n"); 
	return (0);
}

/********************************************************************************
//FUNCTION:
//		META_NFC_deinit
//DESCRIPTION:
//		NFC deinit for META test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_NFC_deinit()
{
    int err=0;
    /* stop RX thread */
    bStop_ReadThread = 1;
    
    /* wait until thread exist */
    pthread_join(read_cnf_thread_handle, NULL);

    /* Close socket port */
    if (nfc_service_sockfd > 0)
    {
        close (nfc_service_sockfd);
        nfc_service_sockfd = -1;
    }
    // kill service process
    META_LOG("META_NFC_deinit: kill: %s\n", "/system/xbin/nfcservice");
    err = execl("kill /system/xbin/nfcservice", "nfcservice", NULL);
    if (err == -1)
    {
        META_LOG("META_NFC_init: kill error: %s\n", strerror(errno));
    }
    return;   
}
/********************************************************************************
//FUNCTION:
//		META_NFC_CMD
//DESCRIPTION:
//		SEND MESSAGE to NFC driver
//      RECEIVE MESSAGE to NFC driver
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//     
********************************************************************************/
void META_NFC_CMD(ilm_struct* nfc_ilm_req_ptr)
{

    int ret = 0;
    int rec_bytes = 0;
    int rety_count = 0;
    META_LOG("META_NFC_CMD:write CMD");

    // Write request command
    ret = write(nfc_service_sockfd, (const char*)nfc_ilm_req_ptr, sizeof(ilm_struct));

    if ( ret <= 0)
    {
        META_LOG("META_NFC_CMD:write failure,%d",ret);
        return;
    }
    else
    {
        META_LOG("META_NFC_CMD:write CMD done,%d",ret);
    }
    return;
}


/********************************************************************************
//FUNCTION:
//		META_NFC_OP
//DESCRIPTION:
//		META NFC test main process function.
//
//PARAMETERS:
//		req: NFC Req struct
//      peer_buff: peer buffer pointer
//      peer_len: peer buffer length
//RETURN VALUE:
//		void
//      
********************************************************************************/
void META_NFC_OP(NFC_REQ *req, char *peer_buff, unsigned short peer_len) 
{     
    ilm_struct nfc_ilm_loc;
    nfc_msg_struct nfc_msg;
    memset(&nfc_cnf, 0, sizeof(NFC_CNF));
    memset(&nfc_msg, 0, sizeof(nfc_msg_struct));
    //nfc_cnf.header.id = FT_NFC_CNF_ID;
    //nfc_cnf.header.token = req->header.token;
    nfc_cnf.op = req->op;

    memset(&nfc_ilm_loc, 0, sizeof(ilm_struct));    
    nfc_ilm_loc.msg_id = MSG_ID_NFC_TEST_REQ;
    nfc_ilm_loc.src_mod_id = MOD_NFC_APP;
    nfc_ilm_loc.dest_mod_id = MOD_NFC;    
    
    switch(req->op)
    {
        META_LOG("META_NFC_OP:NFC request op=%d", req->op);
        
        case NFC_OP_DEINIT:
        { 
        	   
           bStop_ReadThread = 1;
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_STOP_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
        		
        }
        
        case NFC_OP_SETTING:
        {
            //Write handle function here
            nfc_msg.msg_length = sizeof(nfc_setting_request);
            nfc_msg.msg_type = MSG_ID_NFC_SETTING_REQ;
            META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
            memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), (char*)&req->cmd.m_setting_req, sizeof(nfc_setting_request));
            META_NFC_CMD(&nfc_ilm_loc);
            break;
       }
       case NFC_OP_REG_NOTIFY:
	   {
           //Write handle function here
	       nfc_msg.msg_length = sizeof(nfc_reg_notif_request);
           nfc_msg.msg_type = MSG_ID_NFC_NOTIFICATION_REQ;
           
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),(char*)&req->cmd.m_reg_notify_req, sizeof(nfc_reg_notif_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_SECURE_ELEMENT:
       {
            //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_se_set_request);
           nfc_msg.msg_type = MSG_ID_NFC_SE_SET_REQ;

           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_se_set_req, sizeof(nfc_se_set_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_DISCOVERY:
       {
        //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_dis_notif_request);
           nfc_msg.msg_type = MSG_ID_NFC_DISCOVERY_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_dis_notify_req, sizeof(nfc_dis_notif_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_READ:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_read_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_READ_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_read_req, sizeof(nfc_tag_read_request));
           break;
       }
       case NFC_OP_TAG_WRITE:
       {
            //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_write_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_WRITE_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_write_req, sizeof(nfc_tag_write_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_DISCONN:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_disconnect_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_DISCONN_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_discon_req, sizeof(nfc_tag_disconnect_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_FORMAT_NDEF:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_fromat2Ndef_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_F2NDEF_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_fromat2Ndef_req, sizeof(nfc_tag_fromat2Ndef_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_TAG_RAW_COMM:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_tag_raw_com_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_RAWCOM_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_raw_com_req, sizeof(nfc_tag_raw_com_request));
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_P2P_COMM:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_p2p_com_request);
           nfc_msg.msg_type = MSG_ID_NFC_P2P_COMMUNICATION_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_p2p_com_req, sizeof(nfc_p2p_com_request));
           if ((peer_buff != NULL) && (peer_len != 0))
           {
               memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct) + sizeof(nfc_p2p_com_request)), peer_buff, peer_len);          
           }
           META_NFC_CMD(&nfc_ilm_loc);
           break;
       }
       case NFC_OP_RD_COMM:
       {
            //Write handle function here
            nfc_msg.msg_length = sizeof(nfc_rd_com_request);
            nfc_msg.msg_type = MSG_ID_NFC_RD_COMMUNICATION_REQ;
            META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
            
            memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), &req->cmd.m_rd_com_req, sizeof(nfc_rd_com_request));
            if ((peer_buff != NULL) && (peer_len != 0))
            {
                memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)+ sizeof(nfc_rd_com_request)), peer_buff, peer_len);          
            } 
            META_NFC_CMD(&nfc_ilm_loc);
            break;
       }
       case NFC_OP_TX_ALWAYSON_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_tx_alwayson_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_TX_ALWAYSON_WO_ACK_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_tx_alwayson_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_CARD_MODE_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_CARD_EMULATION_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_card_emulation_req, sizeof(nfc_card_emulation_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_READER_MODE_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_READER_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_P2P_MODE_TEST: 
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_P2P_MODE_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_SWP_SELF_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_SWP_SELF_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }        
       case NFC_OP_ANTENNA_SELF_TEST:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_request);
           nfc_msg.msg_type = MSG_ID_NFC_ANTENNA_SELF_TEST_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
           
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }
       case NFC_OP_TAG_UID_RW:
       {
           //Write handle function here
           nfc_msg.msg_length = sizeof(nfc_script_uid_request);
           nfc_msg.msg_type = MSG_ID_NFC_TAG_UID_RW_REQ;
           META_LOG("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);           
                
           memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
           memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_uid_req, sizeof(nfc_script_uid_request));
           META_NFC_CMD(&nfc_ilm_loc);           
           break;
       }    
       default:
       {
           nfc_cnf.status = META_FAILED;
           WriteToEM(&nfc_cnf, sizeof(NFC_CNF), NULL, 0);
           break;
       }
    }
    return;
}


