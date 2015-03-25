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

package com.mediatek.bluetooth.opp.mmi;

import android.content.Context;
import android.net.Uri;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.SystemUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.regex.*;
import android.util.Patterns;

/**
 *
 */
public class Utils {

    private static final SimpleDateFormat TIME_FILENAME_FORMATER = new SimpleDateFormat(
            "yyyyMMdd_HHmmss");

    /**
     * get a valid (can be created) filename for given hint this function will
     * make all parent dirs for the final file.
     *
     * @param context
     * @param hint
     * @return
     */
    public static String getValidStoragePath(Context context, String hint) {

        // default: dir + hint (but hint maybe invalid. e.g. "@#$%^? )
        String storageDirectory = SystemUtils.getReceivedFilePath(context);
        File dir = new File(storageDirectory);

        // make sure dir is ready
        if (!dir.exists()) {
            dir.mkdirs();
        }

        String result = Utils.isValidFilename(dir, hint);

        // hint is valid
        if (result != null) {

            return result;
        } else {
            BtLog.i("invalid filename hint:[" + hint + "]");
        }

        // use current date for filename
        String now = TIME_FILENAME_FORMATER.format(Calendar.getInstance().getTime());
        int idx = hint.lastIndexOf(".");
        if (idx > -1) {

            hint = now + hint.substring(idx);
            result = Utils.isValidFilename(dir, hint);
            if (result != null) {

                return result;
            }
        }
        return storageDirectory + "/" + now;
    }

    private static String isValidFilename(File dir, String name) {

        if (dir == null || name == null) {
            return null;
        }

        File f = new File(dir, name);
        if (!f.exists()) {

            try {
                boolean created = f.createNewFile();
                if (created) {

                    f.delete();
                    if (!f.getParent().equals(dir.getPath())) {

                        return null;
                    } else {
                        return f.getAbsolutePath();
                    }
                } else {
                    return null;
                }
            } catch (IOException ex) {

                return null;
            }
        }
        // if the new file's parent and the given parent is differ then the file
        // name is wrong
        if (f.getParent().equals(dir.getPath())) {

            return f.getAbsolutePath();
        } else {
            return null;
        }
    }

