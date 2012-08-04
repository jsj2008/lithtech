// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAbstractStimulatable.h
//
// PURPOSE : AISensorAbstractStimulatable abstract class definition
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_ABSTRACT_STIMULATABLE_H__
#define __AISENSOR_ABSTRACT_STIMULATABLE_H__

#include "AISensorAbstract.h"
#include "AIWorkingMemory.h"


// Forward declarations.

class	CAIWMFact;
struct	AIDB_StimulusRecord;
struct  StimulusRecordCreateStruct;

typedef std::vector<StimulusRecordCreateStruct> STIMULUS_DISPATCH_LIST;


class CAISensorAbstractStimulatable : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Sensor );

		CAISensorAbstractStimulatable();

		// CAISensorAbstract members.
		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();
		virtual bool	StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual void	DestimulateSensor();

	protected:

		virtual float		GetSenseDistSqr( float fStimulusRadius ) = 0;
		virtual bool		DoComplexCheck( CAIStimulusRecord* /*pStimulusRecord*/, float* /*pfRateModifier*/ ) { return true; }
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord ) = 0;
		virtual void		SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord );
		virtual void		SetFactStimulus( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence );
		virtual void		SetFactStimulusPos( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence );
		virtual void		FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList) { return ; }
		bool				ReactionDelayExists( CAIWMFact* pFact );
		void				DispatchStimulus( const StimulusRecordCreateStruct& scs );

		virtual float	IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus );
		float			DecreaseStimulation( float fCurStimulation, AIDB_StimulusRecord* pStimulus );
		virtual AIDB_StimulusRecord* GetAIDBStimulus( CAIStimulusRecord* pStimulusRecord );

	protected:
		
		AIWORKING_MEMORY_FACT_LIST	m_lstReactionDelays;
		STIMULUS_DISPATCH_LIST		m_lstStimuli;
		double						m_fLastSensorUpdate;

		// The following variables do not need to be saved.
		// They are set once in the constructor.

		float						m_fStimulationThreshold;
		float						m_fStimulationMax;
};

#endif
