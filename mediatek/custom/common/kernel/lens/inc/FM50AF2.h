#ifndef _FM50AF2_H
#define _FM50AF2_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define FM50AF2_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
//Support SR?
bool          bIsSupportSR;
} stFM50AF2_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define FM50AF2IOC_G_MOTORINFO _IOR(FM50AF2_MAGIC,0,stFM50AF2_MotorInfo)

#define FM50AF2IOC_T_MOVETO _IOW(FM50AF2_MAGIC,1,unsigned long)

#define FM50AF2IOC_T_SETINFPOS _IOW(FM50AF2_MAGIC,2,unsigned long)

#define FM50AF2IOC_T_SETMACROPOS _IOW(FM50AF2_MAGIC,3,unsigned long)

#else
#endif
