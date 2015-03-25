#ifndef __ICUSB_MAIN_H__
#define __ICUSB_MAIN_H__

#define ICUSB_TRANSFER_RX_BUFF_SIZE (512)  // This size should be greater than (CCCI_TTY_MAX_PKT_SIZE + sizeof(s_ccci_mtk_header) + sizeof(struct ccid_pc2rdr_xfr_block_msg))
#define ICUSB_TRANSFER_TX_BUFF_SIZE (512) // This size should be greater than (CCCI_TTY_MAX_PKT_SIZE + sizeof(s_ccci_mtk_header) + sizeof(struct ccid_pc2rdr_xfr_block_msg))


#define ICUSB_USB_ENUM_SETUP_TIME_US (500000)

#define ICUSB_USB_DISCONNECT_TIME_US (2000000)

#define ICUSB_USB_DISCONNECT_POLL_INTERVAL_US (200000)

#define ICUSB_THREAD_LOOP_INTERVAL_US (5000)

#define ICUSB_CONTROL_MSG_DELAY_US (1000000)

#define ICUSB_THREAD_TRANSFER_STATE_CHANGE_INTERVAL_US (1000)

#define ICUSB_TX_ERR_DELAY_US (500000)

#define ICUSB_RESET_DELAY_US (10000)

#define ICUSB_NODEV_DELAY_US (500000)


enum icusb_finite_state_machine
{
    ICUSB_FSM_RESET = 0x00,
    ICUSB_FSM_HANDSHAKE_WAIT_RESET_USB ,
    ICUSB_FSM_HANDSHAKE_WAIT_DISCONNECT_DONE ,
    ICUSB_FSM_HANDSHAKE_WAIT_SET_VOLTAGE ,
    ICUSB_FSM_HANDSHAKE_WAIT_ENABLE_USB ,
    ICUSB_FSM_TRANSFER_TX_PROCESS ,
    ICUSB_FSM_TRANSFER_RX_PROCESS ,
};

enum icusb_class_handle_return_codes {
	ICUSB_RX_GOOD_PACKET = 0x10 , 
	ICUSB_RX_CTRL_PACKET = 0x11 , 
	ICUSB_RX_DATA_PACKET	= 0x12 ,
	ICUSB_RX_NO_DATA	= 0x13 ,

	ICUSB_TX_GOOD_PACKET = 0x20 , 

	ICUSB_CTRL_COMPLETE = 0x30 ,	
	ICUSB_CTRL_CHANGE_VOLTAGE = 0x31 ,	
	ICUSB_CTRL_STOP_SESSION = 0x32 ,	
	
	ICUSB_ERROR_PACKET	= -1 ,
	ICUSB_CHANNEL_NOT_READY	= -2 ,	
	ICUSB_HEADERROOM_ERROR	= -3 ,	
	ICUSB_TX_TRANSFER_ERROR	= -4 ,	
};

#define ICUSB_USB_CTRL_PROC_FILE	"/proc/IC_USB_CMD_ENTRY"

#define PREFER_VOL_STS_SHIFT (0)
#define PREFER_VOL_STS_MSK (0x3)

#define PREFER_VOL_NOT_INITED  0x0
#define PREFER_VOL_PWR_NEG_FAIL 0x1
#define PREFER_VOL_PWR_NEG_OK 0x2


#define PREFER_VOL_CLASS_SHIFT (8)
#define PREFER_VOL_CLASS_MSK (0xff)

#define IC_USB_PREFER_CLASSB_ENABLE_BIT 0x80

enum ICUSB_USB_PHY_VOLTAGE_TYPE
{
    ICUSB_VOL_18 = 0x00,
    ICUSB_VOL_30,
    ICUSB_VOL_50,
};

enum ICUSB_USB_SESSION_CONTROL
{
    ICUSB_SESSION_END = 0x00,
    ICUSB_SESSION_START = 0x01,
};


enum IC_USB_AP_MD_CTRL_CMD_TYPE
{
    USB11_START_SESSION = 0x00,
    USB11_INIT_PHY_BY_VOLTAGE=0x01,
    USB11_WAIT_DISCONNECT_DONE=0x02,

    ICUSB_ICC_POWER_ON=0x10,
    ICUSB_ICC_POWER_OFF=0x11,
    ICUSB_GET_SLOT_STATUS=0x12,
};

enum IC_USB_AP_MD_CTRL_RESP_TYPE
{
    ICUSB_ACK_STATUS = 0x00,
    ICUSB_ACK_WITH_ATR = 0x01 ,
};

typedef struct _s_icusb_ctrl_data_field
{
    unsigned char type;
    unsigned char length;
    unsigned char data[0];
}s_icusb_ctrl_data_field;

typedef struct _s_icusb_ctrl_ack_field {
	unsigned char type ;	
	unsigned char length ;	
	unsigned char value ;	
}s_icusb_ctrl_ack_field;


#endif
