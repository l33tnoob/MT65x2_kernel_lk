package com.hissage.message.ip;

public class NmsIpSessionMessage {
    public NmsIpMessage ipMsg;
    public int count; // total of message
    public int unread; // unread of message
    public int failed; // failed count of message
    public int notDelivered; // not delivered count of message
    public boolean isPlaceHolder; // for place holder message(only group-chat
                                  // have this flag), if it's true, that mean
                                  // there is not message in the contact.

    public NmsIpSessionMessage(NmsIpMessage aIpMsg, int aCount, int aUnread, int aFailed,
            int aNotDelivered, boolean aIsPlaceHolder) {
        ipMsg = aIpMsg;
        count = aCount;
        unread = aUnread;
        failed = aFailed;
        notDelivered = aNotDelivered;
        isPlaceHolder = aIsPlaceHolder;
    }
}
