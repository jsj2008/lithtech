#ifndef __EVENTLISTDLG_H__
#define __EVENTLISTDLG_H__

class CObjectEvent;

/////////////////////////////////////////////////////////////////////////////
// CEventListDlg dialog
//
// Dialog that handles the viewing of events in a list manner

class CEventListDlg : public CDialog
{
// Construction
public:

	//note: This should be kept in sync with the constant in the event editor
	//dialog. It is duplicated hear to prevent a cyclic dependency.
	enum	{ MAX_TRACKS		= 8 };

	CEventListDlg(CObjectEvent* pEvents, CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_EVENTLIST };

	//the textual names for all of the tracks
	CString m_sTrackNames[MAX_TRACKS];

protected:

	//accessors for a variety of controls
	CListCtrl*			GetListCtrl()			{ return ((CListCtrl*)GetDlgItem(IDC_LIST_EVENTS)); }

	virtual BOOL OnInitDialog();

	//the list of events
	CObjectEvent*		m_pHeadEvent;

	// Generated message map functions
	//{{AFX_MSG(CEventEditorDlg)
		afx_msg void OnEndLabelEdit(NMHDR* pNotifyStruct, LRESULT* result);
		afx_msg void OnColumnClicked(NMHDR* pNotifyStruct, LRESULT* result);
		afx_msg void OnItemActivated(NMHDR* pNotifyStruct, LRESULT* result);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif 
