// cfgTbar.h : header file
//

#ifndef __CFGTBAR_H__
#define __CFGTBAR_H__

/////////////////////////////////////////////////////////////////////////////
// CMRCCfgToolBar window

class CMRCCfgToolBar : public CToolBar
{
// Construction
public:
	CMRCCfgToolBar();

// Attributes
public:

protected:	
	UINT * m_pBitmapIds;
	int m_nBitmapButtons;

// Operations
public:
	BOOL SetButtons(UINT * pButtons, int nButtons);
	void SetBitmapIds(UINT * pIds, int nButtons);
	void Customize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCCfgToolBar)
	public:
	virtual BOOL Create( CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP | CCS_ADJUSTABLE, UINT nID = AFX_IDW_TOOLBAR );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMRCCfgToolBar();
	int FindBitmapIndex(UINT nID);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMRCCfgToolBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg void OnTBNBeginAdjust(NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNQueryInsert(NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNQueryDelete(NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNToolBarChange(NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNGetButtonInfo(NMHDR *pNMHDR, LRESULT * pResult);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif