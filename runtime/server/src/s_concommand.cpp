#include "bdefs.h"

#include "s_concommand.h"
#include "servermgr.h"
#include "s_net.h"
#include "systimer.h"
#include "dhashtable.h"
#include "s_client.h"
#include "ltobjectcreate.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//server console state
#include "server_consolestate.h"
#include "concommand.h"
static IServerConsoleState *console_state;
define_holder(IServerConsoleState, console_state);

//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);




// ------------------------------------------------------------------ //
// Main globals.
// ------------------------------------------------------------------ //

extern LTEngineVar* GetEngineVars();
extern int GetNumEngineVars();


extern CServerMgr *g_pServerMgr;

// Each server console command can return a status by setting this.
static LTRESULT g_CommandStatus;


// This is implemented in the LTMem library
//------------------------------------------------------------------
extern void LTMemConsole(int argc, char *argv[]);


// ------------------------------------------------------------------ //
// Helpers.
// ------------------------------------------------------------------ //

static LTBOOL _FindClassInList(char *pClassName, char classNames[500][32], int nClassNames, int *pIndex)
{
    int i;

    for (i=0; i < nClassNames; i++)
    {
        if (strcmp(pClassName, classNames[i]) == 0)
        {
            *pIndex = i;
            return LTTRUE;
        }
    }

    return LTFALSE;
}

// ------------------------------------------------------------------ //
// Command functions.
// ------------------------------------------------------------------ //

static void con_ShowUsedFiles(int argc, char *argv[])
{
    HHashIterator *hIterator;
    HHashElement *hElement;

    if (g_pServerMgr)
    {
        hIterator = hs_GetFirstElement(server_filemgr->m_hFileTable);
        while (hIterator)
        {
            hElement = hs_GetNextElement(hIterator);
            
            BPrint("Used file: %s", (char*)hs_GetElementKey(hElement, LTNULL));
        }
    }
}

static void con_ShowGameVars(int argc, char *argv[])
{
    LTCommandVar *pCurVar;
    HHashIterator *hIterator;
    HHashElement *hElement;
    ConsoleState *pState;

    if (g_pServerMgr)
    {
        pState = console_state->State();
        dsi_ConsolePrint("Game vars --------------------");

        hIterator = hs_GetFirstElement(pState->m_VarHash);
        while (hIterator)
        {
            hElement = hs_GetNextElement(hIterator);
            if (!hElement)
                continue;
            
            pCurVar = (LTCommandVar *)hs_GetElementUserData(hElement);
            cc_PrintVarDescription(pState, pCurVar);
        }
    }
}

static void con_ServerWorld(int argc, char *argv[])
{
    uint32 flags;

    if (argc >= 1)
    {
        flags = LOADWORLD_LOADWORLDOBJECTS | LOADWORLD_RUNWORLD;
        if (argc >= 2 && !atoi(argv[1]))
            flags &= ~LOADWORLD_LOADWORLDOBJECTS;

        g_CommandStatus = g_pServerMgr->DoStartWorld(argv[0], flags, time_GetMSTime());
    }
}


static void con_ObjectInfo(int argc, char *argv[])
{
    LTLink *pCur, *pListHead;
    LTObject *pObj;
    char classNames[500][32];
    int classCounts[500], nActiveClassCounts[500], classBytes[500], totalClassBytes[500];
    int totalBytes, nClassNames, index;
    uint32 dwNumActiveObjects;

    // Count them up.
    nClassNames = 0;
    memset(classCounts, 0, sizeof(classCounts));
    memset(nActiveClassCounts, 0, sizeof(nActiveClassCounts));
    memset(classBytes, 0, sizeof(classBytes));
    memset(totalClassBytes, 0, sizeof(totalClassBytes));
    dwNumActiveObjects = 0;

    dsi_PrintToConsole("------------------------- Object Information ------------------------------------");
    dsi_PrintToConsole("Class                (OBJECTS) Total Active    (BYTES) Per Class    Total");
    dsi_PrintToConsole("---------------------------------------------------------------------------------");

	pListHead = &g_pServerMgr->m_Objects.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pObj = (LTObject*)pCur->m_pData;
        
        if (!_FindClassInList(pObj->sd->m_pClass->m_ClassName, classNames, nClassNames, &index))
        {
            if (nClassNames >= 500)
                continue;

            index = nClassNames;
            LTStrCpy(classNames[nClassNames], pObj->sd->m_pClass->m_ClassName, sizeof(classNames[nClassNames]));

            ++nClassNames;
        }

        ++classCounts[index];
        classBytes[index] = pObj->sd->m_pClass->m_ClassObjectSize;
        totalClassBytes[index] += pObj->sd->m_pClass->m_ClassObjectSize;

        if (!(pObj->m_InternalFlags & IFLAG_INACTIVE_MASK))
        {
            nActiveClassCounts[index]++;
            dwNumActiveObjects++;
        }
    }

    // Show the list.
    totalBytes = 0;
    for (int i=0; i < nClassNames; i++)
    {
        totalBytes += totalClassBytes[i];
                
		for (int32 j = strlen(classNames[i]); j < 30; j++)
        {
			(classNames[i])[j] = ' ';
        }

		dsi_PrintToConsole("%s| %3d |  %3d  |             %6d | %6d ", classNames[i], 
			classCounts[i], nActiveClassCounts[i], classBytes[i], totalClassBytes[i]);
    }

	dsi_PrintToConsole(" ");
	dsi_PrintToConsole("Totals               (OBJECTS) Total Active    (BYTES)  Total");
	dsi_PrintToConsole("                                %3d    %3d        %8d", 
		g_pServerMgr->m_Objects.m_nElements, dwNumActiveObjects, totalBytes);
	dsi_PrintToConsole("---------------------------------------------------------------------------------");
}


