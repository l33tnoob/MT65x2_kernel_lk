#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "icusb_smart_card.h"
#include "icusb_util.h"

static unsigned char sim_cmd_seq = 0;

s_icusb_device ut_icusb_device = 
{
	.usb_dev = NULL,
	.usb_device_handle = NULL, 

	.icusb_smartcard_num_ep = 0 ,

	.is_ready = 0 ,
} ;


void ut_LibusbPrintDevs(libusb_device **devs)
{
	libusb_device *usb_dev;
	int i = 0 ;
	libusb_device_handle *handle ;

	while ((usb_dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor device_descriptor;
		struct libusb_config_descriptor *config_descriptor; 
		int r ;
		int j = 0 ;
		
		r = libusb_get_device_descriptor(usb_dev, &device_descriptor);
		if (r < 0) {
			icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to get device descriptor");
			return;
		}

		icusb_print(PRINT_VERB, "\n\n +++++  the %d device   +++++\n", i ) ; 		
		icusb_print(PRINT_VERB, "	0x%04x:0x%04x (bus %d, device %d)\n",
					device_descriptor.idVendor, device_descriptor.idProduct,
					libusb_get_bus_number(usb_dev), libusb_get_device_address(usb_dev));
					
		icusb_print(PRINT_VERB, "[device descriptor] -- \n") ;
		icusb_print(PRINT_VERB, "\t bDeviceClass:0x%02x\n", device_descriptor.bDeviceClass) ;
		icusb_print(PRINT_VERB, "\t bDeviceSubClass:0x%02x\n", device_descriptor.bDeviceSubClass) ;
		icusb_print(PRINT_VERB, "\t bDeviceProtocol:0x%02x\n", device_descriptor.bDeviceProtocol) ;
		icusb_print(PRINT_VERB, "\t idVendor:0x%04x\n", device_descriptor.idVendor) ;
		icusb_print(PRINT_VERB, "\t idProduct:0x%04x\n", device_descriptor.idProduct) ;
		icusb_print(PRINT_VERB, "\t bNumConfigurations:%d\n", device_descriptor.bNumConfigurations) ;


		for (j=0 ; j<device_descriptor.bNumConfigurations ; j++){
			int k = 0 ;
			r = libusb_get_config_descriptor(usb_dev, j, &config_descriptor);
			if (r < 0) {
				icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to get config descriptor");
				return;
			}
			icusb_print(PRINT_VERB, "\t [configuration descriptor] -- \n") ;
			icusb_print(PRINT_VERB, "\t\t bConfigurationValue:%d\n", config_descriptor->bConfigurationValue) ;
			icusb_print(PRINT_VERB, "\t\t bNumInterfaces:%d\n", config_descriptor->bNumInterfaces) ;

			for (k=0 ; k<config_descriptor->bNumInterfaces ; k++){
				struct libusb_interface interf_descriptor;
				const struct libusb_endpoint_descriptor *endpoint_descriptor[3] = {NULL};
				//libusb_device_handle *handle ;
				int driver_exist = 0 ;
				struct libusb_class_usbicc_descriptor *uicc_descriptor;
				
				int l = 0 ;
				interf_descriptor = config_descriptor->interface[k] ;
				icusb_print(PRINT_VERB, "\t\t [interface descriptor] -- \n") ;
				icusb_print(PRINT_VERB, "\t\t\t num_altsetting:%d\n", interf_descriptor.num_altsetting) ;
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceNumber:%d\n", interf_descriptor.altsetting->bInterfaceNumber) ;
				icusb_print(PRINT_VERB, "\t\t\t bNumEndpoints:%d\n", interf_descriptor.altsetting->bNumEndpoints) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceClass:0x%02x\n", interf_descriptor.altsetting->bInterfaceClass) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceSubClass:0x%02x\n", interf_descriptor.altsetting->bInterfaceSubClass) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceProtocol:0x%02x\n", interf_descriptor.altsetting->bInterfaceProtocol) ;
				icusb_print(PRINT_VERB, "\t\t\t extra_length:%d\n", interf_descriptor.altsetting->extra_length) ;

				for (l=0 ; l<interf_descriptor.altsetting->bNumEndpoints ; l++){
					
					endpoint_descriptor[l] = &interf_descriptor.altsetting->endpoint[l] ;
					icusb_print(PRINT_VERB, "\t\t\t [endpoint descriptor] -- \n") ;
					icusb_print(PRINT_VERB, "\t\t\t\t bmAttributes:0x%02x\n", endpoint_descriptor[l]->bmAttributes) ;
					icusb_print(PRINT_VERB, "\t\t\t\t bEndpointAddress:0x%02x\n", endpoint_descriptor[l]->bEndpointAddress) ;
					icusb_print(PRINT_VERB, "\t\t\t\t wMaxPacketSize:%d\n", endpoint_descriptor[l]->wMaxPacketSize) ;					
				}

				if (interf_descriptor.altsetting->bInterfaceClass == 0x0b) /* smart card class*/
				{					
					uicc_descriptor = (struct libusb_class_usbicc_descriptor *)interf_descriptor.altsetting->extra ;
					
					if (interf_descriptor.altsetting->extra_length == 0x36 && uicc_descriptor->bDescriptorType == 0x21){ // CCID class specific 							
						icusb_print(PRINT_VERB, "\t\t\t [UICC class specific descriptor] -- \n") ;
						icusb_print(PRINT_VERB, "\t\t\t\t bLength:%d\n", uicc_descriptor->bLength) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bDescriptorType:0x%02x\n", uicc_descriptor->bDescriptorType) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bcdCCID:0x%04x\n", uicc_descriptor->bcdCCID) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bVoltageSupport:0x%02x\n", uicc_descriptor->bVoltageSupport) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwProtocols:0x%08x\n", uicc_descriptor->dwProtocols) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwMaxIFSD:0x%08x\n", uicc_descriptor->dwMaxIFSD) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwFeatures:0x%08x\n", uicc_descriptor->dwFeatures) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwMaxCCIDMessageLength:%d\n", uicc_descriptor->dwMaxCCIDMessageLength) ;
					}									
				}					
			}
		}							
	}
}

