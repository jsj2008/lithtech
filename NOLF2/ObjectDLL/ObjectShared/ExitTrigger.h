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

LINKTO_MODULE( ExitTrigger );

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

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void PostPropRead(ObjectCreateStruct *pData);

        LTFLOAT      m_fFadeOutTime;
		bool		m_bExitMission;
};

#endif // __EXIT_TRIGGER_H__