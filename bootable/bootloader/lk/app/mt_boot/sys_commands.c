#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <arch/ops.h>

#include <target.h>
#include <platform.h>

#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_rtc.h>
#include <platform/bootimg.h>
#include <mt_partition.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/mt_disp_drv.h>
#include "fastboot.h"
extern struct fastboot_var *varlist;
extern struct fastboot_cmd *cmdlist;
extern int boot_linux_from_storage(void);
extern const char* mt_disp_get_lcm_id(void);
extern void boot_linux(void *kernel, unsigned *tags,char *cmdline, unsigned machtype,void *ramdisk, unsigned ramdisk_size);

#define MODULE_NAME "FASTBOOT"
#define MAX_RSP_SIZE 64


extern BOOTMODE g_boot_mode;
extern char g_CMDLINE [200];

int has_set_p2u = 0; //0-unset; 1-on; 2-off

void cmd_overwirte_cmdline(const char *arg, void *data, unsigned sz)
{
  const char *new_kernel_cmd_line = arg;
  int cmd_len;

  cmd_len = strlen(new_kernel_cmd_line);

  if(cmd_len > 200)
  {
    printf("[FASTBOOT] Input cmdline length is too long!");
    fastboot_fail("cmdline length is too long");
  }

  strcpy(g_CMDLINE, new_kernel_cmd_line);

  printf("[FASTBOOT] New command line is %s\n", g_CMDLINE);
  fastboot_okay("");
}



void cmd_oem_append_cmdline(const char *arg, void *data, unsigned sz)
{
  int cmd_len;
  char new_kernel_cmd_line[200];
  printf("APPEND KERNEL CMDLINE\n");
  fastboot_info("APPEND KERNEL CMDLINE\n");
  sprintf(new_kernel_cmd_line, "%s%s", g_CMDLINE,arg);
  cmd_len = strlen(new_kernel_cmd_line);
  if(cmd_len > 200)
  {
    printf("[FASTBOOT] Input cmdline length is too long!");
    fastboot_fail("cmdline length is too long");
    return;
  }
  strcpy(g_CMDLINE, new_kernel_cmd_line);
  printf("[FASTBOOT] New command line:%s\n", g_CMDLINE);
  fastboot_okay("");
  return;
}


void set_p2u(int mode)
{
  if(mode == 1)
  {
    if(strlen(g_CMDLINE) + strlen(" printk.disable_uart=1") + 1 > sizeof(g_CMDLINE)) 
    {
      printf("command line is too long, will not set printk_on\n");
      fastboot_info("command line is too long, will not set printk_on");
    }
    else
    {
      sprintf(g_CMDLINE, "%s %s", g_CMDLINE, "printk.disable_uart=0");
      fastboot_info("printk to uart is on!");
    }
  }
  else if(mode == 2)
  {
    if(strlen(g_CMDLINE) + strlen(" printk.disable_uart=1") + 1 > sizeof(g_CMDLINE))
    {
      printf("command line is too long, will not set printk_off\n");
      fastboot_info("command line is too long, will not set printk_off");
    } 
    else
    {
      sprintf(g_CMDLINE, "%s %s", g_CMDLINE, "printk.disable_uart=1");
      fastboot_info("printk to uart is off!");
    }
  }
}

void cmd_continue(const char *arg, void *data, unsigned sz)
{
  g_boot_mode = NORMAL_BOOT;
  if(has_set_p2u) 
  {
    set_p2u(has_set_p2u);
    fastboot_info("phone will continue boot up after 5s...");
    fastboot_okay("");
    mdelay(5000);
  }
  else
  {
    fastboot_okay("");
  }
  udc_stop();
  mtk_wdt_init(); //re-open wdt
  /*Will not return*/
  boot_linux_from_storage();
}

void cmd_oem_p2u(const char *arg, void *data, unsigned sz)
{
  if(!strncmp(arg, " on", strlen(" on")))
  {
    has_set_p2u = 1;
  }
  else if(!strncmp(arg, " off", strlen(" off")))
  {
    has_set_p2u = 2;
  }
  else
  {
    has_set_p2u = 0;
    fastboot_info("unknown argument");
  }
  fastboot_okay("");
}

