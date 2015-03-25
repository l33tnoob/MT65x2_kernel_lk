#ifndef _DX_CONFIG_FILE_
#define _DX_CONFIG_FILE_

#include "VOS_API/DX_VOS_Mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DX_NO_CONFIG_FILE

/* Loads a configuration file into memory. It's possible to load several configuration
files. The Union of all loaded config files will reside in the memory.
In case of overriding property names, the property value of the last file loaded will count.
DX_SUCCESS is returned if load succeeded. If failed, previous config files loaded into memory 
will not be affected.
See Appendix1 for config file example. */
DxStatus DxConfigFile_Load(const DxChar* fileName);

/* Saves the current loaded configuration into a file. Notice that the configuration might be 
composed of data from several files, But only one file (containing all config) is created when
DxConfigFile_Save is called. */
DxStatus DxConfigFile_Save(const DxChar* fileName);

/* Clear all configuration loaded from memory */
void DxConfigFile_Terminate(void);

/* Return the number of items that match propertyName. If propertyName doesn't exist in 
the configuration, than defaultValue is returned.
See Appendix2 for config file example. */
DxUint32 DxConfigFile_GetCount(const DxChar *propertyName, DxUint32 defaultValue);

/*! Returns the number of items in the config file. If NULL then 0 returned. */
DxUint32 DxConfigFile_GetItemCount();

/*! Get the name-value pair stored in the config file at the index given.
\return 
- DX_SUCCESS on success
- DX_ITEM_NOT_FOUND if index out of bounds.
- DX_NOT_INITIALIZED if config file is NULL.
*/
DxStatus DxConfigFile_GetItemAtIndex(DxUint32 index, const DxChar** nameOut, const DxChar** valueOut);

/*! Check whether an item identified by propertyName is stored in the config file.
\return 
- DX_TRUE if the config file contains an item by the name of propertyName.
- DX_FALSE No such item exists.
*/
DxBool DxConfigFile_HasItem(const DxChar* propertyName);


/************************************************************************/
/* String Utilities                                                     */
/************************************************************************/

/* Set propertyName to be propertyValue. if propertyName already exists, it will be overwritten. */
DxStatus DxConfigFile_SetString(const DxChar* propertyName, const DxChar* propertyValue);

/* Return the value matching propertyName at the index given. defaultValue is returned if the 
propertyName doesn't exist, or the index is greater than the number of items in the property.
See Appendix2 for config file example. */
const DxChar* DxConfigFile_GetStringItem(const DxChar* propertyName, DxUint32 index, const DxChar* defaultValue);
#define DxConfigFile_GetString(propertyName, defaultValue) DxConfigFile_GetStringItem(propertyName, 0, defaultValue)

/* sets data to be the property value converted from hex representation to binary.
defaultValue is returned if propertyName doesn't exist, or the value is not hex representation,
or dataSize is not exactly half the size of the propertyValue length. */
DxStatus DxConfigFile_GetHexBuffer(const DxChar* propertyName, void* data, DxUint32 dataSize, const void* defaultData);

DxStatus DxConfigFile_GetDefaultHexBuffer(void* data, DxUint32 dataSize, const void* defaultData);

/************************************************************************/
/* DxUint32 Utilities                                                 */
/************************************************************************/

/* Set propertyName to be propertyValue. if propertyName already exists, it will be overwritten. */
DxStatus DxConfigFile_SetNumber(const DxChar* propertyName, DxUint32 propertyValue);

/* Return the value matching propertyName at the index given as DxUint32. defaultValue is 
returned if the propertyName doesn't exist, or the index is greater than the number of items
in the property, or in case the value exist but is not a number.
See Appendix2 for config file example. */
DxUint32 DxConfigFile_GetNumberItem(const DxChar* propertyName, DxUint32 index, DxUint32 defaultValue);

#define DxConfigFile_GetNumber(propertyName, defaultValue) DxConfigFile_GetNumberItem(propertyName, 0, defaultValue)


/************************************************************************/
/* Boolean Utilities                                                    */
/************************************************************************/

/* Set propertyName to be propertyValue. if propertyName already exists, it will be overwritten. */
DxStatus DxConfigFile_SetBoolean(const DxChar* propertyName, DxBool propertyValue);

/* Return the value matching propertyName at the index given as DxBool. defaultValue is 
returned if the propertyName doesn't exist, or the index is greater than the number of items
in the property, or in case the value exist but is not boolean.
See Appendix2 for config file example. */
DxBool DxConfigFile_GetBooleanItem(const DxChar* propertyName, DxUint32 index, DxBool defaultValue);

