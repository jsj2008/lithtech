// ----------------------------------------------------------------------- //
//
// MODULE  : CommandMgr.h
//
// PURPOSE : CommandMgr definition
//
// CREATED : 06/23/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_MGR_H__
#define __COMMAND_MGR_H__

#include "ServerUtilities.h"
#include "ltobjref.h"
#include "BankedList.h"
#include "ParsedMsg.h"
#include "iaggregate.h"

//for the possible COMPILE_OBJECT_DESCRIPTIONS
#include "ltserverobj.h"

class ConParse;
class CCommandMgr;
class CCommandMgrPlugin;
class CParsedCmd;
class CCmdMgr_ClassDesc;
class CCommandDescription;
extern CCommandMgr* g_pCmdMgr;

#define CMDMGR_MAX_VARS				512
#define CMDMGR_MAX_PENDING_COMMANDS 100
#define CMDMGR_MAX_COMMAND_LENGTH	256
#define CMDMGR_MAX_VAR_NAME_LENGTH	64 // Prefab names are huge so we need to accomidate for that >:(
#define CMDMGR_MAX_ID_LENGTH		16
#define CMDMGR_NULL_CHAR			'\0'
#define CMDMGR_MAX_VARS_IN_EVENT	16
#define CMDMGR_MAX_EVENT_COMMANDS	32
#define CMDMGR_MIN_NUMARGS			2
#define CMDMGR_MIN_CLASSMSGS		1	// The minimum number of messages a class has.  Currently every class has one message that should be ignored.


typedef bool (*ProcessCmdFn)(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd);
typedef bool (*PreCheckCmdFn)(CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse );
typedef bool (*IntOperatorFn)(void *arg1, void *arg2);

typedef bool (*TValidateMsgFn)( ILTPreInterface *pInterface, ConParse &cpMsgParams );
typedef void (*THandleMsgFn)( HOBJECT hSender, ILTBaseClass *pObject, IAggregate *pAgg, const CParsedMsg &crParsedMsg );

CCommandDescription* GetCommandDescriptions();
uint32	GetNumCommandDescriptions();

CCmdMgr_ClassDesc *GetCmdmgrClassDescription( const char *pClassName );
CCmdMgr_ClassDesc *GetCmdmgrClassDescription( uint32 nIndex );
uint32 GetNumCmdmgrClassDescriptions();

enum eExpressionVal
{
	kExpress_ERROR = -1,
	kExpress_FALSE = 0,
	kExpress_TRUE = 1,
};

enum ECmdMgrVarType
{
	eCMVar_Int,
	eCMVar_Obj
};

struct VAR_STRUCT : public ILTObjRefReceiver
{
	VAR_STRUCT()
	{
		Clear();
		m_hObjVal.SetReceiver(*this);
	}

	VAR_STRUCT( VAR_STRUCT const& other )
	{
		m_sName = other.m_sName;
		m_eType = other.m_eType;
		m_iVal = other.m_iVal;
		m_hObjVal = ( HOBJECT )other.m_hObjVal;
		m_pObjVal = other.m_pObjVal;
		m_bGlobal = other.m_bGlobal;
		m_nRefCount = other.m_nRefCount;
		m_bShow = other.m_bShow;
		m_bValid = other.m_bValid;
	}

	VAR_STRUCT& operator=( VAR_STRUCT const& other )
	{
		if( this != &other )
		{
			this->~VAR_STRUCT( );
			new( this ) VAR_STRUCT( other );
		}
		return *this;
	}

	void Clear()
	{
		m_sName.clear();

		m_eType		= eCMVar_Int;
		m_iVal		= 0;
		m_hObjVal	= 0;
		m_pObjVal	= 0;
		m_bGlobal	= false;
		m_nRefCount	= 0;
		m_bShow		= false;
		m_bValid	= false;
	}

	

	virtual void OnLinkBroken(LTObjRefNotifier* /*pRef*/, HOBJECT /*hObj*/ )
	{
		m_pObjVal = 0;
	}

	// jrg - 9/7/02 Added bNameOnly flag so global vars can be "declared" before the Events are loaded
	void	Save( ILTMessage_Write *pMsg, bool bNameOnly );
	void	Load( ILTMessage_Read *pMsg, bool bNameOnly );

	void	SetObjVal(ILTBaseClass *pObj)
	{
		m_pObjVal = pObj;
		m_hObjVal = (m_pObjVal) ? m_pObjVal->m_hObject : NULL;
	}

	std::string			m_sName;
	ECmdMgrVarType		m_eType;
	int					m_iVal;
	LTObjRefNotifier	m_hObjVal;
	ILTBaseClass*		m_pObjVal;
	bool				m_bGlobal;
	int					m_nRefCount;
	bool				m_bShow;
	bool				m_bValid;
};

// The operator struct is just for ease of adding new operators and operators that do the same thing (ie. '==', 'equals', 'is')

