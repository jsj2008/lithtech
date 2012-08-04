// ----------------------------------------------------------------------- //
//
// MODULE  : RenderTarget.cpp
//
// PURPOSE : This defines the render target object which is used to handle
//			 rendering scenes to a texture and applying those rendered scenes
//			 to materials in the world 
//
// CREATED : 8/4/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "RenderTarget.h"
#include "RenderTargetGroup.h"

// ----------------------------------------------------------------------- //
// Default Values
// ----------------------------------------------------------------------- //

#define DEFAULT_FOV_X					MATH_DEGREES_TO_RADIANS(90.0f)
#define DEFAULT_FOV_Y					MATH_DEGREES_TO_RADIANS(90.0f)

// ----------------------------------------------------------------------- //
// Class definition
// ----------------------------------------------------------------------- //

LINKFROM_MODULE( RenderTarget );

BEGIN_CLASS(RenderTarget)

	ADD_STRINGPROP_FLAG(RenderTargetGroup, "", PF_OBJECTLINK, "The render target group that this render target should use to get its surface information from")

	ADD_REALPROP_FLAG(FovX, 90.0f, PF_RECTFOVX, "The field of view in the X direction used when setting up the camera for rendering the render target. This is ignored if mirror is set to true.")
	ADD_REALPROP_FLAG(FovY, 90.0f, PF_RECTFOVY, "The field of view in the Y direction used when setting up the camera for rendering the render target. This is ignored if mirror is set to true.")

	ADD_BOOLPROP(Visible, TRUE, "Determines if this render target starts out visible or not.")
	ADD_STRINGPROP_FLAG(LOD, "Low", PF_STATICLIST, "Indicates at which LOD the render target will be visible. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")

	ADD_LONGINTPROP_FLAG(UpdateFrequency, 0, 0, "Indicates the number of frames to skip between updates. 0 means update each frame, 1 means skip one frame between updates, and so on")
	ADD_LONGINTPROP_FLAG(UpdateOffset, 0, 0, "Indicates the offset to use when updating the render target. This can be used to stagger render target updates.")

	ADD_BOOLPROP(Mirror, FALSE, "Determines if this render target should act like a mirror. If so, it will mirror the scene around the plane formed by the object's orientation. Mirrors should ONLY be placed on the edge of the geometry in the sector and not in the middle of sectors")
	ADD_BOOLPROP_FLAG(Refraction, FALSE, PF_HIDDEN, "Determines if this render target will act as a refraction map. If so, it will clip the scene to the plane formed by the object's orientation.")
	ADD_REALPROP_FLAG(RefractionClipPlaneBias, 0.0f, PF_HIDDEN, "The offset distance for the clipping plane when this object acts as a refraction map.")

	ADD_STRINGPROP_FLAG(Material, "", PF_FILENAME, "The name of the material file that this render target will be assigned to")
	ADD_STRINGPROP(Parameter, "", "The name of the texture parameter in the above material that this render target will be assigned to")

END_CLASS_FLAGS_PLUGIN(RenderTarget, GameBase, 0, RenderTarget_Plugin, "An object that will render a scene to a texture and assign that texture to a material for effects such as mirrors or remote cameras")

CMDMGR_BEGIN_REGISTER_CLASS( RenderTarget )
CMDMGR_END_REGISTER_CLASS( RenderTarget, GameBase )

// ----------------------------------------------------------------------- //
//	RenderTarget::RenderTarget
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

RenderTarget::RenderTarget() : 
	GameBase( OT_NORMAL )
{
	m_nRenderTargetGroupID	= 0;
	m_bCreatedFromSave		= false;

	m_vFOV.Init(DEFAULT_FOV_X, DEFAULT_FOV_Y);
	m_fRefractionClipPlaneBias = 0.0f;
	m_nUpdateFrequency	= 0;
	m_nUpdateOffset		= 0;
	m_bMirror			= false;
	m_bRefraction		= false;
}


// ----------------------------------------------------------------------- //
//	RenderTarget::~RenderTarget
//
//	Destructor
//
// ----------------------------------------------------------------------- //

