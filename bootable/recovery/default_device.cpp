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

#include <linux/input.h>

#include "common.h"
#include "device.h"
#include "screen_ui.h"
#if 1 //wschen 2012-07-10
#include "cust_keys.h"
#endif

static const char* HEADERS[] = { "Volume up/down to move highlight;",
                                 "enter button to select.",
                                 "",
                                 NULL };

#if 0 //wschen 2012-07-10
static const char* ITEMS[] =  {"reboot system now",
                               "apply update from ADB",
                               "wipe data/factory reset",
                               "wipe cache partition",
                               NULL };
#else
static const char* ITEMS[] =  {"reboot system now",
                               "apply update from ADB",
                               "apply update from sdcard",
#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
                               "apply update from sdcard2",
#endif //SUPPORT_SDCARD2
                               "apply update from cache",
                               "wipe data/factory reset",
                               "wipe cache partition",
#ifdef SUPPORT_DATA_BACKUP_RESTORE //wschen 2011-03-09 
                               "backup user data",
                               "restore user data",
#endif
                               NULL };

static const char* FORCE_ITEMS[] =  {"reboot system now",
                                     "apply sdcard:update.zip",
                                     NULL };
#endif

class DefaultUI : public ScreenRecoveryUI {
  public:
    virtual KeyAction CheckKey(int key) {
#if 0 //wschen 2012-07-10
        if (key == KEY_HOME) {
            return TOGGLE;
        }
#else
        if (key == RECOVERY_KEY_MENU) {
            return TOGGLE;
        }
#endif
        return ENQUEUE;
    }
};

class DefaultDevice : public Device {
  public:
    DefaultDevice() :
        ui(new DefaultUI) {
    }

    RecoveryUI* GetUI() { return ui; }

    int HandleMenuKey(int key, int visible) {
        if (visible) {
            switch (key) {
#if 0 //wschen 2012-07-10
              case KEY_DOWN:
              case KEY_VOLUMEDOWN:
                return kHighlightDown;

              case KEY_UP:
              case KEY_VOLUMEUP:
                return kHighlightUp;

              case KEY_ENTER:
                return kInvokeItem;
#else
              case RECOVERY_KEY_DOWN:
                return kHighlightDown;

#if (RECOVERY_KEY_UP != RECOVERY_KEY_DOWN)
              case RECOVERY_KEY_UP:
                return kHighlightUp;
#endif

              case RECOVERY_KEY_ENTER:
                return kInvokeItem;
#endif
            }
        }

        return kNoAction;
    }

    BuiltinAction InvokeMenuItem(int menu_position) {
        switch (menu_position) {
#if 0 //wschen 2012-07-10
          case 0: return REBOOT;
          case 1: return APPLY_ADB_SIDELOAD;
          case 2: return WIPE_DATA;
          case 3: return WIPE_CACHE;
          default: return NO_ACTION;
#else
#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
          case 0: return REBOOT;
          case 1: return APPLY_ADB_SIDELOAD;
          case 2: return APPLY_EXT;
          case 3: return APPLY_SDCARD2;
          case 4: return APPLY_CACHE;
          case 5: return WIPE_DATA;
          case 6: return WIPE_CACHE;
          case 7: return USER_DATA_BACKUP;
          case 8: return USER_DATA_RESTORE;
          default: return NO_ACTION;
#else
          case 0: return REBOOT;
          case 1: return APPLY_ADB_SIDELOAD;
          case 2: return APPLY_EXT;
          case 3: return APPLY_CACHE;
          case 4: return WIPE_DATA;
          case 5: return WIPE_CACHE;
          case 6: return USER_DATA_BACKUP;
          case 7: return USER_DATA_RESTORE;
          default: return NO_ACTION;
#endif //SUPPORT_SDCARD2
#endif
        }
    }

#if 1 //wschen 2012-07-10
    BuiltinAction InvokeForceMenuItem(int menu_position) {
        switch (menu_position) {
          case 0: return REBOOT;
          case 1: return FORCE_APPLY_SDCARD_SIDELOAD;
#if defined(SUPPORT_SDCARD2) && !defined(MTK_SHARED_SDCARD) //wschen 2012-11-15
          case 2: return FORCE_APPLY_SDCARD2_SIDELOAD;
#endif //SUPPORT_SDCARD2
          default: return NO_ACTION;
        }
    }
#endif

    const char* const* GetMenuHeaders() { return HEADERS; }
    const char* const* GetMenuItems() { return ITEMS; }
    const char* const* GetForceMenuItems() { return FORCE_ITEMS; }

  private:
    RecoveryUI* ui;
};

Device* make_device() {
    return new DefaultDevice();
}
