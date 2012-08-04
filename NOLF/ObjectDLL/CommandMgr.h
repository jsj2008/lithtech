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

class ConParse;
class CCommandMgr;
extern CCommandMgr* g_pCmdMgr;

#define CMDMGR_MAX_PENDING_COMMANDS 100
#define CMDMGR_MAX_COMMAND_LENGTH	256
#define CMDMGR_MAX_ID_LENGTH		16
#define CMDMGR_NULL_CHAR			'\0'

typedef LTBOOL (*ProcessCmdFn)(CCommandMgr *pCmdMgr, ConParse & parse, int nCmdIndex);


// This struct holds all the information pending commands...

struct CMD_STRUCT
{
	CMD_STRUCT()
	{
		Clear();
	}

	void Clear()
	{
		fDelay		= fMinDelay = fMaxDelay = 0.0f;
		nNumTimes	= nMinTimes	= nMaxTimes	= -1;
		aCmd[0]		= CMDMGR_NULL_CHAR;
		aId[0]		= CMDMGR_NULL_CHAR;
	}

	void Load(HMESSAGEREAD hRead);
	void Save(HMESSAGEWRITE hWrite);

	float	fDelay;
	float	fMinDelay;
	float	fMaxDelay;
	int		nNumTimes;
	int		nMinTimes;
	int		nMaxTimes;
	char	aCmd[CMDMGR_MAX_COMMAND_LENGTH];
	char	aId[CMDMGR_MAX_ID_LENGTH];
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
    CMD_PROCESS_STRUCT(char* pCmd="", int nArgs=0, ProcessCmdFn pFn=LTNULL, char* pSyn="")
	{
		pCmdName	= pCmd;
		nNumArgs	= nArgs;
		pProcessFn	= pFn;
		pSyntax		= pSyn;
	}

	char*			pCmdName;
	char*			pSyntax;
	int				nNumArgs;
	ProcessCmdFn	pProcessFn;
};

class CCommandMgr
{
	public :

		CCommandMgr();
		~CCommandMgr();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void	ListCommands();
        LTBOOL  IsValidCmd(const char* pCmd);
        LTBOOL  Process(const char* pCmd) { return ProcessCmd(pCmd); }
        LTBOOL  Update();

		void	Clear();

		// The following methods should only be called via the static cmdmgr_XXX
		// functions...

        LTBOOL	ProcessListCommands(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessDelay(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessDelayId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessMsg(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessRand(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessRandArgs(ConParse & parse, int nCmdIndex, int nNumArgs);
        LTBOOL  ProcessRepeat(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessRepeatId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessLoop(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessLoopId(ConParse & parse, int nCmdIndex);
        LTBOOL  ProcessAbort(ConParse & parse, int nCmdIndex);

	private :

        LTBOOL	ProcessCmd(const char* pCmd, int nCmdIndex=-1);

        LTBOOL  AddDelayedCmd(CMD_STRUCT_PARAM & cmd, int nCmdIndex);
        LTBOOL  CheckArgs(ConParse & parse, int nNum);
		void	DevPrint(char *msg, ...);

	private :

		CMD_STRUCT	m_PendingCmds[CMDMGR_MAX_PENDING_COMMANDS];
};

inline void	CCommandMgr::Clear()
{
	// Clear all our commands...

	for (int i=0; i < CMDMGR_MAX_PENDING_COMMANDS; i++)
	{
		m_PendingCmds[i].Clear();
	}
}

#endif // __COMMAND_MGR_H__