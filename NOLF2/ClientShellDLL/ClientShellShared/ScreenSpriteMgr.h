// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSpriteMgr.h
//
// PURPOSE : Manage all game-side handling of 2d (screenspace) sprites
//
// CREATED : 12/7/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREEN_SPRITE_MGR_H__
#define __SCREEN_SPRITE_MGR_H__

#include "iltdrawprim.h"

class CScreenSpriteMgr;
extern CScreenSpriteMgr *g_pScreenSpriteMgr;

typedef enum
{
	// default layer, the very back
	SPRITELAYER_DEFAULT = 0,

	// the layers to a subroutine: subroutine bitmap, build state, and condition
	SPRITELAYER_SUBROUTINE_SHAPE,
	SPRITELAYER_SUBROUTINE_BUILD,
	SPRITELAYER_SUBROUTINE_CONDITION,

	// the layers to a subroutine
	SPRITELAYER_ADDITIVE_SHAPE,
	SPRITELAYER_ADDITIVE_GLOW,
	SPRITELAYER_ADDITIVE_HIGHLIGHT,

	// procedurals
	SPRITELAYER_PROCEDURAL_SHAPE,
	SPRITELAYER_PROCEDURAL_PROGRESS,
	SPRITELAYER_PROCEDURAL_HIGHLIGHT,

	// at the very front, the three layers to a mouse cursor
	SPRITELAYER_CURSOR_ADDITIVE,
	SPRITELAYER_CURSOR_BACKGROUND,
	SPRITELAYER_CURSOR_FOREGROUND,
} ScreenSpriteLayer;

// Wrapper struct for HTEXTUREs that expose a bit more information on our end
// Managed by the CScreenSpriteMgr class.  One frame refers to a single texture
// that can be used by multiple sprites

struct ScreenSpriteFrame
{
	int			iFrameID;		// unique ID number (it's an array index)
	char		*pName;
	HTEXTURE	hTex;
	unsigned int iWidth;
	unsigned int iHeight;
};

typedef std::vector<ScreenSpriteFrame *> FrameArray;

class CScreenSprite
{
	friend class CScreenSpriteMgr;
public:
	CScreenSprite();
	~CScreenSprite();

//	LTBOOL		Init(char *pName, LTBOOL bExpires = LTTRUE);

	void		Show(LTBOOL bShow, LTBOOL bRestartAnimation = LTTRUE);

	void		SetAdditive(LTBOOL bAdditive) {m_bAdditive = bAdditive;}
	void		SetCenter(int x, int y);
	void		SetCenter(LTIntPt pos);

	void		SetPosition(int x, int y);
	void		SetPosition(LTIntPt pos);
	LTIntPt		GetPosition();
	LTIntPt		GetDims();

	char *		GetName() {return m_pName;}

	void		SetLifeTime(int iLoops);
	void		SetLifeTime(float fSeconds);

	LTBOOL		KillMe() {return (m_bExpires && (m_fLifeTime < 0.0f));}

private:

	// Private functions used by ScreenSpriteMgr
	void		Init();
	void		SetExpiration(bool bExpires);
	void		Term();

	void		AdvanceTime(float fDelta);	// Tell the sprite how much time has elapsed
	void		Render();			// Render the sprite,called from the Mgr

	ScreenSpriteLayer m_eLayer;		// sprite layer to determine draw-order

	LTIntPt		m_Position;
	LTIntPt		m_Center;			// Offset from top-left corner of bitmap to center of sprite (0,0)
	char		*m_pName;			// filename (spr or dtx)
	int			m_nFrames;			// Number of frames in the sprite
	int			m_nFrameRate;		// frame rate (in frames per second) for animation
	float		m_fOneFrameTime;	// Time in seconds for a single frame to be visible

	LTBOOL		m_bExpires;			// flag to set if the sprite will die (default = true)
	float		m_fLifeTime;		// Life expectancy of sprite

	LTBOOL		m_bShow;			// flag.  Is this sprite supposed to be drawn?
	LTBOOL		m_bAdditive;		// new bonus flag.  Is this an additive texture?
	LTPoly_GT4	m_DrawPrim;			// Drawprim used for rendering

	FrameArray	m_FrameArray;		// array of pointers to the textures used

	// Variables used to animate
	int			m_iCurrentFrame;
	float		m_fCurrentTime;
};

typedef std::vector<CScreenSprite *> ScreenSpriteArray;

// The management class.  Simply takes charge of updating timers and drawing the respective
// frames of all active sprites.  Manages the list of all sprites.

class CScreenSpriteMgr
{
friend class CScreenSprite;
public:
	CScreenSpriteMgr();
	~CScreenSpriteMgr();

	LTBOOL		Init();
	void		Term();

	void		Update();					// Draw all visible sprites

	CScreenSprite * CreateScreenSprite(char * pFile, bool bExpires = true, ScreenSpriteLayer eLayer = SPRITELAYER_DEFAULT);
	void		DestroyScreenSprite(CScreenSprite * pSprite);	// public fn for destructor

private:
	bool		GiveSpriteResources(CScreenSprite * pSprite, char * pFile);	// can only be called by CScreenSprites
	int			CacheTexture(char * pFile);
	LTBOOL		CacheSprite(CScreenSprite * pSprite, char * pFile);

	LTBOOL		m_bInitialized;

	FrameArray	m_FrameArray;
	ScreenSpriteArray m_SpriteArray;

	float		m_fLastTime;
	// TODO if a sprite has expired, go ahead and term() it and delete it!
};

#endif // __SCREEN_SPRITE_MGR_H__