void cmd_oem_reboot2recovery(const char *arg, void *data, unsigned sz)
{
   extern void Set_RTC_Recovery_Mode(bool flag)__attribute__((weak));

   if(Set_RTC_Recovery_Mode)
   {
       Set_RTC_Recovery_Mode(1);
       fastboot_okay("");
       mtk_arch_reset(1); //bypass pwr key when reboot
   }
   else
   {
       fastboot_fail("Not support this function (need RTC porting)");
   }
}
void cmd_getvar(const char *arg, void *data, unsigned sz)
{
	struct fastboot_var *var;
	char response[MAX_RSP_SIZE];

	if(!strcmp(arg, "all")){
		for (var = varlist; var; var = var->next){
			snprintf(response, MAX_RSP_SIZE,"\t%s: %s", var->name, var->value);
			fastboot_info(response);
		}
		fastboot_okay("Done!!");
		return;
	}
	for (var = varlist; var; var = var->next) {
		if (!strcmp(var->name, arg)) {
			fastboot_okay(var->value);
			return;
		}
	}
	fastboot_okay("");
}

void cmd_reboot(const char *arg, void *data, unsigned sz)
{
  dprintf(INFO, "rebooting the device\n");
  fastboot_okay("");
  mtk_arch_reset(1); //bypass pwr key when reboot
}

void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz)
{
  dprintf(INFO, "rebooting the device to bootloader\n");
  fastboot_okay("");
  Set_Clr_RTC_PDN1_bit13(true); //Set RTC fastboot bit
  mtk_arch_reset(1); //bypass pwr key when reboot
}

