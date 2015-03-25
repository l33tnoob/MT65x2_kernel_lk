#ifndef _MSDC_TYPES_H_
#define _MSDC_TYPES_H_

#include <platform/mt_typedefs.h>
#include <platform/mt_reg_base.h>

#ifndef s8
typedef signed char         s8;
#endif
#ifndef u8
//typedef unsigned char       u8;
#endif

#ifndef s16
typedef signed short        s16;
#endif
#ifndef u16
//typedef unsigned short      u16;
#endif

#ifndef s32
typedef signed int          s32;
#endif
#ifndef u32
//typedef unsigned int        u32;
#endif

#ifndef s64
typedef signed long long    s64;
#endif
#ifndef u64
//typedef unsigned long long  u64;
#endif

/* bsd */
#ifndef u_char
//typedef unsigned char		u_char;
#endif
#ifndef u_short
//typedef unsigned short		u_short;
#endif
#ifndef u_int
//typedef unsigned int		u_int;
#endif
#ifndef u_long
//typedef unsigned long		u_long;
#endif

/* sysv */
#ifndef unchar
typedef unsigned char		unchar;
#endif
#ifndef uchar
//typedef unsigned char		uchar;
#endif
#ifndef ushort
//typedef unsigned short		ushort;
#endif
#ifndef uint
//typedef unsigned int		uint;
#endif
#ifndef ulong
//typedef unsigned long		ulong;
#endif

#ifndef uint8
typedef unsigned char       uint8;
#endif
#ifndef uint16
typedef unsigned short      uint16;
#endif
#ifndef uint32
typedef unsigned int        uint32;
#endif
#ifndef int8
typedef signed char         int8;
#endif
#ifndef int16
typedef signed short        int16;
#endif
#ifndef int32
typedef signed int          int32;
#endif

#endif /* _MSDC_TYPES_H_ */

