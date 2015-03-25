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
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h> /* optind */
#include <fcntl.h>
#include <ctype.h>
#include <cutils/log.h>
#include <linux/dm-ioctl.h>
#include <linux/loop.h>
#include <linux/kdev_t.h>
#include <cutils/properties.h>
#include "cutils/properties.h"
#include "superumount.h"


#define ONE_FILE_NAME_LENG 20
#define FILE_LEVEL_HIGH 10
#define FILE_LEVEL_LOW 3
#define TEST_PATH "/sdcard/PathTest/"
#define LOOP_CLR_FD		0x4C01
#define UINT32 unsigned long
#define kal_uint32 unsigned long
#define DEVMAPPER_BUFFER_SIZE 4096
#define DM_DEV_REMOVE    _IOWR(DM_IOCTL, DM_DEV_REMOVE_CMD, struct dm_ioctl)
#define USB_DEVICE_PATH "/sys/class/android_usb/android0/enable"
#define CHKUMOUNT_XLOG_TAG "CHKUMOUNT_MTK"

struct mount_dir 
{
	char name[256];
};
const char *DEV_BLOCKDIR       = "/sys/devices/virtual/block/";
static const int LOOP_MAX = 4096;
int debug = 1;
int print_flag=1;

enum {
	DEBUG_PRINTFLOG = 0,
	DEBUG_ANDROIDLOG = 1,
	DEBUG_ALLLOG = 2,
};
#define my_printf(debug_level, ...)	\
({ 					\
	if(debug_level == 0){		\
		printf(__VA_ARGS__);}	\
	else if( debug_level == 1){	\
 		SLOGD("%s %d %s",__FILE__, __LINE__, __FUNCTION__);	\
		SLOGD(__VA_ARGS__);}	\
 	else if( debug_level == 2){	\
		SLOGD("%s %d %s",__FILE__, __LINE__, __FUNCTION__);	\
		SLOGD(__VA_ARGS__);	\
 		printf(__VA_ARGS__); 	\
 	}\
})
/*
 *
 *
 *	
 */
int readSymLink(const char *path, char *link, size_t max)
{
	struct stat s;
	int length;

	if (lstat(path, &s) < 0)
		return 0;
	if ((s.st_mode & S_IFMT) != S_IFLNK)
		return 0;

	// we have a symlink    
	length = readlink(path, link, max- 1);
	if (length <= 0) 
		return 0;
	link[length] = 0;
	return 1;
}

/*
 *
 *
 *	
 */
int pathMatchesMountPoint(const char* path, const char* mountPoint)
{
	int length = strlen(mountPoint);
	if (length > 1 && strncmp(path, mountPoint, length) == 0) 
	{
		// we need to do extra checking if mountPoint does not end in a '/'
		if (mountPoint[length - 1] == '/')
		    return 1;
		// if mountPoint does not have a trailing slash, we need to make sure
		// there is one in the path to avoid partial matches.
		return (path[length] == 0 || path[length] == '/');
	}
	return 0;
}
/*
 *
 *
 *	
 */
int checkSymLink(int pid, const char *mountPoint, const char *name)
{
	char    path[PATH_MAX];
	char    link[PATH_MAX];

	sprintf(path, "/proc/%d/%s", pid, name);
	if (readSymLink(path, link, sizeof(link)) && pathMatchesMountPoint(link, mountPoint)) 
		return 1;
	return 0;
}

/*
 *
 *
 *	
 */
int checkFileDescriptorSymLinks(int pid, const char *mountPoint, char *openFilename, size_t max) 
{
	// compute path to process's directory of open files
	char    path[PATH_MAX];
	sprintf(path, "/proc/%d/fd", pid);
	DIR *dir = opendir(path);
	if (!dir)
		return 0;

	// remember length of the path
	int parent_length = strlen(path);
	// append a trailing '/'
	path[parent_length++] = '/';

	struct dirent* de;
	while ((de = readdir(dir))) 
	{
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")
		        || strlen(de->d_name) + parent_length + 1 >= PATH_MAX)
			continue;

		// append the file name, after truncating to parent directory
		path[parent_length] = 0;
		strcat(path, de->d_name);

		char link[PATH_MAX];

		if (readSymLink(path, link, sizeof(link)) && pathMatchesMountPoint(link, mountPoint))
		{
			if (openFilename) 
			{
				memset(openFilename, 0, max);
				strncpy(openFilename, link, max-1);
			}
			closedir(dir);
			return 1;
		}
	}

	closedir(dir);
	return 0;
}


/*
 *
 *
 *	
 */
int checkFileMaps(int pid, const char *mountPoint, char *openFilename, size_t max)
{
	FILE *file;
	char buffer[PATH_MAX + 100];

	sprintf(buffer, "/proc/%d/maps", pid);
	file = fopen(buffer, "r");
	if (!file)
		return 0;

	while (fgets(buffer, sizeof(buffer), file))
	{
		// skip to the path
		const char* path = strchr(buffer, '/');
		if (path && pathMatchesMountPoint(path, mountPoint)) {
			if (openFilename) {
				memset(openFilename, 0, max);
				strncpy(openFilename, path, max-1);
			}
			fclose(file);
			return 1;
		}
	}
	fclose(file);
	return 0;
}

/*
 *
 *
 *	
 */
void getProcessName(int pid, char *buffer, size_t max)
{
	int fd;
	snprintf(buffer, max, "/proc/%d/cmdline", pid);
	fd = open(buffer, O_RDONLY);
	if (fd < 0) 
	{
		strcpy(buffer, "???");
	} else
	{
		int length = read(fd, buffer, max - 1);
		buffer[length] = 0;
		close(fd);
	}
}

/*
 *
 *
 *	
 */
int getPid(const  char *s)
{
	int result = 0;
	while (*s) 
	{
		if (!isdigit(*s)) 
			return -1;
		result = 10 * result + (*s++ - '0');
	}
	return result;
}
/*
 *
 *
 *	
 */
int CheckLoopDevHolders(const char *path)
{
	DIR *dir = opendir(path);
	if (!dir)
		return 0;

	// remember length of the path
	//int parent_length = strlen(path);
	// append a trailing '/'
	//path[parent_length++] = '/';

	struct dirent* de;
	while ((de = readdir(dir)))
	{
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;
		if( de->d_name != 0 )
		{
			if( debug )
				my_printf(print_flag,"  dir:%s\n", de->d_name);
		}
	}

	closedir(dir);
	return 0;
}
/*
 *
 *
 *	
 */
