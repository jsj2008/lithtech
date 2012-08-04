#if !defined(AFX_TIMEBARDLG_H__F2D178A7_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_TIMEBARDLG_H__F2D178A7_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeBarDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTimeBarDlg dialog

class CTimeBarDlg : public CDialog
{
// Construction
public:
	CTimeBarDlg(CWnd* pParent = NULL);   // standard constructor


	public :

		// Member Functions

		void					DrawTimeBar(CDC *pDC);

		int						PosToTime(int pos) { return (int)((float)pos / m_fTmPxRatio); }
		int						TimeToPos(int nTime) { return (int)((float)nTime * m_fTmPxRatio); }

		void					SetTotalTime(int nTotalTime);
		void					SetTimePixelRatio();

		void					RedrawParent();

		// Accessors

		int						GetTotalTime() { return m_nTotalTime; }
		float					GetScale() { return m_fScale; }
		int						GetTimeAnchor() { return m_nTimeAnchor; }

		void					SetScale(float fScale) { m_fScale = fScale; }

	private :

		// Member Variables

		int						m_nTimeAnchor;
		int						m_nTotalTime;
		float					m_fScale;
		float					m_fTmPxRatio;
		CDC					   *m_pMemDC;
		CBitmap				   *m_pBitmap;
		CBitmap				   *m_pOldBitmap;
















// Dialog Data
	//{{AFX_DATA(CTimeBarDlg)
	enum { IDD = IDD_TIMEBAR };
	CSliderCtrl	m_timeScroll;
	CSliderCtrl	m_timeScale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeBarDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTimeBarDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMEBARDLG_H__F2D178A7_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
