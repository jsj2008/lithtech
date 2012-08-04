// ----------------------------------------------------------------------- //
//
// MODULE  : HUDItem.h
//
// PURPOSE : Implementation of CHUDItem base class
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "HUDItem.h"
#include "CommonUtilities.h"
#include "ClientDB.h"

//******************************************************************************************

CHUDItem::CHUDItem() :
	m_UpdateFlags(0),
	m_eLevel(kHUDRenderFull),
	m_fCurrentFade(1.0f),
	m_fLastFade(1.0f),
	m_fFadeStartTime(0.0f),
	m_bFadeChanged(false),
	m_bFadeEnabled(false),
	m_bSinglePlayerFade(true),
	m_bMultiplayerFade(true),
	m_hLayout(NULL),
	m_vBasePos(0,0),
	m_vTextOffset(0,0),
	m_cTextColor(argbWhite),
	m_eTextAlignment(kLeft),
	m_vIconOffset(0,0),
	m_vIconSize(32,32),
	m_cIconColor(argbWhite),
	m_fHoldTime(2.0f),
	m_fFadeTime(3.0f),
	m_eHUDRenderLayer(eHUDRenderLayer_Back),
	m_bUseBasePosFromLayout(true),
	m_hSourceString(NULL)
{
	
}

void CHUDItem::Term()
{
	if (m_hSourceString)
	{
		g_pTextureString->ReleaseTextureString(m_hSourceString);
		m_hSourceString = NULL;
	}
}


void CHUDItem::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}


//called before during update loop to update the fade
void CHUDItem::UpdateFade()
{
	m_bFadeChanged = (m_fLastFade != m_fCurrentFade);
	m_fLastFade = m_fCurrentFade;
	bool bFade = m_bFadeEnabled;
	if (IsMultiplayerGameClient())
	{
		bFade = m_bFadeEnabled && m_bMultiplayerFade;
	}
	else
	{
		bFade = m_bFadeEnabled && m_bSinglePlayerFade;
	}
	if (bFade)
	{
		float fElapsedTime =  (float)((RealTimeTimer::Instance().GetTimerAccumulatedS( ) - m_fFadeStartTime));
		fElapsedTime *= GetFadeSpeed();
		if (fElapsedTime > m_fHoldTime)
		{
			fElapsedTime -= m_fHoldTime;
			if (m_fFadeTime == 0.0f || m_fFadeTime <= fElapsedTime)
				m_fCurrentFade = 0.0f;
			else
				m_fCurrentFade = (m_fFadeTime - fElapsedTime) / m_fFadeTime;

		}
	}
	else
	{
		m_fCurrentFade = 1.0f;
	}

	if (FadeLevelChanged())
	{
		float fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_Text.SetColor(FadeARGB(m_cTextColor,fFade));
			DrawPrimSetRGBA(m_IconPoly,FadeARGB(m_cIconColor,fFade));
		}
		else
		{
			m_Text.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		}
	}



}
//called before during update loop to update the fade
void CHUDItem::StartFlicker()
{
	m_fFlickerFreq = GetRandom(5.0f,15.0f);
}

void CHUDItem::EndFlicker()
{
	float fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		m_Text.SetColor(FadeARGB(m_cTextColor,fFade));
		DrawPrimSetRGBA(m_IconPoly,FadeARGB(m_cIconColor,fFade));
	}
	else
	{
		m_Text.SetColor(m_cTextColor);
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
	}
}

void CHUDItem::UpdateFlicker()
{
	float fFlickerLevel = g_pHUDMgr->GetFlickerLevel();
	float fFadeLevel = GetFadeLevel();
	if (fFlickerLevel > 0.0f)
	{
		if (fFadeLevel > 0.25f)
		{
			float fRange = fFadeLevel - 0.25f;
			m_fFlicker = fFadeLevel - (fRange * fFlickerLevel * GetSinCycle(m_fFlickerFreq));
		}
		else
		{
			float fRange = 0.25f - fFadeLevel;
			m_fFlicker = fFadeLevel + (fRange * fFlickerLevel * GetSinCycle(m_fFlickerFreq));
		}
		if (m_fFlicker < 1.0f)
		{
			m_Text.SetColor(FadeARGB(m_cTextColor,m_fFlicker));
			DrawPrimSetRGBA(m_IconPoly,FadeARGB(m_cIconColor,m_fFlicker));
		}
		else
		{
			m_Text.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		}
	}
}

