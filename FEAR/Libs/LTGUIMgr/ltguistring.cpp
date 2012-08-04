// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIString.cpp
//
// PURPOSE : Class to encapsulate a wstring and a HTEXTURESTRING
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguistring.h"
#include "sys/win/mpstrconv.h"

#if defined( PLATFORM_WIN32 )
#include <windows.h>	// For GetTickCount()
#endif

//get a value from [0..1] based on the given frequency
static float GetSinCycle(float fFrequency)
{
	static float fRadiansPerTick = MATH_TWOPI / 1000.0f;
	float fRadians = (float)GetTickCount() * fFrequency * fRadiansPerTick;
	return (1.0f + sinf(fRadians)) / 2.0f;
}

static float GetRandom(float min, float max)
{
	float randNum = (float)rand() / RAND_MAX;
	float num = min + (max - min) * randNum;
	return num;
}


//the interface to the client
extern ILTClient*			g_pLTClient;

CLTGUIString::CLTGUIString() :
	m_hString(NULL),
	m_hSourceString(NULL),
	m_nWidth(0),
	m_Alignment(kLeft),
	m_nDropShadow(0),
	m_nColor(),
	m_nShadowColor(),
	m_bGlow(false),
	m_bGlowEnabled(true),
	m_fGlowAlpha(0.25f),
	m_vGlowSize(1.2f,1.2f),
	m_fTransMod(-1.0f),
	m_bEllipsis(false)
{	
	m_vPos.Init();
}

void CLTGUIString::SetFont(const CFontInfo& Font)
{
	//invalid font info
	if (!Font.m_nHeight)
		return;

	m_Font = Font;

	if (m_hSourceString)
	{
		m_hSourceString = NULL;
	}

	//if we already have a texture, rebuild it
	if (IsValid())
		CreateTexture();
}

void CLTGUIString::SetSourceString(HTEXTURESTRING hSrcString)
{
	//invalid font info
	if (!hSrcString)
		return;

	m_Font.m_nHeight = 0;
	m_Font.m_szTypeface[0] = '\0';

	m_hSourceString = hSrcString;

	//if we already have a texture, rebuild it
	if (IsValid())
		CreateTexture();
}



void CLTGUIString::SetText(const wchar_t *pText, bool bEllipsis)
{
	if( m_bEllipsis )
		FlushTexture();

	m_bEllipsis = bEllipsis;

	//if we are given text use it
	if ( !pText )
	{
		FlushTexture();
		m_sString.erase();
		return;
	}

	// Ignore if it's the same string.
	if( LTStrEquals( m_sString.c_str( ), pText ))
		return;

	m_sString = pText;

	//if we already have a texture, rebuild it
	if (IsValid())
		CreateTexture();
}




// this will set the string's word wrap width, pass in 0 to disable word wrap
//	(defaults to off)
void CLTGUIString::WordWrap(uint32 nWidth)
{
	if (nWidth <= m_Font.m_nHeight)
		nWidth = 0;

	m_nWidth = nWidth;

	if (IsValid())
		g_pTextureString->WordWrapString(m_hString, nWidth);

}

void CLTGUIString::FlushTexture()
{
	if( m_bEllipsis && m_hSourceString )
	{
		g_pTextureString->ReleaseTextureString(m_hSourceString);
		m_hSourceString = NULL;
	}

	if (m_hString)
	{
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("LTGUIDebug");
		if(hVar)
		{
			float fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
			if (fVal > 0.0f)
			{
				g_pLTClient->CPrint("CLTGUIString flushing %s",MPW2A(m_sString.c_str()).c_str());
			}

		}

		g_pTextureString->ReleaseTextureString(m_hString);
		m_hString = NULL;
	}
}

