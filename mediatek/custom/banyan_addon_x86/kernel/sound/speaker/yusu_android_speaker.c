/*******************************************************************************
 *
 * Filename:
 * ---------
 * Yusu_android_speaker.c
 *
 * Project:
 * --------
 *   Yusu
 *
 * Description:
 * ------------
 *   seaker setting
 *
 * Author:
 * -------
 *   ChiPeng Chang (mtk02308)
 *
 *
 *------------------------------------------------------------------------------
 * $Revision$
 * $Modtime:$
 * $Log:$
 *
 * 06 12 2013 weiguo.li
 * [ALPS00793721] [HAL Violation][SC] File: /alps/mediatek/custom/banyan_addon_x86/kernel/sound/speaker/yusu_android_speaker.c
 * .
 *
 * 12 14 2011 weiguo.li
 * [ALPS00102848] [Need Patch] [Volunteer Patch] build waring in yusu_android_speaker.h
 * .
 *
 * 11 10 2011 weiguo.li
 * [ALPS00091610] [Need Patch] [Volunteer Patch]chang yusu_android_speaker.c function name and modules use it
 * .
 *
 * 09 28 2011 weiguo.li
 * [ALPS00076254] [Need Patch] [Volunteer Patch]LGE audio driver using Voicebuffer for incall
 * .
 *
 * 07 08 2011 weiguo.li
 * [ALPS00059378] poring lge code to alps(audio)
 * .
 *
 * 07 03 2010 chipeng.chang
 * [ALPS00002838][Need Patch] [Volunteer Patch] for speech volume step 
 * modify for headset customization.
 *
 *******************************************************************************/
#include "yusu_android_speaker.h"

bool Speaker_Init(void)
{
    return true;
}

bool Speaker_DeInit(void)
{
	return false;
}

void Sound_SpeakerL_SetVolLevel(int level)
{
   return; 
}

void Sound_SpeakerR_SetVolLevel(int level)
{
   return; 
}

void Sound_Speaker_Turnon(int channel)
{
   return; 
}

void Sound_Speaker_Turnoff(int channel)
{
   return; 
}

void Sound_Speaker_SetVolLevel(int level)
{
   return; 
}


void Sound_Headset_Turnon(void)
{
   return; 
}
void Sound_Headset_Turnoff(void)
{
   return; 
}

//kernal use
void AudioAMPDevice_Suspend(void)
{
   return; 
}
void AudioAMPDevice_Resume(void)
{
   return; 
}
void AudioAMPDevice_SpeakerLouderOpen(void)
{
	return ;

}
void AudioAMPDevice_SpeakerLouderClose(void)
{
   return; 
}
void AudioAMPDevice_mute(void)
{
   return; 
}

int Audio_eamp_command(unsigned int type, unsigned long args, unsigned int count)
{
	return 0;
}

