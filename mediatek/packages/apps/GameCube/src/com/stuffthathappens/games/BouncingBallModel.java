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

import java.util.concurrent.atomic.AtomicReference;

import android.os.Vibrator;

/**
 * This data model tracks the width and height of the playing field along 
 * with the current position of a ball. 
 */
public class BouncingBallModel {
	// the ball speed is meters / second. When we draw to the screen,
	// 1 pixel represents 1 meter. That ends up too slow, so multiply
	// by this number. Bigger numbers speeds things up.
	private final float pixelsPerMeter = 10;
	
	private final int ballRadius;
	
	// these are public, so make sure you synchronize on LOCK 
	// when reading these. I made them public since you need to
	// get both X and Y in pairs, and this is more efficient than
	// getter methods. With two getters, you'd still need to 
	// synchronize.
	public float ballPixelX, ballPixelY;
	public boolean bounced;
	
	private int pixelWidth, pixelHeight;
	
	// values are in meters/second
	private float velocityX, velocityY;
	
	// typical values range from -10...10, but could be higher or lower if
	// the user moves the phone rapidly
	private float accelX, accelY;

	/**
     * When the ball hits an edge, multiply the velocity by the rebound.
     * A value of 1.0 means the ball bounces with 100% efficiency. Lower
     * numbers simulate balls that don't bounce very much.
     */
    private static final float rebound = 0.7f;

    // if the ball bounces and the velocity is less than this constant,
    // stop bouncing.
    private static final float STOP_BOUNCING_VELOCITY = 2f;

    private volatile long lastTimeMs = -1;
	
	public final Object LOCK = new Object();
	
	private AtomicReference<Vibrator> vibratorRef =
		new AtomicReference<Vibrator>();
	
	public BouncingBallModel(int ballRadius) {
		this.ballRadius = ballRadius;
	}
	
	public void setAccel(float ax, float ay) {
		synchronized (LOCK) {
			this.accelX = ax;
			this.accelY = ay;
		}
	}
	
	public void setSize(int width, int height) {
		synchronized (LOCK) {
			this.pixelWidth = width;
			this.pixelHeight = height;
		}
	}
	
    public int getBallRadius() {
        return ballRadius;
    }

    /**
     * Call this to move the ball to a particular location on the screen. This
     * resets the velocity to zero, but the acceleration doesn't change so
     * the ball should start falling shortly.
     */
    public void moveBall(int ballX, int ballY) {
    	synchronized (LOCK) {
            this.ballPixelX = ballX;
            this.ballPixelY = ballY;
            velocityX = 0;
            velocityY = 0;
        }
    }
    
    public void updatePhysics() {
        // copy everything to local vars (hence the 'l' prefix)
        float lWidth, lHeight, lBallX, lBallY, lAx, lAy, lVx, lVy;
        synchronized (LOCK) {
            lWidth = pixelWidth;
            lHeight = pixelHeight;
            lBallX = ballPixelX;
            lBallY = ballPixelY;
            lVx = velocityX;            
            lVy = velocityY;
            lAx = -accelX;
            lAy = accelY;
        }


        if (lWidth <= 0 || lHeight <= 0) {
            // invalid width and height, nothing to do until the GUI comes up
            return;
        }


        long curTime = System.currentTimeMillis();
        if (lastTimeMs < 0) {
            lastTimeMs = curTime;
            return;
        }

        long elapsedMs = curTime - lastTimeMs;
        lastTimeMs = curTime;
        
        // update the velocity
        // (divide by 1000 to convert ms to seconds)
        // end result is meters / second
        lVx += ((elapsedMs * lAx) / 1000) * pixelsPerMeter;
        lVy += ((elapsedMs * lAy) / 1000) * pixelsPerMeter;

        // update the position
        // (velocity is meters/sec, so divide by 1000 again)
        lBallX += ((lVx * elapsedMs) / 1000) * pixelsPerMeter;
        lBallY += ((lVy * elapsedMs) / 1000) * pixelsPerMeter;
        
        boolean bouncedX = false;
        boolean bouncedY = false;

        if (lBallY - ballRadius < 0) {
            lBallY = ballRadius;
            lVy = -lVy * rebound;
            bouncedY = true;
        } else if (lBallY + ballRadius > lHeight) {
            lBallY = lHeight - ballRadius;
            lVy = -lVy * rebound;
            bouncedY = true;
        }
        if (bouncedY && Math.abs(lVy) < STOP_BOUNCING_VELOCITY) {
            lVy = 0;  
            bouncedY = false;
        }

        if (lBallX - ballRadius < 0) {
        	lBallX = ballRadius;
        	lVx = -lVx * rebound;
        	bouncedX = true;
        } else if (lBallX + ballRadius > lWidth) {
            lBallX = lWidth - ballRadius;
            lVx = -lVx * rebound;
            bouncedX = true;
        }
        if (bouncedX && Math.abs(lVx) < STOP_BOUNCING_VELOCITY) {
        	lVx = 0;
        	bouncedX = false;
        }
        

        // safely copy local vars back to object fields
        synchronized (LOCK) {
            ballPixelX = lBallX;
            ballPixelY = lBallY;
            
            velocityX = lVx;
            velocityY = lVy;
            
            bounced = (bouncedX || bouncedY);
        }
        
        if (bouncedX || bouncedY) {
        	Vibrator v = vibratorRef.get();
        	if (v != null) {
        		v.vibrate(10L);
        	}
        }
    }
    
    public void setVibrator(Vibrator v) {
    	vibratorRef.set(v);
    }
}
