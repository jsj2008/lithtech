
#include "stdafx.h"
#include "keyframer_light.h"


BEGIN_CLASS(KeyframerLight)
    ADD_BOOLPROP(DirLight, LTFALSE)
	ADD_REALPROP_FLAG(DirLightRadius, 300.0f, PF_FOVRADIUS)
	ADD_REALPROP_FLAG(FOV, 90.0f, PF_FIELDOFVIEW)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS)
	ADD_COLORPROP(InnerColor, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(OuterColor, 0.0f, 0.0f, 0.0f)
    ADD_BOOLPROP(UseShadowMaps, LTTRUE)
END_CLASS_DEFAULT_FLAGS(KeyframerLight, BaseClass, NULL, NULL, 0)



// ----------------------------------------------------------------------------------- //
// Helpers.
// ----------------------------------------------------------------------------------- //
void GetKLLightAnimName(char *pDest, const char *pObjectName)
{
	sprintf(pDest, "%s_KL", pObjectName);
}


void ReadKLProps(ILTPreLight *pInterface, HPREOBJECT hObject, KLProps *pProps)
{
	GenericProp gProp;
    LTVector vUp, vRight;

	// Get light properties.
	pInterface->GetPropGeneric(hObject, "DirLight", &gProp);
	pProps->m_bDirLight = gProp.m_Bool;

	if(pProps->m_bDirLight)
		pInterface->GetPropGeneric(hObject, "DirLightRadius", &gProp);
	else
		pInterface->GetPropGeneric(hObject, "LightRadius", &gProp);

	pProps->m_fRadius = gProp.m_Float;

	pInterface->GetPropGeneric(hObject, "FOV", &gProp);
	pProps->m_fFOV = MATH_DEGREES_TO_RADIANS(gProp.m_Float);

	pInterface->GetPropGeneric(hObject, "InnerColor", &gProp);
	pProps->m_vInnerColor = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "OuterColor", &gProp);
	pProps->m_vOuterColor = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "UseShadowMaps", &gProp);
	pProps->m_bUseShadowMaps = gProp.m_Bool;

	pInterface->GetPropGeneric(hObject, "Pos", &gProp);
	pProps->m_vPos = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "Rotation", &gProp);
	pInterface->GetMathLT()->GetRotationVectors(gProp.m_Rotation, vRight, vUp, pProps->m_vForwardVec);
}


LTBOOL IsKeyframerLight(ILTServer *pServerDE, HOBJECT hObj)
{
	HCLASS hKeylightClass, hObjClass;

	hKeylightClass = pServerDE->GetClass(KEYFRAMERLIGHT_CLASSNAME);
	hObjClass = pServerDE->GetObjectClass(hObj);

	return pServerDE->IsKindOf(hKeylightClass, hObjClass);
}


void SetupLightAnimPosition(LAInfo &info, uint32 nTotalFrames, float fPercent)
{
	float frame0Percent, frame1Percent;

	// Figure out where we are.
	info.m_iFrames[0] = (DWORD)(fPercent * (nTotalFrames - 1));
	info.m_iFrames[1] = info.m_iFrames[0] + 1;
	if(info.m_iFrames[0] >= nTotalFrames)
		info.m_iFrames[0] = nTotalFrames - 1;

	if(info.m_iFrames[1] >= nTotalFrames)
		info.m_iFrames[1] = nTotalFrames - 1;

	frame0Percent = (float)info.m_iFrames[0] / (nTotalFrames - 1);
	frame1Percent = (float)info.m_iFrames[1] / (nTotalFrames - 1);

	info.m_fPercentBetween = 0.0f;
	if(frame1Percent - frame0Percent > 0.01f)
		info.m_fPercentBetween = (fPercent - frame0Percent) / (frame1Percent - frame0Percent);
}




// ----------------------------------------------------------------------------------- //
// KeyframerLight functions.
// ----------------------------------------------------------------------------------- //
KeyframerLight::KeyframerLight()
{
	m_hLightAnim = INVALID_LIGHT_ANIM;
}


uint32 KeyframerLight::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			PreCreate((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


void KeyframerLight::PreCreate(ObjectCreateStruct *pStruct)
{
	GenericProp gProp;


	pStruct->m_Flags = 0;
	pStruct->m_ObjectType = OT_NORMAL;

    g_pLTServer->GetPropGeneric("InnerColor", &gProp);
	m_vLightColor = gProp.m_Color;

    g_pLTServer->GetPropGeneric("DirLight", &gProp);
	if(gProp.m_Bool)
        g_pLTServer->GetPropGeneric("DirLightRadius", &gProp);
	else
        g_pLTServer->GetPropGeneric("LightRadius", &gProp);

	m_fLightRadius = gProp.m_Float;
}


void KeyframerLight::InitialUpdate()
{
	char objectName[64], animName[128];


    if(g_pLTServer->GetObjectName(m_hObject, objectName, sizeof(objectName)) == LT_OK)
	{
		GetKLLightAnimName(animName, objectName);
        g_pLTServer->GetLightAnimLT()->FindLightAnim(animName, m_hLightAnim);
	}
}