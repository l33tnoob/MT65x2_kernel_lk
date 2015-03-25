

#define LOG_TAG "UibcServerHandler"
#include <utils/Log.h>

#include "UibcMessage.h"
#include "UibcServerHandler.h"
#include "WifiDisplayUibcType.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/hexdump.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <linux/input.h>

#include <binder/IServiceManager.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>
#include <gui/ISurfaceComposer.h>
#include <cutils/properties.h>

namespace android {

#define BTN_TOUCH   0x14a

#define WFD_UIBC_HIDC_USAGE_REPORT_INPUT        0x00
#define WFD_UIBC_HIDC_USAGE_REPORT_DESCRIPTOR   0x01

#define HID_LEFTSHIFT_BIT                       0x02
#define HID_RIGHTSHIFT_BIT                      0x20

#define UIBC_KBD_NAME  "uibc"
#define UIBC_KEY_PRESS      1
#define UIBC_KEY_RELEASE    0
#define UIBC_KEY_RESERVE	2
#define UIBC_POINTER_X	    3
#define UIBC_POINTER_Y	    4
#define UIBC_KEYBOARD	    5
#define UIBC_MOUSE	        6
#define UIBC_TOUCH_DOWN		7
#define UIBC_TOUCH_UP		8
#define UIBC_TOUCH_MOVE		9

#define MAX_POINTERS 5

UibcServerHandler::UibcServerHandler()
    : mUibc_kbd_fd(-1),
      m_XCurCoord(-1),
      m_YCurCoord(-1),
      mShiftPressed(false),
      m_touchDown(false),
      m_mouseDown(false),
      m_deltaX(0),
      m_deltaY(0),
      m_XOffset(0),
      m_YOffset(0),
      m_XRevert(false),
      m_YRevert(false),
      m_XYSwitch(false),
      m_mouseCursorSupported(false),
      m_Orientation(DISPLAY_ORIENTATION_0),
      m_KBPluged(false),
      m_MousePluged(false) {
}

UibcServerHandler::~UibcServerHandler() {

}

status_t UibcServerHandler::init() {
    int version;

    UibcHandler::init();

    char mouseCursor[PROPERTY_VALUE_MAX];
    if (property_get("media.wfd.uibc-mouse-cursor", mouseCursor, NULL)) {
        int value = atoi(mouseCursor);
        if(value == 1) {
            ALOGI("media.wfd.uibc-mouse-cursor:%s", mouseCursor);
            m_mouseCursorSupported = true;
        }
    }

    if(mUibc_kbd_fd >= 0) {
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
    }
    mUibc_kbd_fd = open(UIBC_KBD_DEV_PATH, O_WRONLY);
    if(mUibc_kbd_fd < 0) {
        ALOGE("sendKeyEvent error");
        return -1;
    }
    m_KBPluged = false;
    m_MousePluged = false;

    initIoPeripheral();

    return OK;
}

status_t UibcServerHandler::destroy() {

    if(mUibc_kbd_fd >= 0) {
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
    }
    return OK;
}

void UibcServerHandler::initIoPeripheral() {
    ALOGI("initIoPeripheral");
    bool m_touchSupported = !!(mRemote_InpType & GENERIC_SINGLETOUCH);
    bool m_mouseSupported = !!(mRemote_InpType & GENERIC_MOUSE);

    if ((mRemote_InpType & GENERIC_KEYBOARD) != UIBC_NONE) {
        if (!m_KBPluged) {
            if(ioctl(mUibc_kbd_fd, UIBC_KEYBOARD, 0) < 0) {
                ALOGE("sendKeyEvent Fail uibc ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return;
            }
            m_KBPluged = true;
        }
    } else if (m_touchSupported || m_mouseSupported) {
        if (!m_MousePluged) {
            if(ioctl(mUibc_kbd_fd, UIBC_MOUSE, 0) < 0) {
                ALOGE("sendMouseEvent Fail uibc ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return;
            }
            m_MousePluged = true;
        }
    }
}

status_t UibcServerHandler::handleUIBCMessage(const sp<ABuffer> &buffer) {
    //ALOGD("handleUIBCMessage mUibcEnabled=0x%04X, mRemote_InputCat=0x%04X, mRemote_InpType=0x%04X, ",
    //mUibcEnabled, mRemote_InputCat, mRemote_InpType);

    if (mUibcEnabled == UIBC_DISABLED)
        return OK;

    size_t size = buffer->size();
    size_t payloadOffset = 0;
    UIBCInputCategoryCode  inputCategory = WFD_UIBC_INPUT_CATEGORY_UNKNOWN;

    if(size < UIBC_HEADER_SIZE) {
        ALOGE("The size of UIBC message is less than header size");
        return ERROR_MALFORMED;
    }

    const uint8_t *data = buffer->data();

#if TEST_COMMAND
#if 0
    ALOGD("in with");
    hexdump(data, size);
#endif


    int testCmd = data[0];
    ALOGD("testCmd:%d", testCmd);

    if(testCmd >= 0x80) { //Enter test command
        switch(testCmd) {
        case 0x80: {
            for(int j = 0; j < 1; j++) {
                sendKeyEvent(30 + j, 1);
                //sendKeyEvent(30+j, 0);
            }
            break;
        }
        case 0x81: {
            int testCode = data[1];
            sendKeyEvent(testCode, 1);
            sendKeyEvent(testCode, 0);
            break;
        }
        case 0x82: {
            //sendTouchEvent(0, 320, 320, WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN);
            //sendTouchEvent(0, 340, 320, WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE);
            //sendTouchEvent(0, 360, 320, WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE);
            //sendTouchEvent(0, 600, 320, WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP);
        }
        break;
        }

        return OK;
    }

    //Check UIBC version
#if 0
    if ((data[0] >> 6) != 0) {
        // Unsupported version.
        return ERROR_UNSUPPORTED;
    }
#endif
#endif
    //Skip the timestamp
    bool hasTimeStamp = data[0] & UIBC_TIMESTAMP_MASK;
    if(hasTimeStamp) {
        payloadOffset = UIBC_HEADER_SIZE + UIBC_TIMESTAMP_SIZE;
    } else {
        payloadOffset = UIBC_HEADER_SIZE;
    }

    if (size < payloadOffset) {
        // Not enough data to fit the basic header
        ALOGE("Not enough data to fit the basic header");
        return ERROR_MALFORMED;
    }

    buffer->setRange(payloadOffset, size - payloadOffset);

    inputCategory = (UIBCInputCategoryCode) (data[1] & UIBC_INPUT_CATEGORY_MASK);

    switch(inputCategory) {
    case WFD_UIBC_INPUT_CATEGORY_GENERIC:
        if ((mRemote_InputCat & INPUT_CAT_GENERIC) == UIBC_NONE) {
            ALOGD("INPUT_CAT_GENERIC not supported.");
            return OK;
        }

        handleGenericInput(buffer);
        break;
    case WFD_UIBC_INPUT_CATEGORY_HIDC:
        if ((mRemote_InputCat & INPUT_CAT_HIDC) == UIBC_NONE) {
            ALOGD("INPUT_CAT_HIDC not supported.");
            return OK;
        }

        //handleHIDCInput(buffer);
        break;
    default:
        ALOGE("Uknown input category:%d", inputCategory);
        break;
    }

    return OK;
}

status_t UibcServerHandler::handleGenericInput(const sp<ABuffer> &buffer) {
    size_t size = buffer->size();
    WFD_UIBC_GENERIC_BODY_FORMAT_HDR *pHdr = (WFD_UIBC_GENERIC_BODY_FORMAT_HDR*) buffer->data();
    UINT16 bodyLength = ntohs(pHdr->length);

    if (size < bodyLength) {
        ALOGE("Error: not enough space for a complete generic body:%d", bodyLength);
        return ERROR;
    }

    //ALOGV("handleGenericInput with IE:%d", pHdr->ieID);

    switch (pHdr->ieID) {
    case WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN:
    case WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP:
    case WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE: {
        WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *pBody = (WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH*)pHdr;
        ALOGI("uibc (Move,Down,Up) ieID: %d, numptr: %d",
              pBody->ieID,
              pBody->numPointers);

        if ((pBody->numPointers > 1) &&
            ((mRemote_InpType & GENERIC_MULTITOUCH) == UIBC_NONE)) {
            ALOGD("GENERIC_MULTITOUCH not supported.");
            return OK;
        }
        sendMultipleTouchEvent(pBody);
        break;
    }
    case WFD_UIBC_GENERIC_IE_ID_KEY_DOWN:
    case WFD_UIBC_GENERIC_IE_ID_KEY_UP: {
        if ((mRemote_InpType & GENERIC_KEYBOARD) == UIBC_NONE)
            return OK;

        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_KEY)) {
            WFD_UIBC_GENERIC_BODY_KEY *pBody = (WFD_UIBC_GENERIC_BODY_KEY*)pHdr;
            ALOGD("uibc (Key,Down,Up) ieID: %d, code1: %d, code2: %d",
                  pBody->ieID,
                  ntohs(pBody->code1),
                  ntohs(pBody->code2));
            int isDown =   ( (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_KEY_DOWN) ? 1 : 0  );
            if( ntohs(pBody->code1) > 0) {
                sendKeyEvent(ntohs(pBody->code1), isDown);
            }

            if(ntohs(pBody->code2) > 0) {
                sendKeyEvent(ntohs(pBody->code2), isDown);
            }
        }
        break;
    }
    case WFD_UIBC_GENERIC_IE_ID_ZOOM:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_ZOOM)) {
            WFD_UIBC_GENERIC_BODY_ZOOM *pBody = (WFD_UIBC_GENERIC_BODY_ZOOM*)pHdr;
            ALOGD("uibc (ZOOM) ieID: %d, x: %d, y: %d, itimes: %d, ftimes: %d",
                  pBody->ieID,
                  pBody->x,
                  pBody->y,
                  pBody->intTimes,
                  pBody->fractTimes);
        }
        break;
    case WFD_UIBC_GENERIC_IE_ID_VSCROLL:
    case WFD_UIBC_GENERIC_IE_ID_HSCROLL:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_SCROLL)) {
            WFD_UIBC_GENERIC_BODY_SCROLL *pBody = (WFD_UIBC_GENERIC_BODY_SCROLL*)pHdr;
            ALOGD("uibc (SCROLL V/H) ieID: %d, amount: %d",
                  pBody->ieID,
                  pBody->amount);
        }
        break;
    case WFD_UIBC_GENERIC_IE_ID_ROTATE:
        if((sizeof(WFD_UIBC_GENERIC_BODY_FORMAT_HDR) + bodyLength)
            == sizeof(WFD_UIBC_GENERIC_BODY_ROTATE)) {
            WFD_UIBC_GENERIC_BODY_ROTATE *pBody = (WFD_UIBC_GENERIC_BODY_ROTATE*)pHdr;
            ALOGD("uibc (ROTATE V/H) ieID: %d, iamount: %d, famount: %d",
                  pBody->ieID,
                  pBody->intAmount,
                  pBody->fractAmount);
            break;
        }
    default:
        ALOGE("Unknown User input for generic type");
        break;
    }

    return OK;
}

