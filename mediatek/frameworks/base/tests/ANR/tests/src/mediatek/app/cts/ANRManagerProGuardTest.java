/* Build MediaTekCtsAnrTestCases.apk & MediatekAnrTestStubs.apk
 * ./mk mt6582_phone_qhd mm mediatek/frameworks/base/tests/ANR/tests/
 * ./mk mt6582_phone_qhd mm mediatek/frameworks/base/tests/ANR/
 * Execute
 * adb shell am instrument -w -e class mediatek.app.cts.ANRManagerProGuardTest com.mediatek.cts.anr/android.test.InstrumentationCtsTestRunner
 */

package mediatek.app.cts;

import android.app.Instrumentation;
import android.test.AndroidTestCase;
import android.util.Log;
import java.lang.reflect.Method;
import junit.framework.Assert;

//Target Package ANRManager
import android.app.ActivityManager;
import android.app.Activity;
import android.content.Context;
import com.android.server.am.ANRManager;

public class ANRManagerProGuardTest extends AndroidTestCase {
    private static final String TAG = "ANRManagerProGuardTest";
    private boolean ANRManagerObjNameObfuscated = true;
    private boolean ANRManagerAMethodName = false;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.i(TAG, " Setup ProGuard Test Case ");

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Log.i(TAG, " TearDown ProGuard Test Case ");
    }

  public void testANRManagerProGuard() throws Throwable {

      Log.i(TAG, " testANRManagerProGuard ");

      ANRManager checkClass = new ANRManager(null);
      Method[] methods = checkClass.getClass().getDeclaredMethods();

      for ( Method checkMethodName : methods) {
          Log.i(TAG,"ANRManager checkMethodName = " + checkMethodName.getName());
          if (ANRManagerObjNameObfuscated && checkMethodName.getName().equals("preDumpStackTraces")
            && (checkMethodName.getParameterTypes().length==2)){
            ANRManagerObjNameObfuscated = false;
          }
          if (!ANRManagerAMethodName && checkMethodName.getName().equals("a")){
            ANRManagerAMethodName = true;
          }
      }

      Log.i(TAG,"ANRManager checkMethodName = preDumpStackTraces is obfusted : " + ANRManagerObjNameObfuscated);
      Log.i(TAG,"ANRManager checkMethodName = a is                           : " + ANRManagerAMethodName);
      assertTrue("ANRManager object name is not obfuscated",(ANRManagerObjNameObfuscated & ANRManagerAMethodName));

  }

}

