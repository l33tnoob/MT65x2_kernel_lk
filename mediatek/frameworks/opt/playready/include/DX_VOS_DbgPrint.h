
 
#ifndef _DX_VOS_DBG_PRINT_H
#define _DX_VOS_DBG_PRINT_H

/*! \file DX_VOS_DbgPrint.h
This module provides debug printing services.

The module is thread-safe. Every thread has its own debug file.
The debug file name is appended with the thread id.

The way a message is printed can be controlled by setting the value
of a series of internal variables (detailed later).

The printing should be done using the macros DX_DBG_PRINT0 - DX_DBG_PRINT8.
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DX_ERROR_PROPAGATION_LEVEL
#define DX_ERROR_PROPAGATION_LEVEL DX_DBGPRINT_ERROR_LEVEL
#endif
/*! Enum that defines predefined level values for common types of messages.
	The application may (but doesn't have to) use these values as the 
	debugLevel parameter of the printing macros. */
enum {
	DX_DBGPRINT_NOTHING_LEVEL = 0,
    DX_DBGPRINT_CRITICAL_ERROR_LEVEL = 3,
    DX_DBGPRINT_CRITICAL_INFO_LEVEL = 5,
    DX_DBGPRINT_ERROR_LEVEL = 10,
	DX_DBGPRINT_WARNING_LEVEL = 30 ,
	DX_DBGPRINT_INFO_LEVEL = 40,
    DX_DBGPRINT_ADDITIONAL_INFO_LEVEL = 50,
    DX_DBGPRINT_FULL_INFO_LEVEL = 60,
	DX_DBGPRINT_ALL = (DxInt32)0xFFFFFFFF
};

/*! This enum allocate bit ranges for the different Discretix modules. */
typedef enum {
	DX_DBG_MODULE_VOS = 0x0000000F,		/*!< 4 Modules types for VOS */
	DX_DBG_MODULE_CRYS = 0x00000030,	/*!< 2 Modules types for CRYS */
	DX_DBG_MODULE_SST = 0x000000C0,		/*!< 2 Modules types for SST */
	DX_DBG_MODULE_TOOLKIT = 0x00000F00,	/*!< 4 Modules types for Toolkits */
    DX_DBG_MODULE_FUSE = 0x0000FACE,		
    DX_DBG_MODULE_DRM = 0x000FF000,		/*!< 8 Modules types for DRM */
	DX_DBG_MODULE_MOBILETV = 0x07F00000,/*!< 7 Modules types for MobileTV */
    DX_DBG_MODULE_QA = 0x08000000,	    /*!< 1 Modules types for QA */
	DX_DBG_MODULE_USER = (DxInt32)0xF0000000	/*!< 4 Modules types for User application */
} EDxDbgPrintModules;

/*! The prototype of the basic printing function.
	The function should print the accepted parameters according to the user wish.
	The parameters of the function are already adjusted according to the global variables values. 
	*/
typedef void (*DxPrintfFunc_t)(
    DxUint32 moduleCode,  /*!< Code of the module from which the printing macro was called. */
    DxUint32 debugLevel,  /*!< Debug Level of the printed message. */
	const DxChar *fileName, /*!< Name of the file from which the printing macro was called.
								 If the value is NULL or empty fileName should not be printed. */
	DxUint32 lineNum,		/*!< Number of line from which the printing macro was called.
								 If the value is 0 the lineNum should not be printed. */
	const DxChar *funcName, /*!< Name of the function from which the printing macro was called.
								 If the value is NULL or empty the funcName should not be printed. */
	const DxChar* aDate,	/*!< String that represents the current date.
							 	 If the value is NULL or empty the date should not be printed. */
	const DxChar* aTime,	/*!< String that represents the current time.
								 If the value is NULL or empty the time should not be printed. */
	const DxChar *format,	/*!< message format string */
	DX_VA_LIST arg_list
	);

#ifdef DX_NO_DEBUG_PRINT
#undef DX_DEBUG_PRINT
#endif

