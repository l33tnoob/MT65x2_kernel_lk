#ifndef __ICUSB_CCCI_H__
#define __ICUSB_CCCI_H__

#include "icusb_smart_card.h"

#define CCCI_TTY_SIM_ID_1 (0x0)

#define CCCI_TTY_MAX_PKT_SIZE (254)

//#define CCCI_MTK_HEADER_ROOM (3)

//#define CCCI_ICUSB_HEADER_ROOM (CCCI_MTK_HEADER_ROOM + ICUSB_SMARTCARD_CLASS_HEADER_ROOM)

#define CCCI_TX_INTERVAL_US (500)

#define CCCI_TX_ERR_DELAY_US (10000)


enum ccci_trasfer_return_codes {
	CCCI_RX_GOOD_PACKET = 10 , 
	
	CCCI_TX_CTRL_PACKET = 20 , 
	CCCI_TX_DATA_PACKET = 21 , 
	CCCI_TX_NO_DATA = 22 , 
	
	CCCI_ERROR_PACKET	= -1 ,
	CCCI_CHANNEL_NOT_READY	= -2 ,
	CCCI_HEADERROOM_ERROR	= -3 ,		
};

enum ccci_mtk_header_pcb_code {
	CCCI_MTK_H_PCB_MODE = 0x80 , 
	CCCI_MTK_H_PCB_M_BIT = 0x20 , 	
	CCCI_MTK_H_PCB_MODE_CTRL = 0x80 , 
	CCCI_MTK_H_PCB_MODE_DATA = 0x00 , 	
};

#pragma pack(push) 
#pragma pack(1)  

typedef struct _s_ccci_mtk_header {
	unsigned char sif ; // sim interface byte	
	unsigned char pcb ; // protocol control byte	
	unsigned short cp ; // chain parameter
	unsigned short len ;	
	//unsigned char len ;	
}s_ccci_mtk_header;

#pragma pack(pop)

int icusb_ccci_open(void) ;
int icusb_ccci_close(void) ;

int icusb_rx_ccci_write(char *apdu_buff , int apdu_length, s_ccci_mtk_header* ccci_mtk_header) ;
int icusb_tx_ccci_read(char *tx_buff , int tx_buff_size, int header_room, int *apdu_length, char **p_apdu_buff, unsigned short *chain_parameter) ;

int icusb_ccci_notify_md_recover(void) ;


#endif
