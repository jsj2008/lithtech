// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterDisplay.cpp
//
// PURPOSE : Class to manage custom displays on characters
//
// CREATED : 11/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "CharacterDisplay.h"
#include "CharacterFX.h"
#include "LayoutDB.h"
#include "ModelsDB.h"


const char* const szEmissiveParam = "tEmissiveMap";
const char* const szDiffuseParam = "tDiffuseMap";


static uint32 nTextLimits[3] = {8,12,MAX_PLAYER_NAME};

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplay::CharacterDisplay()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CharacterDisplay::CharacterDisplay() :
	m_hRenderTarget		( NULL ),
	m_hMaterial			( NULL ),
	m_bRenderTargetBound ( false )
{
	m_pParent = NULL;
	m_hDisplayRecord = NULL;
	m_hLayoutRecord = NULL;
	memset( m_nTextSize, 0, sizeof( m_nTextSize ));
	m_cTextBackColor = 0;
	m_hTexture = NULL;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplay::Init()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool CharacterDisplay::Init(CCharacterFX* pParent, HRECORD hDisplay, const LTVector2n& sz, ClientDisplayData* pData )
{
	//if it's the same we shouldn't have to do anything...
	if ( m_pParent == pParent && 
		 m_hDisplayRecord == hDisplay &&
		 m_hRenderTarget == pData->hNameTarget &&
		 m_vSize.x == (float)sz.x &&
		 m_vSize.y == (float)sz.y &&
		 m_hMaterial == pData->hNameMaterial
		)
	{
		return true;
	}
	m_pParent = pParent;
	m_vSize = LTVector2((float)sz.x,(float)sz.y);

	if (!m_pParent || !m_pParent->GetModel())
	{
		return false;
	}


	m_hDisplayRecord = hDisplay;
	if (!m_hDisplayRecord)
	{
		LTERROR( "Invalid CharacterDisplay" );
		return false;
	}

	// Get the layout record from the localized database.
	m_hLayoutRecord = DATABASE_CATEGORY( CharacterDisplayDB ).GetRecordLinkToLocalizedDB( g_pLTDatabase->GetAttribute( m_hDisplayRecord, "Layout" ),
		0, DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GetCategory( ));
	if( !m_hLayoutRecord )
	{
		LTERROR_PARAM1( "Invalid CharacterDisplayLayout %s", g_pLTDatabase->GetRecordName( m_hDisplayRecord ));
		return false;
	}

	ReleaseRenderTarget();

	char const* pszMaterial = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, Material );
	//check to see if we're using the same base material
	if (pData->hNameMaterial && !LTStrIEquals(pData->sNameMaterialFile.c_str(),pszMaterial) )
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(pData->hNameMaterial);
		pData->hNameMaterial = NULL;
	}

	//hNameMaterial is a reference to the material stored in a client info record, if this is null
	// we need to clone a material for this client to use
	if (!pData->hNameMaterial)
	{
		HMATERIAL hBaseMat = g_pLTClient->GetRenderer()->CreateMaterialInstance( pszMaterial );
		if (!hBaseMat) return false;
		
		pData->hNameMaterial = g_pLTClient->GetRenderer()->CloneMaterialInstance(hBaseMat);
		pData->sNameMaterialFile = pszMaterial;
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hBaseMat);

	}
	if (m_hMaterial != pData->hNameMaterial)
	{
		if (m_hMaterial)
		{
			g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
		}
		m_hMaterial = pData->hNameMaterial;

		g_pLTClient->GetRenderer()->AddRefMaterialInstance(m_hMaterial);
	}
	
	if (m_hMaterial == NULL) 
	{
		return false;
	}

	if (!SetRenderTarget(pData->hNameTarget))
	{
		return false;
	}

	LTVector2 vPos = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GETRECORDATTRIB( m_hLayoutRecord, TextOffset );
	m_Text.SetPos(LTVector2n(uint32(vPos.x),uint32(vPos.y)));
	m_Text.SetAlignment(kCenter);
	m_Text.SetDropShadow(1);

	for (uint8 n = 0; n < 3; ++n)
	{
		m_nTextSize[n] = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GETRECORDATTRIBINDEX( m_hLayoutRecord, TextSize, n);
	}

	HRECORD hFont = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GETRECORDATTRIB( m_hLayoutRecord, Font );
	char const* pszFont = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GetString( hFont, "Face" );
	CFontInfo textFont(pszFont,m_nTextSize[2]);
	m_Text.SetFont(textFont);
	m_Text.SetColor(DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, TextColor));

	m_cTextBackColor = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, TextBackColor);
	LTVector4 vTextBackRect = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, TextBackRect );
	m_rTextBackRect.Init(( int32 )vTextBackRect.x, ( int32 )vTextBackRect.y, ( int32 )vTextBackRect.z, ( int32 )vTextBackRect.w );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplay::Term()
