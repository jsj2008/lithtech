// ----------------------------------------------------------------------- //
//
// MODULE  : ObjEditMgr.cpp
//
// PURPOSE : Handle client-side editing of in-game objects
//
// CREATED : 3/12/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjEditMgr.h"
#include "MessageMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "VKDefs.h"

extern CCheatMgr*		g_pCheatMgr;
extern CGameClientShell* g_pGameClientShell;

CObjEditMgr* g_pEditMgr = LTNULL;

void ListCommandsFn(int argc, char **argv)
{
	if (g_pEditMgr)
	{
		g_pEditMgr->HandleListCommands(argc, argv);
	}
}

void CmdFn(int argc, char **argv)
{
	if (argc < 2)
	{
        g_pLTClient->CPrint("Cmd <command>");
		return;
	}

	if (g_pEditMgr)
	{
		g_pEditMgr->HandleCommand(argc, argv);
	}
}

void TriggerFn(int argc, char **argv)
{
	if (argc < 2)
	{
        g_pLTClient->CPrint("Trigger <objectname> <message>");
		return;
	}

	if (g_pEditMgr)
	{
		g_pEditMgr->HandleTrigger(argc, argv);
	}
}

void EditFn(int argc, char **argv)
{
	if (argc < 2)
	{
        g_pLTClient->CPrint("Edit <propertyname> <value>");
		return;
	}

	if (g_pEditMgr)
	{
		g_pEditMgr->HandleEdit(argc, argv);
	}
}

void SelectFn(int argc, char **argv)
{
	if (argc < 1)
	{
        g_pLTClient->CPrint("Select <objectname>");
		return;
	}

	if (g_pEditMgr)
	{
		g_pEditMgr->HandleSelect(argc, argv);
	}
}

