/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/* 
 *
 * (C) Copyright 2011 
 * MediaTek <www.MediaTek.com>
 * Hongcheng Xia <Hongcheng.Xia@MediaTek.com>
 *
 */

//for fm auto test and debug

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/resource.h>

#include "fm_main.h"
#include "fm_rds.h"
#include "fm_ioctl.h"
#include "fm_cust_cfg.h"
#include "autofm.h"

static int autofm_run = 1;

static int fm_open_dev(struct fm_cmd *cmd);
static int fm_close_dev(struct fm_cmd *cmd);
static int fm_power_up(struct fm_cmd *cmd);
static int fm_power_up_tx(struct fm_cmd *cmd);
static int fm_power_down(struct fm_cmd *cmd);
static int fm_tune(struct fm_cmd *cmd);
static int fm_tune_tx(struct fm_cmd *cmd);
static int fm_seek(struct fm_cmd *cmd);
static int fm_scan(struct fm_cmd *cmd);
static int fm_jammer_scan(struct fm_cmd *cmd);
static int fm_getcqi(struct fm_cmd *cmd);
static int fm_getrssi(struct fm_cmd *cmd);
static int fm_mute(struct fm_cmd *cmd);
static int fm_unmute(struct fm_cmd *cmd);
static int fm_scan_force_stop(struct fm_cmd *cmd);
static int fm_setvol(struct fm_cmd *cmd);
static int fm_getvol(struct fm_cmd *cmd);
static int fm_over_bt(struct fm_cmd *cmd);
static int fm_32kppm_compensation(struct fm_cmd *cmd);
static int fm_i2s_set(struct fm_cmd *cmd);
static int fm_rds_on_off(struct fm_cmd *cmd);
static int fm_rds_tx(struct fm_cmd *cmd);
static int fm_read_reg(struct fm_cmd *cmd);
static int fm_write_reg(struct fm_cmd *cmd);
static int fm_top_read_reg(struct fm_cmd *cmd);
static int fm_top_write_reg(struct fm_cmd *cmd);
static int fm_host_read_reg(struct fm_cmd *cmd);
static int fm_host_write_reg(struct fm_cmd *cmd);
static int fm_mod_reg(struct fm_cmd *cmd);
static int fm_print_reg(struct fm_cmd *cmd);
static int fm_help(struct fm_cmd *cmd);
static int fm_bye(struct fm_cmd *cmd);
static int fm_delay(struct fm_cmd *cmd);
static int fm_stdsrc_set(struct fm_cmd *cmd);

static struct fm_cmd g_cmd[] = {
    {"open", 0, fm_open_dev, {NULL, NULL, NULL}, "eg: open, desc: open dev/fm"},
    {"close", 0, fm_close_dev, {NULL, NULL, NULL}, "eg: close, desc: close dev/fm"},
    {"pwron", 0, fm_power_up, {NULL, NULL, NULL}, "eg: pwron, desc: power on fm rx subsystem"},
    {"txpwron", 0, fm_power_up_tx, {NULL, NULL, NULL}, "eg: txpwron, desc: power on fm tx subsystem"},
    {"pwroff", 0, fm_power_down, {NULL, NULL, NULL}, "eg: pwroff, desc: power off fm subsystem"},
    {"tune", 1, fm_tune, {NULL, NULL, NULL}, "eg: tune 876, desc: tune to a channel"},
    {"txtune", 1, fm_tune_tx, {NULL, NULL, NULL}, "eg: txtune 876, desc: tune tx to a channel"},
    {"seek", 1, fm_seek, {NULL, NULL, NULL}, "eg: seek up, desc: seek to another channel"},
    {"scan", 0, fm_scan, {NULL, NULL, NULL}, "eg: scan, desc: auto scan"},
    {"jamscan", 0, fm_jammer_scan, {NULL, NULL, NULL}, "eg: jamscan, desc: auto jammer scan"},
    {"cqi", 1, fm_getcqi, {NULL, NULL, NULL}, "eg: cqi, desc: get channel CQI"},
    {"rssi", 0, fm_getrssi, {NULL, NULL, NULL}, "eg: rssi, desc: get current channel RSSI"},
    {"mute", 0, fm_mute, {NULL, NULL, NULL}, "eg: mute, desc: mute audio"},
    {"unmute", 0, fm_unmute, {NULL, NULL, NULL}, "eg: unmute, desc: unmute audio"},
    {"sstop", 0, fm_scan_force_stop, {NULL, NULL, NULL}, "eg: sstop, desc: stop scan flow"},
    {"setvol", 1, fm_setvol, {NULL, NULL, NULL}, "eg: setvol 14, desc: set audio volume(rang:0~14)"},
    {"getvol", 0, fm_getvol, {NULL, NULL, NULL}, "eg: getvol, desc: get audio volume"},
    {"viabt", 1, fm_over_bt, {NULL, NULL, NULL}, "eg: viabt on, desc: set fm over bt on/off"},
    {"gpsrtc", 2, fm_32kppm_compensation, {NULL, NULL, NULL}, "eg: gpsrtc 10 20, desc: gps rtc setting"},
    {"seti2s", 3, fm_i2s_set, {NULL, NULL, NULL}, "eg: seti2s on slave 32k , desc: i2s setting"},
    {"rds", 1, fm_rds_on_off, {NULL, NULL, NULL}, "eg: rds on, desc: rds rx on/off"},
    {"txrds", 3, fm_rds_tx, {NULL, NULL, NULL}, "eg: txrds 0x1234 0 0 , desc: rds tx"},
    {"delay", 2, fm_delay, {NULL, NULL, NULL}, "eg: delay 100 ms, desc: delay n s/ms/us"},
    {"rd", 1, fm_read_reg, {NULL, NULL, NULL}, "eg: rd 0x62, desc: read a register"},
    //{"read", 1, fm_read_reg, {NULL, NULL, NULL}, "eg: read 0x62, desc: read a register"},
    {"wr", 2, fm_write_reg, {NULL, NULL, NULL}, "eg: wr 0x62 0xFF15, desc: write a register"},
    //{"write", 2, fm_write_reg, {NULL, NULL, NULL}, "eg: write 0x62 0xFF15, desc: write a register"},
    {"toprd", 1, fm_top_read_reg, {NULL, NULL, NULL}, "eg: toprd 0x0050, desc: read a top register"},
    {"topwr", 2, fm_top_write_reg, {NULL, NULL, NULL}, "eg: topwr 0x0050 0x00000007, desc: write top register"},
    {"hostrd", 1, fm_host_read_reg, {NULL, NULL, NULL}, "eg: hostrd 0x80103000, desc: read mcu host register"},
    {"hostwr", 2, fm_host_write_reg, {NULL, NULL, NULL}, "eg: hostwr 0x80103000 0x00000001, desc: write host register"},
    {"mod", 3, fm_mod_reg, {NULL, NULL, NULL}, "eg: mod 0x62 0xFFFE 0x0001, desc: modify a register"},
    {"modify", 3, fm_mod_reg, {NULL, NULL, NULL}, "eg: modify 0x62 0xFFFE 0x0001, desc: modify a register"},
    {"showreg", 1, fm_print_reg, {NULL, NULL, NULL}, "eg: showreg, desc: print all register"},
    {"setsrc", 2, fm_stdsrc_set, {NULL, NULL, NULL}, "eg: setsrc in /system/bin/abc.txt, desc: set stdinput"},
    {"help", 0, fm_help, {NULL, NULL, NULL}, "eg: help, desc: show help info"},
    {"bye", 0, fm_bye, {NULL, NULL, NULL}, "eg: bye, desc: exit autofm"}
};

