// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.cpp
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMulti.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "ClientMultiplayerMgr.h"
#include "iserverdir.h"
#include "msgids.h"
#include "WinUtil.h"
#include "direct.h"
#include "ClientButeMgr.h"

namespace
{
	int kMaxCDKeyLength = 24;
	uint16 kWaitWidth = 160;
	uint16 kWaitHeight = 60;


	void EditCDKeyCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenMulti *pThisScreen = (CScreenMulti *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_MULTI);
		if (pThisScreen)
			pThisScreen->SendCommand(bReturn ? CMD_OK : CMD_CANCEL,(uint32)pData,CMD_EDIT_CDKEY);
	}
	void NewVersionCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenMulti *pThisScreen = (CScreenMulti *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_MULTI);
		if (pThisScreen)
			pThisScreen->SendCommand(CMD_OK,NULL,CMD_UPDATE);
	}
}


extern bool g_bLAN;

//local butemgr for use reading DM mission configurations
class CDMButeMgr : public CGameButeMgr
{
	public :
        virtual LTBOOL	Init(const char* szAttributeFile);
		CButeMgr*	GetButeMgr() {return &m_buteMgr;}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2MissionButeMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
LTBOOL CDMButeMgr::Init(const char* szAttributeFile)
{
    if(!szAttributeFile) return LTFALSE;

	// See if we already have this attribute file loaded.
	if( m_strAttributeFile.GetLength( ) && m_strAttributeFile.CompareNoCase( szAttributeFile ) == 0 )
		return LTTRUE;
    
	// Start fresh.
	Term( );

    ILTStream* pDStream = LTNULL;
    LTRESULT dr = g_pLTBase->OpenFile(szAttributeFile, &pDStream);
    bool bFound = (dr == LT_OK && pDStream);
	if (pDStream)
		pDStream->Release();


	if (!bFound || !Parse(szAttributeFile)) 
		return LTFALSE;

	return LTTRUE;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMulti::CScreenMulti() :
	m_sCurCDKey(""),
	m_sLastValidCDKey("")
{
}

CScreenMulti::~CScreenMulti()
{

}

// Build the screen
LTBOOL CScreenMulti::Build()
{

	CreateTitle(IDS_TITLE_MULTI);

	//basic controls
	AddTextItem(IDS_PLAYER_SETUP, CMD_PLAYER, IDS_HELP_PLAYER);

	m_pCDKeyCtrl = AddColumnCtrl(CMD_EDIT_CDKEY, IDS_HELP_CDKEY);
	m_pCDKeyCtrl->AddColumn(LoadTempString(IDS_CDKEY), 200);
	m_pCDKeyCtrl->AddColumn("  ", 320, LTTRUE);
	
	m_pJoin = AddTextItem(IDS_JOIN, CMD_JOIN, IDS_HELP_JOIN);
	m_pHost = AddTextItem(IDS_HOST, CMD_HOST, IDS_HELP_HOST);


	m_pUpdate = AddTextItem(IDS_LAUNCH_UPDATE, CMD_UPDATE, IDS_HELP_LAUNCH_UPDATE);


	HTEXTURE hUp = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup.dtx");
	HTEXTURE hUpH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup_h.dtx");
	HTEXTURE hDown = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn.dtx");
	HTEXTURE hDownH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn_h.dtx");

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_MULTI,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);

	LTRect rect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_MULTI,"SystemMOTDRect");
	LTIntPt pos(rect.left,rect.top);
	LTIntPt size( (rect.right - rect.left),(rect.bottom - rect.top));
	uint8 nMOTDSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_MULTI,"MessageFontSize");
	uint8 nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pSysFrame = debug_new(CLTGUIFrame);
	m_pSysFrame->Create(hFrame,size.x+16,size.y,LTTRUE);
	m_pSysFrame->SetBasePos(pos);
	AddControl(m_pSysFrame);

	m_pSysMOTD = debug_new(CLTGUILargeText);
	m_pSysMOTD->Create("",pFont,nMOTDSize,size);
	m_pSysMOTD->SetBasePos(pos);
	m_pSysMOTD->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pSysMOTD->Enable(LTTRUE);
	m_pSysMOTD->SetFrameWidth(1);
	m_pSysMOTD->SetIndent(2);
	m_pSysMOTD->UseArrows(1.0f,hUp,hUpH,hDown,hDownH);
	AddControl(m_pSysMOTD);



	rect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_MULTI,"GameMOTDRect");
	pos.x = rect.left;
	pos.y = rect.top;
	size.x = (rect.right - rect.left);
	size.y = (rect.bottom - rect.top);

	m_pGameFrame = debug_new(CLTGUIFrame);
	m_pGameFrame->Create(hFrame,size.x+16,size.y,LTTRUE);
	m_pGameFrame->SetBasePos(pos);
	AddControl(m_pGameFrame);

	
	m_pGameMOTD = debug_new(CLTGUILargeText);
	m_pGameMOTD->Create("",pFont,nMOTDSize,size);
	m_pGameMOTD->SetBasePos(pos);
	m_pGameMOTD->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pGameMOTD->Enable(LTTRUE);
	m_pGameMOTD->SetFrameWidth(1);
	m_pGameMOTD->SetIndent(2);
	m_pGameMOTD->UseArrows(1.0f,hUp,hUpH,hDown,hDownH);
	AddControl(m_pGameMOTD);




	nFont = g_pLayoutMgr->GetDialogFontFace();
	pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetDialogFontSize();

	m_pWaitText = debug_new(CLTGUITextCtrl);
    if (!m_pWaitText->Create(LoadTempString(IDS_INTERNET), NULL, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(m_pWaitText);
        return LTFALSE;
	}
	m_pWaitText->SetColors(argbBlack, argbBlack, argbBlack);
	m_pWaitText->Enable(LTFALSE);

	uint16 w = 16+m_pWaitText->GetBaseWidth();
	uint16 h = 16+m_pWaitText->GetBaseHeight();


	m_pWait = debug_new(CLTGUIWindow);

	char szBack[128] = "";
	g_pLayoutMgr->GetDialogFrame(szBack,sizeof(szBack));

	m_pWait->Create(g_pInterfaceResMgr->GetTexture(szBack),w,h);

	uint16 x = (640-w)/2;
	uint16 y = (480-h)/2;
	m_pWait->SetBasePos(LTIntPt(x,y));
	AddControl(m_pWait);

	
	m_pWait->AddControl(m_pWaitText,LTIntPt(8,8));


	// status text -----------------------------------------------
	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_MULTI,"StatusPos");
	char szTmp[256] = "";
	LoadString(IDS_WAITING,szTmp,sizeof(szTmp));
	m_pStatusCtrl = AddTextItem(FormatTempString(IDS_STATUS_STRING,szTmp), 0, 0, pos, LTTRUE);
	m_pStatusCtrl->SetFont(NULL, nMOTDSize);

	CreateDMMissionFile();
