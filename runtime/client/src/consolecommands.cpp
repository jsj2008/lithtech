//------------------------------------------------------------------
//
//	FILE	  : ConsoleCommands.cpp
//
//	PURPOSE	  : Implements the console command handler.
//
//	CREATED	  : May 7 1997
//
//	COPYRIGHT : LithTech Inc., 1996-2000
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"

#include "concommand.h"
#include "sysconsole_impl.h"
#include "clientshell.h"
#include "clientmgr.h"
#include "sysinput.h"
#include "consolecommands.h"
#include "iltinfo.h"
#include "dhashtable.h"
#include "render.h"

#include "client_ticks.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//ILTBenchmarkMgr game interface.
#include "iltbenchmark.h"
static ILTBenchmarkMgr *ilt_benchmark;
define_holder(ILTBenchmarkMgr, ilt_benchmark);


#define INPUTMGR g_pClientMgr->m_InputMgr


// The main console state.
ConsoleState g_ClientConsoleState;

// The client console iterator
//	Note : This iterator is forward only
class CClientConIterator : public CConIterator
{
protected:
	enum EState {
		STATE_COMMAND = 0,
		STATE_CONVAR = 1,
		STATE_ENGINEVAR = 2
	};
	EState m_eState;
	int m_iCommandIndex;
	HHashIterator *m_hVarIndex;
	HHashElement *m_hCurVar;
	int m_iEngineIndex;

	virtual bool Begin();
	virtual bool NextItem();

public:
	CClientConIterator();
	virtual ~CClientConIterator();

	virtual const char *Get() const;
};
CClientConIterator	g_ClientConIterator;

// From engine_vars.cpp.
extern LTEngineVar* GetEngineVars();
extern int GetNumEngineVars();
extern char g_SSFile[];
extern LTBOOL g_bUpdateServer;
extern int32 g_ScreenWidth, g_ScreenHeight;
extern int32 g_CV_ConsoleLeft;
extern int32 g_CV_ConsoleTop;
extern int32 g_CV_ConsoleRight;
extern int32 g_CV_ConsoleBottom;


//------------------------------------------------------------------
//------------------------------------------------------------------
// All the command handlers.
//------------------------------------------------------------------
//------------------------------------------------------------------

extern int32 g_DebugLevel;  // From bdefs.cpp.
extern LTVector g_ConsoleModelAdd;
extern float g_CV_DropChance;




//------------------------------------------------------------------
static void con_BlastServer(int argc, char *argv[])
{
#ifdef _DEBUG
	if(g_pClientMgr && g_pClientMgr->m_pCurShell)
	{
		int count;
		if(argc >= 1)
			count = atoi(argv[0]);
		else
			count = 500;

		CPacket_Write cBlastPacket;
		cBlastPacket.Writeuint8(CMSG_TEST);
		CPacket_Read cBlastPacket_Read(cBlastPacket);

		for(int i=0; i < count; i++)
		{
			g_pClientMgr->m_NetMgr.SendPacket(cBlastPacket_Read,
				g_pClientMgr->m_pCurShell->m_HostID, MESSAGE_GUARANTEED);
		}
		dsi_ConsolePrint("Blasted server with %d guaranteed packets", count);
	}


#endif	
}


//------------------------------------------------------------------
static void con_ModelAdd(int argc, char *argv[])
{
	if(argc >= 3)
	{
		g_ConsoleModelAdd.x = (float)atof(argv[0]);
		g_ConsoleModelAdd.y = (float)atof(argv[1]);
		g_ConsoleModelAdd.z = (float)atof(argv[2]);

		g_ConsoleModelAdd.x = LTCLAMP(g_ConsoleModelAdd.x, 0.0f, 250.0f);
		g_ConsoleModelAdd.y = LTCLAMP(g_ConsoleModelAdd.y, 0.0f, 250.0f);
		g_ConsoleModelAdd.z = LTCLAMP(g_ConsoleModelAdd.z, 0.0f, 250.0f);
	}
}


