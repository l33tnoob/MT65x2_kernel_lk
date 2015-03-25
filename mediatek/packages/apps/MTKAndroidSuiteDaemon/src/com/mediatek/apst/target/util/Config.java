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

package com.mediatek.apst.target.util;

/**
 * Class Name: Config
 * <p>
 * Package: com.mediatek.apst.target.util
 * <p>
 * Created on: 2010-8-5
 * <p>
 * <p>
 * Description:
 * <p>
 * Global configurations of Android PC Sync Tool Daemon.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public abstract class Config {
    // 0x00000002 version : Multi-sim Enhancement

    // public static final int VERSION_CODE = 0x00000002;
    // public static final String VERSION_STRING = "2.1128.0";

    // public static final int VERSION_CODE = 0x00000003;
    // public static final int VERSION_CODE = 1142;
    // public static final String VERSION_STRING = "2.1142.1";

    // public static final int VERSION_CODE = 1150;
    // public static final String VERSION_STRING = "2.1150.0";

    /**
     * Add pictures backup/restore
     */
    // public static final int VERSION_CODE = 1208;
    // public static final String VERSION_STRING = "2.1208.0";

    /**
     * Modify mms import
     */
    // public static final int VERSION_CODE = 1209;
    // public static final String VERSION_STRING = "2.1209.0";

    /**
     * Add backup apps feature(added by jing.jia)
     */
    // public static final int VERSION_CODE = 1215;
    // public static final String VERSION_STRING = "2.1215.0";

    /**
     * Support email in USIM , Modify SIM/USIM URI
     */
    // public static final int VERSION_CODE = 1216;
    // public static final String VERSION_STRING = "2.1216.0";

    /**
     * Support email in USIM , Modify SIM/USIM URI , don't support backup apps
     */
    // public static final int VERSION_CODE = 1217;
    // public static final String VERSION_STRING = "2.1217.0";
    /**
     * 1. Add feature option 2. Support SMS/MMS draft box backup/restore.
     */
    //public static final int VERSION_CODE = 1221;
    //public static final String VERSION_STRING = "2.1221.0";
    
    /**
     * 1. Support gemini plus. 2. Merge util with PC side.
     */
    public static final int VERSION_CODE = 1306;
    public static final String VERSION_STRING = "2.1306.0";

    public static final boolean MTK_GEMINI_SUPPORT = com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_SUPPORT;
    public static final boolean MTK_3SIM_SUPPORT = com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_3SIM_SUPPORT;
    public static final boolean MTK_4SIM_SUPPORT = com.mediatek.common.featureoption.FeatureOption.MTK_GEMINI_4SIM_SUPPORT;


}