//	CreateTDMMissionFile();
	CreateDDMissionFile();


 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (m_eCurState == eState_Startup || m_eCurState == eState_ValidateCDKey) return 0;
	switch(dwCommand)
	{
	case CMD_UPDATE:
		{
			LaunchSierraUp();
		} break;

	case CMD_OK:
		{
			return HandleCallback(dwParam1,dwParam2);
		}	break;

	case CMD_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PLAYER);
			break;
		}
	case CMD_JOIN:
		{
			if (g_bLAN)
			{
				m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOIN_LAN);
			}
			else
			{
				if (m_eCurState == eState_NoCDKey)
				{
					MBCreate mb;
					g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);
					return 0;
				}
				m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOIN);
			}
			
			break;
		}

	case CMD_HOST:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_HOST);
			break;
		}
	case CMD_EDIT_CDKEY :
		{
			ChangeCDKey();
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenMulti::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientMultiplayerMgr->ForceDisconnect();
		}

		std::string sMissionFile;
		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeCooperative:
			// Initialize to the coop mission bute.
			sMissionFile = MISSION_COOP_FILE;
			break;
		
		case eGameTypeDeathmatch:
		case eGameTypeTeamDeathmatch:
			// Initialize to the DM mission bute.
			sMissionFile = MISSION_DM_FILE;
			break;

		case eGameTypeDoomsDay:
			// Initialize to the doomsday mission bute.
			sMissionFile = MISSION_DD_FILE;
			break;
		}

		if( !g_pMissionButeMgr->Init( sMissionFile.c_str() ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", sMissionFile.c_str() );
			return;
		}


		if (g_bLAN)
		{
			m_eCurState = eState_Ready;
			m_pCDKeyCtrl->Show(LTFALSE);
			m_pJoin->Enable(LTTRUE);
			m_pHost->Enable(LTTRUE);
			m_pWait->Show(LTFALSE);
			m_pSysMOTD->Show(LTFALSE);
			m_pGameMOTD->Show(LTFALSE);
			m_pSysFrame->Show(LTFALSE);
			m_pGameFrame->Show(LTFALSE);
			m_pUpdate->Show(LTFALSE);
			m_pStatusCtrl->Show(LTFALSE);

		}
		else
		{
			m_pJoin->Enable(LTFALSE);
			m_pHost->Enable(LTFALSE);

			m_pSysMOTD->Show(LTTRUE);
			m_pGameMOTD->Show(LTTRUE);
			m_pSysFrame->Show(LTTRUE);
			m_pGameFrame->Show(LTTRUE);
			m_pUpdate->Show(LTFALSE);
			m_pStatusCtrl->Show(LTTRUE);
			

			m_pCDKeyCtrl->Show(LTTRUE);
			IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
			// Make a serverdir if we don't have one
			// Note : Find a way to put this back in the game client shell!!!  It has to be
			// here right now so that we can delete it when we start up a server..
			if (!pServerDir)
			{
				pServerDir = g_pClientMultiplayerMgr->CreateServerDir( );
			}
			m_pWaitText->SetString(LoadTempString(IDS_INTERNET));
			m_pWait->Show(LTTRUE);
			SetCapture(m_pWait);

			m_eCurState = eState_Startup;

		}



        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
	if (bFocus)
	{
		//since several failure states can drop us here, make sure that we return to main screen on escape
		m_pScreenMgr->AddScreenToHistory( SCREEN_ID_MAIN );
	}

}