static int g_fm_fd = -1;
static uint16_t g_freq = 967;
static int type = -1;

static int to_upper(char *str)
{
    int i = 0;

    for(i=0; i < (int)strlen(str); i++){
        if(('a' <= str[i]) && (str[i] <= 'z')){
            str[i] = str[i] - ('a' - 'A');
        }
    }
    return 0;
}

static int to_upper_n(char *str, int len)
{
    int i = 0;

    for(i=0; i < len; i++){
        if(('a' <= str[i]) && (str[i] <= 'z')){
            str[i] = str[i] - ('a' - 'A');
        }
    }
    return 0;
}

#if 0
static int check_hex_str(char *str, int len)
{
    int i = 0;

    for(i=0; i < len; i++){
        if((('a' <= str[i]) && (str[i] <= 'z')) || (('A' <= str[i]) && (str[i] <= 'Z')) || (('0' <= str[i]) && (str[i] <= '9'))){
            ;
        }else{
            return -1;
        }
    }
    return 0;
}
#else
static int check_hex_str(char *str, int len)
{
    int i = 0;

    for(i=0; i < len; i++){
        if((('!' <= str[i]) && (str[i] <= '~'))){
            ;
        }else{
            return -1;
        }
    }
    return 0;
}
#endif

static int check_dec_str(char *str, int len)
{
    int i = 0;

    for(i=0; i < len; i++){
        if(('0' <= str[i]) && (str[i] <= '9')){
            ;
        }else{
            return -1;
        }
    }
    return 0;
}

static int ascii_to_hex_u32(char *in_ascii, uint32_t *out_hex)
{
    int len = (int)strlen(in_ascii);
    int i = 0;
    uint32_t tmp;

    len = (len > 8)? 8 : len;
    if (check_hex_str(in_ascii, len))
    {
        return -1;
    }
    to_upper_n(in_ascii, len);
    *out_hex = 0;
    for (i=0; i < len; i++)
    {
        if (in_ascii[len-i-1] < 'A')
        {
            tmp = in_ascii[len-i-1];
            *out_hex |= ((tmp - '0') << (4*i));
        }
        else
        {
            tmp = in_ascii[len-i-1];
            *out_hex |= ((tmp - 'A' + 10) << (4*i));
        }
    }
    return 0;
}

static int ascii_to_hex(char *in_ascii, uint16_t *out_hex)
{
    int len = (int)strlen(in_ascii);
    int i = 0;
    uint16_t tmp;

    len = (len > 4)? 4 : len;
    if(check_hex_str(in_ascii, len)){
        return -1;
    }
    to_upper_n(in_ascii, len);
    *out_hex = 0;
    for(i=0; i < len; i++){
        if(in_ascii[len-i-1] < 'A'){
            tmp = in_ascii[len-i-1];
            *out_hex |= ((tmp - '0') << (4*i));
        }else{
            tmp = in_ascii[len-i-1];
            *out_hex |= ((tmp - 'A' + 10) << (4*i));
        }
    }
    return 0;
}

static int ascii_to_dec_u32(char *in_ascii, uint32_t *out_dec)
{
    int len = (int)strlen(in_ascii);
    int i = 0;
    int flag;
    int multi = 1;

    len = (len > 10)? 10 : len;
    /*if (in_ascii[0] == '-')
    {
        flag = -1;
        in_ascii += 1;
        len -= 1;
    }
    else*/
    {
        flag = 1;
    }
    if (check_dec_str(in_ascii, len))
    {
        return -1;
    }
    *out_dec = 0;
    multi = 1;
    for (i=0; i < len; i++)
    {
        *out_dec += ((in_ascii[len-i-1] - '0') * multi);
        multi *= 10;
    }
    //*out_dec *= flag;
    return 0;
}

static int ascii_to_dec(char *in_ascii, int *out_dec)
{
    int len = (int)strlen(in_ascii);
    int i = 0;
    int flag;
    int multi = 1;

    len = (len > 10)? 10 : len;
    if(in_ascii[0] == '-'){
        flag = -1;
        in_ascii += 1;
        len -= 1;
    }else{
        flag = 1;
    }
    if(check_dec_str(in_ascii, len)){
        return -1;
    }
    *out_dec = 0;
    multi = 1;
    for(i=0; i < len; i++){
        *out_dec += ((in_ascii[len-i-1] - '0') * multi);
        multi *= 10;
    }
    *out_dec *= flag;
    return 0;
}

