// ****************************************************************************************** //
//
// MODULE  : HUDHealth2.h
//
// PURPOSE : Definition of HUD health display
//
// CREATED : 07/29/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#ifndef __HUDHEALTH2_H__
#define __HUDHEALTH2_H__

#include "HUDItem.h"

// ****************************************************************************************** //

class CHUDHealth2 : public CHUDItem
{
public:

	// ------------------------------------------------------------------------------------------ //
	// Construction / destruction

	CHUDHealth2();


	// ------------------------------------------------------------------------------------------ //
	// Initialization and termination

	bool		Init();
	void		Term();


	// ------------------------------------------------------------------------------------------ //
	// General updating

	void        Render();
	void        Update();
	void		UpdateLayout();
	void		ScaleChanged();

private:

	// Allocated texture handles
	TextureReference	m_hBGTexture;
	TextureReference	m_hBarTexture;
	TextureReference	m_hCapTexture;

	// Relative texture positions
	LTVector2n			m_vBGBasePos;
	LTVector2n			m_vBarBasePos;
	LTVector2n			m_vCapBasePos;

	// Texture sizes
	LTVector2n			m_vBGSize;
	LTVector2n			m_vBarSize;
	LTVector2n			m_vCapSize;

	// Render locations and uvs
	LTPoly_GT4			m_BGPoly;
	LTPoly_GT4			m_BarPoly;
	LTPoly_GT4			m_CapPoly;

	// Base position
	LTVector2n			m_vScreenAdjustedBasePos;

};

// ****************************************************************************************** //

#endif//__HUDHEALTH2_H__

