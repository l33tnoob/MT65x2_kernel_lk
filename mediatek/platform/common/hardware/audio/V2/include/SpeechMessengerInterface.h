#ifndef ANDROID_SPEECH_MESSAGE_INTERFACE_H
#define ANDROID_SPEECH_MESSAGE_INTERFACE_H

#include <pthread.h>

#include "AudioType.h"
#include "SpeechType.h"
#include "AudioUtility.h"

#include "SpeechBGSPlayer.h"
#include "SpeechPcm2way.h"

namespace android
{

#define CCCI_BUF_HEADER_SIZE 16
//EEMCS MTU 3584-128 = 3456 byte
#define EEMCS_MAX_PAYLOAD_SIZE 860 //(3456-16)/4 = 860
/** CCCI buffer structure */
typedef struct
{
    uint32_t magic;
    uint32_t message; // message[31:16] = id, message[15:0] = parameters
    uint32_t channel;
    uint32_t reserved;
    uint32_t payload[EEMCS_MAX_PAYLOAD_SIZE];
} ccci_buff_t;
/** CCCI driver & ioctl */

/** CCCI message need/no need ack type */
enum ccci_message_ack_t
{
    MESSAGE_BYPASS_ACK = 0,
    MESSAGE_NEED_ACK   = 1,
    MESSAGE_CANCELED   = 8
};

/** CCCI message queue structure */
typedef struct ccci_queue_element_t
{
    ccci_buff_t        ccci_buff;
    ccci_message_ack_t ack_type;
} ccci_queue_element_t;


/** Speech parameter ack type */
enum speech_param_ack_t
{
    NB_SPEECH_PARAM = 0,
    DMNR_SPEECH_PARAM,
    WB_SPEECH_PARAM,
    NUM_SPEECH_PARAM_ACK_TYPE
};


/** CCCI share buffer related infomation */
const size_t CCCI_SHARE_BUFF_HEADER_LEN = 6;
const size_t EEMCS_PAYLOAD_BUFF_HEADER_LEN = 10;

enum share_buff_sync_t
{
    CCCI_A2M_SHARE_BUFF_HEADER_SYNC = 0xA2A2,
    CCCI_M2A_SHARE_BUFF_HEADER_SYNC = 0x2A2A,
    EEMCS_M2A_SHARE_BUFF_HEADER_SYNC = 0x1234
};
enum share_buff_data_type_t
{
    SHARE_BUFF_DATA_TYPE_PCM_FillSE = 0,
    SHARE_BUFF_DATA_TYPE_PCM_FillSpk,
    SHARE_BUFF_DATA_TYPE_PCM_GetFromMic,
    SHARE_BUFF_DATA_TYPE_PCM_GetfromSD,
    SHARE_BUFF_DATA_TYPE_CCCI_VM_TYPE,
    SHARE_BUFF_DATA_TYPE_CCCI_PCM_TYPE,
    SHARE_BUFF_DATA_TYPE_CCCI_BGS_TYPE,
    SHARE_BUFF_DATA_TYPE_CCCI_EM_PARAM,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_IN,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_IN,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_UL_OUT,
    SHARE_BUFF_DATA_TYPE_CCCI_CTM_DL_OUT,
    SHARE_BUFF_DATA_TYPE_CCCI_VIBSPK_PARAM,
    SHARE_BUFF_DATA_TYPE_CCCI_NXP_SMARTPA_PARAM = 15,
    SHARE_BUFF_DATA_TYPE_CCCI_MAX_TYPE
};

//For VT case, the CCCI message for every 20ms, UL/DL have 2 CCCI message (Put to Speaker / Get from Mic)
//For BGS off ack message, the worst case maybe pending 150 ms. And for other change device control. (BGSoff,2WAY off,SPH off,...)
//The total message maybe > 20 for this period. So enlarge the total CCCI message queue.
//For the CCCI queue in CCCI kernel driver, the size is 60.
//For the CCCI queue in Modem side, the size is 32.
//Modem side would keep on optimized the BGS off ack period.
const size_t CCCI_MAX_QUEUE_NUM = 60;

// BG Sound use 1022 bytes [0, 1028)
const size_t A2M_SHARED_BUFFER_BGS_DATA_SIZE = 1022;
const size_t A2M_SHARED_BUFFER_BGS_DATA_BASE = 0;
const size_t A2M_SHARED_BUFFER_BGS_DATA_END  = A2M_SHARED_BUFFER_BGS_DATA_BASE + CCCI_SHARE_BUFF_HEADER_LEN + A2M_SHARED_BUFFER_BGS_DATA_SIZE;

// PCM2WAY playback use 640 bytes [1028, 1674) // MAX => 16K PCM2WAY use 640+6 bytes
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_BASE = A2M_SHARED_BUFFER_BGS_DATA_END;
const size_t A2M_SHARED_BUFFER_P2W_DL_DATA_END  = A2M_SHARED_BUFFER_P2W_DL_DATA_BASE + CCCI_SHARE_BUFF_HEADER_LEN + PCM2WAY_PLAY_BUFFER_WB_LEN;

// Speech enhacement parameters [1674, 4096) // MAX => WB Param use 2416+6 bytes
const size_t A2M_SHARED_BUFFER_SPH_PARAM_BASE = A2M_SHARED_BUFFER_P2W_DL_DATA_END;

/* CCCI Message ID */
const uint32_t CCCI_MSG_A2M_BASE = 0x2F00;
const uint32_t CCCI_MSG_M2A_BASE = 0xAF00;

class SpeechMessengerInterface
{
    public:
        virtual ~SpeechMessengerInterface() {}
        virtual status_t    Initial() = 0;
        virtual status_t    Deinitial() = 0;
        virtual bool          A2MBufLock() = 0;
        virtual void          A2MBufUnLock() = 0;

