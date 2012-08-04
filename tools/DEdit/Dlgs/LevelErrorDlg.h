#ifndef __LEVELERRORDLG_H__
#define __LEVELERRORDLG_H__

#include "levelerrordb.h"

class CLevelErrorDlg : public CDialog
{
public:

	CLevelErrorDlg();
	~CLevelErrorDlg();

	//this should be called when a region document is closed so that the dialog can
	//potentially clear out its list
	void			NotifyDocumentClosed(CRegionDoc* pDoc);

	//message handlers
	afx_msg void	OnScanLevel();
	afx_msg void	OnOptions();
	afx_msg void	OnSelect();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	afx_msg void	OnSelectionChanged(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnListTooltip(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnSortItems(NMHDR * pNotifyStruct, LRESULT * pResult);
	afx_msg void	OnActivateItem(NMHDR * pNotifyStruct, LRESULT * pResult);

	//notification when the dialog needs to be set up
	BOOL OnInitDialog();

private:

	//retreives the list control
	CListCtrl*		GetList();

	//updates the list with all the items
	void	UpdateList();

	//the error database that holds all the currently found errors
	CLevelErrorDB	m_ErrorDB;

	//the document that the error list pertains to
	CRegionDoc*		m_pSrcDoc;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif
