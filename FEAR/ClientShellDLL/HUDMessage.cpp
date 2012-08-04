//-------------------------------------------------------------------------
//
// MODULE  : HUDMessage.cpp
//
// PURPOSE : Base class for HUD text display
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------


#include "stdafx.h"
#include "GameClientShell.h"
#include "HUDMessage.h"

namespace
{
	float kMinDuration = 1.0f;
	float kMinFade = 0.5f;
}


CHUDMessage::CHUDMessage()
{
	m_hImage = NULL;
	m_baseImageSize = LTVector2n(0,0);
	m_imageSize = LTVector2n(0,0);


	m_nBaseFontSize = 0;
	m_nTextColor = argbWhite;

	m_fInitTime = -1.0f;
	m_fDuration = 0.0f;
	m_fFadeDur  = 0.0f;

	m_fImageGap	= 4.0f;
	m_fAlpha = 1.0f;
	
	m_eJustify = kLeft;
	m_Text.SetText(L"");

	m_nHeaderColor = argbWhite;

	
}


bool CHUDMessage::Create(MsgCreate &mc)
{
	if (!mc.Font.m_nHeight) return false; 


	m_Text.SetFont(mc.Font);
	m_Text.SetText(mc.sString.c_str());
	m_Text.SetColor(mc.nTextColor);


	m_nBaseFontSize = mc.Font.m_nHeight;
	m_nTextColor = mc.nTextColor;
	m_nFixedWidth = mc.nWidth;

	m_Text.SetDropShadow(mc.bDropShadow);
	m_fInitTime = -1.0f;

	m_eJustify = mc.eJustify;
	m_Text.SetAlignment(mc.eJustify);

	m_fDuration = mc.fDuration * GetConsoleFloat("MessageDuration",1.0f);
	if (m_fDuration < kMinDuration)
		m_fDuration = kMinDuration;
	m_fFadeDur  = mc.fFadeDur * GetConsoleFloat("MessageDuration",1.0f);;
	if (m_fFadeDur < kMinFade)
		m_fFadeDur = kMinFade;

	m_hImage = mc.hImage;
	m_baseImageSize = mc.ptImageSize;
	m_imageSize = m_baseImageSize;


	m_Header.SetFont(mc.Font);
	m_Header.SetText(mc.sHeaderString.c_str());
	m_Header.SetColor(mc.nHeaderColor);
	m_Header.SetDropShadow(mc.bDropShadow);
	m_nHeaderColor = mc.nHeaderColor;

	InitPoly();

	m_Text.WordWrap(m_nFixedWidth);

	SetBasePos(LTVector2n(0,0));

	m_bVisible = true;

	return true;
}

void CHUDMessage::Destroy ( )
{
	CLTGUICtrl::Destroy();
	m_Text.FlushTexture();
	m_Header.FlushTexture();
}

void CHUDMessage::SetAlpha(float fAlpha)
{
	if (fAlpha < 0.0f) fAlpha = 0.0f;
	if (fAlpha > 1.0f) fAlpha = 1.0f;
	m_fAlpha = fAlpha;
	uint32 a,r,g,b;
	GET_ARGB(m_nTextColor,a,r,g,b);
	a = (uint32)(255.0f * fAlpha);
	m_nTextColor = SET_ARGB(a,r,g,b);

	m_Text.SetColor(m_nTextColor);

	ASSERT( a == ( uint8 )a );
	DrawPrimSetRGBA(m_Poly, 0xFF, 0xFF, 0xFF, (uint8)a);

	GET_ARGB(m_nHeaderColor,a,r,g,b);
	a = (uint32)(255.0f * fAlpha);
	m_nHeaderColor = SET_ARGB(a,r,g,b);

	m_Header.SetColor(m_nHeaderColor);

}


void CHUDMessage::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;


	if (m_fInitTime < 0.0f)
		m_fInitTime = RealTimeTimer::Instance().GetTimerAccumulatedS( );

	if (m_fDuration > 0.0f)
	{
		double fTime = GetLifetime();
		if (fTime > m_fDuration)
		{
			fTime -= m_fDuration;
			float fAlpha = (float)(1.0f - (fTime / m_fFadeDur));
			if (fAlpha <= 0.0f)
			{
				Show(false);
				return;
			}
			else
				SetAlpha(fAlpha);
		}
	}
	else
		Show(false);

}


void CHUDMessage::Render(bool bForceVisible)
{

	bool bVis = m_bVisible;
	float fAlpha = m_fAlpha;
	if (bForceVisible)
	{
		m_bVisible = true;
		SetAlpha(1.0f);
	}
	Render();
	if (bForceVisible)
	{
		m_bVisible = bVis;
		SetAlpha(fAlpha);
	}

}
void CHUDMessage::Render()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_hImage)
	{
		g_pDrawPrim->SetTexture(m_hImage);
		// set up the render state	
		SetRenderState();

		// draw our button
		g_pDrawPrim->DrawPrim(&m_Poly);
	}

	m_Text.Render();
	m_Header.Render();
}


double CHUDMessage::GetLifetime()
{
	if (m_fInitTime < 0.0f)
		return 0.0f;

	return RealTimeTimer::Instance().GetTimerAccumulatedS( ) - m_fInitTime;
}


