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

#ifndef _UISTRINGS_CHN_H_
#define _UISTRINGS_CHN_H_

/*use define rather than variable. because initializer element must be constant*/
/*No Chinese Punctuation!   暂不支持中文标点，用英文标点代替*/

#define uistr_factory_mode 		 "工厂模式"
#define uistr_full_test 		 "手动测试"
#define uistr_auto_test			"自动测试"
#define uistr_item_test 		 "单项测试"
#define uistr_test_report		 "测试报告"
#define uistr_debug_test	"调试测试项"
#define uistr_clear_flash 	 "清除闪存"
#define uistr_clear_emmc 		 "清除eMMC"
#define uistr_version 			 "版本信息"
#define uistr_reboot 				 "重启手机"
#define uistr_pass 					 "测试通过"
#define uistr_fail 					 "测试失败"
#define uistr_retest 					   "重新测试"
#define uistr_key_back 					 "返回"
#define uistr_key_pass 					 "通过"
#define uistr_key_fail 					 "失败"

#define uistr_keys 					 "按键"
#define uistr_jogball 			 "轨迹球"
#define uistr_ofn 					 "光学手指导航键"
#define uistr_touch 				 "触摸屏"
#define uistr_touch_auto 				 "触摸屏自动测试"
#define uistr_backlight_level 				 "背光等级"
#define uistr_lcm 				 "显示屏"
#define uistr_nand_flash 		 "NAND闪存"
#define uistr_emmc 					 "eMMC"
#define uistr_memory_card 	 "存储卡"
#define uistr_sim_card 			 "SIM卡"
#define uistr_sim_detect 		 "SIM卡探测"
#define uistr_sig_test 			 "信号测试"
#define uistr_vibrator 			 "振动器"
#define uistr_led 					 "LED灯"
#define uistr_rtc 					 "实时时钟"
#define uistr_system_stability                        "系统稳定性"
#define uistr_nfc 					 "近场通信NFC"
#define uistr_cmmb                   "手机电视"
#define uistr_gps               "GPS定位"
#define uistr_atv                   "移动电视"
#define uistr_wifi						"Wi-Fi"
#define uistr_bluetooth      "蓝牙"
#define uistr_idle       "闲置电流"
#define uistr_g_sensor			"加速度传感器"
#define uistr_g_sensor_c		"加速度传感器校准"
#define uistr_m_sensor			"磁力计传感器"
#define uistr_rf_test    "RF Test"
#define uistr_als_ps				"光传感器距离传感器"
#define uistr_gyroscope			"陀螺仪传感器"
#define uistr_gyroscope_c		"陀螺仪传感器校准"
#define uistr_barometer			"气压传感器"
/*audio*/
#define uistr_info_audio_yes							                "是"
#define uistr_info_audio_no							                  "否"
#define uistr_info_audio_press							              "按下"
#define uistr_info_audio_release							            "释放"
#define uistr_info_audio_ringtone							            "铃声"
#define uistr_info_audio_receiver							            "接收器"
#define uistr_info_audio_loopback							            "回路"
#define uistr_info_audio_loopback_phone_mic_headset				"回路(手机麦克风-耳机)"
#define uistr_info_audio_loopback_phone_mic_speaker				"手机麦克扬声器回路"
#define uistr_info_audio_loopback_headset_mic_speaker			"回路(耳机麦克风-扬声器)"
#define uistr_info_audio_loopback_waveplayback						"Wave回放"
#define uistr_info_audio_loopback_note							      "如果测试耳机麦克风到接收器的回路, \n请插入耳机...\n\n\n"
#define uistr_info_audio_headset_note                     "请插入耳机...\n\n"
#define uistr_info_audio_headset_avail                    "耳机可用: %s\n\n"
#define uistr_info_audio_headset_mic_avail                "耳机麦克风: %s\n\n"
#define uistr_info_audio_headset_Button                   "耳机按键: %s\n\n"
#define uistr_info_audio_loopback_complete							  "回路测试完成"
#define uistr_info_audio_loopback_headset_mic						  "测试耳机MIC回路"
#define uistr_info_audio_loopback_dualmic_mic							"测试MIC回路"
#define uistr_info_audio_loopback_dualmic_mi1							"测试MIC1回路"
#define uistr_info_audio_loopback_dualmic_mi2							"测试MIC2回路"
#define uistr_info_audio_acoustic_loopback							  "双麦克风回路"
#define uistr_info_audio_acoustic_loopback_DMNR           "双麦克风回路[有DMNR]"
#define uistr_info_audio_acoustic_loopback_without_DMNR		"双麦克风回路[无DMNR]"
#define uistr_info_audio_headset_debug "耳机调试"
#define uistr_info_audio_receiver_debug "接收器调试"
#define uistr_info_audio_micbias                          "Micbias 测试"
#define uistr_info_audio_micbias_on                       "Micbias ON"
#define uistr_info_audio_micbias_off                      "Micbias OFF"

