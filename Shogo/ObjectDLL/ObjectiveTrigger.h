// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveTrigger.h
//
// PURPOSE : ObjectiveTrigger - Definition
//
// CREATED : 3/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVE_TRIGGER_H__
#define __OBJECTIVE_TRIGGER_H__

#include "cpp_engineobjects_de.h"
#include "activation.h"


class ObjectiveTrigger : public BaseClass
{
	public :

		ObjectiveTrigger();
		~ObjectiveTrigger();

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	ObjectTouch(HOBJECT hObj);

				void	Trigger();

	protected:

		DBOOL			m_bPlayerTriggerable;
		DBOOL			m_bAITriggerable;
		DBOOL			m_bTriggered;
		HSTRING			m_hstrAIName;
		HSTRING			m_hAddObjectives;
		HSTRING			m_hRemoveObjectives;
		HSTRING			m_hCompletedObjectives;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CActivation		m_activation;
		DVector			m_vDims;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __OBJECTIVE_TRIGGER_H__