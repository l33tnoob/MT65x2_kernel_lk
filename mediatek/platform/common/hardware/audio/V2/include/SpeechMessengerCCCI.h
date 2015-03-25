#ifndef ANDROID_SPEECH_MESSAGE_CCCI_H
#define ANDROID_SPEECH_MESSAGE_CCCI_H

#include <pthread.h>

#include "AudioType.h"
#include "SpeechType.h"
#include "AudioUtility.h"

#include "SpeechBGSPlayer.h"
#include "SpeechPcm2way.h"
#include "SpeechMessengerInterface.h"

namespace android
{

class SpeechDriverLAD;

class SpeechMessengerCCCI : public SpeechMessengerInterface
{
    public:
        SpeechMessengerCCCI(modem_index_t modem_index, SpeechDriverLAD *pLad);
        virtual ~SpeechMessengerCCCI();

        virtual status_t    Initial();
        virtual status_t    Deinitial();
        virtual bool                A2MBufLock();
        virtual void                A2MBufUnLock();

        virtual status_t    WaitUntilModemReady();

        virtual ccci_buff_t InitCcciMailbox(uint16_t id, uint16_t param_16bit, uint32_t param_32bit);
        virtual status_t    SendMessageInQueue(ccci_buff_t ccci_buff);

        virtual uint16_t            GetM2AShareBufSyncWord(const ccci_buff_t &ccci_buff);
        virtual uint16_t            GetM2AShareBufDataType(const ccci_buff_t &ccci_buff);
        virtual uint16_t            GetM2AShareBufDataLength(const ccci_buff_t &ccci_buff);

        /**
         * get modem side modem function status
         */
        virtual bool        GetModemSideModemStatus(const modem_status_mask_t modem_status_mask) const;


        /**
         * check whether modem side get all necessary speech enhancement parameters here
         */
        virtual bool        CheckSpeechParamAckAllArrival();


        /**
         * check whether modem is ready. (if w/o SIM && phone_2 => modem sleep)
         */
        virtual bool        CheckModemIsReady();



    protected:
        virtual char        GetModemCurrentStatus();

        virtual uint16_t    GetMessageID(const ccci_buff_t &ccci_buff);
        virtual uint16_t    GetMessageParam(const ccci_buff_t &ccci_buff);

        virtual uint16_t    GetMessageLength(const ccci_buff_t &ccci_buff);
        virtual uint16_t    GetMessageOffset(const ccci_buff_t &ccci_buff);
        virtual bool        CheckOffsetAndLength(const ccci_buff_t &ccci_buff);

        virtual ccci_message_ack_t JudgeAckOfMsg(const uint16_t message_id);

        virtual bool        IsModemFunctionOnOffMessage(const uint16_t message_id);

        virtual status_t    SendMessage(const ccci_buff_t &ccci_buff);
        virtual status_t    ReadMessage(ccci_buff_t &ccci_buff);
        virtual void                SendMsgFailErrorHandling(const ccci_buff_t &ccci_buff);

        virtual RingBuf     GetM2AUplinkRingBuffer(const ccci_buff_t &ccci_buff);
        virtual RingBuf     GetM2ACtmRingBuffer(const ccci_buff_t &ccci_buff);

        virtual status_t    CreateReadingThread();
        virtual status_t    CreateSendSphParaThread();

        static void        *CCCIReadThread(void *arg);
        static void        *SendSphParaThread(void *arg);

        // for message queue
        virtual uint32_t    GetQueueCount() const;
        virtual status_t    ConsumeMessageInQueue();
        virtual bool        MDReset_CheckMessageInQueue();
        virtual void        MDReset_FlushMessageInQueue();

        virtual void        ResetSpeechParamAckCount();
        virtual void        AddSpeechParamAckCount(speech_param_ack_t type);


        /**
         * set/reset AP side modem function status
         */
        virtual void        SetModemSideModemStatus(const modem_status_mask_t modem_status_mask);
        virtual void        ResetModemSideModemStatus(const modem_status_mask_t modem_status_mask);

        // lock
        virtual bool SpeechParamLock();
        virtual void SpeechParamUnLock();


        modem_index_t mModemIndex;
        SpeechDriverLAD *mLad;
        bool CCCIEnable;

        // file handle for CCCI user space interface
        int32_t fHdlRead;
        int32_t fHdlWrite;

        ccci_queue_element_t pQueue[CCCI_MAX_QUEUE_NUM];
        int32 iQRead;
        int32 iQWrite;

        uint32_t mSpeechParamAckCount[NUM_SPEECH_PARAM_ACK_TYPE];


        uint32_t mModemSideModemStatus; // value |= modem_status_mask_t

        uint16_t mWaitAckMessageID;


        AudioLock mCCCIMessageQueueMutex;
        AudioLock mA2MShareBufMutex;
        AudioLock mSetSpeechParamMutex;

        pthread_t hReadThread;
        pthread_t hSendSphThread;

        bool mIsModemResetDuringPhoneCall;

        SpeechMessengerCCCI() {}
};


} // end namespace android

#endif // end of ANDROID_SPEECH_MESSAGE_CCCI_H
