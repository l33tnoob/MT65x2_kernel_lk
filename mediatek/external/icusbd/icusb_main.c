#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "icusb_main.h"
#include "icusb_ccci.h"
#include "icusb_smart_card.h"
#include "icusb_util.h"


static volatile unsigned char current_icusb_voltage = ICUSB_VOL_18;        /* 1:main() received exit signal */
pthread_t icusb_thread_tid;
pthread_t icusb_recover_thread_tid;
static int daemon_exit = 0 ;

static s_icusb_device icusb_device = 
{
	.usb_dev = NULL,
	.usb_device_handle = NULL, 

	.icusb_smartcard_num_ep = 0 ,

	.is_ready = 0 ,
} ;

#if ICUSB_UT_NO_USB
static char ut_rx_sudo_usb_packet_buff[ICUSB_TRANSFER_RX_BUFF_SIZE] ;
static unsigned int ut_rx_sudo_usb_packet_length = 0 ;

int icusb_fill_ut_rx_buff(char *payload_buff , int payload_buff_size)
{
	if (payload_buff_size > ICUSB_TRANSFER_RX_BUFF_SIZE){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_fill_ut_rx_buff, the payload is larger than rx buffer size : %d\n", ICUSB_TRANSFER_RX_BUFF_SIZE);
		return ICUSB_ERROR_PACKET;
	}
	memcpy(ut_rx_sudo_usb_packet_buff, payload_buff, payload_buff_size) ;
	ut_rx_sudo_usb_packet_length = payload_buff_size ;

	return 0 ;

}
#endif


// recieve USB port 
// return the buff with [ICUSB_SMARTCARD_CLASS_HEADER_ROOM] [APDU] 
int icusb_rx_usb_read(char *rx_buff , int rx_buff_size, unsigned int header_room, int *usb_port_read_length , char **p_from_usb_buff)
{	
	//char rx_buff[CCCI_TTY_RX_BUFF_SIZE] ;
	char *tmp_usb_buff = NULL;
	
	char *to_ccci_buff ;
	int apdu_length = 0;
	int tmp_length = 0;
	int ret ;
	ssize_t ret_val;
	int has_read = 0;

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_rx_usb_read\n");
	
	if(header_room < sizeof(s_ccci_mtk_header)){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] the header_room is not enough for icusb_rx_usb_read\n");
		return ICUSB_HEADERROOM_ERROR;
	}
		
	//memset(rx_buff, 0, rx_buff_size);

	/* Receive from USB */
	// only reserved the header room for MTK header 
	tmp_usb_buff = rx_buff + header_room ; //sizeof(s_ccci_mtk_header)

#if ICUSB_UT_NO_USB
	if (ut_rx_sudo_usb_packet_length==0){
		return ICUSB_RX_NO_DATA ;
	}
	
	memcpy(tmp_usb_buff , ut_rx_sudo_usb_packet_buff, ut_rx_sudo_usb_packet_length) ;
	tmp_length = ut_rx_sudo_usb_packet_length ;	

    ut_rx_sudo_usb_packet_length=0 ;
    
#else
	ret = icusb_smartcard_data_bulk_in_transfer(tmp_usb_buff, (rx_buff_size-header_room), &tmp_length) ;

	if (ret == ICUSB_NO_DATA){
		return ICUSB_RX_NO_DATA ;
	}

	// tmp_usb_buff : [ICUSB_SMARTCARD_CLASS_HEADER_ROOM] [APDU] 
	// tmp_length : ICUSB_SMARTCARD_CLASS_HEADER_ROOM + APDU
	
	if (ret){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_smartcard_data_bulk_in_transfer fail.\n");
		return ICUSB_ERROR_PACKET ;
	}

	if (tmp_length<=0){
		icusb_print(PRINT_VERB, "[ICUSB][VERBOSE] icusb_smartcard_data_bulk_in_transfer get 0  data\n");
		return ICUSB_RX_NO_DATA ;
	}

#endif
	//icusb_print(PRINT_INFO, "UT USB RXRXRX payload : \n");
	
	while (has_read < tmp_length)
	{
		icusb_print(PRINT_ERROR, "%02x ",*(tmp_usb_buff+has_read));
		has_read++; // the ret is the sent length
	}
	icusb_print(PRINT_INFO, "\n");
    

	*usb_port_read_length = tmp_length ;
	icusb_print(PRINT_VERB, "[ICUSB] icusb_smartcard_data_bulk_in_transfer get data length: %d\n", tmp_length);

	*p_from_usb_buff = tmp_usb_buff ;	

	icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_rx_usb_read\n");
	
    return ICUSB_RX_GOOD_PACKET ;		

}

// recieve from CCCI TTY port , and send to USB
int icusb_tx_usb_write(char *to_usb_buff , int to_usb_length)
{
#define TX_USB_ERR_CNT (5)
	int usb_has_sent = 0;
	int usb_port_write_length = 0;
	int ret ;
	int error_cnt = 0 ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_tx_usb_write\n");

	/* Sent to USB */			
#if ICUSB_UT_NO_USB
	icusb_print(PRINT_INFO, "UT USB TXTXTX payload : \n");
	while (usb_has_sent < to_usb_length)
    {
		icusb_print(PRINT_ERROR, "%02x ",*(to_usb_buff+usb_has_sent));
		usb_has_sent++; // the ret is the sent length
    }
    icusb_print(PRINT_INFO, "\n");
#else
	while (usb_has_sent < to_usb_length){
		ret = icusb_smartcard_data_bulk_out_transfer(to_usb_buff+usb_has_sent, (to_usb_length-usb_has_sent), &usb_port_write_length) ;
		if (ret){
			icusb_print(PRINT_WARN, "[ICUSB][WARN] icusb_smartcard_data_bulk_out_transfer error, ret = %d\n", ret);
			error_cnt ++ ;
			usleep(ICUSB_TX_ERR_DELAY_US); 
			if (error_cnt>TX_USB_ERR_CNT){
				icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_smartcard_data_bulk_out_transfer error more than %d times\n", TX_USB_ERR_CNT);
				return ICUSB_TX_TRANSFER_ERROR ;
			}
			continue ;
		}
		if (usb_port_write_length<0){
			icusb_print(PRINT_ERROR, "[ICUSB][WARN] icusb_smartcard_data_bulk_out_transfer sent 0 data\n");
			//return ICUSB_CCCI_USB_TX_EMPTY ;
			continue ;
		}else{
			usb_has_sent += usb_port_write_length ;
		}
	}
#endif 

	icusb_print(PRINT_VERB, "[ICUSB] icusb_smartcard_data_bulk_out_transfer sent data length: %d\n", usb_has_sent);

	icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_tx_usb_write\n");

	return ICUSB_OK;		

}