static void con_DoWorldCommand(char *pWorldName, char *pRecordFilename)
{
	CClientShell *pShell = LTNULL;
	char str[200], testName[200];
	FileRef ref;
	StartGameRequest request;
	bool bWorldFound = false;
	const char* pWorldNameExtension = ".dat";

	// Add the filename extension if needed
	if (pWorldName[strlen(pWorldName)-4] != '.') 
	{
		LTSNPrintF(testName, sizeof(testName), "%s%s", pWorldName, pWorldNameExtension);
	}
	else
	{
		LTSNPrintF(testName, sizeof(testName), "%s", pWorldName);
	}

	// Attempt to get the file id
	ref.m_pFilename = testName;
	ref.m_FileType = FILE_ANYFILE;
	if(!client_file_mgr->GetFileIdentifier(&ref, TYPECODE_WORLD))
	{
		// [mds] 4/23/01
		// We no longer check the worlds folder.  The user needs to specify
		// the path explicitly.  As Allan C. pointed out, it's inconsistent
		// and potentially confusing to the user to check only this path.
		/*
		// Retry, prepending the worlds folder name first
		// Add the filename extension if needed
		if (pWorldName[strlen(pWorldName)-4] != '.') 
		{
			sprintf(testName, "worlds\\%s%s", pWorldName, pWorldNameExtension);
		}
		else
		{
			sprintf(testName, "worlds\\%s", pWorldName);
		}

		ref.m_pFilename = testName;
		if(client_file_mgr->GetFileIdentifier(&ref, TYPECODE_WORLD))
		{
			pWorldName = testName;
			bWorldFound = true;
		}
		*/
	}
	else
		bWorldFound = true;

	// If we found the world, send the load request
	if (bWorldFound)
	{
		memset(&request, 0, sizeof(request));

		// Add the record info..
		if(pRecordFilename)
		{
			SAFE_STRCPY(request.m_RecordFilename, pRecordFilename);
		}

		pShell = g_pClientMgr->m_pCurShell;
		if(pShell && (pShell->m_ShellMode == STARTGAME_NORMAL || pShell->m_ShellMode == STARTGAME_HOST) &&
			request.m_RecordFilename[0] != 0)
		{
			LTSNPrintF(str, sizeof(str), "world %s", pWorldName);
			pShell->SendCommandToServer(str);
		}
		else
		{
			request.m_Type = STARTGAME_NORMAL;
			LTStrCpy(request.m_WorldName, pWorldName, MAX_SGR_STRINGLEN);
			g_pClientMgr->StartShell(&request);
		}
	}
	else
		dsi_ConsolePrint("World file '%s' not found", testName);
}


//------------------------------------------------------------------
static void con_World(int argc, char *argv[])
{
	if(argc >= 1)
	{
		con_DoWorldCommand(argv[0], LTNULL);
	}
	else
	{
		dsi_ConsolePrint("World <world name>");
	}
}

//------------------------------------------------------------------
static void con_EnableDevice(int argc, char *argv[])
{
	if( argc >= 1 )
	{
		if( !INPUTMGR->EnableDevice(INPUTMGR, argv[0]) )
			con_Printf( CONRGB(255,255,255), 1, "Error enabling device: %s", argv[0] );
	}
}


//------------------------------------------------------------------
static void con_Bind( int argc, char *argv[] )
{
	int i;
	
	if( argc >= 2 )
	{
		INPUTMGR->ClearBindings(INPUTMGR, argv[0], argv[1]);

		for(i=2; i < argc; i++)
		{
			if(!INPUTMGR->AddBinding(INPUTMGR, argv[0], argv[1], argv[i], 0.0f, 0.0f))
			{
				con_Printf(CONRGB(255,255,255), 1, "Error binding device: %s, trigger: %s",
					argv[0], argv[1]);
			}
		}
	}
}


