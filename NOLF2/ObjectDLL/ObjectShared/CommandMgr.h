// ----------------------------------------------------------------------- //
//
// MODULE  : CommandMgr.h
//
// PURPOSE : CommandMgr definition
//
// CREATED : 06/23/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_MGR_H__
#define __COMMAND_MGR_H__

#include "ServerUtilities.h"
#include "LTObjRef.h"

class ConParse;
class CCommandMgr;
class CCommandMgrPlugin;
extern CCommandMgr* g_pCmdMgr;

#define CMDMGR_MAX_VARS				512
#define CMDMGR_MAX_PENDING_COMMANDS 100
#define CMDMGR_MAX_COMMAND_LENGTH	256
#define CMDMGR_MAX_NAME_LENGTH		32
#define CMDMGR_MAX_VAR_NAME_LENGTH	64 // Prefab names are huge so we need to accomidate for that >:(
#define CMDMGR_MAX_ID_LENGTH		16
#define CMDMGR_NULL_CHAR			'\0'
#define CMDMGR_MAX_VARS_IN_EVENT	16
#define CMDMGR_MAX_EVENT_COMMANDS	32

typedef LTBOOL (*ProcessCmdFn)(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex);
typedef LTBOOL (*PreCheckCmdFn)(CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse );
typedef LTBOOL (*IntOperatorFn)(void *arg1, void *arg2);

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

	void Clear()
	{
		m_szName[0]	= CMDMGR_NULL_CHAR;
		m_eType		= eCMVar_Int;
		m_iVal		= 0;
		m_hObjVal	= 0;
		m_pObjVal	= 0;
		m_bGlobal	= LTFALSE;
		m_nRefCount	= 0;
		m_bShow		= LTFALSE;
		m_bValid	= LTFALSE;
	}

	virtual void OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj)
	{
		m_pObjVal = 0;
	}

	// jrg - 9/7/02 Added bNameOnly flag so global vars can be "declared" before the Events are loaded
	void	Save( ILTMessage_Write *pMsg, LTBOOL bNameOnly );
	void	Load( ILTMessage_Read *pMsg, LTBOOL bNameOnly );

	void	SetObjVal(ILTBaseClass *pObj)
	{
		m_pObjVal = pObj;
		m_hObjVal = (m_pObjVal) ? m_pObjVal->m_hObject : LTNULL;
	}

	char				m_szName[CMDMGR_MAX_VAR_NAME_LENGTH];
	ECmdMgrVarType		m_eType;
	int					m_iVal;
	LTObjRefNotifier	m_hObjVal;
	ILTBaseClass*		m_pObjVal;
	LTBOOL				m_bGlobal;
	int					m_nRefCount;
	LTBOOL				m_bShow;
	LTBOOL				m_bValid;
};

// The operator struct is just for ease of adding new operators and operators that do the same thing (ie. '==', 'equals', 'is')

struct OPERATOR_STRUCT
{
	OPERATOR_STRUCT( char *pOpName = "", LTBOOL bLogical = LTFALSE, IntOperatorFn pOpFn = LTNULL, ECmdMgrVarType eVarType = eCMVar_Int )
		:	m_OpName	( pOpName ),
			m_bLogical	( bLogical ),
			m_OpFn		( pOpFn ),
			m_eVarType	( eVarType )
	{

	}

	char			*m_OpName;
	LTBOOL			m_bLogical;
	IntOperatorFn	m_OpFn;
	ECmdMgrVarType	m_eVarType;

};

// This struct holds the information about an 'event' (ON) command. 

struct CMD_EVENT_STRUCT
{
	CMD_EVENT_STRUCT()
	:	m_hstrExpression	( LTNULL ),
		m_hstrCmds			( LTNULL ),
		m_nNumVarsInCmd		( 0 )
	{
		for( int i = 0; i < CMDMGR_MAX_VARS_IN_EVENT; ++i )
		{
			m_aVars[i] = LTNULL;
			
		}
	}

	~CMD_EVENT_STRUCT()
	{	
		Clear();
	}

	void Clear()
	{
		FREE_HSTRING( m_hstrExpression );
		FREE_HSTRING( m_hstrCmds );

		for( int i = 0; i < CMDMGR_MAX_VARS_IN_EVENT; ++i )
		{
			if( m_aVars[i] )
			{
				--m_aVars[i]->m_nRefCount;
				m_aVars[i] = LTNULL;
			}
		}

		m_nNumVarsInCmd = 0;
	}

	eExpressionVal FillVarArray( ConParse &cpExpression );
	