void CheckChildThread(int pid,const char *path)
{
	char thread_path[100];
	sprintf(thread_path, "/proc/%d/task", pid);
	DIR *thread_dir = opendir(thread_path);
	FILE *thread_file;
	char buffer_1[100];
	char buffer_2[100];
	int i_found=0;
	struct dirent* thread_de;
	char openfile[PATH_MAX];
	//static int i_child_thread_conter = 0;
	//struct timeval tv, tv2;  
	//unsigned long long start_utime, end_utime;  
	        
	if (!thread_dir)
	        return;

	//if (pid==1)
	//	i_child_thread_conter = 0;
	//gettimeofday(&tv,NULL);  
	//start_utime = tv.tv_sec * 1000000 + tv.tv_usec;  
	//printf("checkchildThread!!!\n");
	while ((thread_de = readdir(thread_dir))) 
	{
	        if (atoi(thread_de->d_name))
		{               
			//i_child_thread_conter = i_child_thread_conter + 1;
			//SLOGE("Ahsin child pid:%d  ",atoi(thread_de->d_name));
			if (pid==atoi(thread_de->d_name))
				continue;
			i_found = 0;
			if (checkFileDescriptorSymLinks(atoi(thread_de->d_name), path, openfile, sizeof(openfile))) 
			{
				i_found = 1;
				if( debug )
					my_printf(print_flag,"  Process (%d) has open file: %s\n", atoi(thread_de->d_name), openfile);
			}
			else if (checkFileMaps(atoi(thread_de->d_name), path, openfile, sizeof(openfile))) 
			{
				i_found = 1;
				if( debug )
					my_printf(print_flag,"  Process (%d) has filemap of %s\n", atoi(thread_de->d_name), openfile);
			} 
			else if (checkSymLink(atoi(thread_de->d_name), path, "cwd")) 
			{
				i_found = 1;
				if( debug )
					my_printf(print_flag,"  Process (%d) has cwd within %s\n", atoi(thread_de->d_name), path);
			} 
			else if (checkSymLink(atoi(thread_de->d_name), path, "root")) 
			{
				i_found = 1;
				if( debug )
					my_printf(print_flag,"  Process (%d) has chroot within %s\n", atoi(thread_de->d_name), path);
			} 
			else if (checkSymLink(atoi(thread_de->d_name), path, "exe")) 
			{
			        i_found = 1;
			        if( debug )
					my_printf(print_flag,"  Process (%d) has executable path within %s\n", atoi(thread_de->d_name), path);
			}
			if (i_found)
			{
			        //get process name & status
			        sprintf(buffer_1, "/proc/%d/status", atoi(thread_de->d_name));
			        thread_file = fopen(buffer_1, "r");

			        if (!thread_file)
					continue;
				fgets(buffer_2, sizeof(buffer_2), thread_file);
				fgets(buffer_1, sizeof(buffer_1), thread_file);
				fclose(thread_file);        
				//print backtrace
				sprintf(buffer_1, "/proc/%d/stack", atoi(thread_de->d_name));
				thread_file = fopen(buffer_1, "r");
				if (!thread_file)
					continue;

				while(!feof(thread_file))
				{
					fgets(buffer_2, sizeof(buffer_2), thread_file);
					if (!(strcmp("[<ffffffff>]",strtok(buffer_2," "))))
						break;
					else if( debug )
							my_printf(print_flag,"  %s",buffer_2);
				        
				}
				fclose(thread_file);
			}
		}
	}
	//gettimeofday(&tv2,NULL);  
	//end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;  
	//SLOGE("checkFileDescriptorSymLinks  runtime = %llu\n", end_utime - start_utime );
	//SLOGE("child_thread_conter ");        
	closedir(thread_dir);
}
/*
 *
 *
 *	
 */
static int get_dev_major_minor( const char *dev, int *major, int *minor)
{
	struct stat s;
	char linkto[256] = {0};
	int len;

	if(lstat(dev, &s) < 0)
	{
		if( debug )
			my_printf(print_flag,"  %s:lstat error\n", dev);
		return -1;
	}
	while( linkto[0] == 0)
	{
		if( (s.st_mode & S_IFMT) == S_IFCHR || (s.st_mode & S_IFMT) == S_IFBLK)
		{
			if( debug )
				my_printf(print_flag,"  major:%d minor:%d\n",(int) MAJOR(s.st_rdev), (int) MINOR(s.st_rdev));
			*major = (int) MAJOR(s.st_rdev);
			*minor = (int) MINOR(s.st_rdev);
			return 1;
		}
		else if( (s.st_mode & S_IFMT) == S_IFLNK )
		{
			len = readlink(dev, linkto, 256);
			if(len < 0)
			{
				if( debug )
					my_printf(print_flag,"  readlink error");
				return -1;
			}

			if(len > 255) {
				linkto[252] = '.';
				linkto[253] = '.';
				linkto[254] = '.';
				linkto[255] = 0;
				return -1;
			} else {
				linkto[len] = 0;
			}
			if( debug )
				my_printf(print_flag,"  linkto:%s\n",linkto);
		}else
		{
			if( debug )
				my_printf(print_flag,"  no major minor\n");
			return -1;
		}
		if(lstat(linkto, &s) < 0)
		{
			if( debug )
				my_printf(print_flag,"  %s:lstat error\n", dev);
			return -1;
		}
		linkto[0] = 0;
	}

	return 1;
        
}
int chkUnix(int *dst, const char* source)
{
	FILE *fp;
	char rd_line[256], *pch, buf[256], *token, *substr;
	int i=0, inode_token=0, length=0, counter = 0;
	sprintf(rd_line, "cat /proc/net/unix");
	if ((fp = popen(rd_line, "r")) == NULL)
	{
		if( debug )
				my_printf(print_flag,"could not find the dev file\n");
		return -1;
	}else
	{
		while(fgets(buf,   255,   fp)   !=   NULL)
		{
			pch = strtok(buf, "\n");
			token = strtok(pch, " ");
			i=0;
			while(token != NULL)
			{
				i++;
				if( i==7 )
				{
					inode_token = atoi(token);
					//printf("token:%s\n",token);
				}
				if( i==8 )
				{
					//printf("path:%s\n",token);
					substr = strstr(token, source);
					if( substr != NULL)
					{
			  			if( debug )
							my_printf(print_flag,"match:%d %s\n", inode_token, token);
			  			length++;
			  			if(dst)
			     				dst[counter++] = inode_token;
					}
				}
				token = strtok(NULL, " ");
			}
		}
		pclose(fp);
	}
	return length;
}
int checkSocketLink(int pid, const char *target, size_t max) 
{
   	FILE *fp;
   	char rd_line[256], *pch, buf[256], *token, *substr;
   	int i=0, inode_token;
   	char path[PATH_MAX];
   	int *socketchecker = 0;
   	int socketlength = 0;
   	int counter; 

	socketlength = chkUnix(0, target);
	if( socketlength < 0 )
		return 0;
	if( socketlength < 20)
		socketlength = 20;
   	socketchecker = (int *)malloc(sizeof(int)*socketlength);
   	//printf("len:%d\n", socketlength);
   	chkUnix(socketchecker, target);
   


    	// compute path to process's directory of open files
    
    	sprintf(path, "/proc/%d/fd", pid);
    	DIR *dir = opendir(path);
    	if (!dir)
        	return 0;
	
    	// remember length of the path
    	int parent_length = strlen(path);
    	// append a trailing '/'
    	path[parent_length++] = '/';

    	struct dirent* de;
    	while ((de = readdir(dir))) {
        	if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")
                	|| strlen(de->d_name) + parent_length + 1 >= PATH_MAX)
            	continue;
        
        	// append the file name, after truncating to parent directory
        	path[parent_length] = 0;
        	strcat(path, de->d_name);

        	char link[PATH_MAX];
	
        	if (readSymLink(path, link, sizeof(link))) {
           		//printf("path:%s link:%s\n", path, link);

           		if( strstr( link, "socket:"))
           		{
              			char *start = strstr(link, "[");
              			char *end = strstr(link, "]");
              			char tmppch[20];
              			//printf("%s %s %d\n", start, end, end-start);
              			strncpy(tmppch, start+1, end-start-1);
              			tmppch[end-start] = 0;
              			//printf("inode:%d socket:%d\n", atoi(tmppch), socketlength);
              			for( counter=0; counter<socketlength; counter++)
              			{
                  			if( atoi(tmppch) == socketchecker[counter])
                  			{
                     				if( debug )
							my_printf(print_flag,"  process:%d has socket bind at inode:%d\n",pid, atoi(tmppch) );
		     				return 1;
                  			}
              			}
              
           		}
           		/*
            		if (openFilename) {
                		memset(openFilename, 0, max);
                		strncpy(openFilename, link, max-1);
            		}*/
            		//closedir(dir);
            		//return 1;
        	}
    	}
	if( socketchecker != 0)
		free(socketchecker);
    	closedir(dir);
    
    	return 0;
}
/*
 *
 *
 *	@param[in] action	to indicate if it is check mode or kill mode
 *				@c 0=>check mode, 1=>kill mode
 */