struct OPERATOR_STRUCT
{
	OPERATOR_STRUCT( const char *pOpName = "", bool bLogical = false, IntOperatorFn pOpFn = NULL, ECmdMgrVarType eVarType = eCMVar_Int )
		:	m_OpName	( pOpName ),
			m_bLogical	( bLogical ),
			m_OpFn		( pOpFn ),
			m_eVarType	( eVarType )
	{

	}

	const char		*m_OpName;
	bool			m_bLogical;
	IntOperatorFn	m_OpFn;
	ECmdMgrVarType	m_eVarType;

};

// This struct holds the information about an 'event' (ON) command. 

struct CMD_EVENT_STRUCT
{
	CMD_EVENT_STRUCT()
	:	m_sExpression	( ),
		m_sCmds			( ),
		m_nNumVarsInCmd	( 0 )
	{
		for( int i = 0; i < CMDMGR_MAX_VARS_IN_EVENT; ++i )
		{
			m_aVars[i] = NULL;
			
		}
	}

	~CMD_EVENT_STRUCT()
	{	
		Clear();
	}

	void Clear()
	{
		m_sExpression.clear();
		m_sCmds.clear();

		for( int i = 0; i < CMDMGR_MAX_VARS_IN_EVENT; ++i )
		{
			if( m_aVars[i] )
			{
				--m_aVars[i]->m_nRefCount;
				m_aVars[i] = NULL;
			}
		}

		m_nNumVarsInCmd = 0;
	}

	eExpressionVal FillVarArray( ConParse &cpExpression );
	
	bool	AddVar( VAR_STRUCT *pVar )
	{
		if( !pVar ) return false;
		if( m_nNumVarsInCmd >= CMDMGR_MAX_VARS_IN_EVENT ) return false;

		// Check to see if we already have this var in our list...

		for( int i = 0; i < m_nNumVarsInCmd; ++i )
		{
			if( m_aVars[i] == pVar )
				return true;
		}
		
		// Add it to our list and increment its refcount...

		m_aVars[m_nNumVarsInCmd] = pVar;
		++pVar->m_nRefCount;
		++m_nNumVarsInCmd;

		return true;
	}

	bool	Process();

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	std::string	m_sExpression;
	std::string	m_sCmds;
	VAR_STRUCT	*m_aVars[CMDMGR_MAX_VARS_IN_EVENT];	// list of variables that are in the command. 
	uint8		m_nNumVarsInCmd;
};

// This struct holds all the information pending commands...

class CPendingCmd : public ILTObjRefReceiver
{
	public:		// Methods...

		CPendingCmd()
		:	m_fDelay		( 0.0f ),
			m_fMinDelay		( 0.0f ),
			m_fMaxDelay		( 0.0f ),
			m_nNumTimes		( -1 ),
			m_nMinTimes		( -1 ),
			m_nMaxTimes		( -1 ),
			m_pActiveTarget	( NULL ),
			m_pActiveSender	( NULL ),
			m_sCmd			( ),
			m_sId			( )
		{
			m_hActiveTarget.SetReceiver(*this);
			m_hActiveSender.SetReceiver(*this);
		}

		virtual void OnLinkBroken(LTObjRefNotifier * /*pRef*/, HOBJECT hObj)
		{
			if( m_pActiveTarget && (m_pActiveTarget->m_hObject == hObj) )
				m_pActiveTarget = NULL;
			
			if( m_pActiveSender && (m_pActiveSender->m_hObject == hObj) )
				m_pActiveSender = NULL;
		}

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		void SetActiveTarget(ILTBaseClass *pObject)
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveTarget will be NULL so don't set pActiveTarget.
			
			m_hActiveTarget = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveTarget = (m_hActiveTarget) ? pObject : NULL;
			
		}

		void SetActiveSender(ILTBaseClass *pObject)
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveSender will be NULL so don't set pActiveSender.

			m_hActiveSender = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveSender = (m_hActiveSender) ? pObject : NULL;
		}


	private:	// Methods...

		// Dont allow assignment or copying...
		CPendingCmd( const CPendingCmd &other );
		CPendingCmd& operator=( const CPendingCmd &other );

	
	public:		// Members...

		float				m_fDelay;
		float				m_fMinDelay;
		float				m_fMaxDelay;
		int32				m_nNumTimes;
		int32				m_nMinTimes;
		int32				m_nMaxTimes;
		LTObjRefNotifier	m_hActiveTarget;
		ILTBaseClass*		m_pActiveTarget;
		LTObjRefNotifier	m_hActiveSender;
		ILTBaseClass*		m_pActiveSender;
		std::string			m_sCmd;
		std::string			m_sId;
};

class CCommandDescription : public ICommandDef
{
	public:
		
