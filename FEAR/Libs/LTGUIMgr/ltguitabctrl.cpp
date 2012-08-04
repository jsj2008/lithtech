// ----------------------------------------------------------------------- //
//
// MODULE  : ltguitabctrl.cpp
//
// PURPOSE : Defines the CLTGUITabCtrl class
//
// CREATED : 03/01/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "ltguimgr.h"
#include "ltguitabctrl.h"
#include "InterfaceSound.h"

const int32 CLTGUITabCtrl::s_kInvalidItemIndex = -1;

// constructor
CLTGUITabCtrl::CLTGUITabCtrl() :
	m_bGlowEnable(true), 
	m_fGlowAlpha(0.25f), 
	m_vGlowSize(1.2f, 1.2f),
	m_nBaseFontSize(0),
	m_nTextIndent(0),
	m_sizeIcon(0,0),
	m_nActiveItem(s_kInvalidItemIndex),
	m_nDividerWidth(6),
	m_nFrameWidth(0),
	m_nBackgroundColor(0),
	m_nSelectedColor(0)
{
}

// destructor
CLTGUITabCtrl::~CLTGUITabCtrl()
{
	Destroy();
}

// creates a tab control
bool CLTGUITabCtrl::Create( const CLTGUITabCtrl_create& cs )
{
	m_ItemList.reserve( cs.nItemCount );

	m_bGlowEnable		= cs.bGlowEnable;
	m_fGlowAlpha		= cs.fGlowAlpha;
	m_vGlowSize			= cs.vGlowSize;
	m_nTextIndent		= cs.nTextIdent;
	m_nSelectedColor	= cs.nSelectedColor;
	m_nBackgroundColor	= cs.nBackgroundColor;

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	Init();

	return true;
}

// destroys the scrollbar
void CLTGUITabCtrl::Destroy()
{
	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		debug_delete(m_ItemList[nItem]);
	}

	m_ItemList.clear();
}


// initializes the control
void CLTGUITabCtrl::Init()
{
}

void CLTGUITabCtrl::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}

// Render the control
void CLTGUITabCtrl::Render()
{
	if( !IsVisible() )
		return;

	// set up the render state	
	SetRenderState();

	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		STabItem* pItem = m_ItemList[nItem];
		if( !pItem )
			continue;

		if( m_nActiveItem == nItem)
		{
			if (IsSelected())
			{
				LT_POLYG4 polyBack;
				DrawPrimSetRGBA( polyBack, m_nSelectedColor );
				DrawPrimSetXYWH( polyBack, (float)pItem->m_Rect.Left() + 1, m_rfRect.Top(), (float)pItem->m_Rect.GetWidth() - 1, m_rfRect.GetHeight() );
				g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
				g_pDrawPrim->DrawPrim( &polyBack );
			}
			else
			{
				LT_POLYG4 polyBack;
				DrawPrimSetRGBA( polyBack, m_nBackgroundColor );
				DrawPrimSetXYWH( polyBack, (float)pItem->m_Rect.Left() + 1, m_rfRect.Top(), (float)pItem->m_Rect.GetWidth() - 1, m_rfRect.GetHeight() );
				g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
				g_pDrawPrim->DrawPrim( &polyBack );
			}
		}

		// draw the separator
		if( nItem + 1 != nItemCount )
		{
			if( m_nActiveItem == nItem && IsSelected() )
			{
				LT_POLYG4 polySep;
				DrawPrimSetRGBA( polySep, GetCurrentColor() );
				DrawPrimSetXYWH( polySep, (float)pItem->m_Rect.Right(), (float)pItem->m_Rect.Top() + 1, 1, (float)pItem->m_Rect.GetHeight() - 1 );
				g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
				g_pDrawPrim->DrawPrim( &polySep );
			}
			else
			{
				LT_POLYG4 polySep;
				DrawPrimSetRGBA( polySep, m_argbNormal );
				DrawPrimSetXYWH( polySep, (float)pItem->m_Rect.Right(), (float)pItem->m_Rect.Top() + 1, 1, (float)pItem->m_Rect.GetHeight() - 1 );
				g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
				g_pDrawPrim->DrawPrim( &polySep );
			}
		}

		if( m_nActiveItem == nItem )
		{
			if( m_ItemList[nItem]->m_hIconHot )
			{
				g_pDrawPrim->SetTexture( m_ItemList[nItem]->m_hIconHot );
				g_pDrawPrim->DrawPrim( &pItem->m_PolyIconHot );
			}

			uint32 argbColor = GetCurrentColor();

			pItem->m_Text.SetColor( argbColor );
			pItem->m_Text.SetGlow( true );
		}
		else
		{
			if( m_ItemList[nItem]->m_hIconNormal )
			{
				g_pDrawPrim->SetTexture( m_ItemList[nItem]->m_hIconNormal );
				g_pDrawPrim->DrawPrim( &pItem->m_PolyIconNormal );
			}

			uint32 argbColor = m_argbNormal;
			pItem->m_Text.SetColor( argbColor );
			pItem->m_Text.SetGlow( false );
		}

		LTRect2n rcText = pItem->m_Rect;
		rcText.Expand( -(int32)m_nTextIndent, 0 );
		pItem->m_Text.RenderClipped( rcText );
	}

	// render the border
	if( m_nFrameWidth )
	{
		// set up the render state	
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

		for (int f = 0;f < 3; ++f)
			DrawPrimSetRGBA(m_Frame[f],GetCurrentColor());

		// draw our frames
		g_pDrawPrim->DrawPrim(m_Frame,3);
	}
}

