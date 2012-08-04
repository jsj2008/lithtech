
#include "stdafx.h"

#include "LightGroupFX.h"

static uint32 s_LG;

CLightGroupFXMgr::CLightGroupFXMgr()
{
}

CLightGroupFXMgr::~CLightGroupFXMgr()
{
}

void CLightGroupFXMgr::HandleSFXMsg(ILTMessage_Read *pMsg)
{
	// Get the lightgroup ID
	uint32 nLGID = pMsg->Readuint32();

	// Get the color adjustment
	LTVector vColorAdj = pMsg->ReadLTVector();

	// Change it
	if (!ChangeLGColor(nLGID, vColorAdj))
	{
		// If we couldn't change it, put it on the list for later processing
		m_aWaitingAdjList.push_front(SWaitingAdj(nLGID, vColorAdj));
	}
}

void CLightGroupFXMgr::Update()
{
	if (m_aWaitingAdjList.empty())
		return;

	// Try to clear items out of our waiting list
	TWaitingAdjList::iterator iCurAdj = m_aWaitingAdjList.begin();
	while (iCurAdj != m_aWaitingAdjList.end())
	{
		TWaitingAdjList::iterator iNextAdj = iCurAdj;
		++iNextAdj;
		if (ChangeLGColor(iCurAdj->m_nID, iCurAdj->m_vAdj))
			m_aWaitingAdjList.erase(iCurAdj);
		iCurAdj = iNextAdj;
	}
}


bool CLightGroupFXMgr::ChangeLGColor(uint32 nID, const LTVector &vAdj)
{
	// Get the light's original color
	LTVector vLightColor;
	TLGColorMap::iterator iLGBaseColor = m_cColorMap.find(nID);

	s_LG = nID;

	if (iLGBaseColor == m_cColorMap.end())
	{
		// Find out what color the engine thinks this lightgroup should be
		if (g_pLTClient->GetLightGroupColor(nID, &vLightColor) != LT_OK)
		{
			// This happens when the client gets the request for the light color
			// change before the world has been loaded on the client
			return false;
		}
		m_cColorMap[nID] = vLightColor;
	}
	else
		vLightColor = iLGBaseColor->second;

	// Get the new color
	LTVector vNewColor = vLightColor * vAdj;

//	g_pLTClient->SetLightGroupColor(nID, LTVector( 0.0f, 1.0f, 0.0f ) );

	// Tell the engine
	g_pLTClient->SetLightGroupColor(nID, vNewColor);

	return true;
}

void CLightGroupFXMgr::Clear( )
{
	// Forget what we were waiting on
	m_aWaitingAdjList.clear();
	// Go through our known colors and put them back to their starting values
	// Note : This is very important because of the possibility of SFX messages
	// coming through between level loads when re-loading the same level.
	TLGColorMap::const_iterator iCurLG = m_cColorMap.begin();
	for (; iCurLG != m_cColorMap.end(); ++iCurLG)
	{
		g_pLTClient->SetLightGroupColor(iCurLG->first, iCurLG->second);
	}
	// Clear it
	m_cColorMap.clear();
}