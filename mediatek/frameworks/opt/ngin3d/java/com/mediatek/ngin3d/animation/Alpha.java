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

package com.mediatek.ngin3d.animation;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * For Calculating a value as a function of time
 * @hide
 */
public class Alpha implements Cloneable {

    private Timeline mTimeline;
    private Timeline.Listener mTimelineListener;
    private Mode mMode;

    public Alpha() {
        this(null);
    }

    public Alpha(Timeline timeline) {
        this(timeline, Mode.LINEAR);
    }

    public Alpha(Timeline timeline, Mode mode) {
        setTimeline(timeline);
        setMode(mode);
    }

    public final float getAlpha() {
        if (mTimeline == null) {
            return 0;
        }

        float p = mTimeline.getRealProgress();
        float mp = 0.0f;

        switch (mMode) {
        case LINEAR:
            mp = p;
            break;

        /* quadratic */
        case EASE_IN_QUAD:
            mp = p * p;
            break;

        case EASE_OUT_QUAD:
            mp = -1.0f * p * (p - 2);
            break;

        case EASE_IN_OUT_QUAD:
            p *= 2;
            if (p < 1) {
                mp = 0.5f * p * p;
                break;
            }
            p -= 1;
            mp = -0.5f * (p * (p - 2) - 1);
            break;

        /* cubic */
        case EASE_IN_CUBIC:
            mp = p * p * p;
            break;

        case EASE_OUT_CUBIC:
            p -= 1;
            mp = p * p * p + 1;
            break;

        case EASE_IN_OUT_CUBIC:
            p *= 2;
            if (p < 1) {
                mp = 0.5f * p * p * p;
                break;
            }
            p -= 2;
            mp = 0.5f * (p * p * p + 2);
            break;

        /* quartic */
        case EASE_IN_QUART:
            mp = p * p * p * p;
            break;

        case EASE_OUT_QUART:
            p -= 1;
            mp = -1.0f * (p * p * p * p - 1);
            break;

        case EASE_IN_OUT_QUART:
            p *= 2;
            if (p < 1) {
                mp = 0.5f * p * p * p * p;
                break;
            }
            p -= 2;
            mp = -0.5f * (p * p * p * p - 2);
            break;

        /* quintic */
        case EASE_IN_QUINT:
            mp = p * p * p * p * p;
            break;

        case EASE_OUT_QUINT:
            p -= 1;
            mp = p * p * p * p * p + 1;
            break;

        case EASE_IN_OUT_QUINT:
            p *= 2;
            if (p < 1) {
                mp = 0.5f * p * p * p * p * p;
                break;
            }
            p -= 2;
            mp = 0.5f * (p * p * p * p * p + 2);
            break;

        /* sinusoidal */
        case EASE_IN_SINE:
            mp = (float) (-1.0 * Math.cos(p * Math.PI / 2) + 1.0);
            break;

        case EASE_OUT_SINE:
            mp = (float) Math.sin(p * Math.PI / 2);
            break;

        case EASE_IN_OUT_SINE:
            mp = (float) (-0.5 * (Math.cos(Math.PI * p) - 1));
            break;

        /* exponential */
        case EASE_IN_EXPO: {
            int t = mTimeline.getTime();
            mp = (t == 0) ? 0.0f : (float) Math.pow(2, 10 * (p - 1));
            break;
        }

        case EASE_OUT_EXPO: {
            int t = mTimeline.getTime();
            int d = mTimeline.getOriginalDuration();
            mp = (t == d) ? 1.0f : (float) -Math.pow(2, -10 * p) + 1;
            break;
        }

        case EASE_IN_OUT_EXPO: {
            int t = mTimeline.getTime();
            int d = mTimeline.getOriginalDuration();
            if (t == 0) {
                mp = 0.0f;
                break;
            }
            if (t == d) {
                mp = 1.0f;
                break;
            }
            p *= 2;
            if (p < 1) {
                mp = (float) (0.5 * Math.pow(2, 10 * (p - 1)));
                break;
            }
            p -= 1;
            mp = (float) (0.5 * (-Math.pow(2, -10 * p) + 2));
            break;
        }

        /* circular */
        case EASE_IN_CIRC:
            mp = (float) (-1.0 * (Math.sqrt(1 - p * p) - 1));
            break;

        case EASE_OUT_CIRC:
            mp = (float) (-1.0 * (Math.sqrt(1 - p * p) - 1));
            break;

        case EASE_IN_OUT_CIRC:
            p -= 1;
            mp = (float) (Math.sqrt(1 - p * p));
            break;

        /* elastic */
        case EASE_IN_ELASTIC: {
            int t = mTimeline.getTime();
            int d = mTimeline.getOriginalDuration();
            float q = d * .3f;
            float s = q / 4;

            if (p == 1) {
                mp = 1.0f;
                break;
            }

            p -= 1;
            mp = (float) -(Math.pow(2, 10 * p) * Math.sin((p * d - s) * (2 * Math.PI) / q));
            break;
        }

        case EASE_OUT_ELASTIC: {
            int t = mTimeline.getTime();
            int d = mTimeline.getOriginalDuration();
            float q = d * (.3f * 1.5f);
            float s = q / 4;

            if (p == 1) {
                mp = 1.0f;
                break;
            }

            mp = (float) (Math.pow(2, -10 * p) * Math.sin((p * d - s) * (2 * Math.PI) / q) + 1.0);
            break;
        }

        case EASE_IN_OUT_ELASTIC: {
            int t = mTimeline.getTime();
            int d = mTimeline.getOriginalDuration();
            float q = d * (.3f * 1.5f);
            float s = q / 4;
            p *= 2;

            if (p == 2) {
                mp = 1.0f;
                break;
            }

            if (p < 1) {
                p -= 1;
                mp = (float) (-.5 * Math.pow(2, 10 * p) * Math.sin((p * d - s) * (2 * Math.PI) / q));
                break;
            } else {
                mp = (float) (Math.pow(2, -10 * p) * Math.sin((p * d - s) * (2 * Math.PI) / q) * .5 + 1.0);
                break;
            }
        }

        /* overshooting cubic */
        case EASE_IN_BACK:
            mp = p * p * ((1.70158f + 1) * p - 1.70158f);
            break;
        case EASE_OUT_BACK:
            p -= 1;
            mp = p * p * ((1.70158f + 1) * p + 1.70158f) + 1;
            break;
        case EASE_IN_OUT_BACK:
            p *= 2;
            float s = 1.70158f * 1.525f;

            if (p < 1) {
                mp = 0.5f * (p * p * ((s + 1) * p - s));
                break;
            }
            p -= 2;
            mp = 0.5f * (p * p * ((s + 1) * p + s) + 2);
            break;

        /* exponentially decaying parabolic */
        case EASE_IN_BOUNCE:
            mp = easeInBounce(p);
            break;
        case EASE_OUT_BOUNCE:
            mp = easeOutBounce(p);
            break;
        case EASE_IN_OUT_BOUNCE:
            if (p < 0.5f) {
                mp = easeInBounce(p * 2) * 0.5f;
            } else {
                mp = easeOutBounce(p * 2 - 1) * 0.5f + 1.0f * 0.5f;
            }
            break;
        default:
            mp = p;
        }
        return mp;
    }