// Render the control
void CLTGUITabCtrl::RenderTransition(float fTrans)
{
}

// updates the position and size of all Tabs
void CLTGUITabCtrl::RecalcLayout()
{
	float nStartX = m_rnBaseRect.Left() * m_vfScale.x;
	int32 nCapWidth = (int32)((GetBaseHeight() / 2) * m_vfScale.x);

	uint32 nTabCount = (uint32)m_ItemList.size();
	for(uint32 nTab=0;nTab<nTabCount;++nTab)
	{
		float fWidth = (float)LTMAX(m_ItemList[nTab]->m_nWidth, GetBaseHeight());
		fWidth *= m_vfScale.x;

		if( nTab == nTabCount - 1 )
			m_ItemList[nTab]->m_Rect.Init( (int32)nStartX, (int32)m_rfRect.Top(), (int32)m_rfRect.Right(), (int32)m_rfRect.Bottom() );
		else
			m_ItemList[nTab]->m_Rect.Init( (int32)nStartX, (int32)m_rfRect.Top(), (int32)(nStartX + fWidth), (int32)m_rfRect.Bottom() );

		// if it only contains an icon but not text then center the icon
		if( (m_ItemList[nTab]->m_hIconNormal || m_ItemList[nTab]->m_hIconHot) && m_ItemList[nTab]->m_Text.IsEmpty() )
		{
			int32 nIconStart = (int32)(nStartX + (fWidth - (int32)(m_sizeIcon.x * m_vfScale.x))/2);

			if( m_ItemList[nTab]->m_hIconNormal )
				DrawPrimSetXYWH( m_ItemList[nTab]->m_PolyIconNormal, (float)nIconStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
			if( m_ItemList[nTab]->m_hIconHot )
				DrawPrimSetXYWH( m_ItemList[nTab]->m_PolyIconHot, (float)nIconStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
		}
		else
		{
			int32 nTextStart = (int32)(nStartX + (int32)(m_nTextIndent * m_vfScale.x));
			if( m_ItemList[nTab]->m_hIconNormal )
				DrawPrimSetXYWH( m_ItemList[nTab]->m_PolyIconNormal, (float)nTextStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
			if( m_ItemList[nTab]->m_hIconHot )
				DrawPrimSetXYWH( m_ItemList[nTab]->m_PolyIconHot, (float)nTextStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );

			if( m_ItemList[nTab]->m_hIconNormal || m_ItemList[nTab]->m_hIconHot )
				nTextStart += m_sizeIcon.x + (int32)(m_nTextIndent * m_vfScale.x);

			uint32 nFontHeight = m_ItemList[nTab]->m_Text.GetFontHeight();

			if( m_ItemList[nTab]->m_Text.GetAlignment() == kRight )
				m_ItemList[nTab]->m_Text.SetPos( LTVector2n((int32)(nStartX + fWidth - (m_nTextIndent * m_vfScale.x)), (int32)(m_rfRect.Top() + ((m_rfRect.GetHeight() - nFontHeight)*0.5f)) + 1) );
			else
				m_ItemList[nTab]->m_Text.SetPos( LTVector2n(nTextStart, (int32)(m_rfRect.Top() + ((m_rfRect.GetHeight() - nFontHeight)*0.5f)) + 1) );
		}

		nStartX += fWidth;
	}

	float frameW = ((float)m_nFrameWidth/* * m_vfScale.x*/);
	float frameH = ((float)m_nFrameWidth/* * m_vfScale.y*/);

	//top
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();
	float fw = m_rfRect.GetWidth();
	float fh = frameH;
	DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);

	//left
	fx = m_rfRect.Left();
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

	//right
	fx = m_rfRect.Right() - frameW;
	fy = m_rfRect.Top();
	fw = frameW;
	fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);
}

void CLTGUITabCtrl::FlushTextureStrings()
{
	uint32 nTabCount = (uint32)m_ItemList.size();
	for(uint32 nTab=0;nTab<nTabCount;++nTab)
	{
		m_ItemList[nTab]->m_Text.FlushTexture();
	}
}

void CLTGUITabCtrl::RecreateTextureStrings()
{
	uint32 nTabCount = (uint32)m_ItemList.size();
	for(uint32 nTab=0;nTab<nTabCount;++nTab)
	{
		if( !m_ItemList[nTab]->m_Text.IsEmpty() )
			m_ItemList[nTab]->m_Text.CreateTexture();
	}
}

// set the font of existing and new Tabs
void CLTGUITabCtrl::SetFont( const CFontInfo& Font )
{
	m_Font = Font;
	m_nBaseFontSize = m_Font.m_nHeight;
	m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	uint32 nTabCount = (uint32)m_ItemList.size();
	for(uint32 nTab=0;nTab<nTabCount;++nTab)
	{
		m_ItemList[nTab]->m_Text.SetFont( m_Font );
	}
}

// add an item to the list
bool CLTGUITabCtrl::InsertItem( uint32 nIndex, uint32 nId, const wchar_t* wszString, const char* szHelpId, eTextAlign align, uint32 nWidth, const char* szIconNormal /*= NULL*/, const char* szIconHot /*= NULL*/ )
{
	STabItem* pItem = debug_new(STabItem);
	if( !pItem )
		return false;

	pItem->m_szHelpId	= szHelpId;
	pItem->m_nWidth		= nWidth;
	pItem->m_idTab	= nId;

	pItem->m_Text.SetFont( m_Font );
	pItem->m_Text.SetText( wszString, true );
	pItem->m_Text.SetAlignment( align );
	pItem->m_Text.SetGlowParams( m_bGlowEnable, m_fGlowAlpha, m_vGlowSize );

	if( szIconNormal )
	{
		pItem->m_hIconNormal.Load( szIconNormal );
		SetupQuadUVs( pItem->m_PolyIconNormal, pItem->m_hIconNormal, 0.0f, 0.0f, 1.0f, 1.0f );
		DrawPrimSetRGBA( pItem->m_PolyIconNormal, 0xFF, 0xFF, 0xFF, 0xFF );
	}

	if( szIconHot )
	{
		pItem->m_hIconHot.Load( szIconNormal );
		SetupQuadUVs( pItem->m_PolyIconHot, pItem->m_hIconHot, 0.0f, 0.0f, 1.0f, 1.0f );
		DrawPrimSetRGBA( pItem->m_PolyIconHot, 0xFF, 0xFF, 0xFF, 0xFF );
	}
	else if( szIconNormal )
	{
		// use the normal for the hot
		pItem->m_hIconHot		= pItem->m_hIconNormal;
		pItem->m_PolyIconHot	= pItem->m_PolyIconNormal;
	}


	if( nIndex > GetItemCount() )
		nIndex = GetItemCount();

	m_ItemList.insert( m_ItemList.begin() + nIndex, pItem );

	RecalcLayout();

	return true;
}

// delete an item from the list
bool CLTGUITabCtrl::DeleteItem( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return false;

	debug_delete(m_ItemList[nIndex]);
	m_ItemList.erase( m_ItemList.begin() + nIndex );

	RecalcLayout();

	return true;
}

// select an item
void CLTGUITabCtrl::SelectItem( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
	{
		m_nActiveItem = s_kInvalidItemIndex;
		return;
	}
	m_nActiveItem = nIndex;
}


// return an item's rect
LTRect2n CLTGUITabCtrl::GetItemRect( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return LTRect2n(0,0,0,0);

	return m_ItemList[nIndex]->m_Rect;
}

void CLTGUITabCtrl::SetBasePos( const LTVector2n& pos )
{
	CLTGUICtrl::SetBasePos( pos );
	RecalcLayout();
}

void CLTGUITabCtrl::SetSize( const LTVector2n& sz )
{
	CLTGUICtrl::SetSize( sz );
	RecalcLayout();
}

void CLTGUITabCtrl::SetScale(const LTVector2& vfScale)
{
	bool bRebuild = (vfScale != m_vfScale);

	CLTGUICtrl::SetScale(vfScale);

	if( bRebuild )
	{
		m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

		uint32 nTabCount = (uint32)m_ItemList.size();
		for(uint32 nTab=0;nTab<nTabCount;++nTab)
		{
			m_ItemList[nTab]->m_Text.SetFontHeight( (uint32)(m_vfScale.y * (float)m_nBaseFontSize) );
		}

		RecalcLayout();
	}
}

// scales all items to fit in the available area of the Tab control
void CLTGUITabCtrl::Rescale()
{
	uint32 nItemCount = GetItemCount();
	if( nItemCount == 0 )
		return;

	uint32 nTotalWidth = 0;
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		nTotalWidth += m_ItemList[nItem]->m_nWidth;
	}

	uint32 nBaseWidth = GetBaseWidth();
	for(uint32 nItem=0;nItem<nItemCount - 1;++nItem)
	{
		m_ItemList[nItem]->m_nWidth = m_ItemList[nItem]->m_nWidth * nBaseWidth / nTotalWidth;
	}

	// make the last column take up any remaining space
	nTotalWidth = 0;
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		if( nItem == nItemCount - 1 )
		{
			m_ItemList[nItem]->m_nWidth = nBaseWidth - nTotalWidth;
		}
		else
		{
			nTotalWidth += m_ItemList[nItem]->m_nWidth;
		}
	}

	RecalcLayout();
}