#if defined(_DEBUG) && !defined(DX_NO_DEBUG_PRINT) && !defined(DX_DEBUG_PRINT)
#define DX_DEBUG_PRINT
#endif
#ifdef DX_DEBUG_PRINT

/*! Sets the value of the internal variable that indicates if file name should be printed. 
	Default value of this variable is TRUE. 
	\return old value of the internal variable. 
*/
DxBool DX_VOS_DbgPrint_SetShouldPrintFileName(DxBool value);

/*! Sets the value of the internal variable that indicates if line number should be printed. 
	Default value of this variable is TRUE. 
	\return old value of the internal variable. 
*/
DxBool DX_VOS_DbgPrint_SetShouldPrintLineNum(DxBool value);

/*! Sets the value of the internal variable that indicates if function name should be printed. 
	Default value is TRUE. 
	\return old value of the internal variable. 
*/
DxBool DX_VOS_DbgPrint_SetShouldPrintFuncName(DxBool value);

/*! Sets the value of the internal variable that indicates if time format string. 
	The format string may contain up to 3 occurrences of %d for hours, minutes & seconds respectively. 
	Default value of this variable is "%02d:%02d:%02d"
	If this value is set to NULL time will not be printed.
	\return old value of the internal variable. 
	*/
const DxChar* DX_VOS_DbgPrint_SetTimeFormat(const DxChar* value);

/*! Sets the value of the internal variable that indicates if date format string. 
	The format string may contain up to 3 occurrences of %d for day, month & year respectively. 
	Default value of this variable is "%02d.%02d.%02d"
	If this value is set to NULL date will not be printed.
	\return old value of the internal variable. 
*/
const DxChar* DX_VOS_DbgPrint_SetDateFormat(const DxChar* value);

/*! Sets the value of the internal variable that indicates the debug level threshold. 
	Only message whose debug level is above or equal to the threshold will be printed. 
	Default value of this variable is DX_DBG_PRINT_ALL (all messages are printed)
	\return old value of the internal variable. 
*/
DxUint32 DX_VOS_DbgPrint_SetDebugLevel(DxUint32 value);

/*! Sets the pointer to a basic print function. The module supplies 3 predefined print functions.
	The application may implement a new printing function and set the pointer to it.
	The application's function must follow the rules that are described in the DxPrintfFunc_t
	documentation.
	If function is set to NULL debug printing is disabled.
	\return old value of the internal pointer. 
*/
DxPrintfFunc_t DX_VOS_DbgPrint_SetPrintFunc(DxPrintfFunc_t value);

/*! Sets the value of the internal module mask. The module mask is provides to the ability to
	print only the messages of modules subset. Only the modules whose corresponding bit in the
	mask is on will be printed. Setting the value to 0xFFFFFFFF enables printing of all modules
	(if all other conditions apply) and setting the value to 0 disables all printing.
	Default value of the mask is 0xFFFFFFFF (all messages are printed).
	The module code of a certain message is the same as the value of the DX_DBG_MODULE macro
	that should be defined in the file where the message is written. If DX_DBG_MODULE is not defined
	in the file where the debug print is done then the code will not compile.
	If the application doesn't want to differentiate between the module it can define the value 
	of DX_DBG_MODULE in the scope of the whole application (in the project settings).
	\return old value of the internal variable. 
*/
DxUint32 DX_VOS_DbgPrint_SetModulesMask(DxUint32 value);

/*! Enables the printing of a specific module 
    \return old value of the internal variable. 
*/
DxUint32 DX_VOS_DbgPrint_EnableModulePrinting(DxUint32 ModuleCode);

/*! Disables the printing of a specific module 
    \return old value of the internal variable. 
*/
DxUint32 DX_VOS_DbgPrint_DisableModulePrinting(DxUint32 ModuleCode);

