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

package com.mediatek.banyan.widget;

import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.BitmapDrawable;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.ImageView;
import android.widget.RemoteViews.RemoteView;
import android.view.RemotableViewMethod;

import java.io.InputStream;
import java.io.IOException;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.util.Log;
import android.net.Uri;
import android.content.ContentResolver;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Bitmap.CompressFormat;
import android.media.ExifInterface;
import java.io.FileNotFoundException;
//import com.mediatek.gifDecoder.GifDecoder;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.gifdecoder.IGifDecoder;
import android.os.Handler;
import android.os.Message;

@RemoteView
public class MTKImageView extends ImageView
{
	private Context mContext; 
	
	private static final String STORE_PIC_TAG="storePic"; 
	private static final int MAX_WIDTH = 640;
	private static final int MAX_HEIGHT = 480;
	/// M: fix Gif animation consumes too much memory bug
	private Bitmap mLastFrame = null;
	/**
	 * @param context The Context to attach
	 */
	public MTKImageView(Context context)
	{
		super(context);
		this.mContext = context;

		//add for gif animation
		initForGif();
	}
	
	/**
	 * @param context The Context to attach
	 * @param attrs The attribute set
	 */
	public MTKImageView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		this.mContext = context;

		//add for gif animation
		initForGif();
	}
	
	/**
	 * @param context The Context to attach
	 * @param attrs The attribute set
	 * @param defStyle The used style
	 */
	public MTKImageView(Context context, AttributeSet attrs, int defStyle)
	{
		super(context, attrs, defStyle);
		this.mContext = context;

		//add for gif animation
		initForGif();
	}

	
	/**
	 * Register a callback to be invoked when this view is clicked. If this view is not
     * clickable, it becomes clickable.
     * 
	 * @param action An action be insert into Intent for startService.
	 */
	@android.view.RemotableViewMethod
	public void setOnClickIntent(final String action)
	{
		this.setOnClickListener(new View.OnClickListener()
		{
			public void onClick(View v)
			{
				final float appScale = v.getContext().getResources().getCompatibilityInfo().applicationScale;
				final int[] pos = new int[2];
				v.getLocationOnScreen(pos);

				final Rect rect = new Rect();
				rect.left = (int) (pos[0] * appScale + 0.5f);
				rect.top = (int) (pos[1] * appScale + 0.5f);
				rect.right = (int) ((pos[0] + v.getWidth()) * appScale + 0.5f);
				rect.bottom = (int) ((pos[1] + v.getHeight()) * appScale + 0.5f);

				final Intent intent = new Intent();
				intent.setAction(action);
				intent.putExtra("widgetX",pos[0]);
				intent.putExtra("widgetY",pos[1]);
				intent.putExtra("widgetWidth",v.getWidth());
				intent.putExtra("widgetHeight",v.getHeight());
				intent.setSourceBounds(rect);
				mContext.startService(intent);
			}
		});
	}

	/**
	 * @see android.view.View#setEnabled(boolean)
	 */
	@android.view.RemotableViewMethod
	public void setEnabled(boolean enabled)
	{
		super.setEnabled(enabled);
	}
	
	/**
	 * @param flag An flag to start or stop the animation of an ImageView. true for start, and false for stop.
	 */
	@android.view.RemotableViewMethod
	public void setAnimationRunning(boolean flag)
	{
		Drawable drawable = this.getDrawable();
		if(flag)
		{
			if(drawable!=null && (drawable instanceof AnimationDrawable))
			{
				AnimationDrawable tempAD = (AnimationDrawable)drawable;
				if(!tempAD.isRunning())
					tempAD.start();
			}
			else
			{
				ImageView iv = null;
				ViewParent parent = this.getParent();
				if(parent instanceof ViewGroup)
				{
					ViewGroup vg = (ViewGroup)parent;
					iv = (ImageView) vg.findViewWithTag(STORE_PIC_TAG);
				}
				else
				{
					Log.e("test", "ViewParent is not a ViewGroup!");
					return;
				}
				
				if(iv!=null && (iv instanceof ImageView))
				{
					Drawable d = iv.getBackground();
					if(d!=null && d instanceof AnimationDrawable)
					{
						AnimationDrawable ad = (AnimationDrawable)d;
						this.setImageDrawable(ad);
						ad.start();
					}
				}
				else
				{
					Log.e("com.mediatek.sns", "Iv is null");
				}
			}
		}
		//stop animation
		else
		{
			if(drawable!=null && (drawable instanceof AnimationDrawable))
			{
				AnimationDrawable tempAD = (AnimationDrawable)drawable;
				if(tempAD.isRunning())
					tempAD.stop();
			}
			ImageView iv = null;
			ViewParent parent = this.getParent();
			if(parent instanceof ViewGroup)
			{
				ViewGroup vg = (ViewGroup)parent;
				iv = (ImageView) vg.findViewWithTag(STORE_PIC_TAG);
			}
			else
			{
				Log.e("test", "ViewParent is not a ViewGroup!");
				return;
			}
			
			if(iv!=null && (iv instanceof ImageView))
			{
				Drawable d = iv.getDrawable();
				if(d!=null /*&& d instanceof BitmapDrawable*/)
				{
					this.setImageDrawable(d);
				}
			}
			else
			{
				Log.e("com.mediatek.sns", "Iv is null");
			}
			
		}
	}
	


	private static final int MSG_RUN_OBJECT = 1024 + 1;

	private boolean mSetFromGif = false;
	protected int mResourceId;
	protected Uri mUri = null;
	private boolean mResGif = false;
	private boolean mUriGif = false;
	protected InputStream mGifStream = null;
	private ImageView self;
	private int mCurrGifFrame = 0;
	private Thread mAnimationThread;
	//protected GifDecoder mGifDecoder = null;
	protected IGifDecoder mIGifDecoder = null;
    // This is used to stop the worker thread.
	volatile boolean mAbort = false;
	protected Handler mHandler = new Handler() {
            @Override
            @SuppressWarnings("unchecked")
            public void handleMessage(Message message) {
                switch (message.what) {
                    case MSG_RUN_OBJECT:
                        ((Runnable) message.obj).run();
                        return;
                    default:
                        super.handleMessage(message);
                }
            }
        };

	protected void initForGif() {
		self = this;
	}

	private void closeGifStream() {
		//close previous gif stream if any
		try {
			if (null != mGifStream) {
				mGifStream.close();
				mGifStream = null;
			}
		} catch(IOException e) {
			Log.e("MTKImageView","Close GIF InputStream failed, e="+e);
		}
	}

	private void openGifStream() {
		//close previous gif stream if any
		closeGifStream();
		if (mUriGif == mResGif) {
			Log.e("MTKImageView","openGifStream:not correct status!");
			return;
		}
		if (mResGif) {
			try {
				//Open GIF resource as inputStream
				Log.d("MTKImageView","openGifStream:open new gif strem from "+mResourceId);
				mGifStream = mContext.getResources().openRawResource(mResourceId);
			} catch(Resources.NotFoundException e) {
				Log.e("MTKImageView","Open GIF resource as InputStream failed");
			}
		} else {
			try {
				Log.d("MTKImageView","openGifStream:open new gif strem from "+mUri);
				mGifStream = mContext.getContentResolver().openInputStream(mUri);
			} catch(IOException e) {
				Log.e("MTKImageView","Open GIF URI as InputStream failed");
			}
		}
	}

	/**
	 * Sets a drawable as the content of this MTKImageView.
	 *
	 * @param resId the resource identifier of the the drawable
	 *
	 */
	@android.view.RemotableViewMethod
	public void setImageResource(int resId) {
		Log.i("MTKImageView","setImageResource:abort previous gif animation if any");
		abortAnimationThread();

		InputStream imageStream = null;
		byte []buffer = new byte[4];
		boolean isGifImage = false;
		try {
			imageStream = mContext.getResources().openRawResource(resId);
			if (3 != imageStream.read(buffer,0,3)) {
				Log.w("MTKImageView","can't read data from resource inputstream");
				isGifImage = false;
			} else if (buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
				isGifImage = true;
			} else {
				isGifImage = false;
			}

			imageStream.close();
			imageStream = null;
		} catch(IOException e) {
			Log.e("MTKImageView",""+e);
		}

		if (false == isGifImage) {
			super.setImageResource(resId);
			return;
		}
		//for gif image resource, we should play animation
		mResourceId = resId;

		//recorded gif stream as a Resource
		mUriGif = false;
		mResGif = true;

		startAnimationThread();
	}
	
	private class Bound{
		private int width;
		private int height;
	}
	
    /**
     * corresponding orientation of EXIF to degrees.
     */
    private int getExifRotation(int orientation) {
        int degrees = 0;
        switch (orientation) {
            case ExifInterface.ORIENTATION_NORMAL:
                degrees = 0;
                break;
            case ExifInterface.ORIENTATION_ROTATE_90:
                degrees = 90;
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                degrees = 180;
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                degrees = 270;
                break;
        }
        return degrees;
    }
    
    // Rotates the bitmap by the specified degree.
    // If a new bitmap is created, the original bitmap is recycled.
    public static Bitmap rotate(Bitmap b, int degrees) {
        if (degrees != 0 && b != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees,
                    (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            try {
            	b.setHasAlpha(true);
                Bitmap b2 = Bitmap.createBitmap(
                        b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
            } catch (OutOfMemoryError ex) {
                // We have no memory to rotate. Return the original bitmap.
            }
        }
        return b;
    }
	
	private Bound decodeBoundsInfo(Uri uri) {
        InputStream input = null;
        Bound bound = new Bound();
        try {
            input = mContext.getContentResolver().openInputStream(uri);
            BitmapFactory.Options opt = new BitmapFactory.Options();
            opt.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(input, null, opt);
            
            bound.width = opt.outWidth;
            bound.height = opt.outHeight;
        } catch (FileNotFoundException e) {
            // Ignore
        	Log.e("MTKImageView", e.getMessage(), e);
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    // Ignore
                	Log.e("MTKImageView", e.getMessage(), e);
                }
            }
        }
        
        return bound;
    }
	
	private int decodeDegreeInfo(Uri uri) {
		InputStream inputForRotate = null;
        int orientation = 0;
        int degree = 0;
        try {
        	inputForRotate = mContext.getContentResolver().openInputStream(uri);
        	if (inputForRotate != null) {
        		ExifInterface exif = new ExifInterface(inputForRotate);
        		if (exif != null) {
                    orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
                    degree = getExifRotation(orientation);
                }
        	}   
        } catch (FileNotFoundException e) {
        	Log.e("MTKImageView", e.getMessage(), e);
        } catch (IOException e) {
        	Log.e("MTKImageView", e.getMessage(), e);
        } finally {
            if (inputForRotate != null) {
                try {
                    inputForRotate.close();
                } catch (Exception e) {
                	Log.e("MTKImageView", e.getMessage(), e);
                }
            }
        }
        return degree;
	}
	
    private Bitmap resizeAndRotateImage(Uri uri) {
    	// here we need compress the image to 640*480 max limit 
        // and rotate it if it has rotation degree.
    	Bitmap finalImage = null;
        // get the rotation degree;
        Bound bound = decodeBoundsInfo(uri);
        int imageWidth = bound.width;
        int imageHeight = bound.height;
        
        int scaleFactor = 1;
        int degree = decodeDegreeInfo(uri);

        if (degree == 90 || degree == 270) {
        	imageWidth = bound.height;
        	imageHeight = bound.width;
        }

        if (imageWidth > MAX_WIDTH || imageHeight > MAX_HEIGHT) {
        	Log.e("MTKImageView", "Image need resize: " + imageWidth + "," + imageHeight);
        	do {
                scaleFactor *= 2;
            } while ((imageWidth / scaleFactor > MAX_WIDTH) || (imageHeight / scaleFactor > MAX_HEIGHT));
        }
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = scaleFactor;
        InputStream input = null;
        try {
        	input = mContext.getContentResolver().openInputStream(uri);
            if (input != null) {
                try {
                    finalImage = BitmapFactory.decodeStream(input, null, options);
                } catch (OutOfMemoryError ex) {
                    // decode fail because of no memory, return null to invoke default setImageUri.
                    return null;
                }
                if (finalImage == null) {
                    //decode fail, return null will invoke setImageUri, this method can handle bad pictures.
                    return null;
                }
                try {
                    finalImage = Bitmap.createScaledBitmap(finalImage, (bound.width / scaleFactor) > 0 ? (bound.width / scaleFactor) :1,
                            (bound.height / scaleFactor) >0 ? (bound.height / scaleFactor) :1, false);
                }catch (OutOfMemoryError ex) {
                    // We have no memory to scale. go on process the original bitmap.
                }
                finalImage = rotate(finalImage, degree);
            }
        } catch (FileNotFoundException e) {
        	Log.e("MTKImageView", e.getMessage(), e);
        } finally {
            if (input != null) {
                try {
                	input.close();
                } catch (Exception e) {
                	Log.e("MTKImageView", e.getMessage(), e);
                }
            }
        }
		
        return finalImage;
    }

	/**
	 * Sets the content of this ImageView to the specified Uri.
	 *
	 * @param uri The Uri of an image
	 *
	 */
	@android.view.RemotableViewMethod
	public void setImageURI(Uri uri) {
		Log.v("MTKImageView","setImageURI(uri="+uri+")" + " //this=" + this);
		abortAnimationThread();

		if (null == uri) {
			Log.d("MTKImageView","setImageURI:follow ImageView's routin for "+uri);
			super.setImageURI(uri);
			return;
		}

		InputStream imageStream = null;
		byte []buffer = new byte[4];
		boolean isGifImage = false;
		Bitmap finalImage = null;
		try {
			if ( uri.getScheme().equals(ContentResolver.SCHEME_CONTENT) 
			  || uri.getScheme().equals(ContentResolver.SCHEME_FILE)
			  || uri.getScheme().equals(ContentResolver.SCHEME_ANDROID_RESOURCE))
			{
				imageStream = mContext.getContentResolver().openInputStream(uri);
				if (3 != imageStream.read(buffer,0,3)) {
					Log.w("MTKImageView","can't read data from uri inputstream");
					isGifImage = false;
				} else if (buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F') {
					isGifImage = true;
				} else {
					isGifImage = false;
					finalImage = resizeAndRotateImage(uri);
				}

				imageStream.close();
				imageStream = null;
			} else { 
				Log.w("MTKImageView","Uncoped uri scheme,call ImageView.setImageURI()");
				isGifImage = false;
			}
		} catch(IOException e) {
			Log.e("MTKImageView",""+e);
		}

		Log.i("MTKImageView","setImageURI:isGifImage="+isGifImage + " //this=" + this);

		if (false == isGifImage) {
			Log.d("MTKImageView","setImageURI:follow ImageView's routin for "+uri);
            if (finalImage != null) {
                super.setImageBitmap(finalImage);
            } else {
			    super.setImageURI(uri);
            }
			return;
		}

		//for gif image source, we should play animation
		Log.d("MTKImageView","setImageUri:synchroized lock, start gif animation");
		mUri = uri;

		//recorded gif stream as a Uri
		mUriGif = true;
		mResGif = false;

		startAnimationThread();
	}

	@android.view.RemotableViewMethod
	public void setImageBitmap(Bitmap bm) {
		if (!mSetFromGif) {
			abortAnimationThread();
		}
		super.setImageBitmap(bm);
	}

	@Override
	public void setImageDrawable(Drawable drawable) {
		if (!mSetFromGif) {
			abortAnimationThread();
		}
		super.setImageDrawable(drawable);
	}

	private void startAnimationThread() {
		if (mAnimationThread != null) {
			return;
		}

        mAbort = false;
		Thread t = new Thread(new GifThread());
		t.setName("gif-animation");
		t.start();
		mAnimationThread = t;
	}

	private void abortAnimationThread() {
		Log.v("MTKImageView", "abortAnimationThread()");
		if (mAnimationThread == null) {
			Log.d("MTKImageView", "abortAnimationThread:thread null");
			return;
		}

		mAbort = true;
		try {
			//wake up the thread if it is sleeping
			mAnimationThread.interrupt();
			//mAnimationThread.join(500);
			//mAnimationThread.interrupt();
			mAnimationThread.join();
		} catch (InterruptedException ex) {
			Log.e("MTKImageView", "abortAnimationThread:join interrupted");
		}
		mAnimationThread = null;

		//remove any pending Runnable in the message queue.
		Log.d("MTKImageView","abortAnimationThread:remove messages");
		mHandler.removeMessages(MSG_RUN_OBJECT);
	}

	protected class GifThread implements Runnable {
		public void run() {
			gifAnimation();
		}//run
	}//GifThread

	private void gifAnimation() {
		try {
			//open gif stream
			Log.v("MTKImageView","gifAnimation:call openGifStream()");
			openGifStream();
			if (null == mGifStream) {
				Log.w("MTKImageView","gifAnimation:got null mGifStream");
				return;
			}
			if (mAbort) {
				Log.v("MTKImageView", "gifAnimation:after open stream:thread aborted");
				return;
			}
			//mGifDecoder = new GifDecoder(mGifStream);
			//MTK Framework refactory: call factory to create the GifDecoder
			mIGifDecoder = null;
			try {
				mIGifDecoder = MediatekClassFactory.createInstance(IGifDecoder.class, mGifStream);
			} catch (Exception e) {
				e.printStackTrace();
			}

			if (mAbort) {
				Log.v("MTKImageView", "gifAnimation:after new GifDecoder:thread aborted");
				return;
			}

			if (null == mIGifDecoder) {
				Log.w("MTKImageView","Decode GIF resource failed");
				//when we are sure gif animation is not available, we try to
				//display a static gif image.
				showFirstGifFrame();
				return;
			}
			if (IGifDecoder.INVALID_VALUE == mIGifDecoder.getTotalFrameCount()) {
				Log.e("MTKImageView","gifAnimation:decode gif stream fails");
				//mGifDecoder.close();//no need as Movie implements finalize
				mIGifDecoder = null;
				//when we are sure gif animation is not available, we try to
				//display a static gif image.
				showFirstGifFrame();
				return;
			}

			long frameDuration = 0;
			int totalFrameCount = mIGifDecoder.getTotalFrameCount();
			mCurrGifFrame = 0;

			while (true) {
        	    //load current bitmap
				Bitmap gifFrame = mIGifDecoder.getFrameBitmap(mCurrGifFrame);

				if (mAbort) {
					Log.v("MTKImageView", "gifAnimation:after decode:thread aborted");
					break;
				}

				//post current bitmap to UI thread
				mHandler.sendMessage(
					mHandler.obtainMessage(MSG_RUN_OBJECT, new GifFrameRunnable(gifFrame)));

				//load GiF frame bitmap
				frameDuration = (long)mIGifDecoder.getFrameDuration(mCurrGifFrame);
				Log.v("MTKImageView","sleep for " + frameDuration+" ms for frame " +
										mCurrGifFrame + " //this=" + self);

				if (mAbort) {
					Log.v("MTKImageView", "gifAnimation:animating:thread aborted");
					break;
				}

				try {
					Thread.sleep(frameDuration);
				}  catch (InterruptedException ex) {
					Log.v("MTKImageView", "gifAnimation:sleeping interrupted");
				}

				//if thread is cancelled after wait, then break
				if (1 == totalFrameCount) {
					Log.w("MTKImageView","gifAnim:single frame, cancel");
					break;
				}
				mCurrGifFrame = (mCurrGifFrame+1) % totalFrameCount;
			}//while
		} finally {
			//close GifDecoder when exit the thread
			if (mIGifDecoder != null) {
				//mGifDecoder.close();//no need as Movie implements finalize
				mIGifDecoder = null;
			}//if
			//close gif stream when finish gif animation
			closeGifStream();
		}
	}

	class GifFrameRunnable implements Runnable {
		Bitmap mFrame;
		GifFrameRunnable(Bitmap b) {
			mFrame = b;
		}

		public void run() {
			if (mAbort) {
				Log.v("MTKImageView", "GifFrameRunnable:run:gif aborted");
				if (null != mFrame) {
					Log.d("MTKImageView", "GifFrameRunnable:run:recycle " + mFrame);
					mFrame.recycle();
					mFrame = null;
				}
				return;
			}
			mSetFromGif = true;
			Log.w("MTKImageView","GifFrameRunnable:run:call setImageBitmap(mFrame="+mFrame+")");
			setImageBitmap(mFrame);
			/// M: fix Gif animation consumes too much memory bug. @{
			if (null != mLastFrame) {
			    Log.d("MTKImageView", "GifFrameRunnable:run:recycle " + mLastFrame);
			    mLastFrame.recycle();
			    mLastFrame = null;
			}
			mLastFrame = mFrame;
			/// @}
			//invalidate();
			mSetFromGif = false;
		}
    }

	private void showFirstGifFrame() {
		openGifStream();
		if (null == mGifStream) return;
		Bitmap firstFrame = null;
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inJustDecodeBounds = false;
		options.inSampleSize = 1;
		try {
			firstFrame = BitmapFactory.decodeStream(mGifStream, null, options);
		} catch (OutOfMemoryError ex) {
			Log.e("MTKImageView","showFirstGifFrame:OOM when decoding");
			return;
		}
		if (mAbort) {
			Log.v("MTKImageView", "showFirstGifFrame:thread aborted");
			return;
		}
		if (null != firstFrame) {
			//post current bitmap to UI thread
			mHandler.sendMessage(
				mHandler.obtainMessage(MSG_RUN_OBJECT, new GifFrameRunnable(firstFrame)));
		} else {
			Log.w("MTKImageView", "showFirstGifFrame:failed to decode first frame!");
		}
	}
}
