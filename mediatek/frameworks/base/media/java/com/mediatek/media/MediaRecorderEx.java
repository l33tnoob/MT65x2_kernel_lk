package com.mediatek.media;

import android.media.MediaRecorder;

import com.mediatek.xlog.Xlog;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This class is used to support audio new features including set artist and album tag as well as recording pause functionality.
 *
 * <p>When developing audio application, the tip goes through 4 steps:</p>
 * MediaRecorder.prepare --> MediaRecorder.start --> MediaRecorder.pause --> MediaRecorder.stop
 * Specially you can repeat start and pause recodring file many times
 * MediaRecorder.prepare --> MediaRecorder.start --> MediaRecorder.pause --> MediaRecorder.start --> MediaRecorder.pause --> MediaRecorder.stop
 *
 * <p>Here is an example of prerequisite declaration:</p>
 * <pre class="prettyprint">
 *  public class MainActivity extends Activity {
 *
 *     public void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *         setContentView(R.layout.activity_main);
 *
 *         String mOutPath = new File(Environment.getExternalStorageDirectory(),
 *            "test.3gp").getAbsolutePath();
 *
 *         final MediaRecorder recorder = new MediaRecorder();
 *
 *         recorder.setAudioSource(MediaRecorder.AudioSource.MIC);
 *         recorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
 *         recorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
 *         recorder.setOutputFile(mOutPath);
 *         com.mediatek.media.MediaRecorderEx.setArtistTag(recorder, "artist test");
 *         com.mediatek.media.MediaRecorderEx.setAlbumTag(recorder, "album test");
 *         recorder.prepare();
 *         com.mediatek.media.MediaRecorderEx.setHDRecordMode(recorder,1, false);
 *         recorder.start();
 *         com.mediatek.media.MediaRecorderEx.pause(recorder);
 *         recorder.stop();
 *     }
 *  }
 * </pre>
 */
 
public class MediaRecorderEx {

    private static final String TAG = "MediaRecorderEx";
    private static final String CLASS_NAME = "android.media.MediaRecorder";
    private static final String METHOD_NAME = "setParameter";
    private static final Class[] METHOD_TYPES = new Class[] {String.class};
    private static Method sSetParameter;
    static {
        try {
            Class cls = Class.forName(CLASS_NAME);
            sSetParameter = cls.getDeclaredMethod(METHOD_NAME, METHOD_TYPES);
            if (sSetParameter != null) {
                sSetParameter.setAccessible(true);
            }
        } catch (NoSuchMethodException e) {
            Xlog.e(TAG, "NoSuchMethodException: " + METHOD_NAME);
        } catch (ClassNotFoundException e) {
            Xlog.e(TAG, "ClassNotFoundException: " + CLASS_NAME);
        }
    }

    private static void setParameter(MediaRecorder recorder, String nameValuePair) {
        if (sSetParameter != null) {
            try {
                sSetParameter.invoke(recorder, nameValuePair);
            } catch (IllegalAccessException ae) {
                Xlog.e(TAG, "IllegalAccessException!", ae);
            } catch (InvocationTargetException te) {
                Xlog.e(TAG, "InvocationTargetException!", te);
            } catch (IllegalArgumentException ex) {
                Xlog.e(TAG, "IllegalArgumentException!", ex);
            } catch (NullPointerException npe) {
                Xlog.e(TAG, "NullPointerException!", npe);
            }
        } else {
            Xlog.e(TAG, "setParameter: Null method!");
        }
    }

    /**
     * Pauses the recording.
     * Call this method after MediaRecorder.start().
     * In addition, call MediaRecorder.start() to resume the recorder after this method is called.
     * 
     * @param recorder Recorder used to record audio
     * @throws IllegalStateException If it is not called after MediaRecorder.start()
     */
    public static void pause(MediaRecorder recorder) throws IllegalStateException {
        if (recorder == null) {
            Xlog.e(TAG, "Null MediaRecorder!");
            return;
        }
        recorder.setParametersExtra("media-param-pause=1");
    }

