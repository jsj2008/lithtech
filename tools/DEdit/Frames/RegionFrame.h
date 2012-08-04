//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// RegionFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegionFrame frame

// Includes....

class CRegionFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CRegionFrame)
protected:
	CSplitterWnd m_wndSplitter;
	CRegionFrame();           // protected constructor used by dynamic creation


	public:

		// Member Functions
		CSplitterWnd*		GetSplitter()	{ return &m_wndSplitter; }
		BOOL				m_bValid;

	private:




// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegionFrame)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CMDIFrameWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRegionFrame();

	// Generated message map functions
	//{{AFX_MSG(CRegionFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