		CCommandDescription( const char* pCmd = "",
							const uint8 nArgsMin = 0,
							const uint8 nArgsMax = 0,
							const bool bGlobal = false,
							ProcessCmdFn pFn = NULL,
							PreCheckCmdFn pPreFn = NULL,
							const char* pSyn = "",
							const char* pDesc = "" )
		:	
			ICommandDef		( pCmd, pSyn, pDesc ),
			m_cTok_CmdName	( pCmd ),
			m_nMinArgs		( nArgsMin ),
			m_nMaxArgs		( nArgsMax ),
			m_bGlobal		( bGlobal ),
			m_pProcessFn	( pFn ),
			m_pPreCheckFn	( pPreFn )
		{

		}

		CParsedMsg::CToken		m_cTok_CmdName;
		const uint8				m_nMinArgs;
		const uint8				m_nMaxArgs;
		const bool				m_bGlobal;
		ProcessCmdFn			m_pProcessFn;
		PreCheckCmdFn			m_pPreCheckFn;

	private:

		// Assignment operator not allowed due to const data.
		CCommandDescription& operator=( CCommandDescription const& rhs );
};

class CParsedCmd : public ILTObjRefReceiver
{
	public :	// Methods...

		CParsedCmd()
		:	m_pActiveSender	( NULL ),
			m_hActiveSender	( ),
			m_pActiveTarget	( NULL ),
			m_hActiveTarget	( ),
			m_sCmdName		( ),
			m_cTok_CmdName	( ),
			m_bGlobal		( false ),
			m_saArgs		( CMDMGR_MIN_NUMARGS )
		{
			m_hActiveTarget.SetReceiver( *this );
			m_hActiveSender.SetReceiver( *this );
		}

		~CParsedCmd() {};

		virtual void OnLinkBroken(LTObjRefNotifier* /*pRef*/, HOBJECT hObj)
		{
			if( m_pActiveTarget && (m_pActiveTarget->m_hObject == hObj) )
			{
				m_pActiveTarget = NULL;
			}

			if( m_pActiveSender && (m_pActiveSender->m_hObject == hObj) )
			{
				m_pActiveSender = NULL;
			}
		}

		void SetActiveTarget(ILTBaseClass *pObject)
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveTarget will be NULL so don't set pActiveTarget.

			m_hActiveTarget = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveTarget = (m_hActiveTarget) ? pObject : NULL;

		}

		void SetActiveSender(ILTBaseClass *pObject)
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveSender will be NULL so don't set pActiveSender.

			m_hActiveSender = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveSender = (m_hActiveSender) ? pObject : NULL;
		}

		void SetCommandName( const char *pszCmdName )
		{
			if( !pszCmdName || !pszCmdName[0] )
			{
				m_sCmdName.clear();
				m_cTok_CmdName = "";
				return;
			}

			m_sCmdName = pszCmdName;
			m_cTok_CmdName = m_sCmdName.c_str();
		}

		void Save( ILTMessage_Write *pMsg )
		{
			if( !pMsg )
				return;

			SAVE_HOBJECT( m_hActiveSender );
			SAVE_HOBJECT( m_hActiveTarget );

			uint32 nNumArgs = m_saArgs.size( );
			SAVE_DWORD( nNumArgs );
			for( uint32 nArg = 0; nArg < nNumArgs; ++nArg )
			{
				SAVE_STDSTRING( m_saArgs[ nArg ] );
			}

			SAVE_bool( m_bGlobal );
			SAVE_STDSTRING( m_sCmdName );
		}

		void Load( ILTMessage_Read *pMsg )
		{
			if( !pMsg )
				return;

			LOAD_HOBJECT( m_hActiveSender );
			m_pActiveSender = g_pLTServer->HandleToObject( m_hActiveSender );

			LOAD_HOBJECT( m_hActiveTarget );
			m_pActiveTarget = g_pLTServer->HandleToObject( m_hActiveTarget );

			uint32 nNumArgs = 0;
			LOAD_DWORD( nNumArgs );
			m_saArgs.resize( nNumArgs );

			for( uint32 nArg = 0; nArg < nNumArgs; ++nArg )
			{
				LOAD_STDSTRING( m_saArgs[ nArg ] );
			}

			LOAD_bool( m_bGlobal );
			LOAD_STDSTRING( m_sCmdName );
			m_cTok_CmdName = m_sCmdName.c_str( );
		}


	private:	// Methods...

		// Don't allow assignment or copying...
		CParsedCmd( const CParsedCmd &oter );
		CParsedCmd& operator=( const CParsedCmd &other );

	public :	// Members...

		// The originator of the command...
		ILTBaseClass		*m_pActiveSender;
		LTObjRefNotifier	m_hActiveSender;

		// The recipient of the command...
		ILTBaseClass		*m_pActiveTarget;
		LTObjRefNotifier	m_hActiveTarget;
		
		// Array of arguments for the command...
		StringArray			m_saArgs;

		// Hashed token for quick equality checks of the command name...
		CParsedMsg::CToken	m_cTok_CmdName;

		// Is the command part of the global namespace.
		bool				m_bGlobal;


	private :	// Members...
	  
		// The buffer for the name of the command...
		std::string			m_sCmdName;

};

