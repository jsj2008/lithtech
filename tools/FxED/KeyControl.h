#if !defined(AFX_KEYCONTROL_H__AD1F0088_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_KEYCONTROL_H__AD1F0088_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// KeyControl.h : header file
//

// Includes....

#include "linklist.h"

// Structures

struct KEY
{
				KEY()
				{
					m_tmKey		= 0.0f;
					m_val		= 0.0f;
					m_pData		= NULL;
					m_bSelected = FALSE;

					m_tmAnchor  = 0.0f;
					m_valAnchor = 0.0f;
				}
	
	float		m_tmKey;
	float		m_val;

	float		m_tmAnchor;
	float		m_valAnchor;

	void	   *m_pData;
	BOOL		m_bSelected;
};

/////////////////////////////////////////////////////////////////////////////
// CKeyControl window

class CKeyControl : public CStatic
{
// Construction
public:
	CKeyControl();

	public :

		// Member Functions

		virtual void				ClearMemDC();
		void						DrawTimeBar();
		virtual void				DrawKeys(CLinkListNode<KEY> *pSelectNode = NULL);
		void						Redraw(CLinkListNode<KEY> *pSelectNode = NULL);

		void						AddKey(CPoint ptKey);
		virtual void*				GetNewKeyData() { return NULL; }

		virtual void				EditKey(CLinkListNode<KEY> *pNode) { }

		void						TrackKey(CLinkListNode<KEY> *pNode, CPoint ptAnchor, CPoint ptOffset);
		virtual CString				GetTrackValue(CLinkListNode<KEY> *pNode);

		CPoint						KeyToPos(KEY *pKey);
		CLinkListNode<KEY>*			PtInKey(CPoint ptTest);

		// Accessors

		CLinkList<KEY>*				GetKeys() { return &m_collKeys; }
		CKey*						GetKey() { return m_pKey; }

		void						SetKey(CKey *pKey) { m_pKey = pKey; }

	protected :

		// Member Variables

		CDC						   *m_pMemDC;
		CBitmap					   *m_pBitmap;
		CBitmap					   *m_pOldBitmap;

		int							m_cx;
		int							m_cy;

		float						m_tmStart;
		float						m_tmEnd;

		CLinkList<KEY>				m_collKeys;
		CLinkListNode<KEY>		   *m_pMenuKey;

		CKey					   *m_pKey;

	










// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyControl)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKeyControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKeyControl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYCONTROL_H__AD1F0088_8E00_11D2_9B4A_0060971BDAD8__INCLUDED_)
