#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h> 
#include "hardware/ccci_intf.h"

#include "icusb_ccci.h"

static int fd_ccci_icusb = -1 ;

int icusb_ccci_notify_md_recover(void)
{		
	int fd_ccci_recover ;
	int ret = ICUSB_OK;
	int ioctl_ret ;
	unsigned int data = 0 ;
	char dev_node[32];
	
	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_ICUSB_IOCTL, MD_SYS1));
	fd_ccci_recover = open(dev_node, O_RDWR | O_NONBLOCK);
	if(fd_ccci_recover < 0) {
		icusb_print(PRINT_ERROR, "[ICUSB][Error] Fail to open ICUSB RECOVER DEV NODE: %s\n", strerror(errno));
		return ICUSB_CCCI_OPEN_ERR;
	}

	icusb_print(PRINT_ERROR, "[ICUSB] Open ICUSB RECOVER DEV NODE, FD is: %d\n", fd_ccci_recover);

	ioctl_ret = ioctl(fd_ccci_recover, CCCI_IOC_SEND_ICUSB_NOTIFY, &data) ;
	if (ioctl_ret < 0) {
		icusb_print(PRINT_ERROR, "[ICUSB][Error] Fail to ioctl CCCI_RECOVER_IOC_SEND_ICUSB_NOTIFY,path: %s, errno: %d , %s\n",dev_node, errno, strerror(errno));
		ret = ICUSB_CCCI_IOCTL_FAIL ;
	}else{
		icusb_print(PRINT_WARN, "[ICUSB][Error] CCCI_RECOVER_IOC_SEND_ICUSB_NOTIFY OK !!! ret: %d \n", ioctl_ret);
		ret = ICUSB_OK ;
	}

    close(fd_ccci_recover) ;
    
    return ret ;

}


int icusb_ccci_open(void)
{		
	char dev_node[32];

	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_ICUSB_DATA, MD_SYS1));
	fd_ccci_icusb = open(dev_node, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd_ccci_icusb < 0) {
		icusb_print(PRINT_ERROR, "[ICUSB][Error] Fail to open ICUSB CCCI TTY: %s\n", strerror(errno));
		return ICUSB_CCCI_OPEN_ERR;
	}

	icusb_print(PRINT_ERROR, "[ICUSB] Open ICUSB CCCI TTY, FD is: %d\n", fd_ccci_icusb);
    
	return ICUSB_OK;

}

int icusb_ccci_close(void)
{
	close(fd_ccci_icusb) ;

	fd_ccci_icusb = -1 ;
	
	icusb_print(PRINT_ERROR, "[ICUSB] Close ICUSB CCCI TTY Port.\n");
    
	return ICUSB_OK;

}

void icusb_dump_payload(char *payload , int offset, int len)
{
	int i = 0 ;	
	
	for (i = offset ; i < len ; i++){
		icusb_print(PRINT_INFO, "%02x ",*(payload+i));
	}

    icusb_print(PRINT_INFO, "\n");			

	return ;
}

int icusb_rx_ccci_write(char *apdu_buff , int apdu_length, s_ccci_mtk_header* ccci_mtk_header)
{	
	char *to_ccci_buff = NULL;
	int to_ccci_length;
	int ret_val;
	int has_sent = 0;
	s_ccci_mtk_header* tmp_ccci_mtk_header ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_ccci_rx_ccci_write\n");
		
FILL_THE_MTK_HEADER:
	to_ccci_buff = apdu_buff - sizeof(s_ccci_mtk_header);
	to_ccci_length = apdu_length+sizeof(s_ccci_mtk_header);
	
	if (sizeof(s_ccci_mtk_header)>0){
		tmp_ccci_mtk_header = (s_ccci_mtk_header*)to_ccci_buff ;
		tmp_ccci_mtk_header->sif = ccci_mtk_header->sif ; 
		tmp_ccci_mtk_header->cp = ccci_mtk_header->cp ;		
		tmp_ccci_mtk_header->pcb = ccci_mtk_header->pcb ;		
		tmp_ccci_mtk_header->len = ccci_mtk_header->len ;		
	}	

	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_rx_ccci_write APDU length %d bytes, pcb is 0x%x\n", tmp_ccci_mtk_header->len , tmp_ccci_mtk_header->pcb);
	/* Sent to CCCI */		

	icusb_print(PRINT_INFO, "[ICUSB] CCCI RXRXRX payload : \n");
	icusb_dump_payload(to_ccci_buff, 0, to_ccci_length) ;
    
#if !ICUSB_UT_NO_CCCI	
	has_sent = 0 ; 	
    while (has_sent < to_ccci_length)
    {
	   	//pthread_mutex_lock (&ccci_mutex) ;		
		ret_val = write(fd_ccci_icusb, to_ccci_buff+has_sent, (to_ccci_length - has_sent));
		//pthread_mutex_unlock (&ccci_mutex) ;
		
        if (ret_val<0)
        {
        	icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_rx write to TTY port fail, ret: %d\n",ret_val);
            return ICUSB_CCCI_USB_RX_FAIL;	
        }
        else
        {	
            has_sent += ret_val; // the ret is the sent length
            icusb_print(PRINT_VERB, "[ICUSB] %d bytes has been write to TTY port\n", ret_val);
        }
    }
    
#endif
    icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_ccci_rx_ccci_write\n");
    return has_sent;		

}

// recieve from CCCI TTY port 
// return the apdu buff

#if ICUSB_UT_NO_CCCI
static char ut_tx_sudo_md_packet_buff[512] ;
static unsigned int ut_tx_sudo_md_packet_length = 0 ;