RenderTarget::~RenderTarget()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 RenderTarget::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
			m_bCreatedFromSave = (fData == INITIALUPDATE_SAVEGAME);
			SetNextUpdate(UPDATE_NEVER);
			break;
		}

		case MID_ALLOBJECTSCREATED:
		{
			if(!m_bCreatedFromSave)
				UpdateSpecialFXMessage();
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

		default: 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void RenderTarget::ReadProp(const GenericPropList *pProps)
{
	//read the properties into our data members
	m_vFOV.x = MATH_DEGREES_TO_RADIANS(pProps->GetReal("FovX", 90.0f));
	m_vFOV.y = MATH_DEGREES_TO_RADIANS(pProps->GetReal("FovY", 90.0f));

	m_nUpdateFrequency	= LTMAX(0, pProps->GetLongInt("UpdateFrequency", m_nUpdateFrequency));
	m_nUpdateOffset		= LTMAX(0, pProps->GetLongInt("UpdateOffset", m_nUpdateOffset));

	m_bMirror		= pProps->GetBool("Mirror", false);
	m_bRefraction	= pProps->GetBool("Refraction", false);

	m_fRefractionClipPlaneBias	= pProps->GetReal("RefractionClipPlaneBias", 0.0f);

	m_nRenderTargetLOD		= CEngineLODPropUtil::StringToLOD(pProps->GetString("LOD", "Low"));
	m_sMaterial				= pProps->GetString("Material", "");
	m_sParam				= pProps->GetString("Parameter", "");
	m_sRenderTargetGroup	= pProps->GetString("RenderTargetGroup", "");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void RenderTarget::PostReadProp(ObjectCreateStruct *pStruct)
{
	if(pStruct->m_cProperties.GetBool("Visible", true))
	{
		pStruct->m_Flags |= FLAG_VISIBLE;
	}

	//set flags up on our creation structure

	//set this because we are a normal object and invisible, meaning we will get dropped from the
	//client relevant set and our object will be removed from the client.
	//also, use full position resolution, since the orientation is very important for the plane direction.
	pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::UpdateSpecialFXMessage
//
//	PURPOSE:	update the special effect message assocaited with this object
//
// ----------------------------------------------------------------------- //

void RenderTarget::UpdateSpecialFXMessage()
{
	//first we need to find our corresponding render target group, in order to extract
	//it's ID
	ObjArray<HOBJECT, 1> Objects;

	uint32 nNumFound = 0;
	if(g_pLTServer->FindNamedObjects(m_sRenderTargetGroup.c_str(), Objects, &nNumFound) != LT_OK)
		return;

	//make sure that we found an object
	if(nNumFound != 1)
		return;

	//alright, we found the object, make sure it is the right type
	HCLASS hRTGroupClass	= g_pLTServer->GetClass("RenderTargetGroup");
	HCLASS hObjClass		= g_pLTServer->GetObjectClass(Objects.GetObject(0));

	//make sure the class is of the right type
	if(!g_pLTServer->IsKindOf(hObjClass, hRTGroupClass))
		return;

	//and now get the unique ID
	m_nRenderTargetGroupID = ((RenderTargetGroup*)g_pLTServer->HandleToObject(Objects.GetObject(0)))->GetUniqueGroupID();

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_RENDERTARGET_ID);
	CreateSFXMessage(cMsg);

	// Make sure new clients will get the message
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RenderTarget::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) 
		return;

	CreateSFXMessage(*pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::Setup
//
//	PURPOSE:	given a message, this will fill it out with the RenderTarget data
//
// ----------------------------------------------------------------------- //
void RenderTarget::CreateSFXMessage(ILTMessage_Write& cMsg)
{
	cMsg.Writeuint16(m_nRenderTargetGroupID);
	cMsg.Writeuint8(m_nRenderTargetLOD);

	cMsg.Writeuint16(m_nUpdateFrequency);
	cMsg.Writeuint16(m_nUpdateOffset);

	cMsg.Writefloat(m_vFOV.x);
	cMsg.Writefloat(m_vFOV.y);

	cMsg.Writebool(m_bMirror);
	cMsg.Writebool(m_bRefraction);
	cMsg.Writefloat(m_fRefractionClipPlaneBias);

	cMsg.WriteString(m_sMaterial.c_str());
	cMsg.WriteString(m_sParam.c_str());
	cMsg.WriteString(m_sRenderTargetGroup.c_str());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RenderTarget::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RenderTarget::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_nRenderTargetGroupID		= cMsg.Readuint16();
	m_nRenderTargetLOD			= cMsg.Readuint8();

	m_nUpdateFrequency			= cMsg.Readuint16();
	m_nUpdateOffset				= cMsg.Readuint16();

	m_vFOV.x					= cMsg.Readfloat();
	m_vFOV.y					= cMsg.Readfloat();

	m_bMirror					= cMsg.Readbool();
	m_bRefraction				= cMsg.Readbool();
	m_fRefractionClipPlaneBias	= cMsg.Readfloat();

	char pszBuffer[MAX_PATH + 1];
	pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sMaterial = pszBuffer;

	pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sParam = pszBuffer;

	pMsg->ReadString(pszBuffer, LTARRAYSIZE(pszBuffer));
	m_sRenderTargetGroup = pszBuffer;
}
