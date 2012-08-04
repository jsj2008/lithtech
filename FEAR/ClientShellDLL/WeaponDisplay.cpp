// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDisplay.cpp
//
// PURPOSE : Implementation of a custom weapon display class
//
// CREATED : 09/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "WeaponDisplay.h"
#include "LayoutDB.h"
#include "ClientWeapon.h"
#include "ForensicObjectFX.h"
#include "CMoveMgr.h"

//------------------------------------------------------
// WeaponDisplay record attributes
const char* const WDDB_sWDType =				"Type";
const char* const WDDB_sWDMaterial =			"Material";
const char* const WDDB_nWDMaterialIndex =	"MaterialIndex";
const char* const WDDB_sWDOverlayFX =		"OverlayFX";
const char* const WDDB_v2WDSize =			"Size";
const char* const WDDB_cWDTextColor =		"TextColor";
const char* const WDDB_bWDTextGlow =			"TextGlow";
const char* const WDDB_sWDBackground =		"Background";
const char* const WDDB_sCameraSocket =		"CameraSocket";
const char* const WDDB_fCameraFovX =			"CameraFovX";
const char* const WDDB_fCameraFovY =			"CameraFovY";
const char* const WDDB_fUpdateInterval =		"UpdateInterval";
const char* const WDDB_v2WDAddPoint =		"AdditionalPoint";
const char* const WDDB_v4WDAddRect =			"AdditionalRect";
const char* const WDDB_nWDAddInt =			"AdditionalInt";
const char* const WDDB_fWDAddFloat =			"AdditionalFloat";
const char* const WDDB_cWDAddColor =			"AdditionalColor";
const char* const WDDB_sWDAddTex =			"AdditionalTexture";
const char* const WDDB_sLayout =			"Layout";

const char* const WDLDB_rWDFont =				"Font";
const char* const WDLDB_v2WDTextOffset =		"TextOffset";
const char* const WDLDB_nWDTextSize =			"TextSize";
const char* const WDLDB_sWDTextAlignment =	"TextAlignment";


const char* const szEmissiveParam = "tEmissiveMap";
const char* const szDiffuseParam = "tDiffuseMap";

HRENDERTARGET	WeaponDisplay::s_hRenderTarget = NULL;
uint32			WeaponDisplay::s_nRenderTargetRefCount = 0;
LTVector2n		WeaponDisplay::s_v2nRenderTargetSize = LTVector2n(0,0);

//-----------------------------------------------------------------------------
// WeaponDisplayInterf.
//-----------------------------------------------------------------------------

WeaponDisplayInterf::WeaponDisplayInterf()
{
}

WeaponDisplayInterf::~WeaponDisplayInterf()
{
}

//-----------------------------------------------------------------------------
// create a weapon display for the specified weapon, using the supplied database record