/*phone*/
#define uistr_info_audio_receiver_phone							            "接收器(Phone)"
#define uistr_info_headset_phone							                  "耳机(Phone)"
#define uistr_info_audio_loopback_phone_mic_speaker_phone				"手机麦克(Phone)"
#define uistr_vibrator_phone 			 "振动器(Phone)"

/*speaker oc*/
#define uistr_info_speaker_oc							                "扬声器OC测试"
#define uistr_info_speaker_oc_pass							          "扬声器OC测试通过[%d]\n\n"
#define uistr_info_speaker_oc_fail							          "扬声器OC测试失败[%d]\n\n"
#define uistr_info_speaker_oc_retest							        "扬声器OC测试重新开始\n\n"

/*headset*/
#define uistr_info_headset							                  "耳机"
#define uistr_info_avail						                  "耳机插入"
#define uistr_info_button						                  "耳机按键"
#define uistr_info_press						                  "按下"
#define uistr_info_release						                  "释放"



/*emergency_call*/
#define uistr_info_emergency_call							 "拨打紧急电话112"
#define uistr_info_emergency_call_not_start		 "没有开始"
#define uistr_info_emergency_call_testing      "正在测试"
#define uistr_info_emergency_call_success			 "拨打112成功!"
#define uistr_info_emergency_call_fail			 "拨打112失败!"
#define uistr_info_emergency_call_success_in_modem1           "modem1 拨打112成功! "
#define uistr_info_emergency_call_fail_in_modem1           "modem1 拨打112失败!"
#define uistr_info_emergency_call_success_in_modem2           "modem2 拨打112成功! "
#define uistr_info_emergency_call_fail_in_modem2           "modem2 拨打112失败!"
#define uistr_info_emergency_call_success_in_modem5           "modem5 拨打112成功! "
#define uistr_info_emergency_call_fail_in_modem5           "modem5 拨打112失败!"

/*Gyro*/

/*CMMB*/
#define uistr_info_cmmb_autoscan             "自动搜索"
#define uistr_info_cmmb_channellist          "频点列表"
#define uistr_info_cmmb_init_ok              "CMMB驱动初始化成功"
#define uistr_info_cmmb_init_fail            "CMMB驱动初始化失败"
#define uistr_info_cmmb_scanning             "正在搜索频点……"
#define uistr_info_cmmb_scan_ok              "搜索频点成功"
#define uistr_info_cmmb_scan_fail            "搜索频点失败"
#define uistr_info_cmmb_tune_channel         "请选择一个频点，等待锁定..."
#define uistr_info_cmmb_servicelist          "CMMB节目列表"
#define uistr_info_cmmb_selectstream         "请选择一个节目进行录制"
#define uistr_info_cmmb_recording            "CMMB正在录制节目"
#define uistr_info_cmmb_recording_to         "录制的节目流保存在 "
#define uistr_info_cmmb_stop                 "停止"
#define uistr_info_cmmb_stop_to              "录制结束,文件保存在 "

/*eMMC-SD*/
#define uistr_info_emmc			"eMMC"
#define uistr_info_sd			"SD卡"
#define uistr_info_emmc_fat		"eMMC FAT 分区"
#define uistr_info_emmc_format_item		"格式化 eMMC FAT 分区"
#define uistr_info_emmc_format_stat		"格式化状态"
#define uistr_info_emmc_format_stat_start		"正在格式化 eMMC FAT 分区..."
#define uistr_info_emmc_format_data_start   "正在格式化 eMMC data 分区..."
#define uistr_info_reboot			"正在重启手机"
#define uistr_info_emmc_format_stat_success		"格式化 eMMC FAT 分区成功!"
#define uistr_info_emmc_format_stat_fail		"格式化 eMMC FAT 分区失败!"
#define uistr_info_emmc_sd_avail		"卡状态 "
#define uistr_info_emmc_sd_yes		"有卡"
#define uistr_info_emmc_sd_no		"无卡"
#define uistr_info_emmc_sd_total_size	"总容量  "
#define uistr_info_emmc_sd_free_size	"可用容量  "
#define uistr_info_emmc_sd_checksum		"校验   "
#define uistr_info_sd1			"SD卡 1"
#define uistr_info_sd2			"SD卡 2"

