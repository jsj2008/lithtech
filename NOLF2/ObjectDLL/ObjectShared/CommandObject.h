// ----------------------------------------------------------------------- //
//
// MODULE  : CommandObject.h
//
// PURPOSE : The CommandObject object
//
// CREATED : 10/11/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __COMMAND_OBJECT_H__
#define __COMMAND_OBJECT_H__

//
// Includes...
//
	
	#include "GameBaseLite.h"
	#include "Timer.h"
	#include "CommandMgr.h"

//
// Defines...
//

	#define EVNTED_MAX_TRACKS	6
	#define EVNTED_MAX_OBJ_PER_TRACK 10

LINKTO_MODULE( CommandObject );

//
// Classes...
//

struct EVENT_CMD_STRUCT
{
	EVENT_CMD_STRUCT()
		:	m_hstrCommand	( LTNULL ),
			m_fTime			( 0.0f ),
			m_bProcessed	( LTFALSE )
	{

	}

	HSTRING	m_hstrCommand;
	LTFLOAT	m_fTime;
	LTBOOL	m_bProcessed;
};

class CommandObject : public GameBaseLite 
{
	public:	// Methods...

		CommandObject();
		~CommandObject();


	protected: // Methods...

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	Update( );
		void    Save( ILTMessage_Write *pMsg );
        void    Load( ILTMessage_Read *pMsg );

		bool	ReadProp( ObjectCreateStruct *pOCS );
		
	
	protected : // Members...

		typedef std::vector<EVENT_CMD_STRUCT*> EVENT_CMD_LIST;
		EVENT_CMD_LIST	m_lstEventCmds;

		LTFLOAT	m_fTotalTime;
		CTimer	m_Timer;

		HSTRING	m_hstrFinishedCmd;
		
		bool	m_bLocked;				// When locked the comands will not execute.
		int		m_nNumActivations;		// How many times the comands can be executed (<= 0 is infinite)
		int		m_nNumTimesActivated;	// How many times has the comands been executed
        
};

class CCommandObjectPlugin : public IObjectPlugin 
{
	public: // Methods...

		virtual LTRESULT PreHook_PropChanged(
				const	char		*szObjName,
				const	char		*szPropName,
				const	int			nPropType,
				const	GenericProp	&gpPropValue,
						ILTPreInterface	*pInterface,
				const	char		*szModifiers);

	private: // Members...

		CCommandMgrPlugin m_CmdMgrPlugin;
};

#endif // __COMMAND_OBJECT_H__