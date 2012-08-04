#include "stdafx.h"
#include "ClientFXVertexDeclMgr.h"

//-------------------------------------------------------------------------
// Global instance
//-------------------------------------------------------------------------
CClientFXVertexDeclMgr g_ClientFXVertexDecl;

//-------------------------------------------------------------------------
// CClientFXVertexDeclMgr
//-------------------------------------------------------------------------
CClientFXVertexDeclMgr::CClientFXVertexDeclMgr() :
	m_hTexTangentSpace(NULL)
{
}

CClientFXVertexDeclMgr::~CClientFXVertexDeclMgr()
{
	Term();
}

//called to initialize this object. This will return true if all vertex formats
//were properly initialized
bool CClientFXVertexDeclMgr::Init()
{
	//make sure we are properly cleaned up
	Term();

	//create our vertex declaration for the textured tangent space format if we have the
	//game client around
	if(g_pLTClient)
	{
		SVertexDeclElement VertexDecl[] =
		{
			{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
			{ 0, eVertexDeclDataType_PackedColor, eVertexDeclUsage_Color, 0 },
			{ 0, eVertexDeclDataType_Float2, eVertexDeclUsage_TexCoord, 0 },
			{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Normal, 0 },
			{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Tangent, 0 },
			{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Binormal, 0 }
		};

		g_pLTClient->GetCustomRender()->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hTexTangentSpace);

		if(!m_hTexTangentSpace)
		{
			Term();
			return false;
		}
	}

	//we have successfully created all vertex formats, return success
	return true;
}

//called to destroy all of the objects associated with this
void CClientFXVertexDeclMgr::Term()
{
	if(m_hTexTangentSpace)
	{
		g_pLTClient->GetCustomRender()->ReleaseVertexDeclaration(m_hTexTangentSpace);
		m_hTexTangentSpace = NULL;
	}
}