void killProcessesWithOpenFiles(const char *path, int action)
{
	DIR*    dir;
	struct dirent* de;
	int counter = 0;

	if (!(dir = opendir("/proc")))
	{
		if( debug )
			my_printf(print_flag,"  opendir failed (%s)", strerror(errno));
		return;
	}

	while ((de = readdir(dir))) 
	{
		int killed = 0;
		int pid = getPid(de->d_name);
		char name[PATH_MAX];

		if (pid == -1)
			continue;
		getProcessName(pid, name, sizeof(name));

		char openfile[PATH_MAX];
		if( action == 0)
			CheckChildThread(pid, path);	
		if (checkFileDescriptorSymLinks(pid, path, openfile, sizeof(openfile))) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has open file %s\n", name, pid, openfile);
		} else if (checkFileMaps(pid, path, openfile, sizeof(openfile))) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has open filemap for %s\n", name, pid, openfile);
		} else if (checkSymLink(pid, path, "cwd")) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has cwd within %s\n", name, pid, path);
		} else if (checkSymLink(pid, path, "root")) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has chroot within %s\n", name, pid, path);
		} else if (checkSymLink(pid, path, "exe")) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has executable path within %s\n", name, pid, path);
		} else if (checkSocketLink(pid, path, 0)) {
			if( debug )
				my_printf(print_flag,"  Process %s (%d) has socket within %s\n", name, pid, path);
		} else {
			continue;
		}

		if(!strcmp(name, "/sbin/adbd"))     {
			int fd;
			size_t s;

			fd = open(USB_DEVICE_PATH, O_WRONLY);
			if (fd >= 0) {
				if( debug )
					my_printf(print_flag, "  enable=0");
				write(fd, "0", 1);
				close(fd);
			} else {
				if( debug )
					my_printf(print_flag, "  Fail to open %s", USB_DEVICE_PATH);
			}
		}
		//send the signal
		{
			if (action == 1) {
				if( debug )
					my_printf(print_flag,"  Sending SIGHUP to process %d\n", pid);
				kill(pid, SIGTERM);
			} else if (action == 2) {
				if( debug )
					my_printf(print_flag,"  Sending SIGKILL to process %d\n", pid);
				kill(pid, SIGKILL);
			}
		}




		if(!strcmp(name, "/sbin/adbd"))     {
			int fd;
			size_t s;

			fd = open(USB_DEVICE_PATH, O_WRONLY);
			if (fd >= 0)
			{
				if( debug )
					my_printf(print_flag, "  enable=1");
				write(fd, "1", 1);
				close(fd);
			} else
			{
				if( debug )
					my_printf(print_flag,"  Fail to open %s", USB_DEVICE_PATH);
			}
		}
	}
	closedir(dir);
}
/*
 *
 *
 *	
 */
int doUnmount(const char *path, int force)
{
	int retries = 5;
	while (retries--) 
	{
		if (!umount(path))
		{
			if( debug )
				my_printf(print_flag,"super_umount_action %s sucessfully unmounted\n", path);
			return 0;
		}

		int action = 0;

		if (force)
		{
			if (retries == 1) 
			{
				action = 2; // SIGKILL
			} else if (retries == 2) 
			{
				action = 1; // SIGHUP
			}
		}

		if( debug )
			my_printf(print_flag,"  Failed to unmount %s (, retries %d, action %d)\n",
		        path,  retries, action);

		killProcessesWithOpenFiles(path, action);
		usleep(1000*1000);
	}
	if( debug )
		my_printf(print_flag,"  Giving up on unmount %s\n", path);
	return -1;
}

/*
 *
 *
 *	
 */
int destroyByDevice(const char *loopDevice)
{
	int device_fd;

	device_fd = open(loopDevice, O_RDONLY);
	if (device_fd < 0)
	{
		if( debug )
			my_printf(print_flag,"  Failed to open loop (%d)\n", errno);
		return -1;
	}

	if (ioctl(device_fd, LOOP_CLR_FD, 0) < 0) 
	{
		if( debug )
			my_printf(print_flag,"  Failed to destroy loop (%d)\n", errno);
		close(device_fd);
		return -1;
	}
	if( debug )
		my_printf(print_flag,"super_umount_action destroy Loop device %s\n", loopDevice);
	close(device_fd);
	return 0;
}
/*
 *
 *
 *	
 */
