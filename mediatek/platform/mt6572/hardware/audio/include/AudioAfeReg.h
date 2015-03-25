#ifndef ANDROID_AUDIO_AFEREG_H
#define ANDROID_AUDIO_AFEREG_H

// AudioAfeReg only provide basic funciton to set register ,
// other function should be move to other module to control this class.

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#include "AudioType.h"
#include "AudioDef.h"
#include "AudioUtility.h"
#include "AudioIoctl.h"

//---------- register define ------------------------------------------------------------
#define AUDIO_TOP_CON0  (0x0000)
#define AUDIO_TOP_CON1  (0x0004)
#define AUDIO_TOP_CON2  (0x0008)
#define AUDIO_TOP_CON3  (0x000c)
#define AFE_DAC_CON0    (0x0010)
#define AFE_DAC_CON1    (0x0014)
#define AFE_I2S_CON     (0x0018)


#define AFE_CONN0       (0x0020)
#define AFE_CONN1       (0x0024)
#define AFE_CONN2       (0x0028)
#define AFE_CONN3       (0x002c)
#define AFE_CONN4       (0x0030)

#define AFE_I2S_CON1    (0x0034)
#define AFE_I2S_CON2    (0x0038)


#define AFE_DL1_BASE    (0x0040)
#define AFE_DL1_CUR     (0x0044)
#define AFE_DL1_END     (0x0048)
#define AFE_I2S_CON3    (0x004c)
#define AFE_DL2_BASE     (0x0050)
#define AFE_DL2_CUR     (0x0054)
#define AFE_DL2_END     (0x0058)
#define AFE_AWB_BASE    (0x0070)
#define AFE_AWB_CUR     (0x0078)
#define AFE_AWB_END     (0x007c)
#define AFE_VUL_BASE    (0x0080)
#define AFE_VUL_CUR     (0x0088)
#define AFE_VUL_END     (0x008c)

#define AFE_MEMIF_MON0  (0x00D0)
#define AFE_MEMIF_MON1  (0x00D4)
#define AFE_MEMIF_MON2  (0x00D8)
#define AFE_MEMIF_MON4  (0x00E0)

#define AFE_ADDA_DL_SRC2_CON0   (0x00108)
#define AFE_ADDA_DL_SRC2_CON1   (0x0010C)
#define AFE_ADDA_UL_SRC_CON0    (0x00114)
#define AFE_ADDA_UL_SRC_CON1    (0x00118)
#define AFE_ADDA_TOP_CON0       (0x00120)
#define AFE_ADDA_UL_DL_CON0     (0x00124)
#define AFE_ADDA_SRC_DEBUG      (0x0012C)
#define AFE_ADDA_SRC_DEBUG_MON0 (0x00130)
#define AFE_ADDA_SRC_DEBUG_MON1 (0x00134)
#define AFE_ADDA_NEWIF_CFG0     (0x00138)
#define AFE_ADDA_NEWIF_CFG1     (0x0013C)



#define AFE_SIDETONE_DEBUG   (0x01D0)
#define AFE_SIDETONE_MON     (0x01D4)
#define AFE_SIDETONE_CON0    (0x01E0)
#define AFE_SIDETONE_COEFF   (0x01E4)
#define AFE_SIDETONE_CON1    (0x01E8)
#define AFE_SIDETONE_GAIN    (0x01EC)
#define AFE_SGEN_CON0         (0x01F0)

#define AFE_TOP_CON0          (0x0200)

#define AFE_PREDIS_CON0       (0x0260)
#define AFE_PREDIS_CON1       (0x0264)

#define AFE_MOD_PCM_BASE      (0x0330)
#define AFE_MOD_PCM_END       (0x0338)
#define AFE_MOD_PCM_CUR       (0x033c)
#define AFE_IRQ_MCU_CON       (0x03A0)
#define AFE_IRQ_MCU_STATUS    (0x03A4)
#define AFE_IRQ_CLR           (0x03A8)
#define AFE_IRQ_MCU_CNT1      (0x03AC)
#define AFE_IRQ_MCU_CNT2      (0x03B0)
#define AFE_IRQ_MCU_MON2      (0x03B8)

#define AFE_IRQ1_MCN_CNT_MON   (0x03C0)
#define AFE_IRQ2_MCN_CNT_MON   (0x03C4)
#define AFE_IRQ1_MCU_EN_CNT_MON (0x03c8)

