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

#include "atci_kpd_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/poll.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <dirent.h>
#include <errno.h>

#define DEV_INPUT_PATH	"/dev/input"

/* struct of support keys, you can add new keys into the struct */
struct keymap keymaps[] = { 	\
{KEY_POWER, "B"},	\
{KEY_HOME, "H"},	\
{KEY_BACK, "P"},	\
{KEY_VOLUMEUP, "U"},	\
{KEY_VOLUMEDOWN, "D"},	\
{KEY_CAMERA, "A"},	\
{KEY_MENU, "C"},	\
{KEY_SEND, "S"},	\
{KEY_END, "E"},		\
};

#define KEY_NUMBERS	9

extern int s_fdAtci_generic_command;

char gkpd_state = '0';
char fkpd_state = '0';
char buffer[22] = {'0','2',};	// AT%GKPD use this buffer. '0': AT%GKPD state , '2': date save from buffer[2]

static struct pollfd *ufds;
static char **device_names;
static int nfds = 0;

enum {
    PRINT_DEVICE_ERRORS     = 1U << 0,
    PRINT_DEVICE            = 1U << 1,
    PRINT_DEVICE_NAME       = 1U << 2,
    PRINT_DEVICE_INFO       = 1U << 3,
    PRINT_VERSION           = 1U << 4,
    PRINT_POSSIBLE_EVENTS   = 1U << 5,
};

#define sizeof_bit_array(bits)	((bits + 7) / 8)


static int open_device(const char *device, int print_flags)
{
    int version;
    int fd;
    struct pollfd *new_ufds;
    char **new_device_names;
    char name[80];
    char location[80];
    char idstr[80];
    struct input_id id;

    fd = open(device, O_RDWR);
    if(fd < 0) {
        if(print_flags & PRINT_DEVICE_ERRORS)
            fprintf(stderr, "could not open %s, %s\n", device, strerror(errno));
        return -1;
    }
    
    if(ioctl(fd, EVIOCGVERSION, &version)) {
                return -1;
    }
    if(ioctl(fd, EVIOCGID, &id)) {
                return -1;
    }
    name[sizeof(name) - 1] = '\0';
    location[sizeof(location) - 1] = '\0';
    idstr[sizeof(idstr) - 1] = '\0';
    if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
        name[0] = '\0';
    }
    if(ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1) {
        location[0] = '\0';
    }
    if(ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1) {
        idstr[0] = '\0';
    }

    new_ufds = realloc(ufds, sizeof(ufds[0]) * (nfds + 1));
    if(new_ufds == NULL) {
        return -1;
    }
    ufds = new_ufds;
    new_device_names = realloc(device_names, sizeof(device_names[0]) * (nfds + 1));
    if(new_device_names == NULL) {
        return -1;
    }
    device_names = new_device_names;

    ufds[nfds].fd = fd;
    ufds[nfds].events = POLLIN;
    device_names[nfds] = strdup(device);
    nfds++;

    return 0;
}

static int scan_dir(const char *dirname, int print_flags)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if(de->d_name[0] == '.' &&
           (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[2] == '\0')))
            continue;
        strcpy(filename, de->d_name);
        open_device(devname, print_flags);
    }
    closedir(dir);
    return 0;
}