/* Bluetooth */
#define uistr_info_bt_init            "状态: 蓝牙正在初始化"
#define uistr_info_bt_init_fail       "状态: 蓝牙初始化失败"
#define uistr_info_bt_init_ok         "状态: 蓝牙初始化完成"
#define uistr_info_bt_inquiry_start   "状态: 开始搜索附近蓝牙设备 \n"
#define uistr_info_bt_inquiry_1       "状态: 正在搜索设备 ----- \n"
#define uistr_info_bt_inquiry_2       "状态: 正在搜索设备 +++++ \n"
#define uistr_info_bt_scan_1          "状态: 正在获取设备名 ----- \n"
#define uistr_info_bt_scan_2          "状态: 正在获取设备名 +++++ \n"
#define uistr_info_bt_scan_complete   "状态: 搜索完毕 \n"
#define uistr_info_bt_no_dev          "----设备列表尾  未发现蓝牙设备---- \n"
#define uistr_info_bt_dev_list_end    "----设备列表尾---- \n\n"
#define uistr_info_bt_scan_list_end   "----搜索列表尾---- \n"

/*Wi-Fi*/
#define uistr_info_wifi_test_pass   "测试通过"
#define uistr_info_wifi_test_fail   "测试失败"
#define uistr_info_wifi_renew       "刷新"
#define uistr_info_wifi_error       "[错误]"
#define uistr_info_wifi_warn        "[警告]"
#define uistr_info_wifi_status      "状态"
#define uistr_info_wifi_start       "开始"
#define uistr_info_wifi_init_fail   "初始化错误"
#define uistr_info_wifi_scanning    "扫描中"
#define uistr_info_wifi_timeout     "连接超时"
#define uistr_info_wifi_disconnect  "断开"
#define uistr_info_wifi_connecting  "连接中"
#define uistr_info_wifi_connected   "连接完成"
#define uistr_info_wifi_unknown     "未知问题"
#define uistr_info_wifi_mode        "模式"
#define uistr_info_wifi_infra       "AP模式"
#define uistr_info_wifi_adhoc       "点对点模式"
#define uistr_info_wifi_channel     "信道"
#define uistr_info_wifi_rssi        "接收信号强度"
#define uistr_info_wifi_rate        "速率"

/* camera */
#define uistr_main_sensor 	"主照相机"
#define uistr_main2_sensor 	"主照相机2"
#define uistr_sub_sensor 	"副照相机"
#define uistr_camera_prv_cap_strobe "预览/拍照/闪灯"
#define uistr_camera_prv_strobe "预览/闪灯"
#define uistr_camera_prv_cap "预览/拍照"
#define uistr_camera_back       "返回"
#define uistr_camera_capture     "拍照"
#define uistr_camera_preview 	"预览"


#define uistr_strobe 	"闪光灯"


/* USB */
#define uistr_info_usb_connect 		  "连接"
#define uistr_info_usb_disconnect 	  "断开"

/* battery&charging */
#define uistr_info_title_battery_charger "电池和充电"
#define uistr_info_title_ac_charger "AC充电"
#define uistr_info_title_usb_charger "USB充电"
#define uistr_info_title_battery_yes "是"
#define uistr_info_title_battery_no "否"
#define uistr_info_title_battery_connect "连接"
#define uistr_info_title_battery_no_connect "未连接"
#define uistr_info_title_battery_cal "校准"
#define uistr_info_title_battery_val "电压"
#define uistr_info_title_battery_temp "温度"
#define uistr_info_title_battery_chr "充电器"
#define uistr_info_title_battery_chr_val "充电电压"
#define uistr_info_title_battery_chr_current "充电电流"
#define uistr_info_title_battery_ad32 "AD32"
#define uistr_info_title_battery_ad42 "AD42"
#define uistr_info_title_battery_curad "CurAD"
#define uistr_info_title_battery_fg_cur "电池端电流"
#define uistr_info_title_battery_pmic_chip "电源管理芯片"
#define uistr_info_title_battery_mv "毫伏"
#define uistr_info_title_battery_ma "毫安"
#define uistr_info_title_battery_c "摄氏度"