class CCmdScript : public ILTObjRefReceiver
{
	public:		// Methods...

		CCmdScript()
		:	m_pScriptedObj	( NULL ),
			m_pActiveSender	( NULL ),
			m_pActiveTarget	( NULL )
		{ 
			m_hScriptedObj.SetReceiver( *this );
			m_hActiveSender.SetReceiver( *this );
			m_hActiveTarget.SetReceiver( *this );
		}
		
		~CCmdScript() {}

		void Update( );
		void Save( ILTMessage_Write *pMsg );
		void Load( ILTMessage_Read *pMsg );

		void Kill( )
		{
			// End the script by removing the commands...
			m_saCmds.resize( 0 );
		}
	
		bool IsFinished()
		{
			// The script is finished when all commands have been queued and it has no scripted object...
			return (m_saCmds.empty() && !m_pScriptedObj);
		}

		void OnLinkBroken(LTObjRefNotifier* /*pRef*/, HOBJECT hObj)
		{
			if( m_pScriptedObj && (m_pScriptedObj->m_hObject == hObj) )
			{
				m_pScriptedObj = NULL;
			}

			if( m_pActiveTarget && (m_pActiveTarget->m_hObject == hObj) )
			{
				m_pActiveTarget = NULL;
			}

			if( m_pActiveSender && (m_pActiveSender->m_hObject == hObj) )
			{
				m_pActiveSender = NULL;
			}
		}

		void SetScriptedObject( ILTBaseClass *pObject )
		{
			// Try to set the hObject first.  If the object is going away
			// m_hActiveTarget will be NULL so don't set m_pScriptedObj.

			m_hScriptedObj = (pObject) ? pObject->m_hObject : NULL;
			m_pScriptedObj = (m_hScriptedObj) ? pObject : NULL;
		}

		void SetActiveTarget( ILTBaseClass *pObject )
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveTarget will be NULL so don't set pActiveTarget.

			m_hActiveTarget = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveTarget = (m_hActiveTarget) ? pObject : NULL;
		}

		void SetActiveSender( ILTBaseClass *pObject )
		{
			// Try to set the hObject first.  If the object is going away
			// hActiveSender will be NULL so don't set pActiveSender.

			m_hActiveSender = (pObject) ? pObject->m_hObject : NULL;
			m_pActiveSender = (m_hActiveSender) ? pObject : NULL;
		}


	public:		// Members...

		// The current object that is being scripted...
		ILTBaseClass		*m_pScriptedObj;
		LTObjRefNotifier	m_hScriptedObj;

		// The originator of the command...
		ILTBaseClass		*m_pActiveSender;
		LTObjRefNotifier	m_hActiveSender;

		// The recipient of the command...
		ILTBaseClass		*m_pActiveTarget;
		LTObjRefNotifier	m_hActiveTarget;
		
		// Array of arguments for the command...
		StringArray			m_saCmds;
};

class CCommandMgr
{
	public :	// Methods...

		CCommandMgr();
		~CCommandMgr();

		void	Load(ILTMessage_Read *pMsg);
		void	Save(ILTMessage_Write *pMsg);

		void	SaveGlobalVars( ILTMessage_Write *pMsg, bool bNameOnly );
		void	LoadGlobalVars( ILTMessage_Read *pMsg, bool bNameOnly );

		void	ListCommands();

		// Place the command on the queue for later processing.
		bool	QueueCommand( const char *pszCommand, ILTBaseClass *pSender, ILTBaseClass *pTarget );
		bool	QueueCommand( const char *pszCommand, HOBJECT hSender, HOBJECT hTarget )
		{
			return QueueCommand( pszCommand, g_pLTServer->HandleToObject( hSender ), g_pLTServer->HandleToObject( hTarget ));
		}

		// Specifically queue a message command to the target.
		bool	QueueMessage( ILTBaseClass *pSender, ILTBaseClass *pTarget, const char *pszMessage );
		bool	QueueMessage( HOBJECT hSender, HOBJECT hTarget, const char *pszMessage )
		{
			return QueueMessage( g_pLTServer->HandleToObject( hSender ), g_pLTServer->HandleToObject( hTarget ), pszMessage );
		}

		// Immediately process the global command.
		bool	AddGlobalCommand( const char *pszCommand, ILTBaseClass *pSender, ILTBaseClass *pTarget );
		bool	AddGlobalCommand( const char *pszCommand, HOBJECT hSender, HOBJECT hTarget )
		{
			return AddGlobalCommand( pszCommand, g_pLTServer->HandleToObject( hSender ), g_pLTServer->HandleToObject( hTarget ));
		}

		bool	Update();
		void	Clear();
		
		// The following methods should only be called via the static cmdmgr_XXX
		// functions...

