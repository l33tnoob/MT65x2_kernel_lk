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

package com.stuffthathappens.games;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import android.content.Context;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;

/**
 * This data model tracks bubbles on the screen.
 * 
 * @see BubblesActivity
 */
public class BubblesModel implements OnCompletionListener {
	
	private static final float INITIAL_RADIUS = 20f;
	private static final float MAX_RADIUS = 100f;
	
	// higher numbers make the balls expand faster
	private static final float RADIUS_CHANGE_PER_MS = .08f;
	
	private final List<MediaPlayer> players = new LinkedList<MediaPlayer>();
	private boolean running = false;
	
	public static final class Bubble {
		float x, y, radius;
		
		public Bubble(float x, float y) {
			this.x = x;
			this.y = y;
			radius = INITIAL_RADIUS;
		}
	}
	
	private final List<Bubble> bubbles = new LinkedList<Bubble>();
	

    private volatile long lastTimeMs = -1;
	
	public final Object LOCK = new Object();
	
	public BubblesModel() {		
	}
	
	public void onResume(Context context) {
		synchronized (LOCK) {
			for (int i=0; i<4; i++) {
				MediaPlayer mp = MediaPlayer.create(context, R.raw.pop);
				mp.setVolume(1f, 1f);
				players.add(mp);
				try {
					mp.setLooping(false);
					mp.setOnCompletionListener(this);
					
					// TODO: there is a serious bug here. After a few seconds of
					// inactivity, we see this in LogCat:
					//   AudioHardwareMSM72xx Going to standby 
					// then the sounds don't play until you click several more
					// times, then it starts working again
					
				} catch (Exception e) {
					e.printStackTrace();
					players.remove(mp);
				}
			}
			running = true;
		}
	}
	
	public void onPause(Context context) {
		synchronized (LOCK) {
			running = false;
			for (MediaPlayer p : players) {
				p.release();
			}
			players.clear();
		}
	}
	
	public List<Bubble> getBubbles() {
		synchronized (LOCK) {
			return new ArrayList<Bubble>(bubbles);
		}
	}
	
	public void addBubble(float x, float y) {
		synchronized (LOCK) {
			bubbles.add(new Bubble(x,y));
		}
	}
	
	public void setSize(int width, int height) {
		// TODO ignore this for now...we could hide bubbles that
		// are out of bounds, for example
	}

    public void updateBubbles() {
        long curTime = System.currentTimeMillis();
        if (lastTimeMs < 0) {
            lastTimeMs = curTime;
            // this is the first reading, so don't change anything
            return;
        }
        long elapsedMs = curTime - lastTimeMs;
        lastTimeMs = curTime;
        
        final float radiusChange = elapsedMs * RADIUS_CHANGE_PER_MS;

        MediaPlayer mp = null;

    	synchronized (LOCK) {
    		Set<Bubble> victims = new HashSet<Bubble>();
    		
    		for (Bubble b : bubbles) {
    			b.radius += radiusChange;
    			if (b.radius > MAX_RADIUS) {
    				victims.add(b);
    			}
    		}
    		
    		if (victims.size() > 0) {
    			bubbles.removeAll(victims);
    			// since a bubble popped, try to get a media player
    			if (!players.isEmpty()) {    				
    				mp = players.remove(0);
    			}
    		}
    	}
    	
    	if (mp != null) {
    		//System.out.println("**pop**");
    		mp.start(); 
    	}
    }

	public void onCompletion(MediaPlayer mp) {
		synchronized (LOCK) {
			if (running) {
	    		mp.seekTo(0);
				//System.out.println("on completion!");
	    		// return the player to the pool of available instances
				players.add(mp);
			}
		}
	}
}
