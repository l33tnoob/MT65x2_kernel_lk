

#define LOG_TAG "UibcClientHandler"
#include <utils/Log.h>

#include "UibcClientHandler.h"
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

namespace android {

UibcClientHandler::UibcClientHandler()
    : mSessionID(0) {
}

UibcClientHandler::~UibcClientHandler() {
}

status_t UibcClientHandler::init() {
    int version;

    UibcHandler::init();
    return OK;
}

status_t UibcClientHandler::destroy() {

    return OK;
}

status_t UibcClientHandler::sendUibcMessage(        sp<ANetworkSession> netSession,
        UibcMessage::MessageType type,
        const char *eventDesc) {
    status_t err;
    UibcMessage* message = new UibcMessage(type, eventDesc, m_widthRatio, m_heightRatio);
    if (message != NULL && message->isDataValid()) {
        ALOGI("sendUibcMessage Sending msg");
        err = netSession->sendDirectRequest(
                  mSessionID, message->getPacketData(), message->getPacketDataLen());
    }
    delete message;
    return err;
}

void UibcClientHandler::setSessionID(int32_t SessionID) {
    ALOGI("setSessionID (%d)", SessionID);
    mSessionID = SessionID;
}

int32_t UibcClientHandler::getSessionID() {
    ALOGI("getSessionID (%d)", mSessionID);
    return mSessionID;
}

}
