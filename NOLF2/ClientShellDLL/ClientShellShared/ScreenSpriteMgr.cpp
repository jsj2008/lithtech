// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSpriteMgr.cpp
//
// PURPOSE : Manage all game-side handling of 2d (screenspace) sprites
//
// CREATED : 12/7/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "ScreenSpriteMgr.h"

CScreenSpriteMgr *g_pScreenSpriteMgr = LTNULL;

// ======================================================================= //
//
// Class: CScreenSprite
//
// Purpose: Game-side class encapsulating autonomous sprites
//
// ======================================================================= //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Constructor and Destructor
//
//	PURPOSE:	Set default values for the class, clean up any allocations
//
// ----------------------------------------------------------------------- //

CScreenSprite::CScreenSprite()
{
	m_Center.x = 0;	// default is sprite coord is top-left
	m_Center.y = 0;
	m_pName = LTNULL;
	m_nFrames = 0;
	m_nFrameRate = 1;
	m_fOneFrameTime = 1.0f;
	m_bShow = LTFALSE;
	m_bAdditive = LTFALSE;

	// Set up the drawprim in the Init() function
	// Set up the FrameArray in the Init() function

	m_iCurrentFrame = 0;
	m_fCurrentTime = 0.0f;

	// by default, a sprite lasts FOREVER
	m_bExpires = LTTRUE;
	m_fLifeTime = 1.0f;

	m_eLayer = SPRITELAYER_DEFAULT;
}

CScreenSprite::~CScreenSprite()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::Init
//
//	PURPOSE:	Called by the ScreenSpriteMgr to set up a few variables
//
// ----------------------------------------------------------------------- //

