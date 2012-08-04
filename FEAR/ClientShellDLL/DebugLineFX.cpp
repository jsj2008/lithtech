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
#include "iltrenderer.h"
#include "resourceextensions.h"
#include "sys/win/mpstrconv.h"
#include "PlayerCamera.h"
#include "ClientDB.h"

#pragma warning( disable : 4786 )
#include <algorithm>

CDebugLineFX::~CDebugLineFX()
{
	m_pClientDE->GetCustomRender()->ReleaseVertexDeclaration(m_hVertexDecl);
	m_hVertexDecl = NULL;
	m_bStrVisible = false;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::CreateObject
//
//	PURPOSE:	Creates the object and sends a message to the server for the data.
//
// ----------------------------------------------------------------------- //
bool CDebugLineFX::CreateObject(ILTClient* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) 
		return false;

	LTVector vPos;
	LTRotation rRot;

	vPos.Init();
	rRot.Init();

	if (m_hServerObject)
	{
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
	}
	
	// Setup the LineSystem...
	ObjectCreateStruct createStruct;
	createStruct.m_ObjectType	= OT_CUSTOMRENDER;
	createStruct.m_Flags		= FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_NOTINWORLDTREE;
	createStruct.m_Flags2		= FLAG2_FORCETRANSLUCENT;
	createStruct.m_Pos			= vPos;
    createStruct.m_Rotation		= rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	// Get the material name out of the database.
	ClientDB& ClientDatabase = ClientDB::Instance();
	const char* const pszMaterialName = ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_DebugLineSystemMaterial );
	HMATERIAL hMaterial = m_pClientDE->GetRenderer()->CreateMaterialInstance( pszMaterialName );
	LTASSERT_PARAM1( hMaterial, "Failed to DebugLineSystem Material: %s", (pszMaterialName ? pszMaterialName : "<null>") );

	m_pClientDE->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	m_pClientDE->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//create our vertex declaration
	SVertexDeclElement VertexDecl[] =
	{
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
		{ 0, eVertexDeclDataType_PackedColor, eVertexDeclUsage_Color, 0 },
		{ 0, eVertexDeclDataType_Float2, eVertexDeclUsage_TexCoord, 0 }
	};
	
	m_pClientDE->GetCustomRender()->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hVertexDecl);
	m_pClientDE->GetCustomRender()->SetRenderCallback(m_hObject, RenderLineSystem);
	m_pClientDE->GetCustomRender()->SetCallbackUserData(m_hObject, this);
	m_pClientDE->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);

	//setup the string
	m_nFontHeight = 12;
	m_Str.SetFont(CFontInfo(g_pLayoutDB->GetHUDFont(), m_nFontHeight ));
	m_Str.SetAlignment(kCenter);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::OnServerMessage
