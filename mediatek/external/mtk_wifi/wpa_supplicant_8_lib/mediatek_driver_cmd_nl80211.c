/*
 * Driver interaction with extended Linux CFG8021
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 */
#include "hardware_legacy/driver_nl80211.h"
#include "mediatek_driver_nl80211.h"
#include "wpa_supplicant_i.h"
#include "config.h"
#include "driver_i.h"

#ifdef ANDROID
#include "android_drv.h"
#endif

#ifdef CONFIG_MTK_P2P_SIGMA
#include "p2p/p2p_i.h"
#endif

int send_and_recv_msgs(struct wpa_driver_nl80211_data *drv, struct nl_msg *msg,
		       int (*valid_handler)(struct nl_msg *, void *),
		       void *valid_data);

static int testmode_sta_statistics_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_TESTMODE_STA_STATISTICS_NUM];
    struct wpa_driver_sta_statistics_s *sta_statistics = (struct wpa_driver_sta_statistics_s *)arg;
    unsigned char i = 0;
    static struct nla_policy policy[NL80211_TESTMODE_STA_STATISTICS_NUM] = {
    	[NL80211_TESTMODE_STA_STATISTICS_VERSION] 			    = { .type = NLA_U8 },
    	[NL80211_TESTMODE_STA_STATISTICS_MAC] 		            = { .type = NLA_UNSPEC },
    	[NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE] 		    = { .type = NLA_U8 },
    	[NL80211_TESTMODE_STA_STATISTICS_FLAG] 	                = { .type = NLA_U32 },    	
    	[NL80211_TESTMODE_STA_STATISTICS_PER] 				    = { .type = NLA_U8 },
    	[NL80211_TESTMODE_STA_STATISTICS_RSSI] 			        = { .type = NLA_U8 },
    	[NL80211_TESTMODE_STA_STATISTICS_PHY_MODE] 		        = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_TX_RATE] 		        = { .type = NLA_U16 },
    	[NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT] 	            = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT] 	        = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME] 		    = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT] 	        = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT] 	    = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME] 	    = { .type = NLA_U32 },
    	[NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY]    = { .type = NLA_UNSPEC },
    	[NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY]      = { .type = NLA_UNSPEC },   	
    	[NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY]  = { .type = NLA_UNSPEC },   
    	[NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY] 	= { .type = NLA_UNSPEC },
    	[NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY]        = { .type = NLA_UNSPEC }
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
    	  genlmsg_attrlen(gnlh, 0), NULL);
    
    if (!tb[NL80211_ATTR_TESTDATA] || 
            nla_parse_nested(sinfo, NL80211_TESTMODE_STA_STATISTICS_MAX, tb[NL80211_ATTR_TESTDATA], policy))
    	return NL_SKIP;

    for(i=1; i < NL80211_TESTMODE_STA_STATISTICS_NUM; i++) {
        if(sinfo[i]) { 		
            switch(i) {
            	case NL80211_TESTMODE_STA_STATISTICS_VERSION:
                    sta_statistics->version = nla_get_u8(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_MAC:
                    nla_memcpy(sta_statistics->addr, sinfo[i], ETH_ALEN);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE:
                    sta_statistics->link_score = nla_get_u8(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_FLAG:
                    sta_statistics->flag = nla_get_u32(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_PER:
                    sta_statistics->per = nla_get_u8(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_RSSI:
                    sta_statistics->rssi = (((int)nla_get_u8(sinfo[i]) - 220) / 2);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_PHY_MODE:
                    sta_statistics->phy_mode = nla_get_u32(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_TX_RATE:
                    sta_statistics->tx_rate = (((double)nla_get_u16(sinfo[i])) / 2);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT:
                    sta_statistics->tx_fail_cnt = nla_get_u32(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT:
                    sta_statistics->tx_timeout_cnt = nla_get_u32(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME:
                    sta_statistics->tx_avg_air_time = nla_get_u32(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT:
                    sta_statistics->tx_total_cnt = nla_get_u32(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT:
                    sta_statistics->tx_exc_threshold_cnt = nla_get_u32(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME:
                    sta_statistics->tx_avg_process_time = nla_get_u32(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY:
                    nla_memcpy(sta_statistics->tc_buf_full_cnt, sinfo[i], sizeof(sta_statistics->tc_buf_full_cnt));
                    break;   
            	case NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_que_len, sinfo[i], sizeof(sta_statistics->tc_que_len));
                    break;                      
            	case NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_avg_que_len, sinfo[i], sizeof(sta_statistics->tc_avg_que_len));
                    break;                                   
            	case NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY:
                    nla_memcpy(sta_statistics->tc_cur_que_len, sinfo[i], sizeof(sta_statistics->tc_cur_que_len));
                    break;                    
            	case NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY:
                    nla_memcpy(sta_statistics->reserved, sinfo[i], sizeof(sta_statistics->reserved));
                    break;              
                default:
                    break;
            }
        }
    }
    
    return NL_SKIP;
}
#ifdef CONFIG_MEDIATEK_WIFI_BEAM
// Peter: add for Beamplus application
static int testmode_beamplus_go_ready_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_TESTMODE_GO_READY_NUM];
    u32 *go_ready = (u32 *)arg;
    unsigned char i = 0;
    static struct nla_policy policy[NL80211_TESTMODE_GO_READY_NUM] = {
    	[NL80211_TESTMODE_GO_READY_STATE] 	            = { .type = NLA_U32 },
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
    	  genlmsg_attrlen(gnlh, 0), NULL);
    
    if (!tb[NL80211_ATTR_TESTDATA] || 
            nla_parse_nested(sinfo, NL80211_TESTMODE_GO_READY_MAX, tb[NL80211_ATTR_TESTDATA], policy))
    	return NL_SKIP;

    for(i=1; i < NL80211_TESTMODE_GO_READY_NUM; i++) {
        if(sinfo[i]) { 		
            switch(i) {
            	case NL80211_TESTMODE_GO_READY_STATE:
                    *go_ready = nla_get_u32(sinfo[i]);
                    break;
                default:
                    break;
            }
        }
    }
    
    return NL_SKIP;
}
// Peter: end of Beamplus application
#endif /*Mediatek Modified*/
#ifdef CONFIG_MTK_POOR_LINK_DETECT
static int testmode_sta_link_detect_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *sinfo[NL80211_TESTMODE_LINK_DETECT_NUM];
    struct wpa_driver_sta_link_detect_s *sta_link_detect = (struct wpa_driver_sta_link_detect_s *)arg;
    unsigned char i = 0;
    s64 ack_cnt;
    static struct nla_policy policy[NL80211_TESTMODE_LINK_DETECT_NUM] = {
    	[NL80211_TESTMODE_LINK_TX_FAIL_CNT] 	            = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_TX_RETRY_CNT] 	            = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_TX_MULTI_RETRY_CNT] 	        = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_ACK_FAIL_CNT] 	            = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_FCS_ERR_CNT] 	            = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_TX_CNT] 	                    = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_TX_OK_CNT] 	                = { .type = NLA_U64 },
    	[NL80211_TESTMODE_LINK_TX_OS_CNT] 	                = { .type = NLA_U64 },
    };

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
    	  genlmsg_attrlen(gnlh, 0), NULL);
    
    if (!tb[NL80211_ATTR_TESTDATA] || 
            nla_parse_nested(sinfo, NL80211_TESTMODE_LINK_DETECT_MAX, tb[NL80211_ATTR_TESTDATA], policy))
    	return NL_SKIP;

    for(i=1; i < NL80211_TESTMODE_LINK_DETECT_NUM; i++) {
        if(sinfo[i]) { 		
            switch(i) {
            	case NL80211_TESTMODE_LINK_TX_FAIL_CNT:
                    sta_link_detect->tx_fail_cnt =nla_get_u64(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_LINK_TX_RETRY_CNT:
                    sta_link_detect->tx_retry_cnt = nla_get_u64(sinfo[i]);
                    break;
            	case NL80211_TESTMODE_LINK_TX_MULTI_RETRY_CNT:
                    sta_link_detect->tx_multi_retry_cnt = nla_get_u64(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_LINK_ACK_FAIL_CNT:
                    ack_cnt = nla_get_u64(sinfo[i]);
					if (ack_cnt < 0)
                    	sta_link_detect->ack_fail_cnt = -ack_cnt;
                    else
                    	sta_link_detect->ack_fail_cnt = ack_cnt;                    		
                    break;                    
            	case NL80211_TESTMODE_LINK_FCS_ERR_CNT:
                    sta_link_detect->fcs_err_cnt = nla_get_u64(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_LINK_TX_CNT:
                    sta_link_detect->tx_cnt = nla_get_u64(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_LINK_TX_OK_CNT:
                    sta_link_detect->tx_ok_cnt = nla_get_u64(sinfo[i]);
                    break;                    
            	case NL80211_TESTMODE_LINK_TX_OS_CNT:
                    sta_link_detect->tx_os_cnt = nla_get_u64(sinfo[i]);
                    break;                    
                default:
                    break;
            }
        }
    }
    
    return NL_SKIP;
}
#endif

static void * nl80211_cmd(struct wpa_driver_nl80211_data *drv,
			  struct nl_msg *msg, int flags, uint8_t cmd)
{
	return genlmsg_put(msg, 0, 0, drv->global->nl80211_id,
			   0, flags, cmd, 0);
}

int wpa_driver_nl80211_testmode(void *priv, const u8 *data,
	                        size_t data_len)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
    struct nl_msg *msg, *cqm = NULL;
    struct wpa_driver_testmode_params *params;
    int index;

    msg = nlmsg_alloc();
    if (!msg)
        return -1;

    wpa_printf(MSG_DEBUG, "nl80211: Test Mode buflen = %d, ifindex=%d", data_len,drv->ifindex);

    nl80211_cmd(drv, msg, 0, NL80211_CMD_TESTMODE);

    NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, drv->ifindex);
    NLA_PUT(msg, NL80211_ATTR_TESTDATA, data_len, data);

    params = (struct wpa_driver_testmode_params *)data;

    /* Mask version field */
    index = params->hdr.index & BITS(0, 23);

    switch(index) {
    	case 0x10:
    		{
    		    struct wpa_driver_get_sta_statistics_params *sta_params = data;

    		    return send_and_recv_msgs(drv, msg, testmode_sta_statistics_handler, sta_params->buf);            
            }
#ifdef CONFIG_MEDIATEK_WIFI_BEAM
	case 0x12:
            {
                struct wpa_driver_set_beamplus_params *sta_params = data;
            
    		    return send_and_recv_msgs(drv, msg, testmode_beamplus_go_ready_handler, &sta_params->value);            
            }
#endif
#ifdef CONFIG_MTK_POOR_LINK_DETECT            
        case 0x20:
    		{
    		    struct wpa_driver_get_sta_link_detect_params *sta_params = data;

    		    return send_and_recv_msgs(drv, msg, testmode_sta_link_detect_handler, sta_params->buf);            
    		}
#endif                       
        default:
    		{
    		    int ret = 0;   
    		    ret = send_and_recv_msgs(drv, msg, NULL, NULL);
    		    wpa_printf(MSG_DEBUG, "ret = %d, nl=%d",ret, drv->global->nl);
    		    return ret;
    		}
    }

 nla_put_failure:
	return -ENOBUFS;
}

static int wpa_driver_nl80211_driver_sw_cmd(void *priv,
    int set, unsigned long *adr, unsigned long *dat)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
    struct wpa_driver_sw_cmd_params params;
    struct nl_msg *msg, *cqm = NULL;
    int ret = 0;
    
    os_memset(&params, 0, sizeof(params));
    
    params.hdr.index = 1;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_sw_cmd_params);
    
    params.adr = *adr;
    params.data = *dat;

    if(set)
        params.set = 1;
    else
        params.set = 0;
	
    wpa_driver_nl80211_testmode(priv, (u8 *)&params, sizeof(struct wpa_driver_sw_cmd_params));
    return 0;
}

