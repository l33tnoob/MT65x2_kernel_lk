

#ifndef _SHOW_LOGO_LOG_H
#define _SHOW_LOGO_LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef  BUILD_LK

#include <debug.h>
#include <lib/zlib.h>

#ifndef  LOG_ANIM
#define  LOG_ANIM(x...)     dprintf(INFO, x)

#endif

#else

#include <cutils/xlog.h>
#include "zlib.h"

#ifndef  LOG_ANIM
#define  LOG_ANIM(x...)      XLOGD(x)

#endif

#endif


        
    
#ifdef __cplusplus
}
#endif
#endif
