//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// DEditOptions.h: interface for the CDEditOptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEDITOPTIONS_H__D15CECC2_F290_11D2_BE14_0060971BDC6D__INCLUDED_)
#define AFX_DEDITOPTIONS_H__D15CECC2_F290_11D2_BE14_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "genregmgr.h"
#include "optionsbase.h"
#include "afxtempl.h"

// The options classes
class COptionsDisplay;
class COptionsAdvancedSelect;
class COptionsClassDialog;
class COptionsRun;
class COptionsObjectBrowser;
class COptionsGenerateUniqueNames;
class COptionsClipboard;
class COptionsWindows;
class COptionsModels;
class COptionsViewports;
class COptionsControls;
class COptionsMisc;
class COptionsPrefabs;

class CDEditOptions : public COptionsBase
{
public:
	CDEditOptions();
	virtual ~CDEditOptions();

	// Initialization
	BOOL					Init(CGenRegMgr *pRegMgr, CString sRegRoot);

	// Saves/Loads all of the options classes
	BOOL					Load();
	BOOL					Save();

	// Access to the various options classes
	COptionsDisplay				*GetDisplayOptions()				{ return m_pDisplayOptions; }
	COptionsAdvancedSelect		*GetAdvancedSelectOptions()			{ return m_pAdvancedSelectOptions; }
	COptionsClassDialog			*GetClassDialogOptions()			{ return m_pClassDialogOptions; }
	COptionsRun					*GetRunOptions()					{ return m_pRunOptions; }
	COptionsObjectBrowser		*GetObjectBrowserOptions()			{ return m_pObjectBrowserOptions; }
	COptionsGenerateUniqueNames	*GetGenerateUniqueNamesOptions()	{ return m_pGenerateUniqueNamesOptions; }
	COptionsClipboard			*GetClipboardOptions()				{ return m_pClipboardOptions; }
	COptionsWindows				*GetWindowsOptions()				{ return m_pWindowsOptions; }
	COptionsModels				*GetModelsOptions()					{ return m_pModelsOptions; }
	COptionsControls			*GetControlsOptions()				{ return m_pControlsOptions; }
	COptionsMisc				*GetMiscOptions()					{ return m_pMiscOptions; }
	COptionsPrefabs				*GetPrefabsOptions()				{ return m_pPrefabsOptions; }
	COptionsViewports			*GetViewportOptions()				{ return m_pViewportOptions; }

	// Access to options

	//auto saving
	bool					IsAutoSave() const				{return m_bAutoSave;}
	void					EnableAutoSave(bool bAutoSave)	{m_bAutoSave = bAutoSave;}

	//auto save path
	const char*				GetAutoSavePath() const					{return m_sAutoSavePath;}
	void					SetAutoSavePath(const char* pszPath)	{m_sAutoSavePath = pszPath;}

	//time between auto saves (in minutes)
	DWORD					GetAutoSaveTime() const			{ return m_dwAutoSaveTime; }
	void					SetAutoSaveTime(DWORD dwTime)	{ m_dwAutoSaveTime=dwTime; }

	//number of backups to create for each file
	DWORD					GetNumBackups() const			{ return m_dwNumBackups; }
	void					SetNumBackups(DWORD dwNum)		{ m_dwNumBackups = dwNum; }

	//number of backups to create for each file
	bool					DeleteOnClose() const			{ return m_bDeleteOnClose; }
	void					SetDeleteOnClose(bool bDel)		{ m_bDeleteOnClose = bDel; }

	// Is this the first time running DEdit?
	bool					IsFirstRun()					{ return m_bFirstRun; }
	void					SetFirstRun(bool bFirst)		{ m_bFirstRun = bFirst; }


protected:	
	// The various options classes
	COptionsDisplay				*m_pDisplayOptions;
	COptionsAdvancedSelect		*m_pAdvancedSelectOptions;
	COptionsClassDialog			*m_pClassDialogOptions;
	COptionsRun					*m_pRunOptions;
	COptionsObjectBrowser		*m_pObjectBrowserOptions;
	COptionsGenerateUniqueNames	*m_pGenerateUniqueNamesOptions;
	COptionsClipboard			*m_pClipboardOptions;
	COptionsWindows				*m_pWindowsOptions;
	COptionsModels				*m_pModelsOptions;
	COptionsControls			*m_pControlsOptions;
	COptionsMisc				*m_pMiscOptions;
	COptionsPrefabs				*m_pPrefabsOptions;
	COptionsViewports			*m_pViewportOptions;

	// An array of all of the option class
	CArray<COptionsBase *, COptionsBase *>	m_optionsArray;

	// The autosave interval
	DWORD					m_dwAutoSaveTime;
	DWORD					m_dwNumBackups;
	bool					m_bAutoSave;
	bool					m_bDeleteOnClose;
	CString					m_sAutoSavePath;

	// First run record
	bool					m_bFirstRun;
};

#endif // !defined(AFX_DEDITOPTIONS_H__D15CECC2_F290_11D2_BE14_0060971BDC6D__INCLUDED_)
