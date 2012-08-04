#ifndef __LOADLEVELMENU_H
#define __LOADLEVELMENU_H

#include "BaseMenu.h"

class CLoadLevelMenu : public CBaseMenu
{
public:

	CLoadLevelMenu()	{ m_nFiles = 0; m_pFilenames = LTNULL; m_pSurfaces = LTNULL; m_pSurfacesSelected = LTNULL; m_szSurfaces = LTNULL; }
	~CLoadLevelMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset()		{ m_nTopItem = 0; CBaseMenu::Reset(); }

	int					GetNumFiles()			{ return m_nFiles; }
	
	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }

	virtual void		Up();
	virtual void		Down();
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	virtual void		CheckSelectionOffMenuTop();
	virtual void		CheckSelectionOffMenuBottom();
	
	
	int					CountDatFiles(FileEntry* pFiles);
	void				AddFilesToFilenames(FileEntry* pFiles, char* pPath,  int & nIndex);

protected:

	int					m_nFiles;
	char**				m_pFilenames;
	HSURFACE*			m_pSurfaces;
	HSURFACE*			m_pSurfacesSelected;
	CSize*				m_szSurfaces;
};

#endif
