/* MTK Proprietary Unlock File */

#include <platform/mt_typedefs.h>
#include <platform/sec_status.h>
#include <platform/mt_reg_base.h>
#include <platform/env.h>
#include "sec_unlock.h"
#include "fastboot.h"
#include <target/cust_key.h>
#include <platform/mt_gpt.h>
#include <video.h>
#include <string.h>
#include <debug.h>
#include <platform/mtk_key.h>
#include <printf.h>

#define UNLOCK_KEY_SIZE 32
#define SERIAL_NUMBER_SIZE 16
#define DEFAULT_SERIAL_NUM "0123456789ABCDEF"

char fb_unlock_key_str[UNLOCK_KEY_SIZE+1] = {0};

extern u32 get_devinfo_with_index(u32 index);
extern int get_serial(u64 hwkey, u32 chipid, char ser[38]);
extern void sec_get_serial_number_from_unlock_key(u8 *unlock_key, u32 unlock_key_len, u8 *cal_serial, u32 cal_serial_len);
extern int sec_set_device_lock(int do_lock);
extern int sec_query_device_lock(int *lock_state);

static int fastboot_data_part_wipe()
{
    int ret = B_OK;
    int err;

    int index;
    unsigned long long ptn; 
    unsigned long long size; 

    index = partition_get_index("userdata");

    if (index == -1 || !is_support_erase(index))
    {
        ret = PART_GET_INDEX_FAIL;
        return ret;
    }    

    ptn = partition_get_offset(index);
    size = partition_get_size(index);

    set_env("unlock_erase", "start");

    err = emmc_erase(ptn, size);
    if (err)
    {    
        ret = PART_ERASE_FAIL;
        set_env("unlock_erase", "fail");
    } else
    {
        ret = B_OK;
        set_env("unlock_erase", "pass");
    }

    return ret;
}

void fastboot_boot_menu(void)
{
          const char* title_msg = "Select Boot Mode:\n[VOLUME_UP to select.  VOLUME_DOWN is OK.]\n\n";
          video_clean_screen();
          video_set_cursor(video_get_rows()/2, 0);
          video_printf(title_msg);
          video_printf("[Recovery    Mode]             \n");
#ifdef MTK_FASTBOOT_SUPPORT
          video_printf("[Fastboot    Mode]         <<==\n");
#endif
          video_printf("[Normal      Boot]             \n");
#ifndef USER_BUILD
          video_printf("[Normal      Boot +ftrace]    \n");
          video_printf("[Normal      slub debug off]     \n");
#endif
          video_printf(" => FASTBOOT mode...\n");
}
void unlock_warranty(void)
{
  const char* title_msg = "Unlock bootloader?\n\n";
  video_clean_screen();
  video_set_cursor(video_get_rows()/2, 0);
  video_printf(title_msg);
  video_printf("If you unlock the bootloader,you will be able to install custom operating\n");
  video_printf("system software on this phone.\n\n");

  video_printf("A custom OS is not subject to the same testing as the original OS, and can\n");
  video_printf("cause your phone and installed applications to stop working properly.\n\n");

  video_printf("To prevent unauthorized access to your personal data,unlocking the bootloader\n");
  video_printf("will also delete all personal data from your phone(a \"factory data reset\").\n\n");

  video_printf("Press the Volume UP/Down buttons to select Yes or No. \n\n");
  video_printf("Yes (Volume UP):Unlock(may void warranty).\n\n");
  video_printf("No (Volume Down):Do not unlock bootloader.\n\n");

}

void lock_warranty(void)
{
  const char* title_msg = "lock bootloader?\n\n";
  video_clean_screen();
  video_set_cursor(video_get_rows()/2, 0);
  video_printf(title_msg);
  video_printf("If you lock the bootloader,you will need to install official operating\n");
  video_printf("system software on this phone.\n\n");


  video_printf("To prevent unauthorized access to your personal data,locking the bootloader\n");
  video_printf("will also delete all personal data from your phone(a \"factory data reset\").\n\n");

  video_printf("Press the Volume UP/Down buttons to select Yes or No. \n\n");
  video_printf("Yes (Volume UP):Lock bootloader.\n\n");
  video_printf("No (Volume Down):Do not lock bootloader.\n\n");
}

void fastboot_oem_key(const char *arg, void *data, unsigned sz)
{
  int key_length;
  key_length = strlen(arg+1);
  if(key_length != UNLOCK_KEY_SIZE)
  {
    fastboot_fail("argument size is wrong\n");
  }
  else
  {
    strcpy(fb_unlock_key_str,arg+1);
    dprintf(INFO,"key is '%s' and length is %d\n",fb_unlock_key_str,key_length);
    fastboot_okay("");
  }

  return;
}

void fastboot_oem_query_lock_state(const char *arg, void *data, unsigned sz)
{
    #define LKS_RESP_MAX_SIZE 64
    int ret = B_OK;
    int lock_state;
    char msg[LKS_RESP_MAX_SIZE] = {0};
    
    ret = sec_query_device_lock(&lock_state);
    if (ret != B_OK) {
	snprintf(msg, LKS_RESP_MAX_SIZE, "cannot get lks (ret = 0x%x)", ret);
	msg[LKS_RESP_MAX_SIZE - 1] = '\0'; /* prevent msg from not being ended with '\0'*/
	fastboot_fail(msg);
    }
    else {
	snprintf(msg, LKS_RESP_MAX_SIZE, "lks = %d", lock_state);
	msg[LKS_RESP_MAX_SIZE - 1] = '\0'; /* prevent msg from not being ended with '\0'*/
	fastboot_info(msg);
	fastboot_okay("");
    }
}

