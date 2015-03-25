
 
 #ifndef _DX_VOS_TIME_H
#define _DX_VOS_TIME_H

/*! \file DX_VOS_TIME.h
    \brief This file contains standard time operations 
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************** Typedefs ****************************/

typedef DxInt32 DxTime_t;

typedef struct {
    DxUint32 tm_year;    /*!< full year */
    DxUint32 tm_mon;     /*!< months since January - [1 - 12] */
    DxUint32 tm_mday;    /*!< day of the month - [1,31] */
    DxUint32 tm_hour;    /*!< hours since midnight - [0,23] */
    DxUint32 tm_min;     /*!< minutes after the hour - [0,59] */
	DxUint32 tm_sec;     /*!< seconds after the minute - [0,59] */
} DxTimeStruct_t;

/*! returns the number of seconds passed from 1/1/1970 00:00 UTC. 
	In case of error 0 will be returned.
*/
DxTime_t DX_VOS_GetTime(void);

/*! Converts time defined in seconds to a struct which contains date/time info.
	Time is adjusted to local time using the system time zone definition.
	\return 
	- DX_SUCCESS on success
	- DX_BAD_ARGUMENTS - if aTimeStruct is null.
	- DX_TIME_ERROR - if aTime indicates invalid time or 0.
	*/
DxStatus DX_VOS_GetLocalTime(DxTime_t aTime, DxTimeStruct_t* aTimeStruct); 

/*! Converts time defined in seconds to a struct which contains date/time info.
	Time is NOT adjusted to local time using the system time zone definition. 
	\return 
	- DX_SUCCESS on success
	- DX_BAD_ARGUMENTS - if aTimeStruct is null.
	- DX_TIME_ERROR - if aTime indicates invalid time or 0.
*/
DxStatus DX_VOS_GetGlobalTime(DxTime_t aTime, DxTimeStruct_t* aTimeStruct); 

/*! Converts time defined struct to the time in seconds from 1/1/1970
	Time is NOT adjusted to local time using the system time zone definition. 
	\return 
	- DX_SUCCESS on success
	- DX_BAD_ARGUMENTS - if one of the parameters is null.
	- DX_TIME_ERROR - if aTimeStruct indicates invalid time or 0.
*/
DxStatus DX_VOS_GlobalTimeToSecs(const DxTimeStruct_t* aTimeStruct, DxTime_t* aTime); 

/*! Converts time defined struct to the time in seconds from 1/1/1970.
For any time before 1970 aTime will be 0, and after 2037 aTime will be 0xFFFFFFFF.
Time is NOT adjusted to local time using the system time zone definition. 
\return 
- DX_SUCCESS on success
- DX_BAD_ARGUMENTS - if one of the parameters is null.
- DX_TIME_ERROR - if aTimeStruct indicates invalid time or 0.
*/
DxStatus DX_VOS_SafeGlobalTimeToSecs(const DxTimeStruct_t* aTimeStruct, DxTime_t* aTime);

int DX_VOS_TimeCompare(const DxTimeStruct_t* time1, const DxTimeStruct_t* time2);
/*! Converts time defined struct to the time in seconds from 1/1/1970
\return 
- DX_SUCCESS on success
- DX_BAD_ARGUMENTS - if one of the parameters is null.
- DX_TIME_ERROR - if aTimeStruct indicates invalid time or 0.
*/
DxStatus DX_VOS_LocalTimeToSecs(const DxTimeStruct_t* aTimeStruct, DxTime_t* aTime); 

#define DX_VOS_SIZEOF_TIMESTAMP_STR 32

/* Fills the timeStampBuffer with a NULL terminated string that represents the 
   current time. Time stamp format is "YYYYMMDD_HHMMSS".
*/
DxStatus DX_VOS_GetTimeStamp(DxChar* timeStamp, DxUint32 timeStampSize);

/*! returns number of milliseconds since the device was booted.
	 
	\note This value wraps around about every 50 days. 
	Suitable for duration measurements only.
	*/
DxUint32 DX_VOS_GetTickCount(void);

/*! returns number of high resolution ticks since the device was booted.
	The ticks resolution can be obtained by calling DX_VOS_GetHighResTicksPerSecond().
	This allows applications to measure duration in the maximum accuracy that the device allows. 
	\note This value may wraps around frequently. Suitable for duration measurements only.
	*/
DxUint32 DX_VOS_GetHighResTickCount(void);


/*! returns the number of high resolution ticks per second. */
DxUint32 DX_VOS_GetHighResTicksPerSecond(void);

#ifdef DX_USE_LEGACY_VOS
#include "DX_VOS_TimeUtils.h"
#endif
#ifdef __cplusplus
}
#endif

#endif /* ifndef _DX_VOS_TIME_H */









