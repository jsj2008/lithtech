//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __PROPERTIESDLG_H__
#define __PROPERTIESDLG_H__


// PropertiesDlg.h : header file
//


// Includes....
#include "proplist.h"
#include "resource.h"
#include "mrcext.h"
#include "projecttabcontrolbar.h"

// Defines....
class CPopupNoteDlg;
class CPropertiesDlg;
class CButeMgr;

class CNotifier
{
	public:
		
		virtual void	NotifyPropertyChange( CBaseProp *pProp, bool bUpdateView )	{}
		virtual void	NotifyPropertiesChange( bool bUpdateView )	{}

};


/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg dialog

class CPropertiesControls : public CDialog
{
// Construction
public:
	CPropertiesControls(CWnd* pParent = NULL);   // standard constructor
	CPropertiesControls(UINT id, CPropList *pList, CNotifier *pNotifier);

	~CPropertiesControls();
		
	void			InitMe();

	// Sets up everything to edit the new list of properties.
	BOOL			Init( CPropList *pList, CNotifier *pNotifier );

	// Clears the dialog.
	void			Term( BOOL bClear=TRUE );
	void			TermIf( CPropList *pList, BOOL bClear=TRUE );

	// Re-reads in values from the property list.
	void			ReadControlValues();

	void			Redraw();
	
	CPropList*		GetPropList()	{ return m_pPropList; }
	void OnOK();
	BOOL			OnInitDialog();

	void			GetControlRect( int iControl, CRect &rect );
	CPropList		*m_pPropList;
	DWORD			m_nProperties;
	
	void			NotifyChange( CBaseProp *pProp = NULL, BOOL bUpdateView = TRUE );

protected:

	//handles custom updating for position and rotation props to allow updating to be applied
	//on objects as the position changes
	bool			HandlePosVectorProp(CVectorProp* pVecProp);
	bool			HandleRotationProp(CRotationProp* pRotProp);

	void			GetLabelRect( int iLabel, CRect &rect );
	void			CreateControls();

	// Creates a static label
	CStatic			*CreateStaticLabel(CBaseProp *pProp, int nControlIndex);

	// This uses a file open dialog to browse for a file and place it
	// into the string property.
	//
	// Returns:	TRUE  - The file was succesfully found and copied into the property
	//			FALSE - An error occured or cancel was selected on the browse dialog box
	BOOL			BrowseForFile(CStringProp *pProp);		

	// This function is called to browse for an object using the
	// object browser.
	//
	// Returns:	TRUE  - OK was pressed on the browser
	//			FALSE - OK was not pressed to close the browser
	BOOL			BrowseForObject(CStringProp *pStringProp);

	// Returns the help string for a specific property
	CString			GetPropHelpString(CBaseProp *pProp);

	// Fills CStringArray with the selected object class names that have the specified property.
	void			GetClassNamesFromProperty(CBaseProp *pProp, CStringArray &destNameArray);

	// Get the base class names for the classes specified in nameArray and
	// put them into the baseClassNames array
	void			GetBaseClassNames(CStringArray &classNameArray, CStringArray &destBaseArray);

protected:

	// Created upon initialization.
	CNotifier			*m_pNotifier;
	CFont				*m_pFont;


	BOOL				m_bModal;

	// Created upon Init().
	int					m_ItemWidth, m_ItemHeight;


	//the currently active property
	CBaseProp			*m_pCurProp;

	//keeps track of if the currently active string has been edited
	bool				m_bStringModified;
		
	// The controls and the label controls
	DWORD				m_nControls;
	CWnd				**m_Controls;
	CStatic				**m_ppLabelControls;	
	
	// The help string for each control
	CStringArray		m_sControlHelpArray;
		
	// The popup note dialog
	CPopupNoteDlg		*m_pPopupNoteDlg;

	// The current property help string
	CString				m_sPropHelpString;

	BOOL				m_bReadingControlValues;

	int					m_iCurPos;
	int					m_iMaxPos;
	int					m_iPage;

// Dialog Data
	//{{AFX_DATA(CPropertiesControls)
	enum { IDD = IDD_PROPCONTROLS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesControls)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void PlaceControls( );
	void UpdateScrollBars( );

	// Generated message map functions
	//{{AFX_MSG(CPropertiesControls)
	afx_msg void OnPaint();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CWorldNode;
class CPropertiesDlg : public CProjectTabControlBar
{
// Construction
public:
	CPropertiesDlg();   // standard constructor
	~CPropertiesDlg();


	// Sets up everything to edit the new list of properties.
	BOOL			Init( CPropList *pList, CNotifier *pNotifier, char *pClassName );

	// Clears the dialog.
	void			Term( BOOL bClear=TRUE );
	void			TermIf( CPropList *pList, BOOL bClear=TRUE );

	// Re-reads in values from the property list.
	void			ReadControlValues();

	void			Redraw();

	void			OnOK();
	
	CPropList*		GetPropList()	{ return m_pPropControls->GetPropList( ); }

protected:
	// Returns the number of object nodes that are selected
	BOOL			GetNumObjectNodesSelected();

	// Deselects any nodes that are not objects
	void			DeselectNonObjectNodes();

	// Changes the class of the selected objects
	void			DoChangeClass(CString sClassName);	

protected:
	// Created upon initialization.
	CFont					*m_pFont;

	DWORD					m_dwVertPad;
	CPropertiesControls		*m_pPropControls;

	// The popup note dialog
	CPopupNoteDlg			*m_pPopupNoteDlg;

	// The current classname
	CString					m_sClassName;

	// The help string for the current class
	CString					m_sCurrentClassHelp;

// Dialog Data
	//{{AFX_DATA(CPropertiesDlg)
	enum { IDD = IDD_PROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesDlg)
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	// Repositions the controls
	void	RepositionControls();

	// Generated message map functions
	//{{AFX_MSG(CPropertiesDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonChangeClass();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif  // __PROPERTIESDLG_H__



