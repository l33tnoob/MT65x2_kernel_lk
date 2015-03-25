package mediatek.app.cts;

import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.hardware.input.InputManager;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.graphics.Point;
import android.view.Display;

import com.jayway.android.robotium.solo.Solo;

public class ANRTest extends ActivityInstrumentationTestCase2<MockANRActivity> {
    private Solo solo;

    public ANRTest() {
        super(MockANRActivity.class);
    }

    private void sendKey(KeyEvent event)
    {
        long downTime = event.getDownTime();
        long eventTime = event.getEventTime();
        int action = event.getAction();
        int code = event.getKeyCode();
        int repeatCount = event.getRepeatCount();
        int metaState = event.getMetaState();
        int deviceId = event.getDeviceId();
        int scancode = event.getScanCode();
        int source = event.getSource();
        int flags = event.getFlags();
        if (source == InputDevice.SOURCE_UNKNOWN) {
            source = InputDevice.SOURCE_KEYBOARD;
        }
        if (eventTime == 0) {
            eventTime = SystemClock.uptimeMillis();
        }
        if (downTime == 0) {
            downTime = eventTime;
        }
        KeyEvent newEvent = new KeyEvent(downTime, eventTime, action, code, repeatCount, metaState,
                deviceId, scancode, flags | KeyEvent.FLAG_FROM_SYSTEM, source);
        InputManager.getInstance().injectInputEvent(newEvent,
                InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
        
    }
    
    MockANRActivity mActivity;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        solo = new Solo(getInstrumentation(), getActivity());
        mActivity = getActivity();
    }

    @Override
    protected void tearDown() throws Exception {
        //Robotium will finish all the activities that have been opened
        solo.finishOpenedActivities();
        super.tearDown();
    }

    public void testWNR() throws Exception {
        getInstrumentation().waitForIdleSync();        
        
        solo.clickOnText("OK");
        
        // first key event, but not wait for finish
        sendKey(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK));
        // second key event to trigger ANR
        sendKey(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_BACK));
    }

    private void sendMotionEvent(int x, int y, int action)
    {
        long downTime = SystemClock.uptimeMillis();
        long eventTime = SystemClock.uptimeMillis();
        
        MotionEvent event = MotionEvent.obtain(downTime, eventTime, action, x, y, 0);
        getInstrumentation().sendPointerSync(event);
    }
    
    public void testBroadcastANR() throws Exception {
        getInstrumentation().waitForIdleSync();        

        Intent intent = new Intent();

        intent.setAction("android.content.cts.ANR.BROADCAST_MOCKTEST");
        getInstrumentation().getContext().sendBroadcast(intent);

        solo.sleep(80 * 1000);
        
        // send event to another process will cause exception

        // Display display = mActivity.getWindowManager().getDefaultDisplay();
        // Point size = new Point();
        // display.getSize(size);

        // for 480 x 800 screen
        // int y = 540;
        // int x = 150;

        // sendMotionEvent(x, y, MotionEvent.ACTION_DOWN);
        // sendMotionEvent(x, y, MotionEvent.ACTION_UP);
        
        // solo.sleep(1000);        

        // sendMotionEvent(x, y, MotionEvent.ACTION_DOWN);
        // sendMotionEvent(x, y, MotionEvent.ACTION_UP);
    
    }

}
