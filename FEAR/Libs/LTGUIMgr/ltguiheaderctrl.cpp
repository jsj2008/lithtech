// ----------------------------------------------------------------------- //
//
// MODULE  : ltguiheaderctrl.cpp
//
// PURPOSE : Defines the CLTGUIHeaderCtrl class.  This class creates a
//           header control that can be associated with a list.
//
// CREATED : 06/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguiheaderctrl.h"
#include "InterfaceSound.h"

const int32 CLTGUIHeaderCtrl::s_kInvalidItemIndex = -1;

// constructor
CLTGUIHeaderCtrl::CLTGUIHeaderCtrl() :
	m_pMessageControl(NULL),
	m_pScrollBar(NULL),
	m_pInputCaptureHandler(NULL),
	m_CaptureState(eCapture_None),
	m_bGlowEnable(true), 
	m_fGlowAlpha(0.25f), 
	m_vGlowSize(1.2f, 1.2f),
	m_nBaseFontSize(0),
	m_nTextIndent(0),
	m_sizeIcon(0,0),
	m_nActiveItem(s_kInvalidItemIndex),
	m_nDragItem(s_kInvalidItemIndex),
	m_nDividerWidth(6),
	m_nFrameWidth(0),
	m_nBackgroundColor(0),
	m_nSelectedColor(0),
	m_nSortedColor(0)
{
}

// destructor
CLTGUIHeaderCtrl::~CLTGUIHeaderCtrl()
{
	Destroy();
}

// creates a scrollbar control
bool CLTGUIHeaderCtrl::Create( const CLTGUIHeaderCtrl_create& cs )
{
	//if( !cs.hHeaderTextureNormal || !cs.hHeaderTextureHot ) 
	//	return false;

	//m_hTextureNormal = cs.hHeaderTextureNormal;
	//m_hTextureHot = cs.hHeaderTextureHot;
	m_ItemList.reserve( cs.nItemCount );

	m_bGlowEnable		= cs.bGlowEnable;
	m_fGlowAlpha		= cs.fGlowAlpha;
	m_vGlowSize			= cs.vGlowSize;
	m_nTextIndent		= cs.nTextIdent;
	m_pScrollBar		= cs.pScrollBar;
	m_nSelectedColor	= cs.nSelectedColor;
	m_nBackgroundColor	= cs.nBackgroundColor;
	m_nSortedColor		= cs.nSortedColor;

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	Init();

	return true;
}

// destroys the scrollbar
void CLTGUIHeaderCtrl::Destroy()
{
	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		debug_delete(m_ItemList[nItem]);
	}

	m_ItemList.clear();
}


// initializes the control
void CLTGUIHeaderCtrl::Init()
{
}

void CLTGUIHeaderCtrl::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}

// Render the control
void CLTGUIHeaderCtrl::Render()
{
	if( !IsVisible() )
		return;

	// set up the render state	
	SetRenderState();

	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=0;nItem<nItemCount;++nItem)
	{
		SHeaderItem* pItem = m_ItemList[nItem];
		if( !pItem )
			continue;

		if( m_nActiveItem == nItem && IsSelected() )
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

		//TextureReference& hTexture = (m_nActiveItem == nItem && IsSelected())?m_hTextureHot:m_hTextureNormal;

		//g_pDrawPrim->SetTexture( hTexture );
		//g_pDrawPrim->DrawPrim( &pItem->m_Poly[eElement_Center] );

		//g_pDrawPrim->SetTexture( hTexture );
		//g_pDrawPrim->DrawPrim( &pItem->m_Poly[eElement_LeftCap] );

		//g_pDrawPrim->SetTexture( hTexture );
		//g_pDrawPrim->DrawPrim( &pItem->m_Poly[eElement_RightCap] );

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
		if( m_pScrollBar )
			g_pDrawPrim->DrawPrim(m_Frame,m_pScrollBar->IsVisible()?2:3);
		else
			g_pDrawPrim->DrawPrim(m_Frame,3);
	}
}

// Render the control
void CLTGUIHeaderCtrl::RenderTransition(float fTrans)
{
}

