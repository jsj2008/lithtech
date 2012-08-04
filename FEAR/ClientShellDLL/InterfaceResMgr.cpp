// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceResMgr.cpp
//
// PURPOSE : Manager for resources associated with the interface
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "gameclientshell.h"
#include "InterfaceResMgr.h"
#include "iltrenderer.h"
#include "ilttexturestring.h"


CInterfaceResMgr*   g_pInterfaceResMgr = NULL;

namespace
{
	const uint8 kIndent = 8;
	const uint16 kBaseWidth = 320;
	const uint16 kBaseHeight = 200;
	const uint16 kBigWidth = 480;
	const uint16 kMinWidth = 160;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterfaceResMgr::CInterfaceResMgr()
{
	g_pInterfaceResMgr = this;

	m_dwScreenWidth = 0;
	m_dwScreenHeight = 0;

	m_vfScale.Init(1.0f,1.0f);
	m_bWidescreen = false;

	m_pMsgText = NULL;

}

CInterfaceResMgr::~CInterfaceResMgr()
{
    g_pInterfaceResMgr = NULL;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Init
// Description	    :
// Return type      : bool
//////////////////////////////////////////////////////////////////////
bool CInterfaceResMgr::Init()
{


	// Set the English flag
    if (LTStrICmp(L"english", LoadString("IDS_GAME_LANGUAGE")) != 0)
    {
        m_bEnglish=false;
	}
	else
    {
        m_bEnglish=true;
	}

	// Initialize the fonts
    if (!InitFonts())
	{
        return false;
	}

	// set resolution dependant variables
	ScreenDimsChanged();

	m_DlgFont = CFontInfo(g_pLayoutDB->GetDialogFontFace(),g_pLayoutDB->GetDialogFontSize());

	m_pMsgText = debug_new(CLTGUITextCtrl);
	CLTGUICtrl_create cs;
	cs.rnBaseRect.Right() = kBaseWidth-2*kIndent;
	cs.rnBaseRect.Bottom() = kBaseHeight-2*kIndent;

    if (!m_pMsgText->Create(L"", m_DlgFont, cs))
	{
		debug_delete(m_pMsgText);
		m_pMsgText = NULL;
        return false;
	}
	m_pMsgText->SetColor(argbWhite);
	m_pMsgText->SetWordWrap(true);
	m_pMsgText->Enable(false);

	cs.rnBaseRect.Right() = kBaseWidth;
	cs.rnBaseRect.Bottom() = kBaseHeight;

	TextureReference hFrame = g_pLayoutDB->GetDialogFrame();
	m_MsgDlg.Create(hFrame,cs);
	LTVector2n tmp(0,0);
	m_MsgDlg.AddControl(m_pMsgText, tmp);

    return true;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Term
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CInterfaceResMgr::Term()
{
	Clean();

	m_MsgDlg.Destroy();



	for (FontArray::iterator iter = m_FontArray.begin(); iter != m_FontArray.end(); iter++)
	{
		g_pTextureString->UnregisterCustomFontFile(*iter);
	}
	m_FontArray.clear();

}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Setup
// Description	    :
// Return type      : bool
//////////////////////////////////////////////////////////////////////

bool CInterfaceResMgr::Setup()
{
	//preload common surfaces

    return true;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::Clean
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CInterfaceResMgr::Clean()
{
}

//////////////////////////////////////////////////////////////////////
// Function name	: CInterfaceResMgr::DrawScreen
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////
void CInterfaceResMgr::DrawScreen()
{
    LTASSERT(g_pLTClient,"");
    if (!g_pLTClient) return;

	if (m_dwScreenWidth == 0)
		ScreenDimsChanged();

	// Render the current screen
	g_pInterfaceMgr->GetScreenMgr()->Render();

	return;
}



void CInterfaceResMgr::DrawMessage(const char* szMessageId, uint32 nFontSize)
{
	DrawMessage(LoadString(szMessageId),nFontSize);
}

void CInterfaceResMgr::DrawMessage(const wchar_t *pString, uint32 nFontSize)
{
    LTASSERT(g_pLTClient, "");
    if (!g_pLTClient || !pString) return;

	LTVector2 vfScale = GetScreenScale();
	m_MsgDlg.SetScale(vfScale);

	uint32 nSize;
	if (nFontSize)
		nSize = nFontSize;
	else
		nSize = g_pLayoutDB->GetDialogFontSize();

	LTVector2n sz(kBaseWidth-2*kIndent,kBaseHeight);
	m_pMsgText->SetSize(sz);
	m_pMsgText->SetWordWrap(true);
	m_pMsgText->SetString(pString);
	m_pMsgText->SetFont(m_DlgFont);
	m_pMsgText->SetFontHeight(nSize);
	m_pMsgText->RecreateTextureStrings();

	LTRect2n rcText;
	m_pMsgText->GetExtents(rcText);

	uint32 nHeight = (uint32)( (float)rcText.GetHeight() / vfScale.y );
	uint32 nWidth = (uint32)( (float)rcText.GetWidth() / vfScale.x );
	if (nHeight > kBaseWidth)
	{
		LTVector2n sz(kBigWidth-2*kIndent,kBaseHeight);
		m_pMsgText->SetSize(sz);
		m_pMsgText->GetExtents(rcText);
		nHeight = rcText.GetHeight();
		nWidth = rcText.GetWidth();
	}

	uint32 nDlgWidth = nWidth + 2*kIndent;
	if (nDlgWidth < kMinWidth)
		nDlgWidth = kMinWidth;
	uint32 nDlgHeight = nHeight + 2*kIndent;

	LTVector2n offset;

	m_MsgDlg.SetSize(LTVector2n(nDlgWidth,nDlgHeight));

	offset.x = (nDlgWidth - nWidth) / 2;
	offset.y = kIndent;
	m_MsgDlg.SetControlOffset(m_pMsgText,offset);



	offset.x =  (640 - nDlgWidth) / 2;
	offset.y = (480 - nDlgHeight) / 2;
	m_MsgDlg.SetBasePos(offset);
	

	m_MsgDlg.Show(true);

	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{
		m_MsgDlg.Render();

		g_pLTClient->RenderConsoleToRenderTarget();

		g_pLTClient->GetRenderer()->End3D();
		g_pLTClient->GetRenderer()->FlipScreen();
	}
	

	m_MsgDlg.Show(false);

}

bool CInterfaceResMgr::InitFonts()
{

	if (!g_pTextureString || !g_pILTTextureMgr)
		return (false);

	uint8 nNumFonts = g_pLayoutDB->GetNumFontFiles();
	char szFontFile[_MAX_PATH+1];
	for (uint8 i = 0; i < nNumFonts; i++)
	{

		LTStrCpy(szFontFile,g_pLayoutDB->GetFontFile( i ), LTARRAYSIZE( szFontFile ));
		HCUSTOMFONTFILE hFont = g_pTextureString->RegisterCustomFontFile(szFontFile);
		m_FontArray.push_back(hFont);
		
	}

	if ((int)m_FontArray.size() != nNumFonts)
	{
		return false;
	}


    return true;

}

//given a Y FOV in radians and an aspect ratio scale, this will determine the FOV angles
//in radians that should be used for the camera's FOV in the X and Y directions based
//upon the current aspect ratio of the device
LTVector2 CInterfaceResMgr::GetScreenFOV(float fYFOVRadians, float fAspectRatioScale)
{
	//the maximum FOV that can be used
	static const float kfMaxFOV = MATH_DEGREES_TO_RADIANS(178.0f);

	//determine the Y FOV, which should be the provided value, clamped to the range of
	//[0..MAX_FOV]
	float fClampedFOVY = LTCLAMP(fYFOVRadians, 0.0f, kfMaxFOV);

	//using our Y FOV we can compute how large our virtual screen is in space by using
	//the law of opposite sides
	float fHalfImageHeight = tanf(fClampedFOVY * 0.5f);

	//now that we know how high our virtual screen is, we can determine how large it should
	//be in the X direction since the aspect ratio of the device and virtual screen should match
	float fHalfImageWidth = fHalfImageHeight * (float)m_dwScreenWidth / (float)m_dwScreenHeight;

	//we can now convert that into an angle for the FOV ad apply our additional scale
	float fHalfXFOV = atanf(fHalfImageWidth) * fAspectRatioScale;

	//and calculate our final clamped X FOV
	float fClampedFOVX = LTCLAMP(fHalfXFOV * 2.0f, 0.0f, kfMaxFOV);

	//and now we have our final information
	return LTVector2(fClampedFOVX, fClampedFOVY);
}

#define USABLE_HEIGHT_I 480
#define USABLE_HEIGHT_F 480.0f

void CInterfaceResMgr::ScreenDimsChanged()
{
    if (!g_pLTClient) return;

	RMode currentMode;
    g_pLTClient->GetRenderMode(&currentMode);

	m_vfScale.x = (float)currentMode.m_Width / 640.0f;
	m_vfScale.y = (float)currentMode.m_Height / USABLE_HEIGHT_F;

	//if the resolution aspect ratio is closer to 16x9 than 14x3, the resolution is considered widescreen
	if (currentMode.m_Height > 0.0f)
	{
		float fAspectRatio = (float)currentMode.m_Width/(float)currentMode.m_Height;
		float fWidescreenLimit = 14.0f/9.0f; //halfway between 4x3 and 16x9
		m_bWidescreen = (fAspectRatio >= fWidescreenLimit);
	}

	m_dwScreenWidth = currentMode.m_Width;
	m_dwScreenHeight = currentMode.m_Height;

}

const char *CInterfaceResMgr::GetSoundSelect()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "SelectSound");
};

const char *CInterfaceResMgr::GetSoundUnselectable()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "UnselectableSound");
};