/* OTG */
#define uistr_info_otg_status "当前OTG状态"
#define uistr_info_otg_status_device "设备"
#define uistr_info_otg_status_host "主机"

/* LED */
#define uistr_info_nled_test			"NLED 测试"
#define uistr_info_keypad_led_test		"按键LED测试"

/* Backlight */
#define uistr_info_show_test_images		"显示图像"
#define uistr_info_change_contrast		"改变亮度"

/* System Stability */
#define uistr_info_stress_test_result             "压力测试结果"

/* GPS */
#define uistr_info_gps_hot_restart    "热启动"
#define uistr_info_gps_cold_restart   "冷启动"
#define uistr_info_gps_error          "GPS失败"
#define uistr_info_gps_fixed          "首次定位时间(s)"
#define uistr_info_gps_ttff           "定位时间(s)"
#define uistr_info_gps_svid           "卫星"
#define uistr_info_gps_init           "GPS初始化..."

/*NAND*/
#define uistr_info_nand_clear_flash		"清空闪存"
#define uistr_info_nand_Manufacturer_ID		"制造商识别码"
#define uistr_info_nand_Device_ID		"设备识别码"
#define uistr_info_nand_R_W_tests_result		"读写测试结果"
#define uistr_info_nand_erase_info		" \n\n 正在清空/data(%d)...请等待 \n\n\n 操作完成后系统会自动重启!"

/*mATV*/
#define uistr_info_atv_autoscan             "自动搜索"
#define uistr_info_atv_channellist          "频道列表"
#define uistr_info_atv_init_ok              "初始化成功"
#define uistr_info_atv_initizling              "正在打开，请等待"
#define uistr_info_atv_init_fail            "驱动初始化失败"
#define uistr_info_atv_previewCH                 "测试频道"
#define uistr_info_atv_switchCH                 "切换频道"
#define uistr_info_atv_refreshCH                 "刷新频道"
#define uistr_info_atv_CH 				 "频道"
#define	Country_AFGHANISTAN		"阿富汗"
#define	Country_ARGENTINA		"阿根廷"
#define Country_AUSTRALIA		"澳大利亚"
#define Country_BRAZIL		"巴西"
#define Country_BURMA		"缅甸"
#define Country_CAMBODIA		"柬埔寨"
#define Country_CANADA		"加拿大"
#define Country_CHILE		"智利"
#define Country_CHINA		"中国大陆"
#define Country_CHINA_HONGKONG		"中国香港"
#define Country_CHINA_SHENZHEN		"中国深圳"
#define Country_EUROPE_EASTERN		"东欧"
#define Country_EUROPE_WESTERN		"西欧"
#define Country_FRANCE			"法国"
#define Country_FRENCH_COLONIE		"法属殖民地"
#define Country_INDIA		"印度"
#define Country_INDONESIA		"印度尼西亚"
#define Country_IRAN		"伊朗"
#define Country_ITALY		"意大利"
#define Country_JAPAN		"日本"
#define Country_KOREA		"韩国"
#define Country_LAOS		"老挝"
#define Country_MALAYSIA		"马来西亚"
#define Country_MEXICO		"墨西哥"
#define Country_NEWZEALAND		"新西兰"
#define Country_PAKISTAN		"巴基斯坦"
#define Country_PARAGUAY		"巴拉圭"
#define Country_PHILIPPINES		"菲律宾"
#define Country_PORTUGAL		"葡萄牙"
#define Country_RUSSIA			"俄罗斯"
#define Country_SINGAPORE		"新加坡"
#define Country_SOUTHAFRICA		"南非"
#define Country_SPAIN				"西班牙"
#define Country_TAIWAN			"台湾"
#define Country_THAILAND		"泰国"
#define Country_TURKEY		"土耳其"
#define Country_UNITED_ARAB_EMIRATES	"阿联酋"
#define Country_UNITED_KINGDOM		"英国"
#define Country_USA		"美国"
#define Country_URUGUAY		"乌拉圭"
#define Country_VENEZUELA		"委内瑞拉"
#define Country_VIETNAM		"越南"
#define Country_IRELAND		"爱尔兰"
#define Country_MOROCCO		"摩洛哥"
#define Country_BANGLADESH	"孟加拉"
#define Country_EXIT 				"返回"
#define Country_EXIT 				"返回"


/* TV out */

#define uistr_info_tvout_plugin 				 "电视连接线已插入"
#define uistr_info_tvout_checkifplugin 	 "请检查电视上是否可以看到彩色栅状图象"
#define uistr_info_tvout_notplugin       "*** 电视连接线没有插入 ***"
#define uistr_info_tvout_item       		 "电视输出"


