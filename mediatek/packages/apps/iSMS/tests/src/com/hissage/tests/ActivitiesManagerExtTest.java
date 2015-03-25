package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.ActivitiesManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

/*
 * ActivitiesManagerExt test.
 */
public class ActivitiesManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private ActivitiesManagerExt mActivitiesManagerExt;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mActivitiesManagerExt = new ActivitiesManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

//    public void test01ChatSetting() throws Exception {
//        Intent intent = new Intent(RemoteActivities.CHAT_SETTINGS);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test02SystemSetting() throws Exception {
//        Intent intent = new Intent(RemoteActivities.SYSTEM_SETTINGS);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }

    public void test03Activitation() throws Exception {
        Intent intent = new Intent(RemoteActivities.ACTIVITION);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }

//    public void test04Location() throws Exception {
//        Intent intent = new Intent(RemoteActivities.LOCATION);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test05allMedia() throws Exception {
//        Intent intent = new Intent(RemoteActivities.ALL_MEDIA);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test06allLocation() throws Exception {
//        Intent intent = new Intent(RemoteActivities.ALL_LOCATION);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }

    public void test07chatDetails() throws Exception {
        Intent intent = new Intent(RemoteActivities.CHAT_DETAILS_BY_THREAD_ID);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }

//    public void test08Contact() throws Exception {
//        Intent intent = new Intent(RemoteActivities.CONTACT);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test09NonIpContact() throws Exception {
//        Intent intent = new Intent(RemoteActivities.NON_IPMESSAGE_CONTACT);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }

    public void test10NewGroupChat() throws Exception {
        Intent intent = new Intent(RemoteActivities.NEW_GROUP_CHAT);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }

//    public void test11quickContact() throws Exception {
//        Intent intent = new Intent(RemoteActivities.QUICK_CONTACT);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test12mediateDetail() throws Exception {
//        Intent intent = new Intent(RemoteActivities.MEDIA_DETAIL);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test13Sketch() throws Exception {
//        Intent intent = new Intent(RemoteActivities.SKETCH);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }
//
//    public void test14Audio() throws Exception {
//        Intent intent = new Intent(RemoteActivities.AUDIO);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }

    public void test15ServiceCenter() throws Exception {
        Intent intent = new Intent(RemoteActivities.SERVICE_CENTER);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }

//    public void test16Profile() throws Exception {
//        Intent intent = new Intent(RemoteActivities.PROFILE);
//        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
//    }

    public void test17Term() throws Exception {
        Intent intent = new Intent(RemoteActivities.TERM);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }

    public void test18Unknown() throws Exception {
        Intent intent = new Intent("content://term/100");
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, 9);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mActivitiesManagerExt.startRemoteActivity(mContext,intent);
    }
}
