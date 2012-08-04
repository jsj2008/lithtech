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
#include "LayoutMgr.h"

namespace
{
	float kMinDuration = -1.0f;
	float kMinFade = -1.0f;
}


CHUDMessage::CHUDMessage()
{
	m_hImage = LTNULL;
	m_nBaseImageSize = 0;

	m_pText = LTNULL;

	m_pFont = LTNULL;
	m_nFontSize = 0;
	m_nBaseFontSize = 0;
	m_nTextColor = argbWhite;

	m_fInitTime = -1.0f;
	m_eJustify = kMsgLeft;
	m_fDuration = 0.0f;
	m_fFadeDur  = 0.0f;

	m_fImageGap	= 4.0f;
	m_fAlpha = 1.0f;

	m_bDropShadow = true;
}


LTBOOL CHUDMessage::Create(MsgCreate &mc)
{
	if (!mc.pFont) return LTFALSE; 

	if (kMinDuration < 0.0f)
		kMinDuration = g_pLayoutMgr->GetMessageMinTime();
	if (kMinFade < 0.0f)
		kMinFade = g_pLayoutMgr->GetMessageMinFade();


	m_pFont = mc.pFont;


	if (!m_pText)
	{
		m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont,"",0.0f,0.0f);
		if (!m_pText)
			return LTFALSE;
	}

	m_pText->SetText(mc.sString.c_str());


	m_nFontSize = mc.nFontSize;
	m_nBaseFontSize = mc.nFontSize;
	m_nTextColor = mc.nTextColor;
	m_nFixedWidth = mc.nWidth;

	m_bDropShadow = mc.bDropShadow;
	
	m_pText->SetColor(m_nTextColor);

	m_fInitTime = -1.0f;

	m_eJustify = mc.eJustify;
	m_fDuration = mc.fDuration * GetConsoleFloat("MessageDuration",1.0f);
	if (m_fDuration < kMinDuration)
		m_fDuration = kMinDuration;
	m_fFadeDur  = mc.fFadeDur * GetConsoleFloat("MessageDuration",1.0f);;
	if (m_fFadeDur < kMinFade)
		m_fFadeDur = kMinFade;

	m_hImage = mc.hImage;
	m_nBaseImageSize = mc.nImageSize;

	switch (m_eJustify)
	{
	case kMsgLeft:
		m_pText->SetAlignmentH(CUI_HALIGN_LEFT);
		break;
	case kMsgRight:
		m_pText->SetAlignmentH(CUI_HALIGN_RIGHT);
		break;
	case kMsgCenter:
		m_pText->SetAlignmentH(CUI_HALIGN_CENTER);
		break;
	};


	InitPoly();

	SetBasePos(LTIntPt(0,0));

	SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;

	

	return LTTRUE;
}

void CHUDMessage::Destroy ( )
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
	CLTGUICtrl::Destroy();

}

void CHUDMessage::SetAlpha(float fAlpha)
{
	if (fAlpha < 0.0f) fAlpha = 0.0f;
	if (fAlpha > 1.0f) fAlpha = 1.0f;
	m_fAlpha = fAlpha;
	uint32 a,r,g,b;
	GET_ARGB(m_nTextColor,a,r,g,b);
	a = (uint8)(255.0f * fAlpha);
	m_nTextColor = SET_ARGB(a,r,g,b);
	if (m_pText)
		m_pText->SetColor(m_nTextColor);



	uint32 c  = SET_ARGB(a,255,255,255);
	g_pDrawPrim->SetRGBA(&m_Poly,c);

}


void CHUDMessage::Update()
{
	// Sanity checks...
	if (!IsVisible()) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

	if (m_fInitTime < 0.0f)
		m_fInitTime = g_pLTClient->GetTime();

	if (m_fDuration > 0.0f)
	{
		float fTime = GetLifetime();
		if (fTime > m_fDuration)
		{
			fTime -= m_fDuration;
			float fAlpha = 1.0f - (fTime / m_fFadeDur);
			if (fAlpha <= 0.0f)
			{
				Show(LTFALSE);
				return;
			}
			else
				SetAlpha(fAlpha);
		}
	}
	else
		Show(LTFALSE);

}