void CLTGUIString::CreateTexture()
{
	if (IsEmpty())
		return;

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("LTGUIDebug");
	if(hVar)
	{
		float fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
		if (fVal > 0.0f)
		{
			if (m_hString)
			{
				g_pLTClient->CPrint("CLTGUIString re-creating with %s",MPW2A(m_sString.c_str()).c_str());
			}
			else
			{
				g_pLTClient->CPrint("CLTGUIString creating %s",MPW2A(m_sString.c_str()).c_str());
			}
		}

	}

	if (m_hString)
	{
		if (m_hSourceString)
		{
			if( m_bEllipsis )
			{
				RecalcEllipsis( m_rnLastClip );
			}
			//if we have a source string, then we don't need to create our own string...
			// with the exception of cases where we need to word wrap the string.
			else if (!m_nWidth && m_Alignment == kLeft)
			{
				FlushTexture();
			}
			else
			{
				LTRESULT res = g_pTextureString->RecreateTextureSubstring(m_hString,m_sString.c_str(),m_hSourceString);
				if (res != LT_OK)
				{
					FlushTexture();
				}

			}
		}
		else
		{
			LTRESULT res = g_pTextureString->RecreateTextureString(m_hString,m_sString.c_str(),m_Font);
			if (res != LT_OK)
			{
				FlushTexture();
			}
		}
	}
	else
	{
		if( m_bEllipsis )
		{
			m_hSourceString = g_pTextureString->CreateTextureString((m_sString + L".").c_str(),m_Font);
			m_hString = g_pTextureString->CreateTextureSubstring(m_sString.c_str(),m_hSourceString);
			RecalcEllipsis( m_rnLastClip );
		}
		else
		{
			if (m_hSourceString)
			{
				//if we have a source string, then we don't need to create our own string...
				// with the exception of cases where we need to word wrap the string.
				if (m_nWidth || m_Alignment != kLeft)
					m_hString = g_pTextureString->CreateTextureSubstring(m_sString.c_str(),m_hSourceString);
			}
			else
			{
				m_hString = g_pTextureString->CreateTextureString(m_sString.c_str(),m_Font);
			}
		}
	}



	if (m_nWidth && m_hString)
	{
		g_pTextureString->WordWrapString(m_hString, m_nWidth);
	}
}

LTRESULT CLTGUIString::Render()
{
	if (IsEmpty())
		return LT_OK;

	if (!IsValid())
		CreateTexture();

	LTVector2 vAnchorScale(0.0f,0.0f);

	//regular render, so reset our modifier
	m_fTransMod = -1.0f;

	switch(m_Alignment) 
	{
	case kCenter:
		vAnchorScale.x = 0.5f;
		break;
	case kRight:
		vAnchorScale.x = 1.0f;
		break;
	}
	g_pTextureString->SetupTextRendering(g_pDrawPrim);

	if (m_nDropShadow)
	{
		LTVector2 vDrop = m_vPos;
		vDrop += (float)m_nDropShadow;
		RenderString(vDrop, m_nShadowColor, vAnchorScale );
	}
	if (m_bGlow && m_bGlowEnabled)
	{
		LTVector2 vScale = m_vGlowSize * 1.1f;
		float fAlpha = m_fGlowAlpha * (0.8f + GetSinCycle(5.0f) * GetSinCycle(11.0f) * 0.4f);
		RenderString( m_vPos, FadeARGB(m_nColor,fAlpha), vAnchorScale,LTVector2(1.0f,0.0f), LTVector2(0.0f,1.0f), vScale );

		vScale = m_vGlowSize;
		fAlpha = m_fGlowAlpha * (0.5f + GetSinCycle(9.0f) * GetSinCycle(13.0f));
		RenderString( m_vPos, FadeARGB(m_nColor,fAlpha), vAnchorScale,LTVector2(1.0f,0.0f), LTVector2(0.0f,1.0f), vScale );
	}

	return RenderString( m_vPos, m_nColor, vAnchorScale );
}

LTRESULT CLTGUIString::RenderClipped( const LTRect2n& rClipRect )
{
	if (IsEmpty())
		return LT_OK;
	if (!IsValid())
		CreateTexture();

	LTVector2 vAnchorScale(0.0f,0.0f);

	//regular render, so reset our modifier
	m_fTransMod = -1.0f;

	switch(m_Alignment) 
	{
	case kCenter:
		vAnchorScale.x = 0.5f;
		break;
	case kRight:
		vAnchorScale.x = 1.0f;
		break;
	}

	LTRESULT res = LT_OK;
	if (m_nDropShadow)
	{
		LTVector2 vDrop = m_vPos;
		vDrop += (float)m_nDropShadow;
		res = RenderStringClipped(rClipRect, vDrop, m_nShadowColor, vAnchorScale );
	}
	if (m_bGlow)
	{
	}

	if (res == LT_OK)
	{
		res = RenderStringClipped(rClipRect, m_vPos, m_nColor, vAnchorScale );
	}
	

	 return res;
}



LTRESULT CLTGUIString::GetExtents(LTRect2n& rExtents) const
{ 
	return g_pTextureString->GetStringExtents(m_hString,rExtents); 
}

