// Description: ch7033-35 base type definition 

//------------------ type definition --------------------------
#ifndef	_TYPE_REDEFINITION_
#define _TYPE_REDEFINITION_

typedef unsigned char			uint8;
typedef signed char				int8;
typedef unsigned short			uint16;
typedef signed short			int16;
#ifdef __GNUC__
typedef unsigned int			uint32;
typedef signed int				int32;
typedef unsigned long long		uint64;
typedef signed long long		int64;
#else
#if _MSC_VER > 800
typedef unsigned int			uint32;
typedef signed int				int32;
typedef unsigned __int64		uint64;
typedef signed __int64			int64;
#else
typedef unsigned long			uint32;
typedef signed long				int32;
#endif
#endif
typedef int						ch_bool;
#define ch_true					1
#define ch_false				0

#endif
