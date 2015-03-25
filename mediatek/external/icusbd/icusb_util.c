#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/socket.h>
#include <linux/netlink.h>
#include <malloc.h>
#include <sys/wait.h> /* for wait */



#include "icusb_util.h"


libusb_device **usb_devs;

int netlink_sock_fd = -1;

int icusb_get_device_list(libusb_device ***usb_dev_list)
{
	int cnt ;
	cnt = libusb_get_device_list(NULL, usb_dev_list);

	return cnt;
	
}

int icusb_find_match_device(libusb_device **devs , s_icusb_device *icusb_device)
{
	int r ;
	int i = 0, j= 0, k=0, l=0 ;
	libusb_device *iccd_dev = NULL ;
	libusb_device *one_dev = NULL ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> icusb_find_match_device\n");
	while ((one_dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor device_descriptor;
		struct libusb_config_descriptor *config_descriptor; 
						
		r = libusb_get_device_descriptor(one_dev, &device_descriptor);
		if (r < 0) {
			icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to get device descriptor");
			return ICUSB_DEV_NOT_FOUND ;
		}

		icusb_print(PRINT_VERB, "\n\n +++++  the %d device   +++++\n", i ) ; 		
		icusb_print(PRINT_VERB, "VID:0x%04x PID:0x%04x (bus %d, device %d)\n", device_descriptor.idVendor, device_descriptor.idProduct, libusb_get_bus_number(one_dev), libusb_get_device_address(one_dev));					
		icusb_print(PRINT_VERB, "[device descriptor] -- \n") ;
		icusb_print(PRINT_VERB, "\t bDeviceClass:0x%02x\n", device_descriptor.bDeviceClass) ;
		icusb_print(PRINT_VERB, "\t bDeviceSubClass:0x%02x\n", device_descriptor.bDeviceSubClass) ;
		icusb_print(PRINT_VERB, "\t bDeviceProtocol:0x%02x\n", device_descriptor.bDeviceProtocol) ;
		icusb_print(PRINT_VERB, "\t idVendor:0x%04x\n", device_descriptor.idVendor) ;
		icusb_print(PRINT_VERB, "\t idProduct:0x%04x\n", device_descriptor.idProduct) ;
		icusb_print(PRINT_VERB, "\t bNumConfigurations:%d\n", device_descriptor.bNumConfigurations) ;

		for (j=0 ; j<device_descriptor.bNumConfigurations ; j++){
			
			r = libusb_get_config_descriptor(one_dev, j, &config_descriptor);
			if (r < 0) {
				icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to get config descriptor");
				return ICUSB_DEV_DESCRIPTOR_ERR;
			}
			icusb_print(PRINT_VERB, "\t [configuration descriptor] -- \n") ;
			icusb_print(PRINT_VERB, "\t\t bConfigurationValue:%d\n", config_descriptor->bConfigurationValue) ;
			icusb_print(PRINT_VERB, "\t\t bNumInterfaces:%d\n", config_descriptor->bNumInterfaces) ;

			for (k=0 ; k<config_descriptor->bNumInterfaces ; k++){
				struct libusb_interface interf_descriptor;				
				struct libusb_class_usbicc_descriptor *uicc_descriptor;
								
				interf_descriptor = config_descriptor->interface[k] ;
				icusb_print(PRINT_VERB, "\t\t [interface descriptor] -- \n") ;
				icusb_print(PRINT_VERB, "\t\t\t num_altsetting:%d\n", interf_descriptor.num_altsetting) ;
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceNumber:%d\n", interf_descriptor.altsetting->bInterfaceNumber) ;
				icusb_print(PRINT_VERB, "\t\t\t bNumEndpoints:%d\n", interf_descriptor.altsetting->bNumEndpoints) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceClass:0x%02x\n", interf_descriptor.altsetting->bInterfaceClass) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceSubClass:0x%02x\n", interf_descriptor.altsetting->bInterfaceSubClass) ;				
				icusb_print(PRINT_VERB, "\t\t\t bInterfaceProtocol:0x%02x\n", interf_descriptor.altsetting->bInterfaceProtocol) ;
				icusb_print(PRINT_VERB, "\t\t\t extra_length:%d\n", interf_descriptor.altsetting->extra_length) ;


				if ((interf_descriptor.altsetting->bInterfaceClass == 0x0b) /* smat card class*/
				&& (interf_descriptor.altsetting->extra_length >= ICCD_CLASS_DESCRIPTOR_LENGTH)) /* UICC extra class*/
				{
					uicc_descriptor = (struct libusb_class_usbicc_descriptor *)interf_descriptor.altsetting->extra ;
					if (uicc_descriptor->bDescriptorType == ICCD_CLASS_DESCRIPTOR_TYPE){ /* UICC bDescriptorType*/
						/* FOUND !! */
						const struct libusb_endpoint_descriptor *endpoint_descriptor[ICCD_MAC_EP_NUMBE] = {NULL}; // most three EPs 

						if (interf_descriptor.altsetting->bNumEndpoints>ICCD_MAC_EP_NUMBE){
							icusb_print(PRINT_ERROR, "[ICUSB][Error] the UICC device has more than %d EPs\n", ICCD_MAC_EP_NUMBE);
							return ICUSB_UICC_EP_ERR ;
						}
						
						for (l=0 ; l<interf_descriptor.altsetting->bNumEndpoints ; l++){						
							endpoint_descriptor[l] = &interf_descriptor.altsetting->endpoint[l] ;

							if (!((endpoint_descriptor[l]->bEndpointAddress) & 0x80) && (endpoint_descriptor[l]->bmAttributes == 0x02)){ // OUT , BULK , 
								icusb_print(PRINT_VERB, "\t\t\t [endpoint descriptor] OUT, BULK -- \n") ;
								icusb_device->icusb_smartcard_bulk_out_ep = endpoint_descriptor[l]->bEndpointAddress ;								
								
							}else if (((endpoint_descriptor[l]->bEndpointAddress) & 0x80) && (endpoint_descriptor[l]->bmAttributes == 0x02)){ // IN , BULK , 
								icusb_print(PRINT_VERB, "\t\t\t [endpoint descriptor] IN, BULK -- \n") ;
								icusb_device->icusb_smartcard_bulk_in_ep = endpoint_descriptor[l]->bEndpointAddress ;								

							}else if (((endpoint_descriptor[l]->bEndpointAddress) & 0x80) && (endpoint_descriptor[l]->bmAttributes == 0x03)){
								icusb_print(PRINT_VERB, "\t\t\t [endpoint descriptor] IN, INTERRUPT -- \n") ;
								icusb_device->icusb_smartcard_interrupt_in_ep = endpoint_descriptor[l]->bEndpointAddress ;								

							}							
							icusb_print(PRINT_VERB, "\t\t\t\t bmAttributes:0x%02x\n", endpoint_descriptor[l]->bmAttributes) ;
							icusb_print(PRINT_VERB, "\t\t\t\t bEndpointAddress:0x%02x\n", endpoint_descriptor[l]->bEndpointAddress) ;
							icusb_print(PRINT_VERB, "\t\t\t\t wMaxPacketSize:%d\n", endpoint_descriptor[l]->wMaxPacketSize) ;					
						}												
												
						icusb_print(PRINT_VERB, "\t\t\t [UICC class specific descriptor] -- \n") ;
						icusb_print(PRINT_VERB, "\t\t\t\t bLength:%d\n", uicc_descriptor->bLength) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bDescriptorType:0x%02x\n", uicc_descriptor->bDescriptorType) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bcdCCID:0x%04x\n", uicc_descriptor->bcdCCID) ;
						icusb_print(PRINT_VERB, "\t\t\t\t bVoltageSupport:0x%02x\n", uicc_descriptor->bVoltageSupport) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwProtocols:0x%08x\n", uicc_descriptor->dwProtocols) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwMaxIFSD:0x%08x\n", uicc_descriptor->dwMaxIFSD) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwFeatures:0x%08x\n", uicc_descriptor->dwFeatures) ;
						icusb_print(PRINT_VERB, "\t\t\t\t dwMaxCCIDMessageLength:%d\n", uicc_descriptor->dwMaxCCIDMessageLength) ;

						icusb_device->icusb_smartcard_num_ep = interf_descriptor.altsetting->bNumEndpoints;

						icusb_device->usb_dev = one_dev ;

						icusb_device->icusb_smartcard_intf_number = interf_descriptor.altsetting->bInterfaceNumber ;
						FOUND_UICC:
						icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== icusb_find_match_device, found !!!\n");
						return ICUSB_OK ;
					}																							
				}
			}
		}
	}

	icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== icusb_find_match_device, not found\n");
	NOT_FOUND_UICC:
	return ICUSB_UICC_NOT_FOUND;
}