#define AFE_MEMIF_MINLEN        (0x03D0)
#define AFE_MEMIF_MAXLEN        (0x03D4)
#define AFE_MEMIF_PBUF_SIZE     (0x03D8)

#define AFE_GAIN1_CON0          (0x0410)
#define AFE_GAIN1_CON1          (0x0414)
#define AFE_GAIN1_CON2          (0x0418)
#define AFE_GAIN1_CON3          (0x041C)
#define AFE_GAIN1_CONN          (0x0420)
#define AFE_GAIN1_CUR           (0x0424)
#define AFE_GAIN2_CON0          (0x0428)
#define AFE_GAIN2_CON1          (0x042C)
#define AFE_GAIN2_CON2          (0x0430)
#define AFE_GAIN2_CON3          (0x0434)
#define AFE_GAIN2_CONN          (0x0438)
#define AFE_GAIN2_CUR             (0x043c)
#define AFE_GAIN2_CONN2          (0x0440)


// only valid in FPGA
#define FPGA_CFG2               (0x004B8)
#define FPGA_CFG3               (0x004BC)
#define FPGA_CFG0               (0x04C0)
#define FPGA_CFG1               (0x04C4)

#define FPGA_STC                (0x04CC)


#define AFE_ASRC_CON0           (0x0500)
#define AFE_ASRC_CON1           (0x0504)
#define AFE_ASRC_CON2           (0x0508)
#define AFE_ASRC_CON3           (0x050C)
#define AFE_ASRC_CON4           (0x0510)
#define AFE_ASRC_CON5           (0x0514)
#define AFE_ASRC_CON6           (0x0518)
#define AFE_ASRC_CON7           (0x051c)
#define AFE_ASRC_CON8           (0x0520)
#define AFE_ASRC_CON9           (0x0524)
#define AFE_ASRC_CON10          (0x0528)
#define AFE_ASRC_CON11          (0x052c)
#define PCM_INTF_CON1           (0x0530)
#define PCM_INTF_CON2           (0x0538)
#define PCM2_INTF_CON           (0x053C)

#define AFE_ASRC_CON13  (0x0550)
#define AFE_ASRC_CON14  (0x0554)
#define AFE_ASRC_CON15  (0x0558)
#define AFE_ASRC_CON16  (0x055C)
#define AFE_ASRC_CON17  (0x0560)
#define AFE_ASRC_CON18  (0x0564)
#define AFE_ASRC_CON19  (0x0568)
#define AFE_ASRC_CON20  (0x056C)
#define AFE_ASRC_CON21  (0x0570)

#define AFE_REGISTER_OFFSET (0x574)

#define AFE_MASK_ALL  (0xffffffff)
//-----------register define end

namespace android
{

//!  A AFE register setting  class.
/*!
  this class is hold only the operation digital domain registers
  other complicated should move to higher layer.
*/
class AudioAfeReg
{
    public:
        /**
        * AudioAfeReg getinstance.
        * setting a regiseter need to call getinstance to get pointer AudioAfeReg
        */
        static AudioAfeReg *getInstance();
        /**
        * a basic function to set afe regiseter
        * @param offset an offset of afe register
        * @param value value want to set to register
        * @param mask mask with 32 or 16 bits regiseter
        * @see GetAfeReg()
        * @return The state of set regiseter
        */
        status_t SetAfeReg(uint32 offset, uint32 value, uint32 mask);

        /**
        * a basic function to Get afe regiseter
        * @param offset an offset of afe register
        * @see SetAfeReg()
        * @return The value regiseter want to get
        */
        uint32 GetAfeReg(uint32 offset);

        /**
        * to get audio drvier file descriptor
        * @return Fd
        */
        uint32 GetAfeFd();

    private:
        /**
        * AudioAfeReg contructor .
        * use private constructor to achieve single instance
        */
        AudioAfeReg();
        ~AudioAfeReg();
        /**
        * a basic function to check regiseter range
        * @param offset
        * @return bool
        */
        bool CheckRegRange(uint32 offset);
        /**
        * a private variable.
        * file descriptor to open audio driver
        */
        int mFd;

        /**
        * a loca varible for operation regiseter setting
        */
        Register_Control mReg_Control;
        /**
        * a private variable.
        * single instance to thois class
        */
        static AudioAfeReg *UniqueAfeRegInstance;

        /**
        * AudioAfeReg
        * prevent to copy this object
        */
        AudioAfeReg(const AudioAfeReg &);             // intentionally undefined
        AudioAfeReg &operator=(const AudioAfeReg &);  // intentionally undefined
};

}

#endif
