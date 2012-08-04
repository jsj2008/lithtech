#ifndef __LEVELITEMSDLG_H__
#define __LEVELITEMSDLG_H__

class CLevelItemsDlg : public CDialog
{
public:

	CLevelItemsDlg();
	~CLevelItemsDlg();

	//this should be called when a region document is closed so that the dialog can
	//potentially clear out its list
	void			NotifyDocumentClosed(CRegionDoc* pDoc);

	//message handlers
	afx_msg void	OnScanLevel();
	afx_msg void	OnSelectItemObject();
	afx_msg void	OnSave();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();

	afx_msg void	OnItemSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult);

	afx_msg void	OnItemObjectSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnSortItemObjects(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnActivateItemObject(NMHDR * pNotifyStruct, LRESULT * pResult);

	//notification when the dialog needs to be set up
	BOOL OnInitDialog();

private:

	//an object that contains an item
	struct CItemObject
	{
		CString					m_sName;
		CString					m_sClass;
		uint32					m_nObjectID;
		uint32					m_nCount;
	};

	//a single item
	struct CItem
	{
		CString					m_sName;
		uint32					m_nCount;
		CMoArray<CItemObject>	m_Objects;
	};

	//called to add an object that references a specific object
	void	AddItemObject(CBaseEditObj* pObject, CWorldNode* pSelNode, const char* pszItemName, uint32 nItemCount);

	//retreives the list control
	CListCtrl*		GetItemList();
	CListCtrl*		GetItemObjectList();

	//updates the list with all the items
	void			UpdateList();

	//update the object list
	void			UpdateObjectList(CItem* pItem);

	//used for handling the specific object types
	void			ProcessAmmoBox(CBaseEditObj* pObject, CWorldNode* pSelNode);
	void			ProcessWeaponItem(CBaseEditObj* pObject, CWorldNode* pSelNode);
	void			ProcessGearItem(CBaseEditObj* pObject, CWorldNode* pSelNode);
	void			ProcessModItem(CBaseEditObj* pObject, CWorldNode* pSelNode);

	void			ProcessObject(CBaseEditObj* pObject, CWorldNode* pSelNode);

	//used for searching through the node heirarchy for objects to add into the list
	void			AddObjectsR(CWorldNode* pCurrNode, CWorldNode* pSelNode);

	//sort callbacks
	static int CALLBACK SortItemCallback(LPARAM lParam1, LPARAM lParam2, LPARAM nCol);
	static int CALLBACK SortItemObjectsCallback(LPARAM lParam1, LPARAM lParam2, LPARAM nCol);

	//our list of items
	CMoArray<CItem>				m_Items;

	//the document that the error list pertains to
	CRegionDoc*		m_pSrcDoc;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif
