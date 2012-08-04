#if !defined(AFX_DRAGLIST_H__EF24D841_9B4E_11D3_B959_00609709830E__INCLUDED_)
#define AFX_DRAGLIST_H__EF24D841_9B4E_11D3_B959_00609709830E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Drop notification callback
typedef void (CObject::*FDragListCallback)(int iDropIndex);

/////////////////////////////////////////////////////////////////////////////
// CDragList window

class CDragList : public CListCtrl
{
// Construction
public:
	CDragList();

// Attributes
protected:
	BOOL m_bDragging;
	CImageList *m_pDragImage;
	int m_iDragIndex, m_iDropIndex;
	FDragListCallback m_fDropNotify;
	CObject *m_pDropNotifyObject;

	virtual void ShowDropTarget(int iItem);
	virtual void ClearDropTarget();
public:
	virtual BOOL GetDragging() const { return m_bDragging; };
	virtual CImageList *GetDragImage() const { return m_pDragImage; };

	virtual int GetDragIndex() const { return m_iDragIndex; };
	virtual int GetDropIndex() const { return m_iDropIndex; };

	virtual void SetDropNotify(CObject *pObject, FDragListCallback fNotify) { m_pDropNotifyObject = pObject; m_fDropNotify = fNotify; };
	virtual FDragListCallback GetDropNotify() const { return m_fDropNotify; };
	virtual CObject *GetDropNotifyObject() const { return m_pDropNotifyObject; };

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDragList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDragList)
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGLIST_H__EF24D841_9B4E_11D3_B959_00609709830E__INCLUDED_)