    public static Uri createContextFileForText(Context context, CharSequence subject,
            CharSequence text) {

        if (text == null) {
            return null;
        }
		BtLog.w( "[URL pattern match begin, text: " + text.toString());
		if( subject != null ){

			BtLog.w( "[URL pattern match begin, subject: " + subject.toString());
		}

        FileOutputStream out = null;
        try {
            // delete first
            String filename = context.getString(R.string.bt_opp_push_file_name);
            context.deleteFile(filename);

            // replace subject if it's null
			//subject = (subject == null) ? text : subject;

			// new algorithm for APP link share
			// 1. if text contains content + link, then we need to parse link and set as href, 
			//     left content will be showed as html body. subject feild will not be showed.
			// 2. if text only contains link(Compatible MTK Browser share):
			//     (a) if subject feild is not empty, then show subject as html body, link as href.
			//     (b) if subject feild is empty, then show link as html body, also link as href.
			// 3. if text only contains plain-text, then only fill in text as html body, ignore subject.

			// retrieve URL bt web regular expression	
			Matcher matcher = Patterns.WEB_URL.matcher(text); 
			String urlLink = null;	
			int currentMatchIdx = 0; 
			int currentStartIdx = 0;

			// match url and compose html
			String content = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;"
			                 + " charset=UTF-8\"/></head><body>";
			StringBuffer body = new StringBuffer();

			// fill body and href				  
			while (matcher.find()) {

				urlLink = matcher.group();
				BtLog.w( "URL pattern match result, link: " + urlLink ); 
				BtLog.w( "URL pattern match result, link.length: " + urlLink.length() ); 

				currentMatchIdx = matcher.start();	
				BtLog.w( "URL pattern  current start index - " + currentStartIdx );
				BtLog.w( "URL pattern  current match index - " + currentMatchIdx );

				if (text.length() > urlLink.length()) {
 
					if (currentMatchIdx > currentStartIdx) {
						
						//text not start with uri, add text body
						body.append( text.toString().substring( currentStartIdx, currentMatchIdx ) );
						
						body.append( "<a href=\"" + urlLink + "\">" );
						body.append( urlLink ); 
						body.append( "</a></p>" );		
					} else if (currentMatchIdx == currentStartIdx) {

						//text start with uri
						body.append( "<a href=\"" + urlLink + "\">" );
						body.append( urlLink ); 
						body.append( "</a></p>" );		
						
						// if there is left body(without uri), need to append left part to body
					}
					
					currentStartIdx = currentMatchIdx + urlLink.length();
				} else {
					
					// uri length == text length, no other text as body 
					if (subject == null) {

						body.append( "<a href=\"" + urlLink + "\">" );
						body.append( urlLink );
						body.append( "</a></p>" );	
					} else {

						body.append( "<a href=\"" + urlLink + "\">" );
						body.append( subject );
						body.append( "</a></p>" );	
					}
					
					currentStartIdx = text.length();
					break;
				}
			} 
			
			BtLog.w( "After match currentStartIdx - " + currentStartIdx );
			BtLog.w( "After match current body:" + body.toString());
			
			// append left part(can not handle by match loop) to body
			if ((body.length() != 0) && (currentStartIdx < text.length())) {
				
				body.append( text.toString().substring( currentStartIdx, text.length()) );
			}
			
			// check if no url match in text, fill full text as body, ignore subject feild
			if (body.length() == 0) {
 
				body.append( text );
			}

			// fill end charaters
			content += body.toString();
			content += "</body></html>";

			BtLog.w( "URL final compose content: " + content); 
			byte[] byteBuff = content.getBytes();

			// change the text as hyperlink
			/*
			StringBuilder content = new StringBuilder( 125 + text.length()*2 )
				.append( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/></head><body>" )
				.append( "<a href=\"" ).append( text ).append( "\">" ).append( subject ).append( "</a>" )
				.append( "</body></html>" );
                   */
			// open and write file
			out = context.openFileOutput( filename, Context.MODE_PRIVATE );
			//out.write( content.toString().getBytes() );
			out.write( byteBuff, 0, byteBuff.length);
            out.flush();

            // create Uri
            String filePath = context.getFilesDir().getAbsolutePath();
            if(filePath.startsWith("/data/user/")) {
                //convert "/data/user/userid/com.mediatek.bluetooth/files/bluetooth_share.html" to "/data/data/com.mediatek.bluetooth/files/bluetooth_share.html"
                BtLog.d("createContextFile:: filePath start with /data/user/");
                filePath = "/data/data/" + filePath.substring(filePath.lastIndexOf("com.mediatek.bluetooth"));
                BtLog.d("after convert, new filePath = " + filePath);
            }
            File file = new File(filePath, filename);
            BtLog.d("createContextFile::filePath = " + filePath);

            Uri result = Uri.fromFile(file);

            if (result == null) {

                BtLog.w("createContextFileForText() - can't get Uri for created file.");
                context.deleteFile(filename);
            }

            return result;
        } catch (IOException ex) {

            BtLog.e("createContextFileForText() error:" + ex.toString());
            return null;
        } finally {
            try {
                if (out != null) {
                    out.close();
                }
            } catch (IOException ex) {

                BtLog.w("createContextFileForText() closing file output stream fail: "
                        + ex.toString());
            }
        }
    }

    public static String getGoepResponseCodeString(String responseCode) {

        int code = Integer.parseInt(responseCode);
        code = (code > 0x80) ? (code - 0x80) : code;
        switch (code) {
            case 0:
                return "Success";
            case 1:
                return "Failed";
            case 0x10:
                return "Continue";
            case 0x20:
                return "OK, Success";
            case 0x21:
                return "Created";
            case 0x22:
                return "Accepted";
            case 0x23:
                return "Non-Authoritative Information";
            case 0x24:
                return "No Content";
            case 0x25:
                return "Reset Content";
            case 0x26:
                return "Partial Content";
            case 0x30:
                return "Multiple Choices";
            case 0x31:
                return "Moved Permanently";
            case 0x32:
                return "Moved temporarily";
            case 0x33:
                return "See Other";
            case 0x34:
                return "Not modified";
            case 0x35:
                return "Use Proxy";
            case 0x40:
                return "Bad Request - server couldn't understand request";
            case 0x41:
                return "Unauthorized";
            case 0x42:
                return "Payment required";
            case 0x43:
                return "Forbidden - operation is understood but refused";
            case 0x44:
                return "Not Found";
            case 0x45:
                return "Method not allowed";
            case 0x46:
                return "Not Acceptable";
            case 0x47:
                return "Proxy Authentication required";
            case 0x48:
                return "Request Time Out";
            case 0x49:
                return "Conflict";
            case 0x4A:
                return "Gone";
            case 0x4B:
                return "Length Required";
            case 0x4C:
                return "Precondition failed";
            case 0x4D:
                return "Requested entity too large";
            case 0x4E:
                return "Request URL too large";
            case 0x4F:
                return "Unsupported media type";
            case 0x50:
                return "Internal Server Error";
            case 0x51:
                return "Not Implemented";
            case 0x52:
                return "Bad Gateway";
            case 0x53:
                return "Service Unavailable";
            case 0x54:
                return "Gateway Timeout";
            case 0x55:
                return "HTTP version not supported";
            case 0x60:
                return "Database Full";
            case 0x61:
                return "Database Locked";
            default:
                return "" + responseCode;
        }
    }
}
