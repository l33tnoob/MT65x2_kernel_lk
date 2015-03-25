#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "icusb_ccid.h"

typedef struct _s_ccid_device {
	s_icusb_device * parant_icusb_device ;	
	unsigned char cmdSeqNumber ;	
	int is_ready ;	
	
}s_ccid_device;

static s_ccid_device ccid_device ={
		.parant_icusb_device = NULL ,
		.is_ready = 0 ,
		.cmdSeqNumber = 0 ,
	} ;

int ccid_init(s_icusb_device *icusb_device)
{	
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> ccid_init\n");
	if (icusb_device && icusb_device->is_ready){
		if (ccid_device.is_ready == 0){
			ccid_device.is_ready = 1 ;
			ccid_device.cmdSeqNumber = 0 ;
			ccid_device.parant_icusb_device = icusb_device ;
		}else{
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ===> ccid_init has been inited before\n");
		}
	}else{
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ===> ccid_init Fail\n");
		return ICUSB_UICC_NOT_FOUND ;
	}

	icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== ccid_init OK\n");
	
	return ICUSB_OK ;
}

int ccid_deinit(void)
{	
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> ccid_deinit\n");
	ccid_device.is_ready = 0 ;
	ccid_device.cmdSeqNumber = 0 ;
	ccid_device.parant_icusb_device = NULL ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> ccid_deinit OK\n");
	return ICUSB_OK ;
}

int ccid_usb_bulk_out_transfer(char *data, int data_length, int *transferred)
{	
	int ret = ICUSB_OK ;
	s_icusb_device *icusb_device ; 

	icusb_print(PRINT_VERB, "[ICUSB] ===> ccid_usb_bulk_out_transfer, length : %d\n", data_length);
	if (!ccid_device.parant_icusb_device){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_usb_bulk_out_transfer, the icusb_device is not exist\n");
		return ICUSB_UICC_NOT_FOUND ;
	}

	icusb_device = ccid_device.parant_icusb_device ;

	if (icusb_device->is_ready){
		libusb_claim_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);
		
		ret = libusb_bulk_transfer(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_bulk_out_ep, (unsigned char *)data, data_length, transferred, BULK_TRANSFER_OUT_TIMEOUT) ;

		icusb_print(PRINT_INFO, "[ICUSB] ccid_usb_bulk_out_transfer, return : %d, length: %d, transferred: %d\n", ret, data_length, *transferred);
		
		libusb_release_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);

		if (ret == 0){
			ret = ICUSB_OK;
		}else if (ret == LIBUSB_ERROR_TIMEOUT){
			ret = ICUSB_NO_DATA ;
		}else{
			ret = ICUSB_DEV_GENERAL_ERR ;
		}
		
	}else{
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_usb_bulk_out_transfer with no UICC device\n");
		ret = ICUSB_UICC_NOT_FOUND ;
	}

	icusb_print(PRINT_VERB, "[ICUSB] <=== ccid_usb_bulk_out_transfer\n");
	return ret ;
}

