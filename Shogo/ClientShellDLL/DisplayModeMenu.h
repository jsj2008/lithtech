#ifndef __DISPLAYMODEMENU_H
#define __DISPLAYMODEMENU_H

#include "BaseMenu.h"
#include "DetailSettingsMenu.h"

#define MAX_RESOLUTIONS	12
#define	MAX_RENDERERS	4
#define MAX_RENDERDLLS	8

#define LEN_RENDERDLL	128
#define LEN_RENDERER	128

struct RESOLUTION
{
	RESOLUTION(){ nWidth = 0; nHeight = 0; hSurface = LTNULL; hSurfaceSelected = LTNULL; }

	int			nWidth;
	int			nHeight;
	HSURFACE	hSurface;
	HSURFACE	hSurfaceSelected;
};

struct RENDERER
{
	RENDERER()	{ memset (strRenderer, 0, LEN_RENDERER); memset (strInternalName, 0, LEN_RENDERER); hSurface = LTNULL; hSurfaceSelected = LTNULL; nResolutions = 0; }

	char		strRenderer[LEN_RENDERER];
	char		strInternalName[LEN_RENDERER];
	HSURFACE	hSurface;
	HSURFACE	hSurfaceSelected;

	int			nResolutions;
	RESOLUTION	aResolutions[MAX_RESOLUTIONS];
};

struct RENDERDLL
{
	RENDERDLL()	{ memset (strDllName, 0, LEN_RENDERDLL); hSurface = LTNULL; hSurfaceSelected = LTNULL; nSurfaceWidth = 0; nRenderers = 0; }

	char		strDllName[LEN_RENDERDLL];
	HSURFACE	hSurface;
	HSURFACE	hSurfaceSelected;

	int			nSurfaceWidth;
	int			nRenderers;
	RENDERER	aRenderers[MAX_RENDERERS];
};

class CDisplayModeMenu : public CBaseMenu
{
public:

	CDisplayModeMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	
	virtual LTBOOL		LoadAllSurfaces()		{ return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ UnloadSurfaces(); }
	
	virtual void		Up();
	virtual void		Down();
	virtual void		Left();
	virtual void		Right();
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();
	
	LTBOOL				LoadRealSurfaces();
	LTBOOL				InitRenderDlls();
	void				GetCurrentSettings();
	int					GetClosestResolution (int nWidth, int nHeight);

protected:

	int					m_nSecondColumn;

	int					m_nDrawCount;	
	LTBOOL				m_bRenderersInited;
	
	int					m_nCurrentRenderDll;
	int					m_nCurrentRenderer;
	int					m_nCurrentResolution;
	int					m_nCurrentBitDepth;			// 0 = 16, 1 = 8

	int					m_nOriginalRenderDll;
	int					m_nOriginalRenderer;
	int					m_nOriginalResolution;
	int					m_nOriginalBitDepth;
	
	int					m_nRenderDlls;
	RENDERDLL			m_aRenderDlls[MAX_RENDERDLLS];
	GENERIC_ITEM		m_BitDepth;
};

#endif