uint32 CScreenMulti::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch (dwParam2)
	{
		case CMD_UPDATE:
		{
			//following notification of new version, continue validation process
			RequestValidate();
		} break;

		case CMD_EDIT_CDKEY :
		{
			IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
			if (pServerDir->SetCDKey((const char *)dwParam1))
			{
				m_sCurCDKey = ((const char *)dwParam1);
				pServerDir->SetCDKey(m_sCurCDKey.c_str());

				bool bResult = pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_CDKey);
				if (bResult)
				{
					g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_Validate_CDKey)");
					m_pCDKeyCtrl->SetString(1,m_sCurCDKey.c_str());
					m_eCurState = eState_ValidateCDKey;
					m_pWait->Show(LTTRUE);
					SetCapture(m_pWait);
				}

			}
			if (m_eCurState != eState_ValidateCDKey)
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);

				std::string str = pServerDir->GetLastRequestResultString( );
				g_pLTClient->CPrint( "CDKey validation error: %s", str.c_str( ));


				m_pWait->Show(LTFALSE);
				if (GetCapture() == m_pWait)
					SetCapture(NULL);
				


				m_pCDKeyCtrl->SetString(1,m_sLastValidCDKey.c_str());
				m_pJoin->Enable(!m_sLastValidCDKey.empty());
				m_pHost->Enable(!m_sLastValidCDKey.empty());
				m_sCurCDKey = m_sLastValidCDKey;
				pServerDir->SetCDKey(m_sCurCDKey.c_str());

				if (m_sLastValidCDKey.empty())
					m_eCurState = eState_NoCDKey;
			}

			break;
		}
	}
	return 1;
}

void CScreenMulti::ChangeCDKey()
{
	// Show the CD key change dialog
	MBCreate mb;
	mb.eType = LTMB_EDIT;
	mb.pFn = EditCDKeyCallBack;
	mb.pString = m_sCurCDKey.c_str();
	mb.nMaxChars = kMaxCDKeyLength;
	g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_ENTER,&mb);
}


