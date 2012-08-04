//*************************************************************************
//*************************************************************************
//***** MODULE  : MenuCharacter.cpp
//***** PURPOSE : Blood 2 Character Creation Screen
//***** CREATED : 10/11/98
//*************************************************************************
//*************************************************************************

#include "MenuBase.h"
#include "MenuCharacter.h"
#include "MainMenus.h"
#include "MenuCommands.h"
#include "BloodClientShell.h"
#include "WeaponDefs.h"
#include "VKDefs.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>

//*************************************************************************

CharacterMenuInfo g_CharacterMenuInfo[MAX_CHARACTER_INFO] =
{
	{ "caleb_single.pcx", CHARACTER_CALEB, MULTIPLAY_SKIN_NORMAL, },
	{ "caleb_red.pcx", CHARACTER_CALEB, MULTIPLAY_SKIN_RED, },
	{ "caleb_blue.pcx", CHARACTER_CALEB, MULTIPLAY_SKIN_BLUE, },
	{ "caleb_green.pcx", CHARACTER_CALEB, MULTIPLAY_SKIN_GREEN, },
	{ "caleb_yellow.pcx", CHARACTER_CALEB, MULTIPLAY_SKIN_YELLOW, },
	{ "ophelia_single.pcx", CHARACTER_OPHELIA, MULTIPLAY_SKIN_NORMAL, },
	{ "ophelia_red.pcx", CHARACTER_OPHELIA, MULTIPLAY_SKIN_RED, },
	{ "ophelia_blue.pcx", CHARACTER_OPHELIA, MULTIPLAY_SKIN_BLUE, },
	{ "ophelia_green.pcx", CHARACTER_OPHELIA, MULTIPLAY_SKIN_GREEN, },
	{ "ophelia_yellow.pcx", CHARACTER_OPHELIA, MULTIPLAY_SKIN_YELLOW, },
	{ "ishmael_single.pcx", CHARACTER_ISHMAEL, MULTIPLAY_SKIN_NORMAL, },
	{ "ishmael_red.pcx", CHARACTER_ISHMAEL, MULTIPLAY_SKIN_RED, },
	{ "ishmael_blue.pcx", CHARACTER_ISHMAEL, MULTIPLAY_SKIN_BLUE, },
	{ "ishmael_green.pcx", CHARACTER_ISHMAEL, MULTIPLAY_SKIN_GREEN, },
	{ "ishmael_yellow.pcx", CHARACTER_ISHMAEL, MULTIPLAY_SKIN_YELLOW, },
	{ "gabby_single.pcx", CHARACTER_GABREILLA, MULTIPLAY_SKIN_NORMAL, },
	{ "gabby_red.pcx", CHARACTER_GABREILLA, MULTIPLAY_SKIN_RED, },
	{ "gabby_blue.pcx", CHARACTER_GABREILLA, MULTIPLAY_SKIN_BLUE, },
	{ "gabby_green.pcx", CHARACTER_GABREILLA, MULTIPLAY_SKIN_GREEN, },
	{ "gabby_yellow.pcx", CHARACTER_GABREILLA, MULTIPLAY_SKIN_YELLOW, },

#ifdef _ADDON
	{ "interface_AO\\m_cultist1.pcx", CHARACTER_M_CULTIST, MULTIPLAY_SKIN_NORMAL, },
	{ "interface_AO\\clown2.pcx", CHARACTER_M_CULTIST, MULTIPLAY_SKIN_RED, },
	{ "interface_AO\\clown1.pcx", CHARACTER_M_CULTIST, MULTIPLAY_SKIN_BLUE, },
	{ "interface_AO\\clown3.pcx", CHARACTER_M_CULTIST, MULTIPLAY_SKIN_GREEN, },
	{ "interface_AO\\m_cultist3.pcx", CHARACTER_M_CULTIST, MULTIPLAY_SKIN_YELLOW, },
	{ "interface_AO\\f_cultist1.pcx", CHARACTER_F_CULTIST, MULTIPLAY_SKIN_NORMAL, },
	{ "interface_AO\\souldrudge1.pcx", CHARACTER_SOULDRUDGE, MULTIPLAY_SKIN_NORMAL, },
	{ "interface_AO\\prophet.pcx", CHARACTER_PROPHET, MULTIPLAY_SKIN_NORMAL, },
#endif

};

//*************************************************************************