void ioctlInit(struct dm_ioctl *io, size_t dataSize,
                          const char *name, unsigned flags)
{
	memset(io, 0, dataSize);
	io->data_size = dataSize;
	io->data_start = sizeof(struct dm_ioctl);
	io->version[0] = 4;
	io->version[1] = 0;
	io->version[2] = 0;
	io->flags = flags;
	if (name) {
		int ret = strlcpy(io->name, name, sizeof(io->name));
		if (ret >= (int)sizeof(io->name))
			abort();
	}
}

/*
 *
 *
 *
 *	@param[in]	skipLoopNum the loop number to skipped. 
 *			@c -1=>no need to skip, others=>the skipped matched loop number
 *
 *	@param[out]	skipLoopNum the matched loop number with the device
 *	
 *	@return[out]	@c 1=>if find matched, others=>if not matched 
 */
int lookupLoopdevice(const char *dev, char *major_minor_string, char *link, int *skipLoopNum) 
{
	int i;
	FILE* fp;
	char rd_line[256];
	char *pch;
	char buf[255] = {0};

	//memset(buffer, 0, len);

	for (i = 0; i < LOOP_MAX; i++) {
		int rc;
		DIR  *dip;
		sprintf(rd_line, "%sloop%d/loop", DEV_BLOCKDIR, i);
		if ((dip = opendir(rd_line)) == NULL)   
		{               
			if( debug )
				my_printf(print_flag,"  not a directory\n");
			break;
		}else
		{
			closedir(dip);
			if( major_minor_string )
			{
				sprintf(rd_line, "cat %sloop%d/dev", DEV_BLOCKDIR, i);
				if ((fp = popen(rd_line, "r")) == NULL)
				{
					if( debug )
						my_printf(print_flag,"  could not find the dev file\n");
					return -1;
				}else
				{
					if   (fgets(buf,   255,   fp)   !=   NULL)
					{
						pch = strtok(buf, "\n");
						if( debug )
							my_printf(print_flag,"  Loopdev:%s:%s\n", pch, major_minor_string);
						if( strcmp( major_minor_string, pch) )
						{
							pclose(fp);
							continue;
						}
					}
					pclose(fp);
				}
				
			}
		}

		sprintf(rd_line, "cat %sloop%d/loop/backing_file", DEV_BLOCKDIR, i);

		if ((fp = popen(rd_line, "r")) == NULL)
		{
			if( debug )
				my_printf(print_flag,"  could not find backing file\n");
			continue;
		}else
		{
			while   (fgets(buf,   255,   fp)   !=   NULL)
			{
				pch = strtok(buf," ");
				while (pch != NULL)
				{
					strcpy(link, pch);
					pch = strtok (NULL, " ");
				}   
			}
			
		}      
		pclose(fp);

		if( link[0] == 0 )
			return -1;
		else
		{
			link = strtok(link, " \n");
			if( debug )
				my_printf(print_flag, "  scan filename:%s dev:%s\n", link, dev);
			//match
			if( !strcmp( link, dev) || strstr( link, dev))
			{
				if( debug )
					my_printf(print_flag, "super_umount_check loop device filename:%s dev:%s skip:%d\n", link, dev, *skipLoopNum);
				if( *skipLoopNum < i)
				{
					*skipLoopNum = i;	
					return 1;
				}
			}
		}
	}

	if (i == LOOP_MAX)
	{
		errno = ENOENT;
		return -1;
	}
	// strncpy(buffer, filename, len -1);
	return 0;
}

/*
 *
 *
 *	
 */
int get_dm_table(int major, int minor, char *mount_dir, int action)
{
	char *pch, src[20], loopdevice[256];
	int round=0, dm_major=0, dm_minor=0, loop_number=-1, dm_dev;
	char *buffer = (char *) malloc(1024*64);
	if (!buffer)
	{
		if( debug )
			my_printf(print_flag,"  Error allocating memory (%s)", strerror(errno));
		return -1;
	}

	int fd;
	if ((fd = open("/dev/device-mapper", O_RDWR)) < 0) {
		if( debug )
			my_printf(print_flag,"  Error opening devmapper (%s)", strerror(errno));
		free(buffer);
		return -1;
	}

	struct dm_ioctl *io = (struct dm_ioctl *) buffer;
	ioctlInit(io, 1024*64, NULL, 0);
	// Create the DM device
	//ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);

	if (ioctl(fd, DM_LIST_DEVICES, io))
	{
		if (errno != ENXIO) {
			if( debug )
				my_printf(print_flag,"  Error get table (%s)", strerror(errno));
		}
			free(buffer);
			close(fd);
			return -1;
	}
	struct dm_name_list *n = (struct dm_name_list *) (((char *) buffer) + io->data_start);
	if (!n->dev)
	{
		free(buffer);

		close(fd);
		return 0;
	}
	char *buffer2 = (char *) malloc(1024*64);
	if (!buffer2)
	{
		if( debug )
			my_printf(print_flag,"  Error allocating memory (%s)", strerror(errno));
		free(buffer);
		return -1;
	}

	unsigned nxt = 0;
	do {
		n = (struct dm_name_list *) (((char *) n) + nxt);

		memset(buffer2, 0, 1024*64);
		struct dm_ioctl *io2 = (struct dm_ioctl *) buffer2;
		//io2->flags |= DM_QUERY_INACTIVE_TABLE_FLAG;
		ioctlInit(io2, DEVMAPPER_BUFFER_SIZE, n->name, DM_STATUS_TABLE_FLAG);
		if (ioctl(fd, DM_TABLE_STATUS, io2))
		{
			if (errno != ENXIO) {
				if( debug )
					my_printf(print_flag,"  DM_DEV_STATUS ioctl failed (%s)", strerror(errno));
			}
			io2 = NULL;
		}

		char *tmp;
		if (!io2) {
			if( debug )
				my_printf(print_flag,"  %s %llu:%llu (no status available)", n->name, MAJOR(n->dev), MINOR(n->dev));
		} else {
			struct dm_target_spec *tgt;
		    	if( debug )
				my_printf(print_flag,"  %s %llu:%llu %d %d 0x%.8x %llu:%llu\n", n->name, MAJOR(n->dev),
		            MINOR(n->dev), io2->target_count, io2->data_size, io2->flags, MAJOR(io2->dev),
		                    MINOR(io2->dev));
			tgt = (struct dm_target_spec *) ((char *)io2 + sizeof(struct dm_ioctl));
			tmp = ((char *)tgt+sizeof(*tgt));
			
			if( debug )
				my_printf(print_flag,"  target:%s %s\n", tgt->target_type, tmp);
			//only support crypt mode and linear mode now
			if( !strcmp(tgt->target_type, "crypt") )
			{
				dm_dev = 4;
			}
			else
			{//linear mode
				dm_dev = 1;
			}
			pch = strtok(tmp, " ");
			round = 0;
		    	while (pch != NULL)
			{
				round++;
				if( round == dm_dev )
				{
					sprintf(src, "%d:%d", major, minor);
					//check if it is the target block device
					if( debug )
							my_printf(print_flag,"super_umount_check dm0 %s dev:%s\n", src, pch);
					if( !strcmp( src, pch) )
					{
						if( debug )
							my_printf(print_flag,"super_umount_check dm1 major:%llu minor:%llu input major:%d minor:%d matched dev:%s\n", MAJOR(io2->dev), MINOR(io2->dev), major, minor, pch);
						get_dm_table(MAJOR(io2->dev),MINOR(io2->dev),0,action);
						if( action )
						{
							//umount dm device
							umount_dev( 0, MAJOR(io2->dev), MINOR(io2->dev));
							//destroy device
							destroyDM(n->name);
						}
					}
					
					//check if the loopdevice is correnspond to the block device
					if( mount_dir!=0 && lookupLoopdevice(mount_dir, pch, (char *)loopdevice, &loop_number)==1 )
					{
						if( debug )
							my_printf(print_flag,"super_umount_check dm2 major:%llu minor:%llu input major:%d minor:%d matched dev:%s\n", MAJOR(io2->dev), MINOR(io2->dev), major, minor, pch);
						get_dm_table(MAJOR(io2->dev),MINOR(io2->dev),0,action);
						if( action )
						{
							//umount dm device
							umount_dev( 0, MAJOR(io2->dev), MINOR(io2->dev));
							//destroy device
							destroyDM(n->name);
						}
					}
				}
				pch = strtok (NULL, " ");
			}
		}
		//c->sendMsg(0, tmp, false);
		//free(tmp);
		nxt = n->next;
	} while (nxt);
	free(buffer2);
	free(buffer);
	close(fd);
	return 0;
}