LTBOOL CScreenMulti::Render(HSURFACE hDestSurf)
{
	Update();
	return CBaseScreen::Render(hDestSurf);
}

void CScreenMulti::Update()
{
	if (g_bLAN) return;

	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	char aTempBuffer[256];

	FormatString(IDS_STATUS_STRING,aTempBuffer,sizeof(aTempBuffer),g_pClientMultiplayerMgr->GetServerDir()->GetCurStatusString());
	m_pStatusCtrl->SetString(aTempBuffer);


	// Are we still waiting?
	switch (pServerDir->GetCurStatus())
	{
		case IServerDirectory::eStatus_Processing : 
			return;
		case IServerDirectory::eStatus_Waiting : 
		{

			switch (m_eCurState)
			{
			case eState_Startup:
				{
					//completed startup... check version
					m_eCurState = eState_VersionCheck;

					bool bResult = pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_Version);
					if (bResult)
					{
						g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_Validate_Version)");

					}
					else
					{
						g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_Validate_Version) : FAILED");
						ASSERT(0);

						MBCreate mb;
						g_pInterfaceMgr->ShowMessageBox(IDS_VALIDATION_FAILED,&mb);

						m_pWait->Show(LTFALSE);
						SetCapture(NULL);

						Escape();
						return;
					}

				} break;

			case eState_VersionCheck:
				{
					m_pWait->Show(LTFALSE);
					SetCapture(NULL);
			
					if (pServerDir->IsVersionNewest())
					{
						//passed version check... check CDKey

					}
					else if (pServerDir->IsVersionValid())
					{
						//enable updater
						MBCreate mb;
						mb.pFn = NewVersionCallBack;
						g_pInterfaceMgr->ShowMessageBox(IDS_NEW_VERSION,&mb);

						m_pUpdate->Show(LTTRUE);
						return;
					}
					else
					{
						//error state... bail
						MBCreate mb;
						g_pInterfaceMgr->ShowMessageBox(IDS_VALIDATION_FAILED,&mb);


						Escape();
						return;
					}

					RequestValidate();

				} break;

			case eState_ValidateCDKey:
				{
					//completed validation... check MOTD;
					m_pWait->Show(LTFALSE);
					SetCapture(NULL);
					m_sLastValidCDKey = m_sCurCDKey;

					RequestMOTD();
					
				}break;

			case eState_MOTD:
				{
					//completed system MOTD... check game MOTD

					if ((m_sSysMOTD.empty() && pServerDir->IsMOTDNew(IServerDirectory::eMOTD_System)) ||
						(m_sGameMOTD.empty() && pServerDir->IsMOTDNew(IServerDirectory::eMOTD_Game)))
					{
						//deal with new system motd
						MBCreate mb;
						//add callback
						g_pInterfaceMgr->ShowMessageBox(IDS_NEW_MOTD,&mb);

					}
					
					std::string sTemp = pServerDir->GetMOTD(IServerDirectory::eMOTD_System);
					if (sTemp.length() > 512)
						m_sSysMOTD.assign(sTemp.c_str(),512);
					else
						m_sSysMOTD = sTemp;

					sTemp = pServerDir->GetMOTD(IServerDirectory::eMOTD_Game);
					if (sTemp.length() > 512)
						m_sGameMOTD.assign(sTemp.c_str(),512);
					else
						m_sGameMOTD = sTemp;

					m_pSysMOTD->SetString(m_sSysMOTD.c_str());
					m_pGameMOTD->SetString(m_sGameMOTD.c_str());

					m_pJoin->Enable(LTTRUE);
					m_pHost->Enable(LTTRUE);

					m_eCurState = eState_Ready;
						
				} break;


			}

			break;
		}
		case IServerDirectory::eStatus_Error : 
		{
			g_pLTClient->CPrint( "ServerDir request error: %s", pServerDir->GetLastRequestResultString( ));
	
			// Ignore errors in the MOTD/Version queries for now...
			// NYI
			IServerDirectory::ERequest eErrorRequest = pServerDir->GetLastErrorRequest();
			if (eErrorRequest == IServerDirectory::eRequest_Validate_CDKey)
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);

				if (m_sLastValidCDKey.empty())
				{
					m_pJoin->Enable(LTFALSE);
					m_pHost->Enable(LTFALSE);

				}
				else
				{
					m_pCDKeyCtrl->SetString(1,m_sLastValidCDKey.c_str());
					m_pJoin->Enable(LTTRUE);
					m_pHost->Enable(LTTRUE);
				}
				
				m_sCurCDKey = m_sLastValidCDKey;
				pServerDir->SetCDKey(m_sCurCDKey.c_str());

				m_eCurState = eState_Ready;
								
				m_pWait->Show(LTFALSE);
				if (GetCapture() == m_pWait)
					SetCapture(NULL);
				
								
			}
			else if ((eErrorRequest == IServerDirectory::eRequest_MOTD) )
			{

				g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_MOTD) : FAILED");

				m_eCurState = eState_Ready;
				m_sSysMOTD = "";
				m_sGameMOTD = "";

				m_pSysMOTD->SetString(m_sSysMOTD.c_str());
				m_pGameMOTD->SetString(m_sGameMOTD.c_str());

				m_pJoin->Enable(LTTRUE);
				m_pHost->Enable(LTTRUE);

			}
			else
			{

				// Whoops, something went wrong.
				// Drop out of this screen with an error
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(pServerDir->GetLastRequestResultString(),&mb);

				

				Escape();
			}
			pServerDir->ProcessRequestList();
			break;
		}
		default :
		{
			ASSERT(!"Unknown directory status encountered");
			break;
		}
	}
}




