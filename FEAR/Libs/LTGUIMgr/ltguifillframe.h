// ----------------------------------------------------------------------- //
//
// MODULE  : ltguifillframe.h
//
// PURPOSE : Declares the CLTGUIFillFrame control class.  This class
//           creates a simple frame control that contains an outline
//           border and is filled with a specified color.
//
// CREATED : 06/30/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUIFILLFRAME_H__
#define __LTGUIFILLFRAME_H__

#include "LTGUICtrl.h"

struct CLTGUIFillFrame_create : public CLTGUICtrl_create
{
	CLTGUIFillFrame_create();
	uint32				nBackgroundColor;
	uint32				nSelectedColor;
	uint32				nNonSelectedColor;
};

inline CLTGUIFillFrame_create::CLTGUIFillFrame_create() :
	nBackgroundColor(0),
	nSelectedColor(0),
	nNonSelectedColor(0)
{
}

class CLTGUIFillFrame : public CLTGUICtrl
{
public:
	enum eBorder
	{
		eBorder_Left	= 0x00000001,
		eBorder_Top		= 0x00000002,
		eBorder_Right	= 0x00000004,
		eBorder_Bottom	= 0x00000008
	};

public:
	CLTGUIFillFrame();
	virtual ~CLTGUIFillFrame();

	// Create the control
	bool					Create( const CLTGUIFillFrame_create& cs );
	void					Destroy();

	void					RecalcLayout();

	// Render the control
	virtual void			Render();
	virtual void			RenderTransition(float fTrans);

	virtual void			FlushTextureStrings(){}
	virtual void			RecreateTextureStrings(){}

	virtual void			SetBasePos( const LTVector2n& pos );
	virtual void			SetSize( const LTVector2n& sz );
	virtual void			SetScale(const LTVector2& vfScale);

	void					SetFrameWidth( uint8 nFrameWidth );
	void					SetRenderBorder( uint32 nBorderFlags );

protected:
	// frame data
	uint8					m_nFrameWidth;
	LT_POLYG4				m_Frame[8];

	uint32					m_nBackgroundColor;
	uint32					m_nSelectedColor;
	uint32					m_nNonSelectedColor;

	uint32					m_nBorderFlags;
};


#endif  // __LTGUIFILLFRAME_H__
