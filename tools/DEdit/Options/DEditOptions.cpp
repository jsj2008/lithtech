//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// DEditOptions.cpp: implementation of the CDEditOptions class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "deditoptions.h"
#include "optionsbase.h"
#include "optionsdisplay.h"
#include "optionsadvancedselect.h"
#include "optionsclassdialog.h"
#include "optionsrun.h"
#include "optionsobjectbrowser.h"
#include "optionsgenerateuniquenames.h"
#include "optionsclipboard.h"
#include "optionswindows.h"
#include "optionsmodels.h"
#include "optionscontrols.h"
#include "optionsmisc.h"
#include "optionsprefabs.h"
#include "optionsviewports.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDEditOptions::CDEditOptions()
{
	m_pDisplayOptions				= new COptionsDisplay;
	m_pAdvancedSelectOptions		= new COptionsAdvancedSelect;
	m_pClassDialogOptions			= new COptionsClassDialog;
	m_pRunOptions					= new COptionsRun;
	m_pObjectBrowserOptions			= new COptionsObjectBrowser;
	m_pGenerateUniqueNamesOptions	= new COptionsGenerateUniqueNames;
	m_pClipboardOptions				= new COptionsClipboard;
	m_pWindowsOptions				= new COptionsWindows;
	m_pModelsOptions				= new COptionsModels;
	m_pControlsOptions				= new COptionsControls;
	m_pMiscOptions					= new COptionsMisc;
	m_pPrefabsOptions				= new COptionsPrefabs;
	m_pViewportOptions				= new COptionsViewports;

	m_dwAutoSaveTime	= 5;
	m_dwNumBackups		= 1;
	m_bAutoSave			= FALSE;
	m_bDeleteOnClose	= FALSE;
	m_sAutoSavePath		= "c:\\DEditBackup\\";
}

CDEditOptions::~CDEditOptions()
{
	for (uint32 nCurrOption=0; nCurrOption < m_optionsArray.GetSize(); nCurrOption++)
	{
		delete m_optionsArray[nCurrOption];
	}
}

/************************************************************************/
// Initialization

BOOL CDEditOptions::Init(CGenRegMgr *pRegMgr, CString sRegRoot)
{
	// Call the base class
	if (!COptionsBase::Init(pRegMgr, sRegRoot))
	{
		return FALSE;
	}

	if (m_pDisplayOptions)
	{
		m_pDisplayOptions->Init(m_pRegMgr, m_sRegRoot+"\\OptionsDisplay");
		m_optionsArray.Add(m_pDisplayOptions);
	}
	if (m_pAdvancedSelectOptions)
	{
		m_pAdvancedSelectOptions->Init(m_pRegMgr, m_sRegRoot+"\\AdvancedSelect");
		m_optionsArray.Add(m_pAdvancedSelectOptions);
	}
	if (m_pClassDialogOptions)
	{
		m_pClassDialogOptions->Init(m_pRegMgr, m_sRegRoot+"\\ClassDlg");
		m_optionsArray.Add(m_pClassDialogOptions);
	}
	if (m_pRunOptions)
	{
		m_pRunOptions->Init(m_pRegMgr, m_sRegRoot+"\\Run");
		m_optionsArray.Add(m_pRunOptions);
	}
	if (m_pObjectBrowserOptions)
	{
		m_pObjectBrowserOptions->Init(m_pRegMgr, m_sRegRoot+"\\ObjectBrowser");
		m_optionsArray.Add(m_pObjectBrowserOptions);
	}
	if (m_pGenerateUniqueNamesOptions)
	{
		m_pGenerateUniqueNamesOptions->Init(m_pRegMgr, m_sRegRoot+"\\GenerateUniqueNames");
		m_optionsArray.Add(m_pGenerateUniqueNamesOptions);
	}
	if (m_pClipboardOptions)
	{
		m_pClipboardOptions->Init(m_pRegMgr, m_sRegRoot+"\\Clipboard");
		m_optionsArray.Add(m_pClipboardOptions);
	}
	if (m_pWindowsOptions)
	{
		m_pWindowsOptions->Init(m_pRegMgr, m_sRegRoot+"\\Windows");
		m_optionsArray.Add(m_pWindowsOptions);
	}
	if (m_pModelsOptions)
	{
		m_pModelsOptions->Init(m_pRegMgr, m_sRegRoot+"\\Models");
		m_optionsArray.Add(m_pModelsOptions);
	}
	if (m_pControlsOptions)
	{
		m_pControlsOptions->Init(m_pRegMgr, m_sRegRoot+"\\Controls");
		m_optionsArray.Add(m_pControlsOptions);
	}
	if (m_pPrefabsOptions)
	{
		m_pPrefabsOptions->Init(m_pRegMgr, m_sRegRoot+"\\Prefabs");
		m_optionsArray.Add(m_pPrefabsOptions);
	}
	if (m_pMiscOptions)
	{
		m_pMiscOptions->Init(m_pRegMgr, m_sRegRoot+"\\Misc");
		m_optionsArray.Add(m_pMiscOptions);
	}
	if (m_pViewportOptions)
	{
		m_pViewportOptions->Init(m_pRegMgr, m_sRegRoot+"\\Viewports");
		m_optionsArray.Add(m_pViewportOptions);
	}

	return TRUE;
}

/************************************************************************/
// Loads all of the options classes
BOOL CDEditOptions::Load()
{
	// Load the options
	for (uint32 nCurrOption=0; nCurrOption < m_optionsArray.GetSize(); nCurrOption++)
	{
		m_optionsArray[nCurrOption]->Load();
	}
	
	// Load the autosave time
	m_dwAutoSaveTime	= GetDWordValue("AutoSaveTime", 5);
	m_dwNumBackups		= GetDWordValue("NumBackups", 1);
	m_bAutoSave			= GetDWordValue("EnableAutoSave", 0) ? TRUE : FALSE;
	m_sAutoSavePath		= GetStringValue("AutoSavePath", "c:\\DEditBackup");
	m_bDeleteOnClose	= GetDWordValue("DeleteOnClose", 0) ? TRUE : FALSE;

	// First run record
	m_bFirstRun			= GetDWordValue("FirstRun", 1) ? TRUE : FALSE;

	return TRUE;
}

/************************************************************************/
// Saves all of the options classes
BOOL CDEditOptions::Save()
{
	// Save the options
	for (uint32 nCurrOption=0; nCurrOption < m_optionsArray.GetSize(); nCurrOption++)
	{
		m_optionsArray[nCurrOption]->Save();
	}

	// Save the autosave time
	SetDWordValue("AutoSaveTime", m_dwAutoSaveTime);
	SetDWordValue("NumBackups", m_dwNumBackups);
	SetDWordValue("EnableAutoSave", m_bAutoSave);
	SetDWordValue("DeleteOnClose", m_bDeleteOnClose);
	SetStringValue("AutoSavePath", m_sAutoSavePath);

	// Save first run info
	SetDWordValue("FirstRun", m_bFirstRun);

	return TRUE;
}