static int trim_string(char **start)
{
    char *end = *start;

    /* Advance to non-space character */
    while(*(*start) == ' '){
        (*start)++;
    }

    /* Move to end of string */
    while(*end != '\0'){
        (end)++;
    }

    /* Backup to non-space character */
    do{
        end--;
    }while((end >= *start) && (*end == ' '));

    /* Terminate string after last non-space character */
    *(++end) = '\0';
    return (end - *start);
}

static int trim_path(char **start)
{
    char *end = *start;

    while(*(*start) == ' '){
        (*start)++;
    }

    while(*end != '\0'){
        (end)++;
    }
    
    do{
        end--;
    }while((end >= *start) && ((*end == ' ') || (*end == '\n') || (*end == '\r')));

    *(++end) = '\0';
    return (end - *start);
}

static int cmd_parser(char *src, char **new_pos, char **word)
{
    char *p = src;
    char *word_start = NULL;

    enum fm_cmd_parser_state state = FM_CMD_STAT_NONE;

    for(p = src; *p != '\0'; p++){
        switch(state){
            case FM_CMD_STAT_NONE:{
                if(!check_hex_str(p, 1)){
                    //if we get show char in none state, it means a new word start
                    state = FM_CMD_STAT_WORD;
                    word_start = p;
                }
                break;
            }
            
            case FM_CMD_STAT_WORD:{
                if(check_hex_str(p, 1)){
                    //if we get non show char in word state, it means a word complete
                    *p = '\0';
                    //FIX_ME
                    //record word
                    state = FM_CMD_STAT_NONE;
                    trim_string(&word_start);
                    *word = word_start;
                    *new_pos = p + 1;
                    //printf("%s\n", word_start);
                    return 0;
                }
                break;
            }
            default:
                break;
        }
    }
    return -1;
}

static int get_cmd(char *buf, int buf_size)
{
#if 0
    char tmp = 0;

    do{
        tmp = getchar();
        *buf = tmp;
        buf++;
        buf_size--;
    }while((tmp != '\r')  && (tmp != '\n') && (buf_size > 0));
#else
    int ret;
    struct timeval tval;
    tval.tv_sec = 0;
    tval.tv_usec = 100*1000;

    ret = read(0, buf, buf_size);
    //sleep(1); //sleep 1s  
    select(0, NULL, NULL, NULL, &tval);

#endif
    return 0;
}

//fm funtion APIs
static int fm_open_dev(struct fm_cmd *cmd)
{
    int fd = open(FM_DEVICE_NAME, O_RDWR);
    if(fd == -1){
        printf("FAIL open %s failed\n", FM_DEVICE_NAME);
        return -1;
    }
    printf("OK\n");
    g_fm_fd = fd;

    return 0;
}

static int fm_close_dev(struct fm_cmd *cmd)
{
    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    if(0 != close(g_fm_fd)){
        return -1;
    }

    printf("OK\n");
    g_fm_fd = -1;
    return 0;
}

static int fm_power_up(struct fm_cmd *cmd)
{
    int ret;
    struct fm_tune_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_tune_parm));

    parm.band = FM_BAND_UE;
    parm.freq = 975;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_100K;

    ret = ioctl(g_fm_fd, FM_IOCTL_POWERUP, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK\n");

    type = FM_RX;
    return 0;
}

static int fm_power_up_tx(struct fm_cmd *cmd)
{
    int ret;
    struct fm_tune_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_tune_parm));

    parm.band = FM_BAND_UE;
    parm.freq = 975;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_100K;

    ret = ioctl(g_fm_fd, FM_IOCTL_POWERUP_TX, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK\n");

    type = FM_TX;
    return 0;
}

static int fm_power_down(struct fm_cmd *cmd)
{
    int ret;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_POWERDOWN, &type);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    return 0;
}

static int fm_rds_on_off(struct fm_cmd *cmd)
{
    int ret;
    uint16_t on_off;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    if(strcmp(cmd->para[0], "on") == 0){
        on_off = 1; 
    }else if(strcmp(cmd->para[0], "off") == 0){
        on_off = 0;
    }else{
        return -1;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_RDS_ONOFF, &on_off);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    
    printf("OK\n");
    return 0;
}

static int get_int_val(char *src_val, int *dst_val)
{
    int ret = 0;
    uint16_t tmp_hex;
    int tmp_dec;

    //printf("%s\n", src_val);
    if(memcmp(src_val, "0x", strlen("0x")) == 0){
        src_val += strlen("0x");
        ret = ascii_to_hex(src_val, &tmp_hex);
        if(!ret){
            *dst_val = (int)tmp_hex;
            return 0;
        }else{
            printf("error\n");
            return -1;
        }
    }else{
        ret = ascii_to_dec(src_val, &tmp_dec);
        if(!ret && ((0 <= tmp_dec) && (tmp_dec <= 0xFFFF))){
            *dst_val = tmp_dec;
            return 0;
        }else{
            printf("error\n");
            return -1;
        }
    }
}

static int get_u32_val(char *src_val, uint32_t *dst_val)
{
    int ret = 0;
    uint32_t tmp_hex;
    uint32_t tmp_dec;

    //printf("%s\n", src_val);
    if (memcmp(src_val, "0x", strlen("0x")) == 0)
    {
        src_val += strlen("0x");
        ret = ascii_to_hex_u32(src_val, &tmp_hex);
        if (!ret)
        {
            *dst_val = tmp_hex;
            return 0;
        }
        else
        {
            printf("error\n");
            return -1;
        }
    }
    else
    {
        ret = ascii_to_dec_u32(src_val, &tmp_dec);
        if (!ret && (tmp_dec <= 0xFFFFFFFF))
        {
            *dst_val = tmp_dec;
            return 0;
        }
        else
        {
            printf("error\n");
            return -1;
        }
    }
}