//called to restart the fading process from the beginning
void CHUDItem::ResetFade()
{
	if (m_bFadeEnabled)
	{
		m_fLastFade = 0.0f;
	}
	else
	{
		m_fLastFade = 1.0f;
	}
	
	m_fCurrentFade = 1.0f;
	m_fFadeStartTime = RealTimeTimer::Instance().GetTimerAccumulatedS( );
}


//this represents default behavior, but may be overridden by individual controls
float CHUDItem::GetFadeSpeed() const
{
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile->m_bPersistentHUD)
		return 0.0f;

	return pProfile->m_fHUDFadeSpeed;
}

void CHUDItem::ScaleChanged()
{
	float x = float(m_vBasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = float(m_vBasePos.y) * g_pInterfaceResMgr->GetYRatio();

	DrawPrimSetXYWH(m_IconPoly,x+float(m_vIconOffset.x),y+float(m_vIconOffset.y),float(m_vIconSize.x),float(m_vIconSize.y));

	LTVector2 vPos;
	vPos.x = x + float(m_vTextOffset.x);
	vPos.y = y + float(m_vTextOffset.y);
	m_Text.SetPos(vPos);
}


void CHUDItem::UpdateLayout()
{
//	LTASSERT(m_hLayout,"No layout record specified.");
	if (m_hLayout)
	{
		if( m_bUseBasePosFromLayout )
			m_vBasePos = g_pLayoutDB->GetPosition(m_hLayout,"BasePos");

		const char* pTmp = NULL;

		pTmp = g_pLayoutDB->GetString(m_hLayout,"HorizontalAlignment");
		     if (LTStrICmp(pTmp,"Left")   == 0)		m_eHorizAlign = kHUDHAlignLeft;
		else if (LTStrICmp(pTmp,"Center") == 0)		m_eHorizAlign = kHUDHAlignCenter;
		else if (LTStrICmp(pTmp,"Right")  == 0)		m_eHorizAlign = kHUDHAlignRight;
		else if (LTStrICmp(pTmp,"None")   == 0)		m_eHorizAlign = kHUDHNoAlign;

		pTmp = g_pLayoutDB->GetString(m_hLayout,"VerticalAlignment");
		     if (LTStrICmp(pTmp,"Top")    == 0)		m_eVertAlign = kHUDVAlignTop;
		else if (LTStrICmp(pTmp,"Center") == 0)		m_eVertAlign = kHUDVAlignCenter;
		else if (LTStrICmp(pTmp,"Bottom") == 0)		m_eVertAlign = kHUDVAlignBottom;
		else if (LTStrICmp(pTmp,"None")   == 0)		m_eVertAlign = kHUDVNoAlign;

		m_vTextOffset = g_pLayoutDB->GetPosition(m_hLayout,"TextOffset");
		std::string sFont = g_pLayoutDB->GetFont(m_hLayout,"Font");
		if (sFont.empty())
			sFont = g_pLayoutDB->GetHUDFont();
		uint32 nTextSize = g_pLayoutDB->GetInt32(m_hLayout,"TextSize");
		m_sTextFont = CFontInfo(sFont.c_str(),nTextSize);
		
		m_cTextColor = g_pLayoutDB->GetColor(m_hLayout,"TextColor");

		pTmp = g_pLayoutDB->GetString(m_hLayout,"TextAlignment");
		if (LTStrICmp(pTmp,"Left") == 0)
		{
			m_eTextAlignment = kLeft;
		}
		else if (LTStrICmp(pTmp,"Center") == 0)
		{
			m_eTextAlignment = kCenter;
		}
		else if (LTStrICmp(pTmp,"Right") == 0)
		{
			m_eTextAlignment = kRight;
		}

		
		m_vIconOffset = g_pLayoutDB->GetPosition(m_hLayout,"IconOffset");
		m_vIconSize = g_pLayoutDB->GetPosition(m_hLayout,"IconSize");
		m_cIconColor = g_pLayoutDB->GetColor(m_hLayout,"IconColor");
		m_hIconTexture.Load( g_pLayoutDB->GetString(m_hLayout,"IconTexture") );
		m_fHoldTime = g_pLayoutDB->GetFloat(m_hLayout,"FadeDelay");
		m_fFadeTime = g_pLayoutDB->GetFloat(m_hLayout,"FadeTime");

		m_Text.SetFont(m_sTextFont);
		m_Text.SetColor(m_cTextColor);
		m_Text.SetAlignment(m_eTextAlignment);

		//called to position text etc.
		CHUDItem::ScaleChanged();

		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		if (m_hIconTexture)
		{
			SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f,0.0f,1.0f,1.0f);
		}

		m_bMultiplayerFade = g_pLayoutDB->GetBool(m_hLayout,LDB_bHUDMultiplayerFade);
		m_bSinglePlayerFade = g_pLayoutDB->GetBool(m_hLayout,LDB_bHUDSinglePlayerFade);

	}

}