		bool	ProcessListCommands( const CParsedCmd *pParsedCmd );
		bool	ProcessDelay( const CParsedCmd *pParsedCmd );
		bool	ProcessDelayId( const CParsedCmd *pParsedCmd );
		bool	ProcessMsg( const CParsedCmd *pParsedCmd, bool bObjVar );
		bool	ProcessRandWeight( const CParsedCmd *pParsedCmd );
		bool	ProcessRand( const CParsedCmd *pParsedCmd );
		bool	ProcessRepeat( const CParsedCmd *pParsedCmd );
		bool	ProcessRepeatId( const CParsedCmd *pParsedCmd );
		bool	ProcessLoop( const CParsedCmd *pParsedCmd );
		bool	ProcessLoopId( const CParsedCmd *pParsedCmd );
		bool	ProcessAbort( const CParsedCmd *pParsedCmd );
		bool	ProcessInt( const CParsedCmd *pParsedCmd );
		bool	ProcessObj( const CParsedCmd *pParsedCmd );
		bool	ProcessSet( const CParsedCmd *pParsedCmd );
		bool	ProcessAdd( const CParsedCmd *pParsedCmd );
		bool	ProcessSub( const CParsedCmd *pParsedCmd );
		bool	ProcessIf( const CParsedCmd *pParsedCmd );
		bool	ProcessWhen( const CParsedCmd *pParsedCmd );
		bool	ProcessShowVar( const CParsedCmd *pParsedCmd );
		bool	ProcessScript( const CParsedCmd *pParsedCmd );

		VAR_STRUCT *GetVar( const char *pName, bool bSilent, uint16 *nId = NULL );

		
	private :	// Methods...

		friend class CCmdScript;

		// Allocate and initialize all pools and queues.
		void	Init( );

		// Free all allocated pools and queues.
		void	Term( );

		// Process all the commands on the main queue
		void	ProcessQueuedCommands( );

		// Process a single pre-parsed commands.
		bool	ProcessParsedCmd( CParsedCmd *pParsedCmd );

		// Make sure the parsed command is valid.
		bool	ValidateParsedCmd( const CParsedCmd *pParsedCmd, bool bGlobal );

		// Does the parsed command contain the correct number of arguments.
		bool	CheckArgs( const CParsedCmd *pParsedCmd, uint8 nMin, uint8 nMax );

		// Lock and unlock the main queue.
		void	LockCommandQueue( )	{ m_bCommandQueueLocked = true; }
		void	UnlockCommandQueue( );

		// Add a pending command to the array for later updating...
		bool	AddPendingCommand( const CPendingCmd &crPendingCmd );

		// Update each pending command in the array and add to the command queue if needed...
		void	UpdatePendingCommands( );

		// Run through the scripts and process commands as needed..
		void	UpdateScripts( );

		void	DevPrint(char *msg, ...);

		void	VarChanged( VAR_STRUCT *pVar );

		bool	SendMessageToObject( ILTBaseClass *pSender, ILTBaseClass *pTarget, const char *pszMsg );
		bool	CallMessageHandler( const CCmdMgr_ClassDesc *pClassDesc, ILTBaseClass *pSender, ILTBaseClass *pTargetObj, IAggregate *pTargetAgg, const CParsedMsg &cParsedMsg );

		// Utility function for filling an object list with objects that will recieve a message... 
		void	FindMsgTargets( const char *pszTargetName, BaseObjArray<HOBJECT> &objArray, bool &bSendToPlayer );

		
	private :	// Members...

		typedef std::vector<CParsedCmd*, LTAllocator<CParsedCmd*, LT_MEM_TYPE_OBJECTSHELL> >	TParsedCmdArray;

		// Allocation pool for recycled commands...
		TParsedCmdArray			m_CommandPool;

		// The main Queue for the commands...
		TParsedCmdArray			m_CommandQueue;

		// The SecondaryQueue is used when the main queue is locked.
		// It is a temporary queue and all commands added to it 
		// will be moved to the main queue.
		TParsedCmdArray			m_SecondaryQueue;

		// How many commands have been allocated. Used for debugging.
		uint32					m_dwCommandAllocations;

		// If the main queue is locked all commands will be added to the secondary queue.
		bool					m_bCommandQueueLocked;

		typedef std::vector<CPendingCmd*, LTAllocator<CPendingCmd*, LT_MEM_TYPE_OBJECTSHELL> >	TPendingCmdArray;

		// Allocation pool for recycled pending commands....
		TPendingCmdArray		m_PendingCmdPool;

		// Array of pending commands...
		TPendingCmdArray		m_PendingCmds;

		typedef std::vector<CCmdScript*>	TCmdScriptArray;

		// Allocation pool for recycled scripts...
		TCmdScriptArray			m_CmdScriptPool;

		// The array for all script commands...
		TCmdScriptArray			m_CmdScripts;

		
		VAR_STRUCT				m_aVars[CMDMGR_MAX_VARS];
		uint16					m_nNumVars;