/*
 *
 *
 *	
 */
int lookupActive(const char *dev)
{
	int i;
	int fd;
	char filename[256];

	//memset(buffer, 0, len);

	for (i = 0; i < LOOP_MAX; i++)
	{
		struct loop_info64 li;
		int rc;

		sprintf(filename, "/dev/block/loop%d", i);

		if ((fd = open(filename, O_RDWR)) < 0)
		{
			if (errno != ENOENT)
			{
				if( debug )
					my_printf(print_flag,"  Unable to open %s (%s)", filename, strerror(errno));
			}else
			{
				continue;
			}
			return -1;
		}

		rc = ioctl(fd, LOOP_GET_STATUS64, &li);
		close(fd);
		if (rc < 0 && errno == ENXIO) {
			continue;
		}

		if (rc < 0) {
			if( debug )
				my_printf(print_flag,"  Unable to get loop status for %s (%s)", filename,
				strerror(errno));
			return -1;
		}
		if( debug )
			my_printf(print_flag,"  crypt:%s name:%s\n", li.lo_crypt_name, li.lo_file_name);
		sprintf(filename, "/sys/block/loop%d/holders", i);
		CheckLoopDevHolders(filename);
		//get_dm_table(0,0, "");
		//if (!strncmp((const char*) li.lo_crypt_name, id, LO_NAME_SIZE)) {
		//    break;
		//}
	}

	if (i == LOOP_MAX) {
		errno = ENOENT;
		return -1;
	}
	// strncpy(buffer, filename, len -1);
	return 0;
}

/*
 *
 *
 *	
 */
static int chk_mount(const char *arg, char *dir)
{
	FILE *f;
	char mount_dev[256];
	char mount_dir[256];
	char mount_type[256];
	char mount_opts[256];
	int mount_freq;
	int mount_passno;
	int match;
	char rd_line[128];
	char buf[255] = {0};
	int major=0, minor=0, mount_major=-1, mount_minor=-1;

	/**
	 **	parse the mounts to iterate all the mount points
	 **
	 **/
	f = fopen("/proc/mounts", "r");
	if (!f) {
		if( debug )
			my_printf(print_flag,"  could not open /proc/mounts\n");
		return -1;
	}
	do {
		match = fscanf(f, "%255s %255s %255s %255s %d %d\n",
		mount_dev, mount_dir, mount_type,
		mount_opts, &mount_freq, &mount_passno);
		
		printf("mount_dev:%s\n", mount_dev);
		mount_dev[255] = 0;
		mount_dir[255] = 0;
		mount_type[255] = 0;
		mount_opts[255] = 0;
		if (match == 6 && strcmp(arg, mount_dev) == 0 ) 
		{
			//*dir = strdup(mount_dir);
			strcpy(dir, mount_dir);
			fclose(f);
			lookupActive(dir);
			return 1;
		}
		//check the major & minor number
		if (match == 6 && get_dev_major_minor( mount_dev, &mount_major, &mount_minor) == 1 &&
		major == mount_major && minor == mount_minor) 
		{
			//*dir = strdup(mount_dir);
			strcpy(dir, mount_dir);
			fclose(f);
			lookupActive(dir);
			return 1;
		}
	} while (match != EOF);

	fclose(f);
	return -1;
}
/*
 *
 *
 *	
 */
int destroyDM(const char *name)
{
	char *buffer = (char *) malloc(DEVMAPPER_BUFFER_SIZE);
	if (!buffer) {
		if( debug )
			my_printf(print_flag,"  Error allocating memory (%s)", strerror(errno));
		return -1;
	}

	int fd;
	if ((fd = open("/dev/device-mapper", O_RDWR)) < 0) {
		if( debug )
			my_printf(print_flag,"  Error opening devmapper (%s)", strerror(errno));
		free(buffer);
		return -1;
	}

	struct dm_ioctl *io = (struct dm_ioctl *) buffer;

	// Create the DM device
	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);

	if (ioctl(fd, DM_DEV_REMOVE, io))
	{
		if (errno != ENXIO) {
			if( debug )
				my_printf(print_flag,"  Error destroying device mapping (%s)", strerror(errno));
		}
		free(buffer);
		close(fd);
		return -1;
	}
	if( debug )
		my_printf(print_flag,"super_umount_action destroy DM %s\n", name);
	free(buffer);
	close(fd);
	return 0;
}



/*
 *
 *
 *	
 */
