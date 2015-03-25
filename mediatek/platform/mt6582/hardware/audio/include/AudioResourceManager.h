#ifndef _AUDIO_RESOURCE_MANAGER_H
#define _AUDIO_RESOURCE_MANAGER_H

#include "AudioAnalogType.h"
#include "AudioUtility.h"
#include "AudioType.h"
#include "AudioResourceManagerInterface.h"
#include "AudioMTKVolumeInterface.h"
#include "AudioDigitalControlInterface.h"
#include "AudioAnalogControlInterface.h"
#include "AudioSpeechEnhanceInfo.h"

#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <hardware_legacy/AudioSystemLegacy.h>


#define AUDIOLOCK_DEBUG_ENABLE

namespace android
{
class   AudioSpeechEnhanceInfo;

class AudioResourceManager : public AudioResourceManagerInterface
{
    public:
        static AudioResourceManager *getInstance();

        /**
         * a function for ~AudioResourceManagerInterface destructor
         */
        virtual ~AudioResourceManager() {};

        /**
        * a function for tell AudioResourceManager output device, usually use for output routing
        * outdevice change may also effetc input device , take care of this.
        * @param new_device
        */
        virtual status_t setDlOutputDevice(uint32 new_device);

        /**
        * a function for tell AudioResourceManager input device, usually use for output routing
        * @param new_device
        */
        virtual status_t setUlInputDevice(uint32 new_device);

        /**
        * a function for tell AudioResourceManager ionput source, usually use for input routing
        * @param mDevice
        */
        virtual status_t setUlInputSource(uint32 Source);

        /**
         * a function for tell AudioResourceManager mMode
         * @param mMode
         */
        virtual status_t SetAudioMode(audio_mode_t NewMode);

        /**
         * a function for tell AudioResourceManager mMode
         * @param mMode
         */

        virtual audio_mode_t GetAudioMode();

        /**
         * a function for tell AudioResourceManager select StartOutputDevice
         */
        virtual status_t StartOutputDevice();

        /**
         * a function for tell AudioResourceManager select StopOutputDevice
         */
        virtual status_t StopOutputDevice();

        /**
        * a function for tell AudioResourceManager select outputdevice
        */
        virtual status_t SelectOutputDevice(uint32_t new_device);

        /**
         * a function for tell AudioResourceManager select StartInputDevice
         */
        virtual status_t StartInputDevice();

        /**
         * a function for tell AudioResourceManager select StopInputDevice
         */
        virtual status_t StopInputDevice();

        /**
         * a function for tell AudioResourceManager select inputdevice
         */
        virtual status_t SelectInputDevice(uint32_t device) ;

        /**
         * a function for tell AudioResourceManager get current output device
         */
        virtual uint32 getDlOutputDevice(void);

        /**
         * a function for tell AudioResourceManager get current input device
         */
        virtual uint32 getUlInputDevice(void);

        /**
         * a function for tell AudioResourceManager to aquire lock
         */
        virtual status_t EnableAudioLock(int AudioLockType, int mTimeout);

        /**
         * a function for tell AudioResourceManager to release lock
         */
        virtual status_t DisableAudioLock(int AudioLockType);

        /**
        * a function for tell AudioResourceManager to request  or release clock
        */
        virtual status_t EnableAudioClock(int AudioClockType , bool bEnable);

        /**
        * a  function fo setParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , int command2 , unsigned int data);

        /**
        * a function fo setParameters , provide wide usage of analog control
        * @param command1
        * @param data
        * @return status_t
        */
        virtual status_t setParameters(int command1 , void *data);

        /**
        * a function fo getParameters , provide wide usage of analog control
        * @param command1
        * @param command2
        * @param data
        * @return copy size of data
        */
        virtual int getParameters(int command1 , int command2 , void *data);

        /**
        * a function fo dump AudioResourceManager , can base on command1 to dump different information
        * @param command1
        * @param command2
        */
        virtual int dump(int command1);

        /**
        * a function to turn on audiodevice
        * @param mDlOutputDevice
        */
        status_t  TurnonAudioDevice(unsigned int mDlOutputDevice);

        /**
        * a function to turn on audiodevice when incall mode
        * @param mDlOutputDevice
        */
        status_t TurnonAudioDeviceIncall(unsigned int mDlOutputDevice);

        /**
        * a function to turn off audiodevice
        * @param mDlOutputDevice
        */
        status_t  TurnoffAudioDevice(unsigned int mDlOutputDevice);

        /**

        */
        virtual bool IsWiredHeadsetOn(void);

        /**
        * a function to return Mode Incall
        */
        virtual bool IsModeIncall(void) ;

        /**
         * a function for to set input device gain , and will base on different audio mode normal , incall
         * ex: mic
         */
        virtual status_t SetInputDeviceGain();


        status_t SelectInPutMicEnable(bool bEnable);

        /**
        * a function for to get active device
        */
        uint32_t PopCount(uint32_t u);

        /**
        * a function for to incall mode device base on output device
        */
        uint32_t GetIncallMicDevice(uint32 device);

        /**
        * a function for to get MIC digital gain for HD Record
        */
        virtual long GetSwMICDigitalGain();

        /**
        * a function for to get UL total gain for BesRecord
        */
        virtual uint8_t GetULTotalGainValue(void);

        /**
         * a function for set mic inverse
         */
        virtual status_t SetMicInvserse(bool bEnable);

        /**
         * a function for set AFE_ON
         */
        virtual status_t SetAfeEnable(const bool bEnable);

        /**
         * a function for get mic inverse
         */
        virtual bool GetMicInvserse(void);

        /**
         * a function for set Analog Frequence
         */
        virtual status_t SetFrequency(int DeviceType, unsigned int frequency);

        /*
         * a function for lock debug
         */
        virtual void SetDebugLine(int line);

        virtual status_t AddSubSPKToOutputDevice();
    protected:
        AudioResourceManager();

        // lock for hardware , mopde, streamout , streamin .
        // lock require sequence , always hardware ==> mode ==> streamout ==> streamin
        static AudioLock mAudioLock[AudioResourceManagerInterface::NUM_OF_AUDIO_LOCK];

        // use to open deivce file descriptor
        static int mFd;

        // when change device and output , need to change in Audio resource manager
        static unsigned int mDlOutputDevice;
        static unsigned int mUlInputDevice;
        static unsigned int mUlInputSource;

        static audio_mode_t mAudioMode;

        static bool mMicDefaultsetting;
        static bool mMicInverseSetting;

        static AudioMTKVolumeInterface *mAudioVolumeInstance;
        static AudioAnalogControlInterface *mAudioAnalogInstance;
        static AudioDigitalControlInterface *mAudioDigitalInstance;
        static AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

#ifdef AUDIOLOCK_DEBUG_ENABLE
        unsigned int mAudioLockRecord[AudioResourceManagerInterface::NUM_OF_AUDIO_LOCK + 1][5];           //[Tid][BackAddr][LockType][order in tid][state] state: 1:lock 0:unlock 0xffffffff:fail
        mutable Mutex   mAudioLockRecordLock;
        int mStreamOutLine;
#endif
    private:
        static AudioResourceManager *UniqueAudioResourceInstance;
        AudioResourceManager(const AudioResourceManager &);             // intentionally undefined
        AudioResourceManager &operator=(const AudioResourceManager &);  // intentionally undefined
};

}

#endif