CMenuCharacter::CMenuCharacter()
{
	m_szBackground = "interface\\charscreen\\charbackground.pcx";
	m_bBoxFormat = DFALSE;

	m_hCharacterPic = DNULL;

	m_PlayerIndex = 0;
	m_ExtraPoints = 4;

	m_Player.dwCharacter = g_CharacterMenuInfo[m_PlayerIndex].nCharacter;
	m_Player.dwColor = g_CharacterMenuInfo[m_PlayerIndex].nColor;
	m_Player.dwStrength = DEFAULT_ATTRIBUTE - 1;
	m_Player.dwSpeed = DEFAULT_ATTRIBUTE - 1;
	m_Player.dwResist = DEFAULT_ATTRIBUTE - 1;
	m_Player.dwFocus = DEFAULT_ATTRIBUTE - 1;
	m_Player.dwWeap1 = WEAP_NONE;
	m_Player.dwWeap2 = WEAP_NONE;
	m_Player.dwWeap3 = WEAP_NONE;
	m_Player.dwWeap4 = WEAP_NONE;
	m_Player.dwWeap5 = WEAP_NONE;
	m_Player.dwWeap6 = WEAP_NONE;
	m_Player.dwWeap7 = WEAP_NONE;
	m_Player.dwWeap8 = WEAP_NONE;
	m_Player.dwWeap9 = WEAP_NONE;

	m_hRightArrow = DNULL;

	m_hStats[0] = DNULL;
	m_hStats[1] = DNULL;
	m_hStats[2] = DNULL;
	m_hStats[3] = DNULL;

	m_hExtraCtrl = DNULL;
	m_hExtraNumCtrl = DNULL;

	m_hWeaponsCtrl = DNULL;

	for(int i = 0; i < 10; i++)
	{
		m_hWeaponNums[i] = DNULL;
		m_hWeapons[i] = DNULL;
	}

	m_bSwitchResolutions=DFALSE;
	m_bGoingToFilesMenu=DFALSE;

	m_nOldScreenWidth=640;
	m_nOldScreenHeight=480;
}

//*************************************************************************

CMenuCharacter::~CMenuCharacter()
{
	if(m_hRightArrow)	delete m_hRightArrow;
	if(m_hExtraCtrl)	delete m_hExtraCtrl;
	if(m_hWeaponsCtrl)	delete m_hWeaponsCtrl;

	if(m_hExtraNumCtrl)	delete m_hExtraNumCtrl;

	for(int i = 0; i < 4; i++)
		if(m_hStats[i])		delete m_hStats[i];

	for(i = 0; i < 10; i++)
	{
		if(m_hWeaponNums[i])	delete m_hWeaponNums[i];
		if(m_hWeapons[i])		delete m_hWeapons[i];
	}
}

//*************************************************************************

void CMenuCharacter::Build()
{
	CMenuBase::Build();

	AddFadeItemOption("interface\\charscreen\\arrows\\leftarrow_", 6, "LEFT ARROW", 0, 60, 380);

	AddFadeItemOption("interface\\charscreen\\titles\\strength_", 6, "STRENGTH", 0, 206, 130);
	AddFadeItemOption("interface\\charscreen\\titles\\resistance_", 6, "RESISTANCE", 0, 204, 172);
	AddFadeItemOption("interface\\charscreen\\titles\\speed_", 6, "SPEED", 0, 206, 214);
	AddFadeItemOption("interface\\charscreen\\titles\\focus_", 6, "FOCUS", 0, 206, 256);

	AddFadeItemOption("interface\\charscreen\\fields\\weapon2_", 6, "WEAPON2", 0, 380, 134);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon3_", 6, "WEAPON3", 0, 380, 158);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon4_", 6, "WEAPON4", 0, 380, 181);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon5_", 6, "WEAPON5", 0, 380, 204);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon6_", 6, "WEAPON6", 0, 380, 226);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon7_", 6, "WEAPON7", 0, 380, 249);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon8_", 6, "WEAPON8", 0, 380, 272);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon9_", 6, "WEAPON9", 0, 380, 295);
	AddFadeItemOption("interface\\charscreen\\fields\\weapon10_", 6, "WEAPON10", 0, 380, 319);

	AddFadeItemOption("interface\\charscreen\\titles\\save_", 6, "SAVE", MENU_CMD_SAVE_CHARACTER, 517, 356);
	AddFadeItemOption("interface\\charscreen\\titles\\load_", 6, "LOAD", MENU_CMD_LOAD_CHARACTER, 509, 382);
	AddFadeItemOption("interface\\charscreen\\titles\\delete_", 6, "DELETE", MENU_CMD_DELETE_CHARACTER, 499, 407);

	BuildExtraCtrls();

	ChangeCharacterPic(0);

	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
}

