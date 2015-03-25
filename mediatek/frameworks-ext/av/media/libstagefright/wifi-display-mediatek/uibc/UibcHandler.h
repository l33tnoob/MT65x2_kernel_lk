
#ifndef UIBC_HANDLER_H
#define UIBC_HANDLER_H

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include <linux/input.h>


#define UIBC_ENABLED 		true
#define UIBC_DISABLED 		false
#define UIBC_NONE 			0x00


#define INPUT_CAT_GENERIC 			(0x01 << 0)
#define INPUT_CAT_HIDC 				(0x01 << 1)


#define GENERIC_KEYBOARD 			(0x01 << 0)
#define GENERIC_MOUSE 				(0x01 << 1)
#define GENERIC_SINGLETOUCH 		(0x01 << 2)
#define GENERIC_MULTITOUCH			(0x01 << 3)
#define GENERIC_JOYSTICK			(0x01 << 4)
#define GENERIC_CAMERA				(0x01 << 5)
#define GENERIC_GESTURE				(0x01 << 6)
#define GENERIC_REMOTECONTROL		(0x01 << 7)


#define SOURCE_SUPPORT_CAT INPUT_CAT_GENERIC
#define SOURCE_SUPPORT_TYPE (GENERIC_KEYBOARD|GENERIC_MOUSE|GENERIC_SINGLETOUCH|GENERIC_MULTITOUCH)

#define SINK_SUPPORT_CAT INPUT_CAT_GENERIC
#define SINK_SUPPORT_TYPE (GENERIC_KEYBOARD|GENERIC_MOUSE|GENERIC_SINGLETOUCH|GENERIC_MULTITOUCH)

namespace android {

struct UibcHandler {
    UibcHandler();
    status_t init();
    status_t destroy();

    void setWFDResolution(int width, int heigh);
    void run_consol_cmd(char *command);

    void setUibcEnabled(bool enabled);
    void setPort(int32_t port);

    void setRemoteInputCatList(AString input_category_list);
    void setRemoteGenericCapList(AString generic_cap_list);
    void setRemoteHidcCapList(AString hidc_cap_list);

    bool getUibcEnabled();
    int getPort();

    int getRemoteInputCatList();
    int getRemoteGenericCapList();
    // getRemoteHidcCapList should support getting path and device
    //int getRemoteHidcCapList();


    int getSupportedInputCatList(int localVal);
    int getSupportedGenericCapList(int localVal);

    char* getSupportedInputCatListStr(int localVal);
    char* getSupportedGenericCapListStr(int localVal);



protected:
    virtual ~UibcHandler();
    int m_wfdWidth;
    int m_wfdHeight;
    int m_localWidth;
    int m_localHeight;
    double m_widthRatio;
    double m_heightRatio;

    int mUibcEnabled;
    int mRemote_InputCat;
    int mRemote_InpType;

    AString mInput_category_list;
    AString mGeneric_cap_list;
    AString mHidc_cap_list;
    int32_t mPort;

private:

    DISALLOW_EVIL_CONSTRUCTORS(UibcHandler);
};

}

#endif
