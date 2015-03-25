#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <arch/ops.h>
#include <ctype.h>

#include <target.h>
#include <platform.h>

#include <platform/mt_reg_base.h>
#include <platform/boot_mode.h>
#include <mt_partition.h>
#include <platform/mt_disp_drv.h>
#include <platform/env.h>
#include <target/cust_usb.h>
#include <platform/mt_gpt.h>

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
#include "platform/mtk_wdt.h"
extern int kernel_charging_boot(void);
extern int pmic_detect_powerkey(void);
extern void mt6575_power_off(void);
extern void mt65xx_backlight_off(void);
#endif
extern u32 memory_size(void);
extern unsigned *target_atag_devinfo_data(unsigned *ptr);
extern unsigned *target_atag_videolfb(unsigned *ptr);
extern unsigned *target_atag_mdinfo(unsigned *ptr);
extern void platform_uninit(void);
extern int mboot_android_load_bootimg_hdr(char *part_name, unsigned long addr);
extern int mboot_android_load_bootimg(char *part_name, unsigned long addr);
extern int mboot_android_load_recoveryimg_hdr(char *part_name, unsigned long addr);
extern int mboot_android_load_recoveryimg(char *part_name, unsigned long addr);
extern int mboot_android_load_factoryimg_hdr(char *part_name, unsigned long addr);
extern int mboot_android_load_factoryimg(char *part_name, unsigned long addr);
extern void custom_port_in_kernel(BOOTMODE boot_mode, char *command);
extern const char* mt_disp_get_lcm_id(void);
extern int fastboot_init(void *base, unsigned size);
extern int sec_func_init(int dev_type);
extern int sec_boot_check (int try_lock);
#ifdef DEVICE_TREE_SUPPORT
#include <libfdt.h>
extern BI_DRAM bi_dram[MAX_NR_BANK];
extern unsigned int *device_tree, device_tree_size;
#endif

char g_CMDLINE [200] = COMMANDLINE_TO_KERNEL;

/* Please define SN_BUF_LEN in cust_usb.h */
#ifndef SN_BUF_LEN
#define SN_BUF_LEN	13	/* fastboot use 13 bytes as default, max is 19 */
#endif

#define DEFAULT_SERIAL_NUM "0123456789ABCDEF"

/*
 * Support read barcode from /dev/pro_info to be serial number.
 * Then pass the serial number from cmdline to kernel.
 */
/* #define SERIAL_NUM_FROM_BARCODE */

#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL) || (defined(MTK_SECURITY_SW_SUPPORT) && defined(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT))
#define SERIALNO_LEN	38	/* from preloader */
char sn_buf[SN_BUF_LEN] = "";	/* will read from EFUSE_CTR_BASE */
#else
char sn_buf[SN_BUF_LEN] = FASTBOOT_DEVNAME;
#endif

static struct udc_device surf_udc_device = {
	.vendor_id	= USB_VENDORID,
	.product_id	= USB_PRODUCTID,
	.version_id	= USB_VERSIONID,
	.manufacturer	= USB_MANUFACTURER,
	.product	= USB_PRODUCT_NAME,
};

