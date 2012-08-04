#if !defined(AFX_CHOOSECLRANIMLIST_H__E1AB8B02_8876_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_CHOOSECLRANIMLIST_H__E1AB8B02_8876_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseClrAnimList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseClrAnimList window

class CChooseClrAnimList : public CListBox
{
// Construction
public:
	CChooseClrAnimList();

	void						PosToCol(CK_FAVOURITE *pFavourite, int xPos, int *pRed, int *pGreen, int *pBlue);

	float						m_posColRatio;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseClrAnimList)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChooseClrAnimList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChooseClrAnimList)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSECLRANIMLIST_H__E1AB8B02_8876_11D2_9B4A_0060971BDAD8__INCLUDED_)
