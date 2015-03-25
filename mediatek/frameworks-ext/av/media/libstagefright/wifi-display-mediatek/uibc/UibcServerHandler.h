
#ifndef UIBC_SERVER_HANDLER_H
#define UIBC_SERVER_HANDLER_H

#include "UibcHandler.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>

#include "WifiDisplayUibcType.h"
#include <linux/input.h>


#define UIBC_KBD_DEV_PATH	"/dev/uibc"

namespace android {

enum display_mode {
    DISPLAY_MODE_PORTRAIT = 0,
    DISPLAY_MODE_LANDSCAPE = 1,
};

struct IRemoteDisplayClient;

struct UibcServerHandler : public RefBase, public UibcHandler {
    UibcServerHandler();

    status_t init();
    status_t destroy();
    status_t handleUIBCMessage(const sp<ABuffer> &buffer);

    status_t simulateKeyEvent();
    status_t simulateMouseEvent();

protected:
    virtual ~UibcServerHandler();
    void initIoPeripheral();

private:

    int mUibc_kbd_fd;
    int m_XCurCoord;
    int m_YCurCoord;
    bool m_KBPluged;
    bool m_MousePluged;

    int m_XOffset;
    int m_YOffset;
    bool m_XRevert;
    bool m_YRevert;
    bool m_XYSwitch;
    uint8_t m_Orientation;

    bool mShiftPressed;
    bool m_touchDown;
    bool m_mouseDown;
    int m_deltaX;
    int m_deltaY;

    bool m_touchSupported;
    bool m_mouseSupported;
    bool m_mouseCursorSupported;

    void updateScreenMode();
    bool transTouchToSourcePosition(short* x, short* y);

    status_t handleGenericInput(const sp<ABuffer> &buffer);
    status_t handleHIDCInput(const sp<ABuffer> &buffer);

    status_t sendKeyEvent(UINT16 code, int isPress);
    status_t sendMultipleTouchEvent(WFD_UIBC_GENERIC_BODY_MOUSE_TOUCH *pBody);

    DISALLOW_EVIL_CONSTRUCTORS(UibcServerHandler);
};

}

#endif