int ccid_usb_bulk_in_transfer(char *buf, int buf_size, int *transferred)
{	
	int ret = ICUSB_OK;
	s_icusb_device *icusb_device ; 

	icusb_print(PRINT_VERB, "[ICUSB] ===> ccid_usb_bulk_in_transfer, buf_size : %d\n", buf_size);
	
	if (!ccid_device.parant_icusb_device){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_usb_bulk_in_transfer, the icusb_device is not exist\n");
		return ICUSB_UICC_NOT_FOUND ;
	}

	icusb_device = ccid_device.parant_icusb_device ;
	
	if (icusb_device->is_ready){
		libusb_claim_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);
		
		ret = libusb_bulk_transfer(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_bulk_in_ep, (unsigned char *)buf, buf_size, transferred, BULK_TRANSFER_IN_TIMEOUT) ;

		libusb_release_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);

		if (ret == 0){
			if (*transferred > 0){
				icusb_print(PRINT_INFO, "[ICUSB][INFO] ccid_usb_bulk_in_transfer, return : %d, transferred: %d\n", ret, *transferred);

				if (*transferred == buf_size){
					icusb_print(PRINT_ERROR, "[ICUSB][INFO] ccid_usb_bulk_in_transfer, get the packet size %d may be larger than buffer size %d\n", *transferred, buf_size);
				}
				
			}else{
				icusb_print(PRINT_VERB, "[ICUSB][INFO] ccid_usb_bulk_in_transfer, return : %d, transferred: %d\n", ret, *transferred);
			}	
			ret = ICUSB_OK;
		}else if (ret == LIBUSB_ERROR_TIMEOUT){
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_usb_bulk_in_transfer no data, return : %d, transferred: %d\n", ret, *transferred);
			ret = ICUSB_NO_DATA ;
		}else{
			icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_usb_bulk_in_transfer, return : %d, transferred: %d\n", ret, *transferred);
			ret = ICUSB_DEV_GENERAL_ERR ;
		}
	}else{
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_usb_bulk_in_transfer with no UICC device\n");	
		ret = ICUSB_UICC_NOT_FOUND ;
	}

	icusb_print(PRINT_VERB, "[ICUSB] <=== ccid_usb_bulk_in_transfer\n");
	
	return ret;	
}

int ccid_rx_rm_header(char *p_data_buf_orig,  int data_length_orig, int *p_data_length_new, char **out_buf, unsigned short* chain_parameter, unsigned char *p_icc_status)
{
	char *tmp_buf ;	
	char bMessageType;
	int ret = ICUSB_OK ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> ccid_rx_rm_header\n");
	
	*chain_parameter = 0 ;
	bMessageType= *((char*)p_data_buf_orig) ;
	
	tmp_buf = p_data_buf_orig + CCID_HEADER_SIZE ;

	*p_data_length_new = data_length_orig -CCID_HEADER_SIZE ;

	*out_buf = tmp_buf ;

	*p_icc_status = ICUSB_ACK_STATUS_OK;

	if (bMessageType == CCID_BULK_IN_RDR_TO_PC_DATABLOCK){
		struct ccid_rdr2pc_data_block_msg *rdr2pc_data_block_msg = (struct ccid_rdr2pc_data_block_msg*)p_data_buf_orig ;

		icusb_print(PRINT_WARN, "[ICUSB][INOF] ccid_data_bulk_in_rm_header, RDR_TO_PC DATABLOCK \n");	
		
		if ((rdr2pc_data_block_msg->bStatus & CCID_BSTATUS_FIELD_BMICCSTATUS) != 0){
			*p_icc_status = ICUSB_CCID_STATUS_NO_CARD ;
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_data_bulk_in_rm_header, slot status : %02x\n", rdr2pc_data_block_msg->bStatus);	
		}else if ((rdr2pc_data_block_msg->bStatus & CCID_BSTATUS_FIELD_BMCOMMANDSTATUS) != 0x0){
			*p_icc_status = ICUSB_CCID_STATUS_TIMEOUT ;
		}
		
		if ((rdr2pc_data_block_msg->bError & 0xE0) == 0xE0 ){
			*p_icc_status = ICUSB_CCID_STATUS_CMD_ERROR ;
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_data_bulk_in_rm_header, error : %02x\n", rdr2pc_data_block_msg->bError);	
		}
		//if (rdr2pc_data_block_msg->bChainParameter == CCID_CHAIN_B_C || rdr2pc_data_block_msg->bChainParameter == CCID_CHAIN_C){
		//	*more_data = 1 ;
		//}
		*chain_parameter = rdr2pc_data_block_msg->bChainParameter ;
		
		ret = ICUSB_UICC_DATA_RESP ;		
	}else if (bMessageType == CCID_BULK_IN_RDR_TO_PC_SLOTSTATUS){
		struct ccid_rdr2pc_slot_status_msg *rdr2pc_slot_status_msg = (struct ccid_rdr2pc_slot_status_msg*)p_data_buf_orig ;

		icusb_print(PRINT_WARN, "[ICUSB][INOF] ccid_data_bulk_in_rm_header, RDR_TO_PC SLOTSTATUS \n");	
		
		if ((rdr2pc_slot_status_msg->bStatus & CCID_BSTATUS_FIELD_BMICCSTATUS) != 0){
			*p_icc_status = ICUSB_CCID_STATUS_NO_CARD ;
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_data_bulk_in_rm_header, slot status error: %02x\n", rdr2pc_slot_status_msg->bStatus);	
		}else if ((rdr2pc_slot_status_msg->bStatus & CCID_BSTATUS_FIELD_BMCOMMANDSTATUS) != 0x0){
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_data_bulk_in_rm_header, slot status error: %02x\n", rdr2pc_slot_status_msg->bStatus);	
			*p_icc_status = ICUSB_CCID_STATUS_TIMEOUT ;
		}
		
		if ((rdr2pc_slot_status_msg->bError & 0xE0) == 0xE0 ){
			*p_icc_status = ICUSB_CCID_STATUS_CMD_ERROR ;
			icusb_print(PRINT_WARN, "[ICUSB][WARN] ccid_data_bulk_in_rm_header, error : %02x\n", rdr2pc_slot_status_msg->bError);	
		}
		
		ret = ICUSB_UICC_CTRL_RESP ;
	}else{
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_data_bulk_in_rm_header with unkown Message Type : %d\n", bMessageType);	
		ret = ICUSB_UICC_GENERAL_ERR ;
	}

	icusb_print(PRINT_VERB, "[ICUSB] <=== ccid_rx_rm_header, icc_status is %d, return %d \n", *p_icc_status, ret);
	return ret ;
}

