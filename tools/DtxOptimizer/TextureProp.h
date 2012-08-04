//------------------------------------------------------------------
//
//  FILE      : TextureProp.h
//
//  PURPOSE   :	Texture properties structure
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __TEXTUREPROP_H__
#define __TEXTUREPROP_H__

#pragma once

#include "bdefs.h"
#include "dtxmgr.h"


#define TPROP_FLAGS					(1<<0)
#define TPROP_GROUP					(1<<2)
#define TPROP_NUMMIPMAPS			(1<<3)
#define TPROP_FULLBRITES			(1<<4)
#define TPROP_PREFER4444			(1<<5)
#define TPROP_PREFER16BIT			(1<<6)
#define TPROP_COMMANDSTRING			(1<<7)
#define TPROP_DATAFORMAT			(1<<8)
#define TPROP_NONS3TCMIPMAPOFFSET	(1<<9)
#define TPROP_UIMIPMAPOFFSET		(1<<10)
#define TPROP_TEXTUREPRIORITY		(1<<11)
#define TPROP_DETAILTEXTURESCALE	(1<<12)
#define TPROP_DETAILTEXTUREANGLE	(1<<13)
#define TPROP_PREFER5551			(1<<14)
#define TPROP_32BITSYSCOPY			(1<<15)
#define TPROP_NOSYSCACHE			(1<<16)


class TextureProp
{
public:

	TextureProp()
		: m_pTexture(NULL),
		  m_bFullBrights(FALSE),
		  m_b32BitSysCopy(FALSE),
		  m_bPrefer4444(FALSE),
		  m_bPrefer5551(FALSE),
		  m_bPrefer16Bit(FALSE),
		  m_bNoSysCache(FALSE),
		  m_TextureFlags(0),
		  m_TextureGroup(0),
		  m_nMipmaps(0),
		  m_NonS3TCMipmapOffset(0),
		  m_AlphaCutoff(0),
		  m_AverageAlpha(0),
		  m_UIMipmapOffset(0),
		  m_TexturePriority(0),
		  m_DetailTextureScale(0.0f),
		  m_DetailTextureAngle(0),
		  m_Initted(false)
	{
	}

	~TextureProp();

	// initialize
	bool		Init(const char *pFilename);
	bool		Save(const char *pFilename);

	bool		Optimize();
	bool		OptimizeFor2D();

	// terminate
	void		Term();

public:

	// Texture to use in the preview window.
	// If this is NULL, there won't be a preview window.
	// This texture is deleted automatically when the window is closed.
	TextureData*	m_pTexture;

	BOOL			m_bFullBrights;
	BOOL			m_b32BitSysCopy;
	BOOL			m_bPrefer4444;
	BOOL			m_bPrefer5551;
	BOOL			m_bPrefer16Bit;
	BOOL			m_bNoSysCache;
	DWORD			m_TextureFlags;
	DWORD			m_TextureGroup;
	DWORD			m_nMipmaps;
	DWORD			m_NonS3TCMipmapOffset;
	int				m_AlphaCutoff;
	int				m_AverageAlpha;
	int				m_UIMipmapOffset;
	char			m_CommandString[DTX_COMMANDSTRING_LEN];
	BPPIdent		m_BPPIdent;
	DWORD			m_TexturePriority;
	float			m_DetailTextureScale;
	int16			m_DetailTextureAngle;

	bool			m_Initted;
};



#endif // __TEXTUREPROP_H__
