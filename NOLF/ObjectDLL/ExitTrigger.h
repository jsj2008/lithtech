// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Definition
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __EXIT_TRIGGER_H__
#define __EXIT_TRIGGER_H__

#include "Trigger.h"

class ExitTrigger : public Trigger
{
	public :

		ExitTrigger();
		~ExitTrigger();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        virtual LTBOOL Activate();

	private :

		void Update();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void PostPropRead(ObjectCreateStruct *pData);

        LTFLOAT      m_fFadeOutTime;
};

#endif // __EXIT_TRIGGER_H__