//*************************************************************************

void CMenuCharacter::BuildExtraCtrls()
{
	m_hRightArrow = InitFadeItemCtrl("interface\\charscreen\\arrows\\rightarrow_", 6, "RIGHT ARROW", 0, 188, 377);
	m_hExtraCtrl = InitFadeItemCtrl("interface\\charscreen\\titles\\extrapts_", 6, "EXTRA POINTS", 0, 206, 295);
	m_hWeaponsCtrl = InitFadeItemCtrl("interface\\charscreen\\titles\\weapons_", 6, "WEAPONS", 0, 491, 95);

	m_hExtraNumCtrl = InitNumberCtrl(0, 8, m_ExtraPoints, 0, 330, 313);

	m_hStats[0] = InitNumberCtrl(0, 5, m_Player.dwStrength, 0, 330, 145);
	m_hStats[1] = InitNumberCtrl(0, 5, m_Player.dwResist, 0, 330, 187);
	m_hStats[2] = InitNumberCtrl(0, 5, m_Player.dwSpeed, 0, 330, 229);
	m_hStats[3] = InitNumberCtrl(0, 5, m_Player.dwFocus, 0, 330, 271);

	m_hWeapons[0] = InitWeaponCtrl(WEAP_MELEE, 0, 410, 116, DFALSE);
	m_hWeapons[1] = InitWeaponCtrl(0, 0, 410, 139, DTRUE);
	m_hWeapons[2] = InitWeaponCtrl(0, 0, 410, 162, DTRUE);
	m_hWeapons[3] = InitWeaponCtrl(0, 0, 410, 185, DTRUE);
	m_hWeapons[4] = InitWeaponCtrl(0, 0, 410, 208, DTRUE);
	m_hWeapons[5] = InitWeaponCtrl(0, 0, 410, 231, DTRUE);
	m_hWeapons[6] = InitWeaponCtrl(0, 0, 410, 254, DTRUE);
	m_hWeapons[7] = InitWeaponCtrl(0, 0, 410, 277, DTRUE);
	m_hWeapons[8] = InitWeaponCtrl(0, 0, 410, 300, DTRUE);
	m_hWeapons[9] = InitWeaponCtrl(0, 0, 410, 323, DTRUE);

	m_hWeaponNums[0] = InitWeaponNumCtrl(WEAP_MELEE, 0, 390, 116, DFALSE);
	m_hWeaponNums[1] = InitWeaponNumCtrl(0, 0, 390, 139, DTRUE);
	m_hWeaponNums[2] = InitWeaponNumCtrl(0, 0, 390, 162, DTRUE);
	m_hWeaponNums[3] = InitWeaponNumCtrl(0, 0, 390, 185, DTRUE);
	m_hWeaponNums[4] = InitWeaponNumCtrl(0, 0, 390, 208, DTRUE);
	m_hWeaponNums[5] = InitWeaponNumCtrl(0, 0, 390, 231, DTRUE);
	m_hWeaponNums[6] = InitWeaponNumCtrl(0, 0, 390, 254, DTRUE);
	m_hWeaponNums[7] = InitWeaponNumCtrl(0, 0, 390, 277, DTRUE);
	m_hWeaponNums[8] = InitWeaponNumCtrl(0, 0, 390, 300, DTRUE);
	m_hWeaponNums[9] = InitWeaponNumCtrl(0, 0, 390, 323, DTRUE);
}

//*************************************************************************

