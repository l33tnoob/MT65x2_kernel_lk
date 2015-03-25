
 
 #ifndef DX_TYPES_H
#define DX_TYPES_H

#define DX_NULL 				0

//! Unsigned integer type. Its size is platform dependent.
typedef unsigned int       		DxUint;
//! Unsigned value of 8 bits
typedef unsigned char        	DxUint8;
//! Unsigned value of 16 bits
typedef unsigned short       	DxUint16;
//! Unsigned value of 32 bits
typedef unsigned long        	DxUint32;
//! Unsigned value of 64 bits
typedef unsigned long long		DxUint64;

//! Signed integer type. Its size is platform dependent.
typedef int                   	DxInt;
//! Signed value of 8 bits
typedef char		          	DxInt8;
//! Signed value of 16 bits
typedef short               	DxInt16;
//! Signed value of 32 bits
typedef long          	        DxInt32;
//! Signed value of 64 bits
typedef long long			    DxInt64;
//! Real number. Its size is platform dependent.
typedef float			        DxFloat;

//! Character of 8 bits
typedef char			        DxChar;
//! Wide character of 16 bits
typedef DxUint16 				DxWideChar;
//! Wide character of 32 bits
typedef DxUint32				DxWideChar32;
//! Byte value
typedef DxUint8                 DxByte ;

//! Enumeration of true and false. DX_FALSE = 0, DX_TRUE = 1
typedef enum {
    DX_FALSE = 0,
    DX_TRUE = 1
} EDxBool;

//! Boolean value. Represented by 32 bits
typedef DxUint32                DxBool;
//! Unsigned value of 32 bits used for return codes.
typedef DxUint32	 		    DxStatus;

#define DX_SUCCESS			    0UL 
#define DX_INFINITE			    0xFFFFFFFF
#define DX_INVALID_VALUE	    0xFFFFFFFF
#define DX_INFINITE64			0xFFFFFFFFFFFFFFFFULL
#define DX_INVALID_VALUE64	    0xFFFFFFFFFFFFFFFFULL

#endif