status_t UibcServerHandler::handleHIDCInput(const sp<ABuffer> &buffer) {
    size_t bufferSize = buffer->size();
    size_t payloadOffset = 0;

    if (bufferSize < sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR)) {
        ALOGE("Error: not enough space for a complete HIDC header:%d", bufferSize);
        return ERROR;
    }

    WFD_UIBC_HIDC_BODY_FORMAT_HDR *pHdr = (WFD_UIBC_HIDC_BODY_FORMAT_HDR*) buffer->data();
    UINT16 bodyLength = ntohs(pHdr->length);

    if (bufferSize < bodyLength + sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR)) {
        ALOGE("Error: not enough space for a complete HIDC body:%d", bufferSize);
        return ERROR;
    }

    ALOGI("handleHIDCInput with Info:(%d:%d:%d)", pHdr->inputPath, pHdr->hidType, pHdr->usage);

    payloadOffset = sizeof(WFD_UIBC_HIDC_BODY_FORMAT_HDR);
    buffer->setRange(payloadOffset, bufferSize - payloadOffset); // Move the buffer to the HIDC body part
    uint8_t* pBuffer = buffer->data();

    switch (pHdr->usage) {
    case WFD_UIBC_HIDC_USAGE_REPORT_INPUT: {
        if (pHdr->inputPath == WFD_UIBC_HIDC_IE_ID_USB) {
            switch(pHdr->hidType) {
            case WFD_UIBC_HIDCTYPE_IE_ID_KEYBOARD:
                if(bodyLength == 8) {
                    if(pBuffer[2] != 0) {
                        //Check the shift key
                        if((pBuffer[0] & HID_LEFTSHIFT_BIT) || (pBuffer[0] & HID_RIGHTSHIFT_BIT)) {
                            //sendKeyEvent(AKEYCODE_SHIFT_LEFT, 1);
                            //sendKeyEvent(AKEYCODE_SHIFT_LEFT, 0);
                        }
                        sendKeyEvent(pBuffer[2], 1);
                    } else {
                        sendKeyEvent(pBuffer[2], 0);
                    }
                }
                break;
            case WFD_UIBC_HIDCTYPE_IE_ID_MOUSE:
                if(bodyLength >= 3) {
                    int xOffs, yOffs;
                    xOffs = (char) pBuffer[1];
                    yOffs = (char) pBuffer[2];
                    if (xOffs >= 0x80) xOffs -= 0x80;
                    if (yOffs >= 0x80) yOffs -= 0x80;
                    //sendTouchEvent(0, xOffs, yOffs, WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN);
                    //sendTouchEvent(0, xOffs, yOffs, WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE);
                    //sendTouchEvent(0, xOffs, yOffs, WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP);
                }
                break;
            }
        }
        break;
    }
    case WFD_UIBC_HIDC_USAGE_REPORT_DESCRIPTOR: {

        break;
    }
    }

    return OK;
}