void CScreenMulti::RequestMOTD()
{
	bool bResult = g_pClientMultiplayerMgr->GetServerDir()->QueueRequest(IServerDirectory::eRequest_MOTD);
	if (bResult)
	{
		g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_MOTD)");
		m_eCurState = eState_MOTD;

	}
	else
	{
		//TODO: deal with failure states
		g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_MOTD) : FAILED");

		m_eCurState = eState_Ready;
		m_sSysMOTD = "";
		m_sGameMOTD = "";

		m_pSysMOTD->SetString(m_sSysMOTD.c_str());
		m_pGameMOTD->SetString(m_sGameMOTD.c_str());

		m_pJoin->Enable(LTTRUE);
		m_pHost->Enable(LTTRUE);

		
	}
}


//////////////////////////////////////////////////////////////////////
// Run the game update utility
//////////////////////////////////////////////////////////////////////

LTBOOL CScreenMulti::LaunchSierraUp()
{
	PROCESS_INFORMATION procInfo;
	STARTUPINFO startInfo;
	CString sCmdLine;
	RMode rMode;

	// Save the current render mode.  We'll need to restore it if the serverapp
	// launching fails.
	g_pLTClient->GetRenderMode( &rMode );

	// Shutdown the renderer, minimize it, and hide it...
	g_pLTClient->ShutdownRender( RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW );

	// Initialize the startup info.
	memset( &startInfo, 0, sizeof( STARTUPINFO ));
	startInfo.cb = sizeof( STARTUPINFO );


	

	// Setup the command line.
#ifdef _DEMO
	sCmdLine.Format("SierraUp.exe ProductName \"NOLF2Demo\" CurrentVersion \"%s\" ResourceDllFile \"NOLF2Up.dll\"", g_pVersionMgr->GetNetVersion());
#else
	sCmdLine.Format("SierraUp.exe ProductName \"NOLF2\" CurrentVersion \"%s\" ResourceDllFile \"NOLF2Up.dll\"", g_pVersionMgr->GetNetVersion());
#endif


	// Start the server app.
	if(!CreateProcess("SierraUp.exe", ( char* )( char const* )sCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &procInfo))
	{
		// Serverapp failed.  Restore the render mode.
		g_pLTClient->SetRenderMode( &rMode );
		return LTFALSE;
	}

	// We're done with this process.
	g_pLTClient->Shutdown();

	return LTTRUE;
}


