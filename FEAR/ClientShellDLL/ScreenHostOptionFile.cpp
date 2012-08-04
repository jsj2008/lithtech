// ScreenHostOptionFile.cpp: implementation of the CScreenHostOptionFile class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ScreenHostOptionFile.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "NetDefs.h"
#include "ClientConnectionMgr.h"
#include "WinUtil.h"
#include <IO.h>
#include "sys/win/mpstrconv.h"
#include "GameClientShell.h"
#include "GameModeMgr.h"
#include "resourceextensions.h"
#include "ltfileoperations.h"
#include "iltfilemgr.h"
#include "HostOptionsMapMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	std::wstring	sOldFN;

	bool bCreate = false;

	void DeleteCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenHostOptionFile *pThisScreen = (CScreenHostOptionFile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_DELETE);
	}
	void CreateCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenHostOptionFile *pThisScreen = (CScreenHostOptionFile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_CREATE);
	}
	void LoadCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenHostOptionFile *pThisScreen = (CScreenHostOptionFile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_LOAD);
	}
	void EditCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenHostOptionFile *pThisScreen = (CScreenHostOptionFile *)pUserData;
		if (bReturn && pThisScreen)
		{
			pThisScreen->SendCommand(CMD_EDIT,(uint32)pData,0);
		}
	};

	int kDlgHt = 240;
	int kDlgWd = 240;
	uint32 nCommand = 0;
	std::string sListFont;
	uint8 nListFontSz = 14;

}

CScreenHostOptionFile::CScreenHostOptionFile()
{
	m_pCurrent = NULL;
	m_pLoad = NULL;
	m_pDelete = NULL;
	m_pRename = NULL;
}

CScreenHostOptionFile::~CScreenHostOptionFile()
{
}

