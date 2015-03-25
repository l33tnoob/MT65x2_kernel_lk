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

import android.content.Context;
import android.util.AttributeSet;
import android.widget.TextView;

import java.text.NumberFormat;

/* Customized TextView for storing additional informations */
public class EntryInfoView extends TextView {
	private static final String TAG = "EntryInfoView";

	private static final String UNKNOWN = "unknown";

	private long mEntrySize;
	private String mEntryDate;

	public void setEntryInfo(long size, String date) {
		mEntrySize = size;
		mEntryDate = date;

		String info_date = getContext().getString(R.string.bluetooth_ftp_date) + ": ";
		String info_size = getContext().getString(R.string.bluetooth_ftp_size) + ": ";

		String info = null;
		if (size < 0) {
			if (!BluetoothFtpProviderHelper.UNKNOWN_DATE.equals(date)) {
				info = info_date + date;
			}
		} else {
			info = info_size + getFormattedSize(size);
			if (!BluetoothFtpProviderHelper.UNKNOWN_DATE.equals(date)) {
				info += ", " + info_date + date;
			}
		}
		setText(info);
	}

	public long getEntrySize() {
		return mEntrySize;
	}

	public String getEntryDate() {
		return mEntryDate;
	}

	public EntryInfoView(Context context) {
		super(context);
	}

	public EntryInfoView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public EntryInfoView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	/* Utility function: Simply format file size. */
	private String getFormattedSize(long size) {
		double value1, value2;
		String[] label = {"B", "KB", "MB", "GB"};
		int count = 0;

		if (size < 0) {
			return UNKNOWN;
		}

		value1 = (new Long(size)).doubleValue();
		value2 = value1 / 1024;

		while (value2 > 1.0d) {
			value1 = value2;
			value2 = value2 / 1024;

			if (count < label.length - 1) {
				count++;
			} else {
				break;
			}
		}

		NumberFormat nf = NumberFormat.getNumberInstance();
		nf.setMinimumFractionDigits(2);
		nf.setMaximumFractionDigits(2);

		return nf.format(value1) + label[count];
	}
}