char * ccid_tx_add_header(int message_type, char *p_data_buf_orig, int header_room, int data_length_orig, int *p_data_length_new, unsigned short chain_parameter)
{
	char *out_buf ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> ccid_tx_add_header, message_type is %d, chain_parameter is 0x%x\n", message_type, chain_parameter);
	
	if (header_room < CCID_HEADER_SIZE){
		// Assert
		icusb_assert() ;
		//return NULL ;
	}

	out_buf = p_data_buf_orig - CCID_HEADER_SIZE ;

	if (message_type == CCID_BULK_OUT_PC_TO_RDR_ICCPOWERON){
		struct ccid_pc2rdr_icc_pwr_on_msg *pc2rdr_icc_pwr_on_msg = (struct ccid_pc2rdr_icc_pwr_on_msg *)out_buf ;
		pc2rdr_icc_pwr_on_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_ICCPOWERON;
		pc2rdr_icc_pwr_on_msg->dwLength = 0x00000000 ;
		pc2rdr_icc_pwr_on_msg->bSlot = CCID_SIM_SLOT_1 ;
		pc2rdr_icc_pwr_on_msg->bSeq = ccid_device.cmdSeqNumber++ ;
		pc2rdr_icc_pwr_on_msg->bPowerSelect = 0x0 ; // Automatic Voltage Selection
		memset(pc2rdr_icc_pwr_on_msg->abRFU,0,sizeof(pc2rdr_icc_pwr_on_msg->abRFU)) ;

		*p_data_length_new = sizeof(struct ccid_pc2rdr_icc_pwr_on_msg) ;
		
	}else if(message_type == CCID_BULK_OUT_PC_TO_RDR_ICCPOWEROFF){
		struct ccid_pc2rdr_icc_pwr_off_msg *pc2rdr_icc_pwr_off_msg = (struct ccid_pc2rdr_icc_pwr_off_msg *)out_buf ;
		pc2rdr_icc_pwr_off_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_ICCPOWEROFF;
		pc2rdr_icc_pwr_off_msg->dwLength = 0x00000000 ;
		pc2rdr_icc_pwr_off_msg->bSlot = CCID_SIM_SLOT_1 ;
		pc2rdr_icc_pwr_off_msg->bSeq = ccid_device.cmdSeqNumber++ ;
		memset(pc2rdr_icc_pwr_off_msg->abRFU,0,sizeof(pc2rdr_icc_pwr_off_msg->abRFU)) ;

		*p_data_length_new = sizeof(struct ccid_pc2rdr_icc_pwr_off_msg) ;
		
	}else if(message_type == CCID_BULK_OUT_PC_TO_RDR_GETSLOTSTATUS){
		struct ccid_pc2rdr_get_slot_status_msg *pc2rdr_get_slot_status_msg = (struct ccid_pc2rdr_get_slot_status_msg *)out_buf ;
		pc2rdr_get_slot_status_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_GETSLOTSTATUS;
		pc2rdr_get_slot_status_msg->dwLength = 0x00000000 ;
		pc2rdr_get_slot_status_msg->bSlot = CCID_SIM_SLOT_1 ;
		pc2rdr_get_slot_status_msg->bSeq = ccid_device.cmdSeqNumber++ ;
		memset(pc2rdr_get_slot_status_msg->abRFU,0,sizeof(pc2rdr_get_slot_status_msg->abRFU)) ;

		*p_data_length_new = sizeof(struct ccid_pc2rdr_get_slot_status_msg) ;
		
	}else if(message_type == CCID_BULK_OUT_PC_TO_RDR_XFRBLOCK){
		struct ccid_pc2rdr_xfr_block_msg *pc2rdr_xfr_block_msg = (struct ccid_pc2rdr_xfr_block_msg *)out_buf ;
		pc2rdr_xfr_block_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_XFRBLOCK;
		pc2rdr_xfr_block_msg->dwLength = data_length_orig ;
		pc2rdr_xfr_block_msg->bSlot = CCID_SIM_SLOT_1 ;
		pc2rdr_xfr_block_msg->bSeq = ccid_device.cmdSeqNumber++ ;
		pc2rdr_xfr_block_msg->bBWI = 0x10 ; // Block Waiting Time
		pc2rdr_xfr_block_msg->wLevelParameter = chain_parameter ;

		*p_data_length_new = data_length_orig + CCID_HEADER_SIZE ;
		
	}else{
		// Assert
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ccid_data_bulk_out_add_header with unknown Message Type : %d\n", message_type);	
		icusb_assert() ;
		//return NULL ;
	}	

	return out_buf ;

}