#ifdef CONFIG_HOTSPOT_MGR_SUPPORT
static int wpa_driver_hotspot_block_list_update(void *priv,
    const u8 *bssid, int blocked)
	                           
{
    struct wpa_driver_hotspot_params params;

    os_memset(&params, 0, sizeof(params));

    if(bssid)
        os_memcpy(params.bssid, bssid, ETH_ALEN);

    params.blocked = (u8)blocked;

    params.hdr.index = 3;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_hotspot_params);

    return wpa_driver_nl80211_testmode(priv, (u8 *)&params, 
        sizeof(struct wpa_driver_hotspot_params));
}

static int wpa_driver_sta_block(void *priv, char *cmd)
{
    u8 bssid[ETH_ALEN];
    int blocked = 1;

    /* Block client device */
    if (hwaddr_aton(cmd, bssid)) {
        wpa_printf(MSG_DEBUG, "STA block: invalid DEVICE ADDRESS '%s'",
               cmd);
        return -1;
    }

    wpa_printf(MSG_DEBUG, "Block STA " MACSTR, MAC2STR(bssid));
    return wpa_driver_hotspot_block_list_update(priv, bssid, blocked);
}

static int wpa_driver_sta_unblock(void *priv, char *cmd)
{
    u8 bssid[ETH_ALEN];
    int blocked = 0;

    /* Unblock client device */
    if (hwaddr_aton(cmd, bssid)) {
        wpa_printf(MSG_DEBUG, "STA unblock : invalid DEVICE ADDRESS '%s'",
               cmd);
        return -1;
    }

    wpa_printf(MSG_DEBUG, "Unblock STA " MACSTR, MAC2STR(bssid));
    return wpa_driver_hotspot_block_list_update(priv, bssid, blocked);
}
#endif /* CONFIG_HOTSPOT_MGR_SUPPORT */