int fkpd_cmd_handler(char *cmdline, ATOP_t at_op, char *response){
	
	char log_info[200] = {'\0'};
	char key_strings[20] = {'\0'};
	int time_delay = 0, pause_delay = 0,time_count = 0, i = 0, j = 0;
	int fds;
	struct input_event event;	
	unsigned char key_bitmask[sizeof_bit_array(KEY_MAX + 1)];    

	scan_dir(DEV_INPUT_PATH, 0);

	for(i = 0; i <= nfds-1; i++){
		memset(key_bitmask, 0, sizeof(key_bitmask));
		if(ioctl(ufds[i].fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) >= 0){
			fds = ufds[i].fd;
		}else
			close(ufds[i].fd);
		
	}
	
	event.type = 1;

	switch(at_op){
	        case AT_ACTION_OP: 	//AT%XXXX
        	case AT_READ_OP:	//AT%XXXX?
		{
			sprintf(log_info, "OK");
			break;
		}
	        case AT_TEST_OP:	//AT%XXXX=?
		{	
			sprintf(log_info, "AT\%FKPD=[Key string:");
			for(i = 0; i < KEY_NUMBERS-1; i++){	
				sprintf(log_info, "%s%c,", log_info, keymaps[i].name[0]);
			}
				sprintf(log_info, "%s%c],[Time:0~255],[Delay:0~255]\r\nOK", log_info, keymaps[i].name[0]);
			break;
		}
		case AT_SET_OP:		//AT%XXXX=...
		{
			
			i = 0;
			while(cmdline[i] != ',' && cmdline[i] != '\0'){
				key_strings[i] = cmdline[i];
				i++;
			}

			key_strings[i] = '\0';	
			if(cmdline[i] == ',')
				i++;
			else 
			time_delay = 0;			
			while(cmdline[i] >= '0' && cmdline[i] <= '9' && cmdline[i] != ','){
				time_delay = time_delay*10 + cmdline[i] - '0';
				i++;
			}

			if(cmdline[i] == ',')
				i++;

			pause_delay = 0;
			while(cmdline[i] >= '0' && cmdline[i] <= '9' && cmdline[i] != ','){
				pause_delay = pause_delay*10 + cmdline[i] -'0';
				i++;
			}

			i = 0;
			while(key_strings[i] != '\0'){
				for(j = 0; j < KEY_NUMBERS; j++){
					if(key_strings[i] == keymaps[j].name[0]){
						event.code = keymaps[j].code;
						event.value = 1;
						write(fds, &event, sizeof(event));
						
					}
				}	
				time_count = 1;
				do{
					usleep(100000);
					time_count += 1;
				}while(time_count < time_delay);
				for(j = 0; j < KEY_NUMBERS; j++){
					if(key_strings[i] == keymaps[j].name[0]){
						event.code = keymaps[j].code;
						event.value = 0;
						write(fds, &event, sizeof(event));
						
					}
				}	
				time_count = 1;
				do{
					usleep(100000);
					time_count +=1;
				}while(time_count < pause_delay);
				i++;
			}
				sprintf(log_info, "OK");

			break;
		}
		default:
		break;
	}

error:
	sprintf(response,"\r\n%s\r\n", log_info);
	close(fds);

	return 0;

}

