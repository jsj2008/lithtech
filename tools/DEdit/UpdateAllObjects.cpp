#include "bdefs.h"
#include "dedit.h"
#include "dirdialog.h"
#include "EditProjectMgr.h"
#include "FileUtils.h"
#include "ProjectBar.h"
#include "optionsdisplay.h"

//calling this function will prompt the user for a directory to update prefabs
//under and will recursively traverse that directory structure looking for world
//LT* files and upon finding them, will open them, update the object properties,
//save them and close them. This will keep all prefabs in sync with the object
//LTO information
bool UpdateAllLevelObjects()
{
	//make sure that we have a project open
	if(GetProject() == NULL)
		return false;

	//first off, determine the directory we want to recurse under
	CDirDialog Dlg;
	Dlg.m_hwndOwner		= GetMainFrame()->m_hWnd;
	Dlg.m_strInitDir	= GetProject()->m_BaseProjectDir;
	Dlg.m_strTitle		= "Select a directory to update objects under";

	if(Dlg.DoBrowse() == false)
	{
		//the user canceled, let them go
		return true;
	}

	CWaitCursor WaitCursor;

	//now we can use this directory to recurse under and get the levels
	CMoArray<CString>	LevelList;

	CFileUtils::GetAllWorldFiles(Dlg.m_strPath, LevelList);

	//now we need to run through all levels, open them, modify them, save them, and close them
	for(uint32 nCurrLevel = 0; nCurrLevel < LevelList.GetSize(); nCurrLevel++)
	{
		CString sFile = LevelList[nCurrLevel];

		//first open the level and sync the objects
		CRegionDoc* pLevel = GetProjectBar()->OpenRegionDoc(sFile, true);

		//now that it has been modified, we need to save it
		if(!pLevel->SaveLTA(true))
		{
			//present a message box?
		}

		//and finally, we can close it		
		GetApp()->m_pWorldTemplate->RemoveDocument( pLevel );
		pLevel->OnCloseDocument();
	}

	//success
	return true;
}