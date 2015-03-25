#ifndef WIFI_DISPLAY_UIBC_TYPE_H
#define WIFI_DISPLAY_UIBC_TYPE_H

#define UINT8 unsigned char
#define UINT16 unsigned short

#define MAX_NUM_COORDINATE  5
#define UIBC_MAX_FULL_REQUEST_SIZE    1024
#define UIBC_HEADER_SIZE              4
#define UIBC_TIMESTAMP_SIZE           2


#define UIBC_VERSION             0x00

#define UIBC_TIMESTAMP_MASK      0x08
#define UIBC_INPUT_CATEGORY_MASK 0xF0


#define MAX_TCP_CONNECTIONS 1

#define ERROR    -1
#define SUCCESS   0


typedef enum
{
    WFD_UIBC_INPUT_CATEGORY_GENERIC = 0,
    WFD_UIBC_INPUT_CATEGORY_HIDC,
    WFD_UIBC_INPUT_CATEGORY_UNKNOWN = 0xFF
} UIBCInputCategoryCode;

// Table 4-5: Generic Input Type ID for User Inputs of the Generic Category
typedef enum
{
    WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN = 0,
    WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP,
    WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE,
    WFD_UIBC_GENERIC_IE_ID_KEY_DOWN,
    WFD_UIBC_GENERIC_IE_ID_KEY_UP,
    WFD_UIBC_GENERIC_IE_ID_ZOOM,
    WFD_UIBC_GENERIC_IE_ID_VSCROLL,
    WFD_UIBC_GENERIC_IE_ID_HSCROLL,
    WFD_UIBC_GENERIC_IE_ID_ROTATE
} UIBCGenericInputId;

// Table 4-16: HID Input Path
typedef enum
{
    WFD_UIBC_HIDC_IE_ID_INFRARED = 0,
    WFD_UIBC_HIDC_IE_ID_USB,
    WFD_UIBC_HIDC_IE_ID_BLUETOOTH,
    WFD_UIBC_HIDC_IE_ID_ZIGBEE,
    WFD_UIBC_HIDC_IE_ID_WIFI,
    WFD_UIBC_HIDC_IE_ID_VENDOR = 0xFF
} UIBCHidInputPath;

// Table 4-17: HID type
typedef enum
{
    WFD_UIBC_HIDCTYPE_IE_ID_KEYBOARD = 0,
    WFD_UIBC_HIDCTYPE_IE_ID_MOUSE,
    WFD_UIBC_HIDCTYPE_IE_ID_SINGLETOUCH,
    WFD_UIBC_HIDCTYPE_IE_ID_MULTITOUCH,
    WFD_UIBC_HIDCTYPE_IE_ID_JOYSTICK,
    WFD_UIBC_HIDCTYPE_IE_ID_CAMERA,
    WFD_UIBC_HIDCTYPE_IE_ID_GESTURE,
    WFD_UIBC_HIDCTYPE_IE_ID_REMOTECONTROLLER,
    WFD_UIBC_HIDCTYPE_IE_ID_VENDOR = 0xFF
} UIBCHidType;


// 4.11.3.1 Generic Input Body Format
// Table 4.4: Generic Input Message Format
typedef struct _WFD_UIBC_FORMAT_HDR {
    UINT8      byte1;    // version + T + half of Reserved
    UINT8      byte2;    // half of Reserved + InputCategory
    UINT16     length;
}__attribute__((__packed__)) WFD_UIBC_FORMAT_HDR;

typedef struct _WFD_UIBC_GENERIC_BODY_FORMAT_HDR {
    UINT8    ieID;
    UINT16    length;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_FORMAT_HDR;

// Describe Field for Generic Input Message for Key Down or Key Up
typedef struct _WFD_UIBC_GENERIC_BODY_KEY {
    UINT8      ieID;
    UINT16     length;  
    UINT8      reserved;    
    UINT16     code1;
    UINT16     code2;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_KEY;


typedef struct _WFD_UIBC_GENERIC_BODY_COORDINATE {
    UINT8      pointerID;
    UINT16     x;
    UINT16     y;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_COORDINATE;

// Describe Field of Generic InputMessage for Left Mouse Down(Up) /Touch Down(Up)
typedef struct _WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH {
    UINT8      ieID;
    UINT16     length;
    UINT8      numPointers;
    WFD_UIBC_GENERIC_BODY_COORDINATE coordinates[MAX_NUM_COORDINATE];
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH;

// Describe Field of Generic InputMessage for Zoom
typedef struct _WFD_UIBC_GENERIC_BODY_ZOOM {
    UINT8      ieID;
    UINT16     length;
    UINT16     x;
    UINT16     y;
    UINT8      intTimes;
    UINT8      fractTimes;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_ZOOM;

// Describe Field of Generic InputMessage for Vertical/Horizontal scroll
typedef struct _WFD_UIBC_GENERIC_BODY_SCROLL {
    UINT8      ieID;
    UINT16     length;
    UINT16     amount;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_SCROLL;

// Describe Field of Generic InputMessage for rotate
typedef struct _WFD_UIBC_GENERIC_BODY_ROTATE {
    UINT8      ieID;
    UINT16     length;
    UINT8      intAmount;
    UINT8      fractAmount;
}__attribute__((__packed__)) WFD_UIBC_GENERIC_BODY_ROTATE;

typedef struct ASCII_KEYCODE_MAPPING_STRUCT {
    UINT16     asciiCode;
    UINT8      keyCode;
}__attribute__((__packed__)) ASCII_KEYCODE_MAPPING_STRUCT;

typedef struct _WFD_UIBC_HIDC_BODY_FORMAT_HDR {
    UINT8       inputPath;
    UINT8       hidType;
    UINT8       usage;
    UINT16      length;
}__attribute__((__packed__)) WFD_UIBC_HIDC_BODY_FORMAT_HDR;



#endif