int ut_case_1_LibusbTest(void)
{
	libusb_device **devs;
	int r , cnt;

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 1 ==========> ut_case_1_LibusbTest\n");
	
	r = libusb_init(NULL);
	if (r < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to libusb_init\n");
		return r;
	}

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to libusb_get_device_list\n");
		return (int) cnt;
	}

	//ut_LibusbPrintDevs(devs); // libusb_device list 
 	
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 1 <==========\n");

	return 0 ;
}


int ut_case_IcusbOpen(void)
{
	int r ;

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case ==========> ut_case_IcusbOpen\n");
	
	r = icusb_open_dev(&ut_icusb_device) ;
	if (r < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to icusb_open_dev\n");
		return r;
	}

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case <==========\n");

	return 0 ;
}

int ut_case_IcusbClose(void)
{
	int r ;

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case ==========> ut_case_IcusbClose\n");

	r = icusb_close_dev(&ut_icusb_device) ;
	if (r < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to icusb_close_dev\n");
		return r;
	}

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case <==========\n");

	return 0 ;
}


int ut_case_2_IcusbCCIDPowerOnPowerOff(void)
{	
	int r;
	ssize_t cnt;
	char buf[256] ;
	int transferred ;
	unsigned int bbb ;
	

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 3 ==========> ut_case_3_IcusbBulkTransfer\n");
 
	// test : test the bulk out transfer (power on)	
	struct ccid_pc2rdr_icc_pwr_on_msg *pwr_on_msg = (struct ccid_pc2rdr_icc_pwr_on_msg *)buf ;
	pwr_on_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_ICCPOWERON ;
	pwr_on_msg->dwLength= 0x0 ;
	pwr_on_msg->bSlot = 0x0 ;
	pwr_on_msg->bSeq = sim_cmd_seq++ ;
	pwr_on_msg->bPowerSelect= 0x00 ;
	pwr_on_msg->abRFU[0]= 0x00 ;
	pwr_on_msg->abRFU[1]= 0x00 ;
	//r = libusb_bulk_transfer(handle, dwEndpoint, buf, sizeof(struct iccd_pc2rdr_icc_pwr_on_msg), &transferred, BULK_TRANSFER_TIMEOUT) ;
	r = icusb_smartcard_data_bulk_out_transfer(buf, sizeof(struct ccid_pc2rdr_icc_pwr_on_msg), &transferred) ;
	icusb_print(PRINT_INFO, "[ICUSB] icusb_smartcard_data_bulk_out_transfer, return: %d,transferred bytes: %d\n", r, transferred) ;
	
	// test : test the bulk in transfer (get ATR)
	sleep(1);
	//r = libusb_bulk_transfer(handle, dwEndpoint, buf, sizeof(buf), &transferred, BULK_TRANSFER_TIMEOUT) ;
	r = icusb_smartcard_data_bulk_in_transfer(buf, sizeof(struct ccid_pc2rdr_icc_pwr_on_msg), &transferred) ;
	struct ccid_rdr2pc_data_block_msg*data_block_msg = (struct ccid_rdr2pc_data_block_msg*)buf ;						
	icusb_print(PRINT_INFO, "[ICUSB] iccd_rdr2pc_data_block_msg, return: %d,transferred bytes: %d\n", r, transferred) ;
	icusb_print(PRINT_VERB, "data_block_msg->bMessageType : 0x%02x\n", data_block_msg->bMessageType) ; // should be ICUSB_BULK_IN_RDR_TO_PC_DATABLOCK
	icusb_print(PRINT_VERB, "data_block_msg->dwLength : 0x%02x\n", data_block_msg->dwLength) ;							
	icusb_print(PRINT_VERB, "data_block_msg->bSeq : 0x%02x\n", data_block_msg->bSeq) ;
	icusb_print(PRINT_VERB, "data_block_msg->bStatus : 0x%02x\n", data_block_msg->bStatus) ;
	icusb_print(PRINT_VERB, "data_block_msg->bError : 0x%02x\n", data_block_msg->bError) ;
	icusb_print(PRINT_VERB, "data_block_msg->bError : 0x%02x\n", data_block_msg->bChainParameter) ;							

	for (bbb =0 ; bbb<data_block_msg->dwLength ; bbb++){
		icusb_print(PRINT_VERB, "0x%02x ",data_block_msg->abData[bbb]) ;
	}
	icusb_print(PRINT_VERB, "\n") ;
	
	// test : test the bulk in transfer  (power off)			
	struct ccid_pc2rdr_icc_pwr_off_msg *pwr_off_msg = (struct ccid_pc2rdr_icc_pwr_off_msg *)buf ;
	pwr_off_msg->bMessageType = CCID_BULK_OUT_PC_TO_RDR_ICCPOWEROFF ;
	pwr_off_msg->dwLength= 0x0 ;
	pwr_off_msg->bSlot = 0x0 ;
	pwr_off_msg->bSeq = sim_cmd_seq++ ;
	pwr_off_msg->abRFU[0]= 0x00 ;
	pwr_off_msg->abRFU[1]= 0x00 ;
	pwr_off_msg->abRFU[2]= 0x00 ;

	r = icusb_smartcard_data_bulk_out_transfer(buf, sizeof(struct ccid_pc2rdr_icc_pwr_off_msg), &transferred) ;
	icusb_print(PRINT_INFO, "[ICUSB] iccd_pc2rdr_icc_pwr_off_msg, return: %d,transferred bytes: %d\n", r, transferred) ;

	
	// test : test the bulk in transfer (get Slot Status)
	sleep(1);	
	r = icusb_smartcard_data_bulk_in_transfer(buf, sizeof(buf), &transferred) ;
	struct ccid_rdr2pc_slot_status_msg*slot_status = (struct ccid_rdr2pc_slot_status_msg*)buf ; 					
	icusb_print(PRINT_INFO, "[ICUSB] iccd_rdr2pc_data_block_msg, return: %d,transferred bytes: %d\n", r, transferred) ; // should be ICUSB_BULK_IN_RDR_TO_PC_SLOTSTATUS
	icusb_print(PRINT_VERB, "slot_status->bMessageType : 0x%02x\n", slot_status->bMessageType) ;
	icusb_print(PRINT_VERB, "slot_status->bSeq : 0x%02x\n", slot_status->bSeq) ;
	icusb_print(PRINT_VERB, "slot_status->bStatus : 0x%02x\n", slot_status->bStatus) ;
	icusb_print(PRINT_VERB, "slot_status->bError : 0x%02x\n", slot_status->bError) ;

	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 3 <==========\n");

	return 0 ;
}


int ut_case_3_xxx(void)
{
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 3 ==========> \n");
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 3 <==========\n");
	return 0 ; 

}

int ut_case_4_xxx(void)
{
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 4 ==========> \n");
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 4 <==========\n");
	return 0 ; 
}

int ut_case_5_xxx(void)
{
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 5 ==========> \n");
	icusb_print(PRINT_INFO, "[ICUSB][UT] Test Case 5 <==========\n");
	return 0 ; 

}




int main(void)
{
	ut_case_1_LibusbTest() ;
	ut_case_IcusbOpen() ;
	ut_case_2_IcusbCCIDPowerOnPowerOff() ;
	ut_case_IcusbClose() ;
	
	return 0;
}



