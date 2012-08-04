//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __REGIONDOC_H__
#define __REGIONDOC_H__


// RegionDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegionDoc document


#include "editpoly.h"
#include "editregion.h"
#include "editbrush.h"
#include "refs.h"
#include "editprojectmgr.h"
#include "propertiesdlg.h"
#include "editclipboard.h"
#include "undo_mgr.h"

class CEditRegion;

typedef CLinkedList<CVertRef> VertRefLinkedList;
typedef GenHash<CVertRef, VertRefLinkedList> VertRefHash;


// Used for the CVertRef hashed table.
class VertRefHashHelper : public GenHashHelper<CVertRef>
{
public:
	
	virtual DWORD	GetHashCode(const CVertRef &theObject)
	{
		return (DWORD)theObject.m_iVert + ((DWORD)theObject.m_pBrush >> 2);
	}

	virtual LTBOOL	Compare(const CVertRef &object1, const CVertRef &object2)
	{
		return object1.m_pBrush == object2.m_pBrush &&
			object1.m_iVert == object2.m_iVert;
	}
};		


struct BatchTextureScaleInfo;

class CRegionDoc : public CDocument, public CNotifier
{
	friend class CProjectBar;

protected:
	CRegionDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRegionDoc)




// Attributes
public:

	void				NotifyPropertyChange( CBaseProp *pProp, bool bUpdateView );
	void				NotifyPropertiesChange( bool bUpdateView );


	void				InitUndoBuffers();
	void				TermUndoBuffers();

	//called to have all the properties in objects checked in the proper order for reporting
	//errors back to the debug window
	void				UpdateAllObjectProperties();

	//sets the title for this document for how it shows up in the display
	void				SetTitle(bool bModified);
	void				SetTitleKeepModified();
	
	bool				SaveLTA(bool bFullSave);
	bool				SaveOBJ(bool bFullSave);

	void				StartPreProcessor();
	BOOL				SaveModifiedLTA() { return SaveModified(); }

	CRegionView*		CreateNewRegionView();
 
	bool				LoadLTAIntoRegion( const char* filename, bool bForceSyncObjects, bool bAllowPropSync);

	CEditRegion*		GetRegion()		{ return &m_Region; }

	// Sets the modified flag and (possibly) adds an undo to the buffer.
	void				Modify(CPreAction *pAction=NULL, bool bDeleteAction=false);
	
	// Modify with an action list.  If you pass in NULL or a list with 0 actions, it
	// won't modify the document.
	void				Modify(PreActionList *pActionList, bool bDeleteList=FALSE);

	// Automatically sets up an (ACTION_MODIFYNODE) undo list for all the current selections.
	void				SetupUndoForSelections(bool bOnlyOfType=FALSE, int type=0);

	//invalidates all views of this document causing them to be redrawn
	void				RedrawAllViews( DWORD extraHint=0 );

	//invalidates all perspective views of this document causing them to
	//be redrawn. This should be done for texture operations, and other
	//things which will not be reflected in wireframe modes
	void				RedrawPerspectiveViews(DWORD extraHint=0 );

	void				UpdateSelectionBox( LPARAM extraHint=0 );
	void				NotifySelectionChange();
	void				SetupPropertiesDlg(bool bShow);

	void				SynchronizePropertyLists( CEditRegion *pRegion, bool bForceSync );
	bool				CheckSynchronize();
	
	// Adds an object to the document
	CBaseEditObj		*AddObject(const char *pClassName, CVector vPos);

	// Binds an object
	void				BindNode(CWorldNode *pChild, CWorldNode *pParent, PreActionList *pActionList = NULL);
	void				BindNodeToSelected(CWorldNode *pNode, PreActionList *pActionList = NULL);

	// Selects a node
	void				SelectNode(CWorldNode *pNode, bool bClearSelections=TRUE);
	
	// Moves the selected nodes to the indicated parent node.
	// This sets up the undo information as well.
	void				MoveSelectedNodes(CWorldNode *pParentNode);

	// Converts an object from one class to another.  A pointer to the object is then returned.
	CBaseEditObj		*ConvertObjectClass(CBaseEditObj *pOldObject, const char *lpszClassName);

	// resize texture mapping coordinates based on a new texture size
	void				TextureScale( const CMoArray<BatchTextureScaleInfo*>& textureInfo, CWorldNode* node=NULL );

	// Clone the selected nodes (Note : This uses the clipboard to make the clone)
	void				Clone();
 
	//updates the backup name to reflect any changes in the filename or 
	//to the autosave options
	void				UpdateBackupName(const char* pszFilename);

	// Select the brushes which are using a given texture
	void				SelectBrushesByTexture(const char *pFilename);

	// Tag the indicated polygon.  Note that this handles filtering out polys that
	// are already tagged.
	void				TagPoly(const CPolyRef &cPoly);

	//this will setup the documents backup file name information as well as
	//open up the specified LTA and load it into the region. This is essentially
	//the internals to OnOpenDocument, but are broken out to allow forcing 
	//synchronization without breaking the overriding of OnOpenDocument
	//from CDocument
	bool				InitDocument(LPCTSTR lpszPathname, bool bForceSync, bool bAllowPropSync);



public:	

	//the ratio of the XY coordinate of the center of the splitter view. These
	//ranges from 0 to 1 and are used when switching to full screen views, and then
	//switching back
	float				m_fSavedSplitCenterX;
	float				m_fSavedSplitCenterY;

	// Used while attempting to synchronize property lists.
	bool				m_bCanSynchronize;

	// World name (like 'World1')
	CString				m_WorldName;
	
	// The full filename for the world (like 'c:\proj\blah\World1.lta').
	CString				m_FileName;
	CString				m_BackupName;

	// Current selection boundaries.
	CVector				m_SelectionMin, m_SelectionMax;


	// All the selection stuff.
	CVertRef					m_ImmediateVertexTag;
	CEdgeRef					m_ImmediateEdgeTag;
	CBrushRef					m_ImmediateBrushTag;
	
	CEditBrush					m_DrawingBrush;
	
	VertRefHash					m_TaggedVerts;
	VertRefHashHelper			m_TaggedVertsHelper;

	CBrushRefArray				m_TaggedBrushes;
	
	// Note: there is ALWAYS one entry in here.  It is the immediate poly.
	CPolyRefArray				m_TaggedPolies;


public:

	CEditRegion					m_Region;
	CStringHolder				m_RegionStringHolder;


	// Backups for the undo buffer.
	CUndoMgr					m_UndoMgr;
			
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegionDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	protected:
	virtual BOOL OnNewDocument();
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRegionDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CRegionDoc)
	afx_msg void	OnUpdateEditCopy( CCmdUI *pCmdUI );
	afx_msg void	OnEditCopy();
	afx_msg void	OnUpdateEditCut( CCmdUI *pCmdUI );
	afx_msg void	OnEditCut();
	afx_msg void	OnUpdateEditPaste( CCmdUI *pCmdUI );
	afx_msg void	OnEditPaste();
	afx_msg void	OnUpdateEditStamp( CCmdUI *pCmdUI );
	afx_msg void	OnEditStamp();
afx_msg void OnEditPasteAlternate();
afx_msg void OnUpdateEditPasteAlternate(CCmdUI* pCmdUI);
//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif  // __REGIONDOC_H__


