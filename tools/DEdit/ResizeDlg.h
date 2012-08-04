//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ResizeDlg.h : header file for the dialog resizing handling classes
//

#ifndef __RESIZEDLG_H__
#define __RESIZEDLG_H__


// Anchor objects which are attached to the controls on the dialog
class CResizeAnchor
{
protected:
	int m_iDlgItem;

public:
	CResizeAnchor(int iDlgItem = 0) : m_iDlgItem(iDlgItem) {};
	virtual ~CResizeAnchor() {};

	// Member access
	virtual void SetDlgItem(int iDlgItem) { m_iDlgItem = iDlgItem; };
	virtual int GetDlgItem() const { return m_iDlgItem; };

	// Lock the anchor to the parent's area
	virtual void Lock(const CWnd *pParent, const CRect &cParentRect) = 0;
	// Resize the anchor
	virtual void Resize(const CWnd *pParent, const CRect &cParentRect) = 0;
};

const int ANCHOR_LEFT = 0;
const int ANCHOR_RIGHT = 1;
const int ANCHOR_TOP = 2;
const int ANCHOR_BOTTOM = 3;
const int ANCHOR_NUMSIDES = 4;

typedef CMoArray<CResizeAnchor *> CResizeAnchorList;

// Anchor class which attaches based on offsets from the edges
class CResizeAnchorOffset : public CResizeAnchor
{
public:
	enum EAnchorStyle {
		// Don't change when resizing (All to top+left)
		eAnchorStatic, 
		// Move with resizing (All to bottom right)
		eAnchorPosition, 
		// Size with resizing (top+left to top+left, bottom+right to bottom+right)
		eAnchorSize, 
		// Move based on some other criteria
		eAnchorOther
	};

protected:
	int m_iOffsets[ANCHOR_NUMSIDES];
	int m_iBindings[ANCHOR_NUMSIDES];

	EAnchorStyle m_eStyle;
public:

	CResizeAnchorOffset(int iDlgItem = 0, EAnchorStyle eStyle = eAnchorStatic);
	virtual ~CResizeAnchorOffset() {};

	// Member access
	virtual void SetBinding(int iSide, int iBinding) { SetStyle(eAnchorOther); m_iBindings[iSide] = iBinding; };
	virtual int GetBinding(int iSide) const { return m_iBindings[iSide]; };

	virtual void SetOffset(int iSide, int iOffset) { m_iOffsets[iSide] = iOffset; };
	virtual int GetOffset(int iSide) const { return m_iOffsets[iSide]; };
	// Reset the offsets to zero
	virtual void ResetOffsets(int iValue = 0); 

	virtual void SetStyle(EAnchorStyle eStyle);
	virtual EAnchorStyle GetStyle() const { return m_eStyle; };

	// Inherited member functions
	virtual void Lock(const CWnd *pParent, const CRect &cParentRect);
	virtual void Resize(const CWnd *pParent, const CRect &cParentRect);
};

// The actual dialog resizer class
class CDlgResizer
{
protected:
	CResizeAnchorList m_cAnchorList;
public:
	CDlgResizer() {};
	virtual ~CDlgResizer();

	// Anchor list access
	virtual void AddAnchor(CResizeAnchor *pAnchor) { if (pAnchor) m_cAnchorList.Add(pAnchor); };
	virtual BOOL RemoveAnchor(const CResizeAnchor *pAnchor);
	virtual const CResizeAnchorList &GetAnchorList() const { return m_cAnchorList; };
	// Find an anchor in the list based on the dialog item ID
	virtual CResizeAnchor *FindAnchor(int iDlgItem) const;

	virtual void Lock(const CWnd *pDialog, const CRect &cRect);
	virtual void Resize(const CWnd *pDialog, const CRect &cRect);
};

#endif //__RESIZEDLG_H__