#define NL_MAX_BUF_SIZE	64
#define NL_HANDSHAKE_STRING	"ICUSB_NETLINK"
static char nl_payload_buff[NL_MAX_BUF_SIZE] ;

void icusb_netlink_close()
{
	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_close_netlink_to_kernel_usb\n");
	 
	close(netlink_sock_fd);

	netlink_sock_fd = -1 ;

	return ;	
}

int icusb_netlink_open()
{
	int ret ;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct timeval tv;
	
	struct msghdr msg;


	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_open_netlink_to_kernel_usb\n");
	
	netlink_sock_fd = socket(PF_NETLINK, SOCK_RAW,NETLINK_USERSOCK);
	if (netlink_sock_fd == -1){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_open_netlink_to_kernel_usb, the socket FD create fail\n");
		return ICUSB_USB_NL_NO_FD;
	}
	
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */ 
	src_addr.nl_groups = 0; /* not in mcast groups */ 
	if ( -1 == bind(netlink_sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr))){
		icusb_netlink_close() ;
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_open_netlink_to_kernel_usb, bind socket fd fail\n");
		return ICUSB_USB_NL_BIND_FAIL ;
	}

	tv.tv_sec  = 0;  /* Wait for max 10 sec on recvmsg */
	tv.tv_usec = 5000;
    
	setsockopt(netlink_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(struct timeval)); 

	
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; /* For Linux Kernel */ 
	dest_addr.nl_groups = 0; /* unicast */ 
	 
	nlh=(struct nlmsghdr *)nl_payload_buff ;
	/* Fill the netlink message header */ 
	nlh->nlmsg_len = NLMSG_SPACE(NL_MAX_BUF_SIZE);
	nlh->nlmsg_pid = getpid(); /* self pid */ 
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */ 
	strcpy(NLMSG_DATA(nlh), NL_HANDSHAKE_STRING);
	 
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	
	memset(&msg, 0, sizeof(msg)) ;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Send message to kernel to notify PID */
	ret = sendmsg(netlink_sock_fd, &msg, 0) ;
	if (-1 ==ret){
		icusb_netlink_close() ;
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_open_netlink_to_kernel_usb, sendmsg to handshake failed !!!\n");
		perror("send fail");
		return ICUSB_USB_NL_HANDSHAKE_FAIL ;
	}
	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_open_netlink_to_kernel_usb, sendmsg sent %d bytes to handshake.\n", ret);
	 	 
	return ICUSB_OK;	
}