CLTGUIFadeItemCtrl* CMenuCharacter::InitFadeItemCtrl(char *lpszOptionSurfPrefix, int nSurfaces, char *lpszOptionText, DWORD dwCommandID, int x, int y)
{
	char szTempString[256];

	assert(nSurfaces > 0);

	HSURFACE *pSurfArray=new HSURFACE[nSurfaces];

	// Load the option surfaces
	int i;
	for ( i=0; i < nSurfaces; i++ )
	{		
		// Add the extra zero if we are under 10 (index < 9)
		if ( i+1 < 10 )
			sprintf(szTempString, "%s0%d.pcx", lpszOptionSurfPrefix, i+1);
		else
			sprintf(szTempString, "%s%d.pcx", lpszOptionSurfPrefix, i+1);

		pSurfArray[i]=m_pClientDE->CreateSurfaceFromBitmap(szTempString);
	}

	// Load the disabled surface
	sprintf(szTempString, "%sdis.pcx", lpszOptionSurfPrefix);
	HSURFACE hDisabledSurf=m_pClientDE->CreateSurfaceFromBitmap(szTempString);

	// Create the new menu option
	CLTGUIFadeItemCtrl *pOption=new CLTGUIFadeItemCtrl;
	if ( !pOption->Create(m_pClientDE, dwCommandID, pSurfArray, nSurfaces, hDisabledSurf, TRUE, DNULL) )
	{
		delete []pSurfArray;
		delete pOption;

		return DNULL;
	}

	pOption->SetPos(x, y);

	delete []pSurfArray;

	return pOption;
}

//*************************************************************************

CLTGUITextItemCtrl* CMenuCharacter::InitNumberCtrl(int low, int high, int index, DWORD dwCommandID, int x, int y)
{
	char	str[5];

	itoa(low, str, 10);

	HSTRING hString=m_pClientDE->CreateString(str);

	// Create the new menu option
	CLTGUITextItemCtrl *pOption = new CLTGUITextItemCtrl;
	if(!pOption->Create(m_pClientDE, dwCommandID, hString, m_pMainMenus->GetSmallFont(), 1, DTRUE, DNULL))
	{
		m_pClientDE->FreeString(hString);
		delete pOption;
		return DNULL;
	}
	m_pClientDE->FreeString(hString);

	for(int i = low + 1; i <= high; i++)
	{		
		itoa(i, str, 10);

		hString=m_pClientDE->CreateString(str);
		pOption->AddString(hString);
		m_pClientDE->FreeString(hString);
	}

	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0));
	pOption->SetSelIndex(index);
	pOption->SetPos(x, y);

	return pOption;
}

//*************************************************************************

CLTGUITextItemCtrl* CMenuCharacter::InitWeaponCtrl(int index, DWORD dwCommandID, int x, int y, DBOOL enable)
{
	// Create the new menu option
	HSTRING hString=m_pClientDE->CreateString("NONE");

	CLTGUITextItemCtrl *pOption = new CLTGUITextItemCtrl;
	if(!pOption->Create(m_pClientDE, dwCommandID, hString, m_pMainMenus->GetSmallFont(), 1, DTRUE, DNULL))
	{
		m_pClientDE->FreeString(hString);
		delete pOption;
		return DNULL;
	}
	m_pClientDE->FreeString(hString);

	for(int i = 1; i <= WEAP_LASTPLAYERWEAPON; i++)
	{
		hString = m_pClientDE->FormatString(g_WeaponDefaults[i - 1].m_nWeaponNameID);
		pOption->AddString(hString);
		m_pClientDE->FreeString(hString);
	}

	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0));
	pOption->SetSelIndex(index);
	pOption->SetPos(x, y);
	pOption->Enable(enable);

	return pOption;
}

//*************************************************************************

CLTGUITextItemCtrl* CMenuCharacter::InitWeaponNumCtrl(int index, DWORD dwCommandID, int x, int y, DBOOL enable)
{
	char	str[5];

	HSTRING hString=m_pClientDE->CreateString("0");

	// Create the new menu option
	CLTGUITextItemCtrl *pOption = new CLTGUITextItemCtrl;
	if(!pOption->Create(m_pClientDE, dwCommandID, hString, m_pMainMenus->GetSmallFont(), 1, DTRUE, DNULL))
	{
		m_pClientDE->FreeString(hString);
		delete pOption;
		return DNULL;
	}
	m_pClientDE->FreeString(hString);

	for(int i = 1; i <= WEAP_LASTPLAYERWEAPON; i++)
	{
		itoa(g_WeaponDefaults[i - 1].m_dwStrengthReq, str, 10);

		hString=m_pClientDE->CreateString(str);
		pOption->AddString(hString);
		m_pClientDE->FreeString(hString);
	}

	pOption->SetColor(SETRGB(220,190,170), SETRGB(125,30,0));
	pOption->SetSelIndex(index);
	pOption->SetPos(x, y);
	pOption->Enable(enable);

	return pOption;
}

//*************************************************************************

void CMenuCharacter::Render(HSURFACE hDestSurf)
{
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hCharacterPic, DNULL, 78, 119, m_hTransColor);

	// Render the list of options
	m_listOption.EnableBoxFormat(m_bBoxFormat);
	m_listOption.Render(hDestSurf);

	CheckSelectedLinks();

	m_hRightArrow->Render(hDestSurf);
	m_hExtraCtrl->Render(hDestSurf);
	m_hExtraNumCtrl->Render(hDestSurf);
	m_hWeaponsCtrl->Render(hDestSurf);

	for(int i = 0; i < 4; i++)
		m_hStats[i]->Render(hDestSurf);

	for(i = 0; i < 10; i++)
	{
		m_hWeaponNums[i]->Render(hDestSurf);
		m_hWeapons[i]->Render(hDestSurf);
	}
}

//*************************************************************************

void CMenuCharacter::CheckSelectedLinks()
{
	int selected = m_listOption.GetSelectedItem();

	// Toggle the right arrow with the left arrow
	m_hRightArrow->Select(selected == ARROW_CTRLS);

	// Toggle on the Extra Stats stuff with any of the other stats
	if(selected >= MIN_STAT_INDEX && selected <= MAX_STAT_INDEX)
	{
		int	extraIndex = m_hExtraNumCtrl->GetSelIndex();
		m_hExtraCtrl->Select(extraIndex);
		m_hExtraNumCtrl->Select(extraIndex);

		m_hStats[0]->Select(selected == MIN_STAT_INDEX);
		m_hStats[1]->Select(selected == MIN_STAT_INDEX + 1);
		m_hStats[2]->Select(selected == MIN_STAT_INDEX + 2);
		m_hStats[3]->Select(selected == MIN_STAT_INDEX + 3);
	}
	else
	{
		m_hExtraCtrl->Select(DFALSE);
		m_hExtraNumCtrl->Select(DFALSE);

		m_hStats[0]->Select(DFALSE);
		m_hStats[1]->Select(DFALSE);
		m_hStats[2]->Select(DFALSE);
		m_hStats[3]->Select(DFALSE);
	}

	// Toggle the Weapons title with any of the weapon fields
	if(selected >= MIN_WEAPON_INDEX && selected <= MAX_WEAPON_INDEX)
	{
		m_hWeaponsCtrl->Select(DTRUE);

		for(int i = 1; i < 10; i++)
		{
			m_hWeapons[i]->Select(selected == MIN_WEAPON_INDEX + i - 1);
			m_hWeaponNums[i]->Select(selected == MIN_WEAPON_INDEX + i - 1);
		}
	}
	else
	{
		m_hWeaponsCtrl->Select(DFALSE);

		for(int i = 1; i < 10; i++)
		{
			m_hWeapons[i]->Select(DFALSE);
			m_hWeaponNums[i]->Select(DFALSE);
		}
	}
}

//*************************************************************************

// This is called to determine whether we should ask the user if they wish to switch
// resolutions back to the previous resolution if it had been changed to 640x480.
void CMenuCharacter::SetResolutionSwitch(DBOOL bAsk, int nOldWidth, int nOldHeight)
{
	m_bSwitchResolutions=bAsk;
	m_nOldScreenWidth=nOldWidth;
	m_nOldScreenHeight=nOldHeight;
}

//*************************************************************************
// This is called when the menu gets or loses focus
void CMenuCharacter::OnFocus(DBOOL bFocus)
{
	if (!bFocus)
	{
		if (m_bSwitchResolutions && !m_bGoingToFilesMenu)
		{
			char szMessage[2048];
			sprintf(szMessage, "Would you like to return to your previous screen resolution of %dx%d? (Y/N)", m_nOldScreenWidth, m_nOldScreenHeight);

			HSTRING hString=m_pClientDE->CreateString(szMessage);

			m_pMainMenus->DoMessageBox(hString, this);
			m_pMainMenus->AddMessageKey(m_pMainMenus->GetYesVKeyCode(), MENU_CMD_CHARACTER_SETUP_SWITCH_RES_BACK);
			m_pMainMenus->AddMessageKey(m_pMainMenus->GetNoVKeyCode(), MENU_CMD_KILL_MESSAGEBOX);			

			m_pClientDE->FreeString(hString);
		}
		m_bGoingToFilesMenu=DFALSE;
	}
}