//******************************************************************************************
// Layout functions to help align hud items to the screen borders

LTVector2n CHUDItem::AlignBasePosition( LTVector2n vHudItemSize )
{
	LTVector2 ScreenScale = g_pInterfaceResMgr->GetScreenScale();
	LTRect2f ScreenBounds = g_pInterfaceMgr->GetViewportRect();
	LTVector2 ScreenRes(( float )g_pInterfaceResMgr->GetScreenWidth(), ( float )g_pInterfaceResMgr->GetScreenHeight() );

	// convert bounds from normalized to screen res coordinates.
	ScreenBounds.m_vMin *= ScreenRes;
	ScreenBounds.m_vMax *= ScreenRes;

	// start with the base position
	LTVector2n Result = m_vBasePos;

	// for items aligned to screen borders, scale the spacing by the current resolution
	if( m_eHorizAlign != kHUDHNoAlign )		Result.x = int32( Result.x * ScreenScale.x );
	if( m_eVertAlign  != kHUDVNoAlign )		Result.y = int32( Result.y * ScreenScale.y );

	// center support
	if( m_eHorizAlign == kHUDHAlignCenter )	Result.x += ( int32 )((ScreenBounds.m_vMax.x - vHudItemSize.x) / 2.0f );
	if( m_eVertAlign  == kHUDVAlignCenter )	Result.y += ( int32 )((ScreenBounds.m_vMax.y - vHudItemSize.y) / 2.0f );

	// push top / left justified items into the screen bounds
	if( m_eHorizAlign == kHUDHAlignLeft )	Result.x +=  ( int32 )ScreenBounds.m_vMin.x;
	if( m_eVertAlign  == kHUDVAlignTop )	Result.y +=  ( int32 )ScreenBounds.m_vMin.y;

	// make sure bottom / right justified items stay within screen bounds
	if( m_eHorizAlign == kHUDHAlignRight  )	Result.x +=  ( int32 )ScreenBounds.m_vMax.x - vHudItemSize.x;
	if( m_eVertAlign  == kHUDVAlignBottom )	Result.y +=  ( int32 )ScreenBounds.m_vMax.y - vHudItemSize.y;

#ifdef PLATFORM_XENON
	Result = MakeTitleSafe( Result, vHudItemSize, m_eHorizAlign, m_eVertAlign );
#endif

	return Result;
}

// ****************************************************************************************** //
// Convenience function to pull additional texture data from the game database

