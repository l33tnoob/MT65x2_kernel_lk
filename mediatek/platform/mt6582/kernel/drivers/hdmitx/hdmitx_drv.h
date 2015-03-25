#ifndef     HDMITX_DRV_H
#define     HDMITX_DRV_H

#define HDMI_CHECK_RET(expr)                                                \
    do {                                                                    \
        HDMI_STATUS ret = (expr);                                           \
        if (HDMI_STATUS_OK != ret) {                                        \
            printk("[ERROR][mtkfb] HDMI API return error code: 0x%x\n"      \
                   "  file : %s, line : %d\n"                               \
                   "  expr : %s\n", ret, __FILE__, __LINE__, #expr);        \
        }                                                                   \
    } while (0)


extern unsigned int mtkfb_get_fb_phys_addr(void);
extern unsigned int mtkfb_get_fb_size(void);
extern unsigned int mtkfb_get_fb_va(void);

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
#define MTK_EXT_DISPLAY_ENTER                   HDMI_IO(40)
#define MTK_EXT_DISPLAY_LEAVE                   HDMI_IO(41)
#define MTK_EXT_DISPLAY_START                   HDMI_IO(42)
#define MTK_EXT_DISPLAY_STOP                    HDMI_IO(43)
#define MTK_EXT_DISPLAY_SET_MEMORY_INFO         HDMI_IOW(44, struct ext_memory_info)
#define MTK_EXT_DISPLAY_GET_MEMORY_INFO         HDMI_IOW(45, struct ext_memory_info)
#define MTK_EXT_DISPLAY_GET_BUFFER              HDMI_IOW(46, struct ext_buffer)
#define MTK_EXT_DISPLAY_FREE_BUFFER             HDMI_IOW(47, struct ext_buffer)



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
void hdmi_update(void);
void hdmi_dpi_power_switch(bool enable);
int hdmi_audio_config(int samplerate);
int hdmi_video_enable(bool enable);
int hdmi_audio_enable(bool enable);
int hdmi_audio_delay_mute(int latency);
void hdmi_set_mode(unsigned char ucMode);
void hdmi_reg_dump(void);

void hdmi_read_reg(unsigned char u8Reg, unsigned int *p4Data);
void hdmi_write_reg(unsigned char u8Reg, unsigned char u8Data);
#ifdef MTK_SMARTBOOK_SUPPORT
void smartbook_state_callback();
#endif
#endif