status_t UibcServerHandler::sendKeyEvent(UINT16 code, int isPress) {
    scanCodeBuild_t scanCodeBuild = UibcMessage::asciiToScancodeBuild((char)code);
    uint16_t scenCodeShift = KEY_LEFTSHIFT;
    uint16_t scenCodeAlt = KEY_LEFTALT;

// Temporary disable special character support
#if 0
    if (code == KEY_UNKNOWN) {
        ALOGD("sendKeyEvent:KEY_UNKNOWN");
        char cmd[20];
        sprintf(cmd, "input text\"%c\"", code);
        ALOGD("sendKeyEvent:run_consol_cmd %s", cmd);
        run_consol_cmd(cmd);
        return OK;
    }
#endif

    //ALOGD("sendKeyEvent: Code:%d, keydown:%d, page2Char:%d, ShiftPressed:%d, ",
    //      scanCodeBuild.scanCode,
    //      isPress,
    //      scanCodeBuild.shift_pressed,
    //      mShiftPressed);

    // Latin support
    if (isPress == 1 && code >= 0xA0) {
        if (scanCodeBuild.latinComposeKey != 0x00 ) {
            // Send latin prefix
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeAlt) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scanCodeBuild.latinComposeKey) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scanCodeBuild.latinComposeKey) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeAlt) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
        }
    }




    // Send the target ascii character
    if (isPress) {
        // Control the shift status before send the char scancode
        if (scanCodeBuild.shift_pressed && !mShiftPressed) {
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeShift) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
        }
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scanCodeBuild.scanCode) < 0) {
            ALOGE("sendKeyEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        // release the shift status after send the char scancode
        if (scanCodeBuild.shift_pressed && !mShiftPressed) {
            if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeShift) < 0) {
                ALOGE("sendKeyEvent Fail hid ioctl");
                close(mUibc_kbd_fd);
                mUibc_kbd_fd = -1;
                return -1;
            }
        }
    } else {
        if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scanCodeBuild.scanCode) < 0) {
            ALOGE("sendKeyEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }



    // Save the input device shift status
    if (scanCodeBuild.scanCode == KEY_LEFTSHIFT ||
        scanCodeBuild.scanCode == KEY_RIGHTSHIFT)
        mShiftPressed = (isPress > 0 ? true : false);

    return OK;
}

void UibcServerHandler::updateScreenMode() {
    display_mode localDisplayMode = DISPLAY_MODE_PORTRAIT;
    display_mode remoteDisplayMode = DISPLAY_MODE_PORTRAIT;

    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
                              ISurfaceComposer::eDisplayIdMain);

    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);
    m_Orientation = info.orientation;

    //ALOGD("updateScreenMode: info.orientation:%d info.w:%d info.h:%d", info.orientation, info.w, info.h);

    if (info.w > info.h)
        localDisplayMode = DISPLAY_MODE_LANDSCAPE;

    if (m_wfdWidth > m_wfdHeight)
        remoteDisplayMode = DISPLAY_MODE_LANDSCAPE;

    if (info.orientation == DISPLAY_ORIENTATION_90 ||
        info.orientation == DISPLAY_ORIENTATION_270) {
        localDisplayMode = (localDisplayMode == DISPLAY_MODE_LANDSCAPE ?
                            DISPLAY_MODE_PORTRAIT : DISPLAY_MODE_LANDSCAPE);
        m_localWidth = info.h;
        m_localHeight = info.w;
    } else {
        m_localWidth = info.w;
        m_localHeight = info.h;
    }

    m_widthRatio = (double)m_localWidth / (double)m_wfdWidth;
    m_heightRatio = (double)m_localHeight / (double)m_wfdHeight;
    m_XOffset = 0;
    m_YOffset = 0;

    if (localDisplayMode != remoteDisplayMode) {
        if (localDisplayMode == DISPLAY_MODE_LANDSCAPE) {
            m_heightRatio = m_widthRatio;
            m_YOffset = ((m_wfdHeight - (int)((float)m_localHeight / m_heightRatio)) / 2);
        } else if (localDisplayMode == DISPLAY_MODE_PORTRAIT) {
            m_widthRatio = m_heightRatio;
            m_XOffset = ((m_wfdWidth - (int)((float)m_localWidth / m_widthRatio)) / 2);
        }
    }

    switch (info.orientation) {
    case DISPLAY_ORIENTATION_0:
        m_XYSwitch = false;
        m_XRevert = false;
        m_YRevert = false;
        break;
    case DISPLAY_ORIENTATION_90:
        m_XYSwitch = true;
        m_XRevert = true;
        m_YRevert = false;
        break;
    case DISPLAY_ORIENTATION_180:
        m_XYSwitch = false;
        m_XRevert = true;
        m_YRevert = true;
        break;
    case DISPLAY_ORIENTATION_270:
        m_XYSwitch = true;
        m_XRevert = false;
        m_YRevert = true;
        break;
    default:
        break;
    }
    ALOGD("uibc screen mode: localDisplayMode:%d, remoteDisplayMode:%d" \
          "m_localWidth:%d, m_localHeight:%d" \
          "m_wfdWidth:%d, m_wfdHeight:%d" \
          "m_widthRatio:%f, m_heightRatio:%f" \
          "m_XOffset:%d, m_YOffset:%d",
          localDisplayMode, remoteDisplayMode,
          m_localWidth, m_localHeight,
          m_wfdWidth, m_wfdHeight,
          m_widthRatio, m_heightRatio,
          m_XOffset, m_YOffset);
}

