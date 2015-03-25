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

import com.mediatek.bluetooth.R;

import android.net.Uri;
import android.provider.BaseColumns;

import java.util.HashSet;

public class BluetoothFtpProviderHelper {

	private static final String TAG = "BluetoothFtpProviderHelper";

	public static final String AUTHORITY = "com.mediatek.provider.bluetooth.ftp";

	/* Default string when modified date is worngly formatted or not specified */
	public static final String UNKNOWN_DATE = "unknown";

	/* Default value when size is failed to parse or not specified */
	public static final int UNKNOWN_SIZE = -1;

	private BluetoothFtpProviderHelper() {}

	/* Columns defincation of a folder content entry (a file or folder) */
	public static final class FolderContent implements BaseColumns {
		private FolderContent() {}

		/* Uri for listing current folder content of FTP server */
		public static final Uri SERVER_URI = Uri.parse("content://" + AUTHORITY + "/server_cur_folder");

		/* Uri for querying markable files in current folder */
		public static final Uri SERVER_MARKS_URI = Uri.parse("content://" + AUTHORITY + "/server_cur_folder/marks");

		/* Uri for listing current folder content of FTP server */
		public static final Uri LOCAL_URI = Uri.parse("content://" + AUTHORITY + "/local_cur_folder");

		/* Uri for querying markable files in current folder */
		public static final Uri LOCAL_MARKS_URI = Uri.parse("content://" + AUTHORITY + "/local_cur_folder/marks");

		public static final String CONTENT_TYPE = "vnd.android.cursor.dir/vnd.mtkbt.ftp.folder_content";

		public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/vnd.mtkbt.ftp.folder_content";

		public static final String DEFAULT_SORT_ORDER = "modified DESC";

		public static final String NAME = "name";

		/* Type: Image, Audio, Video, Text, or Folder */
		public static final String TYPE = "type";

		/* Constants for TYPE in FolderContent */
		public static final int TYPE_FOLDER = 10,
								TYPE_TEXT   = 11,
								TYPE_IMAGE  = 12,
								TYPE_AUDIO  = 13,
								TYPE_VIDEO  = 14;

		/* Size of the item */
		public static final String SIZE = "size";

		/* Last modification time */
		public static final String MODIFIED_DATE = "modified"; 
	}

	private static HashSet<String> sHSAudio;
	private static HashSet<String> sHSVideo;
	private static HashSet<String> sHSImage;

	static {
		sHSAudio = new HashSet<String>();
		sHSAudio.add("aac");
		sHSAudio.add("amr");
		sHSAudio.add("mid");
		sHSAudio.add("mp3");
		sHSAudio.add("ogg");
		sHSAudio.add("wav");
		sHSAudio.add("wma");

		sHSImage = new HashSet<String>();
		sHSImage.add("bmp");
		sHSImage.add("gif");
		sHSImage.add("jpeg");
		sHSImage.add("jpg");
		sHSImage.add("png");

		sHSVideo = new HashSet<String>();
		sHSVideo.add("3gp");
		sHSVideo.add("avi");
		sHSAudio.add("flv");
		sHSVideo.add("mp4");
		sHSVideo.add("wmv");
	}

	/* Columns defincation of an entry of the transferring list (queue) */
	public static final class TransferringFile implements BaseColumns {
		private TransferringFile() {}

		public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/transferring");

		public static final String CONTENT_TYPE = "vnd.android.cursor.dir/vnd.mtkbt.ftp.transferring";

		public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/vnd.mtkbt.ftp.transferring";

		public static final String DEFAULT_SORT_ORDER = _ID + " ASC";

		public static final String NAME = "name";

		/* Status: Transferring, Waiting, Successful, or Failed */
		public static final String STATUS = "status";

		/* Constants for STATUS in TransferringFile */
		public static final int STATUS_WAITING	    = 20,
								STATUS_TRANSFERRING = 21,
								STATUS_SUCCESSFUL   = 22,
								STATUS_FAILED	    = 23;

		/* Progressing percentage of the item */
		public static final String PROGRESS = "progress";

		/* The file size */
		public static final String TOTAL = "total";

		/* Direction: Push or Pull from server */
		public static final String DIRECTION = "direction";

		/* Constants for DIRECTION in TransferringFile */
		public static final int DIRECTION_PULL	    = 30,
								DIRECTION_PUSH	    = 31;

    }


/********************************************************************************************
 * Utility Functions for BluetoothFtpProvider Users
 ********************************************************************************************/

	/* Utility function: Returns corrsponding type of file extendsion name */
	public static int getTypeCode(String name) {
		int ret = FolderContent.TYPE_TEXT;
		int lastDot = name.lastIndexOf(".");

		if (lastDot != -1) {
			String ext = name.substring(lastDot + 1, name.length()).toLowerCase();

			if (sHSAudio.contains(ext)) {
				ret = FolderContent.TYPE_AUDIO;
			} else if (sHSImage.contains(ext)) {
				ret = FolderContent.TYPE_IMAGE;
			} else if (sHSVideo.contains(ext)) {
				ret = FolderContent.TYPE_VIDEO;
			} else {
				ret = FolderContent.TYPE_TEXT;
			}

		}
		return ret;
	}

}
