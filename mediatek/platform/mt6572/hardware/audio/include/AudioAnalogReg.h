#ifndef ANDROID_AUDIO_ANALOGREG_H
#define ANDROID_AUDIO_ANALOGREG_H

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

// AudioAnalogReg only provide basic funciton to set register ,
// other function should be move to other module to control this class.
#include "AudioType.h"
#include "AudioDef.h"
#include "AudioUtility.h"
#include "AudioIoctl.h"


//---------------digital pmic  register define -------------------------------------------
#define AFE_PMICDIG_AUDIO_BASE        (0x4000)

#define ABB_AFE_CON0             (AFE_PMICDIG_AUDIO_BASE + 0x0000)
#define ABB_AFE_CON1             (AFE_PMICDIG_AUDIO_BASE + 0x0002)
#define ABB_AFE_CON2             (AFE_PMICDIG_AUDIO_BASE + 0x0004)
#define ABB_AFE_CON3             (AFE_PMICDIG_AUDIO_BASE + 0x0006)
#define ABB_AFE_CON4             (AFE_PMICDIG_AUDIO_BASE + 0x0008)
#define ABB_AFE_CON5             (AFE_PMICDIG_AUDIO_BASE + 0x000A)
#define ABB_AFE_CON6             (AFE_PMICDIG_AUDIO_BASE + 0x000C)
#define ABB_AFE_CON7             (AFE_PMICDIG_AUDIO_BASE + 0x000E)
#define ABB_AFE_CON8             (AFE_PMICDIG_AUDIO_BASE + 0x0010)
#define ABB_AFE_CON9             (AFE_PMICDIG_AUDIO_BASE + 0x0012)
#define ABB_AFE_CON10            (AFE_PMICDIG_AUDIO_BASE + 0x0014)
#define ABB_AFE_CON11            (AFE_PMICDIG_AUDIO_BASE + 0x0016)
#define ABB_AFE_STA0             (AFE_PMICDIG_AUDIO_BASE + 0x0018)
#define ABB_AFE_STA1             (AFE_PMICDIG_AUDIO_BASE + 0x001A)
#define ABB_AFE_STA2             (AFE_PMICDIG_AUDIO_BASE + 0x001C)
#define AFE_UP8X_FIFO_CFG0       (AFE_PMICDIG_AUDIO_BASE + 0x001E)
#define AFE_UP8X_FIFO_LOG_MON0   (AFE_PMICDIG_AUDIO_BASE + 0x0020)
#define AFE_UP8X_FIFO_LOG_MON1   (AFE_PMICDIG_AUDIO_BASE + 0x0022)
#define AFE_PMIC_NEWIF_CFG0      (AFE_PMICDIG_AUDIO_BASE + 0x0024)
#define AFE_PMIC_NEWIF_CFG1      (AFE_PMICDIG_AUDIO_BASE + 0x0026)
#define AFE_PMIC_NEWIF_CFG2      (AFE_PMICDIG_AUDIO_BASE + 0x0028)
#define AFE_PMIC_NEWIF_CFG3      (AFE_PMICDIG_AUDIO_BASE + 0x002A)
#define ABB_AFE_TOP_CON0        (AFE_PMICDIG_AUDIO_BASE + 0x002C)
#define ABB_MON_DEBUG0           (AFE_PMICDIG_AUDIO_BASE + 0x002E)

//---------------digital pmic  register define end ---------------------------------------

//---------------analog pmic  register define start --------------------------------------
#define AFE_PMICANA_AUDIO_BASE        (0x0)