		CMD_EVENT_STRUCT		m_aEventCmds[CMDMGR_MAX_EVENT_COMMANDS];

		// Active target, for use by commands which may use a target
		ILTBaseClass			*m_pActiveTarget;

		// Active sender, for use by commands which may use a sender.
		ILTBaseClass			*m_pActiveSender;
};


//
//	ObjectPlugin class for debugging commands at creation time
//

#include "iobjectplugin.h"
#include "CommandDB.h"

enum
{
	// This class will ignore all messages sent to it when doing a pre check on it.
	CMDMGR_CF_MSGIGNORE	 = (1<<0),
};

enum
{
	// This message blocks all untill the task associated with the message is complete.
	CMDMGR_MF_BLOCKINGMSG	= (1<<0),
};

class CCmdMgr_MsgDesc : public ICommandMessageDef
{
	public :	// Methods...

		CCmdMgr_MsgDesc( const char *pName = "", int32 nMinArgs = 0, int32 nMaxArgs = 0, 
						TValidateMsgFn pValidateFn = NULL, THandleMsgFn pHandleFn = NULL,
						uint32 dwFlags = 0, const char *pSyntax = "", const char *pDesc = "", 
						const char *pExample = "" )
		:	
			ICommandMessageDef	( pName, pSyntax, pDesc, pExample ),
			m_cTok_MsgName		( pName ),
			m_nMinArgs			( nMinArgs ),
			m_nMaxArgs			( nMaxArgs ),
			m_pValidateFn		( pValidateFn ),
			m_pHandleFn			( pHandleFn ),
			m_dwFlags			( dwFlags )
		{ };


	public :	// Members...

		CParsedMsg::CToken	m_cTok_MsgName;
		int32				m_nMinArgs;
		int32				m_nMaxArgs;
		TValidateMsgFn		m_pValidateFn;
		THandleMsgFn		m_pHandleFn;
		uint32				m_dwFlags;
};

class CCmdMgr_ClassDesc : public ICommandClassDef
{
	public :	// Methods...

		CCmdMgr_ClassDesc( const char *pClassName, const char *pParentClass, uint32 nNumMsgs,
							CCmdMgr_MsgDesc *pMsgs, uint32 dwFlags, THandleMsgFn pHandleFn  );


	public :	// Members...

		CParsedMsg::CToken	m_cTok_ClassName;
		CParsedMsg::CToken	m_cTok_ParentClass;
		CCmdMgr_MsgDesc		*m_pMsgs;
		uint32				m_dwFlags;
		THandleMsgFn		m_pHandleFn;
};

typedef std::vector<CCmdMgr_ClassDesc*, LTAllocator<CCmdMgr_ClassDesc*, LT_MEM_TYPE_OBJECTSHELL> > TCmdMgr_ClassDescArray;

#define CMDMGR_BEGIN_REGISTER_CLASS( class_name ) \
	static CCmdMgr_MsgDesc _##class_name##_Msgs_[] = { \
	ADD_MESSAGE( "!__NOMSG__!", 0, NULL, NULL, "", "", "" )

#define _CMDMGR_END_REGISTER_CLASS_INTERNAL_( class_name, parent_class, flags, handle_fn ) \
	}; \
	static CCmdMgr_ClassDesc _CmdMgrClassDesc##class_name(#class_name, \
															#parent_class, \
															sizeof(_##class_name##_Msgs_) / sizeof(CCmdMgr_MsgDesc), \
															&_##class_name##_Msgs_[0], \
															flags, \
															handle_fn );

#define CMDMGR_END_REGISTER_CLASS( class_name, parent_class ) \
	_CMDMGR_END_REGISTER_CLASS_INTERNAL_( class_name, parent_class, 0, NULL )

#define CMDMGR_END_REGISTER_CLASS_FLAGS( class_name, parent_class, flags ) \
	_CMDMGR_END_REGISTER_CLASS_INTERNAL_( class_name, parent_class, flags, NULL )

// Use this for objects that need to pass messages along to other objects. (see Group)
#define CMDMGR_END_REGISTER_CLASS_HANDLER( class_name, parent_class, flags, handle_fn ) \
	_CMDMGR_END_REGISTER_CLASS_INTERNAL_( class_name, parent_class, flags, handle_fn )


// Macros for adding messages to objects for propper handling...

