
#ifndef UIBC_MESSAGE_H
#define UIBC_MESSAGE_H

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>

#include "WifiDisplayUibcType.h"
#include <linux/input.h>


namespace android {

struct scanCodeBuild {
    uint16_t scanCode;
    bool shift_pressed;
    uint16_t latinComposeKey;
};

typedef struct scanCodeBuild scanCodeBuild_t;


struct UibcMessage : public RefBase {

    enum MessageType {
        GENERIC_TOUCH_DOWN = 0,
        GENERIC_TOUCH_UP,
        GENERIC_TOUCH_MOVE,
        GENERIC_KEY_DOWN,
        GENERIC_KEY_UP,
        GENERIC_ZOOM,
        GENERIC_VERTICAL_SCROLL,
        GENERIC_HORIZONTAL_SCROLL,
        GENERIC_ROTATE
    };

    UibcMessage(UibcMessage::MessageType type,
                const char* inEventDesc,
                double widthRatio,
                double heightRatio);
    virtual ~UibcMessage();

    status_t init();
    status_t destroy();

    char* getPacketData();
    int getPacketDataLen();

    bool isDataValid();

    static scanCodeBuild_t asciiToScancodeBuild(UINT16 uibcCode);
    static short scancodeToAcsii(UINT8 keyCode);
    static int asciiToKeycode(char asciiCode);
    static char keycodeToAcsii(int keyCode);
    static void getScreenResolution(int* x, int* y);
    static char** str_split(char* a_str, const char a_delim);

protected:


private:

    int32_t getUIBCGenericTouchPacket(const char *inEventDesc,
                                      char** outData,
                                      double widthRatio,
                                      double heightRatio);
    int32_t getUIBCGenericKeyPacket(const char *inEventDesc, char** outData);
    int32_t getUIBCGenericZoomPacket(const char *inEventDesc, char** outData);
    int32_t getUIBCGenericScalePacket(const char *inEventDesc, char** outData);
    int32_t getUIBCGenericRotatePacket(const char *inEventDesc, char** outData);

    char* m_PacketData;
    int m_PacketDataLen;
    bool m_DataValid;

};

}

#endif
