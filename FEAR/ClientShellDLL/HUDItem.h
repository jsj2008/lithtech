// ----------------------------------------------------------------------- //
//
// MODULE  : HUDItem.h
//
// PURPOSE : Definition of CHUDItem base class
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDITEM_H__
#define __HUDITEM_H__

#include "ltbasedefs.h"
#include "ltguistring.h"
#include "ltpoly.h"
#include "TextureReference.h"
#include "HUDEnums.h"

//******************************************************************************************

enum eHUDHorizAlign
{
	kHUDHNoAlign,
	kHUDHAlignLeft,
	kHUDHAlignCenter,
	kHUDHAlignRight,
};

enum eHUDVertAlign
{
	kHUDVNoAlign,
	kHUDVAlignTop,
	kHUDVAlignCenter,
	kHUDVAlignBottom
};

//******************************************************************************************

enum eHUDRenderLevel
{
	kHUDRenderNone = 0,		// always render
	kHUDRenderText,			// render when only text is allowed (i.e. cinematics, etc.)
	kHUDRenderDead,			// render when the player is dead
	kHUDRenderFull			// render only when full HUD is rendered
};

//******************************************************************************************
//** Base HUD item
//******************************************************************************************
class CHUDItem
{
public:
	CHUDItem();
	virtual ~CHUDItem() {Term();}
	

	virtual bool		Init()  {return true;}
	virtual void		Term();

	virtual void		OnExitWorld() {};

    virtual void        Render() = 0;
    virtual void        Update() = 0;
	virtual void		ScaleChanged();

	virtual void		SetBasePos( LTVector2n vBasePos ) { m_vBasePos = vBasePos; UpdateLayout( ); }
	LTVector2n const&	GetBasePos( ) const { return m_vBasePos; }

	void				SetUseBasePosFromLayout( bool bValue ) { m_bUseBasePosFromLayout = bValue; }
	bool				GetUseBasePosFromLayout( ) const { return m_bUseBasePosFromLayout; }

    virtual void        UpdateLayout();

	virtual uint32		GetUpdateFlags() { return m_UpdateFlags; }

	virtual	eHUDRenderLevel	GetRenderLevel() { return m_eLevel; }

	void				SetHUDRenderLayer( EHUDRenderLayer eHUDRenderLayer ) { m_eHUDRenderLayer = eHUDRenderLayer; }
	EHUDRenderLayer		GetHUDRenderLayer( ) const { return m_eHUDRenderLayer; }

	virtual void		SetRenderState();


	//called during update loop to update the fade
	virtual void        UpdateFade();
	virtual void		EnableFade(bool bEnable) {m_bFadeEnabled = bEnable;}

	virtual void        StartFlicker();
	virtual void        UpdateFlicker();
	virtual void        EndFlicker();

	virtual float		GetFadeSpeed() const;

	virtual void		UpdateFlash();
	virtual bool		IsFlashing() const;
	virtual void		EndFlash();


	//0 = completely faded, 1 = completely visible
	virtual float		GetFadeLevel() {return m_fCurrentFade;}
	virtual bool		FadeLevelChanged() {return m_bFadeChanged;}

	//called to restart the fading process from the beginning
	virtual void		ResetFade();

	// helper functions to layout hud items
	LTVector2n AlignBasePosition( LTVector2n vHudItemSize );

	// convenience function to load texture data from database and set up defaults
	void InitAdditionalTextureData( HRECORD hLayout, uint32 nValueIndex, TextureReference& hTexture, LTVector2n& vPos, LTVector2n& vSize, LTPoly_GT4& poly );

	virtual void Flash(const char* pszFlash);

	virtual void SetSourceString(const wchar_t* pszChars);

	virtual	void Reset();

	HRECORD GetLayoutRecord( ) const { return m_hLayout; }


protected:
	uint32				m_UpdateFlags;
	eHUDRenderLevel		m_eLevel;

	// Render layer.
	EHUDRenderLayer		m_eHUDRenderLayer;

	//common visual elements
	CLTGUIString	m_Text;
	LTPoly_GT4		m_IconPoly;
	HTEXTURESTRING	m_hSourceString;

	//fade related members
	bool				m_bFadeEnabled;
	bool				m_bSinglePlayerFade;
	bool				m_bMultiplayerFade;
	bool				m_bFadeChanged;
	float				m_fCurrentFade;
	float				m_fLastFade;
	double				m_fFadeStartTime;

	//common layout data
	HRECORD				m_hLayout;
	LTVector2n			m_vBasePos;
	LTVector2n			m_vTextOffset;
	CFontInfo			m_sTextFont;
	uint32				m_cTextColor;
	eTextAlign			m_eTextAlignment;
	LTVector2n			m_vIconOffset;
	LTVector2n			m_vIconSize;
	uint32				m_cIconColor;
	TextureReference	m_hIconTexture;
	float				m_fHoldTime;
	float				m_fFadeTime;

	float				m_fFlicker;
	float				m_fFlickerFreq;
	
	bool				m_bUseBasePosFromLayout;
	eHUDHorizAlign		m_eHorizAlign;
	eHUDVertAlign		m_eVertAlign;


	uint32			m_cFlashColor;
	uint32			m_nFlashCount;
	StopWatchTimer	m_FlashTimer;

};


#endif//__HUDITEM_H__

