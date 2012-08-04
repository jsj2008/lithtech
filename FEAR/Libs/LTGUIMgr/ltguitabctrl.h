// ----------------------------------------------------------------------- //
//
// MODULE  : ltguitabctrl.h
//
// PURPOSE : Declares the CLTGUITabCtrl class
//
// CREATED : 03/01/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LTGUITABCTRL_H__
#define __LTGUITABCTRL_H__

#include "LTGUICtrl.h"


struct CLTGUITabCtrl_create : public CLTGUICtrl_create
{
	CLTGUITabCtrl_create();
	uint32				nItemCount;
	uint32				nTextIdent;
	uint32				nBackgroundColor;
	uint32				nSelectedColor;
};


inline CLTGUITabCtrl_create::CLTGUITabCtrl_create() :
	nItemCount(0),
	nTextIdent(0),
	nBackgroundColor(0),
	nSelectedColor(0)
{
}

class CLTGUITabCtrl : public CLTGUICtrl
{
public:
	struct STabItem
	{
		uint32				m_nWidth;
		uint32				m_idTab;
		CLTGUIString		m_Text;
		LTRect2n			m_Rect;
		TextureReference	m_hIconNormal;
		TextureReference	m_hIconHot;
		LT_POLYGT4			m_PolyIconNormal;
		LT_POLYGT4			m_PolyIconHot;
		const char*			m_szHelpId;
	};
	typedef std::vector<STabItem*, LTAllocator<STabItem*, LT_MEM_TYPE_CLIENTSHELL> > TabItemArray;

public:
	CLTGUITabCtrl();
	virtual ~CLTGUITabCtrl();

	static const int32		s_kInvalidItemIndex;

	// Create the control
	bool					Create( const CLTGUITabCtrl_create& cs );
	void					Destroy();

	// Render the control
	virtual void			Render();
	virtual void			RenderTransition(float fTrans);

	virtual void			FlushTextureStrings();
	virtual void			RecreateTextureStrings();

	virtual void			SetBasePos( const LTVector2n& pos );
	virtual void			SetSize( const LTVector2n& sz );
	virtual void			SetScale(const LTVector2& vfScale);

	virtual bool			OnLButtonDown(int x, int y);
	virtual bool			OnMouseMove(int x, int y);


	virtual const char*		GetHelpID();

	void					SetFont( const CFontInfo& Font );

	// add/remove
	bool					InsertItem( uint32 nIndex, uint32 nId, const wchar_t* wszString, const char* szHelpId, eTextAlign align, uint32 nWidth, const char* szIconNormal = NULL, const char* szIconHot = NULL );
	bool					DeleteItem( uint32 nIndex );
	void					SelectItem( uint32 nIndex );

	uint32					GetItemCount() { return (uint32)m_ItemList.size(); }
	LTRect2n				GetItemRect( uint32 nIndex );
	inline uint32			GetItemBaseWidth( uint32 nIndex );

	void					SetIconSize( LTVector2n size ) { m_sizeIcon = size; }
	LTVector2n				GetIconSize() { return m_sizeIcon; }

	void					Rescale();

	void					SetFrameWidth( uint8 nFrameWidth );

protected:
	uint32					m_nTextIndent;

	LTVector2n				m_sizeIcon;				// icon size

	uint32					m_nBackgroundColor;
	uint32					m_nSelectedColor;

	// list of Tab items
	TabItemArray			m_ItemList;
	int32					m_nActiveItem;
	int32					m_nRolloverItem;

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
	void					SetItemWidth( int32 nItem, int32 nWidth );
	uint32					GetItemMaximumWidth( uint32 nThisItem );
	void					FixItemWidths( uint32 nThisItem );
	inline uint32			GetItemPosition( uint32 nThisItem );
};

/*
 *	Inlines
 */
uint32 CLTGUITabCtrl::GetItemPosition( uint32 nThisItem )
{
	uint32 nWidth = 0;
	for(uint32 nItem=0;nItem<nThisItem;++nItem)
		nWidth += m_ItemList[nItem]->m_nWidth;

	return nWidth;
}

// return an item's width
uint32 CLTGUITabCtrl::GetItemBaseWidth( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return 0;

	return m_ItemList[nIndex]->m_nWidth;
}


#endif  // __LTGUITABCTRL_H__
