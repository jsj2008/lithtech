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

enum MsgJustification
{
	kMsgLeft,
	kMsgCenter,
	kMsgRight,
};

typedef struct MsgCreate_t
{
	MsgCreate_t() 
	{
		pFont = LTNULL;
		nFontSize = 0;
		nTextColor = 0;
		hImage = LTNULL;
		nImageSize = 0;
		eJustify = kMsgLeft;
		fDuration = 0.0f;
		fFadeDur  = 0.0f;
		nWidth	  = 640;
		bDropShadow = true;

	};

	MsgCreate_t &operator=(const MsgCreate_t &cOther) {
		sString = cOther.sString;
		pFont = cOther.pFont;
		nFontSize = cOther.nFontSize;
		nTextColor = cOther.nTextColor;
		hImage = cOther.hImage;
		nImageSize = cOther.nImageSize;
		eJustify = cOther.eJustify;
		fDuration = cOther.fDuration;
		fFadeDur  = cOther.fFadeDur;
		nWidth	  = cOther.nWidth;
		bDropShadow = cOther.bDropShadow;
		return *this;
	}


	std::string			sString;
	CUIFont*			pFont;
	uint8				nFontSize;
	uint32				nTextColor;
	HTEXTURE			hImage;
	uint8				nImageSize;
	MsgJustification	eJustify;
	float 				fDuration;
	float 				fFadeDur;
	uint16				nWidth;
	bool				bDropShadow;

} MsgCreate;



class CHUDMessage : public CLTGUICtrl
{
public:
	CHUDMessage();
	virtual ~CHUDMessage() {Destroy();}

	LTBOOL	Create(MsgCreate &mc);
	virtual void	Destroy();

    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);

	// Render the control
	virtual void	Render (LTBOOL bForceVisible);
	virtual void	Render ();

	void	SetAlpha(float  fAlpha);

	virtual void Update();

	virtual uint16	GetHeight() {return m_nHeight;}
	virtual uint16	GetWidth() {return m_nWidth;}

protected:
	void	SetRenderState();
	void	InitPoly();
	void	ScalePoly();
	void	JustifyPoly();
	float	GetLifetime();

protected:
	LTPoly_GT4	m_Poly;
	HTEXTURE	m_hImage;

    uint16		m_nFixedWidth;         // The width of the control
    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The width of the control

    LTIntPt		m_imageSize;
	uint8		m_nBaseImageSize;

	CUIFormattedPolyString* m_pText;

	CUIFont*	m_pFont;				// The font for this control
	uint8		m_nFontSize;			// The scaled font size to use.
	uint8		m_nBaseFontSize;		// The font size before scaling
	uint32		m_nTextColor;

	float 		m_fInitTime;
	float 		m_fDuration;
	float 		m_fFadeDur;

	float 		m_fImageGap;

	float		m_fAlpha;

	bool		m_bDropShadow;

	MsgJustification	m_eJustify;
};


#endif