//------------------------------------------------------------------
static void con_RangeBind( int argc, char *argv[] )
{
	int i, nActions;
	
	if( argc >= 4 )
	{
		INPUTMGR->ClearBindings(INPUTMGR, argv[0], argv[1]);

		nActions = (argc - 2) / 3;
		for(i=0; i < nActions; i++)
		{
			if(!INPUTMGR->AddBinding(INPUTMGR, argv[0], argv[1], 
				argv[i*3+4], (float)atof(argv[i*3+2]), (float)atof(argv[i*3+3])))
			{
				con_Printf(CONRGB(255,255,255), 1, "Error binding device: %s, trigger: %s",
					argv[0], argv[1]);
			}
		}
	}
}


//------------------------------------------------------------------
static void con_Scale( int argc, char *argv[] )
{
	if( argc >= 3 )
	{
		if( !INPUTMGR->ScaleTrigger(INPUTMGR, argv[0], argv[1], (float)atof(argv[2]), 0.0f, 0.0f, 0.0f))
			con_Printf( CONRGB(255,255,255), 1, "Error finding device: %s, trigger: %s",
				argv[0], argv[1] );
	}
}


//------------------------------------------------------------------
static void con_RangeScale( int argc, char *argv[] )
{
	if( argc >= 5 )
	{
		if( !INPUTMGR->ScaleTrigger(INPUTMGR, argv[0], argv[1], (float)atof(argv[2]), 
				(float)atof(argv[3]), (float)atof(argv[4]), (float)atof(argv[5]) ))
			con_Printf( CONRGB(255,255,255), 1, "Error finding device: %s, trigger: %s",
				argv[0], argv[1] );
	}
}


//------------------------------------------------------------------
static void con_AddAction(int argc, char *argv[])
{
	if(argc >= 2)
	{
		INPUTMGR->AddAction(INPUTMGR, argv[0], atoi(argv[1]));
	}
}


//------------------------------------------------------------------
static void con_SSFile( int argc, char *argv[] )
{
	if(argc >= 1)
	{
		LTStrCpy(g_SSFile, argv[0], _MAX_PATH + 1);
	}
}


//------------------------------------------------------------------
static void con_UpdateServer(int argc, char *argv[])
{
	// This is a command because you never really want to save this variable in the config file.
	if(argc >= 1)
	{
		g_bUpdateServer = (LTBOOL)atoi(argv[0]);
	}
}


//------------------------------------------------------------------
static void con_RenderCommand(int argc, char *argv[])
{
	if(r_GetRenderStruct())
	{
		r_GetRenderStruct()->RenderCommand(argc, argv);
	}
}


//------------------------------------------------------------------
static void con_RestartConsole(int argc, char *argv[])
{
	g_pClientMgr->InitConsole();
}


//------------------------------------------------------------------
static void con_RestartRender(int argc, char *argv[])
{
	char str[245];
	LTRESULT dResult;

	// Set renderdll automatically.
	if(argc > 0)
	{
		LTSNPrintF(str, sizeof(str), "renderDLL %s", argv[0]);
		c_CommandHandler(str);
	}
		
	r_TermRender(1, false);

 	if((dResult = g_pClientMgr->StartRenderFromGlobals()) != LT_OK)
	{
		g_pClientMgr->ProcessError(dResult | ERROR_SHUTDOWN);
	}
}


//------------------------------------------------------------------
static void con_ResizeScreen(int argc, char *argv[])
{
	uint32 oldScreenWidth, oldScreenHeight;
	LTRESULT dResult;

	if(argc >= 2)
	{
		oldScreenWidth = g_ScreenWidth;
		oldScreenHeight = g_ScreenHeight;

		g_ScreenWidth = atoi(argv[0]);
		g_ScreenHeight = atoi(argv[1]);

		con_Printf(CONRGB(0,255,0), 1, "Setting screen to %dx%d", g_ScreenWidth, g_ScreenHeight);
		
		r_TermRender(1, false);
		
		if(g_pClientMgr->StartRenderFromGlobals() != LT_OK)
		{
			g_ScreenWidth = oldScreenWidth;
			g_ScreenHeight = oldScreenHeight;
	
			if((dResult = g_pClientMgr->StartRenderFromGlobals()) != LT_OK)
			{
				g_pClientMgr->ProcessError(dResult | ERROR_SHUTDOWN);
			}
		}
	}
}


