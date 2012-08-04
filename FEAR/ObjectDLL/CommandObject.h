// ----------------------------------------------------------------------- //
//
// MODULE  : CommandObject.h
//
// PURPOSE : The CommandObject object
//
// CREATED : 10/11/01
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __COMMAND_OBJECT_H__
#define __COMMAND_OBJECT_H__

//
// Includes...
//
	
	#include "GameBase.h"
	#include "CommandMgr.h"
	#include "EngineTimer.h"

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
		:	m_sCommand		( ),
			m_fTime			( 0.0f ),
			m_bProcessed	( false )
	{

	}

	std::string	m_sCommand;
	float		m_fTime;
	bool		m_bProcessed;
};

class CommandObject : public GameBase
{
	public:	// Methods...

		CommandObject();
		~CommandObject();


	protected: // Methods...

		uint32	EngineMessageFn( uint32 messageID, void *pvData, float fData );

		void	Update( );
		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		bool	ReadProp( const GenericPropList *pProps );
		
	
	protected : // Members...

		typedef std::vector<EVENT_CMD_STRUCT*, LTAllocator<EVENT_CMD_STRUCT*, LT_MEM_TYPE_OBJECTSHELL> > EVENT_CMD_VECTOR;
		EVENT_CMD_VECTOR	m_vecEventCmds;

		float		m_fTotalTime;
		StopWatchTimer m_Timer;

		std::string	m_sFinishedCmd;
		
		bool		m_bLocked;				// When locked the comands will not execute.
		int			m_nNumActivations;		// How many times the comands can be executed (<= 0 is infinite)
		int			m_nNumTimesActivated;	// How many times has the comands been executed

		LTObjRef	m_hActivator;

		// Message Handlers...

		DECLARE_MSG_HANDLER( CommandObject, HandleOnMsg );
		DECLARE_MSG_HANDLER( CommandObject, HandleOffMsg );
		DECLARE_MSG_HANDLER( CommandObject, HandlePauseMsg );
		DECLARE_MSG_HANDLER( CommandObject, HandleResumeMsg );
		DECLARE_MSG_HANDLER( CommandObject, HandleLockMsg );
		DECLARE_MSG_HANDLER( CommandObject, HandleUnlockMsg );
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
