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

package com.mediatek.bluetooth.map;




import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;
import android.util.Base64;
import android.util.Base64OutputStream;


import java.io.BufferedOutputStream;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.mediatek.bluetooth.map.mime.MimeBase;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeHeaders;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeBody;
import com.mediatek.bluetooth.map.mime.MimeBase.MimeAttachment;

import com.mediatek.xlog.Xlog;

/**
 * Utility class to output RFC 822 messages from provider email messages
 */

/* todo: create a class to encapsulate the provider like pdupersiter
*/
public class Rfc822Output {

	private static final String TAG = "MAP-Rfc822Output";

	private static final int MAXIUM_LINE_LENGTH = 998;

    private static final Pattern PATTERN_START_OF_LINE = Pattern.compile("(?m)^");
    private static final Pattern PATTERN_ENDLINE_CRLF = Pattern.compile("\r\n");

	public static String DATE = "Date";
	public static String SUBJECT = "Subject";
	public static String Message_ID = "Message-ID";
	public static String FROM = "From";
	public static String TO = "To";
	public static String CC = "Cc";
	public static String BCC = "Bcc";
	public static String REPLY_TO = "Reply-To";
	public static String MIME_VERSION = "MIME-Version";
	public static String CONTENT_TYPE = "Content-Type";
	public static String CLRF = "\r\n";
	public static String BOUNDARY = "--";
	
    private static boolean needUpdate = false;
	
	private static final SimpleDateFormat DATE_FORMAT =
		 new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss Z", Locale.US);