void con_DisableWMPhysics(int argc, char **argv)
{
    LTLink *pCur, *pListHead;
    LTObject *pObj;

    pListHead = &g_pServerMgr->m_Objects.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pObj = (LTObject*)pCur->m_pData;
        pObj->m_Flags |= FLAG_BOXPHYSICS;
    }
}


void con_ExhaustMemory(int argc, char **argv)
{
    #ifdef _DEBUG
        char *pTest;
		LT_MEM_TRACK_ALLOC(pTest = new char[2000 * 1000 * 1000],LT_MEM_TYPE_MISC); // 2 gigs
    #endif
}


void con_SpawnObject(int argc, char **argv)
{
    HCLASS hClass;
    LPBASECLASS pRet;
    ObjectCreateStruct theStruct;
    Client *pClient;
    char *pSpawnArgs;

    if (argc == 0 || !g_pServerMgr || !ilt_server ||
        g_pServerMgr->m_Clients.m_nElements == 0)
    {
        dsi_ConsolePrint("SpawnObject <class name> \"spawn args\"");
        return;
    }

    if (argc >= 2)
        pSpawnArgs = argv[1];
    else
        pSpawnArgs = "";

    hClass = ilt_server->GetClass(argv[0]);
    if (!hClass)
    {
        dsi_ConsolePrint("Can't find class %s", argv[0]);
        return;
    }

    // Put it near the first client.
    theStruct.Clear();

    pClient = (Client*)g_pServerMgr->m_Clients.m_Head.m_pNext->m_pData;
    theStruct.m_Pos = pClient->m_ViewPos;
    theStruct.m_Pos.z += 200.0f;

    pRet = ilt_server->CreateObjectProps(hClass, &theStruct, pSpawnArgs);
    if (pRet)
    {
        dsi_ConsolePrint("%s spawned successfully", argv[0]);
    }
    else
    {
        dsi_ConsolePrint("Error in CreateObjectProps");
    }
}


// ------------------------------------------------------------------ //
// Tables.
// ------------------------------------------------------------------ //

static LTCommandStruct g_ServerCommandStructs[] =
{
    { "ShowGameVars", con_ShowGameVars, 0 },
    { "ShowUsedFiles", con_ShowUsedFiles, 0 },
    { "world", con_ServerWorld, 0 },
    { "objectinfo", con_ObjectInfo, 0 },
    { "DisableWMPhysics", con_DisableWMPhysics, 0 },
    { "ExhaustMemory", con_ExhaustMemory, 0 },
    { "SpawnObject", con_SpawnObject, 0 },
	{ "Mem", LTMemConsole, 0 },
};

#define NUM_SERVERCOMMANDSTRUCTS    (sizeof(g_ServerCommandStructs) / sizeof(LTCommandStruct))



// Console callbacks.
void sm_NewVar(ConsoleState *pState, LTCommandVar *pVar)
{
	CPacket_Write cPacket;
	cPacket.Writeuint8(SMSG_CONSOLEVAR);
	cPacket.WriteString(pVar->pVarName);
    cPacket.WriteString(pVar->pStringVal);
    SendServerMessage(CPacket_Read(cPacket));
}


void sm_VarChange(ConsoleState *pState, LTCommandVar *pVar)
{

// Not needed since this is done in sm_NewVar
//  if (stricmp(pVar->pVarName, "AutoDeactivate") == 0)
//  {
//      sm_ClearAutoDeactivate(g_pServerMgr);
//  }

    sm_NewVar(pState, pVar);
}


// ------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------ //

void sm_InitConsoleCommands(ConsoleState *pState)
{
    memset(pState, 0, sizeof(ConsoleState));

    pState->m_SaveFns = LTNULL;
    pState->m_nSaveFns = 0;

    pState->m_pEngineVars = GetEngineVars();
    pState->m_nEngineVars = GetNumEngineVars();

    pState->m_pCommandStructs = g_ServerCommandStructs;
    pState->m_nCommandStructs = NUM_SERVERCOMMANDSTRUCTS;

    pState->ConsolePrint = dsi_ConsolePrint;
    
    pState->Alloc = dalloc;
    pState->Free = dfree;

    pState->NewVar = sm_NewVar;
    pState->VarChange = sm_VarChange;

    cc_InitState(pState);
}


void sm_TermConsoleCommands(ConsoleState *pState)
{
    cc_TermState(pState);
}


LTRESULT sm_HandleCommand(ConsoleState *pState, char *pCommand)
{
    g_CommandStatus = LT_OK;
    cc_HandleCommand(pState, pCommand);
    return g_CommandStatus;
}





