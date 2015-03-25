/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#include "BGW_CCCI.h"
#include <cutils/sockets.h>

#define WIFI_PROP "bgw.current3gband"

/*
get data from MD by ccci
return value: 0 success; -1: fail

*/

extern int fd = -1;	/*ccci devicd file descriptor*/
static int fd_common_part = -1;
static char buf[1024];	/*modem data buffer*/
static unsigned char send_buf[1024];	/**/
static unsigned char byte_for_common_part[14];
static char send_buffer[1024] = {0};



#define CLRB(x, b)	(x&=~(0x1<<(int)b))


/*
look for the table, and generate the 14 byte array.
*/
int generate_array(float freq)
{
#if 0
	int i;
	
	
	static int n = 0;

	
	if( n == 0)
	{
		MSG("before set bit map, \n");
		for (i = 0; i < 14; i++)
			MSG("byte_for_common_part[%d] = 0x%2X\n", i, byte_for_common_part[i]);
	}
#endif
//	MSG("get freq is %f\n", freq);
	if( freq >= 2394 && freq <= 2401)
		CLRB(byte_for_common_part[0], (freq	- 2394));
	else if( freq >= 2402 && freq <= 2409)
		CLRB(byte_for_common_part[1], (freq	- 2402));
	else if( freq >= 2410 && freq <= 2417)
		CLRB(byte_for_common_part[2], (freq	- 2410));
	else if( freq >= 2418 && freq <= 2425)
		CLRB(byte_for_common_part[3], (freq	- 2418));
	else if( freq >= 2426 && freq <= 2433)
		CLRB(byte_for_common_part[4], (freq	- 2426));
	else if( freq >= 2434 && freq <= 2441)
		CLRB(byte_for_common_part[5], (freq	- 2434));
	else if( freq >= 2442 && freq <= 2449)
		CLRB(byte_for_common_part[6], (freq	- 2442));
	else if( freq >= 2450 && freq <= 2457)
		CLRB(byte_for_common_part[7], (freq	- 2450));
	else if( freq >= 2458 && freq <= 2465)
		CLRB(byte_for_common_part[8], (freq	- 2458));
	else if( freq >= 2466 && freq <= 2473)
		CLRB(byte_for_common_part[9], (freq	- 2466));
	else if( freq >= 2474 && freq <= 2481)
		CLRB(byte_for_common_part[10], (freq - 2474));
	else if( freq >= 2482 && freq <= 2489)
		CLRB(byte_for_common_part[11], (freq - 2482));
	else if( freq >= 2490 && freq <= 2497)
		CLRB(byte_for_common_part[12], (freq - 2490));
	else if(freq >= 2498 && freq <= 2505)
		CLRB(byte_for_common_part[13], (freq - 2498));
	else
		MSG("out of range\n");

#if 0	
	if(++n == 5)
	{
		MSG("after set bit map, \n");
		for (i = 0; i < 14; i++)
			MSG("byte_for_common_part[%d] = 0x%2X\n", i, byte_for_common_part[i]);	

		/*discuss with zhiguo to define the ioctl interface, send 14 bytes to common part*/
		
		n = 0;
		ioctl(fd_common_part, WMT_IOCTL_SEND_BGW_DS_CMD, byte_for_common_part);
		MSG("send data to common part\n");

	}
#endif	

	return 0;
}


void notify_WIFI(const float freq)
{
	
	if(freq >= 824 && freq <=849)
	{
		MSG("band 5\n");
		if(property_set(WIFI_PROP, "5") != 0)
			ERR("set wifi property error %s\n", strerror(errno));
		return;
	}
	else if(freq >= 880 && freq <= 915)
	{
		MSG("band 8\n");
		if(property_set(WIFI_PROP, "8") != 0)
			ERR("set wifi property error %s\n", strerror(errno));
		return;
	}
	else
	{
		MSG("freq is %d\n", freq);
		MSG("isn't band 5 nor band 8, no need notify WIFI\n");
		return;
	}
}