        virtual status_t    WaitUntilModemReady() = 0;

        virtual ccci_buff_t InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit) = 0;
        virtual status_t    SendMessageInQueue(ccci_buff_t ccci_buff) = 0;


        inline uint32_t     GetA2MShareBufLen() const { return mA2MShareBufLen; }
        inline char        *GetA2MShareBufBase() const { return mA2MShareBufBase; }

        inline uint8_t      SetShareBufHeader(uint16_t *ptr16, uint16_t sync, share_buff_data_type_t type, uint16_t length) const
        {
            ptr16[0] = sync;
            ptr16[1] = type;
            ptr16[2] = length;
            return CCCI_SHARE_BUFF_HEADER_LEN; // 3 * sizeof(uint16_t);
        }
        inline uint32_t     GetM2AShareBufLen() const { return mM2AShareBufLen; }
        inline char        *GetM2AShareBufBase() const { return mM2AShareBufBase; }
        virtual uint16_t            GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff) = 0;
        virtual uint16_t            GetM2AShareBufDataType(const ccci_buff_t &ccci_buff) = 0;
        virtual uint16_t            GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff) = 0;

        /**
         * get modem side modem function status
         */
        virtual bool        GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const = 0;


        /**
         * check whether modem side get all necessary speech enhancement parameters here
         */
        virtual bool        CheckSpeechParamAckAllArrival() = 0;


        /**
         * check whether modem is ready. (if w/o SIM && phone_2 => modem sleep)
         */
        virtual bool        CheckModemIsReady() = 0;

        // share buffer base and len
        uint32_t mA2MShareBufLen;
        uint32_t mM2AShareBufLen;

        char    *mA2MShareBufBase;
        char    *mM2AShareBufBase;

        char    *mA2MShareBufEnd;
        char    *mM2AShareBufEnd;


    protected:
        virtual char        GetModemCurrentStatus() = 0;

        virtual uint16_t    GetMessageID(const ccci_buff_t &ccci_buff) = 0;
        virtual uint16_t    GetMessageParam(const ccci_buff_t &ccci_buff) = 0;

        virtual uint16_t    GetMessageLength(const ccci_buff_t &ccci_buff) = 0;
        virtual uint16_t    GetMessageOffset(const ccci_buff_t &ccci_buff) = 0;
        virtual bool        CheckOffsetAndLength(const ccci_buff_t &ccci_buff) = 0;

        virtual ccci_message_ack_t JudgeAckOfMsg(const uint16_t message_id) = 0;

        virtual bool        IsModemFunctionOnOffMessage(const uint16_t message_id) = 0;

        virtual status_t    SendMessage(const ccci_buff_t &ccci_buff) = 0;
        virtual status_t    ReadMessage(ccci_buff_t &ccci_buff) = 0;
        virtual void          SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff) = 0;

        virtual RingBuf     GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff) = 0;
        virtual RingBuf     GetM2ACtmRingBuffer(const ccci_buff_t &ccci_buff) = 0;

        virtual status_t    CreateReadingThread() = 0;
        virtual status_t    CreateSendSphParaThread() = 0;

        // for message queue
        virtual uint32_t    GetQueueCount() const = 0;
        virtual status_t    ConsumeMessageInQueue() = 0;
        virtual bool        MDReset_CheckMessageInQueue() = 0;
        virtual void        MDReset_FlushMessageInQueue() = 0;

        virtual void        ResetSpeechParamAckCount() = 0;
        virtual void        AddSpeechParamAckCount(speech_param_ack_t type) = 0;


        /**
         * set/reset AP side modem function status
         */

        virtual void        SetModemSideModemStatus(const modem_status_mask_t modem_status_mask) = 0;
        virtual void        ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask) = 0;

        // lock
        virtual bool SpeechParamLock() = 0;
        virtual void SpeechParamUnLock() = 0;

        //    private:
};