   	public static void writeTo(File file, MimeBase mime) throws IOException{
		if (file == null) {
			log("error, the file is null");
		}
   		FileOutputStream stream = new FileOutputStream(file);
        Writer writer = new OutputStreamWriter(stream);

		if (mime == null) {
			log("the mime is null");
			return;
		}

        MimeBase.MimeHeaders header = mime.getHeader();
		MimeBase.MimeBody body = mime.getBody();
		
		String date = DATE_FORMAT.format(new Date(header.mTimeStamp));
        writeHeader(writer, "Date", date);

        writeEncodedHeader(writer, "Subject", header.mSubject);

        writeHeader(writer, "Message-ID", header.mMsgId);

        writeAddressHeader(writer, "From", header.mFrom);
        writeAddressHeader(writer, "To", header.mTo);
        writeAddressHeader(writer, "Cc", header.mCc);
		writeAddressHeader(writer, "Bcc", header.mBcc);
        writeAddressHeader(writer, "Reply-To", header.mReplyTo);
        writeHeader(writer, "MIME-Version", header.mVersion);

        // Analyze message and determine if we have multiparts
        String text = buildBodyText(body);

		if(!mime.hasMultipart()) {
			if (text != null) {
		 		writeTextWithHeaders(writer, stream, text);
			} else {
				writer.write(CLRF); 
			}
		} else {
			   // continue with multipart headers, then into multipart body
                String multipartBoundary = "--_com.android.email_" + System.nanoTime();
				String multipartType = mime.getMultipartType();
              
                writeHeader(writer, "Content-Type",
                        multipartType + "; boundary=\"" + multipartBoundary + "\"");
                // Finish headers and prepare for body section(s)
                writer.write(CLRF);

                // first multipart element is the body
                if (text != null && text.length() > 0) {
					log("text is "+text);
                    writeBoundary(writer, multipartBoundary, false);
                    writeTextWithHeaders(writer, stream, text);
                }

                // Write out the attachments until we run out
                for (MimeBase.MimeAttachment attachment: mime.getAttachment()){
                    writeBoundary(writer, multipartBoundary, false);
                   	writeOneAttachment(writer, stream, attachment);
                    writer.write(CLRF);                   
                } 
                // end of multipart section
                writeBoundary(writer, multipartBoundary, true);
		}

		writer.flush();
        stream.flush();
		stream.close();
	}

	
	private static String buildBodyText(MimeBase.MimeBody body){
	
        String text = body.mTextContent;
		if (body.mIntroText != null) {
			String intro = body.mIntroText == null ? "" : body.mIntroText;
       		text += intro;
		}
      

        String quotedText = body.mTextReply;
        if (quotedText != null) {
            // fix CR-LF line endings to LF-only needed by EditText.
            Matcher matcher = PATTERN_ENDLINE_CRLF.matcher(quotedText);
            quotedText = matcher.replaceAll("\n");
			
            matcher = PATTERN_START_OF_LINE.matcher(quotedText);
            text += matcher.replaceAll(">");
        }
        return text;
        
	}

  
    private static void writeOneAttachment(Writer writer, OutputStream out,
             MimeBase.MimeAttachment attachment) throws IOException{
    	InputStream inStream = attachment.getContent();
    	
    	// changed code
    	
    	
		if (attachment.mName != null){
			writeHeader(writer, "Content-Type",
                attachment.mMimeType + ";\n name=\"" + attachment.mName + "\"");
        
		} else {
			writeHeader(writer, "Content-Type", attachment.mMimeType);
		}

		writeHeader(writer, "Content-Transfer-Encoding", "base64");
		// Most attachments (real files) will send Content-Disposition.  The suppression option
		// is used when sending calendar invites.
		if ((attachment.mFlags & MimeBase.FLAG_ICS_ALTERNATIVE_PART) == 0) {
			if (attachment.mLocation == null) {
				attachment.mLocation = "attachment";
			}
			writeHeader(writer, "Content-Disposition",
                    attachment.mLocation
                    + ";\n filename=\"" + attachment.mFileName + "\";"
                    + "\n size=" + Long.toString(attachment.mSize));
		}
		if (attachment.mContentLocation != null) {
			writeHeader(writer, "Content-Location", attachment.mContentLocation);
		}
		if (attachment.mContentId != null) {
			writeHeader(writer, "Content-ID", attachment.mContentId);
		}
		writer.append("\r\n");

        // Set up input stream and write it out via base64
        try {
            // switch to output stream for base64 text output
            writer.flush();

			Base64OutputStream base64Out = new Base64OutputStream(
                out, Base64.CRLF | Base64.NO_CLOSE);
          
            // attachment is not valid when inStream is null
            if(inStream == null)
            {
            	throw new IOException();
            }            

			IOCopy(inStream, base64Out);
			base64Out.close();

            // The old Base64OutputStream wrote an extra CRLF after
            // the output.  It's not required by the base-64 spec; not
            // sure if it's required by RFC 822 or not.
      //      out.write('\r');
      //      out.write('\n');
            out.flush();
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }


    /**
     * Write a single header with no wrapping or encoding
     *
     * @param writer the output writer
     * @param name the header name
     * @param value the header value
     */
    private static void writeHeader(Writer writer, String name, String value) throws IOException {
        if (value != null && value.length() > 0) {
            writer.append(name);
            writer.append(": ");
            writer.append(value);
            writer.append("\r\n");
        }
    }

    /**
     * Write a single header using appropriate folding & encoding
     *
     * @param writer the output writer
     * @param name the header name
     * @param value the header value
     */
    private static void writeEncodedHeader(Writer writer, String name, String value)
            throws IOException {
        if (value != null && value.length() > 0) {
            writer.append(name);
            writer.append(": ");
			//no encoding
			writer.append(value);
        //    writer.append(MimeUtility.foldAndEncode2(value, name.length() + 2));
            writer.append("\r\n");
        }
    }

    /**
     * Unpack, encode, and fold address(es) into a header
     *
     * @param writer the output writer
     * @param name the header name
     * @param value the header value (a packed list of addresses)
     */
    private static void writeAddressHeader(Writer writer, String name, String value)
            throws IOException {
        if (value != null && value.length() > 0) {
            writer.append(name);
            writer.append(": ");
            writer.append(Address.packedToHeader(value));
            writer.append("\r\n");
        }
    }

    /**
     * Write a multipart boundary
     *
     * @param writer the output writer
     * @param boundary the boundary string
     * @param end false if inner boundary, true if final boundary
     */
    private static void writeBoundary(Writer writer, String boundary, boolean end)
            throws IOException {
        writer.append("--");
        writer.append(boundary);
        if (end) {
            writer.append("--");
        }
        writer.append("\r\n");
    }

     private static void writeTextWithHeaders(Writer writer, OutputStream out, String text)
            throws IOException {
        int length;
        writeHeader(writer, "Content-Type", "text/plain; charset=utf-8");

		//map require 8 bit encoding
        writeHeader(writer, "Content-Transfer-Encoding", "8bit");
        writer.write("\r\n");
        byte[] bytes = text.getBytes("UTF-8");
        writer.flush();        

		length = (bytes == null)? 0 : bytes.length;
		
		if (bytes != null && bytes.length != 0){
			for (int index = 0; (bytes != null && index < bytes.length); index ++){
				out.write(bytes[index]);
				if (index % MAXIUM_LINE_LENGTH == 0){
                                    writer.write("\r\n");
                                }       
		         }		
		} else {
        writer.write("\r\n");
    }
		
    }
	//TODO: inputstream and outputstream
	private static void IOCopy (InputStream in, OutputStream out) {
		byte[] cache = new byte[1];
		BufferedInputStream bufferin = new BufferedInputStream(in);
		BufferedOutputStream bufferout = new BufferedOutputStream(out);
		try {
			while(bufferin.read(cache) != -1) {
				bufferout.write(cache);
			}
			bufferout.flush();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}
	private static void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}
