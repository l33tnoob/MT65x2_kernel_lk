
 
 
/*! \file DX_VOS_config.h
    \brief This file defines the options for using 64bits variables
*/


#ifndef _DX_VOS_CONFIG_H
#define _DX_VOS_CONFIG_H

#ifndef __FUNCTION__
#define __FUNCTION__ ""
#endif

#include <stdarg.h>
#ifdef DX_NO_THREAD_SUPPORT
#define DX_VOS_THREAD_RESOLUTION    1
#else
#define DX_VOS_THREAD_RESOLUTION    256
#endif
#define LINUX 
#define LINUX_SYSCALL_SUCCESS 0
#define LINUX_SYSCALL_ERROR -1
#define DX_VOS_PATH_DIVIDER		"/" 

#define DX_VOS_MAX_PATH 4096

#if !defined(LITTLE__ENDIAN) && !defined(BIG__ENDIAN)
#define LITTLE__ENDIAN
#endif

typedef va_list DX_VA_LIST;


#define DX_VA_START(ap,v)  va_start(ap,v) 
#define DX_VA_ARG(ap,t)    ((sizeof(t)<4)?(t)va_arg(ap, int):(t)va_arg(ap, t))  
#define DX_VA_END(ap)      va_end(ap)

// TODO: temoporary for compiation, sort out 64 bit issues on linux build
#ifndef HASLONGLONG
#define HASLONGLONG
#endif


#ifndef IMPORT_C
#define IMPORT_C
#endif
#ifndef EXPORT_C
#define EXPORT_C
#endif

#ifdef DX_USE_LEGACY_VOS

#define DX_VOS_DEFAULT_ROOT		"~/Test" 
#define DX_VOS_DRM_ROOT_PATH	"~/private/DX"
#define DX_VOS_DRM_PUBLIC_ROOT_PATH	"~/shared/DX"

#endif
#endif /* ifndef _DX_VOS_CONFIG_H*/