// Build the screen
bool CScreenHostOptionFile::Build()
{
	// Set the title's text
	CreateTitle("IDS_TITLE_HOST_OPTIONFILE");

	// Get edit controls position and create it.
//	m_DefaultPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"MissionNamePos");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	m_pCurrent = AddTextItem(L" ", cs, true);

	cs.nCommandID = CMD_CREATE;
	cs.szHelpID = "IDS_HELP_OPTIONFILE_CREATE";
	AddTextItem("IDS_CREATE", cs );

	cs.nCommandID = CMD_LOAD;
	cs.szHelpID = "IDS_HELP_OPTIONFILE_LOAD";
	AddTextItem("IDS_LOAD", cs);

	cs.nCommandID = CMD_DELETE;
	cs.szHelpID = "IDS_HELP_OPTIONFILE_DELETE";
	m_pDelete = AddTextItem("IDS_DELETE", cs);

	LTRect2n dlgRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenDialogRect);
	std::string sDlgFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenDialogFont);
	uint8 nDlgFontSz = ( uint8 )g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenDialogSize);

	sListFont = g_pLayoutDB->GetListFont(m_hLayout,0);
	nListFontSz = g_pLayoutDB->GetListSize(m_hLayout,0);

	CLTGUICtrl_create dcs;
	dcs.rnBaseRect = dlgRect;
	m_pDlg = debug_new(CLTGUIWindow);
	m_pDlg->Create(TextureReference(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenDialogFrame)),dcs);

	LTVector2n tmp(8,8);
	cs.nCommandID = NULL;
	cs.szHelpID = "";
	CLTGUITextCtrl *pCtrl = CreateTextItem("IDS_OPTIONFILE_LIST", cs, true, sDlgFont.c_str(), nDlgFontSz);
	m_pDlg->AddControl(pCtrl, tmp);

	tmp.y += 24;

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect.m_vMin.Init();
	listCs.rnBaseRect.Right() = kDlgWd - 48;
	listCs.rnBaseRect.Bottom() = kDlgWd - 64;
	listCs.bArrows = true;
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	m_pListCtrl = CreateList(listCs);
	if (m_pListCtrl)
	{
		m_pListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
		m_pListCtrl->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
		m_pListCtrl->Show(true);

		m_pDlg->AddControl(m_pListCtrl, tmp);
	}


	cs.nCommandID = CMD_CANCEL;
	cs.szHelpID = "";
	cs.rnBaseRect.Bottom() = nDlgFontSz;
	cs.rnBaseRect.Right() = kDlgWd / 2;

	pCtrl = CreateTextItem("IDS_CANCEL", cs, false, sDlgFont.c_str(), nDlgFontSz);
	pCtrl->SetAlignment(kCenter);
	tmp.x = kDlgWd / 2;
	tmp.y = (kDlgHt - nDlgFontSz) - 8;
	m_pDlg->AddControl(pCtrl, tmp);

	AddControl(m_pDlg);

	m_pDlg->Show(false);

	m_pDlg->SetSelection(1);

	UseBack(true);


	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHostOptionFile::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_OK:
		{
			m_pDlg->Show(false);
			SetCapture(NULL);
			HandleDlgCommand(nCommand,(uint16)dwParam1);
			SetSelection(1);
		}
		break;
	case CMD_CANCEL:
		{
			m_pDlg->Show(false);
			SetCapture(NULL);
			SetSelection(1);
		}
		break;

	case CMD_LOAD:
		{
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(true);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_CONFIRM:
		{
			uint16 nIndex = (uint16)dwParam1;
			if (dwParam2 == CMD_DELETE)
			{
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					WStringSet::iterator iter = m_List.begin();
					for (int i = 0; i < nIndex && iter != m_List.end(); ++i, ++iter);

					if (iter != m_List.end())
					{
						Delete(iter->c_str());
					}
				}
			}
			else if (dwParam2 == CMD_LOAD)
			{
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					WStringSet::iterator iter = m_List.begin();
					for (int i = 0; i < nIndex && iter != m_List.end(); ++i, ++iter);

					if (iter != m_List.end())
					{
						SetName( (*iter).c_str());
					}

				}
			}
			else if (dwParam2 == CMD_CREATE)
			{
				std::wstring sNew = g_pProfileMgr->GetCurrentProfile()->m_sFriendlyName.c_str( );
				bCreate = true;

				//show edit box here	
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pUserData = this;
				mb.pString = sNew.c_str();
				mb.nMaxChars = MAX_PROFILE_NAME;
//				mb.eInput = kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox("IDS_ENTER_OPTIONFILE_NAME",&mb);
			}
		}
		break;
	case CMD_DELETE:
		{
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(true);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_CREATE:
		{
			SendCommand(CMD_CONFIRM,0,CMD_CREATE);
		}
		break;
	case CMD_RENAME:
		{
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(true);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_EDIT:
		{
			std::wstring name = (wchar_t *)dwParam1;

			WStringSet::iterator iter = m_List.find(name);
			if (iter != m_List.end())
				name = (*iter);

			if (bCreate)
			{
				New( name.c_str() );
			}
			else
			{
				Rename( sOldFN.c_str(), name.c_str() );
			}
			
		}
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CScreenHostOptionFile::Escape()
{
	if (m_pDlg->IsVisible())
	{
		m_pDlg->Show(false);
		SetCapture(NULL);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else
		CBaseScreen::Escape();
}

bool	CScreenHostOptionFile::OnLButtonUp(int x, int y)
{
	return CBaseScreen::OnLButtonUp(x,y);
}

bool	CScreenHostOptionFile::OnRButtonUp(int x, int y)
{
	return CBaseScreen::OnRButtonUp(x,y);
}

void CScreenHostOptionFile::CreateFileList()
{
	// Empty the list
	m_pListCtrl->RemoveAll();
	m_List.clear();

	// Get new stuff
	LTFINDFILEINFO file;
	LTFINDFILEHANDLE hFile;

	// Create directory search string.
	char szDirectory[MAX_PATH*2];
	LTSNPrintF( szDirectory, LTARRAYSIZE( szDirectory ), "%s*.txt", GameModeMgr::Instance( ).GetOptionsFolder( ));

	// Create the title of the default option file, which we won't show in the list.
	char szExample[MAX_PATH*2];
	LTFileOperations::SplitPath( SERVEROPTIONS_EXAMPLE, NULL, szExample, NULL );
	CResExtUtil::StripFileExtension( szExample, LTARRAYSIZE( szExample ));

	// find first file
	if( LTFileOperations::FindFirst( szDirectory, hFile, &file ))
	{
		do
		{
			CResExtUtil::StripFileExtension( file.name, LTARRAYSIZE( file.name ));

			if( !LTStrIEquals( szExample, file.name ))
			{
				const wchar_t* wszFriendlyName = CHostOptionsMapMgr::Instance().GetFriendlyNameFromFileName( file.name );
				if( wszFriendlyName )
					m_List.insert( wszFriendlyName );
				else
					m_List.insert( MPA2W(file.name).c_str() );
			}
		}
		while( LTFileOperations::FindNext( hFile, &file ));
	}
	LTFileOperations::FindClose( hFile );

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kDlgWd - 48,nListFontSz);
	cs.nCommandID = CMD_OK;

	// Get the current file title.
	wchar_t wszCurrentName[MAX_PATH*2];
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	const wchar_t* wszFriendlyName = CHostOptionsMapMgr::Instance().GetFriendlyNameFromFileName( pProfile->m_sServerOptionsFile.c_str() );
	if( wszFriendlyName )
		LTStrCpy( wszCurrentName, wszFriendlyName, LTARRAYSIZE( wszCurrentName ));
	else
		LTStrCpy( wszCurrentName, MPA2W( pProfile->m_sServerOptionsFile.c_str( )).c_str( ), LTARRAYSIZE( wszCurrentName ));

	// add files to the list control
	for (WStringSet::iterator iter = m_List.begin(); iter != m_List.end(); ++iter)
	{
		CLTGUITextCtrl* pTextCtrl = NULL;
		uint16 ndx = 0;
		pTextCtrl = CreateTextItem(iter->c_str(), cs, false, sListFont.c_str(), nListFontSz);
		ndx = m_pListCtrl->AddControl(pTextCtrl);
		pTextCtrl->SetParam1(ndx);

		if( LTStrEquals( iter->c_str( ), wszCurrentName ))
			m_pListCtrl->SetSelection(ndx);
	}
}

void CScreenHostOptionFile::UpdateName()
{
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();

	wchar_t wszBuffer[MAX_PATH*2];

	const wchar_t* wszFriendlyName = CHostOptionsMapMgr::Instance().GetFriendlyNameFromFileName( pProfile->m_sServerOptionsFile.c_str() );
	if( wszFriendlyName )
		FormatString( "IDS_CURRENT_OPTIONFILE", wszBuffer, LTARRAYSIZE(wszBuffer), wszFriendlyName);
	else
		FormatString( "IDS_CURRENT_OPTIONFILE", wszBuffer, LTARRAYSIZE(wszBuffer), MPA2W(pProfile->m_sServerOptionsFile.c_str( )).c_str( ));
	m_pCurrent->SetString( wszBuffer );
}

void CScreenHostOptionFile::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		UpdateName();
		CreateFileList();
	}
	else
	{
		UpdateData( true );
		g_pProfileMgr->GetCurrentProfile()->Save( );
	}

	CBaseScreen::OnFocus(bFocus);
}