void CScreenMulti::RequestValidate()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
	// Load the default CD key
	pServerDir->GetCDKey(&m_sCurCDKey);
	if (!pServerDir->SetCDKey(m_sCurCDKey.c_str()))
		m_sCurCDKey = "";	
	
	m_pCDKeyCtrl->SetString(1,m_sCurCDKey.c_str());

	if (m_sCurCDKey.empty())
	{
		m_pJoin->Enable(LTFALSE);
		m_pHost->Enable(LTFALSE);
		m_eCurState = eState_NoCDKey;

		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox(IDS_NO_CDKEY,&mb);

		return;

	}

	if( pServerDir->IsCDKeyValid( ))
	{
		if (m_sSysMOTD.empty() ||  m_sGameMOTD.empty())
			RequestMOTD();
		else
		{
			m_pJoin->Enable(LTTRUE);
			m_pHost->Enable(LTTRUE);
			m_eCurState = eState_Ready;
		}
		return;
	};

	m_pJoin->Enable(LTFALSE);
	m_pHost->Enable(LTFALSE);
	bool bResult = true;
	
	bResult &= pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_CDKey);
	if (bResult)
	{
		g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_Validate_CDKey)");
		m_eCurState = eState_ValidateCDKey;
		m_pWaitText->SetString(LoadTempString(IDS_VALIDATING));
		m_pWait->Show(LTTRUE);
		SetCapture(m_pWait);

	}
	else
	{
		g_pLTClient->CPrint( "QueueRequest(IServerDirectory::eRequest_Validate_CDKey) : FAILED");
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_MAIN)
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);

		}
		m_pWait->Show(LTFALSE);
		if (GetCapture() == m_pWait)
			SetCapture(NULL);
		m_eCurState = eState_NoCDKey;
	}
}


void CScreenMulti::CreateDMMissionFile()
{
	char path[256];
	std::string sFN = _getcwd(path,sizeof(path));
	sFN += "\\";
	sFN += MISSION_DM_FILE;

	if (CWinUtil::FileExist(sFN.c_str()))
	{
		remove(MISSION_DM_FILE);
	}

	// Get a list of world names and sort them alphabetically

	uint8 nNumPaths = g_pClientButeMgr->GetNumMultiWorldPaths();

	char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

			if (pathBuf[0])
			{
				pFilesArray[i] = g_pLTClient->GetFileList(pathBuf);
			}
			else
			{
				pFilesArray[i] = LTNULL;
			}
		}
	}

	
	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	StringSet filenames;

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

		if (pathBuf[0] && pFilesArray[i])
		{
			sprintf(path, "%s\\", pathBuf);
			FileEntry* ptr = pFilesArray[i];

			while (ptr)
			{
				if (ptr->m_Type == TYPE_FILE)
				{
					if (strnicmp(ptr->m_pBaseFilename,"DM_",3) == 0 || strnicmp(ptr->m_pBaseFilename,"DD_",3) == 0)
					{
						SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
						pBaseName = strtok (strBaseName, ".");
						pBaseExt = strtok (NULL, "\0");
						if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
						{
							char szString[512];
							sprintf(szString, "%s%s", path, pBaseName);

							// add this to the array
							filenames.insert(szString);
						}
					}
				}

				ptr = ptr->m_pNext;
			}

			g_pLTClient->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);


	int index = 0;
	char szLabel[256];
	StringSet::iterator iter = filenames.begin();

	CDMButeMgr buteMgr;
	char szTmp[16];
	char szString[512];

	while (iter != filenames.end())
	{
		bool bDefaultWeapons = false;

		sprintf(szLabel,"Mission%d",index);
							
		sprintf(szString, "\"%s\"", (*iter).c_str());
		CWinUtil::WinWritePrivateProfileString( szLabel, "Level0", szString, sFN.c_str());

		std::string sCfg = (*iter);
		sCfg += ".cfg";

		if (buteMgr.Init(sCfg.c_str()))
		{
			MISSION mission;
			mission.Init(*buteMgr.GetButeMgr(),"Mission");

			if (mission.nNameId > 0)
			{
				sprintf(szTmp,"%d",mission.nNameId);
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameId", szTmp, sFN.c_str());
			}

			if (!mission.sName.empty())
			{
				sprintf(szString, "\"%s\"", mission.sName.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameStr", szString, sFN.c_str());
			}

			if (!mission.sPhoto.empty())
			{
				sprintf(szString, "\"%s\"", mission.sPhoto.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "Photo", szString, sFN.c_str());
			}

			if (mission.nNumDefaultWeapons)
			{
				bDefaultWeapons = true;
				std::string sDef = "\"";
				for (int w = 0; w < mission.nNumDefaultWeapons; w++)
				{
					if (w > 0)
						sDef += ",";
					sDef += g_pWeaponMgr->GetWeapon(mission.aDefaultWeapons[w])->szName;
				}
				sDef += "\"";
				CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", sDef.c_str(), sFN.c_str());
			}

			buteMgr.Term();
		}

		if (!bDefaultWeapons)
		{
			sprintf(szString, "\"%s\"", g_pWeaponMgr->GetMPDefaultWeapons());
			CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", szString, sFN.c_str());
		}

		++index;
		iter++;
	}

	
	// Flush the file. (if anything was added)
	if (index > 0)
	{
		CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, sFN.c_str());
	}

	filenames.clear();
}