// returns the column that is under the cursor
int32 CLTGUITabCtrl::GetItemUnderCursor(int x, int y)
{
	int32 nItemCount = (int32)GetItemCount();
	if( nItemCount == 0 )
		return s_kInvalidItemIndex;

	LTVector2n ptCursor( x, y );

	for(int32 nItem=0;nItem<nItemCount;++nItem)
	{
		STabItem* pItem = m_ItemList[nItem];
		if( !pItem )
			continue;

		if( pItem->m_Rect.Contains(ptCursor) )
			return nItem;
	}
	
	return s_kInvalidItemIndex;
}


// sets the width of an item
void CLTGUITabCtrl::SetItemWidth( int32 nItem, int32 nWidth )
{
	if( nItem == s_kInvalidItemIndex || nItem >= (int32)GetItemCount() )
		return;

	nWidth = LTMIN( GetItemMaximumWidth(nItem), LTMAX( nWidth, GetBaseHeight() ));
	m_ItemList[nItem]->m_nWidth = nWidth;

	FixItemWidths( nItem );

	RecalcLayout();
}

// returns the maximum width of an item
uint32 CLTGUITabCtrl::GetItemMaximumWidth( uint32 nThisItem )
{
	uint32 nWidthTaken = 0;
	for(uint32 nItem=0;nItem<nThisItem;++nItem)
	{
		nWidthTaken += m_ItemList[nItem]->m_nWidth;
	}
	
	uint32 nTotalWidth = GetBaseWidth();

	uint32 nWidthReserved = 0;
	uint32 nWidthMin = GetBaseHeight();

	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=nThisItem+1;nItem<nItemCount;++nItem)
	{
		nWidthReserved += m_ItemList[nItem]->m_nWidth;
	}

	return nTotalWidth - nWidthTaken - nWidthReserved;
}

