// ----------------------------------------------------------------------- //
//
// MODULE  : IAISensing.h
//
// PURPOSE : IAISensing interface definition
//
// CREATED : 4/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSING_H__
#define __AISENSING_H__

// Forward declarations.

class	CAIStimulusRecord;
struct	AISenseRecord;
enum	EnumAISenseType;
enum    EnumAIStimulusID;
class   RelationSet;


//
// MAP: 
//
typedef std::map<EnumAIStimulusID, int> AI_PROCESSED_STIMULI_MAP;


//
// CLASS: Interface for objects that can sense through the AIStimulusMgr.
//

// INTERFACE ONLY.  DO NOT ADD DATA MEMBERS!!

class IAISensing
{
	public : // Public methods

		virtual HOBJECT		GetSensingObject() = 0;

		virtual LTBOOL		IsSensing() = 0;
		virtual LTBOOL		IsAlert() = 0;

		virtual LTFLOAT		GetSenseUpdateRate() const = 0;
		virtual LTFLOAT		GetNextSenseUpdate() const = 0;
		virtual void		SetNextSenseUpdate(LTFLOAT fTime) = 0;

		virtual	void		UpdateSensingMembers() = 0;

		virtual	LTBOOL		GetDoneProcessingStimuli() const = 0;
		virtual	void		SetDoneProcessingStimuli(LTBOOL bDone) = 0;
		virtual void		ClearProcessedStimuli() = 0;
		virtual LTBOOL		ProcessStimulus(CAIStimulusRecord* pRecord) = 0;

		virtual int			GetIntersectSegmentCount() const = 0;
		virtual void		ClearIntersectSegmentCount() = 0;
		virtual void		IncrementIntersectSegmentCount() = 0;

		virtual LTBOOL		HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle) = 0;
		virtual	void		HandleSenses(uint32 nCycle) = 0;
		virtual void		HandleSenseTrigger(AISenseRecord* pSenseRecord) = 0;
		virtual LTFLOAT		GetSenseDistance(EnumAISenseType eSenseType) = 0;

		virtual uint32		GetCurSenseFlags() const = 0;

		virtual const RelationSet& GetSenseRelationSet() const = 0;

		virtual const LTVector& GetSensingPosition() const = 0;

		virtual CRange<int>& GetSightGridRangeX() = 0;
		virtual CRange<int>& GetSightGridRangeY() = 0;

	// INTERFACE ONLY.  DO NOT ADD DATA MEMBERS!!
};

#endif