const char *CInterfaceResMgr::GetSoundChange()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "SelectChangeSound");
};

const char *CInterfaceResMgr::GetSoundPageChange()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "PageChangeSound");
};

const char *CInterfaceResMgr::GetSoundArrowUp()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ArrowUpSound");
};

const char *CInterfaceResMgr::GetSoundArrowDown()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ArrowDownSound");
};

const char *CInterfaceResMgr::GetSoundArrowLeft()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ArrowLeftSound");
};

const char *CInterfaceResMgr::GetSoundArrowRight()
{
	return g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ArrowRightSound");
};





const LTVector2 CInterfaceResMgr::ConvertScreenPos(int x, int y) const
{
	if (m_dwScreenWidth == 640 && m_dwScreenHeight == USABLE_HEIGHT_I)
		return LTVector2( (float)x, (float)y );

	return LTVector2( (float)x * m_vfScale.x, (float)y * m_vfScale.y );
};

void CInterfaceResMgr::ScaleScreenPos(LTVector2n &pos)
{
	if (m_dwScreenWidth == 640 && m_dwScreenHeight == USABLE_HEIGHT_I)
		return;

	pos.x = int32(m_vfScale.x * float(pos.x) );
	pos.y = int32(m_vfScale.y * float(pos.y) );
}

void CInterfaceResMgr::ConvertScreenRect(LTRect2n &rect)
{
	if (m_dwScreenWidth == 640 && m_dwScreenHeight == USABLE_HEIGHT_I)
		return;

	rect.Left() = (int32)( (float)rect.Left() * m_vfScale.x );
	rect.Right() = (int32)( (float)rect.Right() * m_vfScale.x );
	rect.Top() = (int32)( (float)rect.Top() * m_vfScale.y );
	rect.Bottom() = (int32)( (float)rect.Bottom() * m_vfScale.y );

};