static int fm_tune(struct fm_cmd *cmd)
{
    int ret;
    int freq;
    struct fm_tune_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_tune_parm));

    ret = get_int_val(cmd->para[0], &freq);
    if(ret){
        return -1;
    }
    parm.band = FM_BAND_UE;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_100K;

    ret = ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK:%d\n", parm.freq);
    g_freq = parm.freq;

    return 0;
}

static int fm_stdsrc_set(struct fm_cmd *cmd)
{
    #define FM_STD_SHELL 0
    #define FM_STD_FILE 1
    int ret;
    int tmp;
    static int fm_stdin = 0;
    static int fm_stdout = 1;
    static int fm_stderr = 2;
    static int input = FM_STD_SHELL;
    static int output = FM_STD_SHELL;
    static int errput = FM_STD_SHELL;
    
    char *path = cmd->para[1];

    if(fm_stdin == 0){
        fm_stdin = dup(0);
        fm_stdout = dup(1);
        fm_stderr = dup(2);
        printf("backup, stdin:%d stdot:%d stderr:%d\n", fm_stdin, fm_stdout, fm_stderr);
    }

    if(strcmp(cmd->para[0], "in") == 0)
	{
        if(strcmp(cmd->para[1], "shell") == 0)
		{
            if(input == FM_STD_FILE)
			{
                dup2(fm_stdin, 0);
                input = FM_STD_SHELL;
            }
			else
			{
                ;//fm already use shell as stdin
            }
        }
		else if(path != NULL)
		{
            tmp = open(path, O_RDONLY);
            if(tmp < 0){
                printf("FAIL: open %s err\n", path);
                return -1;
            }
            ret = dup2(tmp, 0);
            if(ret < 0){
                printf("FAIL: dup %s err\n", path);
                return -1;
            }
            close(tmp);
            input = FM_STD_FILE;
        }
		else
		{
            printf("FAIL: path err\n");
            return -1;
        }
    }
	else if(strcmp(cmd->para[0], "out") == 0)
	{
        if(strcmp(cmd->para[1], "shell") == 0)
		{
            if(output == FM_STD_FILE){
                dup2(fm_stdin, 1);
                output = FM_STD_SHELL;
            }else{
                ;//fm already use shell as stdin
            }
        }
		else if(path != NULL)
		{
            tmp = open(path, O_RDWR | O_CREAT,S_IRUSR | S_IWUSR);
            if(tmp < 0){
                printf("FAIL: open %s err\n", path);
                return -1;
            }
            ret = dup2(tmp, 1);
            if(ret < 0){
                printf("FAIL: dup %s err\n", path);
                return -1;
            }
            close(tmp);
            output = FM_STD_FILE;
        }
		else
        {
            printf("FAIL: path err\n");
            return -1;
        }
    }
	else if(strcmp(cmd->para[0], "err") == 0)
	{
        if(strcmp(cmd->para[1], "shell") == 0)
		{
            if(errput == FM_STD_FILE){
                dup2(fm_stdin, 2);
                errput = FM_STD_SHELL;
            }else{
                ;//fm already use shell as stdin
            }
        }
		else if(path != NULL)
		{
            tmp = open(path, O_RDWR | O_CREAT,S_IRUSR | S_IWUSR);
            if(tmp < 0){
                printf("FAIL: open %s err\n", path);
                return -1;
            }
            ret = dup2(tmp, 2);
            if(ret < 0){
                printf("FAIL: dup %s err\n", path);
                return -1;
            }
            close(tmp);
            errput = FM_STD_FILE;
        }
		else
		{
            printf("FAIL: path err\n");
            return -1;
        }
    }
	else
	{
        return -1;
    }
    
    printf("OK:%s\n", cmd->para[0]);

    return 0;
}

static int fm_tune_tx(struct fm_cmd *cmd)
{
    int ret;
    int freq;
    struct fm_tune_parm parm;

    if (g_fm_fd < 0)
    {
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_tune_parm));

    ret = get_int_val(cmd->para[0], &freq);
    if(ret){
        return -1;
    }
    parm.band = FM_BAND_UE;
    parm.freq = freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_100K;

    ret = ioctl(g_fm_fd, FM_IOCTL_TUNE_TX, &parm);
    if (ret)
    {
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK:%d\n", parm.freq);
    g_freq = parm.freq;

    return 0;
}

static int fm_rds_tx(struct fm_cmd *cmd)
{
    int ret;
    struct fm_rds_tx_parm parm;
    int pi;
    uint16_t ps[12] = {0x3132, 0x3334, 0x3536,
                       0x3132, 0x3334, 0x3536,
                       0x3132, 0x3334, 0x3536,
                       0x3132, 0x3334, 0x3536};
    int i;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_rds_tx_parm));

    ret = get_int_val(cmd->para[0], &pi);
    if(ret){
        return -1;
    }
    parm.pi = (uint16_t)pi;
    for(i = 0; i < 12; i++){
        parm.ps[i] = ps[i];
    }
    parm.other_rds_cnt = 0;

    ret = ioctl(g_fm_fd, FM_IOCTL_RDS_TX, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK\n");

    return 0;
}

