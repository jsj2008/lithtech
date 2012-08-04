// ----------------------------------------------------------------------- //
//
// MODULE  :	LightBase.cpp
//
// PURPOSE :	Provides the implementation for the light base class which
//				is a base class for all light objects so that they can
//				receive and process the same messages.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "LightBase.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( LightBase );

//---------------------------------------------------------------------------------------------
// Command manager plugin support
//---------------------------------------------------------------------------------------------

//-------------------------------------
// Functions to verify different types
//-------------------------------------

CMDMGR_BEGIN_REGISTER_CLASS( LightBase )

//					Message			Num Params	Validation FnPtr		Syntax

	ADD_MESSAGE( DIMS,				4,		NULL,	MSG_HANDLER( LightBase, HandleDimsMsg ),		"DIMS <x> <y> <z>", "Sets the dimensions of the light", "msg Light (dims 1024 256 1024)" )
	ADD_MESSAGE( COLOR,				4,		NULL,	MSG_HANDLER( LightBase, HandleColorMsg ),		"COLOR <r> <g> <b> (all 0..1)", "Changes the light color", "msg Light (color 255 210 163)" )
	ADD_MESSAGE( RADIUS,			2,		NULL,	MSG_HANDLER( LightBase, HandleRadiusMsg ),		"RADIUS <radius>", "Changes the light radius", "msg Light (radius 300)" )
	ADD_MESSAGE( INTENSITY,			2,		NULL,	MSG_HANDLER( LightBase, HandleIntensityMsg ),	"INTENSITY <intensity 0..1>", "Changes the light intensity", "msg Light (intensity 1.3)" )
	ADD_MESSAGE( TEXTURE,			2,		NULL,	MSG_HANDLER( LightBase, HandleTextureMsg ),		"TEXTURE <texture name>", "Changes the texture used for the light.  Regular texture for spot projector.  Cubic environment map for cube projector.", "To change the texture associated with a CubeProjector named \"CubeProjector\" to the texture EnvMap.dds the command would look like:<BR><BR>msg CubeProjector (TEXTURE tex\\EnvMap.dds)"  )
	ADD_MESSAGE( SPOTFOV,			3,		NULL,	MSG_HANDLER( LightBase, HandleSpotFOVMsg ),		"SPOTFOV <fov x> <fov y>", "Changes the FOV in the X and Y directions of a spot projector light type. Both of these are measured in degrees and measure the full range of the cone.", "msg Light (SpotFOV 90 90)" )
	ADD_MESSAGE( SCALEALPHA,		2,		NULL,	MSG_HANDLER( LightBase, HandleScaleAlphaMsg ),	"SCALEALPHA <0..1>", "Scales the alpha associated with this light", "msg Light (ScaleAlpha 0.5)" )

CMDMGR_END_REGISTER_CLASS( LightBase, GameBase )

//---------------------------------------------------------------------------------------------
// LightBase
//---------------------------------------------------------------------------------------------

BEGIN_CLASS(LightBase)
END_CLASS_FLAGS(LightBase, GameBase, CF_HIDDEN, "Provides a base class for all the light types")


LightBase::LightBase() :
	GameBase(OT_LIGHT)
{
	m_vColor				= LTVector(1.0f, 1.0f, 1.0f);
	m_vDirectionalDims		= LTVector(0.0f, 0.0f, 0.0f);
	m_vTranslucentColor		= LTVector(1.0f, 1.0f, 1.0f);
	m_vSpecularColor		= LTVector(1.0f, 1.0f, 1.0f);
	m_fLightRadius			= 300.0f;
	m_fIntensityScale		= 1.0f;
	m_eLightType			= eEngineLight_Invalid;
	m_fSpotFovX				= 90.0f;
	m_fSpotFovY				= 90.0f;
	m_fSpotNearClip			= 0.0f;
	m_eLightLOD				= eEngineLOD_Low;
	m_eWorldShadowsLOD		= eEngineLOD_Low;
	m_eObjectShadowsLOD		= eEngineLOD_Low;
}

LightBase::~LightBase()
{
}

