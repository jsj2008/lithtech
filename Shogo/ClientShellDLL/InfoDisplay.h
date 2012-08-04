#ifndef __INFODISPLAY_H
#define __INFODISPLAY_H

#include "ClientUtilities.h"
#include "TextHelper.h"

#define DI_LEFT			0x0001
#define DI_RIGHT		0x0002
#define DI_CENTER		0x0004
#define DI_TOP			0x0008
#define DI_BOTTOM		0x0010

struct DI_INFO
{
	DI_INFO()	{ hSurface = LTNULL; szSurface.cx = 0; szSurface.cy = 0; bDeleteSurface = LTTRUE; nLocationFlags = 0; nExpireTime = 0.0f; pNext = LTNULL; }

	HSURFACE	hSurface;
	CSize		szSurface;
	LTBOOL		bDeleteSurface;
	uint32		nLocationFlags;
	LTFLOAT		nExpireTime;

	DI_INFO*	pNext;
};

class CInfoDisplay
{
public:

	CInfoDisplay()				{ m_pClientDE = LTNULL; m_pInfoList = LTNULL; }
	~CInfoDisplay()				{ Term(); }

	LTBOOL			Init (ILTClient* pClientDE);
	void			Term ();

	// adds the current surface to the draw list without modifying the surface in any way
	LTBOOL			AddInfo (HSURFACE hSurface, LTFLOAT nSeconds = 3.0f, uint32 nLocationFlags = DI_CENTER | DI_BOTTOM, LTBOOL bDeleteSurface = LTTRUE);
	
	// create an optimized surface using a bitmap font and add it to the draw list
	LTBOOL			AddInfo (char* str, CBitmapFont* pFont, LTFLOAT nSeconds = 3.0f, uint32 nLocationFlags = DI_CENTER | DI_BOTTOM);
	LTBOOL			AddInfo (int nStringID, CBitmapFont* pFont, LTFLOAT nSeconds = 3.0f, uint32 nLocationFlags = DI_CENTER | DI_BOTTOM);

	// create an optimized surface using a truetype font and add it to the draw list
	LTBOOL			AddInfo (char* str, FONT* pFontDef, HLTCOLOR hForeColor, LTFLOAT nSeconds = 3.0f, uint32 nLocationFlags = DI_CENTER | DI_BOTTOM);
	LTBOOL			AddInfo (int nStringID, FONT* pFontDef, HLTCOLOR hForeColor, LTFLOAT nSeconds = 3.0f, uint32 nLocationFlags = DI_CENTER | DI_BOTTOM);

	// draw all surfaces in the draw list
	void			Draw();

protected:
	
	LTBOOL			AddToList (HSURFACE hSurface, LTFLOAT nSeconds, uint32 nLocationFlags, LTBOOL bDeleteSurface);

protected:
	
	ILTClient*		m_pClientDE;

	DI_INFO*		m_pInfoList;
};

#endif