static int fm_seek(struct fm_cmd *cmd)
{
    int ret;
    struct fm_seek_parm parm;

    if (g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }
    bzero(&parm, sizeof(struct fm_tune_parm));

    if(strcmp(cmd->para[0], "up") == 0){
        parm.seekdir = FM_SEEK_UP; 
    }else if(strcmp(cmd->para[0], "down") == 0){
        parm.seekdir = FM_SEEK_DOWN;
    }else{
        return -1;
    }
    parm.band = FM_BAND_UE;
    parm.freq = g_freq;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.space = FM_SPACE_100K;
    parm.seekth = FM_SEEKTH_LEVEL_DEFAULT;
    ret = ioctl(g_fm_fd, FM_IOCTL_SEEK, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK:%d\n",parm.freq);
    g_freq = parm.freq;

    return 0;
}

static int fm_scan(struct fm_cmd *cmd)
{
    int ret;
    int seekdir;
    struct fm_scan_parm parm;
    uint16_t CH_Data[256], tmp_val, ch_offset, MASK_CH;
    int LOFREQ = 875;    
    int i = 0, step = 0, ch_cnt = 0;
    
    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_scan_parm));

    parm.band = FM_BAND_UE;
    parm.space = FM_SPACE_100K;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.freq = g_freq;
    parm.ScanTBLSize = sizeof(parm.ScanTBL)/sizeof(uint16_t);

    ret = ioctl(g_fm_fd, FM_IOCTL_SCAN, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK\n");
    g_freq = parm.freq;
    
    //parm.ScanTBLSize will return valid scan table size. 
    printf("Scan channel:");   
    for(ch_offset = 0; ch_offset < parm.ScanTBLSize; ch_offset++){
		MASK_CH = 0x0001; 
		tmp_val = parm.ScanTBL[ch_offset];
		for(step = 0; step < 16; step++){
			MASK_CH = 1 << step;
			if((MASK_CH & tmp_val)!=0){
				*(CH_Data + ch_cnt) = LOFREQ  + (ch_offset * 16 + step) * (parm.space); //100KHz
                if(*(CH_Data + ch_cnt) <= 1080){
				printf("%d ", *(CH_Data + ch_cnt)); 
				ch_cnt++;
			}
		} 
	} 
	} 
	printf("\nScan channel num:%d\n", ch_cnt); 

    return 0;
}

static int fm_jammer_scan(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    int seekdir;
    struct fm_scan_parm parm;
    uint16_t CH_Data[256], tmp_val, ch_offset, MASK_CH;
    int LOFREQ = 875;    
    int i = 0, step = 0, ch_cnt = 0;
    struct fm_cmd cmd_tmp;
    struct fm_cqi_req cqi_req;
    struct fm_cqi *cqi = NULL;
    
    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    cmd_tmp.para[0] = "0x61";
    cmd_tmp.para[1] = "0xDFFF";
    cmd_tmp.para[2] = "0x2000";
    ret = fm_mod_reg(&cmd_tmp);
    if(ret){
        return ret;
    }
    
    bzero(&parm, sizeof(struct fm_scan_parm));

    parm.band = FM_BAND_UE;
    parm.space = FM_SPACE_100K;
    parm.hilo = FM_AUTO_HILO_OFF;
    parm.freq = g_freq;
    parm.ScanTBLSize = sizeof(parm.ScanTBL)/sizeof(uint16_t);

    ret = ioctl(g_fm_fd, FM_IOCTL_SCAN, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK\n");
    g_freq = parm.freq;
    
    //parm.ScanTBLSize will return valid scan table size. 
    printf("Scan channel:");   
    for(ch_offset = 0; ch_offset < parm.ScanTBLSize; ch_offset++){
		MASK_CH = 0x0001; 
		tmp_val = parm.ScanTBL[ch_offset];
		for(step = 0; step < 16; step++){
			MASK_CH = 1 << step;
			if((MASK_CH & tmp_val)!=0){
				*(CH_Data + ch_cnt) = LOFREQ  + (ch_offset * 16 + step) * (parm.space); //100KHz
				printf("%d ", *(CH_Data + ch_cnt)); 
				ch_cnt++;
			}
		} 
	} 
	printf("\nScan channel num:%d\n", ch_cnt); 

    cqi_req.ch_num = (uint16_t)ch_cnt;
    cqi_req.ch_num = (cqi_req.ch_num > 255) ? 255 : cqi_req.ch_num;
    cqi_req.buf_size = cqi_req.ch_num*sizeof(struct fm_cqi);
    cqi_req.cqi_buf = malloc(cqi_req.buf_size + 1);
    if(!cqi_req.cqi_buf){
        printf("no mem\n");
        return -1;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_CQI_GET, &cqi_req);
    if(ret){
        free(cqi_req.cqi_buf);
        printf("FAIL:%d\n", ret);
        return -1;
    }
    cqi = (struct fm_cqi*)cqi_req.cqi_buf;
    tmp = 0;
    for(tmp = 0; tmp < cqi_req.ch_num; tmp++){
        printf("freq %d, jam_freq 0x%04x, reserve 0x%04x\n", (cqi[tmp].ch/2 + 640), cqi[tmp].rssi, cqi[tmp].reserve);
    }
    free(cqi_req.cqi_buf);
    printf("OK\n");
    
    cmd_tmp.para[0] = "0x61";
    cmd_tmp.para[1] = "0xDFFF";
    cmd_tmp.para[2] = "0x0000";
    ret = fm_mod_reg(&cmd_tmp);
    if(ret){
        return ret;
    }

    return 0;
}

static int fm_getcqi(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    struct fm_cqi_req cqi_req;
    struct fm_cqi *cqi = NULL;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    cqi_req.ch_num = (uint16_t)tmp;
    cqi_req.ch_num = (cqi_req.ch_num > 255) ? 255 : cqi_req.ch_num;
    cqi_req.buf_size = cqi_req.ch_num*sizeof(struct fm_cqi);
    cqi_req.cqi_buf = malloc(cqi_req.buf_size + 1);
    if(!cqi_req.cqi_buf){
        printf("no mem\n");
        return -1;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_CQI_GET, &cqi_req);
    if(ret){
        free(cqi_req.cqi_buf);
        printf("FAIL:%d\n", ret);
        return -1;
    }
    cqi = (struct fm_cqi*)cqi_req.cqi_buf;
    tmp = 0;
    for(tmp = 0; tmp < cqi_req.ch_num; tmp++){
        printf("freq %d, rssi 0x%04x, reserve 0x%04x\n", cqi[tmp].ch/2 + 640, cqi[tmp].rssi, cqi[tmp].reserve);
    }
    free(cqi_req.cqi_buf);
    printf("OK\n");

    return 0;
}

static int fm_scan_force_stop(struct fm_cmd *cmd)
{
    int stop_scan = 1;
    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }
    
    if(lseek(g_fm_fd, 0, SEEK_END) != -1){
        printf("OK\n");
        return 0;
    }else{
        printf("FAIL\n");
        return -1;    
    }
    
}

