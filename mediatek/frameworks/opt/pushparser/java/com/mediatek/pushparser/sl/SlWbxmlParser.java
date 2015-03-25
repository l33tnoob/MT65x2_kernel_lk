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

package com.mediatek.pushparser.sl;

import static com.mediatek.pushparser.sl.SlMessage.*;
import static com.mediatek.pushparser.sl.SlTextParser.*;
import java.io.InputStream;
import org.kxml2.wap.WbxmlParser;
import org.xmlpull.v1.XmlPullParser;
import com.mediatek.pushparser.ParsedMessage;
import com.mediatek.pushparser.Parser;

import android.util.Log;

public class SlWbxmlParser extends Parser {
    
    private static String TAG = "PUSH";
    
    public SlWbxmlParser(String mimetype){
        super(mimetype);
    }
    
    @Override
    public ParsedMessage parse(InputStream input) {
        // TODO Auto-generated method stub
        SlMessage slMsg = null;
        try{
            WbxmlParser parser = new WbxmlParser();
            parser.setTagTable(0, TAG_TABLE);
            parser.setAttrStartTable(0, ATTR_START_TABLE);
            parser.setAttrValueTable(0, ATTR_VALUE_TABLE);
            parser.setInput(input,null);
     
            int eventType = parser.getEventType();
            
            while (eventType != XmlPullParser.END_DOCUMENT){
                String name = null;
                String uri = null;
                switch (eventType){
                    case XmlPullParser.START_DOCUMENT:
                        break;
                    case XmlPullParser.START_TAG:
                        name = parser.getName();
                        if (SL.equalsIgnoreCase(name)){
                            uri = parser.getNamespace();
                            
                            slMsg = new SlMessage(SlMessage.TYPE);
                            slMsg.url = parser.getAttributeValue(uri, "href");
                            String action = parser.getAttributeValue(uri, "action");
                            
                            if(action!=null){
                            	action = action.toLowerCase();
                            }
                            slMsg.action = ACTION_LOW;
                            if("execute-low".equals(action)){
                                slMsg.action = ACTION_LOW;
                            }else if("execute-high".equals(action)){
                                slMsg.action = ACTION_HIGH;
                            }else if("cache".equals(action)){
                                slMsg.action = ACTION_CACHE;
                            }
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        name = parser.getName();
                        if (SL.equalsIgnoreCase(name)){
                            //siMsg.setText(parser.getText());
                        }
                        break;
                }
                eventType = parser.next();
            }
        }catch (Exception e){
            Log.e(TAG, "Parser Error:"+e.getMessage());
            return null;
            //throw new RuntimeException(e);
        }
        return slMsg;

    }
    public static final String [] TAG_TABLE = {
        "sl",
      };
      public static final String [] ATTR_START_TABLE = {
        "action=execute-low",
        "action=execute-high",
        "action=cache",
        "href",
        "href=http://",
        "href=http://www.",
        "href=https://",
        "href=https://www.",
      };
      public static final String [] ATTR_VALUE_TABLE = {
        ".com/",
        ".edu/",
        ".net/",
        ".org/"
      };
}
