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

import android.content.pm.PackageManager;
import android.test.AndroidTestCase;
import java.io.IOException;


/**
 * This test is based on the declarations in AndroidManifest.xml. We create mock declarations
 * in AndroidManifest.xml just for test of PackageManager, and there are no corresponding parts
 * of these declarations in test project.
 */
public class PackageManagerDumpsysTest extends AndroidTestCase {

    public void test_dumpsys_package_h() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "-h").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    /*
    public void test_dumpsys_package_android() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "android").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    */

    public void test_dumpsys_package_l() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "l").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_f() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "f").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    
    /*
    public void test_dumpsys_package_perm() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "perm").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    */
        
    public void test_dumpsys_package_preferred_xml() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "preferred-xml").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    /*
    public void test_dumpsys_package_s() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "s").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_prov() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "prov").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    */

    public void test_dumpsys_package_m() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "m").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_v() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "v").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_a_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "a", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_se_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "se", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_pr_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "pr", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_up_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "up", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_in_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "in", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    public void test_dumpsys_package_log_pe_on() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "log", "pe", "on").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }

    /*
    public void test_dumpsys_package_r() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "r").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    */
    
    public void test_dumpsys_package_pref() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "pref").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    
    /*
    public void test_dumpsys_package_p() {
        try {
            java.lang.Process dumpsys = new ProcessBuilder("/system/bin/dumpsys", "package", "p").redirectErrorStream(true).start();
        } catch (IOException e) {
            fail();
        }
    }
    */
}