void msg_header_error(char *img_name)
{
	printf ("[MBOOT] Load '%s' partition Error\n", img_name);
	printf("\n*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	printf("> If you use NAND boot\n");
	printf("> (1) %s is wrong !!!! \n", img_name);
	printf("> (2) please make sure the image you've downloaded is correct\n");
	printf("\n> If you use MSDC boot\n");
	printf("> (1) %s is not founded in SD card !!!! \n",img_name);
	printf("> (2) please make sure the image is put in SD card\n");
	printf("*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	while(1);
}

void msg_img_error(char *img_name)
{
	printf ("[MBOOT] Load '%s' partition Error\n", img_name);
	printf("\n*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	printf("> Please check kernel and rootfs in %s are both correct.\n",img_name);
	printf("*******************************************************\n");
	printf("ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR.ERROR\n");
	printf("*******************************************************\n");
	while(1);
}

//*********
//* Notice : it's kernel start addr (and not include any debug header)
extern unsigned int g_kmem_off;

//*********
//* Notice : it's rootfs start addr (and not include any debug header)
extern unsigned int g_rmem_off;
extern unsigned int g_rimg_sz;
extern int g_nr_bank;
extern int g_rank_size[4];
extern unsigned int boot_time;
extern BOOT_ARGUMENT *g_boot_arg;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
extern bool g_boot_reason_change;
#endif
extern int has_set_p2u;

static void check_hibernation(char *cmdline)
{
    int hibboot = 0;

    hibboot = get_env("hibboot") == NULL ? 0 : atoi(get_env("hibboot"));

    if (g_boot_mode == RECOVERY_BOOT)
        goto SKIP_HIB_BOOT;
    if (g_boot_mode == FACTORY_BOOT)
        goto SKIP_HIB_BOOT;
    if (g_boot_mode == ALARM_BOOT)
        goto SKIP_HIB_BOOT;
#if 0 // let schedule power on to go hiberantion bootup process
    if (g_boot_arg->boot_reason == BR_RTC)
        goto SKIP_HIB_BOOT;
#endif
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    if (g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT)
        goto SKIP_HIB_BOOT;
#endif

    if(get_env("resume") != NULL) {
        if (1 == hibboot) {
            sprintf(cmdline,"%s%s%s",cmdline," resume=", get_env("resume"));
            //sprintf(cmdline, "%s%s", cmdline, " no_console_suspend");
        } else if (0 != hibboot)
            printf("resume = %s but hibboot = %s\n", get_env("resume"), get_env("hibboot"));
    } else {
        printf("resume = NULL \n");
    }

    return;

SKIP_HIB_BOOT:
    if (hibboot != 0)
        if (set_env("hibboot", "0") != 0)
            printf("lk_env hibboot set failed!!!\n");
    if (get_env("resume") != NULL)
        if (set_env("resume", '\0') != 0)
            printf("lk_evn resume set resume failed!!!\n");
}

#ifdef DEVICE_TREE_SUPPORT
int boot_linux_fdt(void *kernel, unsigned *tags,
		char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
{
    void *fdt = tags;
    int ret;
    int offset;
    unsigned int mem_reg_propsize = 4;
    unsigned int mem_reg_property[mem_reg_propsize];

    int i;
    void (*entry)(unsigned,unsigned,unsigned*) = kernel;
    unsigned int lk_t = 0;
    unsigned int pl_t = 0;
    unsigned int boot_reason = 0;		
    char buf[1024], *ptr;


    memcpy(fdt, (void *)&device_tree, device_tree_size);

    ret = fdt_open_into(fdt, fdt, 0x4000 - 0x0100);
    if (ret) return FALSE;
    ret = fdt_check_header(fdt);
    if (ret) return FALSE;

    mem_reg_property[0] = cpu_to_fdt32(0);
    mem_reg_property[1] = cpu_to_fdt32(bi_dram[0].start);
    mem_reg_property[2] = cpu_to_fdt32(0);
    mem_reg_property[3] = cpu_to_fdt32(memory_size());

    offset = fdt_path_offset(fdt, "/memory");
    ret = fdt_setprop(fdt, offset, "reg", mem_reg_property, mem_reg_propsize * sizeof(unsigned int));
    if (ret) return FALSE;

    offset = fdt_path_offset(fdt, "/chosen");
    ret = fdt_setprop_cell(fdt, offset, "linux,initrd-start",(unsigned int) ramdisk);
    if (ret) return FALSE;
    ret = fdt_setprop_cell(fdt, offset, "linux,initrd-end", (unsigned int)ramdisk + ramdisk_size);
    if (ret) return FALSE;

    ptr = (char *)target_atag_boot((unsigned *)buf);
    ret = fdt_setprop(fdt, offset, "atag,boot", buf, ptr - buf);
    if (ret) return FALSE;

    ptr = (char *)target_atag_mem((unsigned *)buf);
    ret = fdt_setprop(fdt, offset, "atag,mem", buf, ptr - buf);
    if (ret) return FALSE;

    if(g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT || g_boot_mode == ATE_FACTORY_BOOT || g_boot_mode == FACTORY_BOOT)
    {
      ptr = (char *)target_atag_meta((unsigned *)buf);
      ret = fdt_setprop(fdt, offset, "atag,meta", buf, ptr - buf);
      if (ret) return FALSE;
    }
    ptr = (char *)target_atag_devinfo_data((unsigned *)buf);
    ret = fdt_setprop(fdt, offset, "atag,devinfo", buf, ptr - buf);
    if (ret) return FALSE;

    ptr = (char *)target_atag_videolfb((unsigned *)buf);
    ret = fdt_setprop(fdt, offset, "atag,videolfb", buf, ptr - buf);
    if (ret) return FALSE;

    ptr = (char *)target_atag_mdinfo((unsigned *)buf);
    ret = fdt_setprop(fdt, offset, "atag,mdinfo", buf, ptr - buf);
    if (ret) return FALSE;

    if (!has_set_p2u) {
#ifdef USER_BUILD
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=1");
#else
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=0");
#endif 

    /*Append pre-loader boot time to kernel command line*/
    pl_t = g_boot_arg->boot_time;
    sprintf(cmdline, "%s%s%d", cmdline, " bootprof.pl_t=", pl_t);
    /*Append lk boot time to kernel command line*/
    lk_t = ((unsigned int)get_timer(boot_time));
    sprintf(cmdline, "%s%s%d", cmdline, " bootprof.lk_t=", lk_t);

#ifdef LK_PROFILING
    printf("[PROFILE] ------- boot_time takes %d ms -------- \n", lk_t);
#endif
    }
    /*Append pre-loader boot reason to kernel command line*/
#ifdef MTK_KERNEL_POWER_OFF_CHARGING	
    if (g_boot_reason_change) {
      boot_reason = 4;
    }
    else
#endif 
    {
      boot_reason = g_boot_arg->boot_reason;
    }
    sprintf(cmdline, "%s%s%d", cmdline, " boot_reason=", boot_reason);

    check_hibernation(cmdline);

    ptr = (char *)target_atag_commmandline((unsigned *)buf, cmdline);
    ret = fdt_setprop(fdt, offset, "atag,cmdline", buf, ptr - buf);
    if (ret) return FALSE;

    ret = fdt_setprop_string(fdt, offset, "bootargs", cmdline);
    if (ret) return FALSE;

    ret = fdt_pack(fdt);
    if (ret) return FALSE;

    printf("booting linux @ %p, ramdisk @ %p (%d)\n",
		kernel, ramdisk, ramdisk_size);  

    enter_critical_section();
    /* do any platform specific cleanup before kernel entry */
    platform_uninit();
#ifdef HAVE_CACHE_PL310	
    l2_disable();
#endif

    arch_disable_cache(UCACHE);
    arch_disable_mmu();    

    
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	/*Prevent the system jumps to Kernel if we unplugged Charger/USB before*/
	if(kernel_charging_boot() == -1)
	{
		printf("[%s] Unplugged Usb/Charger in Kernel Charging Mode Before Jumping to Kernel, Power Off\n", __func__);
#ifndef NO_POWER_OFF
		mt6575_power_off();
#endif

	}
	if(kernel_charging_boot() == 1)
	{
		if(pmic_detect_powerkey())
		{
			printf("[%s] PowerKey Pressed in Kernel Charging Mode Before Jumping to Kernel, Reboot Os\n", __func__);
			mt65xx_backlight_off();
			mt_disp_power(0);
			mtk_arch_reset(1);
		}
	}
#endif
    printf("DRAM Rank :%d\n", g_nr_bank);
    for(i = 0; i < g_nr_bank; i++) {
         printf("DRAM Rank[%d] Size = 0x%x\n", i, g_rank_size[i]);
    }
    printf("cmdline: %s\n", cmdline);
    printf("lk boot time = %d ms\n", lk_t);
    printf("lk boot mode = %d\n", g_boot_mode);
    printf("lk finished --> jump to linux kernel\n\n");
    entry(0, machtype, tags);
    return 0;
}
#endif // DEVICE_TREE_SUPPORT

void boot_linux(void *kernel, unsigned *tags,
		char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
{
    int i;
    unsigned *ptr = tags;
    void (*entry)(unsigned,unsigned,unsigned*) = kernel;
    unsigned int lk_t = 0;
    unsigned int pl_t = 0;
    unsigned int boot_reason = 0;

#ifdef DEVICE_TREE_SUPPORT
    boot_linux_fdt((void *)kernel, (unsigned *)tags,
        (char *)cmdline, machtype,
        (void *)ramdisk, ramdisk_size);
#endif

	/* CORE */
    *ptr++ = 2;
    *ptr++ = 0x54410001;

    ptr = target_atag_boot(ptr);
    ptr = target_atag_mem(ptr);

    //some platform might not have this function, use weak reference for 
    extern unsigned *target_atag_dfo(unsigned *ptr)__attribute__((weak));
    if(target_atag_dfo)
    {
        ptr = target_atag_dfo(ptr);
    }

    if(g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT || g_boot_mode == ATE_FACTORY_BOOT || g_boot_mode == FACTORY_BOOT)
    {
      ptr = target_atag_meta(ptr);
    }

    ptr = target_atag_devinfo_data(ptr);

    /*Append pre-loader boot time to kernel command line*/
    pl_t = g_boot_arg->boot_time;
    sprintf(cmdline, "%s%s%d", cmdline, " bootprof.pl_t=", pl_t);

    /*Append lk boot time to kernel command line*/
    lk_t = ((unsigned int)get_timer(boot_time));
    sprintf(cmdline, "%s%s%d", cmdline, " bootprof.lk_t=", lk_t);
#ifdef LK_PROFILING
    printf("[PROFILE] ------- boot_time takes %d ms -------- \n", lk_t);
#endif
    if (!has_set_p2u) {
#ifdef USER_BUILD
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=1");
#else
        sprintf(cmdline,"%s%s",cmdline," printk.disable_uart=0");
#endif
    }
	/*Append pre-loader boot reason to kernel command line*/
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if (g_boot_reason_change) {
		boot_reason = 4;
	}
	else
#endif
	{
		boot_reason = g_boot_arg->boot_reason;
	}
    sprintf(cmdline, "%s%s%d", cmdline, " boot_reason=", boot_reason);

#ifdef SERIAL_NUM_FROM_BARCODE
    /* Append androidboot.serialno=xxxxyyyyzzzz in cmdline */
    sprintf(cmdline, "%s%s%s", cmdline, " androidboot.serialno=", sn_buf);
#endif

    check_hibernation(cmdline);
    ptr = target_atag_commmandline(ptr, cmdline);
    ptr = target_atag_initrd(ptr,(unsigned long) ramdisk, ramdisk_size);
    ptr = target_atag_videolfb(ptr);

    extern unsigned int *target_atag_mdinfo(unsigned *ptr)__attribute__((weak));
    if(target_atag_mdinfo)
    {
      ptr = target_atag_mdinfo(ptr);
    }
    else
    {
      printf("DFO_MODEN_INFO Only support in MT6582/MT6592\n");      
    }

	/* END */
    *ptr++ = 0;
    *ptr++ = 0;

    printf("booting linux @ %p, ramdisk @ %p (%d)\n",
		kernel, ramdisk, ramdisk_size);

    enter_critical_section();
    /* do any platform specific cleanup before kernel entry */
    platform_uninit();
#ifdef HAVE_CACHE_PL310
    l2_disable();
#endif

    arch_disable_cache(UCACHE);
    arch_disable_mmu();


#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	/*Prevent the system jumps to Kernel if we unplugged Charger/USB before*/
	if(kernel_charging_boot() == -1)
	{
		printf("[%s] Unplugged Usb/Charger in Kernel Charging Mode Before Jumping to Kernel, Power Off\n", __func__);
#ifndef NO_POWER_OFF
		mt6575_power_off();
#endif

	}
	if(kernel_charging_boot() == 1)
	{
		if(pmic_detect_powerkey())
		{
			printf("[%s] PowerKey Pressed in Kernel Charging Mode Before Jumping to Kernel, Reboot Os\n", __func__);
			mt65xx_backlight_off();
			mt_disp_power(0);
			mtk_arch_reset(1);
		}
	}
#endif
    printf("DRAM Rank :%d\n", g_nr_bank);
    for(i = 0; i < g_nr_bank; i++) {
         printf("DRAM Rank[%d] Size = 0x%x\n", i, g_rank_size[i]);
    }
    printf("cmdline: %s\n", cmdline);
    printf("lk boot time = %d ms\n", lk_t);
    printf("lk boot mode = %d\n", g_boot_mode);
    printf("lk finished --> jump to linux kernel\n\n");
    entry(0, machtype, tags);
}

int boot_linux_from_storage(void)
{
    int ret=0;
    char *commanline = g_CMDLINE;
    int strlen=0;
#ifdef LK_PROFILING
    unsigned int time_load_recovery=0;
    unsigned int time_load_bootimg=0;
    unsigned int time_load_factory=0;
    time_load_recovery = get_timer(0);
    time_load_bootimg = get_timer(0);
#endif

#if 1

    switch(g_boot_mode)
    {
        case NORMAL_BOOT:
        case META_BOOT:
        case ADVMETA_BOOT:
        case SW_REBOOT:
        case ALARM_BOOT:
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
		case KERNEL_POWER_OFF_CHARGING_BOOT:
		case LOW_POWER_OFF_CHARGING_BOOT:
#endif
#if defined(CFG_NAND_BOOT)
            strlen += sprintf(commandline, "%s%s%x%s%x",
                commandline, NAND_MANF_CMDLINE, nand_flash_man_code, NAND_DEV_CMDLINE, nand_flash_dev_id);
#endif
            ret = mboot_android_load_bootimg_hdr(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_header_error("Android Boot Image");
            }

            ret = mboot_android_load_bootimg(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_img_error("Android Boot Image");
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load boot.img takes %d ms -------- \n", get_timer(time_load_bootimg));
#endif
        break;

        case RECOVERY_BOOT:
            ret = mboot_android_load_recoveryimg_hdr(PART_RECOVERY, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_header_error("Android Recovery Image");
            }

            ret = mboot_android_load_recoveryimg(PART_RECOVERY, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                msg_img_error("Android Recovery Image");
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load recovery.img takes %d ms -------- \n", get_timer(time_load_recovery));
#endif
        break;

        case FACTORY_BOOT:
        case ATE_FACTORY_BOOT:
#if defined(CFG_NAND_BOOT)
            strlen += sprintf(commandline, "%s%s%x%s%x",
                commandline, NAND_MANF_CMDLINE, nand_flash_man_code, NAND_DEV_CMDLINE, nand_flash_dev_id);
#endif
            ret = mboot_android_load_factoryimg_hdr(CFG_FACTORY_NAME, CFG_BOOTIMG_LOAD_ADDR);
            if (ret < 0) {
                printf("factory image doesn't exist in SD card\n");

                ret = mboot_android_load_bootimg_hdr(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);
                if (ret < 0) {
                    msg_header_error("Android Boot Image");
                }

                ret = mboot_android_load_bootimg(PART_BOOTIMG, CFG_BOOTIMG_LOAD_ADDR);
                if (ret < 0) {
                    msg_img_error("Android Boot Image");
	        }
            } else {
                ret = mboot_android_load_factoryimg(CFG_FACTORY_NAME, CFG_BOOTIMG_LOAD_ADDR);
                if (ret < 0) {
                    msg_img_error("Android Factory Image");
                }
            }
#ifdef LK_PROFILING
            printf("[PROFILE] ------- load factory.img takes %d ms -------- \n", get_timer(time_load_factory));
#endif
         break;

       case FASTBOOT:
       case DOWNLOAD_BOOT:
       case UNKNOWN_BOOT:
          break;
             
    }


    /* relocate rootfs (ignore rootfs header) */
    memcpy((char *)CFG_RAMDISK_LOAD_ADDR, (char *)(g_rmem_off), g_rimg_sz);
    g_rmem_off = CFG_RAMDISK_LOAD_ADDR;

#endif

    // 2 weak function for mt6572 memory preserved mode
    platform_mem_preserved_load_img();
    platform_mem_preserved_dump_mem();

    custom_port_in_kernel(g_boot_mode, commanline);

#ifndef MACH_FPGA
//FIXME, Waiting LCM Driver owner to fix it
    strlen += sprintf(commanline, "%s lcm=%1d-%s", commanline, DISP_IsLcmFound(), mt_disp_get_lcm_id());
    strlen += sprintf(commanline, "%s fps=%1d", commanline, mt_disp_get_lcd_time());
#endif

#ifdef SELINUX_STATUS
	#if SELINUX_STATUS == 1
		sprintf(commanline, "%s androidboot.selinux=disabled", commanline);
	#elif SELINUX_STATUS == 2
		sprintf(commanline, "%s androidboot.selinux=permissive", commanline);
	#endif
#endif

    boot_linux((void *)CFG_BOOTIMG_LOAD_ADDR, (unsigned *)CFG_BOOTARGS_ADDR,
		   (char *)commanline, board_machtype(),
		   (void *)CFG_RAMDISK_LOAD_ADDR, g_rimg_sz);


     return 0;
}

#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL) || (defined(MTK_SECURITY_SW_SUPPORT) && defined(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT))
static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWSYZ456789"};
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int get_serial(u64 hwkey, u32 chipid, char ser[SERIALNO_LEN])
{
	u16 hashkey[4];
	int idx, ser_idx;
	u32 digit, id;
	u64 tmp = hwkey;

	memset(ser, 0x00, SERIALNO_LEN);

	/* split to 4 key with 16-bit width each */
	tmp = hwkey;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		hashkey[idx] = (u16)(tmp & 0xffff);
		tmp >>= 16;
	}

	/* hash the key with chip id */
	id = chipid;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		digit = (id % 10);
		hashkey[idx] = (hashkey[idx] >> digit) | (hashkey[idx] << (16-digit));
		id = (id / 10);
	}

	/* generate serail using hashkey */
	ser_idx = 0;
	for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
		ser[ser_idx++] = (hashkey[idx] & 0x001f);
		ser[ser_idx++] = (hashkey[idx] & 0x00f8) >> 3;
		ser[ser_idx++] = (hashkey[idx] & 0x1f00) >> 8;
		ser[ser_idx++] = (hashkey[idx] & 0xf800) >> 11;
	}
	for (idx = 0; idx < ser_idx; idx++)
		ser[idx] = udc_chr[(int)ser[idx]];
	ser[ser_idx] = 0x00;
	return 0;
}
#endif /* CONFIG_MTK_USB_UNIQUE_SERIAL */

#ifdef SERIAL_NUM_FROM_BARCODE
static inline int read_product_info(char *buf)
{
	int tmp = 0;

	mboot_recovery_load_raw_part("PRO_INFO", buf, SN_BUF_LEN);

	for( ; tmp < SN_BUF_LEN ; tmp++) {
		if( buf[tmp] == 0 && tmp > 0) {
			break;
		} else if( !isalpha(buf[tmp]) && !isdigit(buf[tmp]))
			return 0;
	}
	return 1;
}
#endif

void mt_boot_init(const struct app_descriptor *app)
{
	unsigned usb_init = 0;
	unsigned sz = 0;
#ifdef SERIAL_NUM_FROM_BARCODE
	char tmp[SN_BUF_LEN] = {0};
#endif
#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
	u64 key;
	u32 chip_code;
	u8 serial_num[SERIALNO_LEN];
#endif

#ifdef CONFIG_MTK_USB_UNIQUE_SERIAL
	/* Please enable EFUSE clock in platform.c before reading sn key */

	/* serial string adding */

	key = readl(SERIAL_KEY_HI);
	key = (key << 32) | readl(SERIAL_KEY_LO);
	chip_code = DRV_Reg32(APHW_CODE);

	if (key != 0)
		get_serial(key, chip_code, serial_num);
	else
		memcpy(serial_num, DEFAULT_SERIAL_NUM, SN_BUF_LEN);
	/* copy serial from serial_num to sn_buf */
	memcpy(sn_buf, serial_num, SN_BUF_LEN);
#endif

#ifdef SERIAL_NUM_FROM_BARCODE
	if(!read_product_info(tmp)) {
		strncpy(tmp, DEFAULT_SERIAL_NUM, SN_BUF_LEN);
	}

	strncpy(sn_buf, tmp, sizeof(sn_buf)-1);
#endif
	sn_buf[SN_BUF_LEN-1] = '\0';
	surf_udc_device.serialno = sn_buf;

#ifdef MTK_SECURITY_SW_SUPPORT    

#ifdef MTK_EMMC_SUPPORT
    #ifdef MTK_NEW_COMBO_EMMC_SUPPORT
        sec_func_init(3);
    #else
    sec_func_init(1);
    #endif
#else
    sec_func_init(0);
#endif

#endif

	if (g_boot_mode == FASTBOOT)
		goto fastboot;

#ifdef MTK_SECURITY_SW_SUPPORT    
    /* Do not block fastboot if check failed */
    if(0 != sec_boot_check(0))
    {
        printf("<ASSERT> %s:line %d\n",__FILE__,__LINE__);
        while(1);
    }
#endif

	/* Will not return */
	boot_linux_from_storage();

fastboot:
	target_fastboot_init();
	if(!usb_init)
		/*Hong-Rong: wait for porting*/
		udc_init(&surf_udc_device);

	mt_part_dump();
/*test*/
#if 0

{
	char buf[2048];
	char buf_t[2048];
	int i;
	part_t *part;
	part_dev_t *dev = mt_part_get_device();
	u64 start_addr;
	bool ret = true;
	memset(buf,0x00,2048);
	memset(buf_t,0x00,2048);

	part = mt_part_get_partition(PART_LOGO);
	start_addr = (u64)part->startblk*BLK_SIZE;
	printf("---partition test -----%s %llx\n",PART_LOGO,start_addr);


	dev->write(dev,buf,start_addr,2048);
	for(i=0;i<2048;i++){
		buf[i]= i;
	}
	dev->write(dev,buf,start_addr+4,1024);
	dev->read(dev,start_addr+4,buf_t,1024);

	for(i=0;i<1024;i++){
		if(buf[i]!=buf_t[i]){
			printf("compare error. s=%x,d=%x\n",buf[i],buf_t[i]);
			ret = false;
		}

	}
	if(ret == true){
		printf("---partition test sucess\n-----");
	}


}
#endif
/*test*/
	sz = target_get_max_flash_size();
	fastboot_init(target_get_scratch_address(), sz);
	udc_start();

}


APP_START(mt_boot)
	.init = mt_boot_init,
APP_END
