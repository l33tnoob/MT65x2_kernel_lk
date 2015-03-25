package com.mediatek.media;

import android.media.TimedText;

import com.mediatek.xlog.Xlog;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Field;

/**
 * This class is used to get more types of  meta data in  timed text including
 * get byte arrary of characters in timed text
 * <p>Here is an example of prerequisite declaration:</p>
 * <pre class="prettyprint">
 *  public class MainActivity extends Activity {
 *
 *     public void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *         setContentView(R.layout.activity_main);
 *
 *         MediaPlayer mediaplayer = new MediaPlayer();
 *
 *         mediaplayer.setOnTimedTextListener(new TimedTextListener() {
 *              public void onTimedText(MediaPlayer mp, TimedText text){
 *                   byte[] bytes = com.mediatek.media.TimedTextEx.getTextByteChars(text);
 *              }
 *          });
 *     }
 *  }
 * </pre>
 *@hide
 */

public class TimedTextEx {

    private static final String TAG = "TimedTextEx";
    private static final String CLASS_NAME = "android.media.TimedText";
    private static final String TEXT_FIELD_NAME = "mTextByteChars";
    private static final String WIDTH_FIELD_NAME = "mBitMapWidth";
    private static final String HEIGHT_FIELD_NAME = "mBitMapHeight";
    private static final String FD_FIELD_NAME = "mBitMapFd";

    private static Field textField;
    private static Field widthField;
    private static Field heightField;
    private static Field fdField;

    static {
        try {
            Class cls = Class.forName(CLASS_NAME);

            textField = cls.getDeclaredField(TEXT_FIELD_NAME);
            widthField = cls.getDeclaredField(WIDTH_FIELD_NAME);
            heightField = cls.getDeclaredField(HEIGHT_FIELD_NAME);
            fdField = cls.getDeclaredField(FD_FIELD_NAME);

            textField.setAccessible(true);
            widthField.setAccessible(true);
            heightField.setAccessible(true);
            fdField.setAccessible(true);

        } catch (NoSuchFieldException e) {
            Xlog.e(TAG, "NoSuchFieldException");
        } catch (ClassNotFoundException e) {
            Xlog.e(TAG, "ClassNotFoundException: " + CLASS_NAME);
        }
    }

    /**
     * Get the byte array of characters in the timed text.
     *
     * @param timedText TimedText used to hold timed text's meta data
     * @return the characters as byte array in the TimedText. Applications
     * should stop rendering previous timed text at the current rendering region if
     * a null is returned, until the next non-null timed text is received.
     *
     * @hide
     */
    public static byte[] getTextByteChars(TimedText timedText) throws IllegalArgumentException, IllegalAccessException{
        return (byte[])textField.get(timedText);
    }

    /**
     * Get the width of bitmap  in the timed text.
     *
     * @param timedText TimedText used to hold timed text's meta data
     * @return the width in the TimedText. Applications
     * should stop rendering previous timed text at the current rendering region if
     * a null is returned, until the next non-null timed text is received.
     *
     * @hide
     */
    public static int getBitMapWidth(TimedText timedText) throws IllegalArgumentException, IllegalAccessException{
        return (Integer)widthField.get(timedText);
    }

    /**
     * Get the height of bitmap in the timed text.
     *
     * @param timedText TimedText used to hold timed text's meta data
     * @return height in the TimedText. Applications
     * should stop rendering previous timed text at the current rendering region if
     * a null is returned, until the next non-null timed text is received.
     *
     * @hide
     */
    public static int getBitMapHeight(TimedText timedText) throws IllegalArgumentException, IllegalAccessException{
        return (Integer)heightField.get(timedText);
    }

    /**
     * Get the address of bitmap in the timed text.
     *
     * @param timedText TimedText used to hold timed text's meta data
     * @return the address of bitmap in the TimedText. Applications
     * should stop rendering previous timed text at the current rendering region if
     * a null is returned, until the next non-null timed text is received.
     *
     * @hide
     */
    public static int getBitMapFd(TimedText timedText) throws IllegalArgumentException, IllegalAccessException{
        return (Integer)fdField.get(timedText);
    }

}