#define DxConfigFile_GetBoolean(propertyName, defaultValue) DxConfigFile_GetBooleanItem(propertyName, 0, defaultValue)



#else

#define	DxConfigFile_Load(fileName) DX_SUCCESS
#define	DxConfigFile_Save(fileName) DX_SUCCESS
#define	DxConfigFile_Terminate()

#define	DxConfigFile_GetCount(propertyName, defaultValue) defaultValue

#define	DxConfigFile_HasItem(propertyName) DX_FALSE

#define DxConfigFile_SetString(propertyName, propertyValue) DX_SUCCESS
#define DxConfigFile_GetString(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetStringItem(propertyName, index, defaultValue) defaultValue

#define DxConfigFile_SetNumber(propertyName, propertyValue) DX_SUCCESS
#define DxConfigFile_GetNumber(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetNumberItem(propertyName, index, defaultValue) defaultValue

#define DxConfigFile_SetBoolean(propertyName, propertyValue) DX_SUCCESS
#define	DxConfigFile_GetBoolean(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetBooleanItem(propertyName, index, defaultValue) defaultValue

#define DxConfigFile_GetHexBuffer(propertyName, data, dataSize, defaultData) DX_VOS_FastMemCpy(data, defaultData, dataSize)
#endif


/* Use DxConfigFile_GetSecureString() for config values that are for debug purposes only, and that must not be altered in the
   final product (for example, a flag that disables integrity check).
   When DX_SECURITY_LEVEL is less than 3, DxConfigFile_GetSecureString behaves the same as DxConfigFile_GetString, otherwise it returns default value.*/
#ifndef DX_SECURITY_LEVEL
#define DX_SECURITY_LEVEL 3
#endif
#if DX_SECURITY_LEVEL < 3
#define DxConfigFile_GetSecureString(propertyName, defaultValue) DxConfigFile_GetString(propertyName, defaultValue)
#define DxConfigFile_GetSecureNumber(propertyName, defaultValue) DxConfigFile_GetNumber(propertyName, defaultValue)
#define DxConfigFile_GetSecureBoolean(propertyName, defaultValue) DxConfigFile_GetBoolean(propertyName, defaultValue)
#define DxConfigFile_GetSecureHexBuffer(propertyName, data, dataSize, defaultValue) DxConfigFile_GetHexBuffer(propertyName, data, dataSize, defaultValue)
#define	DxConfigFile_HasSecureItem(propertyName) DxConfigFile_HasItem(propertyName)
#else
#define DxConfigFile_GetSecureString(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetSecureNumber(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetSecureBoolean(propertyName, defaultValue) defaultValue
#define DxConfigFile_GetSecureHexBuffer(propertyName, data, dataSize, defaultValue) DxConfigFile_GetDefaultHexBuffer(data, dataSize, defaultValue)
#define	DxConfigFile_HasSecureItem(propertyName) DX_FALSE
#endif

#ifdef __cplusplus
}
#endif

#endif


/*

##############
Appendix1: 
Config file example:

name1 = value1
listOfValues = 1,2,3,4
names = Bobbie,McGee
flag = true
flag2 = false
#noName = this will be discarded
 #name2 = this entry will be discarded and the next one as well.
name3 =
num1 = 1
##############

Appendix2: 
In relation to the config file above, we will get the following:

GetCount:
DxConfigFile_GetCount("name1", -1) -> 1
DxConfigFile_GetCount("listOfValues", -1) -> 4
DxConfigFile_GetCount("name3", -1) -> -1
DxConfigFile_GetCount("abcd", -2) -> -2

GetStringItem:
DxConfigFile_GetStringItem("name1", 0, "Error") -> "value1"
DxConfigFile_GetStringItem("name1", 1, "Error") -> "Error"
DxConfigFile_GetStringItem("listOfValues", 1, "Error") -> "2"
DxConfigFile_GetStringItem("name2", 2, "default") -> "default"

GetNumberItem:
DxConfigFile_GetNumberItem("num1", 0, -1) -> 1
DxConfigFile_GetNumberItem("name1", 0, -1) -> -1

GetBooleanItem:
DxConfigFile_GetNumberItem("flag", 0, DX_FALSE) -> DX_TRUE
DxConfigFile_GetNumberItem("flag2", 0, DX_FALSE) -> DX_FALSE
DxConfigFile_GetNumberItem("name1", 0, DX_FALSE) -> DX_FALSE
*/


