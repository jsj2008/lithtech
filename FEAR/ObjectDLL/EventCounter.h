// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.h
//
// PURPOSE : EventCounter - Definition
//
// CREATED : 04.23.1999
//
// (c) 2000-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EVENT_COUNTER_H__
#define __EVENT_COUNTER_H__

#include "ltengineobjects.h"
#include "ServerUtilities.h"
#include "CommandMgr.h"
#include "GameBase.h"

#define EC_MAX_NUMBER_OF_EVENTS		10

LINKTO_MODULE( EventCounter );

struct EventData
{
	EventData()
	:	nValue			( 0 ),
		sIncToValCmd	( ),
		sDecToValCmd	( )
	{
		
	}

	virtual ~EventData()
	{
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_INT(nValue);
		SAVE_STDSTRING(sIncToValCmd);
		SAVE_STDSTRING(sDecToValCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_INT(nValue);
		LOAD_STDSTRING(sIncToValCmd);
		LOAD_STDSTRING(sDecToValCmd);
	}

	// Data...

	int				nValue;
	std::string		sIncToValCmd;
	std::string		sDecToValCmd;
};


class EventCounter : public GameBase
{
	public :

		EventCounter();
		~EventCounter();

		uint32		EngineMessageFn(uint32 messageID, void *pData, float fData);

	protected :

		bool		ReadProp(const GenericPropList *pProps);

		void		Save(ILTMessage_Write *pMsg);
		void		Load(ILTMessage_Read *pMsg);

	private :

		bool		m_bLocked;
		int			m_nCurVal;

		EventData	m_EventData[EC_MAX_NUMBER_OF_EVENTS];

		void		Increment();
		void		Decrement();
		

		// Message Handlers...

		DECLARE_MSG_HANDLER( EventCounter, HandleIncMsg );
		DECLARE_MSG_HANDLER( EventCounter, HandleDecMsg );
		DECLARE_MSG_HANDLER( EventCounter, HandleLockMsg );
		DECLARE_MSG_HANDLER( EventCounter, HandleUnlockMsg );
};


class CEventCounterPlugin : public IObjectPlugin
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

#endif // __EVENT_COUNTER_H__
