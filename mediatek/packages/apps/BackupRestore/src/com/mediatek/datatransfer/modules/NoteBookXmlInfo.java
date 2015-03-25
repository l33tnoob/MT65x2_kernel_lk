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

package com.mediatek.datatransfer.modules;

/**
 * Describe class <code>NoteBookXmlInfo</code> here.
 * 
 * @author
 * @version 1.0
 */
public class NoteBookXmlInfo {
    public static final String ROOT = "NoteBook";
    public static final String RECORD = "record";
    public static final String TITLE = "title";
    public static final String NOTE = "note";
    public static final String CREATED = "created";
    public static final String MODIFIED = "modified";
    public static final String NOTEGROUP = "notegroup";

    private String mTitle;
    private String mNote;
    private String mCreated;
    private String mModified;
    private String mNoteGroup;

    public NoteBookXmlInfo() {
    }

    public NoteBookXmlInfo(String argTitle, String argNote, String argCreated, String argModified,
            String argNoteGroup) {
        setTitle(argTitle);
        setNote(argNote);
        setCreated(argCreated);
        setModified(argModified);
        setNoteGroup(argNoteGroup);
    }

    /**
     * Gets the value of mTitle
     * 
     * @return the value of mTitle
     */
    public final String getTitle() {
        return this.mTitle;
    }

    /**
     * Sets the value of mTitle
     * 
     * @param argMTitle
     *            Value to assign to this.mTitle
     */
    public final void setTitle(final String argMTitle) {
        this.mTitle = argMTitle;
    }

    /**
     * Gets the value of mNote
     * 
     * @return the value of mNote
     */
    public final String getNote() {
        return this.mNote;
    }

    /**
     * Sets the value of mNote
     * 
     * @param argMNote
     *            Value to assign to this.mNote
     */
    public final void setNote(final String argMNote) {
        this.mNote = argMNote;
    }

    /**
     * Gets the value of mCreated
     * 
     * @return the value of mCreated
     */
    public final String getCreated() {
        return this.mCreated;
    }

    /**
     * Sets the value of mCreated
     * 
     * @param argMCreated
     *            Value to assign to this.mCreated
     */
    public final void setCreated(final String argMCreated) {
        this.mCreated = argMCreated;
    }

    /**
     * Gets the value of mModified
     * 
     * @return the value of mModified
     */
    public final String getModified() {
        return this.mModified;
    }

    /**
     * Sets the value of mModified
     * 
     * @param argMModified
     *            Value to assign to this.mModified
     */
    public final void setModified(final String argMModified) {
        this.mModified = argMModified;
    }

    /**
     * Gets the value of mNoteGroup
     * 
     * @return the value of mNoteGroup
     */
    public final String getNoteGroup() {
        return this.mNoteGroup;
    }

    /**
     * Sets the value of mNoteGroup
     * 
     * @param argMNoteGroup
     *            Value to assign to this.mNoteGroup
     */
    public final void setNoteGroup(final String argMNoteGroup) {
        this.mNoteGroup = argMNoteGroup;
    }

}