//*************************************************************************

DDWORD CMenuCharacter::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{	
	switch (dwCommand)
	{
		case MENU_CMD_SAVE_CHARACTER:
		{
			(m_pMainMenus->GetCharacterFiles())->SetAction(MENU_ACTION_SAVE);
			(m_pMainMenus->GetCharacterFiles())->Build();

			m_bGoingToFilesMenu=DTRUE;
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTERFILES);
			break;
		}

		case MENU_CMD_LOAD_CHARACTER:
		{
			(m_pMainMenus->GetCharacterFiles())->SetAction(MENU_ACTION_LOAD);
			(m_pMainMenus->GetCharacterFiles())->Build();

			m_bGoingToFilesMenu=DTRUE;
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTERFILES);
			break;
		}

		case MENU_CMD_DELETE_CHARACTER:
		{
			(m_pMainMenus->GetCharacterFiles())->SetAction(MENU_ACTION_DELETE);
			(m_pMainMenus->GetCharacterFiles())->Build();

			m_bGoingToFilesMenu=DTRUE;
			m_pMainMenus->SetCurrentMenu(MENU_ID_CHARACTERFILES);
			break;
		}

		case MENU_CMD_KILL_MESSAGEBOX:
		{
			m_pMainMenus->KillMessageBox();
			break;
		}

		case MENU_CMD_CHARACTER_SETUP_SWITCH_RES_BACK:
		{
			m_pMainMenus->SwitchResolution(m_nOldScreenWidth, m_nOldScreenHeight);
			m_pMainMenus->KillMessageBox();
			break;
		}
	}
	return 0;
}

//*************************************************************************

DBOOL CMenuCharacter::SaveB2CFile(char *szFile)
{
	FILE	*file = 0;
	char	str1[128];

	if(chdir("players") != 0)
		mkdir("players");
	else
		chdir("..");

	sprintf(str1, "players\\%s.b2c", szFile);

	if(file = fopen(str1, "wb"))
	{
		fwrite(&m_Player, sizeof(B2C), 1, file);
		fclose(file);
	}
	else
		return	DFALSE;

	return	DTRUE;
}

//*************************************************************************

DBOOL CMenuCharacter::LoadB2CFile(char *szFile)
{
	FILE	*file = 0;
	char	str1[128];

	sprintf(str1, "players\\%s", szFile);

	if(file = fopen(str1, "rb"))
	{
		fread(&m_Player, sizeof(B2C), 1, file);
		fclose(file);
	}
	else
		return	DFALSE;

	return	DTRUE;
}

//*************************************************************************

DBOOL CMenuCharacter::DeleteB2CFile(char *szFile)
{
	char	str1[128];

	sprintf(str1, "players\\%s", szFile);

	// Don't delete the default files
	if(_mbscmp((const unsigned char*)szFile, (const unsigned char*)"caleb.b2c") == 0)	return DFALSE;
	if(_mbscmp((const unsigned char*)szFile, (const unsigned char*)"ophelia.b2c") == 0)	return DFALSE;
	if(_mbscmp((const unsigned char*)szFile, (const unsigned char*)"ishmael.b2c") == 0)	return DFALSE;
	if(_mbscmp((const unsigned char*)szFile, (const unsigned char*)"gabby.b2c") == 0)	return DFALSE;
	
	if(!remove(str1))
		return	DTRUE;
	else
		return	DFALSE;
}

//*************************************************************************

void CMenuCharacter::ChangeCharacterPic(int index)
{
	if((index < 0) || (index >= MAX_CHARACTER_INFO))
		return;

	char file[128];
	sprintf(file, "interface\\charscreen\\characterpics\\%s", g_CharacterMenuInfo[m_PlayerIndex].szFile);

#ifdef _ADDON
	char* sPic = g_CharacterMenuInfo[m_PlayerIndex].szFile;
	if (sPic)
	{
		if (strstr(sPic, "_AO"))
		{
			strcpy(file, sPic);
		}
	}
#endif

	if(m_hCharacterPic)
	{
		m_pClientDE->DeleteSurface(m_hCharacterPic);
		m_hCharacterPic = 0;
	}

	m_hCharacterPic = m_pClientDE->CreateSurfaceFromBitmap(file);

	m_Player.dwCharacter = g_CharacterMenuInfo[m_PlayerIndex].nCharacter;
	m_Player.dwColor = g_CharacterMenuInfo[m_PlayerIndex].nColor;
}