// handle the class specific action
// p_apdu_buff is the buff with [APDU] 
// return the proccess result of packet category
int icusb_rx_class_handle(char *from_usb_buff, int usb_port_read_length, int *apdu_length, char **p_apdu_buff, unsigned short* chain_parameter, unsigned char *icc_status)
{	
	char *apdu_buff = NULL;
	int tmp_length = 0, ret;
	unsigned short tmp_chain_parameter =0;
	unsigned char tmp_status ;
	
HANDLE_AND_REMOVE_THE_CLASS_HEADER:
	ret = icusb_smartcard_rm_rx_header(from_usb_buff , usb_port_read_length, &tmp_length, &apdu_buff, &tmp_chain_parameter, &tmp_status) ;
	if (ret == ICUSB_UICC_GENERAL_ERR){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_smartcard_rm_rx_header return NULL\n");
		return ICUSB_ERROR_PACKET ;
	}	
	*apdu_length = tmp_length;
	*p_apdu_buff = apdu_buff ;
	*chain_parameter = tmp_chain_parameter ;
	*icc_status = tmp_status ;
	
	if (ret == ICUSB_UICC_DATA_RESP){
		return ICUSB_RX_DATA_PACKET ;
	}else if(ret == ICUSB_UICC_CTRL_RESP){
		return ICUSB_RX_CTRL_PACKET ;
	}else{
		return ICUSB_ERROR_PACKET ;
	}
	
}	

// handle the class specific action
// p_to_usb_buff is the buff with [Class Header][APDU] 
// return the proccess result of packet category
int icusb_tx_class_handle(int message_type, char *apdu_buff, int apdu_length, int *to_usb_length , char **p_to_usb_buff, int chain_parameter)
{
	char *to_usb_buff = NULL;
	int tmp_length = 0;

FILL_THE_CLASS_HEADER:
	to_usb_buff = icusb_smartcard_add_tx_header(message_type, apdu_buff, ICUSB_SMARTCARD_CLASS_HEADER_ROOM, apdu_length, &tmp_length, chain_parameter);
	if (to_usb_buff == NULL){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_smartcard_add_tx_header return NULL\n");
		return ICUSB_ERROR_PACKET ;
	}
	// to_usb_buff : [CCCI_CLASS_HEADER_ROOM] [APDU] 
	// to_usb_length : CCCI_CLASS_HEADER_ROOM + APDU Length
	*to_usb_length = tmp_length ;
	*p_to_usb_buff= to_usb_buff ;
	
	return ICUSB_TX_GOOD_PACKET;		

}

int icusb_rx_ctrl_ccci_ack(char *rx_buff, unsigned int rx_buff_length, unsigned char value)
{
	s_ccci_mtk_header rx_mtk_header ; 
	s_icusb_ctrl_ack_field *icusb_ctrl_ack_field ; 

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_rx_ctrl_ccci_ack \n");

	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_rx_ctrl_ccci_ack the value is 0x%x\n", value);
	
	if (rx_buff_length < sizeof(s_ccci_mtk_header) + sizeof(s_icusb_ctrl_ack_field)){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_rx_ctrl_ccci_ack, rx_buff_length is too small\n");
		return ICUSB_ERROR_PACKET ;
	}
	
	rx_mtk_header.sif = CCCI_TTY_SIM_ID_1 ;
	rx_mtk_header.pcb = CCCI_MTK_H_PCB_MODE_CTRL ; 
	rx_mtk_header.cp = 0x0 ; 
	rx_mtk_header.len = sizeof(s_icusb_ctrl_ack_field) ;
	
	icusb_ctrl_ack_field = (s_icusb_ctrl_ack_field *)(rx_buff + sizeof(s_ccci_mtk_header)) ;
	icusb_ctrl_ack_field->type = ICUSB_ACK_STATUS ;
	icusb_ctrl_ack_field->length = 0x1 ;
	icusb_ctrl_ack_field->value = value ;

	icusb_rx_ccci_write((char *)icusb_ctrl_ack_field, sizeof(s_icusb_ctrl_ack_field), &rx_mtk_header) ; 

	icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_rx_ctrl_ccci_ack\n");
	
	return ICUSB_RX_GOOD_PACKET;		

}