void CScreenHostOptionFile::HandleDlgCommand(uint32 dwCommand, uint16 nIndex)
{
	switch (dwCommand)
	{
	case CMD_LOAD:
		{
			if (nIndex < m_pListCtrl->GetNumControls()) 
			{
				WStringSet::iterator iter = m_List.begin();
				for (int i = 0; i < nIndex && iter != m_List.end(); ++i, ++iter);

				if (iter != m_List.end())
				{
					SendCommand(CMD_CONFIRM,nIndex,CMD_LOAD);
				}
			}
		}
		break;
	case CMD_DELETE:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = DeleteCallBack,
			mb.pData = (void *)nIndex;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRM_DELETE",&mb);
		}
		break;
	case CMD_RENAME:
		if (nIndex < m_pListCtrl->GetNumControls()) 
		{
			WStringSet::iterator iter = m_List.begin();
			for (int i = 0; i < nIndex && iter != m_List.end(); ++i, ++iter);

			if (iter != m_List.end())
			{
				sOldFN = *iter;
				bCreate = false;
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = iter->c_str( );
				mb.nMaxChars = 16;
				mb.eInput = kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox("IDS_ENTER_OPTIONFILE_NAME",&mb);
			}
		}
		break;
	}

};


void CScreenHostOptionFile::Delete(const wchar_t* pwsName)
{
	char szPath[MAX_PATH*2];

	const char* szFileName = CHostOptionsMapMgr::Instance().GetFileNameFromFriendlyName( pwsName );
	if( szFileName )
		GameModeMgr::Instance( ).GetOptionsFilePath( szFileName, szPath, LTARRAYSIZE( szPath ));
	else
		GameModeMgr::Instance( ).GetOptionsFilePath( MPW2A( pwsName ).c_str( ), szPath, LTARRAYSIZE( szPath ));
		
	LTFileOperations::DeleteFile( szPath );

	CHostOptionsMapMgr::Instance().Delete( pwsName );

	CreateFileList();
}

