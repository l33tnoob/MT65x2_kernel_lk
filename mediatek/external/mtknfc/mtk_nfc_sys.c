/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2012
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
 *  mtk_nfc_osal.c
 *
 * Project:
 * --------
 *
 * Description:
 * ------------
 *
 * Author:
 * -------
 *  Hiki Chen, ext 25281, hiki.chen@mediatek.com, 2012-05-10
 * 
 *******************************************************************************/
/***************************************************************************** 
 * Include
 *****************************************************************************/ 
#ifdef WIN32
#include <windows.h>
#include <assert.h>
#endif

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <sys/un.h>

#include <utils/Log.h> // For Debug

#include "mtk_nfc_sys_type.h"
#include "mtk_nfc_sys.h"


/***************************************************************************** 
 * Define
 *****************************************************************************/
//#define USE_SIGNAL_EVENT_TO_TIMER_CREATE

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

#ifdef DEBUG_LOG

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "NFC-MW"
#endif
    
#define NFCD(...)    ALOGD(__VA_ARGS__)
#define NFCW(...)    ALOGW(__VA_ARGS__)
#define NFCE(...)    ALOGE(__VA_ARGS__)
#else
#define NFCD(...)
#define NFCW(...)
#define NFCE(...)
#endif

/***************************************************************************** 
 * Data Structure
 *****************************************************************************/
#ifdef WIN32
typedef struct
{
    CRITICAL_SECTION    cs;
    BOOL                is_used;    // 1 = used; 0 = unused    
    UINT32              timer_id;   //timer's id returned from SetTimer()
    ppCallBck_t         timer_expiry_callback;
    VOID                *timer_expiry_context;
} nfc_timer_table_struct;
#else // for linux
typedef struct
{
    BOOL                is_used;    // 1 = used; 0 = unused
    timer_t             handle;     // system timer handle
    ppCallBck_t         timer_expiry_callback; // timeout callback
    VOID                *timer_expiry_context; // timeout callback context    
    BOOL                is_stopped;    // 1 = stopped; 0 = running    
} nfc_timer_table_struct;
#endif


/***************************************************************************** 
 * Extern Area
 *****************************************************************************/ 
extern int gconn_fd_tmp;
extern int gInterfaceHandle;

#ifdef SUPPORT_BLOCKING_READ_MECHANISM
#define NFC_MSG_HDR_SIZE        sizeof(MTK_NFC_MSG_T)
#endif

/***************************************************************************** 
 * GLobal Variable
 *****************************************************************************/ 
// timer pool
static nfc_timer_table_struct nfc_timer_table[MTK_NFC_TIMER_MAX_NUM];

static pthread_mutex_t g_hMutex[MTK_NFC_MUTEX_MAX_NUM];

#define MTK_NFC_MSG_RING_SIZE 128

MTK_NFC_MSG_RING_BUF nfc_main_msg_ring_body;
MTK_NFC_MSG_RING_BUF * nfc_main_msg_ring = NULL;
MTK_NFC_MSG_T * nfc_main_msg_ring_buffer[MTK_NFC_MSG_RING_SIZE]; //pointer array
INT32 nfc_main_msg_cnt;

MTK_NFC_MSG_RING_BUF nfc_service_msg_ring_body;
MTK_NFC_MSG_RING_BUF * nfc_service_msg_ring = NULL;
MTK_NFC_MSG_T * nfc_service_msg_ring_buffer[MTK_NFC_MSG_RING_SIZE]; //pointer array
INT32 nfc_service_msg_cnt;


/***************************************************************************** 
 * Function
 *****************************************************************************/ 
VOID *mtk_nfc_sys_i2c_init(INT32 type);
INT32 mtk_nfc_sys_i2c_read(UINT8* data, UINT16 len);    
INT32 mtk_nfc_sys_i2c_write(UINT8* data, UINT16 len);
VOID mtk_nfc_sys_i2c_uninit(VOID *);

#ifdef SUPPORT_BLOCKING_READ_MECHANISM
static INT32 mtk_nfc_sys_socket_send(INT32 sockfd, MTK_NFC_MSG_T *p_msg)
{
   INT32 i4SendLen;

   NFCD("mtk_nfc_sys_socket_send...\n");

   if (p_msg == NULL)
   {
      NFCE("socket send fail: due to msg == null\n");
      return -1;
   }

   if (sockfd < 0)
   {
      NFCW("socket send msg fail: due to invalid sockfd: %d\n", sockfd);
      return -1;
   }

   NFCD("socket send msg: type %d, length, %d\n", p_msg->type, p_msg->length);
   
   /* send data to nfc daemon */
   i4SendLen = send(sockfd, (void *)p_msg, sizeof(MTK_NFC_MSG_T) + p_msg->length ,0);
   if (i4SendLen < 0)
   {
      NFCW("socket send fail: %d, %s\n", errno, strerror(errno));
   }
   else
   {
      NFCD("mtk_nfc_sys_socket_send ok (send len: %d)\n", i4SendLen);
   }
   
   return i4SendLen;
}

static INT32 mtk_nfc_sys_socket_read(INT32 sockfd, UINT8 *pRecvBuff, INT32 i4RecvLen)
{
   INT32 i4ReadLen;
   INT32 i4TotalReadLen;
    
   NFCD("mtk_nfc_sys_socket_read...(fd:%d, pRecvBuff:0x%x, i4RecvLen:%d)\n", sockfd, (UINT32)pRecvBuff, i4RecvLen);
       
   if (sockfd < 0)
   {
      NFCE("socket recv msg fail: due to invalid sockfd: %d\n", sockfd);
      return -1;
   }   
    
   /* read data to nfc daemon */
   i4TotalReadLen = 0;
   while (i4TotalReadLen < i4RecvLen)
   {
      i4ReadLen = read(sockfd, pRecvBuff, i4RecvLen - i4TotalReadLen);
      NFCD("i4ReadLen (%d)\n", i4ReadLen);         
      if (i4ReadLen < 0)
      {
         NFCW("socket read fail: %d, %s\n", errno, strerror(errno));
         i4TotalReadLen = i4ReadLen; // keep read fail return value
         break; // exit loop
      }
      else if (i4ReadLen == 0)
      {
         NFCW("socket read fail due to socket be closed\n");
         i4TotalReadLen = i4ReadLen; // keep read fail return value
         break; // exit loop
      }
      else
      {
         i4TotalReadLen += i4ReadLen;
      }   
      
      NFCD("mtk_nfc_sys_socket_read ok (read len: %d, target len: %d)\n", i4TotalReadLen, i4RecvLen);      
   }

   return i4TotalReadLen;
}