int icusb_tx_ctrl_process(char *info_buff, int info_length, unsigned char *ack_message)
{
	s_icusb_ctrl_data_field *tx_ctrl_data_field ;
	unsigned char set_value ;	
	char *to_usb_buff = NULL;
	int to_usb_length= 0 ;
	int ret = ICUSB_CTRL_COMPLETE, tx_usb_ret;
	unsigned int g_ic_usb_status; 
	
	tx_ctrl_data_field = (s_icusb_ctrl_data_field *)info_buff ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_tx_ctrl_process the type is %d\n", tx_ctrl_data_field->type);

	if (tx_ctrl_data_field->type == USB11_START_SESSION){

		set_value = tx_ctrl_data_field->data[0] ;

		if (set_value == ICUSB_SESSION_START){		
			// 4-1. do proc write to YC to enable USB
#if ICUSB_UT_NO_USB				
			ret = ICUSB_OK ;
#else			
			ret = icusb_util_write_proc(ICUSB_USB_CTRL_PROC_FILE, (char *)tx_ctrl_data_field, 3) ;						
#endif
			if (ICUSB_OK != ret){
				icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc fail\n");
				*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
				ret = ICUSB_CHANNEL_NOT_READY ; 
		
			}else{
				unsigned int polling_cnt = 500 ;
				while (polling_cnt > 0) {
					polling_cnt -- ;
					usleep(ICUSB_USB_DISCONNECT_POLL_INTERVAL_US); // delay 100 ms for USB reset
					ret = icusb_util_get_proc(ICUSB_USB_CTRL_PROC_FILE, &g_ic_usb_status, sizeof(g_ic_usb_status)) ;
					if (ICUSB_OK != ret){
						icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_get_proc fail\n");
						*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
						ret = ICUSB_CHANNEL_NOT_READY ; 
						break ;
					}
 					
					icusb_print(PRINT_INFO, "[ICUSB][INFO] ic_usb_status : 0x%x\n", g_ic_usb_status);

					if((g_ic_usb_status & (PREFER_VOL_STS_MSK << PREFER_VOL_STS_SHIFT)) != PREFER_VOL_NOT_INITED){
						break ;
					}											
				}

				if (polling_cnt == 0 || ICUSB_OK != ret){
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] polling_cnt to 0, but we don't get the ready g_ic_usb_status, the g_ic_usb_status is 0x%x\n", g_ic_usb_status);
					*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
					ret = ICUSB_CHANNEL_NOT_READY ; 
				}else{								
					// 4-2 YC set session -> 1 , enum and power negotiation and check prefer voltage
					if (g_ic_usb_status == PREFER_VOL_PWR_NEG_FAIL) {
						icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->start session:power negotiation fail! call to end sesstion \n");
						*ack_message = ICUSB_ACK_STATUS_CMD_EN_SESSION_ERROR;
						ret = ICUSB_CHANNEL_NOT_READY ;						
					}else if (((g_ic_usb_status>>PREFER_VOL_CLASS_SHIFT)&IC_USB_PREFER_CLASSB_ENABLE_BIT) && (current_icusb_voltage != ICUSB_VOL_30)){
					//if (((g_ic_usb_status>>PREFER_VOL_CLASS_SHIFT)&IC_USB_PREFER_CLASSB_ENABLE_BIT)){
						icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->start session->change voltage(current %d to prefer %d) \n", current_icusb_voltage, g_ic_usb_status);
						//icusb_rx_ctrl_ccci_ack(rx_buff, rx_buff_length, ICUSB_ACK_STATUS_PREFER_3V) ;
						*ack_message = ICUSB_ACK_STATUS_PREFER_3V ;
						ret = ICUSB_CTRL_CHANGE_VOLTAGE ;
					}else{										
						// 4-3. polling the USB enable state and prefer voltage 	
						usleep(ICUSB_USB_ENUM_SETUP_TIME_US); // delay 1 s for USB enumeration

						icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->start session \n");			
						icusb_open_dev(&icusb_device) ;
						if (ICUSB_OK == ccid_init(&icusb_device) && ICUSB_OK == icusb_netlink_open()){
							*ack_message = ICUSB_ACK_STATUS_OK ;
							ret = ICUSB_CTRL_COMPLETE ; 				
						}else{
							*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
							ret = ICUSB_CHANNEL_NOT_READY;	
						}		

		
						// 4-4. Ack to MD
						//icusb_rx_ctrl_ccci_ack(rx_buff, rx_buff_length, ICUSB_ACK_STATUS_OK) ;				
					}	
				}
			}
		}else{
#if ICUSB_UT_NO_USB				
			ret = ICUSB_OK ;
#else
			// 4-1. do proc write to YC to reset USB 
			icusb_storage_do_unmomunt() ;
			
			ret = icusb_util_write_proc(ICUSB_USB_CTRL_PROC_FILE, (char *)tx_ctrl_data_field, 3) ;			
			//ret = ICUSB_OK ;
#endif
			if (ICUSB_OK != ret){
				icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc fail\n");
				*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
				ret = ICUSB_CHANNEL_NOT_READY ; 
		
			}else{				
							
				icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->end session ............ \n");						

#if 0
				icusb_netlink_close() ;
				ccid_deinit() ;	
				icusb_close_dev(&icusb_device) ;				
#endif				

				//usleep(ICUSB_USB_DISCONNECT_TIME_US); // delay 1 s for USB enumeration

				icusb_print(PRINT_INFO, "[ICUSB][INFO] send end session response!!!\n");
				
				*ack_message = ICUSB_ACK_STATUS_OK ;
				//icusb_rx_ctrl_ccci_ack(rx_buff, rx_buff_length, ICUSB_ACK_STATUS_OK) ;
				ret = ICUSB_CTRL_STOP_SESSION ;			
			}

		}
	}else if (tx_ctrl_data_field->type == USB11_WAIT_DISCONNECT_DONE){		
#if ICUSB_UT_NO_USB				
		ret = ICUSB_OK ;
#else
		// 2-1. do proc write to YC to wait disconnect done
		usleep(ICUSB_USB_DISCONNECT_POLL_INTERVAL_US); // delay 100 ms for USB reset
		
		ret = icusb_util_write_proc(ICUSB_USB_CTRL_PROC_FILE, (char *)tx_ctrl_data_field, 3) ;

#endif
		if (ICUSB_OK != ret){
			icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc fail\n");
			*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
			ret = ICUSB_CHANNEL_NOT_READY ; 
		}else{			
			icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->wait USB disconnect done!!! \n");

#if 1
			icusb_netlink_close() ;
			ccid_deinit() ;	
			icusb_close_dev(&icusb_device) ;				
#endif	

			// 2-3. ack to MD (set voltage ACK) 			
			*ack_message = ICUSB_ACK_STATUS_OK ;
			//icusb_rx_ctrl_ccci_ack(rx_buff, rx_buff_length , ICUSB_ACK_STATUS_OK) ;	
			ret = ICUSB_CTRL_COMPLETE ;
		}					

	}else if (tx_ctrl_data_field->type == USB11_INIT_PHY_BY_VOLTAGE){		
#if ICUSB_UT_NO_USB				
		ret = ICUSB_OK ;
#else
		// 2-1. do proc write to YC to set voltage 
		ret = icusb_util_write_proc(ICUSB_USB_CTRL_PROC_FILE, (char *)tx_ctrl_data_field, 3) ;

#endif
		if (ICUSB_OK != ret){
			icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc fail\n");
			*ack_message = ICUSB_ACK_STATUS_NO_CARD ;
			ret = ICUSB_CHANNEL_NOT_READY ; 
		}else{
			current_icusb_voltage = tx_ctrl_data_field->data[0] ;
			icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->set voltage to %d \n", current_icusb_voltage);
			// 2-3. ack to MD (set voltage ACK) 			
			*ack_message = ICUSB_ACK_STATUS_OK ;
			//icusb_rx_ctrl_ccci_ack(rx_buff, rx_buff_length , ICUSB_ACK_STATUS_OK) ;	
			ret = ICUSB_CTRL_COMPLETE ;
		}					

	}else if (tx_ctrl_data_field->type == ICUSB_ICC_POWER_ON){
		icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->ICUSB_ICC_POWER_ON\n");
		
		to_usb_buff = icusb_smartcard_add_tx_header(CCID_BULK_OUT_PC_TO_RDR_ICCPOWERON, info_buff, ICUSB_SMARTCARD_CLASS_HEADER_ROOM, info_length, &to_usb_length, 0);

		tx_usb_ret = icusb_tx_usb_write(to_usb_buff , to_usb_length) ;

		*ack_message = ICUSB_ACK_STATUS_NEED_RX_TO_ACK ;
		
		if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
			ret =  ICUSB_TX_TRANSFER_ERROR ;
		}else{
			ret = ICUSB_CTRL_COMPLETE ;
		}			
	}else if (tx_ctrl_data_field->type == ICUSB_ICC_POWER_OFF){
		icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->ICUSB_ICC_POWER_OFF\n");
		
		to_usb_buff = icusb_smartcard_add_tx_header(CCID_BULK_OUT_PC_TO_RDR_ICCPOWEROFF, info_buff, ICUSB_SMARTCARD_CLASS_HEADER_ROOM, info_length, &to_usb_length, 0);

		tx_usb_ret = icusb_tx_usb_write(to_usb_buff , to_usb_length) ;

		*ack_message = ICUSB_ACK_STATUS_NEED_RX_TO_ACK ;
		
		if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
			ret = ICUSB_TX_TRANSFER_ERROR ;		
		}else{
			ret = ICUSB_CTRL_COMPLETE ;
		}		
	}else if (tx_ctrl_data_field->type == ICUSB_GET_SLOT_STATUS){
		icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ctrl_process->ICUSB_GET_SLOT_STATUS\n");
		
		to_usb_buff = icusb_smartcard_add_tx_header(CCID_BULK_OUT_PC_TO_RDR_GETSLOTSTATUS, info_buff, ICUSB_SMARTCARD_CLASS_HEADER_ROOM, info_length, &to_usb_length, 0);

		tx_usb_ret = icusb_tx_usb_write(to_usb_buff , to_usb_length) ;

		*ack_message = ICUSB_ACK_STATUS_NEED_RX_TO_ACK ;
		
		if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
			ret = ICUSB_TX_TRANSFER_ERROR ;
		}else{
			ret = ICUSB_CTRL_COMPLETE ;
		}		
	}else{
		*ack_message = ICUSB_ACK_STATUS_CMD_TYPE_ERROR ;
		ret = ICUSB_ACK_STATUS_CMD_TYPE_ERROR ;	
	}

	//usleep(ICUSB_CONTROL_MSG_DELAY_US); // Delay for special card

	icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_tx_ctrl_process\n") ;
	
	return ret;		

}