int icusb_netlink_recv_msg()
{
	struct msghdr msg;
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	char str_buf[64];
	char *rcv_buf ;
	int bytes_received ;

	if (netlink_sock_fd <0){
		return ICUSB_USB_NL_NO_FD ;
	}

	nlh=(struct nlmsghdr *)nl_payload_buff ;
	 
	iov.iov_base = (void *)nlh;
	iov.iov_len = NL_MAX_BUF_SIZE;
	
	memset(&msg, 0, sizeof(msg)) ;
	msg.msg_name = (void *)&(dest_addr);
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sprintf(str_buf, "%d", getpid());
	bytes_received = recvmsg(netlink_sock_fd, &msg, 0); 
	if (bytes_received > 0){
		rcv_buf = NLMSG_DATA(msg.msg_iov->iov_base) ;
	 
		icusb_print(PRINT_INFO, "[ICUSB][INFO] PID : %s , Received message payload: %s\n", str_buf , rcv_buf);
		if(!strcmp(str_buf, rcv_buf)){
			icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_netlink_recv_msg. recieve the correct PID ACK from Kernel\n");
			return ICUSB_USB_NL_ACK_PID;		
		}else if (!strcmp("HELLO, SS7_IC_USB!!!", rcv_buf)){
			icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_netlink_recv_msg. recieve the correct Recover from Kernel\n");
			return ICUSB_OK ;
		}else{	
			icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_netlink_recv_msg : get unknown message from Kernel\n");
			return ICUSB_USB_NL_UNKNOWN_MSG ;
		}
	}else{
		return ICUSB_NO_DATA ;
	}
		
}

int icusb_do_exec(char *cmd , char *argv[])
{
    /*Spawn a child to run the program.*/
    pid_t pid=fork();
    if (pid==0) { /* child process */
        //static char *argv[]={"echo","Foo is my name.",NULL};
        //execv("/bin/echo",argv);
        execv(cmd , argv) ;
        exit(127); /* only if execv fails */
    }
    else { /* pid!=0; parent process */
        waitpid(pid,0,0); /* wait for child to exit */
    }
    return ICUSB_OK;
}