int mtk_nfc_sys_socket_recv_msg(INT32 sockfd, MTK_NFC_MSG_T **p_msg)
{  
   int i4ReadLen = 0;
   MTK_NFC_MSG_T msg_hdr;
   void *p_msg_body;
   unsigned char *pBuff;

   NFCD("mtk_nfc_sys_socket_recv_msg...\n");

   // read msg header (blocking read)
   pBuff = (unsigned char *)&msg_hdr;
   i4ReadLen = mtk_nfc_sys_socket_read(sockfd, pBuff, NFC_MSG_HDR_SIZE);
   if (i4ReadLen <= 0) // error case
   {         
      return MTK_NFC_ERROR;
   }
   else if (NFC_MSG_HDR_SIZE != i4ReadLen)
   {
      NFCD("unexpected length (hdr len: %d, read len: %d)\n", NFC_MSG_HDR_SIZE, i4ReadLen);
      return MTK_NFC_ERROR;
   }
   else
   {
      NFCD("msg hdr (type: %d, len: %d)\n", msg_hdr.type, msg_hdr.length);
       
      // malloc msg
      *p_msg = (MTK_NFC_MSG_T *)mtk_nfc_sys_mem_alloc(NFC_MSG_HDR_SIZE + msg_hdr.length);
      if (*p_msg == NULL)
      {
         NFCD("malloc fail\n");
         return MTK_NFC_ERROR;
      }   

      // fill type & length 
      memcpy((unsigned char *)*p_msg, (unsigned char *)&msg_hdr, NFC_MSG_HDR_SIZE); 
   }
  
   // read msg body (blocking read)
   if (msg_hdr.length > 0)
   {
      p_msg_body = (unsigned char *)*p_msg + NFC_MSG_HDR_SIZE;
      pBuff = (unsigned char *)p_msg_body;
      i4ReadLen = mtk_nfc_sys_socket_read(sockfd, pBuff, msg_hdr.length);
      if (i4ReadLen <= 0) // error case
      {
         NFCD("read error (%d)\n", i4ReadLen);
         mtk_nfc_sys_mem_free(*p_msg);
         *p_msg = NULL;
         return MTK_NFC_ERROR;
      }
      else if (msg_hdr.length != (UINT32)i4ReadLen)
      {      
         NFCD("unexpected length (body len: %d, read len %d)\n", msg_hdr.length, i4ReadLen);      
         mtk_nfc_sys_mem_free(*p_msg);
         *p_msg = NULL;
         return MTK_NFC_ERROR;
      }
   }

   NFCD("mtk_nfc_sys_socket_recv_msg ok\n");
   
   return MTK_NFC_SUCCESS;
   }

#endif

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mem_alloc
 * DESCRIPTION
 *  Allocate a block of memory
 * PARAMETERS
 *  size [IN] the length of the whole memory to be allocated
 * RETURNS
 *  On success, return the pointer to the allocated memory
 * NULL (0) if failed
 *****************************************************************************/ 