#if defined (COMPILE_OBJECT_DESCRIPTIONS)
	#define _CMDMGR_ADD_MSG_INTERNAL_( msg_name, min_args, max_args, validate_fn, handle_fn, flags, syntax, desc, example ) \
		CCmdMgr_MsgDesc( #msg_name, min_args, max_args, validate_fn, handle_fn, flags, syntax, desc, example ),
#else
	#define _CMDMGR_ADD_MSG_INTERNAL_( msg_name, min_args, max_args, validate_fn, handle_fn, flags, syntax, desc, example ) \
		CCmdMgr_MsgDesc( #msg_name, min_args, max_args, validate_fn, handle_fn, flags, "", "", "" ),
#endif

#define ADD_MESSAGE( msg_name, num_args, validate_fn, handle_fn, syntax, desc, example ) \
	_CMDMGR_ADD_MSG_INTERNAL_( msg_name, num_args, num_args, validate_fn, handle_fn, 0, syntax, desc, example )

#define ADD_MESSAGE_FLAGS( msg_name, num_args, validate_fn, handle_fn, flags, syntax, desc, example ) \
	_CMDMGR_ADD_MSG_INTERNAL_( msg_name, num_args, num_args, validate_fn, handle_fn, flags, syntax, desc, example )

#define ADD_MESSAGE_ARG_RANGE( msg_name, min_args, max_args, validate_fn, handle_fn, syntax, desc, example ) \
	_CMDMGR_ADD_MSG_INTERNAL_( msg_name, min_args, max_args, validate_fn, handle_fn, 0, syntax, desc, example )

#define ADD_MESSAGE_ARG_RANGE_FLAGS( msg_name, min_args, max_args, validate_fn, handle_fn, flags, syntax, desc, example ) \
	_CMDMGR_ADD_MSG_INTERNAL_( msg_name, min_args, max_args, validate_fn, handle_fn, flags, syntax, desc, example )
	

// Macros for adding the handler functions to objects...

#define _CMDMGR_DECLARE_MSG_HANDLER_INTERNAL_( class_name, handler ) \
		public: \
			static void cmdmgr_relay_##handler( HOBJECT hS, ILTBaseClass *pObj, IAggregate *pAgg, const CParsedMsg &crPM ) \
			{ \
				class_name *pClass = NULL; \
				if( pAgg ) \
				{ \
					pClass = dynamic_cast<class_name*>( pAgg ); \
				} \
				else if( pObj ) \
				{ \
					pClass = dynamic_cast<class_name*>( pObj ); \
				} \
				LTASSERT( pClass, "Target object is not of correct type!" ); \
				if( !pClass ) return; \
				pClass->handler( hS, crPM ); \
			} \
		protected: \
			void handler( HOBJECT hSender, const CParsedMsg &crParsedMsg );	

#define DECLARE_MSG_HANDLER( class_name, handler ) \
	_CMDMGR_DECLARE_MSG_HANDLER_INTERNAL_( class_name, handler )


#define _CMDMGR_DECLARE_MSG_HANDLER_AS_PROXY_INTERNAL_( class_name, handler ) \
		public: \
			static void cmdmgr_relay_##handler( HOBJECT hS, ILTBaseClass *pObj, IAggregate *pAgg, const CParsedMsg &crPM ) \
			{ \
				class_name *pClass = NULL; \
				if( pAgg ) \
				{ \
					pClass = dynamic_cast<class_name*>( pAgg ); \
				} \
				else if( pObj ) \
				{ \
					pClass = dynamic_cast<class_name*>( pObj ); \
				} \
				LTASSERT( pClass, "Target object is not of correct type!" ); \
				if( !pClass ) return; \
				pClass->cmdmgr_GetMsgHandlerProxy()->handler( hS, crPM ); \
			} \
		protected: \
			void handler( HOBJECT hSender, const CParsedMsg &crParsedMsg );

#define DECLARE_MSG_HANDLER_AS_PROXY( class_name, handler ) \
	_CMDMGR_DECLARE_MSG_HANDLER_AS_PROXY_INTERNAL_( class_name, handler )

#define DEFINE_MSG_HANDLER_PROXY_CLASS( proxy_class, proxy_var ) \
		friend class proxy_class; \
		proxy_class* cmdmgr_GetMsgHandlerProxy() { return proxy_var; }
	
#define MSG_HANDLER( class_name, handle_fn ) \
		class_name::cmdmgr_relay_##handle_fn


class CCommandMgrPlugin : public IObjectPlugin
{
	public: // Methods...

		virtual LTRESULT PreHook_PropChanged(
				const	char		*szObjName,
				const	char		*szPropName,
				const	int			nPropType,
				const	GenericProp	&gpPropValue,
						ILTPreInterface	*pInterface,
				const	char		*szModifiers );

		// The following methods should only be called via the static cmdmgr_XXX
		// functions...

		bool	PreCheckListCommands( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckMsg( ILTPreInterface *pInterface, ConParse & parse, bool bObjVar );
		bool	PreCheckRandWeight( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckRand( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckRepeat( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckRepeatId( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckDelay( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckDelayId( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckLoop( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckLoopId( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckAbort( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckInt( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckObj( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckSet( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckAdd( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckSub( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckIf( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckWhen( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckShowVar( ILTPreInterface *pInterface, ConParse & parse );
		bool	PreCheckScript( ILTPreInterface *pInterface, ConParse & parse );

		// Checks the validity of the command.
		bool	IsValidCmd( ILTPreInterface *pInterface, const char *pCmd );
		
		// Checks the list of valid commands to see if the string is a command.
		bool	CommandExists( const char *pCmd );

		struct DYNAMIC_OBJECT
		{
			std::string		m_sName;
			std::string		m_sClassName;
		};

		static void	AddDynamicObject( CCommandMgrPlugin::DYNAMIC_OBJECT &DynaObj );

		static char* GetCurrentObjectName() { return s_szCurObject; }

		static void SetForceDisplayPropInfo( bool bForce ) { s_bForceDisplayPropInfo = bForce; }
		
		static bool DoesObjectExist( ILTPreInterface *pInterface, const char *pszObjectName );

		static const char* GetObjectClass( ILTPreInterface *pInterface, const char *pszObjectName );

		static void PrintCmdError( ILTPreInterface* pInterface, const char* pszFunctionName, ConParse& cpMsgParams, const char* pszFormat, ... );


	protected: // Methods...

		bool	CheckArgs( ILTPreInterface *pInterface, ConParse &parse, uint8 nMin, uint8 nMax );
		bool	CheckPendingCommand( ILTPreInterface *pInterface, const CPendingCmd &cmd );
		void	ListObjectMsgs( ILTPreInterface *pInterface, const char *pObj );
		bool	DevelopVars( ILTPreInterface *pInterface );
		VAR_STRUCT*	FindVar( const char *pVarName );
		bool	IsValidExpression( ILTPreInterface *pInterface, ConParse &cpExpression );

		typedef std::vector<DYNAMIC_OBJECT, LTAllocator<DYNAMIC_OBJECT, LT_MEM_TYPE_OBJECTSHELL> > DynamicObjectList;
		static DynamicObjectList s_lstDynaObjects;

		
	private: // Members...

		friend class CCommandDB;

		static VAR_STRUCT		s_aVars[CMDMGR_MAX_VARS];
		static uint16			s_nNumVars;
		static bool				s_bFileLoadError;
		static char				s_szLastWorld[128];
		static char				s_szCurObject[128];
		static bool				s_bCanClearVars;
		static bool				s_bCanClearDynaObjs;
		static bool				s_bDisplayPropInfo;
		static bool				s_bForceDisplayPropInfo;


	public: // Members...
		
		// Modifiers...

		static bool				s_bShowMsgErrors;		// Validates MSG commands but doesn't display any errors
		static bool				s_bShowVarErrors;		// Validates commands dealing with variables but doesn't display errors
		static bool				s_bValidateVarCmds;		// Doesn't validate any commands dealing with variables 
		static bool				s_bValidateNonVarCmds;	// Doesn't validate any non variable commands
		static bool				s_bVarDeclerationsOnly;	// Only validate commands that declare variables
		static bool				s_bValidateVarDecs;		// Doesn't validate any variable decleration commands
		static bool				s_bClearVarsRequested;	// Got a request to clear our variables and rebuild them
		static bool				s_bClearDynaObjsRequested; // Got a rewuest to clear our dynamic objects
		static bool				s_bAddDynamicObjects;	// Allows the adding of dynamic objects to the list
};

// These macros are helpers to wrap the common error block structure:
// 
//		if( CCommandMgrPlugin::s_bShowMsgErrors )
//		{
//			pInterface->ShowDebugWindow( true );
//			pInterface->CPrint( "ERROR! - <FunctionName>()" );
//			pInterface->CPrint( "    MSG - <CommandName> - <Parameters> );
//		}

#define WORLDEDIT_ERROR_MSG( pILTPreInterface, rConParse, pszFormat )	\
	CCommandMgrPlugin::PrintCmdError( pILTPreInterface, __FUNCTION__, rConParse, pszFormat );

#define WORLDEDIT_ERROR_MSG1( pILTPreInterface, rConParse, pszFormat, Arg1 )	\
	CCommandMgrPlugin::PrintCmdError( pILTPreInterface, __FUNCTION__, rConParse, pszFormat, Arg1 );

#define WORLDEDIT_ERROR_MSG2( pILTPreInterface, rConParse, pszFormat, Arg1, Arg2 )	\
	CCommandMgrPlugin::PrintCmdError( pILTPreInterface, __FUNCTION__, rConParse, pszFormat, Arg1, Arg2 );

#define WORLDEDIT_ERROR_MSG3( pILTPreInterface, rConParse, pszFormat, Arg1, Arg2, Arg3 )	\
	CCommandMgrPlugin::PrintCmdError( pILTPreInterface, __FUNCTION__, rConParse, pszFormat, Arg1, Arg2, Arg3 );

#define WORLDEDIT_ERROR_MSG4( pILTPreInterface, rConParse, pszFormat, Arg1, Arg2, Arg3, Arg4 )	\
	CCommandMgrPlugin::PrintCmdError( pILTPreInterface, __FUNCTION__, rConParse, pszFormat, Arg1, Arg2, Arg3, Arg4 );

#endif // __COMMAND_MGR_H__