//*************************************************************************

void CMenuCharacter::UpdateCharacterStruct()
{
	m_Player.dwCharacter	= g_CharacterMenuInfo[m_PlayerIndex].nCharacter;
	m_Player.dwColor		= g_CharacterMenuInfo[m_PlayerIndex].nColor;
	m_Player.dwStrength		= m_hStats[0]->GetSelIndex();
	m_Player.dwResist		= m_hStats[1]->GetSelIndex();
	m_Player.dwSpeed		= m_hStats[2]->GetSelIndex();
	m_Player.dwFocus		= m_hStats[3]->GetSelIndex();
	m_Player.dwWeap1		= m_hWeapons[1]->GetSelIndex();
	m_Player.dwWeap2		= m_hWeapons[2]->GetSelIndex();
	m_Player.dwWeap3		= m_hWeapons[3]->GetSelIndex();
	m_Player.dwWeap4		= m_hWeapons[4]->GetSelIndex();
	m_Player.dwWeap5		= m_hWeapons[5]->GetSelIndex();
	m_Player.dwWeap6		= m_hWeapons[6]->GetSelIndex();
	m_Player.dwWeap7		= m_hWeapons[7]->GetSelIndex();
	m_Player.dwWeap8		= m_hWeapons[8]->GetSelIndex();
	m_Player.dwWeap9		= m_hWeapons[9]->GetSelIndex();
}

//*************************************************************************

void CMenuCharacter::SetPlayerIndexFromStruct()
{
	for(int i = 0; i < MAX_CHARACTER_INFO; i++)
	{
		if(m_Player.dwCharacter == g_CharacterMenuInfo[i].nCharacter)
			if(m_Player.dwColor == g_CharacterMenuInfo[i].nColor)
				break;
	}

	m_PlayerIndex = i;
}

//*************************************************************************

void CMenuCharacter::UpdateScreenFromStruct()
{
	int total = m_Player.dwStrength + m_Player.dwSpeed + m_Player.dwResist + m_Player.dwFocus;
	int i;
	DBOOL nuke = DFALSE;

	// Check the total attributes
	if(total > MAX_ATTRIBUTES)		nuke = DTRUE;

	// Check the individual attributes
	if((m_Player.dwStrength < 0) || (m_Player.dwStrength > MAX_ATTRIBUTE))	nuke = DTRUE;
	if((m_Player.dwSpeed < 0) || (m_Player.dwSpeed > MAX_ATTRIBUTE))		nuke = DTRUE;
	if((m_Player.dwResist < 0) || (m_Player.dwResist > MAX_ATTRIBUTE))		nuke = DTRUE;
	if((m_Player.dwFocus < 0) || (m_Player.dwFocus > MAX_ATTRIBUTE))		nuke = DTRUE;

	if(nuke)
	{
		// If the attributes in the file are too high (someone trys to cheat) then nuke 'em
		m_PlayerIndex = 0;

		for(i = 0; i < 4; i++)
			m_hStats[i]->SetSelIndex(DEFAULT_ATTRIBUTE);

		m_hExtraNumCtrl->SetSelIndex(0);
	}
	else
	{
		// Otherwise just set them to what's in the file
		SetPlayerIndexFromStruct();

		m_hStats[0]->SetSelIndex(m_Player.dwStrength);
		m_hStats[1]->SetSelIndex(m_Player.dwResist);
		m_hStats[2]->SetSelIndex(m_Player.dwSpeed);
		m_hStats[3]->SetSelIndex(m_Player.dwFocus);
		m_hExtraNumCtrl->SetSelIndex(MAX_ATTRIBUTES - total);
	}

	// Get a pointer to the first weapon for easier loop checkin'
	DDWORD	*weapon = &(m_Player.dwWeap1);

	for(i = 1; i < 10; i++)
	{
		DDWORD	weap = weapon[i - 1];

		if((weap > 0) && (weap <= WEAP_LASTPLAYERWEAPON))
		{
			m_hWeapons[i]->SetSelIndex(weap);
			m_hWeaponNums[i]->SetSelIndex(weap);
		}
		else
		{
			m_hWeapons[i]->SetSelIndex(0);
			m_hWeaponNums[i]->SetSelIndex(0);
			weapon[i - 1] = 0;
		}
	}

	ChangeCharacterPic(m_PlayerIndex);
}