#ifdef CONFIG_WAPI_SUPPORT
int wpa_driver_nl80211_set_wapi_key(void *priv,
				       const u8 *addr, int key_idx,
				       int set_tx, const u8 *seq,
				       size_t seq_len,
				       const u8 *key, size_t key_len)
{
    struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
	struct nl_msg *msg, *cqm = NULL;
    struct wpa_driver_wapi_key_params params;
	int ret = 0;

    os_memset(&params, 0, sizeof(params));
    
    params.hdr.index = 2;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_wapi_key_params);

	wpa_printf(MSG_DEBUG, "[WAPI-DEBUG]1 %s: ",
		   __FUNCTION__);

   if(seq_len > IW_ENCODE_SEQ_MAX_SIZE * 2){
   	    wpa_printf(MSG_DEBUG, "[WAPI-Debug]%s: Invalid seq_len %lu",
		   __FUNCTION__, (unsigned long) seq_len);
		return -1;
   	}

    params.key_index = key_idx + 1;
    params.key_len = key_len;

    if (addr == NULL ||
	    os_memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0)
		params.extparams.ext_flags |= IW_ENCODE_EXT_GROUP_KEY;
	if (set_tx)
		params.extparams.ext_flags |= IW_ENCODE_EXT_SET_TX_KEY;
    
	if (addr)
		os_memcpy(params.extparams.addr, addr, ETH_ALEN);
	else
		os_memset(params.extparams.addr, 0xff, ETH_ALEN);
    
	if (key && key_len) {
		os_memcpy(params.extparams.key, key, key_len);
		params.extparams.key_len = key_len;
	}

    wpa_printf(MSG_DEBUG, "[WAPI-DEBUG]2 %s:",__FUNCTION__);

    wpa_printf(MSG_DEBUG, "%s: Set IW_ENCODE_ALG_SMS4 to ext->alg",
			   __FUNCTION__);
    
    params.extparams.alg = IW_ENCODE_ALG_SMS4;

	wpa_printf(MSG_DEBUG, "[WAPI-DEBUG]3 %s: ",__FUNCTION__);
    
    if(seq && seq_len)
        os_memcpy(params.extparams.tx_seq, seq, seq_len);
    
    wpa_hexdump(MSG_DEBUG, "seq", seq, seq_len);

    wpa_printf(MSG_DEBUG, "[WAPI-DEBUG]4 Copy buffer %s: ",__FUNCTION__);

	wpa_driver_nl80211_testmode(priv, (u8 *)&params, sizeof(struct wpa_driver_wapi_key_params));

    return 0;
}

/**
     * wpa_driver_nl80211_send_msg - send some information to driver
     * @priv: private driver interface data from init()
     * @msg_in: the message sent to driver
     * @msg_in_len: the length of sent message
     * @msg_out: the message given back from driver
     * @msg_out_len: the length of message given back from driver
	 *
	 * Returns: 0 on success, -1 on failure
	 *
	 */
