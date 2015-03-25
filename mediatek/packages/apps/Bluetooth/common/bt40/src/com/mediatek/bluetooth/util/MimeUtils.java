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

package com.mediatek.bluetooth.util;

import android.media.MediaFile;
import android.webkit.MimeTypeMap;

public class MimeUtils {

    public static final String VCARD_TYPE    = "text/x-vcard",
                               VCARD_EXT    = ".vcf";

    /**
     * Returns mime type of input file path
     *
     * @param filename
     * @return mime-type string or null
     */
    public static String getMimeType( String filename ){

        if( filename == null )    return null;

        // For ALPS00287958, hanlde .ape etc file mimeType
        String mimeType = MediaFile.getMimeTypeForFile( filename );
        if( mimeType == null ){

            String fileExtension = null;
            int idx = filename.lastIndexOf('.');
            if( idx > 0 ){

                fileExtension = filename.substring( idx + 1 ).toLowerCase();
            }
            mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension( fileExtension );
        }
        return mimeType;
    }

    /**
     * Returns applied file name with ".vcf" extension
     *
     * @param filename
     * @param limit: Maximum size of applied filename
     * @return applied file name or null. A file name with ".vcf" extension will be return directly.
     */
    public static String applyVcardExt(String filename, int limit) {
        String ret;

        if ( filename == null ) return null;
        int idx = filename.lastIndexOf('.');
        if ( idx > 0 ){
            String ext = filename.substring( idx ).toLowerCase();
            if ( VCARD_EXT.equals(ext) ) return filename;
        }

        if ( filename.length() < (limit - 4) ){
            ret = filename + VCARD_EXT;
        }
        else {
            ret = filename.substring( 0, limit - 5 ) + VCARD_EXT;
        }
        return ret;
    }
}
