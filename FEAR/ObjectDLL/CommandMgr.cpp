// ----------------------------------------------------------------------- //
//
// MODULE  : CommandMgr.cpp
//
// PURPOSE : CommandMgr implemenation
//
// CREATED : 06/23/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "CommandMgr.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "EngineTimer.h"
#include "GameModeMgr.h"

CCommandMgr* g_pCmdMgr = NULL;

extern const char	*g_sPlayerClass;

extern VarTrack	g_ShowMessagesTrack;
extern VarTrack	g_ShowMessagesFilter;

static TCmdMgr_ClassDescArray *s_pVecClassDesc = NULL;

#define INITIAL_CMDS_PER_FRAME	32
#define INITIAL_PENDING_CMDS	16
#define INITIAL_SCRIPT_CMDS		8


// Following are wrapper functions to make adding new commands a little
// easier.
//
// To add a new command simply add a new ProcessXXX function in CommandMgr
// to process the command.  Then add a new cmdmgr_XXX static function that
// calls the new ProcessXXX function.  Then add a new entry in the s_ValidCmds
// array to map your new command to the appropriate processor function.

static bool cmdmgr_ListCmds(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessListCommands( pParsedCmd );
    return false;
}

static bool cmdmgr_Delay(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessDelay( pParsedCmd );
    return false;
}

static bool cmdmgr_DelayId(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessDelayId( pParsedCmd );
    return false;
}

static bool cmdmgr_Msg(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd)
{
	if (pCmdMgr) return pCmdMgr->ProcessMsg( pParsedCmd, false );
    return false;
}

static bool cmdmgr_VMsg(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessMsg( pParsedCmd, true );
    return false;
}

static bool cmdmgr_Rand(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessRand( pParsedCmd );
    return false;
}

static bool cmdmgr_RandWeight(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessRandWeight( pParsedCmd );
	return false;
}

static bool cmdmgr_Repeat(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessRepeat( pParsedCmd );
    return false;
}

static bool cmdmgr_RepeatId(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessRepeatId( pParsedCmd );
    return false;
}

static bool cmdmgr_Loop(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessLoop( pParsedCmd );
    return false;
}

static bool cmdmgr_LoopId(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessLoopId( pParsedCmd );
    return false;
}

static bool cmdmgr_Abort(CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if (pCmdMgr) return pCmdMgr->ProcessAbort( pParsedCmd );
    return false;
}

static bool cmdmgr_Int( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessInt( pParsedCmd );
	return false;
}

static bool cmdmgr_Obj( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessObj( pParsedCmd );
	return false;
}

static bool cmdmgr_Set( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessSet( pParsedCmd );
	return false;
}

static bool cmdmgr_Add( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessAdd( pParsedCmd );
	return false;
}

static bool cmdmgr_Sub( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessSub( pParsedCmd );
	return false;
}

static bool cmdmgr_If( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessIf( pParsedCmd );
	return false;
}

static bool cmdmgr_When( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessWhen( pParsedCmd );
	return false;
}

static bool cmdmgr_ShowVar( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessShowVar( pParsedCmd );
	return false;
}

static bool cmdmgr_Script( CCommandMgr *pCmdMgr, const CParsedCmd *pParsedCmd )
{
	if( pCmdMgr ) return pCmdMgr->ProcessScript( pParsedCmd );
	return false;
}


///////////////////////////////////
// PreCheck methods
///////////////////////////////////

static bool cmdmgr_PreListCmds( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckListCommands( pInterface, parse );
	return false;
}

static bool cmdmgr_PreMsg( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckMsg( pInterface, parse, false );
	return false;
}

static bool cmdmgr_PreVMsg( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckMsg( pInterface, parse, true );
	return false;
}

static bool cmdmgr_PreRand( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRand( pInterface, parse );
	return false;
}

static bool cmdmgr_PreRandWeight( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandWeight( pInterface, parse );
	return false;
}

static bool cmdmgr_PreRepeat( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRepeat( pInterface, parse );
	return false;
}

static bool cmdmgr_PreRepeatId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRepeatId( pInterface, parse );
	return false;
}

static bool cmdmgr_PreDelay( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckDelay( pInterface, parse );
	return false;
}

static bool cmdmgr_PreDelayId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckDelayId( pInterface, parse );
	return false;
}

static bool cmdmgr_PreLoop( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckLoop( pInterface, parse );
	return false;
}

static bool cmdmgr_PreLoopId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckLoopId( pInterface, parse );
	return false;
}

static bool cmdmgr_PreAbort( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckAbort( pInterface, parse );
	return false;
}

static bool cmdmgr_PreInt( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckInt( pInterface, parse );
	return false;
}

static bool cmdmgr_PreObj( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckObj( pInterface, parse );
	return false;
}

static bool cmdmgr_PreSet( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckSet( pInterface, parse );
	return false;
}

static bool cmdmgr_PreAdd( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckAdd( pInterface, parse );
	return false;
}

static bool cmdmgr_PreSub( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckSub( pInterface, parse );
	return false;
}

static bool cmdmgr_PreIf( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckIf( pInterface, parse );
	return false;
}

static bool cmdmgr_PreWhen( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckWhen( pInterface, parse );
	return false;
}

static bool cmdmgr_PreShowVar( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckShowVar( pInterface, parse );
	return false;
}

static bool cmdmgr_PreScript( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckScript( pInterface, parse );
	return false;
}


