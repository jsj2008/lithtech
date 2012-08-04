// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.h
//
// PURPOSE : An alarm object
//
// CREATED : 4/15/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ALARM_H__
#define __ALARM_H__

#include "ltengineobjects.h"
#include "CommandMgr.h"
#include "Prop.h"

LINKTO_MODULE( Alarm );

class Alarm : public Prop
{
	public :

		Alarm();
		~Alarm();

		// Simple accessors

		bool	IsLocked() const { return m_bLocked; }

	protected :

		enum State
		{
			eStateOff,
			eStateOn,
			eStateDestroyed,
			eStateDisabled,
		};

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, float lData);
		virtual uint32	ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	protected :

		bool	ReadProp(const GenericPropList *pProps);
		bool	Setup(ObjectCreateStruct *pData );
		void	PostPropRead(ObjectCreateStruct* pData);
		bool	InitialUpdate();

	private :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	protected :

		State			m_eState;
		bool			m_bPlayerUsable;
		bool			m_bLocked;
		float			m_fAlarmSoundTime;
		std::string		m_sPlayerActivateCommand;


		// Message Handlers...

		DECLARE_MSG_HANDLER( Alarm, HandleLockMsg );
		DECLARE_MSG_HANDLER( Alarm, HandleUnlockMsg );
		DECLARE_MSG_HANDLER( Alarm, HandleActivateMsg );
};

class CAlarmPlugin : public CPropPlugin
{
	public:	

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif // __ALARM_H__
