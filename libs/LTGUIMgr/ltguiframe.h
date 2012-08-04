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


class CLTGUIFrame : public CLTGUICtrl
{
public:
	CLTGUIFrame();
	virtual ~CLTGUIFrame();

	// Create the control
    LTBOOL	Create (HTEXTURE hFrame, uint16 nWidth, uint16 nHeight, LTBOOL bSimpleStretch = LTFALSE);
    LTBOOL	Create (uint32 argbColor, uint16 nWidth, uint16 nHeight);



    virtual void    SetBasePos ( LTIntPt pos );
	virtual void	SetScale(float fScale);
	virtual void	SetTextureScale(float fScale);
	
	void SetFrame(HTEXTURE hFrame);
	void SetColor(uint32 argbColor);
	void SetSize(uint16 nWidth, uint16 nHeight);

	// Sets the width of the frames's border, set to 0 to not show the frame
	void SetBorder(uint8 nBorderWidth, uint32 nBorderColor);


	// Render the control
	virtual void	Render ();

	// Width/Height calculations
	virtual uint16	GetWidth ( )						{ return m_nWidth; }
	virtual uint16	GetHeight ( )						{ return m_nHeight; }


protected:
	void			SetRenderState();
	void			InitPolies();
	void			ScalePolies();

protected:

	LT_POLYGT4	m_Poly[9];
	HTEXTURE	m_hFrame;			// normal texture
	uint32		m_argbColor;
	LTFLOAT		m_fTextureScale;

	LTBOOL		m_bSimpleStretch;

    uint16		m_nWidth;              // The width of the control
    uint16		m_nHeight;             // The height of the control
    uint16		m_nBaseWidth;          // The unscaled width of the control
    uint16		m_nBaseHeight;         // The unscaled height of the control

	uint8		m_nBorderWidth;
	uint32		m_BorderColor;
	LT_POLYF4	m_Border[4];

};

#endif // _LTGUIFRAME_H_