VOID *
mtk_nfc_sys_mem_alloc (
    UINT32 u4Size
)
{
    void *pMem = NULL;

    if (u4Size != 0)
    {
        pMem = malloc(u4Size);
    }
    
    return pMem;

}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mem_free
 * DESCRIPTION
 *  Release unused memory
 * PARAMETERS
 *  pMem        [IN] the freed memory address
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID 
mtk_nfc_sys_mem_free (
    VOID *pMem
)
{
   if (pMem != NULL)
   {
       free(pMem);       
       pMem=NULL;
   }

   return;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_initialize
 * DESCRIPTION
 *  mutex initialization
 * PARAMETERS
 *  void
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_initialize (
    void
)
{
    INT8 index;

    for (index = 0; index < MTK_NFC_MUTEX_MAX_NUM; index++)
    {
        pthread_mutex_init(&g_hMutex[index], NULL);
    }

    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_create
 * DESCRIPTION
 *  Create a mutex object
 * PARAMETERS
 *  mutex_id    [IN] mutex index used by NFC library
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_create (
    MTK_NFC_MUTEX_E mutex_id
)
{
    if (mutex_id >= MTK_NFC_MUTEX_MAX_NUM)
    {
        return MTK_NFC_ERROR;
    }

    pthread_mutex_init(&g_hMutex[mutex_id], NULL);
    
    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_take
 * DESCRIPTION
 *  Request ownership of a mutex and if it's not available now, then block the
 *  thread execution
 * PARAMETERS
 *  mutex_id    [IN] mutex index used by NFC library
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_take (
    MTK_NFC_MUTEX_E mutex_id
)
{
    pthread_mutex_lock(&g_hMutex[mutex_id]);
    
    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_give
 * DESCRIPTION
 *  Release a mutex ownership
 * PARAMETERS
 *  mutex_id    [IN] mutex index used by NFC library
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_give (
    MTK_NFC_MUTEX_E mutex_id
)
{
    pthread_mutex_unlock(&g_hMutex[mutex_id]);
    
    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_destory
 * DESCRIPTION
 *  Destory a mutex object
 * PARAMETERS
 *  mutex_id    [IN] mutex index used by NFC library
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_destory (
    MTK_NFC_MUTEX_E mutex_id
)
{
    if (pthread_mutex_destroy(&g_hMutex[mutex_id]))
    {
        return MTK_NFC_ERROR;
    }
    
    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_mutex_deinitialize
 * DESCRIPTION
 *  mutex deinitialization
 * PARAMETERS
 *  void
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_mutex_deinitialize (
    void
)
{
    INT8 index;

    for (index = 0; index < MTK_NFC_MUTEX_MAX_NUM; index++)
    {
        if (pthread_mutex_destroy(&g_hMutex[index]))
        {
            return MTK_NFC_ERROR;
        }
    }

    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_msg_alloc
 * DESCRIPTION
 *  Allocate a block of memory for message
 * PARAMETERS
 *  u2Size      [IN] the length of the whole MTK_NFC_MSG structure
 * RETURNS
 *  Pinter to the created message if successed
 *  NULL (0) if failed
 *****************************************************************************/ 
MTK_NFC_MSG_T *
mtk_nfc_sys_msg_alloc (
    UINT16 u2Size
)
{
    return mtk_nfc_sys_mem_alloc(u2Size);
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_msg_initialize
 * DESCRIPTION
 *  Send a message to a task
 * PARAMETERS
 *  task_id     [IN] target task id
 *  msg         [IN] the send message
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_msg_initialize (
    void
)
{
    //For Main Message
    nfc_main_msg_ring = &nfc_main_msg_ring_body;
    nfc_main_msg_ring->start_buffer = &nfc_main_msg_ring_buffer[0];
    nfc_main_msg_ring->end_buffer = &nfc_main_msg_ring_buffer[MTK_NFC_MSG_RING_SIZE-1];
    nfc_main_msg_ring->next_write = nfc_main_msg_ring->start_buffer;
    nfc_main_msg_ring->next_read = nfc_main_msg_ring->start_buffer;
    nfc_main_msg_cnt = 0;

    //For Service Message
    nfc_service_msg_ring = &nfc_service_msg_ring_body;
    nfc_service_msg_ring->start_buffer = &nfc_service_msg_ring_buffer[0];
    nfc_service_msg_ring->end_buffer = &nfc_service_msg_ring_buffer[MTK_NFC_MSG_RING_SIZE-1];
    nfc_service_msg_ring->next_write = nfc_service_msg_ring->start_buffer;
    nfc_service_msg_ring->next_read = nfc_service_msg_ring->start_buffer;
    nfc_service_msg_cnt = 0;

    return MTK_NFC_SUCCESS;
}


/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_msg_send
 * DESCRIPTION
 *  Send a message to a task
 * PARAMETERS
 *  task_id     [IN] target task id
 *  msg         [IN] the send message
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_msg_send (
    MTK_NFC_TASKID_E task_id, 
    const MTK_NFC_MSG_T *msg
)
{
    NFCD("Send message type:%d\n",task_id);
  
    if (msg == NULL)
    {
        NFCD("Send message full:%d\n",task_id);
        return MTK_NFC_ERROR;
    }

    if (MTK_NFC_TASKID_MAIN == task_id)
    {
        mtk_nfc_sys_mutex_take(MTK_MUTEX_MSG_Q);
        
        /* msg queue full check */
        if (nfc_main_msg_cnt == MTK_NFC_MSG_RING_SIZE)
        {
            mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
            NFCD("Send message to main, full\n");
            return MTK_NFC_ERROR;
        }
        
        if ( nfc_main_msg_ring != NULL)
        {
            *(nfc_main_msg_ring->next_write) = (MTK_NFC_MSG_T*)msg;
        
            nfc_main_msg_ring->next_write++;
        
            // Wrap check the input circular buffer
            if ( nfc_main_msg_ring->next_write > nfc_main_msg_ring->end_buffer )
            {
                nfc_main_msg_ring->next_write = nfc_main_msg_ring->start_buffer;
            }
        
            nfc_main_msg_cnt++;
            
            #ifdef SUPPORT_BLOCKING_READ_MECHANISM
            NFCD("mtk_nfc_sys_event_set...2MAIN\n");
            if (MTK_NFC_SUCCESS != mtk_nfc_sys_event_set(MTK_NFC_EVENT_2MAIN))
            {
                NFCD("mtk_nfc_sys_event_set,2MAIN,fail\n");
                mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
                return MTK_NFC_ERROR;
            }
            #endif
            
            mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);            
            return MTK_NFC_SUCCESS;
        }
        else
        {
            mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
            return MTK_NFC_ERROR;
        }
    }
    else if (MTK_NFC_TASKID_SERVICE == task_id)
    {
        mtk_nfc_sys_mutex_take(MTK_MUTEX_SERVICE_MSG_Q);
        
         /* msg queue full check */
        if (nfc_service_msg_cnt == MTK_NFC_MSG_RING_SIZE)
        {
            mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
            NFCD("Send message to service, full\n");
            return MTK_NFC_ERROR;
        }
        
        if ( nfc_service_msg_ring != NULL)
        {
            *(nfc_service_msg_ring->next_write) = (MTK_NFC_MSG_T*)msg;
        
            nfc_service_msg_ring->next_write++;
        
            // Wrap check the input circular buffer
            if ( nfc_service_msg_ring->next_write > nfc_service_msg_ring->end_buffer )
            {
                nfc_service_msg_ring->next_write = nfc_service_msg_ring->start_buffer;
            }
        
            nfc_service_msg_cnt++;
            
            #ifdef SUPPORT_BLOCKING_READ_MECHANISM
            NFCD("mtk_nfc_sys_event_set...2SERV\n");
            if (MTK_NFC_SUCCESS != mtk_nfc_sys_event_set(MTK_NFC_EVENT_2SERV))
            {
                NFCD("mtk_nfc_sys_event_set,2SERV,fail\n");
                mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
                return MTK_NFC_ERROR;
            }
            #endif
        
            mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);            
            return MTK_NFC_SUCCESS;
        }
        else
        {
            mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
            NFCD("Send message to service, fail, null\n");
            return MTK_NFC_ERROR;
        }

    }
    else if (MTK_NFC_TASKID_SOCKET == task_id)
    {
        INT32 ret;
        
        ret = write(gconn_fd_tmp, msg, (sizeof(MTK_NFC_MSG_T) + msg->length)); 

        NFCD("mtk_nfc_sys_msg_send: ret,%d\n",ret); 
        
        mtk_nfc_sys_mem_free( (VOID*)msg);

        return MTK_NFC_SUCCESS;

    }
    else
    {
        return MTK_NFC_ERROR;
    }

    return MTK_NFC_ERROR;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_msg_recv
 * DESCRIPTION
 *  Recv a message from a task
 * PARAMETERS
 *  task_id     [IN] target task id
 *  msg         [IN] the receive message pointer
 * RETURNS
 *  MTK_NFC_SUCCESS
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_msg_recv (
    MTK_NFC_TASKID_E task_id, 
    MTK_NFC_MSG_T **msg
)
{
    if (msg != NULL)
    {
        if (MTK_NFC_TASKID_MAIN == task_id)
        {
             mtk_nfc_sys_mutex_take(MTK_MUTEX_MSG_Q);

             /* wait signal if no msg in queue */
             if ( nfc_main_msg_cnt <= 0 )
             {   
                 #ifdef SUPPORT_BLOCKING_READ_MECHANISM
                 NFCD("mtk_nfc_sys_event_wait...2MAIN\n");
                 if (MTK_NFC_SUCCESS != mtk_nfc_sys_event_wait(MTK_NFC_EVENT_2MAIN, MTK_MUTEX_MSG_Q))
                 {
                     NFCD("mtk_nfc_sys_event_wait,2MAIN,fail\n");
                     mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
                     return MTK_NFC_ERROR;
                 }
                 else
                 {
                     NFCD("mtk_nfc_sys_event_wait,ok,2MAIN,cnt,%d\n", nfc_main_msg_cnt);                 
                 }
                 #else
                 mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);                 
                 return MTK_NFC_ERROR;
                 #endif
             } 
             
             if (*(nfc_main_msg_ring->next_read) == NULL)
             {
                 mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
                 return MTK_NFC_ERROR;
             }    
            (*msg) = *(nfc_main_msg_ring->next_read);
            
            
            nfc_main_msg_ring->next_read++;
            
            // Wrap check output circular buffer
            if ( nfc_main_msg_ring->next_read > nfc_main_msg_ring->end_buffer )
            {
                nfc_main_msg_ring->next_read = nfc_main_msg_ring->start_buffer;
            }
            
            nfc_main_msg_cnt--;

            mtk_nfc_sys_mutex_give(MTK_MUTEX_MSG_Q);
            
            return MTK_NFC_SUCCESS;
        }
        else if(MTK_NFC_TASKID_SERVICE == task_id)
        {
             mtk_nfc_sys_mutex_take(MTK_MUTEX_SERVICE_MSG_Q);

             /* wait signal if no msg in queue */
             if ( nfc_service_msg_cnt <= 0 )
             {   
                 #ifdef SUPPORT_BLOCKING_READ_MECHANISM
                 NFCD("mtk_nfc_sys_event_wait...2SERV\n");
                 if (MTK_NFC_SUCCESS != mtk_nfc_sys_event_wait(MTK_NFC_EVENT_2SERV, MTK_MUTEX_SERVICE_MSG_Q))
                 {
                     NFCD("mtk_nfc_sys_event_wait,2SERV,fail\n");
                     mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
                     return MTK_NFC_ERROR;
                 }
                 else
                 {
                     NFCD("mtk_nfc_sys_event_wait,ok,2SERV,cnt,%d\n", nfc_service_msg_cnt);                 
                 }
                 #else
                 mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);                 
                 return MTK_NFC_ERROR;
                 #endif                 
             }
             
             if (*(nfc_service_msg_ring->next_read) == NULL)
             {
                 mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
                 return MTK_NFC_ERROR;
             }    
            (*msg) = *(nfc_service_msg_ring->next_read);
            
            
            nfc_service_msg_ring->next_read++;
            
            // Wrap check output circular buffer
            if ( nfc_service_msg_ring->next_read > nfc_service_msg_ring->end_buffer )
            {
                nfc_service_msg_ring->next_read = nfc_service_msg_ring->start_buffer;
            }
            
            nfc_service_msg_cnt--;

            mtk_nfc_sys_mutex_give(MTK_MUTEX_SERVICE_MSG_Q);
            
            return MTK_NFC_SUCCESS;
        }
        #ifdef SUPPORT_BLOCKING_READ_MECHANISM
        else if(MTK_NFC_TASKID_SOCKET == task_id)
        {                        
            return mtk_nfc_sys_socket_recv_msg(gconn_fd_tmp, msg);
        }
        #endif
        else
        {
           return MTK_NFC_ERROR;
        }

   }
   else
   {
       return MTK_NFC_ERROR;
   }
    return MTK_NFC_SUCCESS;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_msg_free
 * DESCRIPTION
 *  Free a block of memory for message
 * PARAMETERS
 *  msg         [IN] the freed message
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID 
mtk_nfc_sys_msg_free (
    MTK_NFC_MSG_T *msg
)
{
    mtk_nfc_sys_mem_free(msg);
}


/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_dbg_string
 * DESCRIPTION
 *  Output a given string
 * PARAMETERS
 *  pString     [IN] pointer to buffer content to be displayed
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID
mtk_nfc_sys_dbg_string (
    const CH *pString
)
{
    NFCD("%s", pString);
    return;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_dbg_trace
 * DESCRIPTION
 *  Output the traced raw data
 * PARAMETERS
 *  pString     [IN] data Data block
 *  length      [IN] size buffer size of the data block
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID
mtk_nfc_sys_dbg_trace (
    UINT8   pData[], 
    UINT32  u4Len
)
{
    UINT32 i;
    
    for (i = 0; i < u4Len; i++)
    {
        NFCD("%02X,",*(pData+i));
    }

    return;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_sleep
 * DESCRIPTION
 *  task sleep funciton
 * PARAMETERS
 *  pString     [IN] data Data block
 *  length      [IN] size buffer size of the data block
 * RETURNS
 *  VOID
 *****************************************************************************/ 
VOID 
mtk_nfc_sys_sleep (
    UINT32 u4MilliSeconds
)
{
    usleep(1000*u4MilliSeconds);
}

VOID
mtk_nfc_sys_assert (
    INT32 value
)
{
    return;
}

#ifdef WIN32 // PHY layer
/* ***************************************************************************
Physical Link Function
    gLinkFunc.init  = mtkNfcDal_uart_init;
    gLinkFunc.open = mtkNfcDal_uart_open;
    gLinkFunc.close = mtkNfcDal_uart_close;
    gLinkFunc.read  = mtkNfcDal_uart_read;
    gLinkFunc.write  = mtkNfcDal_uart_write;
    gLinkFunc.flush  = mtkNfcDal_uart_flush;
    gLinkFunc.reset = mtkNfcDal_chip_reset;    „³ GPIO control for NFC pins
UART
    void mtkNfcDal_uart_init (void);
    NFCSTATUS mtkNfcDal_uart_open (const char* deviceNode, void ** pLinkHandle)
    int mtkNfcDal_uart_read (uint8_t * pBuffer, int nNbBytesToRead);
    int mtkNfcDal_uart_write (uint8_t * pBuffer, int nNbBytesToWrite);
    void mtkNfcDal_uart_flush (void);
    void mtkNfcDal_uart_close (void);
GPIO
    int mtkNfcDal_chip_reset (int level);
 ************************************************************************** */

extern HANDLE g_hUart;

// UART settings for Windows UART driver
#define NFC_UART_BAUD                   (115200)
#define NFC_UART_BUF_TX                 (1024)
#define NFC_UART_BUF_RX                 (1024)

VOID *
mtk_nfc_sys_uart_init (
    const CH* strDevPortName
)
{
    HANDLE hUARTHandle = INVALID_HANDLE_VALUE;

    hUARTHandle = CreateFile(strDevPortName, GENERIC_READ | GENERIC_WRITE,
                  0, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE != hUARTHandle)
    {
        DCB dcb;
        BOOL fSuccess;

        fSuccess = GetCommState(hUARTHandle, &dcb);
        if (fSuccess)
        {
            dcb.BaudRate = NFC_UART_BAUD;
            dcb.ByteSize = 8;
            dcb.Parity = NOPARITY;
            dcb.StopBits = ONESTOPBIT;
            dcb.fOutxDsrFlow = FALSE;
            dcb.fOutxCtsFlow = FALSE;
            dcb.fDtrControl = DTR_CONTROL_DISABLE;
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fInX = FALSE;           // No Xon/Xoff flow control
            dcb.fOutX = FALSE;
            dcb.fBinary = TRUE;
            dcb.fAbortOnError = FALSE;  // Do not abort reads/writes on error
            dcb.fErrorChar = FALSE;     // Disable error replacement
            dcb.fNull = FALSE;          // Disable null stripping

            fSuccess = SetCommState(hUARTHandle, &dcb);
            
            if (fSuccess)
            {
                COMMTIMEOUTS timeouts;

                // setup device buffer
                SetupComm(hUARTHandle, NFC_UART_BUF_RX, NFC_UART_BUF_TX);

                // setup timeout
                GetCommTimeouts(hUARTHandle, &timeouts);
                timeouts.ReadIntervalTimeout = MAXDWORD;
                timeouts.ReadTotalTimeoutConstant = 0;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.WriteTotalTimeoutConstant = 0;
                timeouts.WriteTotalTimeoutMultiplier = 0;
                SetCommTimeouts(hUARTHandle, &timeouts);
            }
        }

        if (!fSuccess)
        {
            CloseHandle(hUARTHandle);
            hUARTHandle = INVALID_HANDLE_VALUE;
        }
    }

    return hUARTHandle;
}

INT32 
mtk_nfc_sys_uart_read (
//    VOID *pLinkHandle,
    UINT8 *pBuffer, 
    UINT32 nNbBytesToRead
)
{
    DWORD dwRead = 0;    

    if (INVALID_HANDLE_VALUE != g_hUart)
    {
        if (ReadFile(g_hUart, pBuffer, nNbBytesToRead, (LPDWORD)&dwRead, NULL))
        {
            // read success - one shot read and return
        }
        else
        {
            //assert(0);
            dwRead = -1;
        }
    }
    else
    {
        mtk_nfc_sys_dbg_string("UART Handle is invalid\r\n");
        dwRead = -2;
    }
    
    return dwRead;
}

INT32
mtk_nfc_sys_uart_write (
//    VOID *pLinkHandle,
    UINT8 *pBuffer, 
    UINT32 nNbBytesToWrite
)
{
    DWORD dwWritten;
    UINT32 u4Offset = 0;

    mtk_nfc_sys_dbg_string("            ---> PHY TX: ");
    mtk_nfc_sys_dbg_trace(pBuffer, nNbBytesToWrite);
    mtk_nfc_sys_dbg_string("\r\n");

    if (INVALID_HANDLE_VALUE != g_hUart)
    {
        while (u4Offset < nNbBytesToWrite)
        {
            if (WriteFile(g_hUart, &pBuffer[u4Offset], nNbBytesToWrite - u4Offset, &dwWritten, NULL))
            {
                // write success - continuely write if the write data is not completed
                u4Offset += dwWritten;
            }
            else
            {
                //assert(0);            
                break;
            }
        }
    }
    else
    {
        mtk_nfc_sys_dbg_string("UART Handle is invalid\r\n");
    }    

    return dwWritten;
}

VOID 
mtk_nfc_sys_uart_flush (
    VOID *pLinkHandle
)
{
    // purge any information in buffer
    PurgeComm(pLinkHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

// uninit UART
VOID
mtk_nfc_sys_uart_uninit (
    VOID *pLinkHandle
)
{
    if (INVALID_HANDLE_VALUE != pLinkHandle)
    {
        CloseHandle(pLinkHandle);
    }
}
#endif


VOID 
mtk_nfc_sys_dbg_trx_to_file(
    BOOL    fgIsTx, 
    UINT8   pData[], 
    UINT32  u4Len
)
{


}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_interface_init
 * DESCRIPTION
 *  Initialize communication interface between DH and NFCC
 * PARAMETERS
 *  strDevPortName      [IN] Device Name
 *  i4Baud              [IN] Baudrate
 * RETURNS
 *  NONE
 *****************************************************************************/
VOID *
mtk_nfc_sys_interface_init (
    const CH* strDevPortName,
    const INT32 i4Baud
)
{
#ifdef WIN32
    return mtk_nfc_sys_uart_init(strDevPortName, i4Baud);
#else
    return mtk_nfc_sys_i2c_init(0);
#endif
}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_interface_read
 * DESCRIPTION
 *  Read data from NFCC
 * PARAMETERS
 *  pBuffer             [IN] read buffer
 *  nNbBytesToRead      [IN] number of bytes to read
 * RETURNS
 *  number of bytes read
 *****************************************************************************/
INT32 
mtk_nfc_sys_interface_read (
    UINT8 *pBuffer, 
    UINT16 nNbBytesToRead
)
{       
#ifdef WIN32
    return mtk_nfc_sys_uart_read(pBuffer, nNbBytesToRead);
#else // 
   return mtk_nfc_sys_i2c_read(pBuffer, nNbBytesToRead);
#endif
}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_interface_write
 * DESCRIPTION
 *  Write data to NFCC
 * PARAMETERS
 *  pBuffer             [IN] write buffer
 *  nNbBytesToWrite     [IN] number of bytes to write
 * RETURNS
 *  number of bytes written
 *****************************************************************************/
INT32
mtk_nfc_sys_interface_write (
    UINT8 *pBuffer, 
    UINT16 nNbBytesToWrite
)
{
#ifdef WIN32
    return mtk_nfc_sys_uart_write(pBuffer, nNbBytesToWrite);
#else // 
   return mtk_nfc_sys_i2c_write(pBuffer, nNbBytesToWrite);
#endif
}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_interface_flush
 * DESCRIPTION
 *  Flush communication interface
 * PARAMETERS
 *  pLinkHandle         [IN] Link Handle
 * RETURNS
 *  NONE
 *****************************************************************************/
VOID 
mtk_nfc_sys_interface_flush (
    VOID *pLinkHandle
)
{
#ifdef WIN32
    mtk_nfc_sys_uart_flush(pLinkHandle);
#endif
}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_interface_uninit
 * DESCRIPTION
 *  mt6605 gpio config
 * PARAMETERS
 *  pLinkHandle         [IN] Link Handle
 * RETURNS
 *  NONE
 *****************************************************************************/
VOID
mtk_nfc_sys_interface_uninit (
    VOID *pLinkHandle
)
{
#ifdef WIN32
    mtk_nfc_sys_uart_uninit(pLinkHandle);
#else
    mtk_nfc_sys_i2c_uninit(pLinkHandle);
#endif
}


/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_gpio_write
 * DESCRIPTION
 *  mt6605 gpio config
 * PARAMETERS
 *  ePin        [IN] GPIO PIN
 *  eHighLow    [IN] High or How
 * RETURNS
 *  NONE
 *****************************************************************************/
VOID 
mtk_nfc_sys_gpio_write(
    MTK_NFC_GPIO_E ePin, 
    MTK_NFC_PULL_E eHighLow
)
{
   UINT32 result;

   result = ioctl(gInterfaceHandle, MTK_NFC_IOCTL_WRITE, ((ePin << 8) | (eHighLow)));
   
   if(result != MTK_NFC_SUCCESS)
   {
      ;//ERROR
   }
   
   return;

}

/*****************************************************************************
 * FUNCTION
 *  mtk_nfc_sys_gpio_read
 * DESCRIPTION
 *  mt6605 gpio config
 * PARAMETERS
 *  ePin        [IN] GPIO PIN
 * RETURNS
 *  MTK_NFC_PULL_E
 *****************************************************************************/
MTK_NFC_PULL_E 
mtk_nfc_sys_gpio_read(
    MTK_NFC_GPIO_E ePin
)
{
   MTK_NFC_PULL_E result;

   result = ioctl(gInterfaceHandle, MTK_NFC_IOCTL_READ, (ePin << 8));
      
   return result;
}

INT32 mtk_nfc_sys_i2c_read(UINT8* data, UINT16 len)
{
   INT32 result, i;
   
   result = read(gInterfaceHandle, data, len);   

   if (result > 0)
   {
      CHAR pDeBugBuffer[2048];
      uint32_t DeBugL = 0;   
      
       memset(pDeBugBuffer, 0x00, 2048);
       NFCD("mtk_nfc_sys_i2c_read :len,0x%x\r\n", len);
       sprintf(pDeBugBuffer, "[RX],");
       DeBugL = strlen(pDeBugBuffer);
       for( i =0;i < len;i++)
       {
           sprintf((pDeBugBuffer+DeBugL),"%02x,",data[i]);
           DeBugL = strlen(pDeBugBuffer);   
       }
       sprintf((pDeBugBuffer+DeBugL),"\n");        
       NFCD("%s", pDeBugBuffer);
   }
   
   return result;
}


INT32 mtk_nfc_sys_i2c_write(UINT8* data, UINT16 len)
{
    INT32 result, i;
    CHAR pDeBugBuffer[2048];
    uint32_t DeBugL = 0;  
    
    NFCD("mtk_nfc_sys_i2c_write :len,0x%x\r\n", len);

    memset(pDeBugBuffer, 0x00, 2048);    
    
    sprintf(pDeBugBuffer, "[TX],");
    DeBugL = strlen(pDeBugBuffer);
   
    for( i =0;i < len;i++)
    {
       sprintf((pDeBugBuffer+DeBugL),"%02x,",data[i]);
       DeBugL = strlen(pDeBugBuffer);       
    }
    sprintf((pDeBugBuffer+DeBugL),"\n");      
    NFCD("%s", pDeBugBuffer);
    result = write(gInterfaceHandle, data, len);
    
    return result;
}

VOID *mtk_nfc_sys_i2c_init(INT32 type)
{
   char *pComPort;
   
   pComPort = "/dev/mt6605";

   gInterfaceHandle = open(pComPort, O_RDWR | O_NOCTTY);

   return (VOID *)gInterfaceHandle;
}

VOID mtk_nfc_sys_i2c_uninit(VOID *pLinkHandle)
{
   if (gInterfaceHandle != -1)
   {
       close(gInterfaceHandle);
       gInterfaceHandle = -1;
   }
}

#ifdef USE_SIGNAL_EVENT_TO_TIMER_CREATE
VOID nfc_timer_expiry_hdlr (int sig, siginfo_t *si, void *uc)
{
    INT32 timer_slot;
    timer_t *tidp;
    ppCallBck_t cb_func;
    VOID *param;

    NFCD("[TIMER]Caugh signal %d\n", sig);

//    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);

    tidp = si->si_value.sival_ptr;

    /* Look up timer_slot of this timeout, range = 0 ~ (MTK_NFC_TIMER_MAX_NUM-1) */
    for(timer_slot = 0; timer_slot < MTK_NFC_TIMER_MAX_NUM; timer_slot++)
    {
        if ( ( nfc_timer_table[timer_slot].is_used == TRUE ) && 
             ( nfc_timer_table[timer_slot].handle == *tidp ) )
        {
            break;
        }
    }
    
    if(timer_slot == MTK_NFC_TIMER_MAX_NUM)    //timer not found in table
    {
        NFCD("[TIMER]timer no found in the table : (handle: 0x%x)\r\n", *tidp);
        return;
    }
    
    //get the cb and param from gps timer pool
    cb_func = nfc_timer_table[timer_slot].timer_expiry_callback;
    param = nfc_timer_table[timer_slot].timer_expiry_context;
    
//    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);

    //stop time (windows timer is periodic timer)
    mtk_nfc_sys_timer_stop(timer_slot);
    
    //execute cb
    (*cb_func)(timer_slot, param);
}
#else
static VOID nfc_timer_expiry_hdlr (union sigval sv)
{
    INT32 timer_slot;
    ppCallBck_t cb_func;
    VOID *param;

    NFCD("[TIMER]nfc_timer_expiry_hdlr...\n");

    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);

    timer_slot = (INT32)(sv.sival_int);

    if(timer_slot >= MTK_NFC_TIMER_MAX_NUM)
    {
        NFCD("[TIMER]invalid timer_slot,%d\r\n", timer_slot);
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;
    }

    if (nfc_timer_table[timer_slot].is_stopped == 1)
    {
        NFCD("[TIMER] Expired but already stopped timer_slot=%d\n", timer_slot);
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;
    }
        
    //get the cb and param from gps timer pool
    cb_func = nfc_timer_table[timer_slot].timer_expiry_callback;
    param = nfc_timer_table[timer_slot].timer_expiry_context;
    
    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);

    //stop time (windows timer is periodic timer)
    mtk_nfc_sys_timer_stop(timer_slot);
    
    //execute cb
    (*cb_func)(timer_slot, param);
}

#endif


/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_timer_init
 * DESCRIPTION
 *  Create a new timer
 * PARAMETERS
 *  NONE
 * RETURNS
 *  a valid timer ID or MTK_NFC_TIMER_INVALID_ID if an error occured
 *****************************************************************************/ 
INT32 
mtk_nfc_sys_timer_init (
    VOID
)
{
    int ret = MTK_NFC_SUCCESS;
    int timer_slot;
    
#ifdef USE_SIGNAL_EVENT_TO_TIMER_CREATE
    struct sigaction sa;

    /* Establish handler for timer signal */ 
    NFCD("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = nfc_timer_expiry_hdlr;
    sigemptyset(&sa.sa_mask);
    
    ret = sigaction(SIG, &sa, NULL);
    if (ret == -1) {
        NFCD("sigaction fail\r\n");
    }
#endif

    /* Initialize timer pool */
    for (timer_slot = 0; timer_slot < MTK_NFC_TIMER_MAX_NUM; timer_slot++) {                
        nfc_timer_table[timer_slot].is_used = FALSE;
        nfc_timer_table[timer_slot].handle  = 0;
        nfc_timer_table[timer_slot].is_used = FALSE;
        nfc_timer_table[timer_slot].timer_expiry_callback = NULL;
        nfc_timer_table[timer_slot].timer_expiry_context = NULL;
        nfc_timer_table[timer_slot].is_stopped = TRUE;
    }
    
    return ret;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_timer_create
 * DESCRIPTION
 *  Create a new timer
 * PARAMETERS
 *  selected_timer  [IN] select the timer slot 
 * RETURNS
 *  a valid timer ID or MTK_NFC_TIMER_INVALID_ID if an error occured
 *****************************************************************************/ 
UINT32 
mtk_nfc_sys_timer_create (
    MTK_NFC_TIMER_E selected_timer
)
{
    INT32 ret;
    UINT32 timer_slot;
    struct sigevent se;

    if (selected_timer >= MTK_NFC_TIMER_MAX_NUM) {
        NFCD("[TIMER]Invalid timer request %d\r\n", (UINT32)selected_timer);
        return MTK_NFC_TIMER_INVALID_ID;
    }

    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);

    timer_slot = selected_timer;
    if(nfc_timer_table[timer_slot].is_used == TRUE)
    {
        NFCD("[TIMER]timer already created\r\n");
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return timer_slot;
    }

#ifdef USE_SIGNAL_EVENT_TO_TIMER_CREATE
    /* Create the timer */
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIG;    
    se.sigev_value.sival_ptr = &nfc_timer_table[timer_slot].handle;
#else
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_notify_function = nfc_timer_expiry_hdlr;
    se.sigev_notify_attributes = NULL;    
    se.sigev_value.sival_int = (int) timer_slot;
#endif

    /* Create a POSIX per-process timer */
    if ((ret = timer_create(CLOCKID, &se, &(nfc_timer_table[timer_slot].handle))) == -1)
    {
        NFCD("[TIMER]timer_create fail, ret:%d, errno:%d, %s\r\n", ret, errno, strerror(errno));
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return MTK_NFC_TIMER_INVALID_ID;
    }
        
    nfc_timer_table[timer_slot].is_used = TRUE;
    NFCD("[TIMER]create,time_slot,%d,handle,0x%x\r\n", timer_slot, nfc_timer_table[timer_slot].handle);

    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
    
    return timer_slot;
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_timer_start
 * DESCRIPTION
 *  Start a timer
 * PARAMETERS
 *  timer_slot  [IN] a valid timer slot
 *  period      [IN] expiration time in milliseconds
 *  timer_expiry[IN] callback to be called when timer expires
 *  arg         [IN] callback fucntion parameter
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID 
mtk_nfc_sys_timer_start (
    UINT32      timer_slot, 
    UINT32      period, 
    ppCallBck_t timer_expiry, 
    VOID        *arg
)
{
    struct itimerspec its;

    if (timer_slot >= MTK_NFC_TIMER_MAX_NUM)
    {
        NFCD("[TIMER]timer_slot(%d) exceed max num of nfc timer\r\n", timer_slot);  
        return;
    }

    if (timer_expiry == NULL)
    {
        NFCD("[TIMER]timer_expiry_callback == NULL\r\n");    
        return;    
    }

    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);

    if (nfc_timer_table[timer_slot].is_used == FALSE)
    {
        NFCD("[TIMER]timer_slot(%d) didn't be created\r\n", timer_slot);
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;        
    }

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = period / 1000;
    its.it_value.tv_nsec = 1000000 * (period % 1000);
    if ((its.it_value.tv_sec == 0) && (its.it_value.tv_nsec == 0))
    {
        // this would inadvertently stop the timer (TODO: HIKI)
        its.it_value.tv_nsec = 1;
    }

    nfc_timer_table[timer_slot].timer_expiry_callback = timer_expiry;
    nfc_timer_table[timer_slot].timer_expiry_context = arg;
    nfc_timer_table[timer_slot].is_stopped = FALSE;   
    timer_settime(nfc_timer_table[timer_slot].handle, 0, &its, NULL);
    
    NFCD("[TIMER]timer_slot(%d) start, handle(0x%x)\r\n", timer_slot, nfc_timer_table[timer_slot].handle);

    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);    
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_timer_stop
 * DESCRIPTION
 *  Start a timer
 * PARAMETERS
 *  timer_slot    [IN] a valid timer slot
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID 
mtk_nfc_sys_timer_stop (
    MTK_NFC_TIMER_E timer_slot
)
{
    struct itimerspec its = {{0, 0}, {0, 0}};
    
    if (timer_slot >= MTK_NFC_TIMER_MAX_NUM)
    {
        NFCD("[TIMER]timer_slot(%d) exceed max num of nfc timer\r\n", timer_slot);
        return;
    }

    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);
    
    if (nfc_timer_table[timer_slot].is_used == FALSE)
    {
        NFCD("[TIMER]timer_slot(%d) already be deleted\r\n", timer_slot);
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;        
    }

    if (nfc_timer_table[timer_slot].is_stopped == TRUE)
    {
        NFCD("[TIMER]timer_slot(%d) already be stopped\r\n", timer_slot);
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;
    }
    
    nfc_timer_table[timer_slot].is_stopped = TRUE;
    timer_settime(nfc_timer_table[timer_slot].handle, 0, &its, NULL);
    
    NFCD("[TIMER]timer_slot(%d) stop, handle(0x%x)\r\n", timer_slot, nfc_timer_table[timer_slot].handle);
    
    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
}

/***************************************************************************** 
 * Function
 *  mtk_nfc_sys_timer_delete
 * DESCRIPTION
 *  Delete a timer
 * PARAMETERS
 *  timer_slot    [IN] a valid timer slot
 * RETURNS
 *  NONE
 *****************************************************************************/ 
VOID
mtk_nfc_sys_timer_delete (
    MTK_NFC_TIMER_E timer_slot
)
{
    if (timer_slot >= MTK_NFC_TIMER_MAX_NUM)
    {
        NFCD("[TIMER]exceed max num of nfc timer,%d\r\n", timer_slot);
        return;
    }

    mtk_nfc_sys_mutex_take(MTK_NFC_MUTEX_TIMER);
    
    if (nfc_timer_table[timer_slot].is_used == FALSE)
    {
        NFCD("[TIMER]timer_slot(%d) already be deleted\r\n", timer_slot);        
        mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
        return;
    }
    
    timer_delete(nfc_timer_table[timer_slot].handle);
    nfc_timer_table[timer_slot].handle = 0;
    nfc_timer_table[timer_slot].timer_expiry_callback = NULL;
    nfc_timer_table[timer_slot].timer_expiry_context = NULL;
    nfc_timer_table[timer_slot].is_used = FALSE; // clear used flag
    NFCD("[TIMER]timer_slot(%d) delete\r\n", timer_slot);    

    mtk_nfc_sys_mutex_give(MTK_NFC_MUTEX_TIMER);
}

//*****************************************************************************
// File Functions
//*****************************************************************************

//*****************************************************************************
// mtk_nfc_sys_file_open : Open a file
//
// PARAMETER : szFileName [IN] - name of the file to be opened 
//             i4Mode     [IN] - file access mode (read / write / read + write)
//                               0 -- open file for reading (r)
//                               1 -- create file for writing,
//                                    discard previous contents if any (w)
//                               2 -- open or create file for writing at end of file (a)
//                               3 -- open file for reading and writing (r+)
//                               4 -- create file for reading and writing,
//                                    discard previous contents if any (w+)
//                               5 -- open or create file for reading and writing at end of file (a+)
//
// NOTE : For system which treats binary mode and text mode differently,
//        such as Windows / DOS, please make sure to open file in BINARY mode
//
// RETURN : On success, return the file handle
//          If fail, return 0
NFC_FILE mtk_nfc_sys_file_open(const CHAR *szFileName, UINT32 i4Mode)
{
    FILE *fp;
    char szMode[4];

    // For system which treats binary mode and text mode differently,
    // such as Windows / DOS, please make sure to open file in BINARY mode

    switch (i4Mode)
    {
    case MTK_NFC_FS_READ:       // 0
        sprintf(szMode, "rb");
        break;
    case MTK_NFC_FS_WRITE:      // 1
        sprintf(szMode, "wb");
        break;
    case MTK_NFC_FS_APPEND:     // 2
        sprintf(szMode, "ab");
        break;
    case MTK_NFC_FS_RW:         // 3
        sprintf(szMode, "r+b");
        break;
    case MTK_NFC_FS_RW_DISCARD: // 4
        sprintf(szMode, "w+b");
        break;
    case MTK_NFC_FS_RW_APPEND:  // 5
        sprintf(szMode, "a+b");
        break;
    default:
        return 0;
    }
    
    fp = fopen(szFileName, szMode);

    if (fp != NULL)
    {
        return (NFC_FILE)fp;
    }

    return 0;
}

//*****************************************************************************
// mtk_nfc_sys_file_close : Close a file
//
// PARAMETER : hFile [IN] - handle of file to be closed
//
// RETURN : void
VOID mtk_nfc_sys_file_close (NFC_FILE hFile)
{
    fclose((FILE *)hFile);
}

//*****************************************************************************
// mtk_nfc_sys_file_read : Read a block of data from file
//
// PARAMETER : hFile    [IN]  - handle of file
//             DstBuf   [OUT] - pointer to data buffer to be read
//             u4Length [IN]  - number of bytes to read
//
// RETURN : Number of bytes read
UINT32 mtk_nfc_sys_file_read (NFC_FILE hFile, void *DstBuf, UINT32 u4Length)
{
    if (hFile != 0)
    {
        return (UINT32)fread(DstBuf, 1, u4Length, (FILE *)hFile);
    }

    return 0;
}

//*****************************************************************************
// mtk_nfc_sys_file_seek : Set the position indicator associated with file handle 
//                     to a new position defined by adding offset to a reference
//                     position specified by origin
//
// PARAMETER : hFile    [IN] - handle of file
//             u4OffSet [IN] - number of bytes to offset from origin
//             u4Origin [IN] - position from where offset is added
//                             0 -- seek from beginning of file
//                             1 -- seek from current position
//                             2 -- seek from end of file
//
// RETURN : On success, return a zero value
//          Otherwise, return a non-zero value
UINT32 mtk_nfc_sys_file_seek (NFC_FILE hFile, UINT32 u4OffSet, UINT32 u4Origin)
{
    return fseek((FILE *)hFile, u4OffSet, u4Origin); 
}

//*****************************************************************************
// mtk_nfc_sys_file_tell : Returns the current value of the position indicator of the stream.
//
// PARAMETER : hFile    [IN] - handle of file
//
// RETURN : On success, the current value of the position indicator is returned.
//               On failure, -1L is returned, and errno is set to a system-specific positive value.
UINT32 mtk_nfc_sys_file_tell (NFC_FILE hFile)
{
    return (UINT32)ftell((FILE *)hFile);
}

//*****************************************************************************
// mtk_nfc_sys_file_rewind : Set position of stream to the beginning.
//
// PARAMETER : hFile    [IN] - handle of file
//
// RETURN : none
VOID mtk_nfc_sys_file_rewind (NFC_FILE hFile)
{
    rewind((FILE *)hFile);
}

#ifdef SUPPORT_BLOCKING_READ_MECHANISM
static pthread_cond_t g_nfc_event_cond[MTK_NFC_EVENT_END];
//static pthread_mutex_t g_nfc_event_mtx[MTK_NFC_EVENT_END];

INT32 mtk_nfc_sys_event_delete(MTK_NFC_EVENT_E event_idx)
{
    INT32 ret = MTK_NFC_SUCCESS;

    if (pthread_cond_destroy(&g_nfc_event_cond[event_idx]))
    {
        ret = MTK_NFC_ERROR;
    }
//    if (pthread_mutex_destroy(&g_nfc_event_mtx[event_idx]))
//    {
//        ret = MTK_NFC_ERROR;
//    }
    
    return ret;  
}

INT32 mtk_nfc_sys_event_create(MTK_NFC_EVENT_E event_idx)
{
    INT32 ret = MTK_NFC_SUCCESS;

//    if (pthread_mutex_init(&g_nfc_event_mtx[event_idx], NULL))
//    {
//        ret = MTK_NFC_ERROR;
//    }
    if (pthread_cond_init(&g_nfc_event_cond[event_idx], NULL))
    {
        ret = MTK_NFC_ERROR;
    }

    return ret;
}

INT32 mtk_nfc_sys_event_set(MTK_NFC_EVENT_E event_idx)
{
    INT32 ret = MTK_NFC_SUCCESS;
    
//    if (pthread_mutex_lock(&g_nfc_event_mtx[event_idx]))
//    {
//         ret = MTK_NFC_ERROR;
//    }
    if (pthread_cond_signal(&g_nfc_event_cond[event_idx]))
    {
        ret = MTK_NFC_ERROR;
    }
//    pthread_mutex_unlock(&g_nfc_event_mtx[event_idx]);

    return ret;
}

INT32 mtk_nfc_sys_event_wait(MTK_NFC_EVENT_E event_idx, MTK_NFC_MUTEX_E mutex_idx)
{
    INT32 ret = MTK_NFC_SUCCESS;

//    if (pthread_mutex_lock(&g_nfc_event_mtx[event_idx]))
//    {
//         ret = MTK_NFC_ERROR;
//    }

//    if (pthread_cond_wait(&g_nfc_event_cond[event_idx], &g_nfc_event_mtx[event_idx]))
    if (pthread_cond_wait(&g_nfc_event_cond[event_idx], &g_hMutex[mutex_idx]))
    {
        ret = MTK_NFC_ERROR;
    }
//    pthread_mutex_unlock(&g_nfc_event_mtx[event_idx]);

    return ret;
}
#endif


