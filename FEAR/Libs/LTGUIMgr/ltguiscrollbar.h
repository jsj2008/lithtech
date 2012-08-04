// ----------------------------------------------------------------------- //
//
// MODULE  : ltguiscrollbar.h
//
// PURPOSE : Declares the CLTGUIScrollBar class.  This class creates a
//           scrollbar control.
//
// CREATED : 06/20/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUISCROLLBAR_H__
#define __LTGUISCROLLBAR_H__

#include "LTGUICtrl.h"
#include "EngineTimer.h"

struct CLTGUIScrollBar_create : public CLTGUICtrl_create
{
	CLTGUIScrollBar_create();
	TextureReference	hBarTextureNormal;
	TextureReference	hBarTextureHot;
	uint32				nBackgroundColor;
	uint32				nSelectedColor;
};

inline CLTGUIScrollBar_create::CLTGUIScrollBar_create() :
	nBackgroundColor(0),
	nSelectedColor(0)
{
}

enum eScrollBarCmd
{
	eScrollBarCmd_Top = 0x100,
	eScrollBarCmd_Bottom,
	eScrollBarCmd_LineUp,
	eScrollBarCmd_LineDown,
	eScrollBarCmd_ThumbTrack,
	eScrollBarCmd_ThumbPosition,
	eScrollBarCmd_PageUp,
	eScrollBarCmd_PageDown,
};

class CLTGUIScrollBar : public CLTGUICtrl
{
public:
	enum eBarElement
	{
		eBarElement_Thumb,
		eBarElement_Track,
		eBarElement_ArrowUp,
		eBarElement_ArrowDown,
		eBarElement_ThumbTop,
		eBarElement_ThumbBottom,
		eBarElement_Grip,

		eBarElementCount
	};

	enum eCapture
	{
		eCapture_None,
		eCapture_UpArrow,
		eCapture_DownArrow,
		eCapture_Thumb,
		eCapture_PageUp,
		eCapture_PageDown,
	};

public:
	CLTGUIScrollBar();
	virtual ~CLTGUIScrollBar();

	// Create the control
	bool					Create( const CLTGUIScrollBar_create& cs );
	void					Destroy();

	void					Scroll( float fScrollAmount );

	void					SetMessageControl( CLTGUICtrl* pControl ) { m_pMessageControl = pControl; }
	CLTGUICtrl*				GetMessageControl() { return m_pMessageControl; }

	void					SetInputCaptureHandler( CLTGUICommandHandler* pHandler ) { m_pInputCaptureHandler = pHandler; }
	CLTGUICommandHandler*	GetInputCaptureHandler() { return m_pInputCaptureHandler; }

	// Render the control
	virtual void			Render();
	virtual void			RenderTransition(float fTrans);

	virtual void			FlushTextureStrings(){}
	virtual void			RecreateTextureStrings(){}

	virtual void			SetBasePos( const LTVector2n& pos );
	virtual void			SetSize( const LTVector2n& sz );
	virtual void			SetScale(const LTVector2& vfScale);

	// sets the scroll information
	void					SetScrollPage( uint32 nPage ) { m_nPage = nPage; RecalcLayout(); }
	uint32					GetScrollPage() { return m_nPage; }
	void					SetScrollMin( int32 nMin ) { m_nMin = nMin; RecalcLayout(); }
	int32					GetScrollMin() { return m_nMin; }
	void					SetScrollMax( int32 nMax ) { m_nMax = nMax; RecalcLayout(); }
	int32					GetScrollMax() { return m_nMax; }

	// scroll speeds
	float					GetScrollSpeed() { return m_fScrollSpeed; }
	void					SetScrollSpeed( float fSpeed ) { m_fScrollSpeed = fSpeed; }
	float					GetScrollDelay() { return m_fScrollDelay; }
	void					SetScrollDelay( float fDelay ) { m_fScrollDelay = fDelay; }

	// sets the scroll position
	void					SetScrollPos( int32 nPos ) { m_nPos = nPos; RecalcLayout(); }
	int32					GetScrollPos() { return m_nPos; }

	void					SetFrameWidth( uint8 nFrameWidth );

	virtual void			Show ( bool bShow );

	virtual bool			OnMouseMove(int x, int y);
	virtual bool			OnLButtonDown(int x, int y);
	virtual bool			OnLButtonUp(int x, int y);
	virtual bool			OnMouseWheel(int x, int y, int zDelta);

protected:
	// control to which we send messages
	CLTGUICtrl*				m_pMessageControl;

	CLTGUICommandHandler*	m_pInputCaptureHandler;

	eCapture				m_CaptureState;
	LTVector2n				m_ptCursor;
	LTVector2n				m_ptThumbOffset;

	StopWatchTimer			m_ScrollTimer;

	LT_POLYGT4				m_Bar[eBarElementCount];
	TextureReference		m_hBarNormal;
	TextureReference		m_hBarHot;

	uint32					m_nPage;
	int32					m_nMin;
	int32					m_nMax;
	int32					m_nPos;

	LTRect2n				m_rcTrackWithoutArrows;
	LTRect2n				m_rcUpArrow;
	LTRect2n				m_rcDownArrow;
	LTRect2n				m_rcThumb;

	float					m_fShaftScale;

	float					m_fScrollSpeed;
	float					m_fScrollDelay;

	// frame data
	uint8					m_nFrameWidth;
	LT_POLYG4				m_Frame[8];

	uint32					m_nBackgroundColor;
	uint32					m_nSelectedColor;

	void					SetRenderState();
	void					InitBar();
	void					RecalcLayout();
	void					ComputeScaleAndThumb( uint32& nThumbSize );
};

#endif  // __LTGUISCROLLBAR_H__