// updates the position and size of all headers
void CLTGUIHeaderCtrl::RecalcLayout()
{
	float nStartX = m_rnBaseRect.Left() * m_vfScale.x;
	int32 nCapWidth = (int32)((GetBaseHeight() / 2) * m_vfScale.x);

	uint32 nHeaderCount = (uint32)m_ItemList.size();
	for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
	{
		float fWidth = (float)LTMAX(m_ItemList[nHeader]->m_nWidth, GetBaseHeight());
		fWidth *= m_vfScale.x;

		if( nHeader == nHeaderCount - 1 )
			m_ItemList[nHeader]->m_Rect.Init( (int32)nStartX, (int32)m_rfRect.Top(), (int32)m_rfRect.Right(), (int32)m_rfRect.Bottom() );
		else
			m_ItemList[nHeader]->m_Rect.Init( (int32)nStartX, (int32)m_rfRect.Top(), (int32)(nStartX + fWidth), (int32)m_rfRect.Bottom() );

		//DrawPrimSetXYWH( m_ItemList[nHeader]->m_Poly[eElement_LeftCap], (float)m_ItemList[nHeader]->m_Rect.Left(), (float)m_ItemList[nHeader]->m_Rect.Top(), (float)nCapWidth, (float)m_ItemList[nHeader]->m_Rect.GetHeight() );
		//DrawPrimSetXYWH( m_ItemList[nHeader]->m_Poly[eElement_RightCap], (float)(m_ItemList[nHeader]->m_Rect.Right() - nCapWidth), (float)m_ItemList[nHeader]->m_Rect.Top(), (float)nCapWidth, (float)m_ItemList[nHeader]->m_Rect.GetHeight() );
		//DrawPrimSetXYWH( m_ItemList[nHeader]->m_Poly[eElement_Center], (float)(m_ItemList[nHeader]->m_Rect.Left() + nCapWidth), (float)m_ItemList[nHeader]->m_Rect.Top(), (float)(m_ItemList[nHeader]->m_Rect.GetWidth() - (nCapWidth*2)), (float)m_ItemList[nHeader]->m_Rect.GetHeight() );

		// if it only contains an icon but not text then center the icon
		if( (m_ItemList[nHeader]->m_hIconNormal || m_ItemList[nHeader]->m_hIconHot) && m_ItemList[nHeader]->m_Text.IsEmpty() )
		{
			int32 nIconStart = (int32)(nStartX + (fWidth - (int32)(m_sizeIcon.x * m_vfScale.x))/2);

			if( m_ItemList[nHeader]->m_hIconNormal )
				DrawPrimSetXYWH( m_ItemList[nHeader]->m_PolyIconNormal, (float)nIconStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
			if( m_ItemList[nHeader]->m_hIconHot )
				DrawPrimSetXYWH( m_ItemList[nHeader]->m_PolyIconHot, (float)nIconStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
		}
		else
		{
			int32 nTextStart = (int32)(nStartX + (int32)(m_nTextIndent * m_vfScale.x));
			if( m_ItemList[nHeader]->m_hIconNormal )
				DrawPrimSetXYWH( m_ItemList[nHeader]->m_PolyIconNormal, (float)nTextStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );
			if( m_ItemList[nHeader]->m_hIconHot )
				DrawPrimSetXYWH( m_ItemList[nHeader]->m_PolyIconHot, (float)nTextStart, m_rfRect.Top() + ((GetHeight() - (m_sizeIcon.y * m_vfScale.y)) * 0.5f), m_sizeIcon.x * m_vfScale.x, m_sizeIcon.y * m_vfScale.y );

			if( m_ItemList[nHeader]->m_hIconNormal || m_ItemList[nHeader]->m_hIconHot )
				nTextStart += m_sizeIcon.x + (int32)(m_nTextIndent * m_vfScale.x);

			uint32 nFontHeight = m_ItemList[nHeader]->m_Text.GetFontHeight();

			if( m_ItemList[nHeader]->m_Text.GetAlignment() == kRight )
				m_ItemList[nHeader]->m_Text.SetPos( LTVector2n((int32)(nStartX + fWidth - (m_nTextIndent * m_vfScale.x)), (int32)(m_rfRect.Top() + ((m_rfRect.GetHeight() - nFontHeight)*0.5f)) + 1) );
			else
				m_ItemList[nHeader]->m_Text.SetPos( LTVector2n(nTextStart, (int32)(m_rfRect.Top() + ((m_rfRect.GetHeight() - nFontHeight)*0.5f)) + 1) );
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

void CLTGUIHeaderCtrl::FlushTextureStrings()
{
	uint32 nHeaderCount = (uint32)m_ItemList.size();
	for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
	{
		m_ItemList[nHeader]->m_Text.FlushTexture();
	}
}

void CLTGUIHeaderCtrl::RecreateTextureStrings()
{
	uint32 nHeaderCount = (uint32)m_ItemList.size();
	for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
	{
		if( !m_ItemList[nHeader]->m_Text.IsEmpty() )
			m_ItemList[nHeader]->m_Text.CreateTexture();
	}
}

// set the font of existing and new headers
void CLTGUIHeaderCtrl::SetFont( const CFontInfo& Font )
{
	m_Font = Font;
	m_nBaseFontSize = m_Font.m_nHeight;
	m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	uint32 nHeaderCount = (uint32)m_ItemList.size();
	for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
	{
		m_ItemList[nHeader]->m_Text.SetFont( m_Font );
	}
}

// add an item to the list
bool CLTGUIHeaderCtrl::InsertItem( uint32 nIndex, uint32 nId, const wchar_t* wszString, const char* szHelpId, eTextAlign align, uint32 nWidth, bool bSizeable, const char* szIconNormal /*= NULL*/, const char* szIconHot /*= NULL*/ )
{
	SHeaderItem* pItem = debug_new(SHeaderItem);
	if( !pItem )
		return false;

	pItem->m_szHelpId	= szHelpId;
	pItem->m_nWidth		= nWidth;
	pItem->m_bSizeable	= bSizeable;
	pItem->m_idHeader	= nId;

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

	//SetupQuadUVs( pItem->m_Poly[eElement_LeftCap], m_hTextureNormal, 0.0f, 0.0f, 0.25f, 1.0f );
	//SetupQuadUVs( pItem->m_Poly[eElement_RightCap], m_hTextureNormal, 0.25f, 0.0f, 0.25f, 1.0f );
	//SetupQuadUVs( pItem->m_Poly[eElement_Center], m_hTextureNormal, 0.5f, 0.0f, 0.5f, 1.0f );

	//for(uint32 nElement=0;nElement<eElementCount;++nElement)
	//	DrawPrimSetRGBA(pItem->m_Poly[nElement], 0xFF, 0xFF, 0xFF, 0xFF);

	if( nIndex > GetItemCount() )
		nIndex = GetItemCount();

	m_ItemList.insert( m_ItemList.begin() + nIndex, pItem );

	RecalcLayout();

	return true;
}

// delete an item from the list
bool CLTGUIHeaderCtrl::DeleteItem( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return false;

	debug_delete(m_ItemList[nIndex]);
	m_ItemList.erase( m_ItemList.begin() + nIndex );

	RecalcLayout();

	return true;
}

// return an item's rect
LTRect2n CLTGUIHeaderCtrl::GetItemRect( uint32 nIndex )
{
	if( nIndex >= GetItemCount() )
		return LTRect2n(0,0,0,0);

	return m_ItemList[nIndex]->m_Rect;
}

void CLTGUIHeaderCtrl::SetBasePos( const LTVector2n& pos )
{
	CLTGUICtrl::SetBasePos( pos );
	RecalcLayout();
}

void CLTGUIHeaderCtrl::SetSize( const LTVector2n& sz )
{
	CLTGUICtrl::SetSize( sz );
	RecalcLayout();
}

void CLTGUIHeaderCtrl::SetScale(const LTVector2& vfScale)
{
	bool bRebuild = (vfScale != m_vfScale);

	CLTGUICtrl::SetScale(vfScale);

	if( bRebuild )
	{
		m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

		uint32 nHeaderCount = (uint32)m_ItemList.size();
		for(uint32 nHeader=0;nHeader<nHeaderCount;++nHeader)
		{
			m_ItemList[nHeader]->m_Text.SetFontHeight( (uint32)(m_vfScale.y * (float)m_nBaseFontSize) );
		}

		RecalcLayout();
	}
}

// scales all items to fit in the available area of the header control
void CLTGUIHeaderCtrl::Rescale()
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
int32 CLTGUIHeaderCtrl::GetItemUnderCursor(int x, int y)
{
	int32 nItemCount = (int32)GetItemCount();
	if( nItemCount == 0 )
		return s_kInvalidItemIndex;

	LTVector2n ptCursor( x, y );

	for(int32 nItem=0;nItem<nItemCount;++nItem)
	{
		SHeaderItem* pItem = m_ItemList[nItem];
		if( !pItem )
			continue;

		if( pItem->m_Rect.Contains(ptCursor) )
			return nItem;
	}
	
	return s_kInvalidItemIndex;
}

// determines if the cursor is under a divider
int32 CLTGUIHeaderCtrl::IsCursorUnderDivider(int x, int y)
{
	int32 nItemCount = (int32)GetItemCount();
	if( nItemCount <= 1 )
		return s_kInvalidItemIndex;

	LTVector2n ptCursor( x, y );

	int32 nDividerWidth = (int32)(m_nDividerWidth * m_vfScale.x);

	for(int32 nItem=0;nItem<nItemCount - 1;++nItem)
	{
		SHeaderItem* pItem = m_ItemList[nItem];
		if( !pItem )
			continue;

		LTRect2n rcDivider = pItem->m_Rect;
		rcDivider.Left() = rcDivider.Right() - (nDividerWidth/2);
		rcDivider.Right() = rcDivider.Left() + nDividerWidth;

		if( rcDivider.Contains(ptCursor) )
			return nItem;
	}

	return s_kInvalidItemIndex;
}

// sets the width of an item
void CLTGUIHeaderCtrl::SetItemWidth( int32 nItem, int32 nWidth )
{
	if( nItem == s_kInvalidItemIndex || nItem >= (int32)GetItemCount() )
		return;

	nWidth = LTMIN( GetItemMaximumWidth(nItem), LTMAX( nWidth, GetBaseHeight() ));
	m_ItemList[nItem]->m_nWidth = nWidth;

	FixItemWidths( nItem );

	if( m_pMessageControl )
		m_pMessageControl->SendCommand( eHeaderCtrlCmd_Size, nItem, nWidth );

	RecalcLayout();
}

// returns the maximum width of an item
uint32 CLTGUIHeaderCtrl::GetItemMaximumWidth( uint32 nThisItem )
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
		if( !m_ItemList[nItem]->m_bSizeable )
			nWidthReserved += m_ItemList[nItem]->m_nWidth;
		else
			nWidthReserved += nWidthMin;
	}

	return nTotalWidth - nWidthTaken - nWidthReserved;
}

// fix the widths of all items after this one
void CLTGUIHeaderCtrl::FixItemWidths( uint32 nThisItem )
{
	uint32 nTotalWidth = GetBaseWidth();

	uint32 nWidthMin = GetBaseHeight();

	uint32 nItemCount = GetItemCount();
	for(uint32 nItem=nItemCount-1;nItem>nThisItem;--nItem)
	{
		uint32 nItemMinWidth = m_ItemList[nItem]->m_bSizeable?nWidthMin:m_ItemList[nItem]->m_nWidth;
		uint32 nItemPosition = GetItemPosition(nItem);
		if( nItemPosition + nItemMinWidth > nTotalWidth )
		{
			m_ItemList[nItem]->m_nWidth = nItemMinWidth;
			nTotalWidth -= nItemMinWidth;
		}
		else if( m_ItemList[nItem]->m_bSizeable )
		{
			m_ItemList[nItem]->m_nWidth = nTotalWidth - nItemPosition;
			break;
		}
		else
		{
			nTotalWidth -= nItemMinWidth;
		}
	}
}

// called when the mouse is over the control
bool CLTGUIHeaderCtrl::OnMouseMove(int x, int y)
{
	int32 nOldActiveItem = m_nActiveItem;

	m_nActiveItem = GetItemUnderCursor(x, y);
	int32 nDivider = IsCursorUnderDivider( x, y );
	if( nDivider != s_kInvalidItemIndex )
		m_nActiveItem = s_kInvalidItemIndex;

	switch( m_CaptureState )
	{
	case eCapture_Divider:
		{
			m_nActiveItem = s_kInvalidItemIndex;

			if( m_pInputCaptureHandler )
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCursor, (uint32)"SplitVert", 0 );

			if( m_nDragItem == s_kInvalidItemIndex || m_nDragItem >= (int32)GetItemCount() )
				break;

			SHeaderItem* pItem = m_ItemList[m_nDragItem];
			if( !pItem )
				break;

			int32 nWidth = (int32)(((x - pItem->m_Rect.Left()) - m_ptDragOffset.x) / m_vfScale.x);
			SetItemWidth( m_nDragItem, nWidth );
		}
		break;

	default:
		{
			if( nDivider != s_kInvalidItemIndex )
			{
				if( m_ItemList[nDivider]->m_bSizeable && m_pInputCaptureHandler )
					m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCursor, (uint32)"SplitVert", 0 );
			}
		}
		break;
	}

	if( nOldActiveItem != m_nActiveItem && m_pInputCaptureHandler )
	{
		if (m_nActiveItem != s_kInvalidItemIndex)
		{
			m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_CHANGE, 0 );
		}
		m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_UpdateHelp, 0, 0 );
	}

	return false;
}

