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

#include "BGWC.h"
#include "hardware/ccci_intf.h"

static BGWD_T gvariable[1];
extern int fd;	/*ccci devicd file descriptor*/






static int send_init_message(BGWD_T * const state)
{
	struct sockaddr_nl nl_dest_addr;/*destination address*/
	struct nlmsghdr nlhdr;
	struct iovec iov;
	struct msghdr msg;
	struct req r;
	memset(&msg, 0, sizeof(msg));
//	msg.msg_name = (void*)&(nladdr);
//	msg.msg_namelen = sizeof(nladdr);

	/*init destination address*/
	memset(&nl_dest_addr, 0, sizeof(nl_dest_addr));
	nl_dest_addr.nl_family = AF_NETLINK;
	nl_dest_addr.nl_pid = 0; /*destination is kernel so set to 0*/
	nl_dest_addr.nl_groups = 0;
	
	
//	char init_data[MAX_NL_MSG_LEN] = {0};
	

	/*init struct nlmsghdr*/
//	nlhdr = (struct nlmsghdr*)malloc(NLMSG_SPACE(128));
	r.nlh.nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
	r.nlh.nlmsg_pid = getpid();
	r.nlh.nlmsg_flags = 0;
	memset(r.buf, 0, MAX_NL_MSG_LEN);
	strcpy(NLMSG_DATA(&(r.nlh)), "BGW");


	
	/*init buffer vector*/
	iov.iov_base = (void*)&r;
	iov.iov_len = sizeof(r);

	
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_name = (void*)&nl_dest_addr;
	msg.msg_namelen = sizeof(nl_dest_addr);

	if(sendmsg(state->sock_id, &msg, 0) < 0)
	{
		ERR("sendmsg error, reason %s\n", strerror(errno));	
		return -1;
	}
	MSG("send msg to kernel\n");
		
	/*send_message();*/
	return 0;
}

static void init_state(void* arg)
{
	BGWD_T * const state = (BGWD_T *)arg;
	do
	{
		send_init_message(state);
		usleep(10000000);
	}
	while(state->state_machine != WAITING);
	return;
}


static int init_socket(BGWD_T * const state)
{
//	BGWD_T * const state = gvariable;
	struct sockaddr_nl local;	/*used to describe local address*/
	
	int res;
	state->sock_id = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if(state->sock_id < 0)
	{
		ERR("get control socket error, reason:%s\n", strerror(errno));
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_pid = getpid();/*local process id*/
	
	MSG("native process pid is %d\n", local.nl_pid);
	local.nl_pad = 0;
	local.nl_groups = 0;

	res = bind(state->sock_id, (struct sockaddr*)&local, sizeof(struct sockaddr_nl));
	if(res != 0)
	{
		ERR("bind error, reason: %s\n", strerror(errno));
		close(state->sock_id);
		return -1;
	}
	MSG("bind done\n");
	return 0;
	
}


static int pthread_over(pthread_t pid)
{
	if(pthread_kill(pid, SIGUSR1) != 0)
	{
		ERR("use pthread_kill to terminate thread error\n");
		return -1;
	}
	MSG("pthread_kill to send signal to thread for killing it\n");
	return 0;
}

static void thread_exit_hander(int sig)
{
	MSG("let the thread over\n");
	disable_coexist();

	pthread_exit(0);
	return;
}



static int register_sig_handler()
{
	struct sigaction actions;
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_hander;
	if(sigaction(SIGUSR1, &actions, NULL) != 0)
	{
		ERR("sigaction error\n");
		return -1;
	}
	return 0;
}


static void * BGWM_thread(void *arg)
{
	int ret = 0;
	register_sig_handler();
	/*used to set parameter to connectivity chip*/
	init_ccci();
	enable_coexist();
	while(1)
	{	
		ret = get_data_from_ccci();
		if(ret<0)
		{
		    ERR("read ccci error,out of loop!\n");
			break;
		}
		md_data_process();
	
	}
	return NULL;
}


static void get_msg(BGWD_T * const state)
{
//	BGWD_T *const state = gvariable;
	ssize_t msg_len;
//	struct req r;
	
	memset(&(state->r.nlh), 0, MAX_NL_MSG_LEN);
	memset(&(state->iov),0, sizeof(struct iovec));
	memset(&(state->msg), 0, sizeof(struct msghdr));
	
	state->iov.iov_base = (void*)(&(state->r));
	state->iov.iov_len = sizeof(state->r);
	
	state->msg.msg_name = (void*)&(state->nladdr);
	state->msg.msg_namelen = sizeof(state->nladdr);
	state->msg.msg_iov = &(state->iov);
	state->msg.msg_iovlen = 1;
	MSG("here wait message from kernel\n");
	msg_len = recvmsg(state->sock_id, &(state->msg), 0);
	if(msg_len <= 0)
	{
		ERR("recvmsg error, msg_len is %d\n", msg_len);
		return;
	}
	MSG("received msg, it's len is %d\n", msg_len);
//	MSG("message payload %d\n", NLMSG_DATA(&(state->r.nlh)));
	return;
}


static int parse_msg(BGWD_T * const state)
{
//	BGWD_T * const state = gvariable;
	char *msg = NLMSG_DATA(&(state->r.nlh));
	MSG("received msg %d\n", *msg);
//	int a = atoi(msg);
	switch(*msg)
	{
		case ON:
			if(state->state_machine != WAITING)
			{
				ERR("received ON, but we are in %d\n", state->state_machine);
				break;
			}
			/*create a thread for co-exist mechenisam*/
			if(pthread_create(&state->BGWM_id, NULL, BGWM_thread, NULL) !=0 )
			{
				ERR("create thread error\n");
				return -1;
			}
			MSG("start co-exist macheniasm. BGW thread has been created\n");
			state->state_machine = RUNNING;
			MSG("got kernel ON, set our state to RUNNING\n");
			break;
		case OFF:
			if(state->state_machine != RUNNING)
			{
				ERR("received OFF, but we are in %d\n", state->state_machine);
				break;
			}
			/*cancel the thread*/
			//if(pthread_cancel(state->BGWM_id) != 0)
			if(pthread_over(state->BGWM_id) != 0)
			{
				ERR("cancel thread error\n");
				return -1;
			}
			if(pthread_join(state->BGWM_id, NULL) != 0)
			{
				ERR("join thread exit error\n");
				return -1;
			}
			state->BGWM_id = -1;
			MSG("kill thread done\n");
			state->state_machine = WAITING;
			MSG("got kernel OFF, set our state to WAITING\n");
			break;
		case ACK: 
			if(state->state_machine != INIT)
			{
				ERR("received ACK, but we are in %d\n", state->state_machine);
				break;
			}
			/*got message from kernel response, set state to init state*/
			state->state_machine = WAITING;
			MSG("got kernel ACK , set our state to WAITING\n");
			break;
		default:
			ERR("unknown msg\n");
			break;
	}
	return 0;
}


static void receiving_thread(void * arg)
{
	BGWD_T * const state = (BGWD_T *)arg;
	char dev_node[32];
	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_GPS_IPC, MD_SYS1));
	MSG("ccci node is %s\n", dev_node);
	/*here we open ccci device, and never close it, caused by ccci device has a limitation
	write operation is blocked for a long while, so there can be a case that after write data to ccci
	then close ccci in a very little moment, and ccci kernel driver will clean all data that just write into ccci buffer
	and at the same time, modem read ccci buffer, but the buffer is zero, this will cause modem reset
	so we just open ccci device once, and never close it, to avoid ccci device clean it's kernel buffer*/
	fd = open(dev_node, O_RDWR/*|O_NONBLOCK*/);
	if(fd < 0)
	{
		ERR("open %s error, %s\n", dev_node, strerror(errno));
		return -1;
	}
	else
		MSG("open %s done\n", dev_node);
	while(1)
	{
		get_msg(state);
		parse_msg(state);
	}
	return;
}


