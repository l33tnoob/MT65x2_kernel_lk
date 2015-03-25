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

package com.mediatek.bluetooth.ftp;

import com.mediatek.bluetooth.ftp.BluetoothFtpProviderHelper.FolderContent;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

/* Parsing and extracting data from folder-listing-object.
 * Then push the data into content provider.
 */
public class XMLParsingThread extends Thread {
	private static final String TAG = "XMLParsingThread";

	public interface ParsingDoneListener {
		public void onParsingDone(int result);
	}

	private String mXMLFilePath;

	private Uri mUri;

	private ContentResolver mContentResolver;

	private ParsingDoneListener mListener;

	public XMLParsingThread(String path, Uri uri, ContentResolver resolver, ParsingDoneListener listener) {
		super();

		mXMLFilePath = path;
		mUri = uri;
		mContentResolver = resolver;
		mListener = listener;
	}

	public synchronized boolean isDone(ParsingDoneListener l) {
		if (this.getState() == Thread.State.TERMINATED) {
			return true;
		} else {
			mListener = l;
			return false;
		}
	}

	public synchronized void removeListener() {
		mListener = null;
	}

	private synchronized void postResult(int res) {
		// Result codes: success(0), fail(1), error(-1)
		if (mListener != null) {
			mListener.onParsingDone(res);
		}
	}

	private void printErr(String errMsg) {
		Log.e(TAG, "[BT][FTP] " + errMsg);
	}

	public void run() {
		int res = 0;

		try {
			SAXParserFactory spf = SAXParserFactory.newInstance();
			SAXParser sp = spf.newSAXParser();
			XMLReader xr = sp.getXMLReader();

			// Clear old data.
			mContentResolver.delete(mUri, null, null);

			// Here we got an XML parser with reader
			// 1.) Set XML content handler.
			xr.setContentHandler(new FtpObexXMLHandler(mUri, mContentResolver));

			// 2.) Feed input source to start parsing.
			FileInputStream fis = new FileInputStream(new File(mXMLFilePath));
			xr.parse(new InputSource(fis));

		} catch (SAXException se) {
			printErr("Parsing failed: " + se);
			String msg = se.getMessage();
			if (msg != null) {
				if (msg.equals(FtpObexXMLHandler.INVALID_NAME)) {
					res = 1;
				} else {
					// ignore.
				}
			}
		} catch (IOException ie) {
			printErr("Open file failed: " + ie);
			res = 1;
		} catch (ParserConfigurationException pce) {
			printErr("Parser config failed: " + pce);
			res = 1;
		} catch (Exception ex) {
			printErr("Exception: " + ex);
			res = -1;
		}

		if (res == 1) {
			// Clear data when errors occurred.
			mContentResolver.delete(mUri, null, null);
		}

		// Notify the listener that the parsing has done
		postResult(res);
	}
}

class FtpObexXMLHandler extends DefaultHandler {
	private static final String TAG = "FtpObexXMLHandler";

	private static final String MIME = "x-obex/folder-listing";

	private static final String UNKNOWN_DATE = BluetoothFtpProviderHelper.UNKNOWN_DATE;

	private static final int UNKNOWN_SIZE = BluetoothFtpProviderHelper.UNKNOWN_SIZE;

	/* Extended regular expression for Date Format in OBEX folder listing object */
	private static final String DATE_ERE = "(19|20)[0-9]{2}"
										 + "(0[1-9]|1[012])"
										 + "(0[1-9]|[12][0-9]|3[01])T"
										 + "([01][0-9]|2[0-3])"
										 + "([0-5][0-9]){2}Z";

	/* Element tags */
	private static final String ELEM_FILE			= "file",
								ELEM_FOLDER			= "folder",
								ELEM_FOLDER_LISTING	= "folder-listing",
								ELEM_PARENT_FOLDER	= "parent-folder";

	/* Attributes */
	private static final String ATTR_FS_NAME		= "name",
								ATTR_FS_SIZE		= "size",
								ATTR_FS_MODIFIED	= "modified",
								ATTR_FS_CREATED		= "created",
								ATTR_FS_ACCESSED	= "accessed",
								ATTR_FS_USER_PERM	= "user-perm",
								ATTR_FS_GROUP_PERM	= "group-perm",
								ATTR_FS_OTHER_PERM	= "other-perm",
								ATTR_FS_OWNER		= "owner",
								ATTR_FS_GROUP		= "group",
								ATTR_FS_TYPE		= "type",
								ATTR_FS_XML_LANG	= "xml:lang",
								ATTR_VERSION		= "version";

	/* Message used when file/folder name is invalid */
	protected static final String INVALID_NAME = "invalid_name";

	private Uri mUri;
	private ContentResolver mContentResolver;

	private int mParsingDepth;
	private boolean bErrorDetect;

	public FtpObexXMLHandler(Uri uri, ContentResolver resolver) {
		super();

		mUri = uri;
		mContentResolver = resolver;

		mParsingDepth = 0;
		bErrorDetect = false;
	} 

	@Override
	public void startDocument() {
		// Clear the old data
		// mContentResolver.delete(mUri, null, null);
	}

