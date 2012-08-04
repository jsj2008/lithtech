/****************************************************************************
;
;	 MODULE:		ConfigMgr (.CPP)
;
;	PURPOSE:		Config Manager for .b2c files
;
;	HISTORY:		10/15/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "Windows.h"
#include "ConfigMgr.h"
#include "Direct.h"
#include "StdIo.h"
#include "Io.h"
#include "..\Shared\ClientRes.h"
#include <mbstring.h>

// Statics...

char	s_sText[128];


// Functions...

/* *********************************************************************** */
/* CConfig                                                                 */

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfig::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CConfig::Init(char* sFilename)
{
	// Sanity checks...

	if (!sFilename) return(FALSE);
	if (sFilename[0] == '\0') return(FALSE);


	// Set simple members...

	_mbsncpy((unsigned char*)m_sDisplay, (const unsigned char*)sFilename, CM_MAX_NAME - 1);
	_mbsncpy((unsigned char*)m_sFile, (const unsigned char*)sFilename, CM_MAX_NAME - 1);


	// Strip off the file extension...

	if (_mbsstr((const unsigned char*)m_sDisplay, (const unsigned char*)"."))
	{
		int nLen = _mbstrlen(m_sDisplay);
		if (nLen > 4) m_sDisplay[nLen - 4] = '\0';
	}


	// Open the file and read the values...

	FILE* pFile = fopen(m_sFile, "rb");
	if (!pFile) return(FALSE);

	int cBytes = fread(&m_B2c, 1, sizeof(B2C), pFile);

	fclose(pFile);

	if (cBytes != sizeof(B2C)) return(FALSE);


	// Do a non-add-on check if...

#ifndef _ADDON
	if (m_B2c.dwCharacter > 3)
	{
		return(FALSE);
	}
#endif


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfig::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CConfig::FillDialog(HWND hDlg)
{
	// Sanity checks...

	if (!hDlg) return(FALSE);


	// Set the character and color...

	SetDlgItemText(hDlg, IDC_CHARACTER, GetCharacterAndColor());


	// Set the four main attributes...

	SetDlgItemText(hDlg, IDC_STRENGTH, GetStrength());
	SetDlgItemText(hDlg, IDC_SPEED, GetSpeed());
	SetDlgItemText(hDlg, IDC_RESIST, GetResist());
	SetDlgItemText(hDlg, IDC_FOCUS, GetFocus());


	// Set the weapon lines...

	SetDlgItemText(hDlg, IDC_WEAP1, GetWeaponLine1());
	SetDlgItemText(hDlg, IDC_WEAP2, GetWeaponLine2());
	SetDlgItemText(hDlg, IDC_WEAP3, GetWeaponLine3());


	// All done...

	return(TRUE);
}

char* CConfig::GetCharacterAndColor()
{
	char sChar[32];
	char sColor[32];

	_mbscpy((unsigned char*)sChar, (const unsigned char*)GetCharacter());
	_mbscpy((unsigned char*)sColor, (const unsigned char*)GetColor());

	sprintf(s_sText, "%s (%s)", sChar, sColor);
	return(s_sText);
}

char* CConfig::GetCharacter()
{
	switch (m_B2c.dwCharacter)
	{
		case CHARACTER_CALEB:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Caleb"); break;
		case CHARACTER_OPHELIA:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Ophelia"); break;
		case CHARACTER_ISHMAEL:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Ishmael"); break;
		case CHARACTER_GABREILLA:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Gabriella"); break;

#ifdef _ADDON
		case CHARACTER_M_CULTIST:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Male Cultist"); break;
		case CHARACTER_F_CULTIST:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Female Cultist"); break;
		case CHARACTER_SOULDRUDGE:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Soul Drudge"); break;
		case CHARACTER_PROPHET:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Prophet"); break;
#endif

		default: _mbscpy((unsigned char*)s_sText, (const unsigned char*)"Caleb");
	}

	return(s_sText);
}

char* CConfig::GetColor()
{
	switch (m_B2c.dwColor)
	{
		case MULTIPLAY_SKIN_NORMAL: _mbscpy((unsigned char*)s_sText, (const unsigned char*)"Normal"); break;
		case MULTIPLAY_SKIN_RED:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Red"); break;
		case MULTIPLAY_SKIN_BLUE:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Blue"); break;
		case MULTIPLAY_SKIN_GREEN:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Green"); break;
		case MULTIPLAY_SKIN_YELLOW: _mbscpy((unsigned char*)s_sText, (const unsigned char*)"Yellow"); break;

		default: _mbscpy((unsigned char*)s_sText, (const unsigned char*)"Normal");
	}

	return(s_sText);
}

char* CConfig::GetStrength()
{
	sprintf(s_sText, "%i", m_B2c.dwStrength);
	return(s_sText);
}

char* CConfig::GetSpeed()
{
	sprintf(s_sText, "%i", m_B2c.dwSpeed);
	return(s_sText);
}

char* CConfig::GetResist()
{
	sprintf(s_sText, "%i", m_B2c.dwResist);
	return(s_sText);
}

char* CConfig::GetFocus()
{
	sprintf(s_sText, "%i", m_B2c.dwFocus);
	return(s_sText);
}

char* CConfig::GetWeapon(DWORD dwWeap)
{
	switch (dwWeap)
	{
		case WEAP_NONE:				_mbscpy((unsigned char*)s_sText, (const unsigned char*)"None"); break;
		case WEAP_BERETTA:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Brta"); break;
		case WEAP_SUBMACHINEGUN:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Sub"); break;
		case WEAP_FLAREGUN:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Flare"); break;
		case WEAP_SHOTGUN:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Shot"); break;
		case WEAP_SNIPERRIFLE:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Snipe"); break;
		case WEAP_HOWITZER:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Howtz"); break;
		case WEAP_NAPALMCANNON:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Naplm"); break;
		case WEAP_SINGULARITY:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Sing"); break;
		case WEAP_ASSAULTRIFLE:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Aslt"); break;
		case WEAP_BUGSPRAY:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Bug"); break;
		case WEAP_MINIGUN:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Mini"); break;
		case WEAP_DEATHRAY:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Death"); break;
		case WEAP_TESLACANNON:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Tesla"); break;
		case WEAP_VOODOO:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Voodoo"); break;
		case WEAP_ORB:				_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Orb"); break;
		case WEAP_LIFELEECH:		_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Leech"); break;
#ifdef _ADD_ON
		case WEAP_COMBATSHOTGUN:	_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Combat"); break;
		case WEAP_FLAYER:			_mbscpy((unsigned char*)s_sText, (const unsigned char*)"Flayer"); break;
#endif

		default: _mbscpy((unsigned char*)s_sText, (const unsigned char*)"N/A");
	}

	return(s_sText);
}

char* CConfig::GetWeaponLine1()
{
	char s1[32];
	char s2[32];
	char s3[32];

	_mbscpy((unsigned char*)s1, (const unsigned char*)GetWeapon(m_B2c.dwWeap1));
	_mbscpy((unsigned char*)s2, (const unsigned char*)GetWeapon(m_B2c.dwWeap2));
	_mbscpy((unsigned char*)s3, (const unsigned char*)GetWeapon(m_B2c.dwWeap3));

	sprintf(s_sText, "%s, %s, %s", s1, s2, s3);
	return(s_sText);
}

char* CConfig::GetWeaponLine2()
{
	char s1[32];
	char s2[32];
	char s3[32];

	_mbscpy((unsigned char*)s1, (const unsigned char*)GetWeapon(m_B2c.dwWeap4));
	_mbscpy((unsigned char*)s2, (const unsigned char*)GetWeapon(m_B2c.dwWeap5));
	_mbscpy((unsigned char*)s3, (const unsigned char*)GetWeapon(m_B2c.dwWeap6));

	sprintf(s_sText, "%s, %s, %s", s1, s2, s3);
	return(s_sText);
}

char* CConfig::GetWeaponLine3()
{
	char s1[32];
	char s2[32];
	char s3[32];

	_mbscpy((unsigned char*)s1, (const unsigned char*)GetWeapon(m_B2c.dwWeap7));
	_mbscpy((unsigned char*)s2, (const unsigned char*)GetWeapon(m_B2c.dwWeap8));
	_mbscpy((unsigned char*)s3, (const unsigned char*)GetWeapon(m_B2c.dwWeap9));

	sprintf(s_sText, "%s, %s, %s", s1, s2, s3);
	return(s_sText);
}


/* *********************************************************************** */
/* CConfigMgr                                                              */

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CConfigMgr::Init(char* sDir)
{
	// Sanity checks...

	if (!sDir) return(FALSE);
	if (sDir[0] == '\0') return(FALSE);


	// Terminate everything...

	Term();


	// Change to the given direcotry...

	if (chdir(sDir) != 0)
	{
		return(FALSE);
	}


	// Enumerate the available .b2c files and add them to the list box...

	long	hFile;
    struct	_finddata_t fd;

	hFile = _findfirst("*.b2c", &fd);
	if (hFile == -1)
	{
		chdir("..");
		return(FALSE);
	}

	AddConfig(fd.name);

	BOOL bContinue = TRUE;

	while (bContinue)
	{
		if (_findnext(hFile, &fd) != 0)
		{
			bContinue = FALSE;
		}
		else
		{
			AddConfig(fd.name);
		}
	}


	// Restore the directory...

	chdir ("..");


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::FillListBox
//
//	PURPOSE:	Fills the given list box
//
// ----------------------------------------------------------------------- //

CConfig* CConfigMgr::AddConfig(char* sFilename)
{
	// Sanity checks...

	if (!sFilename) return(FALSE);
	if (sFilename[0] == '\0') return(FALSE);
	if (m_cConfigs >= CM_MAX_CONFIGS) return(NULL);


	// Init the next available config object...

	CConfig* pConfig = &m_aConfigs[m_cConfigs];

	if (!pConfig->Init(sFilename))
	{
		return(NULL);
	}


	// Increment the number of configs...

	m_cConfigs++;


	// All done...

	return(pConfig);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::FillListBox
//
//	PURPOSE:	Fills the given list box
//
// ----------------------------------------------------------------------- //

BOOL CConfigMgr::FillListBox(HWND hList, char* sCurSel, char* sDefSel)
{
	// Sanity checks...

	if (!hList) return(FALSE);


	// Reset the contents of the list box...

	SendMessage(hList, LB_RESETCONTENT, 0, 0);


	// Add each config...

	for (int i = 0; i < m_cConfigs; i++)
	{
		CConfig* pConfig = GetConfig(i);
		if (pConfig)
		{
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)pConfig->GetDisplayName());
		}
	}


	// Set the current selection if requested...

	BOOL bSel = FALSE;

	if (sCurSel && sCurSel[0] != '\0')
	{
		char sCmp[128];
		_mbscpy((unsigned char*)sCmp, (const unsigned char*)sCurSel);
		int nLen = _mbstrlen(sCmp);
		if (nLen > 4) sCmp[nLen-4] = '\0';

		int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
		if (count == LB_ERR) return(TRUE);

		for (int i = 0; i < count; i++)
		{
			char sText[128] = { "" };

			int nRet = SendMessage(hList, LB_GETTEXT, i, (LPARAM)sText);
			if (nRet != LB_ERR)
			{
				if (_mbsicmp((const unsigned char*)sText, (const unsigned char*)sCmp) == 0)
				{
					nRet = SendMessage(hList, LB_SETCURSEL, i, 0);
					if (nRet != LB_ERR) bSel = TRUE;
				}
			}
		}
	}

	if (!bSel && sDefSel && sDefSel[0] != '\0')
	{
		char sCmp[128];
		_mbscpy((unsigned char*)sCmp, (const unsigned char*)sDefSel);
		int nLen = _mbstrlen(sCmp);
		if (nLen > 4) sCmp[nLen-4] = '\0';

		int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
		if (count == LB_ERR) return(TRUE);

		for (int i = 0; i < count; i++)
		{
			char sText[128] = { "" };

			int nRet = SendMessage(hList, LB_GETTEXT, i, (LPARAM)sText);
			if (nRet != LB_ERR)
			{
				if (_mbsicmp((const unsigned char*)sText, (const unsigned char*)sCmp) == 0)
				{
					nRet = SendMessage(hList, LB_SETCURSEL, i, 0);
				}
			}
		}
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::GetListBoxSelection
//
//	PURPOSE:	Gets the current selection from the given list box
//
// ----------------------------------------------------------------------- //

CConfig* CConfigMgr::GetListBoxSelection(HWND hList)
{
	// Sanity checks...

	if (!hList) return(NULL);


	// Get the currently select list-box item...

	int nIndex = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (nIndex == LB_ERR) return(NULL);


	// Get the text for this item...

	char sText[128] = { "" };

	int nRet = SendMessage(hList, LB_GETTEXT, nIndex, (LPARAM)sText);
	if (nRet == LB_ERR) return(NULL);


	// Find this item...

	return(GetConfigFromDisplay(sText));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::FillComboBox
//
//	PURPOSE:	Fills the given combo box
//
// ----------------------------------------------------------------------- //

BOOL CConfigMgr::FillComboBox(HWND hCombo, char* sCurSel, char* sDefSel)
{
	// Sanity checks...

	if (!hCombo) return(FALSE);


	// Reset the contents of the list box...

	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);


	// Add each config...

	for (int i = 0; i < m_cConfigs; i++)
	{
		CConfig* pConfig = GetConfig(i);
		if (pConfig)
		{
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)pConfig->GetDisplayName());
		}
	}


	// Set the current selection if requested...

	BOOL bSel = FALSE;

	if (sCurSel && sCurSel[0] != '\0')
	{
		char sCmp[128];
		_mbscpy((unsigned char*)sCmp, (const unsigned char*)sCurSel);
		int nLen = _mbstrlen(sCmp);
		if (nLen > 4) sCmp[nLen-4] = '\0';

		int count = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
		if (count == CB_ERR) return(TRUE);

		for (int i = 0; i < count; i++)
		{
			char sText[128] = { "" };

			int nRet = SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)sText);
			if (nRet != CB_ERR)
			{
				if (_mbsicmp((const unsigned char*)sText, (const unsigned char*)sCmp) == 0)
				{
					nRet = SendMessage(hCombo, CB_SETCURSEL, i, 0);
					if (nRet != CB_ERR) bSel = TRUE;
				}
			}
		}
	}

	if (!bSel && sDefSel && sDefSel[0] != '\0')
	{
		char sCmp[128];
		_mbscpy((unsigned char*)sCmp, (const unsigned char*)sDefSel);
		int nLen = _mbstrlen(sCmp);
		if (nLen > 4) sCmp[nLen-4] = '\0';

		int count = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
		if (count == CB_ERR) return(TRUE);

		for (int i = 0; i < count; i++)
		{
			char sText[128] = { "" };

			int nRet = SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)sText);
			if (nRet != CB_ERR)
			{
				if (_mbsicmp((const unsigned char*)sText, (const unsigned char*)sCmp) == 0)
				{
					nRet = SendMessage(hCombo, CB_SETCURSEL, i, 0);
				}
			}
		}
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CConfigMgr::GetComboBoxSelection
//
//	PURPOSE:	Gets the current selection from the given combo box
//
// ----------------------------------------------------------------------- //

CConfig* CConfigMgr::GetComboBoxSelection(HWND hCombo)
{
	// Sanity checks...

	if (!hCombo) return(NULL);


	// Get the currently select list-box item...

	int nIndex = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
	if (nIndex == CB_ERR) return(NULL);


	// Get the text for this item...

	char sText[128] = { "" };

	int nRet = SendMessage(hCombo, CB_GETLBTEXT, nIndex, (LPARAM)sText);
	if (nRet == CB_ERR) return(NULL);


	// Find this item...

	return(GetConfigFromDisplay(sText));
}