//virtual function that derived classes must override to handle loading in of
//property data
void LightBase::ReadLightProperties(const GenericPropList *pProps)
{
}

//virtual function that derived classes may override to change the creation struct before the object is created
void LightBase::PostReadProp(ObjectCreateStruct *pStruct)
{
}

//handles events sent from the engine. These are primarily messages
//associated with saving and loading
uint32 LightBase::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData != PRECREATE_SAVEGAME)
			{
				PostReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				//let the child object handle loading in the necessary properties
				ReadLightProperties((const GenericPropList *)pData);
				SetupEngineLight();
			}
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

		case MID_ALLOBJECTSCREATED:
		{
			// Don't eat ticks please...
			SetNextUpdate(UPDATE_NEVER);
			break;
		}

		default : 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

void LightBase::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Write& cMsg = *pMsg;

	cMsg.WriteLTVector(m_vColor);
	cMsg.WriteLTVector(m_vDirectionalDims);
	cMsg.WriteLTVector(m_vTranslucentColor);
	cMsg.WriteLTVector(m_vSpecularColor);
	cMsg.Writefloat(m_fLightRadius);
	cMsg.Writefloat(m_fIntensityScale);
	cMsg.Writeuint8((uint8)m_eLightType);
	cMsg.Writefloat(m_fSpotFovX);
	cMsg.Writefloat(m_fSpotFovY);
	cMsg.Writefloat(m_fSpotNearClip);
	cMsg.Writeuint8((uint8)m_eLightLOD);
	cMsg.Writeuint8((uint8)m_eWorldShadowsLOD);
	cMsg.Writeuint8((uint8)m_eObjectShadowsLOD);
	cMsg.WriteString(m_sLightTexture.c_str());
	cMsg.WriteString(m_sLightAttenuationTexture.c_str());
}

void LightBase::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_vColor				= cMsg.ReadLTVector();
	m_vDirectionalDims		= cMsg.ReadLTVector();
	m_vTranslucentColor		= cMsg.ReadLTVector();
	m_vSpecularColor		= cMsg.ReadLTVector();
	m_fLightRadius			= cMsg.Readfloat();
	m_fIntensityScale		= cMsg.Readfloat();
	m_eLightType			= (EEngineLightType)cMsg.Readuint8();
	m_fSpotFovX				= cMsg.Readfloat();
	m_fSpotFovY				= cMsg.Readfloat();
	m_fSpotNearClip			= cMsg.Readfloat();
	m_eLightLOD				= (EEngineLOD)cMsg.Readuint8();
	m_eWorldShadowsLOD		= (EEngineLOD)cMsg.Readuint8();
	m_eObjectShadowsLOD		= (EEngineLOD)cMsg.Readuint8();

	char pszTextureBuffer[MAX_PATH + 1];

	cMsg.ReadString(pszTextureBuffer, LTARRAYSIZE(pszTextureBuffer));
	m_sLightTexture = pszTextureBuffer;

	cMsg.ReadString(pszTextureBuffer, LTARRAYSIZE(pszTextureBuffer));
	m_sLightAttenuationTexture = pszTextureBuffer;
}

//called to setup all variables on the engine object once it has been
//created
void LightBase::SetupEngineLight()
{
	//don't bother if we don't have a light
	if(!m_hObject)
		return;

	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	g_pLTServer->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);
	g_pLTServer->SetLightTranslucentColor(m_hObject, m_vTranslucentColor);
	g_pLTServer->SetLightSpecularColor(m_hObject, m_vSpecularColor);
	g_pLTServer->SetLightRadius(m_hObject, m_fLightRadius);
	g_pLTServer->SetLightIntensityScale(m_hObject, m_fIntensityScale);
	g_pLTServer->SetLightType(m_hObject, m_eLightType);
	g_pLTServer->SetLightTexture(m_hObject, m_sLightTexture.c_str());
	g_pLTServer->SetLightAttenuationTexture(m_hObject, m_sLightAttenuationTexture.c_str());
	g_pLTServer->SetLightDirectionalDims(m_hObject, m_vDirectionalDims * LTVector(0.5f, 0.5f, 1.0f));

	g_pLTServer->SetLightSpotInfo(m_hObject, MATH_DEGREES_TO_RADIANS(m_fSpotFovX * 0.5f), 
											 MATH_DEGREES_TO_RADIANS(m_fSpotFovY * 0.5f), 
											 m_fSpotNearClip); 

	g_pLTServer->SetLightDetailSettings(m_hObject, m_eLightLOD, m_eWorldShadowsLOD, m_eObjectShadowsLOD);
}