bool UibcServerHandler::transTouchToSourcePosition(short* x, short* y) {
    ALOGD("uibc XY trans+: x:%d, y:%d", *x, *y);
    short tmp;
    // in the black part
    if ((m_XOffset > 0) &&
        (*x < m_XOffset ||
         *x > (m_wfdWidth - m_XOffset))) {
        return false;
    }
    // in the black part
    if ((m_YOffset > 0) &&
        (*y < m_YOffset ||
         *y > (m_wfdHeight - m_YOffset))) {
        return false;
    }

    *x -= m_XOffset;
    *y -= m_YOffset;
    //ALOGD("transTouchToSourcePosition remove offset: * x:%d, * y:%d", *x, *y);

    *x *= m_widthRatio;
    *y *= m_heightRatio;
    //ALOGD("transTouchToSourcePosition map to source: * x:%d, * y:%d", *x, *y);

    if (m_XYSwitch) {
        tmp = *x;
        *x = *y;
        *y = tmp;
    }
    //ALOGD("transTouchToSourcePosition XY switch: * x:%d, * y:%d", *x, *y);

    if (m_XRevert) {
        if (m_XYSwitch)
            *x = m_localHeight - *x;
        else
            *x = m_localWidth - *x;
    }

    if (m_YRevert) {
        if (m_XYSwitch)
            *y = m_localWidth - *y;
        else
            *y = m_localHeight - *y;
    }
    ALOGD("uibc XY trans-: x:%d, y:%d", *x, *y);
    return true;
}

