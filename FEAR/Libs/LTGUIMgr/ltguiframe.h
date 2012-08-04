// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIFrame.h
//
// PURPOSE : Simple resizeable frame control
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_LTGUIFRAME_H_)
#define _LTGUIFRAME_H_

#include "LTGUICtrl.h"

class CLTGUIFrame : public CLTGUICtrl
{
public:
	CLTGUIFrame();
	virtual ~CLTGUIFrame();

	// Create the control
    bool	Create (HTEXTURE hFrame, const CLTGUICtrl_create& cs, bool bSimpleStretch = false);
    bool	Create (uint32 argbColor, const CLTGUICtrl_create& cs);

    virtual void    SetBasePos (const LTVector2n& pos);
	virtual void	SetScale(const LTVector2& vfScale);
	virtual void	SetSize(const LTVector2n& sz);
	
	void SetFrame(HTEXTURE hFrame);
	void SetColor(uint32 argbColor);

	// Sets the width of the frames's border, set to 0 to not show the frame
	void SetBorder(uint8 nBorderWidth, uint32 nBorderColor);


	// Render the control
	virtual void Render();
	virtual void RenderTransition(float fTrans);

	// no texturestrings to worry about...
	virtual void	FlushTextureStrings() {}
	virtual void	RecreateTextureStrings() {}


protected:
	void			SetRenderState();
	void			InitPolies();
	void			ScalePolies();

protected:

	LT_POLYGT4	m_Poly[9];
	TextureReference	m_hFrame;			// normal texture
	uint32		m_argbColor;
	float		m_fTextureScale;

	bool		m_bSimpleStretch;

	uint8		m_nBorderWidth;
	uint32		m_BorderColor;
	LT_POLYG4	m_Border[4];

};

#endif // _LTGUIFRAME_H_