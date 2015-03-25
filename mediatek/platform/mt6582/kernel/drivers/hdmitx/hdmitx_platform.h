#ifndef     _HDMITX_PLATFORM_H_
#define     _HDMITX_PLATFORM_H_
#include <mach/m4u.h>

void hdmi_read_reg_platform(unsigned char u8Reg, unsigned int *p4Data);
void deliver_driver_interface(HDMI_DRIVER *hdmi_drv);
void hdmi_m4u_port_config(M4U_PORT_STRUCT *m4u_port);
int check_edid_header();
void hdmi_config_main_disp();
int hdmi_ioctl_platform(unsigned int cmd, unsigned long arg);

#endif
