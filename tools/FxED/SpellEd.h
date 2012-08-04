// SpellEd.h : main header file for the SPELLED application
//

#if !defined(AFX_SPELLED_H__A74036A4_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLED_H__A74036A4_6F17_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "SpellMgr.h"
#include "rezmgr.h"

// Defines....

#define IM_CLOSED					0
#define IM_OPEN						1
#define IM_SPELL					2
#define IM_RESOURCE					3

#define WM_VALUEUPDATE				WM_USER + 1000

#define FLOAT_PRECISION				"%6.4f"
#define	VECTOR_PRECISION			"%4.3f"
#define	COLORKEY_PRECISION			"%1.4f"

// Formats to write files in...

#define FORMAT_TEXT					0
#define	FORMAT_BIN					1

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

/////////////////////////////////////////////////////////////////////////////
// CSpellEdApp:
// See SpellEd.cpp for the implementation of this class
//

// Includes....

#include "fxmgr.h"
#include "butemgr.h"

// Structures....

struct CK_FAVOURITE
{
	CString							m_sName;
	CLinkList<COLOURKEY>			m_collKeys;
};

struct SK_FAVOURITE
{
	CString							m_sName;
	CLinkList<SCALEKEY>				m_collKeys;
	float							m_minScale;
	float							m_maxScale;
};

struct MK_FAVOURITE
{
	CString							m_sName;
	CLinkList<MOVEKEY>				m_collKeys;
};

struct FK_FAVOURITE
{
	CString							m_sName;
	FX_REF							m_ref;
	CFastList<FX_PROP>				m_collProps;
	CLinkList<COLOURKEY>			m_collColourKeys;
	CLinkList<SCALEKEY>				m_collScaleKeys;
	CLinkList<MOVEKEY>				m_collMoveKeys;
	float							m_minScale;
	float							m_maxScale;
	DWORD							m_tmLength;
};

class CSpellEdView;

class CSpellEdApp : public CWinApp
{
public:
	CSpellEdApp();

	public :
	virtual BOOL PreTranslateMessage(MSG* pMsg);

		// Member Functions

		BOOL						LoadSpells(const char *sFilename);
		
		BOOL						LoadSpellmgr(const char *sFilename, CSpellMgr *pSpellMgr);
		BOOL						LoadSpellmgr_t(const char *sFilename, CSpellMgr *pSpellMgr );
		BOOL						LoadSpellmgr_b(const char *sFileName, CSpellMgr *pSpellMgr );

		BOOL						LoadCfgFile(const char *sFilename);

		BOOL						SaveSpellsAs();

		BOOL						SaveSpells(const char *sFilename);
		BOOL						SaveSpells_t(const char *sFileName);
		BOOL						SaveSpells_b(const char *sFileName);

		BOOL						SaveCfgFile(const char *sFilename);

		BOOL						OpenFile(const char* pszFile);

		void						LoadFavourites();
		
		void						SaveFavourites();
		void						SaveColorFavs_b(const char* sFilename);
		void						SaveColorFavs_t(const char* sFilename);
		void						SaveScaleFavs_b(const char* sFilename);
		void						SaveScaleFavs_t(const char* sFilename);
		void						SaveMotionFavs_b(const char* sFilename);
		void						SaveMotionFavs_t(const char* sFilename);
		void						SaveKeyFavs_b(const char* sFilename);
		void						SaveKeyFavs_t(const char* sFilename);

		void						ForceSaveAs() { OnFileSaveAs(); }

		void						ReloadResourceFile();

		void						AddColourFavourites(const char *sFilename);
		void						AddScaleFavourites(const char *sFilename);
		void						AddKeyFavourites(const char *sFilename);
		void						AddMoveFavourites(const char *sFilename);

		void						UpdateViewNames();

		CSpellEdView*				GetViewBySpell(CSpell *pSpell);
		CK_FAVOURITE*				GetColourFavourite(CString sName);

		// Accessors

		CImageList*					GetImageList() { return &m_collImages; }
		CSpellMgr*					GetSpellMgr() { return &m_spellMgr; }
		CFxMgr*						GetFxMgr() { return &m_fxMgr; }
		CRezMgr*					GetRezMgr() { return &m_rezMgr; }
		CDocTemplate*				GetDocTemplate() { return m_pDocTemplate; }

		HCURSOR						GetArrowCursor() { return m_hArrow; }
		HCURSOR						GetLeftCursor() { return m_hLeftCursor; }
		HCURSOR						GetRightCursor() { return m_hRightCursor; }
		HCURSOR						GetLeftRightCursor() { return m_hLeftRightCursor; }

		CLinkList<CK_FAVOURITE *>*	GetColourFavourites() { return &m_collClrFavourites; }
		CLinkList<SK_FAVOURITE *>*	GetScaleFavourites() { return &m_collSclFavourites; }
		CLinkList<FK_FAVOURITE *>*	GetKeyFavourites() { return &m_collKeyFavourites; }
		CLinkList<MK_FAVOURITE *>*	GetMoveFavourites() { return &m_collMvFavourites; }

		int							GetCastAnimTime(int nCastType);
		inline const int			GetFormat() const { return m_wFormat; }

		//given a name it will attempt to open up the file in the specified mode, printing
		//the appropriate messages if it fails.
		FILE*						OpenFile(const char* pszFilename, bool bWrite, bool bText, bool bDisplayErrors = true);

	private :

		// Member Variables

		BOOL						m_bFirstActivation;
		BOOL						m_bHaveFilename;
		CString						m_sDicFile;
		CImageList					m_collImages;
		CSpellMgr					m_spellMgr;
		CFxMgr						m_fxMgr;
		CRezMgr						m_rezMgr;

		CDocTemplate			   *m_pDocTemplate;
		
		HCURSOR						m_hArrow;
		HCURSOR						m_hLeftCursor;
		HCURSOR						m_hRightCursor;
		HCURSOR						m_hLeftRightCursor;

		CLinkList<CK_FAVOURITE *>	m_collClrFavourites;
		CLinkList<SK_FAVOURITE *>	m_collSclFavourites;
		CLinkList<FK_FAVOURITE *>	m_collKeyFavourites;
		CLinkList<MK_FAVOURITE *>	m_collMvFavourites;

		CButeMgr					m_bm;
		int							m_nFastCastSpeed;
		int							m_nMediumCastSpeed;
		int							m_nSlowCastSpeed;

		WORD						m_wFormat;

		//the name of the file that we opened to get the effects from
		CString						m_sSrcFile;

		//the directory where our favorites are located
		CString						m_sFavDir;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellEdApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSpellEdApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileLaunch();
	afx_msg void OnFileImportColourFavourites();
	afx_msg void OnFileImportKeyFavourites();
	afx_msg void OnFileImportScaleFavourties();
	afx_msg void OnFileImportMoveFavourites();
	afx_msg void OnFileReloadresourcefile();
	afx_msg void OnFileEditgameinfo();
	afx_msg void OnFileFindResource();
	afx_msg void OnFormatText();
	afx_msg void OnFormatBinary();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLED_H__A74036A4_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
