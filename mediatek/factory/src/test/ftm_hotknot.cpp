/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <linux/input.h>

#include <common.h>
#include <miniui.h>
#include <ftm.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <string.h> 

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
#define BYTE u8


#define HOTKNOT_MASTER_END      0
#define HOTKNOT_SLAVE_END       1
#define ERR_ENTER_MASTER        0x8001
#define ERR_ENTER_SLAVE         0x8002
#define ERR_EXIT_TEST           0x8004
#define ERR_LOAD_HOTKNOT_FW     0x8008
#define ERR_ENTER_XFER          0x8010
#define ERR_MASTER_SEND         0x8020
#define ERR_MASTER_RECEIVE      0x8040
#define ERR_PATTERN_NOT_MATCH   0x8080
#define ERR_SLAVE_RECEIVE       0x8100
#define ERR_SLAVE_SEND          0x8200
#define ERR_SWITCH_LCD          0x8400
#define ERR_OTHER               0x8800


#define SUBSYSTEM_LINK          0x00
#define SUBSYSTEM_AUTHORIZATION 0x01
#define SUBSYSTEM_RECOVERY      0x02
extern s32 write_node(u8*, s32 );
extern s32 read_node(u8* , s32 );
extern s32 read_register(u8*,u16,u16);
extern s32 write_register(u8*,u16,u16);
extern s32 auto_set_node();
extern s32 guitar_reset();
extern bool load_subsystem(u8 subsystem);
extern bool check_crc(u8* ,u16 ,u8 );
extern u8 calculate_check_sum(u8* ,u8 );

extern bool read_pair_state(void);
extern bool clear_pair_buf(void);
extern bool enter_slave_mode(void);
extern bool enter_master_mode(void);
extern bool enter_transfer_mode(void);
extern bool exit_slave_mode(void);
extern bool exit_master_mode(void);
extern bool exit_transfer_mode(void);
extern bool send_data(u8* buf, s32 length,  s32 timeout);
extern s32 receive_data(u8* buf, s32 length,  s32 timeout);
extern s32 query_version_internal(void);
extern bool enable_vendor(void);
extern bool disable_vendor(void);
extern bool block_read_pair_state();
extern bool block_send_data(u8* buf, s32 length);
extern s32 block_receive_data(u8* buf);
extern bool read_depart_state(u8 cOrs, int timeout);
extern bool enable_common(void);
extern bool disable_common(void);

#ifdef __cplusplus
extern "C" {
#endif


#define TAG	"[HOTKNOT] "

//#undef 
#if 1
#define DEBUG(fmt, arg...)  LOGD(TAG fmt"\n", ##arg)
#else
#define DEBUG(fmt, arg...) ALOGD
#endif

u8 test_pattern[] = 
{
    0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03,
    0x55, 0x54, 0x55, 0x55, 0x55, 0x56, 0x55, 0x57,
    0xAA, 0xA8, 0xAA, 0xA9, 0xAA, 0xAA, 0xAA, 0xAB,
    0xFF, 0xFC, 0xFF, 0xFD, 0xFF, 0xFE, 0xFF, 0xFF,
    
    0x1B, 0x1B, 0x05, 0xAF, 0xE4, 0xE4, 0xFA, 0x50,
    0x00, 0x55, 0x55, 0xAA, 0xAA, 0xFF, 0xFF, 0x00,
    0x1B, 0x1B, 0x05, 0xAF, 0xE4, 0xE4, 0xFA, 0x50,
    0x00, 0x55, 0x55, 0xAA, 0xAA, 0xFF, 0xFF, 0x00
};

bool test_running = false;

static int
write_int(char const* path, int value)
{
	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDWR);
	if (fd >= 0) {
		char buffer[20];
		int bytes = sprintf(buffer, "%d\n", value);
		int amt = write(fd, buffer, bytes);
		close(fd);
		return amt == -1 ? -errno : 0;
	}

	DEBUG("write_int failed to open %s\n", path);
	return -errno;
}

