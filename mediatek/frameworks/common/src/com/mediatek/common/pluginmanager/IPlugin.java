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

package com.mediatek.common.pluginmanager;

import android.content.Context;
import android.content.res.Resources;
import java.lang.Exception;

public interface IPlugin {
    /**
     * To get Context of plug-in APP
     * @return Context of plug-in APP
     */
    Context getContext();

    /**
     * To create an instance which its class name defined as value of meta-data, "class", in plug-in APP.
     * @return The instance
     * @throws ObjectCreationException If the class is not defined in plug-in APP or the plug-in APP is not valid anymore.
     */
    Object createObject() throws Exception;
    
    /**
     * To create an instance which its class name being as value of input meta-data item in plug-in APP.
     * @param metaName The name of meta-data item
     * @return The instance
     * @throws ObjectCreationException If the class is not defined in plug-in APP or the plug-in APP is not valid anymore.
     */
    Object createObject(String metaName) throws Exception;

    /**
     * To get plug-in APP name
     * @return Name of plug-in APP
     */
    CharSequence getName();

    /**
     * To get the string defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The string assigned for input meta-data name
     */
    String getMetaDataValueString(String metaName);

    /**
     * To get the integer defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The integer assigned for input meta-data name
     */
    int getMetaDataValueInt(String metaName);

    /**
     * To get the boolean defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The boolean assigned for input meta-data name
     */
    boolean getMetaDataValueBoolean(String metaName);

    /**
     * To get the float defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The float assigned for input meta-data name
     */
    float getMetaDataValueFloat(String metaName);

    /**
     * To get the color defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The color assigned for input meta-data name
     */
    int getMetaDataValueColor(String metaName);
    
    /**
     * To get the resource id defined with input meta-data name
     * @param metaName  The name of meta-data item
     * @return The integer assigned as android:resources for input meta-data name
     */
    int getMetaDataResourceID(String metaName);

    /**
     * To get the resource of plug-in APP
     * @return resource of plug-in APP
     */
    Resources getResources();
}