//------------------------------------------------------------------
static void con_ServerCommand(int argc, char *argv[])
{
	char tempStr[300], fullCommand[500];
	int i;

	if(argc >= 1)
	{
		if(g_pClientMgr->m_pCurShell)
		{
			// 'unparse' the string to send it to the server.
			fullCommand[0] = 0;
			for(i=0; i < argc; i++)
			{
				LTSNPrintF(tempStr, sizeof(tempStr), "\"%s\" ", argv[i]);
				LTStrCat(fullCommand, tempStr, sizeof(fullCommand));
			}

			g_pClientMgr->m_pCurShell->SendCommandToServer(fullCommand);
		}
	}
}


//------------------------------------------------------------------
static void con_Quit(int argc, char *argv[])
{
	dsi_OnClientShutdown( LTNULL );
}



//------------------------------------------------------------------
static void con_ListInputDevices(int argc, char *argv[])
{
	INPUTMGR->ListDevices(INPUTMGR);
}


//------------------------------------------------------------------
static void con_RebindTextures(int argc, char *argv[])
{
	if(g_pClientMgr)
	{
		g_pClientMgr->RebindTextures();
	}
}

//------------------------------------------------------------------
static void con_LogTextureInfo(int argc, char *argv[])
{
	if(g_pClientMgr)
	{
		if(argc != 1)
		{
			dsi_ConsolePrint("Usage: LogTextureInfo <filename.csv>");
		}
		else
		{
			g_pClientMgr->LogTextureMemory(argv[0]);
		}
	}
}

