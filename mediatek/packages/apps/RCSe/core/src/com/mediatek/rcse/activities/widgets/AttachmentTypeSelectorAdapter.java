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

import android.content.Context;

import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.List;

/**
 * An adapter used to store attachment type.
 */
public class AttachmentTypeSelectorAdapter extends IconListAdapter {

    public static final int ADD_FILE_FROM_GALLERY = 0;
    public static final int ADD_FILE_FROM_CAMERA = 1;
    public static final int ADD_FILE_FROM_FILE_MANAGER = 2;

    public AttachmentTypeSelectorAdapter(Context context) {
        super(context, getData(context));
    }

    /**
     * Change from the item you clicked to a type of command.
     * 
     * @param whichButton The button that was clicked or the position of the
     *            item clicked.
     * @return The type stands for a special command.
     */
    public int buttonToCommand(int whichButton) {
        AttachmentListItem item = (AttachmentListItem) getItem(whichButton);
        return item.getCommand();
    }

    protected static List<IconListItem> getData(Context context) {
        List<IconListItem> data = new ArrayList<IconListItem>();
        addItem(data, context.getString(R.string.attach_picture),
                R.drawable.ic_attach_picture_holo_light, ADD_FILE_FROM_GALLERY);
        addItem(data, context.getString(R.string.attach_capture_picture),
                R.drawable.ic_attach_capture_picture_holo_light, ADD_FILE_FROM_CAMERA);
        addItem(data, context.getString(R.string.file_type_file),
                R.drawable.ic_menu_move_to_holo_light, ADD_FILE_FROM_FILE_MANAGER);
        return data;
    }

    protected static void addItem(List<IconListItem> data, String title, int resource, int command) {
        AttachmentListItem temp = new AttachmentListItem(title, resource, command);
        data.add(temp);
    }

    /**
     * The item in attachment list.
     */
    public static class AttachmentListItem extends IconListAdapter.IconListItem {

        private int mCommand;

        /**
         * Constructor of AttachmentListItem.
         * 
         * @param title The title of the item.
         * @param resource The image of the item.
         * @param command The command will be executed when you click an item.
         */
        public AttachmentListItem(String title, int resource, int command) {
            super(title, resource);
            mCommand = command;
        }

        /**
         * Get the command of the item.
         * 
         * @return The type stands for a special command.
         */
        public int getCommand() {
            return mCommand;
        }
    }
}