	LTBOOL	AddVar( VAR_STRUCT *pVar )
	{
		if( !pVar ) return LTFALSE;
		if( m_nNumVarsInCmd >= CMDMGR_MAX_VARS_IN_EVENT ) return LTFALSE;

		// Check to see if we already have this var in our list...

		for( int i = 0; i < m_nNumVarsInCmd; ++i )
		{
			if( m_aVars[i] == pVar )
				return LTTRUE;
		}
		
		// Add it to our list and increment its refcount...

		m_aVars[m_nNumVarsInCmd] = pVar;
		++pVar->m_nRefCount;
		++m_nNumVarsInCmd;

		return LTTRUE;
	}

	LTBOOL	Process();

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	HSTRING		m_hstrExpression;
	HSTRING		m_hstrCmds;
	VAR_STRUCT	*m_aVars[CMDMGR_MAX_VARS_IN_EVENT];	// list of variables that are in the command. 
	uint8		m_nNumVarsInCmd;
};

// This struct holds all the information pending commands...

struct CMD_STRUCT : public ILTObjRefReceiver
{
	CMD_STRUCT()
	{
		Clear();
		hActiveTarget.SetReceiver(*this);
		hActiveSender.SetReceiver(*this);
	}

	void Clear()
	{
		fDelay			= fMinDelay = fMaxDelay = 0.0f;
		nNumTimes		= nMinTimes	= nMaxTimes	= -1;
		hActiveTarget	= LTNULL;
		pActiveTarget	= LTNULL;
		aCmd[0]			= CMDMGR_NULL_CHAR;
		aId[0]			= CMDMGR_NULL_CHAR;
	}

	virtual void OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj)
	{
		if (pActiveTarget && (pActiveTarget->m_hObject == hObj))
			pActiveTarget = LTNULL;
		if (pActiveSender && (pActiveSender->m_hObject == hObj))
			pActiveSender = LTNULL;
	}

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	void SetActiveTarget(ILTBaseClass *pObject)
	{
		// Try to set the hObject first.  If the object is going away
		// hActiveTarget will be NULL so don't set pActiveTarget.
		
		hActiveTarget = (pObject) ? pObject->m_hObject : LTNULL;
		pActiveTarget = (hActiveTarget) ? pObject : LTNULL;
		
	}

	void SetActiveSender(ILTBaseClass *pObject)
	{
		// Try to set the hObject first.  If the object is going away
		// hActiveSender will be NULL so don't set pActiveSender.

		hActiveSender = (pObject) ? pObject->m_hObject : LTNULL;
		pActiveSender = (hActiveSender) ? pObject : LTNULL;
	}

	float				fDelay;
	float				fMinDelay;
	float				fMaxDelay;
	int					nNumTimes;
	int					nMinTimes;
	int					nMaxTimes;
	LTObjRefNotifier	hActiveTarget;
	ILTBaseClass*		pActiveTarget;
	LTObjRefNotifier	hActiveSender;
	ILTBaseClass*		pActiveSender;
	char				aCmd[CMDMGR_MAX_COMMAND_LENGTH];
	char				aId[CMDMGR_MAX_ID_LENGTH];
};

// This struct is basically exactly the same as the above struct, except that it
// uses pointers instead of arrays.  The idea is to use this struct to pass
// command data between functions without having to copy buffers around (see
// CCommandMgr::AddDelayedCmd() )

struct CMD_STRUCT_PARAM
{
	CMD_STRUCT_PARAM()
	{
		fDelay		= fMinDelay = fMaxDelay = 0.0f;
		nNumTimes	= nMinTimes	= nMaxTimes	= -1;
		pCmd		= 0;
		pId			= 0;
	}

	float	fDelay;
	float	fMinDelay;
	float	fMaxDelay;
	int		nNumTimes;
	int		nMinTimes;
	int		nMaxTimes;
	char*	pCmd;
	char*	pId;
};

struct CMD_PROCESS_STRUCT
{
    CMD_PROCESS_STRUCT(char* pCmd="", int nArgs=0, ProcessCmdFn pFn=LTNULL, char* pSyn="", PreCheckCmdFn pPreFn=LTNULL)
	{
		pCmdName	= pCmd;
		nNumArgs	= nArgs;
		pProcessFn	= pFn;
		pSyntax		= pSyn;
		pPreCheckFn	= pPreFn;
	}

	char*			pCmdName;
	char*			pSyntax;
	int				nNumArgs;
	ProcessCmdFn	pProcessFn;
	PreCheckCmdFn	pPreCheckFn;
};