#if 0 /*ICCD related parts*/
#define ICCD_V_A_TRANSFER_TIMEOUT	(5*1000)
#define ICCD_V_B_TRANSFER_TIMEOUT	(5*1000)

enum icusb_control_transfer_requst_type {
	ICUSB_REQ_TYPE_ICC_POWER_ON = 0x62 ,	
	ICUSB_REQ_TYPE_ICC_POWER_OFF = 0x63,		
	ICUSB_REQ_TYPE_XFR_BLOCK = 0x65,		
	ICUSB_REQ_TYPE_DATA_BLOCK = 0x6F,
	ICUSB_REQ_TYPE_GET_ICC_STATUS = 0xA0,
	ICUSB_REQ_TYPE_SLOT_STATUS = 0x81,
};

enum icusb_control_A_status_byte {
	ICUSB_CONTROL_A_STATUS_BUSY = 0x4F ,	
	ICUSB_CONTROL_A_STATUS_READY = 0x20 ,	

	ICUSB_CONTROL_A_STATUS_MUTE = 0x80 ,	
	ICUSB_CONTROL_A_STATUS_READY_RCV_APDU = 0x00 ,	
};

enum icusb_control_B_bresponse_type {
	ICUSB_CONTROL_B_RSP_ABDATA_BY_REQUEST = 0x00 ,	
	ICUSB_CONTROL_B_RSP_ABDATA_IS_STATUS = 0x40 ,	
	ICUSB_CONTROL_B_RSP_ABDATA_POLLING = 0x80 ,	
	