#define CMDMGR_ADD_CMD( cmd_name, args, global, process_fn, validate_fn, syntax ) \
	CCommandDescription( #cmd_name, args, args, global, process_fn, validate_fn, syntax ),

#define CMDMGR_ADD_CMD_ARGS( cmd_name, minargs, maxargs, global, process_fn, validate_fn, syntax ) \
	CCommandDescription( #cmd_name, minargs, maxargs, global, process_fn, validate_fn, syntax ),

#define GLOBAL true

static CCommandDescription s_ValidCmds[] =
{
	CMDMGR_ADD_CMD( MSG,		2,	!GLOBAL,	cmdmgr_Msg,			cmdmgr_PreMsg,			"  MSG <object name(s)> <object msg>" )
	CMDMGR_ADD_CMD( DELAY,		2,	!GLOBAL,	cmdmgr_Delay,		cmdmgr_PreDelay,		"  DELAY <time> <cmd>" )
	CMDMGR_ADD_CMD_ARGS( RAND,	1,	32,	!GLOBAL,cmdmgr_Rand,		cmdmgr_PreRand,			"  RAND <cmd1> <cmd2> ... <cmdN>" )
	CMDMGR_ADD_CMD( RANDWEIGHT,	3,	!GLOBAL,	cmdmgr_RandWeight,	cmdmgr_PreRandWeight,	"  RANDWEIGHT <cmd1 percent> <cmd1> <cmd2>" )
	CMDMGR_ADD_CMD( REPEAT,		5,	!GLOBAL,	cmdmgr_Repeat,		cmdmgr_PreRepeat,		"  REPEAT <min times> <max times> <min delay> <max delay> <cmd>" )
	CMDMGR_ADD_CMD( REPEATID,	6,	!GLOBAL,	cmdmgr_RepeatId,	cmdmgr_PreRepeatId,		"  REPEATID <cmd id> <min times> <max times> <min delay> <max delay> <cmd>" )
	CMDMGR_ADD_CMD( DELAYID,	3,	!GLOBAL,	cmdmgr_DelayId,		cmdmgr_PreDelayId,		"  DELAYID <cmd id> <time> <cmd>" )
	CMDMGR_ADD_CMD( LOOP,		3,	!GLOBAL,	cmdmgr_Loop,		cmdmgr_PreLoop,			"  LOOP <min delay> <max delay> <cmd>" )
	CMDMGR_ADD_CMD( LOOPID,		4,	!GLOBAL,	cmdmgr_LoopId,		cmdmgr_PreLoopId,		"  LOOPID <cmd id> <min delay> <max delay> <cmd>" )
	CMDMGR_ADD_CMD( ABORT,		1,	!GLOBAL,	cmdmgr_Abort,		cmdmgr_PreAbort,		"  ABORT <cmd id>" )
	CMDMGR_ADD_CMD_ARGS( SCRIPT,1,	32,	!GLOBAL,cmdmgr_Script,		cmdmgr_PreScript,		"  SCRIPT <cmd1> <cmd2> ... <cmdN>" )
	CMDMGR_ADD_CMD( SET,		2,	!GLOBAL,	cmdmgr_Set,			cmdmgr_PreSet,			"  SET <variable> <value>" )
	CMDMGR_ADD_CMD( ADD,		2,	!GLOBAL,	cmdmgr_Add,			cmdmgr_PreAdd,			"  ADD <variable> <amount>" )
	CMDMGR_ADD_CMD( SUB,		2,	!GLOBAL,	cmdmgr_Sub,			cmdmgr_PreSub,			"  SUB <variable> <amount>" )
	CMDMGR_ADD_CMD( IF,			3,	!GLOBAL,	cmdmgr_If,			cmdmgr_PreIf,			"  IF <expressions> THEN <commands>" )
	CMDMGR_ADD_CMD( INT,		2,	GLOBAL,		cmdmgr_Int,			cmdmgr_PreInt,			"  INT <variable> <value>" )
	CMDMGR_ADD_CMD( OBJ,		2,	!GLOBAL,	cmdmgr_Obj,			cmdmgr_PreObj,			"  OBJ <variable> <value>" )
	CMDMGR_ADD_CMD( WHEN,		3,	!GLOBAL,	cmdmgr_When,		cmdmgr_PreWhen,			"  WHEN <expressions> THEN <commands>" )
	CMDMGR_ADD_CMD( SHOWVAR,	2,	!GLOBAL,	cmdmgr_ShowVar,		cmdmgr_PreShowVar,		"  SHOWVAR <variable> <1 or 0>" )
	CMDMGR_ADD_CMD( VMSG,		2,	!GLOBAL,	cmdmgr_VMsg,		cmdmgr_PreVMsg,			"  MSG <object name(s)> <object msg>" )
	CMDMGR_ADD_CMD( LISTCMDS,	0,	!GLOBAL,	cmdmgr_ListCmds,	cmdmgr_PreListCmds,		"Available Object Commands:" )
};

const int c_nNumValidCmds = sizeof(s_ValidCmds)/sizeof(s_ValidCmds[0]);

CCommandDescription* GetCommandDescriptions()
{
	return s_ValidCmds;
}

uint32	GetNumCommandDescriptions()
{
	return c_nNumValidCmds;
}

///////////////////////////////////
// Operator methods
///////////////////////////////////

static eExpressionVal CheckExpression( ConParse &cpExpression );

static bool Op_Int_equals( void *arg1, void *arg2 )
{
	return( *(int*)arg1 == *(int*)arg2 );
}

static bool Op_Int_notequals( void *arg1, void *arg2 )
{
	return( *(int*)arg1 != *(int*)arg2 );
}

static bool Op_Int_greaterthan( void *arg1, void *arg2 )
{
	return( *(int*)arg1 > *(int*)arg2 );
}

static bool Op_Int_lessthan( void *arg1, void *arg2 )
{
	return( *(int*)arg1 < *(int*)arg2 );
}

static bool Op_Int_greaterthan_equalto( void *arg1, void *arg2 )
{
	return( *(int*)arg1 >= *(int*)arg2 );
}

static bool Op_Int_lessthan_equalto( void *arg1, void *arg2 )
{
	return( *(int*)arg1 <= *(int*)arg2 );
}

static bool Op_Logical_and( void *arg1, void *arg2 )
{
	ConParse cpArg1;
	cpArg1.Init( (char*)arg1 );

	if( g_pCommonLT->Parse( &cpArg1 ) == LT_OK )
	{
		if( CheckExpression( cpArg1 ) == kExpress_TRUE )
		{
			ConParse cpArg2;
			cpArg2.Init( (char*)arg2 );

			if( g_pCommonLT->Parse( &cpArg2 ) == LT_OK )
			{
				if( CheckExpression( cpArg2 ) == kExpress_TRUE )
					return true;
			}
		}
	}

	return false;
}

static bool Op_Logical_or( void *arg1, void *arg2 )
{
	ConParse cpArg1;
	cpArg1.Init( (char*)arg1 );

	ConParse cpArg2;
	cpArg2.Init( (char*)arg2 );

	if( (g_pCommonLT->Parse( &cpArg1 ) == LT_OK) && (g_pCommonLT->Parse( &cpArg2 ) == LT_OK) )
	{
		if( (CheckExpression( cpArg1 ) == kExpress_TRUE) || (CheckExpression( cpArg2 ) == kExpress_TRUE) )
			return true;
	}

	return false;
}

static OPERATOR_STRUCT s_Operators[] =
{
					// Name						Logical?	Function
	// Integer
	OPERATOR_STRUCT( "==",						false,	Op_Int_equals ),
	OPERATOR_STRUCT( "EQUALS",					false,	Op_Int_equals ),
	OPERATOR_STRUCT( "!=",						false,	Op_Int_notequals ),
	OPERATOR_STRUCT( "NOT_EQUALS",				false,	Op_Int_notequals ),
	OPERATOR_STRUCT( ">",						false,	Op_Int_greaterthan ),
	OPERATOR_STRUCT( "GREATER_THAN",			false,	Op_Int_greaterthan ),
	OPERATOR_STRUCT( "<",						false,	Op_Int_lessthan ),
	OPERATOR_STRUCT( "LESS_THAN",				false,	Op_Int_lessthan ),
	OPERATOR_STRUCT( ">=",						false,	Op_Int_greaterthan_equalto ),
	OPERATOR_STRUCT( "GREATER_THAN_OR_EQUAL_TO",false,	Op_Int_greaterthan_equalto ),
	OPERATOR_STRUCT( "<=",						false,	Op_Int_lessthan_equalto ),
	OPERATOR_STRUCT( "LESS_THAN_OR_EQUAL_TO",	false,	Op_Int_lessthan_equalto ),
	OPERATOR_STRUCT( "&&",						true,		Op_Logical_and ),
	OPERATOR_STRUCT( "AND",						true,		Op_Logical_and ),
	OPERATOR_STRUCT( "||",						true,		Op_Logical_or ),
	OPERATOR_STRUCT( "OR",						true,		Op_Logical_or ),
	// Object
	OPERATOR_STRUCT( "==",						false,	Op_Int_equals,		eCMVar_Obj ),
	OPERATOR_STRUCT( "EQUALS",					false,	Op_Int_equals,		eCMVar_Obj ),
	OPERATOR_STRUCT( "!=",						false,	Op_Int_notequals,	eCMVar_Obj ),
	OPERATOR_STRUCT( "NOT_EQUALS",				false,	Op_Int_notequals,	eCMVar_Obj )
};

const int c_NumOperators = sizeof( s_Operators ) / sizeof( s_Operators[0] );


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	eExpressionVal CheckExpression
//
//  PURPOSE:	Break the expression into it's three components and depending
//				if the operator is a logical, further break down the arguments 
//				as expressions.  If it's a comparison operator get the value of the 
//				two arguments and send it to the operator function to be evaluated. 
//
// ----------------------------------------------------------------------- //

static eExpressionVal CheckExpression( ConParse &cpExpression )
{
	if( cpExpression.m_nArgs != 3 )
		return kExpress_ERROR;

	char *pArg1 = cpExpression.m_Args[0];
	const char *pOp = cpExpression.m_Args[1];
	char *pArg2 = cpExpression.m_Args[2];

	if( !pArg1 || !pOp || !pArg2 || !g_pCmdMgr )
		return kExpress_ERROR;

	// Make sure the operator is valid...

	for( int iOp = 0; iOp < c_NumOperators; ++iOp )
	{
		if( !LTStrICmp( s_Operators[iOp].m_OpName, pOp ))
		{
			OPERATOR_STRUCT *pOperator = &s_Operators[iOp];

			if( pOperator->m_bLogical )
			{
				// For logical operators just pass the args to the operator...
				// It will recurse into this function to evaluate the two expressions

				if( pOperator->m_OpFn )
					return (eExpressionVal)pOperator->m_OpFn( pArg1, pArg2 );
			}
			
			// It's not a logical, therefore the first arg must be a variable...

			VAR_STRUCT *pVar1 = g_pCmdMgr->GetVar( pArg1, false );
			if( !pVar1 )	return kExpress_ERROR;

			// Make sure we're operating on the type of object we're looking for

			if( pVar1->m_eType != pOperator->m_eVarType )
			{
				continue;
			}

			int	nValue1 = (pVar1->m_eType == eCMVar_Obj) ? (int)pVar1->m_pObjVal : pVar1->m_iVal;
			int nValue2 = 0;
					
			// Is the second arg a number value or another variable...

			if( (pArg2[0] >= '0') && (pArg2[0] <= '9') )
			{
				// Integer comparisons are only valid on integers

				if( pVar1->m_eType != eCMVar_Int )
					return kExpress_ERROR;

				nValue2 = atoi( pArg2 );
			}
			else
			{	
				VAR_STRUCT *pVar2 = g_pCmdMgr->GetVar( pArg2, true );
				if( !pVar2 )
				{
					// If it's not a variable, and it's not an object comparison, this is invalid

					if( pVar1->m_eType != eCMVar_Obj )
					{
						// Display the proper error message...
						g_pCmdMgr->GetVar( pArg2, false );
						return kExpress_ERROR;
					}

					// Find the object

					ILTBaseClass *pVar2Obj = 0;
					FindNamedObject( pArg2, pVar2Obj, true );
					nValue2 = (int)pVar2Obj;
				}
				else if( pVar1->m_eType != pVar2->m_eType )
				{
					return kExpress_ERROR;
				}
				else
				{
					// Get the comparison value

					nValue2 = (pVar2->m_eType == eCMVar_Obj) ? (int)pVar2->m_pObjVal : pVar2->m_iVal;
				}
			}
					
			// Compare the two values based on the operator...

			if( pOperator->m_OpFn )
				return (eExpressionVal)pOperator->m_OpFn( &nValue1, &nValue2 );
			
		}
	}

	return kExpress_ERROR;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetCmdmgrClassDescription
//
//  PURPOSE:	Grab the requested record of a class description
//
// ----------------------------------------------------------------------- //

CCmdMgr_ClassDesc *GetCmdmgrClassDescription( const char *pClassName )
{
	if( !pClassName || !s_pVecClassDesc )
		return NULL;

	CCmdMgr_ClassDesc	*pClassDesc;
	CParsedMsg::CToken	cTok_ClassName( pClassName );
    
	TCmdMgr_ClassDescArray::iterator iter;
	for( iter = s_pVecClassDesc->begin(); iter != s_pVecClassDesc->end(); ++iter )
	{
		pClassDesc = *iter;
		if( pClassDesc->m_cTok_ClassName == cTok_ClassName )
		{
			return pClassDesc;	
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetCmdmgrClassDescription
//
//  PURPOSE:	Grab the requested record of a class description
//
// ----------------------------------------------------------------------- //

CCmdMgr_ClassDesc *GetCmdmgrClassDescription( uint32 nIndex )
{
	if( !s_pVecClassDesc || s_pVecClassDesc->empty() )
		return NULL;

	return s_pVecClassDesc->at( nIndex );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetNumCmdmgrClassDescriptions
//
//  PURPOSE:	Grab the number of class descriptions...
//
// ----------------------------------------------------------------------- //

uint32 GetNumCmdmgrClassDescriptions()
{
	if( !s_pVecClassDesc )
		return 0;

	return s_pVecClassDesc->size();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMDMGR_CLASS_DESC::CMDMGR_CLASS_DESC
//
//  PURPOSE:	Constructor adds this instance to the static vector of class descriptions
//
// ----------------------------------------------------------------------- //

CCmdMgr_ClassDesc::CCmdMgr_ClassDesc( const char *pClassName, const char *pParentClass, uint32 nNumMsgs,
									 CCmdMgr_MsgDesc *pMsgs, uint32 dwFlags, THandleMsgFn pHandleFn )
:	ICommandClassDef( pClassName, pParentClass, nNumMsgs )
{
	if( nNumMsgs < CMDMGR_MIN_CLASSMSGS )
		return;

	// Declare static vector here to guarantee it exists before trying to use it.

	static TCmdMgr_ClassDescArray s_vecClassDesc;
	s_pVecClassDesc = &s_vecClassDesc; 


	m_cTok_ClassName	= pClassName;
	m_cTok_ParentClass	= pParentClass;
	m_pMsgs				= pMsgs;
	m_dwFlags			= dwFlags;
	m_pHandleFn			= pHandleFn;

	// Add the class description.

	s_vecClassDesc.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CCommandMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCommandMgr::CCommandMgr()
:	m_nNumVars				( 0 ),
	m_dwCommandAllocations	( 0 ),
	m_pActiveTarget			( NULL ),
	m_pActiveSender			( NULL )
{
	g_pCmdMgr = this;

	Init( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::~CCommandMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCommandMgr::~CCommandMgr()
{
    g_pCmdMgr = NULL;

	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Init()
//
//	PURPOSE:	Allocate and initialize all pools and queues.
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Init( )
{
	Term( );

	// Initialize the command pool...

	m_CommandQueue.reserve( INITIAL_CMDS_PER_FRAME );
	m_CommandPool.reserve( INITIAL_CMDS_PER_FRAME );
	
	CParsedCmd *pParsedCmd = NULL;
	for( uint32 nCmd = 0; nCmd < INITIAL_CMDS_PER_FRAME; ++nCmd )
	{
		pParsedCmd = debug_new( CParsedCmd );
		if( pParsedCmd )
		{
			++m_dwCommandAllocations;
			m_CommandPool.push_back( pParsedCmd );
		}
	}

	// Initialize the pending command pool...

	m_PendingCmds.reserve( INITIAL_PENDING_CMDS );
	m_PendingCmdPool.reserve( INITIAL_PENDING_CMDS );
	
	CPendingCmd *pPendingCmd = NULL;
	for( uint32 nPendingCmd = 0; nPendingCmd < INITIAL_PENDING_CMDS; ++nPendingCmd )
	{
		pPendingCmd = debug_new( CPendingCmd );
		if( pPendingCmd )
		{
			m_PendingCmdPool.push_back( pPendingCmd );
		}
	}

	// Initialize the script pool...

	m_CmdScripts.reserve( INITIAL_SCRIPT_CMDS );
	m_CmdScriptPool.reserve( INITIAL_SCRIPT_CMDS );

	CCmdScript *pCmdScript = NULL;
	for( uint32 nCmdScript = 0; nCmdScript < INITIAL_SCRIPT_CMDS; ++nCmdScript )
	{
		pCmdScript = debug_new( CCmdScript );
		if( pCmdScript )
		{
			m_CmdScriptPool.push_back( pCmdScript );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Term()
//
//	PURPOSE:	Free all allocated pools and queues.
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Term( )
{
	LTASSERT( (m_CommandPool.size() + m_CommandQueue.size()) == m_dwCommandAllocations, "Deleting a different number of commands than were allocated!" );

	// Delete all commands allocated in the pool...

	TParsedCmdArray::iterator iterParsedCmd;
	for( iterParsedCmd = m_CommandPool.begin(); iterParsedCmd != m_CommandPool.end(); ++iterParsedCmd )
	{
		debug_delete( *iterParsedCmd );
	}
	m_CommandPool.clear();

	// Delete any commands left in the queue...
	
	for( iterParsedCmd = m_CommandQueue.begin(); iterParsedCmd != m_CommandQueue.end(); ++iterParsedCmd )
	{
		debug_delete( *iterParsedCmd );
	}
	m_CommandQueue.clear();

		
	// Delete all pending commands allocated in the pool...
	 
	TPendingCmdArray::iterator iterPendingCmd;
	for( iterPendingCmd = m_PendingCmdPool.begin(); iterPendingCmd != m_PendingCmdPool.end(); ++iterPendingCmd )
	{
		debug_delete( *iterPendingCmd );
	}
	m_PendingCmdPool.clear();

	for( iterPendingCmd = m_PendingCmds.begin(); iterPendingCmd != m_PendingCmds.end(); ++iterPendingCmd )
	{
		debug_delete( *iterPendingCmd );
	}
	m_PendingCmds.clear();


	// Delete any commands left in the script queue...

	TCmdScriptArray::iterator iterCmdScript;
	for( iterCmdScript = m_CmdScriptPool.begin(); iterCmdScript != m_CmdScriptPool.end(); ++iterCmdScript )
	{
		debug_delete( *iterCmdScript );
	}
	m_CmdScriptPool.clear();

	for( iterCmdScript = m_CmdScripts.begin(); iterCmdScript != m_CmdScripts.end(); ++iterCmdScript )
	{
		debug_delete( *iterCmdScript );
	}
	m_CmdScripts.clear();

	m_dwCommandAllocations = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Clear()
//
//	PURPOSE:	Move all commands and scripts currently queued up back into their respective
//				pools.  This effectively clears the queues so no more processing will be done
//				and allows the allocated commands and scripts to be pulled from the pool again.
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Clear( )
{
	// Move all commands in the queue back into the pool. Any currently processing command
	// has already been removed from the queue so no iterators will be invalidated.
	TParsedCmdArray::iterator iterParsedCmd = m_CommandQueue.begin( );
	while( iterParsedCmd != m_CommandQueue.end( ) )
	{
		m_CommandPool.push_back( *iterParsedCmd );
		iterParsedCmd = m_CommandQueue.erase( iterParsedCmd );
	}

	// The secondary queue may have commands if we are being cleared in the middle of a process.
	TParsedCmdArray::iterator iterParsedCmdSecondary = m_SecondaryQueue.begin( );
	while( iterParsedCmdSecondary != m_SecondaryQueue.end( ) )
	{
		m_CommandPool.push_back( *iterParsedCmdSecondary );
		iterParsedCmdSecondary = m_SecondaryQueue.erase( iterParsedCmd );
	}

	TPendingCmdArray::iterator iterPendingCmd = m_PendingCmds.begin( );
	while( iterPendingCmd != m_PendingCmds.end( ) )
	{
		m_PendingCmdPool.push_back( *iterPendingCmd );
		iterPendingCmd = m_PendingCmds.erase( iterPendingCmd );
	}

	TCmdScriptArray::iterator iterCmdScript = m_CmdScripts.begin( );
	while( iterCmdScript != m_CmdScripts.end( ) )
	{
		m_CmdScriptPool.push_back( *iterCmdScript );
		iterCmdScript = m_CmdScripts.erase( iterCmdScript );
	}

	// Clear all the variables.
	for( uint16 i = 0; i < m_nNumVars; ++i )
	{
		m_aVars[i].Clear( );
	}
	m_nNumVars = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Update()
//
//	PURPOSE:	Update the CommandMgr (process any pending commands)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::Update()
{
	UpdatePendingCommands();
	UpdateScripts();

	// Process all of the queued commands for this frame...
	// Do this last so all commands that got queued from the 
	// updates above will be processed.
	ProcessQueuedCommands();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::UpdateScripts()
//
//	PURPOSE:	Update each script in the array...
//
// ----------------------------------------------------------------------- //

void CCommandMgr::UpdateScripts()
{
	// No need to update if there are no scripts queued up...
	if( m_CmdScripts.empty() )
		return;

	CCmdScript		*pCmdScript = NULL;

	// Go through the scripts and update each one...

	TCmdScriptArray::iterator iter = m_CmdScripts.begin();
	while( iter != m_CmdScripts.end() )
	{
		// Update the script...
		pCmdScript = *iter;
		pCmdScript->Update();

		// The script is finished when all commands have been queued and 
		// it has no scripted object...

		if( pCmdScript->IsFinished() )
		{
			// Remove it from the queue and place it back in the pool...

			iter = m_CmdScripts.erase( iter );
			m_CmdScriptPool.push_back( pCmdScript );
		}
		else
		{
			++iter;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::UpdatePendingCommands()
//
//	PURPOSE:	Update each pending command in the array and add to the command queue if needed...
//
// ----------------------------------------------------------------------- //

void CCommandMgr::UpdatePendingCommands( )
{
	float fTimeDelta = SimulationTimer::Instance().GetTimerElapsedS();

	// Check the pending commands to see if any need to be queued for this update...

	CPendingCmd *pPendingCmd = NULL;

	TPendingCmdArray::iterator iter = m_PendingCmds.begin();
	while( iter != m_PendingCmds.end() )
	{
		pPendingCmd = *iter;

		pPendingCmd->m_fDelay -= fTimeDelta;
		if( pPendingCmd->m_fDelay <= 0.0f )
		{
			QueueCommand( pPendingCmd->m_sCmd.c_str(), pPendingCmd->m_pActiveSender, pPendingCmd->m_pActiveTarget );

			bool bRecycleCmd = false;

			// If this is a counted command, decrement the count...
			if( pPendingCmd->m_nNumTimes >= 0 )
			{
				--pPendingCmd->m_nNumTimes;
				if( pPendingCmd->m_nNumTimes <= 0 )
				{
					// Command is finished so it's time to recycle it...
					bRecycleCmd = true;
				}
			}

			// If this is a repeating command, reset the delay...
			if( pPendingCmd->m_fMaxDelay > 0.0f )
			{
				pPendingCmd->m_fDelay = GetRandom( pPendingCmd->m_fMinDelay, pPendingCmd->m_fMaxDelay );
			}
			else
			{
				// Command is finished so it's time to recycle it...
				bRecycleCmd = true;
			}

			if( bRecycleCmd )
			{
				// Remove it from the array and place it back in the pool...

				iter = m_PendingCmds.erase( iter );
				m_PendingCmdPool.push_back( pPendingCmd );
			}
			else
			{
				++iter;
			}
		}
		else
		{
			++iter;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessDelay()
//
//	PURPOSE:	Process the Delay command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessDelay( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_fDelay	= (float)atof( pParsedCmd->m_saArgs[0].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[1].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );
	
    if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessDelay() ERROR!" );
		DevPrint( "    Delayed command, %s, is invalid.", cmd.m_sCmd.c_str() );

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessDelayId()
//
//	PURPOSE:	Process the DelayId command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessDelayId( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_sId		= pParsedCmd->m_saArgs[0].c_str();
	cmd.m_fDelay	= (float)atof( pParsedCmd->m_saArgs[1].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[2].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );

	if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessDelayId() ERROR!" );
		DevPrint( "    Delayed Id command, %s, is invalid.", cmd.m_sCmd.c_str() );

		return false;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ParseMsg_Target()
//
//	PURPOSE:	Break up a message target into class and/or result name
//				Note : pName is non-const because this function modifies the string
//
// ----------------------------------------------------------------------- //

static void ParseMsg_Target(char *pName, char **pResultClass, char **pResultName)
{
	// Look for <ClassName>
	char *pClassStart = strchr(pName, '<');
	char *pClassEnd = pClassStart ? strchr(pClassStart, '>') : 0;
	
	// Jump out if we don't actually seem to have a class name...
	if (!pClassStart || !pClassEnd)
	{
		*pResultClass = 0;
		*pResultName = pName;
		return;
	}

	// Cut up the string
	*pClassEnd = 0;

	// Return the two pieces
	*pResultClass = pClassStart + 1;
	*pResultName = pClassEnd + 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::SendMessageToObject()
//
//	PURPOSE:	Using the class description of the object let the target
//				target handle the message.
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::SendMessageToObject( ILTBaseClass *pSender, ILTBaseClass *pTarget, const char *pszMsg )
{
	if( !pTarget || !pszMsg || !pszMsg[0] )
		return false;
	
	if( !IsGameBase( pTarget->m_hObject ))
		return false;

	// Get the class of the target object so we can get the class description...
	HCLASS hTargetClass = g_pLTServer->GetObjectClass( pTarget->m_hObject );
	
	if( !hTargetClass )
	{
		DevPrint( "ERROR - SendMessageToObject: Could not find class of target object!" );
		return false;
	}

	char szTargetClassName[128];
	g_pLTServer->GetClassName( hTargetClass, szTargetClassName, ARRAY_LEN(szTargetClassName) );

	if( !szTargetClassName[0] )
	{
		DevPrint( "ERROR - SendMessageToObject: Invalid name for class of target object!" );
		return false;
	}

	CCmdMgr_ClassDesc *pTargetClassDesc = GetCmdmgrClassDescription( szTargetClassName );
	if( !pTargetClassDesc )
	{
		DevPrint( "ERROR - SendMessageToObject: No class description for target object!" );	
		return false;
	}

	// Parse the message into its name and arguments and let the object handle the 
	// message if it's valid...

	ConParse cpMsg( pszMsg );
	if( g_pCommonLT->Parse( &cpMsg ) != LT_OK )
	{
		DevPrint( "ERROR - SendMessageToObject: Could not parse message %s", pszMsg );
		return false;
	}
	
	// Ignore any empty messages...
	if( (cpMsg.m_nArgs == 0) || !cpMsg.m_Args[0] )
	{
		DevPrint( "ERROR - SendMessageToObject: Empty message encountered." );
		return false;
	}

	// Show debug information for the message...
	if( g_ShowMessagesTrack.GetFloat() != 0.0f )
	{
		char szTargetName[256] = {0};
		const char *pszTargetName = NULL;

		// Get the target's name...
		g_pLTServer->GetObjectName( pTarget->m_hObject, szTargetName, LTARRAYSIZE(szTargetName) );
		pszTargetName = szTargetName;
		
		char szSenderName[256] = {0};
		const char *pszSenderName = NULL;

		if( pSender )
		{
			// Get the sender's name...
			g_pLTServer->GetObjectName( pSender->m_hObject, szSenderName, LTARRAYSIZE(szSenderName) );
			pszSenderName = szSenderName;
		}
		else
		{
			pszSenderName = "Command Manager";
		}

		const char* pFilter   = g_ShowMessagesFilter.GetStr();

		// Filter out displaying any unwanted messages...

		bool bPrintMsg = (!pFilter || !pFilter[0]);
		if( !bPrintMsg )
		{
			bPrintMsg = (pszMsg ? LTSubStrIEquals( pFilter, pszMsg, LTStrLen(pszMsg) ) : true);
		}

		if (bPrintMsg)
		{
			g_pLTServer->CPrint(" ");
			g_pLTServer->CPrint("Message:    %s", pszMsg ? pszMsg : "NULL");
			g_pLTServer->CPrint("Sent from: '%s' to '%s'", pszSenderName, pszTargetName ? pszTargetName : "OBJECT NOT FOUND");
			g_pLTServer->CPrint(" ");
		}

	}

	// Set up a Parsed message...
	CParsedMsg cParsedMsg( cpMsg.m_nArgs, cpMsg.m_Args );

	// Look for the message name within the targets class description.  If the message is not
	// found, check the parent class messages...

	while( pTargetClassDesc )
	{
		// If a handler was called there is no need to check the parent...
		if( CallMessageHandler( pTargetClassDesc, pSender, pTarget, NULL, cParsedMsg ))
			break;

		pTargetClassDesc = GetCmdmgrClassDescription( pTargetClassDesc->m_cTok_ParentClass.c_str() );
	}

	// The message now meeds to be sent to all of the object's aggregates to handle...
	
	IAggregate *pAggregate = pTarget->m_pFirstAggregate;
	while( pAggregate )
	{
		pTargetClassDesc = GetCmdmgrClassDescription( pAggregate->GetType() );
		if( pTargetClassDesc )
		{
			while( pTargetClassDesc )
			{
				// If a handler was called there is no need to check the parent...
				if( CallMessageHandler( pTargetClassDesc, pSender, pTarget, pAggregate, cParsedMsg ))
					break;

				pTargetClassDesc = GetCmdmgrClassDescription( pTargetClassDesc->m_cTok_ParentClass.c_str() );
			}
		}
		else
		{
			DevPrint( "ERROR - No class description for aggregate %s!", pAggregate->GetType() );
		}

		// Even if this aggregate handled the message all other aggregates need to check, so keep going...
		pAggregate = pAggregate->m_pNextAggregate;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CallMessageHandler()
//
//	PURPOSE:	Find the messages handler and call it.  Returns true if the handler
//				was succesfully found and called, false otherwise...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::CallMessageHandler( const CCmdMgr_ClassDesc *pClassDesc, ILTBaseClass *pSender,
									 ILTBaseClass *pTargetObj, IAggregate *pTargetAgg, const CParsedMsg &cParsedMsg )
{
	if( !pClassDesc || !pTargetObj )
		return false;

	// Check the class to see if it has an overriding handler...
	if( pClassDesc->m_pHandleFn )
	{
		pClassDesc->m_pHandleFn( (pSender ? pSender->m_hObject : NULL), pTargetObj, pTargetAgg, cParsedMsg );
	}

	CCmdMgr_MsgDesc *pTargetMsgs = pClassDesc->m_pMsgs;
	if( !pTargetMsgs )
		return false;

	GameBase *pGameObj = dynamic_cast<GameBase*>(pTargetObj);
	if( !pGameObj )
		return false;
		
	CParsedMsg::CToken cTok_MsgName( cParsedMsg.GetArg(0) );
	int32 nArgCount = cParsedMsg.GetArgCount();

	bool bWasScripted = false;

	for( uint32 nMsg = CMDMGR_MIN_CLASSMSGS; nMsg <= pClassDesc->m_nNumMsgs; ++nMsg )
	{
		if( cTok_MsgName == pTargetMsgs[nMsg].m_cTok_MsgName )
		{
			// Make sure the message is valid and then let the object handle it...

			// TODO: Send to a validate function?
			if( (pTargetMsgs[nMsg].m_nMinArgs < 0) ||
				((nArgCount >= pTargetMsgs[nMsg].m_nMinArgs) &&
				(nArgCount <= pTargetMsgs[nMsg].m_nMaxArgs)) )
			{
				if( !pTargetMsgs[nMsg].m_pHandleFn )
				{
					DevPrint( "ERROR - Msg %s in class %s does not have a handler function!", cTok_MsgName.c_str(), pClassDesc->m_cTok_ClassName.c_str() );
					return false;
				}

				// If the object is currently being scripted then we must interupt the script...
				if( pGameObj->IsScripted() )
				{
					bWasScripted = true;
					pGameObj->InterruptScript();
				}
				
				if( bWasScripted )
				{
					// The object was scripted so we need to kill every 
					// script that the object was being scripted by...

					CCmdScript *pCmdScript = NULL;
					TCmdScriptArray::iterator iter;
					for( iter = m_CmdScripts.begin(); iter != m_CmdScripts.end(); ++iter )
					{
						pCmdScript = *iter;
						if( pCmdScript->m_pScriptedObj == pTargetObj )
						{
							pCmdScript->Kill();
						}
					}
				}

				// When the message is a blocking message, the objet is about to begin scripting...
				if( pTargetMsgs[nMsg].m_dwFlags & CMDMGR_MF_BLOCKINGMSG )
				{
					pGameObj->SetScripted();
				}

				pTargetMsgs[nMsg].m_pHandleFn( (pSender ? pSender->m_hObject : NULL), pTargetObj, pTargetAgg, cParsedMsg );

				// The message was found in the class so stop looking...
				return true;
			}

			DevPrint( "ERROR - Invalid number of arguments for message %s", cTok_MsgName.c_str() );
			return false;
		}
	}

	// No error, the message just wasn't in this class description... 
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessMsg()
//
//	PURPOSE:	Process the Msg or VMsg command (send the message to the
//				specified object(s))
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessMsg( const CParsedCmd *pParsedCmd, bool bObjVar)
{
	const char* pObjectNames	= pParsedCmd->m_saArgs[0].c_str();
	const char* pMsg			= pParsedCmd->m_saArgs[1].c_str();

    if( !pObjectNames || !pMsg )
		return false;

	// If the sender is a player, then store it
	// as the ActivePlayer.  Triggers can now be sent to an object called "ActivePlayer"
	// and it will go to this object.  Triggers sent to "OtherPlayers" will go
	// to all players except the "ActivePlayer".
	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( m_pActiveSender );
	if( pPlayerObj )
	{
		g_pGameServerShell->SetActivePlayer( pPlayerObj->m_hObject );
	}

	char szTargetName[256] = {0};

	ConParse cpObjects;
	cpObjects.Init( pObjectNames );

    if (g_pCommonLT->Parse( &cpObjects ) == LT_OK)
	{
		for (int i=0; i < cpObjects.m_nArgs; i++)
		{
			// For error reporting purposes, was there something wrong with the object, or the variable?
			const char * const k_pProblemTarget_Object = "object";
			const char * const k_pProblemTarget_Variable = "variable";
			const char *pProblemTarget = k_pProblemTarget_Object;
			// Did we succeed?
			bool bResult = false;

			char *pTargetClass = NULL;
			const char *pTargetName = NULL;
			ParseMsg_Target(cpObjects.m_Args[i], &pTargetClass, (char**)&pTargetName);

			// Check for an object variable with this name, if this is an object variable message command
			if (bObjVar)
			{
				VAR_STRUCT *pVar = (bObjVar ? GetVar( pTargetName, true ) : 0);
				if( (pVar) && (pVar->m_eType == eCMVar_Obj) )
				{
					// Make sure we're actually going to be talking to someone...
					if( !pVar->m_pObjVal )
					{
						DevPrint( "CCommandMgr::ProcessMsg() ERROR!" );
						DevPrint( "    <NULL> object variable %s.  Unable to send message.", cpObjects.m_Args[i] );
						continue;
					}

					// Send the message
					bResult = true;
					SendMessageToObject( m_pActiveSender, pVar->m_pObjVal, pMsg );
				}
				else
				{
					// Remember that there was a problem with the variable, not the object
					pProblemTarget = k_pProblemTarget_Variable;
				}
			}
			else
			{
				// Get the activesender of this message...
								
				if(( !pTargetName || !pTargetName[0] ) && m_pActiveTarget )
				{
					// If no target name given, use the activetarget object name.
					if( g_pLTServer->GetObjectName( m_pActiveTarget->m_hObject, szTargetName, ARRAY_LEN( szTargetName )) == LT_OK )
					{
						pTargetName = szTargetName;
					}
				}

				if( pTargetName && pTargetName[0] )
				{

					bool bSendToPlayer = false;
					ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

					FindMsgTargets( pTargetName, objArray, bSendToPlayer );

					// Send the message to all objects with the target name...

					uint32 nNumObjects = objArray.NumObjects();

					if( nNumObjects > 0 )
					{
						for( uint32 nObj = 0; nObj < nNumObjects; ++nObj )
						{
							ILTBaseClass *pTargetObj = g_pLTServer->HandleToObject( objArray.GetObject(nObj) );
							SendMessageToObject( m_pActiveSender, pTargetObj, pMsg );
						}
					}
										
					bResult = true;
				}
			}
			
			// Did we succeed?
			if( !bResult )
			{
				DevPrint( "CCommandMgr::ProcessMsg() ERROR!" );
				DevPrint( "    Could not find %s %s.", pProblemTarget, cpObjects.m_Args[i] );
			}
		}
	}
	else
	{
		DevPrint("CCommandMgr::ProcessMsg() ERROR!");
		DevPrint("    Could not parse object name(s) '%s'!", pObjectNames);
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRandWeight()
//
//	PURPOSE:	Process the Rand command (Pick one or the other of the
//				messages based on the percent)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessRandWeight( const CParsedCmd *pParsedCmd )
{
	float fPercent		= (float)atof( pParsedCmd->m_saArgs[0].c_str() );
	const char* pCmd1	= pParsedCmd->m_saArgs[1].c_str();
	const char* pCmd2	= pParsedCmd->m_saArgs[2].c_str();

	if( fPercent < 0.001f || fPercent > 1.0f || !pCmd1 || !pCmd2 )
		return false;

	if (GetRandom(0.0f, 1.0f) < fPercent)
	{
		if( !QueueCommand( pCmd1, pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget ))
		{
			DevPrint( "CCommandMgr::ProcessRandWeight() ERROR!" );
			DevPrint( "    Random command1, %s, is invalid.", pCmd1 );

			return false;
		}
		
		return true;
	}
	
	if( !QueueCommand( pCmd2, pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget ))
	{	
		DevPrint( "CCommandMgr::ProcessRandWeight() ERROR!" );
		DevPrint( "    Random command2, %s, is invalid.", pCmd2 );

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRand()
//
//	PURPOSE:	Process the Rand command (Pick one of the listed commands)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessRand( const CParsedCmd *pParsedCmd )
{
	uint32 nNumCmds	= pParsedCmd->m_saArgs.size();
	uint32 nCmd		= GetRandom( 0, nNumCmds - 1 );

	const char* pCmd	= pParsedCmd->m_saArgs[nCmd].c_str();
	if( !pCmd || !pCmd[0] )
		return false;

	if( !QueueCommand( pCmd, pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget ))
	{
		DevPrint( "CCommandMgr::ProcessRand() ERROR!" );
		DevPrint( "    Random command, %s, is invalid.", pCmd );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRepeat()
//
//	PURPOSE:	Process the Repeat command (Add the command)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessRepeat( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_nMinTimes	= (int32)atol( pParsedCmd->m_saArgs[0].c_str() );
	cmd.m_nMaxTimes	= (int32)atol( pParsedCmd->m_saArgs[1].c_str() );
	cmd.m_fMinDelay	= (float)atof( pParsedCmd->m_saArgs[2].c_str() );
	cmd.m_fMaxDelay	= (float)atof( pParsedCmd->m_saArgs[3].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[4].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );

	if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessRepeat() ERROR!" );
		DevPrint( "    Repeat command, %s, is invalid", cmd.m_sCmd.c_str() );

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRepeatId()
//
//	PURPOSE:	Process the RepeatId command (Add the command)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessRepeatId( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_sId		= pParsedCmd->m_saArgs[0].c_str();
	cmd.m_nMinTimes	= (int32)atol( pParsedCmd->m_saArgs[1].c_str() );
	cmd.m_nMaxTimes	= (int32)atol( pParsedCmd->m_saArgs[2].c_str() );
	cmd.m_fMinDelay	= (float)atof( pParsedCmd->m_saArgs[3].c_str() );
	cmd.m_fMaxDelay	= (float)atof( pParsedCmd->m_saArgs[4].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[5].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );

	if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessRepeatId() ERROR!" );
		DevPrint( "    Repeat Id command, %s, is invalid.", cmd.m_sCmd.c_str() );

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessLoop()
//
//	PURPOSE:	Process the Loop command (Add the command)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessLoop( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_fMinDelay	= (float)atof( pParsedCmd->m_saArgs[0].c_str() );
	cmd.m_fMaxDelay	= (float)atof( pParsedCmd->m_saArgs[1].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[2].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );

	if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessLoop() ERROR!" );
		DevPrint( "    Loop command, %s, is invalid.", cmd.m_sCmd.c_str() );

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessLoopId()
//
//	PURPOSE:	Process the LoopId command (Add the command)
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessLoopId( const CParsedCmd *pParsedCmd )
{
	CPendingCmd cmd;
	cmd.m_sId		= pParsedCmd->m_saArgs[0].c_str();
	cmd.m_fMinDelay	= (float)atof( pParsedCmd->m_saArgs[1].c_str() );
	cmd.m_fMaxDelay	= (float)atof( pParsedCmd->m_saArgs[2].c_str() );
	cmd.m_sCmd		= pParsedCmd->m_saArgs[3].c_str();

	cmd.SetActiveSender( pParsedCmd->m_pActiveSender );
	cmd.SetActiveTarget( pParsedCmd->m_pActiveTarget );

	if( !AddPendingCommand( cmd ))
	{
		DevPrint( "CCommandMgr::ProcessLoopId() ERROR!" );
		DevPrint( "    Loop Id command, %s, is invalid.", cmd.m_sCmd.c_str() );

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessAbort()
//
//	PURPOSE:	Process the Abort command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessAbort( const CParsedCmd *pParsedCmd )
{
	const char* pId = pParsedCmd->m_saArgs[0].c_str();

	if (!pId || pId[0] == CMDMGR_NULL_CHAR)
	{
		DevPrint("CCommandMgr::ProcessAbort() ERROR!");
		DevPrint("    Invalid Command Id!");
		return false;
	}


	// Remove the specified command...

	TPendingCmdArray::iterator iter;
	for( iter = m_PendingCmds.begin(); iter != m_PendingCmds.end(); ++iter )
	{
		if( !(*iter)->m_sId.empty() )
		{
			if( LTStrIEquals( (*iter)->m_sId.c_str(), pId ))
			{
				m_PendingCmds.erase( iter );
				return true;
			}
		}
	}

	DevPrint("CCommandMgr::ProcessAbort() WARNING!");
	DevPrint("    Could not find command associated with id '%s'!", pId);
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessInteger
//
//  PURPOSE:	Process the Integer command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessInt( const CParsedCmd *pParsedCmd )
{
	if( m_nNumVars >= CMDMGR_MAX_VARS )
	{
		DevPrint( "CCommandMgr::ProcessInteger() ERROR!" );
		DevPrint( "    Max number of variables reached!" );

		return false;
	}

	// Make sure we don't try to declare a variable we already have...

	for( int i = 0; i < m_nNumVars; ++i )
	{
		if( LTStrIEquals( m_aVars[i].m_sName.c_str(), pParsedCmd->m_saArgs[0].c_str() ))
		{
			DevPrint( "CCommandMgr::ProcessInteger() WARNING!" );
			DevPrint( "    Variable, %s, already defined!", m_aVars[i].m_sName.c_str() );

			return false;
		}
	}

	m_aVars[m_nNumVars].m_sName		= pParsedCmd->m_saArgs[0].c_str();
	m_aVars[m_nNumVars].m_eType		= eCMVar_Int;
	m_aVars[m_nNumVars].m_iVal		= atoi(pParsedCmd->m_saArgs[1].c_str());
	m_aVars[m_nNumVars].m_bGlobal	= pParsedCmd->m_bGlobal;

	++m_nNumVars;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessObj
//
//  PURPOSE:	Process the Obj command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessObj( const CParsedCmd *pParsedCmd )
{
	if( m_nNumVars >= CMDMGR_MAX_VARS )
	{
		DevPrint( "CCommandMgr::ProcessObj() ERROR!" );
		DevPrint( "    Max number of variables reached!" );

		return false;
	}

	// Make sure we don't try to declare a variable we already have...

	for( int i = 0; i < m_nNumVars; ++i )
	{
		if( LTStrIEquals( m_aVars[i].m_sName.c_str(), pParsedCmd->m_saArgs[0].c_str() ))
		{
			DevPrint( "CCommandMgr::ProcessObj() WARNING!" );
			DevPrint( "    Variable '%s' already defined!", m_aVars[i].m_sName.c_str() );

			return false;
		}
	}

	// Find the object

	ILTBaseClass *pTargetObj = 0;
	if( FindNamedObject( pParsedCmd->m_saArgs[1].c_str(), pTargetObj, true ) != LT_OK )
	{
		DevPrint( "CCommandMgr::ProcessObj() WARNING!" );
		DevPrint( "    Object '%s' not found!", pParsedCmd->m_saArgs[1].c_str() );
	}

	// Set up the new variable

	VAR_STRUCT &sNewVar = m_aVars[m_nNumVars];

	sNewVar.m_sName	= pParsedCmd->m_saArgs[0].c_str();
	sNewVar.m_eType	= eCMVar_Obj;
	sNewVar.SetObjVal(pTargetObj);

	// We now have a new variable.

	++m_nNumVars;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessSet
//
//  PURPOSE:	Process the Set command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessSet( const CParsedCmd *pParsedCmd )
{
	const char *pVarName	= pParsedCmd->m_saArgs[0].c_str();
	const char *pVarValue	= pParsedCmd->m_saArgs[1].c_str();

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessSet() ERROR!");
		DevPrint("    Invalid Variable Name!");
        return false;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return false;

	// Set the new value

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		pVar->m_iVal = atoi( pVarValue );
	}
	else
	{
		VAR_STRUCT *pSrcVar = GetVar( pVarValue, true );

		const char * const k_pInvalidDesc_UnknownVar = "    SET - Unknown variable '%s'!";
		const char * const k_pInvalidDesc_UnknownObj = "    SET - Unknown object '%s'!";
		const char * const k_pInvalidDesc_TypeMismatch = "    SET - Type mismatch with variable '%s'!";

		const char *pInvalidDesc = 0;

		if( pVar->m_eType == eCMVar_Obj )
		{
			if( pSrcVar )
			{
				if( pSrcVar->m_eType != eCMVar_Obj )
				{
					pInvalidDesc = k_pInvalidDesc_TypeMismatch;
				}
				else
				{
					pVar->SetObjVal(pSrcVar->m_pObjVal);
				}
			}
			else
			{
				ILTBaseClass *pTargetObject = 0;
				if( FindNamedObject(pVarValue, pTargetObject, true) != LT_OK )
				{
					pInvalidDesc = k_pInvalidDesc_UnknownObj;
				}
				else
				{
					pVar->SetObjVal(pTargetObject);
				}
			}
		}
		else if( !pSrcVar )
		{
			pInvalidDesc = k_pInvalidDesc_UnknownVar;
		}
		else
		{
			if( pSrcVar->m_eType != eCMVar_Int )
			{
				DevPrint("CommandMgr::ProcessSet() ERROR!");
				DevPrint("    Type mismatch %s/%s", pVarName, pVarValue);
				return false;
			}

			pVar->m_iVal = pSrcVar->m_iVal;
		}

		if( pInvalidDesc )
		{
			DevPrint( "CCommandMgr::ProcessSet() ERROR!" );
			DevPrint( const_cast<char*>(pInvalidDesc), pVarValue );
			return false;
		}
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessAdd
//
//  PURPOSE:	Process the Add command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessAdd( const CParsedCmd *pParsedCmd )
{
	const char *pVarName	= pParsedCmd->m_saArgs[0].c_str();
	const char *pVarValue	= pParsedCmd->m_saArgs[1].c_str();

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessAdd() ERROR!");
		DevPrint("    Invalid Variable Name!");
		return false;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return false;

	if( pVar->m_eType != eCMVar_Int )
	{
		DevPrint("CommandMgr::ProcessAdd() ERROR!");
		DevPrint("    Invalid variable type (%s) for add!", pVarName);
		return false;
	}

	// Set the new value

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		pVar->m_iVal += atoi( pVarValue );
	}
	else
	{
		// Find the var that has the value we want...

		VAR_STRUCT *pVar2 = GetVar( pVarValue, false );
		if( !pVar2 ) return false;

		if( pVar2->m_eType != eCMVar_Int )
		{
			DevPrint("CommandMgr::ProcessAdd() ERROR!");
			DevPrint("    Invalid variable type (%s) for add!", pVarName);
			return false;
		}

		pVar->m_iVal += pVar2->m_iVal;
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );
	
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessSub
//
//  PURPOSE:	Process the Subtract command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessSub( const CParsedCmd *pParsedCmd )
{
	const char *pVarName	= pParsedCmd->m_saArgs[0].c_str();
	const char *pVarValue	= pParsedCmd->m_saArgs[1].c_str();

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessSub() ERROR!");
		DevPrint("    Invalid Variable Name!");
		return false;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return false;

	if( pVar->m_eType != eCMVar_Int )
	{
		DevPrint("CommandMgr::ProcessSub() ERROR!");
		DevPrint("    Invalid variable type (%s) for sub!", pVarName);
		return false;
	}

	// Set the new value

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		pVar->m_iVal -= atoi( pVarValue );
	}
	else
	{
		// Find the var that has the value we want...

		VAR_STRUCT *pVar2 = GetVar( pVarValue, false );
		if( !pVar2 ) return false;

		if( pVar2->m_eType != eCMVar_Int )
		{
			DevPrint("CommandMgr::ProcessSub() ERROR!");
			DevPrint("    Invalid variable type (%s) for sub!", pVarName);
			return false;
		}

		pVar->m_iVal -= pVar2->m_iVal;
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessIf
//
//  PURPOSE:	Process the If command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessIf( const CParsedCmd *pParsedCmd )
{
	// Skip the 2nd argument since it should just be 'THEN'...
	const char *pExpression	= pParsedCmd->m_saArgs[0].c_str();
	const char *pCmds	= pParsedCmd->m_saArgs[2].c_str();

	if( !pExpression || !pCmds )
	{
		DevPrint( "CCommandMgr::ProcessIf() ERROR!" );
		DevPrint( "    Bad arguments!" );
		return false;
	}

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( g_pCommonLT->Parse( &cpExpression ) == LT_OK )
	{
		eExpressionVal kRet = CheckExpression( cpExpression );
		if( kRet == kExpress_FALSE )
		{
			// If statement failed to meet conditions...
			return true;
		}
		else if( kRet == kExpress_ERROR )
		{	
			DevPrint( "CCommandMgr::ProcessIf() ERROR!" );
			DevPrint( "    CheckExpression had an error!" );
			return false;
		}	

	}

	// Expressions evaluated to TRUE, process commands...

	if( QueueCommand( pCmds, pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget ))
	{
		// There was only a single command so were done...
		return true;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			QueueCommand( cpCommands.m_Args[i], pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessOn
//
//  PURPOSE:	Process the On command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessWhen( const CParsedCmd *pParsedCmd )
{
	// Skip the 2nd argument since it should just be 'THEN'...
	const char *pExpression	= pParsedCmd->m_saArgs[0].c_str();
	const char *pCmds		= pParsedCmd->m_saArgs[2].c_str();

	if( !pExpression || !pCmds )
	{
		DevPrint( "CCommandMgr::ProcessOn() ERROR!" );
		DevPrint( "    Bad arguments!" );
		return false;
	}

	// First check to see if the conditions have been met...

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( g_pCommonLT->Parse( &cpExpression ) == LT_OK )
	{
		eExpressionVal kRet = CheckExpression( cpExpression );
		if( kRet == kExpress_FALSE )
		{
			// If statement failed to meet conditions setup
			// a CMD_EVENT_STRUCT to check the condition later...
			
			for( int i = 0; i < CMDMGR_MAX_EVENT_COMMANDS; ++i )
			{
				if( m_aEventCmds[i].m_sExpression.empty() )
				{
					// We have an open slot, add it here..

					m_aEventCmds[i].m_sExpression	= pExpression;
					m_aEventCmds[i].m_sCmds			= pCmds;

					if( kExpress_ERROR == m_aEventCmds[i].FillVarArray( cpExpression ))
						return false;

					return true;
				}
			}

			// Couldn't find an empty slot...

			DevPrint( "CCommandMgr::ProcessWhen() ERROR!" );
			DevPrint( "    Max amount of event commands reached!" );

			return false;
			
		}
		else if( kRet == kExpress_ERROR )
		{	
			DevPrint( "CCommandMgr::ProcessWhen() ERROR!" );
			DevPrint( "    CheckExpression had an error!" );
			return false;
		}	

	}

	// Expressions evaluated to TRUE, process commands...

	if( QueueCommand( pCmds, pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget ))
	{
		// There was only a single command so were done...
		return true;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			QueueCommand( cpCommands.m_Args[i], pParsedCmd->m_pActiveSender, pParsedCmd->m_pActiveTarget );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::VarChanged
//
//  PURPOSE:	Handle a variabel changing... 
//
// ----------------------------------------------------------------------- //

void CCommandMgr::VarChanged( VAR_STRUCT *pVar )
{
	if( !pVar )
		return;
	
	ASSERT( pVar->m_nRefCount >= 0 );

	// Display the new value if the var was told to SHOW...

	if( pVar->m_bShow )
	{
		if( pVar->m_eType == eCMVar_Int )
		{
			g_pLTServer->CPrint( "%s: %i", pVar->m_sName.c_str(), pVar->m_iVal );
		}
		else if( pVar->m_eType == eCMVar_Obj )
		{
			const char *pObjName = "<NULL>";
			if( pVar->m_hObjVal )
				pObjName = GetObjectName( pVar->m_hObjVal );
			
			g_pLTServer->CPrint( "%s: %s", pVar->m_sName.c_str(), pObjName );
		}
	}

	if( pVar->m_nRefCount <= 0 ) return;

	for( int i = 0; i < CMDMGR_MAX_EVENT_COMMANDS; ++i )
	{
		if( m_aEventCmds[i].m_nNumVarsInCmd > 0 )
		{
			// See if the var is in the events var list

			for( int j = 0; j < m_aEventCmds[i].m_nNumVarsInCmd; ++j )
			{
				if( m_aEventCmds[i].m_aVars[j] == pVar )
				{
					// Check the conditions...

					if( m_aEventCmds[i].Process() )
					{
						// Ok the event happened, clear the slot
						m_aEventCmds[i].Clear();
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessShowVar
//
//  PURPOSE:	Display the value of this var in the console.
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessShowVar( const CParsedCmd *pParsedCmd )
{
	const char *pVarName	= pParsedCmd->m_saArgs[0].c_str();
	const char *pVarValue	= pParsedCmd->m_saArgs[1].c_str();

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessShowVar() ERROR!");
		DevPrint("    Invalid Variable Name!");
		return false;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return false;

	pVar->m_bShow = !!atoi( pVarValue );

	// Display the value if the var was told to SHOW...

	if( pVar->m_bShow )
	{
		if( pVar->m_eType == eCMVar_Int )
		{
			g_pLTServer->CPrint( "%s: %i", pVar->m_sName.c_str(), pVar->m_iVal );
		}
		else if( pVar->m_eType == eCMVar_Obj )
		{
			const char *pObjName = "<NULL>";
			if( pVar->m_hObjVal )
				pObjName = GetObjectName( pVar->m_hObjVal );
			
			g_pLTServer->CPrint( "%s: %s", pVar->m_sName.c_str(), pObjName );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessScript()
//
//	PURPOSE:	Process a SCRIPT command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessScript( const CParsedCmd *pParsedCmd )
{
	// See if a new script needs to be allocated...
	if( m_CmdScriptPool.empty() )
	{
		CCmdScript *pNewScript = debug_new( CCmdScript );
		if( pNewScript )
		{
			m_CmdScriptPool.push_back( pNewScript );
		}
	}

	LTASSERT( !m_CmdScriptPool.empty(), "CCommandMgr::ProcessScript() - Failed to allocate new CCmdScript on to pool!" );
	if( m_CmdScriptPool.empty() )
		return false;

	// Grab a free command from the pool...
	CCmdScript *pCmdScript = m_CmdScriptPool.back();
	if( !pCmdScript )
	{
		LTERROR( "CCommandMgr::ProcessScript() - Failed to create new CCmdScript." );
		return false;
	}

	pCmdScript->SetActiveSender( pParsedCmd->m_pActiveSender );
	pCmdScript->SetActiveTarget( pParsedCmd->m_pActiveTarget );
	pCmdScript->SetScriptedObject( NULL );

	// This is big, every command in the script needs to be coppied over...
	pCmdScript->m_saCmds.reserve( pParsedCmd->m_saArgs.size() );
	pCmdScript->m_saCmds.assign( pParsedCmd->m_saArgs.begin(), pParsedCmd->m_saArgs.end() );

	// Give the script an update...
	pCmdScript->Update();

	// Don't move the script to the list to update if it has already finished...
	if( !pCmdScript->IsFinished() )
	{
		// Add it to the script array...
		m_CmdScripts.push_back( pCmdScript );	

		// Remove it form the pool...
		m_CmdScriptPool.pop_back();
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::AddPendingCommand()
//
//	PURPOSE:	Add a pending command
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::AddPendingCommand( const CPendingCmd &crPendingCmd )
{
	if( (crPendingCmd.m_fDelay < 0.0f && crPendingCmd.m_fMaxDelay < 0.0f) || crPendingCmd.m_sCmd.empty() )
	{
		DevPrint("CCommandMgr::AddPendingCommand() ERROR!");
		DevPrint("Invalid arguments:");
		DevPrint("  Delay %.2f", crPendingCmd.m_fDelay);
		DevPrint("  MinTimes %d", crPendingCmd.m_nMinTimes);
		DevPrint("  MaxTimes %d", crPendingCmd.m_nMaxTimes);
		DevPrint("  MinDelay %.2f", crPendingCmd.m_fMinDelay);
		DevPrint("  MaxDelay %.2f", crPendingCmd.m_fMaxDelay);
		DevPrint("  Command %s", (!crPendingCmd.m_sCmd.empty() ? crPendingCmd.m_sCmd.c_str() : "NULL") );
		DevPrint("  Id %s", (!crPendingCmd.m_sId.empty() ? crPendingCmd.m_sId.c_str() : "NULL") );
		return false;
	}

	// See if a new command needs to be allocated...
	if( m_PendingCmdPool.empty() )
	{
		CPendingCmd *pNewCmd = debug_new( CPendingCmd );
		if( pNewCmd )
		{
			m_PendingCmdPool.push_back( pNewCmd );
		}
	}

	LTASSERT( !m_PendingCmdPool.empty(), "CCommandMgr::AddPendingCommand() - Failed to allocate new CPendingCmd on to pool!" );
	if( m_PendingCmdPool.empty() )
		return false;

	// Grab a free command from the pool...
	CPendingCmd *pPendingCmd = m_PendingCmdPool.back();
	if( !pPendingCmd )
	{
		LTERROR( "CCommandMgr::AddPendingCommand() - Failed to create new CPendingCmd." );
		return false;
	}

	pPendingCmd->m_nMinTimes	= crPendingCmd.m_nMinTimes;
	pPendingCmd->m_nMaxTimes	= crPendingCmd.m_nMaxTimes;

	// Set times if appropriate...
	if( pPendingCmd->m_nMaxTimes > 0 )
	{
		pPendingCmd->m_nNumTimes = GetRandom( pPendingCmd->m_nMinTimes, pPendingCmd->m_nMaxTimes );
	}
	else
	{
		pPendingCmd->m_nNumTimes = crPendingCmd.m_nNumTimes;
	}

	
	pPendingCmd->m_fMinDelay	= crPendingCmd.m_fMinDelay;
	pPendingCmd->m_fMaxDelay	= crPendingCmd.m_fMaxDelay;

	// Set delay...
	if( pPendingCmd->m_fMaxDelay > 0.0f )
	{
		pPendingCmd->m_fDelay = GetRandom( pPendingCmd->m_fMinDelay, pPendingCmd->m_fMaxDelay );
	}
	else
	{
		pPendingCmd->m_fDelay = crPendingCmd.m_fDelay;
	}

	// Remember the active target and sender

	pPendingCmd->SetActiveTarget( crPendingCmd.m_pActiveTarget );
	pPendingCmd->SetActiveSender( crPendingCmd.m_pActiveSender );


	// The id isn't used with every command, so it may not be valid...
	if( !crPendingCmd.m_sId.empty() )
	{
		pPendingCmd->m_sId = crPendingCmd.m_sId;
	}
	else
	{
		pPendingCmd->m_sId.clear();
	}

	// Set the command...
	pPendingCmd->m_sCmd = crPendingCmd.m_sCmd;

	// Add it to the pending command array...
	m_PendingCmds.push_back( pPendingCmd );	

	// Remove it form the pool...
	m_PendingCmdPool.pop_back();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::QueueCommand()
//
//	PURPOSE:	Queue a command for later processing...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::QueueCommand( const char *pszCommand, ILTBaseClass *pSender, ILTBaseClass *pTarget )
{
	if( !pszCommand || !pszCommand[0] )
		return false;

	// Break the command string into it's seperate commands and add a 
	// pre-parsed commnad to the queue for each valid command...

	CParsedCmd *pParsedCmd = NULL;
	ConParse cpCommand( pszCommand );
	
	while( g_pCommonLT->Parse( &cpCommand ) == LT_OK )
	{
		if( cpCommand.m_nArgs > 0 )
		{
			// See if a new command needs to be allocated...
			if( m_CommandPool.empty() )
			{
				CParsedCmd *pNewCmd = debug_new( CParsedCmd );
				if( pNewCmd )
				{
					m_CommandPool.push_back( pNewCmd );
					++m_dwCommandAllocations;
				}
			}

			LTASSERT( !m_CommandPool.empty(), "Failed to allocate new CParsedCmd on to pool!" );
			if( m_CommandPool.empty() )
				continue;

			// Grab a free command from the pool...
			pParsedCmd = m_CommandPool.back();
			if( !pParsedCmd )
			{
				LTERROR( "CCommandMgr::QueueCommand() - Failed to create new CParsedCmd." );
				return false;
			}

			// Save the pre-parsed command name...
			pParsedCmd->SetCommandName( cpCommand.m_Args[0] );

			// The command arguments are the remaining ConParse arguments...
			uint32 nNumArgs = cpCommand.m_nArgs - 1;

			// Make sure the correct number of arguments are specified...
			pParsedCmd->m_saArgs.resize( nNumArgs );
			
			// Save the pre-parsed command arguments...
			for( uint32 nArg = 0; nArg < nNumArgs; ++nArg )
			{
				pParsedCmd->m_saArgs[nArg] = cpCommand.m_Args[nArg + 1];
			}

			// Make sure the command is valid before placing it on the queue...
			if( !ValidateParsedCmd( pParsedCmd, !GLOBAL ))
			{
				DevPrint( "Failed to validate command: %s", cpCommand.m_Args[0] );
				return false;
			}

			// The command is valid, finish initializing it and push it on a queue...

			pParsedCmd->SetActiveSender( pSender );
			pParsedCmd->SetActiveTarget( pTarget );

			if( m_bCommandQueueLocked )
			{
				// When the main command queue is locked add it to the secondary...
				m_SecondaryQueue.push_back( pParsedCmd );
			}
			else
			{
				// The queue is not locked so add it...
				m_CommandQueue.push_back( pParsedCmd );
			}

			// Remove it form the pool...
			m_CommandPool.pop_back();
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::QueueMessage()
//
//	PURPOSE:	Specifically queue a message command to the target...
//				pszMessage should be in the form <MESSAGE_NAME param1 param2 ... paramN>
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::QueueMessage( ILTBaseClass *pSender, ILTBaseClass *pTarget, const char *pszMessage )
{
	if( !pTarget || !pszMessage || !pszMessage[0] )
		return false;

	// The name of the target object is needed...
	char szTargetName[128] = {0};
	g_pLTServer->GetObjectName( pTarget->m_hObject, szTargetName, LTARRAYSIZE(szTargetName) );
	
	// Develop a message command to queue...
	char szCmd[512] = {0};
	LTSNPrintF( szCmd, LTARRAYSIZE(szCmd), "msg %s (%s)", szTargetName, pszMessage );
	if( !QueueCommand( szCmd, pSender, pTarget ))
	{
		DevPrint( "CCommandMgr::QueueMessage() ERROR!" );
		DevPrint( "    Message, %s, is invalid.", pszMessage );
		
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::AddGlobalCommand()
//
//	PURPOSE:	Immediately process the global command.
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::AddGlobalCommand( const char *pszCommand, ILTBaseClass *pSender, ILTBaseClass *pTarget )
{
	if( LTStrEmpty( pszCommand ))
		return false;

	// Break the command string into it's separate commands and process it...
	ConParse cpCommand( pszCommand );
	while( g_pCommonLT->Parse( &cpCommand ) == LT_OK )
	{
		if( cpCommand.m_nArgs > 0 )
		{
			CParsedCmd ParsedCmd;

			// Save the pre-parsed command name...
			ParsedCmd.SetCommandName( cpCommand.m_Args[0] );

			// The command arguments are the remaining ConParse arguments...
			uint32 nNumArgs = cpCommand.m_nArgs - 1;

			// Make sure the correct number of arguments are specified...
			ParsedCmd.m_saArgs.resize( nNumArgs );

			// Save the pre-parsed command arguments...
			for( uint32 nArg = 0; nArg < nNumArgs; ++nArg )
			{
				ParsedCmd.m_saArgs[nArg] = cpCommand.m_Args[nArg + 1];
			}

			// Make sure the command is valid before placing it on the queue...
			if( !ValidateParsedCmd( &ParsedCmd, GLOBAL ))
			{
				DevPrint( "Failed to validate command: %s", cpCommand.m_Args[0] );
				return false;
			}

			ParsedCmd.m_bGlobal = true;
			
			// The command is valid, finish initializing it and immediately process it...
			ParsedCmd.SetActiveSender( pSender );
			ParsedCmd.SetActiveTarget( pTarget );

			// Process the command...
			m_pActiveSender	= ParsedCmd.m_pActiveSender;
			m_pActiveTarget	= ParsedCmd.m_pActiveTarget;
			ProcessParsedCmd( &ParsedCmd );
			m_pActiveSender	= NULL;
			m_pActiveTarget	= NULL;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessQueuedCommands()
//
//	PURPOSE:	Process all the queued commands...
//
// ----------------------------------------------------------------------- //

void CCommandMgr::ProcessQueuedCommands( )
{
	// No need to process if there are no commands queued up...
	if( m_CommandQueue.empty( ) )
		return;

	// Go through the queue and process each command...

	CParsedCmd *pParsedCmd = NULL;
	TParsedCmdArray::iterator iter = m_CommandQueue.begin( );
	while( iter != m_CommandQueue.end( ) )
	{
		// Once the server is paused, all command processing should stop.
		if( g_pGameServerShell->IsPaused( ) )
			break;

		// Cache the parsed command pointer and remove it from the queue. It must be
		// removed from the queue before being processed to avoid invalidating the iterator
		// by adding additional commands to the queue.
		pParsedCmd = *iter;
		m_CommandQueue.erase( iter );

		// Lock the main queue so all newly created commands get added to the secondary...
		LockCommandQueue( );

		// Process the command...
		m_pActiveSender	= pParsedCmd->m_pActiveSender;
		m_pActiveTarget	= pParsedCmd->m_pActiveTarget;
		ProcessParsedCmd( pParsedCmd );
		m_pActiveSender	= NULL;
		m_pActiveTarget	= NULL;

		// Place it back in the pool after it has been processed. This is done after processing
		// to avoid overwriting the command the is currently being processed.
		m_CommandPool.push_back( pParsedCmd );

		// Move any secondary queue commands to the main queue and unlock the main queue...
		// NOTE: Do this after erasing the just processed command from the main queue so
		// the order will remain consistent and the iterator will not become invalid.
		UnlockCommandQueue( );

		// Always set the iterator to the head of the queue after unlocking the main queue...
		iter = m_CommandQueue.begin( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessParsedCmd()
//
//	PURPOSE:	Process a Pre-Parsed command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ProcessParsedCmd( CParsedCmd *pParsedCmd )
{
	if( !pParsedCmd || !pParsedCmd->m_cTok_CmdName.c_str() || pParsedCmd->m_saArgs.empty() )
		return false;

	// Find the command in the valid list and process it...

	for( uint8 nCmd = 0; nCmd < c_nNumValidCmds; ++nCmd )
	{
		if( pParsedCmd->m_cTok_CmdName == s_ValidCmds[nCmd].m_cTok_CmdName )
		{
			if( s_ValidCmds[nCmd].m_pProcessFn )
			{
				if( !s_ValidCmds[nCmd].m_pProcessFn( this, pParsedCmd ))
				{
					return false;
				}
				
				// Success...
				return true;
			}
			else
			{
				DevPrint("CCommandMgr::ProcessParsedCmd() ERROR!");
				DevPrint("pProcessFn is Invalid for command: %s", s_ValidCmds[nCmd].m_cTok_CmdName.c_str() );
				return false;
			}
		}
	}

	// Failure...
	DevPrint( "CCommandMgr::ProcessParsedCmd() - Command not valid: %s", pParsedCmd->m_cTok_CmdName.c_str() );
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ValidateParsedCmd()
//
//	PURPOSE:	Validate a single pre-parsed command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::ValidateParsedCmd( const CParsedCmd *pParsedCmd, bool bGlobal )
{
	if( !pParsedCmd->m_cTok_CmdName.c_str() || pParsedCmd->m_saArgs.empty() )
		return false;

// TODO: Do more validation than just checking the number of args?

	// Find the command in the valid list and verify it's number of arguments...
	for( uint8 nCmd = 0; nCmd < c_nNumValidCmds; ++nCmd )
	{
		if( pParsedCmd->m_cTok_CmdName == s_ValidCmds[nCmd].m_cTok_CmdName )
		{
			// Make sure the global context of the command is correct.
			if( bGlobal && !s_ValidCmds[nCmd].m_bGlobal )
			{
				DevPrint( "Command %s cannot be processed in the global namespace.", s_ValidCmds[nCmd].m_cTok_CmdName.c_str() );
				return false;
			}

			if( !CheckArgs( pParsedCmd, s_ValidCmds[nCmd].m_nMinArgs, s_ValidCmds[nCmd].m_nMaxArgs  ))
			{
				DevPrint( "Syntax for %s command is: %s", s_ValidCmds[nCmd].m_cTok_CmdName.c_str(), s_ValidCmds[nCmd].m_pszSyntax );
				return false;
			}

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CheckArgs()
//
//	PURPOSE:	Make sure the number of args match what is expected
//
// ----------------------------------------------------------------------- //

bool CCommandMgr::CheckArgs( const CParsedCmd *pParsedCmd, uint8 nMin, uint8 nMax )
{
	uint32 nArgs = pParsedCmd->m_saArgs.size();
	if( (nArgs < nMin) || (nArgs > nMax) )
	{
		DevPrint("CCommandMgr::CheckArgs() ERROR!");
		DevPrint("    %s command had %d arguments instead of the %d min and %d max!",
			pParsedCmd->m_cTok_CmdName.c_str(), nArgs, nMin, nMax );

		for( uint32 nArg = 0; nArg < pParsedCmd->m_saArgs.size(); ++nArg )
		{
			DevPrint("  Arg[%d] = '%s'", nArg, pParsedCmd->m_saArgs[nArg].c_str() );
		}

		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::UnlockCommandQueue()
//
//	PURPOSE:	Unlock the main command queue and copy any commands in the
//				secondary queue to the head of the main queue.
//
// ----------------------------------------------------------------------- //

void CCommandMgr::UnlockCommandQueue()
{
	m_bCommandQueueLocked = false;

	if( !m_SecondaryQueue.empty() )
	{
		// Add all the commands in the secondary queue to the head of the 
		// main queue so they will be processed next...
		uint32 nCommandSize = m_CommandQueue.size();
		uint32 nSecondarySize = m_SecondaryQueue.size();
		uint32 nNewSize = nCommandSize + nSecondarySize;

		//make room for our new list
		m_CommandQueue.resize( nNewSize );

		//slide our original list over (without stomping our own data, so move last items first)
		for(uint32 nMoveCmd = 0; nMoveCmd < nCommandSize; nMoveCmd++)
		{
			m_CommandQueue[nNewSize - nMoveCmd - 1] = m_CommandQueue[nCommandSize - nMoveCmd - 1];
		}

		//and insert our new commands
		for(uint32 nCopyCmd = 0; nCopyCmd < nSecondarySize; nCopyCmd++)
		{
			m_CommandQueue[nCopyCmd] = m_SecondaryQueue[nCopyCmd];
		}

		// Remove elements from the Secondary queue since we already coppied them into the main queue...
		m_SecondaryQueue.resize( 0 );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessListCommands()
//
//	PURPOSE:	List available commands
//
// --------------------------------------------------------------------------- //

bool CCommandMgr::ProcessListCommands( const CParsedCmd *pParsedCmd )
{
	for (int i=0; i < c_nNumValidCmds; i++)
	{
        g_pLTServer->CPrint(s_ValidCmds[i].m_pszSyntax);
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::DevPrint()
//
//	PURPOSE:	Print out info that should only be seend during development
//
// ----------------------------------------------------------------------- //

void CCommandMgr::DevPrint(char *msg, ...)
{
#define _DEV_BUILD
#ifdef _DEV_BUILD
	char pMsg[256];
	va_list marker;
	va_start(marker, msg);
	LTVSNPrintF(pMsg, LTARRAYSIZE(pMsg), msg, marker);
	va_end(marker);

    g_pLTServer->CPrint("%s", pMsg);
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Save()
//
//	PURPOSE:	Save the command mgr
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Save( ILTMessage_Write *pMsg )
{
	SAVE_DWORD( m_CommandQueue.size( ) );
	for( TParsedCmdArray::iterator iter = m_CommandQueue.begin( ); iter != m_CommandQueue.end( ); ++iter )
	{
		(*iter)->Save( pMsg );
	}

	SAVE_DWORD( m_PendingCmds.size() );
	for( TPendingCmdArray::iterator iter = m_PendingCmds.begin(); iter != m_PendingCmds.end(); ++iter )
	{
		(*iter)->Save( pMsg );
	}

	// Count the number of level specific vars...

	int nVars = 0;
	uint16 i;
	for( i = 0; i < m_nNumVars; ++i )
	{
		if( !m_aVars[i].m_bGlobal )
			++nVars;
	}

	SAVE_INT( nVars );

	// Save the level specific vars...

	for( i = 0; i < m_nNumVars; ++i )
	{
		if( !m_aVars[i].m_bGlobal )
			m_aVars[i].Save( pMsg, false );
	}

	//jrg - 9/8/02 - we need to save the names of global variables here
	// so that when loading, the Event commands can reference them
	// the values of the global variables need to be saved with the player, so that 
	// the will be saved/loaded on transitions
	SaveGlobalVars(pMsg,true);

	for( i = 0; i < CMDMGR_MAX_EVENT_COMMANDS; i++ )
	{
		m_aEventCmds[i].Save( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Load()
//
//	PURPOSE:	Load the command mgr
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Load( ILTMessage_Read *pMsg )
{
	uint32 nNumCommands = 0;
	LOAD_DWORD( nNumCommands );
	
	CParsedCmd *pParsedCmd = NULL;
	for( uint32 nCommand = 0; nCommand < nNumCommands; ++nCommand )
	{
		// See if a new command needs to be allocated...
		if( m_CommandPool.empty() )
		{
			CParsedCmd *pNewCmd = debug_new( CParsedCmd );
			if( pNewCmd )
			{
				m_CommandPool.push_back( pNewCmd );
				++m_dwCommandAllocations;
			}
		}

		LTASSERT( !m_CommandPool.empty(), "Failed to allocate new CParsedCmd on to pool!" );
		if( m_CommandPool.empty() )
			continue;

		// Grab a free command from the pool...
		pParsedCmd = m_CommandPool.back();
		if( !pParsedCmd )
		{
			LTERROR( "CCommandMgr::QueueCommand() - Failed to create new CParsedCmd." );
			return;
		}

		pParsedCmd->Load( pMsg );
		m_CommandQueue.push_back( pParsedCmd );

		// Remove it form the pool...
		m_CommandPool.pop_back();
	}

	uint32 dwNumPendingCmds = 0;
	LOAD_DWORD( dwNumPendingCmds );

	CPendingCmd cPendingCmd;
	for( uint32 nPendingCmd = 0; nPendingCmd < dwNumPendingCmds; ++nPendingCmd )
	{
		cPendingCmd.Load( pMsg );
		AddPendingCommand( cPendingCmd );
	}

	int nVars = 0;
	LOAD_INT( nVars );

	for( uint16 i = 0; i < nVars; ++i )
	{
		ASSERT( m_nNumVars < CMDMGR_MAX_VARS );

		m_aVars[m_nNumVars].Load( pMsg, false );
		++m_nNumVars;
	}

	//jrg - 9/8/02 - we need to load the names of global variables here
	// so that the Event commands can reference them
	// the values of the global variables need to be saved with the player, so that 
	// the will be saved/loaded on transitions
	LoadGlobalVars(pMsg,true);

	for( uint16 i = 0; i < CMDMGR_MAX_EVENT_COMMANDS; i++ )
	{
		m_aEventCmds[i].Load( pMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::SaveVars
//
//  PURPOSE:	Save only the vars
//
// ----------------------------------------------------------------------- //

void CCommandMgr::SaveGlobalVars( ILTMessage_Write *pMsg, bool bNameOnly )
{
	// Count the number of globals...

	int nGlobals = 0;
	int i;
	for( i = 0; i < m_nNumVars; ++i )
	{
		if( m_aVars[i].m_bGlobal )
			++nGlobals;
	}

	SAVE_INT( nGlobals );

	// Save the globals...

	for( i = 0; i < m_nNumVars; ++i )
	{
		if( m_aVars[i].m_bGlobal )
			m_aVars[i].Save( pMsg, bNameOnly );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::LoadVars
//
//  PURPOSE:	Load only the vars
//
// ----------------------------------------------------------------------- //

void CCommandMgr::LoadGlobalVars( ILTMessage_Read *pMsg, bool bNameOnly )
{
	int nGlobals = 0;
	LOAD_INT( nGlobals );

	VAR_STRUCT TempVar;
	VAR_STRUCT *pVar;
	
	// Load to a temp first and see if the var already exists...

	for( int i = 0; i < nGlobals; ++i )
	{
		ASSERT( m_nNumVars < CMDMGR_MAX_VARS );

		TempVar.Clear();
		TempVar.Load( pMsg, bNameOnly );

		uint16 nId;
		pVar = GetVar( TempVar.m_sName.c_str(), true, &nId );
		if( !pVar || (!bNameOnly && !pVar->m_bValid) )
		{
			// If we already have the var reassign the same one to avoid duplicates...
			
			if( pVar && (nId < CMDMGR_MAX_VARS) )
			{
				m_aVars[nId] = TempVar;
			}
			else
			{
				m_aVars[m_nNumVars] = TempVar;
				++m_nNumVars;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::GetVar
//
//  PURPOSE:	Find the requested variable
//
// ----------------------------------------------------------------------- //

VAR_STRUCT* CCommandMgr::GetVar( const char *pName, bool bSilent, uint16 *nId /*= NULL*/ )
{
	if( !pName ) return NULL;

	if( nId )
		*nId = (uint16)-1;

	for( int iVar = 0; iVar < m_nNumVars; ++iVar )
	{
		if( !LTStrICmp( m_aVars[iVar].m_sName.c_str(), pName ))
		{
			if( nId )
			{
				ASSERT( iVar == ( uint16 )iVar ); 
				*nId = ( uint16 )iVar;
			}

			return &m_aVars[iVar];
		}
	}

	if (!bSilent)
	{
		DevPrint( "CCommandMgr::GetVar() ERROR!" );
		DevPrint( "    Could not find requested variable '%s'!", pName );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::FindMsgTargets()
//
//	PURPOSE:	Utility function for filling an object list with objects
//				that will recieve a message... 
//
// ----------------------------------------------------------------------- //

void CCommandMgr::FindMsgTargets( const char *pszTargetName, BaseObjArray<HOBJECT> &objArray, bool &bSendToPlayer )
{
	// Nothing to do if the target name is empty.
	if( LTStrEmpty( pszTargetName ))
		return;

	// Set to true if any special target names are identified.
	bool bFoundSpecialTargets = false;

	// Initialize to not sending to player.
	bSendToPlayer = false;

	// Do special checks for MP team modes only.
	if( IsMultiplayerGameServer( ) && GameModeMgr::Instance().m_grbUseTeams )
	{
		// See if this is a message for team0 or team1.
		ETeamId eTeamId = INVALID_TEAM;
		// Check if sending message to team0.
		if( LTStrIEquals( pszTargetName, "Team0" ))
		{
			eTeamId = kTeamId0;
		}
		// Check if sending message to team1.
		else if( LTStrIEquals( pszTargetName, "Team1" ))
		{
			eTeamId = kTeamId1;
		}
		// Check if sending message to the team of the activeplayer.
		else if( LTStrIEquals( pszTargetName, "ActiveTeam" ))
		{
			HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );
			CPlayerObj* pActivePlayer = CPlayerObj::DynamicCast( hActivePlayer );
			if( pActivePlayer )
			{
				eTeamId = ( ETeamId )pActivePlayer->GetTeamID();
			}
		}
		// Check if sending message to the team other than that of the activeplayer.
		else if( LTStrIEquals( pszTargetName, "OtherTeam" ))
		{
			HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );
			CPlayerObj* pActivePlayer = CPlayerObj::DynamicCast( hActivePlayer );
			if( pActivePlayer )
			{
				if( pActivePlayer->GetTeamID() == kTeamId0 )
					eTeamId = kTeamId1;
				else if( pActivePlayer->GetTeamID() == kTeamId1 )
					eTeamId = kTeamId0;
			}
		}

		// See if we have a team to send the message to.
		if( eTeamId != INVALID_TEAM )
		{
			// Go through all the players on the team and add them to the list.
			for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( ); 
				iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
			{
				CPlayerObj* pPlayerObj = *iter;

				// Send the message if this is not the active player.
				if( pPlayerObj && pPlayerObj->GetTeamID() == eTeamId )
				{
					objArray.AddObject( pPlayerObj->m_hObject );
				}
			}

			bFoundSpecialTargets = true;
			bSendToPlayer = true;
		}
	}

	// No special targets found yet, see if it's activeplayer/otherplayers.
	if( !bFoundSpecialTargets )
	{
		if( LTStrIEquals( pszTargetName, "ActivePlayer" ))
		{
			// Only send the message to the ActivePlayer...

			HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );
			if( hActivePlayer )
			{
				objArray.AddObject( hActivePlayer );
			}

			bFoundSpecialTargets = true;
			bSendToPlayer = true;
		}
		else if( LTStrIEquals( pszTargetName, "OtherPlayers" ))
		{
			// Send the message to all players except the ActivePlayer...
			HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );
			CPlayerObj* pActivePlayer = CPlayerObj::DynamicCast( hActivePlayer );

			// Go through all the players and add them to the list except 
			// the active player.
			for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
				iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
			{
				CPlayerObj* pPlayerObj = *iter;

				// Send the message if this is not the active player.
				if( pPlayerObj && (pPlayerObj != pActivePlayer) )
				{
					objArray.AddObject( pPlayerObj->m_hObject );
				}
			}

			bFoundSpecialTargets = true;
			bSendToPlayer = true;
		}
	}

	// No special targets identified, must be a name of an object.
	if( !bFoundSpecialTargets )
	{
		// The name is not a special case so just find all objects with the target name...
		g_pLTServer->FindNamedObjects( pszTargetName, objArray );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::Load()
//
//	PURPOSE:	Load all the info associated with the CMD_STRUCT
//
// ----------------------------------------------------------------------- //

void CPendingCmd::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	LOAD_FLOAT(m_fDelay);
	LOAD_FLOAT(m_fMinDelay);
	LOAD_FLOAT(m_fMaxDelay);
	LOAD_INT(m_nNumTimes);
	LOAD_INT(m_nMinTimes);
	LOAD_INT(m_nMaxTimes);
	LOAD_COBJECT(m_pActiveTarget, ILTBaseClass);
	m_hActiveTarget = m_pActiveTarget ? m_pActiveTarget->m_hObject : NULL;
	LOAD_COBJECT(m_pActiveSender, ILTBaseClass);
	m_hActiveSender = m_pActiveSender ? m_pActiveSender->m_hObject : NULL;
	
	LOAD_STDSTRING( m_sCmd );
	LOAD_STDSTRING( m_sId );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::Save()
//
//	PURPOSE:	Save all the info associated with the CMD_STRUCT
//
// ----------------------------------------------------------------------- //

void CPendingCmd::Save(ILTMessage_Write *pMsg)
{
	if( !pMsg )
		return;

	SAVE_FLOAT(m_fDelay);
	SAVE_FLOAT(m_fMinDelay);
	SAVE_FLOAT(m_fMaxDelay);
	SAVE_INT(m_nNumTimes);
	SAVE_INT(m_nMinTimes);
	SAVE_INT(m_nMaxTimes);
	SAVE_COBJECT(m_pActiveTarget);
	SAVE_COBJECT(m_pActiveSender);
	SAVE_STDSTRING( m_sCmd );
	SAVE_STDSTRING( m_sId );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCmdScript::Update()
//
//	PURPOSE:	Update the script...
//
// ----------------------------------------------------------------------- //

void CCmdScript::Update()
{
	static CParsedMsg::CToken s_cTok_Msg( "MSG" );

	ConParse		cpCommand;
	GameBase		*pScriptObj		= NULL;

	// Don't update the script while the object is still being scripted...
	if( m_pScriptedObj )
	{
		pScriptObj = dynamic_cast<GameBase*>(m_pScriptedObj);
		if( pScriptObj && pScriptObj->IsScripted() )
			return;

		// Give the object a chance to cleanup...
		pScriptObj->ScriptCleanup();
				
		// Clear the scripted object since it is now finished scripting...
		SetScriptedObject( NULL );
	}

	// Nothing to update if we have no commands...
	if( m_saCmds.empty() )
		return;

	char szTargetName[256] = {0};

	// Check the commands, queue the commands until a blocking message is reached...
	StringArray::iterator CmdIter = m_saCmds.begin();
	while( CmdIter != m_saCmds.end() )
	{
		// Stop updating the script if there is a scripted object...
		if( m_pScriptedObj )
			break;

		cpCommand.Init( (*CmdIter).c_str() );
		if( g_pCommonLT->Parse( &cpCommand ) == LT_OK )
		{
			CParsedMsg::CToken cTok_CmdName( cpCommand.m_Args[0] );

			// Only messages can stall the script, just add all other commands to the queue...
			if( cTok_CmdName == s_cTok_Msg )
			{
				// Parse the message to see if its a blocking message and get the
				// target object if it is...

				ConParse cpObjects;
				cpObjects.Init( cpCommand.m_Args[1] );

				ConParse cpMsg;
				cpMsg.Init( cpCommand.m_Args[2] );

				if( (g_pCommonLT->Parse( &cpObjects ) == LT_OK) && (g_pCommonLT->Parse( &cpMsg ) == LT_OK ) )
				{
					CParsedMsg::CToken cTok_MsgName( cpMsg.m_Args[0] );

					char *pTargetClass = NULL;
					const char *pTargetName = NULL;
					ParseMsg_Target(cpObjects.m_Args[0], &pTargetClass, (char**)&pTargetName);

					// Get the activetarget of this message...
					if(( !pTargetName || !pTargetName[0] ) && m_pActiveTarget )
					{
						// If no target name given, use the activetarget object name.
						if( g_pLTServer->GetObjectName( m_pActiveTarget->m_hObject, szTargetName, ARRAY_LEN( szTargetName )) == LT_OK )
						{
							pTargetName = szTargetName;
						}
					}

					// Get the object that will recieve the message...
					bool bSendToPlayer = false;
					ObjArray <HOBJECT, 1> objArray;
					g_pCmdMgr->FindMsgTargets( pTargetName, objArray, bSendToPlayer );

					// Get the class of the recipient and the object...

					HCLASS hTargetClass = NULL;
					ILTBaseClass *pScriptObj = NULL;
					if( objArray.NumObjects() > 0 )
					{
						if( IsGameBase( objArray.GetObject(0) ))
						{
							// Get the target's class...
							hTargetClass = g_pLTServer->GetObjectClass( objArray.GetObject(0) );

							pScriptObj = g_pLTServer->HandleToObject( objArray.GetObject(0) );
						}
					}

					if( !pScriptObj )
					{
						// Remove the command from the script...
						CmdIter = m_saCmds.erase( CmdIter );					
						continue;
					}

					char szTargetClassName[128];
					g_pLTServer->GetClassName( hTargetClass, szTargetClassName, ARRAY_LEN(szTargetClassName) );

					CCmdMgr_ClassDesc *pTargetClassDesc = GetCmdmgrClassDescription( szTargetClassName );
					if( !pTargetClassDesc )
					{
						g_pCmdMgr->DevPrint( "CCmdScript::Update() ERROR!" );
						g_pCmdMgr->DevPrint( "    No class description for for class %s!", szTargetClassName );	
						
						// Remove the command from the script...
						CmdIter = m_saCmds.erase( CmdIter );
						continue;
					}

					// Look for the message name within the targets class description.  If the message is not
					// found, check the parent class messages...

					bool bMsgFound = false;

					while( pTargetClassDesc && !bMsgFound )
					{
						for( uint32 nMsg = CMDMGR_MIN_CLASSMSGS; nMsg <= pTargetClassDesc->m_nNumMsgs; ++nMsg )
						{
							if( cTok_MsgName == pTargetClassDesc->m_pMsgs[nMsg].m_cTok_MsgName )
							{
								// See if this is a blocking message...
								if( pTargetClassDesc->m_pMsgs[nMsg].m_dwFlags & CMDMGR_MF_BLOCKINGMSG )
								{
									// Save the target object as the scripted object...
									SetScriptedObject( pScriptObj );
								}

								// The message was found so stop looking...
								bMsgFound = true;
								break;
							}
						}

						pTargetClassDesc = GetCmdmgrClassDescription( pTargetClassDesc->m_cTok_ParentClass.c_str() );
					}

					// If the message was not found in the class or any of it's bases, check the aggragates...

					IAggregate *pAggregate = pScriptObj->m_pFirstAggregate;
					while( pAggregate && !bMsgFound )
					{
						pTargetClassDesc = GetCmdmgrClassDescription( pAggregate->GetType() );
						while( pTargetClassDesc && !bMsgFound )
						{
							for( uint32 nMsg = CMDMGR_MIN_CLASSMSGS; nMsg <= pTargetClassDesc->m_nNumMsgs; ++nMsg )
							{
								if( cTok_MsgName == pTargetClassDesc->m_pMsgs[nMsg].m_cTok_MsgName )
								{
									// See if this is a blocking message...
									if( pTargetClassDesc->m_pMsgs[nMsg].m_dwFlags & CMDMGR_MF_BLOCKINGMSG )
									{
										// Save the target object as the scripted object...
										SetScriptedObject( pScriptObj );
									}

									// The message was found so stop looking...
									bMsgFound = true;
									break;
								}
							}

							pTargetClassDesc = GetCmdmgrClassDescription( pTargetClassDesc->m_cTok_ParentClass.c_str() );
						}

						// Even if this aggregate handled the message all other aggregates need to check, so keep going...
						pAggregate = pAggregate->m_pNextAggregate;
					}
				}
			}

			// Queue this command for processing...
			if( !g_pCmdMgr->QueueCommand( (*CmdIter).c_str(), m_pActiveSender, m_pActiveTarget ))
			{
				g_pCmdMgr->DevPrint( "CCmdScript::Update() ERROR!" );
				g_pCmdMgr->DevPrint( "    Command, %s, is invalid.", (*CmdIter).c_str() );
			}
		}

		// Remove the command...
		CmdIter = m_saCmds.erase( CmdIter );
	}
}

void CCmdScript::Save( ILTMessage_Write *pMsg )
{

}

void CCmdScript::Load( ILTMessage_Read *pMsg )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VAR_STRUCT::Save
//
//  PURPOSE:	Save a variable
//
// ----------------------------------------------------------------------- //

void VAR_STRUCT::Save( ILTMessage_Write *pMsg, bool bNameOnly  )
{
	SAVE_STDSTRING( m_sName );

	if (!bNameOnly )
	{
		SAVE_DWORD( (uint32)m_eType );
		SAVE_INT( m_iVal );
		SAVE_COBJECT( m_pObjVal );
		SAVE_BOOL( m_bGlobal );
		SAVE_INT( m_nRefCount );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VAR_STRUCT::Load
//
//  PURPOSE:	Load a var
//
// ----------------------------------------------------------------------- //

void VAR_STRUCT::Load( ILTMessage_Read *pMsg, bool bNameOnly  )
{
	LOAD_STDSTRING( m_sName );

	if (!bNameOnly )
	{
		LOAD_DWORD_CAST( m_eType, ECmdMgrVarType );
		LOAD_INT( m_iVal );
		LOAD_COBJECT( m_pObjVal, ILTBaseClass );
		m_hObjVal = (m_pObjVal) ? m_pObjVal->m_hObject : NULL;
		LOAD_BOOL( m_bGlobal );
		LOAD_INT( m_nRefCount );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMD_EVENT_STRUCT::FillVarArray
//
//  PURPOSE:	Fill the list of vars from the expression
//
// ----------------------------------------------------------------------- //

eExpressionVal CMD_EVENT_STRUCT::FillVarArray( ConParse &cpExpression )
{
	if( cpExpression.m_nArgs != 3 )
		return kExpress_ERROR;

	char *pArg1 = cpExpression.m_Args[0];
	const char *pOp = cpExpression.m_Args[1];
	char *pArg2 = cpExpression.m_Args[2];

	if( !pArg1 || !pOp || !pArg2 || !g_pCmdMgr )
		return kExpress_ERROR;

	// Make sure the operator is valid...

	for( int iOp = 0; iOp < c_NumOperators; ++iOp )
	{
		if( !LTStrICmp( s_Operators[iOp].m_OpName, pOp ))
		{
			OPERATOR_STRUCT *pOperator = &s_Operators[iOp];

			if( pOperator->m_bLogical )
			{
				// For logical operators just pass the args back to this function...
				
				ConParse cpArg1;
				cpArg1.Init( pArg1 );

				if( g_pCommonLT->Parse( &cpArg1 ) == LT_OK )
				{
					if( FillVarArray( cpArg1 ) == kExpress_ERROR )
						return kExpress_ERROR;
				}

				ConParse cpArg2;
				cpArg2.Init( pArg2 );

				if( g_pCommonLT->Parse( &cpArg2 ) == LT_OK )
				{
					if( FillVarArray( cpArg2 ) == kExpress_ERROR )
						return kExpress_ERROR;
				}

				return kExpress_TRUE;
			}
			
			// It's not a logical, therefore the first arg must be a variable...

			VAR_STRUCT *pVar1 = g_pCmdMgr->GetVar( pArg1, false );
			if( !pVar1 )	return kExpress_ERROR;

			if( !AddVar( pVar1 )) return kExpress_ERROR;
			
					
			// Is the second arg a number value or another variable...

			if( !((pArg2[0] >= '0') && (pArg2[0] <= '9')) )
			{
				VAR_STRUCT *pVar2 = g_pCmdMgr->GetVar( pArg2, false );
				if( !pVar2 ) return kExpress_ERROR;

				if( !AddVar( pVar2 )) return kExpress_TRUE;
			}
			
			return kExpress_TRUE;
		}
	}

	return kExpress_ERROR;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMD_EVENT_STRUCT::Process
//
//  PURPOSE:	See if the event has happened and procees commands...
//
// ----------------------------------------------------------------------- //

bool CMD_EVENT_STRUCT::Process( )
{
	if( m_sExpression.empty() || m_sCmds.empty() ) return false;

	const char *pExpression	= m_sExpression.c_str();
	const char *pCmds		= m_sCmds.c_str();

	// First check to see if the conditions have been met...

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( g_pCommonLT->Parse( &cpExpression ) == LT_OK )
	{
		eExpressionVal kRet = CheckExpression( cpExpression );
		if( kRet != kExpress_TRUE )
		{
			return false;
			
		}
	}

	// Expressions evaluated to TRUE, process commands...

	if( g_pCmdMgr->QueueCommand( pCmds, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL ))
	{
		// There was only a single command so process we're done...
		return true;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			g_pCmdMgr->QueueCommand( cpCommands.m_Args[i], (ILTBaseClass*)NULL, (ILTBaseClass*)NULL );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_EVENT_STRUCT::Save()
//
//	PURPOSE:	Save all the info associated with the CMD_EVENT_STRUCT
//
// ----------------------------------------------------------------------- //

void CMD_EVENT_STRUCT::Save(ILTMessage_Write *pMsg)
{
	SAVE_STDSTRING( m_sExpression );
	SAVE_STDSTRING( m_sCmds );
	SAVE_BYTE( m_nNumVarsInCmd );
	
	for( uint8 nVar = 0; nVar < m_nNumVarsInCmd; nVar++ )
	{
		VAR_STRUCT* pVar = m_aVars[nVar];
		if( pVar )
		{
			SAVE_CHARSTRING( pVar->m_sName.c_str() );
		}
		else
		{
			SAVE_CHARSTRING( "" );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_EVENT_STRUCT::Load()
//
//	PURPOSE:	Load all the info associated with the CMD_EVENT_STRUCT
//
// ----------------------------------------------------------------------- //

void CMD_EVENT_STRUCT::Load(ILTMessage_Read *pMsg)
{
	LOAD_STDSTRING( m_sExpression );
	LOAD_STDSTRING( m_sCmds );
	LOAD_BYTE( m_nNumVarsInCmd );
	
	char szName[CMDMGR_MAX_VAR_NAME_LENGTH];
	for( uint8 nVar = 0; nVar < m_nNumVarsInCmd; nVar++ )
	{
		LOAD_CHARSTRING( szName, ARRAY_LEN( szName ));
		VAR_STRUCT* pVar = g_pCmdMgr->GetVar( szName, true );
		if( pVar )
		{
			m_aVars[nVar] = pVar;
			++pVar->m_nRefCount;
		}
	}
}

//////////////////////////////////////////////
// CommandMgrPlugin implementation
//

//
// Class statics...
//

	VAR_STRUCT		CCommandMgrPlugin::s_aVars[CMDMGR_MAX_VARS];
	uint16			CCommandMgrPlugin::s_nNumVars;

	CCommandMgrPlugin::DynamicObjectList CCommandMgrPlugin::s_lstDynaObjects;

	bool			CCommandMgrPlugin::s_bFileLoadError = false;
	bool			CCommandMgrPlugin::s_bShowMsgErrors = true;
	bool			CCommandMgrPlugin::s_bShowVarErrors = true;
	bool			CCommandMgrPlugin::s_bValidateVarCmds = true;
	bool			CCommandMgrPlugin::s_bValidateNonVarCmds = true;
	bool			CCommandMgrPlugin::s_bVarDeclerationsOnly = false;
	bool			CCommandMgrPlugin::s_bValidateVarDecs = true;
	char			CCommandMgrPlugin::s_szLastWorld[128] = {0};
	char			CCommandMgrPlugin::s_szCurObject[128] = {0};
	bool			CCommandMgrPlugin::s_bCanClearVars = true;
	bool			CCommandMgrPlugin::s_bClearVarsRequested = false;
	bool			CCommandMgrPlugin::s_bDisplayPropInfo = true;
	bool			CCommandMgrPlugin::s_bForceDisplayPropInfo = false;
	bool			CCommandMgrPlugin::s_bCanClearDynaObjs = true;
	bool			CCommandMgrPlugin::s_bClearDynaObjsRequested = false;
	bool			CCommandMgrPlugin::s_bAddDynamicObjects = true;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgrPlugin::PreHook_PropChanged()
//
//	PURPOSE:	Check the passed in value to make sure it's a valid command
//
// ----------------------------------------------------------------------- //

LTRESULT CCommandMgrPlugin::PreHook_PropChanged(
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
					ILTPreInterface	*pInterface,
			const	char		*szModifiers )
{
	if( !szObjName || !szPropName || !pInterface ) return LT_UNSUPPORTED;

	// Reset the modifiers to their defaults...

	s_bShowMsgErrors			= true;
	s_bShowVarErrors			= true;
	s_bValidateVarCmds			= true;
	s_bValidateNonVarCmds		= true;
	s_bVarDeclerationsOnly		= false;
	s_bValidateVarDecs			= true;
	s_bClearVarsRequested		= false;
	s_bClearDynaObjsRequested	= false;
	s_bAddDynamicObjects		= true;

	// Read the modifiers...

	if( szModifiers[0] )
	{
		ConParse cpModifiers;
		cpModifiers.Init( szModifiers );

		while( pInterface->Parse( &cpModifiers ) == LT_OK )
		{
			if( cpModifiers.m_nArgs > 0 && cpModifiers.m_Args[0] )
			{
				if( !LTStrICmp( cpModifiers.m_Args[0], "IMsgErs" ))
				{
					s_bShowMsgErrors = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "IVarErs" ))
				{
					s_bShowVarErrors = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "IVarCmds" ))
				{
					s_bValidateVarCmds = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "INonVarCmds" ))
				{
					s_bValidateNonVarCmds = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "VVarDecsOnly" ))
				{
					s_bVarDeclerationsOnly = true;
					s_bValidateNonVarCmds = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "IVarDecs" ))
				{
					s_bValidateVarDecs = false;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "ClearVars" ))
				{
					s_bClearVarsRequested = true;
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "ClearDynaObjs" ))
				{
					s_bClearDynaObjsRequested = true;
					
					if( s_bCanClearDynaObjs )
					{
						s_lstDynaObjects.clear();
						s_bCanClearDynaObjs = false;
					}
				}
				else if( !LTStrICmp( cpModifiers.m_Args[0], "IAddDynaObj" ))
				{
					s_bAddDynamicObjects = false;
				}
			}
		}
	}

	// We got no clear var request this time so we can clear our variables again...

	if( !s_bClearVarsRequested )
	{
		s_bCanClearVars = true;
	}
	
	if( !s_bClearDynaObjsRequested )
	{
		s_bCanClearDynaObjs = true;
	}

	if( nPropType == LT_PT_COMMAND )
	{
		if( !gpPropValue.GetCommand()[0] )
			return LT_UNSUPPORTED;

		bool	bCmdFound = false;
		ConParse CommandString;
		CommandString.Init( gpPropValue.GetCommand() );

		while( pInterface->Parse( &CommandString ) == LT_OK )
		{
			if( CommandString.m_nArgs > 0 && CommandString.m_Args[0] )
			{
				CParsedMsg::CToken cTok_Cmd( CommandString.m_Args[0] );

				for( int i = 0; i < c_nNumValidCmds; ++i )
				{
					if( cTok_Cmd == s_ValidCmds[i].m_cTok_CmdName )
					{
						// We have a valid command name, check the arguments...
						
						bCmdFound = true;
						if( CheckArgs( pInterface, CommandString, s_ValidCmds[i].m_nMinArgs, s_ValidCmds[i].m_nMaxArgs ))
						{
							if( s_ValidCmds[i].m_pPreCheckFn )
							{
								// The actual PreCheck method can change these...

								s_bDisplayPropInfo		= true;
								s_bForceDisplayPropInfo	= false;
								
								LTStrCpy( s_szCurObject, "<NULL>", ARRAY_LEN( s_szCurObject ));

								if( !s_ValidCmds[i].m_pPreCheckFn( this, pInterface, CommandString ))
								{
									if( s_bDisplayPropInfo )
									{
										pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.GetCommand() );
										pInterface->CPrint( "" );
									}
								}
								else if( s_bForceDisplayPropInfo )								
								{
									// We are forced to display this info even if the validation succeded...

									pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.GetCommand() );
									pInterface->CPrint( "" );
								}

								LTStrCpy( s_szCurObject, "<NULL>", ARRAY_LEN( s_szCurObject ));

							}
							else
							{
								pInterface->CPrint( "WARNING! - PreHook_PropChanged()" );
								pInterface->CPrint( "    s_ValidCmds[%s].pPreCheckFn is Invalid!", s_ValidCmds[i].m_cTok_CmdName.c_str() );
								pInterface->CPrint( "    Unable to determine validity for command %s!", CommandString.m_Args[0] );
								pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.GetCommand() );
								pInterface->CPrint( "" );
							}
						}
						else
						{
							pInterface->CPrint( "  Syntax for %s command is: %s", s_ValidCmds[i].m_cTok_CmdName.c_str(), s_ValidCmds[i].m_pszSyntax);
							pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.GetCommand() );
							pInterface->CPrint( "" );
						}
					}
				}
			}
		}

		if( !bCmdFound )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "Not a valid command string! - %s.", gpPropValue.GetCommand() );
			pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.GetCommand() );
			pInterface->CPrint( "" );
			return LT_UNSUPPORTED;
		}
			
		return LT_OK;
	}
	
	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::CheckArgs
//
//  PURPOSE:	Make sure the number of args match what is expected
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::CheckArgs( ILTPreInterface *pInterface, ConParse &Parse, uint8 nMin, uint8 nMax )
{
	// Subtract one because the name of the command isn't counted as an argument...
	uint32 nArgs = Parse.m_nArgs - 1;
	if( (nArgs < nMin) || (nArgs > nMax) )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint("ERROR! - CheckArgs()");
		pInterface->CPrint("    %s command had %d arguments instead of the %d min and %d max!",
				  Parse.m_Args[0], nArgs, nMin, nMax);

		for (uint32 i=0; i < nArgs; i++)
		{
			pInterface->CPrint("  Arg[%d] = '%s'", i, Parse.m_Args[i]);
		}

        return false;
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::IsValidCmd
//
//  PURPOSE:	Make sure the passed in command is valid
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::IsValidCmd( ILTPreInterface *pInterface, const char *pCmd )
{
	if( !pInterface ) return false;
	
	if( !pCmd || pCmd[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - IsValidCmd()" );
		pInterface->CPrint( "    Command is empty!" );
		return false;
	}

	// ConParse does not destroy szMsg, so this is safe
	ConParse CommandString;
	CommandString.Init((char*)pCmd);

	if( pInterface->Parse( &CommandString ) == LT_OK)
	{
		if( CommandString.m_nArgs > 0 && CommandString.m_Args[0] )
		{
			CParsedMsg::CToken cTok_Cmd( CommandString.m_Args[0] );

			for( int i=0; i < c_nNumValidCmds; ++i )
			{
				if( cTok_Cmd == s_ValidCmds[i].m_cTok_CmdName )
				{
					if( !CheckArgs( pInterface, CommandString, s_ValidCmds[i].m_nMinArgs, s_ValidCmds[i].m_nMaxArgs ))
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "    Syntax for %s command is: %s", s_ValidCmds[i].m_cTok_CmdName.c_str(), s_ValidCmds[i].m_pszSyntax );
						
						return false;
					}

					if( s_ValidCmds[i].m_pPreCheckFn )
					{
						// Let the command check itself
						return s_ValidCmds[i].m_pPreCheckFn( this, pInterface, CommandString );
					}
					else
					{
						pInterface->CPrint( "ERROR! - IsValidCmd()" );
						pInterface->CPrint( "    s_ValidCmds[%s].pPreCheckFn is Invalid!", s_ValidCmds[i].m_cTok_CmdName.c_str() );
						pInterface->CPrint( "    Unable to determine validity for command %s!", pCmd );
					}

					return true;
				}
			}
			
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - IsValidCmd()" );
			pInterface->CPrint( "    Not a valid command! - %s.", CommandString.m_Args[0] );
		
		}
	}

    return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::CheckPendingCommand
//
//  PURPOSE:	Check commands that will use AddPendingCommand()
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::CheckPendingCommand( ILTPreInterface *pInterface, const CPendingCmd &cmd )
{
	if( !pInterface )
		return false;

	if( cmd.m_fDelay < 0.0f || cmd.m_fMaxDelay < 0.0f || cmd.m_fMinDelay < 0.0f ||
		(cmd.m_fMinDelay > cmd.m_fMaxDelay) )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - CheckPendingCommand()" );
		pInterface->CPrint( "  Invalid delay arguments:" );
		pInterface->CPrint( "    Delay %.2f", cmd.m_fDelay );
		pInterface->CPrint( "    MinDelay %.2f", cmd.m_fMinDelay );
		pInterface->CPrint( "    MaxDelay %.2f", cmd.m_fMaxDelay );
		return false;
	}	

	if( cmd.m_nMinTimes > cmd.m_nMaxTimes )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - CheckPendingCommand()" );
		pInterface->CPrint( "  Invalid time arguments:" );
		pInterface->CPrint( "    MinTimes %i", cmd.m_nMinTimes );
		pInterface->CPrint( "    MaxTimes %i", cmd.m_nMaxTimes );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommsndMgrPlugin::ListObjectMsgs
//
//  PURPOSE:	List all valid messages for an object going up the hierarchy
//
// ----------------------------------------------------------------------- //

void CCommandMgrPlugin::ListObjectMsgs( ILTPreInterface *pInterface, const char *pObj )
{
	if( !pInterface || !pObj[0] ) return;

	// Save a copy of the buffer since we're going to mess with it a bit
	char aMsgBuffer[CMDMGR_MAX_COMMAND_LENGTH + 1];
	LTStrCpy(aMsgBuffer, pObj, LTARRAYSIZE(aMsgBuffer));

	// Pull out the class & target names
	char *pTargetClass, *pTargetName;
	ParseMsg_Target(aMsgBuffer, &pTargetClass, &pTargetName);

	if (!pTargetClass)
	{
		// See if it is one of our dynamic objects...

		DynamicObjectList::iterator	iter;
		for( iter = s_lstDynaObjects.begin(); iter != s_lstDynaObjects.end(); ++iter )
		{
			if( !LTStrICmp( (*iter).m_sName.c_str(), pTargetName ))
			{
				pTargetClass = const_cast<char*>((*iter).m_sClassName.c_str());
				break;
			}
		}

		// See if the object is a body of an AI...

		if( !pTargetClass && strstr( pTargetName, "_body" ))
		{
			pTargetClass = "Body";
		}

		if( !pTargetClass && (LTStrIEquals( pTargetName, "player" ) ||
									  LTStrIEquals( pTargetName, "activeplayer" ) ||
									  LTStrIEquals( pTargetName, "otherplayers" ) ||
									  LTStrIEquals( pTargetName, "Team0" ) ||
									  LTStrIEquals( pTargetName, "Team1" ) ))
		{
			pTargetClass = "CPlayerObj";
		}
	}

	// Get the class type of the object we are messaging...
	// Special case "player"...

	const char	*szClassType;
	if (pTargetClass)
		szClassType = pTargetClass;
	else
		szClassType = pInterface->GetObjectClass( pTargetName );

	pInterface->CPrint( "    Valid messages for Object '%s' of class <%s>:", pObj, szClassType );

	while( szClassType )
	{
		CCmdMgr_ClassDesc	*pClassDesc = GetCmdmgrClassDescription( szClassType );
		if( !pClassDesc )
		{
			// We have no class description record for this class type.					
			return;
		}
				
		CCmdMgr_MsgDesc		*pClassMsgs = pClassDesc->m_pMsgs;
		if( !pClassMsgs || pClassDesc->m_nNumMsgs <= CMDMGR_MIN_CLASSMSGS )
		{
			// We have no messages registered for this class type, but we might for it's parent.

			szClassType = pClassDesc->m_cTok_ParentClass.c_str();
			continue; 
		}

		for( uint32 nMsg = CMDMGR_MIN_CLASSMSGS; nMsg < pClassDesc->m_nNumMsgs; ++nMsg )
		{
			pInterface->CPrint( "     %s", pClassMsgs[nMsg].m_pszSyntax );
			szClassType = pClassDesc->m_cTok_ParentClass.c_str();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckListCommands
//
//  PURPOSE:	Display the commands to the debug console
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckListCommands( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !pInterface ) return false;

	pInterface->ShowDebugWindow( true );

	for (int i=0; i < c_nNumValidCmds; i++)
	{
        pInterface->CPrint(s_ValidCmds[i].m_pszSyntax);
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckMsg
//
//  PURPOSE:	Check Msg commands for validity and make sure the object we
//				are messaging can process the message and make sure it's valid.
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckMsg( ILTPreInterface *pInterface, ConParse &parse, bool bObjVar )
{
	if( !s_bValidateNonVarCmds )
		return true;

	if( !s_bShowMsgErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show msg errors

		s_bDisplayPropInfo = false;
	}

	char* pObjectNames	= parse.m_Args[1];
	char* pMsg			= parse.m_Args[2];

    if( !pObjectNames || !pMsg || !pInterface ) return false;

	// Unfortunately, object variable messages can't be reliably checked at this point...
	if( bObjVar )
		return true;

	ConParse cpObjNames;
	cpObjNames.Init(pObjectNames);

    if( pInterface->Parse( &cpObjNames ) == LT_OK )
	{
		for( int i = 0; i < cpObjNames.m_nArgs; ++i )
		{
			// Save a copy of the buffer since we're going to mess with it a bit
			char aMsgBuffer[CMDMGR_MAX_COMMAND_LENGTH + 1];
			LTStrCpy(aMsgBuffer, cpObjNames.m_Args[i], LTARRAYSIZE(aMsgBuffer));

			// Pull out the class & target names
			char *pTargetClass, *pTargetName;
			ParseMsg_Target(aMsgBuffer, &pTargetClass, &pTargetName);

			if (!pTargetClass)
			{
				LTRESULT ltresFindObject = pInterface->FindObject( pTargetName );
				if( LT_ERROR == ltresFindObject )
				{
					// WorldEdit may not be ready so it's not really a msg failure.
					return true;
				}
				else if( LT_NOTFOUND == ltresFindObject)
				{
					// Skip objects inside prefabs...

					uint32 nNameLen = LTStrLen( pTargetName );

					if( (pTargetName[0] == '#' && pTargetName[1] == '.') ||
						(pTargetName[nNameLen-2] == '.' && pTargetName[nNameLen-1] == '#') )
					{
						continue;
					}

					// See if it is one of our dynamic objects...

					DynamicObjectList::iterator	iter;
					for( iter = s_lstDynaObjects.begin(); iter != s_lstDynaObjects.end(); ++iter )
					{
						if( !LTStrICmp( (*iter).m_sName.c_str(), pTargetName ))
						{
							pTargetClass = const_cast<char*>((*iter).m_sClassName.c_str());
							break;
						}
					}


					// Special case "player" since it is not in the world...
					// and objects inside prefabs (ie #.)

					if( !pTargetClass && (LTStrIEquals( pTargetName, "player" ) ||
											  LTStrIEquals( pTargetName, "activeplayer" ) ||
											  LTStrIEquals( pTargetName, "otherplayers" ) ||
											  LTStrIEquals( pTargetName, "Team0" ) ||
											  LTStrIEquals( pTargetName, "Team1" ) ))
					{
						pTargetClass = "CPlayerObj";
					}

					if( !pTargetClass )
					{
						if( s_bShowMsgErrors )
						{
							pInterface->ShowDebugWindow( true );
							pInterface->CPrint( "ERROR! - PreCheckMsg()" );
							pInterface->CPrint( "    MSG - Could not find object '%s'!", cpObjNames.m_Args[i] );
						}

						return false;
					}
				}
			}

			// Get the class type of the object we are messaging...
			// Special case "player"...

			const char	*szClassType;
			if (pTargetClass)
				szClassType = pTargetClass;
			else
				szClassType = pInterface->GetObjectClass( pTargetName );

			// Save the current object name for use within the validation methods...

			LTStrCpy( s_szCurObject, pTargetName, ARRAY_LEN( s_szCurObject ));

			// Check here if the object is a template object.  We shouldn't be messageing templates.
			// But our wonderful LD's alread know that!

			GenericProp gProp;
			if( pInterface->GetPropGeneric( s_szCurObject, "Template", &gProp ) == LT_OK )
			{
				if( (gProp.GetType() == LT_PT_BOOL) && gProp.GetBool())
				{
					if( s_bShowMsgErrors )
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "ERROR! - PreCheckMsg()" );
						pInterface->CPrint( "    MSG - Template objects should not be sent messages since they don't exist in the game!" );
					}

					return false;
				}
			}

				
			bool		bFoundClassType = false;
			bool		bFoundClassDesc = false;
			bool		bFoundClassMsgs = false;
			bool		bFoundMsg = false;

			while( szClassType )
			{
				bFoundClassType = true;
								
				CCmdMgr_ClassDesc	*pClassDesc = GetCmdmgrClassDescription( szClassType );
				if( !pClassDesc )
				{
					// We have no class description record for this class type.					
					break;
				}
				bFoundClassDesc = true;

				// See if we should even bother checking this class for messages...
				
				if( pClassDesc->m_dwFlags & CMDMGR_CF_MSGIGNORE )
				{
					// Pretend we found a valid message and move on to the next object...

					bFoundClassMsgs = true;
					bFoundMsg = true;

					break;
				}
				
				CCmdMgr_MsgDesc		*pClassMsgs = pClassDesc->m_pMsgs;
				if( !pClassMsgs || pClassDesc->m_nNumMsgs <= CMDMGR_MIN_CLASSMSGS )
				{
					// We have no messages registered for this class type, but we might for it's parent.

					szClassType = pClassDesc->m_cTok_ParentClass;
					continue; 
				}
				bFoundClassMsgs = true;


				ConParse cpMsgParams;
				cpMsgParams.Init( pMsg );
				
				if( pInterface->Parse( &cpMsgParams ) == LT_OK )
				{
					// Check the class descriptions list of registered messages...
					// Skip the very first one, [0], because it's a bogus message.

					CParsedMsg::CToken cTok_MsgName( cpMsgParams.m_Args[0] );

					for( uint32 nMsg = CMDMGR_MIN_CLASSMSGS; nMsg < pClassDesc->m_nNumMsgs; ++nMsg )
					{
						// Look for the message name to process...
						
						if( cTok_MsgName == pClassMsgs[nMsg].m_cTok_MsgName )
						{
							// Ok! The object can process the msg, try and validate it...

							// Negative number of arguments means a varying number or optional arguments
							if( pClassMsgs[nMsg].m_nMinArgs < 0 || 
									((cpMsgParams.m_nArgs >= pClassMsgs[nMsg].m_nMinArgs) && (cpMsgParams.m_nArgs <= pClassMsgs[nMsg].m_nMaxArgs)))
							{
								bFoundMsg = true;
								if( pClassMsgs[nMsg].m_pValidateFn )
								{
									if( !pClassMsgs[nMsg].m_pValidateFn( pInterface, cpMsgParams ))
									{
										// The message is invalid...
									
										return false;
									}
									else
									{
										// The message was validated and all is well...

										return true;
									}
								}
								else
								{
									// The object can recieve the message and there is no reason to validate it...

									return true;
								}
							}
							else
							{
								if( s_bShowMsgErrors )
								{
									pInterface->ShowDebugWindow( true );
									pInterface->CPrint( "ERROR! - PreCheckMsg()" );
									pInterface->CPrint( "    MSG - '%s' message had %i arguments instead of the %i minimum and %i maximum required.",
														pClassMsgs[nMsg].m_cTok_MsgName.c_str(), cpMsgParams.m_nArgs, pClassMsgs[nMsg].m_nMinArgs, pClassMsgs[nMsg].m_nMaxArgs );
									
									for( int nArg = 0; nArg < cpMsgParams.m_nArgs; ++nArg )
									{
										pInterface->CPrint( "    Arg[%i] = '%s'", nArg, cpMsgParams.m_Args[nArg] );
									}

									pInterface->CPrint( "  Syntax for '%s' message is: %s", pClassMsgs[nMsg].m_cTok_MsgName.c_str(), pClassMsgs[nMsg].m_pszSyntax );
								}

								return false;
							}
						}
					}
					
					if( bFoundMsg )
					{
						// Msg is good so don't bother going up the hierarchy, just go to next object.
						break;
					}

				}

				// See if the parent class can handle the message

				szClassType = pClassDesc->m_cTok_ParentClass.c_str();
			}

			if( pTargetClass && (!bFoundClassType || !bFoundClassDesc) )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Could not find class type <%s>.", pTargetClass );
				}

				return false;
			}

			if( !bFoundClassType )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->CPrint( "WARNING! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Could not find class type for object '%s'.  Grab an engineer!!", cpObjNames.m_Args[i] );
				}
				
				continue;
			}

			if( !bFoundClassDesc )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->CPrint( "WARNING! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Class type '%s' is not registered with the command mgr.  Grab an engineer!!", szClassType );
				}

				continue;
			}

			if( !bFoundClassMsgs )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->CPrint( "WARNING! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Class type '%s' has no registered messages.  Grab an engineer!!", szClassType );
				}

				continue; 
			}

			if( !bFoundMsg )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Message '%s' is not a valid message for object '%s'!", pMsg, cpObjNames.m_Args[i], szClassType );
					ListObjectMsgs( pInterface, cpObjNames.m_Args[i] );
				}

				return false;
			}
		}
	}
	else
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint("ERROR! - PreCheckMsg()");
			pInterface->CPrint("    MSG - Could not parse object name(s) '%s'!", pObjectNames);
		}

        return false;
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRandWeight
//
//  PURPOSE:	Check the rand commands for validity
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckRandWeight( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	if( !pInterface ) return false;

	float fPercent = (float) atof(parse.m_Args[1]);
	char* pCmd1		= parse.m_Args[2];
	char* pCmd2		= parse.m_Args[3];

    // Make sure percent value is good...

	if( fPercent < 0.001f || fPercent > 1.0f )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckRandWeight()" );
		pInterface->CPrint( "    RAND - Percent is %.3f. Should be between 0.001 and 1.0", fPercent );
		return false;
	}
		
	// Make sure command1 is good...

	if( !IsValidCmd( pInterface, pCmd1 ) )
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckRandWeight()" );
			pInterface->CPrint( "    RAND - command1 '%s' is invalid.", pCmd1 );
		}
		
		return false;
	}

	// Make sure command2 is good...

	if( !IsValidCmd( pInterface, pCmd2 ) )
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckRandWeight()" );
			pInterface->CPrint( "    RAND - command2 '%s' is invalid.", pCmd2 );
		}
		
		return false;
	}

	// Looks good to me
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRand
//
//  PURPOSE:	Check the rand commands for validity
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckRand( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	if( !pInterface )
		return false;

	uint32 nNumCmds	= parse.m_nArgs - 1;
	
	const char *pszCmd = NULL;
	for( uint32 nCmd = 1; nCmd <= nNumCmds; ++nCmd )
	{
		// Make sure the command is good...

		pszCmd = parse.m_Args[nCmd];

		if( !IsValidCmd( pInterface, pszCmd ) )
		{
			if( s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckRand()" );
				pInterface->CPrint( "    RAND - command%d '%s' is invalid.", nCmd, pszCmd );
			}

			return false;
		}
	}
	
	// Looks good to me
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRepeat
//
//  PURPOSE:	Check the Repeat command and the command it is repeating
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckRepeat( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;
	
	if( !pInterface ) return false;

	CPendingCmd cmd;
	cmd.m_nMinTimes	= (int) atol(parse.m_Args[1]);
	cmd.m_nMaxTimes	= (int) atol(parse.m_Args[2]);
	cmd.m_fMinDelay	= (float) atof(parse.m_Args[3]);
	cmd.m_fMaxDelay	= (float) atof(parse.m_Args[4]);
	cmd.m_sCmd		= parse.m_Args[5];

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ))
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckRepeat()" );
		pInterface->CPrint( "    REPEAT - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}
	
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRepeatId
//
//  PURPOSE:	Check the RepeatId command and the command it is repeating
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckRepeatId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	CPendingCmd cmd;
	cmd.m_sId		= parse.m_Args[1];
	cmd.m_nMinTimes	= (int) atol(parse.m_Args[2]);
	cmd.m_nMaxTimes	= (int) atol(parse.m_Args[3]);
	cmd.m_fMinDelay	= (float) atof(parse.m_Args[4]);
	cmd.m_fMaxDelay	= (float) atof(parse.m_Args[5]);
	cmd.m_sCmd		= parse.m_Args[6];

	if( cmd.m_sId.empty() )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckRepeatId()" );
		pInterface->CPrint( "    REPEATID - Invalid command Id!" );
		return false;
	}

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ) )
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckRepeatId()" );
		pInterface->CPrint( "    REPEATID - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckDelay
//
//  PURPOSE:	Check the Delay command and the command it is delaying
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckDelay( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	CPendingCmd cmd;
	cmd.m_fDelay	= (float) atof(parse.m_Args[1]);
	cmd.m_sCmd		= parse.m_Args[2];

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ))
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckDelay()" );
		pInterface->CPrint( "    DELAY - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckDelayId
//
//  PURPOSE:	Check the DelayId command and the command it is delaying
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckDelayId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	CPendingCmd cmd;
	cmd.m_sId		= parse.m_Args[1];
	cmd.m_fDelay	= (float) atof(parse.m_Args[2]);
	cmd.m_sCmd		= parse.m_Args[3];

	if( cmd.m_sId.empty() )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckDelayId()" );
		pInterface->CPrint( "    DELAYID - Invalid command Id!" );
		return false;
	}

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ))
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckDelayId()" );
		pInterface->CPrint( "    DELAYID - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckLoop
//
//  PURPOSE:	Check the Loop command and the command it is looping
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckLoop( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;
	
	CPendingCmd cmd;
	cmd.m_fMinDelay	= (float) atof(parse.m_Args[1]);
	cmd.m_fMaxDelay	= (float) atof(parse.m_Args[2]);
	cmd.m_sCmd		= parse.m_Args[3];

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ))
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckLoop()" );
		pInterface->CPrint( "    LOOP - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}
	
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckLoopId
//
//  PURPOSE:	Check the LoopId command and the command it is looping
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckLoopId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;
	
	CPendingCmd cmd;
	cmd.m_sId		= parse.m_Args[1];
	cmd.m_fMinDelay	= (float) atof(parse.m_Args[2]);
	cmd.m_fMaxDelay	= (float) atof(parse.m_Args[3]);
	cmd.m_sCmd		= parse.m_Args[4];

	if( cmd.m_sId.empty() )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckLoopId()" );
		pInterface->CPrint( "    LOOPID - Invalid command Id!" );
		return false;
	}

	if( IsValidCmd( pInterface, cmd.m_sCmd.c_str() ))
	{
		return CheckPendingCommand( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckLoopId()" );
		pInterface->CPrint( "    LOOPID - command '%s' is invalid.", cmd.m_sCmd.c_str() );
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckAbort
//
//  PURPOSE:	Check the Abort command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckAbort( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	char* pId = parse.m_Args[1];

	if( !pId || pId[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckAbort()" );
		pInterface->CPrint( "    ABORT - Invalid command Id!" );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::DevelopVars
//
//  PURPOSE:	Develop the vars at the beginig of every PreCheck that deals
//				with vars so we can edit the commands file while we edit the level
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::DevelopVars( ILTPreInterface *pInterface )
{
	// Re-parse the commands file in case it changed...

	bool bNewWorld = true;
	if( pInterface->GetWorldName() )
	{
		if( !LTStrICmp( s_szLastWorld, pInterface->GetWorldName() ))
		{
			bNewWorld = false;
		}
	}

	// Check to see if we are being forced to develop our variables...

	bool bForce = (s_bClearVarsRequested && s_bCanClearVars);
	
	// If it didn't change, no since in reading in the commands again.

	if( !s_bFileLoadError && !bNewWorld && !bForce )
		return true;

	// Wait until we get the next true request before we can clear our variables again...

	if( bForce )
	{
		s_bCanClearVars = false;
	}

	// Save the world we are checking...

	if( pInterface->GetWorldName() )
	{
		LTStrCpy( s_szLastWorld, pInterface->GetWorldName(), ARRAY_LEN( s_szLastWorld ));
	}
	else
	{
		s_szLastWorld[0] = '\0';
	}

	// Clear the current list of vars...

	for( int i = 0; i < s_nNumVars; ++i )
	{
		s_aVars[i].Clear();
	}

	s_nNumVars = 0;

	s_bFileLoadError	= false;
	s_bValidateVarCmds	= true;
	s_bValidateVarDecs	= true;

	// Check all the global commands first...
	if( !g_pCommandDB->Pre_CheckGlobalCmds( pInterface, this, "INT" ) )
		s_bFileLoadError = true;


	if( !g_pCommandDB->Pre_CheckLevelCmds( pInterface, this ))
		s_bFileLoadError = true;

	if( s_bFileLoadError && pInterface->GetWorldName() )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - DevelopVars()" );
		pInterface->CPrint( "    Could not build variable list because of error in commands.txt!");

		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::FindVar
//
//  PURPOSE:	Searches the array of vars to find the one we want
//
// ----------------------------------------------------------------------- //

VAR_STRUCT *CCommandMgrPlugin::FindVar( const char *pVarName )
{
	if( !pVarName )
		return 0;

	for( int i = 0; i < s_nNumVars; ++i )
	{
		if( !LTStrICmp( s_aVars[i].m_sName.c_str(), pVarName ))
			return &s_aVars[i];
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckInt
//
//  PURPOSE:	Check the Int command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckInt( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || !s_bValidateVarDecs )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;
	
	if( s_bFileLoadError )
		return true;

	if( s_nNumVars >= CMDMGR_MAX_VARS )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckInt()" );
		pInterface->CPrint( "    INT - Max (%i) number of variables reached!", s_nNumVars );
		
		return false;
	}

	const char *pVarName = parse.m_Args[1];
	
	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || (pVarName[0] >= '0' && pVarName[0] <= '9') )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckInt()" );
			pInterface->CPrint( "    INT - Invalid Variable name '%s'!", (pVarName ? pVarName : "NULL") );
		}

		return false;
	}

	// Make sure we don't already have a variable with this name...

	if( FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckInt()" );
			pInterface->CPrint( "    INT - Variable '%s' already declared!", pVarName );
		}
		
		return false;
	}

	// Just store the name to check for existance ...

	s_aVars[s_nNumVars].m_sName	= pVarName;
	s_aVars[s_nNumVars].m_eType	= eCMVar_Int;
	++s_nNumVars;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckObj
//
//  PURPOSE:	Check the Obj command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckObj( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || !s_bValidateVarDecs )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	if( s_nNumVars >= CMDMGR_MAX_VARS )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - PreCheckObj()" );
		pInterface->CPrint( "    OBJ - Max (%i) number of variables reached!", s_nNumVars );
		
		return false;
	}

	const char *pVarName = parse.m_Args[1];
	
	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || (pVarName[0] >= '0' && pVarName[0] <= '9') )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckObj()" );
			pInterface->CPrint( "    OBJ - Invalid Variable name '%s'!", (pVarName ? pVarName : "NULL") );
		}

		return false;
	}

	// Make sure we don't already have a variable with this name...

	if( FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckObj()" );
			pInterface->CPrint( "    OBJ - Variable '%s' already declared!", pVarName );
		}
		
		return false;
	}

	// Just store the name to check for existance ...

	s_aVars[s_nNumVars].m_sName	= pVarName;
	s_aVars[s_nNumVars].m_eType	= eCMVar_Obj;
	++s_nNumVars;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckSet
//
//  PURPOSE:	Check the Set command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckSet( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return true;
	
	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSet()" );
			pInterface->CPrint( "    SET - Invalid variable or value!" );
		}

		return false;
	}

	// Make sure we have the variable 
	
	VAR_STRUCT *pDestVar = FindVar( pVarName );
	if( !pDestVar )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSet()" );
			pInterface->CPrint( "    SET - Unknown variable '%s'!", pVarName );
		}

		return false;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return true;
	}
	else
	{
		VAR_STRUCT *pSrcVar = FindVar( pVarValue );

		const char * const k_pInvalidDesc_UnknownVar = "    SET - Unknown variable '%s'!";
		const char * const k_pInvalidDesc_UnknownObj = "    SET - Unknown object '%s'!";
		const char * const k_pInvalidDesc_TypeMismatch = "    SET - Type mismatch with variable '%s'!";

		const char *pInvalidDesc = 0;

		if( pDestVar->m_eType == eCMVar_Obj )
		{
			if( pSrcVar )
			{
				if( pSrcVar->m_eType != eCMVar_Obj )
				{
					pInvalidDesc = k_pInvalidDesc_TypeMismatch;
				}
			}
			else
			{
				LTRESULT ltresFindObject = pInterface->FindObject( pVarValue );
				if( LT_NOTFOUND == ltresFindObject )
				{
					pInvalidDesc = k_pInvalidDesc_UnknownObj;
				}
			}
		}
		else if( !pSrcVar )
		{
			pInvalidDesc = k_pInvalidDesc_UnknownVar;
		}

		if( pInvalidDesc )
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckSet()" );
				pInterface->CPrint( pInvalidDesc, pVarValue );
			}
			
			return false;
		}
	}

	return true;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckAdd
//
//  PURPOSE:	Check the Add command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckAdd( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckAdd()" );
			pInterface->CPrint( "    ADD - Invalid variable or value!" );
		}

		return false;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckAdd()" );
			pInterface->CPrint( "    ADD - Unknown variable '%s'!", pVarName );
		}

		return false;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return true;
	}
	else
	{
		if( !FindVar( pVarValue ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckAdd()" );
				pInterface->CPrint( "    ADD - Unknown variable '%s'!", pVarValue );
			}
			
			return false;
		}
	}

	return true;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckSub
//
//  PURPOSE:	Check the Subtract command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckSub( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}
	
	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SUB - Invalid variable or value!" );
		}

		return false;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SUB - Unknown variable '%s'!", pVarName );
		}

		return false;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return true;
	}
	else
	{
		if( !FindVar( pVarValue ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckSub()" );
				pInterface->CPrint( "    SUB - Unknown variable '%s'!", pVarValue );
			}

			return false;
		}
	}

	return true;	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckShowVar
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckShowVar( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SHOWVAR - Invalid variable or value!" );
		}

		return false;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SHOWVAR - Unknown variable '%s'!", pVarName );
		}
		
		return false;
	}

	return true;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckIf
//
//  PURPOSE:	Check the If command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckIf( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pExpression = parse.m_Args[1];
	const char *pThen		= parse.m_Args[2];
	const char *pCmds		= parse.m_Args[3];

	if( !pExpression || !pThen || !pCmds )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckIf()" );
			pInterface->CPrint( "    IF - Invalid expression or command!" );
		}

		return false;
	}
	
	// Check for the 'THEN'...

	if(LTStrICmp( pThen, "THEN" ) != 0 )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckIf()" );
			pInterface->CPrint( "    IF - Invalid syntax! Need 'THEN' after expressions!" );
		}

		return false;
	}

	// Check the expressions...

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( pInterface->Parse( &cpExpression ) == LT_OK )
	{
		if( !IsValidExpression( pInterface, cpExpression ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckIf()" );
				pInterface->CPrint( "    IF - Invalid expression '%s'!", pExpression );
			}
			
			return false;
		}
	}

	// Cheat by adding parens around the commands if there is only one so it will still parse correctly...

	char szCommands[256] = {0};
	
	if( pCmds[0] != '(' )
	{
		LTSNPrintF( szCommands, LTARRAYSIZE(szCommands), "(%s)", pCmds );
	}
	else
	{
		LTStrCpy( szCommands, pCmds, LTARRAYSIZE(szCommands) );
	}

	ConParse cpCommands;
	cpCommands.Init( szCommands );

	// Check the commands...

	if( pInterface->Parse( &cpCommands ) == LT_OK )
	{
		// Must have at least one command...

		if( cpCommands.m_nArgs < 1 )
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckIf()" );
				pInterface->CPrint( "    IF - No commands are listed! You need at least one command!" );
			}

			return false;
		}

		if( cpCommands.m_nArgs == 1 )
		{
			// There was only one command.  Is it valid?...
			
			if( !IsValidCmd( pInterface, cpCommands.m_Args[0] ))
			{
				if( s_bShowMsgErrors )
				{	
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - PreCheckIf()" );
					pInterface->CPrint( "    IF - Command '%s' is invalid!", cpCommands.m_Args[0] );
				}

				return false;
			}
		}
		else
		{
			// We have more than one command.  Loop over the commands and check for validity...

			for( int i = 0; i < cpCommands.m_nArgs; ++i )
			{
				if( !IsValidCmd( pInterface, cpCommands.m_Args[i] ) )
				{
					if( s_bShowMsgErrors )
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "ERROR! - PreCheckIf()" );
						pInterface->CPrint( "    IF - Command '%s' is invalid!", cpCommands.m_Args[i] );
					}

					return false;
				}
			}
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckWhen
//
//  PURPOSE:	Check the On command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckWhen( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return false;

	if( s_bFileLoadError ) return true;

	const char *pExpression = parse.m_Args[1];
	const char *pThen		= parse.m_Args[2];
	const char *pCmds		= parse.m_Args[3];

	if( !pExpression || !pThen || !pCmds )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckWhen()" );
			pInterface->CPrint( "    ON - Invalid expression or command!" );
		}

		return false;
	}
	
	// Check for the 'THEN'...

	if(LTStrICmp( pThen, "THEN" ) != 0 )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - PreCheckWhen()" );
			pInterface->CPrint( "    ON - Invalid syntax! Need 'THEN' after expressions!" );
		}

		return false;
	}

	// Check the expressions...

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( pInterface->Parse( &cpExpression ) == LT_OK )
	{
		if( !IsValidExpression( pInterface, cpExpression ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckWhen()" );
				pInterface->CPrint( "    ON - Invalid expression '%s'!", pExpression );
			}
			
			return false;
		}
	}

	// Cheat by adding parens around the commands if there is only one so it will still parse correctly...

	char szCommands[256] = {0};
	
	if( pCmds[0] != '(' )
	{
		LTSNPrintF( szCommands, LTARRAYSIZE(szCommands), "(%s)", pCmds );
	}
	else
	{
		LTStrCpy( szCommands, pCmds, LTARRAYSIZE(szCommands) );
	}

	ConParse cpCommands;
	cpCommands.Init( szCommands );

	// Check the commands...

	if( pInterface->Parse( &cpCommands ) == LT_OK )
	{
		// Must have at least one command...

		if( cpCommands.m_nArgs < 1 )
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckWhen()" );
				pInterface->CPrint( "    ON - No commands are listed! You need at least one command!" );
			}

			return false;
		}

		if( cpCommands.m_nArgs == 1 )
		{
			// There was only one command.  Is it valid?...
			
			if( !IsValidCmd( pInterface, cpCommands.m_Args[0] ))
			{
				if( s_bShowMsgErrors )
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - PreCheckWhen()" );
					pInterface->CPrint( "    ON - Command '%s' is invalid!", cpCommands.m_Args[0] );
				}

				return false;
			}
		}
		else
		{
			// We have more than one command.  Loop over the commands and check for validity...

			for( int i = 0; i < cpCommands.m_nArgs; ++i )
			{
				if( !IsValidCmd( pInterface, cpCommands.m_Args[i] ) )
				{
					if( s_bShowMsgErrors )
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "ERROR! - PreCheckWhen()" );
						pInterface->CPrint( "    ON - Command '%s' is invalid!", cpCommands.m_Args[i] );
					}
					
					return false;
				}
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckScript
//
//  PURPOSE:	Check the SCRIPT command
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::PreCheckScript(	ILTPreInterface *pInterface, ConParse & parse )
{
	if( !s_bValidateNonVarCmds )
		return true;

	if( !pInterface )
		return false;

	uint32 nNumCmds	= parse.m_nArgs - 1;

	const char *pszCmd = NULL;
	for( uint32 nCmd = 1; nCmd <= nNumCmds; ++nCmd )
	{
		// Make sure the command is good...

		pszCmd = parse.m_Args[nCmd];

		if( !IsValidCmd( pInterface, pszCmd ) )
		{
			if( s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - PreCheckScript()" );
				pInterface->CPrint( "    SCRIPT - command%d '%s' is invalid.", nCmd, pszCmd );
			}

			return false;
		}
	}

	// Looks good to me
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::IsValidExpression
//
//  PURPOSE:	Check the expressions...
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::IsValidExpression( ILTPreInterface *pInterface, ConParse &cpExpression )
{
	if( !s_bValidateVarCmds )
		return true;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}
	
	// Expressions need 3 arguments! (arg1 operator arg2)...

	if( cpExpression.m_nArgs != 3 )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - IsValidExpression()" );
			pInterface->CPrint( "    Expression had %i arguments instead of the 3 required!", cpExpression.m_nArgs );

			for( int i = 0; i < cpExpression.m_nArgs; ++i )
			{
				pInterface->CPrint("  Arg[%i] = '%s'", i, cpExpression.m_Args[i]);
			}
		}

		return false;
	}

	const char *pArg1	= cpExpression.m_Args[0];
	const char *pOp		= cpExpression.m_Args[1];
	const char *pArg2	= cpExpression.m_Args[2];

	if( !pArg1 || !pOp || !pArg2 )
	{	
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - IsValidExpression()" );
			pInterface->CPrint( "    Invalid expression '%s %s %s!", (pArg1 ? pArg1 : "NULL"), (pOp ? pOp : "NULL"), (pArg2 ? pArg2 : "NULL") );
		}
		
		return false;
	}

	// Find the operator...

	for( int iOp = 0; iOp < c_NumOperators; ++iOp )
	{
		if( !LTStrICmp( s_Operators[iOp].m_OpName, pOp ))
		{
			OPERATOR_STRUCT *pOperator = &s_Operators[iOp];

			if( pOperator->m_bLogical )
			{
				// For logical operators just parse the args and recurse into this function
				// to evaluate the two expressions...
				
				ConParse cpArg1;
				cpArg1.Init( pArg1 );

				if( pInterface->Parse( &cpArg1 ) == LT_OK )
				{
					if( !IsValidExpression( pInterface, cpArg1 ))
						return false;

					ConParse cpArg2;
					cpArg2.Init( pArg2 );

					if( pInterface->Parse( &cpArg2 ) == LT_OK )
					{
						if( !IsValidExpression( pInterface, cpArg2 ))
							return false;

						return true;
					}
				}

				if( s_bShowVarErrors )
				{	
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - IsValidExpression()" );
					pInterface->CPrint( "    Invalid expression '%s %s %s!", (pArg1 ? pArg1 : "NULL"), (pOp ? pOp : "NULL"), (pArg2 ? pArg2 : "NULL") );
				}
				
				return false;
			}

			// It's not a logical, therefore the first arg must be a variable...

			if( !FindVar( pArg1 ))
			{
				if( s_bShowVarErrors )
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - IsValidExpression()" );
					pInterface->CPrint( "    Unknown variable '%s'!", pArg1 );
				}
				
				return false;
			}

			// The second arg may either be a variable or a number...

			if( (pArg2[0] >= '0') && (pArg2[0] <= '9') )
			{
				return true;
			}
			else
			{	
				if( !FindVar( pArg2 ))
				{
					if( s_bShowVarErrors )
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "ERROR! - IsValidExpression()" );
						pInterface->CPrint( "    Unknown variable '%s'!", pArg2 );
					}

					return false;
				}
			}

			return true;
		}

	}

	if( s_bShowVarErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - IsValidExpression()" );
		pInterface->CPrint( "    Unknown operator '%s'!", pOp );
	}
	
	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::AddDynamicObject
//
//  PURPOSE:	Add an entry to the dynamic object list...
//
// ----------------------------------------------------------------------- //

void CCommandMgrPlugin::AddDynamicObject( CCommandMgrPlugin::DYNAMIC_OBJECT &DynaObj )
{
	if( !s_bAddDynamicObjects )
		return;

	// Don't add the object more than once...

	DynamicObjectList::const_iterator iter = s_lstDynaObjects.begin();
	while( iter != s_lstDynaObjects.end() )
	{
		if( !LTStrICmp( (*iter).m_sName.c_str(), DynaObj.m_sName.c_str() ))
		{
			return;	
		}

		++iter;
	}

	s_lstDynaObjects.push_back( DynaObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::DoesObjectExist
//
//  PURPOSE:	Tries to find the specified object...
//				This method should be used over ILTPreInterface::FindObject() as
//				it searches the list of dynamic objects and takes into account any
//				special object names, such as 'player'.
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::DoesObjectExist( ILTPreInterface *pInterface, const char *pszObjectName )
{
	if( !pInterface || !pszObjectName || !pszObjectName[0] )
		return false;

	// Check if the object is currently in the world...
	if( pInterface->FindObject( pszObjectName ) == LT_OK )
		return true;

	// Check the dynamic objects...
	DynamicObjectList::const_iterator iter = s_lstDynaObjects.begin();
	while( iter != s_lstDynaObjects.end() )
	{
		if( LTStrIEquals( (*iter).m_sName.c_str(), pszObjectName ))
			return true;

		++iter;
	}
	
	// Check for any special objects we know exist...
	if( LTStrIEquals( pszObjectName, "player" ))
		return true;

	// Failed to find object...
	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::GetObjectClass
//
//  PURPOSE:	Tries to find the specified object...
//				This method should be used over ILTPreInterface::FindObject() as
//				it searches the list of dynamic objects and takes into account any
//				special object names, such as 'player'.
//
// ----------------------------------------------------------------------- //

const char* CCommandMgrPlugin::GetObjectClass( ILTPreInterface *pInterface, const char *pszObjectName )
{
	if( !pInterface || !pszObjectName || !pszObjectName[0] )
		return false;

	// Check if the object is currently in the world...
	const char* pszObjectClass = pInterface->GetObjectClass( pszObjectName );
	if ( pszObjectClass )
		return pszObjectClass;

	// Check the dynamic objects...
	DynamicObjectList::const_iterator iter = s_lstDynaObjects.begin();
	while( iter != s_lstDynaObjects.end() )
	{
		if( LTStrIEquals( (*iter).m_sName.c_str(), pszObjectName ))
		{
			return (*iter).m_sClassName.c_str();
		}

		++iter;
	}
	
	// Check for any special objects we know exist...
	if( LTStrIEquals( pszObjectName, "player" ))
		return "CPlayerObj";

	// Failed to find object...
	return NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PrintCmdError
//
//  PURPOSE:	Handles printing out the command error error message 
//				structure used during command validation.
// 
//				Prints out an error message in the form:
//
//				"ERROR! - <FunctionName>()"
//				"    MSG - <CommandName> - <Parameters>"
//
// ----------------------------------------------------------------------- //

void CCommandMgrPlugin::PrintCmdError( ILTPreInterface* pInterface, const char* pszFunctionName, ConParse& cpMsgParams, const char* pszFormat, ... )
{
	if ( !pInterface 
		|| !pszFunctionName 
		|| !pszFormat 
		|| cpMsgParams.m_Args <= 0 )
	{
		return;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		char szErrorMsg[512];
		char szCmdToken[PARSE_MAXTOKENSIZE];

		LTStrCpy( szCmdToken, cpMsgParams.m_Args[0], LTARRAYSIZE( szCmdToken ) );
		LTStrUpr( szCmdToken );

		va_list marker;
		va_start( marker, pszFormat );
		LTVSNPrintF( szErrorMsg, LTARRAYSIZE( szErrorMsg ), pszFormat, marker );
		va_end( marker );

		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - %s()", pszFunctionName );
		pInterface->CPrint( "    MSG - %s - %s.", szCmdToken, szErrorMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::CommandExists
//
//  PURPOSE:	Looks at the static table of commands to see if the given string is a command...
//
// ----------------------------------------------------------------------- //

bool CCommandMgrPlugin::CommandExists( const char *pCmd )
{
	if( !pCmd )
		return false;

	CParsedMsg::CToken cTok_Cmd( pCmd );

	for( int i=0; i < c_nNumValidCmds; ++i )
	{
		if( cTok_Cmd == s_ValidCmds[i].m_cTok_CmdName )
		{
			return true;			
		}
	}

	return false;
}
