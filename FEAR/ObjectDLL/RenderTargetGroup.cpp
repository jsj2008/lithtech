// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTargetGroup.cpp
//
// PURPOSE : This defines the render target group object which is used to 
//			 specify information about a single render target surface, of
//			 which other render target objects can refer to in order to
//			 have a surface of which to render to.
//
// CREATED : 5/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "RenderTargetGroup.h"

// ----------------------------------------------------------------------- //
// Called to generate a unique ID. This is guaranteed to be less than
// 0xFFFF
// ----------------------------------------------------------------------- //
static uint32 GenerateUniqueGroupID()
{
	static uint32 s_nCurrID = 0;

	//just return our current ID
	uint32 nRV = s_nCurrID;

	//handle incrementing our ID, but wrap around if we hit 0xFFFF, which will
	//be fine as long as a level does not have 0xFFFF render target groups
	//(which is pretty much impossible)
	s_nCurrID = (s_nCurrID + 1) % 0xFFFF;

	return nRV;
}


// ----------------------------------------------------------------------- //
// Default Values
// ----------------------------------------------------------------------- //

#define DEFAULT_WIDTH					128
#define DEFAULT_HEIGHT					128

// ----------------------------------------------------------------------- //
// Class definition
// ----------------------------------------------------------------------- //

LINKFROM_MODULE( RenderTargetGroup );

BEGIN_CLASS(RenderTargetGroup)

	ADD_LONGINTPROP(LowTextureWidth, DEFAULT_WIDTH, "The width of the texture in the X direction for low LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). 0 disables the render target at this group.")
	ADD_LONGINTPROP(LowTextureHeight, DEFAULT_HEIGHT, "The height of the texture in the Y direction for low LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). This is ignored if it is a cube map. 0 disables the render target at this group.")
	ADD_LONGINTPROP(MedTextureWidth, DEFAULT_WIDTH, "The width of the texture in the X direction for medium LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). 0 disables the render target at this group.")
	ADD_LONGINTPROP(MedTextureHeight, DEFAULT_HEIGHT, "The height of the texture in the Y direction for medium LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). This is ignored if it is a cube map. 0 disables the render target at this group.")
	ADD_LONGINTPROP(TextureWidth, DEFAULT_WIDTH, "The width of the texture in the X direction for high LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). 0 disables the render target at this group.")
	ADD_LONGINTPROP(TextureHeight, DEFAULT_HEIGHT, "The height of the texture in the Y direction for high LOD. This should be a power of 2 (i.e. 16, 32, 64, 128, 256, 512). This is ignored if it is a cube map. 0 disables the render target at this group.")

	ADD_BOOLPROP(CubeMap, FALSE, "Determines if this render target is a cubic render target or not. This is ignored if mirror is set to true")
	ADD_BOOLPROP(MipMap, TRUE, "Determines if mip maps should be generated for this render target. This is ignored for mirrors.")
	ADD_BOOLPROP(FogVolumes, FALSE, "Specifies if this render target supports displaying fog volumes.")
	ADD_BOOLPROP(LastFrameEffect, FALSE, "Specifies if this render target supports displaying shaders that use the last frame effect. This is ignored if mirror is set to true.")
	ADD_BOOLPROP(CurrFrameEffect, FALSE, "Specifies if this render target supports displaying shaders that use the current frame effect.")

END_CLASS(RenderTargetGroup, GameBase, "An object that will render a scene to a texture and assign that texture to a material for effects such as mirrors or remote cameras")

CMDMGR_BEGIN_REGISTER_CLASS( RenderTargetGroup )
CMDMGR_END_REGISTER_CLASS( RenderTargetGroup, GameBase )

// ----------------------------------------------------------------------- //
//	RenderTargetGroup::RenderTargetGroup
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

RenderTargetGroup::RenderTargetGroup() : 
	GameBase( OT_NORMAL )
{
	for(uint32 nCurrDims = 0; nCurrDims < LTARRAYSIZE(m_nDimensions); nCurrDims++)
		m_nDimensions[nCurrDims].Init(DEFAULT_WIDTH, DEFAULT_HEIGHT);

	m_bCubeMap			= false;
	m_bMipMap			= true;
	m_bFogVolumes		= false;
	m_bLastFrameEffects = false;
	m_bCurrFrameEffects = false;
	m_nUniqueGroupID	= GenerateUniqueGroupID();
}


