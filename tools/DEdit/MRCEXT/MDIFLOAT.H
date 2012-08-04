// mdifloat.h : header file
// $Revision:   1.0  $
// $Date:   23 May 1996 16:45:46  $
// $Author:   MRC  $

/////////////////////////////////////////////////////////////////////////////
// CMRCMDIFloatWnd frame

#define CMDIFloat_Parent CMDIChildWnd

class CMRCMDIFloatWnd : public CMDIFloat_Parent
{
	DECLARE_DYNCREATE(CMRCMDIFloatWnd)
protected:
	CMRCMDIFloatWnd();           // protected constructor used by dynamic creation

public:
	CDockBar	m_wndMDIDockBar;		// shouldn't need to be a CSizeDockBar

// Attributes
public:

// Operations
public:
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);
	void RecalcLayout(BOOL bNotify = TRUE);

// Overrides
	BOOL PreCreateWindow(CREATESTRUCT& cs); 
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCMDIFloatWnd)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMRCMDIFloatWnd();

	// Generated message map functions
	//{{AFX_MSG(CMRCMDIFloatWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