static bool switch_lcd(bool on)
{
    const char *NODE_PATH = "/sys/class/leds/lcd-backlight/brightness";
    s32 ret = -1;

    if(on)
    	ret = write_int(NODE_PATH, 102);
    else
    	ret = write_int(NODE_PATH, 0);	
    
    if(ret < 0)
    {
    	DEBUG("[hotknot factory]Switch lcd fail :%s.", strerror(errno));
    	return false;
    }
    
    DEBUG("[hotknot factory]Switch lcd success.");
    return true;
}

s32 factory_test(u8 cOrs, s32 timeout)
{
    bool bret = false;
    s32 ret = 0;
    test_running = true;
    u8 rcv_data[sizeof(test_pattern)];
    s32 len = sizeof(test_pattern);

	
    enable_common();
    clear_pair_buf();
    
    switch(cOrs)
    {
        case HOTKNOT_MASTER_END:
            DEBUG("[hotknot1 factory]Step1:Enter master mode.");
            bret = enter_master_mode();
            if(!bret)
            {
                DEBUG("[hotknot factory]Enter master mode failed.");
                
                ret = ERR_ENTER_MASTER;
                goto proc;
            }
            
            DEBUG("[hotknot factory]Step2:Waiting master paired.");
            while(test_running && (timeout-- > 0))
            {
                bret = read_pair_state();
                if(bret)
                {
                    DEBUG("[hotknot factory]Master paired success.");
                    break;
                }
                usleep(50*1000);
            }
            exit_master_mode();
            if(!test_running || (timeout <= 0))
            {
                DEBUG("[hotknot factory]Force exit testing.");
                ret = ERR_EXIT_TEST;
                goto proc;
            }
            
            DEBUG("[hotknot factory]Step3:Close LCD.");
            bret = switch_lcd(false);
            if(!bret)
            {
                DEBUG("[hotknot factory]Step3:Close lcd failed.");
                ret = ERR_SWITCH_LCD;
                goto proc;
            }
            #if 0
            DEBUG("[hotknot factory]Step4:Load hotknot firmware.");
            //bret = load_subsystem(SUBSYSTEM_LINK);
            if(!bret)
            {
                DEBUG("[hotknot factory]Load hotknot subsystem failed.");
                ret = ERR_LOAD_HOTKNOT_FW;
                goto master_test_exit;
            }
            #endif
            DEBUG("[hotknot factory]Step5:Enter transfer mode.");
            bret = enter_transfer_mode();
            if(!bret)
            {
                DEBUG("[hotknot factory]Enter transfer mode failed.");
                ret = ERR_ENTER_XFER;
                goto master_test_exit;
            }
            
            DEBUG("[hotknot factory]Step6:Master send test pattern.");
            bret = send_data(test_pattern, sizeof(test_pattern), -1);
            if(!bret)
            {
                DEBUG("[hotknot factory]Master send test pattern failed.");
                ret = ERR_MASTER_SEND;
                goto master_test_recovery_exit;
            }
            
            DEBUG("[hotknot factory]Step7:Master receive response pattern");
            ret = receive_data(rcv_data,len, -1);
            if(ret <= 0)
            {
                DEBUG("[hotknot factory]Master receive response pattern failed.");
                ret = ERR_MASTER_RECEIVE;
                goto master_test_recovery_exit;
            }
            
            if(memcmp(rcv_data, test_pattern, sizeof(test_pattern)))
            {
                DEBUG("[hotknot factory]Master receive pattern match failed.");
                ret = ERR_PATTERN_NOT_MATCH;
                goto master_test_recovery_exit;
            }
            ret = 0;
            DEBUG("[hotknot factory]Master Test success.");
        master_test_recovery_exit:
            load_subsystem(SUBSYSTEM_RECOVERY);
        master_test_exit:
            switch_lcd(true);
        proc:
        	  disable_common();
            return ret;
        //break;
        case HOTKNOT_SLAVE_END:
            DEBUG("[hotknot factory]Step1:Enter slave mode.");
            bret = enter_slave_mode();
            if(!bret)
            {
                DEBUG("[hotknot factory]Enter slave mode failed.");
                ret = ERR_ENTER_SLAVE;
                goto proc1;
            }
            
            DEBUG("[hotknot factory]Step2:Waiting slave paired.");
            while(test_running)
            {
                bret = read_pair_state();
                if(bret)
                {
                    DEBUG("[hotknot factory]Slave paired success.");
                    break;
                }
                usleep(50*1000);
            }
            //exit_master_mode();
            if(!test_running)
            {
                DEBUG("[hotknot factory]Force exit testing.");
                ret = ERR_EXIT_TEST;
                goto proc1;
            }
            
            DEBUG("[hotknot factory]Step3:Close LCD.");
            bret = switch_lcd(false);
            if(!bret)
            {
                DEBUG("[hotknot factory]Step3:Close lcd failed.");
                ret = ERR_SWITCH_LCD;
                goto proc1;
            }
            #if 0
            DEBUG("[hotknot factory]Step4:Load hotknot firmware.");
            bret = load_subsystem(SUBSYSTEM_LINK);
            if(!bret)
            {
                DEBUG("[hotknot factory]Load hotknot subsystem failed.");
                ret = ERR_LOAD_HOTKNOT_FW;
                goto slave_test_exit;
            }
			#endif
            
            DEBUG("[hotknot factory]Step5:Enter transfer mode.");
            bret = enter_transfer_mode();
            if(!bret)
            {
                DEBUG("[hotknot factory]Enter transfer mode failed.");
                ret = ERR_ENTER_XFER;
                goto slave_test_exit;
            }
                        
            DEBUG("[hotknot factory]Step6:Slave receive pattern.");
            ret = receive_data(rcv_data, len,-1);
            if(ret <= 0)
            {
                DEBUG("[hotknot factory]Slave receive pattern failed.");
                ret = ERR_SLAVE_RECEIVE;
                goto slave_test_recovery_exit;
            }
            
            DEBUG("[hotknot factory]Step7:Slave send response pattern.");
            bret = send_data(rcv_data, ret, -1);
            if(!bret)
            {
                DEBUG("[hotknot factory]Slave send response pattern failed.");
                ret = ERR_SLAVE_SEND;
                goto slave_test_recovery_exit;
            }
            ret = 0;
            DEBUG("[hotknot factory]Slave Test sucess");
        slave_test_recovery_exit:
            load_subsystem(SUBSYSTEM_RECOVERY);
        slave_test_exit:
            switch_lcd(true);
        proc1:
        	  disable_common();    
            return ret;
        //break;
        default:
            DEBUG("[hotknot factory]Invalid test parameter!");
        break;
    }
    DEBUG("[hotknot factory] end!");
    return -1;
}

void exit_factory_test()
{
    DEBUG("[hotknot factory]Exit factory test.");
    test_running = false;
    load_subsystem(SUBSYSTEM_RECOVERY);
    switch_lcd(true);
}

#if 0
int main()
{
	int mode = -1, timeout;
test_begin:
	DEBUG("Select test mode: 0--Master,1--Slave.\n");
	scanf("%d", &mode);
	if(mode == HOTKNOT_MASTER_END)
	{
    	DEBUG("Input timeout count(time = count*50ms).\n");
    	scanf("%d", &timeout);	
	    DEBUG("Mode:MASTER, timeout:%d (x50ms)\n", timeout);
	    factory_test(mode, timeout);
	}
	else if(mode == HOTKNOT_SLAVE_END)
	{
	    DEBUG("Mode:SLAVE\n");
	    while(1)
        { 
            factory_test(mode, 0);
        }
	}
	else
	{
	    DEBUG("Invalid test mode,please select again.\n");
	    goto test_begin;
	}
 
	return 0;
}
#endif

//#ifdef FEATURE_FTM_KEYS
#if 1 //hotknot compile options


static int kpd_fd = 0;

enum {
	ITEM_PASS,
	ITEM_FAIL
};

/*static item_t keys_item[] = {
	item(ITEM_PASS,   "Test Pass"),
	item(ITEM_FAIL,   "Test Fail"),
	item(-1, NULL),
};*/

enum {
    ITEM_HOTKNOT_MASTER = 0,
    ITEM_HOTKNOT_SLAVE,
    ITEM_HOTKNOT_PASS,
    ITEM_HOTKNOT_FAIL,
};

