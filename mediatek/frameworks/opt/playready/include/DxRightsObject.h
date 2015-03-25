#ifndef DX_RIGHTS_OBJECT_H
#define DX_RIGHTS_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"

//! \file DxRightsObject.h
//! \brief DxRightsObject is a used to make queries regarding a specific Rights Object (RO).
//!
//! RO is an object that contains a set of permissions (i.e. permission to
//! play, display, execute,...) and a set of constraints (i.e. date-time, 
//! count, interval,...). 
//! The RO can contain one constraint (at most) of every of the following kinds:
//! 1. Count - The user can use the content for specific number of times.
//! 2. Timed Count - Same as count but the counter is decreased only after
//! several seconds (specified in the RO) of usage.
//! 3. Date Time - The user can use the content only after a specified start time
//! has arrived (if start time is defined) and until a specified end time arrives
//! (if end time is defined).
//! 4. Interval - The user can use the content for the specified period of time
//! starting from the first usage of content.
//! 5. Accumulated - The user can use the content for a specified period of time.
//! Only periods of active usage decrease the usage period counter.
//! 6. Individual - Only a specific individual (i.e SIMs identified by their IMSI)
//! can use the content.
//! 
//! All the constraints apply to all the permissions.
//! The object is valid only if all the constraints are sufficed (i.e. the
//! constraints conditions are ANDed).


//! HDxRightsObject is a rights object handle. It allows questioning an RO about its permissions and constraints.
typedef const void* HDxRightsObject;

//! HDxUserMessage is a handle associated with User message to return from license acquisition procedure.
typedef void* HDxUserMessage;


//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return ORed value of all RO permissions.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_Permissions(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return ORed value of all RO constraints. 0 means unlimited rights.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_Constraints(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The initial count of count constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_InitialCount(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The number of counts left of count constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_CountLeft(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The initial count of timed count constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_InitialTimedCount(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The number of counts left of timed count constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_TimedCountLeft(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The number of seconds that should pass from usage start	till counter decrease.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_TimedCountTimer(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The start time of date-time constraint. NULL if StartTime is not specified.
DX_FUNC DRM_C_API const DxTimeStruct* DxRightsObject_StartTime(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The end time of date-time constraint. NULL if EndTime is not specified.
DX_FUNC DRM_C_API const DxTimeStruct* DxRightsObject_EndTime(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The period of time in seconds of the interval constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_IntervalPeriodInSeconds(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The period of time in seconds of the accumulated constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_InitialAccumulatedSeconds(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The period of time in seconds that was left in the accumulated constraint.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_AccumulatedSecondsLeft(HDxRightsObject rightsObject);

//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return The number of available individual values.
DX_FUNC DRM_C_API DxUint32 DxRightsObject_GetNumOfIndividualValues(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \param[in] index			Index of the requested individual value.
//! \return The identification of the individual that may use the content.
DX_FUNC DRM_C_API const DxChar* DxRightsObject_IndividualValue(HDxRightsObject rightsObject, DxUint32 index);

//! NOT SUPPORTED in PLAYREADY
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
//! \return TRUE is the RO constraints are stateful.
//! (i.e. constraint of count, timed count, not active interval or accumulated exist)
DX_FUNC DRM_C_API DxBool DxRightsObject_IsStateful(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! Retrieves the status of the RO by evaluating its constraints
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
DX_FUNC DRM_C_API EDxRightsObjectStatus DxRightsObject_GetStatus(HDxRightsObject rightsObject);

//! NOT SUPPORTED in PLAYREADY
//! Retrieves the Use-Restrictions of the RO.
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
DX_FUNC DRM_C_API EDxUseRestriction DxRightsObject_GetUseRestrictions(HDxRightsObject rightsObject);

//! ONLY FOR PLAYREADY
//! Retrieves the category of the RO.
//! \param[in] rightsObject		A handle previously created by DxDrmFile_GetRO()
DX_FUNC DRM_C_API DxLicenseStateCategory DxRightsObject_GetCategory(HDxRightsObject rightsObject);

#ifdef __cplusplus
}
#endif

#endif