//*************************************************************************

DBOOL CMenuCharacter::IsWeaponADupe(int index)
{
	for(int i = 0; i < 10; i++)
	{
		if(i != index)
		{
			int	index1 = m_hWeapons[index]->GetSelIndex();
			int	index2 = m_hWeapons[i]->GetSelIndex();
			if(index1 == index2) return DTRUE;
		}
	}

	return	DFALSE;
}

//*************************************************************************

void CMenuCharacter::OnLeft()
{
	int selected = m_listOption.GetSelectedItem();

	// Handle left on arrows
	if(selected == ARROW_CTRLS)
	{
		m_PlayerIndex--;

		if(m_PlayerIndex < 0)
			m_PlayerIndex = MAX_CHARACTER_INFO - 1;

		ChangeCharacterPic(m_PlayerIndex);
	}

	// Handle left on stats
	if(selected >= MIN_STAT_INDEX && selected <= MAX_STAT_INDEX)
	{
		int	index1 = m_hStats[selected - MIN_STAT_INDEX]->GetSelIndex();
		int index2 = m_hExtraNumCtrl->GetSelIndex();

		if(index1 > 1)
		{
			m_hStats[selected - MIN_STAT_INDEX]->OnLeft();
			index2++;
			m_hExtraNumCtrl->SetSelIndex(index2);
		}
	}

	// Handle left on weapons
	if(selected >= MIN_WEAPON_INDEX && selected <= MAX_WEAPON_INDEX)
	{
		int value = selected - MIN_WEAPON_INDEX + 1;
		int	index1 = m_hWeapons[value]->GetSelIndex();

		// Find the next available weapon to the left or set to NONE
		if(index1 > 0)
		{
			while(1)
			{
				m_hWeapons[value]->OnLeft();
				m_hWeaponNums[value]->OnLeft();
				index1--;

				if((index1 == 0) || (!IsWeaponADupe(value)))
					break;
			}
		}
	}

	UpdateCharacterStruct();
}

//*************************************************************************

void CMenuCharacter::OnRight()
{
	int selected = m_listOption.GetSelectedItem();

	// Handle left on arrows
	if(selected == ARROW_CTRLS)
	{
		m_PlayerIndex++;

		if(m_PlayerIndex >= MAX_CHARACTER_INFO)
			m_PlayerIndex = 0;

		ChangeCharacterPic(m_PlayerIndex);
	}

	// Toggle on the Extra Stats stuff with any of the other stats
	if(selected >= MIN_STAT_INDEX && selected <= MAX_STAT_INDEX)
	{
		int	index1 = m_hStats[selected - MIN_STAT_INDEX]->GetSelIndex();
		int index2 = m_hExtraNumCtrl->GetSelIndex();

		if((index1 < MAX_ATTRIBUTE) && (index2 > 0))
		{
			m_hStats[selected - MIN_STAT_INDEX]->OnRight();
			index2--;
			m_hExtraNumCtrl->SetSelIndex(index2);
		}
	}

	// Toggle the Weapons title with any of the weapon fields
	if(selected >= MIN_WEAPON_INDEX && selected <= MAX_WEAPON_INDEX)
	{
		int value = selected - MIN_WEAPON_INDEX + 1;
		int	index1 = m_hWeapons[value]->GetSelIndex();
		int index2 = index1;
		DBOOL resetWeap = DFALSE;

		// Find the next available weapon to the left or set to NONE
		if(index2 < WEAP_LASTPLAYERWEAPON)
		{
			while(1)
			{
				m_hWeapons[value]->OnRight();
				m_hWeaponNums[value]->OnRight();
				index2++;

				if(!IsWeaponADupe(value))
					break;
				else if((index2 == WEAP_LASTPLAYERWEAPON) && (IsWeaponADupe(value)))
				{
					resetWeap = DTRUE;
					break;
				}
			}
		}

		// If there wasn't a valid weapon to the right... then reset it to what it was
		if(resetWeap)
		{
			m_hWeapons[value]->SetSelIndex(index1);
			m_hWeaponNums[value]->SetSelIndex(index1);
		}
	}

	UpdateCharacterStruct();
}