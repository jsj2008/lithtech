// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterDisplay.h
//
// PURPOSE : Class to manage custom displays on characters
//
// CREATED : 11/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTERDISPLAY_H__
#define __CHARACTERDISPLAY_H__

#include "LTPoly.h"

class CCharacterFX;
class ClientDisplayData;

// ----------------------------------------------------------------------- //
//
//	CLASS:		CharacterDisplayDB
//
//	PURPOSE:	Database for accessing character display db info.
//
// ----------------------------------------------------------------------- //
BEGIN_DATABASE_CATEGORY( CharacterDisplayDB, "Interface/CharacterDisplay" )
	DEFINE_GETRECORDATTRIB( Material, char const* );
	DEFINE_GETRECORDATTRIB( MaterialIndex, int32 );
	DEFINE_GETRECORDATTRIB( MaterialParameter, char const* );
	DEFINE_GETRECORDATTRIB( TextColor, int32 );
	DEFINE_GETRECORDATTRIB( TextBackRect, LTVector4 );
	DEFINE_GETRECORDATTRIB( TextBackColor, int32 );
	DEFINE_GETRECORDATTRIB( Layout, char const* );
END_DATABASE_CATEGORY( );

// ----------------------------------------------------------------------- //
//
//	CLASS:		CharacterDisplayLayoutDB
//
//	PURPOSE:	Database for accessing character display db info.
//
// ----------------------------------------------------------------------- //
BEGIN_DATABASE_CATEGORY( CharacterDisplayLayoutDB, "Interface/CharacterDisplayLayout" )
	DEFINE_GETRECORDATTRIB( Font, HRECORD );
	DEFINE_GETRECORDATTRIB( TextOffset, LTVector2 );
	DEFINE_GETRECORDATTRIB( TextSize, int32 );
	DEFINE_GETRECORDATTRIB( TextAlignment, char const* );
END_DATABASE_CATEGORY( );

// ----------------------------------------------------------------------- //
//
//	CLASS:		CharacterDisplay
//
//	PURPOSE:	Character display class.
//
// ----------------------------------------------------------------------- //
class CharacterDisplay
{
public:
	CharacterDisplay();
	virtual ~CharacterDisplay() {	Term( ); }

	//set up the display
	virtual bool Init(CCharacterFX* pParent, HRECORD hDisplay, const LTVector2n& sz, ClientDisplayData* pData );

	//clean up
	virtual void Term( );

	//updates the render target
	virtual void UpdateDisplay(bool bForceClear);

	//does the actual render
	virtual void Render();

	//set the display material on the character
	virtual bool SetMaterial();

	virtual void SetText(const wchar_t* szTxt);
	virtual void SetTexture(const char* szTex) { if (szTex) m_hTexture.Load(szTex); }
protected:
	//this handles creation of the render target
	virtual bool SetRenderTarget(HRENDERTARGET hRenderTarget);

	//this will release the associated render target
	virtual bool ReleaseRenderTarget();

	// set up info
	CCharacterFX*	m_pParent;

	// Record in main database for characterdisplay.
	HRECORD			m_hDisplayRecord;

	// Record in localized database for layout info on characterdisplay.
	HRECORD			m_hLayoutRecord;

	//our render target that we will be displaying
	HRENDERTARGET	m_hRenderTarget;
	HMATERIAL		m_hMaterial;
	bool			m_bRenderTargetBound;

	// draw prim elements to render ammo display
	CLTGUIString	m_Text;
	uint32			m_nTextSize[3];
	uint32			m_cTextBackColor;
	LTRect2n		m_rTextBackRect;
	TextureReference m_hTexture;
	LTVector2		m_vSize;


};

class CharacterDisplaySimple
{
public:
	CharacterDisplaySimple();
	virtual ~CharacterDisplaySimple() {	Term( ); }

	//set up the display
	virtual bool Init(CCharacterFX* pParent, HRECORD hDisplay, ClientDisplayData* pData);

	//clean up
	virtual void Term( );

	//set the display material on the character
	virtual bool SetMaterial();

	virtual void SetTexture(const char* szTex);
protected:

	// set up info
	CCharacterFX*	m_pParent;
	HRECORD			m_hDisplayRecord;

	//our render target that we will be displaying
	HMATERIAL		m_hMaterial;

	TextureReference m_hTexture;
	LTVector2		m_vSize;


};




#endif  // __CHARACTERDISPLAY_H__
