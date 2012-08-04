// ----------------------------------------------------------------------- //
//
// MODULE  : DialogTrigger.h
//
// PURPOSE : DialogTrigger - Definition
//
// CREATED : 1/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __DIALOG_TRIGGER_H__
#define __DIALOG_TRIGGER_H__

#include "cpp_engineobjects_de.h"
#include "activation.h"

#define MAX_MESSAGES_NUM	3

class DialogTrigger : public BaseClass
{
	public :

		DialogTrigger();
		~DialogTrigger();

				void	Trigger (int nSelection);

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	ObjectTouch(HOBJECT hObj);

				void	ShowDialog();

	protected:

		CActivation		m_activation;
		
		DBOOL			m_bTriggered;
		DBOOL			m_bFirstUpdate;

		HOBJECT			m_hPlayerObject;
		DDWORD			m_nStringID[MAX_MESSAGES_NUM];
		HSTRING			m_hTarget[MAX_MESSAGES_NUM];
		HSTRING			m_hMessage[MAX_MESSAGES_NUM];
		DFLOAT			m_fSendDelay;
		DVector			m_vDims;
		

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void Trigger();
};

#endif // __DIALOG_TRIGGER_H__