int generate_Freq(float freq)
{
	int i;
//	memset(byte_for_common_part, 0xff, sizeof(byte_for_common_part));
	float freq_after_calculate[5] = {0};
	MSG("freq = %f\n", freq);
	freq_after_calculate[0] = 1575.42 + freq + 1;
	freq_after_calculate[1] = 1575.42 + freq + 0;
	freq_after_calculate[2] = 1575.42 + freq - 1;
	freq_after_calculate[3] = 1575.42 + freq + 2;
	freq_after_calculate[4] = 1575.42 + freq - 2;



/*
	for(i = 0; i < 5; i++)
		MSG("freq_after_calculate[%d] = %f\n", i, freq_after_calculate[i]);
*/
	for(i = 0; i < 5; i++)
		generate_array(freq_after_calculate[i]);
	
	return 0;

}

/*
open ccci interface and get file descriptor.
return value: 0:success; -1 fail
*/
int init_ccci()
{
//	MSG("ccci fd = %d\n", fd);

	fd_common_part = open("/dev/stpwmt", O_RDWR);
	if(fd_common_part < 0)
	{
		ERR("open /dev/stpwmt error, %s\n", strerror(errno));
		return -1;
	}
	MSG("open /dev/stpwmt done\n");
	return 0;
}

/*
read MD data from ccci
return value:  -1: fail; length read from ccci
*/
int get_data_from_ccci()//TBD block read or nonblock read?
{
	/*int ret;*/
	int len;
	int retry = 10;
	if(fd == -1)
	{
		ERR("invalid ccci fd\n");
		return -1;
	}
#if 0
	while((len = read(fd, buf, sizeof(buf))) <= 0)
	{
		if(errno == EINTR)	/* interrupt by signal, ignore this case*/
			continue;
		if(errno == EAGAIN)
		{
			if(retry-- > 0)
			{
				usleep(100000);
				continue;
			}
		}
		ERR("read ccci error,%s\n",strerror(errno));
		return -1;
	}
#endif
	if((len = read(fd, buf, sizeof(buf))) <= 0)
	{		
		ERR("read ccci error,%s\n",strerror(errno));
		return -1;
	}
	MSG("read ccci done, data has been read to buf\n");
	return len;
	
}


/*
write data   to MD by ccci 
data: data pointer
len: data length
return value:  -1: fai or data length write to cccil
*/
int write_data_to_ccci(char *data, int len) //TBD block write or nonblock wrete?
{
	int n;
	int retry = 10;
	if(fd == -1)
	{
		ERR("invalid ccci fd\n");
		return -1;
	}
	MSG("ccci fd = %d\n", fd);
	MSG("data start address = %p\n", data);
	MSG("data length = %d\n", len);
	if( (n = write(fd, data, len)) != len)
	{
		ERR("write data to ccci error, write return %d, %s, \n", n, strerror(errno));
		return -1;
	}
#if 0
	while( (n = write(fd, data, len)) != len)
	{
		if(errno == EINTR)
			continue;
		if(errno == EAGAIN) /*non-block write, sleep a while then retry again*/
		{
			if(retry-- > 0)
			{
				usleep(100000);
				continue;
			}
		}
		if(errno == EBUSY)
		{
			if(retry-- > 0)
			{
				usleep(100000);
				continue;
			}
		}
		ERR("write data to ccci error, %s\n", strerror(errno));
		return -1;
	}
#endif
	MSG("write ccci done, write data length is %d\n", n);
	return n;
}

