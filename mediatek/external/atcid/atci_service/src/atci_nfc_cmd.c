#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>


#include <ctype.h>
#include <dirent.h>
#include "atci_nfc_cmd.h"
#include "atci_service.h"
#include <at_tok.h>
#include "mtk_nfc_sys_type_ext.h"
#include "mtk_nfc_dynamic_load.h"

NFC_CNF nfc_cnf;
static int nfc_service_sockfd = -1;
static pid_t g_pid  = (-1);

#define MTKNFC_COMM_SOCK    "/data/mtknfc_server"




//query_verison
// nfc_dynamic_load.so
// Run msr_nfc_query_version Check OK or NG
// If NG Run mtk_nfc_query_version Check OK or NG
// Return Value 
//   0x01: 3110 
//   0x02: 6605
//   0xFF: Error
int nfc_dynamicload_queryversion(void)
{ 
    int version = 0xFF;
    int read_length = 0;

    version = query_nfc_chip();
    ALOGD("[NFC_queryVersion],version,%d",version);
    if( (version != 0x01) && (version != 0x02) )
    {      
        version = msr_nfc_get_chip_type();
        if (version != 0x01)
        {
            version = mtk_nfc_get_chip_type();
        }
       
        if((version == 0x1) || (version == 0x02))
        {
            update_nfc_chip(version);
        }
    }
    return version;
}


void clean_up_child_process (int signal_number)
{
   int status;

   ALOGD("clean_up_child_process...(sig_num: %d)\n", signal_number);
   /* Clean up the child process. */
   wait (&status);
   ALOGD("clean_up_child_process status: %d\n", status);

}



/********************************************************************************
//FUNCTION:
//		NFC_6605_Socket_Open
//DESCRIPTION:
//		NFC_Socket_Open for AT command test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
int NFC_6605_Socket_Open(void)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;	
    
    struct sockaddr_un address;
    int len;

    pid_t my_pid, parent_pid, child_pid;
    int ret;   
 
    struct sigaction sigchld_action;
    memset (&sigchld_action, 0, sizeof (sigchld_action));
    sigchld_action.sa_handler = &clean_up_child_process;
    sigaction (SIGCHLD, &sigchld_action, NULL);    
    

     ALOGD("android_nfc_daemon_init...\n");
  
     /* get and print my pid and my parent's pid */
     my_pid = getpid();    
     parent_pid = getppid();
     ALOGD("Parent: my pid is %d\n", my_pid);
     ALOGD("Parent: my parent's pid is %d\n", parent_pid);
  
     /* fork child process to execute nfc daemon */
     if ((child_pid = fork()) < 0)
     {
        ALOGE("fork() fail: %d, (%s)\n", errno, strerror(errno));
        return (-2);
     }
     else if (child_pid == 0) // child process
     {   
        my_pid = getpid();
        parent_pid = getppid();   
        ALOGD("Child Process fork success!!!\n");
        ALOGD("Child: my pid is: %d\n", my_pid);
        ALOGD("Child: my parent's pid is: %d\n", parent_pid);   
        ALOGD("Child Process execl nfcstackp\n");
        ret = execl("/system/xbin/nfcstackp", "nfcstackp", "NFC_TEST_MODE", NULL);        
        if (ret == -1)
        {
           ALOGE("execl() fail: %d, %s\n", errno, strerror(errno));
           return (-3);
        }
     }
     else // parent process
     {
        ALOGD("Parent: my child's pid is: %d\n", child_pid);
        g_pid = child_pid;
     }
     
     ALOGD("android_nfc_daemon_init ok\n");    
    
    /* Create a socket point */

    //nfc_sockfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    nfc_service_sockfd = socket(AF_LOCAL, SOCK_STREAM, 0); 
    if (nfc_service_sockfd < 0)     
    {        
       ALOGE("atcid nfc_open: ERROR opening socket");        
       return (-4);    
    } 

    address.sun_family = AF_LOCAL;
    strcpy (address.sun_path, MTKNFC_COMM_SOCK);
    len = sizeof (address);
            
    sleep(2);  // sleep 5sec for libmnlp to finish initialization        
      
    ALOGD("atcid connecting(%s)...\r\n",address.sun_path);   
            
    /* Now connect to the server */    
    if (connect(nfc_service_sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)     
    {         
       ALOGE("atcid nfc_open: ERROR connecting\r\n");         
       return (-6);    
    }    

//    config Socket read function to non-blocking type
    {
       int x;
       x=fcntl(nfc_service_sockfd,F_GETFL,0);
       fcntl(nfc_service_sockfd,F_SETFL,x | O_NONBLOCK);
    }
  
    ALOGD("atcid nfc_open: success\n"); 
    return 0;  
}