/* SIM detect */
#define uistr_info_sim_detect_item_sim_1	"侦测卡1"
#define uistr_info_sim_detect_item_sim_2	"侦测卡2"
#define uistr_info_sim_detect_item_pass		"测试通过"
#define uistr_info_sim_detect_item_fail		"测试失败"
#define uistr_info_sim_detect_test_result	"侦测卡%d: %s\n"
#define uistr_info_sim_detect_result_pass	"通过"
#define uistr_info_sim_detect_result_fail	"失败"

/* FM Radio */
#define uistr_info_fmr_title                "FM收音机"
#define uistr_info_fmr_no_headset_warning   "请插入耳机!\n"
#define uistr_info_fmr_open_fail            "收音机打开失败\n"
#define uistr_info_fmr_poweron_fail         "收音机上电失败\n"
#define uistr_info_fmr_mute_fail            "静音失败\n"
#define uistr_info_fmr_poweron_ok           "收音机上电成功\n"
#define uistr_info_fmr_setfreq              "设置接收频率:"
#define uistr_info_fmr_mhz                  "兆赫兹"
#define uistr_info_fmr_fail                 "失败\n"
#define uistr_info_fmr_success              "成功\n"
#define uistr_info_fmr_rssi                 "信号强度: %d(dBm)\n"
#define uistr_info_fmr_freq0                "测试频率 0"
#define uistr_info_fmr_freq1                "测试频率 1"
#define uistr_info_fmr_freq2                "测试频率 2"
#define uistr_info_fmr_freq3                "测试频率 3"
#define uistr_info_fmr_pass                 "信号强度>=%d(dBm)测试通过"
#define uistr_info_fmr_failed               "信号强度< %d(dBm)测试失败,请重测!"

/* FM Transmitter */
#define uistr_info_fmt_title                "FM发射机"
#define uistr_info_fmt_open_fail            "发射机打开失败\n"
#define uistr_info_fmt_poweron_fail         "发射机上电失败\n"
#define uistr_info_fmt_poweron_ok           "发射机上电成功\n"
#define uistr_info_fmt_audio_out            "1KHz声音输出\n"
#define uistr_info_fmt_setfreq              "设置发射频率: "
#define uistr_info_fmt_mhz                  "兆赫兹"
#define uistr_info_fmt_fail                 "失败\n"
#define uistr_info_fmt_success              "成功\n"
#define uistr_info_fmt_check_rds_fail       "数字信号检查失败\n"
#define uistr_info_fmt_enable_rds_fail      "打开数字信号功能失败\n"
#define uistr_info_fmt_set_rds_fail         "发射数字信号失败\n"
#define uistr_info_fmt_rds                  "数字信号:"
#define uistr_info_fmt_freq0                "测试频率 0"
#define uistr_info_fmt_freq1                "测试频率 1"
#define uistr_info_fmt_freq2                "测试频率 2"
#define uistr_info_fmt_freq3                "测试频率 3"
#define uistr_info_fmt_freq4                "测试频率 4"
#define uistr_info_fmt_freq5                "测试频率 5"
#define uistr_info_fmt_freq6                "测试频率 6"
#define uistr_info_fmt_freq7                "测试频率 7"
#define uistr_info_fmt_freq8                "测试频率 8"
#define uistr_info_fmt_freq9                "测试频率 9"
#define uistr_info_fmt_pass                 "测试通过"
#define uistr_info_fmt_failed               "测试失败"

/* Touchpanel */
#define uistr_info_touch_ctp_main				"CTP 测试"
#define uistr_info_touch_calibration			"口径测试"
#define uistr_info_touch_rtp_linearity			"RTP线性测试"
#define uistr_info_touch_ctp_linearity			"CTP线性测试"
#define uistr_info_touch_rtp_accuracy		"RTP准确测试"
#define uistr_info_touch_ctp_accuracy		"CTP准确测试"
#define uistr_info_touch_sensitivity			"敏感性测试"
#define uistr_info_touch_deadzone				"边缘测试"
#define uistr_info_touch_zoom					"焦距测试"
#define uistr_info_touch_freemode				"自由触屏测试"
#define uistr_info_touch_start					"点击屏开始!"
#define uistr_info_touch_red_cross			"点击红色标记!"
#define uistr_info_touch_pass_continue		"通过!点击屏继续"

