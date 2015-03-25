#ifndef __ICUSB_SMART_CARD_H__
#define __ICUSB_SMART_CARD_H__

#include "icusb_ccid.h"

#define ICUSB_SMARTCARD_CLASS_HEADER_ROOM (CCID_HEADER_SIZE) // Max CCID Header Room is 10

#define icusb_smartcard_init(p_icusb_device) ccid_init(p_icusb_device) 

#define icusb_smartcard_deinit() ccid_deinit()

//int ccid_data_bulk_out_transfer(s_icusb_device *icusb_device, unsigned char *data, int data_length, int *transferred) ;
#define icusb_smartcard_data_bulk_out_transfer(p_data_buf, data_len, p_transferred) ccid_usb_bulk_out_transfer(p_data_buf, data_len, p_transferred) 

//int ccid_data_bulk_in_transfer(s_icusb_device *icusb_device, unsigned char *buf, int buf_size, int *transferred) ;
#define icusb_smartcard_data_bulk_in_transfer(p_data_buf, data_len, p_transferred) ccid_usb_bulk_in_transfer(p_data_buf, data_len, p_transferred) 

#define icusb_smartcard_rm_rx_header(p_data_buf_orig, data_length_orig, p_data_length_new, pp_data_buf_new, p_chain_parameter, p_icc_status) ccid_rx_rm_header(p_data_buf_orig, data_length_orig, p_data_length_new, pp_data_buf_new, p_chain_parameter, p_icc_status) 

#define icusb_smartcard_add_tx_header(message_type, p_data_buf_orig , header_room, data_length_orig, p_data_length_new, chain_parameter) ccid_tx_add_header(message_type, p_data_buf_orig, header_room, data_length_orig, p_data_length_new, chain_parameter) 

#endif
