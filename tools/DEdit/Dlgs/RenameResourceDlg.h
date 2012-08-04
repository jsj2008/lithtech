////////////////////////////////////////////////////////////////
//
// renameresourcedlg.h
//
// The implementation for the dialog that handles renaming and
// moving textures, updating levels appropriately
//
// Author: John O'Rorke
// Created: 7/10/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __RENAMERESOURCEDLG_H__
#define __RENAMERESOURCEDLG_H__

#include "stdafx.h"
#include "resource.h"

class CFilePalette;
class CLTANode;
class ILTAAllocator;
class CRegionDoc;
class CWorldNode;


enum	EResourceAction
{
	eResAction_Move,
	eResAction_Replace
};


class CRenameResourceDlg : 
	public CDialog
{
public:

	CRenameResourceDlg(CWnd* pParentWnd);
	~CRenameResourceDlg();

	//given the name of a file, it will add it to the list appropriately
	void			AddNewFile(const char* pszFile);

	//initializes our icon bitmap. Should be called after new files have been added
	void			UpdateIcons();

	//frees all the items in the list
	void			FreeAllItems();

private:

	//this value will supress the edit from changing the listbox items
	//it should be set to true while the change notifications should
	//be ignored
	bool			m_bIgnoreEditChanges;

	//The bitmap that holds the icon images
	CBitmap			*m_pIconBitmap;

	//Image lists for the items in the list
	CImageList		m_IconList;

	//accessor for obtaining the list control that holds the resources
	CListCtrl*		GetResList()		{ return (CListCtrl*)GetDlgItem(IDC_LIST_RES_RENAME); }

	//determines the number of items selected
	uint32			GetNumSelected();

	//handles updating the edit controls to reflect a change in selection
	void			UpdateResEdits();

	//given the index to an item, it will update the data in the columns
	//to reflect the change
	void			UpdateItem(int nItem);

	//given the index to an item, it will remove it from the list
	void			RemoveItem(int nItem);

	//given a level it will update the textures to reflect the changes in resource names
	bool			UpdateOpenWorldTextures(CRegionDoc* pDoc);
	
	//given a level it will update the prefabs to reflect the changes in resource names
	bool			UpdateOpenWorldPrefabs(CRegionDoc* pDoc);

	//given a loaded world LTA, it will adjust the tree accordingly for texture changes
	//will return true if it modified the tree
	bool			UpdateWorldTexturesLTA(CLTANode* pWorld, ILTAAllocator* pAllocator);

	//Updates all prefab references in the specified LTA file, and returns if it modified the
	//file or not
	bool			UpdateWorldPrefabsLTA(CLTANode* pWorld, ILTAAllocator* pAllocator);


	//given an LTA it will open it up, and if it is a world, will modify the changes
	//and save it back out to the same file. Returns true if the file was modified
	bool			UpdateLTA(const char* pszFile);

	//given the document of a currently open world, it will update it to reflect the new
	//changes. Returns true if the level was modified
	bool			UpdateOpenLevel(CRegionDoc* pDoc);

	//determines if based upon the current dialog settings whether or not levels on disk should
	//be modified
	bool			ShouldUpdateAllLevels();

	//this will recursively go through the specified directory and subdirectories
	//looking for LTA files, and for each one it will call UpdateLTA
	// If open levels are to be updated, any levels upen in the editor will be modified
	// If closed files are to be updated, any LTA that is not open will be modified
	//returns the number of levels that were modified
	uint32			UpdateAllLevels(const char* pszDir, bool bUpdateClosedLevels);

	//updates the project bar to reflect changes. 
	bool			UpdateProjectBar();

	//handles the actual moving of the resources. This will take all items in the
	//list, move them from the source to the destination
	void			MoveResources();

	//handles actually changing the action on the selection
	void			OnActionChanged(EResourceAction eAction);

	//set the radio button to a specified action (-1 clears all)
	void			SetAction(int nSelAction);

	//standard button handlers
	void			OnOK();
	void			OnCancel();

	//handles updating the various controls
	afx_msg void	UpdateEnabled();

	//handles the action radio buttons
	afx_msg void	OnRadioActionMove();
	afx_msg void	OnRadioActionReplace();

	//handle the button for adding resources to the list
	afx_msg void	OnAddResources();

	//handle the button that clears the entire list
	afx_msg void	OnClear();

	//handle the button for removing resources from the list
	afx_msg void	OnRemoveResources();

	//handle when the selection changes
	afx_msg void	OnSelectionChange(NMHDR* pmnh, LRESULT* pResult);

	//handle when one of the resource edit fields is changed
	afx_msg void	OnResEditChange();

	//handle browsing for an output directory
	afx_msg void	OnBrowseDestDir();

	//handle browsing for an output name
	afx_msg void	OnBrowseDestName();

	//handle browsing for the starting directory
	afx_msg void	OnBrowseLevelDir();

	//handle initialization and loading of icons
	BOOL			OnInitDialog();

	DECLARE_MESSAGE_MAP()

};

#endif

