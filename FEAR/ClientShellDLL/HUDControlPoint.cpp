// ----------------------------------------------------------------------- //
//
// MODULE  : HUDControlPoint.cpp
//
// PURPOSE : HUDItem to display control point status
//
// CREATED : 01/20/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDControlPoint.h"
#include "HUDMgr.h"
#include "ControlPointSFX.h"



CHUDControlPoint::CHUDControlPoint()
{
	m_nTeamID = INVALID_TEAM;
	m_eLevel = kHUDRenderDead;
}

bool CHUDControlPoint::Init()
{
	SetUseBasePosFromLayout(false);

	UpdateLayout();

	m_Text.SetDropShadow(2);

	ScaleChanged();

	SetSourceString(LoadString("HUD_Ammo_Chars"));

	return true;
}


void CHUDControlPoint::Render()
{
	SetRenderState();

	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}

	m_Text.Render();

}

void CHUDControlPoint::Update()
{
}

void CHUDControlPoint::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDControlPoint");
	}

	CHUDItem::UpdateLayout();

	for (uint32 i = 0; i < 3; ++i)
	{
		m_cTeamColor[i] = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,i);
		m_hTeamIcon[i].Load( g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,i) );
	}

}

void CHUDControlPoint::SetIndex(uint16 nIndex)
{
	if (nIndex >= MAX_CONTROLPOINT_OBJECTS)
	{
		m_Text.SetText(L"");
		m_nIndex = 0;
	}
	else
	{
		wchar_t wsText[8] = L"";
		LTSNPrintF(wsText,LTARRAYSIZE(wsText),L"%d",nIndex);
		m_Text.SetText(wsText);
		m_nIndex = nIndex;
	}
}

void CHUDControlPoint::SetTeam(uint8 nTeamID)
{
	uint8 nLocalID = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalTeam( );

	if (nTeamID == INVALID_TEAM)
	{
		m_hIconTexture = m_hTeamIcon[1];
		m_cIconColor = m_cTeamColor[1];
	}
	else
	{
		if (nTeamID == nLocalID)
		{
			m_hIconTexture = m_hTeamIcon[0];
			m_cIconColor = m_cTeamColor[0];
		}
		else
		{
			m_hIconTexture = m_hTeamIcon[2];
			m_cIconColor = m_cTeamColor[2];
		}
	}

	m_Text.SetColor(m_cTextColor);
	DrawPrimSetRGBA(m_IconPoly,m_cIconColor);

	if (nTeamID != m_nTeamID)
	{
		m_nTeamID = nTeamID;
		Flash("CPStatus");
	}

}


CHUDControlPointList::CHUDControlPointList()
{
	m_eLevel = kHUDRenderDead;
}


bool CHUDControlPointList::Init()
{

	m_delegatePlayerChangedTeamsEvent.Attach( this, g_pInterfaceMgr->GetClientInfoMgr( ), g_pInterfaceMgr->GetClientInfoMgr( )->PlayerChangedTeamsEvent );

	m_UpdateFlags = kHUDControlPoint;
	m_nActivePoints = 0;

	UpdateLayout();

	for (uint16 i = 0; i < LTARRAYSIZE(m_ControlPoints); ++i )
	{
		m_ControlPoints[i].Init();
	}

	return true;
}
void CHUDControlPointList::Term()
{
	m_delegatePlayerChangedTeamsEvent.Detach();
}

void CHUDControlPointList::Render()
{
	if (m_nActivePoints <= 0)
	{
		return;
	}


	for (uint16 i = 0; i < m_nActivePoints; ++i )
	{
		m_ControlPoints[i].Render();
	}

}
void CHUDControlPointList::Update()
{
	//TODO: early out for game mode
	uint16 nSize = ControlPointSFX::GetControlPointSFXList().size();
	if (nSize != m_nActivePoints)
	{
		m_nActivePoints = nSize;

		//calculate how many full rows there are
		uint16 nNumRows = m_nActivePoints / m_nMaxPerRow;
		//add one if there are is a partial row
		if (m_nActivePoints % m_nMaxPerRow)
		{
			nNumRows++;
		}

		//place all of the full rows
		LTVector2n vPos = m_vBasePos;

		//step through all of the rows
		uint16 nIndex = 0;
		for (uint16 nRow = 0; nRow < nNumRows; ++nRow )
		{
			
			//calculate how many columns to use
			uint16 nNumCols = m_nActivePoints / nNumRows;
			//check for extra columns in this row
			if (nRow < m_nActivePoints % nNumRows)
			{
				nNumCols++;
			}

			//go to the far left position for this row
			vPos.x = m_vBasePos.x - ((nNumCols-1) * m_vPointSize.x ) / 2;

			//step through the columns
			for (uint16 nCol = 0; nCol < nNumCols; ++nCol )
			{
				//place and update the icon in the column
				m_ControlPoints[nIndex].SetBasePos(vPos);

				ControlPointSFX* pCP = ControlPointSFX::GetControlPointSFXList()[nIndex];
				if (pCP)
				{
					m_ControlPoints[nIndex].SetIndex(pCP->GetCS().m_nControlPointId);
					//slide the position to the right for the next column
					vPos.x += m_vPointSize.x;

					nIndex++;
				}

				
			}


			//move the position down for the next row
			vPos.y += m_vPointSize.y;

		}


	}

	if (m_nActivePoints <= 0)
	{
		return;
	}

	for (uint16 i = 0; i < m_nActivePoints; ++i )
	{
		ControlPointSFX* pCP = ControlPointSFX::GetControlPointSFXList()[i];
		if (pCP)
		{
			switch(pCP->GetCS().m_eControlPointState)
			{
			case kControlPointState_Team0:
				m_ControlPoints[i].SetTeam(0);
				break;
			case kControlPointState_Team1:
				m_ControlPoints[i].SetTeam(1);
				break;
			default:
				m_ControlPoints[i].SetTeam(INVALID_TEAM);
			};
		}
		m_ControlPoints[i].Update();
	}


}

void CHUDControlPointList::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDControlPointList");
	}

	CHUDItem::UpdateLayout();

	m_vPointSize = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);
	m_nMaxPerRow = static_cast< uint16 >(g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0));
	if (!m_nMaxPerRow)
	{
		LTERROR("CHUDControlPointList::UpdateLayout() - m_nMaxPerRow == 0");
		m_nMaxPerRow = MAX_CONTROLPOINT_OBJECTS;
	}

}

void CHUDControlPointList::OnExitWorld()
{
	for (uint16 i = 0; i < m_nActivePoints; ++i )
	{
		m_ControlPoints[i].OnExitWorld();
	}
	m_nActivePoints = 0;
}

void CHUDControlPointList::UpdateFlash()
{
	for (uint16 i = 0; i < m_nActivePoints; ++i )
	{
		m_ControlPoints[i].UpdateFlash();
	}
}

void CHUDControlPointList::ScaleChanged()
{
	for (uint16 i = 0; i < m_nActivePoints; ++i )
	{
		m_ControlPoints[i].ScaleChanged();
	}

}
