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

package com.mediatek.bluetooth.util;


public class IdentityManager {

    private int maxCapacity;
    private int curCapacity;
    private int curUsed;
    private boolean[] idTable;

    public IdentityManager( int initCapacity, int maxCapacity ){

        this.maxCapacity = maxCapacity;
        this.curCapacity = initCapacity;
        this.curUsed = 0;
        this.idTable = new boolean[this.curCapacity];
    }

    public synchronized int acquireId(){

        // extend array if necessary
        if( this.curUsed >= this.curCapacity ){

            if( this.curUsed > this.maxCapacity ){

                throw new IllegalStateException( "max connection id[" + this.maxCapacity + "] is reach. no more connection can be created." );
            }

            this.curCapacity = this.curCapacity + (this.curCapacity/4) + 1;
            boolean[] newArray = new boolean[this.curCapacity];
            System.arraycopy( this.idTable, 0, newArray, 0, this.idTable.length );
            this.idTable = newArray;
        }

        // find the next available connId
        for( int i=curUsed; i<this.curCapacity; i++ ){

            if( !this.idTable[i] ){

                this.idTable[i] = true;
                this.curUsed++;
                return i;
            }
        }
        for( int i=curUsed-1; i>=0; i-- ){

            if( !this.idTable[i] ){

                this.idTable[i] = true;
                this.curUsed++;
                return i;
            }
        }

        // error condition
        BtLog.e( "IdentityManager.acquireId() error: curUsed[" + curUsed + "], curCapacity[" + curCapacity + "]" );
        throw new IllegalStateException( "Can't find available id. This should be a bug in IdentityManager" );
    }

    public synchronized boolean releaseId( int id ){

        if( id >= this.idTable.length || !this.idTable[id] ){

            BtLog.e( "IdentityManager.releaseId() error: release unused id[" + id + "]" );
            return false;
        }

        this.idTable[id] = false;
        this.curUsed--;
        return true;
    }
}