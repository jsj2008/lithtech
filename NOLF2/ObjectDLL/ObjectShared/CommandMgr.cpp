// ----------------------------------------------------------------------- //
//
// MODULE  : CommandMgr.cpp
//
// PURPOSE : CommandMgr implemenation
//
// CREATED : 06/23/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CommandMgr.h"
#include "ServerUtilities.h"
#include "GameBaseLite.h"

CCommandMgr* g_pCmdMgr = LTNULL;

extern const char* g_sPlayerClass;


// Following are wrapper functions to make adding new commands a little
// easier.
//
// To add a new command simply add a new ProcessXXX function in CommandMgr
// to process the command.  Then add a new cmdmgr_XXX static function that
// calls the new ProcessXXX function.  Then add a new entry in the s_ValidCmds
// array to map your new command to the appropriate processor function.

static LTBOOL cmdmgr_ListCommands(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessListCommands(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Delay(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessDelay(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_DelayId(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessDelayId(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Msg(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessMsg(parse, nCmdIndex, false);
    return LTFALSE;
}

static LTBOOL cmdmgr_VMsg(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessMsg(parse, nCmdIndex, true);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRand(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand2(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 2);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand3(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 3);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand4(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 4);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand5(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 5);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand6(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 6);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand7(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 7);
    return LTFALSE;
}

static LTBOOL cmdmgr_Rand8(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRandArgs(parse, nCmdIndex, 8);
    return LTFALSE;
}

static LTBOOL cmdmgr_Repeat(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRepeat(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_RepeatId(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessRepeatId(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Loop(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessLoop(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_LoopId(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessLoopId(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Abort(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex)
{
	if (pCmdMgr) return pCmdMgr->ProcessAbort(parse, nCmdIndex);
    return LTFALSE;
}

static LTBOOL cmdmgr_Int( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessInt( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_Obj( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessObj( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_Set( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessSet( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_Add( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessAdd( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_Sub( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessSub( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_If( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessIf( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_When( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessWhen( parse, nCmdIndex );
	return LTFALSE;
}

static LTBOOL cmdmgr_ShowVar( CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex )
{
	if( pCmdMgr ) return pCmdMgr->ProcessShowVar( parse, nCmdIndex );
	return LTFALSE;
}

///////////////////////////////////
// PreCheck methods
///////////////////////////////////

static LTBOOL cmdmgr_PreListCommands( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckListCommands( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreMsg( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckMsg( pInterface, parse, false );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreVMsg( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckMsg( pInterface, parse, true );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRand( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand2( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 2 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand3( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 3 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand4( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 4 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand5( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 5 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand6( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 6 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand7( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 7 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRand8( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRandArgs( pInterface, parse, 8 );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRepeat( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRepeat( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreRepeatId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckRepeatId( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreDelay( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckDelay( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreDelayId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckDelayId( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreLoop( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckLoop( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreLoopId( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckLoopId( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreAbort( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckAbort( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreInt( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckInt( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreObj( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckObj( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreSet( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckSet( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreAdd( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckAdd( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreSub( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckSub( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreIf( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckIf( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreWhen( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckWhen( pInterface, parse );
	return LTFALSE;
}

static LTBOOL cmdmgr_PreShowVar( CCommandMgrPlugin *pPlugin, ILTPreInterface *pInterface, ConParse &parse )
{
	if( pPlugin ) return pPlugin->PreCheckShowVar( pInterface, parse );
	return LTFALSE;
}

static CMD_PROCESS_STRUCT s_ValidCmds[] =
{
	CMD_PROCESS_STRUCT("LISTCOMMANDS", 1, cmdmgr_ListCommands, "Available Object Commands:", cmdmgr_PreListCommands),
	CMD_PROCESS_STRUCT("MSG", 3, cmdmgr_Msg, "  MSG <object name(s)> <object msg>", cmdmgr_PreMsg),
	CMD_PROCESS_STRUCT("VMSG", 3, cmdmgr_VMsg, "  MSG <object name(s)> <object msg>", cmdmgr_PreVMsg),
	CMD_PROCESS_STRUCT("RAND", 4, cmdmgr_Rand, "  RAND <cmd1 percent> <cmd1> <cmd2>", cmdmgr_PreRand),
	CMD_PROCESS_STRUCT("RAND2", 3, cmdmgr_Rand2, "  RAND2 <cmd1> <cmd2>", cmdmgr_PreRand2),
	CMD_PROCESS_STRUCT("RAND3", 4, cmdmgr_Rand3, "  RAND3 <cmd1> <cmd2> <cmd3>", cmdmgr_PreRand3),
	CMD_PROCESS_STRUCT("RAND4", 5, cmdmgr_Rand4, "  RAND4 <cmd1> <cmd2> <cmd3> <cmd4>", cmdmgr_PreRand4),
	CMD_PROCESS_STRUCT("RAND5", 6, cmdmgr_Rand5, "  RAND5 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5>", cmdmgr_PreRand5),
	CMD_PROCESS_STRUCT("RAND6", 7, cmdmgr_Rand6, "  RAND6 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6>", cmdmgr_PreRand6),
	CMD_PROCESS_STRUCT("RAND7", 8, cmdmgr_Rand7, "  RAND7 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6> <cmd7>", cmdmgr_PreRand7),
	CMD_PROCESS_STRUCT("RAND8", 9, cmdmgr_Rand8, "  RAND8 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6> <cmd7> <cmd8>", cmdmgr_PreRand8),
	CMD_PROCESS_STRUCT("REPEAT", 6, cmdmgr_Repeat, "  REPEAT <min times> <max times> <min delay> <max delay> <cmd>", cmdmgr_PreRepeat),
	CMD_PROCESS_STRUCT("REPEATID", 7, cmdmgr_RepeatId, "  REPEATID <cmd id> <min times> <max times> <min delay> <max delay> <cmd>", cmdmgr_PreRepeatId),
	CMD_PROCESS_STRUCT("DELAY", 3, cmdmgr_Delay, "  DELAY <time> <cmd>", cmdmgr_PreDelay),
	CMD_PROCESS_STRUCT("DELAYID", 4, cmdmgr_DelayId, "  DELAYID <cmd id> <time> <cmd>", cmdmgr_PreDelayId),
	CMD_PROCESS_STRUCT("LOOP", 4, cmdmgr_Loop, "  LOOP <min delay> <max delay> <cmd>", cmdmgr_PreLoop),
	CMD_PROCESS_STRUCT("LOOPID", 5, cmdmgr_LoopId, "  LOOPID <cmd id> <min delay> <max delay> <cmd>", cmdmgr_PreLoopId),
	CMD_PROCESS_STRUCT("ABORT", 2, cmdmgr_Abort, "  ABORT <cmd id>", cmdmgr_PreAbort),
	CMD_PROCESS_STRUCT("SET", 3, cmdmgr_Set, "  SET <variable> <value>", cmdmgr_PreSet ),
	CMD_PROCESS_STRUCT("ADD", 3, cmdmgr_Add, "  ADD <variable> <amount>", cmdmgr_PreAdd ),
	CMD_PROCESS_STRUCT("SUB", 3, cmdmgr_Sub, "  SUB <variable> <amount>", cmdmgr_PreSub ),
	CMD_PROCESS_STRUCT("IF", 4, cmdmgr_If, "  IF <expressions> THEN <commands>", cmdmgr_PreIf ),
	CMD_PROCESS_STRUCT("INT", 3, cmdmgr_Int, "  INT <variable> <value>", cmdmgr_PreInt ),
	CMD_PROCESS_STRUCT("OBJ", 3, cmdmgr_Obj, "  OBJ <variable> <value>", cmdmgr_PreObj ),
	CMD_PROCESS_STRUCT("WHEN", 4, cmdmgr_When, "  WHEN <expressions> THEN <commands>", cmdmgr_PreWhen ),
	CMD_PROCESS_STRUCT("SHOWVAR", 3, cmdmgr_ShowVar, "  SHOWVAR <variable> <1 or 0>", cmdmgr_PreShowVar )
};

const int c_nNumValidCmds = sizeof(s_ValidCmds)/sizeof(s_ValidCmds[0]);


///////////////////////////////////
// Operator methods
///////////////////////////////////

eExpressionVal CheckExpression( ConParse &cpExpression );

static LTBOOL Op_Int_equals( void *arg1, void *arg2 )
{
	return( *(int*)arg1 == *(int*)arg2 );
}

static LTBOOL Op_Int_notequals( void *arg1, void *arg2 )
{
	return( *(int*)arg1 != *(int*)arg2 );
}

static LTBOOL Op_Int_greaterthan( void *arg1, void *arg2 )
{
	return( *(int*)arg1 > *(int*)arg2 );
}

static LTBOOL Op_Int_lessthan( void *arg1, void *arg2 )
{
	return( *(int*)arg1 < *(int*)arg2 );
}

static LTBOOL Op_Int_greaterthan_equalto( void *arg1, void *arg2 )
{
	return( *(int*)arg1 >= *(int*)arg2 );
}

static LTBOOL Op_Int_lessthan_equalto( void *arg1, void *arg2 )
{
	return( *(int*)arg1 <= *(int*)arg2 );
}

static LTBOOL Op_Logical_and( void *arg1, void *arg2 )
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
					return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

static LTBOOL Op_Logical_or( void *arg1, void *arg2 )
{
	ConParse cpArg1;
	cpArg1.Init( (char*)arg1 );

	ConParse cpArg2;
	cpArg2.Init( (char*)arg2 );

	if( (g_pCommonLT->Parse( &cpArg1 ) == LT_OK) && (g_pCommonLT->Parse( &cpArg2 ) == LT_OK) )
	{
		if( (CheckExpression( cpArg1 ) == kExpress_TRUE) || (CheckExpression( cpArg2 ) == kExpress_TRUE) )
			return LTTRUE;
	}

	return LTFALSE;
}

static OPERATOR_STRUCT s_Operators[] =
{
					// Name						Logical?	Function
	// Integer
	OPERATOR_STRUCT( "==",						LTFALSE,	Op_Int_equals ),
	OPERATOR_STRUCT( "EQUALS",					LTFALSE,	Op_Int_equals ),
	OPERATOR_STRUCT( "!=",						LTFALSE,	Op_Int_notequals ),
	OPERATOR_STRUCT( "NOT_EQUALS",				LTFALSE,	Op_Int_notequals ),
	OPERATOR_STRUCT( ">",						LTFALSE,	Op_Int_greaterthan ),
	OPERATOR_STRUCT( "GREATER_THAN",			LTFALSE,	Op_Int_greaterthan ),
	OPERATOR_STRUCT( "<",						LTFALSE,	Op_Int_lessthan ),
	OPERATOR_STRUCT( "LESS_THAN",				LTFALSE,	Op_Int_lessthan ),
	OPERATOR_STRUCT( ">=",						LTFALSE,	Op_Int_greaterthan_equalto ),
	OPERATOR_STRUCT( "GREATER_THAN_OR_EQUAL_TO",LTFALSE,	Op_Int_greaterthan_equalto ),
	OPERATOR_STRUCT( "<=",						LTFALSE,	Op_Int_lessthan_equalto ),
	OPERATOR_STRUCT( "LESS_THAN_OR_EQUAL_TO",	LTFALSE,	Op_Int_lessthan_equalto ),
	OPERATOR_STRUCT( "&&",						LTTRUE,		Op_Logical_and ),
	OPERATOR_STRUCT( "AND",						LTTRUE,		Op_Logical_and ),
	OPERATOR_STRUCT( "||",						LTTRUE,		Op_Logical_or ),
	OPERATOR_STRUCT( "OR",						LTTRUE,		Op_Logical_or ),
	// Object
	OPERATOR_STRUCT( "==",						LTFALSE,	Op_Int_equals,		eCMVar_Obj ),
	OPERATOR_STRUCT( "EQUALS",					LTFALSE,	Op_Int_equals,		eCMVar_Obj ),
	OPERATOR_STRUCT( "!=",						LTFALSE,	Op_Int_notequals,	eCMVar_Obj ),
	OPERATOR_STRUCT( "NOT_EQUALS",				LTFALSE,	Op_Int_notequals,	eCMVar_Obj )
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
		if( !_stricmp( s_Operators[iOp].m_OpName, pOp ))
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
					FindNamedObject( pArg2, pVar2Obj, LTTRUE );
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


static CMDMGR_CLASS_DESC_VECTOR *s_pVecClassDesc = LTNULL;
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMDMGR_CLASS_DESC *GetClassDesc
//
//  PURPOSE:	Grab the requested record of a class description
//
// ----------------------------------------------------------------------- //

static CMDMGR_CLASS_DESC *GetClassDesc( const char *pClassName )
{
	if( !pClassName || !s_pVecClassDesc ) return LTNULL;

	CMDMGR_CLASS_DESC *pClassDesc;

	for( CMDMGR_CLASS_DESC_VECTOR::iterator iterClassDesc = s_pVecClassDesc->begin(); iterClassDesc != s_pVecClassDesc->end(); iterClassDesc++ )
	{
		pClassDesc = *iterClassDesc;
		if( !_stricmp( pClassDesc->m_szClassName, pClassName ))
		{
			return pClassDesc;	
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CCommandMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCommandMgr::CCommandMgr()
:	m_nNumVars		( 0 )
{
	g_pCmdMgr = this;
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
    g_pCmdMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Update()
//
//	PURPOSE:	Update the CommandMgr (process any pending commands)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::Update()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		if (m_PendingCmds[i].aCmd[0] != CMDMGR_NULL_CHAR)
		{
			m_PendingCmds[i].fDelay -= fTimeDelta;

			if (m_PendingCmds[i].fDelay <= 0.0f)
			{
				m_pActiveTarget = m_PendingCmds[i].pActiveTarget;
				m_pActiveSender = m_PendingCmds[i].pActiveSender;
				ProcessCmd(m_PendingCmds[i].aCmd, i);
				m_pActiveTarget = 0;
				m_pActiveSender = 0;

				// If this is a counted command, decrement the count...

				if (m_PendingCmds[i].nNumTimes >= 0)
				{
					m_PendingCmds[i].nNumTimes--;

					if (m_PendingCmds[i].nNumTimes <= 0)
					{
						m_PendingCmds[i].Clear();
						continue;
					}
				}

				// If this is a repeating command, reset the delay...

				if (m_PendingCmds[i].fMaxDelay > 0.0f)
				{
					m_PendingCmds[i].fDelay = GetRandom(m_PendingCmds[i].fMinDelay,
						m_PendingCmds[i].fMaxDelay);
				}
				else
				{
					// Clear the slot

					m_PendingCmds[i].Clear();
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessCmd()
//
//	PURPOSE:	Process the specified command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessCmd(const char* pCmd, int nCmdIndex)
{
    if (!pCmd || pCmd[0] == CMDMGR_NULL_CHAR) return LTFALSE;

	// Process the command...

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)pCmd);

    while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			for (int i=0; i < c_nNumValidCmds; i++)
			{
				if (_stricmp(parse.m_Args[0], s_ValidCmds[i].pCmdName) == 0)
				{
					if (CheckArgs(parse, s_ValidCmds[i].nNumArgs))
					{
						if (s_ValidCmds[i].pProcessFn)
						{
							if (!s_ValidCmds[i].pProcessFn(this, parse, nCmdIndex))
							{
                                return LTFALSE;
							}
						}
						else
						{
							DevPrint("CCommandMgr::ProcessCmd() ERROR!");
							DevPrint("s_ValidCmds[%d].pProcessFn is Invalid!", i);
                            return LTFALSE;
						}
					}
				}
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessDelay()
//
//	PURPOSE:	Process the Delay command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessDelay(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
    cmd.fDelay  = (LTFLOAT) atof(parse.m_Args[1]);
	cmd.pCmd	= parse.m_Args[2];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd(cmd, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessDelay() ERROR!" );
	DevPrint( "    Delayed command, %s, is invalid.", cmd.pCmd );

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessDelayId()
//
//	PURPOSE:	Process the DelayId command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessDelayId(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
	cmd.pId		= parse.m_Args[1];
    cmd.fDelay  = (LTFLOAT) atof(parse.m_Args[2]);
	cmd.pCmd	= parse.m_Args[3];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd( cmd, nCmdIndex );
	}

	DevPrint( "CCommandMgr::ProcessDelayId() ERROR!" );
	DevPrint( "    Delayed Id command, %s, is invalid.", cmd.pCmd );

	return LTFALSE;
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
//	ROUTINE:	CCommandMgr::ProcessMsg()
//
//	PURPOSE:	Process the Msg or VMsg command (send the message to the
//				specified object(s))
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessMsg(ConParse & parse, int nCmdIndex, bool bObjVar)
{
	char* pObjectNames	= parse.m_Args[1];
	char* pMsg			= parse.m_Args[2];

    if (!pObjectNames || !pMsg) return LTFALSE;

	ConParse parse2;
	parse2.Init(pObjectNames);

    if (g_pCommonLT->Parse(&parse2) == LT_OK)
	{
		for (int i=0; i < parse2.m_nArgs; i++)
		{
			// For error reporting purposes, was there something wrong with the object, or the variable?
			const char * const k_pProblemTarget_Object = "object";
			const char * const k_pProblemTarget_Variable = "variable";
			const char *pProblemTarget = k_pProblemTarget_Object;
			// Did we succeed?
			bool bResult = false;

			char *pTargetClass = NULL;
			const char *pTargetName = NULL;
			ParseMsg_Target(parse2.m_Args[i], &pTargetClass, (char**)&pTargetName);

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
						DevPrint( "    <NULL> object variable %s.  Unable to send message.", parse2.m_Args[i] );
						continue;
					}

					// Send the message
					bResult = true;
					SendTriggerMsgToObject( m_pActiveSender, pVar->m_pObjVal, LTTRUE, pMsg );
				}
				else
				{
					// Remember that there was a problem with the variable, not the object
					pProblemTarget = k_pProblemTarget_Variable;
				}
			}
			else
			{
				// Get the activesender of this message.
				char szTargetName[256];
				
				// If no target name given, use the activetarget object name.
				if(( !pTargetName || !pTargetName[0] ) && m_pActiveTarget )
				{
					if (m_pActiveTarget->m_hObject)
					{
						if( g_pLTServer->GetObjectName( m_pActiveTarget->m_hObject, szTargetName, ARRAY_LEN( szTargetName )) == LT_OK )
						{
							pTargetName = szTargetName;
						}
					}
					else
					{
						pTargetName = ((GameBaseLite*)m_pActiveTarget)->GetName();
					}
				}

				if( pTargetName && pTargetName[0] )
				{
					SendTriggerMsgToObjects( m_pActiveSender, pTargetName, pMsg );
					bResult = true;
				}
			}
			
			// Did we succeed?
			if( !bResult )
			{
				DevPrint( "CCommandMgr::ProcessMsg() ERROR!" );
				DevPrint( "    Could not find %s %s.", pProblemTarget, parse2.m_Args[i] );
			}
		}
	}
	else
	{
		DevPrint("CCommandMgr::ProcessMsg() ERROR!");
		DevPrint("    Could not parse object name(s) '%s'!", pObjectNames);
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRand()
//
//	PURPOSE:	Process the Rand command (Pick one or the other of the
//				messages based on the percent)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessRand(ConParse & parse, int nCmdIndex)
{
    LTFLOAT fPercent = (LTFLOAT) atof(parse.m_Args[1]);
	char* pCmd1		= parse.m_Args[2];
	char* pCmd2		= parse.m_Args[3];

    if (fPercent < 0.001f || fPercent > 1.0f || !pCmd1 || !pCmd2) return LTFALSE;

	if (GetRandom(0.0f, 1.0f) < fPercent)
	{
		if( IsValidCmd( pCmd1 ) )
		{
			return ProcessCmd(pCmd1, nCmdIndex);
		}

		DevPrint( "CCommandMgr::ProcessRand() ERROR!" );
		DevPrint( "    Random command1, %s, is invalid.", pCmd1 );

		return LTFALSE;
	}

	if( IsValidCmd( pCmd2 ) )
	{
		return ProcessCmd(pCmd2, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessRand() ERROR!" );
	DevPrint( "    Random command2, %s, is invalid.", pCmd2 );

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRandArgs()
//
//	PURPOSE:	Process the Rand command.  Randomly pick between one of
//				the commands to process.
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessRandArgs(ConParse & parse, int nCmdIndex, int nNumArgs)
{
    if (nNumArgs < 2) return LTFALSE;

	int nCmd = GetRandom(1, nNumArgs);

	if( IsValidCmd( parse.m_Args[nCmd] ))
	{
		return ProcessCmd(parse.m_Args[nCmd], nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessRandArgs() ERROR!" );
	DevPrint( "    Random command%d, %s, is invalid.", nCmd, parse.m_Args[nCmd] );

	return LTFALSE;
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRepeat()
//
//	PURPOSE:	Process the Repeat command (Add the command)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessRepeat(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
	cmd.nMinTimes	= (int) atol(parse.m_Args[1]);
	cmd.nMaxTimes	= (int) atol(parse.m_Args[2]);
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[3]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[4]);
	cmd.pCmd		= parse.m_Args[5];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd(cmd, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessRepeat() ERROR!" );
	DevPrint( "    Repeat command, %s, is invalid", cmd.pCmd );

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessRepeatId()
//
//	PURPOSE:	Process the RepeatId command (Add the command)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessRepeatId(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
	cmd.pId			= parse.m_Args[1];
	cmd.nMinTimes	= (int) atol(parse.m_Args[2]);
	cmd.nMaxTimes	= (int) atol(parse.m_Args[3]);
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[4]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[5]);
	cmd.pCmd		= parse.m_Args[6];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd(cmd, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessRepeatId() ERROR!" );
	DevPrint( "    Repeat Id command, %s, is invalid.", cmd.pCmd );

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessLoop()
//
//	PURPOSE:	Process the Loop command (Add the command)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessLoop(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[1]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[2]);
	cmd.pCmd		= parse.m_Args[3];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd(cmd, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessLoop() ERROR!" );
	DevPrint( "    Loop command, %s, is invalid.", cmd.pCmd );

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessLoopId()
//
//	PURPOSE:	Process the LoopId command (Add the command)
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessLoopId(ConParse & parse, int nCmdIndex)
{
	CMD_STRUCT_PARAM cmd;
	cmd.pId			= parse.m_Args[1];
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[2]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[3]);
	cmd.pCmd		= parse.m_Args[4];

	if( IsValidCmd( cmd.pCmd ) )
	{
		return AddDelayedCmd(cmd, nCmdIndex);
	}

	DevPrint( "CCommandMgr::ProcessLoopId() ERROR!" );
	DevPrint( "    Loop Id command, %s, is invalid.", cmd.pCmd );

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessAbort()
//
//	PURPOSE:	Process the Abort command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessAbort(ConParse & parse, int nCmdIndex)
{
	char* pId = parse.m_Args[1];

	if (!pId || pId[0] == CMDMGR_NULL_CHAR)
	{
		DevPrint("CCommandMgr::ProcessAbort() ERROR!");
		DevPrint("    Invalid Command Id!");
        return LTFALSE;
	}


	// Remove the specified command...

	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		if (m_PendingCmds[i].aCmd[0] != CMDMGR_NULL_CHAR &&
			m_PendingCmds[i].aId[0] != CMDMGR_NULL_CHAR)
		{
			if (_stricmp(m_PendingCmds[i].aId, pId) == 0)
			{
				m_PendingCmds[i].Clear();
                return LTTRUE;
			}
		}
	}

	DevPrint("CCommandMgr::ProcessAbort() WARNING!");
	DevPrint("    Could not find command associated with id '%s'!", pId);
    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessInteger
//
//  PURPOSE:	Process the Integer command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessInt( ConParse & parse, int nCmdIndex )
{
	if( m_nNumVars >= CMDMGR_MAX_VARS )
	{
		DevPrint( "CCommandMgr::ProcessInteger() ERROR!" );
		DevPrint( "    Max number of variables reached!" );

		return LTFALSE;
	}

	// Make sure we don't try to declare a variable we already have...

	for( int i = 0; i < m_nNumVars; ++i )
	{
		if( !_stricmp( m_aVars[i].m_szName, parse.m_Args[1] ))
		{
			DevPrint( "CCommandMgr::ProcessInteger() WARNING!" );
			DevPrint( "    Variable, %s, already defined!", m_aVars[i].m_szName );

			return LTFALSE;
		}
	}

	SAFE_STRCPY( m_aVars[m_nNumVars].m_szName, parse.m_Args[1] );
	m_aVars[m_nNumVars].m_eType = eCMVar_Int;
	m_aVars[m_nNumVars].m_iVal = atoi(parse.m_Args[2]);

	++m_nNumVars;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessObj
//
//  PURPOSE:	Process the Obj command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessObj( ConParse & parse, int nCmdIndex )
{
	if( m_nNumVars >= CMDMGR_MAX_VARS )
	{
		DevPrint( "CCommandMgr::ProcessObj() ERROR!" );
		DevPrint( "    Max number of variables reached!" );

		return LTFALSE;
	}

	// Make sure we don't try to declare a variable we already have...

	for( int i = 0; i < m_nNumVars; ++i )
	{
		if( !_stricmp( m_aVars[i].m_szName, parse.m_Args[1] ))
		{
			DevPrint( "CCommandMgr::ProcessObj() WARNING!" );
			DevPrint( "    Variable '%s' already defined!", m_aVars[i].m_szName );

			return LTFALSE;
		}
	}

	// Find the object

	ILTBaseClass *pTargetObj = 0;
	if( FindNamedObject(parse.m_Args[2], pTargetObj, LTTRUE) != LT_OK )
	{
		DevPrint( "CCommandMgr::ProcessObj() WARNING!" );
		DevPrint( "    Object '%s' not found!", parse.m_Args[2] );
	}

	// Set up the new variable

	VAR_STRUCT &sNewVar = m_aVars[m_nNumVars];

	SAFE_STRCPY( sNewVar.m_szName, parse.m_Args[1] );
	sNewVar.m_eType = eCMVar_Obj;
	sNewVar.SetObjVal(pTargetObj);

	// We now have a new variable.

	++m_nNumVars;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessSet
//
//  PURPOSE:	Process the Set command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessSet( ConParse &parse, int nCmdIndex )
{
	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessSet() ERROR!");
		DevPrint("    Invalid Variable Name!");
        return LTFALSE;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return LTFALSE;

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
				if( FindNamedObject(pVarValue, pTargetObject, LTTRUE) != LT_OK )
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
				return LTFALSE;
			}

			pVar->m_iVal = pSrcVar->m_iVal;
		}

		if( pInvalidDesc )
		{
			DevPrint( "CCommandMgr::ProcessSet() ERROR!" );
			DevPrint( const_cast<char*>(pInvalidDesc), pVarValue );
			return LTFALSE;
		}
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessAdd
//
//  PURPOSE:	Process the Add command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessAdd( ConParse &parse, int nCmdIndex )
{
	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessAdd() ERROR!");
		DevPrint("    Invalid Variable Name!");
		return LTFALSE;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return LTFALSE;

	if( pVar->m_eType != eCMVar_Int )
	{
		DevPrint("CommandMgr::ProcessAdd() ERROR!");
		DevPrint("    Invalid variable type (%s) for add!", pVarName);
		return LTFALSE;
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
		if( !pVar2 ) return LTFALSE;

		if( pVar2->m_eType != eCMVar_Int )
		{
			DevPrint("CommandMgr::ProcessAdd() ERROR!");
			DevPrint("    Invalid variable type (%s) for add!", pVarName);
			return LTFALSE;
		}

		pVar->m_iVal += pVar2->m_iVal;
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );
	
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessSub
//
//  PURPOSE:	Process the Subtract command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessSub( ConParse &parse, int nCmdIndex )
{
	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessSub() ERROR!");
		DevPrint("    Invalid Variable Name!");
		return LTFALSE;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return LTFALSE;

	if( pVar->m_eType != eCMVar_Int )
	{
		DevPrint("CommandMgr::ProcessSub() ERROR!");
		DevPrint("    Invalid variable type (%s) for sub!", pVarName);
		return LTFALSE;
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
		if( !pVar2 ) return LTFALSE;

		if( pVar2->m_eType != eCMVar_Int )
		{
			DevPrint("CommandMgr::ProcessSub() ERROR!");
			DevPrint("    Invalid variable type (%s) for sub!", pVarName);
			return LTFALSE;
		}

		pVar->m_iVal -= pVar2->m_iVal;
	}

	// Let every pending event command that contains this var know it changed so it can check its condition

	VarChanged( pVar );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessIf
//
//  PURPOSE:	Process the If command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessIf( ConParse &parse, int nCmdIndex )
{
	char *pExpression	= parse.m_Args[1];
	char *pCmds			= parse.m_Args[3];

	if( !pExpression || !pCmds )
	{
		DevPrint( "CCommandMgr::ProcessIf() ERROR!" );
		DevPrint( "    Bad arguments!" );
		return LTFALSE;
	}

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( g_pCommonLT->Parse( &cpExpression ) == LT_OK )
	{
		eExpressionVal kRet = CheckExpression( cpExpression );
		if( kRet == kExpress_FALSE )
		{
			// If statement failed to meet conditions...
			return LTTRUE;
		}
		else if( kRet == kExpress_ERROR )
		{	
			DevPrint( "CCommandMgr::ProcessIf() ERROR!" );
			DevPrint( "    CheckExpression had an error!" );
			return LTFALSE;
		}	

	}

	// Expressions evaluated to TRUE, process commands...

	if( IsValidCmd( pCmds ))
	{
		// There is only a single command so process it...

		ProcessCmd( pCmds );
		return LTTRUE;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			if( IsValidCmd( cpCommands.m_Args[i] ) )
			{
				ProcessCmd(cpCommands.m_Args[i], nCmdIndex);
			}
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgr::ProcessOn
//
//  PURPOSE:	Process the On command...
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessWhen( ConParse &parse, int nCmdIndex )
{
	char *pExpression	= parse.m_Args[1];
	char *pCmds			= parse.m_Args[3];

	if( !pExpression || !pCmds )
	{
		DevPrint( "CCommandMgr::ProcessOn() ERROR!" );
		DevPrint( "    Bad arguments!" );
		return LTFALSE;
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
				if( m_aEventCmds[i].m_hstrExpression == LTNULL )
				{
					// We have an open slot, add it here..

					m_aEventCmds[i].m_hstrExpression = g_pLTServer->CreateString( pExpression );
					m_aEventCmds[i].m_hstrCmds		= g_pLTServer->CreateString( pCmds );

					if( kExpress_ERROR == m_aEventCmds[i].FillVarArray( cpExpression ))
						return LTFALSE;

					return LTTRUE;
				}
			}

			// Couldn't find an empty slot...

			DevPrint( "CCommandMgr::ProcessWhen() ERROR!" );
			DevPrint( "    Max amount of event commands reached!" );

			return LTFALSE;
			
		}
		else if( kRet == kExpress_ERROR )
		{	
			DevPrint( "CCommandMgr::ProcessWhen() ERROR!" );
			DevPrint( "    CheckExpression had an error!" );
			return LTFALSE;
		}	

	}

	// Expressions evaluated to TRUE, process commands...

	if( IsValidCmd( pCmds ))
	{
		// There is only a single command so process it...

		ProcessCmd( pCmds );
		return LTTRUE;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			if( IsValidCmd( cpCommands.m_Args[i] ) )
			{
				ProcessCmd(cpCommands.m_Args[i], nCmdIndex);
			}
		}
	}

	return LTTRUE;
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
			g_pLTServer->CPrint( "%s: %i", pVar->m_szName, pVar->m_iVal );
		}
		else if( pVar->m_eType == eCMVar_Obj )
		{
			const char *pObjName = "<NULL>";
			if( pVar->m_hObjVal )
				pObjName = GetObjectName( pVar->m_hObjVal );
			else if( pVar->m_pObjVal )
				pObjName = ((GameBaseLite*)pVar->m_pObjVal)->GetName();
			g_pLTServer->CPrint( "%s: %s", pVar->m_szName, pObjName );
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

LTBOOL CCommandMgr::ProcessShowVar( ConParse &parse, int nCmdIndex )
{
	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || !pVarValue )
	{
		DevPrint("CCommandMgr::ProcessShowVar() ERROR!");
		DevPrint("    Invalid Variable Name!");
        return LTFALSE;
	}

	// Find the variable we wish to set...

	VAR_STRUCT *pVar = GetVar( pVarName, false );
	if( !pVar )	return LTFALSE;

	pVar->m_bShow = LTBOOL(atoi( pVarValue ));

	// Display the value if the var was told to SHOW...

	if( pVar->m_bShow )
	{
		if( pVar->m_eType == eCMVar_Int )
		{
			g_pLTServer->CPrint( "%s: %i", pVar->m_szName, pVar->m_iVal );
		}
		else if( pVar->m_eType == eCMVar_Obj )
		{
			const char *pObjName = "<NULL>";
			if( pVar->m_hObjVal )
				pObjName = GetObjectName( pVar->m_hObjVal );
			else if( pVar->m_pObjVal )
				pObjName = ((GameBaseLite*)pVar->m_pObjVal)->GetName();
			g_pLTServer->CPrint( "%s: %s", pVar->m_szName, pObjName );
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::AddDelayedCmd()
//
//	PURPOSE:	Add a delayed command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::AddDelayedCmd(CMD_STRUCT_PARAM & cmd, int nCmdIndex)
{
	if ((cmd.fDelay < 0.0f && cmd.fMaxDelay < 0.0f) || !cmd.pCmd ||
		cmd.pCmd[0] == CMDMGR_NULL_CHAR)
	{
		DevPrint("CCommandMgr::AddDelayedCmd() ERROR!");
		DevPrint("Invalid arguments:");
		DevPrint("  Delay %.2f", cmd.fDelay);
		DevPrint("  MinTimes %d", cmd.nMinTimes);
		DevPrint("  MaxTimes %d", cmd.nMaxTimes);
		DevPrint("  MinDelay %.2f", cmd.fMinDelay);
		DevPrint("  MaxDelay %.2f", cmd.fMaxDelay);
		DevPrint("  Command %s", cmd.pCmd ? cmd.pCmd : "NULL");
		DevPrint("  Id %s", cmd.pId ? cmd.pId : "NULL");
        return LTFALSE;
	}


	// See if this command was part of a previously pending command...

    LTBOOL bWasPending = LTFALSE;
	if (nCmdIndex >= 0 && nCmdIndex < CMDMGR_MAX_PENDING_COMMANDS)
	{
        bWasPending = LTTRUE;
	}


	// Find a the first open position for the command...
    int i;
    for (i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		if (m_PendingCmds[i].aCmd[0] == CMDMGR_NULL_CHAR)
		{
			// Make sure we don't try to use the same slot that our parent
			// command was using...

			if (!bWasPending || i != nCmdIndex)
			{
				m_PendingCmds[i].Clear();

				// Set times if appropriate...

				if (cmd.nMaxTimes > 0)
				{
					m_PendingCmds[i].nMinTimes = cmd.nMinTimes;
					m_PendingCmds[i].nMaxTimes = cmd.nMaxTimes;
					m_PendingCmds[i].nNumTimes = GetRandom(cmd.nMinTimes, cmd.nMaxTimes);
				}

				// Set delay...

				if (cmd.fMaxDelay > 0.0f)
				{
					m_PendingCmds[i].fMinDelay = cmd.fMinDelay;
					m_PendingCmds[i].fMaxDelay = cmd.fMaxDelay;
					m_PendingCmds[i].fDelay = GetRandom(cmd.fMinDelay, cmd.fMaxDelay);
				}
				else
				{
					m_PendingCmds[i].fDelay = cmd.fDelay;
				}

				// Remember the active target and sender

				m_PendingCmds[i].SetActiveTarget(m_pActiveTarget);
				m_PendingCmds[i].SetActiveSender(m_pActiveSender);


				// The id isn't used with every command, so it may not be valid...

				if (cmd.pId && cmd.pId[0])
				{
					strncpy(m_PendingCmds[i].aId, cmd.pId, CMDMGR_MAX_ID_LENGTH);
					m_PendingCmds[i].aId[CMDMGR_MAX_ID_LENGTH-1] = CMDMGR_NULL_CHAR;
				}

				// Set the command...

				strncpy(m_PendingCmds[i].aCmd, cmd.pCmd, CMDMGR_MAX_COMMAND_LENGTH);
				m_PendingCmds[i].aCmd[CMDMGR_MAX_COMMAND_LENGTH-1] = CMDMGR_NULL_CHAR;

				break;
			}
		}
	}


	// Make sure we found a slot for it...

	if (i == CMDMGR_MAX_PENDING_COMMANDS)
	{
		DevPrint("CCommandMgr::AddDelayedCmd() WARNING!");
		DevPrint("    Pending command buffer overflow!");
		DevPrint("    Could not delay command '%s'!", cmd.pCmd);
		DevPrint("    Processing command now!");

		ProcessCmd(cmd.pCmd);
        return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::IsValidCmd()
//
//	PURPOSE:	See if the command is valid
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::IsValidCmd(const char* pCmd)
{
    if (!pCmd || pCmd[0] == CMDMGR_NULL_CHAR) return LTFALSE;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)pCmd);

    if (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			for (int i=0; i < c_nNumValidCmds; i++)
			{
				if (_stricmp(parse.m_Args[0], s_ValidCmds[i].pCmdName) == 0)
				{
					if( !CheckArgs(parse, s_ValidCmds[i].nNumArgs) )
					{
						DevPrint( "Syntax for %s command is: %s", s_ValidCmds[i].pCmdName, s_ValidCmds[i].pSyntax );
						return LTFALSE;
					}

					return LTTRUE;
				}
			}
		}
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Process()
//
//	PURPOSE:	Process a command request
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::Process(const char* pCmd, ILTBaseClass *pSender, ILTBaseClass *pDefaultTarget) 
{ 
	m_pActiveTarget = pDefaultTarget;  
	m_pActiveSender = pSender;
	LTBOOL bResult = ProcessCmd(pCmd); 
	m_pActiveTarget = NULL;
	m_pActiveSender = NULL;
	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CheckArgs()
//
//	PURPOSE:	Make sure the number of args match what is expected
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::CheckArgs(ConParse & parse, int nNum)
{
	if (parse.m_nArgs != nNum)
	{
		DevPrint("CCommandMgr::CheckArgs() ERROR!");
		DevPrint("    %s command had %d arguments instead of the %d required!",
				  parse.m_Args[0], parse.m_nArgs, nNum);

		for (int i=0; i < parse.m_nArgs; i++)
		{
			DevPrint("  Arg[%d] = '%s'", i, parse.m_Args[i]);
		}

        return LTFALSE;
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessListCommands()
//
//	PURPOSE:	List available commands
//
// --------------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessListCommands(ConParse & parse, int nCmdIndex)
{
	for (int i=0; i < c_nNumValidCmds; i++)
	{
        g_pLTServer->CPrint(s_ValidCmds[i].pSyntax);
	}

    return LTTRUE;
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
	int nSuccess = vsprintf(pMsg, msg, marker);
	va_end(marker);

	if (nSuccess < 0) return;

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

void CCommandMgr::Save(ILTMessage_Write *pMsg)
{
	int i;
	for (i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Save(pMsg);
	}

	// Count the number of level specific vars...

	int nVars = 0;
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
			m_aVars[i].Save( pMsg, LTFALSE );
	}

	//jrg - 9/8/02 - we need to save the names of global variables here
	// so that when loading, the Event commands can reference them
	// the values of the global variables need to be saved with the player, so that 
	// the will be saved/loaded on transitions
	SaveGlobalVars(pMsg,LTTRUE);

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

void CCommandMgr::Load(ILTMessage_Read *pMsg)
{
	int i;
	for (i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Load(pMsg);
	}

	int nVars = 0;
	LOAD_INT( nVars );

	for( i = 0; i < nVars; ++i )
	{
		ASSERT( m_nNumVars < CMDMGR_MAX_VARS );

		m_aVars[m_nNumVars].Load( pMsg, LTFALSE );
		++m_nNumVars;
	}

	//jrg - 9/8/02 - we need to load the names of global variables here
	// so that the Event commands can reference them
	// the values of the global variables need to be saved with the player, so that 
	// the will be saved/loaded on transitions
	LoadGlobalVars(pMsg,LTTRUE);

	for( i = 0; i < CMDMGR_MAX_EVENT_COMMANDS; i++ )
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

void CCommandMgr::SaveGlobalVars( ILTMessage_Write *pMsg, LTBOOL bNameOnly )
{
	// Count the number of globals...

	int nGlobals = 0;
	for( int i = 0; i < m_nNumVars; ++i )
	{
		if( m_aVars[i].m_bGlobal )
			++nGlobals;
	}

	SAVE_INT( nGlobals );

	// Save the globals...

	for(int i = 0; i < m_nNumVars; ++i )
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

void CCommandMgr::LoadGlobalVars( ILTMessage_Read *pMsg, LTBOOL bNameOnly )
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
		pVar = GetVar( TempVar.m_szName, true, &nId );
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

VAR_STRUCT* CCommandMgr::GetVar( const char *pName, bool bSilent, uint16 *nId /*= LTNULL*/ )
{
	if( !pName ) return LTNULL;

	if( nId )
		*nId = (uint16)-1;

	for( int iVar = 0; iVar < m_nNumVars; ++iVar )
	{
		if( !_stricmp( m_aVars[iVar].m_szName, pName ))
		{
			if( nId )
				*nId = iVar;

			return &m_aVars[iVar];
		}
	}

	if (!bSilent)
	{
		DevPrint( "CCommandMgr::GetVar() ERROR!" );
		DevPrint( "    Could not find requested variable '%s'!", pName );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::Load()
//
//	PURPOSE:	Load all the info associated with the CMD_STRUCT
//
// ----------------------------------------------------------------------- //

void CMD_STRUCT::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_FLOAT(fDelay);
    LOAD_FLOAT(fMinDelay);
    LOAD_FLOAT(fMaxDelay);
    LOAD_INT(nNumTimes);
    LOAD_INT(nMinTimes);
    LOAD_INT(nMaxTimes);
	LOAD_COBJECT(pActiveTarget, ILTBaseClass);
	hActiveTarget = pActiveTarget ? pActiveTarget->m_hObject : LTNULL;
	LOAD_COBJECT(pActiveSender, ILTBaseClass);
	hActiveSender = pActiveSender ? pActiveSender->m_hObject : LTNULL;

	HSTRING hstr;
    LOAD_HSTRING(hstr);
	if (hstr)
	{
        strncpy(aCmd, g_pLTServer->GetStringData(hstr), CMDMGR_MAX_COMMAND_LENGTH);
		aCmd[CMDMGR_MAX_COMMAND_LENGTH-1] = CMDMGR_NULL_CHAR;
        g_pLTServer->FreeString(hstr);
	}
	else
	{
		aCmd[0] = CMDMGR_NULL_CHAR;
	}

	LOAD_HSTRING(hstr);
	if (hstr)
	{
        strncpy(aId, g_pLTServer->GetStringData(hstr), CMDMGR_MAX_ID_LENGTH);
		aId[CMDMGR_MAX_ID_LENGTH-1] = CMDMGR_NULL_CHAR;
        g_pLTServer->FreeString(hstr);
	}
	else
	{
		aId[0] = CMDMGR_NULL_CHAR;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::Save()
//
//	PURPOSE:	Save all the info associated with the CMD_STRUCT
//
// ----------------------------------------------------------------------- //

void CMD_STRUCT::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_FLOAT(fDelay);
    SAVE_FLOAT(fMinDelay);
    SAVE_FLOAT(fMaxDelay);
    SAVE_INT(nNumTimes);
    SAVE_INT(nMinTimes);
    SAVE_INT(nMaxTimes);
	SAVE_COBJECT(pActiveTarget);
	SAVE_COBJECT(pActiveSender);

    HSTRING hstr = LTNULL;
	if (aCmd[0] != CMDMGR_NULL_CHAR)
	{
        hstr = g_pLTServer->CreateString(aCmd);
	}

	SAVE_HSTRING(hstr);

	if (hstr)
	{
        g_pLTServer->FreeString(hstr);
	}

    hstr = LTNULL;
	if (aId[0] != CMDMGR_NULL_CHAR)
	{
        hstr = g_pLTServer->CreateString(aId);
	}

	SAVE_HSTRING(hstr);

	if (hstr)
	{
        g_pLTServer->FreeString(hstr);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VAR_STRUCT::Save
//
//  PURPOSE:	Save a variable
//
// ----------------------------------------------------------------------- //

void VAR_STRUCT::Save( ILTMessage_Write *pMsg, LTBOOL bNameOnly  )
{
	HSTRING hstr = LTNULL;
	if( m_szName[0] != CMDMGR_NULL_CHAR )
	{
		hstr = g_pLTServer->CreateString( m_szName );
	}

	SAVE_HSTRING( hstr );

	if (!bNameOnly )
	{
		SAVE_DWORD( (uint32)m_eType );
		SAVE_INT( m_iVal );
		SAVE_COBJECT( m_pObjVal );
		SAVE_BOOL( m_bGlobal );
		SAVE_INT( m_nRefCount );
	}

	FREE_HSTRING( hstr );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	VAR_STRUCT::Load
//
//  PURPOSE:	Load a var
//
// ----------------------------------------------------------------------- //

void VAR_STRUCT::Load( ILTMessage_Read *pMsg, LTBOOL bNameOnly  )
{
	HSTRING hstr = LTNULL;

	LOAD_HSTRING( hstr );
	if( hstr )
	{
        strncpy( m_szName, g_pLTServer->GetStringData( hstr ), CMDMGR_MAX_VAR_NAME_LENGTH );
		m_szName[ CMDMGR_MAX_VAR_NAME_LENGTH-1 ] = CMDMGR_NULL_CHAR;
        g_pLTServer->FreeString( hstr );
	}
	else
	{
		m_szName[0] = CMDMGR_NULL_CHAR;
	}


	if (!bNameOnly )
	{
		LOAD_DWORD_CAST( m_eType, ECmdMgrVarType );
		LOAD_INT( m_iVal );
		LOAD_COBJECT( m_pObjVal, ILTBaseClass );
		m_hObjVal = (m_pObjVal) ? m_pObjVal->m_hObject : LTNULL;
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
		if( !_stricmp( s_Operators[iOp].m_OpName, pOp ))
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

LTBOOL CMD_EVENT_STRUCT::Process( )
{
	if( !m_hstrExpression || !m_hstrCmds ) return LTFALSE;

	const char *pExpression = g_pLTServer->GetStringData( m_hstrExpression );
	const char *pCmds		= g_pLTServer->GetStringData( m_hstrCmds );

	// First check to see if the conditions have been met...

	ConParse cpExpression;
	cpExpression.Init( pExpression );

	if( g_pCommonLT->Parse( &cpExpression ) == LT_OK )
	{
		eExpressionVal kRet = CheckExpression( cpExpression );
		if( kRet != kExpress_TRUE )
		{
			return LTFALSE;
			
		}
	}

	// Expressions evaluated to TRUE, process commands...

	if( g_pCmdMgr->IsValidCmd( pCmds ))
	{
		// There is only a single command so process it...

		g_pCmdMgr->Process( pCmds, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL );
		return LTTRUE;
	}

	ConParse cpCommands;
	cpCommands.Init( pCmds );

	if( g_pCommonLT->Parse( &cpCommands ) == LT_OK )
	{
		for( int i = 0; i < cpCommands.m_nArgs; ++i )
		{
			if( g_pCmdMgr->IsValidCmd( cpCommands.m_Args[i] ) )
			{
				g_pCmdMgr->Process( cpCommands.m_Args[i], (ILTBaseClass*)NULL, (ILTBaseClass*)LTNULL );
			}
		}
	}

	return LTTRUE;
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
	SAVE_HSTRING( m_hstrExpression );
	SAVE_HSTRING( m_hstrCmds );
	SAVE_BYTE( m_nNumVarsInCmd );
	
	for( uint8 nVar = 0; nVar < m_nNumVarsInCmd; nVar++ )
	{
		VAR_STRUCT* pVar = m_aVars[nVar];
		if( pVar )
		{
			SAVE_CHARSTRING( pVar->m_szName );
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
	LOAD_HSTRING( m_hstrExpression );
	LOAD_HSTRING( m_hstrCmds );
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
	uint8			CCommandMgrPlugin::s_nNumVars;
	CCommandButeMgr CCommandMgrPlugin::s_CommandButeMgr;

	CCommandMgrPlugin::DynamicObjectList CCommandMgrPlugin::s_lstDynaObjects;

	LTBOOL			CCommandMgrPlugin::s_bFileLoadError = LTFALSE;
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
//  ROUTINE:	CMDMGR_CLASS_DESC::CMDMGR_CLASS_DESC
//
//  PURPOSE:	Constructor adds this instance to the static vector of class descriptions
//
// ----------------------------------------------------------------------- //

CMDMGR_CLASS_DESC::CMDMGR_CLASS_DESC( char *pClassName, char *pParentClass, int nNumMsgs, MSG_PRECHECK *pMsgs, uint32 dwFlags )
{
	if( nNumMsgs < 1 ) return;
	
	m_szClassName	= pClassName;
	m_szParentClass	= pParentClass;
	m_nNumMsgs		= nNumMsgs;
	m_pMsgs			= pMsgs;
	m_dwFlags		= dwFlags;

	// Declare static vector here to guarantee it exists before trying to use it.
	
	static CMDMGR_CLASS_DESC_VECTOR s_vecClassDesc;
	s_pVecClassDesc = &s_vecClassDesc; 

	// Add the class description.

	s_vecClassDesc.push_back( this );
}




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
				if( !_stricmp( cpModifiers.m_Args[0], "IMsgErs" ))
				{
					s_bShowMsgErrors = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "IVarErs" ))
				{
					s_bShowVarErrors = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "IVarCmds" ))
				{
					s_bValidateVarCmds = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "INonVarCmds" ))
				{
					s_bValidateNonVarCmds = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "VVarDecsOnly" ))
				{
					s_bVarDeclerationsOnly = true;
					s_bValidateNonVarCmds = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "IVarDecs" ))
				{
					s_bValidateVarDecs = false;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "ClearVars" ))
				{
					s_bClearVarsRequested = true;
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "ClearDynaObjs" ))
				{
					s_bClearDynaObjsRequested = true;
					
					if( s_bCanClearDynaObjs )
					{
						s_lstDynaObjects.clear();
						s_bCanClearDynaObjs = false;
					}
				}
				else if( !_stricmp( cpModifiers.m_Args[0], "IAddDynaObj" ))
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

	if( nPropType == LT_PT_STRING )
	{
		if( !gpPropValue.m_String[0] ) 
			return LT_UNSUPPORTED;

		LTBOOL	bCmdFound = LTFALSE;
		ConParse CommandString;
		CommandString.Init( gpPropValue.m_String );

		while( pInterface->Parse( &CommandString ) == LT_OK )
		{
			if( CommandString.m_nArgs > 0 && CommandString.m_Args[0] )
			{
				for( int i = 0; i < c_nNumValidCmds; ++i )
				{
					if( !_stricmp( CommandString.m_Args[0], s_ValidCmds[i].pCmdName ))
					{
						// We have a valid command name, check the arguments...
						
						bCmdFound = LTTRUE;

						if( CheckArgs( pInterface, CommandString, s_ValidCmds[i].nNumArgs ))
						{
							if( s_ValidCmds[i].pPreCheckFn )
							{
								// The actual PreCheck method can change these...

								s_bDisplayPropInfo		= true;
								s_bForceDisplayPropInfo	= false;
								
								LTStrCpy( s_szCurObject, "<NULL>", ARRAY_LEN( s_szCurObject ));

								if( !s_ValidCmds[i].pPreCheckFn( this, pInterface, CommandString ))
								{
									if( s_bDisplayPropInfo )
									{
										pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.m_String );
										pInterface->CPrint( "" );
									}
								}
								else if( s_bForceDisplayPropInfo )								
								{
									// We are forced to display this info even if the validation succeded...

									pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.m_String );
									pInterface->CPrint( "" );
								}

								LTStrCpy( s_szCurObject, "<NULL>", ARRAY_LEN( s_szCurObject ));

							}
							else
							{
								pInterface->CPrint( "WARNING! - PreHook_PropChanged()" );
								pInterface->CPrint( "    s_ValidCmds[%s].pPreCheckFn is Invalid!", s_ValidCmds[i].pCmdName );
								pInterface->CPrint( "    Unable to determine validity for command %s!", CommandString.m_Args[0] );
								pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.m_String );
								pInterface->CPrint( "" );
							}
						}
						else
						{
							pInterface->CPrint( "  Syntax for %s command is: %s", s_ValidCmds[i].pCmdName, s_ValidCmds[i].pSyntax);
							pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.m_String );
							pInterface->CPrint( "" );
						}
					}
				}
			}
		}

		if( !bCmdFound )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "Not a valid command string! - %s.", gpPropValue.m_String );
			pInterface->CPrint( "Object: %s    Property: %s    Command: '%s'", szObjName, szPropName, gpPropValue.m_String );
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

LTBOOL CCommandMgrPlugin::CheckArgs( ILTPreInterface *pInterface, ConParse &Parse, int nNum )
{
	if( Parse.m_nArgs != nNum )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint("ERROR! - CheckArgs()");
		pInterface->CPrint("    %s command had %d arguments instead of the %d required!",
				  Parse.m_Args[0], Parse.m_nArgs, nNum);

		for (int i=0; i < Parse.m_nArgs; i++)
		{
			pInterface->CPrint("  Arg[%d] = '%s'", i, Parse.m_Args[i]);
		}

        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::IsValidCmd
//
//  PURPOSE:	Make sure the passed in command is valid
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::IsValidCmd( ILTPreInterface *pInterface, const char *pCmd )
{
	if( !pInterface ) return LTFALSE;
	
	if( !pCmd || pCmd[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - IsValidCmd()" );
		pInterface->CPrint( "    Command is empty!" );
		return LTFALSE;
	}

	// ConParse does not destroy szMsg, so this is safe
	ConParse CommandString;
	CommandString.Init((char*)pCmd);

    if( pInterface->Parse( &CommandString ) == LT_OK)
	{
		if( CommandString.m_nArgs > 0 && CommandString.m_Args[0] )
		{
			for( int i=0; i < c_nNumValidCmds; ++i )
			{
				if( !_stricmp( CommandString.m_Args[0], s_ValidCmds[i].pCmdName ))
				{
					if( !CheckArgs( pInterface, CommandString, s_ValidCmds[i].nNumArgs ))
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "    Syntax for %s command is: %s", s_ValidCmds[i].pCmdName, s_ValidCmds[i].pSyntax );
						
						return LTFALSE;
					}

					if( s_ValidCmds[i].pPreCheckFn )
					{
						// Let the command check itself
						return s_ValidCmds[i].pPreCheckFn( this, pInterface, CommandString );
					}
					else
					{
						pInterface->CPrint( "ERROR! - IsValidCmd()" );
						pInterface->CPrint( "    s_ValidCmds[%s].pPreCheckFn is Invalid!", s_ValidCmds[i].pCmdName );
						pInterface->CPrint( "    Unable to determine validity for command %s!", pCmd );
					}

					return LTTRUE;
				}
			}
			
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - IsValidCmd()" );
			pInterface->CPrint( "    Not a valid command! - %s.", CommandString.m_Args[0] );
		
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::CheckDelayedCmd
//
//  PURPOSE:	Check commands that will use AddDelayedCommand()
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::CheckDelayedCmd( ILTPreInterface *pInterface, CMD_STRUCT_PARAM &cmd )
{
	if( !pInterface ) return LTFALSE;

	if( cmd.fDelay < 0.0f || cmd.fMaxDelay < 0.0f || cmd.fMinDelay < 0.0f ||
		(cmd.fMinDelay > cmd.fMaxDelay) )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - CheckDelayedCmd()" );
		pInterface->CPrint( "  Invalid delay arguments:" );
		pInterface->CPrint( "    Delay %.2f", cmd.fDelay );
		pInterface->CPrint( "    MinDelay %.2f", cmd.fMinDelay );
		pInterface->CPrint( "    MaxDelay %.2f", cmd.fMaxDelay );
		return LTFALSE;
	}	

	if( cmd.nMinTimes > cmd.nMaxTimes )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - CheckDelayedCmd()" );
		pInterface->CPrint( "  Invalid time arguments:" );
		pInterface->CPrint( "    MinTimes %i", cmd.nMinTimes );
		pInterface->CPrint( "    MaxTimes %i", cmd.nMaxTimes );
		return LTFALSE;
	}

	return LTTRUE;
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
	SAFE_STRCPY(aMsgBuffer, pObj);

	// Pull out the class & target names
	char *pTargetClass, *pTargetName;
	ParseMsg_Target(aMsgBuffer, &pTargetClass, &pTargetName);

	if (!pTargetClass)
	{
		// See if it is one of our dynamic objects...

		DynamicObjectList::iterator	iter;
		for( iter = s_lstDynaObjects.begin(); iter != s_lstDynaObjects.end(); ++iter )
		{
			if( !_stricmp( (*iter).m_sName.c_str(), pTargetName ))
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

		if( !pTargetClass && (!_stricmp( pTargetName, "player" ) ||
									  !_stricmp( pTargetName, "activeplayer" ) ||
									  !_stricmp( pTargetName, "otherplayers" ) ))
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
		CMDMGR_CLASS_DESC	*pClassDesc = GetClassDesc( szClassType );
		if( !pClassDesc )
		{
			// We have no class description record for this class type.					
			return;
		}
				
		MSG_PRECHECK		*pClassMsgs = pClassDesc->m_pMsgs;
		if( !pClassMsgs || pClassDesc->m_nNumMsgs <= 1 )
		{
			// We have no messages registered for this class type, but we might for it's parent.

			szClassType = pClassDesc->m_szParentClass;
			continue; 
		}

		for( int nMsg = 1; nMsg < pClassDesc->m_nNumMsgs; ++nMsg )
		{
			// No syntax for special messages...

			if( !pClassMsgs[nMsg].m_bSpecial )
			{
				pInterface->CPrint( "     %s", pClassMsgs[nMsg].m_szSyntax );
			}

			szClassType = pClassDesc->m_szParentClass;
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

LTBOOL CCommandMgrPlugin::PreCheckListCommands( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !pInterface ) return LTFALSE;

	pInterface->ShowDebugWindow( LTTRUE );

	for (int i=0; i < c_nNumValidCmds; i++)
	{
        pInterface->CPrint(s_ValidCmds[i].pSyntax);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckMsg
//
//  PURPOSE:	Check Msg commands for validity and make sure the object we
//				are messaging can process the message and make sure it's valid.
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckMsg( ILTPreInterface *pInterface, ConParse &parse, bool bObjVar )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	if( !s_bShowMsgErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show msg errors

		s_bDisplayPropInfo = false;
	}

	char* pObjectNames	= parse.m_Args[1];
	char* pMsg			= parse.m_Args[2];

    if( !pObjectNames || !pMsg || !pInterface ) return LTFALSE;

	// Unfortunately, object variable messages can't be reliably checked at this point...
	if( bObjVar )
		return LTTRUE;

	ConParse cpObjNames;
	cpObjNames.Init(pObjectNames);

    if( pInterface->Parse( &cpObjNames ) == LT_OK )
	{
		for( int i = 0; i < cpObjNames.m_nArgs; ++i )
		{
			// Save a copy of the buffer since we're going to mess with it a bit
			char aMsgBuffer[CMDMGR_MAX_COMMAND_LENGTH + 1];
			SAFE_STRCPY(aMsgBuffer, cpObjNames.m_Args[i]);

			// Pull out the class & target names
			char *pTargetClass, *pTargetName;
			ParseMsg_Target(aMsgBuffer, &pTargetClass, &pTargetName);

			if (!pTargetClass)
			{
				LTRESULT ltresFindObject = pInterface->FindObject( pTargetName );
				if( LT_ERROR == ltresFindObject )
				{
					// DEdit may not be ready so it's not really a msg failure.
					return LTTRUE;
				}
				else if( LT_NOTFOUND == ltresFindObject)
				{
					// Skip objects inside prefabs...

					if( pTargetName[0] == '#' && pTargetName[1] == '.' )
					{
						continue;
					}

					// See if it is one of our dynamic objects...

					DynamicObjectList::iterator	iter;
					for( iter = s_lstDynaObjects.begin(); iter != s_lstDynaObjects.end(); ++iter )
					{
						if( !_stricmp( (*iter).m_sName.c_str(), pTargetName ))
						{
							pTargetClass = const_cast<char*>((*iter).m_sClassName.c_str());
							break;
						}
					}

					// See if the object is a body of an AI...

					if( !pTargetClass && strstr( pTargetName, "_body" ))
					{
						char szName[CMDMGR_MAX_COMMAND_LENGTH] = {0};
						LTStrCpy( szName, pTargetName, ARRAY_LEN( szName ));

						const char* pAI = strtok( szName, "_body" );

						if( pAI )
						{
							// Make sure the AI exists...
							
							if( pInterface->FindObject( pAI ) == LT_OK )
							{
								pTargetClass = "Body";
							}
							
							// See if it is one of our dynamic objects...

							if( !pTargetClass )
							{
								DynamicObjectList::iterator	iter;
								for( iter = s_lstDynaObjects.begin(); iter != s_lstDynaObjects.end(); ++iter )
								{
									if( !_stricmp( (*iter).m_sName.c_str(), pAI ))
									{
										pTargetClass = "Body";
										break;
									}
								}
							}
						}	
					}

					// Special case "player" since it is not in the world...
					// and objects inside prefabs (ie #.)

					if( !pTargetClass && (!_stricmp( pTargetName, "player" ) ||
										  !_stricmp( pTargetName, "activeplayer" ) ||
										  !_stricmp( pTargetName, "otherplayers" ) ))
					{
						pTargetClass = "CPlayerObj";
					}

					if( !pTargetClass )
					{
						if( s_bShowMsgErrors )
						{
							pInterface->ShowDebugWindow( LTTRUE );
							pInterface->CPrint( "ERROR! - PreCheckMsg()" );
							pInterface->CPrint( "    MSG - Could not find object '%s'!", cpObjNames.m_Args[i] );
						}

						return LTFALSE;
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
				if( gProp.m_Bool )
				{
					if( s_bShowMsgErrors )
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "ERROR! - PreCheckMsg()" );
						pInterface->CPrint( "    MSG - Template objects should not be sent messages since they don't exist in the game!" );
					}

					return LTFALSE;
				}
			}

				
			LTBOOL		bFoundClassType = LTFALSE;
			LTBOOL		bFoundClassDesc = LTFALSE;
			LTBOOL		bFoundClassMsgs = LTFALSE;
			LTBOOL		bFoundMsg = LTFALSE;

			while( szClassType )
			{
				bFoundClassType = LTTRUE;
								
				CMDMGR_CLASS_DESC	*pClassDesc = GetClassDesc( szClassType );
				if( !pClassDesc )
				{
					// We have no class description record for this class type.					
					break;
				}
				bFoundClassDesc = LTTRUE;

				// See if we should even bother checking this class for messages...
				
				if( pClassDesc->m_dwFlags & CMDMGR_CF_MSGIGNORE )
				{
					// Pretend we found a valid message and move on to the next object...

					bFoundClassMsgs = LTTRUE;
					bFoundMsg = LTTRUE;

					break;
				}
				
				MSG_PRECHECK		*pClassMsgs = pClassDesc->m_pMsgs;
				if( !pClassMsgs || pClassDesc->m_nNumMsgs <= 1 )
				{
					// We have no messages registered for this class type, but we might for it's parent.

					szClassType = pClassDesc->m_szParentClass;
					continue; 
				}
				bFoundClassMsgs = LTTRUE;


				ConParse cpMsgParams;
				cpMsgParams.Init( pMsg );
				
				if( pInterface->Parse( &cpMsgParams ) == LT_OK )
				{
					// Check the class descriptions list of registered messages...
					// Skip the very first one, [0], because it's a bogus message.

					for( int nMsg = 1; nMsg < pClassDesc->m_nNumMsgs; ++nMsg )
					{
						// Look for the message name or if it's a special message always process it...
						
						if( (!_stricmp( cpMsgParams.m_Args[0], pClassMsgs[nMsg].m_szMsgName )) || pClassMsgs[nMsg].m_bSpecial )
						{
							// Ok! The object can process the msg, try and validate it...

							// Negative number of arguments means a varying number or optional arguments
							if( pClassMsgs[nMsg].m_nMinArgs < 0 || 
									((cpMsgParams.m_nArgs >= pClassMsgs[nMsg].m_nMinArgs) && (cpMsgParams.m_nArgs <= pClassMsgs[nMsg].m_nMaxArgs)))
							{
								bFoundMsg = LTTRUE;
								if( pClassMsgs[nMsg].m_pValidateFn )
								{
									if( !pClassMsgs[nMsg].m_pValidateFn( pInterface, cpMsgParams ))
									{
										// A false return by a special message means it was not handled and we should continue to try and validate it...

										if( pClassMsgs[nMsg].m_bSpecial )
										{
											bFoundMsg = LTFALSE;
										}
										else
										{
											// The message is invalid...

											return LTFALSE;
										}
									}
									else
									{
										// The message was validated and all is well...

										return LTTRUE;
									}
								}
								else
								{
									// The object can recieve the message and there is no reason to validate it...

									return LTTRUE;
								}
							}
							else
							{
								if( s_bShowMsgErrors )
								{
									pInterface->ShowDebugWindow( LTTRUE );
									pInterface->CPrint( "ERROR! - PreCheckMsg()" );
									pInterface->CPrint( "    MSG - '%s' message had %i arguments instead of the %i minimum and %i maximum required.", pClassMsgs[nMsg].m_szMsgName, cpMsgParams.m_nArgs, pClassMsgs[nMsg].m_nMinArgs, pClassMsgs[nMsg].m_nMaxArgs );
									
									for( int nArg = 0; nArg < cpMsgParams.m_nArgs; ++nArg )
									{
										pInterface->CPrint( "    Arg[%i] = '%s'", nArg, cpMsgParams.m_Args[nArg] );
									}

									pInterface->CPrint( "  Syntax for '%s' message is: %s", pClassMsgs[nMsg].m_szMsgName, pClassMsgs[nMsg].m_szSyntax );
								}

								return LTFALSE;
							}
						}
					}
					
					if( bFoundMsg )
					{
						// Msg is good so don't bother going up the hierarchy, just go to next object.
						break;
					}

				}

				// See if the parent class can handel the message

				szClassType = pClassDesc->m_szParentClass;
			}

			if( pTargetClass && (!bFoundClassType || !bFoundClassDesc) )
			{
				if( s_bShowMsgErrors )
				{
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Could not find class type <%s>.", pTargetClass );
				}

				return LTFALSE;
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
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - PreCheckMsg()" );
					pInterface->CPrint( "    MSG - Message '%s' is not a valid message for object '%s'!", pMsg, cpObjNames.m_Args[i], szClassType );
					ListObjectMsgs( pInterface, cpObjNames.m_Args[i] );
				}

				return LTFALSE;
			}
		}
	}
	else
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint("ERROR! - PreCheckMsg()");
			pInterface->CPrint("    MSG - Could not parse object name(s) '%s'!", pObjectNames);
		}

        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRand
//
//  PURPOSE:	Check the rand commands for validity
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckRand( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	if( !pInterface ) return LTFALSE;

	LTFLOAT fPercent = (LTFLOAT) atof(parse.m_Args[1]);
	char* pCmd1		= parse.m_Args[2];
	char* pCmd2		= parse.m_Args[3];

    // Make sure percent value is good...

	if( fPercent < 0.001f || fPercent > 1.0f )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckRand()" );
		pInterface->CPrint( "    RAND - Percent is %.3f. Should be between 0.001 and 1.0", fPercent );
		return LTFALSE;
	}
		
	// Make sure command1 is good...

	if( !IsValidCmd( pInterface, pCmd1 ) )
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckRand()" );
			pInterface->CPrint( "    RAND - command1 '%s' is invalid.", pCmd1 );
		}
		
		return LTFALSE;
	}

	// Make sure command2 is good...

	if( !IsValidCmd( pInterface, pCmd2 ) )
	{
		if( s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckRand()" );
			pInterface->CPrint( "    RAND - command2 '%s' is invalid.", pCmd2 );
		}
		
		return LTFALSE;
	}

	// Looks good to me
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRandArgs
//
//  PURPOSE:	Check all the rand commands for validity
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckRandArgs( ILTPreInterface *pInterface, ConParse &parse, int nNumArgs )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	if( !pInterface ) return LTFALSE;

	for( int i = 1; i <= nNumArgs; ++i )
	{
		if( !IsValidCmd( pInterface, parse.m_Args[i] ))
		{
			if( s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckRandArgs()" );
				pInterface->CPrint( "    RAND%i - command%i '%s' is invalid.", nNumArgs, i, parse.m_Args[i] );
			}
			
			return LTFALSE;
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRepeat
//
//  PURPOSE:	Check the Repeat command and the command it is repeating
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckRepeat( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;
	
	if( !pInterface ) return LTFALSE;

	CMD_STRUCT_PARAM cmd;
	cmd.nMinTimes	= (int) atol(parse.m_Args[1]);
	cmd.nMaxTimes	= (int) atol(parse.m_Args[2]);
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[3]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[4]);
	cmd.pCmd		= parse.m_Args[5];

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckRepeat()" );
		pInterface->CPrint( "    REPEAT - command '%s' is invalid.", cmd.pCmd );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckRepeatId
//
//  PURPOSE:	Check the RepeatId command and the command it is repeating
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckRepeatId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	CMD_STRUCT_PARAM cmd;
	cmd.pId			= parse.m_Args[1];
	cmd.nMinTimes	= (int) atol(parse.m_Args[2]);
	cmd.nMaxTimes	= (int) atol(parse.m_Args[3]);
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[4]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[5]);
	cmd.pCmd		= parse.m_Args[6];

	if( !cmd.pId || cmd.pId[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckRepeatId()" );
		pInterface->CPrint( "    REPEATID - Invalid command Id!" );
		return LTFALSE;
	}

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckRepeatId()" );
		pInterface->CPrint( "    REPEATID - command '%s' is invalid.", cmd.pCmd );
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckDelay
//
//  PURPOSE:	Check the Delay command and the command it is delaying
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckDelay( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	CMD_STRUCT_PARAM cmd;
    cmd.fDelay  = (LTFLOAT) atof(parse.m_Args[1]);
	cmd.pCmd	= parse.m_Args[2];

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckDelay()" );
		pInterface->CPrint( "    DELAY - command '%s' is invalid.", cmd.pCmd );
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckDelayId
//
//  PURPOSE:	Check the DelayId command and the command it is delaying
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckDelayId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	CMD_STRUCT_PARAM cmd;
	cmd.pId		= parse.m_Args[1];
    cmd.fDelay  = (LTFLOAT) atof(parse.m_Args[2]);
	cmd.pCmd	= parse.m_Args[3];

	if( !cmd.pId || cmd.pId[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckDelayId()" );
		pInterface->CPrint( "    DELAYID - Invalid command Id!" );
		return LTFALSE;
	}

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckDelayId()" );
		pInterface->CPrint( "    DELAYID - command '%s' is invalid.", cmd.pCmd );
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckLoop
//
//  PURPOSE:	Check the Loop command and the command it is looping
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckLoop( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;
	
	CMD_STRUCT_PARAM cmd;
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[1]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[2]);
	cmd.pCmd		= parse.m_Args[3];

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckLoop()" );
		pInterface->CPrint( "    LOOP - command '%s' is invalid.", cmd.pCmd );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckLoopId
//
//  PURPOSE:	Check the LoopId command and the command it is looping
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckLoopId( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;
	
	CMD_STRUCT_PARAM cmd;
	cmd.pId			= parse.m_Args[1];
    cmd.fMinDelay   = (LTFLOAT) atof(parse.m_Args[2]);
    cmd.fMaxDelay   = (LTFLOAT) atof(parse.m_Args[3]);
	cmd.pCmd		= parse.m_Args[4];

	if( !cmd.pId || cmd.pId[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckLoopId()" );
		pInterface->CPrint( "    LOOPID - Invalid command Id!" );
		return LTFALSE;
	}

	if( IsValidCmd( pInterface, cmd.pCmd ) )
	{
		return CheckDelayedCmd( pInterface, cmd );
	}

	if( s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckLoopId()" );
		pInterface->CPrint( "    LOOPID - command '%s' is invalid.", cmd.pCmd );
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckAbort
//
//  PURPOSE:	Check the Abort command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckAbort( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateNonVarCmds )
		return LTTRUE;

	char* pId = parse.m_Args[1];

	if( !pId || pId[0] == CMDMGR_NULL_CHAR )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckAbort()" );
		pInterface->CPrint( "    ABORT - Invalid command Id!" );
		return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::DevelopVars
//
//  PURPOSE:	Develop the vars at the beginig of every PreCheck that deals
//				with vars so we can edit the commands file while we edit the level
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::DevelopVars( ILTPreInterface *pInterface )
{
	// Re-parse the commands file in case it changed...

	s_CommandButeMgr.Pre_Reload( pInterface );

	bool bNewWorld = true;
	if( pInterface->GetWorldName() )
	{
		if( !_stricmp( s_szLastWorld, pInterface->GetWorldName() ))
		{
			bNewWorld = false;
		}
	}

	// Check to see if we are being forced to develop our variables...

	bool bForce = (s_bClearVarsRequested && s_bCanClearVars);
	
	// If it didn't change, no since in reading in the commands again.

	if( !s_CommandButeMgr.FileChanged() && !s_bFileLoadError && !bNewWorld && !bForce )
		return LTTRUE;

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

	s_bFileLoadError	= LTFALSE;
	s_bValidateVarCmds	= true;
	s_bValidateVarDecs	= true;

	// Check all the global commands first...

	if( !s_CommandButeMgr.Pre_CheckGlobalCmds( pInterface, this, "INT" ) )
		s_bFileLoadError = LTTRUE;


	if( !s_CommandButeMgr.Pre_CheckLevelCmds( pInterface, this ))
		s_bFileLoadError = LTTRUE;

	if( s_bFileLoadError && pInterface->GetWorldName() )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - DevelopVars()" );
		pInterface->CPrint( "    Could not build variable list because of error in commands.txt!");

		return LTFALSE;
	}

	return LTTRUE;
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
		if( !_stricmp( s_aVars[i].m_szName, pVarName ))
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

LTBOOL CCommandMgrPlugin::PreCheckInt( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || !s_bValidateVarDecs )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;
	
	if( s_bFileLoadError )
		return LTTRUE;

	if( s_nNumVars >= CMDMGR_MAX_VARS )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckInt()" );
		pInterface->CPrint( "    INT - Max (%i) number of variables reached!", s_nNumVars );
		
		return LTFALSE;
	}

	const char *pVarName = parse.m_Args[1];
	
	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || (pVarName[0] >= '0' && pVarName[0] <= '9') )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckInt()" );
			pInterface->CPrint( "    INT - Invalid Variable name '%s'!", (pVarName ? pVarName : "NULL") );
		}

		return LTFALSE;
	}

	// Make sure we don't already have a variable with this name...

	if( FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckInt()" );
			pInterface->CPrint( "    INT - Variable '%s' already declared!", pVarName );
		}
		
		return LTFALSE;
	}

	// Just store the name to check for existance ...

	SAFE_STRCPY( s_aVars[s_nNumVars].m_szName, pVarName );
	s_aVars[s_nNumVars].m_eType = eCMVar_Int;
	++s_nNumVars;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckObj
//
//  PURPOSE:	Check the Obj command...
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckObj( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bValidateVarDecs )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	if( s_nNumVars >= CMDMGR_MAX_VARS )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - PreCheckObj()" );
		pInterface->CPrint( "    OBJ - Max (%i) number of variables reached!", s_nNumVars );
		
		return LTFALSE;
	}

	const char *pVarName = parse.m_Args[1];
	
	if( !pVarName || pVarName[0] == CMDMGR_NULL_CHAR || (pVarName[0] >= '0' && pVarName[0] <= '9') )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckObj()" );
			pInterface->CPrint( "    OBJ - Invalid Variable name '%s'!", (pVarName ? pVarName : "NULL") );
		}

		return LTFALSE;
	}

	// Make sure we don't already have a variable with this name...

	if( FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckObj()" );
			pInterface->CPrint( "    OBJ - Variable '%s' already declared!", pVarName );
		}
		
		return LTFALSE;
	}

	// Just store the name to check for existance ...

	SAFE_STRCPY( s_aVars[s_nNumVars].m_szName, pVarName );
	s_aVars[s_nNumVars].m_eType = eCMVar_Obj;
	++s_nNumVars;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckSet
//
//  PURPOSE:	Check the Set command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckSet( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return LTTRUE;
	
	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSet()" );
			pInterface->CPrint( "    SET - Invalid variable or value!" );
		}

		return LTFALSE;
	}

	// Make sure we have the variable 
	
	VAR_STRUCT *pDestVar = FindVar( pVarName );
	if( !pDestVar )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSet()" );
			pInterface->CPrint( "    SET - Unknown variable '%s'!", pVarName );
		}

		return LTFALSE;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return LTTRUE;
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
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckSet()" );
				pInterface->CPrint( pInvalidDesc, pVarValue );
			}
			
			return LTFALSE;
		}
	}

	return LTTRUE;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckAdd
//
//  PURPOSE:	Check the Add command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckAdd( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckAdd()" );
			pInterface->CPrint( "    ADD - Invalid variable or value!" );
		}

		return LTFALSE;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckAdd()" );
			pInterface->CPrint( "    ADD - Unknown variable '%s'!", pVarName );
		}

		return LTFALSE;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return LTTRUE;
	}
	else
	{
		if( !FindVar( pVarValue ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckAdd()" );
				pInterface->CPrint( "    ADD - Unknown variable '%s'!", pVarValue );
			}
			
			return LTFALSE;
		}
	}

	return LTTRUE;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckSub
//
//  PURPOSE:	Check the Subtract command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckSub( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}
	
	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SUB - Invalid variable or value!" );
		}

		return LTFALSE;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SUB - Unknown variable '%s'!", pVarName );
		}

		return LTFALSE;
	}

	// Is the value a number or another variable...

	if( (pVarValue[0] >= '0') && (pVarValue[0] <= '9') )
	{
		return LTTRUE;
	}
	else
	{
		if( !FindVar( pVarValue ))
		{
			if( s_bShowVarErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckSub()" );
				pInterface->CPrint( "    SUB - Unknown variable '%s'!", pVarValue );
			}

			return LTFALSE;
		}
	}

	return LTTRUE;	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckShowVar
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckShowVar( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pVarName = parse.m_Args[1];
	const char *pVarValue = parse.m_Args[2];

	if( !pVarName || !pVarValue )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SHOWVAR - Invalid variable or value!" );
		}

		return LTFALSE;
	}

	// Make sure we have the variable 

	if( !FindVar( pVarName ))
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckSub()" );
			pInterface->CPrint( "    SHOWVAR - Unknown variable '%s'!", pVarName );
		}
		
		return LTFALSE;
	}

	return LTTRUE;	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckIf
//
//  PURPOSE:	Check the If command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckIf( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pExpression = parse.m_Args[1];
	const char *pThen		= parse.m_Args[2];
	const char *pCmds		= parse.m_Args[3];

	if( !pExpression || !pThen || !pCmds )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckIf()" );
			pInterface->CPrint( "    IF - Invalid expression or command!" );
		}

		return LTFALSE;
	}
	
	// Check for the 'THEN'...

	if( stricmp( pThen, "THEN" ) != 0 )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckIf()" );
			pInterface->CPrint( "    IF - Invalid syntax! Need 'THEN' after expressions!" );
		}

		return LTFALSE;
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
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckIf()" );
				pInterface->CPrint( "    IF - Invalid expression '%s'!", pExpression );
			}
			
			return LTFALSE;
		}
	}

	// Cheat by adding parens around the commands if there is only one so it will still parse correctly...

	char szCommands[256] = {0};
	
	if( pCmds[0] != '(' )
	{
		sprintf( szCommands, "(%s)", pCmds );
	}
	else
	{
		SAFE_STRCPY( szCommands, pCmds );
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
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckIf()" );
				pInterface->CPrint( "    IF - No commands are listed! You need at least one command!" );
			}

			return LTFALSE;
		}

		if( cpCommands.m_nArgs == 1 )
		{
			// There was only one command.  Is it valid?...
			
			if( !IsValidCmd( pInterface, cpCommands.m_Args[0] ))
			{
				if( s_bShowVarErrors )
				{	
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - PreCheckIf()" );
					pInterface->CPrint( "    IF - Command '%s' is invalid!", cpCommands.m_Args[0] );
				}

				return LTFALSE;
			}
		}
		else
		{
			// We have more than one command.  Loop over the commands and check for validity...

			for( int i = 0; i < cpCommands.m_nArgs; ++i )
			{
				if( !IsValidCmd( pInterface, cpCommands.m_Args[i] ) )
				{
					if( s_bShowVarErrors )
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "ERROR! - PreCheckIf()" );
						pInterface->CPrint( "    IF - Command '%s' is invalid!", cpCommands.m_Args[i] );
					}

					return LTFALSE;
				}
			}
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::PreCheckWhen
//
//  PURPOSE:	Check the On command
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::PreCheckWhen( ILTPreInterface *pInterface, ConParse &parse )
{
	if( !s_bValidateVarCmds || s_bVarDeclerationsOnly )
		return LTTRUE;

	if( !s_bShowVarErrors )
	{
		// Don't display any info on the property that is being checked if we are not supposed to show var errors

		s_bDisplayPropInfo = false;
	}

	// Build the list of vars first...

	if( !DevelopVars( pInterface ))
		return LTFALSE;

	if( s_bFileLoadError ) return LTTRUE;

	const char *pExpression = parse.m_Args[1];
	const char *pThen		= parse.m_Args[2];
	const char *pCmds		= parse.m_Args[3];

	if( !pExpression || !pThen || !pCmds )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckWhen()" );
			pInterface->CPrint( "    ON - Invalid expression or command!" );
		}

		return LTFALSE;
	}
	
	// Check for the 'THEN'...

	if( stricmp( pThen, "THEN" ) != 0 )
	{
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - PreCheckWhen()" );
			pInterface->CPrint( "    ON - Invalid syntax! Need 'THEN' after expressions!" );
		}

		return LTFALSE;
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
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckWhen()" );
				pInterface->CPrint( "    ON - Invalid expression '%s'!", pExpression );
			}
			
			return LTFALSE;
		}
	}

	// Cheat by adding parens around the commands if there is only one so it will still parse correctly...

	char szCommands[256] = {0};
	
	if( pCmds[0] != '(' )
	{
		sprintf( szCommands, "(%s)", pCmds );
	}
	else
	{
		SAFE_STRCPY( szCommands, pCmds );
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
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - PreCheckWhen()" );
				pInterface->CPrint( "    ON - No commands are listed! You need at least one command!" );
			}

			return LTFALSE;
		}

		if( cpCommands.m_nArgs == 1 )
		{
			// There was only one command.  Is it valid?...
			
			if( !IsValidCmd( pInterface, cpCommands.m_Args[0] ))
			{
				if( s_bShowVarErrors )
				{
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - PreCheckWhen()" );
					pInterface->CPrint( "    ON - Command '%s' is invalid!", cpCommands.m_Args[0] );
				}

				return LTFALSE;
			}
		}
		else
		{
			// We have more than one command.  Loop over the commands and check for validity...

			for( int i = 0; i < cpCommands.m_nArgs; ++i )
			{
				if( !IsValidCmd( pInterface, cpCommands.m_Args[i] ) )
				{
					if( s_bShowVarErrors )
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "ERROR! - PreCheckWhen()" );
						pInterface->CPrint( "    ON - Command '%s' is invalid!", cpCommands.m_Args[i] );
					}
					
					return LTFALSE;
				}
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::IsValidExpression
//
//  PURPOSE:	Check the expressions...
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::IsValidExpression( ILTPreInterface *pInterface, ConParse &cpExpression )
{
	if( !s_bValidateVarCmds )
		return LTTRUE;

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
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - IsValidExpression()" );
			pInterface->CPrint( "    Expression had %i arguments instead of the 3 required!", cpExpression.m_nArgs );

			for( int i = 0; i < cpExpression.m_nArgs; ++i )
			{
				pInterface->CPrint("  Arg[%i] = '%s'", i, cpExpression.m_Args[i]);
			}
		}

		return LTFALSE;
	}

	const char *pArg1	= cpExpression.m_Args[0];
	const char *pOp		= cpExpression.m_Args[1];
	const char *pArg2	= cpExpression.m_Args[2];

	if( !pArg1 || !pOp || !pArg2 )
	{	
		if( s_bShowVarErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - IsValidExpression()" );
			pInterface->CPrint( "    Invalid expression '%s %s %s!", (pArg1 ? pArg1 : "NULL"), (pOp ? pOp : "NULL"), (pArg2 ? pArg2 : "NULL") );
		}
		
		return LTFALSE;
	}

	// Find the operator...

	for( int iOp = 0; iOp < c_NumOperators; ++iOp )
	{
		if( !_stricmp( s_Operators[iOp].m_OpName, pOp ))
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
						return LTFALSE;

					ConParse cpArg2;
					cpArg2.Init( pArg2 );

					if( pInterface->Parse( &cpArg2 ) == LT_OK )
					{
						if( !IsValidExpression( pInterface, cpArg2 ))
							return LTFALSE;

						return LTTRUE;
					}
				}

				if( s_bShowVarErrors )
				{	
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - IsValidExpression()" );
					pInterface->CPrint( "    Invalid expression '%s %s %s!", (pArg1 ? pArg1 : "NULL"), (pOp ? pOp : "NULL"), (pArg2 ? pArg2 : "NULL") );
				}
				
				return LTFALSE;
			}

			// It's not a logical, therefore the first arg must be a variable...

			if( !FindVar( pArg1 ))
			{
				if( s_bShowVarErrors )
				{
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - IsValidExpression()" );
					pInterface->CPrint( "    Unknown variable '%s'!", pArg1 );
				}
				
				return LTFALSE;
			}

			// The second arg may either be a variable or a number...

			if( (pArg2[0] >= '0') && (pArg2[0] <= '9') )
			{
				return LTTRUE;
			}
			else
			{	
				if( !FindVar( pArg2 ))
				{
					if( s_bShowVarErrors )
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "ERROR! - IsValidExpression()" );
						pInterface->CPrint( "    Unknown variable '%s'!", pArg2 );
					}

					return LTFALSE;
				}
			}

			return LTTRUE;
		}

	}

	if( s_bShowVarErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - IsValidExpression()" );
		pInterface->CPrint( "    Unknown operator '%s'!", pOp );
	}
	
	return LTFALSE;
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
		if( !_stricmp( (*iter).m_sName.c_str(), DynaObj.m_sName.c_str() ))
		{
			return;	
		}

		++iter;
	}

	s_lstDynaObjects.push_back( DynaObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandMgrPlugin::CommandExists
//
//  PURPOSE:	Looks at the static table of commands to see if the given string is a command...
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgrPlugin::CommandExists( const char *pCmd )
{
	if( !pCmd )
		return LTFALSE;

	for( int i=0; i < c_nNumValidCmds; ++i )
	{
		if( !_stricmp( pCmd, s_ValidCmds[i].pCmdName ))
		{
			return LTTRUE;			
		}
	}

	return LTFALSE;
}