    private float easeInBounce(float p) {
        return 1.0f - easeOutBounce(1 - p);
    }

    private float easeOutBounce(float p) {
        float mp = p;
        if (mp < (1 / 2.75)) {
            return 7.5625f * mp * mp;
        } else if (mp < (2 / 2.75)) {
            mp -= (1.5 / 2.75);
            return 7.5625f * mp * mp + 0.75f;
        } else if (mp < (2.5 / 2.75)) {
            mp -= (2.25 / 2.75);
            return 7.5625f * mp * mp + 0.9375f;
        } else {
            mp -= (2.625 / 2.75);
            return 7.5625f * mp * mp + 0.984375f;
        }
    }

    public final Mode getMode() {
        return mMode;
    }

    public final void setMode(Mode mode) {
        mMode = mode;
    }

    public final void setTimeline(Timeline timeline) {
        if (mTimeline != null) {
            if (mTimeline.equals(timeline)) {
                return;
            }
            mTimeline.removeListener(mTimelineListener);
            mTimelineListener = null;
        }

        if (timeline != null) {
            mTimeline = timeline;
            mTimelineListener = new Timeline.Listener() {
                public void onStarted(Timeline timeline) {
                    for (Listener l : mListeners) {
                        l.onStarted();
                    }
                }

                public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                    float alpha = getAlpha();
                    for (Listener l : mListeners) {
                        l.onAlphaUpdate(alpha);
                    }
                }

                public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
                    // do nothing now
                }

                public void onPaused(Timeline timeline) {
                    for (Listener l : mListeners) {
                        l.onPaused();
                    }
                }

                public void onCompleted(Timeline timeline) {
                    for (Listener l : mListeners) {
                        l.onCompleted(timeline.getDirection());
                    }
                }

                public void onLooped(Timeline timeline) {
                    // do nothing now
                }
            };
            mTimeline.addListener(mTimelineListener);
        }
    }

    public final Timeline getTimeline() {
        return mTimeline;
    }

    // Alpha Listener

    public interface Listener {
        void onAlphaUpdate(float progress);

        void onStarted();

        void onPaused();

        void onCompleted(int direction);
    }

    List<Listener> mListeners = new CopyOnWriteArrayList<Listener>();

    public final void addListener(Listener l) {
        mListeners.add(l);
    }

    public final void removeListener(Listener l) {
        mListeners.remove(l);
    }

    /**
     * Clone the Alpha, the value of each field in cloned alpha is same of original one.
     * @return the cloned Alpha
     */
    @Override
    protected Alpha clone() {
        try {
            Alpha alpha = (Alpha) super.clone();
            alpha.mListeners =  new ArrayList<Listener>();
            return alpha;
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }
}
