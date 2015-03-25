
#ifndef UIBC_CLIENT_HANDLER_H
#define UIBC_CLIENT_HANDLER_H

#include "UibcHandler.h"
#include "UibcMessage.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ANetworkSession.h>


#include <utils/RefBase.h>
#include <utils/Thread.h>

#include "WifiDisplayUibcType.h"
#include <linux/input.h>

namespace android {

struct UibcClientHandler   : public RefBase, public UibcHandler  {
    UibcClientHandler();
    status_t init();
    status_t destroy();

    void setSessionID(int32_t SessionID);
    int32_t getSessionID();

    status_t sendUibcMessage(sp<ANetworkSession> netSession,
                             UibcMessage::MessageType type,
                             const char *eventDesc) ;

protected:
    virtual ~UibcClientHandler();

private:
    int32_t mSessionID;
    int hid_kbd_fd;
};

}

#endif
