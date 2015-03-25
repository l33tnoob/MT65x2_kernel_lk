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

package com.orangelabs.rcs.utils;

import com.orangelabs.rcs.platform.network.DatagramConnection;
import com.orangelabs.rcs.platform.network.NetworkFactory;
import com.orangelabs.rcs.platform.network.SocketServerConnection;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.logger.Logger;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Network ressource manager
 *
 * @author jexa7410
 */
public class NetworkSwitchInfo {

	/**
		 * The logger
		 */
		private Logger logger = Logger.getLogger(this.getClass().getName());
	

    private static boolean ims_off_network = false;

	public void set_ims_off_by_network(){
		if (logger.isActivated()) {
    		logger.info("networkswichinfo set_ims_off_by_network");
			}
		ims_off_network = true;
		}

	public boolean get_ims_off_by_network(){
		if (logger.isActivated()) {
			if(ims_off_network){
    		logger.info("networkswichinfo get_ims_off_by_network true");
				}
			else {
    		logger.info("networkswichinfo get_ims_off_by_network false");
				}
			}
		return ims_off_network;
		}
	
	public void reset_ims_off_by_network(){
		if (logger.isActivated()) {
    		logger.info("networkswichinfo reset_ims_off_by_network");
			}
		ims_off_network = false;
		}
}