#if ICUSB_UT_NO_CCCI

int icusb_ut_tx_packet_gen(int test_item , int argc_1 , int argc_2)
{
	char ut_buff[ICUSB_TRANSFER_TX_BUFF_SIZE] ;
	s_ccci_mtk_header *tx_mtk_header ; 
	s_icusb_ctrl_data_field *tx_ctrl_data_field ;
	char* tx_data_data_field ;
	unsigned pkt_len = 0;

	tx_mtk_header = (s_ccci_mtk_header *)ut_buff ;	
	tx_ctrl_data_field = (s_icusb_ctrl_data_field *)(ut_buff+sizeof(s_ccci_mtk_header)) ;
	tx_data_data_field = (char *)(ut_buff+sizeof(s_ccci_mtk_header)) ;
	
	if (test_item == USB11_START_SESSION){
		
		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
	
		tx_ctrl_data_field->type = USB11_START_SESSION ;
		tx_ctrl_data_field->length= 0x1 ;
		if (argc_1 > 0)
			tx_ctrl_data_field->data[0] = ICUSB_SESSION_START ; // enable sesstion 
		else
			tx_ctrl_data_field->data[0] = ICUSB_SESSION_END ; // disable sesstion 
	}else if(test_item == USB11_WAIT_DISCONNECT_DONE){

		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
	
		tx_ctrl_data_field->type = USB11_WAIT_DISCONNECT_DONE ;
		tx_ctrl_data_field->length= 0x1 ;
		tx_ctrl_data_field->data[0] = 0 ; // RFU
		
	}else if(test_item == USB11_INIT_PHY_BY_VOLTAGE){

		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
	
		tx_ctrl_data_field->type = USB11_INIT_PHY_BY_VOLTAGE ;
		tx_ctrl_data_field->length= 0x1 ;
		tx_ctrl_data_field->data[0] = ICUSB_VOL_30 ; // Class B sesstion 
		
	}else if(test_item == ICUSB_ICC_POWER_ON){

		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
		
		tx_ctrl_data_field->type = ICUSB_ICC_POWER_ON ;
		tx_ctrl_data_field->length= 0x1 ;
		tx_ctrl_data_field->data[0] = 0x0 ; // RFU

	}else if(test_item == ICUSB_ICC_POWER_OFF){

		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
		
		tx_ctrl_data_field->type = ICUSB_ICC_POWER_OFF ;
		tx_ctrl_data_field->length= 0x1 ;
		tx_ctrl_data_field->data[0] = 0x0 ; // RFU

	}else if(test_item == ICUSB_GET_SLOT_STATUS){

		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_CTRL ;
		tx_mtk_header->cp = 0x0 ;
		tx_mtk_header->len = 3 ;		
		
		tx_ctrl_data_field->type = ICUSB_GET_SLOT_STATUS ;
		tx_ctrl_data_field->length= 0x1 ;
		tx_ctrl_data_field->data[0] = 0x0 ; // RFU

	}else if(test_item == 0xFF){// data 
/*	
		APDU for SIM Card :
		Select MF: 		A0 A4 00 00 02 3F 00
		Select DFGSM: 	A0 A4 00 00 02 7F 20
		Select EFIMSI: 	A0 A4 00 00 02 6F 07
		Read EFIMSI: 	A0 B0 00 00 09
		
		APDU for USIM Card :
		Select MF: 		00 A4 00 00 02 3F 00
		Select DFGSM:	00 A4 00 00 02 7F 20
		Select EFIMSI: 	00 A4 00 00 02 6F 07
		Read EFIMSI: 	00 B0 00 00 09
*/		
	
		tx_mtk_header->sif = 0x0 ;
		tx_mtk_header->pcb = CCCI_MTK_H_PCB_MODE_DATA  ;  
		tx_mtk_header->cp = (unsigned short)argc_2 ;
		
		if (argc_1 == 0x0){ // Select MF
#if USIM_APDU		
			tx_data_data_field[0] = 0x00 ; 
#else
			tx_data_data_field[0] = 0xA0 ; 
#endif
			tx_data_data_field[1] = 0xA4 ; 
			tx_data_data_field[2] = 0x00 ; 
			tx_data_data_field[3] = 0x00 ; 
			tx_data_data_field[4] = 0x02 ; 
			tx_data_data_field[5] = 0x3F ; 
			tx_data_data_field[6] = 0x00 ; 

			tx_mtk_header->len = 7 ;

		}else if(argc_1 == 0x1){ // Select DFGSM
#if USIM_APDU		
			tx_data_data_field[0] = 0x00 ; 
#else
			tx_data_data_field[0] = 0xA0 ; 
#endif
			tx_data_data_field[1] = 0xA4 ; 
			tx_data_data_field[2] = 0x00 ; 
			tx_data_data_field[3] = 0x00 ; 
			tx_data_data_field[4] = 0x02 ; 
			tx_data_data_field[5] = 0x7F ; 
			tx_data_data_field[6] = 0x20 ; 

			tx_mtk_header->len = 7 ;		
		}else if(argc_1 == 0x2){ // Select EFIMSI
#if USIM_APDU		
			tx_data_data_field[0] = 0x00 ; 
#else
			tx_data_data_field[0] = 0xA0 ; 
#endif
			tx_data_data_field[1] = 0xA4 ; 
			tx_data_data_field[2] = 0x00 ; 
			tx_data_data_field[3] = 0x00 ; 
			tx_data_data_field[4] = 0x02 ; 
			tx_data_data_field[5] = 0x6F ; 
			tx_data_data_field[6] = 0x07 ; 

			tx_mtk_header->len = 7 ;		
		}else if(argc_1 == 0x3){ // Read EFIMSI
#if USIM_APDU		
			tx_data_data_field[0] = 0x00 ; 
#else
			tx_data_data_field[0] = 0xA0 ; 
#endif
			tx_data_data_field[1] = 0xB0 ; 
			tx_data_data_field[2] = 0x00 ; 
			tx_data_data_field[3] = 0x00 ; 
			tx_data_data_field[4] = 0x09 ; 

			tx_mtk_header->len = 5 ;
		}else{
			icusb_assert() ;  
		}					

	}

	pkt_len= tx_mtk_header->len +sizeof(s_ccci_mtk_header) ; 
	
	icusb_fill_ut_tx_buff(ut_buff, pkt_len) ;
	
	return 0 ;
}

