#ifndef _AUDIO_DIGITAL_TYPE_H
#define _AUDIO_DIGITAL_TYPE_H
#include "AudioType.h"

/*!
 *     AudioDigitalType is a public class to let user to use of defined enum
 */

class AudioDigitalType
{
    public:

        enum Digital_Block
        {
            // memmory interfrace
            MEM_DL1,
            MEM_DL2,
            MEM_VUL,
            MEM_DAI,
            MEM_I2S,  // currently no use
            MEM_AWB,
            MEM_MOD_DAI,
            // connection to int main modem
            MODEM_PCM_1_O ,
            // connection to extrt/int modem
            MODEM_PCM_2_O ,
            // 1st I2S for DAC and ADC
            I2S_OUT_DAC ,
            I2S_IN_ADC ,
            // 2nd I2S
            I2S_OUT_2 ,
            I2S_IN_2 ,
            // HW gain contorl
            HW_GAIN1,
            HW_GAIN2,
            // megrge interface
            MRG_I2S_OUT,
            MRG_I2S_IN,
            DAI_BT,
            NUM_OF_DIGITAL_BLOCK,
            NUM_OF_MEM_INTERFACE = MEM_MOD_DAI + 1
        };


        enum MemIF_Direction
        {
            DIRECTION_OUTPUT,
            DIRECTION_INPUT
        };

        enum InterConnectionInput
        {
            I00,
            I01,
            I02,
            I03,
            I04,
            I05,
            I06,
            I07,
            I08,
            I09,
            I10,
            I11,
            I12,
            I13,
            I14,
            Num_Input
        };

        enum InterConnectionOutput
        {
            O00,
            O01,
            O02,
            O03,
            O04,
            O05,
            O06,
            O07,
            O08,
            O09,
            O10,
            O11,
            O12,
            O13,
            O14,
            O15,
            O16,
            O17,
            O18,
            Num_Output
        };

        enum InterConnectionState
        {
            DisConnect = 0x0,
            Connection = 0x1,
            ConnectionShift = 0x2
        };

        enum TopClockType
        {
            APB_CLOCK = 1,
            AFE_CLOCK = 2,
            I2S_INPUT_CLOCK = 6,
            AFE_CK_DIV_RRST = 16,
            PDN_APLL_TUNER  = 19,
            PDN_HDMI_CK     = 20,
            PDN_SPDIF_CK    = 21
        };

        enum AFEClockType
        {
            AFE_ON = 0,
            DL1_ON = 1,
            DL2_ON = 2,
            VUL_ON = 3,
            DAI_ON = 4,
            I2S_ON = 5,
            AWB_ON = 6,
            MOD_PCM_ON = 7
        };

        enum IRQ_MCU_MODE
        {
            IRQ1_MCU_MODE = 0,
            IRQ2_MCU_MODE,
            IRQ3_MCU_MODE,
            NUM_OF_IRQ_MODE
        };
        enum Hw_Digital_Gain
        {
            HW_DIGITAL_GAIN1,
            HW_DIGITAL_GAIN2
        };

        enum OUTPUT_DATA_FORMAT
        {
            OUTPUT_DATA_FORMAT_16BIT = 0,
            OUTPUT_DATA_FORMAT_24BIT
        };
};

class AudioDigtalI2S
{
    public:
        enum I2S_IN_PAD_SEL
        {
            I2S_IN_FROM_CONNSYS = 0,
            I2S_IN_FROM_IO_MUX = 1
        };

        enum LR_SWAP
        {
            NO_SWAP = 0,
            LR_DATASWAP = 1
        };

        enum I2S_HD_EN
        {
            NORMAL_CLOCK = 0,
            LOW_JITTER_CLOCK = 1
        };

        enum INV_LRCK
        {
            NO_INVERSE = 0,
            INVESE_LRCK = 1
        };

        enum I2S_FORMAT
        {
            EIAJ = 0,
            I2S  = 1
        };

        enum I2S_SRC
        {
            MASTER_MODE = 0,
            SLAVE_MODE = 1
        };

        enum I2S_WLEN
        {
            WLEN_16BITS = 0 ,
            WLEN_32BITS = 1
        };

        enum I2S_SAMPLERATE
        {
            I2S_8K = 0,
            I2S_11K = 1,
            I2S_12K = 2,
            I2S_16K = 4,
            I2S_22K = 5,
            I2S_24K = 6,
            I2S_32K = 8,
            I2S_44K = 9,
            I2S_48K = 10
        };

        bool mLR_SWAP;
        bool mI2S_HD_EN;
        bool mI2S_SLAVE;
        int mI2S_SAMPLERATE;
        bool mINV_LRCK;
        bool mI2S_FMT;
        bool mI2S_WLEN;
        bool mI2S_EN;
        bool mI2S_IN_PAD_SEL;