int NFC_6605_Socket_Close(void)
{  
   int ret = 0;
   int err = 0;   
   s_mtk_nfc_main_msg *pnfc_msg = NULL;
  // unsigned char cmd[8];
   
  // memset(cmd , 0x00, sizeof(cmd));
   //Set NEC MEssage
   pnfc_msg = malloc(sizeof(s_mtk_nfc_main_msg));                
   pnfc_msg->msg_type = MTK_NFC_FM_STOP_CMD;
   pnfc_msg->msg_length = 0x00;
   
   ret = write(nfc_service_sockfd, (const char*)pnfc_msg, sizeof(s_mtk_nfc_main_msg) ); 

    if (pnfc_msg != NULL)
    {
        free(pnfc_msg);
        pnfc_msg = NULL;
        
    }

    sleep(3);
    /* Close socket port */
    if (nfc_service_sockfd > 0)
    {
        close (nfc_service_sockfd);
        nfc_service_sockfd = -1;
    }

    // kill service process
    ALOGD("atcid 6605 close: kill: %d\n", g_pid);

    err = kill(g_pid, SIGTERM);
    if (err != 0)
    {
        ALOGE("atcid 6605 kill error: %s\n", strerror(errno));
    }  
    else
    {
        ALOGD("atcid 6605 kill ok,%d\n", err);
    }

    g_pid = -1;
    return 0;
}


int ATCMD_NFC_6605_Send( void* data, unsigned int length)
{
    int ret = 0;

    ALOGD("atcid NFC 6605 Send,%d" , length);

    // Write request command
    if ((length > 0) &&  (data != NULL))
    {
        ret = write(nfc_service_sockfd, (const char*)data, length);
    
        if ( ret <= 0)
        {
            ALOGE("AT_NFC_CMD:write failure,%d",ret);
        }
        else
        {
            ALOGI("AT_NFC_CMD:write CMD done,%d",ret);
        }
    }
    else
    {
        ret = -1;
    }
    return ret;

}

int ATCMD_NFC_6605_Read(unsigned int  type)
{
    int rec_bytes = 0, count = 0, ret = 1;
    unsigned char buf[1024];
    s_mtk_nfc_main_msg *pnfc_msg_ptr = NULL;


    while ( count < 800)
    {                

        memset(buf, 0x00, 1024);
        //ALOGD("atcid 6605 read!!");        
        rec_bytes = read(nfc_service_sockfd, &buf[0], sizeof(buf));     
        //ALOGD("atcid 6605 read,%d",rec_bytes);
        if (rec_bytes > 0)
        {
            pnfc_msg_ptr = (s_mtk_nfc_main_msg *)&buf[0];
            
            if ((pnfc_msg_ptr->msg_type == MTK_NFC_FM_SWP_TEST_RSP) && (type == MTK_NFC_FM_SWP_TEST_RSP))
            {
                s_mtk_nfc_fm_swp_test_rsp* pRspData = NULL;
                
                pRspData = (s_mtk_nfc_fm_swp_test_rsp*)((unsigned char*)pnfc_msg_ptr + sizeof(s_mtk_nfc_main_msg));

                if (pRspData != NULL)
                {
                    ret = pRspData->result;                   
                }
                else
                {
                	  ALOGE("atcid 6605 read,,%d,null",type);
                }
                break;
            }
            else  if ((pnfc_msg_ptr->msg_type == MTK_NFC_FM_READ_DEP_TEST_RSP) && (type == MTK_NFC_FM_READ_DEP_TEST_RSP))
            {
                s_mtk_nfc_em_als_readerm_opt_rsp* pRspData = NULL;
                
                pRspData = (s_mtk_nfc_em_als_readerm_opt_rsp*)((unsigned char*)pnfc_msg_ptr + sizeof(s_mtk_nfc_main_msg));

                if (pRspData != NULL)
                {
                    ret = pRspData->result;                   
                }
                else
                {
                	  ALOGE("atcid 6605 read,,%d,null",type);
                }
                break;
            }
            else  if ((pnfc_msg_ptr->msg_type == MTK_NFC_FM_CARD_MODE_TEST_RSP) && (type == MTK_NFC_FM_CARD_MODE_TEST_RSP))
            {
                s_mtk_nfc_em_als_cardm_rsp* pRspData = NULL;
                
                pRspData = (s_mtk_nfc_em_als_cardm_rsp*)((unsigned char*)pnfc_msg_ptr + sizeof(s_mtk_nfc_main_msg));

                if (pRspData != NULL)
                {
                    ret = pRspData->result;                   
                }
                else
                {
                	  ALOGE("atcid 6605 read,,%d,null",type);
                }                
                break;
            }           
            else  if ((pnfc_msg_ptr->msg_type == MTK_NFC_FM_VIRTUAL_CARD_RSP) && (type == MTK_NFC_FM_VIRTUAL_CARD_RSP))
            {
                s_mtk_nfc_em_virtual_card_rsp* pRspData = NULL;
                
                pRspData = (s_mtk_nfc_em_virtual_card_rsp*)((unsigned char*)pnfc_msg_ptr + sizeof(s_mtk_nfc_main_msg));

                if (pRspData != NULL)
                {
                    ret = pRspData->result;                   
                }
                else
                {
                	  ALOGE("atcid 6605 read,,%d,null",type);
                }                
                break;
            }               
            else
            {
            	 int index = 0, DbgL = 0;
            	 unsigned char DbgBuf[64];
            	 
            	 memset(DbgBuf, 0x00, 64);
            	 for (index = 0; (index< 10); index++)
            	 {
            	 	   sprintf((DbgBuf+DbgL), "0x%x ", buf[index]);
            	 	   DbgL = strlen(DbgBuf);
        }
            	 ALOGE("atcid 6605 read,%d, %s",rec_bytes, DbgBuf);            	 
            }             
        }       
        usleep(10000);  //sleep 10ms for waiting        
        count++;
       // ALOGD("atcid 6605 read,retry,%d,%d,%d",count, ret, type);
    }
    ALOGD("atcid 6605 read,%d,%d,%d",count, ret, type);

    return ret;
}