void CHUDMessage::Render(LTBOOL bForceVisible)
{

	LTBOOL bVis = m_bVisible;
	float fAlpha = m_fAlpha;
	if (bForceVisible)
	{
		m_bVisible = LTTRUE;
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

	if (m_pText)
	{
		if (m_bDropShadow)
		{
			float px,py;
			m_pText->GetPosition(&px,&py);
			px += 2.0f;
			py += 2.0f;
			m_pText->SetColor(0xBF000000);
			m_pText->SetPosition(px,py);
			m_pText->Render();

			px -= 2.0f;
			py -= 2.0f;
			m_pText->SetColor(m_nTextColor);
			m_pText->SetPosition(px,py);
			
		}
		m_pText->Render();
		
	}
}


float CHUDMessage::GetLifetime()
{
	if (m_fInitTime < 0.0f)
		return 0.0f;

	return g_pLTClient->GetTime() - m_fInitTime;
}


void CHUDMessage::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePoly();
	if (m_pText)
	{
		float x = (float)m_pos.x;
		float y = (float)m_pos.y;

		if (m_hImage)
		{
			switch (m_eJustify)
			{
			case kMsgLeft:
			case kMsgCenter:
				//adjust for image size
				x += (float)m_imageSize.x + m_fImageGap;
				y += ((float)m_imageSize.y - m_pText->GetHeight()) / 2;
				break;
			case kMsgRight:
				break;
			};

		}
		m_pText->SetPosition(x,y);
	}
	JustifyPoly();


}

void CHUDMessage::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	ScalePoly();
	m_fImageGap = 4.0f * m_fScale;
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	if (m_pText)
	{
		float x = (float)m_pos.x;
		float y = (float)m_pos.y;
		m_pText->SetCharScreenHeight(m_nFontSize);
		if (m_hImage)
		{
			switch (m_eJustify)
			{
			case kMsgLeft:
			case kMsgCenter:
				//adjust for image size
				x += (float)m_imageSize.x + m_fImageGap;
				y += ((float)m_imageSize.y - m_pText->GetHeight()) / 2;
				break;
			case kMsgRight:
				break;
			};			
		}
		m_pText->SetPosition(x,y);

		uint16 nTextWidth = (uint16)(m_fScale * (float)m_nFixedWidth - ((float)m_imageSize.x + m_fImageGap));
		m_pText->SetWrapWidth(nTextWidth);

		m_nWidth = m_imageSize.x + (uint16)( m_fImageGap + m_pText->GetWidth());
		m_nHeight = Max((uint16)m_imageSize.y,(uint16)m_pText->GetHeight());

	}
	else
	{
		m_nWidth = m_imageSize.x;
		m_nHeight = m_imageSize.y;
	}
	JustifyPoly();

}


void CHUDMessage::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

void CHUDMessage::InitPoly()
{
	if (!m_hImage) return;
	ScalePoly();
	SetupQuadUVs(m_Poly, m_hImage, 0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly,0xFFFFFFFF);
	JustifyPoly();
}

void CHUDMessage::ScalePoly()
{
	if (!m_hImage) return;

	uint32 w,h;
	g_pTexInterface->GetTextureDims(m_hImage,w,h);

	if (m_nBaseImageSize > 0)
	{
		w = (uint32)( (float)w * (float)m_nBaseImageSize/ (float)h);
		h = m_nBaseImageSize;
	}


	m_imageSize.x = (int)((float)w * m_fScale);
	m_imageSize.y = (int)((float)h * m_fScale);

}


void CHUDMessage::JustifyPoly()
{
	if (!m_hImage) return;

	float x = (float)m_basePos.x * m_fScale;
	float y = (float)m_basePos.y * m_fScale;
	float fw = (float)m_imageSize.x;
	float fh = (float)m_imageSize.y;

	if (m_pText)
	{
		switch (m_eJustify)
		{
		case kMsgCenter:
			//adjust for text size
			x -= ( m_pText->GetWidth() + m_fImageGap ) / 2.0f;
			break;
		case kMsgRight:
			//adjust for text size
			x -= ( m_pText->GetWidth() + m_fImageGap );
			break;
		case kMsgLeft:
			break;
		};
	}

//	y += (m_pText->GetHeight() - (float)m_imageSize.y) / 2.0f;

	g_pDrawPrim->SetXYWH(&m_Poly,x,y,fw,fh);


}




