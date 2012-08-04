// ----------------------------------------------------------------------- //
//
// MODULE  : HUDNavMarkerMgr.h
//
// PURPOSE : HUDItem to manage display of multiple NavMarkers
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDNavMarkerMgr.h"
#include "HUDNavMarker.h"
#include "NavMarkerTypeDB.h"

CHUDNavMarkerMgr::CHUDNavMarkerMgr()
{
	m_UpdateFlags = kHUDFrame;
	m_pArrowMarker = NULL;
	m_hRangeTexture = NULL;
	m_eLevel = kHUDRenderText;
	m_bFirstUpdate = true;
}
CHUDNavMarkerMgr::~CHUDNavMarkerMgr()
{
	Term();
	g_pNavMarkerMgr = NULL;
}

bool CHUDNavMarkerMgr::Init()
{
	m_ActiveMarkers.reserve(8);

	DebugCPrint(0,"%s - m_bMultiplayerFilter = false",__FUNCTION__);
	m_bMultiplayerFilter = false;

	CFontInfo rangeFont(g_pLayoutDB->GetHUDFont(),g_pNavMarkerTypeDB->GetRangeSize());

	static std::wstring srcStr(L"0123456789.m");
	if (m_hRangeTexture)
	{
		LTRESULT res = g_pTextureString->RecreateTextureString(m_hRangeTexture,srcStr.c_str(),rangeFont);
		if (res != LT_OK)
		{
			g_pTextureString->ReleaseTextureString(m_hRangeTexture);
			m_hRangeTexture = NULL;
		}
	}
	else
	{
		m_hRangeTexture = g_pTextureString->CreateTextureString(srcStr.c_str(),rangeFont);
	}

	return true;
}
void CHUDNavMarkerMgr::Term()
{
	RemoveAllMarkers();
	if (m_hRangeTexture)
	{
		g_pTextureString->ReleaseTextureString(m_hRangeTexture);
		m_hRangeTexture = NULL;
	}


}

void CHUDNavMarkerMgr::Render()
{
	SetRenderState();

	m_pArrowMarker = NULL;

	MarkerArray::iterator iter = m_ActiveMarkers.begin();
	while (iter != m_ActiveMarkers.end())
	{
		CHUDNavMarker* pMarker = (*iter);
		pMarker->Render();
		// should this marker draw an arrow?
		if (pMarker->IsActive() && !pMarker->IsOnScreen() && pMarker->GetPriority() > 0)
		{
			//check to see if the marker should be faded in MP
			if (!IsMultiplayerGameClient() || !MultiplayerFilter() || pMarker->GetFadeAngle() <= 0.0f)
			{
				if (pMarker->GetPriority() == 0xFF) 
				{
					pMarker->RenderArrow();
				}
				//check to see if this arrow has a higher priority than the current one slated to draw an arrow
				else if (!m_pArrowMarker ||												//if we don't have one already
					m_pArrowMarker->GetPriority() < pMarker->GetPriority() ||		//or this one is higer priority
					(m_pArrowMarker->GetPriority() == pMarker->GetPriority() &&	//or they have the same priority
					m_pArrowMarker->GetRange() > pMarker->GetRange())			// and this one is closer
					)
				{
					m_pArrowMarker = pMarker;
				}
			}

		}
		iter++;
	}

	if (m_pArrowMarker)
		m_pArrowMarker->RenderArrow();

}
void CHUDNavMarkerMgr::Update()
{
	if (m_bFirstUpdate)
	{
		bool bNavFilter = g_pProfileMgr->GetCurrentProfile()->m_bFilterNavMarkers;
		SetMultiplayerFilter(bNavFilter);
		m_bFirstUpdate = false;
	}
	MarkerArray::iterator iter = m_ActiveMarkers.begin();
	while (iter != m_ActiveMarkers.end())
	{
		(*iter)->Update();
		iter++;
	}

}

void CHUDNavMarkerMgr::AddMarker(CHUDNavMarker* pHUDNM)
{
	if (!pHUDNM) return;

	if (m_bMultiplayerFilter)
	{
		pHUDNM->SetAlpha(0.0f);
	}
	else
	{
		pHUDNM->SetAlpha(1.0f);
	}


	m_ActiveMarkers.push_back(pHUDNM);
}

void CHUDNavMarkerMgr::RemoveMarker(CHUDNavMarker* pHUDNM)
{
	MarkerArray::iterator iter = m_ActiveMarkers.begin();
	while (iter != m_ActiveMarkers.end() )
	{
		if ((*iter) == pHUDNM)
		{
			m_ActiveMarkers.erase(iter);
			return;
		}
		else
			iter++;
	}
}

void CHUDNavMarkerMgr::RemoveAllMarkers()
{
	m_ActiveMarkers.clear();
}

void CHUDNavMarkerMgr::ScaleChanged()
{
	MarkerArray::iterator iter = m_ActiveMarkers.begin();
	while (iter != m_ActiveMarkers.end())
	{
		(*iter)->ScaleChanged();
		iter++;
	}

}

void CHUDNavMarkerMgr::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	MarkerArray::iterator iter = m_ActiveMarkers.begin();
	while (iter != m_ActiveMarkers.end())
	{
		(*iter)->SetAlpha(m_fFlicker*m_fFlicker);
		iter++;
	}

}

void CHUDNavMarkerMgr::SetMultiplayerFilter(bool bOn) 
{ 
	//if we're turning it off, make sure they are all visible
	if (m_bMultiplayerFilter && !bOn)
	{
		MarkerArray::iterator iter = m_ActiveMarkers.begin();
		while (iter != m_ActiveMarkers.end())
		{
			(*iter)->SetAlpha(1.0f);
			iter++;
		}
	}
	DebugCPrint(0,"%s - m_bMultiplayerFilter = %d",__FUNCTION__,bOn);
	m_bMultiplayerFilter = bOn; 
}