// fix the widths of all items after this one
void CLTGUITabCtrl::FixItemWidths( uint32 nThisItem )
{
	uint32 nTotalWidth = GetBaseWidth();

	uint32 nWidthMin = GetBaseHeight();

	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=nItemCount-1;nItem>nThisItem;--nItem)
	{
		uint32 nItemMinWidth = m_ItemList[nItem]->m_nWidth;
		uint32 nItemPosition = GetItemPosition(nItem);
		if( nItemPosition + nItemMinWidth > nTotalWidth )
		{
			m_ItemList[nItem]->m_nWidth = nItemMinWidth;
			nTotalWidth -= nItemMinWidth;
		}
		else
		{
			nTotalWidth -= nItemMinWidth;
		}
	}
}


// called when the left mouse button is pressed over the scroll control
bool CLTGUITabCtrl::OnLButtonDown(int x, int y)
{
	m_nActiveItem = GetItemUnderCursor(x, y);
	if( m_nActiveItem != s_kInvalidItemIndex )
	{
		if (m_pCommandHandler)
		{
			m_pCommandHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_SELECT, 0 );
			m_pCommandHandler->SendCommand( m_ItemList[m_nActiveItem]->m_idTab, 0, 0 );
			return true;
		}
	}

	return false;
}


bool CLTGUITabCtrl::OnMouseMove(int x, int y) 
{
	int32 nItem = GetItemUnderCursor(x,y);
	if (nItem != m_nRolloverItem)
	{
		if (m_pCommandHandler && nItem != s_kInvalidItemIndex)
		{
			m_pCommandHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_CHANGE, 0 );
		}
		m_nRolloverItem = nItem;
		return true;
	}
	
	return false;
}



// set the frame width
void CLTGUITabCtrl::SetFrameWidth( uint8 nFrameWidth )
{
	m_nFrameWidth = nFrameWidth;
	RecalcLayout();
}

// returns the help ID for the item
const char* CLTGUITabCtrl::GetHelpID()
{
	if( m_nRolloverItem != -1 && m_nRolloverItem < (int32)m_ItemList.size() )
		return m_ItemList[m_nRolloverItem]->m_szHelpId;

	return "";
}