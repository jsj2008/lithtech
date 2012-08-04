#if !defined(AFX_MOTIONKEYSDLG_H__AD1F0089_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_MOTIONKEYSDLG_H__AD1F0089_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MotionKeysDlg.h : header file
//

// Includes....

#include "keycontrol.h"
#include "linklist.h"
#include "matrix.h"
#include "intbutton.h"

// Defines....

#define	VT_PERSPECTIVE				0
#define VT_BACK						1
#define VT_TOP						2
#define VT_RIGHT					3

#define CM_CREATE					0
#define CM_SELECT					1
#define CM_MOVE						2
#define CM_DELETE					3
#define CM_SPLIT					4
#define CM_ROTATE					5
#define CM_SCALE					6

/////////////////////////////////////////////////////////////////////////////
// CMotionKeysDlg dialog

class CMotionKeysDlg : public CDialog
{
// Construction
public:
	CMotionKeysDlg(CKey *pKey, CWnd* pParent = NULL);   // standard constructor

	public :

		void						DrawView();
		void						SetViewType(int nViewType);
		void						TrackPerspectiveView(CPoint ptAnchor);

		void						SetCurrentMode(int nNewMode);

		void						Project(CFXVector *pVec);

		CLinkListNode<MOVEKEY>*		GetKeyByPos(CPoint ptClick);

	private :
	
		CKey					   *m_pKey;

		CDC						   *m_pMemDC;
		CBitmap					   *m_pBitmap;
		CBitmap					   *m_pOldBitmap;
		
		BOOL						m_bShowTrack;
		CRect						m_rcTrack;

		int							m_sx;
		int							m_sy;

		int							m_nViewType;
		char						m_sViewType[32];
		CFXMatrix					m_mView;

		float						m_xRot;
		float						m_yRot;
		float						m_fzDist;
	
		CLinkList<MOVEKEY>			m_collKeys;
		BOOL						m_bUsePreset;
		int							m_nPresetAnim;
		int							m_nSize;
		int							m_nReps;

		BOOL						m_bShowPaths;
		BOOL						m_bShowMarkers;
		BOOL						m_bShowNumbers;

		int							m_nCurMode;


























// Dialog Data
	//{{AFX_DATA(CMotionKeysDlg)
	enum { IDD = IDD_MOTIONKEYS };
	CIntButton	m_reps;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionKeysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMotionKeysDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnShowmarkers();
	afx_msg void OnShownumbers();
	afx_msg void OnShowpaths();
	afx_msg void OnDeleteall();
	afx_msg void OnMirrorx();
	afx_msg void OnMirrory();
	afx_msg void OnMirrorz();
	afx_msg void OnCreateKey();
	afx_msg void OnDeleteSingleKey();
	afx_msg void OnMoveKey();
	afx_msg void OnSelectKey();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectAll();
	afx_msg void OnUsePreset();
	afx_msg void OnAddToMoveFavourites();
	afx_msg void OnChooseMoveFavourite();
	afx_msg void OnSplit();
	afx_msg void OnRotate();
	afx_msg void OnScale();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONKEYSDLG_H__AD1F0089_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_)