//this will determine the rectangle a particular character is being rendered to in screen space
LTRESULT CLTGUIString::GetCharRect(uint32 nCharIndex, LTRect2n& rRect) const
{
	LTRect2n rPlacementRect;
	LTRect2n rBlackBox;
	LTVector2 vUVPos;
	LTVector2 vUVDims;
	LTRESULT res = g_pTextureString->GetCharRect(m_hString,nCharIndex,rPlacementRect,rBlackBox,vUVPos,vUVDims);
	
	if (res == LT_OK)
	{
		rRect = rPlacementRect;
		rRect.Offset((int32)m_vPos.x,(int32)m_vPos.y);
	}

	return res;
}



void CLTGUIString::SetFontHeight(uint32 nFontHeight)
{
	if (m_hSourceString) return;

	m_Font.m_nHeight = nFontHeight;

	//if we already have a texture, rebuild it
	if (IsValid())
		CreateTexture();

}


void CLTGUIString::SetColor(uint32 nColor)
{
	m_nColor = nColor;
	//set drop shadow alpha
	uint32 a = GET_A(nColor);
	m_nShadowColor = SET_ARGB(a,0x00,0x00,0x00);

}

void CLTGUIString::SetAlignment(eTextAlign align) 
{
	m_Alignment = align;
}


void CLTGUIString::SetGlowParams(bool bEnable, float fAlpha /* = 0.25f */, const LTVector2& vSize /* = LTVector2 */)
{
	m_bGlowEnabled = bEnable;
	m_fGlowAlpha = fAlpha;
	m_vGlowSize = vSize;
}


void CLTGUIString::SetPos(LTVector2n vPos) 
{ 
	m_vPos.x = (float)vPos.x;
	m_vPos.y = (float)vPos.y;
}

void CLTGUIString::SetPos(LTVector2 vPos) 
{ 
	m_vPos.x = floor(vPos.x);
	m_vPos.y = floor(vPos.y);
}

bool CLTGUIString::IsValid() const 
{
	//if we have a string, we're valid
	if (m_hString != NULL)
		return true;

	//if we have a source string and are not word-wrapped we're valid...
	if (m_hSourceString && !m_nWidth && m_Alignment != kLeft)
		return true;

	//otherwise...
	return false;
}


LTRESULT CLTGUIString::RenderString(const LTVector2& vAnchor,
									uint32			nColor			/* = 0xFFFFFFFF            */,
									const LTVector2& vAnchorScale	/* = LTVector2(0.0f, 0.0f) */, 
									const LTVector2& vGround		/* = LTVector2(1.0f, 0.0f) */,
									const LTVector2& vDown			/* = LTVector2(0.0f, 1.0f) */,
									const LTVector2& vStretch		/* = LTVector2(1.0f, 1.0f) */)
{
	if (m_hString)
	{
		return g_pTextureString->RenderString( m_hString, g_pDrawPrim, vAnchor, nColor, vAnchorScale, vGround, vDown, vStretch );
	}
	else
	{
		return g_pTextureString->RenderSubString( m_hSourceString, m_sString.c_str(), g_pDrawPrim, vAnchor, nColor, vGround, vDown, vStretch );
	}

	return LT_ERROR;
}

LTRESULT CLTGUIString::RenderStringClipped(	const LTRect2n& rClipRect,
											const LTVector2& vAnchor, 
											uint32 nColor					/* = 0xFFFFFFFF */,
											const LTVector2& vAnchorScale	/* = LTVector2(0.0f, 0.0f) */)
{
	if( m_bEllipsis && (m_rnLastClip.m_vMax != rClipRect.m_vMax || m_rnLastClip.m_vMin != rClipRect.m_vMin) )
		RecalcEllipsis( rClipRect );

	m_rnLastClip = rClipRect;

	if (m_hString)
	{
		return g_pTextureString->RenderStringClipped(m_hString, g_pDrawPrim, rClipRect, m_vPos, m_nColor, vAnchorScale );
	}
	else if (m_hSourceString)
	{
		return g_pTextureString->RenderSubStringClipped(m_hSourceString, m_sString.c_str(), g_pDrawPrim, rClipRect, m_vPos, m_nColor );
	}
	return LT_ERROR;
}



LTRESULT CLTGUIString::RenderTransition(float fTrans)
{
	if (IsEmpty())
		return LT_OK;

	if (!IsValid())
		CreateTexture();

	//set a random modifier if we don't already have one
	if (m_fTransMod < 0.0f)
	{
		m_fTransMod = GetRandom(0.5f,1.5f);
	}
	fTrans = LTCLAMP(m_fTransMod*fTrans,0.0f,1.0f);
	

	LTVector2 vAnchorScale(0.0f,0.0f);

	switch(m_Alignment) 
	{
	case kLeft:
		vAnchorScale.x = 0.0f;//1.0f - fTrans;
		break;
	case kCenter:
		vAnchorScale.x = 0.5f;
		break;
	case kRight:
		vAnchorScale.x = 1.0f;//fTrans;
		break;
	}
	vAnchorScale.y = m_fTransMod-1.0f;
	g_pTextureString->SetupTextRendering(g_pDrawPrim);

	float fScale1 = 25.0f * (1.0f - fTrans);
	float fScale3 = fTrans * fTrans * fTrans;


	LTVector2 vGround(1.0f, 0.0f );
	LTVector2 vDown(0.0f, 1.0f);
	LTVector2 vStretch(fScale1,fScale3);
	return RenderString( m_vPos, FadeARGB(m_nColor,fScale3), vAnchorScale, vGround, vDown, vStretch );
}

LTRESULT CLTGUIString::RenderTransitionClipped( const LTRect2n& rClipRect, float fTrans )
{
	if (IsEmpty())
		return LT_OK;
	if (!IsValid())
		CreateTexture();
	//set a random modifier if we don't already have one
	if (m_fTransMod < 0.0f)
	{
		m_fTransMod = GetRandom(0.5f,1.5f);
	}
	fTrans = LTCLAMP(m_fTransMod*fTrans,0.0f,1.0f);
	float fScale1 = 25.0f * (1.0f - fTrans);
	float fScale3 = fTrans * fTrans * fTrans;

	LTVector2 vAnchorScale(0.0f,0.0f);

	switch(m_Alignment) 
	{
	case kCenter:
		vAnchorScale.x = 0.5f;
		break;
	case kRight:
		vAnchorScale.x = 1.0f;
		break;
	}

	LTRESULT res = LT_OK;
	res = RenderStringClipped(rClipRect, m_vPos, FadeARGB(m_nColor,fScale3), vAnchorScale );


	return res;
}

// recalculates the string based on the clipping rectangle
void CLTGUIString::RecalcEllipsis( const LTRect2n& rClipRect )
{
	if( !m_bEllipsis || !m_hString || !m_hSourceString )
		return;

	LTRect2n rPlacementRect;
	LTRect2n rBlackBox;
	LTVector2 vUVPos;
	LTVector2 vUVDims;

	uint32 nLength = LTStrLen( m_sString.c_str() );

	int32 nStringWidth = 0;
	for(uint32 nChar=0;nChar<nLength;++nChar)
	{
		g_pTextureString->GetCharRect( m_hSourceString, nChar, rPlacementRect, rBlackBox, vUVPos, vUVDims );
		nStringWidth += rPlacementRect.GetWidth();
	}

	if( nStringWidth >= rClipRect.GetWidth() )
	{
		// find out how large the dots are, the dots were appended to the end of the string
		g_pTextureString->GetCharRect( m_hSourceString, nLength, rPlacementRect, rBlackBox, vUVPos, vUVDims );
		int32 nDotWidth = rPlacementRect.GetWidth();
		int32 nTripleDotWidth = nDotWidth * 3;

		std::wstring wstrClipped;
		wstrClipped.reserve( nLength + 3 ); // worse case

		int32 nWidthLeft = rClipRect.GetWidth();
		nWidthLeft -= nTripleDotWidth;
		for(uint32 nChar=0;nChar<nLength;++nChar)
		{
			if( nWidthLeft < 0 )
				break;

			g_pTextureString->GetCharRect( m_hSourceString, nChar, rPlacementRect, rBlackBox, vUVPos, vUVDims );
			nWidthLeft -= rPlacementRect.GetWidth();
			if( nWidthLeft >= 0 )
				wstrClipped += m_sString[nChar];
		}

		wstrClipped += L"...";

		g_pTextureString->RecreateTextureSubstring(m_hString, wstrClipped.c_str(), m_hSourceString);
	}
	else
	{
		g_pTextureString->RecreateTextureSubstring(m_hString, m_sString.c_str(), m_hSourceString);
	}
}