        // her for ADC usage , DAC will not use this
        int mBuffer_Update_word;
        bool mloopback;
        bool mFpga_bit;
        bool mFpga_bit_test;
};

class AudioDigitalPCM
{
    public:
        enum TX_LCH_RPT
        {
            TX_LCH_NO_REPEAT = 0,
            TX_LCH_REPEAT    = 1
        };

        enum VBT_16K_MODE
        {
            VBT_16K_MODE_DISABLE = 0,
            VBT_16K_MODE_ENABLE  = 1
        };

        enum EXT_MODEM
        {
            MODEM_2_USE_INTERNAL_MODEM = 0,
            MODEM_2_USE_EXTERNAL_MODEM = 1
        };

        enum PCM_SYNC_TYPE
        {
            BCK_CYCLE_SYNC        = 0, // bck sync length = 1
            EXTEND_BCK_CYCLE_SYNC = 1  // bck sync length = PCM_INTF_CON[9:13]
        };

        enum BT_MODE
        {
            DUAL_MIC_ON_TX   = 0,
            SINGLE_MIC_ON_TX = 1
        };

        enum BYPASS_SRC
        {
            SLAVE_USE_ASRC       = 0, // slave mode & external modem uses different crystal
            SLAVE_USE_ASYNC_FIFO = 1  // slave mode & external modem uses the same crystal
        };

        enum PCM_CLOCK_SOURCE
        {
            MASTER_MODE = 0,
            SALVE_MODE  = 1
        };

        enum PCM_WLEN_LEN
        {
            PCM_16BIT = 0,
            PCM_32BIT = 1
        };

        enum PCM_MODE
        {
            PCM_MODE_8K  = 0,
            PCM_MODE_16K = 1
        };

        enum PCM_FMT
        {
            PCM_I2S    = 0,
            PCM_EIAJ   = 1,
            PCM_MODE_A = 2,
            PCM_MODE_B = 3
        };

        enum PCM_INV
        {
            PCM_NO_INVERSE = 0,
            PCM_INVERSE = 1
        };

        TX_LCH_RPT          mTxLchRepeatSel;
        VBT_16K_MODE        mVbt16kModeSel;
        EXT_MODEM           mExtModemSel;
        uint8_t             mExtendBckSyncLength;
        PCM_SYNC_TYPE       mExtendBckSyncTypeSel;
        BT_MODE             mSingelMicSel;
        BYPASS_SRC          mAsyncFifoSel;
        PCM_CLOCK_SOURCE    mSlaveModeSel;
        PCM_WLEN_LEN        mPcmWordLength;
        PCM_MODE            mPcmModeWidebandSel;
        PCM_FMT             mPcmFormat;
        PCM_INV       mPcmBckInInv;
        PCM_INV       mPcmSyncInInv;
        PCM_INV       mPcmBckOutInv;
        PCM_INV       mPcmSyncOutInv;
        bool                mModemPcmOn;
};

class AudioDigitalDAIBT
{
    public:
        enum BT_DAI_INPUT
        {
            FROM_BT,
            FROM_MGRIF
        };

        enum DATBT_MODE
        {
            Mode8K,
            Mode16K
        };

        enum DAI_DEL
        {
            HighWord,
            LowWord
        };

        enum BTSYNC
        {
            Short_Sync,
            Long_Sync
        };

        bool mUSE_MRGIF_INPUT;
        bool mDAI_BT_MODE;
        bool mDAI_DEL;
        int mBT_LEN;
        bool mDATA_RDY;
        bool mBT_SYNC;
        bool mBT_ON;
        bool mDAIBT_ON;
};

class AudioMrgIf
{
    public:
        enum MRFIF_I2S_SAMPLERATE
        {
            MRFIF_I2S_8K = 0,
            MRFIF_I2S_11K = 1,
            MRFIF_I2S_12K = 2,
            MRFIF_I2S_16K = 4,
            MRFIF_I2S_22K = 5,
            MRFIF_I2S_24K = 6,
            MRFIF_I2S_32K = 8,
            MRFIF_I2S_44K = 9,
            MRFIF_I2S_48K = 10
        };

        bool Mergeif_I2S_Enable;
        bool Merge_cnt_Clear;
        int Mrg_I2S_SampleRate;
        int Mrg_Sync_Dly;
        int Mrg_Clk_Edge_Dly;
        int Mrg_Clk_Dly;
        bool MrgIf_En;
};

// class for irq mode and counter.
class AudioIrqMcuMode
{
    public:
        unsigned int mStatus;  // on,off
        unsigned int mIrqMcuCounter;
        unsigned int mSampleRate;
};


#endif
