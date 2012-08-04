// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.h
//
// PURPOSE : EventCounter - Definition
//
// CREATED : 04.23.1999
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
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
	{
		nValue				= 0;
        hstrIncToValCmd     = LTNULL;
        hstrDecToValCmd     = LTNULL;
	}

    virtual ~EventData()
	{
		FREE_HSTRING(hstrIncToValCmd);
		FREE_HSTRING(hstrDecToValCmd);
	}

	inline void Save(ILTMessage_Write *pMsg)
	{
		SAVE_INT(nValue);
		SAVE_HSTRING(hstrIncToValCmd);
		SAVE_HSTRING(hstrDecToValCmd);
	}

	inline void Load(ILTMessage_Read *pMsg)
	{
		LOAD_INT(nValue);
		LOAD_HSTRING(hstrIncToValCmd);
		LOAD_HSTRING(hstrDecToValCmd);
	}

	// Data...

	int		nValue;
	HSTRING	hstrIncToValCmd;
	HSTRING	hstrDecToValCmd;
};


class EventCounter : public GameBase
{
	public :

		EventCounter();
		~EventCounter();

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	protected :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	private :

        LTBOOL      m_bLocked;
		int			m_nCurVal;

		EventData	m_EventData[EC_MAX_NUMBER_OF_EVENTS];

		void		Increment();
		void		Decrement();
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