static item_t hotknot_items[] = {
    {ITEM_HOTKNOT_MASTER,   uistr_info_hotknot_master, 0},
    {ITEM_HOTKNOT_SLAVE,    uistr_info_hotknot_slave, 0},
    {ITEM_HOTKNOT_PASS,        uistr_info_hotknot_pass, 0},
    {ITEM_HOTKNOT_FAIL,        uistr_info_hotknot_fail, 0},
    {-1, NULL, 0},

	
};


#define INFO_SIZE (1024)
struct hotknot {
	struct ftm_module *mod;
    char         info[INFO_SIZE];
    char        *mntpnt;
    bool         exit_thd;

    text_t title;
    text_t text;
    
    pthread_t update_thd;
	struct itemview *iv;

	struct textview tv;
	//struct itemview *iv;
};

#define mod_to_hotknot(p)	(struct hotknot *)((char *)(p) + sizeof(struct ftm_module))

static int TestItem_hotknot = -1;
static void *hotknot_handler(void *priv)
{
    int i, pend, k = 0;
	struct hotknot *keys = (struct hotknot *)priv;
	struct itemview *iv = keys->iv;
    int count = 1, chkcnt = 10;
    int init_status, ret = -1;

	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
	//init NFC driver
	memset(keys->info, '\n', INFO_SIZE);
	sprintf(keys->info, "%s\n",uistr_info_hotknot_mode_select);
	iv->redraw(iv);
	sleep(1);
	init_status = 0;
	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
	if(init_status != 0)	// nfc init fail
	{
		//memset(nfc->info, '\n', INFO_SIZE);
		//sprintf(nfc->info, "NFC failed! (%d)\n", init_status);
		//sprintf(nfc->info, "%s,%s\n", uistr_nfc,uistr_info_test_fail);
		//iv->redraw(iv);
		DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
	}
	else
	{
		int exitThread = 0;
		//memset(keys->info, '\n', INFO_SIZE);
		//iv->redraw(iv);
		DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
		while (!exitThread) 
		{
			usleep(100000); // wake up every 0.1sec	
			DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
			if( k++ == 10)
			{
				DEBUG("[%s]TestItem_hotknot = %d\n", __FUNCTION__, TestItem_hotknot);
				k = 0;
			}
			if(TestItem_hotknot == ITEM_HOTKNOT_MASTER)
			{
			   memset(keys->info, '\n', INFO_SIZE);
			   sprintf(keys->info, "%s\n",uistr_info_hotknot_master);
			   iv->redraw(iv);	
			   //timeout 10s: 10*1000/50
			   DEBUG("Master mode test\n");
			   ret = factory_test(HOTKNOT_MASTER_END, 10*1000/50);
			   if(ret == 0)
			   {
				   memset(keys->info, '\n', INFO_SIZE);
				   sprintf(keys->info, "%s\n",uistr_info_hotknot_pass);
				   iv->redraw(iv);	
				   
				   DEBUG("Master mode pass\n");			       
				   keys->mod->test_result = FTM_TEST_PASS;
				   TestItem_hotknot = -1;
				   //break;
			   }
               else
			   {
				   memset(keys->info, '\n', INFO_SIZE);
				   sprintf(keys->info, "%s\n",uistr_info_hotknot_fail);
				   iv->redraw(iv);	
				   DEBUG("Master mode fail\n");
				   keys->mod->test_result = FTM_TEST_FAIL;
				   TestItem_hotknot = -1;
				   //break;
			   }
			}

			if(TestItem_hotknot == ITEM_HOTKNOT_SLAVE)
			{
			   DEBUG("Slave mode test\n");
			   ret = factory_test(HOTKNOT_SLAVE_END, -1);
			   if(ret == 0)
			   {
				   memset(keys->info, '\n', INFO_SIZE);
				   sprintf(keys->info, "%s\n","Enter Slave mode OK");
				   iv->redraw(iv);	
				   DEBUG("Slave mode pass\n");
				   //keys->mod->test_result = FTM_TEST_PASS;
				   //break;
			   }
               else
			   {
				   memset(keys->info, '\n', INFO_SIZE);
				   sprintf(keys->info, "%s\n","Enter Slave mode fail");
				   iv->redraw(iv);	
				   DEBUG("Slave mode fail\n");
				   //keys->mod->test_result = FTM_TEST_FAIL;  
				   //break;
			   }
			   
			}

			if (keys->exit_thd)
            {
                //LOGD(TAG "%s, nfc->exit_thd = true,exitThread,%d\n", __FUNCTION__,exitThread);
                DEBUG("%s, nfc->exit_thd = true,exitThread,%d\n", __FUNCTION__,exitThread);
                exitThread = 1;
                break;
            }
		}
	}	

	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    pthread_exit(NULL); // thread exit
    return NULL;

}