enum ccci_message_id_t
{
    //------------------ A2M -----------------------
    MSG_A2M_SPH_DL_DIGIT_VOLUME = CCCI_MSG_A2M_BASE,
    MSG_A2M_SPH_UL_DIGIT_VOLUME,
    MSG_A2M_MUTE_SPH_UL,
    MSG_A2M_MUTE_SPH_DL,
    MSG_A2M_SIDETONE_VOLUME,
    MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME,

    MSG_A2M_SET_SAMPLE_RATE = CCCI_MSG_A2M_BASE | 0x10,

    MSG_A2M_SPH_ON = CCCI_MSG_A2M_BASE | 0x20,
    MSG_A2M_SPH_OFF,
    MSG_A2M_SET_SPH_MODE,
    MSG_A2M_CTRL_SPH_ENH,
    MSG_A2M_CONFIG_SPH_ENH,
    MSG_A2M_SET_ACOUSTIC_LOOPBACK,
    MSG_A2M_PRINT_SPH_PARAM,
    MSG_A2M_SPH_ON_FOR_HOLD_CALL, // speech on with mute, for call hold use, no any other application can be turn on


    MSG_A2M_PNW_ON = CCCI_MSG_A2M_BASE | 0x30,
    MSG_A2M_PNW_OFF,
    MSG_A2M_RECORD_ON,
    MSG_A2M_RECORD_OFF,
    MSG_A2M_DMNR_RECPLAY_ON,
    MSG_A2M_DMNR_RECPLAY_OFF,
    MSG_A2M_DMNR_REC_ONLY_ON,
    MSG_A2M_DMNR_REC_ONLY_OFF,
#ifdef SPEECH_PCM_VM_SUPPORT
    MSG_A2M_PCM_REC_ON,
    MSG_A2M_PCM_REC_OFF,
    MSG_A2M_VM_REC_ON,
    MSG_A2M_VM_REC_OFF,
#endif

    MSG_A2M_CTM_ON = CCCI_MSG_A2M_BASE | 0x40,
    MSG_A2M_CTM_OFF,
    MSG_A2M_CTM_DUMP_DEBUG_FILE,
    MSG_A2M_BGSND_ON,
    MSG_A2M_BGSND_OFF,
    MSG_A2M_BGSND_CONFIG,

    MSG_A2M_PNW_DL_DATA_NOTIFY = CCCI_MSG_A2M_BASE | 0x50,
    MSG_A2M_BGSND_DATA_NOTIFY,
    MSG_A2M_CTM_DATA_NOTIFY,

    MSG_A2M_PNW_UL_DATA_READ_ACK = CCCI_MSG_A2M_BASE | 0x60,
    MSG_A2M_REC_DATA_READ_ACK,
    MSG_A2M_CTM_DEBUG_DATA_READ_ACK,
#ifdef SPEECH_PCM_VM_SUPPORT
    MSG_A2M_PCM_REC_DATA_READ_ACK,
    MSG_A2M_VM_REC_DATA_READ_ACK,
#endif