void CHUDItem::InitAdditionalTextureData( HRECORD hLayout,
										  uint32 nValueIndex,
										  TextureReference& hTexture,
										  LTVector2n& vPos,
										  LTVector2n& vSize,
										  LTPoly_GT4& poly )
{
	HATTRIBUTE hTextureStruct = g_pLayoutDB->GetAttribute(hLayout, "TextureDataList");
	if( hTextureStruct )
	{
		// get the texture filename
		HATTRIBUTE hFilename = g_pLayoutDB->GetStructAttribute(hTextureStruct,nValueIndex,"TextureFile");
		if( hFilename )
		{
			hTexture.Load( g_pLayoutDB->GetString( hFilename ));

			DrawPrimSetRGBA( poly, argbWhite );
			SetupQuadUVs( poly,  hTexture,  0.0f, 0.0f, 1.0f, 1.0f );
		}

		// get the texture position and size
		HATTRIBUTE hRectangle = g_pLayoutDB->GetStructAttribute(hTextureStruct,nValueIndex,"ImageRect");
		if( hRectangle )
		{
			LTVector4 v = g_pLayoutDB->GetVector4(hRectangle);
			vPos.x = int32(v.x);
			vPos.y = int32(v.y);
			vSize.x = int32(v.z);
			vSize.y = int32(v.w);
		}
	}
}

void CHUDItem::Flash(const char* pszFlash)
{
	if (!m_FlashTimer.GetEngineTimer().IsValid())
	{
		m_FlashTimer.SetEngineTimer(RealTimeTimer::Instance());
	}

	ClientDB& clientDB = ClientDB::Instance();
	HRECORD hFlash = clientDB.GetHUDFlashRecord(pszFlash);
	if (hFlash)
	{
		m_cFlashColor = ( uint32 )clientDB.GetInt32(hFlash,CDB_cFlashColor, 0, 0xFFFFFFFF);

		float fDuration = clientDB.GetFloat(hFlash,CDB_fFlashDuration);
		float fRate = LTMAX(1.0f,clientDB.GetFloat(hFlash,CDB_fFlashRate));
		m_nFlashCount = uint32(fDuration * fRate);

		float fTime = (fDuration / fRate);
		if (!m_FlashTimer.IsStarted() || m_FlashTimer.GetTimeLeft() > fDuration / fRate)
		{
			m_FlashTimer.Start(fDuration / fRate);
		}
	

	}
}

void CHUDItem::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	//current flash timed out
	if (m_FlashTimer.IsTimedOut())
	{

		if (m_nFlashCount > 0)
		{
			//still flashing, restart timer
			m_FlashTimer.Start(m_FlashTimer.GetDuration());
			--m_nFlashCount;
		}
		else
		{
			//done flashing
			m_FlashTimer.Stop();
			ResetFade();
		}

	}

	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		DrawPrimSetRGBA(m_IconPoly,m_cFlashColor);
		m_Text.SetColor(m_cFlashColor);
	}
	else
	{
		DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		m_Text.SetColor(m_cTextColor);
	}
}

void CHUDItem::EndFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	m_nFlashCount = 0;
	m_FlashTimer.Stop();

}


bool CHUDItem::IsFlashing() const
{
	return m_FlashTimer.IsStarted() && (m_nFlashCount > 0);
}

void CHUDItem::SetSourceString(const wchar_t* pszChars)
{
	if (m_hSourceString)
	{
		LTRESULT res = g_pTextureString->RecreateTextureString(m_hSourceString,pszChars,m_sTextFont);
		if (res != LT_OK)
		{
			g_pTextureString->ReleaseTextureString(m_hSourceString);
			m_hSourceString = NULL;
		}
	}
	else
	{
		m_hSourceString = g_pTextureString->CreateTextureString(pszChars,m_sTextFont);
	}
	m_Text.SetSourceString(m_hSourceString);

}

void CHUDItem::Reset()
{
	ResetFade();
	EndFlicker();
	EndFlash();
}