	ICUSB_CONTROL_B_EXTRSP_RESP_BEG_END = 0x00 ,	
	ICUSB_CONTROL_B_EXTRSP_RESP_BEG_CONT = 0x01 ,	
	ICUSB_CONTROL_B_EXTRSP_ABDATA_CON_END = 0x02 ,	
	ICUSB_CONTROL_B_EXTRSP_ABDATA_CON = 0x03 ,	
	ICUSB_CONTROL_B_EXTRSP_ABDATA_EMPTY = 0x10 ,	
};

// The return v
int icusb_data_control_vA_transfer(s_icusb_device *icusb_device, enum icusb_control_transfer_requst_type request_type, unsigned char bLevelParameter, unsigned char *data, int data_length, int *transferred) 
{	
	int r ;
	uint8_t bmRequestType, bRequest ;
	uint16_t wVaue, wIndex, wLength ;
	
	if (icusb_device->is_ready){
		if (request_type == ICUSB_REQ_TYPE_ICC_POWER_ON){
			bmRequestType = 0xA1 ; // 10100001B
			bRequest= ICUSB_REQ_TYPE_ICC_POWER_ON ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = data_length ;  // Return ATR , so the data_length should be larger as possible
		}else if(request_type == ICUSB_REQ_TYPE_ICC_POWER_OFF){
			bmRequestType = 0x21 ; // 00100001B
			bRequest= ICUSB_REQ_TYPE_ICC_POWER_OFF ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;	
			wLength = 0x0000 ;  // No Data stage
		}else if(request_type == ICUSB_REQ_TYPE_XFR_BLOCK){
			bmRequestType = 0x21 ; // 00100001B
			bRequest= ICUSB_REQ_TYPE_XFR_BLOCK ;
			wVaue = (0xFF00 & (bLevelParameter<<8)) ; 
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = data_length ;  // send Command APDU
		}else if(request_type == ICUSB_REQ_TYPE_DATA_BLOCK){
			bmRequestType = 0xA1 ; // 10100001B 
			bRequest= ICUSB_REQ_TYPE_DATA_BLOCK ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = data_length ;  // return Response APDU
		}else if(request_type == ICUSB_REQ_TYPE_GET_ICC_STATUS){
			bmRequestType = 0xA1 ; // 10100001B
			bRequest= ICUSB_REQ_TYPE_GET_ICC_STATUS ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = 0x0001 ;  // return StatusByte
		}else{
			return -98 ;
		}
		libusb_claim_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);

		r = libusb_control_transfer(icusb_device->usb_device_handle, bmRequestType, bRequest , wVaue, wIndex, data, wLength, ICCD_V_A_TRANSFER_TIMEOUT );

		if (r>=0){
			*transferred = r ;
		}else{
			*transferred = 0 ;
		}
		
		libusb_release_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);
		
		return r ;
	}else{
		return -99 ;
	}
	
	return 0;

}

