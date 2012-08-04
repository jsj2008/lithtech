//*************************************************************************
//*************************************************************************
//***** MODULE  : MenuCharacterFiles.cpp
//***** PURPOSE : Blood 2 Character Creation Screen
//***** CREATED : 10/11/98
//*************************************************************************
//*************************************************************************

#include "MenuBase.h"
#include "MenuCharacterFiles.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include <stdio.h>
#include <io.h>
#include <direct.h>

//*************************************************************************

#define		MENU_FILE_FIELD_X		50
#define		MENU_FILE_FIELD_Y		300
#define		MENU_FILE_TEXT_OFFSET_X	50
#define		MENU_FILE_TEXT_OFFSET_Y	28

//*************************************************************************

CMenuCharacterFiles::CMenuCharacterFiles()
{
	m_szBackground = "interface\\charscreen\\charbackground.pcx";
	m_bBoxFormat = DTRUE;
	m_hFileField = DNULL;
}

//*************************************************************************

CMenuCharacterFiles::~CMenuCharacterFiles()
{
	// Delete the surface of the file field
	if(m_hFileField)
		{ m_pClientDE->DeleteSurface(m_hFileField); m_hFileField = 0; }
}

//*************************************************************************

void CMenuCharacterFiles::Build()
{
	CMenuBase::Build();

	SetOptionPos(MENU_FILE_FIELD_X + MENU_FILE_TEXT_OFFSET_X, MENU_FILE_FIELD_Y + MENU_FILE_TEXT_OFFSET_Y);
	SetItemSpacing(0);
	SetScrollWrap(DFALSE);

	// Remove all of the menu options
	RemoveAllOptions();

	if(m_nAction == MENU_ACTION_SAVE)
	{
		m_hEdit = AddEditOption("Type the filename:", 0, m_pMainMenus->GetSmallFont(), 135, 16, DNULL);
		m_hEdit->SetText("Default");
	}
	else
		InitFileList("Players");

	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);

	// Delete the surface of the file field
	if(m_hFileField)
		{ m_pClientDE->DeleteSurface(m_hFileField); m_hFileField = 0; }

	m_hFileField = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/dialog.pcx");
}

//*************************************************************************

void CMenuCharacterFiles::InitFileList(char *lpszPath)
{
	// Sanity checks...
	if(!lpszPath) return;
	if(lpszPath[0] == '\0') return;

	// Change to the given direcotry...
	if(chdir(lpszPath) != 0)
		return;

	// Enumerate the available .b2c files and add them to the list box...
	long	hFile;
    struct	_finddata_t fd;

	hFile = _findfirst("*.b2c", &fd);
	if (hFile == -1)
		{ chdir(".."); return; }

	AddTextItemOption(fd.name, MENU_CMD_B2C_FILE, m_pMainMenus->GetSmallFont());

	BOOL bContinue = TRUE;

	while (bContinue)
	{
		if (_findnext(hFile, &fd) != 0)
			bContinue = FALSE;
		else
			AddTextItemOption(fd.name, MENU_CMD_B2C_FILE, m_pMainMenus->GetSmallFont());
	}

	// Restore the directory...
	chdir ("..");
}

//*************************************************************************

void CMenuCharacterFiles::Render(HSURFACE hDestSurf)
{
	// Render the character screen behind this menu
	m_pParentMenu->Render(hDestSurf);

	// Draw the file field box
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hFileField, DNULL, MENU_FILE_FIELD_X, MENU_FILE_FIELD_Y, m_hTransColor);

	// Render the list of options
	m_listOption.EnableBoxFormat(m_bBoxFormat);
	m_listOption.Render(hDestSurf);
}

//*************************************************************************

DDWORD CMenuCharacterFiles::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{	
	CLTGUITextItemCtrl *pCtrl=(CLTGUITextItemCtrl *)m_listOption.GetControl(m_listOption.GetSelectedItem());

	if(pCtrl)
	{				
		if(m_nAction == MENU_ACTION_SAVE)
		{
			(m_pMainMenus->GetCharacterSetup())->SaveB2CFile(m_hEdit->GetText());
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTER);
		}
		else if(m_nAction == MENU_ACTION_LOAD)
		{
			(m_pMainMenus->GetCharacterSetup())->LoadB2CFile(m_pClientDE->GetStringData(pCtrl->GetString(0)));
			(m_pMainMenus->GetCharacterSetup())->UpdateScreenFromStruct();
		}
		else if(m_nAction == MENU_ACTION_DELETE)
		{
			(m_pMainMenus->GetCharacterSetup())->DeleteB2CFile(m_pClientDE->GetStringData(pCtrl->GetString(0)));
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTER);
		}
	}

	return 0;
}