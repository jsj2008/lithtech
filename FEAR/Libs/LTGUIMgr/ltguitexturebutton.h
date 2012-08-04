// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextureButton.h
//
// PURPOSE : button control with three states (normal, selected, and disabled)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUITEXTUREBUTTON_H_)
#define _LTGUITEXTUREBUTTON_H_

#include "LTGUICtrl.h"

struct CLTGUITextureButton_create : public CLTGUICtrl_create
{
	CLTGUITextureButton_create();
	TextureReference hNormal;
	TextureReference hSelected;
	TextureReference hDisabled;
	LTRect2n rnImageRect;
	LTRect2n rnTextRect;
	bool bCenterImage;
	bool bCenterText;
};

inline CLTGUITextureButton_create::CLTGUITextureButton_create()
{
	bCenterImage = false;
	bCenterText = true;
};


// This control displays a textured button
class CLTGUITextureButton : public CLTGUICtrl
{
public:
	CLTGUITextureButton();
	virtual ~CLTGUITextureButton();

	// Create the control
    bool           Create ( const CLTGUITextureButton_create& cs);

	virtual	void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale );
	virtual void	SetSize( const LTVector2n& sz );
	
	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	// Commonly used keyboard messages
    virtual bool   OnEnter ( );
    virtual bool   OnLButtonUp(int x, int y)			
	{ 
		LTUNREFERENCED_PARAMETER(x); LTUNREFERENCED_PARAMETER(y);
		return OnEnter(); 
	}

	// Set the font, note this always expects an unscaled height
	virtual bool	SetFont ( const CFontInfo& Font );
	virtual bool	SetFontHeight (uint32 nFontHeight);
	virtual void	SetAlignment(eTextAlign align);

	//this fails if no font has been set
	virtual bool	SetText(const wchar_t *pText, bool bHighlightText);

	void	SetTexture(HTEXTURE hNormal, HTEXTURE hSelected = NULL, HTEXTURE hDisabled = NULL);

	virtual void	SetCommandHandler(CLTGUICommandHandler *pCommandHandler)
									{ m_pCommandHandler = pCommandHandler; }

	// free texture memory by flushing any texture strings owned by the control
	virtual void	FlushTextureStrings();

	// rebuild any texture strings owned by the control
	virtual void	RecreateTextureStrings();


protected:
	void			SetRenderState();
	void			InitPoly();
	void			ScalePoly();

protected:

	LT_POLYGT4	m_Poly;
	TextureReference	m_hNormal;			// normal texture
	TextureReference	m_hSelected;		// highlighted texture
	TextureReference	m_hDisabled;		// disabled texture

	CLTGUIString	m_Text;
	uint32			m_nBaseFontSize;		// The font size before scaling
	bool			m_bHighlightText;
	eTextAlign		m_eAlignment;

	LTRect2n		m_rnImageRect;
	LTRect2n		m_rnTextRect;

	bool			m_bCenterImage;
	bool			m_bCenterText;


};

#endif // _LTGUITEXTUREBUTTON_H_