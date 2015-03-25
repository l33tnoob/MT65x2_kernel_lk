/*
 * Copyright (C) 2008 The Android Open Source Project
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
 */

package mediatek.app.cts;

import java.io.IOException;
import java.io.InputStreamReader;


public class ActivityManagerLogTest extends ActivityTestsBase {

    final String ERROR_STRING1 = "  Invalid argument!";
    final String ERROR_STRING2 = "Bad activity command, or no activities match: log";
    StringBuilder sb = new StringBuilder(1024);
    InputStreamReader input = null;
    char[] buf = new char[1024];
    int num = 0;

    public void testAmsLog() {

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "a", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "da", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "br", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "s", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "cp", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "p", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "m", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "s", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "x", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        //test case for wrong arguments "ggg"
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log", "ggg", "on").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(!sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

        //test case for no arguments
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys",
                "activity", "log").redirectErrorStream(true).start();

            input = new InputStreamReader(dumpsys.getInputStream());

            while ((num = input.read(buf)) > 0) {
                sb.append(buf, 0, num);
                assertTrue(!sb.toString().equals(ERROR_STRING1) || sb.toString().equals(ERROR_STRING2));
            }
        } catch (IOException e) {
            assertTrue(true);
        }

    }
}