void ListFn(int argc, char **argv)
{
	if (argc < 1)
	{
        g_pLTClient->CPrint("List <classtype>");
		return;
	}

	if (g_pEditMgr)
	{
		g_pEditMgr->HandleList(argc, argv);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::CObjEditMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CObjEditMgr::CObjEditMgr()
{
	g_pEditMgr = this;

    m_bEditMode = LTFALSE;
    m_hstrCurEditObject = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::~CObjEditMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CObjEditMgr::~CObjEditMgr()
{
	if (m_hstrCurEditObject)
	{
        g_pLTClient->FreeString(m_hstrCurEditObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CObjEditMgr::Init()
{
    g_pLTClient->RegisterConsoleProgram("Commands", ListCommandsFn);
    g_pLTClient->RegisterConsoleProgram("Cmd", CmdFn);
    g_pLTClient->RegisterConsoleProgram("Trigger", TriggerFn);
    g_pLTClient->RegisterConsoleProgram("Edit", EditFn);
    g_pLTClient->RegisterConsoleProgram("Select", SelectFn);
    g_pLTClient->RegisterConsoleProgram("List", ListFn);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleListCommands()
//
//	PURPOSE:	Handle commands console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleListCommands(int argc, char **argv)
{
	// List out all our commands...
    g_pLTClient->CPrint("Object Edit Commands:");
    g_pLTClient->CPrint("  Cmd <command> [Send a command] (e.g. Cmd Delay 0.5 (msg Prop0 Fire))");
    g_pLTClient->CPrint("  Trigger <objectname> <message> [Send message to object] (e.g. Trigger Prop0 Fire)");
    g_pLTClient->CPrint("  Edit <propertyname> <value> [Edit specific object property] (e.g. Edit speed 10.0)");
    g_pLTClient->CPrint("  Select <objectname> [Select object to edit] (e.g. Select Prop0)");
    g_pLTClient->CPrint("  List <classtype> [List all objects of a particular class] (e.g. List Prop)");

	// List the server CommandMgr commands too...
	char* Argv[1] = { "LISTCOMMANDS" };
	HandleCommand(1, Argv);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleCommand()
//
//	PURPOSE:	Handle cmd console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleCommand(int argc, char **argv)
{
	if (argc < 1 || !argv) return;

	// Send message to server...

	char buf[256];
	buf[0] = '\0';
	sprintf(buf, "%s", argv[0]);
	for (int i=1; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, "\"");
		strcat(buf, argv[i]);
		strcat(buf, "\"");
	}

    HSTRING hstrCmd = g_pLTClient->CreateString(buf);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_COMMAND);
    g_pLTClient->WriteToMessageHString(hMessage, hstrCmd);
    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hstrCmd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleTrigger()
//
//	PURPOSE:	Handle trigger console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleTrigger(int argc, char **argv)
{
	if (argc < 2 || !argv) return;

	// Send message to server...

    HSTRING hstrObjName = g_pLTClient->CreateString(argv[0]);
    HSTRING hstrMsg     = g_pLTClient->CreateString(argv[1]);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_TRIGGER);
    g_pLTClient->WriteToMessageHString(hMessage, hstrObjName);
    g_pLTClient->WriteToMessageHString(hMessage, hstrMsg);
    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hstrObjName);
    g_pLTClient->FreeString(hstrMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleEdit()
//
//	PURPOSE:	Handle edit console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleEdit(int argc, char **argv)
{
	if (!m_bEditMode || argc < 2 || !argv || !m_hstrCurEditObject) return;

	// Send message to server...

	char buf[100];
	sprintf(buf, "Edit");

	for (int i=0; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}

    HSTRING hstrMsg = g_pLTClient->CreateString(buf);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_TRIGGER);
    g_pLTClient->WriteToMessageHString(hMessage, m_hstrCurEditObject);
    g_pLTClient->WriteToMessageHString(hMessage, hstrMsg);
    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hstrMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleSelect()
//
//	PURPOSE:	Handle select console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleSelect(int argc, char **argv)
{
	if (!m_bEditMode || argc < 1 || !argv) return;

	if (m_hstrCurEditObject)
	{
        g_pLTClient->FreeString(m_hstrCurEditObject);
        m_hstrCurEditObject = LTNULL;
	}

    m_hstrCurEditObject = g_pLTClient->CreateString(argv[0]);

	// Display this object's properties...

    HSTRING hstrMsg = g_pLTClient->CreateString("DISPLAYPROPERTIES");

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_TRIGGER);
    g_pLTClient->WriteToMessageHString(hMessage, m_hstrCurEditObject);
    g_pLTClient->WriteToMessageHString(hMessage, hstrMsg);
    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hstrMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::HandleList()
//
//	PURPOSE:	Handle list console command
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleList(int argc, char **argv)
{
	if (!m_bEditMode || argc < 1 || !argv) return;

	// Send message to server...

	char buf[100];
	sprintf(buf, "List %s", argv[0]);

    HSTRING hstrMsg = g_pLTClient->CreateString(buf);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_TRIGGER);
    g_pLTClient->WriteToMessageHString(hMessage, m_hstrCurEditObject);
    g_pLTClient->WriteToMessageHString(hMessage, hstrMsg);
    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hstrMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::ToggleEditMode()
//
//	PURPOSE:	Toggle edit mode on/off
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::ToggleEditMode()
{
	m_bEditMode = !m_bEditMode;

	// Make sure we are in clip mode when in edit mode...

	if (g_pGameClientShell->IsSpectatorMode() != m_bEditMode)
	{
		if (g_pCheatMgr)
		{
			g_pCheatMgr->Check("mpclip");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjEditMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::OnKeyDown(int nKey)
{
	if (!m_bEditMode) return;

	switch (nKey)
	{
		// Select the object in front of us...

		case VK_T :
		{
            g_pGameClientShell->DoActivate(LTTRUE);
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleEditObjectInfo()
//
//	PURPOSE:	Handle edit object info message
//
// ----------------------------------------------------------------------- //

void CObjEditMgr::HandleEditObjectInfo(HMESSAGEREAD hMessage)
{
    if (!g_pLTClient || !hMessage) return;

	if (m_hstrCurEditObject)
	{
        g_pLTClient->FreeString(m_hstrCurEditObject);
        m_hstrCurEditObject = LTNULL;
	}

    m_hstrCurEditObject = g_pLTClient->ReadFromMessageHString(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetCurObjectName()
//
//	PURPOSE:	Get the currently select object's name
//
// ----------------------------------------------------------------------- //

char* CObjEditMgr::GetCurObjectName() const
{
	if (m_hstrCurEditObject)
	{
        return g_pLTClient->GetStringData(m_hstrCurEditObject);
	}

	return "None";
}