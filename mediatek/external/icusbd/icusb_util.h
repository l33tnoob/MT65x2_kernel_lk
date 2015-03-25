#ifndef __ICUSB_UTIL_H__
#define __ICUSB_UTIL_H__

#include <libusb/libusb.h>
#include <assert.h>
#include <cutils/xlog.h>

#define LOG_TAG "ICUSBD"

#define ANDROID_ICUSB (1)

enum icusb_print_levels {
	PRINT_VERB = 0 , 
	PRINT_INFO = 1 , 
	PRINT_WARN = 2 , 
	PRINT_ERROR = 3 , 	
} ;

#define icusb_print_level (PRINT_INFO)

#define print_level_avail(level) (level >= icusb_print_level ? 1:0)

#if ANDROID_ICUSB
#define icusb_print(level, ...) 		\
{ 										\
    if(print_level_avail(level)){ 		\
    	if (level == PRINT_ERROR){		\
    		XLOGE(__VA_ARGS__);			\
    	}else{							\
	    	XLOGD(__VA_ARGS__);			\
    	}								\
	}				     				\
}

#define icusb_assert()					\
{										\
	printf("[ICUSB][ASSERT] Assert at %s|%s, line: %d\n", __FILE__ , __FUNCTION__, __LINE__); \
	XLOGE("[ICUSB][ASSERT] Assert at %s|%s, line: %d\n", __FILE__ , __FUNCTION__, __LINE__); \
	assert(0);		\
}

#else

#define icusb_print(level, ...) 		\
{ 										\
    if(print_level_avail(level)){ 		\
    	printf(__VA_ARGS__);			\
	}				     				\
}

#define icusb_assert()					\
{										\
	printf("[ICUSB][ASSERT] Assert at %s|%s, line: %d\n", __FILE__ , __FUNCTION__, __LINE__); \
	assert(0);		\
}  


#endif



#define ICCD_CLASS_DESCRIPTOR_LENGTH	(0x36)
#define ICCD_CLASS_DESCRIPTOR_TYPE		(0x21)
#define ICCD_MAC_EP_NUMBE				(3)

enum icusb_sw_return_codes {
	ICUSB_UICC_CTRL_RESP 		= 101 ,
	ICUSB_UICC_DATA_RESP 		= 100 ,

	ICUSB_NO_DATA = 1 , 
	ICUSB_OK = 0 , 

	ICUSB_DEV_GENERAL_ERR 		= -100 ,
	ICUSB_DEV_NOT_FOUND 		= -101 ,
	ICUSB_DEV_DESCRIPTOR_ERR 	= -102 ,
	ICUSB_DEV_INIT_ERR 			= -103 ,
	ICUSB_DEV_OPEN_FAIL			= -104 ,
	ICUSB_DEV_HAS_DRIVER		= -105 ,

	ICUSB_UICC_GENERAL_ERR 		= -200 ,
	ICUSB_UICC_EP_ERR 			= -201 ,
	ICUSB_UICC_NOT_FOUND		= -202 ,

	ICUSB_CCCI_GENERAL_ERR		= -300 ,
	ICUSB_CCCI_OPEN_ERR			= -301 ,
	ICUSB_CCCI_NO_PORT			= -302 ,
	ICUSB_CCCI_USB_RX_FAIL		= -303 ,
	ICUSB_CCCI_USB_RX_EMPTY		= -304 ,
	ICUSB_CCCI_TTY_RX_FAIL		= -305 ,
	ICUSB_CCCI_IOCTL_FAIL		= -306 ,

	ICUSB_CCCI_USB_TX_FAIL		= -310 ,
	ICUSB_CCCI_USB_TX_LEN_ERR	= -311 ,
	ICUSB_CCCI_USB_TX_UNDERFLOW	= -312 ,
	ICUSB_CCCI_USB_TX_EMPTY		= -313 ,

	ICUSB_USB_PROC_NO_FD		= -400 , 
	ICUSB_USB_PROC_WRITE_FAIL	= -401 , 
	ICUSB_USB_PROC_READ_FAIL	= -402 , 

	ICUSB_USB_NL_NO_FD			= -500 , 
	ICUSB_USB_NL_BIND_FAIL		= -501 , 
	ICUSB_USB_NL_HANDSHAKE_FAIL	= -502 , 
	ICUSB_USB_NL_ACK_PID		= -503 , 
	ICUSB_USB_NL_UNKNOWN_MSG	= -504 , 

} ;

struct s_icusb_device {
	libusb_device *usb_dev ;
	libusb_device_handle *usb_device_handle ;

	int icusb_smartcard_intf_number ;
	
	int icusb_smartcard_num_ep ;
	int icusb_smartcard_bulk_in_ep ;
	int icusb_smartcard_bulk_out_ep ;
	int icusb_smartcard_interrupt_in_ep ;
	
	int is_ready ;	
};

typedef struct s_icusb_device s_icusb_device;

#pragma pack(push) 
#pragma pack(1)   
struct libusb_class_usbicc_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdCCID;
	uint8_t  bMaxSlotIndex;
	uint8_t bVoltageSupport; // bReserved
	uint32_t  dwProtocols;
	uint32_t  dwDefaultClock; // bReserved
	uint32_t  dwMaximumClock; // bReserved
	uint8_t  bNumClockSupported;	 // bReserved
	uint32_t  dwDataRate; // bReserved
	uint32_t  dwMaxDataRate; // bReserved
	uint8_t  bNumDataRatesSupported; // bReserved
	uint32_t  dwMaxIFSD;
	uint32_t  dwSynchProtocols; // bReserved
	uint32_t  dwMechanical;	
	uint32_t  dwFeatures;	
	uint32_t  dwMaxCCIDMessageLength;	
	uint8_t  bClassGetResponse;	// bReserved
	uint8_t  bClassEnveloppe; // bReserved	
	uint16_t  wLcdLayout; // bRFU	
	uint8_t  bPinSupport; // bRFU
	uint8_t  bMaxCCIDBusySlots;	
};
#pragma pack(pop)


int icusb_netlink_open();

int icusb_netlink_recv_msg();

void icusb_netlink_close() ;

int icusb_do_exec(char *cmd , char *argv[]) ;

int icusb_util_write_proc(char * file_path, char* buf, int size) ;

int icusb_util_get_proc(char * file_path, char* buf, int size) ;

int icusb_get_device_list(libusb_device ***usb_dev_list) ;

int icusb_open_dev(s_icusb_device *icusb_device) ;

int icusb_close_dev(s_icusb_device *icusb_device) ;

#endif