int icusb_util_write_proc(char * file_path, char* buf, int size)
{
	int proc_fd ;
	int writed ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_util_write_proc, proc:%s, write bytes: %d, buf[3]: 0x%02x 0x%02x 0x%02x\n", file_path, size, buf[0], buf[1], buf[2]);
	
	proc_fd = open(file_path, O_WRONLY) ;
	if(proc_fd==-1){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc %s proc file not found\n", file_path);
        return ICUSB_USB_PROC_NO_FD;
    }

    writed = write(proc_fd, buf, size);
#if 0    
	if(writed !=size){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_write_proc write proc not complte, proc:%s, write bytes: %d, writed: %d\n", file_path, size, writed);
        return ICUSB_USB_PROC_WRITE_FAIL;
    }
#endif

    close(proc_fd) ;

    return ICUSB_OK ;

}

int icusb_util_get_proc(char * file_path, char* buf, int size)
{
	int proc_fd ;
	int readed ;

	proc_fd = open(file_path, O_RDONLY) ;
	if(proc_fd==-1){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_get_proc %s proc file not found\n", file_path);
        return ICUSB_USB_PROC_NO_FD;
    }

    readed = read(proc_fd, buf, size);
#if 0    
	if(readed !=size){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_util_get_proc read proc not complte, proc:%s, read bytes: %d, readed: %d\n", file_path, size, readed);
        return ICUSB_USB_PROC_READ_FAIL;
    }
#endif

    close(proc_fd) ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] icusb_util_get_proc, proc:%s, get bytes: %d, buf[3]: 0x%02x 0x%02x 0x%02x\n", file_path, size, buf[0], buf[1], buf[2]);
	
    return ICUSB_OK ;

}

int icusb_open_dev(s_icusb_device *icusb_device)
{
	int r , cnt ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> icusb_open_dev\n");
	
	if (icusb_device->is_ready == 1){
		icusb_print(PRINT_WARN, "[ICUSB][WARN] icusb_open_dev has been opened before\n");
		return ICUSB_OK ;
	}
	
	r = libusb_init(NULL);
	if (r < 0){
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_open_dev->libusb_init fail, ret %d\n", r);
		return ICUSB_DEV_INIT_ERR;
	}

	cnt = icusb_get_device_list(&usb_devs);
	if (cnt < 0){
		libusb_exit(NULL);
		icusb_print(PRINT_ERROR, "[ICUSB][ERROR] icusb_open_dev->icusb_get_device_list is 0\n");
		return ICUSB_DEV_NOT_FOUND;
	}

	r = icusb_find_match_device(usb_devs, icusb_device) ;
	if (r < 0){
		libusb_free_device_list(usb_devs, 1);
		libusb_exit(NULL);
		icusb_print(PRINT_WARN, "[ICUSB][WARN] icusb_open_dev->icusb_find_match_device can't find icusb device\n");
		return r;
	}

	r = libusb_open(icusb_device->usb_dev, &icusb_device->usb_device_handle);
	if (r < 0) {		
		libusb_free_device_list(usb_devs, 1);
		libusb_exit(NULL);
		icusb_print(PRINT_ERROR, "[ICUSB][Error] failed to open icusb device handle");
		return ICUSB_DEV_OPEN_FAIL;
	}

#if CHECK_MORE	
	if (libusb_kernel_driver_active(icusb_device->usb_device_handle, icusb_device->icusb_smartcard_intf_number) == 1){ // check the ICCD Interface has driver served ?
		icusb_print(PRINT_ERROR, "[ICUSB][Error] iccd class should not have driver served !");
		return ICUSB_DEV_HAS_DRIVER;
	}
#endif

	icusb_device->is_ready = 1 ;

	icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== icusb_open_dev\n");
	
	return ICUSB_OK;
}

int icusb_close_dev(s_icusb_device *icusb_device)
{		
	icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> icusb_close_dev\n");

	if (icusb_device->is_ready == 1){
		icusb_device->is_ready = 0 ;
		
		if (icusb_device->usb_device_handle){
			icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> libusb_close\n");
			libusb_close(icusb_device->usb_device_handle);
			icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== libusb_close\n");
		}

		icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> libusb_free_device_list\n");
		libusb_free_device_list(usb_devs, 1);
		icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== libusb_free_device_list\n");

		icusb_print(PRINT_INFO, "[ICUSB][INFO] ===> libusb_exit\n");
		libusb_exit(NULL);
		icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== libusb_exit\n");		

	}
	icusb_print(PRINT_INFO, "[ICUSB][INFO] <=== icusb_close_dev\n");
	
	return ICUSB_OK;
}