int umount_dev(const char *srcdev, int src_major, int src_minor)
{
	FILE *f;
	char mount_dev[256];
	char mount_dir[256];
	char mount_type[256];
	char mount_opts[256];
	int mount_freq;
	int mount_passno;
	int match;
	char rd_line[128];
	char buf[255] = {0};
	int major=0, minor=0, mount_major=-1, mount_minor=-1;

	if( srcdev != 0 )
		get_dev_major_minor(srcdev, &major, &minor);
	else
	{
		major = src_major;
		minor = src_minor;
	}
	
	/**
	 **	parse the mounts to iterate all the mount points
	 **
	 **/
	f = fopen("/proc/mounts", "r");
	if (!f) {
		if( debug )
			my_printf(print_flag,"  could not open /proc/mounts\n");
		return -1;
	}
	do {
		match = fscanf(f, "%255s %255s %255s %255s %d %d\n",
		mount_dev, mount_dir, mount_type,
		mount_opts, &mount_freq, &mount_passno);
		
		printf("mount_dev:%s\n", mount_dev);
		mount_dev[255] = 0;
		mount_dir[255] = 0;
		mount_type[255] = 0;
		mount_opts[255] = 0;
		if (match == 6 && srcdev !=0 && strcmp(srcdev, mount_dir) == 0 ) 
		{
			doUnmount(mount_dir, 1);
		}
		//check the major & minor number
        else if (match == 6 && get_dev_major_minor( mount_dev, &mount_major, &mount_minor) == 1 &&
		major == mount_major && minor == mount_minor) 
		{
			doUnmount(mount_dir, 1);
		}
	} while (match != EOF);

	fclose(f);
	if( debug )
		my_printf(print_flag,"  unmount dev\n");
	return 1;
}


/*
 *
 *
 *	
 */

int umount_subdir(char *src_dir, int *major, int *minor)
{
	FILE *f;
	char mount_dev[256];
	char mount_dir[256];
	char mount_type[256];
	char mount_opts[256];
	int mount_freq;
	int mount_passno;
	int match;
	char buf[512] = {0};
	int mount_major=-1, mount_minor=-1;
	int ret=0;

	/**
	 **	parse the mounts to iterate all the mount points
	 **
	 **/
	sprintf(buf, "%s/", src_dir);
	f = fopen("/proc/mounts", "r");
	if (!f) {
		if( debug )
			my_printf(print_flag,"  could not open /proc/mounts\n");
		return -1;
	}
	do {
		match = fscanf(f, "%255s %255s %255s %255s %d %d\n",
		mount_dev, mount_dir, mount_type,
		mount_opts, &mount_freq, &mount_passno);
		
		//printf("mount_dev:%s\n", mount_dev);
		mount_dev[255] = 0;
		mount_dir[255] = 0;
		mount_type[255] = 0;
		mount_opts[255] = 0;
		if (match == 6 && strstr( mount_dir, buf) ) 
		{
			super_umount( mount_dir);
		}
        else if( match == 6 && !strcmp( mount_dir, src_dir) )
		{
			if( major && minor)
			{
				get_dev_major_minor(mount_dev, major, minor);
				if( debug )
					my_printf(print_flag,"  umount_subdir mount_dev:%s major:%d minor:%d\n", mount_dev, *major, *minor);
			}
		}
		if( match == 6)
			ret++;	
	} while (match != EOF);

	fclose(f);
	return ret;
}
/*
 *
 *
 *	
 */
int get_matched_mountdir(struct mount_dir *dst_dir, int major, int minor)
{
	FILE *f;
	char mount_dev[256];
	char mount_dir[256];
	char mount_type[256];
	char mount_opts[256];
	int mount_freq;
	int mount_passno;
	int match;

	int mount_major=-1, mount_minor=-1;
	int ret = 0;

	/**
	 **	parse the mounts to iterate all the mount points
	 **
	 **/

	f = fopen("/proc/mounts", "r");
	if (!f) {
		if( debug )
			my_printf(print_flag,"  could not open /proc/mounts\n");
		return -1;
	}
	do {
		match = fscanf(f, "%255s %255s %255s %255s %d %d\n",
		mount_dev, mount_dir, mount_type,
		mount_opts, &mount_freq, &mount_passno);
		
		mount_dev[255] = 0;
		mount_dir[255] = 0;
		mount_type[255] = 0;
		mount_opts[255] = 0;
		if (match == 6 ) 
		{
			get_dev_major_minor(mount_dev, &mount_major, &mount_minor);
			if( mount_major == major && mount_minor == minor)
			{
				strcpy(dst_dir->name, mount_dir);
				dst_dir++;
				ret++;
				mount_major = mount_minor = -1;
			}
		}	
	} while (match != EOF);

	fclose(f);
	return ret;
}
/*
 *
 *
 *	
 */
void super_umount(char *src_dir)
{
	char dir[256], path[256];
	char *umount_target;
	char *pch;
	int mount_major=0, mount_minor=0;
	int totalmount_cnt=0, need_unmount_target=0;
	struct mount_dir *mountlist, *head_mountlist;
	int matched_mount_cnt=0, loopnumber=-1;
	int ch, option=0, fd=-1, i, loopret=0;
	
	if( debug )
		my_printf(print_flag,"super_umount %s\n", src_dir);
	umount_target = src_dir;
	pch = strrchr(umount_target,'/');
	
	if( (pch - umount_target+1) == (int)strlen(umount_target) )
		umount_target[strlen(umount_target)-1] = '\0';
	/*
	 *	check if the target mount dir has sub directory is mounted
	 *	if so, umount the subdirectory first
	 *	get the major minor of the related device
	 */
	totalmount_cnt = umount_subdir(umount_target, &mount_major, &mount_minor);
	if( debug )
		my_printf(print_flag,"total mount count:%d\n", totalmount_cnt);
	mountlist = (struct mount_dir *)malloc(totalmount_cnt*sizeof(struct mount_dir));
	head_mountlist = mountlist;
	matched_mount_cnt = get_matched_mountdir( mountlist, mount_major, mount_minor);
	if( debug )
		my_printf(print_flag,"matched mount count:%d\n", matched_mount_cnt);
	
	/*
	 *
	 *
	 *
	 */
	mountlist = head_mountlist;
	for( i=0; i< matched_mount_cnt; i++,mountlist++)
	{
		get_dm_table( mount_major, mount_minor, mountlist->name, 1);	
	}
	/*
	 *	check the block device list 
	 *	destroy the block device
	 *
	 *
	 */
	mountlist = head_mountlist;
	for( i=0; i< matched_mount_cnt; i++,mountlist++)
	{
		loopret = lookupLoopdevice(mountlist->name, 0, (char *)dir, &loopnumber);
		if( loopret != 1)
		    continue;
		sprintf(path, "/dev/block/loop%d", loopnumber);
		if( debug )
			my_printf(print_flag,"matched loop:%s path:%s\n", dir, path);
		destroyByDevice(path);
	}
	/*
	 *	umount the target
	 *
	 *
	 */
	mountlist = head_mountlist;
	for( i=0; i< matched_mount_cnt; i++,mountlist++)
	{
		if( !strcmp(mountlist->name, umount_target) )
		{
			need_unmount_target = 1;
			continue;
		}
		doUnmount(mountlist->name, 1);
	}
	if( need_unmount_target == 1)
		doUnmount(umount_target, 1);
	free(head_mountlist);
	if( matched_mount_cnt == 0)
	    doUnmount(umount_target, 1);
}
/*
 *
 *
 *	
 */