int enable_coexist()
{
	if(fd == -1)
	{
		ERR("ccci has not init\n");
		return -1;
	}
	int i = 0;
	unsigned int header_size = sizeof(mtk_ilm_struct);
	unsigned int local_para_size = 0;
	unsigned int peer_buff_size = 0;
	int total_size = 0;
	local_para_struct *local_para  = NULL;
	peer_buff_struct *local_peer_buff = NULL;
	mtk_ilm_struct ilm;	/*header*/
	l4c_rf_info_req_struct MD_data_req;	/*payload*/

//	char send_buffer[1024] = {0};
	memset(&MD_data_req, 0, sizeof(l4c_rf_info_req_struct));
	MD_data_req.ref_count = 1;	/*hardcode*/
	MD_data_req.msg_len = sizeof(l4c_rf_info_req_struct);	/*length of payload*/
	MD_data_req.mode = 1;	/*1 means enable MD report data*/
		
//	ilm.src_mod_id = APMOD_GPS;
	ilm.src_mod_id = 2;
	ilm.sap_id = 8;/*ask Hugo*/
//	ilm.dest_mod_id = 0;/*ask hugo or MD owner*/
	ilm.msg_id = (msg_type)IPC_MSG_ID_L4C_RF_INFO_REQ;
	ilm.local_para_ptr = (local_para_struct *)&MD_data_req;
	ilm.peer_buff_ptr = NULL; /*ask MD owner, what should fill into this item*/

	for(i = 0; i < 2; i++)
	{
		ilm.dest_mod_id = i;/*ask hugo or MD owner*/
		memcpy(send_buffer, &ilm, sizeof(ilm));	/*fill in buffer, add header*/
		MSG("before copy to ilm local para len is %d\n",MD_data_req.msg_len);
		memcpy(send_buffer+sizeof(ilm), ilm.local_para_ptr, MD_data_req.msg_len);	/*fill in buffer , add payload*/
		MSG("after copy to ilm local para len is %d\n",ilm.local_para_ptr->msg_len);
		total_size = header_size + ilm.local_para_ptr->msg_len;
	//	MSG("buffer address is %p", send_buffer);
		if(write_data_to_ccci(send_buffer, total_size) != total_size)
		{
			ERR("send data to MD incomplete, total length is %d\n", total_size);
			return -1;
		}
		MSG("enable BGW sim%d coexist\n", i);
		usleep(100000);
	}
	return 0;	
	
}

int disable_coexist()
{
	int i = 0;
	
	if(fd == -1)
	{
		ERR("ccci has not init\n");
		goto err_handle;
	}
	unsigned int header_size = sizeof(mtk_ilm_struct);
	unsigned int local_para_size = 0;
	unsigned int peer_buff_size = 0;
	int total_size = 0;
	local_para_struct *local_para  = NULL;
	peer_buff_struct *local_peer_buff = NULL;
	mtk_ilm_struct ilm;	/*header*/
	l4c_rf_info_req_struct MD_data_req;	/*payload*/

//	char send_buffer[1024] = {0};
	memset(&MD_data_req, 0, sizeof(l4c_rf_info_req_struct));
	MD_data_req.ref_count = 1;	/*hardcode*/
	MD_data_req.msg_len = sizeof(l4c_rf_info_req_struct);	/*length of payload*/
	MD_data_req.mode = 0;	/*1 means enable MD report data, 0 means disabel MD*/
		
//	ilm.src_mod_id = APMOD_GPS;
	ilm.sap_id = 8;/*ask Hugo*/
//	ilm.dest_mod_id = 4;/*ask hugo or MD owner*/
	ilm.msg_id = (msg_type)IPC_MSG_ID_L4C_RF_INFO_REQ;
	ilm.local_para_ptr = (local_para_struct *)&MD_data_req;
	ilm.peer_buff_ptr = NULL; /*ask MD owner, what should fill into this item*/

	for(i = 0; i < 2; i++)
	{
		ilm.dest_mod_id = i;/*ask hugo or MD owner*/
		memcpy(send_buffer, &ilm, sizeof(ilm));	/*fill in buffer, add header*/
		memcpy(send_buffer+sizeof(ilm), ilm.local_para_ptr, ilm.local_para_ptr->msg_len);	/*fill in buffer , add payload*/

		total_size = header_size + ilm.local_para_ptr->msg_len;
	
		if(write_data_to_ccci(send_buffer, total_size) != total_size)
		{
			ERR("send data to MD incomplete, total length is %d\n", total_size);
			goto err_handle;
		}
		MSG("disable BGW coexist %d\n", i);
	}
err_handle:
//	close(fd);
	close(fd_common_part);
	return 0;	
}