static int hotknot_entry(struct ftm_param *param, void *priv)
{
	int i, err, num = 0;
	int x = 0, y = CHAR_HEIGHT;
	bool exit=false;
	static char buf[128];
	struct hotknot *keys = (struct hotknot *)priv;
	struct itemview *iv = NULL;
	int chosen = -1;


	err = 0;
	
    DEBUG("%s, %d\n", __FUNCTION__, __LINE__);


	init_text(&keys->title, param->name, COLOR_YELLOW);
    init_text(&keys->text, &keys->info[0], COLOR_YELLOW);	

	if (!keys->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        keys->iv = iv;
    }  

    
	//ui_init_textview(tv, hotknot_handler, (void*)keys);
	DEBUG("%s, %d, iv:0x%x, iv->set_title:0x%x, title:0x%x\n", __FUNCTION__, __LINE__, iv,iv->set_title, &keys->title);
    iv->set_title(iv, &keys->title);
	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

	iv->set_items(iv, hotknot_items, 0);
	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
	iv->set_text(iv, &keys->text);
	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

    pthread_create(&keys->update_thd, NULL, hotknot_handler, priv);
    DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
    TestItem_hotknot = -1;
    do {
        chosen = iv->run(iv, &exit);
        //LOGD(TAG "%s, chosen = %d\n", __FUNCTION__, chosen);
        DEBUG("%s, chosen = %d\n", __FUNCTION__, chosen);
        switch (chosen) {
        case ITEM_HOTKNOT_MASTER:
        	   TestItem_hotknot = ITEM_HOTKNOT_MASTER;
			   DEBUG("[%s]TestItem_hotknot = %d\n", __FUNCTION__, TestItem_hotknot);
            break;

        case ITEM_HOTKNOT_SLAVE:
        	   TestItem_hotknot = ITEM_HOTKNOT_SLAVE;
			   DEBUG("[%s]TestItem_hotknot = %d\n", __FUNCTION__, TestItem_hotknot);
            break;
        case ITEM_HOTKNOT_PASS:
        case ITEM_HOTKNOT_FAIL:
            if (chosen == ITEM_HOTKNOT_PASS) {
                keys->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_HOTKNOT_FAIL) {
                keys->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        }
        
        if (exit) {
            keys->exit_thd = true;
            LOGD(TAG "%s, nfc->exit_thd = true\n", __FUNCTION__);
            break;
        }        
        
        usleep(100000); // wake up every 0.1sec		
    } while (1);
    pthread_join(keys->update_thd, NULL);


	return 0;
}

int hotknot_init(void)
{
	int r;
	struct ftm_module *mod = {NULL};
	struct hotknot *hotknot = {NULL};
	DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

	mod = ftm_alloc(ITEM_HOTKNOT, sizeof(struct hotknot));
	if (!mod)
		return -ENOMEM;

	hotknot = mod_to_hotknot(mod);
	hotknot->mod = mod;
	/*keys->iv = ui_new_itemview();
	if (!keys->iv)
		return -ENOMEM;*/
    DEBUG("%s, %d\n", __FUNCTION__, __LINE__);

	r = ftm_register(mod, hotknot_entry, (void*)hotknot);
	if (r) {
		LOGD(TAG "register hotknot failed (%d)\n", r);
		return r;
	}
    DEBUG("%s, %d\n", __FUNCTION__, __LINE__);
	return 0;
}

#endif

#ifdef __cplusplus
};
#endif

