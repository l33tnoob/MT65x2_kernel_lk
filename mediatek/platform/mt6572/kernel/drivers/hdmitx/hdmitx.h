// ---------------------------------------------------------------------------

#ifndef     HDMITX_H
#define     HDMITX_H

#define HDMI_CHECK_RET(expr)                                                \
    do {                                                                    \
        HDMI_STATUS ret = (expr);                                           \
        if (HDMI_STATUS_OK != ret) {                                        \
            printk("[ERROR][mtkfb] HDMI API return error code: 0x%x\n"      \
                   "  file : %s, line : %d\n"                               \
                   "  expr : %s\n", ret, __FILE__, __LINE__, #expr);        \
        }                                                                   \
    } while (0)


typedef enum
{
    HDMI_STATUS_OK = 0,

    HDMI_STATUS_NOT_IMPLEMENTED,
    HDMI_STATUS_ALREADY_SET,
    HDMI_STATUS_ERROR,
} HDMI_STATUS;

typedef enum
{
    SMART_BOOK_DISCONNECTED = 0,
    SMART_BOOK_CONNECTED,
} SMART_BOOK_STATE;

typedef enum
{
    HDMI_POWER_STATE_OFF = 0,
    HDMI_POWER_STATE_ON,
    HDMI_POWER_STATE_STANDBY,
} HDMI_POWER_STATE;

typedef struct
{

} HDMI_CAPABILITY;


typedef enum
{
    HDMI_TO_TV = 0x0,
    HDMI_TO_SMB,
} hdmi_device_type;

typedef enum
{
    HDMI_IS_DISCONNECTED = 0,
    HDMI_IS_CONNECTED = 1,
    HDMI_IS_RES_CHG = 0x11,
} hdmi_connect_status;

#define MAKE_MTK_HDMI_FORMAT_ID(id, bpp)  (((id) << 8) | (bpp))

typedef enum
{
    MTK_HDMI_FORMAT_UNKNOWN = 0,

    MTK_HDMI_FORMAT_RGB565   = MAKE_MTK_HDMI_FORMAT_ID(1, 2),
    MTK_HDMI_FORMAT_RGB888   = MAKE_MTK_HDMI_FORMAT_ID(2, 3),
    MTK_HDMI_FORMAT_BGR888   = MAKE_MTK_HDMI_FORMAT_ID(3, 3),
    MTK_HDMI_FORMAT_ARGB8888 = MAKE_MTK_HDMI_FORMAT_ID(4, 4),
    MTK_HDMI_FORMAT_ABGR8888 = MAKE_MTK_HDMI_FORMAT_ID(5, 4),
    MTK_HDMI_FORMAT_YUV422   = MAKE_MTK_HDMI_FORMAT_ID(6, 2),
    MTK_HDMI_FORMAT_XRGB8888 = MAKE_MTK_HDMI_FORMAT_ID(7, 4),
    MTK_HDMI_FORMAT_XBGR8888 = MAKE_MTK_HDMI_FORMAT_ID(8, 4),
    MTK_HDMI_FORMAT_BPP_MASK = 0xFF,
} MTK_HDMI_FORMAT;

typedef struct
{
    bool is_audio_enabled;
    bool is_video_enabled;
} hdmi_device_status;

typedef struct
{
    void *src_base_addr;
    void *src_phy_addr;
    int src_fmt;
    unsigned int  src_pitch;
    unsigned int  src_offset_x, src_offset_y;
    unsigned int  src_width, src_height;

    int next_buff_idx;
    int identity;
    int connected_type;
    unsigned int security;

} hdmi_video_buffer_info;


typedef struct mtk_hdmi_info
{
    unsigned int display_id;
    unsigned int isHwVsyncAvailable;
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int displayFormat;
    unsigned int vsyncFPS;
    unsigned int xDPI;
    unsigned int yDPI;
    unsigned int isConnected;
} mtk_hdmi_info_t;


typedef struct
{
    //  Input
    int ion_fd;
    // Output
    unsigned int index; //fence count
    int fence_fd;   //fence fd
} hdmi_buffer_info;

#define MTK_HDMI_NO_FENCE_FD        ((int)(-1)) //((int)(~0U>>1))
#define MTK_HDMI_NO_ION_FD        ((int)(-1))   //((int)(~0U>>1))

extern unsigned int mtkfb_get_fb_phys_addr(void);
extern unsigned int mtkfb_get_fb_size(void);
extern unsigned int mtkfb_get_fb_va(void);

#define HDMI_IOW(num, dtype)     _IOW('H', num, dtype)
#define HDMI_IOR(num, dtype)     _IOR('H', num, dtype)
#define HDMI_IOWR(num, dtype)    _IOWR('H', num, dtype)
#define HDMI_IO(num)             _IO('H', num)

#define MTK_HDMI_AUDIO_VIDEO_ENABLE         HDMI_IO(1)
#define MTK_HDMI_AUDIO_ENABLE                           HDMI_IO(2)
#define MTK_HDMI_VIDEO_ENABLE                           HDMI_IO(3)
#define MTK_HDMI_GET_CAPABILITY                     HDMI_IOWR(4, HDMI_CAPABILITY)
#define MTK_HDMI_GET_DEVICE_STATUS              HDMI_IOWR(5, hdmi_device_status)
#define MTK_HDMI_VIDEO_CONFIG                           HDMI_IOWR(6, int)
#define MTK_HDMI_AUDIO_CONFIG                           HDMI_IOWR(7, int)
#define MTK_HDMI_FORCE_FULLSCREEN_ON        HDMI_IOWR(8, int)
#define MTK_HDMI_FORCE_FULLSCREEN_OFF   HDMI_IOWR(9, int)
#define MTK_HDMI_IPO_POWEROFF                       HDMI_IOWR(10, int)
#define MTK_HDMI_IPO_POWERON                            HDMI_IOWR(11, int)
#define MTK_HDMI_POWER_ENABLE                       HDMI_IOW(12, int)
#define MTK_HDMI_PORTRAIT_ENABLE                    HDMI_IOW(13, int)
#define MTK_HDMI_FORCE_OPEN                         HDMI_IOWR(14, int)
#define MTK_HDMI_FORCE_CLOSE                            HDMI_IOWR(15, int)
#define MTK_HDMI_IS_FORCE_AWAKE                 HDMI_IOWR(16, int)
#define MTK_HDMI_ENTER_VIDEO_MODE               HDMI_IO(17)
#define MTK_HDMI_LEAVE_VIDEO_MODE               HDMI_IO(18)
#define MTK_HDMI_REGISTER_VIDEO_BUFFER          HDMI_IOW(19, hdmi_video_buffer_info)
#define MTK_HDMI_POST_VIDEO_BUFFER              HDMI_IOW(20,  hdmi_video_buffer_info)
#define MTK_HDMI_FACTORY_MODE_ENABLE            HDMI_IOW(21, int)


struct ext_memory_info
{
    unsigned int buffer_num;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
};

struct ext_buffer
{
    unsigned int id;
    unsigned int ts_sec;
    unsigned int ts_nsec;
};

#define MTK_EXT_DISPLAY_ENTER                                   HDMI_IO(40)
#define MTK_EXT_DISPLAY_LEAVE                                   HDMI_IO(41)
#define MTK_EXT_DISPLAY_START                                   HDMI_IO(42)
#define MTK_EXT_DISPLAY_STOP                                    HDMI_IO(43)
#define MTK_EXT_DISPLAY_SET_MEMORY_INFO                     HDMI_IOW(44, struct ext_memory_info)
#define MTK_EXT_DISPLAY_GET_MEMORY_INFO                     HDMI_IOW(45, struct ext_memory_info)
#define MTK_EXT_DISPLAY_GET_BUFFER                          HDMI_IOW(46, struct ext_buffer)
#define MTK_EXT_DISPLAY_FREE_BUFFER                         HDMI_IOW(47, struct ext_buffer)

#define MTK_HDMI_GET_DEV_INFO                                HDMI_IOWR(50, mtk_dispif_info_t)
#define MTK_HDMI_PREPARE_BUFFER                              HDMI_IOW(52, hdmi_buffer_info)
#define MTK_HDMI_SCREEN_CAPTURE                              HDMI_IOW(53, unsigned long)
#define MTK_HDMI_USBOTG_STATUS                              HDMI_IOWR(80, int)
#define MTK_HDMI_GET_DRM_ENABLE                              HDMI_IOWR(81, int)

#define MTK_HDMI_FACTORY_GET_STATUS                           HDMI_IOWR(82, int)
#define MTK_HDMI_FACTORY_DPI_TEST                             HDMI_IOWR(83, int)


enum HDMI_report_state
{
    NO_DEVICE = 0,
    HDMI_PLUGIN = 1,
};

typedef enum
{
    HDMI_CHARGE_CURRENT,

} HDMI_QUERY_TYPE;

int get_hdmi_dev_info(HDMI_QUERY_TYPE type);
bool is_hdmi_enable(void);
void hdmi_setorientation(int orientation);
void hdmi_suspend(void);
void hdmi_resume(void);
void hdmi_power_on(void);
void hdmi_power_off(void);
void hdmi_update_buffer_switch(void);
void hdmi_dpi_config_clock(void);
void hdmi_dpi_power_switch(bool enable);
int hdmi_audio_config(int samplerate);
int hdmi_video_enable(bool enable);
int hdmi_audio_enable(bool enable);
int hdmi_audio_delay_mute(int latency);
void hdmi_set_mode(unsigned char ucMode);
void hdmi_reg_dump(void);

void hdmi_read_reg(unsigned char u8Reg);
void hdmi_write_reg(unsigned char u8Reg, unsigned char u8Data);
#ifdef MTK_SMARTBOOK_SUPPORT
void smartbook_state_callback();
#endif
#endif