void CScreenHostOptionFile::New(const wchar_t* pwsName )
{
	if( !LTFileOperations::DirectoryExists( GameModeMgr::Instance( ).GetOptionsFolder( )))
	{
		if( !LTFileOperations::CreateNewDirectory( GameModeMgr::Instance( ).GetOptionsFolder( )))
		{
			//TODO: error message
			return;
		}
	}

	// check if this name exists in the map file
	if( CHostOptionsMapMgr::Instance().IsFriendlyNameMapped(pwsName) )
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox("IDS_OPTIONFILE_EXISTS",&mb);
		return;
	}

	// create a new file name that is in ANSI
	char szFileTitle[64];
	char szPath[MAX_PATH*2];
	for(uint32 nFile=0;;++nFile)
	{
		LTSNPrintF( szFileTitle, LTARRAYSIZE(szFileTitle), "ServerOptions%.4d", nFile );
		GameModeMgr::Instance( ).GetOptionsFilePath( szFileTitle, szPath, LTARRAYSIZE( szPath ));
		if( !LTFileOperations::FileExists(szPath) && 
			!CHostOptionsMapMgr::Instance().IsFileNameMapped(szPath) )
			break;
	}

	// add this combination
	CHostOptionsMapMgr::Instance().Add( szFileTitle, pwsName );

	g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile = szFileTitle;

	CreateFileList();
	Escape( );
}

void CScreenHostOptionFile::Rename(const wchar_t* oldName,const wchar_t* newName)
{
	if (!newName || !newName[0]) return;

	CHostOptionsMapMgr::Instance().Rename( oldName, newName );

	CreateFileList();
}


void CScreenHostOptionFile::SetName(const wchar_t* newName)
{
	const char* szFileName = CHostOptionsMapMgr::Instance().GetFileNameFromFriendlyName( newName );
	if( szFileName )
	{
		if( LTStrEquals( g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile.c_str( ), szFileName ))
			return;

		g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile = szFileName;
	}
	else
	{
		char szNewName[256];
		LTStrCpy( szNewName, MPW2A( newName ).c_str( ), LTARRAYSIZE( szNewName ));

		if( LTStrEquals( g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile.c_str( ), szNewName ))
			return;

		g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile = szNewName;
	}

	UpdateName();
};