int md_data_process()
{
	int i = 0;
	float md_freq;
	char ilm_buf[1024] = {0};	/*target buffer*/
	mtk_ilm_struct * dest = (mtk_ilm_struct *)ilm_buf;
	kal_uint16 tx_freq[64] = {0};

	/*reset  target buffer first
	dest->src_mod_id = 0;
	dest->dest_mod_id = 0;
	dest->sap_id = 0;
	dest->msg_id = 0;
	dest->local_para_ptr = NULL;
	dest->peer_buff_ptr = NULL;
	*/

	memcpy(dest, buf, sizeof(mtk_ilm_struct)); /*copy header*/
/*	MSG("==============================================");
	for(i = 0; i < 160; i++)
		MSG("%d =  %d\n",i,  buf[i]);
	MSG("==============================================");
*/	if(dest->local_para_ptr != NULL)
	{
		MSG("there is payload in ilm\n");
		mtk_local_para_struct * p_local = (mtk_local_para_struct *)(buf + sizeof(mtk_ilm_struct));
		dest->local_para_ptr = (mtk_local_para_struct *)(dest + 1);	/*point to global buffer*/
		memcpy(dest + 1,buf+sizeof(mtk_ilm_struct), p_local->msg_len); /*copy payload*/
		
	}
	else
	{
		ERR("no local data\n");
		return -1;
	}

	/*till now, ilm message has been in ilm_buf, and dest points the start address*/

	if(dest->msg_id == (msg_type)IPC_MSG_ID_L4C_RF_INFO_IND)/*MD report data to AP*/
	{
		MSG("it's MD INFO_IND\n");
//		kal_uint16 tx_freq[64] = {0};
		/*copy MD tx_freq to local*/
		l4cps_rf_info_ind_struct * tmp = (l4cps_rf_info_ind_struct *)(dest->local_para_ptr);
		memcpy(tx_freq, tmp->tx_freq, sizeof(kal_uint16) * 64);
	}
	else
		ERR("I don't know this message\n");

	/*now, tx_feq is in buffer tx_freq[64], add log while Hong Yu provide more details*/

	while(tx_freq[i] != 0xffff)
	{
		MSG("get modem freq[%d] = %d\n", i, tx_freq[i]);
		i++;
	}
	i = 0;
	/*******************before notify WIFI, set property to 0 as the default value***********************/
	if(property_set(WIFI_PROP, "0") != 0)
		ERR("set wifi property error %s\n", strerror(errno));
	memset(byte_for_common_part, 0xff, sizeof(byte_for_common_part));
	while((tx_freq[i] != 0xffff) && (i < 64))
	{
		md_freq = tx_freq[i++]/(float)10.0; /*unit 100Khz, so if tx_freq[0] == 9150, means MD tx freq = 915Mhz*/
		//notify_WIFI(md_freq);
		generate_Freq(md_freq);	
	}
	MSG("14 bytes are %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",\
		byte_for_common_part[0],byte_for_common_part[1], byte_for_common_part[2],\
		byte_for_common_part[3],byte_for_common_part[4], byte_for_common_part[5],\
		byte_for_common_part[6], byte_for_common_part[7],byte_for_common_part[8],\
		byte_for_common_part[9], byte_for_common_part[10], byte_for_common_part[11],\
		byte_for_common_part[12],byte_for_common_part[13]);
	ioctl(fd_common_part, WMT_IOCTL_SEND_BGW_DS_CMD, byte_for_common_part);
	return 0;
}