int ATCMD_NFC_6605_OP(unsigned int  type)
{
    s_mtk_nfc_main_msg *pnfc_msg = NULL;
    unsigned int TotalLength = 0;
    int ret = 0;
    
    switch (type)
    {
        case MTK_NFC_FM_SWP_TEST_REQ:
        {
            s_mtk_nfc_fm_swp_test_req* pReqData = NULL;

            TotalLength = (sizeof(s_mtk_nfc_main_msg) + sizeof(s_mtk_nfc_fm_swp_test_req));

            pnfc_msg = malloc(TotalLength);
            memset(pnfc_msg, 0x00, TotalLength);
            pnfc_msg->msg_type = MTK_NFC_FM_SWP_TEST_REQ;
            pnfc_msg->msg_length = sizeof(s_mtk_nfc_fm_swp_test_req);
            
            pReqData = (s_mtk_nfc_fm_swp_test_req*)((unsigned char*)pnfc_msg + sizeof(s_mtk_nfc_main_msg));

            if (pReqData != NULL)
            {
                #ifdef MTK_NFC_SE_NUM
                ALOGD("atcid 6605 MTK_NFC_SE_NUM,%s",MTK_NFC_SE_NUM);
                pReqData->SEmap = atoi(&MTK_NFC_SE_NUM);
                #else
                pReqData->SEmap = 7;
                #endif
                
                pReqData->action = 1;               
                ret = ATCMD_NFC_6605_Send((void*)pnfc_msg, TotalLength);

                if (ret >= 0)
                {
                    // Read response
                    ret = ATCMD_NFC_6605_Read(MTK_NFC_FM_SWP_TEST_RSP);
                }
                else
                {
                    ALOGD("atcid 6605 send fail,%d,%d",type, TotalLength);
                }
            }
            if (pnfc_msg != NULL)
            {
               free(pnfc_msg);
               pnfc_msg = NULL;
            }            
        }
        break;
        case MTK_NFC_FM_READ_DEP_TEST_REQ:
        {
            s_mtk_nfc_em_als_readerm_req* pReqData = NULL;

            TotalLength = (sizeof(s_mtk_nfc_main_msg) + sizeof(s_mtk_nfc_em_als_readerm_req));
            pnfc_msg = malloc(TotalLength);
            memset(pnfc_msg, 0x00, TotalLength);
            pnfc_msg->msg_type = MTK_NFC_FM_READ_DEP_TEST_REQ;
            pnfc_msg->msg_length = sizeof(s_mtk_nfc_em_als_readerm_req);            
            pReqData = (s_mtk_nfc_em_als_readerm_req*)((unsigned char*)pnfc_msg + sizeof(s_mtk_nfc_main_msg));

            if (pReqData != NULL)
            {
                pReqData->action = 0x00;
                pReqData->supporttype = EM_ALS_READER_M_TYPE_A;
                pReqData->typeA_datarate = (EM_ALS_READER_M_SPDRATE_106 + EM_ALS_READER_M_SPDRATE_212 + EM_ALS_READER_M_SPDRATE_424 + EM_ALS_READER_M_SPDRATE_848);

                ret = ATCMD_NFC_6605_Send((void*)pnfc_msg, TotalLength); 
                if (ret >= 0)
                {
                    // Read response
                    ret = ATCMD_NFC_6605_Read(MTK_NFC_FM_READ_DEP_TEST_RSP);
                }                
                else
                {
                    ALOGD("atcid 6605 send fail,%d,%d",type, TotalLength);
                }                
            }
            if (pnfc_msg != NULL)
            {
               free(pnfc_msg);
               pnfc_msg = NULL;
            }            
        }
        break;

        case MTK_NFC_FM_CARD_MODE_TEST_REQ:
        {
            s_mtk_nfc_em_als_cardm_req* pReqData = NULL;

            TotalLength = (sizeof(s_mtk_nfc_main_msg) + sizeof(s_mtk_nfc_em_als_cardm_req));
            pnfc_msg = malloc(TotalLength);
            memset(pnfc_msg, 0x00, TotalLength);
            pnfc_msg->msg_type = MTK_NFC_FM_CARD_MODE_TEST_REQ;
            pnfc_msg->msg_length = sizeof(s_mtk_nfc_em_als_cardm_req);            
            pReqData = (s_mtk_nfc_em_als_cardm_req*)((unsigned char*)pnfc_msg + sizeof(s_mtk_nfc_main_msg));

            if (pReqData != NULL)
            {
                pReqData->action = 0x00;
                pReqData->SWNum = 1;
                pReqData->supporttype = 0x01; //type A/B/F/B'
                pReqData->fgvirtualcard = 0;

                ret = ATCMD_NFC_6605_Send((void*)pnfc_msg, TotalLength); 

                if (ret >= 0)
                {
                    // Read response
                    ret = ATCMD_NFC_6605_Read(MTK_NFC_FM_CARD_MODE_TEST_RSP);
                }                
                else
                {
                    ALOGD("atcid 6605 send fail,%d,%d",type, TotalLength);
                }                
            }
            if (pnfc_msg != NULL)
            {
               free(pnfc_msg);
               pnfc_msg = NULL;
            }            
        }
        break;

        case MTK_NFC_FM_VIRTUAL_CARD_REQ:
        {
            s_mtk_nfc_em_virtual_card_req* pReqData = NULL;

            TotalLength = (sizeof(s_mtk_nfc_main_msg) + sizeof(s_mtk_nfc_em_virtual_card_req));
            pnfc_msg = malloc(TotalLength);
            memset(pnfc_msg, 0x00, TotalLength);
            pnfc_msg->msg_type = MTK_NFC_FM_VIRTUAL_CARD_REQ;
            pnfc_msg->msg_length = sizeof(s_mtk_nfc_em_virtual_card_req);            
            pReqData = (s_mtk_nfc_em_virtual_card_req*)((unsigned char*)pnfc_msg + sizeof(s_mtk_nfc_main_msg));

            if (pReqData != NULL)
            {
                pReqData->action = 0x00;
                pReqData->supporttype = (EM_ALS_READER_M_TYPE_A  + EM_ALS_READER_M_TYPE_B  + EM_ALS_READER_M_TYPE_F);
                pReqData->typeF_datarate = (EM_ALS_READER_M_SPDRATE_212 + EM_ALS_READER_M_SPDRATE_424);
                
                ret = ATCMD_NFC_6605_Send((void*)pnfc_msg, TotalLength); 

                if (ret >= 0)
                {
                    // Read response
                    ret = ATCMD_NFC_6605_Read(MTK_NFC_FM_VIRTUAL_CARD_RSP);
                }                
                else
                {
                    ALOGD("atcid 6605 send fail,%d,%d",type, TotalLength);
                }                
            }
            if (pnfc_msg != NULL)
            {
               free(pnfc_msg);
               pnfc_msg = NULL;
            }
        }
        break;


        default:
        {
            ALOGE("atcid 6605 not support,%d\n", type);
        }
        break;
         

    }

    return ret;

}
void ATCMD_NFC_Read_CNF(void)
{
    int rec_bytes = 0;
    int retry_count = 0;
    // Read resonse
    ALOGI("ATCMD_NFC_Read_CNF:NFC read start");
    while(retry_count < 5)
    {
        ilm_struct nfc_ilm_rec;
        nfc_msg_struct nfc_msg;
        unsigned char nfc_msg_length;
        unsigned char fgSupport = 1;
        //clean struct buffer
        memset(&nfc_ilm_rec, 0, sizeof(ilm_struct));
        //read fd
        //if get response break
        rec_bytes = read(nfc_service_sockfd,&nfc_ilm_rec, sizeof(ilm_struct));
        ALOGI("retry_count=%d,rec_bytes=%d",retry_count, rec_bytes);
        if (rec_bytes > 0)
        {
            // check msg id
            ALOGI("ATCMD_NFC_Read_CNF:NFC read (msg_id,dest_mod_id) = (%d,%d)",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);

            if ((nfc_ilm_rec.msg_id == MSG_ID_NFC_TEST_RSP) && (nfc_ilm_rec.dest_mod_id == MOD_NFC_APP))
            {
                nfc_msg_length = sizeof(nfc_msg_struct);
                memcpy( &nfc_msg, (nfc_msg_struct*)nfc_ilm_rec.local_para_ptr, nfc_msg_length);
                ALOGI("ATCMD_NFC_Read_CNF:NFC read msg_type=%d,length=%d", nfc_msg.msg_type,nfc_msg_length);
                switch (nfc_msg.msg_type)
                {
#if 0
                case MSG_ID_NFC_SETTING_RSP:
                {
                    nfc_cnf.op = NFC_OP_SETTING;
                    memcpy(&nfc_cnf.result.m_setting_cnf, (nfc_setting_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_setting_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_SETTING =%d/%d/%d/%d/%d/%d/%d/%d/%d/",
                         nfc_cnf.result.m_setting_cnf.card_mode,
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
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/",
                         nfc_cnf.result.m_reg_notify_cnf.status);
                    break;
                }
                case MSG_ID_NFC_SE_SET_RSP:
                {
                    nfc_cnf.op = NFC_OP_SECURE_ELEMENT;
                    memcpy(&nfc_cnf.result.m_se_set_cnf, (nfc_se_set_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_se_set_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_SECURE_ELEMENT =%d/",
                         nfc_cnf.result.m_se_set_cnf.status);
                    break;
                }
                case MSG_ID_NFC_DISCOVERY_RSP:
                {
                    nfc_cnf.op = NFC_OP_DISCOVERY;
                    memcpy(&nfc_cnf.result.m_dis_notify_cnf, (nfc_dis_notif_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_dis_notif_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_DISCOVERY =%d/%d/",
                         nfc_cnf.result.m_dis_notify_cnf.status,
                         nfc_cnf.result.m_dis_notify_cnf.type);
                    break;
                }
                case MSG_ID_NFC_TAG_READ_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_READ;
                    memcpy(&nfc_cnf.result.m_tag_read_cnf, (nfc_tag_read_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_read_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_READ =%d/%d/",
                         nfc_cnf.result.m_tag_read_cnf.status,
                         nfc_cnf.result.m_tag_read_cnf.type);
                    break;
                }
                case MSG_ID_NFC_TAG_WRITE_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_WRITE;
                    memcpy(&nfc_cnf.result.m_tag_write_cnf, (nfc_tag_write_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_write_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_WRITE =%d/%d/",
                         nfc_cnf.result.m_tag_write_cnf.status,
                         nfc_cnf.result.m_tag_write_cnf.type);
                    break;
                }
                case MSG_ID_NFC_TAG_DISCONN_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_DISCONN;
                    memcpy(&nfc_cnf.result.m_tag_discon_cnf, (nfc_tag_disconnect_request*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_disconnect_request));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_DISCONN =%d/",
                         nfc_cnf.result.m_tag_discon_cnf.status);
                    break;
                }
                case MSG_ID_NFC_TAG_F2NDEF_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_FORMAT_NDEF;
                    memcpy(&nfc_cnf.result.m_tag_fromat2Ndef_cnf, (nfc_tag_fromat2Ndef_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_fromat2Ndef_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_FORMAT_NDEF =%d/",
                         nfc_cnf.result.m_tag_fromat2Ndef_cnf.status);
                    break;
                }
                case MSG_ID_NFC_TAG_RAWCOM_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_RAW_COMM;
                    memcpy(&nfc_cnf.result.m_tag_raw_com_cnf, (nfc_tag_raw_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_tag_raw_com_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_RAW_COMM =%d/%d/",
                         nfc_cnf.result.m_tag_raw_com_cnf.status,
                         nfc_cnf.result.m_tag_raw_com_cnf.type);
                    break;
                }
                case MSG_ID_NFC_P2P_COMMUNICATION_RSP:
                {
                    nfc_cnf.op = NFC_OP_P2P_COMM;
                    memcpy(&nfc_cnf.result.m_p2p_com_cnf, (nfc_p2p_com_response*)nfc_ilm_rec.local_para_ptr, sizeof(nfc_p2p_com_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_P2P_COMM =%d/%d/",
                         nfc_cnf.result.m_p2p_com_cnf.status,
                         nfc_cnf.result.m_p2p_com_cnf.length);

                    break;
                }
                case MSG_ID_NFC_RD_COMMUNICATION_RSP:
                {
                    nfc_cnf.op = NFC_OP_RD_COMM;
                    memcpy(&nfc_cnf.result.m_rd_com_cnf, (nfc_rd_com_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_rd_com_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_RD_COMM =%d/%d/",
                         nfc_cnf.result.m_rd_com_cnf.status,
                         nfc_cnf.result.m_rd_com_cnf.length);
                    break;
                }
                case MSG_ID_NFC_TX_ALWAYSON_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_TX_ALWAYSON_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_TX_ALWAYSON_WO_ACK_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TX_ALWAYSON_WO_ACK_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_CARD_EMULATION_MODE_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_CARD_MODE_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_CARD_MODE_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_READER_MODE_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_READER_MODE_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_READER_MODE_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_P2P_MODE_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_P2P_MODE_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_P2P_MODE_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }

#endif
                case MSG_ID_NFC_SWP_SELF_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_SWP_SELF_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_SWP_SELF_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_ANTENNA_SELF_TEST_RSP:
                {
                    nfc_cnf.op = NFC_OP_ANTENNA_SELF_TEST;
                    memcpy(&nfc_cnf.result.m_script_cnf, (nfc_script_response*)(nfc_ilm_rec.local_para_ptr + nfc_msg_length), sizeof(nfc_script_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_ANTENNA_SELF_TEST =%d/",
                         nfc_cnf.result.m_script_cnf.result);
                    break;
                }
                case MSG_ID_NFC_TAG_UID_RW_RSP:
                {
                    nfc_cnf.op = NFC_OP_TAG_UID_RW;
                    memcpy(&nfc_cnf.result.m_script_uid_cnf,
                           (nfc_ilm_rec.local_para_ptr + nfc_msg_length),
                           sizeof(nfc_script_uid_response));
                    ALOGI("AT_NFC_CMD:NFC NFC_OP_TAG_UID_RW =%d/%d/%X/%X/%X/%X/",
                         nfc_cnf.result.m_script_uid_cnf.result,
                         nfc_cnf.result.m_script_uid_cnf.uid_type,
                         nfc_cnf.result.m_script_uid_cnf.data[0],
                         nfc_cnf.result.m_script_uid_cnf.data[1],
                         nfc_cnf.result.m_script_uid_cnf.data[2],
                         nfc_cnf.result.m_script_uid_cnf.data[3]);
                    break;
                }
                case MSG_ID_NFC_CARD_MODE_TEST_RSP:
                case MSG_ID_NFC_STOP_TEST_RSP:
                default:
                {
                    fgSupport = 0;
                    ALOGI("AT_NFC_CMD:Don't support CNF CMD %d",nfc_msg.msg_type);
                    break;
                }
                }
                if (fgSupport == 1)
                {
                    ALOGI("AT_NFC_CMD:NFC read nfc_cnf.op=%d,nfc_msg.msg_type=%d", nfc_cnf.op,nfc_msg.msg_type);
                    nfc_cnf.status = 0;
                    break;
                }
                else
                {
                    ALOGI("AT_NFC_CMD:Don't Write to PC MSGID,%d,",nfc_msg.msg_type);
                }
            }
            else
            {
                ALOGI("AT_NFC_CMD:Don't support MSGID,%d,DestID,%d",nfc_ilm_rec.msg_id, nfc_ilm_rec.dest_mod_id);
            }
        }
        retry_count++;
    }
    return;

}
/********************************************************************************
//FUNCTION:
//		NFC_Socket_Open
//DESCRIPTION:
//		NFC_Socket_Open for AT command test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		true : success
//      false: failed
//
********************************************************************************/
int NFC_Socket_Open(void)
{

    pid_t pid;
    //int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    // Run nfc service process


#if 1
    if ((pid = fork()) < 0)
    {
        ALOGE("NFC_Socket_Open: fork fails: %d (%s)\n", errno, strerror(errno));
        return (-2);
    }
    else if (pid == 0)  /*child process*/
    {
        int err;

        ALOGI("nfc_open: execute: %s\n", "/system/xbin/nfcservice");
        err = execl("/system/xbin/nfcservice", "nfcservice", NULL);
        if (err == -1)
        {
            ALOGE("NFC_Socket_Open: execl error: %s\n", strerror(errno));
            return (-3);
        }
        return 0;
    }
    else  /*parent process*/
    {
        ALOGI("NFC_Socket_Open: pid = %d\n", pid);
    }
#endif
    // Create socket

    nfc_service_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfc_service_sockfd < 0)
    {
        ALOGE("META_NFC_init: ERROR opening socket");
        return (-4);
    }
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        ALOGE("META_NFC_init: ERROR, no such host\n");
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
        ALOGE("META_NFC_init: ERROR connecting");
        return (-6);
    }

#if 0
    ALOGI("META_NFC_init: create read command thread\n");

    if(pthread_create(&read_cnf_thread_handle, NULL, META_NFC_read_cnf,
                      NULL) != 0)
    {
        META_LOG("META_NFC_init:Fail to create read command thread");
        return (-7);
    }
#endif

    ALOGI("META_NFC_init: done\n");
    return (0);
}

/********************************************************************************
//FUNCTION:
//		NFC_Socket_Close
//DESCRIPTION:
//		NFC NFC_Socket_Close for AT CMD test.
//
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//
********************************************************************************/
void NFC_Socket_Close()
{
    int err=0;
#if  0
    /* stop RX thread */
    bStop_ReadThread = 1;

    /* wait until thread exist */
    pthread_join(read_cnf_thread_handle, NULL);
#endif
    /* Close socket port */
    if (nfc_service_sockfd > 0)
    {
        close (nfc_service_sockfd);
        nfc_service_sockfd = -1;
    }

#if 1
    // kill service process
    ALOGI("NFC_Socket_Close: kill: %s\n", "/system/xbin/nfcservice");
    err = execl("kill /system/xbin/nfcservice", "nfcservice", NULL);
    if (err == -1)
    {
        ALOGE("META_NFC_init: kill error: %s\n", strerror(errno));
    }
#endif
    return;
}
/********************************************************************************
//FUNCTION:
//		AT_NFC_CMD
//DESCRIPTION:
//		SEND MESSAGE to NFC driver
//      RECEIVE MESSAGE to NFC driver
//PARAMETERS:
//		void
//RETURN VALUE:
//		void
//
********************************************************************************/
void AT_NFC_CMD(ilm_struct* nfc_ilm_req_ptr)
{

    int ret = 0;
    int rec_bytes = 0;
    int rety_count = 0;
    ALOGI("AT_NFC_CMD:write CMD");

    // Write request command
    ret = write(nfc_service_sockfd, (const char*)nfc_ilm_req_ptr, sizeof(ilm_struct));

    if ( ret <= 0)
    {
        ALOGE("AT_NFC_CMD:write failure,%d",ret);
        return;
    }
    else
    {
        ALOGI("AT_NFC_CMD:write CMD done,%d",ret);
    }
    return;
}


/********************************************************************************
//FUNCTION:
//		ATCMD_NFC_OP
//DESCRIPTION:
//		ATCMD NFC test main process function.
//
//PARAMETERS:
//		req: NFC Req struct
//RETURN VALUE:
//		void
//
********************************************************************************/
void ATCMD_NFC_OP(NFC_REQ *req)
{
    ilm_struct nfc_ilm_loc;
    nfc_msg_struct nfc_msg;
    memset(&nfc_cnf, 0, sizeof(NFC_CNF));
    memset(&nfc_msg, 0, sizeof(nfc_msg_struct));
    nfc_cnf.op = req->op;

    memset(&nfc_ilm_loc, 0, sizeof(ilm_struct));
    nfc_ilm_loc.msg_id = MSG_ID_NFC_TEST_REQ;
    nfc_ilm_loc.src_mod_id = MOD_NFC_APP;
    nfc_ilm_loc.dest_mod_id = MOD_NFC;

    switch(req->op)
    {
        ALOGI("ATCMD_NFC_OP:NFC request op=%d", req->op);
#if 0
    case NFC_OP_SETTING:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_setting_request);
        nfc_msg.msg_type = MSG_ID_NFC_SETTING_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), (char*)&req->cmd.m_setting_req, sizeof(nfc_setting_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_REG_NOTIFY:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_reg_notif_request);
        nfc_msg.msg_type = MSG_ID_NFC_NOTIFICATION_REQ;

        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),(char*)&req->cmd.m_reg_notify_req, sizeof(nfc_reg_notif_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_SECURE_ELEMENT:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_se_set_request);
        nfc_msg.msg_type = MSG_ID_NFC_SE_SET_REQ;

        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);
        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_se_set_req, sizeof(nfc_se_set_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_DISCOVERY:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_dis_notif_request);
        nfc_msg.msg_type = MSG_ID_NFC_DISCOVERY_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_dis_notify_req, sizeof(nfc_dis_notif_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TAG_READ:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_tag_read_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_READ_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_read_req, sizeof(nfc_tag_read_request));
        break;
    }
    case NFC_OP_TAG_WRITE:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_tag_write_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_WRITE_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_write_req, sizeof(nfc_tag_write_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TAG_DISCONN:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_tag_disconnect_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_DISCONN_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_discon_req, sizeof(nfc_tag_disconnect_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TAG_FORMAT_NDEF:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_tag_fromat2Ndef_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_F2NDEF_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_fromat2Ndef_req, sizeof(nfc_tag_fromat2Ndef_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TAG_RAW_COMM:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_tag_raw_com_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_RAWCOM_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_tag_raw_com_req, sizeof(nfc_tag_raw_com_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_P2P_COMM:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_p2p_com_request);
        nfc_msg.msg_type = MSG_ID_NFC_P2P_COMMUNICATION_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_p2p_com_req, sizeof(nfc_p2p_com_request));
        if ((peer_buff != NULL) && (peer_len != 0))
        {
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct) + sizeof(nfc_p2p_com_request)), peer_buff, peer_len);
        }
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_RD_COMM:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_rd_com_request);
        nfc_msg.msg_type = MSG_ID_NFC_RD_COMMUNICATION_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)), &req->cmd.m_rd_com_req, sizeof(nfc_rd_com_request));
        if ((peer_buff != NULL) && (peer_len != 0))
        {
            memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)+ sizeof(nfc_rd_com_request)), peer_buff, peer_len);
        }
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TX_ALWAYSON_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_tx_alwayson_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_TX_ALWAYSON_WO_ACK_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_TX_ALWAYSON_WO_ACK_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_nfc_tx_alwayson_req, sizeof(nfc_card_emulation_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_CARD_MODE_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_CARD_EMULATION_MODE_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_READER_MODE_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_READER_MODE_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
    case NFC_OP_P2P_MODE_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_P2P_MODE_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        break;
    }