//
//	PURPOSE:	clean up...
//
// ----------------------------------------------------------------------- //

void CharacterDisplay::Term( )
{
	//this will release our texture as well
	ReleaseRenderTarget();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::UpdateDisplay
//
//	PURPOSE:	updates the render target
//
// ----------------------------------------------------------------------- //

void CharacterDisplay::UpdateDisplay(bool bForceClear)
{
	if (!m_hRenderTarget || !m_bRenderTargetBound)
	{
		return;
	}

	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{

		//install our render target to the device
		if(g_pLTClient->GetRenderer()->SetRenderTarget(m_hRenderTarget) != LT_OK)
			return;

		//clear the target
		if (bForceClear) 
		{
			g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, argbBlack) ;
		}
		g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_COLOR, 0) ;
		

		//draw to the target
		Render();

		//uninstall our render target
		g_pLTClient->GetRenderer()->SetRenderTargetScreen();

		g_pLTClient->GetRenderer()->End3D();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::Render
//
//	PURPOSE:	renders to the render target
//
// ----------------------------------------------------------------------- //

void CharacterDisplay::Render()
{
	//draw the background
	LTRESULT res;

	LTPoly_G4 poly;
	uint8 a,r,g,b;
	GET_ARGB(m_cTextBackColor,a,r,g,b);
	DrawPrimSetRGBA(poly,r, g, b, a);
	DrawPrimSetXYWH(poly, (float)m_rTextBackRect.Left(), (float)m_rTextBackRect.Top(), (float)m_rTextBackRect.GetWidth(), (float)m_rTextBackRect.GetHeight() );

	res= g_pDrawPrim->DrawPrim(&poly,1);

	if (m_hTexture != (HTEXTURE)NULL)
	{
		LTPoly_GT4 img;
		DrawPrimSetRGBA(img,0xFF,0xFF,0xFF,0xFF);
		DrawPrimSetXYWH(img, 0.0f, 0.0f, m_vSize.x, m_vSize.y );
		SetupQuadUVs(img, m_hTexture, 0.0f,0.0f,1.0f,1.0f );
		g_pDrawPrim->SetTexture(m_hTexture);
		res= g_pDrawPrim->DrawPrim(&img,1);
		if (res != LT_OK)
		{
			res = res;
		}
	}

	//draw the text
	if (!m_Text.IsEmpty()) 
	{
		res = m_Text.Render(); //( m_rTextBackRect );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::SetRenderTarget
//
//	PURPOSE:	this handles creation of the render target
//
// ----------------------------------------------------------------------- //

bool CharacterDisplay::SetRenderTarget(HRENDERTARGET hRenderTarget)
{

	HATTRIBUTE hAttrib = DATABASE_CATEGORY( CharacterDisplayDB ).GetAttribute( m_hDisplayRecord, "MaterialParameter" );
	const char* pszParam = DATABASE_CATEGORY( CharacterDisplayDB ).GetString( hAttrib,0,szEmissiveParam);

	m_hRenderTarget = hRenderTarget;
	if (!SetMaterial())
	{
		ReleaseRenderTarget();
		return false;
	}

	//and now install it on the associated material
	LTRESULT lr = g_pLTClient->GetRenderer()->SetInstanceParamRenderTarget( m_hMaterial, pszParam, m_hRenderTarget);

	m_bRenderTargetBound = (lr == LT_OK);
	if (!m_bRenderTargetBound)
	{
		return false;
	}


	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::ReleaseRenderTarget
//
//	PURPOSE:	this will release the associated render target
//
// ----------------------------------------------------------------------- //

bool CharacterDisplay::ReleaseRenderTarget()
{
	m_hTexture.Free();

	if (m_hMaterial)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);	
		m_hMaterial = NULL;
	}
	

	//if we don't have one just bail
	if(!m_hRenderTarget)
		return true;

	//and clear our handle
	m_hRenderTarget = NULL;
	m_bRenderTargetBound = false;



	//success
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::SetMaterial
//
//	PURPOSE:	set the character model to use the display material
//
// ----------------------------------------------------------------------- //

bool CharacterDisplay::SetMaterial()
{
	if (!m_pParent || !m_hMaterial)
	{
//		DebugCPrint(0,"%s - failed, no parent or no material",__FUNCTION__);
		return false;
	}

	int32 nMaterialIndex = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, MaterialIndex );
	if (g_pILTModelClient->SetMaterial(m_pParent->GetServerObj(),nMaterialIndex,m_hMaterial) != LT_OK)
	{
		return false;
	}

	return true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplay::SetText
//
//	PURPOSE:	set the string to be displayed
//
// ----------------------------------------------------------------------- //

void CharacterDisplay::SetText(const wchar_t* szTxt) 
{ 
	if (!m_hDisplayRecord)
		return;

	if ( LTStrEquals(m_Text.GetText(),szTxt ) )
	{
		return;
	}

	if (szTxt) 
	{
		m_Text.SetText(szTxt); 
	}

	uint32 nLen = LTStrLen(szTxt);

	HRECORD hFont = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GETRECORDATTRIB( m_hLayoutRecord, Font );
	char const* pszFont = DATABASE_CATEGORY( CharacterDisplayLayoutDB ).GetString( hFont, "Face" );
	if (nLen <= nTextLimits[0])
	{
		CFontInfo textFont(pszFont,m_nTextSize[0]);
		textFont.m_nStyle = CFontInfo::kStyle_Bold;
		m_Text.SetFont(textFont);
	}
	else if (nLen <= nTextLimits[1])
	{
		CFontInfo textFont(pszFont,m_nTextSize[1]);
		textFont.m_nStyle = CFontInfo::kStyle_Bold;
		m_Text.SetFont(textFont);
	}
	else
	{
		CFontInfo textFont(pszFont,m_nTextSize[2]);
		textFont.m_nStyle = CFontInfo::kStyle_Bold;
		m_Text.SetFont(textFont);
	}
	
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplaySimple::CharacterDisplaySimple()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CharacterDisplaySimple::CharacterDisplaySimple() :
	m_hMaterial			( NULL )
{
	m_pParent = NULL;
	m_hDisplayRecord = NULL;
	m_hTexture = NULL;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplaySimple::Init()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool CharacterDisplaySimple::Init(CCharacterFX* pParent, HRECORD hDisplay, ClientDisplayData* pData)
{
	m_pParent = pParent;

	if (!m_pParent || !m_pParent->GetModel())
	{
		return false;
	}

	m_hDisplayRecord = hDisplay;
	if (!m_hDisplayRecord)
	{
		return false;
	}

	m_hTexture.Free();

	char const* pszMaterial = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, Material );
	//check to see if we're using the same base material
	if (pData->hInsigniaMaterial && !LTStrIEquals(pData->sInsigniaMaterialFile.c_str(),pszMaterial) )
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(pData->hInsigniaMaterial);
		pData->hInsigniaMaterial = NULL;
	}

	//hNameMaterial is a reference to the material stored in a client info record, if this is null
	// we need to clone a material for this client to use
	if (!pData->hInsigniaMaterial)
	{
		HMATERIAL hBaseMat = g_pLTClient->GetRenderer()->CreateMaterialInstance( pszMaterial );
		if (!hBaseMat) return false;

		pData->hInsigniaMaterial = g_pLTClient->GetRenderer()->CloneMaterialInstance(hBaseMat);
		pData->sInsigniaMaterialFile = pszMaterial;
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hBaseMat);

	}

	//if we're changing materials release the old one
	if (m_hMaterial != pData->hInsigniaMaterial)
	{
		if (m_hMaterial)
		{
			g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
		}
		m_hMaterial = pData->hInsigniaMaterial;

		g_pLTClient->GetRenderer()->AddRefMaterialInstance(m_hMaterial);
	}

	if (m_hMaterial == NULL) 
	{
		return false;
	}


	return SetMaterial();
}




// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CharacterDisplaySimple::Term()
//
//	PURPOSE:	clean up...
//
// ----------------------------------------------------------------------- //

void CharacterDisplaySimple::Term( )
{
	m_hTexture.Free();
	if (m_hMaterial	)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
		m_hMaterial = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplaySimple::SetMaterial
//
//	PURPOSE:	set the character model to use the display material
//
// ----------------------------------------------------------------------- //

bool CharacterDisplaySimple::SetMaterial()
{
	if (!m_pParent || !m_hMaterial)
	{
		//		DebugCPrint(0,"%s - failed, no parent or no material",__FUNCTION__);
		return false;
	}

	int32 nMaterialIndex = DATABASE_CATEGORY( CharacterDisplayDB ).GETRECORDATTRIB( m_hDisplayRecord, MaterialIndex );
	if (g_pILTModelClient->SetMaterial(m_pParent->GetServerObj(),nMaterialIndex,m_hMaterial) != LT_OK)
	{
		return false;
	}

	return true;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CharacterDisplaySimple::UpdateDisplay
//
//	PURPOSE:	updates the render target
//
// ----------------------------------------------------------------------- //

void CharacterDisplaySimple::SetTexture(const char* szTex)
{
	if (!m_hDisplayRecord || !m_hMaterial) return;

	HATTRIBUTE hAttrib = DATABASE_CATEGORY( CharacterDisplayDB ).GetAttribute( m_hDisplayRecord, "MaterialParameter" );
	const char* pszParam = DATABASE_CATEGORY( CharacterDisplayDB ).GetString( hAttrib,0,szEmissiveParam);

	m_hTexture.Load(szTex);
	//and now install it on the associated material
	LTRESULT lr = g_pLTClient->GetRenderer()->SetInstanceParamTexture( m_hMaterial, pszParam, m_hTexture);

}