/*! Main printing function. This function is called by the printing macros. */
void DX_VOS_DebugPrint(DxUint32 ModuleCode, const DxChar *fileName, DxUint32 lineNum, 
					   const DxChar *funcName, DxUint32 debugLevel, const DxChar *format, ...);

/*! A predefined basic printing function that uses DX_VOS_Printf in order to print message 
    to standard output.*/
void DX_VOS_DebugStdoutPrint(DxUint32 moduleCode, DxUint32 debugLevel,
                             const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, 
							 const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list);

/*! A predefined basic printing function that uses DX_VOS_SocketWrite in order to write the log data to
a socket. The socket should be opened using DX_VOS_OpenLogSocket.
to an opened.*/
void DX_VOS_DebugSocketPrint(DxUint32 moduleCode, DxUint32 debugLevel,
							 const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, 
							 const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list);

/*! A predefined basic print function that uses LogCat in order to print log messages to the debugger 
Only available when compiling for Android */
#ifdef DX_ANDROID

void DX_VOS_DebugLogcatPrint(DxUint32 moduleCode, DxUint32 debugLevel,
							 const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, 
							 const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list);
#endif


/*! Opens a socket that will be used later by the DX_VOS_DebugSocketPrint() function.
After calling this function any thread which will write a debug log will write to this socket.
The log data will be sent as UDP packets.
The fileName is a suggested log name for the file on the receiving device.
The ipAddress should be of format: "X.X.X.X" (e.g. "127.0.0.1")
The port is a DxUint16 port number.
*/
void DX_VOS_OpenLogSocket(const DxChar* fileName, const DxChar* ipAddress, DxUint16 port);

void DX_VOS_CloseLogSocket();

/*! A predefined basic printing function that prints the debug message to the file that was
opened using DX_VOS_OpenLogFile(). For best results change the values of the internal 
variables only before calling DX_VOS_OpenLogFile(). If DX_VOS_OpenLogFile() was not called
this function does nothing.
*/
void DX_VOS_DebugFilePrint(DxUint32 moduleCode, DxUint32 debugLevel,
                           const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, 
						   const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list);

/*! Opens a log file that will be used later by the DX_VOS_DebugFilePrint() function.
    After calling this function any thread which will write a debug log will create a new file
    whose name is fileName + current data & time + thread id + ".DxLog".
    For Example: if fileName == "LogFile" the actual log file name will be:
    "LogFile_200706010_1245343_7456.DxLog"
\return 
- DX_SUCCESS on success.
- DX_BAD_ARGUMENTS - if fileName is NULL.
- DX_VOS_FILE_ERROR - if file cannot be opened.
*/
const DxChar* DX_VOS_OpenLogFile(const DxChar* fileName);

/*! Closes the log file that was previously opened by DX_VOS_OpenLogFile().
This function can be called safely even if DX_VOS_OpenLogFile() was not called earlier.
This function should be called only when no thread is using the debug log.
*/
void DX_VOS_CloseLogFile(void);

/*! Returns TRUE iff the log file is currently opened */
DxBool DX_VOS_IsLogFileOpened(void);
/*! Causes the debug file of the current thread to flush its content to the disk. 
    \warning Calling this function too often may influence the application performance.
*/
void DX_VOS_DbgFlush(void);

/*! A predefined basic printing function that calls a series of other basic print functions 
which are stored in an internal array. The array can be manipulated using 
DX_VOS_ClearDebugPrintFuncArray() & DX_VOS_RegisterDebugPrintFunc().
This function is useful for example when you want to print to the screen and to a log 
file simultaneously.

This is the default basic print function. The array holds pointers to DX_VOS_DebugScreenPrint()	&
DX_VOS_DebugFilePrint(). Until DX_VOS_OpenLogFile() is called messages are printed only to
the standard output. After DX_VOS_DebugFilePrint() is called the messages are printed also to
a log file.
*/	
void DX_VOS_DebugMultiPrint(DxUint32 moduleCode, DxUint32 debugLevel,
                            const DxChar *fileName, DxUint32 lineNum, const DxChar *funcName, 
							const DxChar* aDate, const DxChar* aTime, const DxChar *format, DX_VA_LIST arg_list);