#endif

static void *icusb_recovery_thread(void *priv)
{
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ====> icusb_recovery_thread\n");
	while(!daemon_exit){
		usleep(ICUSB_THREAD_LOOP_INTERVAL_US);		
		if (icusb_device.is_ready){
			if (ICUSB_OK == icusb_netlink_recv_msg()){
				icusb_ccci_notify_md_recover() ; 
			}
		}else{
			usleep(ICUSB_NODEV_DELAY_US);
		}
	}

	icusb_print(PRINT_INFO, "[ICUSB][INFO] <==== icusb_recovery_thread\n");
	
	return 0;
}

static void *icusb_transfer_thread(void *priv)
{
	char tx_buff[ICUSB_TRANSFER_TX_BUFF_SIZE] ;
	char rx_buff[ICUSB_TRANSFER_RX_BUFF_SIZE] ;

	char *tx_apdu_buff ;
	char *tx_to_usb_buff;
	int tx_apdu_length , tx_to_usb_length;
	int tx_ccci_ret , tx_pass_ret, tx_usb_ret;
	unsigned short tx_chain_parameter = 0 ;
	s_icusb_ctrl_data_field *tx_ctrl_data_field ;
	int tx_wait_rx = 0 ;

	char *rx_apdu_buff ;
	char *rx_from_usb_buff;
	int rx_apdu_length , rx_from_usb_length;
	int rx_usb_ret , rx_pass_ret;
	unsigned short rx_chain_parameter = 0 ;

	unsigned char icc_status ;
	unsigned char ack_message ;
	int fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] ====> icusb_transfer_thread\n");

#if ICUSB_UT_NO_CCCI
	int steps = 0 ;
	int rx_done = 1 ;
	int rx_no_resp_cnt = 0 ; 
