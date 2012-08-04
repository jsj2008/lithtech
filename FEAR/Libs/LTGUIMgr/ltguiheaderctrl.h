// ----------------------------------------------------------------------- //
//
// MODULE  : ltguiheaderctrl.h
//
// PURPOSE : Declares the CLTGUIHeaderCtrl class.  This class creates a
//           header control that can be associated with a list.
//
// CREATED : 06/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUIHEADERCTRL_H__
#define __LTGUIHEADERCTRL_H__

#include "LTGUICtrl.h"

class CLTGUIScrollBar;

struct CLTGUIHeaderCtrl_create : public CLTGUICtrl_create
{
	CLTGUIHeaderCtrl_create();
	//TextureReference	hHeaderTextureNormal;
	//TextureReference	hHeaderTextureHot;
	uint32				nItemCount;
	uint32				nTextIdent;
	CLTGUIScrollBar*	pScrollBar;
	uint32				nBackgroundColor;
	uint32				nSelectedColor;
	uint32				nSortedColor;
};

enum eHeaderCtrlCmd
{
	eHeaderCtrlCmd_Size = 0x200,
	eHeaderCtrlCmd_Click,
};

inline CLTGUIHeaderCtrl_create::CLTGUIHeaderCtrl_create() :
	nItemCount(0),
	pScrollBar(NULL),
	nBackgroundColor(0),
	nSelectedColor(0),
	nSortedColor(0)
{
}

class CLTGUIHeaderCtrl : public CLTGUICtrl
{
public:
	enum eCapture
	{
		eCapture_None,
		eCapture_Divider,
	};

	enum eElement
	{
		eElement_LeftCap,
		eElement_RightCap,
		eElement_Center,

		eElementCount
	};

	struct SHeaderItem
	{
		uint32				m_nWidth;
		bool				m_bSizeable;
		uint32				m_idHeader;
		CLTGUIString		m_Text;
		//LT_POLYGT4			m_Poly[eElementCount];
		LTRect2n			m_Rect;
		TextureReference	m_hIconNormal;
		TextureReference	m_hIconHot;
		LT_POLYGT4			m_PolyIconNormal;
		LT_POLYGT4			m_PolyIconHot;
		const char*			m_szHelpId;
	};
	typedef std::vector<SHeaderItem*, LTAllocator<SHeaderItem*, LT_MEM_TYPE_CLIENTSHELL> > HeaderItemArray;

public:
	CLTGUIHeaderCtrl();
	virtual ~CLTGUIHeaderCtrl();

	static const int32		s_kInvalidItemIndex;

	// Create the control
	bool					Create( const CLTGUIHeaderCtrl_create& cs );
	void					Destroy();

	void					SetMessageControl( CLTGUICtrl* pControl ) { m_pMessageControl = pControl; }
	CLTGUICtrl*				GetMessageControl() { return m_pMessageControl; }

	void					SetInputCaptureHandler( CLTGUICommandHandler* pHandler ) { m_pInputCaptureHandler = pHandler; }
	CLTGUICommandHandler*	GetInputCaptureHandler() { return m_pInputCaptureHandler; }

	// Render the control
	virtual void			Render();
	virtual void			RenderTransition(float fTrans);

	virtual void			FlushTextureStrings();
	virtual void			RecreateTextureStrings();

	virtual void			SetBasePos( const LTVector2n& pos );
	virtual void			SetSize( const LTVector2n& sz );
	virtual void			SetScale(const LTVector2& vfScale);

	virtual bool			OnMouseMove(int x, int y);
	virtual bool			OnLButtonDown(int x, int y);
	virtual bool			OnLButtonUp(int x, int y);

	virtual const char*		GetHelpID();

	void					SetFont( const CFontInfo& Font );

	// add/remove
	bool					InsertItem( uint32 nIndex, uint32 nId, const wchar_t* wszString, const char* szHelpId, eTextAlign align, uint32 nWidth, bool bSizeable, const char* szIconNormal = NULL, const char* szIconHot = NULL );
	bool					DeleteItem( uint32 nIndex );

	uint32					GetItemCount() { return (uint32)m_ItemList.size(); }
	LTRect2n				GetItemRect( uint32 nIndex );
	inline uint32			GetItemBaseWidth( uint32 nIndex );

	void					SetIconSize( LTVector2n size ) { m_sizeIcon = size; }
	LTVector2n				GetIconSize() { return m_sizeIcon; }

	void					Rescale();

	void					SetFrameWidth( uint8 nFrameWidth );

protected:
	// control to which we send messages
	CLTGUICtrl*				m_pMessageControl;
	CLTGUIScrollBar*		m_pScrollBar;

	CLTGUICommandHandler*	m_pInputCaptureHandler;

	eCapture				m_CaptureState;
	LTVector2n				m_ptCursor;

	//TextureReference		m_hTextureNormal;
	//TextureReference		m_hTextureHot;

	uint32					m_nTextIndent;

	LTVector2n				m_sizeIcon;				// icon size

	uint32					m_nBackgroundColor;
	uint32					m_nSelectedColor;
	uint32					m_nSortedColor;

	// list of header items
	HeaderItemArray			m_ItemList;
	int32					m_nActiveItem;
	int32					m_nDragItem;

	LTVector2n				m_ptDragOffset;

	// font data
	CFontInfo				m_Font;
	uint32					m_nBaseFontSize;		// The font size before scaling
	bool					m_bGlowEnable;
	float					m_fGlowAlpha;
	LTVector2				m_vGlowSize;
	uint32					m_nDividerWidth;

	// frame data
	uint8					m_nFrameWidth;
	LT_POLYG4				m_Frame[4];

	void					Init();
	void					SetRenderState();
	void					RecalcLayout();
	int32					GetItemUnderCursor(int x, int y);
	int32					IsCursorUnderDivider(int x, int y);
	void					SetItemWidth( int32 nItem, int32 nWidth );
	uint32					GetItemMaximumWidth( uint32 nThisItem );
	void					FixItemWidths( uint32 nThisItem );
	inline uint32			GetItemPosition( uint32 nThisItem );
};

/*
 *	Inlines
 */
uint32 CLTGUIHeaderCtrl::GetItemPosition( uint32 nThisItem )
{
	uint32 nWidth = 0;
	for(uint32 nItem=0;nItem<nThisItem;++nItem)
		nWidth += m_ItemList[nItem]->m_nWidth;

	return nWidth;
}

// return an item's width
uint32 CLTGUIHeaderCtrl::GetItemBaseWidth( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return 0;

	return m_ItemList[nIndex]->m_nWidth;
}


#endif  // __LTGUIHEADERCTRL_H__
