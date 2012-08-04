// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIString.h
//
// PURPOSE : Class to encapsulate a wstring and a HTEXTURESTRING
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUISTRING_H_)
#define _LTGUISTRING_H_

#include "ilttexturestring.h"

enum eTextAlign
{
	kLeft,
	kCenter,
	kRight
};

class CLTGUIString
{
public:
	CLTGUIString();
	~CLTGUIString() {FlushTexture();}

	//set font or source texture for string, but does not create texture
	// - if texture had been created, this will recreate it
	//setting the font will clear any source string
	void SetFont(const CFontInfo& Font);
	//setting the source string will clear the font info
	void SetSourceString(HTEXTURESTRING hSrcString);


	//set text for string, but does not create texture
	// - if texture had been created, this will recreate it
	void SetText(const wchar_t *pText, bool bEllipsis = false);


	const wchar_t* GetText() const {return m_sString.c_str(); }


	//does nothing if source string is used
	void SetFontHeight(uint32 nFontHeight);
	//returns 0 if source string is used
	uint32 GetFontHeight() const {return m_Font.m_nHeight; }

	void SetPos(LTVector2 vPos);
	void SetPos(LTVector2n vPos);

	void SetColor(uint32 nColor);
	void SetAlignment(eTextAlign align);
	void SetDropShadow(uint8 drop) {m_nDropShadow = drop;}
	void SetGlowParams(bool bEnable, float fAlpha = 0.25f, const LTVector2& vSize = LTVector2(1.2f,1.2f));
	void SetGlow(bool bGlow) {m_bGlow = bGlow;}

	LTVector2 GetPos()	const  { return m_vPos;}
	uint32 GetColor() const {return m_nColor;}
	eTextAlign GetAlignment() const {return m_Alignment;}




	// this will set the string's word wrap width, pass in 0 to disable word wrap
	//	(defaults to off)
	void WordWrap(uint32 nWidth);


	void CreateTexture();
	void FlushTexture();	

	bool IsValid() const;
	bool IsEmpty() const {return m_sString.empty(); }

	//render functions will recreate textures as needed
	LTRESULT	Render();
	LTRESULT	RenderClipped(const LTRect2n& rClipRect);

	//these functions render with a transition effect...
	LTRESULT	RenderTransition(float fTrans);
	LTRESULT	RenderTransitionClipped(const LTRect2n& rClipRect,float fTrans);

	//functions to get info about the texture
	LTRESULT	GetExtents(LTRect2n& rExtents) const;

	//this will determine the rectangle a particular character is being rendered to in screen space
	LTRESULT	GetCharRect(uint32 nCharIndex, LTRect2n& rRect) const;

private:
	LTRESULT	RenderString(	const LTVector2& vAnchor,
								uint32 nColor	= 0xFFFFFFFF,
								const LTVector2& vAnchorScale	= LTVector2(0.0f, 0.0f), 
								const LTVector2& vGround		= LTVector2(1.0f, 0.0f),
								const LTVector2& vDown			= LTVector2(0.0f, 1.0f),
								const LTVector2& vStretch		= LTVector2(1.0f, 1.0f));
	LTRESULT	RenderStringClipped(	const LTRect2n& rClipRect,
										const LTVector2& vAnchor, 
										uint32 nColor					= 0xFFFFFFFF,
										const LTVector2& vAnchorScale	= LTVector2(0.0f, 0.0f));

	void		RecalcEllipsis( const LTRect2n& rClipRect );

	CFontInfo		m_Font;
	HTEXTURESTRING	m_hString;
	HTEXTURESTRING	m_hSourceString;
	std::wstring	m_sString;
	uint32			m_nWidth;
	eTextAlign		m_Alignment;
	uint8			m_nDropShadow;
	uint32			m_nColor;
	uint32			m_nShadowColor;
	LTVector2		m_vPos;
	bool			m_bEllipsis;

	bool			m_bGlow;
	bool			m_bGlowEnabled;
	float			m_fGlowAlpha;
	LTVector2		m_vGlowSize;

	// last clip rect, if this changed then we need to recalculate the ellipsis
	LTRect2n		m_rnLastClip;

	//random factor applied to transitions
	float			m_fTransMod;


};


typedef std::vector<CLTGUIString*, LTAllocator<CLTGUIString*, LT_MEM_TYPE_CLIENTSHELL> > LTGUIStringArray;

#endif //_LTGUISTRING_H_