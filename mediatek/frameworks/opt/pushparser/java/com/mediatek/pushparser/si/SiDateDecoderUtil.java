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

package com.mediatek.pushparser.si;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

public class SiDateDecoderUtil {
    static SimpleDateFormat FORMATTER_WBXML = new SimpleDateFormat("yyyyMMddHHmmss");
    
    static SimpleDateFormat FORMATTER_XML = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
    
    /*
     * In wbxml,date string like this:20100812024800
     * '0' in the tail will be trimmed
     */
    static int WbXmlDateDecoder(String dateStr){
        if(dateStr == null){
            return 0;
        }
        try {
        	FORMATTER_WBXML.setTimeZone(TimeZone.getTimeZone("GMT"));
            if(dateStr.length()<14){
                //pading '0'
                String res = String.format("%-14s", dateStr).replace(' ', '0');
                return (int)(FORMATTER_WBXML.parse(res).getTime()/1000);
            }else{
                return (int)(FORMATTER_WBXML.parse(dateStr).getTime()/1000);
            }
        } catch (ParseException e) {
            //throw new RuntimeException(e);
            return 0;
        }
    }
    
    /*
     * In xml,date string like this:2010-08-12T02:48:00Z 
     */
    static int XmlDateDecoder(String dateStr){
        if(dateStr  == null){
            return 0;
        }
        
        try {
        	FORMATTER_XML.setTimeZone(TimeZone.getTimeZone("GMT"));
            return (int)(FORMATTER_XML.parse(dateStr).getTime()/1000);
        } catch (ParseException e) {
            //throw new RuntimeException(e);
            return 0;
        }
    }
}