void CScreenSprite::Init()
{
	m_FrameArray.clear();

	SetupQuadUVs(m_DrawPrim, NULL, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_DrawPrim,argbWhite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::SetExpiration
//
//	PURPOSE:	Called by the ScreenSpriteMgr to set up sprite life duration
//
// ----------------------------------------------------------------------- //

void CScreenSprite::SetExpiration(bool bExpires)
{
	m_bExpires = bExpires;
	if (m_bExpires)
	{
		m_fLifeTime = m_fOneFrameTime * (float)m_nFrames;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::Init
//
//	PURPOSE:	Have the ScreenSpriteMgr add it to its array of items to manage
//				Also caches all assets needed
//
// ----------------------------------------------------------------------- //
/*
LTBOOL CScreenSprite::Init(char *pName, LTBOOL bExpires)
{
	_ASSERT(g_pScreenSpriteMgr != LTNULL);
	_ASSERT(g_pDrawPrim != LTNULL);

	m_FrameArray.clear();

	g_pDrawPrim->SetUVWH(&m_DrawPrim, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_DrawPrim,argbWhite);

	m_bExpires = bExpires;

	LTBOOL bResult = g_pScreenSpriteMgr->AddSprite(this, pName);

	if (m_bExpires)
	{
		m_fLifeTime = m_fOneFrameTime * (float)m_nFrames;
	}
	return (bResult);
}
*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::Term
//
//	PURPOSE:	Clean up any local allocations
//
// ----------------------------------------------------------------------- //
void CScreenSprite::Term()
{
	if (m_pName)
	{
		free(m_pName);
		m_pName = LTNULL;
	}
	m_FrameArray.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::Show
//
//	PURPOSE:	Determines the visibility of the sprite.  If bRestartAnimation
//				is true, then also restart the animation from the first frame
//				instead of wherever it was last at.
//
// ----------------------------------------------------------------------- //

void CScreenSprite::Show(LTBOOL bShow, LTBOOL bRestartAnimation)
{
	m_bShow = bShow;
	if (m_bShow && bRestartAnimation)
	{
		m_iCurrentFrame = 0;
		m_fCurrentTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::AdvanceTime
//
//	PURPOSE:	Used by the ScreenSpriteMgr to tell this sprite how much
//				time has elapsed since it was last drawn in order to
//				compute if the frame number should change.
//
// ----------------------------------------------------------------------- //
void CScreenSprite::AdvanceTime(float fDelta)
{
	if (m_nFrames <= 1)
		return;

	if (m_bExpires)
	{
		m_fLifeTime -= fDelta;
	}

	m_fCurrentTime += fDelta;

	// Is it time to change frames?
	if (m_fCurrentTime > m_fOneFrameTime)
	{
		// See how many frames to advance (typically one)
		int nAdvanceFrames = (int)(m_fCurrentTime / m_fOneFrameTime);
		// reduce the frame clock by that amount
		m_fCurrentTime -= (float)nAdvanceFrames * m_fOneFrameTime;
		// Advance the frame counter (and modulus it!)
		m_iCurrentFrame = (m_iCurrentFrame + nAdvanceFrames) % m_nFrames;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::Render
//
//	PURPOSE:	Used by the ScreenSpriteMgr.  Draws this sprite to screen
//
// ----------------------------------------------------------------------- //

void CScreenSprite::Render()
{
	g_pDrawPrim->SetAlphaBlendMode(m_bAdditive ? DRAWPRIM_BLEND_ADD : DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(m_FrameArray[m_iCurrentFrame]->hTex);

	float x = (float)m_Position.x - (float)m_Center.x;
	float y = (float)m_Position.y - (float)m_Center.y;
	float w = (float)m_FrameArray[m_iCurrentFrame]->iWidth;
	float h = (float)m_FrameArray[m_iCurrentFrame]->iHeight;
	g_pDrawPrim->SetXYWH(&m_DrawPrim, x, y, w, h);
	SetupQuadUVs(m_DrawPrim, m_FrameArray[m_iCurrentFrame]->hTex, 0.0f, 0.0f, 1.0f, 1.0f);

	g_pDrawPrim->DrawPrim(&m_DrawPrim);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::SetCenter
//
//	PURPOSE:	Change the "center" of the sprite i.e. drawing offset
//
// ----------------------------------------------------------------------- //
void CScreenSprite::SetCenter(int x, int y)
{
	m_Center.x = x;
	m_Center.y = y;
}

void CScreenSprite::SetCenter(LTIntPt pos)
{
	m_Center.x = pos.x;
	m_Center.y = pos.y;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::SetPosition
//
//	PURPOSE:	Position the sprite in screen coordinates
//				Note that all coordinates are absolute and not scaled for
//				differing screen resolutions, so any repositioning needs
//				to be done by the calling routine.
//
// ----------------------------------------------------------------------- //
void CScreenSprite::SetPosition(int x, int y)
{
	m_Position.x = x;
	m_Position.y = y;
}

void CScreenSprite::SetPosition(LTIntPt pos)
{
	m_Position.x = pos.x;
	m_Position.y = pos.y;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::GetPosition
//
//	PURPOSE:	Query for the current location of the sprite
//
// ----------------------------------------------------------------------- //

LTIntPt CScreenSprite::GetPosition()
{
	LTIntPt pos;
	pos.x = m_Position.x;
	pos.y = m_Position.y;
	return (pos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::GetDims
//
//	PURPOSE:	Query for the current dimensions of the sprite.  Computed
//				by taking the dims of the first frame texture
//
// ----------------------------------------------------------------------- //

LTIntPt CScreenSprite::GetDims()
{
	_ASSERT(m_nFrames > 0);

	unsigned int iWidth = 1;
	unsigned int iHeight = 1;

	// Grab the dimensions of the first frame
	HTEXTURE hTex = m_FrameArray[0]->hTex;
	g_pTexInterface->GetTextureDims(hTex,iWidth,iHeight);

	LTIntPt dims(iWidth, iHeight);
	return dims;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSprite::SetLifeTime
//
//	PURPOSE:	Calculate and set up a sprite for self-termination
//
// ----------------------------------------------------------------------- //

void CScreenSprite::SetLifeTime(int iLoop)
{
	if (iLoop < 0)
	{
		m_bExpires = LTFALSE;
		return;
	}
	SetLifeTime((float)iLoop * m_fOneFrameTime * m_nFrames);
}

void CScreenSprite::SetLifeTime(float fSeconds)
{
	m_bExpires = LTTRUE;
	m_fLifeTime = fSeconds;
}

// ======================================================================= //
//
// Class: CScreenSpriteMgr
//
// Purpose: Black-box management of screen-based sprites
//
// ======================================================================= //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr constructor and destructor
//
//	PURPOSE:	Initialization and cleanup
//
// ----------------------------------------------------------------------- //

CScreenSpriteMgr::CScreenSpriteMgr()
{
	g_pScreenSpriteMgr = this;
	m_bInitialized = LTFALSE;
}

CScreenSpriteMgr::~CScreenSpriteMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::Init
//
//	PURPOSE:	Make sure the arrays are cleaned up, start the local timer
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSpriteMgr::Init()
{
	if (m_bInitialized)
		return LTTRUE;

	m_SpriteArray.clear();
	m_FrameArray.clear();

	m_fLastTime = g_pLTClient->GetTime();

	m_bInitialized = LTTRUE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::Term
//
//	PURPOSE:	Calls Term() on all attached sprites, and frees up all
//				dynamically allocated memory (textures and strings)
//
// ----------------------------------------------------------------------- //

void CScreenSpriteMgr::Term()
{
	if (!m_bInitialized)
		return;

	// clean up the sprite array
	ScreenSpriteArray::iterator sIter = m_SpriteArray.begin();
	while (sIter != m_SpriteArray.end())
	{
		delete (*sIter);
		sIter++;
	}
	m_SpriteArray.clear();

	// clean up the texture array
	FrameArray::iterator fIter = m_FrameArray.begin();
	while (fIter != m_FrameArray.end())
	{
		if ((*fIter)->pName)
			free((*fIter)->pName);

		if ((*fIter)->hTex)
			g_pTexInterface->ReleaseTextureHandle((*fIter)->hTex);

		delete (*fIter);
		fIter++;
	}
	m_FrameArray.clear();

	g_pScreenSpriteMgr = LTNULL;
	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::DestroyScreenSprite
//
//	PURPOSE:	Delete a sprite from the list of managed screensprites
//
// ----------------------------------------------------------------------- //

void CScreenSpriteMgr::DestroyScreenSprite(CScreenSprite * pSprite)
{
	if (pSprite == LTNULL)
		return;

	ScreenSpriteArray::iterator sIter = m_SpriteArray.begin();
	while (sIter != m_SpriteArray.end())
	{
		if (*sIter == pSprite)
		{
			// Remove a single sprite
			debug_delete(*sIter);
			m_SpriteArray.erase(sIter);
			return;
		}
		sIter++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::Update
//
//	PURPOSE:	Iterate through the array of sprites and render all that are visible
//
// ----------------------------------------------------------------------- //

void CScreenSpriteMgr::Update()
{
	if (!m_bInitialized)
		return;

	LTBOOL bKillingSprites = LTFALSE;
	LTBOOL bDraw = LTFALSE;

	// Note the elapsed time
	float fCurrentTime = g_pLTClient->GetTime();
	float fDeltaTime = fCurrentTime - m_fLastTime;
	m_fLastTime = fCurrentTime;

	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		if ((*iter)->m_bShow)
		{
			bDraw = LTTRUE;

			(*iter)->AdvanceTime(fDeltaTime);
			if ((*iter)->KillMe())
			{
				bKillingSprites = LTTRUE;
			}
		}
		iter++;
	}

	// Cull out any dead sprites
	if (bKillingSprites)
	{
		iter = m_SpriteArray.begin();
		while (iter != m_SpriteArray.end())
		{
			if ((*iter)->KillMe())
			{
				delete (*iter);
				m_SpriteArray.erase(iter);
//				iter = m_SpriteArray.begin();
			}
			else
			{
				iter++;
			}
		}
	}

	if(bDraw)
	{
		g_pLTClient->Start3D();
		g_pLTClient->StartOptimized2D();

		// Set up the rendering
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);

		iter = m_SpriteArray.begin();
		while (iter != m_SpriteArray.end())
		{
			if ((*iter)->m_bShow)
			{
				(*iter)->Render();
			}
			iter++;
		}

		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
	}
}

CScreenSprite * CScreenSpriteMgr::CreateScreenSprite(char * pFile, bool bExpires, ScreenSpriteLayer eLayer)
{
	// Make sure the ScreenSpriteMgr is operational
	if (!m_bInitialized)
		Init();

	CScreenSprite * pSpr = debug_new(CScreenSprite);
	pSpr->Init();

	bool bResult = GiveSpriteResources(pSpr, pFile);

	if (!bResult)
	{
		debug_delete(pSpr);
		return LTNULL;
	}

	pSpr->SetExpiration(bExpires);
	pSpr->m_eLayer = eLayer;		// determine the draw-order with the layer number

//	m_SpriteArray.push_back(pSpr);
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		if (pSpr->m_eLayer <= (*iter)->m_eLayer)
		{
			m_SpriteArray.insert(iter, pSpr);
			return pSpr;
		}
		iter++;
	}
	// pSpr didn't come before anything in the array, or the array is empty.  Add it
	m_SpriteArray.push_back(pSpr);	
	return pSpr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::AddSprite
//
//	PURPOSE:	Called by each sprite as it is initialized.  This routine
//				handles the parsing of the filename (sprite or dtx) and
//				adds the sprite to the internal array used for rendering
//
// ----------------------------------------------------------------------- //

bool CScreenSpriteMgr::GiveSpriteResources(CScreenSprite * pSprite, char * pFile)
{
/*
	if (!m_bInitialized)
		Init();

	// Do a quick check to see if this sprite already exists in the array
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		if (pSprite == *iter)
			return LTTRUE;
		iter++;
	}
*/
	// See if it's a texture (DTX)
	if (strnicmp(&pFile[strlen(pFile) - 4], ".dtx", 4) == 0)
	{
		int iFrameID = CacheTexture(pFile);
		if (iFrameID == -1)
			return (LTFALSE);

		pSprite->m_FrameArray.push_back(m_FrameArray[iFrameID]);
		pSprite->m_pName = strdup(pFile);

//		m_SpriteArray.push_back(pSprite);
		return LTTRUE;
	}

	// See if it's an SPR file
	if (strnicmp(&pFile[strlen(pFile) - 4], ".spr", 4) == 0)
	{
		// Cache the textures
		if (CacheSprite(pSprite, pFile))
		{
			// then add it to the array
//			m_SpriteArray.push_back(pSprite);
			return (LTTRUE);
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::CacheSprite
//
//	PURPOSE:	Called by AddSprite(), used to parse the SPR file and cache
//				all assets needed into the appropriate arrays.
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSpriteMgr::CacheSprite(CScreenSprite * pSprite, char * pFile)
{
	ILTStream * pStream;
	char buf[1024];

	pSprite->m_pName = strdup(pFile);

	g_pLTClient->OpenFile(pFile, &pStream);
	if (pStream)
	{
		int nFrames;
		int nFrameRate;
		int bTransparency;
		int bTranslucency;
		int iChromakey;

		pStream->Read(&nFrames, 4);
		pSprite->m_nFrames = nFrames;

		pStream->Read(&nFrameRate, 4);
		pSprite->m_nFrameRate = nFrameRate;
		pSprite->m_fOneFrameTime = 1.0f / (float)nFrameRate;

		pStream->Read(&bTransparency, 4);
		pStream->Read(&bTranslucency, 4);
		pStream->Read(&iChromakey, 4);

		if (nFrames < 1 && nFrames > 64)
		{
			// We have an error.
			// FIXME report it
			pStream->Release();
			return LTFALSE;
		}
		for (int i = 0; i < nFrames; i++)
		{
			short iLen;
			pStream->Read(&iLen, 2);
			if (iLen < 1 || iLen > 1023)
			{
				// We have an error.
				// FIXME report it
				pStream->Release();
				return LTFALSE;
			}
			memset(buf,0,1024);
			pStream->Read(&buf, iLen);

			int iFrameID = CacheTexture(buf);
			pSprite->m_FrameArray.push_back(m_FrameArray[iFrameID]);
		}
		pStream->Release();

		return LTTRUE;
	}
	return (LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSpriteMgr::CacheTextrue
//
//	PURPOSE:	Called by AddSprite() and CacheSprite.  Given a file name,
//				this routine adds a new texture to the local texture array
//				complete with supplemental data (dims and name)
//
// ----------------------------------------------------------------------- //

int CScreenSpriteMgr::CacheTexture(char * pFile)
{
	// First, check to see if we already have the texture
	// iterate through the array and return the index of the element that matches
	// this string.  Otherwise, return -1
	FrameArray::iterator iter = m_FrameArray.begin();

	while (iter != m_FrameArray.end())
	{
		char * pTextureName = (*iter)->pName;

		if (!strcmpi(pTextureName, pFile))
		{
			return (*iter)->iFrameID;
		}
		iter++;
	}

	HTEXTURE hTex = LTNULL;
	
	LTRESULT res = g_pTexInterface->FindTextureFromName(hTex, pFile);

	if (res != LT_OK)
	{
		g_pTexInterface->CreateTextureFromName(hTex, pFile);
	}

	if (hTex)
	{
		ScreenSpriteFrame * pFrame = new ScreenSpriteFrame;
		_ASSERT(pFrame != LTNULL);

		pFrame->iFrameID = m_FrameArray.size();
		pFrame->pName = strdup(pFile);
		pFrame->hTex = hTex;
		g_pTexInterface->GetTextureDims(hTex,pFrame->iWidth,pFrame->iHeight);

		m_FrameArray.push_back(pFrame);
		return (pFrame->iFrameID);
	}
	return -1;
}