WeaponDisplayInterf* WeaponDisplayInterf::CreateDisplay(CWeaponModelData* pParent, HRECORD hDisplay )
{
	if (!hDisplay)
		return NULL;

	const char* szType = DATABASE_CATEGORY( WeaponDisplayDB ).GetString(hDisplay,WDDB_sWDType);

	WeaponDisplayInterf* pDisplay = NULL;

	if (LTStrEquals(szType,"VerticalAmmoBar"))
	{
		pDisplay = debug_new(WeaponDisplayAmmoBarV);
	}
	else if (LTStrEquals(szType,"RechargeBar"))
	{
		pDisplay = debug_new(WeaponDisplayRecharge);
	}
	else if (LTStrEquals(szType,"Spectrometer"))
	{
		pDisplay = debug_new(WeaponDisplaySpectrometer);
	}
	else if (LTStrEquals(szType, "DigitalCamera"))
	{
		pDisplay = debug_new(WeaponDisplayRenderTargetOverlay);
	}
	else
	{
		pDisplay = debug_new(WeaponDisplay);
	}

	if (!pDisplay)
	{
		return NULL;
	}

	if (!pDisplay->Init(pParent,hDisplay))
	{
		pDisplay->Term();
		return NULL;
	}

	return pDisplay;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplay::WeaponDisplay()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

WeaponDisplay::WeaponDisplay() :
	m_hRenderTarget		( NULL ),
	m_hMaterial			( NULL ),
	m_hBackground		( NULL ),
	m_sCameraSocket		( "" ),
	m_vCameraFov		( MATH_DEGREES_TO_RADIANS(90.0f), MATH_DEGREES_TO_RADIANS(90.0f) ),
	m_nMaterialIndex	( -1 ),
	m_nCount			( -1 ),
	m_bRenderTargetBound ( false ),
	m_fUpdateInterval	( 0.0f ),
	m_fLastUpdate		( 0.0f )
{
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplay::Init()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool WeaponDisplay::Init(CWeaponModelData* pParent, HRECORD hDisplay )
{
	m_pParent = pParent;
	m_hDisplayRecord = hDisplay;

	if (!m_pParent || !m_hDisplayRecord )
	{
		return false;
	}

	m_hLayoutRecord = DATABASE_CATEGORY( WeaponDisplayDB ).GetRecordLinkToLocalizedDB( g_pLTDatabase->GetAttribute( m_hDisplayRecord, WDDB_sLayout ),
		0, DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetCategory( ));
	if( !m_hLayoutRecord)
	{
		LTERROR_PARAM1( "Invalid weapondisplay %s", g_pLTDatabase->GetRecordName( m_hDisplayRecord ));
		return false;
	}

	m_sCameraSocket = DATABASE_CATEGORY( WeaponDisplayDB ).GetString(m_hDisplayRecord,WDDB_sCameraSocket);
	m_vCameraFov.x = MATH_DEGREES_TO_RADIANS(DATABASE_CATEGORY( WeaponDisplayDB ).GetFloat(m_hDisplayRecord,WDDB_fCameraFovX));
	m_vCameraFov.y = MATH_DEGREES_TO_RADIANS(DATABASE_CATEGORY( WeaponDisplayDB ).GetFloat(m_hDisplayRecord,WDDB_fCameraFovY));

	m_fUpdateInterval = DATABASE_CATEGORY( WeaponDisplayDB ).GetFloat(m_hDisplayRecord,WDDB_fUpdateInterval);

	HMATERIAL hBaseMat = g_pLTClient->GetRenderer()->CreateMaterialInstance( DATABASE_CATEGORY( WeaponDisplayDB ).GetString(m_hDisplayRecord,WDDB_sWDMaterial ) );
	if (!hBaseMat) return false;
	m_hMaterial = g_pLTClient->GetRenderer()->CloneMaterialInstance(hBaseMat);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hBaseMat);
	if (m_hMaterial == NULL) 
	{
		return false;
	}

	if (!CreateRenderTarget())
	{
		//release our material
		if (m_hMaterial)
		{
			g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
			m_hMaterial = NULL;
		}
		return false;
	}


	LTVector2 vPos = DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetVector2(m_hLayoutRecord,WDLDB_v2WDTextOffset);
	m_Text.SetPos(LTVector2n(uint32(vPos.x),uint32(vPos.y)));

	HRECORD hFont = DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetRecordLink( m_hLayoutRecord, WDLDB_rWDFont );
	char const* pszFont = DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetString( hFont, "Face" );
	CFontInfo textFont( pszFont, DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetInt32(m_hLayoutRecord,WDLDB_nWDTextSize));
	m_Text.SetFont(textFont);
	m_Text.SetColor(( uint32 )DATABASE_CATEGORY( WeaponDisplayDB ).GetInt32(m_hDisplayRecord,WDDB_cWDTextColor,0,0xFFFFFFFF));

	if (DATABASE_CATEGORY( WeaponDisplayDB ).GetBool(m_hDisplayRecord,WDDB_bWDTextGlow)) 
	{
		m_Text.SetGlowParams(true);
		m_Text.SetGlow(true);
	}

	const char* pTmp = DATABASE_CATEGORY( WeaponDisplayLayoutDB ).GetString(m_hLayoutRecord,WDLDB_sWDTextAlignment);
	if (LTStrICmp(pTmp,"Left") == 0)
	{
		m_Text.SetAlignment(kLeft);
	}
	else if (LTStrICmp(pTmp,"Center") == 0)
	{
		m_Text.SetAlignment(kCenter);
	}
	else if (LTStrICmp(pTmp,"Right") == 0)
	{
		m_Text.SetAlignment(kRight);
	}

	m_hBackground = g_pLTClient->GetRenderer()->CreateMaterialInstance( DATABASE_CATEGORY( WeaponDisplayDB ).GetString(m_hDisplayRecord,WDDB_sWDBackground) );

	if (m_hBackground)
	{
		// Setup the poly.
		DrawPrimSetRGBA(m_Background,0xFF, 0xFF, 0xFF, 0xFF);
		DrawPrimSetUVWH(m_Background, 0.0f, 0.0f, 1.0f, 1.0f );

		LTVector2 vSize = DATABASE_CATEGORY( WeaponDisplayDB ).GetVector2(m_hDisplayRecord,WDDB_v2WDSize	);
		DrawPrimSetXYWH(m_Background,0.0f,0.0f,vSize.x,vSize.y);
	}


	return true;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplay::Reset()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool WeaponDisplay::Reset()
{

	if (!m_hMaterial) return false;
	if (!m_hRenderTarget) return false;

	const char* pszParam = DATABASE_CATEGORY( WeaponDisplayDB ).GetString(m_hDisplayRecord,"MaterialParameter",0,szEmissiveParam);

	//and now install it on the associated material
	LTRESULT lr = g_pLTClient->GetRenderer()->SetInstanceParamRenderTarget(  m_hMaterial, pszParam, m_hRenderTarget);

	m_bRenderTargetBound = (lr == LT_OK);
	if (!m_bRenderTargetBound)
		return false;

	if (!SetMaterial())
	{
		m_bRenderTargetBound = false;
		return false;
	}

	return true;
}




// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplay::Term()
//
//	PURPOSE:	clean up...
//
// ----------------------------------------------------------------------- //

void WeaponDisplay::Term( )
{
	ReleaseRenderTarget();

	//release our background material
	if (m_hBackground)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hBackground);
		m_hBackground = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::UpdateDisplay
//
//	PURPOSE:	updates the render target based on the current ammo
//
// ----------------------------------------------------------------------- //

void WeaponDisplay::UpdateDisplay(bool /*bFiredWeapon*/)
{
	if (!m_hRenderTarget || !m_bRenderTargetBound)
	{
		return;
	}

	// Restrict update rate
	if (m_fUpdateInterval > 0.0f)
	{
		double fCurTime = g_pLTClient->GetTime();
		if (fCurTime < (m_fLastUpdate + m_fUpdateInterval))
		{
			return;
		}

		m_fLastUpdate = fCurTime;
	}

	//ammo count hasn't changed, no need to update the string
	if (m_nCount != m_pParent->m_nAmmoInClip)
	{
		m_nCount = m_pParent->m_nAmmoInClip;
		wchar_t wstr[32];
		LTSNPrintF(wstr,LTARRAYSIZE(wstr),L"%02d", m_nCount);
		m_Text.SetText(wstr);
	}

	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{

		//install our render target to the device
		if(g_pLTClient->GetRenderer()->SetRenderTarget(m_hRenderTarget) != LT_OK)
			return;


		//clear the target
		if (!m_sCameraSocket.empty())
            g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, argbBlack);
		else
			g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_COLOR, argbBlack);

		//draw to the target
		Render();

		//uninstall our render target
		g_pLTClient->GetRenderer()->SetRenderTargetScreen();

		g_pLTClient->GetRenderer()->End3D();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::Render
//
//	PURPOSE:	renders to the render target
//
// ----------------------------------------------------------------------- //

void WeaponDisplay::Render()
{
	//draw a pretty camera
	if (!m_sCameraSocket.empty())
	{
		HMODELSOCKET hSocket;
		if (LT_OK == g_pModelLT->GetSocket(m_pParent->m_hObject, m_sCameraSocket.c_str(), hSocket))
		{
			LTTransform tSocket;
			if (LT_OK == g_pModelLT->GetSocketTransform(m_pParent->m_hObject, hSocket, tSocket, true))
			{
				//a rectangle that can be used as a viewport for when rendering to a texture
				LTRect2f rViewport(0.0f, 0.0f, 1.0f, 1.0f);

				//build up our transform for the camera
				LTRigidTransform tCamera(tSocket.m_vPos, tSocket.m_rRot);

				//and now we can render the scene
				g_pLTClient->GetRenderer()->RenderCamera(tCamera, tCamera.m_vPos, m_vCameraFov, rViewport, NULL);
			}
		}
	}

	//draw the background
	LTRESULT res;

	if (m_hBackground)
	{
		res = g_pDrawPrim->DrawPrimMaterial( &m_Background, 1, m_hBackground );
	}

	//draw the text
	if (!m_Text.IsEmpty()) 
	{
		res = m_Text.Render();
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::CreateRenderTarget
//
//	PURPOSE:	this handles creation of the render target
//
// ----------------------------------------------------------------------- //

bool WeaponDisplay::CreateRenderTarget()
{
	//first off clear up any existing render target
	ReleaseRenderTarget();

	LTVector2 vSize = DATABASE_CATEGORY( WeaponDisplayDB ).GetVector2(m_hDisplayRecord,WDDB_v2WDSize	);
	m_v2nSize = LTVector2n((uint32)vSize.x,(uint32)vSize.y);

	const char* pszParam = DATABASE_CATEGORY( WeaponDisplayDB ).GetString(m_hDisplayRecord,"MaterialParameter",0,szEmissiveParam);

	uint32 nRTFlags = eRTO_AutoGenMipMaps;

	if (!m_sCameraSocket.empty())
		nRTFlags |= eRTO_DepthBuffer;

	//now that we have our options, attempt to create the render target
	if (!s_hRenderTarget)
	{
		HRECORD hShared = g_pLayoutDB->GetSharedRecord();
		LTVector2 vSize = g_pLayoutDB->GetVector2(hShared,"WeaponDisplaySize",0,LTVector2(256.0f,256.0f));
		s_v2nRenderTargetSize.x = (uint32)vSize.x;
		s_v2nRenderTargetSize.y = (uint32)vSize.y;

		if(g_pLTClient->GetRenderer()->CreateRenderTarget(s_v2nRenderTargetSize.x, s_v2nRenderTargetSize.y, nRTFlags, s_hRenderTarget) != LT_OK)
		{
			return false;
		}
	}
	m_hRenderTarget = s_hRenderTarget;
	s_nRenderTargetRefCount++;

	//and now install it on the associated material
	LTRESULT lr = g_pLTClient->GetRenderer()->SetInstanceParamRenderTarget(  m_hMaterial, pszParam, m_hRenderTarget);

	m_bRenderTargetBound = (lr == LT_OK);
	if (!m_bRenderTargetBound)
		return false;


	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::ReleaseRenderTarget
//
//	PURPOSE:	this will release the associated render target
//
// ----------------------------------------------------------------------- //

bool WeaponDisplay::ReleaseRenderTarget()
{
	//if we don't have one just bail
	if(!m_hRenderTarget)
		return true;

	//release our material
	if (m_hMaterial)
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hMaterial);
		m_hMaterial = NULL;
	}


	//and now release our reference to the render target
	if (s_nRenderTargetRefCount > 0)
	{
		s_nRenderTargetRefCount--;
		if (s_nRenderTargetRefCount == 0)
		{
			g_pLTClient->GetRenderer()->ReleaseRenderTarget(s_hRenderTarget);
		}
		
	}

	//and clear our handle
	m_hRenderTarget = NULL;
	m_bRenderTargetBound = false;

	//success
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::SetMaterial
//
//	PURPOSE:	set the weapon to use the display material
//
// ----------------------------------------------------------------------- //

bool WeaponDisplay::SetMaterial()
{
	if (!m_pParent || !m_hMaterial)
		return false;
	if (g_pILTModelClient->SetMaterial(m_pParent->m_hObject,DATABASE_CATEGORY( WeaponDisplayDB ).GetInt32(m_hDisplayRecord,WDDB_nWDMaterialIndex),m_hMaterial) != LT_OK)
	{
		return false;
	}

	return true;

}



// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplayAmmoBarV::WeaponDisplayAmmoBarV()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

WeaponDisplayAmmoBarV::WeaponDisplayAmmoBarV() : WeaponDisplay()
{
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplayAmmoBarV::Init()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool WeaponDisplayAmmoBarV::Init(CWeaponModelData* pParent, HRECORD hDisplay )
{
	if (!WeaponDisplay::Init(pParent,hDisplay))
	{
		return false;
	}

	m_cFullColor = ( uint32 )DATABASE_CATEGORY( WeaponDisplayDB ).GetInt32(m_hDisplayRecord,WDDB_cWDTextColor,0,0xFFFFFFFF);
	m_cEmptyColor = ( uint32 )DATABASE_CATEGORY( WeaponDisplayDB ).GetInt32(hDisplay,WDDB_cWDAddColor,0,0xFFFFFFFF);

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(pParent->m_hWeaponRecord, !USE_AI_DATA );
	m_nMaxCount = g_pWeaponDB->GetInt32(hWpnData,WDB_WEAPON_nShotsPerClip);

	LTVector4 vRect = DATABASE_CATEGORY( WeaponDisplayDB ).GetVector4(hDisplay,WDDB_v4WDAddRect,0);
	m_rfRect = LTRect2f( vRect.x, vRect.y, vRect.z, vRect.w );

	DrawPrimSetXYWH(m_Poly[0],m_rfRect.Left(),m_rfRect.Top(),0.0f,0.0f);
	DrawPrimSetXYWH(m_Poly[1],m_rfRect.Left(),m_rfRect.Top(),m_rfRect.GetWidth(),m_rfRect.GetHeight());

	DrawPrimSetRGBA(m_Poly[0],m_cEmptyColor);
	DrawPrimSetRGBA(m_Poly[1],m_cFullColor);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplayAmmoBarV::UpdateDisplay
//
//	PURPOSE:	updates the render target based on the current ammo
//
// ----------------------------------------------------------------------- //

void WeaponDisplayAmmoBarV::UpdateDisplay(bool bFiredWeapon)
{
	WeaponDisplay::UpdateDisplay(bFiredWeapon);

	//ammo count hasn't changed, no need to update the string
	if (m_nCount != m_pParent->m_nAmmoInClip)
	{
		float fPercent = 1.0f;
		if (m_pParent->m_nAmmoInClip <= 0 ) 
		{
			fPercent = 0.0f;
		}
		else if (m_pParent->m_nAmmoInClip >= m_nMaxCount)
		{
			fPercent = 1.0f;
		}
		else
		{
			fPercent = float(m_pParent->m_nAmmoInClip)/float(m_nMaxCount);
		}

		//height of full bar
		float fHeight1 = fPercent * m_rfRect.GetHeight();
		float fTop1 = m_rfRect.Bottom() - fHeight1;

		//height of empty bar
		float fHeight0 = m_rfRect.GetHeight() - fHeight1;

		DrawPrimSetXYWH(m_Poly[0],m_rfRect.Left(),m_rfRect.Top(),m_rfRect.GetWidth(),fHeight0);
		DrawPrimSetXYWH(m_Poly[1],m_rfRect.Left(),fTop1,m_rfRect.GetWidth(),fHeight1);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::Render
//
//	PURPOSE:	renders to the render target
//
// ----------------------------------------------------------------------- //

void WeaponDisplayAmmoBarV::Render()
{
	WeaponDisplay::Render();

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	g_pDrawPrim->DrawPrim(m_Poly,2);
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplayRecharge::WeaponDisplayRecharge()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

WeaponDisplayRecharge::WeaponDisplayRecharge() : WeaponDisplay()
{
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	WeaponDisplayRecharge::Init()
//
//	PURPOSE:	set up the display
//
// ----------------------------------------------------------------------- //

bool WeaponDisplayRecharge::Init(CWeaponModelData* pParent, HRECORD hDisplay )
{
	if (!WeaponDisplay::Init(pParent,hDisplay))
	{
		return false;
	}

	//set up recharge timer
	m_RechargeTimer.SetEngineTimer(SimulationTimer::Instance());
	m_fRecharge = DATABASE_CATEGORY( WeaponDisplayDB ).GetFloat(hDisplay,WDDB_fWDAddFloat,0);

	//set up the recharge bar
	LTVector4 vRect = DATABASE_CATEGORY( WeaponDisplayDB ).GetVector4(hDisplay,WDDB_v4WDAddRect,0);
	m_rfRect.Init( vRect.x, vRect.y, vRect.z, vRect.w );

	DrawPrimSetXYWH(m_Poly[0],m_rfRect.Left(),m_rfRect.Top(),0.0f,0.0f);
	DrawPrimSetXYWH(m_Poly[1],m_rfRect.Left(),m_rfRect.Top(),m_rfRect.GetWidth(),m_rfRect.GetHeight());

	DrawPrimSetRGBA(m_Poly[0],argbWhite);
	DrawPrimSetRGBA(m_Poly[1],argbWhite);

	//set up the ready indicator
	vRect = DATABASE_CATEGORY( WeaponDisplayDB ).GetVector4(hDisplay,WDDB_v4WDAddRect,1);
	LTRect2f rfRect = LTRect2f( vRect.x, vRect.y, vRect.z, vRect.w );
	DrawPrimSetXYWH(m_Poly[2],rfRect.Left(),rfRect.Top(),rfRect.GetWidth(),rfRect.GetHeight());
	DrawPrimSetRGBA(m_Poly[2],argbWhite);

	//load textures
	for (uint8 i =0; i < 4; ++i)
	{
		m_Textures[i].Load(DATABASE_CATEGORY( WeaponDisplayDB ).GetString(hDisplay,WDDB_sWDAddTex,i));
	}

	SetupQuadUVs(m_Poly[2], m_Textures[2], 0.0f,0.0f,1.0f,1.0f );

	m_bReady = true;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplayRecharge::UpdateDisplay
//
//	PURPOSE:	updates the render target based on the current ammo
//
// ----------------------------------------------------------------------- //

void WeaponDisplayRecharge::UpdateDisplay(bool bFiredWeapon)
{

	float fPercent;

	if (bFiredWeapon)
	{
		//just fired, start timer and set percent ready to 0
		m_RechargeTimer.Start(m_fRecharge);
		fPercent = 1.0f;
		m_bReady = false;
	}
	else if (!m_RechargeTimer.IsStarted() || m_RechargeTimer.IsTimedOut())
	{
		//use full bar if not fired yet or finished recharging
		fPercent = 0.0f;
		m_bReady = true;
	}
	else
	{
		//figure out percent ready
		fPercent = (float)(m_RechargeTimer.GetTimeLeft() / m_RechargeTimer.GetDuration());
		m_bReady = false;
	}

	//height of empty bar
	float fHeightEmpty = fPercent * m_rfRect.GetHeight();
	float fTopEmpty = m_rfRect.Bottom() - fHeightEmpty;

	//height of full bar
	float fHeightFull = m_rfRect.GetHeight() - fHeightEmpty;

	DrawPrimSetXYWH(m_Poly[0],m_rfRect.Left(),fTopEmpty,m_rfRect.GetWidth(),fHeightEmpty);
	DrawPrimSetXYWH(m_Poly[1],m_rfRect.Left(),m_rfRect.Top(),m_rfRect.GetWidth(),fHeightFull);

	SetupQuadUVs(m_Poly[0], m_Textures[0], 0.0f,1.0f-fPercent,1.0f,fPercent );
	SetupQuadUVs(m_Poly[1], m_Textures[1], 0.0f,0.0f,1.0f,1.0f-fPercent );


	WeaponDisplay::UpdateDisplay(bFiredWeapon);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponDisplay::Render
//
//	PURPOSE:	renders to the render target
//
// ----------------------------------------------------------------------- //

void WeaponDisplayRecharge::Render()
{
	WeaponDisplay::Render();

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	//empty bar
	g_pDrawPrim->SetTexture(m_Textures[0]);
	g_pDrawPrim->DrawPrim(&m_Poly[0],1);

	//full bar
	g_pDrawPrim->SetTexture(m_Textures[1]);
	g_pDrawPrim->DrawPrim(&m_Poly[1],1);

	if (m_bReady)
	{
		g_pDrawPrim->SetTexture(m_Textures[3]);
	}
	else
	{
		g_pDrawPrim->SetTexture(m_Textures[2]);
	}
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

}

//-----------------------------------------------------------------------------
// WeaponDisplay3DProgressBar.
//-----------------------------------------------------------------------------

WeaponDisplay3DProgressBar::WeaponDisplay3DProgressBar() : WeaponDisplayInterf()
{
}

//-----------------------------------------------------------------------------

bool WeaponDisplay3DProgressBar::Init(CWeaponModelData* pParent, HRECORD hDisplay )
{
	m_pParent = pParent;
	m_hDisplayRecord = hDisplay;

	if (!m_pParent || !m_hDisplayRecord )
	{
		return false;
	}

	if( !InitMaterialListAttribute() ||
		!InitFirstMaterialIndex() ||
		!InitNumMaterials() )
	{
		return false;
	}

	// first is the default material set, followed by the active material set
	// strings already checked to be valid when determining the number of total materials
	for(uint32 iMaterial=0; iMaterial < m_nNumMaterials; ++iMaterial)
	{
		m_DefaultMaterials.push_back(g_pWeaponDB->GetString(m_hMaterialListAtt, m_nFirstMaterialIndex + iMaterial, ""));
		m_ActiveMaterials.push_back(g_pWeaponDB->GetString(m_hMaterialListAtt, m_nFirstMaterialIndex + iMaterial + m_nNumMaterials, ""));
	}

	return true;
}

//-----------------------------------------------------------------------------

void WeaponDisplay3DProgressBar::Term()
{
}

//-----------------------------------------------------------------------------

bool WeaponDisplay3DProgressBar::Reset()
{
	return true;
}

//-----------------------------------------------------------------------------

void WeaponDisplay3DProgressBar::UpdateDisplay(bool bFiredWeapon)
{
	// check our current progress - rounding to the nearest material index
	uint8 nProgressIndex = (uint8)((GetProgress() * m_nNumMaterials) + 0.5f);

	if (nProgressIndex == m_nLastProgressIndex)		// nothing to update
		return;

	// set the materials
	for(uint8 iMaterial = 0; iMaterial < m_nNumMaterials; ++iMaterial)
	{
		const char* pszMat = (iMaterial >= nProgressIndex) ?
			m_DefaultMaterials[iMaterial].c_str() : 
			m_ActiveMaterials[iMaterial].c_str();

		g_pModelLT->SetMaterialFilename( m_pParent->m_hObject, iMaterial + m_nFirstMaterialIndex, pszMat );
	}

	// remember if last progress value
	m_nLastProgressIndex = nProgressIndex;
}

//-----------------------------------------------------------------------------

void WeaponDisplay3DProgressBar::Render()
{
	//weapon model rendering will take care of displaying the progress meter
}

//-----------------------------------------------------------------------------
// WeaponDisplaySpectrometer.
//-----------------------------------------------------------------------------

WeaponDisplaySpectrometer::WeaponDisplaySpectrometer() : WeaponDisplay3DProgressBar()
{
}

//-----------------------------------------------------------------------------

bool WeaponDisplaySpectrometer::InitFirstMaterialIndex()
{
	m_nFirstMaterialIndex = g_pLayoutDB->GetInt32(m_hDisplayRecord,WDDB_nWDMaterialIndex,0);
	return true;
}

//-----------------------------------------------------------------------------

bool WeaponDisplaySpectrometer::InitNumMaterials()
{
	// the default materials and the active materials should
	// be at the end of the material list.

	uint32 nTotalMaterials = 0;
	for(uint32 iMaterial=m_nFirstMaterialIndex; iMaterial < MAX_MATERIALS_PER_MODEL; ++iMaterial)
	{
		if(LTStrEmpty(g_pWeaponDB->GetString(m_hMaterialListAtt, iMaterial, "")))
			break;
		else
			++nTotalMaterials;
	}

	// there should be just as many default materials as active ones.
	if(nTotalMaterials % 2 != 0)
	{
		LTERROR( "Bad initial material index or the material list has an odd number of weapon display materials." );
		return false;
	}

	m_nNumMaterials = nTotalMaterials / 2;
	return true;
}

//-----------------------------------------------------------------------------

bool WeaponDisplaySpectrometer::InitMaterialListAttribute()
{
	// find the material list in the weapon model struct
	HATTRIBUTE hMaterialAtt = g_pWeaponDB->GetStructAttribute( m_pParent->m_hWeaponModelStruct, 0, WDB_WEAPON_sHHMaterial );
	if( !hMaterialAtt )
	{
		LTERROR( "Failed to retrieve the weapon model materials." );
		return false;
	}

	m_hMaterialListAtt = hMaterialAtt;
	return true;
}

//-----------------------------------------------------------------------------

float WeaponDisplaySpectrometer::GetProgress()
{
	// figure out how close we are to the evidence
	float fDistPct = 0.0f;

	CForensicObjectFX* pFX = g_pPlayerMgr->GetForensicObject();
	if( pFX )
	{
		float fDist = pFX->GetDistance(g_pPlayerMgr->GetMoveMgr()->GetServerObject());
		if( (fDist > 0.0f) && (fDist <= pFX->m_cs.m_fMaxDistance) )
		{
			fDist -= pFX->m_cs.m_fCoreRadius;
			float fMaxDist = (pFX->m_cs.m_fMaxDistance - pFX->m_cs.m_fCoreRadius);	// max distance is guaranteed to be larger than core radius (see ForensicObject.cpp)

			fDistPct = (fDist <= 0.0f) ? 1.0f : ( 1.0f - ( fDist / fMaxDist ) );
		}
	}

	return LTCLAMP(fDistPct, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------

void WeaponDisplaySpectrometer::Render()
{
}

//-----------------------------------------------------------------------------
// WeaponDisplayRenderTargetOverlay.
//-----------------------------------------------------------------------------

WeaponDisplayRenderTargetOverlay::WeaponDisplayRenderTargetOverlay() : WeaponDisplay()
{
}

//-----------------------------------------------------------------------------

bool WeaponDisplayRenderTargetOverlay::Init(CWeaponModelData* pParent, HRECORD hDisplay)
{
	if( !WeaponDisplay::Init(pParent,hDisplay) )
		return false;

	// Setup the overlay that will be associated with the render target

	const char* pszOverlayFXName = g_pLayoutDB->GetString(hDisplay,WDDB_sWDOverlayFX);
	if(!pszOverlayFXName)
		return false;

	m_pClientFXMgr = g_pGameClientShell->GetRenderTargetClientFXMgr(m_hRenderTarget,true);
	if(!m_pClientFXMgr)
		return false;

	// Create a local ClientFX object (the FxEdit kind)
	CLIENTFX_CREATESTRUCT cCFXCS(pszOverlayFXName, FXFLAG_LOOP, pParent->m_hObject);
	if (!m_pClientFXMgr->CreateClientFX(&m_linkClientFX, cCFXCS, true))
		return false;

	return true;
}

//-----------------------------------------------------------------------------

void WeaponDisplayRenderTargetOverlay::Term()
{
	// get the overlay client fx manager and shut it down.
	g_pGameClientShell->ShutdownRenderTargetClientFXMgr(m_hRenderTarget);

	WeaponDisplay::Term();
}

//-----------------------------------------------------------------------------

void WeaponDisplayRenderTargetOverlay::Render()
{
	WeaponDisplay::Render();
	if(m_pClientFXMgr)
		m_pClientFXMgr->RenderOverlays();
}

//-----------------------------------------------------------------------------

bool WeaponDisplayRenderTargetOverlay::CreateRenderTarget()
{
	bool Result = WeaponDisplay::CreateRenderTarget();
	if( m_linkClientFX.IsValid() )
		m_linkClientFX.GetInstance()->Show();
	return Result;
}

//-----------------------------------------------------------------------------

bool WeaponDisplayRenderTargetOverlay::ReleaseRenderTarget()
{
	if( m_linkClientFX.IsValid() )
		m_linkClientFX.GetInstance()->Hide();
	return WeaponDisplay::ReleaseRenderTarget();
}