/*
void CScreenMulti::CreateTDMMissionFile()
{
	char path[256];
	std::string sFN = _getcwd(path,sizeof(path));
	sFN += "\\";
	sFN += MISSION_TDM_FILE;

	if (CWinUtil::FileExist(sFN.c_str()))
	{
		remove(MISSION_TDM_FILE);
	}

	// Get a list of world names and sort them alphabetically

	uint8 nNumPaths = g_pClientButeMgr->GetNumMultiWorldPaths();

	char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

			if (pathBuf[0])
			{
				pFilesArray[i] = g_pLTClient->GetFileList(pathBuf);
			}
			else
			{
				pFilesArray[i] = LTNULL;
			}
		}
	}

	
	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	StringSet filenames;

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

		if (pathBuf[0] && pFilesArray[i])
		{
			sprintf(path, "%s\\", pathBuf);
			FileEntry* ptr = pFilesArray[i];

			while (ptr)
			{
				if (ptr->m_Type == TYPE_FILE)
				{
					if (strnicmp(ptr->m_pBaseFilename,"DM_",3) == 0 || strnicmp(ptr->m_pBaseFilename,"DD_",3) == 0)
					{
						SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
						pBaseName = strtok (strBaseName, ".");
						pBaseExt = strtok (NULL, "\0");
						if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
						{
							char szString[512];
							sprintf(szString, "%s%s", path, pBaseName);

							// add this to the array
							filenames.insert(szString);
						}
					}
				}

				ptr = ptr->m_pNext;
			}

			g_pLTClient->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);


	int index = 0;
	char szLabel[256];
	StringSet::iterator iter = filenames.begin();

	CDMButeMgr buteMgr;
	char szTmp[16];
	char szString[512];

	while (iter != filenames.end())
	{
		bool bDefaultWeapons = false;

		sprintf(szLabel,"Mission%d",index);
							
		sprintf(szString, "\"%s\"", (*iter).c_str());
		CWinUtil::WinWritePrivateProfileString( szLabel, "Level0", szString, sFN.c_str());

		std::string sCfg = (*iter);
		sCfg += ".cfg";

		if (buteMgr.Init(sCfg.c_str()))
		{
			MISSION mission;
			mission.Init(*buteMgr.GetButeMgr(),"Mission");

			if (mission.nNameId > 0)
			{
				sprintf(szTmp,"%d",mission.nNameId);
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameId", szTmp, sFN.c_str());
			}

			if (!mission.sName.empty())
			{
				sprintf(szString, "\"%s\"", mission.sName.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameStr", szString, sFN.c_str());
			}

			if (!mission.sPhoto.empty())
			{
				sprintf(szString, "\"%s\"", mission.sPhoto.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "Photo", szString, sFN.c_str());
			}

			if (mission.nNumDefaultWeapons)
			{
				bDefaultWeapons = true;
				std::string sDef = "\"";
				for (int w = 0; w < mission.nNumDefaultWeapons; w++)
				{
					if (w > 0)
						sDef += ",";
					sDef += g_pWeaponMgr->GetWeapon(mission.aDefaultWeapons[w])->szName;
				}
				sDef += "\"";
				CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", sDef.c_str(), sFN.c_str());
			}

			buteMgr.Term();
		}

		if (!bDefaultWeapons)
		{
			sprintf(szString, "\"%s\"", g_pWeaponMgr->GetMPDefaultWeapons());
			CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", szString, sFN.c_str());
		}

		++index;
		iter++;
	}

	
	// Flush the file. (if anything was added)
	if (index > 0)
	{
		CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, sFN.c_str());
	}

	filenames.clear();
}
*/