    /**
     * Defines the HD record mode. These constants are used with
     * {@link MediaRecorderEx#setHDRecordMode(MediaRecorder, int, boolean)}.
     */
    public final class HDRecordMode {
        /* Do not change these values without updating their counterparts
         * in AudioYusuHardware.cpp.
         */
        private HDRecordMode() {}
        /** Normal mode */
        public static final int NORMAL = 0;
        /** Indoor mode */
        public static final int INDOOR = 1;
        /** Outdoor mode */
        public static final int OUTDOOR = 2;
    }

    /**
     * Sets up the HD record mode to be used for recording. 
     * Call this method before MediaRecorder.prepare().
     *
     * @param recorder Recorder used to record audio or video
     * @param mode HD record mode to be used
     * @param isVideo True if it is used for record video; otherwise, false.
     * @throws IllegalStateException If it is called after MediaRecorder.prepare()
     * @see com.mediatek.media.MediaRecorderEx.HDRecordMode
     */
    public static void setHDRecordMode(MediaRecorder recorder, int mode, boolean isVideo)
            throws IllegalStateException, IllegalArgumentException {
        if (mode < HDRecordMode.NORMAL || mode > HDRecordMode.OUTDOOR) {
            throw new IllegalArgumentException("Illegal HDRecord mode:" + mode);
        }
        
        if (isVideo) {
            setParameter(recorder, "audio-param-hdrecvideomode=" + mode);
        } else {
            setParameter(recorder, "audio-param-hdrecvoicemode=" + mode);
        }
    }

    /**
     * Sets up the artist meta data to be saved in file header during recording.
     * It only works for MediaRecorder.OutputFormat.THREE_GPP. For other MediaRecorder.OutputFormat, it does nothing.
     * Call this method before MediaRecorder.prepare().
     *
     * @param recorder Recorder used to record audio
     * @param artist Artist name to be set
     * @throws IllegalStateException If it is called after MediaRecorder.prepare()
     */
    public static void setArtistTag(MediaRecorder recorder, String artist) throws IllegalStateException {
        setParameter(recorder, "media-param-tag-artist=" + artist);
    }

    /**
     * Sets up the album meta data to be saved in file header during recording.
     * It only works for MediaRecorder.OutputFormat.THREE_GPP. For other MediaRecorder.OutputFormat, it does nothing.
     * Call this method only before MediaRecorder.prepare().
     * 
     * @param recorder Recorder used to record audio
     * @param album Album name to be set
     * @throws IllegalStateException If it is called after MediaRecorder.prepare()
     */
    public static void setAlbumTag(MediaRecorder recorder, String album) throws IllegalStateException {
        setParameter(recorder, "media-param-tag-album=" + album);
    }

    /**
     * Sets up the effect option during recording. 
     * 
     * @param recorder Recorder used to record audio
     * @param effectOption Effect option to be set
     * @throws IllegalStateException If it is called after MediaRecorder.prepare()
     * 
     * @hide
     */
    public static void setPreprocessEffect(MediaRecorder recorder, int effectOption) throws IllegalStateException {
        setParameter(recorder, "audio-param-preprocesseffect=" + effectOption);
    }
    
    /**
     * @hide
     */
    public static void setVideoBitOffSet(MediaRecorder recorder, int offset, boolean video) {
        if(video) {
            setParameter(recorder,"param-use-64bit-offset=" + offset);
            Xlog.v(TAG,"setVideoBitOffSet is true,offset= " + offset);
        }
    }

    /**
     * Enable live photo mode.
     * 
     * @param recorder Recorder used to record.
     * @hide
     */
    public static void setLivePhotoMode(MediaRecorder recorder) {
        recorder.setParametersExtra("media-param-livephoto=1");
    }

    /**
     * Set live photo tag.
     * 
     * @param recorder Recorder used to record.
     * @param value The tag value to be set.
     * @hide
     */
    public static void setLivePhotoTag(MediaRecorder recorder, int value) {
        recorder.setParametersExtra("media-param-tag-livephoto=" + value);
    }

    /**
     * Enable to save live photo.
     * 
     * @param recorder Recorder used to record.
     * @hide
     */
    public static void captureLivePhoto(MediaRecorder recorder) {
        recorder.setParametersExtra("media-param-capture-livephoto=1");
    }
}