#define TOP_CKPDN0                  (AFE_PMICANA_AUDIO_BASE + 0x102)
#define TOP_CKPDN0_SET              (AFE_PMICANA_AUDIO_BASE + 0x104)
#define TOP_CKPDN0_CLR              (AFE_PMICANA_AUDIO_BASE + 0x106)
#define TOP_CKPDN1                  (AFE_PMICANA_AUDIO_BASE + 0x108)
#define TOP_CKPDN1_SET              (AFE_PMICANA_AUDIO_BASE + 0x10A)
#define TOP_CKPDN1_CLR              (AFE_PMICANA_AUDIO_BASE + 0x10C)
#define TOP_CKPDN2                  (AFE_PMICANA_AUDIO_BASE + 0x10E)
#define TOP_CKPDN2_SET              (AFE_PMICANA_AUDIO_BASE + 0x110)
#define TOP_CKPDN2_CLR              (AFE_PMICANA_AUDIO_BASE + 0x112)
#define TOP_CKCON1                  (AFE_PMICANA_AUDIO_BASE + 0x126)

#define SPK_CON0                    (AFE_PMICANA_AUDIO_BASE + 0x052)
#define SPK_CON1                    (AFE_PMICANA_AUDIO_BASE + 0x054)
#define SPK_CON2                    (AFE_PMICANA_AUDIO_BASE + 0x056)
#define SPK_CON6                    (AFE_PMICANA_AUDIO_BASE + 0x05E)
#define SPK_CON7                    (AFE_PMICANA_AUDIO_BASE + 0x060)
#define SPK_CON8                    (AFE_PMICANA_AUDIO_BASE + 0x062)
#define SPK_CON9                    (AFE_PMICANA_AUDIO_BASE + 0x064)
#define SPK_CON10                   (AFE_PMICANA_AUDIO_BASE + 0x066)
#define SPK_CON11                   (AFE_PMICANA_AUDIO_BASE + 0x068)
#define SPK_CON12                   (AFE_PMICANA_AUDIO_BASE + 0x06A)

#define AUDTOP_CON0                 (AFE_PMICANA_AUDIO_BASE + 0x700)
#define AUDTOP_CON1                 (AFE_PMICANA_AUDIO_BASE + 0x702)
#define AUDTOP_CON2                 (AFE_PMICANA_AUDIO_BASE + 0x704)
#define AUDTOP_CON3                 (AFE_PMICANA_AUDIO_BASE + 0x706)
#define AUDTOP_CON4                 (AFE_PMICANA_AUDIO_BASE + 0x708)
#define AUDTOP_CON5                 (AFE_PMICANA_AUDIO_BASE + 0x70A)
#define AUDTOP_CON6                 (AFE_PMICANA_AUDIO_BASE + 0x70C)
#define AUDTOP_CON7                 (AFE_PMICANA_AUDIO_BASE + 0x70E)
#define AUDTOP_CON8                 (AFE_PMICANA_AUDIO_BASE + 0x710)
#define AUDTOP_CON9                 (AFE_PMICANA_AUDIO_BASE + 0x712)


//---------------analog pmic  register define end ---------------------------------------

namespace android
{

class AudioAnalogReg
{
    public:
        static AudioAnalogReg *getInstance();
        status_t SetAnalogReg(uint32 offset, uint32 value, uint32 mask);
        uint32 GetAnalogReg(uint32 offset);

    private:
        /**
        * AudioAnalogReg contructor .
        * use private constructor to achieve single instance
        */
        AudioAnalogReg();
        ~AudioAnalogReg();
        /**
        * a basic function to check regiseter range
        * @param offset
        * @return bool
        */
        bool CheckAnaRegRange(uint32 offset);

        /**
        * a private variable.
        * single instance to thois class
        */
        static AudioAnalogReg *UniqueAnalogRegInstance;

        AudioAnalogReg(const AudioAnalogReg &);             // intentionally undefined
        AudioAnalogReg &operator=(const AudioAnalogReg &);  // intentionally undefined

        /**
        * a loca varible for operation regiseter settingsAllowed
        */
        Register_Control mReg_Control;

        /**
        * a private variable.
        * file descriptor to open audio driver
        */
        int mFd;
};

}

#endif