static int start_init_thread(BGWD_T * const state)
{
	int err;
	pthread_t init_thread_id;
	err = pthread_create(&init_thread_id, NULL, &init_state, state);
	if(err != 0)
	{
		ERR("create init thread error, reason %s\n", strerror(errno));
		return -1;
	}
	MSG("create init thread done\n");
	return 0;
}




int main()
{
	BGWD_T * const state = gvariable;
	struct sockaddr sock_addr;
	socklen_t sock_len = sizeof(sock_addr);
	if(init_socket(state) == -1)
		exit(1);
	if(start_init_thread(state) == -1)
		close(state->sock_id);

	receiving_thread(state);

	return 0;
}





#if 0


int main()
{
	MSG("this main just for test ccci\n");
	if(init_ccci() < 0)
	{
		ERR("init ccci error\n");
		exit(1);
	}
	if(enable_coexist() < 0)
	{
		ERR("enable coexist error\n");
		exit(1);
	}	
	if(get_data_from_ccci() < 0)
	{
		ERR("read ccci error\n");
		exit(1);
	}
	if(md_data_process() < 0)
	{
		ERR("data process error\n");
		exit(1);
	}
	return 0;

}

#endif

#if 0

int main()
{
	int fd, n, retry = 10;
	char buf = 0;
	fd = open(BGW_CCCI_DEVICE, O_RDWR|O_NONBLOCK);
	if(fd < 0)
	{
		ERR("open %s error, %s\n", BGW_CCCI_DEVICE, strerror(errno));
		return -1;
	}
	else
		MSG("open %s done\n", BGW_CCCI_DEVICE);

	if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        ERR("ccci_write fcntl failure reason=[%s]\n", strerror(errno));
    }
	MSG("ccci fd = %d\n", fd);
	
		while((n = write(fd, &buf, 1)) != 1) {
			if(errno == EINTR) continue;
			if(errno == EAGAIN) {
				if(retry-- > 0) {
					usleep(100 * 1000);
					continue;
				}
				goto exit;
			}
			if(errno == EBUSY) {
				if(retry-- > 0) {
				//	AGPS_CCCI_IPC_RESET_SEND(fd);
					usleep(100 * 1000);
					continue;
				}
				goto exit;
			}
			goto exit;
		}
		return n;
	exit:
		ERR("ccci_safe_write reason=[%s]\n", strerror(errno));
		return -1;

}

#endif