//scan test.
static int fm_seek_all(struct fm_cmd *cmd)
{
    int ret;
    int start_freq = 1080;

    struct fm_seek_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    g_freq = start_freq;

    do{
        bzero(&parm, sizeof(struct fm_tune_parm));
        parm.band = FM_BAND_UE;
        parm.freq = g_freq;
        parm.hilo = FM_AUTO_HILO_OFF;
        parm.space = FM_SPACE_100K;
        parm.seekdir = FM_SEEK_UP;
        parm.seekth = FM_SEEKTH_LEVEL_DEFAULT;
        
        ret = ioctl(g_fm_fd, FM_IOCTL_SEEK, &parm);
        if(ret){
            printf("FAIL:%d:%d\n", ret, parm.err);
            return -1;
        }
        printf("OK:%d:%d:%d\n", ret, parm.err, parm.freq);
        g_freq = parm.freq;
    }while(parm.err == FM_SUCCESS);
    
    return 0;
}

static int fm_mute(struct fm_cmd *cmd)
{
    int ret;
    uint32_t mute = 1;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_MUTE, &mute);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }

    printf("OK\n");
    return 0;
}

static int fm_unmute(struct fm_cmd *cmd)
{
    int ret;
    uint32_t mute = 0;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_MUTE, &mute);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }

    printf("OK\n");
    return 0;
}

static int fm_setvol(struct fm_cmd *cmd)
{
    int ret;
    uint32_t vol;
    int tmp;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    vol = (uint32_t)tmp;
    ret = ioctl(g_fm_fd, FM_IOCTL_SETVOL, &vol);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }

    printf("OK\n");
    return 0;
}

static int fm_getvol(struct fm_cmd *cmd)
{
    int ret;
    uint32_t vol;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_GETVOL, &vol);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    printf("volume:%d\n", (int)vol);

    return 0;
}

static int fm_getrssi(struct fm_cmd *cmd)
{
    int ret;
    int rssi;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_GETRSSI, &rssi);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    printf("RSSI:%d\n", rssi);

    return 0;
}

static int fm_32kppm_compensation(struct fm_cmd *cmd)
{
    int ret;
    int tmp = -1;
    struct fm_gps_rtc_info rtcInfo;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&rtcInfo, sizeof(struct fm_gps_rtc_info));
    rtcInfo.ageThd = 2;
    rtcInfo.driftThd = 30;
    rtcInfo.retryCnt = 2;
    rtcInfo.tvThd.tv_sec = 10;
    
    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    rtcInfo.age = tmp;

    ret = get_int_val(cmd->para[1], &tmp);
    if(ret){
        return -1;
    }
    rtcInfo.drift = tmp;
    ret = ioctl(g_fm_fd, FM_IOCTL_GPS_RTC_DRIFT, &rtcInfo);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    return 0;
}

static int fm_i2s_set(struct fm_cmd *cmd)
{
    int ret;
    int tmp = -1;
    static struct fm_i2s_setting i2s_cfg;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&i2s_cfg, sizeof(struct fm_i2s_setting));
    i2s_cfg.onoff = FM_I2S_ON;
    i2s_cfg.mode = FM_I2S_SLAVE;
    i2s_cfg.sample = FM_I2S_48K;

    if(strcmp(cmd->para[0], "on") == 0){
        i2s_cfg.onoff = FM_I2S_ON; 
    }else if(strcmp(cmd->para[0], "off") == 0){
        i2s_cfg.onoff = FM_I2S_OFF;
    }else{
        return -1;
    }

    if(strcmp(cmd->para[1], "master") == 0){
        i2s_cfg.mode = FM_I2S_MASTER; 
    }else if(strcmp(cmd->para[1], "slave") == 0){
        i2s_cfg.mode = FM_I2S_SLAVE;
    }else{
        return -1;
    }

    if(strcmp(cmd->para[2], "32k") == 0){
        i2s_cfg.sample = FM_I2S_32K; 
    }else if(strcmp(cmd->para[2], "44k") == 0){
        i2s_cfg.mode = FM_I2S_44K;
    }else if(strcmp(cmd->para[2], "48k") == 0){
        i2s_cfg.mode = FM_I2S_48K;
    }else{
        return -1;
    }

    ret = ioctl(g_fm_fd, FM_IOCTL_I2S_SETTING, &i2s_cfg);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    return 0;
}

static int fm_over_bt(struct fm_cmd *cmd)
{
    int ret;
    uint32_t viabt = -1;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    if(strcmp(cmd->para[0], "on") == 0){
        viabt = 1; 
    }else if(strcmp(cmd->para[0], "off") == 0){
        viabt = 0;
    }else{
        return -1;
    }
    ret = ioctl(g_fm_fd, FM_IOCTL_OVER_BT_ENABLE, &viabt);
    if(ret){
        printf("FAIL:%d\n", ret);
        return -1;
    }
    printf("OK\n");
    return 0;
}

static int fm_read_reg(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    struct fm_ctl_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_ctl_parm));

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    parm.addr = (uint8_t)tmp;
    parm.rw_flag = 1;

    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%02x = 0x%04x\n", parm.addr, parm.val);

    return 0;
}

static int print_page(uint16_t page, int mode)
{
    int ret;
    uint8_t addr = 0x00;
    int i = 0;
    int log_fd = -1;
    int std_out = -1;
    struct fm_ctl_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }
    if(mode == 0){
        ;//        
    }else if(mode == 1){
        std_out = dup(1);
        if(std_out < 0){
            ;//dup std out error
        }
        close(1);
        log_fd = open("/data/data/fmlog", O_RDWR | O_CREAT | O_APPEND,S_IRUSR | S_IWUSR);
        if(log_fd < 0){
            ;//error
        }
    }
    
    //setup page
    printf("page %d.\n", (int)page);
    bzero(&parm, sizeof(struct fm_ctl_parm));
    parm.addr = 0x9F;  //page addr
    parm.val = 0x0000; //page 0
    parm.rw_flag = 0; //write
    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    
