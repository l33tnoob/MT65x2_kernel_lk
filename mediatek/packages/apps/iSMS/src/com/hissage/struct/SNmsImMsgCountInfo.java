package com.hissage.struct;

import com.hissage.message.ip.NmsIpMessageConsts;

public class SNmsImMsgCountInfo {
    public int allMsgCount;
    public int unreadMsgCount;
    public int notDeliveredCount;
    public int failedCount;
    public int readModeMsgCount[];

    public static int NMS_MESSAGE_TYPE_EXT_MMS = 12; // info:
                                                     // NMS_IM_READ_MODE_EXT_MMS

    public int getAllReadModeMsgCount(int readModeMsgCount) {
        int notDownCount = readModeMsgCount >> 16;
        int downCount = readModeMsgCount & 0xFFFF;
        return notDownCount + downCount;
    }

    public int getDownReadModeMsgCount(int readModeMsgCount) {
        return readModeMsgCount & 0xFFFF;
    }

    public int getNotDownReadModeMsgCount(int readModeMsgCount) {
        return readModeMsgCount >> 16;
    }

    public boolean showAllMedia() {
        if (getDownReadModeMsgCount(readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.PICTURE]) != 0
                || getDownReadModeMsgCount(readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.SKETCH]) != 0
                || getDownReadModeMsgCount(readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.VOICE]) != 0
                || getDownReadModeMsgCount(readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.VIDEO]) != 0
                || getDownReadModeMsgCount(readModeMsgCount[NMS_MESSAGE_TYPE_EXT_MMS]) != 0) {
            return true;
        }
        return false;
    }

    public boolean showAllLocation() {
        if (getAllReadModeMsgCount(readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.LOCATION]) != 0) {
            return true;
        } else {
            return false;
        }
    }

    public int getGroupCfgCount() {
        return readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG]
                + readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG]
                + readModeMsgCount[NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG];
    }
}
