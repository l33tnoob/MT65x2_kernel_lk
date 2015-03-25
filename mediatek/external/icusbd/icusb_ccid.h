#ifndef __ICUSB_CCID_H__
#define __ICUSB_CCID_H__

#include <libusb/libusb.h>
#include "icusb_util.h"


#define BULK_TRANSFER_IN_TIMEOUT		(8000) // longer timeout to avoid partial data
#define BULK_TRANSFER_OUT_TIMEOUT		(2000)

#define CONTROL_TRANSFER_TIMEOUT	(1000)
#define INTERRUPT_TRANSFER_TIMEOUT	(200)

#define CCID_HEADER_SIZE (10)

#define CCID_SIM_SLOT_1 (0x0)

/* PC TO RDR Message Types */
#pragma pack(push) 
#pragma pack(1)   
struct ccid_pc2rdr_icc_pwr_on_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bPowerSelect; // bReserved
	uint8_t  abRFU[2];
};

struct ccid_pc2rdr_icc_pwr_off_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_get_slot_status_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_xfr_block_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bBWI; // bReserved
	uint16_t  wLevelParameter; 
};

struct ccid_pc2rdr_get_parameters_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_reset_parameters_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_set_parameters_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bProtocolNum;	
	uint8_t  abRFU[2];
};

struct ccid_pc2rdr_escape_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_icc_clock_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bClockCommand;	
	uint8_t  abRFU[2];
};

struct ccid_pc2rdr_t0_apdu_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bmChanges;	
	uint8_t  bmClassGetResponse;
	uint8_t  bClassEnvelope;
};

struct ccid_pc2rdr_secure_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bBWI;	
	uint16_t  wLevelParameter;
};

struct ccid_pc2rdr_mechanical_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bFunction;	
	uint8_t  abRFU[2];
};

struct ccid_pc2rdr_abort_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

struct ccid_pc2rdr_set_data_rate_and_clock_frequency_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  abRFU[3];
};

/* RDR TO PC Message Types */
struct ccid_rdr2pc_data_block_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bStatus; 
	uint8_t  bError; 
	uint8_t  bChainParameter; 
	uint8_t  abData[0] ;
};

struct ccid_rdr2pc_slot_status_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bStatus;
	uint8_t  bError;
	uint8_t  bClockStatus;		
};

struct ccid_rdr2pc_parameters_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bStatus;
	uint8_t  bError;
	uint8_t  bProtocolNum;		
};

struct ccid_rdr2pc_escape_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bStatus;
	uint8_t  bError;
	uint8_t  bRFU;		
};

struct ccid_rdr2pc_data_rate_and_clock_frequency_msg {
	uint8_t  bMessageType;
	uint32_t  dwLength;
	uint8_t	 bSlot;
	uint8_t  bSeq;
	uint8_t  bStatus;
	uint8_t  bError;
	uint8_t  bRFU;		
};

#pragma pack(pop)

enum ccid_bulk_out_transfer_requst_type {
	CCID_BULK_OUT_PC_TO_RDR_ICCPOWERON = 0x62 ,	
	CCID_BULK_OUT_PC_TO_RDR_ICCPOWEROFF = 0x63 ,	
	CCID_BULK_OUT_PC_TO_RDR_GETSLOTSTATUS = 0x65 ,	
	CCID_BULK_OUT_PC_TO_RDR_XFRBLOCK = 0x6F ,		

	CCID_BULK_OUT_PC_TO_RDR_GETPARAMETERS = 0x6C ,		
	CCID_BULK_OUT_PC_TO_RDR_RESETPARAMETERS = 0x6D ,		
	CCID_BULK_OUT_PC_TO_RDR_SETPARAMETERS = 0x61 ,		
	CCID_BULK_OUT_PC_TO_RDR_ESCAPE = 0x6B ,		
	CCID_BULK_OUT_PC_TO_RDR_ICCCLOCK = 0x6E ,		
	CCID_BULK_OUT_PC_TO_RDR_T0APDU = 0x6A ,		
	CCID_BULK_OUT_PC_TO_RDR_SECURE = 0x69 ,		
	CCID_BULK_OUT_PC_TO_RDR_MECHANICAL = 0x71 ,		
	CCID_BULK_OUT_PC_TO_RDR_ABORT = 0x72 ,			
	CCID_BULK_OUT_PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY = 0x73 ,
};

enum ccid_data_bulk_in_transfer_response_type {
	CCID_BULK_IN_RDR_TO_PC_DATABLOCK = 0x80 ,	
	CCID_BULK_IN_RDR_TO_PC_SLOTSTATUS = 0x81 ,	

	CCID_BULK_IN_RDR_TO_PC_PARAMETERS = 0x82 ,	
	CCID_BULK_IN_RDR_TO_PC_ESCAPE = 0x83 ,	
	CCID_BULK_IN_RDR_TO_PC_DATARATEANDCLOCKFREQUENCY = 0x84 ,	
};

enum ccid_bstatus_filed {
	CCID_BSTATUS_FIELD_BMICCSTATUS = 0x03 ,	
	CCID_BSTATUS_FIELD_BRFU = 0x3C ,	
	CCID_BSTATUS_FIELD_BMCOMMANDSTATUS = 0xC0 ,	
};

enum ccid_bstatus_codes{
    ICUSB_ACK_STATUS_OK = 0x00,

	ICUSB_CCID_STATUS_NO_CARD = 0xFF ,	
	ICUSB_CCID_STATUS_TIMEOUT = 0xFE ,	
	ICUSB_CCID_STATUS_CMD_ERROR = 0xFD ,	

    ICUSB_ACK_STATUS_PREFER_3V = 0x10 ,   

    ICUSB_ACK_STATUS_NEED_RX_TO_ACK = 0xEE ,   
    ICUSB_ACK_STATUS_CMD_TYPE_ERROR = 0xEC ,
    ICUSB_ACK_STATUS_CMD_SET_VOLTAGE_ERROR = 0xEB ,
    ICUSB_ACK_STATUS_CMD_EN_SESSION_ERROR = 0xEA ,
    ICUSB_ACK_STATUS_NO_CARD = 0xE9 ,
};

enum ccid_berror_codes {
	CCID_BERROR_ICC_MUTE = 0xFE ,	
	CCID_BERROR_XFR_OVERRUN = 0xFC ,	
	CCID_BERROR_HW_ERROR = 0xFB ,	
};

enum ccid_bchainparameters_codes {
	CCID_CHAIN_B_E = 0x00 ,	
	CCID_CHAIN_B_C 	= 0x01 ,	
	CCID_CHAIN_C_E = 0x02 ,	
	CCID_CHAIN_C = 0x03 ,	
	CCID_CHAIN_N = 0x10 ,	
};


char * ccid_tx_add_header(int message_type, char *p_data_buf_orig, int header_room, int data_length_orig, int *p_data_length_new, unsigned short chain_parameter) ;

int ccid_rx_rm_header(char *p_data_buf_orig,  int data_length_orig, int *p_data_length_new, char **out_buf, unsigned short *chain_parameter, unsigned char *p_icc_status) ;

int ccid_usb_bulk_out_transfer(char *data, int data_length, int *transferred) ;


int ccid_usb_bulk_in_transfer(char *buf, int buf_size, int *transferred) ;

int ccid_init(s_icusb_device *icusb_device) ;

int ccid_deinit() ;

#endif

