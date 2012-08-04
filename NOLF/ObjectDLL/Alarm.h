// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.h
//
// PURPOSE : An alarm object
//
// CREATED : 4/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __ALARM_H__
#define __ALARM_H__

#include "ltengineobjects.h"
#include "Prop.h"

class Alarm : public Prop
{
	public :

		Alarm();
		~Alarm();

		// Simple accessors

		LTBOOL IsLocked() const { return m_bLocked; }

	protected :

		enum State
		{
			eStateOff,
			eStateOn,
			eStateDestroyed,
			eStateDisabled,
		};

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
        LTBOOL  Setup(ObjectCreateStruct *pData );
		void	PostPropRead(ObjectCreateStruct* pData);
        LTBOOL  InitialUpdate();

	protected :

		State	m_eState;
        LTBOOL  m_bPlayerUsable;
		HSTRING	m_hstrActivateMessage;
		HSTRING	m_hstrActivateTarget;
		LTBOOL	m_bLocked;

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();
};

#endif // __ALARM_H__