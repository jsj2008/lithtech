// ----------------------------------------------------------------------- //
//
// MODULE  : HUDOverlay.cpp
//
// PURPOSE : HUDItem to display crosshair
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDOverlay.h"
#include "HUDMgr.h"
#include "ResourceExtensions.h"
#include "ClientDB.h"
#include <algorithm>

//******************************************************************************************
//** HUD overlay
//******************************************************************************************
CHUDOverlay::CHUDOverlay() :
	m_bExclusive(false)
{
	m_nZOrder = 0;
}

bool CHUDOverlay::Init(HRECORD hOverlayRec)
{
	LTASSERT(hOverlayRec,"Invalid overlay record.");

	if (!hOverlayRec)
		return false;
	m_sName = g_pLTDatabase->GetRecordName(hOverlayRec);
	m_Anim.GetAnimationData().rnBaseRect = g_pLayoutDB->GetRect(hOverlayRec,"ScreenRect");
	m_Anim.GetAnimationData().bCache = g_pLayoutDB->GetBool(hOverlayRec,"Cache");
	m_Anim.GetAnimationData().pszMaterialName = g_pLayoutDB->GetString(hOverlayRec,"Mask");

	m_nZOrder = g_pLayoutDB->GetInt32( hOverlayRec, "ZOrder" );

	return true;
}

//******************************************************************************************
//** HUD overlay mgr
//******************************************************************************************
CHUDOverlayMgr::CHUDOverlayMgr()
{
	m_UpdateFlags = kHUDNone;
}
	

// Callback function to sort the overlays by zorder.
static bool SortOverlayCallback( CHUDOverlay const* pLhs, CHUDOverlay const* pRhs )
{
	return ( pLhs->m_nZOrder > pRhs->m_nZOrder );
}

bool CHUDOverlayMgr::Init()
{
	uint32 i = 0;

	ClientDB& clientDB = ClientDB::Instance();
	uint32 nNumOverlays = g_pLTDatabase->GetNumRecords( clientDB.GetOverlayCategory( ));
	m_vecOverlays.reserve(nNumOverlays);

	for (i=0; i< nNumOverlays; i++)
	{
		HRECORD hRec = g_pLTDatabase->GetRecordByIndex( clientDB.GetOverlayCategory( ), i );
		if (hRec)
		{
			CHUDOverlay* pOverlay = debug_new(CHUDOverlay);
			m_vecOverlays.push_back(pOverlay);
			pOverlay->Init(hRec);
		}
	}

	// Sort the list.
	std::sort( m_vecOverlays.begin( ), m_vecOverlays.end( ), SortOverlayCallback );

	return true;
}

void CHUDOverlayMgr::Term()
{
	OverlayList::iterator iter = m_vecOverlays.begin();
	while (iter != m_vecOverlays.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_vecOverlays.clear();
}

void CHUDOverlayMgr::Render()
{
	SetRenderState();
	bool bDrawnExclusive = false;

	//step through the overlays and render them.
	OverlayList::iterator iter = m_vecOverlays.begin();
	for( ; iter != m_vecOverlays.end(); iter++ )
	{
		CHUDOverlay* pOverlay = *iter;

		if ( pOverlay->m_Anim.IsVisible())
		{
			//make sure only one exclusive overlay is drawn
			if (pOverlay->m_bExclusive)
			{
				//this is an exlusive overlay, but we haven't drawn one yet
				if (!bDrawnExclusive)
				{
					pOverlay->m_Anim.Render();
					bDrawnExclusive = true;
				}
			}
			else
			{
				pOverlay->m_Anim.Render();
			}
		}
	}
}


void CHUDOverlayMgr::Update()
{
}


void CHUDOverlayMgr::ScaleChanged()
{
	OverlayList::iterator iter = m_vecOverlays.begin();
	while (iter != m_vecOverlays.end())
	{
		if ((*iter)->m_Anim.IsVisible())
		{
			(*iter)->m_Anim.SetScale(g_pInterfaceResMgr->GetScreenScale());
		}
		iter++;
	}
};

CHUDOverlay* CHUDOverlayMgr::GetHUDOverlay( char const* pszName ) const
{
	if( !pszName || !pszName[0] )
		return NULL;

	for( OverlayList::const_iterator iter = m_vecOverlays.begin(); iter != m_vecOverlays.end( ); iter++ )
	{
		if (LTStrICmp(pszName,(*iter)->m_sName.c_str()) == 0)
		{
			return (*iter);
		}
	}

	return NULL;
}


void CHUDOverlayMgr::Show( CHUDOverlay* pOverlay )
{
	if( !pOverlay )
		return;

	if(!pOverlay->m_Anim.IsInitted())
		pOverlay->InitAnim();
	pOverlay->m_Anim.Show(true);
}

void CHUDOverlayMgr::Hide( CHUDOverlay* pOverlay )
{
	if( !pOverlay )
		return;

	pOverlay->m_Anim.Show(false);
}

bool CHUDOverlayMgr::IsVisible( CHUDOverlay* pOverlay )
{
	if( !pOverlay )
		return false;

	return pOverlay->m_Anim.IsVisible();
}

uint32 CHUDOverlayMgr::GetColor( CHUDOverlay* pOverlay ) const
{
	if( !pOverlay )
		return 0;

	return pOverlay->m_Anim.GetColor();
}

void CHUDOverlayMgr::SetColor( CHUDOverlay* pOverlay, uint32 nColor )
{
	if( !pOverlay )
		return;

	pOverlay->m_Anim.SetColor( nColor );
}


// Retrieve the material instance of the specified overlay...
HMATERIAL CHUDOverlayMgr::GetMaterialInstance( CHUDOverlay* pOverlay )
{
	if( !pOverlay )
		return NULL;

	if(!pOverlay->m_Anim.IsInitted())
		pOverlay->InitAnim();

	return pOverlay->m_Anim.GetMaterialInstance();
}