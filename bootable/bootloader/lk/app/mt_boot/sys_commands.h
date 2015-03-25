#ifndef __COMMANDS_H
#define __COMMANDS_H

void cmd_getvar(const char *arg, void *data, unsigned sz);
void cmd_boot(const char *arg, void *data, unsigned sz);
void cmd_reboot(const char *arg, void *data, unsigned sz);
void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz);
void cmd_download(const char *arg, void *data, unsigned sz);
void cmd_overwirte_cmdline(const char *arg, void *data, unsigned sz);
void cmd_continue(const char *arg, void *data, unsigned sz);
void cmd_oem_p2u(const char *arg, void *data, unsigned sz);
void cmd_oem_reboot2recovery(const char *arg, void *data, unsigned sz);
void cmd_oem_append_cmdline(const char *arg, void *data, unsigned sz);
#endif