#endif
    case NFC_OP_SWP_SELF_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_SWP_SELF_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        ATCMD_NFC_Read_CNF();
        break;
    }
    case NFC_OP_ANTENNA_SELF_TEST:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_request);
        nfc_msg.msg_type = MSG_ID_NFC_ANTENNA_SELF_TEST_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_req, sizeof(nfc_script_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        ATCMD_NFC_Read_CNF();
        break;
    }
    case NFC_OP_TAG_UID_RW:
    {
        //Write handle function here
        nfc_msg.msg_length = sizeof(nfc_script_uid_request);
        nfc_msg.msg_type = MSG_ID_NFC_TAG_UID_RW_REQ;
        ALOGI("META_NFC_OP:NFC msg_type,msg_length = (%d,%d)", nfc_msg.msg_type, nfc_msg.msg_length);

        memcpy(nfc_ilm_loc.local_para_ptr, (char*)&nfc_msg, sizeof(nfc_msg_struct));
        memcpy((nfc_ilm_loc.local_para_ptr + sizeof(nfc_msg_struct)),&req->cmd.m_script_uid_req, sizeof(nfc_script_uid_request));
        AT_NFC_CMD(&nfc_ilm_loc);
        ATCMD_NFC_Read_CNF();
        break;
    }
    default:
    {
        nfc_cnf.status = 1;
        break;
    }
    }
    return;
}