static int wpa_driver_nl80211_send_msg(void *priv, const u8 *msg_in, int msg_in_len,
							 u8 *msg_out, int *msg_out_len)
{
	struct i802_bss *bss = priv;
    struct wpa_driver_nl80211_data *drv = bss->drv;
	int ret = 0;

	if(msg_in_len > 1024){
		wpa_printf(MSG_DEBUG, "wpa_driver_nl80211_send_msg: msg too long");
		return -1;
	}
    
	return ret;
}
#endif /* CONFIG_WAPI_SUPPORT */

/*************************************************************
OVERLAPPED functins, previous defination is in driver_nl80211.c,
it will be modified
**************************************************************/

/**************************************************************************/
extern int wpa_config_write(const char *name, struct wpa_config *config);

static int wpa_driver_mediatek_set_country(void *priv, const char *alpha2_arg)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	int ioctl_sock=-1;
	struct iwreq iwr;
	int ret=-1;
	char buf[11];

	wpa_printf(MSG_DEBUG, "wpa_driver_nl80211_set_country");
	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ioctl_sock < 0) {
		wpa_printf(MSG_ERROR, "%s: socket(PF_INET,SOCK_DGRAM)", __func__);
		return -1;
	}
	os_memset(&iwr, 0, sizeof(iwr));
	os_strncpy(iwr.ifr_name, drv->first_bss.ifname, IFNAMSIZ);
	sprintf(buf,"COUNTRY %s",alpha2_arg);
	iwr.u.data.pointer = buf;
	iwr.u.data.length = strlen(buf);
	if ((ret = ioctl(ioctl_sock, 0x8B0C, &iwr)) < 0) {//SIOCSIWPRIV
		wpa_printf(MSG_DEBUG, "ioctl[SIOCSIWPRIV]: %s",buf);
		close(ioctl_sock);
		return ret;
	}
	else {
		close(ioctl_sock);
		return 0;
	}

}
/* Move GET_STA_STATISTICS to "DRIVER GET_STA_STATISTICS", implement in 3rd part lib */
/* [ALPS00618361] [WFD Quality Enhancement] */
int wpas_get_sta_statistics(struct wpa_supplicant *wpa_s, char *sta_addr, u8 *buf)
{
    struct wpa_driver_get_sta_statistics_params params;
    
    os_memset(&params, 0, sizeof(params));

    if(sta_addr)
        os_memcpy(params.addr, sta_addr, ETH_ALEN);

	wpa_printf(MSG_DEBUG, "get_sta_statistics ["MACSTR"]", MAC2STR(params.addr));
    
    params.hdr.index = 0x10;
	params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_get_sta_statistics_params);

    /* buffer for return structure */ 
    params.buf = buf;
	
    return wpa_driver_nl80211_testmode(wpa_s->drv_priv, (u8 *)&params, 
        sizeof(struct wpa_driver_get_sta_statistics_params));
}

/*  [ALPS00618361] [WFD Quality Enhancement] [changelist 1686130] */
static int print_sta_statistics(struct wpa_supplicant *wpa_s, struct wpa_driver_sta_statistics_s *sta_stats,
			  unsigned long mask, char *buf, size_t buflen)
{
	size_t i;
	int ret;
	char *pos, *end;

	pos = buf;
	end = buf + buflen;

	ret = os_snprintf(pos, end - pos, "sta_addr="MACSTR"\n", MAC2STR(sta_stats->addr));
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "link_score=%d\n", sta_stats->link_score);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "per=%d\n", sta_stats->per);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "rssi=%d\n", sta_stats->rssi);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "phy=0x%08X\n", sta_stats->phy_mode);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;
    
	ret = os_snprintf(pos, end - pos, "rate=%.1f\n", sta_stats->tx_rate);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "total_cnt=%d\n", sta_stats->tx_total_cnt);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "threshold_cnt=%d\n", sta_stats->tx_exc_threshold_cnt);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    
    
	ret = os_snprintf(pos, end - pos, "fail_cnt=%d\n", sta_stats->tx_fail_cnt);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;      

	ret = os_snprintf(pos, end - pos, "timeout_cnt=%d\n", sta_stats->tx_timeout_cnt);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "apt=%d\n", sta_stats->tx_avg_process_time);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    
    
	ret = os_snprintf(pos, end - pos, "aat=%d\n", sta_stats->tx_avg_air_time);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "TC_buf_full_cnt=%d:%d:%d:%d\n", 
                                    sta_stats->tc_buf_full_cnt[TC0_INDEX],
                                    sta_stats->tc_buf_full_cnt[TC1_INDEX], 
                                    sta_stats->tc_buf_full_cnt[TC2_INDEX], 
                                    sta_stats->tc_buf_full_cnt[TC3_INDEX]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "TC_sta_que_len=%d:%d:%d:%d\n", 
                                    sta_stats->tc_que_len[TC0_INDEX], 
                                    sta_stats->tc_que_len[TC1_INDEX], 
                                    sta_stats->tc_que_len[TC2_INDEX], 
                                    sta_stats->tc_que_len[TC3_INDEX]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "TC_avg_que_len=%d:%d:%d:%d\n", 
                                    sta_stats->tc_avg_que_len[TC0_INDEX], 
                                    sta_stats->tc_avg_que_len[TC1_INDEX], 
                                    sta_stats->tc_avg_que_len[TC2_INDEX], 
                                    sta_stats->tc_avg_que_len[TC3_INDEX]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "TC_cur_que_len=%d:%d:%d:%d\n", 
                                    sta_stats->tc_cur_que_len[TC0_INDEX], 
                                    sta_stats->tc_cur_que_len[TC1_INDEX], 
                                    sta_stats->tc_cur_que_len[TC2_INDEX], 
                                    sta_stats->tc_cur_que_len[TC3_INDEX]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;        

	ret = os_snprintf(pos, end - pos, "flag=0x%08X\n", sta_stats->flag);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;    

	ret = os_snprintf(pos, end - pos, "reserved0=");
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;        
    for(i = 0; i < 16; i++) {
    	ret = os_snprintf(pos, end - pos, "%02X", sta_stats->reserved[i]);
    	if (ret < 0 || ret >= end - pos)
    		return 0;
    	pos += ret;   

        if(((i + 1) % 4) == 0) {
        	ret = os_snprintf(pos, end - pos, " ", sta_stats->reserved[i]);
        	if (ret < 0 || ret >= end - pos)
        		return 0;
        	pos += ret;   
        }
    }
	ret = os_snprintf(pos, end - pos, "\n", sta_stats->reserved[i]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;       

	ret = os_snprintf(pos, end - pos, "reserved1=");
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;        
    for(i = 16; i < 32; i++) {
    	ret = os_snprintf(pos, end - pos, "%02X", sta_stats->reserved[i]);
    	if (ret < 0 || ret >= end - pos)
    		return 0;
    	pos += ret;   

        if(((i + 1) % 4) == 0) {
        	ret = os_snprintf(pos, end - pos, " ", sta_stats->reserved[i]);
        	if (ret < 0 || ret >= end - pos)
        		return 0;
        	pos += ret;   
        }
    }
	ret = os_snprintf(pos, end - pos, "\n", sta_stats->reserved[i]);
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;  
    
	ret = os_snprintf(pos, end - pos, "====\n");
	if (ret < 0 || ret >= end - pos)
		return 0;
	pos += ret;

	return pos - buf;
}