int main(int argc, char **argv)
{
	static const char opts[] = "apbcdulmnrf";
	char dir[256], path[256];
	char *umount_target;
	char *pch;
	int mount_major=0, mount_minor=0;
	int totalmount_cnt=0, need_unmount_target=0;
	struct mount_dir *mountlist, *head_mountlist;
	int matched_mount_cnt=0, loopnumber=-1, loopret=0;
	int ch, option=0, fd=-1, i, ret=0, stopframework=0;
  
	while ((ch = getopt(argc, argv, opts)) != -1)
	{
		switch (ch) {
			case 'a':
	    			print_flag = 1;
	    			break;
			case 'p':
	    			print_flag = 0;
	    			break;
			case 'b':
	    			print_flag = 2;
	    			break;
			case 'c':
	    			option = 1;
	    			break;
			case 'u':
	    			option = 2;
	    			break;
			case 'l'://check loop device
	    			option = 3;
	    			break;
			case 'm'://check dm table
	    			option = 4;
	    			break;
			case 'n':
				option = 5;
				break;
			case 'd'://destroy dm table
				option = 6;
				break;
			case 'r':
				debug = 0;
				break;
			case 'f':
				stopframework = 1;
				break;	
			default:
	    			usage();
		}
	}
	if( !debug )
	{
 		fd = open( "/dev/null", O_RDWR );
    		/*
    	 	* handle failure of open() somehow
    	 	*/
   	 	//dup2( fd, 0 );//in
   	 	dup2( fd, 1 );//out
   	 	dup2( fd, 2 );//error
	}
	
	argc -= optind;
	if (argc < 1 || argc > 3)
		usage();
	
	umount_target = argv[optind];

	if( stopframework == 1)
	{
		property_set("vold.decrypt", "trigger_reset_main");
		property_set("vold.decrypt", "trigger_stop_modem_and_default");
	}
	//normal umount process 
	//it will kill the dm => loop device => unmount
  	
	if( option == 1)
	{
		
		killProcessesWithOpenFiles(umount_target, 0);
		goto exit;
	}
	if( option == 2)
	{
		
		doUnmount(umount_target, 1);
		goto exit;
	}
	if( option == 3)//loop device
	{
		pch = strrchr(umount_target,'/');
		
		if( (pch - umount_target+1) == (int)strlen(umount_target) )
			umount_target[strlen(umount_target)-1] = '\0';
		/*
		 *	check if the target mount dir has sub directory is mounted
		 *	if so, umount the subdirectory first
		 *	get the major minor of the related device
		 */
		totalmount_cnt = umount_subdir(umount_target, &mount_major, &mount_minor);
		if( debug )
			my_printf(print_flag,"total mount count:%d\n", totalmount_cnt);
		if( totalmount_cnt == 0)
		{
			return -2;
		}
		mountlist = (struct mount_dir *)malloc(totalmount_cnt*sizeof(struct mount_dir));
		head_mountlist = mountlist;
		matched_mount_cnt = get_matched_mountdir( mountlist, mount_major, mount_minor);
		if( debug )
			my_printf(print_flag,"matched mount count:%d\n", matched_mount_cnt);
		if( matched_mount_cnt == 0)
		{
			free(head_mountlist);
			return -3;
		}
		/*
		 *	check the block device list 
		 *	destroy the block device
		 *
		 *
		 */
		mountlist = head_mountlist;
		for( i=0; i< matched_mount_cnt; i++,mountlist++)
		{
			loopret = lookupLoopdevice(mountlist->name, 0, (char *)dir, &loopnumber);
			
			if( loopret != 1)
			    continue;
			sprintf(path, "/dev/block/loop%d", loopnumber);
			if( debug )
				my_printf(print_flag,"matched loop:%s path:%s\n", dir, path);
		}
		free(head_mountlist);
		goto exit;
	}
	if( option == 4)//dm table
	{
		pch = strrchr(umount_target,'/');
		
		if( (pch - umount_target+1) == (int)strlen(umount_target) )
			umount_target[strlen(umount_target)-1] = '\0';
		/*
		 *	check if the target mount dir has sub directory is mounted
		 *	if so, umount the subdirectory first
		 *	get the major minor of the related device
		 */
		totalmount_cnt = umount_subdir(umount_target, &mount_major, &mount_minor);
		if( debug )
			my_printf(print_flag,"total mount count:%d\n", totalmount_cnt);
		if( totalmount_cnt == 0)
		{
			return -2;
		}
		mountlist = (struct mount_dir *)malloc(totalmount_cnt*sizeof(struct mount_dir));
		head_mountlist = mountlist;
		matched_mount_cnt = get_matched_mountdir( mountlist, mount_major, mount_minor);
		if( debug )
			my_printf(print_flag,"matched mount count:%d\n", matched_mount_cnt);
		if( matched_mount_cnt == 0)
		{
			free(head_mountlist);
			return -3;
		}
		/*
		 *
		 *
		 *
		 */
		mountlist = head_mountlist;
		for( i=0; i< matched_mount_cnt; i++,mountlist++)
		{
			get_dm_table( mount_major, mount_minor, mountlist->name, 0);	
		}
		free(head_mountlist);
		goto exit;
	}
	if( option == 5)
	{//for test only
		dmcreate(argv[optind+1],umount_target,"ss7",10240,path, sizeof(path), atoi(argv[optind+2]));
		if( debug )
			my_printf(print_flag,"dm:%s\n", path);
		goto exit;
	}
	if( option == 6)
	{//for test only
		get_dm_table( atoi(argv[optind]), atoi(argv[optind+1]), mountlist->name, atoi(argv[optind+2]));
		goto exit;
	}
	if( option == 0)
	{
		
		pch = strrchr(umount_target,'/');
		
		if( (pch - umount_target+1) == (int)strlen(umount_target) )
			umount_target[strlen(umount_target)-1] = '\0';
		/*
		 *	check if the target mount dir has sub directory is mounted
		 *	if so, umount the subdirectory first
		 *	get the major minor of the related device
		 */
		totalmount_cnt = umount_subdir(umount_target, &mount_major, &mount_minor);
		if( debug )
			my_printf(print_flag,"total mount count:%d\n", totalmount_cnt);
		if( totalmount_cnt == 0)
		{
			return -2;
		}
		mountlist = (struct mount_dir *)malloc(totalmount_cnt*sizeof(struct mount_dir));
		head_mountlist = mountlist;
		matched_mount_cnt = get_matched_mountdir( mountlist, mount_major, mount_minor);
		if( debug )
			my_printf(print_flag,"matched mount count:%d\n", matched_mount_cnt);
		if( matched_mount_cnt == 0)
		{
			free(head_mountlist);
			return -3;
		}
		/*
		 *	check the dm table
		 *
		 *
		 */
		mountlist = head_mountlist;
		for( i=0; i< matched_mount_cnt; i++,mountlist++)
		{
			get_dm_table( mount_major, mount_minor, mountlist->name, 1);	
		}
		/*
		 *	check the block device list 
		 *	destroy the block device
		 *
		 *
		 */
		mountlist = head_mountlist;
		for( i=0; i< matched_mount_cnt; i++,mountlist++)
		{
			loopret = lookupLoopdevice(mountlist->name, 0, (char *)dir, &loopnumber);
			
			if( loopret != 1)
			    continue;
			sprintf(path, "/dev/block/loop%d", loopnumber);
			if( debug )
				my_printf(print_flag,"matched loop:%s path:%s\n", dir, path);
			destroyByDevice(path);
		}
		/*
		 *	umount the target
		 *
		 *
		 */
		mountlist = head_mountlist;
		for( i=0; i< matched_mount_cnt; i++,mountlist++)
		{
			if( !strcmp(mountlist->name, umount_target) )
			{
				need_unmount_target = 1;
				continue;
			}
			doUnmount(mountlist->name, 1);
		}
		if( need_unmount_target == 1)
			ret = doUnmount(umount_target, 1);
		free(head_mountlist);
		if( ret == -1)
			return ret;
		if( debug )
			my_printf(print_flag,"umount %s finish\n", umount_target);
		
	}
exit:
  	if( !debug )
	{
  		if ( fd > 2 )
		{
			close( fd );
		}
	}
	if( debug )
		my_printf(print_flag,"finish\n");
  	return 0;
  
  
}

