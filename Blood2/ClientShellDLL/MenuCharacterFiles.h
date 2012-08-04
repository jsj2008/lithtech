//*************************************************************************
//*************************************************************************
//***** MODULE  : MenuCharacterFiles.h
//***** PURPOSE : Blood 2 Character Creation Screen
//***** CREATED : 10/11/98
//*************************************************************************
//*************************************************************************

#if !defined(AFX_MENUCHARACTERFILES_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUCHARACTERFILES_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//*************************************************************************

#include "MenuBase.h"
#include "SharedDefs.h"

//*************************************************************************

class CMenuCharacterFiles : public CMenuBase  
{
	public:
		CMenuCharacterFiles();
		virtual ~CMenuCharacterFiles();	

		// Build the menu
		void	Build();		
		void	SetAction(DBYTE action)		{ m_nAction = action; }

		// Renders the menu to a surface
		void	Render(HSURFACE hDestSurf);

	protected:
		DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

		// Custom control setup functions
		void InitFileList(char *lpszPath);

	private:

		HSURFACE	m_hFileField;
		HDECOLOR	m_hTransColor;
		DBYTE		m_nAction;			// Should we delete or load the file

		CLTGUIEditCtrl	*m_hEdit;		// Handle to the edit ctrl
};

//*************************************************************************

#endif // !defined(AFX_MENUCHARACTER_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)