#endif
		
	while(!daemon_exit) {
#if ICUSB_UT_NO_CCCI
#include <stdio.h>
		if (steps == 0){
			icusb_ut_tx_packet_gen(USB11_START_SESSION , 0,0) ;				
		}else if (steps == 1){
			icusb_ut_tx_packet_gen(USB11_WAIT_DISCONNECT_DONE, 0,0) ;
		}else if (steps == 2){
			icusb_ut_tx_packet_gen(USB11_INIT_PHY_BY_VOLTAGE , 0,0) ;
		}else if (steps == 3){
			icusb_ut_tx_packet_gen(USB11_START_SESSION , 1,0) ;				
		}else if (steps == 4){
			usleep(2000000);
			printf("wait for key .........................................\n") ;
			//getchar();		
			icusb_ut_tx_packet_gen(ICUSB_ICC_POWER_ON , 0,0) ;					
		}else if (steps == 5){
			icusb_ut_tx_packet_gen(ICUSB_GET_SLOT_STATUS , 0,0) ;		
		}else if (steps == 6){
			icusb_ut_tx_packet_gen(0xFF , 0x00 , 0x00) ;
		}else if (steps == 7){
			icusb_ut_tx_packet_gen(0xFF , 0x01 , 0x00) ;
		}else if (steps == 8){
			icusb_ut_tx_packet_gen(0xFF , 0x02 , 0x00) ;
		}else if (steps == 9){
			icusb_ut_tx_packet_gen(0xFF , 0x03 , 0x00) ;
		}else if (steps > 20){
			usleep(50000);
			if (rx_done == 1){
				icusb_ut_tx_packet_gen(0xFF , 0x03 , 0x00) ;
				rx_done = 0 ;
				rx_no_resp_cnt = 0 ;
			}else{
				usleep(100000);
				rx_no_resp_cnt ++ ;				
			}

			if (rx_no_resp_cnt >200){
				printf("Rx no response for %d times, wait for key .........................................\n", rx_no_resp_cnt) ;
				getchar();		
				rx_no_resp_cnt = 0;
				icusb_ut_tx_packet_gen(0xFF , 0x03 , 0x00) ;
			}
		}
		
		if (fsm_current_state!= ICUSB_FSM_TRANSFER_RX_PROCESS){
			steps ++ ;
		}
#endif

		usleep(ICUSB_THREAD_LOOP_INTERVAL_US);
		
		switch (fsm_current_state)
		{
			case ICUSB_FSM_RESET :
			{
				fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;
				tx_wait_rx = 0 ;
				break ;
			}			
			case ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB :
			{
				tx_ccci_ret = icusb_tx_ccci_read(tx_buff , ICUSB_TRANSFER_TX_BUFF_SIZE, ICUSB_SMARTCARD_CLASS_HEADER_ROOM , &tx_apdu_length, &tx_apdu_buff, NULL) ;
				
				if (tx_ccci_ret == CCCI_TX_NO_DATA){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;
					break ;
				}
				
				if (tx_ccci_ret != CCCI_TX_CTRL_PACKET){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] get data packet in handshake - power on USB state\n");
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;
					break ;
				}			

				tx_ctrl_data_field = (s_icusb_ctrl_data_field *)tx_apdu_buff ;
				if (tx_ctrl_data_field->type != USB11_START_SESSION || tx_ctrl_data_field->data[0] != ICUSB_SESSION_END){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] start session error in handshake state, cmd type is %d, cmd data is %d\n", tx_ctrl_data_field->type, tx_ctrl_data_field->data[0]);
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE , ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;
					break ;
				}

				tx_usb_ret = icusb_tx_ctrl_process(tx_apdu_buff, tx_apdu_length, &ack_message) ;
								
				 if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_TX_TRANSFER_ERROR in ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB.. \n");
					
#if !ICUSB_UT_NO_USB					
				}else if (tx_usb_ret == ICUSB_CHANNEL_NOT_READY){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_CHANNEL_NOT_READY in ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB.. \n");
#endif
				}else if (tx_usb_ret == ICUSB_CTRL_STOP_SESSION){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;
					icusb_print(PRINT_ERROR, "[ICUSB][INFO] Move to ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE state n");
					
				}else{
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] Assert !! \n");
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;		
					icusb_assert() ; 
				}		

				if (ack_message != ICUSB_ACK_STATUS_NEED_RX_TO_ACK){
					if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
						icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
					} // Not OK case let MD to timeout and recover the card 
				}else{
					icusb_assert() ; 
				}
							
				break ;
			}	
			case ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE :
			{
				tx_ccci_ret = icusb_tx_ccci_read(tx_buff , ICUSB_TRANSFER_TX_BUFF_SIZE, ICUSB_SMARTCARD_CLASS_HEADER_ROOM , &tx_apdu_length, &tx_apdu_buff, NULL) ;
				
				if (tx_ccci_ret == CCCI_TX_NO_DATA){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;
					break ;
				}
				
				if (tx_ccci_ret != CCCI_TX_CTRL_PACKET){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] get data packet in handshake - power on USB state\n");
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;
					break ;
				}			

#if 0
				tx_ctrl_data_field = (s_icusb_ctrl_data_field *)tx_apdu_buff ;
				if (tx_ctrl_data_field->type != USB11_WAIT_DISCONNECT_DONE){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] start session error in handshake state, cmd type is %d, cmd data is %d\n", tx_ctrl_data_field->type, tx_ctrl_data_field->data[0]);
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE , ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;
					break ;
				}
#endif				

				tx_usb_ret = icusb_tx_ctrl_process(tx_apdu_buff, tx_apdu_length, &ack_message) ;

				if (tx_usb_ret == ICUSB_CTRL_COMPLETE){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;
					
				}else if (tx_usb_ret == ICUSB_CTRL_STOP_SESSION){
					fsm_current_state = ICUSB_FSM_RESET;
					icusb_print(PRINT_ERROR, "[ICUSB][INFO] get ICUSB_CTRL_STOP_SESSION, Move to ICUSB_FSM_RESET state n");
					
				}else if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_TX_TRANSFER_ERROR in ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE.. \n");
					
