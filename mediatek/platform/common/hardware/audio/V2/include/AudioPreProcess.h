#ifndef _AUDIO_PRE_PROCESS_H
#define _AUDIO_PRE_PROCESS_H

#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <system/audio.h>
#include <hardware/audio.h>
#include "AudioType.h"
#include "AudioSpeechEnhanceInfo.h"

#include <audio_utils/resampler.h>
#include <audio_utils/echo_reference.h>
#include <hardware/audio_effect.h>
#include <audio_effects/effect_aec.h>



#define MAX_PREPROCESSORS 3 /* maximum one AGC + one NS + one AEC per input stream */

namespace android
{

struct effect_info_s
{
    effect_handle_t effect_itfe;
    size_t num_channel_configs;
    channel_config_t *channel_configs;
};

class AudioPreProcess
{
    public:
        AudioPreProcess();
        ~AudioPreProcess();

        void stop_echo_reference(struct echo_reference_itfe *reference);
        //tuna: void put_echo_reference(struct echo_reference_itfe *reference);

        struct echo_reference_itfe *start_echo_reference(audio_format_t format, uint32_t channel_count, uint32_t sampling_rate);
        //tuna: echo_reference_itfe *get_echo_reference(audio_format_t format,uint32_t channel_count,uint32_t sampling_rate);
        void push_echo_reference(size_t frames);
        int in_configure_reverse(uint32_t channel_count, uint32_t sampling_rate);

        int num_preprocessors;
        bool need_echo_reference;
        struct effect_info_s preprocessors[MAX_PREPROCESSORS];
        int16_t *proc_buf_in;
        int16_t *proc_buf_out;
        size_t proc_buf_size;
        size_t proc_buf_frames;

    private:

        void add_echo_reference(struct echo_reference_itfe *reference);
        void clear_echo_reference(struct echo_reference_itfe *reference);
        void remove_echo_reference(struct echo_reference_itfe *reference);
        int32_t update_echo_reference(size_t frames);
        int set_preprocessor_echo_delay(effect_handle_t handle, int32_t delay_us);
        int set_preprocessor_param(effect_handle_t handle, effect_param_t *param);
        void get_capture_delay(size_t frames, struct echo_reference_buffer *buffer);


        bool MutexLock(void);
        bool MutexUnlock(void);

        int16_t *ref_buf;
        size_t ref_buf_size;
        size_t ref_buf_frames;
        struct echo_reference_itfe *echo_reference;

        Mutex   mLock;
        //      AudioMTKHardware *mHw;

        int mInChn;
        uint32 mInSampleRate;
        AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

};
}
#endif
