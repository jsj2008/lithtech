//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __DEDIT_H__
#define __DEDIT_H__


// DEdit.h : main header file for the DEDIT application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "deditoptions.h"
#include "oldtypes.h"
#include "modelmgr.h"

class CMainFrame;
class CDEditOptions;
class CHotKeyDB;


/////////////////////////////////////////////////////////////////////////////
// CDEditApp:
// See DEdit.cpp for the implementation of this class
//
class CButeMgr;
class CDEditApp : public CWinApp
{
	
// Member functions...

public:

	CDEditApp();
	~CDEditApp();
	
	virtual BOOL			PreTranslateMessage(MSG* pMsg);
	void					LoadRegistryStuff();
	void					SaveRegistryStuff();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDEditApp)
public:
		virtual BOOL InitInstance();
		virtual BOOL OnIdle(LONG lCount);
		virtual BOOL SaveAllModified();
//}}AFX_VIRTUAL


// Member data...
	
public:

	char		m_WorkingDir[MAX_PATH];		// The working directory
	char		m_ExeDirectory[MAX_PATH];	// The directory which the DEdit.exe file was run resides	

	// Global preferences.
	BOOL		m_bFullUpdate;
	int			m_TextureDisplaySize;

	//gets the model manager for the application
	CModelMgr&	GetModelMgr()						{ return m_ModelMgr; }


	//sets up the hot key database to reflect a specified database
	void		SetHotKeyDB(const CHotKeyDB& NewDB);

	// Document templates.
	CMultiDocTemplate		*m_pWorldTemplate;

	// Access to member variables
	CDEditOptions			&GetOptions()			{ return m_DEditOptions; }

	//attempts to get a string from the bute help files. It will first try the aggregate
	//if one exists, if that fails it will try the base, and if that fails the function will
	//fail. If it succeeds, the string will be returned in sMatch
	BOOL					GetHelpString(const char* pszTag, const char* pszAttrib, CString& sMatch);

	//this sets the class help bute aggregate. This will be in charge of freeing memory
	//associated with it. Assumes it is allocated with new
	void					 SetClassHelpButeAgg(CButeMgr* pMgr);
	
	//access to the class help aggregate
	CButeMgr				*GetClassHelpButeAgg()	{ return m_pClassHelpButeAgg; }
	
	CButeMgr				*GetClassHelpButeMgr()	{ return m_pClassHelpButeMgr; }


	// Implementation
	//{{AFX_MSG(CDEditApp)
		afx_msg void OnAppAbout();
		afx_msg void OnFileNewProject();
		afx_msg void OnFileSave();
		afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
		afx_msg void OnFileCloseProject();
		afx_msg void OnUpdateFileCloseProject(CCmdUI *pCmdUI);
		afx_msg void OnFileOpen();
		afx_msg void OnFileNewWorld();
		afx_msg void OnUpdateFileNewWorld(CCmdUI *pCmdUI);
		afx_msg void OnFileReloadWorld();
		afx_msg void OnUpdateFileReloadWorld(CCmdUI* pCmdUI);
		afx_msg void OnUpdateFileCloseWorld(CCmdUI *pCmdUI);
		afx_msg void OnFileCloseWorld();
		afx_msg void OnFileSaveWorldAs();
		afx_msg void OnUpdateFileSaveWorldAs(CCmdUI *pCmdUI);
		afx_msg void OnEditKeyConfiguration();
		afx_msg BOOL OnOpenRecentFile(UINT nID);
		afx_msg void OnHelpIntroduction();
		afx_msg void OnHelpCreationGuide();
		afx_msg void OnDEHomePage();
		afx_msg void OnOnlineDoc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:


	CGenRegMgr			m_genRegMgr;
	CDEditOptions		m_DEditOptions;

	CModelMgr			m_ModelMgr;

	// The ButeMgr that is used to read the property help file
	CButeMgr			*m_pClassHelpButeMgr;

	//the aggregate bute mgr so that projects can override the default
	CButeMgr			*m_pClassHelpButeAgg;
};


#include "edithelpers.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

/////////////////////////////////////////////////////////////////////////////


#endif  // __DEDIT_H__


