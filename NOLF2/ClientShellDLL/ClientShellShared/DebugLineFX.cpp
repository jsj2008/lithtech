// ----------------------------------------------------------------------- //
//
// MODULE  : DebugLineFX.cpp
//
// PURPOSE : DebugLine special FX - Implementation
//
// CREATED : 3/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebugLineFX.h"
#include "ClientUtilities.h"
#include "MsgIDs.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"

#pragma warning( disable : 4786 )
#include <algorithm>

#ifdef _DEBUG
//#define DEBUGLINEFX_DEBUG 
#endif


namespace /* unnamed */
{
	void SetLineDrawingMode(ILTDrawPrim * pDrawPrim)
	{
		ASSERT( pDrawPrim );

		pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
		pDrawPrim->SetZBufferMode(DRAWPRIM_ZRO); 
		pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		pDrawPrim->SetColorOp(DRAWPRIM_NOCOLOROP);
		pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	}

}; // namespace /* unnamed */



CDebugLineFX::~CDebugLineFX()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::CreateObject
//
//	PURPOSE:	Creates the object and sends a message to the server for the data.
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::CreateObject(ILTClient* pClientDE)
{
	if (!CBaseLineSystemFX::CreateObject(pClientDE)) return LTFALSE;

	ASSERT( g_pLTClient );

#ifdef DEBUGLINEFX_DEBUG 
		g_pLTClient->CPrint("%f : Linesystem %d created.", 
			g_pLTClient->GetTime(), GetObject() );
#endif

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,"debug",0.0f, 0.0f);
	m_pStr->SetColor(argbWhite);
	m_pStr->SetAlignmentH(CUI_HALIGN_CENTER);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::OnServerMessage
//
//	PURPOSE:	Read an update message from the server.
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::OnServerMessage(ILTMessage_Read * pMsg)
{

	// Read the number of lines.
	const int num_lines = pMsg->Readuint16();

	// Set the new maximum number of lines
	m_nMaxLines = pMsg->Readuint32();

	// See if the server is telling us to clear our old lines.
	m_bClearOldLines = (pMsg->Readuint8() != 0);

#ifdef DEBUGLINEFX_DEBUG 
		g_pLTClient->CPrint("Reading %d lines, clear lines is %s.",
			num_lines, m_bClearOldLines ? "true" : "false");
#endif

	// If we don't have any lines, we want to clear our old lines
	// so that the object will be re-positioned correctly.
	if( lines.empty() )
		m_bClearOldLines = true;

	// Clear the lines from memory.  The lines will be removed from
	// the line system in Update.
	if( m_bClearOldLines )
	{
#ifdef DEBUGLINEFX_DEBUG 
		g_pLTClient->CPrint("Clearing %d lines.", lines.size());
#endif

		lines.clear();
	}

	// Read each line.
	DebugLine new_line;
	LT_LINEF  new_linef;
	LTVector maxV(0.0f,0.0f,0.0f);
	LTVector minV(0.0f,0.0f,0.0f);
	bool first = true;
	for(int i = 0; i < num_lines; ++i)
	{
		pMsg->ReadType(&new_line);

		new_linef.verts[0].x = new_line.vSource.x;
		new_linef.verts[0].y = new_line.vSource.y;
		new_linef.verts[0].z = new_line.vSource.z;

		new_linef.verts[1].x = new_line.vDest.x;
		new_linef.verts[1].y = new_line.vDest.y;
		new_linef.verts[1].z = new_line.vDest.z;

		new_linef.rgba.r = new_line.rgba.r;
		new_linef.rgba.g = new_line.rgba.g;
		new_linef.rgba.b = new_line.rgba.b;
		new_linef.rgba.a = new_line.rgba.a;

		lines.push_back( new_linef );

		if (first)
		{
			first = false;
			maxV.x = Max(new_line.vSource.x,new_line.vDest.x);
			maxV.y = Max(new_line.vSource.y,new_line.vDest.y);
			maxV.z = Max(new_line.vSource.z,new_line.vDest.z);

			minV.x = Min(new_line.vSource.x,new_line.vDest.x);
			minV.y = Min(new_line.vSource.y,new_line.vDest.y);
			minV.z = Min(new_line.vSource.z,new_line.vDest.z);

		}
		else
		{
			maxV.x = Max(maxV.x,new_line.vSource.x);
			maxV.y = Max(maxV.y,new_line.vSource.y);
			maxV.z = Max(maxV.z,new_line.vSource.z);
			maxV.x = Max(maxV.x,new_line.vDest.x);
			maxV.y = Max(maxV.y,new_line.vDest.y);
			maxV.z = Max(maxV.z,new_line.vDest.z);

			minV.x = Min(minV.x,new_line.vSource.x);
			minV.y = Min(minV.y,new_line.vSource.y);
			minV.z = Min(minV.z,new_line.vSource.z);
			minV.x = Min(minV.x,new_line.vDest.x);
			minV.y = Min(minV.y,new_line.vDest.y);
			minV.z = Min(minV.z,new_line.vDest.z);

		}
	}

	char szDebugString[256];
	pMsg->ReadString(szDebugString,sizeof(szDebugString));
	m_pStr->SetText(szDebugString);

	if (num_lines)
	{
		vStrPos = (maxV + minV) / 2.0f;
		vStrPos.y += 16.0f;
		uint32 color = SET_ARGB(new_linef.rgba.a,new_linef.rgba.r,new_linef.rgba.g,new_linef.rgba.b);
		m_pStr->SetColor(color);
	}

	// Make sure the lines get updated.
	m_bUpdateLines = true;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Update
//
//	PURPOSE:	If needed, update the cast line (recalculate end point),
//
// ----------------------------------------------------------------------- //
LTBOOL CDebugLineFX::Update()
{
	if(!m_hObject || !g_pLTClient || !m_hServerObject) return LTFALSE;

	if ( IsWaitingForRemove() )
	{
		// No sense in adding lines now.
		return LTFALSE;
	}


	// Trim the list to max lines or less (from the front, as they are the oldest).
	if( !lines.empty() && lines.size() > m_nMaxLines )
	{
		const uint32 nNumOver = lines.size() - m_nMaxLines;

		lines.erase(lines.begin(), lines.begin() + nNumOver);
	}

	if (!lines.empty())
	{


		LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(vStrPos, g_pPlayerMgr->GetCamera());

		if (pos.z > 0.0f && pos.z < 900.0f)
		{
			
//			LTVector vScrnPos = g_pInterfaceMgr->GetWorldFromScreenPos(pos,verts.z);

			uint8 h = (uint8)(g_pInterfaceResMgr->GetXRatio() * 5000.0f / (100.0f + pos.z));

			if (h != m_pStr->GetCharScreenHeight())
			{
				m_pStr->SetCharScreenHeight(h);
			}
			
			m_pStr->SetPosition( pos.x, pos.y);
		}
		else
		{
			m_pStr->SetPosition( 2000.0f, -1000.0f);
		}
	}	
		
	// If we cleared our lines out, then remove ourselves.
	if( m_bClearOldLines && lines.empty( ))
	{
		return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Render
//
//	PURPOSE:	Render the line system.
//
// ----------------------------------------------------------------------- //
void CDebugLineFX::Render(HOBJECT hCamera)
{
	// Be sure it is in the right state.
	SetLineDrawingMode(g_pDrawPrim);

	// Draw all the lines.
	if( !lines.empty() )
	{
		g_pDrawPrim->DrawPrim( &lines[0], lines.size() );
		m_pStr->Render();
	}
}