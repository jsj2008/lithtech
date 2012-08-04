// ----------------------------------------------------------------------- //
//
// MODULE  : GlassProp.cpp
//
// PURPOSE : Model GlassProp - Definition
//
// CREATED : 1/26/98
//
// ----------------------------------------------------------------------- //

#include "GlassProp.h"
#include "cpp_server_de.h"

BEGIN_CLASS(GlassProp)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_BOOLPROP(MoveToFloor, DFALSE)
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\1x1_square.abc", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Masses, "100000", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DestroyedModels, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DestroyedSkins, "", PF_HIDDEN)
	ADD_STRINGPROP(HitPoints, "5")
	ADD_STRINGPROP(ArmorPoints, "5")
	ADD_STRINGPROP(Skin, "SpriteTextures\\glasstest.dtx" )
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS | PF_LOCALDIMS)
	ADD_COLORPROP(TintColor, 255.0f, 255.0f, 255.0f) 
	ADD_REALPROP(Translucency, 0.5f)
END_CLASS_DEFAULT(GlassProp, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::GlassProp()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GlassProp::GlassProp() : Prop()
{
	VEC_SET(m_vDims, 20.0f, 20.0f, 20.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD GlassProp::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProp((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if ((int)fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void GlassProp::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pServerDE->GetPropVector("Dims", &m_vDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void GlassProp::InitialUpdate()
{
	if (!g_pServerDE) return;

	// Scale object based on dims...

	HMODELANIM hAnim = g_pServerDE->GetAnimIndex(m_hObject, "static_model");
	DVector vScale, vCurDims;
	g_pServerDE->GetModelAnimUserDims(m_hObject, &vCurDims, hAnim);

	vScale.x = m_vDims.x / vCurDims.x;
	vScale.y = m_vDims.y / vCurDims.y;
	vScale.z = m_vDims.z / vCurDims.z;

	g_pServerDE->ScaleObject(m_hObject, &vScale);
	g_pServerDE->SetObjectDims(m_hObject, &m_vDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GlassProp::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GlassProp::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GlassProp::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vDims);
}