class CCommandMgr
{
	public :

		CCommandMgr();
		~CCommandMgr();

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		void	SaveGlobalVars( ILTMessage_Write *pMsg, LTBOOL bNameOnly );
		void	LoadGlobalVars( ILTMessage_Read *pMsg, LTBOOL bNameOnly );

		void	ListCommands();
        LTBOOL  IsValidCmd(const char* pCmd);
        LTBOOL  Process(const char* pCmd, ILTBaseClass *pSender, ILTBaseClass *pDefaultTarget);
        inline LTBOOL  Process(const char* pCmd, HOBJECT hSender, HOBJECT hDefaultTarget)
		{
			return Process(pCmd, g_pLTServer->HandleToObject(hSender), g_pLTServer->HandleToObject(hDefaultTarget));
		}
        LTBOOL  Update();

		void	Clear();

		// The following methods should only be called via the static cmdmgr_XXX
		// functions...

        LTBOOL	ProcessListCommands(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessDelay(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessDelayId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessMsg(ConParse & parse, int nCmdIndex, bool bObjVar);
        LTBOOL  ProcessRand(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessRandArgs(ConParse & parse, int nCmdIndex, int nNumArgs);
        LTBOOL  ProcessRepeat(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessRepeatId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessLoop(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessLoopId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessAbort(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessInt(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessObj(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessSet(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessAdd(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessSub(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessIf(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessWhen(ConParse & parse, int nCmdIndex);
		LTBOOL	ProcessShowVar(ConParse & parse, int nCmdIndex);

		VAR_STRUCT *GetVar( const char *pName, bool bSilent, uint16 *nId = LTNULL );

	private :

        LTBOOL	ProcessCmd(const char* pCmd, int nCmdIndex=-1);

        LTBOOL  AddDelayedCmd(CMD_STRUCT_PARAM & cmd, int nCmdIndex);
        LTBOOL  CheckArgs(ConParse & parse, int nNum);
		void	DevPrint(char *msg, ...);

		void	VarChanged( VAR_STRUCT *pVar );

	private :

		CMD_STRUCT	m_PendingCmds[CMDMGR_MAX_PENDING_COMMANDS];
		
		VAR_STRUCT	m_aVars[CMDMGR_MAX_VARS];
		uint16		m_nNumVars;

		CMD_EVENT_STRUCT	m_aEventCmds[CMDMGR_MAX_EVENT_COMMANDS];

		// Active target, for use by commands which may use a target
		ILTBaseClass*	m_pActiveTarget;

		// Active sender, for use by commands which may use a sender.
		ILTBaseClass*	m_pActiveSender;
};

inline void	CCommandMgr::Clear()
{
	// Clear all our commands...

	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Clear();
	}

	// Clear all our vars...

	for(int i = 0; i < m_nNumVars; ++i )
	{
		m_aVars[i].Clear();
	}

	m_nNumVars = 0;
}


//
//	ObjectPlugin class for debugging commands at creation time
//

#include "iobjectplugin.h"
#include "CommandButeMgr.h"

enum
{
	// This class will ignore all messages sent to it when doing a pre check on it.
	CMDMGR_CF_MSGIGNORE	 = (1<<0),
};

typedef LTBOOL (*ValidateMsgFn)( ILTPreInterface *pInterface, ConParse &cpMsgParams );

struct MSG_PRECHECK
{
	MSG_PRECHECK( char *pName = "", int nMinArgs = 0, int nMaxArgs = 0, 
					ValidateMsgFn pFn = LTNULL, char *pSyntax = "", bool bSpecial = false )
	{
		m_szMsgName		= pName;
		m_nMinArgs		= nMinArgs;
		m_nMaxArgs		= nMaxArgs;
		m_pValidateFn	= pFn;
		m_szSyntax		= pSyntax;
		m_bSpecial		= bSpecial;
	};

	char			*m_szMsgName;
	int				m_nMinArgs;
	int				m_nMaxArgs;
	char			*m_szSyntax;
	ValidateMsgFn	m_pValidateFn;
	bool			m_bSpecial;
};

struct CMDMGR_CLASS_DESC
{
	CMDMGR_CLASS_DESC( char *pClassName, char *pParentClass, int nNumMsgs, MSG_PRECHECK *pMsgs, uint32 dwFlags  );

	char			*m_szClassName;
	char			*m_szParentClass;
	int				m_nNumMsgs;
	MSG_PRECHECK	*m_pMsgs;
	uint32			m_dwFlags;
};

typedef std::vector<CMDMGR_CLASS_DESC*> CMDMGR_CLASS_DESC_VECTOR;


#define CMDMGR_BEGIN_REGISTER_CLASS( class_name ) \
	static MSG_PRECHECK _##class_name##_Msgs_[] = { \
		CMDMGR_ADD_MSG( "!__NOMSG__!", 0, NULL, "" )

#define CMDMGR_END_REGISTER_CLASS_FLAGS( class_name, parent_class, flags ) \
	}; \
	static CMDMGR_CLASS_DESC _CmdMgrClassDesc##class_name##(#class_name, \
															#parent_class, \
															sizeof(_##class_name##_Msgs_) / sizeof(MSG_PRECHECK), \
															&_##class_name##_Msgs_[0], \
															flags );

#define CMDMGR_END_REGISTER_CLASS( class_name, parent_class ) \
	CMDMGR_END_REGISTER_CLASS_FLAGS( class_name, parent_class, 0 )
	 

#define CMDMGR_ADD_MSG( msg_name, num_args, validate_fn, syntax ) \
	MSG_PRECHECK( #msg_name, num_args, num_args, validate_fn, syntax ),

#define CMDMGR_ADD_MSG_ARG_RANGE( msg_name, min_args, max_args, validate_fn, syntax ) \
	MSG_PRECHECK( #msg_name, min_args, max_args, validate_fn, syntax ),

#define CMDMGR_ADD_MSG_SPECIAL( validate_fn ) \
	MSG_PRECHECK( "!__SPECIAL__!", -1, -1, validate_fn, "", true ),

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

        LTBOOL	PreCheckListCommands( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL  PreCheckMsg( ILTPreInterface *pInterface, ConParse & parse, bool bObjVar );
		LTBOOL  PreCheckRand( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL  PreCheckRandArgs( ILTPreInterface *pInterface, ConParse & parse, int nNumArgs );
        LTBOOL  PreCheckRepeat( ILTPreInterface *pInterface, ConParse & parse );
        LTBOOL  PreCheckRepeatId( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL  PreCheckDelay( ILTPreInterface *pInterface, ConParse & parse );
        LTBOOL  PreCheckDelayId( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL  PreCheckLoop( ILTPreInterface *pInterface, ConParse & parse );
        LTBOOL  PreCheckLoopId( ILTPreInterface *pInterface, ConParse & parse );
        LTBOOL  PreCheckAbort( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckInt( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckObj( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckSet( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckAdd( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckSub( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckIf( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckWhen( ILTPreInterface *pInterface, ConParse & parse );
		LTBOOL	PreCheckShowVar( ILTPreInterface *pInterface, ConParse & parse );

		// Checks the validity of the command.
		LTBOOL	IsValidCmd( ILTPreInterface *pInterface, const char *pCmd );
		
		// Checks the list of valid commands to see if the string is a command.
		LTBOOL	CommandExists( const char *pCmd );

		struct DYNAMIC_OBJECT
		{
			std::string		m_sName;
			std::string		m_sClassName;
		};

		static void	AddDynamicObject( CCommandMgrPlugin::DYNAMIC_OBJECT &DynaObj );

		static char* GetCurrentObjectName() { return s_szCurObject; }

		static void SetForceDisplayPropInfo( bool bForce ) { s_bForceDisplayPropInfo = bForce; }
		

	protected: // Methods...

		LTBOOL	CheckArgs( ILTPreInterface *pInterface, ConParse &parse, int nNum );
		LTBOOL	CheckDelayedCmd( ILTPreInterface *pInterface, CMD_STRUCT_PARAM &cmd );
		void	ListObjectMsgs( ILTPreInterface *pInterface, const char *pObj );
		LTBOOL	DevelopVars( ILTPreInterface *pInterface );
		VAR_STRUCT*	FindVar( const char *pVarName );
		LTBOOL	IsValidExpression( ILTPreInterface *pInterface, ConParse &cpExpression );

		typedef std::vector<DYNAMIC_OBJECT> DynamicObjectList;
		static DynamicObjectList s_lstDynaObjects;

		
	private: // Members...

		friend class CCommandButeMgr;

		static VAR_STRUCT		s_aVars[CMDMGR_MAX_VARS];
		static uint8			s_nNumVars;
		static CCommandButeMgr	s_CommandButeMgr;
		static LTBOOL			s_bFileLoadError;
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

#endif // __COMMAND_MGR_H__