#if !ICUSB_UT_NO_USB					
				}else if (tx_usb_ret == ICUSB_CHANNEL_NOT_READY){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_CHANNEL_NOT_READY in ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE.. \n");
#endif
				}else{
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] Assert !! \n");
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE; 	
					icusb_assert() ; 
				}		

				if (ack_message != ICUSB_ACK_STATUS_NEED_RX_TO_ACK){
					if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
						icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
					} // Not OK case let MD to timeout and recover the card 	
				}else{
					icusb_assert() ; 
				}
							
				break ;
			}			
			case ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE :
			{				
				// 1. Argus Power off SIM_VCC
				
				// 2-0.  get the ccci control packet
				tx_ccci_ret = icusb_tx_ccci_read(tx_buff , ICUSB_TRANSFER_TX_BUFF_SIZE, ICUSB_SMARTCARD_CLASS_HEADER_ROOM , &tx_apdu_length, &tx_apdu_buff, NULL) ;
				if (tx_ccci_ret == CCCI_TX_NO_DATA){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;
					break ;
				}
				
				if (tx_ccci_ret != CCCI_TX_CTRL_PACKET){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] get data packet in handshake - set voltage state\n");
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE , ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;
					break ;
				}

				icusb_print(PRINT_INFO, "[ICUSB][INFO] MD-AP-USB Inital Handshake Start ..... \n");
				
				tx_usb_ret= icusb_tx_ctrl_process(tx_apdu_buff, tx_apdu_length, &ack_message) ;	
				
				// 3. Argus Power on SIM_VCC	
				if (tx_usb_ret == ICUSB_CTRL_COMPLETE){
					if (tx_ctrl_data_field->type == USB11_WAIT_DISCONNECT_DONE){
						fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;
					}else{
						fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;
					}					
				}else if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_TX_TRANSFER_ERROR in ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE\n");
					
				}else if (tx_usb_ret == ICUSB_CTRL_STOP_SESSION){					
					fsm_current_state = ICUSB_FSM_RESET;
					icusb_print(PRINT_ERROR, "[ICUSB][INFO] get ICUSB_CTRL_STOP_SESSION, Move to ICUSB_FSM_RESET state n");
					
				}else{
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE;
				}

				if (ack_message != ICUSB_ACK_STATUS_NEED_RX_TO_ACK){
					if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
						icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
					} // Not OK case let MD to timeout and recover the card 	

				}else{
					icusb_assert() ; 
				}
				
				break ;
			}
			case ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB :
			{
				// 4-0.  get the ccci control packet
				tx_ccci_ret = icusb_tx_ccci_read(tx_buff , ICUSB_TRANSFER_TX_BUFF_SIZE, ICUSB_SMARTCARD_CLASS_HEADER_ROOM , &tx_apdu_length, &tx_apdu_buff, NULL) ;
				
				if (tx_ccci_ret == CCCI_TX_NO_DATA){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;
					break ;
				}
				
				if (tx_ccci_ret != CCCI_TX_CTRL_PACKET){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] get data packet in handshake - enable USB state\n");
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;
					break ;
				}			

				tx_ctrl_data_field = (s_icusb_ctrl_data_field *)tx_apdu_buff ;
				if (tx_ctrl_data_field->type != USB11_START_SESSION){
					icusb_print(PRINT_WARN, "[ICUSB][WARN] start session error in handshake state, cmd type is %d, cmd data is %d\n", tx_ctrl_data_field->type, tx_ctrl_data_field->data[0]);
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE , ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 
					usleep(ICUSB_TX_ERR_DELAY_US);

					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;
					break ;
				}

				tx_usb_ret = icusb_tx_ctrl_process(tx_apdu_buff, tx_apdu_length, &ack_message) ;
								
				if (tx_usb_ret == ICUSB_CTRL_CHANGE_VOLTAGE){	
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB;			
#if !ICUSB_UT_NO_USB					
				}else if (tx_usb_ret == ICUSB_CHANNEL_NOT_READY){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_CHANNEL_NOT_READY in ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB.. \n");
#endif
				}else if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
					fsm_current_state = ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB;	
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] ICUSB_TX_TRANSFER_ERROR in ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB.. \n");
					
				}else if (tx_usb_ret == ICUSB_CTRL_STOP_SESSION){
					fsm_current_state = ICUSB_FSM_RESET;
					icusb_print(PRINT_ERROR, "[ICUSB][INFO] Back to Reset state...\n");
					
				}else{
					icusb_print(PRINT_INFO, "[ICUSB][INFO] MD-AP-USB Inital Handshake Done !! \n");
					fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;								
				}		

				if (ack_message != ICUSB_ACK_STATUS_NEED_RX_TO_ACK){
					if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
						icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
					} // Not OK case let MD to timeout and recover the card 	
				}else{
					icusb_assert() ; 
				}
				
				break ;	
			}				
			case ICUSB_FSM_TRANSFER_TX_PROCESS :
			{
				tx_ccci_ret = icusb_tx_ccci_read(tx_buff , ICUSB_TRANSFER_TX_BUFF_SIZE, ICUSB_SMARTCARD_CLASS_HEADER_ROOM , &tx_apdu_length, &tx_apdu_buff, &tx_chain_parameter) ;

				if (tx_ccci_ret == CCCI_TX_NO_DATA){	
					if (tx_wait_rx > 0){
						fsm_current_state = ICUSB_FSM_TRANSFER_RX_PROCESS;		
					}else{
						fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;		
					}				
				}else if (tx_ccci_ret == CCCI_TX_CTRL_PACKET){					
					tx_usb_ret = icusb_tx_ctrl_process(tx_apdu_buff, tx_apdu_length, &ack_message ) ;
				
					if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR || tx_usb_ret == ICUSB_CHANNEL_NOT_READY){
						//icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_CCID_STATUS_TIMEOUT) ;
						// Not OK case let MD to timeout and recover the card 
						fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;
						icusb_print(PRINT_ERROR, "[ICUSB][ERROR] error on sending TX packet... \n");
						
					}else if (tx_usb_ret == ICUSB_CTRL_STOP_SESSION){
					
						fsm_current_state = ICUSB_FSM_RESET;

						if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
							icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
						} // Not OK case let MD to timeout and recover the card 						
						//icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
						icusb_print(PRINT_ERROR, "[ICUSB][INFO] Back to Reset state...\n");
						
					}else{
						if (ack_message != ICUSB_ACK_STATUS_NEED_RX_TO_ACK){
							if (ack_message == ICUSB_ACK_STATUS_OK || ack_message == ICUSB_ACK_STATUS_PREFER_3V){
								icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
							} // Not OK case let MD to timeout and recover the card 						
							//icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ack_message) ;
							usleep(ICUSB_THREAD_TRANSFER_STATE_CHANGE_INTERVAL_US);
							fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;
						}else{
							usleep(ICUSB_THREAD_TRANSFER_STATE_CHANGE_INTERVAL_US);
							tx_wait_rx ++ ;
							fsm_current_state = ICUSB_FSM_TRANSFER_RX_PROCESS;
#if ICUSB_UT_NO_USB
							{
								char tmp_buff[20] = "XXXXXXXXXXCONTROLACK" ;
								struct ccid_rdr2pc_slot_status_msg *rdr2pc_slot_status_msg ;

								rdr2pc_slot_status_msg = (struct ccid_rdr2pc_slot_status_msg *)tmp_buff ;
								rdr2pc_slot_status_msg->bMessageType = CCID_BULK_IN_RDR_TO_PC_SLOTSTATUS;
								rdr2pc_slot_status_msg->dwLength = 10 ;
								rdr2pc_slot_status_msg->bSlot = CCID_SIM_SLOT_1 ;
								rdr2pc_slot_status_msg->bSeq = 0 ;
								rdr2pc_slot_status_msg->bStatus = 0 ;
								rdr2pc_slot_status_msg->bError = 0x0 ;
								rdr2pc_slot_status_msg->bClockStatus= 0 ;
							
								icusb_fill_ut_rx_buff(tmp_buff , 20) ; 
							}
#endif							
						}				
					}
				}else if (tx_ccci_ret == CCCI_TX_DATA_PACKET){
					tx_pass_ret = icusb_tx_class_handle(CCID_BULK_OUT_PC_TO_RDR_XFRBLOCK, tx_apdu_buff,tx_apdu_length, &tx_to_usb_length , &tx_to_usb_buff, tx_chain_parameter) ;
					tx_usb_ret = icusb_tx_usb_write(tx_to_usb_buff , tx_to_usb_length) ;
					if (tx_usb_ret == ICUSB_TX_TRANSFER_ERROR){
						//icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_CCID_STATUS_TIMEOUT) ;	
						// Not OK case let MD to timeout and recover the card 
						usleep(ICUSB_RESET_DELAY_US);
						fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;
						icusb_print(PRINT_ERROR, "[ICUSB][ERROR] error on sending TX packet... \n");
						
					}else{
#if ICUSB_UT_NO_USB
						{
							char tmp_buff[17] = "XXXXXXXXXXDATAACK" ;
							struct ccid_rdr2pc_data_block_msg *rdr2pc_data_block_msg ;

							rdr2pc_data_block_msg = (struct ccid_rdr2pc_data_block_msg *)tmp_buff ;
							rdr2pc_data_block_msg->bMessageType = CCID_BULK_IN_RDR_TO_PC_DATABLOCK ;
							rdr2pc_data_block_msg->dwLength = 7 ;
							rdr2pc_data_block_msg->bSlot = CCID_SIM_SLOT_1 ;
							rdr2pc_data_block_msg->bSeq = 1 ;
							rdr2pc_data_block_msg->bStatus = 0 ;
							rdr2pc_data_block_msg->bError = 0xFF ;
							rdr2pc_data_block_msg->bChainParameter = CCID_CHAIN_C ;
							
							icusb_fill_ut_rx_buff(tmp_buff , 17) ; 						
						}
#endif
						usleep(ICUSB_THREAD_TRANSFER_STATE_CHANGE_INTERVAL_US);
						tx_wait_rx ++ ;
						fsm_current_state = ICUSB_FSM_TRANSFER_RX_PROCESS;
					}		
				}else{
					// round robin to RX process
					icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE , ICUSB_ACK_STATUS_CMD_TYPE_ERROR) ; // ACK ERROR to MD 					
					fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;					
				}	
				
				break ;
			}
			case ICUSB_FSM_TRANSFER_RX_PROCESS :
			{
				s_ccci_mtk_header rx_mtk_header ; 
				rx_chain_parameter = 0 ;				
				rx_usb_ret = icusb_rx_usb_read(rx_buff ,ICUSB_TRANSFER_RX_BUFF_SIZE, sizeof(s_ccci_mtk_header) , &rx_from_usb_length , &rx_from_usb_buff) ;

				if (rx_usb_ret == ICUSB_RX_GOOD_PACKET){										
					tx_wait_rx -- ;
#if ICUSB_UT_NO_CCCI
					rx_done = 1 ;
#endif
					rx_pass_ret = icusb_rx_class_handle(rx_from_usb_buff, rx_from_usb_length, &rx_apdu_length, &rx_apdu_buff, &rx_chain_parameter, &icc_status) ;
					if (rx_pass_ret == ICUSB_RX_CTRL_PACKET){											
						icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, icc_status) ;
					}else if (rx_pass_ret == ICUSB_RX_DATA_PACKET){
						rx_mtk_header.sif = CCCI_TTY_SIM_ID_1 ;
						rx_mtk_header.pcb = CCCI_MTK_H_PCB_MODE_DATA ; //| (rx_chain_parameter? CCCI_MTK_H_PCB_M_BIT:0); 
						rx_mtk_header.cp = rx_chain_parameter ;
						rx_mtk_header.len = rx_apdu_length ;
						icusb_rx_ccci_write(rx_apdu_buff, rx_apdu_length, &rx_mtk_header) ;					
					}
					
					fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;		
					
				}else if (rx_usb_ret == ICUSB_ERROR_PACKET){				
					tx_wait_rx -- ;
					fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS ;
					//icusb_rx_ctrl_ccci_ack(rx_buff, ICUSB_TRANSFER_RX_BUFF_SIZE, ICUSB_CCID_STATUS_NO_CARD) ;
					// Not OK case let MD to timeout and recover the card
					icusb_print(PRINT_ERROR, "[ICUSB][ERROR] get the Rx error packets..\n");
					
				}else{									
					fsm_current_state = ICUSB_FSM_TRANSFER_TX_PROCESS;		
					
				}

				break ;
			}

		}
						
	}

		
	icusb_print(PRINT_INFO, "[ICUSB][INFO] <==== icusb_transfer_thread\n");

	return 0;
}

void sig_handler(int signal)
{
	icusb_print(PRINT_WARN, "[ICUSB][WARN] ====> sig_handler, receive signal %d \n", signal);
	daemon_exit = 1 ;

	return ;
}


int main(void)
{
	int ret ;
	
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ====> main , hello world! I am icusb_main \n");

	/* Init handle */
#if 0 //ICUSB_UT_NO_CCCI
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ====> enter ICUSB UT mode .\n");
#else
	ret = icusb_ccci_open() ;
	if (ret < 0 ){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_open() fail , return %d\n", ret);
		return -1 ;
	}
#endif	
	signal(SIGKILL, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGSEGV, sig_handler);
	
	pthread_create(&icusb_thread_tid, NULL, icusb_transfer_thread, NULL); 
	
	pthread_create(&icusb_recover_thread_tid, NULL, icusb_recovery_thread, NULL);
	
	/* Waiting Thread exit */
	pthread_join(icusb_thread_tid, NULL);
	
	/* Exit handle */
	icusb_ccci_close() ;
	icusb_print(PRINT_INFO, "[ICUSB][INFO] <==== main\n");
	
	return 0;
}