status_t UibcServerHandler::sendMultipleTouchEvent(WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *pBody) {
    //ALOGD("sendMultipleTouchEvent ieID: %d, numptr: %d, m_touchDown = %d",
    //      pBody->ieID,
    //      pBody->numPointers,
    //      m_touchDown);
    WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH* pHdr = (WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *)pBody;
    short touchPosition[16] = {0};
    int TouchPressed = 0;
    short deltaX = 0, deltaY = 0;
    short x, y;
    uint16_t scenCodeMouse = BTN_MOUSE;
    bool m_touchSupported = !!(mRemote_InpType & GENERIC_SINGLETOUCH);
    bool m_mouseSupported = !!(mRemote_InpType & GENERIC_MOUSE);

    if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_DOWN && !m_mouseDown && !m_touchDown) {
        touchPosition[0] = (short)pBody->numPointers;
        //ALOGD("TOUCH_DOWN: pointerID:%d x:%d y:%d", pBody->coordinates[0].pointerID,
        //ntohs(pBody->coordinates[0].x), ntohs(pBody->coordinates[0].y));
        x = ntohs(pBody->coordinates[0].x);
        y = ntohs(pBody->coordinates[0].y);

        if (m_XCurCoord == x && m_YCurCoord == y) {
            if (m_mouseCursorSupported) {
                m_mouseDown = true;
                if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeMouse) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            } else if (m_mouseSupported) {
                m_touchDown = true;
                updateScreenMode();
                if (!transTouchToSourcePosition(&x, &y))
                    return -1;
                touchPosition[1] = (short)pBody->coordinates[0].pointerID;
                touchPosition[2] = x;
                touchPosition[3] = y;

                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_DOWN, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        } else if (!(m_XCurCoord == x && m_YCurCoord == y)) {
            m_touchDown = true;
            if (m_touchSupported) {
                updateScreenMode();
                if (!transTouchToSourcePosition(&x, &y))
                    return -1;
                touchPosition[1] = (short)pBody->coordinates[0].pointerID;
                touchPosition[2] = x;
                touchPosition[3] = y;

                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_DOWN, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        }
    } else  if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_LMOUSE_TOUCH_UP) {
        if (m_mouseDown) {
            if (m_mouseCursorSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeMouse) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
            m_mouseDown = false;
        } else if (m_touchDown) {
            if (m_touchSupported || m_mouseSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_UP, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
            m_touchDown = false;
        }
    } else  if (pHdr->ieID == WFD_UIBC_GENERIC_IE_ID_MOUSE_TOUCH_MOVE)  {
        if (m_touchDown) {
            touchPosition[0] = (short)pBody->numPointers;
            for(int i = 0; i < pBody->numPointers && i < (short)pBody->numPointers; i++) {
                //ALOGD("TOUCH_MOVE %dth: pointerID:%d x:%d y:%d", i, pBody->coordinates[i].pointerID,
                //ntohs(pBody->coordinates[i].x), ntohs(pBody->coordinates[i].y));
                x = ntohs(pBody->coordinates[i].x);
                y = ntohs(pBody->coordinates[i].y);
                touchPosition[i * 3 + 1] = (short)pBody->coordinates[i].pointerID;
                touchPosition[i * 3 + 2] = x;
                touchPosition[i * 3 + 3] = y;
                if (!transTouchToSourcePosition(&touchPosition[i * 3 + 2], &touchPosition[i * 3 + 3]))
                    return -1;
            }
            if (m_touchSupported || m_mouseSupported) {
                if(ioctl(mUibc_kbd_fd, UIBC_TOUCH_MOVE, &touchPosition) < 0) {
                    ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                    close(mUibc_kbd_fd);
                    mUibc_kbd_fd = -1;
                    return -1;
                }
            }
        } else {
            if (pBody->numPointers == 1) {
                x = ntohs(pBody->coordinates[0].x);
                y = ntohs(pBody->coordinates[0].y);
                if (m_mouseCursorSupported) {
                    if (m_XCurCoord == -1 && m_YCurCoord == -1) {
                        if (m_Orientation == DISPLAY_ORIENTATION_90 ||
                            m_Orientation == DISPLAY_ORIENTATION_270) {
                            m_XCurCoord = m_wfdHeight / 2;
                            m_YCurCoord = m_wfdWidth / 2;
                        } else {
                            m_XCurCoord = m_wfdWidth / 2;
                            m_YCurCoord = m_wfdHeight / 2;
                        }
                    }
                    deltaX = (x - m_XCurCoord);
                    deltaY = (y - m_YCurCoord);
                    // Align the cursor to the boundary
                    if (deltaX == 0 && deltaY != 0) {
                        if (x < 3) {
                            deltaX = -1280;
                        } else  if ((m_wfdWidth - x) < 3 ) { // x == m_wfdWidth
                            deltaX = 1280;
                        }
                    }
                    if (deltaY == 0 && deltaX != 0) {
                        if (y  < 3) {
                            deltaY = -1280;
                        } else  if ((m_wfdHeight - y) < 3 ) { // x == m_wfdHeight
                            deltaY = 1280;
                        }
                    }
                    //ALOGD("TOUCH_MOVE x:%d y:%d deltaX:%d deltaY:%d", x, y, m_deltaX, m_deltaY);
                    if (m_mouseSupported) {
                        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &deltaX) < 0) {
                            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                            close(mUibc_kbd_fd);
                            mUibc_kbd_fd = -1;
                            return -1;
                        }
                        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &deltaY) < 0) {
                            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
                            close(mUibc_kbd_fd);
                            mUibc_kbd_fd = -1;
                            return -1;
                        }
                    }
                    m_deltaX = deltaX;
                    m_deltaY = deltaY;
                }
                m_XCurCoord = x;
                m_YCurCoord = y;
            }
        }
    }
    return OK;
}

