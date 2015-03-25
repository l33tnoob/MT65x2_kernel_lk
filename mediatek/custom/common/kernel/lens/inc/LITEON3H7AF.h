#ifndef _LITEON3H7AF_H
#define _LITEON3H7AF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define LITEON3H7AF_MAGIC 'A'
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
} stLITEON3H7AF_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define LITEON3H7AFIOC_G_MOTORINFO _IOR(LITEON3H7AF_MAGIC,0,stLITEON3H7AF_MotorInfo)

#define LITEON3H7AFIOC_T_MOVETO _IOW(LITEON3H7AF_MAGIC,1,unsigned long)

#define LITEON3H7AFIOC_T_SETINFPOS _IOW(LITEON3H7AF_MAGIC,2,unsigned long)

#define LITEON3H7AFIOC_T_SETMACROPOS _IOW(LITEON3H7AF_MAGIC,3,unsigned long)

#else
#endif