/*! Clears the internal array of registered printing functions. */
void DX_VOS_ClearDebugPrintFuncArray(void);

/*! Adds a function to the array of printing functions. The array can hold up to 5 functions.
\return 
- DX_SUCCESS - on success
- DX_FAILURE - if array is already full.
*/
DxStatus DX_VOS_RegisterDebugPrintFunc(DxPrintfFunc_t func);

/* if the compiler does not support the __FUNCTION__ compiler definition.
   the following line should be added to the appropriate DX_VOS_config.h file:

   #define __FUNCTION__ ""
*/

#ifndef DX_DBG_MODULE
#define DX_DBG_MODULE 0x80000000
#endif

/*! Printing macros. */
#define DX_DBG_PRINT0(debugLevel, format) \
	DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format)
#define DX_DBG_PRINT1(debugLevel, format, arg1) \
	DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1)
#define DX_DBG_PRINT2(debugLevel, format, arg1, arg2) \
	DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2)
#define DX_DBG_PRINT3(debugLevel, format, arg1, arg2, arg3) \
	DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3)
#define DX_DBG_PRINT4(debugLevel, format, arg1, arg2, arg3, arg4) \
	DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3, arg4)
#define DX_DBG_PRINT5(debugLevel, format, arg1, arg2, arg3, arg4, arg5) \
    DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3, arg4, arg5)
#define DX_DBG_PRINT6(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6) \
    DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6)
#define DX_DBG_PRINT7(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define DX_DBG_PRINT8(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
    DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__, debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#else

#define DX_DBG_PRINT0(debugLevel, format) 
#define DX_DBG_PRINT1(debugLevel, format, arg1) 
#define DX_DBG_PRINT2(debugLevel, format, arg1, arg2) 
#define DX_DBG_PRINT3(debugLevel, format, arg1, arg2, arg3) 
#define DX_DBG_PRINT4(debugLevel, format, arg1, arg2, arg3, arg4) 
#define DX_DBG_PRINT5(debugLevel, format, arg1, arg2, arg3, arg4, arg5) 
#define DX_DBG_PRINT6(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6) 
#define DX_DBG_PRINT7(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) 
#define DX_DBG_PRINT8(debugLevel, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) 

#define DX_VOS_DbgPrint_SetShouldPrintFileName(value)	DX_FALSE
#define DX_VOS_DbgPrint_SetShouldPrintLineNum(value)	DX_FALSE
#define DX_VOS_DbgPrint_SetShouldPrintFuncName(value)	DX_FALSE
#define DX_VOS_DbgPrint_SetTimeFormat(value)			DX_NULL
#define DX_VOS_DbgPrint_SetDateFormat(value)			DX_NULL
#define DX_VOS_DbgPrint_SetDebugLevel(value)			0
#define DX_VOS_DbgPrint_SetPrintFunc(value)				DX_NULL
#define DX_VOS_DbgPrint_SetModulesMask(value)			0
#define DX_VOS_DbgPrint_EnableModulePrinting(value)		0
#define DX_VOS_DbgPrint_DisableModulePrinting(value)	0
#define DX_VOS_DebugStdoutPrint							DX_NULL
#define DX_VOS_DebugFilePrint							DX_NULL
#define DX_VOS_DebugMultiPrint							DX_NULL
#define DX_VOS_OpenLogFile(fileName)					DX_SUCCESS
#define DX_VOS_OpenLogSocket(fileName,addr,port)			DX_SUCCESS
#define DX_VOS_CloseLogFile()
#define DX_VOS_CloseLogSocket()
#define DX_VOS_DbgFlush()
#define DX_VOS_ClearDebugPrintFuncArray()
#define DX_VOS_RegisterDebugPrintFunc(func)				DX_SUCCESS

#endif
#ifdef  __cplusplus
}
#endif

#endif