//
//	PURPOSE:	Read an update message from the server.
//
// ----------------------------------------------------------------------- //
bool CDebugLineFX::OnServerMessage(ILTMessage_Read * pMsg)
{

	// Read the number of lines.
	const int num_lines = pMsg->Readuint16();

	// See if the server is telling us to clear our old lines.
	m_bClearOldLines = pMsg->Readbool();
	m_bRelative = pMsg->Readbool();

	//update our object space accordingly
	m_pClientDE->GetCustomRender()->SetRenderingSpace(m_hObject, (m_bRelative) ? eRenderSpace_Object : eRenderSpace_World);

	// If we don't have any lines, we want to clear our old lines
	// so that the object will be re-positioned correctly.
	if( lines.empty() )
		m_bClearOldLines = true;

	// Clear the lines from memory.  The lines will be removed from
	// the line system in Update.
	if( m_bClearOldLines )
	{
		lines.clear();
	}

	// Read each line.
	DebugLine new_line;
	LT_LINEG  new_linef;

	//avoid lots of unnecessary allocations and growing
	lines.reserve(lines.size() + num_lines);

	for(int i = 0; i < num_lines; ++i)
	{
		pMsg->ReadType(&new_line);

		DrawPrimSetXYZ(new_linef, 0, VEC_EXPAND(new_line.vSource));
		DrawPrimSetXYZ(new_linef, 1, VEC_EXPAND(new_line.vDest));

		DrawPrimSetRGBA(new_linef, new_line.rgba.r, new_line.rgba.g, new_line.rgba.b, new_line.rgba.a);

		lines.push_back( new_linef );
	}

	char szDebugString[256];
	pMsg->ReadString(szDebugString, LTARRAYSIZE(szDebugString));

	m_vTextPos = pMsg->ReadLTVector();

	m_Str.SetText( MPA2W(szDebugString).c_str() );

	if (num_lines)
	{
		if(m_bRelative)
		{
			//use a bounding sphere for relative geometry since it can change with the object
			LineList::const_iterator it = lines.begin();
			float fRadiusSqr = LTMAX((it->verts[0].pos).MagSqr(), (it->verts[1].pos).MagSqr());
			it++;

			for(; it != lines.end(); it++)
			{
				fRadiusSqr = LTMAX(fRadiusSqr, (it->verts[0].pos).MagSqr());
				fRadiusSqr = LTMAX(fRadiusSqr, (it->verts[1].pos).MagSqr());
			}

			m_pClientDE->GetCustomRender()->SetVisBoundingSphere(m_hObject, LTVector::GetIdentity(), LTSqrt(fRadiusSqr));
		}
		else
		{
			//determine the bounding box around our line system
			LineList::const_iterator it = lines.begin();
			LTVector minV = (it->verts[0].pos).GetMin(it->verts[1].pos);
			LTVector maxV = (it->verts[0].pos).GetMax(it->verts[1].pos);
			it++;

			for(; it != lines.end(); it++)
			{
				minV.Min(it->verts[0].pos);
				minV.Min(it->verts[1].pos);

				maxV.Max(it->verts[0].pos);
				maxV.Max(it->verts[1].pos);
			}

			LTVector vCenter = (maxV + minV) / 2.0f;

			// Ensure that bounding box has some thickness.
			// This ensures that lines drawn on the edge of the world get drawn.
			LTVector vFudge( 10.f, 10.f, 10.f );
			minV -= vFudge;
			maxV += vFudge;

			m_pClientDE->SetObjectPos(m_hObject, vCenter);
			m_pClientDE->GetCustomRender()->SetVisBoundingBox(m_hObject, minV - vCenter, maxV - vCenter);
		}

		uint32 color = SET_ARGB(new_line.rgba.a,new_line.rgba.r,new_line.rgba.g,new_line.rgba.b);
		m_Str.SetColor(color);
	}
	
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Update
//
//	PURPOSE:	If needed, update the cast line (recalculate end point),
//
// ----------------------------------------------------------------------- //
bool CDebugLineFX::Update()
{
	if(!m_hObject || !g_pLTClient || !m_hServerObject) 
		return false;

	if ( IsWaitingForRemove() )
	{
		// No sense in adding lines now.
		return false;
	}

	//the position of our text
	LTVector vTextPos = m_vTextPos;

	if(m_bRelative)
	{
		LTRigidTransform tParentTrans;
		m_pClientDE->GetObjectTransform(m_hServerObject, &tParentTrans);
		m_pClientDE->SetObjectTransform(m_hObject, tParentTrans);

		//our text offset is relative to us
		vTextPos = tParentTrans * m_vTextPos;
	}

	if (!LTStrEmpty(m_Str.GetText()))
	{
		bool bOnScreen = false;
		LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(vTextPos, g_pPlayerMgr->GetPlayerCamera()->GetCamera(), bOnScreen);

		float fMaxRange = GetConsoleFloat("MaxAIDebugRange",2000.0f);
		if (bOnScreen && pos.z > 0.0f && pos.z <= fMaxRange)
		{
			uint32 h = (uint32)(g_pInterfaceResMgr->GetXRatio() * (18.0f - (12.0f * pos.z / fMaxRange) )   );

			if (h != m_nFontHeight)
			{
				m_nFontHeight = h;
				m_Str.SetFontHeight(m_nFontHeight);
			}
			
			m_Str.SetPos( LTVector2( pos.x, pos.y) );
			m_bStrVisible = true;
		}
		else
		{
			m_bStrVisible = false;
		}
	}	
		
	// If we cleared our lines out, then remove ourselves.
	if( m_bClearOldLines && lines.empty( ))
	{
		return false;
	}

	return true;
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
	if(m_bStrVisible)
		m_Str.Render();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebugLineFX::Render
//
//	PURPOSE:	Callback for when rendering the custom render object
//
// ----------------------------------------------------------------------- //
void CDebugLineFX::RenderLineSystem(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	//check parameters
	if(!pInterface || !pUser)
		return;

	//convert our user pointer to a line system
	CDebugLineFX* pLines = (CDebugLineFX*)pUser;

	//ignore empty systems
	if(pLines->lines.empty())
		return;

	//setup our device to handle rendering the lines
	if(pInterface->SetVertexDeclaration(pLines->m_hVertexDecl) != LT_OK)
		return;

	//a rendering line vertex structure
	struct SLineRenderVert
	{
		LTVector	m_vPos;
		uint32		m_nPackedColor;
		LTVector2	m_vUV;
	};

	//determine the largest number of lines we can render in a single list
	uint32 nMaxLinesPerBatch = DYNAMIC_RENDER_VERTEX_STREAM_SIZE / (sizeof(SLineRenderVert) * 2);

	uint32 nNumLines = pLines->lines.size();

	//now break the lines up into batches and render
	for(uint32 nCurrLine = 0; nCurrLine < nNumLines; )
	{
		//determine how many particles are left
		uint32 nNumLeft = nNumLines - nCurrLine;
		uint32 nBatchSize = LTMIN(nNumLeft, nMaxLinesPerBatch);
		uint32 nBatchEnd = nCurrLine + nBatchSize;

		//lock down our buffer for rendering
		SDynamicVertexBufferLockRequest LockRequest;
		if(pInterface->LockDynamicVertexBuffer(nBatchSize * 2, LockRequest) != LT_OK)
			return;

		//fill in a batch of particles
		SLineRenderVert* pCurrOut = (SLineRenderVert*)LockRequest.m_pData;

		for(; nCurrLine < nBatchEnd; nCurrLine++)
		{
			const LT_LINEG& CurrLine = pLines->lines[nCurrLine];

			pCurrOut[0].m_vPos			= CurrLine.verts[0].pos;
			pCurrOut[0].m_nPackedColor	= CurrLine.verts[0].rgba.m_nColor;
			pCurrOut[0].m_vUV.Init(0.0f, 0.0f);

			pCurrOut[1].m_vPos			= CurrLine.verts[1].pos;
			pCurrOut[1].m_nPackedColor	= CurrLine.verts[1].rgba.m_nColor;
			pCurrOut[1].m_vUV.Init(1.0f, 1.0f);

			pCurrOut += 2;
		}

		//unlock and render the batch
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->Render(	eCustomRenderPrimType_LineList, LockRequest.m_nStartIndex, nBatchSize * 2);
	}
}

