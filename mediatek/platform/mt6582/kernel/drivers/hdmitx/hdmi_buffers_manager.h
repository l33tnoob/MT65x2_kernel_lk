#ifndef _HDMI_BUFFERS_MANAGER_H_
#define _HDMI_BUFFERS_MANAGER_H_

void hdmi_sync_init();
void hdmi_sync_destroy();
void hdmi_buffer_list_init();
bool hdmi_buffer_list_empty();
void hdmi_buffer_to_RDMA();
void hdmi_buffer_state_update();
void hdmi_remove_buffers();
int hdmi_insert_buffer(hdmi_buffer_info *buffer_info);
int hdmi_post_buffer(hdmi_video_buffer_info *buffer_info, bool clock_on);
void hdmi_mmp_debug_enable(int mmp_debug_level);

#endif