void CHUDMessage::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePoly();

	LTRect2n rExt;
	LTRect2n rHeaderExt;

	if (!m_Text.IsEmpty())
	{
		if (!m_Text.IsValid())
		{
			m_Text.CreateTexture();
		}
		m_Text.GetExtents(rExt);
	}
	if (!m_Header.IsEmpty())
	{
		if (!m_Header.IsValid())
		{
			m_Header.CreateTexture();
		}
		m_Header.GetExtents(rHeaderExt);
	}

	

	LTVector2 headerPos = m_rfRect.m_vMin;
	LTVector2 textPos;
	//adjust for header
	textPos.x = headerPos.x + rHeaderExt.GetWidth() + 2;
	textPos.y = headerPos.y;
	if (m_hImage)
	{
		switch (m_eJustify)
		{
		case kLeft:
		case kCenter:
			{

				//adjust for image size
				textPos.x += (float)m_imageSize.x + m_fImageGap;
				textPos.y += (float)(m_imageSize.y - rExt.GetHeight()) / 2.0f;
			}
			break;
		case kRight:
			{
			}
			break;
		};
	}
	m_Text.SetPos(textPos);
	m_Header.SetPos(headerPos);

	uint32 nHeight = LTMAX(m_imageSize.y, rExt.GetHeight());
	uint32 nTotalWidth = (uint32)(g_pInterfaceResMgr->GetXRatio() * (rHeaderExt.GetWidth() + (float)m_nFixedWidth));
	SetSize(LTVector2n(nTotalWidth,nHeight));

	JustifyPoly();
}

void CHUDMessage::SetScale(const LTVector2& vfScale)
{
// do not use bas class's SetScale because we do not want to stretch the control vertically
	CLTGUICtrl::SetScale(LTVector2(1.0f,1.0f));
	m_rfRect.m_vMin.x = (vfScale.x * (float)m_rnBaseRect.m_vMin.x);
	m_rfRect.m_vMin.y = (vfScale.y * (float)m_rnBaseRect.m_vMin.y);
//	m_vfScale = vfScale;

	ScalePoly();
	m_fImageGap = 4.0f; // * m_vfScale.x;
	LTRect2n rHeaderExt;
	m_Header.GetExtents(rHeaderExt);

	uint32 nTextWidth = (uint32)(vfScale.x *  (float)m_nFixedWidth);
	uint32 nTotalWidth = nTextWidth + (uint32)(vfScale.x * (float)rHeaderExt.GetWidth());
	if (m_hImage)
	{
		nTextWidth -= (uint32)(m_fImageGap + (float)m_imageSize.x);
	}
	
	m_Text.WordWrap(nTextWidth);

	LTRect2n rExt;
	m_Text.GetExtents(rExt);

	LTVector2 headerPos = m_rfRect.m_vMin;
	LTVector2 textPos;
	//adjust for header
	textPos.x = headerPos.x + rHeaderExt.GetWidth() + 2;
	textPos.y = headerPos.y;

	if (m_hImage)
	{
		switch (m_eJustify)
		{
		case kLeft:
		case kCenter:
			{
				//adjust for image size
				textPos.x += (float)m_imageSize.x + m_fImageGap;
				textPos.y += (float)(m_imageSize.y - rExt.GetHeight()) / 2.0f;
			}
			break;
		case kRight:
			break;
		};
	}

	m_Header.SetPos(headerPos);
	m_Text.SetPos(textPos);

	uint32 nHeight = LTMAX(m_imageSize.y, rExt.GetHeight());
	SetSize(LTVector2n(nTotalWidth,nHeight));

	JustifyPoly();
	
}


void CHUDMessage::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}

void CHUDMessage::InitPoly()
{
	if (!m_hImage) return;
	ScalePoly();
	SetupQuadUVs(m_Poly, m_hImage, 0.0f,0.0f,1.0f,1.0f);
	DrawPrimSetRGBA(m_Poly,0xFF, 0xFF, 0xFF, 0xFF);
	JustifyPoly();
}

void CHUDMessage::ScalePoly()
{
	if (!m_hImage) return;

	//because getting the dimensions of a texture are no longer supported
	//we have to make some sort of assumption about the size of each image
	uint32 w = 0;
	uint32 h = 0;
	g_pILTTextureMgr->GetTextureDims(m_hImage,w,h);

	if (m_baseImageSize.x > 0)
	{
		w = m_baseImageSize.x;
		h = m_baseImageSize.y;
	}


	m_imageSize.x = (int)((float)w * m_vfScale.x);
	m_imageSize.y = (int)((float)h * m_vfScale.y);
}


void CHUDMessage::JustifyPoly()
{
	if (!m_hImage) return;

	float x = m_rfRect.Left();
	float y = m_rfRect.Top();
	float fw = (float)m_imageSize.x;
	float fh = (float)m_imageSize.y;

	LTRect2n rExt;
	m_Text.GetExtents(rExt);

	switch (m_eJustify)
	{
	case kCenter:
		//adjust for text size
		x -= ( rExt.GetWidth() + m_fImageGap ) / 2.0f;
		break;
	case kRight:
		//adjust for text size
		x -= ( rExt.GetWidth() + m_fImageGap );
		break;
	case kLeft:
		break;
	}

	DrawPrimSetXYWH(m_Poly,x,y,fw,fh);
}




void CHUDMessage::FlushTextureStrings()
{
	m_Text.FlushTexture();
	m_Header.FlushTexture();
}

void CHUDMessage::RecreateTextureStrings()
{
	m_Text.CreateTexture();
	m_Header.CreateTexture();
}