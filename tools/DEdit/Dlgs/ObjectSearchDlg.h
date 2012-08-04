////////////////////////////////////////////////////////////////
//
// ObjectSearchDlg.h
//
// The implementation for the dialog that handles searching property
// strings in objects in order to help level designers find information
// quicker and easier
//
// Author: John O'Rorke
// Created: 7/02/02
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __OBJECTSEARCHDLG_H__
#define __OBJECTSEARCHDLG_H__

#include "stdafx.h"
#include "resource.h"

class CWorldNode;
class CEditRegion;
class CBaseProp;
class CRegionDoc;

class CObjectSearchDlg : 
	public CDialog
{
public:

	CObjectSearchDlg();
	~CObjectSearchDlg();

	//this should be called when a region document is closed so that the dialog can
	//potentially clear out its list
	void NotifyDocumentClosed(CRegionDoc* pDoc);

private:

	//texture information
	struct SSearchResult
	{
		enum ELineType	{	eLineType_ObjectName,
							eLineType_Property
						};

		//the object that this is contained within
		uint32			m_nNodeID;

		//the type of line this is
		ELineType		m_eType;

		//the name of the object or property that this item refers to
		CString			m_sName;

		//the type of the object 
		CString			m_sType;

		//the property value that this might refer to
		CString			m_sPropertyValue;

		//whether or not we can edit properties of this object
		bool			m_bCanEditProps;
	};

	//adds various types
	void			InsertObject(const CString& sObjectName, CWorldNode* pNode, const char* pszType);
	void			InsertProperty(const CString& sPropertyName, const CString& sPropertyVal, CWorldNode* pNode, bool bCanEditProps, const char* pszType);
	void			InsertBlankLine();

	//this will initialize all the columns
	void			InitColumns();

	//accessor for obtaining the list control that holds the list of textures
	CListCtrl*		GetResultsList()		{ return (CListCtrl*)GetDlgItem(IDC_LIST_SEARCH_RESULTS); }

	//given the ID of a button, it will determine if that button is checked or not
	bool			GetCheck(uint32 nID)	{ return !!((CButton*)GetDlgItem(nID))->GetCheck(); }

	//this will search a property based upon the specified parameters and return whether or not it should be included
	bool			SearchProperty(CBaseProp* pProp, const CString& sSearchText, bool bMatchWholeWord, bool bCaseSensitive);

	//This function does the actual searching, and will recurse through the node tree finding
	//and adding selections
	void			ApplySearchR(	CWorldNode* pNode, const CString& sSearchText, const CString& sPrefabPrefix, CWorldNode* pPrefabRoot, 
									bool bMatchWholeWord, bool bCaseSensitive, bool bSearchSelection, bool bSearchPrefabs,
									bool bIgnoreHidden, bool bIgnoreFrozen);

	//given a proprty, this will handle in place editing of the property if applicable
	bool			EditProperty(int nItem, CBaseProp* pProp, SSearchResult* pResult);

	//determines the number of items selected
	uint32			GetNumSelected();

	//handles updating the various controls
	void			UpdateEnabled();

	//standard button handlers
	void			OnOK();
	void			OnCancel();

	//handle initialization and loading of icons
	BOOL			OnInitDialog();

	//clears out the list control of all items
	void			ClearResultsList();

	//handle the user changing the search text field
	afx_msg void	OnSearchTextChanged();

	//handle the button for saving the list to a csv
	afx_msg void	OnButtonSearch();

	//handle when the selection changes
	afx_msg void	OnSelectionChange(NMHDR* pmnh, LRESULT* pResult);

	//handle a double click on a search result
	afx_msg void	OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult);

	//the item data that it indexes into
	CMoArray<SSearchResult>		m_Results;

	//the document where the search results came from
	CRegionDoc*					m_pSrcDoc;

	DECLARE_MESSAGE_MAP()

};

#endif

