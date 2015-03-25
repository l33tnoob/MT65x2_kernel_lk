
#define LOG_TAG "UibcHandler"
#include "UibcHandler.h"

#include <utils/Log.h>
#include "UibcMessage.h"
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



namespace android {

UibcHandler::UibcHandler()
    : m_wfdWidth(0),
      m_wfdHeight(0),
      m_localWidth(0),
      m_localHeight(0),
      m_widthRatio(0.0),
      m_heightRatio(0.0),
      mUibcEnabled(false),
      mRemote_InputCat(0),
      mRemote_InpType(0),
      mPort(0) {
}

UibcHandler::~UibcHandler() {

}

status_t UibcHandler::init() {
    int version;

    UibcMessage::getScreenResolution(&m_localWidth, &m_localHeight);

    return OK;
}

status_t UibcHandler::destroy() {
    return OK;
}

void UibcHandler::setWFDResolution(int width, int heigh) {
    m_wfdWidth = width;
    m_wfdHeight = heigh;

    m_widthRatio = (double)m_localWidth / (double)m_wfdWidth;
    m_heightRatio = (double)m_localHeight / (double)m_wfdHeight;
    ALOGD("setWFDResolution w:%d/%d=%f, h:%d/%d=%f",
          m_localWidth, m_wfdWidth, m_widthRatio,
          m_localHeight, m_wfdHeight, m_heightRatio);
}

void UibcHandler::run_consol_cmd(char *command) {
    ALOGD("run_consol_cmd:\"%s\"", command);
    FILE *fpipe;
    char line[256];
    if ( !(fpipe = (FILE*)popen(command, "r")) )
        return;

    while ( fgets( line, sizeof line, fpipe)) {
        puts(line);
    }
    pclose(fpipe);
}

void UibcHandler::setUibcEnabled(bool enabled) {
    mUibcEnabled = enabled;
}

void UibcHandler::setPort(int32_t port) {
    ALOGD("setServerPort (%d)", port);
    mPort = port;
}

void UibcHandler::setRemoteInputCatList(AString input_category_list) {
    ALOGD("setRemoteInputCatList (%s)", input_category_list.c_str());
    mRemote_InputCat = 0x00;

    char* pch = strstr(input_category_list.c_str(), "GENERIC");

    if(pch)
        mRemote_InputCat |= INPUT_CAT_GENERIC;

    pch = strstr(input_category_list.c_str(), "HIDC");
    if(pch)
        mRemote_InputCat |= INPUT_CAT_HIDC;
}

void UibcHandler::setRemoteGenericCapList(AString generic_cap_list) {
    ALOGD("setRemoteGenericCapList (%s)", generic_cap_list.c_str());
    mRemote_InpType = 0x00;
    char* pch = strstr(generic_cap_list.c_str(), "Keyboard");
    if(pch)
        mRemote_InpType |= GENERIC_KEYBOARD;
    pch = strstr(generic_cap_list.c_str(), "Mouse");
    if(pch)
        mRemote_InpType |= GENERIC_MOUSE;
    pch = strstr(generic_cap_list.c_str(), "SingleTouch");
    if(pch)
        mRemote_InpType |= GENERIC_SINGLETOUCH;
    pch = strstr(generic_cap_list.c_str(), "MultiTouch");
    if(pch)
        mRemote_InpType |= GENERIC_MULTITOUCH;
    pch = strstr(generic_cap_list.c_str(), "Joystick");
    if(pch)
        mRemote_InpType |= GENERIC_JOYSTICK;
    pch = strstr(generic_cap_list.c_str(), "Camera");
    if(pch)
        mRemote_InpType |= GENERIC_CAMERA;
    pch = strstr(generic_cap_list.c_str(), "Gesture");
    if(pch)
        mRemote_InpType |= GENERIC_GESTURE;
    pch = strstr(generic_cap_list.c_str(), "RemoteControl");
    if(pch)
        mRemote_InpType |= GENERIC_REMOTECONTROL;
}

void UibcHandler::setRemoteHidcCapList(AString hidc_cap_list) {
    ALOGI("setServerHidcCapList (%s)", hidc_cap_list.c_str());
}

bool UibcHandler::getUibcEnabled() {
    ALOGD("getUibcEnabled mUibcEnabled=%d", mUibcEnabled);
    return mUibcEnabled;
}
int UibcHandler::getRemoteInputCatList() {
    ALOGD("getRemoteInputCatList mRemote_InputCat=0x%04X", mRemote_InputCat);
    return mRemote_InputCat;
}
int UibcHandler::getRemoteGenericCapList() {
    ALOGD("getRemoteGenericCapList mRemote_InpType=0x%04X", mRemote_InpType);
    return mRemote_InpType;
}
int UibcHandler::getPort() {
    ALOGD("getPort mPort=%d", mPort);
    return mPort;
}

int UibcHandler::getSupportedInputCatList(int localVal) {
    return localVal & mRemote_InputCat;
}

int UibcHandler::getSupportedGenericCapList(int localVal) {
    return localVal & mRemote_InpType;
}

char* UibcHandler::getSupportedInputCatListStr(int localVal) {
    int supportedCat = localVal & mRemote_InputCat;

    char* output = (char*)malloc(128);
    *output = 0x00;
    strcpy(output, "input_category_list=");

    if (supportedCat == 0x0) {
        strcat(output, "none,");
    } else {
        if ((supportedCat & INPUT_CAT_GENERIC) != 0x0)
            strcat(output, "GENERIC,");
        if ((supportedCat & INPUT_CAT_HIDC) != 0x0)
            strcat(output, "HIDC,");
    }

    // Remove the latest comma
    output[strlen(output) - 1] = 0x0;
    ALOGD("getSupportedInputCatListStr output=%s", output);
    return output;
}

char* UibcHandler::getSupportedGenericCapListStr(int localVal) {
    int supportedCap = localVal & mRemote_InpType;
    char* output = (char*)malloc(128);
    *output = 0x00;
    strcpy(output, "generic_cap_list=");

    if (supportedCap == 0x0) {
        strcat(output, "none,");
    } else {
        if ((supportedCap & GENERIC_MOUSE) != 0x0)
            strcat(output, "Mouse, ");
        if ((supportedCap & GENERIC_KEYBOARD) != 0x0)
            strcat(output, "Keyboard, ");
        if ((supportedCap & GENERIC_SINGLETOUCH) != 0x0)
            strcat(output, "SingleTouch, ");
        if ((supportedCap & GENERIC_MULTITOUCH) != 0x0)
            strcat(output, "MultiTouch, ");
        if ((supportedCap & GENERIC_JOYSTICK) != 0x0)
            strcat(output, "Joystick, ");
        if ((supportedCap & GENERIC_CAMERA) != 0x0)
            strcat(output, "Camera, ");
        if ((supportedCap & GENERIC_GESTURE) != 0x0)
            strcat(output, "Gesture, ");
        if ((supportedCap & GENERIC_REMOTECONTROL) != 0x0)
            strcat(output, "RemoteControl, ");
    }
    // Remove the latest comma
    if (output[strlen(output) - 2] == ',')
        output[strlen(output) - 2] = 0x0;
    ALOGD("getSupportedGenericCapListStr output=%s", output);
    return output;
}
}