int fastboot_oem_unlock(const char *arg, void *data, unsigned sz)
{
    int ret = B_OK;
    char msg[128] = {0};
    unlock_warranty();
    while(1)
    {
       if(mtk_detect_key(MT65XX_MENU_SELECT_KEY)) //VOL_UP
       {
         fastboot_info("Start unlock flow\n");
         //Invoke security check after confiming "yes" by user
         ret = fastboot_oem_unlock_chk();
         if(ret != B_OK)
         {
            sprintf(msg, "\nUnlock failed - Err:0x%x \n", ret);
            video_printf("Unlock failed...return to fastboot in 3s\n");
            mdelay(3000);
            fastboot_boot_menu();
            fastboot_fail(msg);
         }
         else
         {
            video_printf("Unlock Pass...return to fastboot in 3s\n");
            mdelay(3000);
            fastboot_boot_menu();
            fastboot_okay("");
         }
         break;
       }
       else if(mtk_detect_key(MT65XX_MENU_OK_KEY))//VOL_DOWN
       {
         video_printf("return to fastboot in 3s\n");
         mdelay(3000);
         fastboot_boot_menu();
         fastboot_okay("");
         break;
       }
       else
       {
           //If we press other keys, discard it.
       }
    }
    return ret;

}

int fastboot_oem_unlock_chk()
{
   int ret = B_OK;
    u32 chip_code = 0x0;
    char serial_number[SERIAL_NUMBER_SIZE+1] = {0};
    u64 key;
    char cal_serial_number[SERIAL_NUMBER_SIZE+1] = {0};
    lock_warranty();
    /* Check for the unlock key */
    if(UNLOCK_KEY_SIZE != strlen(fb_unlock_key_str))
    {
        //fastboot_fail("Unlock key length is incorrect!");
        ret = ERR_UNLOCK_KEY_WRONG_LENGTH;
        goto _wrong_key_length;
    }

    /* Get the device serial number */
    key = get_devinfo_with_index(13);
    key = (key << 32) | get_devinfo_with_index(12);
    chip_code = DRV_Reg32(APHW_CODE);
    if (key != 0)
	get_serial(key, chip_code, serial_number);
    else
	memcpy(serial_number, DEFAULT_SERIAL_NUM, SERIAL_NUMBER_SIZE);

    /* Calculate the serial number from the unlock key */
    sec_get_serial_number_from_unlock_key((u8 *)fb_unlock_key_str, UNLOCK_KEY_SIZE, (u8 *)cal_serial_number, SERIAL_NUMBER_SIZE);

    /* Compare the results */
    if(0 != memcmp(serial_number, cal_serial_number, SERIAL_NUMBER_SIZE))
    {
        //fastboot_fail("Unlock key code is incorrect!");
        ret = ERR_UNLOCK_WRONG_KEY_CODE;
        goto _wrong_key_code;
    }

    /* Do format operation of data partition */
    if(B_OK != (ret = fastboot_data_part_wipe()))
    {
        //fastboot_fail("Data partition wipe failed!");
        goto _erase_data_fail;
    }

    /* Set unlock done flag */
    ret = sec_set_device_lock(0);

_erase_data_fail:
_wrong_key_code:
_wrong_key_length:
    return ret;
}


int fastboot_oem_lock(const char *arg, void *data, unsigned sz)
{
    int ret = B_OK;
    char msg[128] = {0};
    lock_warranty();
    while(1)
    {
       if(mtk_detect_key(MT65XX_MENU_SELECT_KEY)) //VOL_UP
       {
         fastboot_info("Start lock flow\n");
         //Invoke security check after confiming "yes" by user
         ret = fastboot_oem_lock_chk();
         if(ret != B_OK)
         {
            sprintf(msg, "\nLock failed - Err:0x%x \n", ret);
            video_printf("Lock failed...return to fastboot in 3s\n");
            mdelay(3000);
            fastboot_boot_menu();
            fastboot_fail(msg);
         }
         else
         {
             video_printf("Lock Pass...return to fastboot in 3s\n");
             mdelay(3000);
             fastboot_boot_menu();
             fastboot_okay("");
         }
         break;
       }
       else if(mtk_detect_key(MT65XX_MENU_OK_KEY))//VOL_DOWN
       {
         video_printf("return to fastboot in 3s\n");
         mdelay(3000);
         fastboot_boot_menu();
         fastboot_okay("");
         break;
       }
       else
       {
         //If we press other keys, discard it.
       }
    }
      return ret;
}

int fastboot_oem_lock_chk()
{
    #define TRY_LOCK 1
    int ret = B_OK;
    
    if(B_OK != (ret = sec_boot_check(TRY_LOCK)))
    {
	goto _image_verify_fail;
    }

    /* Do format operation of data partition */
    if(B_OK != (ret = fastboot_data_part_wipe()))
    {
        //fastboot_fail("Data partition wipe failed!");
        goto _erase_data_fail;
    }

    /* Set lock done flag */
    ret = sec_set_device_lock(1);
    
_image_verify_fail:
_erase_data_fail:
    return ret;
}

