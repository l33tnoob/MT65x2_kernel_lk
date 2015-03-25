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

package com.mediatek.apst.target.tests;

import android.test.AndroidTestCase;

import com.mediatek.apst.util.entity.contacts.Organization;

import java.nio.ByteBuffer;

public class OrganizationTest extends AndroidTestCase {
    private Organization mOrganization;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mOrganization = new Organization();

    }

    @Override
    protected void tearDown() throws Exception {

        mOrganization = null;
        super.tearDown();
    }

    public void test01_getCompany() {
        mOrganization.setCompany("company");
        assertEquals("company", mOrganization.getCompany());
    }

    public void test02_getType() {
        mOrganization.setType(Organization.TYPE_WORK);
        assertEquals(Organization.TYPE_WORK, mOrganization.getType());
    }

    public void test03_getLabel() {
        mOrganization.setLabel("label");
        assertEquals("label", mOrganization.getLabel());
    }

    public void test04_getTitle() {
        mOrganization.setTitle("title");
        assertEquals("title", mOrganization.getTitle());
    }

    public void test05_getDepartment() {
        mOrganization.setDepartment("department");
        assertEquals("department", mOrganization.getDepartment());

    }

    public void test06_getJobDescription() {
        mOrganization.setJobDescription("jobDescription");
        assertEquals("jobDescription", mOrganization.getJobDescription());
    }

    public void test07_getSymbol() {
        mOrganization.setSymbol("symbol");
        assertEquals("symbol", mOrganization.getSymbol());
    }

    public void test08_getPhoneticName() {
        mOrganization.setPhoneticName("phoneticName");
        assertEquals("phoneticName", mOrganization.getPhoneticName());
    }

    public void test08_getOfficeLocation() {
        mOrganization.setOfficeLocation("officeLocation");
        assertEquals("officeLocation", mOrganization.getOfficeLocation());
    }

    public void test09_getPhoneticNameStyle() {
        mOrganization.setPhoneticNameStyle("phoneticNameStyle");
        assertEquals("phoneticNameStyle", mOrganization.getPhoneticNameStyle());
    }

    public void test10_getMimeTypeString() {
        assertEquals(Organization.MIME_TYPE_STRING, mOrganization
                .getMimeTypeString());
    }

    public void test11_clone() {
        try {
            mOrganization.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test12_readRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(200);
        Organization organization = new Organization();
        organization.setCompany("google");
        organization.setDepartment("AP");
        organization.setJobDescription("software development");
        organization.setLabel("label");
        organization.setOfficeLocation("20F");
        organization.setPhoneticName("guge");
        organization.setPhoneticNameStyle("style");
        organization.setSymbol("symbol");
        organization.setTitle("org");
        organization.setType(Organization.TYPE_WORK);
        organization.writeRaw(buffer);
        buffer.position(0);
        mOrganization.readRaw(buffer);

        assertEquals("google", mOrganization.getCompany());
        assertEquals("AP", mOrganization.getDepartment());
        assertEquals("software development", mOrganization.getJobDescription());
        assertEquals("label", mOrganization.getLabel());
        assertEquals("20F", mOrganization.getOfficeLocation());
        assertEquals("guge", mOrganization.getPhoneticName());
        assertEquals("symbol", mOrganization.getSymbol());
        assertEquals("org", mOrganization.getTitle());
        assertEquals(Organization.TYPE_WORK, mOrganization.getType());
    }
}
