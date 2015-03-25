#ifndef ANDROID_SPEECH_DRIVER_FACTORY_H
#define ANDROID_SPEECH_DRIVER_FACTORY_H

#include <utils/Vector.h>
#include "AudioType.h"
#include "SpeechDriverInterface.h"

namespace android
{
class SpeechDriverFactory
{
    public:
        virtual ~SpeechDriverFactory();

        static SpeechDriverFactory *GetInstance();

        SpeechDriverInterface      *GetSpeechDriver();
        SpeechDriverInterface      *GetSpeechDriverByIndex(const modem_index_t modem_index);

        modem_index_t               GetActiveModemIndex() const;
        status_t                    SetActiveModemIndex(const modem_index_t modem_index);
        status_t                    SetActiveModemIndexByAudioMode(const audio_mode_t audio_mode);

    protected:
        SpeechDriverFactory();

        /**
          * sub factory class can override this function
          * to create types of speech driver instances
          */
        virtual status_t CreateSpeechDriverInstances();
        virtual status_t DestroySpeechDriverInstances();


        /**
          * centralized management of speech drivers
          */
        modem_index_t mActiveModemIndex;

        SpeechDriverInterface *mSpeechDriver1; // for modem 1
        SpeechDriverInterface *mSpeechDriver2; // for modem 2
        SpeechDriverInterface *mSpeechDriverExternal; // for modem External

    private:
        /**
         * singleton pattern
         */
        static SpeechDriverFactory *mSpeechDriverFactory;
};

} // end namespace android

#endif // end of ANDROID_SPEECH_DRIVER_FACTORY_H