	@Override
	public void startElement(String nsUri, String localName, String qName, Attributes attrs)
			throws SAXException {
		// Log.v(TAG, "Start element: " + localName);

		if (bErrorDetect) {
			return;
		}

		int len = attrs.getLength();
		ContentValues values = new ContentValues();

		if (localName.equals(ELEM_FOLDER_LISTING)) {
			if (mParsingDepth == 0) {
				mParsingDepth++;

			} else {
				// Error case, just lock the parser.
				bErrorDetect = true;
				return;
			}

		} else if (localName.equals(ELEM_FILE)) {
			if (mParsingDepth == 1) {
				values.clear();

				for (int i = 0; i < len; i++) {
					String attr_name = attrs.getLocalName(i);
					String attr_value = attrs.getValue(i);

					if (attr_name.equals(ATTR_FS_NAME)) {
						values.put(FolderContent.NAME, attr_value);
						values.put(FolderContent.TYPE, BluetoothFtpProviderHelper.getTypeCode(attr_value));

					} else if (attr_name.equals(ATTR_FS_SIZE)) {
						try {
							values.put(FolderContent.SIZE, Long.parseLong(attr_value));
						} catch (Exception e) {
							Log.w(TAG, "File size parsing failed. Set to 0");
							values.put(FolderContent.SIZE, 0);
						}

					} else if (attr_name.equals(ATTR_FS_CREATED)) {
						if (!values.containsKey(FolderContent.MODIFIED_DATE)) {
							// If there's no modified date, use created date instead.
							values.put(FolderContent.MODIFIED_DATE, getFormattedDate(attr_value));
						}

					} else if (attr_name.equals(ATTR_FS_MODIFIED)) {
						values.put(FolderContent.MODIFIED_DATE, getFormattedDate(attr_value));

					} else {
						// skip.
					}
				}

				if (values.containsKey(FolderContent.NAME) &&
					values.containsKey(FolderContent.TYPE)) {

					if (!values.containsKey(FolderContent.MODIFIED_DATE)) {
						values.put(FolderContent.MODIFIED_DATE, UNKNOWN_DATE);
					}

					if (!values.containsKey(FolderContent.SIZE)) {
						values.put(FolderContent.SIZE, UNKNOWN_SIZE);
					}

					// Insert data to content provider */
					mContentResolver.insert(mUri, values);

				} else {
					Log.d(TAG, "Defected file data, do not insert.");
				}

			} else {
				bErrorDetect = true;
				return;
			}

		} else if (localName.equals(ELEM_FOLDER)) {
			if (mParsingDepth == 1) {

				values.put(FolderContent.TYPE, FolderContent.TYPE_FOLDER);

				for (int i = 0; i < len; i++) {
					String attr_name = attrs.getLocalName(i);
					String attr_value = attrs.getValue(i);

					if (attr_name.equals(ATTR_FS_NAME)) {
						values.put(FolderContent.NAME, attr_value);

					} else if (attr_name.equals(ATTR_FS_CREATED)) {
						if (!values.containsKey(FolderContent.MODIFIED_DATE)) {
							// If there's no modified date, use created date instead.
							values.put(FolderContent.MODIFIED_DATE, getFormattedDate(attr_value));
						}

					} else if (attr_name.equals(ATTR_FS_MODIFIED)) {
						values.put(FolderContent.MODIFIED_DATE, getFormattedDate(attr_value));

					} else {
						// skip.
					}
				}

				if (values.containsKey(FolderContent.NAME) && values.containsKey(FolderContent.TYPE)) {

					if (!values.containsKey(FolderContent.MODIFIED_DATE)) {
						values.put(FolderContent.MODIFIED_DATE, UNKNOWN_DATE);
					}

					// Insert data to content provider
					mContentResolver.insert(mUri, values);

				} else {
					Log.d(TAG, "Defected folder data, do not insert.");
				}

			} else {
				bErrorDetect = true;
				return;
			}

		} else {
			// skip.
		}

	}

	@Override
	public void characters(char[] ch, int start, int length) {
		// Log.v(TAG, "chars: " + new String(ch) + "(" + start + ", " + length + ")");
	}

	@Override
	public void endElement(String nsUri, String localName, String qName) {
		// Log.v(TAG, "End element: " + localName);

		if (localName.equals(ELEM_FOLDER_LISTING)) {
			mParsingDepth--;
		}
	}

	@Override
	public void endDocument() {
		if (bErrorDetect || mParsingDepth != 0) {
			Log.e(TAG, "Parsing failed, clear stored data.");
			mContentResolver.delete(mUri, null, null);
		}
	}

	/* Utility function: Returns formatted date if it's valid, or just return default date */
	private String getFormattedDate(String date) {
		String ret = UNKNOWN_DATE;

		try {
			if (date.matches(DATE_ERE)) {
				int idx = date.indexOf("T");
				ret = date.substring(idx - 8, idx - 4) + "/"
					+ date.substring(idx - 4, idx - 2) + "/"
					+ date.substring(idx - 2, idx);

			} else {
				Log.d(TAG, "Modification date in wrong format, " + date);
			}

		} catch (Exception e) {
			Log.w(TAG, "Modification date parsing failed. Set as default");
		}		

		return ret;
	}

}
