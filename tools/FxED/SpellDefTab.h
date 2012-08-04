#if !defined(AFX_SPELLDEFTAB_H__28504DE3_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SPELLDEFTAB_H__28504DE3_724F_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpellDefTab.h : header file
//

// Includes....

#include "SpellBaseParmDlg.h"
#include "SpellResolutionDlg.h"
#include "FoundationWnd.h"
#include "TrackWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellDefTab window

class CSpellDefTab : public CTabCtrl
{
// Construction
public:
	CSpellDefTab();


	public :

		// Member Functions

		BOOL						Init();
		BOOL						ShowTab(int nTab);

		CRect						GetDisplayRect() { return m_rcDisp; }

	private :

		// Member Variables

		CSpellBaseParmDlg			m_baseParmDlg;
		CSpellResolutionDlg			m_resolutionDlg;

		CTrackWnd					m_castTrackWnd;
		CTrackWnd					m_activeTrackWnd;
		CTrackWnd					m_resolutionTrackWnd;

//		CSequencerWnd				m_castTrackWnd;
//		CSequencerWnd				m_activeTrackWnd;
//		CSequencerWnd				m_resolutionTrackWnd;

//		CFoundationWnd				m_castTrackWnd;
//		CFoundationWnd				m_activeTrackWnd;
//		CFoundationWnd				m_resolutionTrackWnd;
		

		CBitmap						m_bitmap;
		CDC							m_memDC;

		CPtrArray					m_collWnds;

		CRect						m_rcDisp;


















// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellDefTab)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpellDefTab();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpellDefTab)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLDEFTAB_H__28504DE3_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