int gkpd_cmd_handler(char *cmdline, ATOP_t at_op, char *response){
	
	int fd;
	char log_info[200] = {'\0'};
	int i = 1 ,j = 1, k = 0;
	struct input_event event;	
	struct pollfd *ufd;
	int nfd = 0;
	int ret;
	unsigned char key_bitmask[sizeof_bit_array(KEY_MAX + 1)];    
	int fd2;
	nfds = 0;

	scan_dir("/dev/input", 0);
	ufd = realloc(ufd, sizeof(struct pollfd));
	for(i = 0; i <= nfds-1; i++){
		memset(key_bitmask, 0, sizeof(key_bitmask));
		if(ioctl(ufds[i].fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) >= 0){
			ufd[0].fd = ufds[i].fd;
			nfd = nfd + 1;
		}else{
			close(ufds[i].fd);
		}
		
	}

	fd2 = open("/cache/test", O_RDWR|O_CREAT, 0666);		// first create a file to save date  
	close(fd2);
	switch(at_op){
	        case AT_ACTION_OP:
			fd2 = open("/cache/test", O_RDWR, 0666);
			read(fd2,  buffer, sizeof(buffer));
			close(fd2);	
			sprintf(log_info, "%c\r\nOK", buffer[0]);
			break;
        	case AT_READ_OP:
		{
			if(gkpd_state == '1'){						//when gkpd_state == '1' then print the buffer date.
				fd2 = open("/cache/test", O_RDWR, 0666);
				read(fd2, buffer, sizeof(buffer));
				close(fd2);
				if(buffer[1] -'0' == 22){
					j = 2;
				}else if(buffer[buffer[1] - '0'] == '\0'){
					j = 2;
				}else{
					j = buffer[1] - '0';
					sprintf(log_info, "%c",buffer[j]);
					j++;
				}
			if( j == 2 && buffer[2] == '\0'){
				sprintf(log_info, "OK");
				goto error;
			}
				while (j != buffer[1] - '0'){
					if( j == 22){
						j = 2;
					}
					sprintf(log_info, "%s%c", log_info, buffer[j]);
					j++;
				}
			sprintf(log_info, "%s\r\nOK", log_info);
			}else
				sprintf(log_info, "OK");
			
			break;
		}
	        case AT_TEST_OP:
		{
			for(i = 0; i < KEY_NUMBERS-1; i++){	
				sprintf(log_info, "%s%c,", log_info, keymaps[i].name[0]);
			}
			sprintf(log_info, "%s%c\r\nOK", log_info, keymaps[i].name[0]);
			break;	
		}
		case AT_SET_OP:
			if(cmdline[0] == '1' || cmdline[0] == '0'){
				fd2 = open("/cache/test", O_RDWR, 0666);
				read(fd2, buffer, sizeof(buffer));
				close(fd2);

				if(cmdline[0] == '1' && buffer[0] == '1'){
					memset(buffer, 0, sizeof(buffer));
					buffer[0] = '1';
					buffer[1] = '2';
					fd2 = open("/cache/test", O_RDWR, 0666);
					write(fd2, buffer, sizeof(buffer));
					close(fd2);
					sprintf(log_info, "OK");
					goto error;
				}
				
				if(cmdline[0] == '1' && buffer[0] == '0'){
					sprintf(log_info, "GKPD BEGIN\r\nOK");
					gkpd_state = cmdline[0];
					if(fork() == 0){					 //create a process for saving the the keys value.
						memset(buffer, 0, sizeof(buffer));
						buffer[0] = '1';
						buffer[1] = '2';
						memset(log_info, 0, sizeof(log_info));
						fd2 = open("/cache/test", O_RDWR, 0666);
						write(fd2,  buffer, sizeof(buffer));
						close(fd2);
					
						i = 0;
						while(1){
							ret = poll(ufds ,nfds , -1);
							fd2 = open("/cache/test", O_RDWR, 0666);
							read(fd2,  buffer, sizeof(buffer));
							close(fd2);
							if(buffer[0] == '0'){
								return 0;
							}
					
							if(ret == 0){
								sprintf(log_info, "%s\r\nOK", log_info);
							}
							for(k = 0; k < nfds; k++){
								if(ufds[k].revents & POLLIN){
									read(ufds[k].fd, &event, sizeof(event));
									for(j = 0; j < KEY_NUMBERS; j++){
										if(event.code == keymaps[j].code){
											if(i == 0){
												i++;
											}else{
												i = 0;
												buffer[buffer[1] - '0'] = keymaps[j].name[0];
												if(buffer[1] - '0' == 21){
													buffer[1] = '2';
												}else{
													buffer[1]++;
												}
											}
											event.code = 0;
											fd2 = open("/cache/test", O_RDWR, 0666);
											write(fd2,  buffer, sizeof(buffer));
											close(fd2);	
										}	
									}
									sprintf(log_info, "%s", log_info);
								}
							}
						}
						sprintf(log_info, "%s\r\nOK", buffer);
					}	

				}else{
					gkpd_state = cmdline[0];
					memset(buffer, 0, sizeof(buffer));
					buffer[0] = '0';
					buffer[1] = '2';
					fd2 = open("/cache/test", O_RDWR, 0666);
					write(fd2,  buffer, sizeof(buffer));
					close(fd2);	
					sprintf(log_info, "GKPD END\r\nOK");
				}

			}			
		if(cmdline[0] == '\0'){
			fd2 = open("/cache/test", O_RDWR, 0666);
			read(fd2,  buffer, sizeof(buffer));
			close(fd2);	
			sprintf(log_info, "%c\r\nOK", buffer[0]);
		}
			break;
	default:
			break;
	}

error:
	sprintf(response,"\r\n%s\r\n\r\n", log_info);
	free(ufd);
	return 0;

}