/*
 * Print usage message.
 */
static void
usage(void)
{
	fprintf(stderr,
	    "usage: chkumount [ -options ] target\n"
	    "where the options are:\n"
	    "\t-a use LOGE\n"
	    "\t-b use LOGE&printf\n"
	    "\t-p use printf\n"
	    "\t-c check the process & child thread\n"
	    "\t-l check loop device\n"
	    "\t-m check device mapper\n"
	    "\t-r no debug message\n"
	    "\t-n create dm device\n"
	    "\t-f stop framework and modem\n"
	    "\t-u just umount the target, no need to check the subdir and dm\n");
	exit(1);
}

void *_align(void *ptr, unsigned int a)
{
        register unsigned long agn = --a;

        return (void *) (((unsigned long) ptr + agn) & ~agn);
}


int dmcreate(const char *name, const char *loopFile, const char *key,
                      unsigned int numSectors, char *ubuffer, size_t len, int dmtype)
{
	
	char *buffer = (char *) malloc(DEVMAPPER_BUFFER_SIZE);
	if (!buffer) {
		if( debug )
			my_printf(print_flag,"Error allocating memory (%s)", strerror(errno));
		return -1;
	}

	int fd;
	if ((fd = open("/dev/device-mapper", O_RDWR)) < 0) {
		if( debug )
			my_printf(print_flag,"Error opening devmapper (%s)", strerror(errno));
		free(buffer);
		return -1;
	}

	struct dm_ioctl *io = (struct dm_ioctl *) buffer;

	// Create the DM device
	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);

	if (ioctl(fd, DM_DEV_CREATE, io)) {
		if( debug )
			my_printf(print_flag,"Error creating device mapping (%s)", strerror(errno));
		free(buffer);
		close(fd);
		return -1;
	}

	// Set the legacy geometry
	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);

	char *geoParams = buffer + sizeof(struct dm_ioctl);
	// bps=512 spc=8 res=32 nft=2 sec=8190 mid=0xf0 spt=63 hds=64 hid=0 bspf=8 rdcl=2 infs=1 bkbs=2
	strcpy(geoParams, "0 64 63 0");
	geoParams += strlen(geoParams) + 1;
	geoParams = (char *) _align(geoParams, 8);
	if (ioctl(fd, DM_DEV_SET_GEOMETRY, io)) {
		if( debug )
			my_printf(print_flag,"Error setting device geometry (%s)", strerror(errno));
		free(buffer);
		close(fd);
		return -1;
	}

	// Retrieve the device number we were allocated
	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);
	if (ioctl(fd, DM_DEV_STATUS, io)) {
		if( debug )
			my_printf(print_flag,"Error retrieving devmapper status (%s)", strerror(errno));
		free(buffer);
		close(fd);
		return -1;
	}

	unsigned minor = (io->dev & 0xff) | ((io->dev >> 12) & 0xfff00);
	snprintf(ubuffer, len, "/dev/block/dm-%u", minor);

	// Load the table
	struct dm_target_spec *tgt;
	tgt = (struct dm_target_spec *) &buffer[sizeof(struct dm_ioctl)];

	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, DM_STATUS_TABLE_FLAG);
	io->target_count = 1;
	tgt->status = 0;

	tgt->sector_start = 0;
	tgt->length = numSectors;

	char *cryptParams;
	if( dmtype == 0)
	{
		strlcpy(tgt->target_type, "crypt", sizeof(tgt->target_type));

		cryptParams = buffer + sizeof(struct dm_ioctl) + sizeof(struct dm_target_spec);
		snprintf(cryptParams,
	    	DEVMAPPER_BUFFER_SIZE - (sizeof(struct dm_ioctl) + sizeof(struct dm_target_spec)),
	    		"twofish %s 0 %s 0", key, loopFile);
	}else
	{
		strlcpy(tgt->target_type, "linear", sizeof(tgt->target_type));

		cryptParams = buffer + sizeof(struct dm_ioctl) + sizeof(struct dm_target_spec);
		snprintf(cryptParams,
	    	DEVMAPPER_BUFFER_SIZE - (sizeof(struct dm_ioctl) + sizeof(struct dm_target_spec)),
	    		"%s 0", loopFile);
	}
	printf("%s\n", cryptParams);
	cryptParams += strlen(cryptParams) + 1;
	cryptParams = (char *) _align(cryptParams, 8);
	tgt->next = cryptParams - buffer;

	if (ioctl(fd, DM_TABLE_LOAD, io)) {
		if( debug )
			my_printf(print_flag,"Error loading mapping table (%s)", strerror(errno));
		free(buffer);
		close(fd);
		return -1;
	}

	// Resume the new table
	ioctlInit(io, DEVMAPPER_BUFFER_SIZE, name, 0);

	if (ioctl(fd, DM_DEV_SUSPEND, io)) {
		if( debug )
			my_printf(print_flag,"Error Resuming (%s)", strerror(errno));
		free(buffer);
		close(fd);
		return -1;
	}

	free(buffer);

	close(fd);
	return 0;
}