status_t UibcServerHandler::simulateKeyEvent() {
    UINT16 aChar = 0x61;
    int i;

    for (i = 0; i < 26; i++) {
        sendKeyEvent(aChar, 1);
        sendKeyEvent(aChar, 0);
        aChar++;
    }

    return OK;
}

status_t UibcServerHandler::simulateMouseEvent() {
    int i;
    int temp;
    uint16_t scenCodeMouse = BTN_MOUSE;

    // Move to left bondary from right bondary
    for (i = 0; i < 500; i++) {
        temp = -1;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        temp = 0;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }

    if(ioctl(mUibc_kbd_fd, UIBC_KEY_PRESS, &scenCodeMouse) < 0) {
        ALOGE("sendKeyEvent Fail hid ioctl");
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
        return -1;
    }

    // Move to left bondary from right bondary
    for (i = 0; i < 1000; i++) {
        temp = 1;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_X, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
        temp = 0;
        if(ioctl(mUibc_kbd_fd, UIBC_POINTER_Y, &temp) < 0) {
            ALOGE("sendMultipleTouchEvent Fail hid ioctl");
            close(mUibc_kbd_fd);
            mUibc_kbd_fd = -1;
            return -1;
        }
    }


    if(ioctl(mUibc_kbd_fd, UIBC_KEY_RELEASE, &scenCodeMouse) < 0) {
        ALOGE("sendKeyEvent Fail hid ioctl");
        close(mUibc_kbd_fd);
        mUibc_kbd_fd = -1;
        return -1;
    }


    return OK;
}
}