#ifdef CONFIG_MTK_POOR_LINK_DETECT
int wpas_get_sta_link_detect(struct wpa_supplicant *wpa_s, u8 *buf)
{
	struct wpa_driver_get_sta_link_detect_params params;
    
	os_memset(&params, 0, sizeof(params));
	wpa_printf(MSG_DEBUG, "get_sta_link_detect");
	params.hdr.index = 0x20;
	params.hdr.index = params.hdr.index | (0x01 << 24);
	params.hdr.buflen = sizeof(struct wpa_driver_get_sta_link_detect_params);

	/* buffer for return structure */ 
	params.buf = buf;
	
	return wpa_driver_nl80211_testmode(wpa_s->drv_priv, (u8 *)&params, 
        	sizeof(struct wpa_driver_get_sta_link_detect_params));
}
#endif
/*  [ALPS00618361] [WFD Quality Enhancement] [changelist 1686130] */
int wpa_driver_get_sta_statistics(struct wpa_supplicant *wpa_s, char *addr, 
                                char *buf, size_t buflen)
{
	char *str = NULL;
	int len = 0;
    u8 sta_addr[ETH_ALEN];

    struct wpa_driver_sta_statistics_s sta_statistics;
	

	if (hwaddr_aton(addr, sta_addr)) {
		wpa_printf(MSG_DEBUG, "CTRL_IFACE GET_STA_STATISTICS: invalid "
			   "address '%s'", addr);
		return -1;
	}
    
	if (wpas_get_sta_statistics(wpa_s, sta_addr, (u8 *)&sta_statistics) < 0) {		
		wpa_printf(MSG_DEBUG, "CTRL_IFACE GET_STA_STATISTICS: command failed");
		return -1;
	}
	len = print_sta_statistics(wpa_s, &sta_statistics, 0x00, buf, buflen);

	return len;

}