int icusb_data_control_vB_transfer(s_icusb_device *icusb_device, enum icusb_control_transfer_requst_type request_type, unsigned char bLevelParameter, unsigned char *data, int data_length, int *transferred) 
{	
	int r ;
	uint8_t bmRequestType, bRequest ;
	uint16_t wVaue, wIndex, wLength ;
	
	if (icusb_device->is_ready){
		if (request_type == ICUSB_REQ_TYPE_ICC_POWER_ON){
			bmRequestType = 0x21 ; // 10100001B
			bRequest= ICUSB_REQ_TYPE_ICC_POWER_ON ;
			wVaue = 0x0001 ; // bRFU + bReserved=01h
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = 0x0000 ;
		}else if(request_type == ICUSB_REQ_TYPE_ICC_POWER_OFF){
			bmRequestType = 0x21 ; // 00100001B
			bRequest= ICUSB_REQ_TYPE_ICC_POWER_OFF ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;	
			wLength = 0x0000 ;
		}else if(request_type == ICUSB_REQ_TYPE_XFR_BLOCK){
			bmRequestType = 0x21 ; // 00100001B
			bRequest= ICUSB_REQ_TYPE_XFR_BLOCK ;
			wVaue = (0xFF00 & (bLevelParameter<<8)) ; 
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = data_length ;  // Send the abData
		}else if(request_type == ICUSB_REQ_TYPE_DATA_BLOCK){
			bmRequestType = 0xA1 ; // 10100001B 
			bRequest= ICUSB_REQ_TYPE_DATA_BLOCK ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			if (data_length <= 4){
				return -97 ; // the DATA BLOCK request should has the length greater than or equal to 4
			}
			wLength = data_length ;  // Return the bResponseType + abData
		}else if(request_type == ICUSB_REQ_TYPE_SLOT_STATUS){
			bmRequestType = 0xA1 ; // 10100001B
			bRequest= ICUSB_REQ_TYPE_SLOT_STATUS ;
			wVaue = 0x0000 ;
			wIndex = (0x00FF & icusb_device->icusb_smartcard_intf_number) ;
			wLength = 0x0003 ;  // Return bStatus+bError+bReserved(0x00)
		}else{
			return -98 ;
		}
		libusb_claim_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);

		r = libusb_control_transfer(icusb_device->usb_device_handle, bmRequestType, bRequest , wVaue, wIndex, data, wLength, ICCD_V_B_TRANSFER_TIMEOUT );

		if (r>=0){
			*transferred = r ;
		}else{
			*transferred = 0 ;
		}
		
		libusb_release_interface(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number);
		
		return r ;
	}else{
		return -99 ;
	}
	
	return 0;

}

#define USBDEVFS_ICUSB_SET_PWRINFO _IOR('U', 80, unsigned int) // copy_from_user
#define USBDEVFS_ICUSB_GET_PWRINFO _IOR('U', 81, unsigned int) // copy_to_user 


struct device_handle_priv {
	int fd;
};


struct usbdevfs_icusb_powerinfo {
	unsigned char bVoltageClass;
	unsigned char bMaxCurrent;
};


int icusb_pwr_neg_set_power(libusb_device_handle *handle, struct usbdevfs_icusb_powerinfo *pwr_info)
{
	//struct device_handle_priv *priv = (struct device_handle_priv *)handle->os_priv ;
	//int fd = priv->fd;
	int fd = libusb_get_handle_fd(handle) ;

	icusb_print(PRINT_INFO, "[ICUSB] the fd is %d\n", fd);
	//int *xxx = (int *)handle->os_priv ;
	//int fd = *xxx ;
	//int fd = *((int *)(handle->os_priv)) ;
	int r = ioctl(fd, USBDEVFS_ICUSB_SET_PWRINFO, pwr_info);
	if (r) {
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to ioctl USBDEVFS_ICUSB_SET_PWRINFO, return %d, errno %d\n", r, errno);
		return -1 ;
	}
	
	return 0;
}

int icusb_pwr_neg_get_power(libusb_device_handle *handle, struct usbdevfs_icusb_powerinfo *pwr_info)
{
	
	//struct device_handle_priv *priv = (struct device_handle_priv *)handle->os_priv ;
	//int fd = priv->fd;
	int fd = libusb_get_handle_fd(handle) ;
	int r = ioctl(fd, USBDEVFS_ICUSB_GET_PWRINFO, pwr_info);
	if (r) {
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to ioctl USBDEVFS_ICUSB_SET_PWRINFO, return %d, errno %d\n", r, errno);
		return -1 ;
	}

	icusb_print(PRINT_INFO, "[ICUSB] icusb_pwr_neg_get_power OK, bVoltageClass: 0x%02x, bMaxCurrent: 0x%02x\n", pwr_info->bVoltageClass, pwr_info->bMaxCurrent);
	
	return 0;

}


#endif