    MSG_A2M_EM_DATA_REQUEST_ACK = CCCI_MSG_A2M_BASE | 0x70,
    MSG_A2M_EM_NB,
    MSG_A2M_EM_DMNR,
    MSG_A2M_EM_WB,
    MSG_A2M_VIBSPK_PARAMETER = CCCI_MSG_A2M_BASE | 0x80,
    MSG_A2M_NXP_SMARTPA_PARAMETER,
    //------------------- M2A ----------------------
    MSG_M2A_SPH_DL_DIGIT_VOLUME_ACK = CCCI_MSG_M2A_BASE,
    MSG_M2A_SPH_UL_DIGIT_VOLUME_ACK,
    MSG_M2A_MUTE_SPH_UL_ACK,
    MSG_M2A_MUTE_SPH_DL_ACK,
    MSG_M2A_SIDETONE_VOLUME_ACK,
    MSG_M2A_SPH_DL_ENH_REF_DIGIT_VOLUME_ACK, // just define, not used.

    MSG_M2A_SET_SAMPLE_RATE_ACK = CCCI_MSG_M2A_BASE + 0x10,


    MSG_M2A_SPH_ON_ACK = CCCI_MSG_M2A_BASE + 0x20,
    MSG_M2A_SPH_OFF_ACK,
    MSG_M2A_SET_SPH_MODE_ACK,
    MSG_M2A_CTRL_SPH_ENH_ACK,
    MSG_M2A_CONFIG_SPH_ENH_ACK,
    MSG_M2A_SET_ACOUSTIC_LOOPBACK_ACK,
    MSG_M2A_PRINT_SPH_COEFF_ACK,
    MSG_M2A_SPH_ON_FOR_HOLD_CALL_ACK,


    MSG_M2A_PNW_ON_ACK = CCCI_MSG_M2A_BASE + 0x30,
    MSG_M2A_PNW_OFF_ACK,
    MSG_M2A_RECORD_ON_ACK,
    MSG_M2A_RECORD_OFF_ACK,
    MSG_M2A_DMNR_RECPLAY_ON_ACK,
    MSG_M2A_DMNR_RECPLAY_OFF_ACK,
    MSG_M2A_DMNR_REC_ONLY_ON_ACK,
    MSG_M2A_DMNR_REC_ONLY_OFF_ACK,
#ifdef SPEECH_PCM_VM_SUPPORT
    MSG_M2A_PCM_REC_ON_ACK,
    MSG_M2A_PCM_REC_OFF_ACK,
    MSG_M2A_VM_REC_ON_ACK,
    MSG_M2A_VM_REC_OFF_ACK,
#endif

    MSG_M2A_CTM_ON_ACK = CCCI_MSG_M2A_BASE + 0x40,
    MSG_M2A_CTM_OFF_ACK,
    MSG_M2A_CTM_DUMP_DEBUG_FILE_ACK,
    MSG_M2A_BGSND_ON_ACK,
    MSG_M2A_BGSND_OFF_ACK,
    MSG_M2A_BGSND_CONFIG_ACK,

    MSG_M2A_PNW_DL_DATA_REQUEST = CCCI_MSG_M2A_BASE + 0x50,
    MSG_M2A_BGSND_DATA_REQUEST,
    MSG_M2A_CTM_DATA_REQUEST,

    MSG_M2A_PNW_UL_DATA_NOTIFY = CCCI_MSG_M2A_BASE + 0x60,
    MSG_M2A_REC_DATA_NOTIFY,
    MSG_M2A_CTM_DEBUG_DATA_NOTIFY,
#ifdef SPEECH_PCM_VM_SUPPORT
    MSG_M2A_PCM_REC_DATA_NOTIFY,
    MSG_M2A_VM_REC_DATA_NOTIFY,
#endif

    MSG_M2A_EM_DATA_REQUEST = CCCI_MSG_M2A_BASE + 0x70,
    MSG_M2A_EM_NB_ACK,
    MSG_M2A_EM_DMNR_ACK,
    MSG_M2A_EM_WB_ACK,
    MSG_M2A_VIBSPK_PARAMETER_ACK = CCCI_MSG_M2A_BASE + 0x80,
    MSG_M2A_NXP_SMARTPA_PARAMETER_ACK,
};

} // end namespace android

#endif // end of ANDROID_SPEECH_MESSAGE_INTERFACE_H
