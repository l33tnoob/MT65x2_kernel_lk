
#ifndef ANDROID_AUDIO_MTK_STREAMINMANAGER_INTERFACE_H
#define ANDROID_AUDIO_MTK_STREAMINMANAGER_INTERFACE_H

#include "AudioMTKStreamInClient.h"

namespace android
{
//!  AudioMTKStreaminInterface interface
/*!
this class is hold for StreamIn , need to concer multitple user and mode change.
need to take care both input and output volume.
*/

class AudioMTKStreamInManagerInterface
{
        /**
        * virtual destrutor
        */
        virtual ~AudioMTKStreamInManagerInterface() {};

        /**
        * check init done.
        * @return status_t*/
        virtual status_t  initCheck() = 0;

        /**
        * do input standby , all read will be blocked.
        * @return status_t*/
        virtual status_t  Do_input_standby() = 0 ;

        /**
        * do input standby
        * @return status_t*/
        virtual status_t  Do_input_start(AudioMTKStreamInClient *Client) = 0 ;

        /**
        * do request StreaminClient
        * @return status_t*/
        virtual AudioMTKStreamInClient *RequestClient(uint32_t Buflen) = 0;

        /**
        * do release StreaminClient
        * @return status_t*/
        virtual status_t ReleaseClient(AudioMTKStreamInClient *Client) = 0;

        /**
        * this function is set mic mute
        * @return status_t*/
        virtual uint32 SetInputMute(bool bEnable) = 0;

        /**
        * this function is get record drop time
        * @return status_t*/
        virtual uint32 GetRecordDropTime() = 0;

};

}

#endif