// ----------------------------------------------------------------------- //
//	RenderTargetGroup::~RenderTargetGroup
//
//	Destructor
//
// ----------------------------------------------------------------------- //

RenderTargetGroup::~RenderTargetGroup()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 RenderTargetGroup::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
				ReadProp(&pStruct->m_cProperties);
				PostReadProp(pStruct);
			}
			break;
		}

	case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				UpdateSpecialFXMessage();
			}
			SetNextUpdate(UPDATE_NEVER);
			break;
		}

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

	case MID_UPDATE:
		{
		}
		break;

	default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void RenderTargetGroup::ReadProp(const GenericPropList *pProps)
{
	m_nDimensions[0].x	= pProps->GetLongInt("LowTextureWidth", DEFAULT_WIDTH);
	m_nDimensions[0].y	= pProps->GetLongInt("LowTextureHeight", DEFAULT_HEIGHT);
	m_nDimensions[1].x	= pProps->GetLongInt("MedTextureWidth", DEFAULT_WIDTH);
	m_nDimensions[1].y	= pProps->GetLongInt("MedTextureHeight", DEFAULT_HEIGHT);
	m_nDimensions[2].x	= pProps->GetLongInt("TextureWidth", DEFAULT_WIDTH);
	m_nDimensions[2].y	= pProps->GetLongInt("TextureHeight", DEFAULT_HEIGHT);

	m_bCubeMap			= pProps->GetBool("CubeMap", false);

	m_bMipMap			= pProps->GetBool("MipMap", true);
	m_bFogVolumes		= pProps->GetBool("FogVolumes", false);
	m_bLastFrameEffects = pProps->GetBool("LastFrameEffect", false);
	m_bCurrFrameEffects = pProps->GetBool("CurrFrameEffect", false);

	//if we are a cubic map, we must have width = height
	if(m_bCubeMap)
	{
		for(uint32 nCurrDim = 0; nCurrDim < LTARRAYSIZE(m_nDimensions); nCurrDim++)
			m_nDimensions[nCurrDim].y = m_nDimensions[nCurrDim].x;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void RenderTargetGroup::PostReadProp(ObjectCreateStruct *pStruct)
{
	//set this because we are a normal object and invisible, meaning we will get dropped from the
	//client relevant set and our object will be removed from the client.
	//also, use full position resolution, since the orientation is very important for the plane direction.
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::UpdateSpecialFXMessage
//
//	PURPOSE:	update the special effect message assocaited with this object
//
// ----------------------------------------------------------------------- //

void RenderTargetGroup::UpdateSpecialFXMessage()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_RENDERTARGETGROUP_ID);
	CreateSFXMessage(cMsg);

	// Make sure new clients will get the message
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RenderTargetGroup::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

	CreateSFXMessage(*pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::Setup
//
//	PURPOSE:	given a message, this will fill it out with the RenderTarget data
//
// ----------------------------------------------------------------------- //
void RenderTargetGroup::CreateSFXMessage(ILTMessage_Write& cMsg)
{
	cMsg.Writeuint16(m_nUniqueGroupID);

	for(uint32 nCurrDims = 0; nCurrDims < LTARRAYSIZE(m_nDimensions); nCurrDims++)
	{
		cMsg.Writeuint16(m_nDimensions[nCurrDims].x);
		cMsg.Writeuint16(m_nDimensions[nCurrDims].y);
	}

	cMsg.Writebool(m_bCubeMap);
	cMsg.Writebool(m_bMipMap);
	cMsg.Writebool(m_bFogVolumes);
	cMsg.Writebool(m_bLastFrameEffects);
	cMsg.Writebool(m_bCurrFrameEffects);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTargetGroup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RenderTargetGroup::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_nUniqueGroupID	= cMsg.Readuint16();

	for(uint32 nCurrDims = 0; nCurrDims < LTARRAYSIZE(m_nDimensions); nCurrDims++)
	{
		m_nDimensions[nCurrDims].x = cMsg.Readuint16();
		m_nDimensions[nCurrDims].y = cMsg.Readuint16();
	}

	m_bCubeMap			= cMsg.Readbool();
	m_bMipMap			= cMsg.Readbool();
	m_bFogVolumes		= cMsg.Readbool();
	m_bLastFrameEffects = cMsg.Readbool();
	m_bCurrFrameEffects = cMsg.Readbool();
}
