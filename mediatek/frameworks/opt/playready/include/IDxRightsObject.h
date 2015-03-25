#ifndef IDX_RIGHTS_OBJECT_H
#define IDX_RIGHTS_OBJECT_H

#include "DxTypes.h"
#include "DxDrmDefines.h"

/*! IDxRightsObject is an interface that represents a Rights Object (RO).
	RO is an object that contains a set of permissions (i.e. permission to
	play, display, execute,...) and a set of constraints (i.e. date-time, 
	count, interval,...). 
	The RO can contain one constraint (at most) of every of the following kinds:
	1. Count - The user can use the content for specific number of times.
	2. Timed Count - Same as count but the counter is decreased only after
		several seconds (specified in the RO) of usage.
	3. Date Time - The user can use the content only after a specified start time
		has arrived (if start time is defined) and until a specified end time arrives
		(if end time is defined).
	4. Interval - The user can use the content for the specified period of time
		starting from the first usage of content.
	5. Accumulated - The user can use the content for a specified period of time.
		Only periods of active usage decrease the usage period counter.
	6. Individual - Only a specific individual (i.e SIMs identified by their IMSI)
        can use the content.
	
	All the constraints apply to all the permissions.
	The object is valid only if all the constraints are sufficed (i.e. the
	constraints conditions are ANDed).
*/

class IDxRightsObject 
{
public:
	enum EDxRightsEvaulationMode
	{
		DX_EVAL_RIGHTS_USING_DRM_TIME,
		DX_EVAL_RIGHTS_ALLOW_DEVICE_TIME,
	};

    virtual ~IDxRightsObject()  {};
	//! \return ORed value of all RO permissions.
    virtual DxUint32 Permissions() const = 0;

	//! \return ORed value of all RO constraints. 0 means unlimited rights.
    virtual DxUint32 Constraints() const = 0;
	
	//! \return The initial count of count constraint.
    virtual DxUint32 InitialCount() const = 0;

	//! \return The number of counts left of count constraint.
    virtual DxUint32 CountLeft() const = 0;

	//! \return The initial count of timed count constraint.
    virtual DxUint32 InitialTimedCount() const = 0;

	//! \return The number of counts left of timed count constraint.
    virtual DxUint32 TimedCountLeft() const = 0;

	//! \return The number of seconds that should pass from usage start	till counter decrease.
    virtual DxUint32 TimedCountTimer() const = 0;

	//! \return The start time of date-time constraint. NULL if StartTime is not specified.
    virtual const DxTimeStruct* StartTime() const = 0;

	//! \return The end time of date-time constraint. NULL if EndTime is not specified.
    virtual const DxTimeStruct* EndTime() const = 0;

	//! \return The time left in seconds until the RO expires.
	virtual DxStatus TimeLeft(DxUint32& secsLeft) const = 0;

	//! \return The period of time in seconds of the interval constraint.
    virtual DxUint32 IntervalPeriodInSeconds() const = 0;
	
	//! \return The period of time in seconds of the accumulated constraint.
    virtual DxUint32 InitialAccumulatedSeconds() const = 0;

	//! \return The period of time in seconds that was left in the accumulated constraint.
    virtual DxUint32 AccumulatedSecondsLeft() const = 0;

    //! \return The number of available individual values.
    virtual DxUint32 GetNumOfIndividualValues() const = 0;

	//! \return The identification of the individual that may use the content.
    virtual const DxChar* IndividualValue(DxUint32 index) const = 0;

    virtual EDxUseRestriction GetUseRestriction() const = 0;

    //! \return the license category (PR only)
    virtual DxLicenseStateCategory GetCategory() const = 0;

    
	/*! \return TRUE is the RO constraints are stateful.
		(i.e. constraint of count, timed count, not active interval or accumulated exist)
	*/
    virtual DxBool IsStateful() const = 0;
	
	/*! Retrieves the status of the RO by evaluating its constraints.
	    evalMode param affect the way we evaluate the rights:
		- DX_EVAL_RIGHTS_USING_DRM_TIME - Use DRM time to evaulate time based rights.
		- DX_EVAL_RIGHTS_ALLOW_DEVICE_TIME - If the DRM time is not set, we evaluate any time based rights using the device time.
	*/
    virtual EDxRightsObjectStatus GetStatus(EDxRightsEvaulationMode evalMode = DX_EVAL_RIGHTS_USING_DRM_TIME) const = 0;


};

#endif
