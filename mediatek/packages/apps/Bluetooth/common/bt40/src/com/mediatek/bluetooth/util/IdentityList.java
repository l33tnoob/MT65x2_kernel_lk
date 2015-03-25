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

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import com.mediatek.bluetooth.Options;

public class IdentityList<E> {

    /**
     * id manager
     */
    protected IdentityManager identityMgr;

    /**
     * id-element map: <Id, Element>
     */
    protected Map<Integer, E> identityMap;

    /**
     * Constructor
     *
     * @param initialCapacity
     * @param maxCapacity
     */
    public IdentityList( int initialCapacity, int maxCapacity ){

        if( Options.LL_VERBOSE ){

            BtLog.v( "IdentityList()[+]: initialCapacity[" + initialCapacity + "], maxCapacity[" + maxCapacity + "]" );
        }

        this.identityMgr = new IdentityManager( initialCapacity, maxCapacity );
        this.identityMap = new HashMap<Integer, E>( initialCapacity );
    }

    /**
     * add element to list and get the identity of element
     * will throw runtime-exception when fail to acquire id
     *
     * @param element
     * @return element id >=0
     */
    public int registerElement( E element ){

        if( Options.LL_VERBOSE ){

            BtLog.v( "registerElement(): " + element );
        }

        int id = this.identityMgr.acquireId();
        this.identityMap.put( id, element );
        return id;
    }

    /**
     * remove element and release identity (can be used latter)
     *
     * @param id
     * @return
     */
    public boolean unregisterElement( int id ){

        if( Options.LL_VERBOSE ){

            BtLog.v( "unregisterElement(): " + id );
        }

        E element = this.identityMap.remove( id );
        if( element != null ){

            return this.identityMgr.releaseId(id);
        }
        else {
            BtLog.e( "unregisterElement() error: unknown id[" + id + "]" );
            return false;
        }
    }

    public E get( int id ){

        return this.identityMap.get(id);
    }

    public int size(){

        return this.identityMap.size();
    }

    public Collection<E> getElements(){

        return this.identityMap.values();
    }
}