extern char g_CMDLINE [200];
extern unsigned int g_kmem_off;
extern unsigned int g_rmem_off;
extern unsigned int g_bimg_sz;
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
void cmd_boot(const char *arg, void *data, unsigned sz)
{
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	static struct boot_img_hdr hdr;
	char *ptr = ((char*) data);
   unsigned page_size = 0;
   unsigned page_mask = 0;
   int strlen = 0;

	if (sz < sizeof(hdr)) {
		fastboot_fail("invalid bootimage header");
		return;
	}

	memcpy(&hdr, data, sizeof(hdr));

  printf("\n============================================================\n");
	hdr.magic[7] = '\0';
  printf("[%s] Android Boot IMG Hdr - Magic 	        : %s\n",MODULE_NAME,hdr.magic);
  printf("[%s] Android Boot IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,hdr.kernel_size);
  printf("[%s] Android Boot IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,hdr.ramdisk_size);
  printf("[%s] Android Boot IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,hdr.page_size);
  printf("============================================================\n");

	/* ensure commandline is terminated */
	hdr.cmdline[BOOT_ARGS_SIZE-1] = 0;

	if(hdr.page_size) {
		page_size = hdr.page_size;
		page_mask = page_size - 1;
		//page_mask = 2*1024 ; /*FIXME*/
	}
   else
   {
     printf("[FASTBOOT] Please specify the storage page-size in the boot header!\n");
     fastboot_fail("Please specify the storage page-size in the boot header!\n");
     return;
   }

	kernel_actual = ROUND_TO_PAGE(hdr.kernel_size, page_mask);
	ramdisk_actual = ROUND_TO_PAGE(hdr.ramdisk_size, page_mask);

	/* sz should have atleast raw boot image */
	if (page_size + kernel_actual + ramdisk_actual > sz) {
		fastboot_fail("incomplete bootimage");
		return;
	}

	memmove((void*) hdr.kernel_addr, (ptr + MKIMG_HEADER_SZ + BIMG_HEADER_SZ), hdr.kernel_size);
	memmove((void*) hdr.ramdisk_addr, (ptr + MKIMG_HEADER_SZ + BIMG_HEADER_SZ + kernel_actual), hdr.ramdisk_size);

  //strlen += sprintf((void *) hdr.cmdline, "%s lcm=%1d-%s", hdr.cmdline, DISP_IsLcmFound(),(char *) mt_disp_get_lcm_id());
  //strlen += sprintf((void *) hdr.cmdline, "%s fps=%1d", hdr.cmdline, mt_disp_get_lcd_time());

	fastboot_okay("");
	udc_stop();
   mtk_wdt_init(); //re-open wdt

  g_boot_mode = NORMAL_BOOT;

	boot_linux((void*) hdr.kernel_addr, (void*) hdr.tags_addr,
		   (char*) hdr.cmdline, board_machtype(),
		   (void*) hdr.ramdisk_addr, hdr.ramdisk_size);
#if 0
  unsigned kernel_actual;
  unsigned ramdisk_actual;
  struct boot_img_hdr boot_hdr;
  unsigned int k_pg_cnt = 0;
  unsigned int r_pg_cnt = 0;
  unsigned int b_pg_cnt = 0;
  unsigned int size_b = 0;
  unsigned int pg_sz = 2*1024 ;
  int strlen = 0;

  /*copy hdr data from download_base*/
  memcpy(&boot_hdr, data, sizeof(boot_hdr));

  /* ensure commandline is terminated */
  boot_hdr.cmdline[BOOT_ARGS_SIZE-1] = 0;

  printf("\n============================================================\n");
	boot_hdr.magic[7] = '\0';
  printf("[%s] Android Boot IMG Hdr - Magic 	        : %s\n",MODULE_NAME,boot_hdr.magic);
  printf("[%s] Android Boot IMG Hdr - Kernel Size 	: 0x%x\n",MODULE_NAME,boot_hdr.kernel_size);
  printf("[%s] Android Boot IMG Hdr - Rootfs Size 	: 0x%x\n",MODULE_NAME,boot_hdr.ramdisk_size);
  printf("[%s] Android Boot IMG Hdr - Page Size    	: 0x%x\n",MODULE_NAME,boot_hdr.page_size);
  printf("============================================================\n");

  //***************
  //* check partition magic
  //*
  if (strncmp(boot_hdr.magic,BOOT_MAGIC, sizeof(BOOT_MAGIC))!=0) {
    printf("[%s] boot image header magic error\n", MODULE_NAME);
    return -1;
  }

  g_kmem_off =  (unsigned int)target_get_scratch_address();
  g_kmem_off = g_kmem_off + MKIMG_HEADER_SZ + BIMG_HEADER_SZ;


  if(boot_hdr.kernel_size % pg_sz == 0)
  {
    k_pg_cnt = boot_hdr.kernel_size / pg_sz;
  }
  else
  {
    k_pg_cnt = (boot_hdr.kernel_size / pg_sz) + 1;
  }

  if(boot_hdr.ramdisk_size % pg_sz == 0)
  {
    r_pg_cnt = boot_hdr.ramdisk_size / pg_sz;
  }
  else
  {
    r_pg_cnt = (boot_hdr.ramdisk_size / pg_sz) + 1;
  }

  printf(" > page count of kernel image = %d\n",k_pg_cnt);
  g_rmem_off = g_kmem_off + k_pg_cnt * pg_sz;

  printf(" > kernel mem offset = 0x%x\n",g_kmem_off);
  printf(" > rootfs mem offset = 0x%x\n",g_rmem_off);

  //***************
  //* specify boot image size
  //*
  g_bimg_sz = (k_pg_cnt + r_pg_cnt + 2)* pg_sz;
  printf(" > boot image size = 0x%x\n",g_bimg_sz);

  memmove((void*)CFG_BOOTIMG_LOAD_ADDR , g_kmem_off, boot_hdr.kernel_size);
  memmove((void*)CFG_RAMDISK_LOAD_ADDR , g_rmem_off, boot_hdr.ramdisk_size);

  //custom_port_in_kernel(g_boot_mode, commanline);
  //strlen += sprintf(commanline, "%s lcm=%1d-%s", commanline, DISP_IsLcmFound(), mt_disp_get_lcm_id());
  //strlen += sprintf(commanline, "%s fps=%1d", commanline, mt_disp_get_lcd_time());

  fastboot_okay("");

  udc_stop();

  mtk_wdt_init();
  boot_linux((void *)CFG_BOOTIMG_LOAD_ADDR, (unsigned *)CFG_BOOTARGS_ADDR,
		   (const char*) boot_hdr.cmdline, board_machtype(),
		   (void *)CFG_RAMDISK_LOAD_ADDR, boot_hdr.ramdisk_size);
#endif
}

