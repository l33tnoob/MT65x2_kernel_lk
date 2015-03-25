/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/


#ifndef _OSAL_TYPEDEF_H_
#define _OSAL_TYPEDEF_H_

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/err.h>
#ifdef WMT_PLAT_ALPS
#include <linux/aee.h>
#endif
#include <linux/kfifo.h>
#include <linux/wakelock.h>
#include <linux/log2.h>

typedef void VOID;
typedef void *PVOID;

typedef char CHAR;
typedef char *PCHAR;
typedef signed char INT8;
typedef signed char *PINT8;
typedef unsigned char UINT8;
typedef unsigned char *PUINT8;
typedef unsigned char UCHAR;
typedef unsigned char  *PUCHAR;

typedef signed short INT16;
typedef signed short *PINT16;
typedef unsigned short UINT16;
typedef unsigned short *PUINT16;

typedef signed long LONG;
typedef signed long *PLONG;

typedef signed int INT32;
typedef signed int *PINT32;
typedef unsigned int UINT32;
typedef unsigned int *PUINT32;

typedef unsigned long ULONG;
typedef unsigned long  *PULONG;

typedef int MTK_WCN_BOOL;
#ifndef MTK_WCN_BOOL_TRUE
#define MTK_WCN_BOOL_FALSE               ((MTK_WCN_BOOL) 0)
#define MTK_WCN_BOOL_TRUE                ((MTK_WCN_BOOL) 1)
#endif

#endif /*_OSAL_TYPEDEF_H_*/