void CScreenMulti::CreateDDMissionFile()
{
	char path[256];
	std::string sFN = _getcwd(path,sizeof(path));
	sFN += "\\";
	sFN += MISSION_DD_FILE;

	if (CWinUtil::FileExist(sFN.c_str()))
	{
		remove(MISSION_DD_FILE);
	}

	// Get a list of world names and sort them alphabetically

	uint8 nNumPaths = g_pClientButeMgr->GetNumMultiWorldPaths();

	char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

			if (pathBuf[0])
			{
				pFilesArray[i] = g_pLTClient->GetFileList(pathBuf);
			}
			else
			{
				pFilesArray[i] = LTNULL;
			}
		}
	}

	
	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	StringSet filenames;

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf),LTFALSE);

		if (pathBuf[0] && pFilesArray[i])
		{
			sprintf(path, "%s\\", pathBuf);
			FileEntry* ptr = pFilesArray[i];

			while (ptr)
			{
				if (ptr->m_Type == TYPE_FILE)
				{
					if (strnicmp(ptr->m_pBaseFilename,"DD_",3) == 0)
					{
						SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
						pBaseName = strtok (strBaseName, ".");
						pBaseExt = strtok (NULL, "\0");
						if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
						{
							char szString[512];
							sprintf(szString, "%s%s", path, pBaseName);

							// add this to the array
							filenames.insert(szString);
						}
					}
				}

				ptr = ptr->m_pNext;
			}

			g_pLTClient->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);


	int index = 0;
	char szLabel[256];
	StringSet::iterator iter = filenames.begin();

	CDMButeMgr buteMgr;
	char szTmp[16];
	char szString[512];

	while (iter != filenames.end())
	{
		bool bDefaultWeapons = false;

		sprintf(szLabel,"Mission%d",index);
							
		sprintf(szString, "\"%s\"", (*iter).c_str());
		CWinUtil::WinWritePrivateProfileString( szLabel, "Level0", szString, sFN.c_str());

		std::string sCfg = (*iter);
		sCfg += ".cfg";

		if (buteMgr.Init(sCfg.c_str()))
		{
			MISSION mission;
			mission.Init(*buteMgr.GetButeMgr(),"Mission");

			if (mission.nNameId > 0)
			{
				sprintf(szTmp,"%d",mission.nNameId);
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameId", szTmp, sFN.c_str());
			}

			if (!mission.sName.empty())
			{
				sprintf(szString, "\"%s\"", mission.sName.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "NameStr", szString, sFN.c_str());
			}

			if (!mission.sPhoto.empty())
			{
				sprintf(szString, "\"%s\"", mission.sPhoto.c_str());
				CWinUtil::WinWritePrivateProfileString( szLabel, "Photo", szString, sFN.c_str());
			}

			if (mission.nNumDefaultWeapons)
			{
				bDefaultWeapons = true;
				std::string sDef = "\"";
				for (int w = 0; w < mission.nNumDefaultWeapons; w++)
				{
					if (w > 0)
						sDef += ",";
					sDef += g_pWeaponMgr->GetWeapon(mission.aDefaultWeapons[w])->szName;
				}
				sDef += "\"";
				CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", sDef.c_str(), sFN.c_str());
			}

			buteMgr.Term();
		}

		if (!bDefaultWeapons)
		{
			sprintf(szString, "\"%s\"", g_pWeaponMgr->GetMPDefaultWeapons());
			CWinUtil::WinWritePrivateProfileString( szLabel, "DefaultWeapons", szString, sFN.c_str());
		}

		++index;
		iter++;
	}

	
	// Flush the file. (if anything was added)
	if (index > 0)
	{
		CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, sFN.c_str());
	}

	filenames.clear();
}