start:    
    bzero(&parm, sizeof(struct fm_ctl_parm));
    parm.addr = addr;
    printf("0x%02x ", parm.addr);
    parm.rw_flag = 1;
    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("0x%04x\n", parm.val);
    addr++;
    if(++i < 256)
        goto start;

    if(mode == 0){
        ;//        
    }else if(mode == 1){
        close(log_fd);
        std_out = dup(std_out);
        printf("stdout [fd=%d]\n", std_out);
    }
    return 0;
}

static int fm_print_reg(struct fm_cmd *cmd)
{
    int ret = 0;
    int tmp = -1;

    if(strcmp(cmd->para[0], "shell") == 0){
        tmp = 0; 
    }else if(strcmp(cmd->para[0], "file") == 0){
        tmp = 1;
    }else{
        return -1;
    }
    ret = print_page(0x03, tmp);
    ret = print_page(0x02, tmp);
    ret = print_page(0x01, tmp);
    ret = print_page(0x00, tmp);
    printf("OK\n");

    return 0;
}

static int fm_write_reg(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    struct fm_ctl_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_ctl_parm));

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    parm.addr = (uint8_t)tmp;
    ret = get_int_val(cmd->para[1], &tmp);
    if(ret){
        return -1;
    }
    parm.val = (uint16_t)tmp;
    parm.rw_flag = 0;

    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%02x = 0x%04x\n", parm.addr, parm.val);

    return 0;
}

/*
 * fm_mod_reg
 * mod addr mask val -> mod 0x60 0xFFFE 0x0001
 */
static int fm_mod_reg(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    uint16_t mask;
    uint16_t val;
    struct fm_ctl_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }
    bzero(&parm, sizeof(struct fm_ctl_parm));

    ret = get_int_val(cmd->para[0], &tmp); //get addr
    if(ret){
        return -1;
    }
    parm.addr = (uint8_t)tmp;
    parm.rw_flag = 1;

    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    //printf("addr: 0x%02x\n", parm.addr);
    
    ret = get_int_val(cmd->para[1], &tmp); //get mask
    if(ret){ 
        return -1;
    }
    mask = (uint16_t)tmp;
    //printf("mask: 0x%04x\n", mask);

    ret = get_int_val(cmd->para[2], &tmp); //get bits
    if(ret){
        return -1;
    }
    val = (uint16_t)tmp;
    //printf("val: 0x%04x\n", val);
    
    parm.val = (parm.val & mask) | (val & (~mask));
    parm.rw_flag = 0;
    ret = ioctl(g_fm_fd, FM_IOCTL_RW_REG, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%02x = 0x%04x\n", parm.addr, parm.val);

    return 0;
}

static int fm_delay(struct fm_cmd *cmd)
{
    int ret = 0;
    int tmp;
    struct timeval tval;
    
    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }

    if(strcmp(cmd->para[1], "s") == 0){
        tval.tv_sec = tmp;
        tval.tv_usec = 0;
    }else if(strcmp(cmd->para[1], "ms") == 0){
        tval.tv_sec = 0;
        tval.tv_usec = tmp*1000;
    }else if(strcmp(cmd->para[1], "us") == 0){
        tval.tv_sec = 0;
        tval.tv_usec = tmp;
    }else{
        return -1;
    }
    printf("OK: %ds,%dus \n", (int)tval.tv_sec, (int)tval.tv_usec);
    select(0, NULL, NULL, NULL, &tval);

    return 0;
}

static int fm_help(struct fm_cmd *cmd)
{
    int id = -1;
    
    printf("-------------------------------------------------------\n");
    printf("%s\t%s\t\t%s\n", "cmd id", "cmd name", "cmd para num");
    printf("-------------------------------------------------------\n");
    while((++id) < (int)(sizeof(g_cmd)/sizeof(g_cmd[0]))) {
        printf("%d\t%s\t\t%d\t%s\n", id, g_cmd[id].name, g_cmd[id].para_size, g_cmd[id].description);
    }
    printf("-------------------------------------------------------\n");
    printf("OK\n");

    return 0;
}

static int fm_bye(struct fm_cmd *cmd)
{
    autofm_run = 0;
    
    printf("OK\n");
    return 0;
}

static enum fm_event_type get_key_type(char *key, struct fm_cmd **pcmd)
{
    int i;
    
    for(i = 0; i < (int)(sizeof(g_cmd)/sizeof(g_cmd[0])); i++){
        if(strcmp(g_cmd[i].name, key) == 0){
            //printf("%s %d\n", g_cmd[i].name, g_cmd[i].para_size);
            *pcmd = &g_cmd[i];
            return FM_EVENT_CMD;
        }
    }
    return FM_EVENT_VAL;
}

static int daemonize(int argc, char **argv)
{
    int ret = 0;
    int fd0, fd1, fd2;
    unsigned int i;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    ret = getrlimit(RLIMIT_NOFILE, &rl);
    if (ret < 0){
        return -1;
    }

    pid = fork();
    if (pid < 0){
        return -1;
    }else if (pid > 0){
        exit(0);
    }

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0){
        return -1;
    }

    pid = fork();
    if (pid < 0){
        return -1;
    }else if (pid > 0){
        exit(0);
    }

    chdir("/");
    
    if (rl.rlim_max == RLIM_INFINITY){
        rl.rlim_max = 1024;    
    }

    for (i = 0; i < (unsigned int)argc; i++){
        printf("%s ", argv[i]);
    }
    printf("\n");

    for (i = 0; i < rl.rlim_max; i++){
        close(i);
    }

    open(argv[1], O_RDWR | O_CREAT);
    open(argv[2], O_RDWR | O_CREAT);
    open(argv[3], O_RDWR | O_CREAT);

    return 0;
}