/*
  AT%NFC=<option>,[<param1>[,<param2>[,<param3>,[param4]]]]

     Option:
                    0: SWP self test
                    1: NFC antenna self test
                    2: NFC read uuid 
    Parameter number is according to <option>

    For SWP self test & NFC antenna self test, there will be <param1> and  <param2>.(all integer type)
    For NFC Tag Uid read/write, there will be <param1>,<param2>,<param3> and <param4>( param4 will be string type)
    For NFC Tag Uid card Mode, is still wait for LiangChi¡¦s comment
*/

// Handle AT command from ATCI service
// char*  request: the AT command for NFC, such as AT+ENFC
// char** response: the response of AT command
// return value: 1 means OK; 3 means response



int nfc_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{

    char* cmd_ptr = cmdline;
    NFC_REQ nfc_req;
    int  ReqCMD[3];
    int  actinoID = 0;
    int  Version = 0x00;
    int  ret = 0;

    Version = nfc_dynamicload_queryversion();
    ALOGD("nfc_cmd_handler at_op:%d,%d", at_op, Version);    
    if (Version == 0x02) // 6605
    {
        ret = NFC_6605_Socket_Open();
        if (ret >= 0)
        {

            switch(at_op)
            {
                case AT_NONE_OP:
                case AT_BASIC_OP:
                case AT_ACTION_OP:
                case AT_READ_OP:
                case AT_TEST_OP:
                {
                  sprintf(response,"\r\nOK\r\n");
                }
                break;
                case AT_SET_OP:
                {
                    at_tok_nextint(&cmdline, &actinoID);
                    ALOGD("nfc_cmd_handler action ID:%d", actinoID);
                    if ( (actinoID < 0) || (actinoID > 3) ) 
                    {
                        sprintf(response,"\r\nNFC ERROR\r\n");
                    }
                    else 
                    {

                        if (actinoID == 0)
                        {
                            ret = ATCMD_NFC_6605_OP(MTK_NFC_FM_SWP_TEST_REQ);
                        }
                        else if (actinoID == 1)
                        {
                            ret = ATCMD_NFC_6605_OP(MTK_NFC_FM_READ_DEP_TEST_REQ);
                        }
                        else if (actinoID == 2)
                        {
                            ret = ATCMD_NFC_6605_OP(MTK_NFC_FM_CARD_MODE_TEST_REQ);
                        }
                        else if (actinoID == 3)
                        {
                            ret = ATCMD_NFC_6605_OP(MTK_NFC_FM_VIRTUAL_CARD_REQ);
                        }
                        
                    } 
                }
                break;
            }
        }
        if ( ret == 0)
        {
            snprintf(response, 2048, "NFC=%d,%d\r\n", (unsigned char)actinoID, ret);                        
        }
        else
        {
            snprintf(response, 2048, "NFC=%d,Failure,%d\r\n", (unsigned char)actinoID, ret);   
        }        
        ret = NFC_6605_Socket_Close();
    }
    else if (Version == 0x01) // 3110
    {    
        memset(&nfc_req, 0, sizeof(nfc_req));
        memset(ReqCMD, 0, sizeof(ReqCMD));
        NFC_Socket_Open();
        ALOGI ("handleNfcCommand");
    
        switch(at_op)
        {
            case AT_NONE_OP:
            case AT_BASIC_OP:
            case AT_ACTION_OP:
            case AT_READ_OP:
            case AT_TEST_OP:
            {
              sprintf(response,"\r\nOK\r\n");
            }
            break;
            case AT_SET_OP:
            {
                at_tok_nextint(&cmdline, &actinoID);
                ALOGD("nfc_cmd_handler action ID:%d", actinoID);
                if ( (actinoID < 0) || (actinoID > 2) ) 
                {
                    sprintf(response,"\r\nNFC ERROR\r\n");
                }
                else if (actinoID == 0)
                {
                    nfc_req.op = NFC_OP_SWP_SELF_TEST;
                    nfc_req.cmd.m_script_req.type   = 1;
                    nfc_req.cmd.m_script_req.action = 1;
                    ATCMD_NFC_OP(&nfc_req);
                    if (nfc_cnf.op == NFC_OP_SWP_SELF_TEST)
                    {
                       snprintf(response, 2048, "NFC=0,%d",nfc_cnf.result.m_script_cnf.result);
                    }
                    else
                    {
                       snprintf(response, 2048, "NFC=0,Response Failure");
                    }
                }
                else if (actinoID == 1)
                {
                    nfc_req.op = NFC_OP_ANTENNA_SELF_TEST;
                    nfc_req.cmd.m_script_req.type   = 1;
                    nfc_req.cmd.m_script_req.action = 1;
                    ATCMD_NFC_OP(&nfc_req);
                    if (nfc_cnf.op == NFC_OP_ANTENNA_SELF_TEST)
                    {
                        snprintf(response, 2048, "NFC=1,%d",nfc_cnf.result.m_script_cnf.result);
                    }
                    else
                    {
                        snprintf(response, 2048, "NFC=1,Response Failure");
                    }
                }
                else if(actinoID == 2)
                {
                    nfc_req.op = NFC_OP_TAG_UID_RW;
                    nfc_req.cmd.m_script_uid_req.type   = 1;
                    nfc_req.cmd.m_script_uid_req.action = 1;
                    nfc_req.cmd.m_script_uid_req.uid_type = 2;
                    ATCMD_NFC_OP(&nfc_req);
                    if (nfc_cnf.op == NFC_OP_TAG_UID_RW)
                    {
                        int BufLength = 0;
                        int UIDLength = 7;
                        int index = 0;
                        snprintf(response, 2048, "NFC=2,%d,%d",
                             nfc_cnf.result.m_script_uid_cnf.result,
                             nfc_cnf.result.m_script_uid_cnf.uid_type);
                        BufLength = strlen(response);
                        if (nfc_cnf.result.m_script_uid_cnf.uid_type == 1)
                        {
                            UIDLength = 4;
                        }
                        for (index=0; index < UIDLength ; index++)
                        {
                            snprintf((response+BufLength), (2048-BufLength), ",%X",
                                 nfc_cnf.result.m_script_uid_cnf.data[index]);
                            BufLength = strlen(response);
                        }
                    }
                    else
                    {
                        snprintf(response, 2048, "NFC=2,Response Failure");
                    }                
                 }
              }
              break;
        }
        NFC_Socket_Close();
    }
    ALOGI ("%s",response);
    return 0;
}