int icusb_fill_ut_tx_buff(char *payload_buff , int payload_buff_size)
{
	if (payload_buff_size > 512){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_fill_ut_tx_buff, the payload is larger than tx buffer size : %d\n", 512);
		return CCCI_ERROR_PACKET ;
	}
	memcpy(ut_tx_sudo_md_packet_buff, payload_buff, payload_buff_size) ;
	ut_tx_sudo_md_packet_length = payload_buff_size ;

	return 0 ;

}
#endif

int icusb_tx_ccci_read(char *tx_buff , int tx_buff_size, int header_room, int *apdu_length, char **p_apdu_buff, unsigned short * chain_parameter)
{
	//char tx_buff[CCCI_TTY_TX_BUFF_SIZE] ;
	char *from_ccci_buff = NULL;
	//char *apdu_buff ;
	//char *to_usb_buff ;
	unsigned int has_read = 0 ;
	unsigned int payload_len = 0 ;
	int ret_val;
	s_ccci_mtk_header* ccci_mtk_header ;

	icusb_print(PRINT_VERB, "[ICUSB] ===> icusb_ccci_tx_ccci_read\n");

#if !ICUSB_UT_NO_CCCI
	if(fd_ccci_icusb < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] fd_ccci_icusb is not exist\n");
		return CCCI_CHANNEL_NOT_READY;
	}
#endif

	//memset(tx_buff, 0, tx_buff_size);

	// only reserved the header room for CLASS header 
	from_ccci_buff = tx_buff + header_room ;
	
	/* Receive the MTK HEADER from CCCI */
#if ICUSB_UT_NO_CCCI	
	if (ut_tx_sudo_md_packet_length == 0){
		return CCCI_TX_NO_DATA ;
	}
	icusb_print(PRINT_ERROR, "[ICUSB][INFO] ut_tx_sudo_md_packet_length is %d\n", ut_tx_sudo_md_packet_length);

	memcpy(from_ccci_buff , ut_tx_sudo_md_packet_buff, ut_tx_sudo_md_packet_length) ;
	ccci_mtk_header= (s_ccci_mtk_header*)from_ccci_buff ;
	if (chain_parameter != NULL){
		//*chain_parameter = (ccci_mtk_header->pcb & 0x3F)  ; 		
		*chain_parameter = ccci_mtk_header->cp ;
		icusb_print(PRINT_INFO, "[ICUSB][INFO] chain_parameter is 0x%x\n", *chain_parameter);		
	}
	
	if ((ccci_mtk_header->pcb & CCCI_MTK_H_PCB_MODE) == CCCI_MTK_H_PCB_MODE_CTRL){
		icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ccci_read CONTROL packet, length %d bytes\n", ccci_mtk_header->len);
	}else{
		icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_tx_ccci_read DATA packet, length %d bytes\n", ccci_mtk_header->len);
	}
	
	ut_tx_sudo_md_packet_length = 0 ;

#else
	ret_val = read(fd_ccci_icusb, from_ccci_buff, sizeof(s_ccci_mtk_header));
	
	if(ret_val <= 0){
		if (errno == EAGAIN){
            icusb_print(PRINT_VERB, "[ICUSB] icusb_ccci_tx read 0 bytes, loop again\n");
            return CCCI_TX_NO_DATA ;
		}else{
			icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_tx Error, the errno : %d\n", errno);
			return CCCI_ERROR_PACKET;
		}
	}else if (ret_val != sizeof(s_ccci_mtk_header)){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_tx header is smaller than %d bytes\n", ret_val);
		return CCCI_ERROR_PACKET;
	}

	ccci_mtk_header= (s_ccci_mtk_header*)from_ccci_buff ;

	if (chain_parameter != NULL){
		//*chain_parameter = (ccci_mtk_header->pcb & 0x3F)  ; 
		*chain_parameter = ccci_mtk_header->cp ;
		icusb_print(PRINT_INFO, "[ICUSB][INFO] chain_parameter is 0x%x\n" , *chain_parameter);
	}

	if ((ccci_mtk_header->len + sizeof(s_ccci_mtk_header)) > tx_buff_size){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_tx length %d bytes is greater than tx buffer\n", ccci_mtk_header->len);
		return CCCI_ERROR_PACKET;
	}
	
	/* Receive the DATA from CCCI */
	while(has_read<ccci_mtk_header->len)
	{
		usleep(CCCI_TX_INTERVAL_US);

		//pthread_mutex_lock (&ccci_mutex) ;
		ret_val = read(fd_ccci_icusb, from_ccci_buff + sizeof(s_ccci_mtk_header) + has_read, (ccci_mtk_header->len-has_read));
		//pthread_mutex_unlock (&ccci_mutex) ;

		if(ret_val <= 0){
			if (errno == EAGAIN){
                icusb_print(PRINT_VERB, "[ICUSB] icusb_ccci_tx read 0 bytes, loop again\n");
			}else{
				icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_ccci_tx Error, the errno : %d\n", errno);
				usleep(CCCI_TX_ERR_DELAY_US);
			}
		}else{
			has_read += ret_val;			
		}		
	}
#endif	

	*p_apdu_buff = from_ccci_buff + sizeof(s_ccci_mtk_header) ;
	*apdu_length = ccci_mtk_header->len ;

	icusb_print(PRINT_INFO, "[ICUSB] CCCI TXTXTX payload : \n");
	icusb_dump_payload(from_ccci_buff, 0, (*apdu_length + sizeof(s_ccci_mtk_header))) ;

	icusb_print(PRINT_VERB, "[ICUSB] <=== icusb_ccci_tx_ccci_read\n");
	if ((ccci_mtk_header->pcb & CCCI_MTK_H_PCB_MODE) == CCCI_MTK_H_PCB_MODE_CTRL){
		return CCCI_TX_CTRL_PACKET ;
	}else{
		return CCCI_TX_DATA_PACKET ;
	}

}


