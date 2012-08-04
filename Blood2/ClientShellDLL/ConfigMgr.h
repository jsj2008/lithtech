/****************************************************************************
;
;	 MODULE:		ConfigMgr (.H)
;
;	PURPOSE:		Config Manager for .b2c files
;
;	HISTORY:		10/15/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _CONFIGMGR_H_
#define _CONFIGMGR_H_


// Includes...

#include "cpp_client_de.h"
#include "SharedDefs.h"
#include <mbstring.h>


// Defines...

#define CM_MAX_CONFIGS	64
#define CM_MAX_NAME		64


// Classess...

class CConfig
{
	// Member functions...

public:
	CConfig() { Clear(); }
	~CConfig() { Term(); }

	BOOL				Init(char* sFilename);
	void				Term();
	void				Clear();

	char*				GetDisplayName() { return(m_sDisplay); }
	char*				GetFileName() { return(m_sFile); }

	BOOL				FillDialog(HWND hDlg);

private:
	char*				GetCharacterAndColor();
	char*				GetCharacter();
	char*				GetColor();

	char*				GetStrength();
	char*				GetSpeed();
	char*				GetResist();
	char*				GetFocus();

	char*				GetWeapon(DWORD dwWeap);
	char*				GetWeaponLine1();
	char*				GetWeaponLine2();
	char*				GetWeaponLine3();


	// Member variables...

private:
	B2C					m_B2c;
	char				m_sDisplay[CM_MAX_NAME];
	char				m_sFile[CM_MAX_NAME];
};

class CConfigMgr
{
	// Member functions...

public:
	CConfigMgr() { Clear(); }
	~CConfigMgr() { Term(); }

	BOOL				Init(char* sDir);
	void				Term();
	void				Clear();

	int					GetNumConfigs() { return(m_cConfigs); }
	CConfig*			GetConfig(int i);
	CConfig*			GetConfigFromDisplay(char* sName);
	CConfig*			GetConfigFromFile(char* sName);

	CConfig*			AddConfig(char* sFilename);

	int					FillListBox(HWND hList, char* sCurSel = NULL, char* sDefSel = NULL);
	CConfig*			GetListBoxSelection(HWND hList);

	int					FillComboBox(HWND hCombo, char* sCurSel = NULL, char* sDefSel = NULL);
	CConfig*			GetComboBoxSelection(HWND hCombo);


	// Member variables...

private:
	CConfig				m_aConfigs[CM_MAX_CONFIGS];
	int					m_cConfigs;
};


// Inlines...

inline void CConfig::Clear()
{

}

inline void CConfig::Term()
{
	Clear();
}

inline void CConfigMgr::Clear()
{
	m_cConfigs = 0;
}

inline void CConfigMgr::Term()
{
	Clear();
}

inline CConfig* CConfigMgr::GetConfig(int i)
{
	if (i < 0) return(NULL);
	if (i >= m_cConfigs) return(NULL);
	if (i >= CM_MAX_CONFIGS) return(NULL);

	return(&m_aConfigs[i]);
}

inline CConfig* CConfigMgr::GetConfigFromDisplay(char* sName)
{
	for (int i = 0; i < m_cConfigs; i++)
	{
		CConfig* pConfig = GetConfig(i);
		if (pConfig)
		{
			if (_mbscmp((const unsigned char*)pConfig->GetDisplayName(), (const unsigned char*)sName) == 0)
			{
				return(pConfig);
			}
		}
	}

	return(NULL);
}

inline CConfig* CConfigMgr::GetConfigFromFile(char* sName)
{
	for (int i = 0; i < m_cConfigs; i++)
	{
		CConfig* pConfig = GetConfig(i);
		if (pConfig)
		{
			if (_mbscmp((const unsigned char*)pConfig->GetFileName(), (const unsigned char*)sName) == 0)
			{
				return(pConfig);
			}
		}
	}

	return(NULL);
}


// EOF...

#endif






