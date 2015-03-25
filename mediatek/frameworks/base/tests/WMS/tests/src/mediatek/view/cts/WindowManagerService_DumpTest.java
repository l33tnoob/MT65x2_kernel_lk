/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.mediatek.cts.window;

import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;
import dalvik.annotation.ToBeFixed;

import android.test.AndroidTestCase;
import android.util.Slog;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;


public class WindowManagerService_DumpTest extends AndroidTestCase {

    static final String DUMPSYS = "/system/bin/dumpsys";

    static final String ERROR_MESSAGE_1 = "Bad window command";
    static final String ERROR_MESSAGE_2 = "Unknown argument:";
    static final String ERROR_MESSAGE_3 = "use -h for help";
    static final String ERROR_MESSAGE_4 = "Window manager dump options:";

    static final int MAX_DEBUG_ZONE_ID = 33;
    static final int DEBUG_WINDOW_ZONE_ID = 33;


    /**
     * Check the given process output whatever contains any error message.
     */
    private boolean checkHasError(Process dumpsys) throws IOException {
        BufferedReader reader =
                new BufferedReader(
                        new InputStreamReader(dumpsys.getInputStream()));
        String strLine;
        while ((strLine = reader.readLine()) != null) {
            if (strLine.contains(ERROR_MESSAGE_1) ||
                        strLine.contains(ERROR_MESSAGE_2) ||
                                strLine.contains(ERROR_MESSAGE_3) ||
                                        strLine.contains(ERROR_MESSAGE_4)) {
                return true;
            }
        }

        return false;
    }

    public void testOptionA() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-a").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testOptionH() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-h").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testErrorArgs() {
        // Error: wrong option
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-X").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }

        // Error: wrong command
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "abcdefg").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testLastAnr() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "lastanr").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testPolicy() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "policy").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testSessions() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "sessions").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testTokens() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "tokens").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testWindows() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "windows").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testAll() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "all").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testVisible() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "visible").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testStatus() {
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "Status").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testDebugErrorArgs() {
        // Error: no argument
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }

        // Error: wrong argumenet
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "asdfg").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }

        // Error: no debug type
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "enable").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }

        // Error: no debug type
        try {
            Process dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "disable").redirectErrorStream(true).start();
            assertTrue(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }

    public void testDebug() {
        try {
            Process dumpsys;

            // Enable all debug flags
            int i;
            for (i = 0; i <= MAX_DEBUG_ZONE_ID; i++) {
                dumpsys = new ProcessBuilder(
                        DUMPSYS,
                        "window",
                        "-d",
                        "enable",
                        Integer.toString(i)).redirectErrorStream(true).start();
                assertFalse(checkHasError(dumpsys));
            }

            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "list",
                    "window").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // Restore all debug flags
            for (i = 0; i <= MAX_DEBUG_ZONE_ID; i++) {
                dumpsys = new ProcessBuilder(
                        DUMPSYS,
                        "window",
                        "-d",
                        "disable",
                        Integer.toString(i)).redirectErrorStream(true).start();
                assertFalse(checkHasError(dumpsys));
            }
        } catch (IOException e) {
            fail();
        }
    }

    public void testDebugEnableWindow() {
        try {
            Process dumpsys;

            // Enable
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "enable",
                    Integer.toString(DEBUG_WINDOW_ZONE_ID),
                    "Status").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // Add an unused window name
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "enable",
                    Integer.toString(DEBUG_WINDOW_ZONE_ID),
                    "Test").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // List
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "list",
                    "window").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // Add exist window name
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "enable",
                    Integer.toString(DEBUG_WINDOW_ZONE_ID),
                    "Unused1",
                    "Status",
                    "Unused2").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // List again
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "list",
                    "window").redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));

            // Restore
            dumpsys = new ProcessBuilder(
                    DUMPSYS,
                    "window",
                    "-d",
                    "disable",
                    Integer.toString(DEBUG_WINDOW_ZONE_ID)).redirectErrorStream(true).start();
            assertFalse(checkHasError(dumpsys));
        } catch (IOException e) {
            fail();
        }
    }
}
