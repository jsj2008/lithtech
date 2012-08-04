// ----------------------------------------------------------------------- //
//
// MODULE  : HUDRadar.cpp
//
// PURPOSE : HUDItem to display radar
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HUDMgr.h"
	#include "HUDRadar.h"
	#include "GameClientShell.h"
	#include "CharacterFX.h"
	#include "RadarTypeMgr.h"

LTPoly_GT4	teamPoly;
uint32 nTeamColors[2][3] = {	{argbBlack,argbBlack,argbBlack},
								{argbBlack,argbBlack,argbBlack} };


float	fFlashSpeed = 0.25f;
uint32	nLiveColor = 0xFFFFFF00;
uint32	nDeadColor = 0xFF808080;
uint32	nTalkColor = 0xFFFFFFFF;
float	fTotalFlashTime = 2.0f;

typedef RADAROBJECT* LPRADAROBJECT;
class RadarObjectLesser
{
public:
	
	bool operator()(const LPRADAROBJECT & x, const LPRADAROBJECT & y) const
	{
		return (x->m_nDrawOrder < y->m_nDrawOrder );
	}
};


#define RADAR_PLAYER_ALIVE_TYPE		"PlayerAlive"
#define RADAR_PLAYER_DEAD_TYPE		"PlayerDead"
#define RADAR_PLAYER_TALK_TYPE		"PlayerTalk"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::CHUDRadar
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDRadar::CHUDRadar()
:	CHUDItem		(),
	m_BasePos		( 0, 0 ),
	m_NamePos		( 0, 0 ),
	m_nBaseSize		( 0 ),
	m_nObjectSize	( 0 ),
	m_bDraw			( false ),
	m_nMaxShowDist	( 0 ),
	m_fNameXRatio	( 0.0f ),
	m_fNameYRatio	( 0.0f )
{
	m_UpdateFlags	= kHUDFrame;
	m_eLevel		= kHUDRenderDead;
	m_fLastRotation	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::CHUDRadar
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CHUDRadar::~CHUDRadar()
{	
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::Term
//
//  PURPOSE:	Release the list of radar objects...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::Term()
{
	Reset();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::Init
//
//  PURPOSE:	Setup the icon and poly we are going to draw...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDRadar::Init()
{
	UpdateLayout();

	g_pDrawPrim->SetRGBA( &teamPoly, SET_ARGB(0x7F,0xFF,0xFF,0xFF));

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::Render
//
//  PURPOSE:	Draw the radar...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::Render()
{
	if( !m_bDraw )
		return;


	SetRenderState();

	RADAROBJECT *pRadarObj = LTNULL;
	RadarObjectSortList::iterator iter;

	uint8 nCount = 0;
	for( iter = m_ROSortList.begin(); iter != m_ROSortList.end(); ++iter )
	{	
		pRadarObj = (*iter);

		// Check if the radar object has team information in it.  Only
		// show on same team's radar.
		if( IsTeamGameType( ))
		{
			// Get local clientinfo.
			CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
			if( !pLocalCI )
				return;

			// Check if the radar is for an opposing player, show them on radar.
			if( pRadarObj->m_nVisibleTeamId != INVALID_TEAM &&
				pLocalCI->nTeamID != pRadarObj->m_nVisibleTeamId )
				continue;
		}

		if( pRadarObj->m_bDraw )
		{
			g_pDrawPrim->SetTexture( pRadarObj->m_hTex );
			g_pDrawPrim->DrawPrim( &pRadarObj->m_Poly );
			++nCount;
		}
	}

	if( IsTeamGameType() )
	{
		g_pDrawPrim->SetTexture( LTNULL );
		g_pDrawPrim->DrawPrim( &teamPoly );
	}


	RadarPlayerList::iterator sIter = m_Players.begin();
	while (sIter != m_Players.end())
	{
		(*sIter)->Render();
		sIter++;
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::Update
//
//  PURPOSE:	Decide if we should draw the radar...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::Update()
{
	// No need to update if we aren't going to render...

	if( !m_bDraw || (m_mapRadarObjects.size() == 0 && m_Players.size() == 0) )
		return;

	//see if we have changed ratios and need to recalculate the name positions
	if(	(m_fNameXRatio != g_pInterfaceResMgr->GetXRatio()) ||
		(m_fNameYRatio != g_pInterfaceResMgr->GetYRatio()))
	{
		UpdateNamePositions();
		m_fNameXRatio = g_pInterfaceResMgr->GetXRatio();
		m_fNameYRatio = g_pInterfaceResMgr->GetYRatio();
	}

	LTVector vLocalPos, vObjPos, vDir;
	g_pLTClient->GetObjectPos( g_pLTClient->GetClientObject(), &vLocalPos );

	float fYaw = g_pPlayerMgr->GetYaw() - MATH_PI;

	if (g_pPlayerMgr->IsCameraAttachedToHead())
	{
		fYaw = m_fLastRotation;
	}
	else
	{
		m_fLastRotation = fYaw;
	}


	LTRotation rRot( 0.0f, fYaw, 0.0f );
	LTMatrix mMat;

	rRot.ConvertToMatrix( mMat );
	mMat = ~mMat;

	RADAROBJECT *pRadarObj = LTNULL;
	HOBJECT		hObj = LTNULL;
	RadarObjectList::iterator iter;

	for( iter = m_mapRadarObjects.begin(); iter != m_mapRadarObjects.end(); ++iter )
	{
		hObj = iter->first;
		pRadarObj = iter->second;
		
		pRadarObj->m_bDraw = false;
		
		g_pLTClient->GetObjectPos( hObj, &vObjPos );
		
		float fDist = vLocalPos.Dist( vObjPos );

		// Focus on all objects within maxshowdist.  All objects father than maxshowdist
		// are pushed to within 1/e^2 of the edge.
		// This will scale fDist between 0 and 1.
		fDist = 1.0f - 1.0f / ( float )( exp( 2.0f * fDist / m_nMaxShowDist ));

		// This will scale fDist within the hud element size.
		fDist = ( fDist * (float)m_nBaseSize / 2.0f);
		
		vDir = vLocalPos - vObjPos;
		MatVMul_InPlace_3x3( &mMat, &vDir );

		vDir.y = 0.0f;
		if( vDir.LengthSquared() > 0.01f )
		{
			vDir.Normalize();
		}
		else
		{
			vDir.Init();
		}

		vDir *= fDist;

		float fx = (float)(m_BasePos.x + vDir.x) * g_pInterfaceResMgr->GetXRatio();
		float fy = (float)(m_BasePos.y - vDir.z) * g_pInterfaceResMgr->GetXRatio();
		float fw = (float)(m_nObjectSize) * g_pInterfaceResMgr->GetXRatio();

		g_pDrawPrim->SetXYWH( &pRadarObj->m_Poly, fx, fy, fw, fw );
		pRadarObj->m_bDraw = true;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::UpdateLayout
//
//  PURPOSE:	Get data for this HUDItem from layouts.txt...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);
	
	// Use the compass values to determine where the radar should go...

	m_BasePos		= g_pLayoutMgr->GetCompassPos( nCurrentLayout );
	m_nBaseSize		= g_pLayoutMgr->GetCompassSize( nCurrentLayout );

	m_NamePos = m_BasePos;
	m_NamePos.x += m_nBaseSize;
	m_NamePos.y += (m_nBaseSize + m_nObjectSize);

	m_nMaxShowDist	= g_pLayoutMgr->GetRadarMaxShowDist( nCurrentLayout );
	m_nObjectSize	= g_pLayoutMgr->GetRadarObjectSize( nCurrentLayout );

	nLiveColor		= g_pLayoutMgr->GetRadarLivePlayerColor(nCurrentLayout);
	nDeadColor		= g_pLayoutMgr->GetRadarDeadPlayerColor(nCurrentLayout);
	nTalkColor		= g_pLayoutMgr->GetRadarTalkPlayerColor(nCurrentLayout);
	fTotalFlashTime	= g_pLayoutMgr->GetRadarFlashTime(nCurrentLayout);


	LTVector vCol = g_pLayoutMgr->GetVector("Scores","Team1Color");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	nTeamColors[0][0]=  SET_ARGB(0x80,nR,nG,nB);
	nTeamColors[0][1]=  SET_ARGB(0x20,nR,nG,nB);
	nTeamColors[0][2]=  SET_ARGB(0x00,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector("Scores","Team2Color");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	nTeamColors[1][0]=  SET_ARGB(0x80,nR,nG,nB);
	nTeamColors[1][1]=  SET_ARGB(0x20,nR,nG,nB);
	nTeamColors[1][2]=  SET_ARGB(0x00,nR,nG,nB);


	int nCenterOffset = int(m_nBaseSize / 2) - int(m_nObjectSize / 2);

	m_BasePos.x += nCenterOffset;
	m_BasePos.y += nCenterOffset;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::AddObject
//
//  PURPOSE:	Add a new object to the radar and display with the specified type...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::AddObject( HOBJECT hObj, uint8 nRadarType, uint8 nVisibleTeamId )
{
	if( !hObj || !g_pRadarTypeMgr )
		return;

	// Don't add the object more than once...

	if( m_mapRadarObjects.find( hObj ) != m_mapRadarObjects.end() )
	{
		g_pLTClient->CPrint("HUDRadar: failed adding object (duplicate)");
		return;
	}

	RADARTYPE *pType = g_pRadarTypeMgr->GetRadarType( nRadarType );
	if( pType && pType->szIcon )
	{
		HTEXTURE hTex = g_pInterfaceResMgr->GetTexture( pType->szIcon );
		if( !hTex )
		{
			g_pLTClient->CPrint("HUDRadar: failed adding object (no texture %s)", pType->szIcon);
			return;
		}
		
		RADAROBJECT *pRadObj = debug_new( RADAROBJECT );
		if( !pRadObj )
			return;

		pRadObj->m_bDraw		= false;
		pRadObj->m_hTex			= hTex;
		pRadObj->m_nDrawOrder	= pType->nDrawOrder;
		pRadObj->m_nVisibleTeamId = nVisibleTeamId;

		g_pDrawPrim->SetRGBA( &pRadObj->m_Poly, argbWhite );
		SetupQuadUVs( pRadObj->m_Poly, hTex, 0.0f, 0.0f, 1.0f, 1.0f );

		m_mapRadarObjects[hObj] = pRadObj;
		m_ROSortList.push_back(pRadObj);
		m_ROSortList.sort(RadarObjectLesser());

	}
	else
	{
		g_pLTClient->CPrint("HUDRadar: failed adding object (unknown type %d)",nRadarType);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::AddObject
//
//  PURPOSE:	Add a new object to the radar and display with the specified type...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::AddObject( HOBJECT hObj, const char *pRadarType, uint8 nVisibleTeamId )
{
	if( !hObj || !pRadarType || !g_pRadarTypeMgr )
		return;

	RADARTYPE *pType = g_pRadarTypeMgr->GetRadarType( pRadarType );
	if( pType )
	{
		AddObject( hObj, pType->nId, nVisibleTeamId );
	}
	else
	{
		g_pLTClient->CPrint("HUDRadar: failed adding object (unknown type %s)",pRadarType);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::RemoveObject
//
//  PURPOSE:	Remove the object from the radar...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::RemoveObject( HOBJECT hObj )
{
	if( !hObj )
		return;

	//if we have a player record for this object, remove it
	RemovePlayer(hObj);

	// Make sure we actually have the object in our list...

	RadarObjectList::iterator iter = m_mapRadarObjects.find( hObj );
	if(  iter == m_mapRadarObjects.end() )
		return;

	m_ROSortList.remove(iter->second);

	debug_delete( iter->second );
	m_mapRadarObjects.erase( iter );


}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::ChangeRadarType
//
//  PURPOSE:	Change the texture used to display the object...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::ChangeRadarType( HOBJECT hObj, const char *pRadarType )
{
	if( !hObj || !pRadarType || !g_pRadarTypeMgr )
		return;

	RadarObjectList::iterator iter = m_mapRadarObjects.find( hObj );
	if( iter == m_mapRadarObjects.end() )
		return;

	RADARTYPE *pType = g_pRadarTypeMgr->GetRadarType( pRadarType );
	if( pType && pType->szIcon )
	{
		HTEXTURE hTex = g_pInterfaceResMgr->GetTexture( pType->szIcon );
		if( !hTex )
			return;
		
		iter->second->m_hTex = hTex;

		iter->second->m_nDrawOrder = pType->nDrawOrder;
		m_ROSortList.sort(RadarObjectLesser());

	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::AddPlayer
//
//  PURPOSE:	Add a player name and icon to the radar...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::AddPlayer(HOBJECT hObj, uint32 nId)
{
	if (!hObj) return;
	CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);
	HOBJECT hLocalObj = g_pLTClient->GetClientObject();


	uint8 nTeamID = INVALID_TEAM;

	// Check for teams and only display players of the same team...
	if( IsTeamGameType() && hLocalObj != hObj)
	{
		CLIENT_INFO *pLocalCI = pCIMgr->GetLocalClient();
		if( !pLocalCI || !pCI )
			return;
	
		if( pLocalCI->nTeamID != pCI->nTeamID )
			return;

		nTeamID = pCI->nTeamID;

	}
	

	bool bDead = false;

	if (hLocalObj != hObj)
	{

		AddObject( hObj, RADAR_PLAYER_ALIVE_TYPE, nTeamID );
		
	}

	CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hObj);
	if (pCharacter && pCharacter->IsPlayerDead())
	{
		bDead = true;
	}


	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  ( (*iter)->hObj != hObj ) )
	{
		iter++;
	}

	RADAR_PLAYER_OBJ* pPlayer = NULL;

	//new player...
	if (iter == m_Players.end())
	{
		pPlayer = debug_new(RADAR_PLAYER_OBJ);
		m_Players.push_back(pPlayer);
	}
	else
	{
		pPlayer = (*iter);
	}

	pPlayer->nID = nId;
	pPlayer->hObj = hObj;
	
	if (!pPlayer->pName)
	{
		uint8 nFont = 0;
		CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
		pPlayer->pName = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f,0.0f);
		pPlayer->pName->SetAlignmentH(CUI_HALIGN_RIGHT);
	}

	if (pCI)
	{
		pPlayer->pName->SetText(pCI->sName.c_str());
	}

	SetPlayerDead(hObj,bDead);
	
	UpdateNamePositions();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::RemovePlayer
//
//  PURPOSE:	Remove a player name from the radar...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::RemovePlayer(HOBJECT hObj)
{
	if (!hObj) return;

	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  ( (*iter)->hObj != hObj ) )
	{
		iter++;
	}

	if (iter != m_Players.end())
	{
		g_pFontManager->DestroyPolyString((*iter)->pName);
		debug_delete(*iter);
		m_Players.erase(iter);
		UpdateNamePositions();
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDRadar::SetPlayerDead
//
//  PURPOSE:	Mark a player's name as dead...
//
// ----------------------------------------------------------------------- //

void CHUDRadar::SetPlayerDead(HOBJECT hObj, bool bDead)
{
	if (!hObj) return;

	if (bDead)
		ChangeRadarType(hObj, RADAR_PLAYER_DEAD_TYPE );
	else
		ChangeRadarType(hObj, RADAR_PLAYER_ALIVE_TYPE );


	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  (*iter)->hObj != hObj )
	{
		iter++;
	}

	if (iter != m_Players.end())
	{
		CUIFormattedPolyString* pStr = (*iter)->pName;
		(*iter)->bDead = bDead;
		if (bDead)
			pStr->SetColor(nDeadColor);
		else
			pStr->SetColor(nLiveColor);
	}
}

void CHUDRadar::UpdatePlayerName( uint32 nId, const char*pName, uint8 nTeamID )
{
	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  (*iter)->nID != nId )
	{
		iter++;
	}

	if (iter != m_Players.end())
	{
		CUIFormattedPolyString* pStr = (*iter)->pName;
		pStr->SetText(pName);


	}
	UpdateNamePositions();
}

void CHUDRadar::SetPlayerTalk( uint32 nId )
{
	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  (*iter)->nID != nId )
	{
		iter++;
	}

	if (iter != m_Players.end())
	{
		(*iter)->fFlashTime = fTotalFlashTime;
	}
}



void CHUDRadar::UpdatePlayerID(HOBJECT hObj, uint32 nId)
{
	if (!hObj) return;
	CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end() &&  (*iter)->hObj != hObj )
	{
		iter++;
	}

	if (iter != m_Players.end())
	{
		(*iter)->nID = nId;

		if (pCI)
		{
			(*iter)->pName->SetText(pCI->sName.c_str());
			UpdateNamePositions();
		}
	}
}


void CHUDRadar::UpdateNamePositions()
{

	float fx = (float)(m_NamePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_NamePos.y) * g_pInterfaceResMgr->GetYRatio();

	if( IsTeamGameType() )
	{
		CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
		CLIENT_INFO *pLocalCI = pCIMgr->GetLocalClient();
		uint8 nTeam = 0;
		if( pLocalCI)
		{
			nTeam = pLocalCI->nTeamID;
		}

		g_pDrawPrim->SetRGBA4( &teamPoly, nTeamColors[nTeam][1], nTeamColors[nTeam][0], nTeamColors[nTeam][1], nTeamColors[nTeam][2] );

		float fw = 90.0f * g_pInterfaceResMgr->GetXRatio();
		float fh = 90.0f * g_pInterfaceResMgr->GetYRatio();

		g_pDrawPrim->SetXYWH( &teamPoly, fx-fw, fy, fw, fh);

	}

		
	RadarPlayerList::iterator iter = m_Players.begin();
	while (iter != m_Players.end())
	{
		CUIFormattedPolyString* pStr = (*iter)->pName;

		if (pStr->GetLength())
		{
			uint8 nSize = (uint8)(12.0f * g_pInterfaceResMgr->GetXRatio());
			pStr->SetCharScreenHeight(nSize);
			
			pStr->SetPosition(fx,fy);
			fy += (float)pStr->GetHeight();
			
		}
		iter++;
	}
}


void CHUDRadar::Reset()
{

	m_ROSortList.clear();

	RadarObjectList::iterator iter;
	for( iter = m_mapRadarObjects.begin(); iter != m_mapRadarObjects.end(); ++iter )
	{
		debug_delete( iter->second );
	}
	m_mapRadarObjects.clear();

	for (uint16 i = 0; i < m_Players.size(); i++)
	{
		g_pFontManager->DestroyPolyString(m_Players[i]->pName);
		debug_delete(m_Players[i]);
	}
	m_Players.clear();
}

void RADAR_PLAYER_OBJ::Render()
{
	if (fFlashTime > 0.0f)
	{
		fFlashTime -= g_pLTClient->GetFrameTime();
		if (fFlashTime < 0.0f)
		{
			bFlashOn = false;
		}
		else
		{
			fCurFlashTime -= g_pLTClient->GetFrameTime();
			if (fCurFlashTime <= 0.0f)
			{
				bFlashOn = !bFlashOn;
				fCurFlashTime = Min(fFlashSpeed,fFlashTime);
			}
		}

		if (bFlashOn)
		{
			g_pRadar->ChangeRadarType(hObj, RADAR_PLAYER_TALK_TYPE );
			pName->SetColor(argbWhite);
		}
		else
		{
			if (bDead)
			{
				g_pRadar->ChangeRadarType(hObj, RADAR_PLAYER_DEAD_TYPE );
				pName->SetColor(nDeadColor);
			}
			else
			{
				g_pRadar->ChangeRadarType(hObj, RADAR_PLAYER_ALIVE_TYPE );
				pName->SetColor(nLiveColor);
			}
		}

	}


	pName->Render();
}