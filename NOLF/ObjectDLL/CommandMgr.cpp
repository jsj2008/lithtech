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

CCommandMgr* g_pCmdMgr = LTNULL;


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
	if (pCmdMgr) return pCmdMgr->ProcessMsg(parse, nCmdIndex);
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

static CMD_PROCESS_STRUCT s_ValidCmds[] =
{
	CMD_PROCESS_STRUCT("LISTCOMMANDS", 1, cmdmgr_ListCommands, "Available Object Commands:"),
	CMD_PROCESS_STRUCT("MSG", 3, cmdmgr_Msg, "  MSG <object name(s)> <object msg>"),
	CMD_PROCESS_STRUCT("RAND", 4, cmdmgr_Rand, "  RAND <cmd1 percent> <cmd1> <cmd2>"),
	CMD_PROCESS_STRUCT("RAND2", 3, cmdmgr_Rand2, "  RAND2 <cmd1> <cmd2>"),
	CMD_PROCESS_STRUCT("RAND3", 4, cmdmgr_Rand3, "  RAND3 <cmd1> <cmd2> <cmd3>"),
	CMD_PROCESS_STRUCT("RAND4", 5, cmdmgr_Rand4, "  RAND4 <cmd1> <cmd2> <cmd3> <cmd4>"),
	CMD_PROCESS_STRUCT("RAND5", 6, cmdmgr_Rand5, "  RAND5 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5>"),
	CMD_PROCESS_STRUCT("RAND6", 7, cmdmgr_Rand6, "  RAND6 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6>"),
	CMD_PROCESS_STRUCT("RAND7", 8, cmdmgr_Rand7, "  RAND7 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6> <cmd7>"),
	CMD_PROCESS_STRUCT("RAND8", 9, cmdmgr_Rand8, "  RAND8 <cmd1> <cmd2> <cmd3> <cmd4> <cmd5> <cmd6> <cmd7> <cmd8>"),
	CMD_PROCESS_STRUCT("REPEAT", 6, cmdmgr_Repeat, "  REPEAT <min times> <max times> <min delay> <max delay> <cmd>"),
	CMD_PROCESS_STRUCT("REPEATID", 7, cmdmgr_RepeatId, "  REPEATID <cmd id> <min times> <max times> <min delay> <max delay> <cmd>"),
	CMD_PROCESS_STRUCT("DELAY", 3, cmdmgr_Delay, "  DELAY <time> <cmd>"),
	CMD_PROCESS_STRUCT("DELAYID", 4, cmdmgr_DelayId, "  DELAYID <cmd id> <time> <cmd>"),
	CMD_PROCESS_STRUCT("LOOP", 4, cmdmgr_Loop, "  LOOP <min delay> <max delay> <cmd>"),
	CMD_PROCESS_STRUCT("LOOPID", 5, cmdmgr_LoopId, "  LOOPID <cmd id> <min delay> <max delay> <cmd>"),
	CMD_PROCESS_STRUCT("ABORT", 2, cmdmgr_Abort, "  ABORT <cmd id>")
};

const int c_nNumValidCmds = sizeof(s_ValidCmds)/sizeof(s_ValidCmds[0]);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::CCommandMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCommandMgr::CCommandMgr()
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
				ProcessCmd(m_PendingCmds[i].aCmd, i);

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

    while (g_pLTServer->Common()->Parse(&parse) == LT_OK)
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

	return AddDelayedCmd(cmd, nCmdIndex);
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

	return AddDelayedCmd(cmd, nCmdIndex);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::ProcessMsg()
//
//	PURPOSE:	Process the Msg command (send the message to the
//				specified object(s))
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandMgr::ProcessMsg(ConParse & parse, int nCmdIndex)
{
	char* pObjectNames	= parse.m_Args[1];
	char* pMsg			= parse.m_Args[2];

    if (!pObjectNames || !pMsg) return LTFALSE;

	ConParse parse2;
	parse2.Init(pObjectNames);

    if (g_pLTServer->Common()->Parse(&parse2) == LT_OK)
	{
		for (int i=0; i < parse2.m_nArgs; i++)
		{
            SendTriggerMsgToObjects(LTNULL, parse2.m_Args[i], pMsg);
		}
	}
	else
	{
		DevPrint("CCommandMgr::ProcessMsg() ERROR!");
		DevPrint("Could not parse object name(s) '%s'!", pObjectNames);
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
		return ProcessCmd(pCmd1, nCmdIndex);
	}

	return ProcessCmd(pCmd2, nCmdIndex);
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

	return ProcessCmd(parse.m_Args[nCmd], nCmdIndex);
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

	return AddDelayedCmd(cmd, nCmdIndex);
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

	return AddDelayedCmd(cmd, nCmdIndex);
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

	return AddDelayedCmd(cmd, nCmdIndex);
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

	return AddDelayedCmd(cmd, nCmdIndex);
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
		DevPrint("Invalid Command Id!");
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
	DevPrint("Could not find command associated with id '%s'!", pId);
    return LTFALSE;
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
		DevPrint("Pending command buffer overflow!");
		DevPrint("Could not delay command '%s'!", cmd.pCmd);
		DevPrint("Processing command now!");

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

    if (g_pLTServer->Common()->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			for (int i=0; i < c_nNumValidCmds; i++)
			{
				if (_stricmp(parse.m_Args[0], s_ValidCmds[i].pCmdName) == 0)
				{
					return CheckArgs(parse, s_ValidCmds[i].nNumArgs);
				}
			}
		}
	}

    return LTFALSE;
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
		DevPrint("%s command had %d arguments instead of the %d required!",
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

void CCommandMgr::Save(HMESSAGEWRITE hWrite)
{
	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Save(hWrite);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandMgr::Load()
//
//	PURPOSE:	Load the command mgr
//
// ----------------------------------------------------------------------- //

void CCommandMgr::Load(HMESSAGEREAD hRead)
{
	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Load(hRead);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMD_STRUCT::Load()
//
//	PURPOSE:	Load all the info associated with the CMD_STRUCT
//
// ----------------------------------------------------------------------- //

void CMD_STRUCT::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    fDelay      = g_pLTServer->ReadFromMessageFloat(hRead);
    fMinDelay   = g_pLTServer->ReadFromMessageFloat(hRead);
    fMaxDelay   = g_pLTServer->ReadFromMessageFloat(hRead);
    nNumTimes   = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    nMinTimes   = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    nMaxTimes   = (int) g_pLTServer->ReadFromMessageFloat(hRead);

    HSTRING hstr = g_pLTServer->ReadFromMessageHString(hRead);
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

    hstr = g_pLTServer->ReadFromMessageHString(hRead);
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

void CMD_STRUCT::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, fDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, fMinDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, fMaxDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)nNumTimes);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)nMinTimes);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)nMaxTimes);

    HSTRING hstr = LTNULL;
	if (aCmd[0] != CMDMGR_NULL_CHAR)
	{
        hstr = g_pLTServer->CreateString(aCmd);
	}

    g_pLTServer->WriteToMessageHString(hWrite, hstr);

	if (hstr)
	{
        g_pLTServer->FreeString(hstr);
	}

    hstr = LTNULL;
	if (aId[0] != CMDMGR_NULL_CHAR)
	{
        hstr = g_pLTServer->CreateString(aId);
	}

    g_pLTServer->WriteToMessageHString(hWrite, hstr);

	if (hstr)
	{
        g_pLTServer->FreeString(hstr);
	}
}