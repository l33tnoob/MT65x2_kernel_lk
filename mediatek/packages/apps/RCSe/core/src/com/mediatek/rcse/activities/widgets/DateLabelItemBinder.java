/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.activities.widgets;

import android.view.View;

import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;

import java.util.Date;

/**
 * This is an item binder for date label
 */
public class DateLabelItemBinder extends AbsItemBinder {
    public static final String TAG = "DateLabelItemBinder";
    private Date mDate = null;
    private int mPosition = -1;

    public DateLabelItemBinder(Date date, int position) {
        mDate = (Date) date.clone();
        mPosition = position;
    }

    @Override
    public void bindView(View itemView) {
        if (Logger.getIsIntegrationMode()) {
            DateView date = (DateView) itemView.findViewById(R.id.tv_time_divider);
            date.setDate(mDate, mPosition == 0);
        } else if (itemView instanceof DateView) {
            ((DateView) itemView).setDate(mDate, mPosition == 0);
        }
    }

    @Override
    public int getLayoutResId() {
        if (Logger.getIsIntegrationMode()) {
            return R.layout.plugin_chat_item_date_lable;
        } else {
            return R.layout.chat_item_date_label;
        }
    }

    @Override
    public int getItemType() {
        return ChatAdapter.ITEM_TYPE_DATE_LABEL;
    }

    public String getDate() {
        return DateView.convertDate(mDate, AndroidFactory.getApplicationContext());
    }
}
