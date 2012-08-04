//-------------------------------------------------------------------------
//
// MODULE  : HUDMessage.h
//
// PURPOSE : Base class for HUD text display
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __HUD_MESSAGE_H__
#define __HUD_MESSAGE_H__


#include "LTGUIMgr.h"
#include "LTPoly.h"

typedef struct MsgCreate_t
{
	MsgCreate_t() 
	{
		nTextColor = 0;
		nHeaderColor = 0;
		hImage = NULL;
		ptImageSize = LTVector2n(0,0);
		eJustify = kLeft;
		fDuration = 0.0f;
		fFadeDur  = 0.0f;
		nWidth	  = 480; 
		bDropShadow = true;

	};

	std::wstring		sString;
	CFontInfo			Font;
	uint32				nTextColor;
	TextureReference	hImage;
	LTVector2n			ptImageSize;
	eTextAlign			eJustify;
	float 				fDuration;
	float 				fFadeDur;
	uint32				nWidth;
	bool				bDropShadow;

	std::wstring		sHeaderString;
	uint32				nHeaderColor;


} MsgCreate;



class CHUDMessage : public CLTGUICtrl
{
public:
	CHUDMessage();
	virtual ~CHUDMessage() {Destroy();}

	bool	Create(MsgCreate &mc);
	virtual void	Destroy();

	virtual void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale);

	// Render the control
	virtual void	Render (bool bForceVisible);
	virtual void	Render ();
	virtual void	RenderTransition(float fTrans) {Render();}

	void	SetAlpha(float  fAlpha);

	virtual void Update();

	virtual void FlushTextureStrings();
	virtual void RecreateTextureStrings();


	const wchar_t* GetHeader() { return m_Header.GetText(); }
	const wchar_t* GetText() { return m_Text.GetText(); }


protected:
	void	SetRenderState();
	void	InitPoly();
	void	ScalePoly();
	void	JustifyPoly();
	double	GetLifetime();

protected:
	LTPoly_GT4			m_Poly;
	TextureReference	m_hImage;
    LTVector2n		m_imageSize;
	LTVector2n		m_baseImageSize;

	CLTGUIString	m_Text;
	uint32			m_nFixedWidth;         // The width of the control
	uint32			m_nBaseFontSize;		// The font size before scaling
	uint32			m_nTextColor;
	eTextAlign		m_eJustify;

	CLTGUIString	m_Header;
	uint32			m_nHeaderColor;

	double 		m_fInitTime;
	float 		m_fDuration;
	float 		m_fFadeDur;

	float 		m_fImageGap;

	float		m_fAlpha;

};


#endif