static int fm_top_read_reg(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    struct fm_top_rw_parm parm;

    if (g_fm_fd < 0)
    {
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_top_rw_parm));

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    parm.addr = (uint16_t)tmp;
    parm.rw_flag = 1;

    ret = ioctl(g_fm_fd, FM_IOCTL_TOP_RDWR, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%04x = 0x%08x\n", parm.addr, parm.val);

    return 0;
}

static int fm_top_write_reg(struct fm_cmd *cmd)
{
    int ret;
    int tmp;
    struct fm_top_rw_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_top_rw_parm));

    ret = get_int_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    parm.addr = (uint16_t)tmp;
    ret = get_int_val(cmd->para[1], &tmp);
    if(ret){
        return -1;
    }
    parm.val = (uint32_t)tmp;
    parm.rw_flag = 0;

    ret = ioctl(g_fm_fd, FM_IOCTL_TOP_RDWR, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%04x = 0x%08x\n", parm.addr, parm.val);

    return 0;
}

static int fm_host_read_reg(struct fm_cmd *cmd)
{
    int ret;
    uint32_t tmp;
    struct fm_host_rw_parm parm;

    if (g_fm_fd < 0)
    {
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_host_rw_parm));

    ret = get_u32_val(cmd->para[0], &tmp);
    printf("tmp = 0x%08x\n", tmp);
    if (ret)
    {
        return -1;
    }
    parm.addr = tmp;
    parm.rw_flag = 1;

    ret = ioctl(g_fm_fd, FM_IOCTL_HOST_RDWR, &parm);
    if (ret)
    {
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%08x = 0x%08x\n", parm.addr, parm.val);

    return 0;
}

static int fm_host_write_reg(struct fm_cmd *cmd)
{
    int ret;
    uint32_t tmp;
    struct fm_host_rw_parm parm;

    if(g_fm_fd < 0){
        printf("WARN fd unavailable\n");
        return -2;
    }

    bzero(&parm, sizeof(struct fm_host_rw_parm));

    ret = get_u32_val(cmd->para[0], &tmp);
    if(ret){
        return -1;
    }
    parm.addr = tmp;
    ret = get_u32_val(cmd->para[1], &tmp);
    if(ret){
        return -1;
    }
    parm.val = tmp;
    parm.rw_flag = 0;

    ret = ioctl(g_fm_fd, FM_IOCTL_HOST_RDWR, &parm);
    if(ret){
        printf("FAIL:%d:%d\n", ret, parm.err);
        return -1;
    }
    printf("OK: 0x%08x = 0x%08x\n", parm.addr, parm.val);

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;    
    int i;
    char *buf = NULL;
#define BUF_SIZE 1024*16

    buf = malloc(BUF_SIZE);
    if(!buf){
        printf("fm malloc memory failed!\n");
        return -1;
    }
    
    printf("\n%s:%s:%s\n", argv[0], __DATE__, __TIME__); 
    for (i = 0; i < argc; i++){
        printf("%s ", argv[i]);
    }
    printf("\n");

#ifdef FM_TOOL_BUILD_DEAMON
    daemonize(argc, argv);
#endif

    while(autofm_run){
        char *p = buf;
        char *key = NULL;
        enum fm_event_type keytype;
        struct fm_cmd *cmd = NULL;
        enum fm_sta sta = FM_STA_NON;
        
        memset(buf, 0, BUF_SIZE);
        get_cmd(buf, BUF_SIZE); //get a line form input file

        while(cmd_parser(p, &p, &key) == 0){
            keytype = get_key_type(key, &cmd); //this function will update "cmd" pointer if need
            switch(sta){
                case FM_STA_NON:
                    if(keytype == FM_EVENT_VAL){
                        printf("bad format\n");
                    }else if(cmd->para_size == 0){
                        //printf("%s\n", cmd->name);
                        cmd->handler(cmd); //yes, do cmd
                    }else{
                        sta = FM_STA_CMD; //cmd need one or more para
                    }
                    break;
                    
                case FM_STA_CMD:
                    if(keytype == FM_EVENT_VAL){
                        cmd->para[0] = key;
                        if(cmd->para_size == 1){
                            //printf("%s\n", cmd->name);
                            cmd->handler(cmd); //yes, do cmd
                            sta = FM_STA_NON;
                        }else{
                            sta = FM_STA_VAL_1;
                        }
                    }else{
                        printf("bad format\n");
                        if(cmd->para_size == 0){
                            //printf("%s\n", cmd->name);
                            cmd->handler(cmd); //yes, do cmd 
                            sta = FM_STA_NON;
                        }else{
                            ;
                        }
                    }
                    break;
                    
                case FM_STA_VAL_1:
                    if(keytype == FM_EVENT_VAL){
                        cmd->para[1] = key;
                        if(cmd->para_size == 2){
                            //printf("%s\n", cmd->name);
                            cmd->handler(cmd); 
                            sta = FM_STA_NON;
                        }else{
                            sta = FM_STA_VAL_2;
                        }
                    }else{
                        printf("bad format\n");
                        if(cmd->para_size == 0){
                            //printf("%s\n", cmd->name);
                            cmd->handler(cmd); //yes, do cmd 
                            sta = FM_STA_NON;
                        }else{
                            sta = FM_STA_CMD;
                        }
                    }
                    break;
                    
                case FM_STA_VAL_2:
                    if(keytype == FM_EVENT_VAL){
                        cmd->para[2] = key;
                        //printf("%s\n", cmd->name);
                        cmd->handler(cmd); //yes, do cmd
                        sta = FM_STA_NON;
                    }else{
                        printf("bad format\n");
                        if(cmd->para_size == 0){
                            //printf("%s\n", cmd->name);
                            cmd->handler(cmd); //yes, do cmd 
                            sta = FM_STA_NON;
                        }else{
                            sta = FM_STA_CMD;
                        }
                    }
                default:
                    break;
            }
        }
    }

    return 0;
}

