#ifndef __PROPERTYWND_H__
#define __PROPERTYWND_H__

/////////////////////////////////////////////////////////////////////////////
// CPropertyWnd window

#ifndef __PROPERTYGROUP_H__
#	include "PropertyGroup.h"
#endif

#ifndef __AUTOTOOLTIPCTRL_H__
#	include "AutoToolTipCtrl.h"
#endif

class CAutoToolTipCtrl;

//control limits
#define MAX_PROPS_PER_GROUP			512

//this is the number of controls that each property can have.
#define MAX_CONTROLS_PER_PROP		2

class IPackerImpl;
class CPropertyMgr;

class CPropertyWnd : public CWnd
{
// Construction
public:
	CPropertyWnd(IPackerImpl* pIPackerImpl, CPropertyMgr* pPropMgr);

// Attributes
public:

// Operations
public:

// Overrides
	//{{AFX_VIRTUAL(CPropertyWnd)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyWnd();

	//determines the height of the m_Font object
	uint32			GetFontHeight();

	//determines the absolute height (this includes the descending area)
	uint32			GetAbsoluteFontHeight();

	//called when the properties need to be recreated most likely because of a group
	//change
	BOOL			CreatePropertyList(CPropertyGroup* pGroup);

	//frees all the properties from this window and erases all attachements to the group
	BOOL			EmptyPropertyList();

	//gets the group associated with the list
	CPropertyGroup*	GetAssociatedGroup();

	//called to update the enabled status of the controls
	BOOL			UpdateControlEnabledStatus();

	//called to update the values inside of each of the controls
	BOOL			UpdateControlValues();

	//called when a property has been modified from the outside (just pass
	//NULL if it is a multiple property change)
	BOOL			PropertyModified(CPackerProperty* pProp);

	//the font for all the items
	CFont			m_Font;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyWnd)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
private:

	//given a group it will run through the group and find the maximum width 
	//of the names in pixels
	uint32			FindMaxNameWidth(CPropertyGroup* pGroup);

	//removes all the controls from the tool tip mgr
	void			RemoveTooltips();
	

	//sets all the control pointers to NULL
	void			SetAllControlsToNull();

	//creates the appropriate controls for a property
	void			CreatePropertyControls(CPackerProperty* pProp, uint32 nProp, uint32 nID, const CRect& rArea);

	//member data

	//the list of static controls for the item name
	CStatic*			m_pPropName[MAX_PROPS_PER_GROUP];

	//the packer that is hooked up to this property list
	IPackerImpl*		m_pIPacker;
	CPropertyMgr*		m_pPropMgr;

	//the controls for each property
	CWnd*				m_pPropControl[MAX_PROPS_PER_GROUP][MAX_CONTROLS_PER_PROP];

	//the number of properties that appear on a page (used for scrolling)
	uint32				m_nPropsPerPage;

	CAutoToolTipCtrl	m_ToolTip;

	//the group that controls these properties
	CPropertyGroup*		m_pGroup;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif
