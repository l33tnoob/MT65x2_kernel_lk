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

package com.mediatek.pushparser;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import android.util.Log;

import com.mediatek.pushparser.co.CoTextParser;
import com.mediatek.pushparser.co.CoWbxmlParser;
import com.mediatek.pushparser.si.SiTextParser;
import com.mediatek.pushparser.si.SiWbxmlParser;
import com.mediatek.pushparser.sl.SlTextParser;
import com.mediatek.pushparser.sl.SlWbxmlParser;

public abstract class Parser {
    
    public static final String TAG = "PUSH";
    //MIME Type
    protected String m_mimetype;
    
    //set mimetype
    protected Parser(String mimetype){
        m_mimetype = mimetype;
    }
    
    //create a parser according to mimetype
    public static Parser createParser(String mimetype){
        
        Parser parser = null;
        
        if(mimetype.equals("text/vnd.wap.si")){
            parser = new SiTextParser(mimetype);
        }else if(mimetype.equals("application/vnd.wap.sic")){
            parser = new SiWbxmlParser(mimetype);
        }else if(mimetype.equals("text/vnd.wap.sl")){
            parser = new SlTextParser(mimetype);
        }else if(mimetype.equals("application/vnd.wap.slc")){
            parser = new SlWbxmlParser(mimetype);
        }else if(mimetype.equals("text/vnd.wap.co")){
            parser = new CoTextParser(mimetype);
        }else if(mimetype.equals("application/vnd.wap.coc")){
            parser = new CoWbxmlParser(mimetype);
        }else{
            Log.e(TAG,"createParser: wrong type!" + mimetype);
        }
        
        return parser;
    }
    
    //
    public ParsedMessage parseFile(String filename){
        
        if (filename != null) {
            InputStream in;
            try {
                in = new FileInputStream(filename);
                ParsedMessage msg = parse(in);
                in.close();
                return msg;
            } catch (FileNotFoundException e) {
                //e.printStackTrace();
                Log.e(TAG,"File Not Found" + filename);
                return null;
            } catch (IOException e){
                Log.e(TAG,"InputStream Close Error");
                return null;
            }
        }
        return null;
    }
    
    //
    public ParsedMessage parseData(byte[] data){
        
        if(data!=null){
            InputStream in = new ByteArrayInputStream(data);
            ParsedMessage msg = parse(in);
            try {
                in.close();
            } catch (IOException e) {
                //e.printStackTrace();
                Log.e(TAG,"InputStream Close Error:");
                return null;
            }
            return msg;
        }
        return null;
    }
    
    //
    protected abstract ParsedMessage parse(InputStream in);
}
