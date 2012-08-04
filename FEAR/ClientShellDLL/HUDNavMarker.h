// ----------------------------------------------------------------------- //
//
// MODULE  : HUDNavMarker.h
//
// PURPOSE : HUDItem to display a navigation marker
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_NAV_MARKER_H
#define __HUD_NAV_MARKER_H

#include "idatabasemgr.h"

class CCharacterFX;

class HUDNavMarker_create
{
public:
	HUDNavMarker_create() : m_bIsActive(false),m_hType(NULL),m_nTeamId(NULL),m_pText(NULL), m_pCharFx(NULL), m_pOverrideTex(NULL) {}
	virtual ~HUDNavMarker_create() {}
	bool		m_bIsActive;
	HRECORD		m_hType;
	LTObjRef	m_hTarget;
	uint8		m_nTeamId;
	const wchar_t* m_pText;
	const char*	m_pOverrideTex;
	
	// Filled in if from a character.
	CCharacterFX* m_pCharFx;

private:
	// Copy ctor and assignment operator not implemented and should never be used.
	HUDNavMarker_create( HUDNavMarker_create const& other );
	HUDNavMarker_create& operator=( HUDNavMarker_create const& other );
};

//******************************************************************************************
//** HUD Nav Marker
//******************************************************************************************
class CHUDNavMarker
{
public:
	CHUDNavMarker();

	void    UpdateData(const HUDNavMarker_create* pNMCS);
	void	ClearData();

	// this function will fail if the marker has a valid target object
	bool    SetPos(const LTVector& vPos);

	//forces an alpha value for use with HUD flicker effects
	void	SetAlpha(float fAlpha);


    void    Update();

	void    Render();
	void    RenderArrow();
	void	ScaleChanged();

	float	GetRange() const {return m_fRange;}
	bool	IsOnScreen() const { return m_bOnScreen; }

	bool	IsActive() const;
	uint8	GetPriority() const;
	float	GetFadeAngle() const {return m_fFadeAngle;	}

	void	SetType(HRECORD hType);

	void	Flash(const char* pszFlash);
	void	SetScaling(bool bScale) {m_bScale = bScale;}

protected:
	void	UpdateFlash();

private:
	HUDNavMarker_create m_NMCS;

	LTPoly_GT4			m_Icon;
	TextureReference	m_hIcon;			//  icon

	LTPoly_GT4			m_Arrow;
	TextureReference	m_hArrow;			//  directional indicator

	CLTGUIString	m_Range;
	CLTGUIString	m_Text;

	bool			m_bOnScreen;
	LTVector2n		m_vPos;
	float			m_fRange;
	bool			m_bUseRange;
	float			m_fFadeAngle;

	bool			m_bIsActive;
	HRECORD			m_hType;
	uint8			m_nTeamId;
	LTObjRef		m_hTarget;
	LTVector		m_vTargetPos;

	// Filled in if from a character.
	CCharacterFX*	m_pCharFx;

	LTVector		m_vWorldOffset;

	LTVector2		m_vIconSize;
	LTVector2		m_vIconOffset;
	LTVector2		m_vArrowSize;
	LTVector2		m_vRangeOffset;
	LTVector2		m_vTextOffset;

	uint32			m_cIconColor;
	uint32			m_cArrowColor;
	uint32			m_cTextColor;

	static LTVector2		s_vCenterPt;
	static float			s_fArrowRadius;

	uint32			m_cFlashColor;
	uint32			m_nFlashCount;
	StopWatchTimer	m_FlashTimer;

	float			m_fMinAlpha;
	float			m_fMinFade;
	float			m_fMaxFade;

	float			m_fFadeRange;
	float			m_fAlphaRange;

	bool			m_bScale;

};

#endif //__HUD_NAV_MARKER_H