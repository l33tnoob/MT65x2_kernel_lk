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

package com.orangelabs.rcs.service.api.client.ipcall;

/**
 * IP call API intents
 * 
 * @author opob7414
 */
public class IPCallApiIntents {

    /**
     * Intent broadcasted when a new IP call invitation has been received
     * 
     * <p>The intent will have the following extra values:
     * <ul>
     *   <li><em>contact</em> - Contact phone number.</li>
     *   <li><em>contactDisplayname</em> - Display name associated to the contact.</li>
     *   <li><em>sessionId</em> - Session ID of the IP call session.</li>
     *   <li><em>audiotype</em> - Audio encoding.</li>
     *   <li><em>videotype</em> - Video encoding.</li>
     *   <li><em>videowidth</em> - Width of the video.</li>
     *   <li><em>videoheight</em> - Height of the video.</li>
     * </ul>
     * </ul>
     */
	public final static String IPCALL_INVITATION = "com.orangelabs.rcs.ipcall.IPCALL_INVITATION";

	/**
     * Intent broadcasted when an action to add video has been received
     * 
     * <p>The intent will have the following extra values:
     * <ul>
     *   <li><em>sessionId</em> - Session ID of the IP call session.</li>
     *   <li><em>videotype</em> - Video encoding.</li>
     *   <li><em>videowidth</em> - Width of the video.</li>
     *   <li><em>videoheight</em> - Height of the video.</li>
     * </ul>
     * </ul>
     */
	public final static String IPCALL_ADD_VIDEO = "com.orangelabs.rcs.ipcall.IPCALL_ADD_VIDEO";
	
	/**
     * Intent broadcasted when an action to remove video has been received
     * 
     * <p>The intent will have the following extra values:
     * <ul>
     *   <li><em>sessionId</em> - Session ID of the IP call session.</li>
     * </ul>
     * </ul>
     */
	public final static String IPCALL_REMOVE_VIDEO = "com.orangelabs.rcs.ipcall.IPCALL_REMOVE_VIDEO";
}