//---------------------------------------------------------------------------------------------
// LightBase message handlers
//---------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a DIMS message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleDimsMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 4)
	{
		//read in our new dimensions and apply them
		m_vDirectionalDims.Init(	LTMAX(0.0f, (float)atof(crParsedMsg.GetArg(1))),
									LTMAX(0.0f, (float)atof(crParsedMsg.GetArg(2))),
									LTMAX(0.0f, (float)atof(crParsedMsg.GetArg(3))));
		g_pLTServer->SetLightDirectionalDims(m_hObject, m_vDirectionalDims * LTVector(0.5f, 0.5f, 1.0f));
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a COLOR message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleColorMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 4)
	{
		//read in our new dimensions and apply them
		m_vColor.Init(	LTCLAMP((float)atof(crParsedMsg.GetArg(1)), 0.0f, 255.0f) / 255.0f,
						LTCLAMP((float)atof(crParsedMsg.GetArg(2)), 0.0f, 255.0f) / 255.0f,
						LTCLAMP((float)atof(crParsedMsg.GetArg(3)), 0.0f, 255.0f) / 255.0f);
		g_pLTServer->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 1.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a INTENSITY message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleIntensityMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 2)
	{
		//read in our new dimensions and apply them
		m_fIntensityScale =	LTCLAMP((float)atof(crParsedMsg.GetArg(1)), 0.0f, 1.0f);						
		g_pLTServer->SetLightIntensityScale(m_hObject, m_fIntensityScale);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a RADIUS message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleRadiusMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 2)
	{
		//read in our new dimensions and apply them
		m_fLightRadius =	LTMAX((float)atof(crParsedMsg.GetArg(1)), 0.0f);						
		g_pLTServer->SetLightRadius(m_hObject, m_fLightRadius);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a TEXTURE message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleTextureMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 2)
	{
		//read in our new dimensions and apply them
		m_sLightTexture = crParsedMsg.GetArg(1);			
		g_pLTServer->SetLightTexture(m_hObject, m_sLightTexture.c_str());
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleDimsMsg
//
//  PURPOSE:	Handle a SPOTFOV message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleSpotFOVMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if(crParsedMsg.GetArgCount() == 3)
	{
		//read in our new dimensions and apply them
		m_fSpotFovX = LTCLAMP((float)atof(crParsedMsg.GetArg(1)), 0.0f, 90.0f);			
		m_fSpotFovY = LTCLAMP((float)atof(crParsedMsg.GetArg(2)), 0.0f, 90.0f);

		g_pLTServer->SetLightSpotInfo(m_hObject,	MATH_DEGREES_TO_RADIANS(m_fSpotFovX * 0.5f), 
													MATH_DEGREES_TO_RADIANS(m_fSpotFovY * 0.5f), 
													m_fSpotNearClip); 
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LightBase::HandleScaleAlphaMsg
//
//  PURPOSE:	Handle a SCALEALPHA message...
//
// ----------------------------------------------------------------------- //

void LightBase::HandleScaleAlphaMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	float fAlpha = (float)LTStrToDouble(crParsedMsg.GetArg(1).c_str());

	// Update the object's alpha
	LTVector4 vOldColor;
	g_pLTServer->GetObjectColor( m_hObject, &vOldColor.x, &vOldColor.y, &vOldColor.z, &vOldColor.w );
	g_pLTServer->SetObjectColor( m_hObject, vOldColor.x, vOldColor.y, vOldColor.z, fAlpha );
}

