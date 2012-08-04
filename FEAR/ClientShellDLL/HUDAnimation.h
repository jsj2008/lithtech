// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAnimation.h
//
// PURPOSE : Definition of an animating HUD component
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ANIM_H
#define __HUD_ANIM_H

#include "CommonUtilities.h"

struct CHUDAnimationData
{
	CHUDAnimationData();

	LTRect2n	rnBaseRect;
	char const* pszMaterialName;
	bool		bCache;
};


class CHUDAnimation
{
public:
	CHUDAnimation();
	virtual ~CHUDAnimation() { Unload(); }
	

	bool Init();
	
	void Show(bool bShow);
	bool IsInitted() {return m_bInitted;}
	bool IsVisible() {return (m_bInitted && m_bVisible);}

	void Render();
	void SetScale(const	LTVector2& vfScale);

	void Unload();

	CHUDAnimationData& GetAnimationData( ) { return m_AnimationData; }

	HMATERIAL GetMaterialInstance( ) const { return m_hMaterial; }

	uint32			GetColor( ) const { return m_nColor; }
	void			SetColor( uint32 nColor );

private:
	bool			m_bInitted;
	bool			m_bVisible;

	LTPoly_GTTS4	m_Poly;
	HMATERIAL		m_hMaterial;
	LTVector2		m_vfScale;
	uint32			m_nColor;

	CHUDAnimationData	m_AnimationData;
};

#endif