#ifndef __XBOX
void dm_HeapCompact();
#endif
static void con_HeapCompact(int argc, char *argv[])
{
#ifndef __XBOX
	dm_HeapCompact();
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Toggle settings in the client ticks
extern int32 g_ShowTickCounts;
static void con_ShowTicks(int argc, char *argv[])
{
	// Show some help if they don't specify a section
	if (argc < 1)
	{
		con_Printf(CONRGB(192,192,255), 0, "Please include at least one command:");
		con_Printf(CONRGB(192,192,255), 0, "  ALL - Show Everything");
		con_Printf(CONRGB(192,192,255), 0, "  NONE - Show Nothing");
		con_Printf(CONRGB(192,192,255), 0, "  SUMMARY - Toggle Summary information");
		con_Printf(CONRGB(192,192,255), 0, "  RENDER - Toggle Renderer");
		con_Printf(CONRGB(192,192,255), 0, "  GAME - Toggle Game processing");
		con_Printf(CONRGB(192,192,255), 0, "  ENGINE - Toggle Engine processing");
		con_Printf(CONRGB(192,192,255), 0, "  GRAPH - Toggle Summary graph");
		con_Printf(CONRGB(192,192,255), 0, "          (Red = Renderer, Green = Game, Blue = Engine, White = System)");
		return;
	}

	// Update the ShowTickCounts flags
	for (int iArgLoop = 0; iArgLoop < argc; ++iArgLoop)
	{
		if (stricmp(argv[iArgLoop], "ALL") == LTNULL)
			g_ShowTickCounts = CLIENT_TICKS_ALL;
		else if (stricmp(argv[iArgLoop], "NONE") == LTNULL)
			g_ShowTickCounts = 0;
		else if ((stricmp(argv[iArgLoop], "SUMMARY") == LTNULL) ||
			(stricmp(argv[0], "1") == LTNULL))
			g_ShowTickCounts ^= CLIENT_TICKS_SUMMARY;
		else if (stricmp(argv[iArgLoop], "RENDER") == LTNULL)
			g_ShowTickCounts ^= CLIENT_TICKS_RENDER;
		else if (stricmp(argv[iArgLoop], "GAME") == LTNULL)
			g_ShowTickCounts ^= CLIENT_TICKS_GAME;
		else if (stricmp(argv[iArgLoop], "ENGINE") == LTNULL)
			g_ShowTickCounts ^= CLIENT_TICKS_ENGINE;
		else if (stricmp(argv[iArgLoop], "GRAPH") == LTNULL)
			g_ShowTickCounts ^= CLIENT_TICKS_GRAPH;
		// Dunno what they wanted....
		else
		{
			con_Printf(CONRGB(255,192,192), 0, "Error: \"%s\" is not a valid ShowTicks section", argv[iArgLoop]);
		}
	}
}

// Manipulate the console's history
static void con_ConsoleHistory(int argc, char *argv[])
{
	// Get the history iterator
	CConsole *pConsole = GETCONSOLE();
	CConHistory *pHistory = pConsole->GetCommandHistory();

	// List the current history
	if ( !pHistory->First() )
		return;

	int iCount = 0;
	do
	{
		const char *pLine = pHistory->Get();
		if ( !pLine )
			break;
		
		dsi_ConsolePrint( (char *)pLine );
		iCount++;
	} while ( pHistory->Next() );
	dsi_ConsolePrint( "%d commands", iCount );
}

// Clear the console's history list
static void con_ClearHistory(int argc, char *argv[])
{
	GETCONSOLE()->GetCommandHistory()->Clear();
	dsi_ConsolePrint( "Command history cleared." );
}

// Write the console history to a file
static void con_WriteHistory(int argc, char *argv[])
{
	// Argument checking
	if ( argc < 1 )
	{
		dsi_ConsolePrint( "Specify a filename." );
		return;
	}

	// Start the history iterator
	CConsole *pConsole = GETCONSOLE();
	CConHistory *pHistory = pConsole->GetCommandHistory();

	if ( !pHistory->First() )
	{
		dsi_ConsolePrint( "No commands in history." );
		return;
	}

	// Open the file
	FILE *fOutput;
	if ( (fOutput = fopen( argv[0], "wt" )) == LTNULL )
	{
		dsi_ConsolePrint( "Error opening file %s.", argv[0] );
		return;
	}

	int iCount = 0;

	do
	{
		// Write this history line to the file
		const char *pLine = pHistory->Get();
		if ( !pLine )
			break;

		fputs( pLine, fOutput );
		fputc( '\n', fOutput );
		iCount++;
	} while ( pHistory->Next() );

	// Close the file
	fclose( fOutput );

	dsi_ConsolePrint( "Successfully wrote %d commands to %s.", iCount, argv[0] );
}

// Read a file into the console history
static void con_ReadHistory(int argc, char *argv[])
{
	// Argument checking
	if ( argc < 1 )
	{
		dsi_ConsolePrint( "Specify a filename." );
		return;
	}

	// Open the file
	FILE *fInput;
	if ( (fInput = fopen( argv[0], "rt" )) == LTNULL )
	{
		dsi_ConsolePrint( "Error opening file %s.", argv[0] );
		return;
	}

	// Get the history iterator
	CConsole *pConsole = GETCONSOLE();
	CConHistory *pHistory = pConsole->GetCommandHistory();

	int iCount = 0;

	char aBuffer[MAX_CONSOLE_TEXTLEN];
	while ( !feof( fInput ) )
	{
		// Read a line from the file
		if ( fgets( aBuffer, MAX_CONSOLE_TEXTLEN, fInput ) > 0 )
		{
			int iLength = strlen( aBuffer );
			// Remove trailing newlines
			if ( (iLength > 0) && (aBuffer[iLength - 1] == '\n') )
				aBuffer[--iLength] = 0;

			// Skip blank lines
			if ( !iLength )
				continue;

			// Add the line to the history
			pHistory->Add( aBuffer );
			iCount++;
		}
	} 

	// Close the file
	fclose( fInput );

	dsi_ConsolePrint( "Successfully read %d commands from %s.", iCount, argv[0] );
}

// Execute a file
static void con_Exec(int argc, char *argv[])
{
	// Argument checking
	if ( argc < 1 )
	{
		dsi_ConsolePrint( "Specify a filename." );
		return;
	}

	// Execute the file
	if ( cc_RunConfigFile(&g_ClientConsoleState, argv[0], 0, VARFLAG_SAVE) )
		dsi_ConsolePrint( "Successfully executed %s.", argv[0] );
	else
		dsi_ConsolePrint( "Error executing %s.", argv[0] );
}

// Show version information
static void con_ShowVersionInfo(int argc, char *argv[])
{
	// Sanity checks
	IFBREAKRETURN(ilt_client == NULL);
	if (ilt_client->GetVersionInfoExt == NULL ) return; 

	LTVERSIONINFOEXT info;

	// Initialize our structure
	memset(&info, 0, sizeof(LTVERSIONINFOEXT));
	info.m_dwSize = sizeof(LTVERSIONINFOEXT);
	
	// Get the actual information
	LTRESULT ltResult = ilt_client->GetVersionInfoExt(&info);

	// Display the info
	if (LT_OK == ltResult)
	{
		// Display results
		dsi_ConsolePrint("%s: %d.%d, %s: %d, %s: %d/%d/%d, %s, %s: %s, %s: %s",
							"Version",
							info.m_dwMajorVersion,
							info.m_dwMinorVersion,
							"Build Number",
							info.m_dwBuildNumber,
							"Build Date",
							info.m_dwBuildMonth,
							info.m_dwBuildDay,
							info.m_dwBuildYear,
							info.m_sBuildName,
							"Language",
							"English",
							"Platform",
							"PC/Windows");
	}
	else
		dsi_ConsolePrint("Error attempting to show version info.");
}

// Move the console window
static void con_MoveConsole(int argc, char *argv[])
{
	// Report the console location
	if (!argc)
	{
		dsi_ConsolePrint("Console position : (%d,%d, %d,%d)", g_CV_ConsoleLeft, g_CV_ConsoleTop, g_CV_ConsoleRight, g_CV_ConsoleBottom);
		return;
	}

	// Change the console size
	LTRect rect;
	if (stricmp(argv[0], "Top") == 0)
	{
		rect.left = -1;
		rect.top = -1;
		rect.right = -1;
		rect.bottom = -2;
	}
	else if (stricmp(argv[0], "Bottom" ) == 0)
	{
		rect.left = -1;
		rect.top = -2;
		rect.right = -1;
		rect.bottom = -1;
	}
	else if (stricmp(argv[0], "Middle") == 0)
	{
		rect.left = -4;
		rect.top = -4;
		rect.right = -4;
		rect.bottom = -4;
	}
	else if (stricmp(argv[0], "Full") == 0)
	{
		rect.left = -1;
		rect.top = -1;
		rect.right = -1;
		rect.bottom = -1;
	}
	else
	{
		if (argc < 4)
		{
			dsi_ConsolePrint("Specify Top, Bottom, Full, Middle, or 4 screen coordinates");
			return;
		}

		// Use the coordinates entered by the user
		rect.left = atoi(argv[0]);
		rect.top = atoi(argv[1]);
		rect.right = atoi(argv[2]);
		rect.bottom = atoi(argv[3]);
	}

	// Write the new console rectangle to the console variables

	char cmd[200];
	LTSNPrintF(cmd, sizeof(cmd), "ConsoleLeft %d", rect.left);
	c_CommandHandler(cmd);
	LTSNPrintF(cmd, sizeof(cmd), "ConsoleTop %d", rect.top);
	c_CommandHandler(cmd);
	LTSNPrintF(cmd, sizeof(cmd), "ConsoleRight %d", rect.right);
	c_CommandHandler(cmd);
	LTSNPrintF(cmd, sizeof(cmd), "ConsoleBottom %d", rect.bottom);
	c_CommandHandler(cmd);
}

// These ones are implemented below the static table definitions.
//------------------------------------------------------------------
static void con_ListCommands(int argc, char *argv[]);
static void con_Set(int argc, char *argv[]);

// This is implemented in the LTMem library
//------------------------------------------------------------------
extern void LTMemConsole(int argc, char *argv[]);


// ------------------------------------------------------------------ //
// Save functions
// ------------------------------------------------------------------ //

static void SaveModelAdd(FILE *fp)
{
	fprintf(fp, "ModelAdd %f %f %f\n", g_ConsoleModelAdd.x, g_ConsoleModelAdd.y, g_ConsoleModelAdd.z);
}

static LTSaveFn g_SaveFns[] =
{
	input_SaveBindings,
	SaveModelAdd
};

#define NUM_SAVEFNS	(sizeof(g_SaveFns) / sizeof(LTSaveFn))




//------------------------------------------------------------------
//------------------------------------------------------------------
// The main table of command handlers.
//------------------------------------------------------------------
//------------------------------------------------------------------

static LTCommandStruct g_LTCommandStructs[] =
{
	"BlastServer", con_BlastServer, 0,
	"ListInputDevices", con_ListInputDevices, 0,
	"quit", con_Quit, 0,
	"serv", con_ServerCommand, 0,
	"ModelAdd", con_ModelAdd, 0,
	"World", con_World, 0,
	"EnableDevice", con_EnableDevice, 0,
	"RangeBind", con_RangeBind, 0,
	"Bind", con_Bind, 0,
	"Scale", con_Scale, 0,
	"RangeScale", con_RangeScale, 0,
	"AddAction", con_AddAction, 0,
	"SSFile", con_SSFile, 0,
	"UpdateServer", con_UpdateServer, 0,
	"RenderCommand", con_RenderCommand, 0,
	"RCom", con_RenderCommand, 0,
	"Set", con_Set, 0,
	"ListCommands", con_ListCommands, 0,
	"RestartConsole", con_RestartConsole, 0,
	"RestartRender", con_RestartRender, 0,
	"ResizeScreen", con_ResizeScreen, 0,
	"RebindTextures", con_RebindTextures, 0,
	"LogTextureInfo", con_LogTextureInfo, 0,
	"HeapCompact", con_HeapCompact, 0,
	"ConsoleHistory", con_ConsoleHistory, 0,
	"ClearHistory", con_ClearHistory, 0,
	"WriteHistory", con_WriteHistory, 0,
	"ReadHistory", con_ReadHistory, 0,
	"Exec", con_Exec, 0,
	"ShowVersionInfo", con_ShowVersionInfo, 0,
	"MoveConsole", con_MoveConsole, 0,
	"Mem", LTMemConsole, 0,
	"ShowTicks", con_ShowTicks, 0,
};	

#define NUM_COMMANDSTRUCTS	(sizeof(g_LTCommandStructs) / sizeof(LTCommandStruct))






//------------------------------------------------------------------
static void con_ListCommands(int argc, char *argv[])
{
	int i;

	for(i=0; i < g_ClientConsoleState.m_nCommandStructs; i++)
	{
		con_WhitePrintf(g_ClientConsoleState.m_pCommandStructs[i].pCmdName);
	}
}


//------------------------------------------------------------------
static void con_Set(int argc, char *argv[])
{
	LTCommandVar *pCurVar;
	HHashIterator *hIterator;
	HHashElement *hElement;
	hIterator = hs_GetFirstElement( g_ClientConsoleState.m_VarHash );
	while(hIterator)
	{
		hElement = hs_GetNextElement(hIterator);
		if( !hElement )
			continue;
		
		pCurVar = ( LTCommandVar * )hs_GetElementUserData( hElement );
		cc_PrintVarDescription(&g_ClientConsoleState, pCurVar);
	}
}


//------------------------------------------------------------------
//------------------------------------------------------------------
// Main parsing / command handler
//------------------------------------------------------------------
//------------------------------------------------------------------

void c_InitConsoleCommands()
{
	memset(&g_ClientConsoleState, 0, sizeof(g_ClientConsoleState));

	g_ClientConsoleState.m_SaveFns = g_SaveFns;
	g_ClientConsoleState.m_nSaveFns = NUM_SAVEFNS;

	g_ClientConsoleState.m_pEngineVars = GetEngineVars();
	g_ClientConsoleState.m_nEngineVars = GetNumEngineVars();

	g_ClientConsoleState.m_pCommandStructs = g_LTCommandStructs;
	g_ClientConsoleState.m_nCommandStructs = NUM_COMMANDSTRUCTS;

	g_ClientConsoleState.ConsolePrint = con_WhitePrintf;
	
	g_ClientConsoleState.Alloc = dalloc;
	g_ClientConsoleState.Free = dfree;

	cc_InitState(&g_ClientConsoleState);

	// Set up the completion iterator
	GETCONSOLE()->SetCompletionIterator( &g_ClientConIterator );
}


void c_TermConsoleCommands()
{
	cc_TermState(&g_ClientConsoleState);
}


void c_CommandHandler(const char *pCommand)
{
	cc_HandleCommand(&g_ClientConsoleState, pCommand);
}	



CClientConIterator::CClientConIterator() :
	m_eState(STATE_COMMAND),
	m_iCommandIndex(0),
	m_hVarIndex(0),
	m_hCurVar(0),
	m_iEngineIndex(0)
{
}

CClientConIterator::~CClientConIterator()
{
	// Nothing to destruct...
}

bool CClientConIterator::Begin()
{
	// Go to the beginning of the command list
	m_eState = STATE_COMMAND;
	m_iCommandIndex = 0;

	// This isn't guaranteed to be true, but I'm pretty sure it will be...
	return TRUE;
}

bool CClientConIterator::NextItem()
{
	bool bEnd = LTFALSE;

	switch (m_eState)
	{
		case STATE_COMMAND :
		{
			// Move to the next command
			m_iCommandIndex++;
			// Overflow to the variable list
			if ( m_iCommandIndex >= g_ClientConsoleState.m_nCommandStructs )
			{
				// Go to variable mode
				m_eState = STATE_CONVAR;
				m_hVarIndex = 0;
				bEnd = !NextItem();
			}
			break;
		}
		case STATE_CONVAR :
		{
			if (!m_hVarIndex)
			{
				m_hVarIndex = hs_GetFirstElement( g_ClientConsoleState.m_VarHash );
				m_hCurVar = LTNULL;
			}

			// find the next variable
			do { 
				m_hCurVar = hs_GetNextElement(m_hVarIndex);
			} while ( m_hVarIndex && !m_hCurVar );

			// Overflow to the engine variable list
			if (!m_hVarIndex)
			{
				m_eState = STATE_ENGINEVAR;
				m_iEngineIndex = -1;
				bEnd = !NextItem();
			}
			break;
		}
		case STATE_ENGINEVAR :
		{
			if (m_iCommandIndex >= GetNumEngineVars())
			{
				// Nothing to overflow to, so we're at the end
				bEnd = TRUE;
			}
			else
			{
				m_iEngineIndex++;
			}
		}
	}

	return !bEnd;
}

const char *CClientConIterator::Get() const
{
	const char *pResult = LTNULL;

	switch (m_eState)
	{
		// Get a command
		case STATE_COMMAND :
		{
			// Make sure we're not past the end of the list
			if ( m_iCommandIndex < g_ClientConsoleState.m_nCommandStructs )
				pResult = g_ClientConsoleState.m_pCommandStructs[m_iCommandIndex].pCmdName;
			break;
		}
		// Get a console variable
		case STATE_CONVAR :
		{
			// Make sure we're not past the end of the list
			if ( m_hCurVar )
				pResult = (( LTCommandVar * )hs_GetElementUserData( m_hCurVar ))->pVarName;
			break;
		}
		// Get an engine variable
		case STATE_ENGINEVAR :
		{
			// Make sure we're not past the end of the list
			if ( m_iEngineIndex < GetNumEngineVars() )
				pResult = GetEngineVars()[m_iEngineIndex].pVarName;
			break;
		}
	}

	return pResult;
}

