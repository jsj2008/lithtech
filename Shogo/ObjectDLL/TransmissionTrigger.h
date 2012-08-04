// ----------------------------------------------------------------------- //
//
// MODULE  : TransmissionTrigger.h
//
// PURPOSE : TransmissionTrigger - Definition
//
// CREATED : 3/21/98
//
// ----------------------------------------------------------------------- //

#ifndef __TRANSMISSION_TRIGGER_H__
#define __TRANSMISSION_TRIGGER_H__

#include "cpp_engineobjects_de.h"
#include "activation.h"

class TransmissionTrigger : public BaseClass
{
	public :

		TransmissionTrigger();
		~TransmissionTrigger();

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	ObjectTouch(HOBJECT hObj);

				void	Trigger();

	protected:

		CActivation		m_activation;
		
		DVector			m_vDims;
		DBOOL			m_bPlayerTriggerable;
		DBOOL			m_bAITriggerable;
		DBOOL			m_bTriggered;
		HSTRING			m_hstrAIName;
		HSTRING			m_hImageFilename;
		DFLOAT			m_fStringID;
		DFLOAT			m_fSendDelay;


	private :

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __TRANSMISSION_TRIGGER_H__