/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.provider.messaging;

/**
 * Information of a message in RichMessaging
 *
 * @author Benot JOGUET
 */
public class MessageInfo {

    /**
     * Message ID
     */
    private String msgId;

    /**
     * Contact
     */
    private String contact;

    /**
     * Type of message
     */
    private int type;

    /**
     * Session ID
     */
    private String sessionId;

    /**
     * Constructor
     *
     * @param msgId msgId
     * @param contact contact
     * @param type type of message
     * @param sessionId sessionId
     */
    public MessageInfo(String msgId, String contact, int type, String sessionId) {
        this.msgId = msgId;
        this.contact = contact;
        this.type = type;
        this.sessionId = sessionId;
    }

    /**
     * Get MsgId
     *
     * @return MsgId
     */
    public String getMsgId() {
        return msgId;
    }

    /**
     * Get contact
     *
     * @return contact
     */
    public String getContact() {
        return contact;
    }

    /**
     * Get type
     *
     * @return type
     */
    public int getType() {
        return type;
    }

    /**
     * Get sessionId
     *
     * @return sessionId
     */
    public String getSessionId() {
        return sessionId;
    }
}
