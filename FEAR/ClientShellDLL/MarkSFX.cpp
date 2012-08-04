// ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.cpp
//
// PURPOSE : Mark special FX - Implementation
//
// CREATED : 10/13/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MarkSFX.h"
#include "iltclient.h"
#include "ltlink.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "AccuracyMgr.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarShowMarks;
VarTrack	g_cvarMarkJoinRadius;
VarTrack	g_cvarMaxMarksInRegion;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

bool CMarkSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return false;

	CSpecialFX::Init(psfxCreateStruct);

	MARKCREATESTRUCT* pMark = (MARKCREATESTRUCT*)psfxCreateStruct;

	m_hParent		= pMark->m_hParent;
	m_tTransform	= pMark->m_tTransform;
	m_hAmmo			= pMark->hAmmo;
	m_nSurfaceType	= pMark->nSurfaceType;

	if (!g_cvarShowMarks.IsInitted())
	{
        g_cvarShowMarks.Init(g_pLTClient, "MarkShow", NULL, 1.0f);
	}

	if(!g_cvarMarkJoinRadius.IsInitted())
	{
		g_cvarMarkJoinRadius.Init(g_pLTClient, "MarkJoinRadius", NULL, 60.0f);
	}

	if(!g_cvarMaxMarksInRegion.IsInitted())
	{
		g_cvarMaxMarksInRegion.Init(g_pLTClient, "MaxMarksInRegion", NULL, 10.0f);
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateMarkFX()
//
//	PURPOSE:	Creates a mark based on surface.
//
// ----------------------------------------------------------------------- //

static bool CreateMarkFX( SurfaceType eSurfType, HAMMO hAmmo, HOBJECT hParent, LTRigidTransform const& tTransform,
						 CClientFXLink& markFx ) 
{
	if (!g_pWeaponDB || !ShowsClientFX(eSurfType))
	{
		return false;
	}

#ifndef _FINAL
	// Make special mark if we're debugging perturb.
	if ( CAccuracyMgr::Instance().s_vtDebugPerturb.GetFloat( ) > 0.0f)
	{
		CLIENTFX_CREATESTRUCT fxInit( "Impact_Debug", 0, hParent, tTransform );
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &markFx, fxInit, true );
		return true;
	}
#endif // _FINAL

	// Make sure the ammo is supposed to make a mark.
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo); 
	const char *pszSurfaceFXType = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sSurfaceFXType );
	if( !hAmmo || !pszSurfaceFXType[0] )
		return false;

	HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurfType);
	if( !hSurf )
		return false;

	HSRF_IMPACT hSWF = g_pSurfaceDB->GetSurfaceImpactFX(hSurf , pszSurfaceFXType );
	if( !hSWF )
		return false;

	uint32 nMarkFxCount = g_pSurfaceDB->GetNumValues(hSWF,SrfDB_Imp_sMarkFX);
	for(uint32 nMarkFx=0;nMarkFx<nMarkFxCount;++nMarkFx)
	{
		char const* pszMarkFXName = g_pSurfaceDB->GetString(hSWF,SrfDB_Imp_sMarkFX, nMarkFx);
		if( LTStrEmpty(pszMarkFXName) )
			continue;

		// Make the markfx.
		CLIENTFX_CREATESTRUCT fxInit( pszMarkFXName, 0, hParent, tTransform );
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &markFx, fxInit, true );
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

bool CMarkSFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) 
		return false;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) 
		return false;

	// If we're not showing marks, don't bother...
	if (!g_cvarShowMarks.GetFloat()) 
		return false;

    // Before we create a new bullet hole see if there is already another
	// bullet hole close by that we could use instead...
	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_MARK_ID);
    if (!pList) 
		return false;

	int nNumMarks = pList->GetNumItems();

	float fMarkJoinRadiusSqr	= g_cvarMarkJoinRadius.GetFloat() * g_cvarMarkJoinRadius.GetFloat();
    float fClosestMarkDistSqr	= fMarkJoinRadiusSqr;
	uint32 nMaxInRegion			= (uint32)(g_cvarMaxMarksInRegion.GetFloat() + 0.5f);
    uint32 nNumInRegion			= 0;
	CMarkSFX* pClosestMark		= NULL;

	for (int i=0; i < nNumMarks; i++)
	{
		//see if we have a mark, and if so, if it is attached to the same object as us
		CMarkSFX* pMark= (CMarkSFX*)(*pList)[i];
		if (pMark && (pMark->m_hParent == m_hParent))
		{
			CClientFXLink* pFxLink = pMark->GetClientFXLink();

			if(pFxLink && pFxLink->IsValid())
			{
				//it is attached to the same object as us, so make sure it is close enough to us
				float fDistSqr = (m_tTransform.m_vPos - pMark->m_tTransform.m_vPos).MagSqr();

				if (fDistSqr < fMarkJoinRadiusSqr)
				{
					if (fDistSqr < fClosestMarkDistSqr)
					{
						fClosestMarkDistSqr = fDistSqr;
						pClosestMark = pMark;
					}

					if (++nNumInRegion > nMaxInRegion && pClosestMark)
					{
						// Just move this mark to the correct pos, and
						// remove thyself...
						pFxLink->GetInstance()->SetParent(m_hParent, INVALID_MODEL_NODE, INVALID_MODEL_SOCKET, m_tTransform);
						return false;
					}
				}
			}
		}
	}


	// Need to create a new mark...
	if( !CreateMarkFX( (SurfaceType)m_nSurfaceType, m_hAmmo, m_hParent, m_tTransform, m_fxMark ))
	{
        return false;
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Update
//
//	PURPOSE:	Remove the mark if it's no longer valid
//
// ----------------------------------------------------------------------- //

bool CMarkSFX::Update()
{
	if (!g_cvarShowMarks.GetFloat() || m_bWantRemove)
	{
		// Remove the object...
		return false;
	}

	return (m_fxMark.IsValid());
}