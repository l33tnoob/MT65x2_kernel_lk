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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   tst_main.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Lu.Zhang (MTK80251) 09/11/2009
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 05 11 2010 lu.zhang
 * [ALPS00005327]CCAP 
 * .
 *
 * 04 01 2010 lu.zhang
 * [ALPS00004362]CCAP APIs 
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *  *  * u1rwduu`wvpghlqg|ip+mdkb
 *
 *
 *
 *
 *
 *******************************************************************************/



//
// TST driver.
//


#ifndef __TST_MAIN_H__
#define __TST_MAIN_H__


#include "meta.h"
#include "ft_main.h"

#include "CFG_META_FILE.h"
#include "WM2Linux.h"

#include <pthread.h>


//*****************************************************************************
//
//                          TST Driver MACRO def
//
//*****************************************************************************
#define TST_DBG 1

#define BOOT_MODE_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"
#define BOOT_MODE_STR_LEN 1
#define BOOT_MODE_NORMAL 	0
#define BOOT_MODE_META 		1
#define BOOT_MODE_ADV_META 	5

#define COM_PORT_TYPE_FILE "/sys/bus/platform/drivers/meta_com_type_info/meta_com_type_info"
#define COM_PORT_TYPE_STR_LEN 1
typedef enum
{
	META_UNKNOWN_COM=0,
	META_UART_COM,
	META_USB_COM
}META_COM_TYPE;

#define COM_MODE_UART 0
#define COM_MODE_USB 1

/* the define of com port */
#define MCIPORT    _T("MCI3:")
#define USBPORT    _T("COM0:")


//define the timer of reset system
#define SYSRSTTIME  50000
// define the maximum of a frame
#define  MAX_TST_RECEIVE_BUFFER_LENGTH       (4096*16)//2048
#define  MAX_TST_TX_BUFFER_LENGTH            (4096*16)

//define the AP frame
#define AP_FRAME_REQ_ID_LENGTH 2 
#define AP_FRAME_TOKEN_LENGTH 2

//define the AP req id
#define AP_CHECK_SIM_REQ_ID 104

//define the change port
#define CHANGE_PORT_REQ_ID 106

//define the MD frame
#define MD_FRAME_TREACE_OFFSITE 3
#define MD_FRAME_HREADER_LENGTH 4
#define MD_FRAME_TST_INJECT_PRIMITIVE_LENGTH 10
#define MD_FRAME_FAILED_TST_LOG_PRIMITIVE_LENGTH  20
#define MD_FRAME_SUCCESS_TST_LOG_PRIMITIVE_LENGTH 102
#define MD_FRAME_REF_LENGTH 2
#define MD_FRAME_MSG_LEN_LENGTH 2
#define MD_FRAME_MAX_LENGTH 256
#define MD_FRAME_FAILED_CHECEK_SIM_OFFISTE 76
#define MD_FRAME_SUCCESS_CHECEK_SIM_OFFISTE 116
#define MD_FRAME_DS269_OFFSITE 8

//the define of the type of meta frame
#define  RS232_LOGGED_PRIMITIVE_TYPE   		0x60
#define  RS232_PS_TRACE_TYPE           		0x61
#define  RS232_PS_PROMPT_TRACE_TYPE    		0x62
#define  RS232_COMMAND_TYPE_OCTET      		0x63
#define  RS232_INJECT_PRIMITIVE_OCTET  		0x64
#define  RS232_INJECT_UT_PRIMITIVE     		0x65
#define  RS232_INJECT_APPRIMITIVE_OCTET     0x66

#define  RS232_INJECT_PRIMITIVE_OCTETMODEM2  		0xA0
#define  RS232_COMMAND_TYPE_MD2_MEMORY_DUMP      		0xC0

#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_START   0xD0
#define  RS232_COMMAND_TYPE_MD_DATA_TUNNEL_END 0xD7
#define  RS232_RESPONSE_MD_DATA_TUNNEL_START   0xD8
#define  RS232_RESPONSE_MD_DATA_TUNNEL_END  0xDF


/* teh define of escape key */
#define   STX_OCTET            	0x55
#define   MUX_KEY_WORD		    0x5A
#define   SOFT_FLOW_CTRL_BYTE   0x77
#define   STX_L1HEADER            	0xA5



/* Define the rs232 frame phase states */
#define  RS232_FRAME_STX               				0
#define  RS232_FRAME_LENHI             				1
#define  RS232_FRAME_LENLO             				2
#define  RS232_FRAME_TYPE              				3
#define  RS232_FRAME_LOCAL_LENHI       				4
#define  RS232_FRAME_LOCAL_LENLO       				5
#define  RS232_FRAME_PEER_LENHI        				6
#define  RS232_FRAME_PEER_LENLO        				7
#define  RS232_FRAME_COMMAND_DATA      				8
#define  RS232_FRAME_COMMAND_HEADER    				9
#define  RS232_FRAME_UT_DATA		   				10
#define  RS232_FRAME_MD_DATA		   				11
#define  RS232_FRAME_AP_INJECT_PIRIMITIVE_HEADER 	12
#define  RS232_FRAME_AP_PRIM_LOCAL_PARA_DATA     	13
#define  RS232_FRAME_AP_PRIM_PEER_DATA           	14
#define  RS232_FRAME_CHECKSUM          				15
#define  RS232_FRAME_KEYWORD		   				16
#define  RS232_FRAME_SOFT_CTRL         				17
#define  RS232_FRAME_MD_CONFIRM_DATA				18
#define  RS232_FRAME_MD_TUNNELING_DATA 19
#define  RS232_FRAME_MD_TUNNELING_CHECKSUM 20

//-------------------------------------
// define com mask parameter
//-------------------------------------
#define DEFAULT_COM_MASK    (EV_RXCHAR | EV_RLSD | EV_ERR | EV_BREAK | EV_RING)



//-------------------------------------
//define modem count
#define MODEM_COUNT      2
//-------------------------------------
//*****************************************************************************
//
//                          TST Driver data stru def
//
//*****************************************************************************

typedef struct
{
    kal_uint16     local_len;
    kal_uint16     peer_len;
}
TST_PRIMITIVE_HEADER_STRUCT;

/* the define of buf of meta type */
typedef struct
{
    TST_PRIMITIVE_HEADER_STRUCT 	inject_prim;					//lenght of peer buf and local buf
    kal_uint16     					received_prig_header_length;	//recieved header count
    kal_uint16     					received_buf_para_length;		//recieved buf count
    kal_uint8      					*header_ptr;					//header pointer
    kal_uint8      					*buf_ptr;						//buf pointer
}TST_CURRENT_HANDLE_FREAME_STRUCT;

typedef struct 
{
	kal_uint16	frame_len;
	kal_uint8		frame_state;
	kal_uint8		frame_cksm;
	kal_uint8		frame_md_index;
	kal_uint8		frame_buf[FrameMaxSize]; // Must be 4-byte aligned
	kal_uint8		*frame_data_ptr; // this is a frame type dependent data pointer
} TST_FRMAE_INTERNAL_STRUCT;

#define TST_CHECKSUM_SIZE					(1)

typedef struct 
{
	unsigned int	data_len;
	char			preserve_head_buf[MD_FRAME_HREADER_LENGTH*2]; // Double the buffer space to preserve extension for escape translation
	char			data[MAX_TST_RECEIVE_BUFFER_LENGTH];
	char			preserve_tail_buf[TST_CHECKSUM_SIZE*2];// Double the buffer space to preserve extension for escape translation
} TST_MD_RECV_BUF;


//*****************************************************************************
//
//                          TST Driver var def
//
//*****************************************************************************
int g_hMciComPort1 = -1;		//mci com port handle
int g_hMciComPort2 = -1;		//mci com port handle
int g_hMciComPort5 = -1;

int g_hUsbComPort = -1;		//usb com port handle
// Added for USB communication
int g_hUsbComPort2 = -1;	//usb com port handle


int g_pipe_tsttx_fd = -1;


pthread_t g_hMCIComTh;
pthread_t g_hMCIComThModem2;
pthread_t g_hMCIComThModem5;
pthread_t g_hUSBComTh;



int  g_bTerminateAll = 0;		//terminate all thread
int  g_bLogEnable = 0;			//enable all log


char g_cTstFrameState;				//the framte state 
char g_cMDFrameState[MODEM_COUNT];

char g_cOldTstFrameState;			//record the state when escape
char g_cOldMDFrameState[MODEM_COUNT];			//record the state when escape

META_CFG_Struct g_stPortCfg ;		//nvram setting

unsigned short g_MD_SIM_CHECK_TOKEN[MODEM_COUNT]; //check the md sim token

unsigned short g_AP_SIM_CHECK_TOKEN[MODEM_COUNT]; //check the ap sim token
unsigned short g_AP_SIM_CHECK_REQ_ID[MODEM_COUNT]; //check the ap sim req id


//RX or TX bufer define
unsigned char g_cUSBRxBuffer[MAX_TST_RECEIVE_BUFFER_LENGTH];
unsigned char g_cMCIRxBuffer[MAX_TST_RECEIVE_BUFFER_LENGTH];
unsigned char g_cMCIRxBuffer2[MAX_TST_RECEIVE_BUFFER_LENGTH];


//Added for data dumping
unsigned char g_cMetaFrameRxBuffer[MAX_TST_RECEIVE_BUFFER_LENGTH];
unsigned int g_iMetaFrameRxBufIndex = 0;

//record state of recieving meta frame
TST_CURRENT_HANDLE_FREAME_STRUCT  	g_sRs232Frame;


//Added for modem data dumping
unsigned char g_cModemFrameRxBuffer[MODEM_COUNT][MAX_TST_RECEIVE_BUFFER_LENGTH];
unsigned int g_iModemFrameRxBufIndex[MODEM_COUNT] = {0};

typedef enum
{
	DUALMODEM=1,
	MODEMONEONLY,
	MODEMTWOONLY
}META_MODEM_TYPE;



#endif /* __TST_H__ */

