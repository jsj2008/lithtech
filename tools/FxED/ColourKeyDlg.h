#if !defined(AFX_COLOURKEYDLG_H__0BBD1181_8461_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_COLOURKEYDLG_H__0BBD1181_8461_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColourKeyDlg.h : header file
//

// Includes....

#include "LinkList.h"
#include "key.h"

/////////////////////////////////////////////////////////////////////////////
// CColourKeyDlg dialog

class CColourKeyDlg : public CDialog
{
// Construction
public:
	CColourKeyDlg(CLinkList<COLOURKEY> *pList, CKey *pKey, CWnd* pParent = NULL);   // standard constructor

	public :

		// Member Functions

		void							DrawKeys(CDC *pDC, BOOL bShowVal = FALSE);
		void							PosToCol(int xPos, int *pRed, int *pGreen, int *pBlue);
		void							TrackKey(CLinkListNode<COLOURKEY> *pNode);
		void							SelectNode(CLinkListNode<COLOURKEY> *pSelNode);
		void							DrawTimeBar();

		// Accessors

		CLinkList<COLOURKEY>*			GetKeys() { return &m_collKeys; }

	private :

		// Member Variables

		CKey						   *m_pKey;
		CDC								m_memDC;
		CBitmap							m_bitmap;
		CBitmap						   *m_pOldBitmap;
		CLinkList<COLOURKEY>			m_collKeys;
		float							m_posColRatio;
		CLinkListNode<COLOURKEY>       *m_pSelKey;










// Dialog Data
	//{{AFX_DATA(CColourKeyDlg)
	enum { IDD = IDD_COLOURKEYS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CColourKeyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFullOpacity();
	afx_msg void OnFullTranslucency();
	afx_msg void OnHalfAndHalf();
	afx_msg void OnChooseFavourite();
	afx_msg void OnAddToFavourites();
	afx_msg void OnReset();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeleteKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOURKEYDLG_H__0BBD1181_8461_11D2_9B4A_0060971BDAD8__INCLUDED_)