#ifdef CONFIG_MTK_POOR_LINK_DETECT
static int wpa_driver_get_sta_link_detect(struct wpa_supplicant *wpa_s,
                                char *buf, size_t buflen)
{
	int ret;
	char *pos, *end;
	struct wpa_driver_sta_link_detect_s sta_link_detect;

	//wpa_printf(MSG_DEBUG, "Get sta(%s) statistics", addr);
    
	pos = buf;
	end = buf + buflen;
	if (wpas_get_sta_link_detect(wpa_s, (u8 *)&sta_link_detect) < 0) {
		wpa_printf(MSG_DEBUG, "CTRL_IFACE GET_STA_LINK_DETECT: command failed");
        return -1;    
	}
	
	ret = os_snprintf(pos, end - pos, "rFailedCount=%llu\n", sta_link_detect.tx_fail_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      
	
	ret = os_snprintf(pos, end - pos, "rRetryCount=%llu\n", sta_link_detect.tx_retry_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      
	
	ret = os_snprintf(pos, end - pos, "rMultipleRetryCount=%llu\n", sta_link_detect.tx_multi_retry_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      
	ret = os_snprintf(pos, end - pos, "rACKFailureCount=%llu\n", sta_link_detect.ack_fail_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      
	
	ret = os_snprintf(pos, end - pos, "rFCSErrorCount=%llu\n", sta_link_detect.fcs_err_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      

	ret = os_snprintf(pos, end - pos, "tx_total=%llu\n", sta_link_detect.tx_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      

	ret = os_snprintf(pos, end - pos, "tx_ok=%llu\n", sta_link_detect.tx_ok_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      

	ret = os_snprintf(pos, end - pos, "tx_os=%llu\n", sta_link_detect.tx_os_cnt);
	if (ret < 0 || ret >= end - pos)
		return (pos - buf);
	pos += ret;      

	return (pos - buf);

}
#endif

#ifdef CONFIG_MTK_P2P_SIGMA
static int wpas_p2p_sigma_test_mode(struct wpa_supplicant *wpa_s,
	                           int index, int value)
{
    struct wpa_driver_p2p_sigma_params params;

    os_memset(&params, 0, sizeof(params));
    
    params.hdr.index = 1;
    params.hdr.index = params.hdr.index | (0x01 << 24);
    params.hdr.buflen = sizeof(struct wpa_driver_p2p_sigma_params);
    
    params.idx = (u32)index;
    params.value = (u32)value;
    
    return wpa_driver_nl80211_testmode(wpa_s->drv_priv, (u8 *)&params, 
        sizeof(struct wpa_driver_p2p_sigma_params));
}
static int p2p_ctrl_iface_set_opps(
    struct wpa_supplicant *wpa_s, char *cmd, char *buf, size_t buflen)
{
    char *str = NULL;
    u8 addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int len = 0;
    size_t ssid_len = 0;
    char *ssid;
    int CTWin;

    wpa_printf(MSG_DEBUG, "CTRL_IFACE set_opps cmd=%s\n", cmd);

    CTWin = atoi(cmd);

    str = os_strchr(cmd, ' ');
    *str ++ = '\0';

    if (hwaddr_aton(str, addr))
        return -1;

    str = os_strchr(str, ' ');
    *str ++ = '\0';

    ssid = wpa_config_parse_string(str, &ssid_len);
    if (ssid) {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE set_opps CTWin=%d "MACSTR" SSID(%d)%s\n",
            CTWin, MAC2STR(addr), ssid_len, ssid);
        os_free(ssid);
    }
    else {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE set_opps CTWin=%d "MACSTR" SSID(%d)\n",
            CTWin, MAC2STR(addr), ssid_len);
    }

    wpas_p2p_sigma_test_mode(wpa_s, 107, (int)CTWin);

    //len = os_snprintf(buf, buflen, "return OK");

    return len;

}
static int p2p_ctrl_iface_set_power_save(
    struct wpa_supplicant *wpa_s, char *cmd, char *buf, size_t buflen)
{
    char *str = NULL;
    int len = 0;
    int value = 0;

    wpa_printf(MSG_DEBUG, "CTRL_IFACE set_power_save cmd=%s\n", cmd);

    value = atoi(cmd);

    wpa_printf(MSG_DEBUG, "CTRL_IFACE set_power_save value=%d\n", value);

    wpas_p2p_sigma_test_mode(wpa_s, 108, (int)value);

    //len = os_snprintf(buf, buflen, "return OK");

    return len;

}
static int p2p_ctrl_iface_set_sleep(
    struct wpa_supplicant *wpa_s, char *cmd, char *buf, size_t buflen)
{
    char *str = NULL;
    u8 addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int len = 0;
    size_t ssid_len = 0;
    char *ssid;

    if (hwaddr_aton(cmd, addr))
        return -1;

    str = os_strchr(cmd, ' ');
    *str ++ = '\0';

    ssid = wpa_config_parse_string(str, &ssid_len);
    if (ssid) {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE set_sleep "MACSTR" SSID(%d)%s\n",
            MAC2STR(addr), ssid_len, ssid);
        os_free(ssid);
    }
    else {
        wpa_printf(MSG_DEBUG, "CTRL_IFACE set_sleep "MACSTR" SSID(%d)\n",
            MAC2STR(addr), ssid_len);
    }

    wpas_p2p_sigma_test_mode(wpa_s, 106, 0);

    //len = os_snprintf(buf, buflen, "return OK");

    return len;

}
#endif

int wpa_driver_nl80211_driver_cmd(void *priv, char *cmd, char *buf,
				  size_t buf_len )
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct wpa_supplicant *wpa_s;
	struct hostapd_data *hapd;
	int ret = -1;

	if (drv == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv is NULL. Exiting", __func__);
		return -1;
	}
	if (drv->ctx == NULL) {
		wpa_printf(MSG_ERROR, "%s: drv->ctx is NULL. Exiting", __func__);
		return -1;
	}

	if (os_strcmp(bss->ifname, "ap0") == 0) {
		hapd = (struct hostapd_data *)(drv->ctx);
	}
	else {
		wpa_s = (struct wpa_supplicant *)(drv->ctx);
		if (wpa_s->conf == NULL) {
			wpa_printf(MSG_ERROR, "%s: wpa_s->conf is NULL. Exiting", __func__);
			return -1;
		}
	}

	wpa_printf(MSG_DEBUG, "iface %s recv cmd %s", bss->ifname, cmd);

	if (os_strncasecmp(cmd, "POWERMODE ", 10) == 0) {
		int state;
		state = atoi(cmd + 10);
		wpa_printf(MSG_DEBUG, "POWERMODE=%d", state);
	}  else if (os_strncasecmp(cmd, "GET_STA_STATISTICS ", 19) == 0) { 
		ret = wpa_driver_get_sta_statistics(wpa_s, cmd + 19, buf, buf_len);
#ifdef CONFIG_MTK_POOR_LINK_DETECT		
	}  else if (os_strncasecmp(cmd, "GET_STA_LINK_DETECT", 19) == 0) { 
		ret = wpa_driver_get_sta_link_detect(wpa_s, buf, buf_len);
#endif		
	}  else if (os_strncmp(cmd, "MACADDR", os_strlen("MACADDR"))==0) {
		u8 macaddr[ETH_ALEN] = {};
        os_memcpy(&macaddr, wpa_s->own_addr, ETH_ALEN);
        ret = snprintf(buf, buf_len, "Macaddr = " MACSTR "\n", MAC2STR(macaddr));
        wpa_printf(MSG_DEBUG, "%s", buf);
    } else if(os_strncasecmp(cmd, "COUNTRY", os_strlen("COUNTRY"))==0) {
        wpa_printf(MSG_DEBUG, "set country: %s", cmd+8);
        //ret = wpa_drv_set_country(wpa_s, cmd+8);
        ret = wpa_driver_mediatek_set_country(priv, cmd+8);
        if(ret == 0) {
            //os_memcpy(wpa_s->conf->country,cmd+8,2);
            //ret = wpa_config_write(wpa_s->confname, wpa_s->conf);
        }
    } else if (os_strcasecmp(cmd, "start") == 0) {
        if (ret = linux_set_iface_flags(drv->global->ioctl_sock, 
                                       drv->first_bss.ifname, 1)) {
			wpa_printf(MSG_INFO, "nl80211: Could not set interface UP, ret=%d \n", ret);
		} else {
			wpa_msg(drv->ctx, MSG_INFO, "CTRL-EVENT-DRIVER-STATE STARTED");
		}
    } else if (os_strcasecmp(cmd, "stop") == 0) {
        if (drv->associated) {
        	ret = wpa_drv_deauthenticate(wpa_s, drv->bssid, WLAN_REASON_DEAUTH_LEAVING);
        	if (ret != 0)
        		wpa_printf(MSG_DEBUG,"DRIVER-STOP error, ret=%d", ret);
        } else {
        	wpa_printf(MSG_INFO, "nl80211: not associated, no need to deauthenticate \n");
        }
        
		if (ret = linux_set_iface_flags(drv->global->ioctl_sock, 
                                       drv->first_bss.ifname, 0)) {
			wpa_printf(MSG_INFO, "nl80211: Could not set interface Down, ret=%d \n", ret);
		} else {
			wpa_msg(drv->ctx, MSG_INFO, "CTRL-EVENT-DRIVER-STATE STOPPED");
		}
    } else if (os_strncasecmp(cmd, "getpower", 8) == 0) {
        u32 mode;
//        ret = wpa_driver_wext_driver_get_power(drv, &mode);
        if (ret == 0) {
            ret = snprintf(buf, buf_len, "powermode = %u\n", mode);
            wpa_printf(MSG_DEBUG, "%s", buf);
            if (ret < (int)buf_len) {
                return( ret );
            }
        }
    } else if (os_strncasecmp(cmd, "get-rts-threshold", 17) == 0) {
        u32 thd;
//        ret = wpa_driver_wext_driver_get_rts(drv, &thd);
        if (ret == 0) {
            ret = snprintf(buf, buf_len, "rts-threshold = %u\n", thd);
            wpa_printf(MSG_DEBUG, "%s", buf);
            if (ret < (int)buf_len)
                return ret;
        }
    } else if (os_strncasecmp(cmd, "set-rts-threshold", 17) == 0) {
        u32 thd = 0;
        char *cp = cmd + 17;
        char *endp;
        if (*cp != '\0') {
            thd = (u32)strtol(cp, &endp, 0);
//            if (endp != cp)
//                ret = wpa_driver_wext_driver_set_rts(drv, thd);
        }
	} else if (os_strncasecmp(cmd, "rxfilter-add", 12) == 0 ) {
		unsigned long sw_cmd = 0x9F000000;
		unsigned long idx = 0;
		char *cp = cmd + 12;
		char *endp;
	
		if (*cp != '\0') {
			idx = (u32)strtol(cp, &endp, 0);
			if (endp != cp) {
				idx += 0x00900200;
				wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
				ret = 0;
			}
		}
	} else if (os_strncasecmp(cmd, "rxfilter-remove", 15) == 0 ) {
		unsigned long sw_cmd = 0x9F000000;
		unsigned long idx = 0;
		char *cp = cmd + 15;
		char *endp;
	
		if (*cp != '\0') {
			idx = (u32)strtol(cp, &endp, 0);
			if (endp != cp) {
				idx += 0x00900300;
				wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
				ret = 0;
			}
		}
	} else if (os_strncasecmp(cmd, "rxfilter-stop", 13) == 0 ) {
		unsigned long sw_cmd = 0x9F000000;
		unsigned long idx = 0x00900000;
		wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
		ret = 0;
	} else if (os_strncasecmp(cmd, "rxfilter-start", 14) == 0 ) {
		unsigned long sw_cmd = 0x9F000000;
		unsigned long idx = 0x00900100;
		wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
		ret = 0;
    } else if (os_strcasecmp(cmd, "btcoexscan-start") == 0) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if (os_strcasecmp(cmd, "btcoexscan-stop") == 0) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if( os_strncasecmp(cmd, "btcoexmode", 10) == 0 ) {
        ret = 0; /* mt5921 linux driver not implement yet */
    } else if (os_strncasecmp(cmd, "smt-rate", 8) == 0 ) {
        unsigned long sw_cmd = 0xFFFF0123;
        unsigned long idx = 0;
        char *cp = cmd + 8;
        char *endp;

        if (*cp != '\0') {
            idx = (u32)strtol(cp, &endp, 0);
            if (endp != cp) {
                wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
                ret = 0;
            }
        }
    } else if (os_strncasecmp(cmd, "smt-test-on", 11) == 0 ) {
        unsigned long sw_cmd = 0xFFFF1234;
        unsigned long idx = 0;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
    } else if (os_strncasecmp(cmd, "smt-test-off", 12) == 0 ) {
        unsigned long sw_cmd = 0xFFFF1235;
        unsigned long idx = 0;
        wpa_driver_nl80211_driver_sw_cmd(priv, 1, &sw_cmd, &idx);
        ret = 0;
#ifdef CONFIG_HOTSPOT_MGR_SUPPORT
    } else if (os_strncmp(cmd, "STA-BLOCK ", 10) == 0) {
        if (wpa_driver_sta_block(priv, cmd + 10)) {
            ret = -1;
        } else {
            ret = 0;
        }
    } else if (os_strncmp(cmd, "STA-UNBLOCK ", 12) == 0) {
        if (wpa_driver_sta_unblock(priv, cmd + 12)) {
            ret = -1;
        } else {
            ret = 0;
        }
#endif /* CONFIG_HOTSPOT_MGR_SUPPORT */
#ifdef CONFIG_WAPI_SUPPORT
	} else if (os_strncasecmp(cmd, "set-wapi-key", 12) == 0) {
        struct wapi_key_param_type{
            u8 *addr;
            int key_idx;
        	int set_tx;
            u8 *seq;
        	size_t seq_len;
        	u8 *key;
            size_t key_len;
        } *wapi_key_param;
        wapi_key_param = (struct wapi_key_param_type*)buf;

        ret = wpa_driver_nl80211_set_wapi_key(priv, 
                    (const u8*)wapi_key_param->addr,
                    wapi_key_param->key_idx, wapi_key_param->set_tx, 
                    (const u8*)wapi_key_param->seq, wapi_key_param->seq_len, 
                    (const u8*)wapi_key_param->key, wapi_key_param->key_len);
    } else if (os_strncasecmp(cmd, "wapi-msg-send", 13) == 0) {
        struct wapi_msg_send_param_type {
            u8 *msg_in;
            int msg_in_len;
        	u8 *msg_out;
            int *msg_out_len;
        } *wapi_msg_send_param;
        wapi_msg_send_param = (struct wapi_msg_send_param_type*)buf;
        ret = wpa_driver_nl80211_send_msg(priv, (const u8*)wapi_msg_send_param->msg_in,
                    wapi_msg_send_param->msg_in_len, wapi_msg_send_param->msg_out,
                    wapi_msg_send_param->msg_out_len);
#endif /* CONFIG_WAPI_SUPPORT */
#ifdef CONFIG_MTK_P2P_SIGMA  
	} else if (os_strncasecmp(cmd, "mcc", 3) == 0) {
	   if(wpa_s->drv_priv) {
		   int	  mcc = 0;
		   char *value = NULL;
		   value = os_strchr(cmd, ' ');
		   if (value == NULL)
			   return -1;
		   *value++ = '\0';		   
		   struct wpa_supplicant *_wpa_s;
		   mcc = atoi(value);
		   if(mcc) {

			   for (_wpa_s = wpa_s->global->ifaces; _wpa_s; _wpa_s = _wpa_s->next) {
				   _wpa_s->num_multichan_concurrent = 2; /* 2 is appropriate? just legacy wifi vs p2p wifi? */
				   _wpa_s->drv_flags |= WPA_DRIVER_FLAGS_P2P_CONCURRENT;
				   if (_wpa_s->global->p2p && _wpa_s->global->p2p->cfg ) {
					   _wpa_s->global->p2p->cfg->concurrent_operations = 1;
				   }
			   }
		   } else {

			   for (_wpa_s = wpa_s->global->ifaces; _wpa_s; _wpa_s = _wpa_s->next) {
				   _wpa_s->num_multichan_concurrent = 0; /* assign as 0 beacause our driver will
														   not report iface_combination to supplicant*/
				   _wpa_s->drv_flags &= ~WPA_DRIVER_FLAGS_P2P_CONCURRENT;
				   if (_wpa_s->global->p2p && _wpa_s->global->p2p->cfg ) {
					   _wpa_s->global->p2p->cfg->concurrent_operations = 0;
				   }
			   }

		   }

		   wpa_printf(MSG_DEBUG, "mcc = %d wpa_s drv_flags 0x%x",mcc,  wpa_s->drv_flags);
		   ret = 0;
	   }
	} else if (os_strncmp(cmd, "p2p_set_opps ", 13) == 0) {
	   ret = p2p_ctrl_iface_set_opps(wpa_s, cmd + 13, 
							buf, buf_len);
	} else if (os_strncmp(cmd, "p2p_set_power_save ", 19) == 0) {
	   ret = p2p_ctrl_iface_set_power_save(wpa_s, cmd + 19, 
							buf, buf_len);
	} else if (os_strncmp(cmd, "p2p_set_sleep ", 14) == 0) {
	   ret = p2p_ctrl_iface_set_sleep(wpa_s, cmd + 14, 
						   buf, buf_len);
	} else if (os_strncmp(cmd, "p2p_set_sleep", 13) == 0) {
	   char cmd2[] = {"ff:ff:ff:ff:ff:ff \"\""};
	   ret = p2p_ctrl_iface_set_sleep(wpa_s, cmd2, 
							buf, buf_len);	   
#endif
    } else {
        wpa_printf(MSG_DEBUG,"Unsupported command");
    }

    return ret;
}

int wpa_driver_set_p2p_noa(void *priv, u8 count, int start, int duration)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;

	wpa_printf(MSG_DEBUG, "iface %s P2P_SET_NOA %d %d %d, ignored", bss->ifname, count, start, duration); 
	return -1;
}

int wpa_driver_get_p2p_noa(void *priv, u8 *buf, size_t len)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;

	wpa_printf(MSG_DEBUG, "iface %s P2P_GET_NOA, ignored", bss->ifname); 
	return -1;
}

int wpa_driver_set_p2p_ps(void *priv, int legacy_ps, int opp_ps, int ctwindow)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;

	wpa_printf(MSG_DEBUG, "iface %s P2P_SET_PS, ignored", bss->ifname); 
	return -1;
}

int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
				 const struct wpabuf *proberesp,
				 const struct wpabuf *assocresp)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;

	wpa_printf(MSG_DEBUG, "iface %s set_ap_wps_p2p_ie, ignored", bss->ifname); 
	return 0;
}