/* OFN */
#define uistr_info_ofn_fail							"测试失败"
#define uistr_info_ofn_pass						"测试通过"
#define uistr_info_ofn_back						"测试返回"
#define uistr_info_ofn_free_mode_item		"自由模式测试"
#define uistr_info_ofn_pass_item				"通过"
#define uistr_info_ofn_fail_item					"失败"
#define uistr_info_ofn_return_item				"返回"

/* G/M-Sensor & ALS/PS & Gyroscope */
#define uistr_info_sensor_back							"返回"
#define uistr_info_sensor_pass							"通过"
#define uistr_info_sensor_init_fail						"初始化失败\n"
#define uistr_info_sensor_initializing					"初始化...\n"
#define uistr_info_sensor_fail								"失败"

#define uistr_info_g_sensor_testing						"测试中"
#define uistr_info_g_sensor_unknow						"无效"
#define uistr_info_g_sensor_doing							"进行中"
#define uistr_info_g_sensor_done							"完成"
#define uistr_info_g_sensor_max							"最大"
#define uistr_info_g_sensor_min							"最小"
#define uistr_info_g_sensor_range							"最大与最小应该在0~0.2范围内"
#define uistr_info_g_sensor_selftest						"自测:"
#define uistr_info_g_sensor_statistic						"统计结果:"
#define uistr_info_g_sensor_notsupport					"不支持"
#define uistr_info_g_sensor_avg							"均值"
#define uistr_info_g_sensor_std								"标准差"

#define uistr_info_m_sensor_self								"自测"
#define uistr_info_m_sensor_notsupport								"不支持，请忽略"
#define uistr_info_m_sensor_ok									"通过"
#define uistr_info_m_sensor_testing								"测试中"
#define uistr_info_m_sensor_fail								"失败"
#define uistr_info_m_sensor_status								"状态"
#define uistr_info_m_sensor_data								"磁传感器数据"

#define uistr_info_sensor_cali_clear								"清除校准数据"
#define uistr_info_sensor_cali_do_20								"进行校准(误差20%)"
#define uistr_info_sensor_cali_do_40								"进行校准(误差40%)"
#define uistr_info_sensor_cali_ok									"执行成功"
#define uistr_info_sensor_cali_fail								"执行失败"
#define uistr_info_sensor_cali_ongoing							"执行校准中,请勿触碰"

#define uistr_info_sensor_alsps_thres_high					"距离传感器上门限"
#define uistr_info_sensor_alsps_thres_low					"距离传感器下门限"
#define uistr_info_sensor_alsps_check_command					"不支持此命令，请忽略"
#define uistr_info_sensor_alsps_result							"测试结果"

#define uistr_info_sensor_pressure_value							"气压"
#define uistr_info_sensor_temperature_value							"温度"
/*test report*/
#define uistr_info_test_report_back                        "返回"

#define uistr_info_detect_sim1                     "检测SIM1"
#define uistr_info_detect_sim2                     "检测SIM2"
#define uistr_info_test_pass                     "测试通过"
#define uistr_info_test_fail                     "测试失败"
#define uistr_info_detect_sim                     "检测SIM"
#define uistr_info_yes                     "是"
#define uistr_info_no                     "否"
#define uistr_info_fail                     "失败"
#define uistr_info_pass                     "通过"
#endif

/* NFC */
#define uistr_info_nfc_swp_test       "SWP测试"
#define uistr_info_nfc_tag_dep        "读卡测试"
#define uistr_info_nfc_card_mode      "卡模拟测试"
#define uistr_info_nfc_vcard_mode     "虚拟卡片模拟"
#define uistr_info_nfc_colse2reader   "靠近读卡机"
#define uistr_info_nfc_testing        "测试中..."
#define uistr_info_nfc_init           "初始化..."
#define uistr_info_nfc_vcard_removedSIM     "请移除SIM或是uSD"
#define uistr_info_nfc_put_dut2reader_cm     "卡模拟模式设定成功，请将待测物靠近读卡机"
#define uistr_info_nfc_put_dut2reader_vcm     "虚拟卡片模式设定成功，请将待测物靠近读卡机"
/*Hotknot*/
#define uistr_hotknot       "hotknot"
#define uistr_info_hotknot_master				   "主设备"
#define uistr_info_hotknot_slave				   "从设备"
#define uistr_info_hotknot_fail				   "失败"
#define uistr_info_hotknot_pass				   "成功"
#define uistr_info_hotknot_mode_select         "选择主或从设备"	