// called when the left mouse button is pressed over the scroll control
bool CLTGUIHeaderCtrl::OnLButtonDown(int x, int y)
{
	if( m_pInputCaptureHandler )
	{
		int32 nDivider = IsCursorUnderDivider( x, y );
		if( nDivider != s_kInvalidItemIndex )
		{
			SHeaderItem* pItem = m_ItemList[nDivider];
			if( pItem && pItem->m_bSizeable )
			{
				m_ptDragOffset.x = x - pItem->m_Rect.Right();
				m_ptDragOffset.y = y - pItem->m_Rect.Top();

				m_nDragItem = nDivider;

				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_SetCapture, (uint32)this, true );
				m_CaptureState = eCapture_Divider;
			}
		}
	}

	if( m_CaptureState == eCapture_None )
	{
		m_nActiveItem = GetItemUnderCursor(x, y);
		if( m_nActiveItem != s_kInvalidItemIndex )
		{
			if( m_pMessageControl )
				m_pMessageControl->SendCommand( eHeaderCtrlCmd_Click, m_ItemList[m_nActiveItem]->m_idHeader, m_nActiveItem );
			if( m_pInputCaptureHandler )
				m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_PlaySound, IS_SELECT, 0 );
			if (m_pCommandHandler)
			{
				m_pCommandHandler->SendCommand( m_ItemList[m_nActiveItem]->m_idHeader, 0, 0 );
			}
		}
	}

	return false;
}

// called when the left mouse button is released over the scroll control
bool CLTGUIHeaderCtrl::OnLButtonUp(int x, int y)
{
	if( m_CaptureState != eCapture_None )
	{
		m_pInputCaptureHandler->SendCommand( eGUICtrlCmd_ReleaseCapture, 0, 0 );
		m_CaptureState = eCapture_None;
	}

	return false;
}

// set the frame width
void CLTGUIHeaderCtrl::SetFrameWidth( uint8 nFrameWidth )
{
	m_nFrameWidth = nFrameWidth;
	RecalcLayout();
}

// returns the help ID for the item
const char* CLTGUIHeaderCtrl::GetHelpID()
{
	if( m_nActiveItem != -1 && m_nActiveItem < (int32)m_ItemList.size() )
		return m_ItemList[m_nActiveItem]->m_szHelpId;

	return "";
}