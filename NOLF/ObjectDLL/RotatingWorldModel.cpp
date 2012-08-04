// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingWorldModel.cpp
//
// PURPOSE : RotatingWorldModel implementation
//
// CREATED : 10/27/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RotatingWorldModel.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "SurfaceFunctions.h"

#include "keyframer_light.h"

BEGIN_CLASS(RotatingWorldModel)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(1, 0)
	ADD_GRAVITY_FLAG(0, 0)
    ADD_BOOLPROP(BlockLight, LTFALSE) // Used by pre-processor
    ADD_BOOLPROP(BoxPhysics, LTTRUE)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_STRINGPROP(SpinUpSound, "")
	ADD_STRINGPROP(BusySound, "")
	ADD_STRINGPROP(SpinDownSound, "")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)
	ADD_REALPROP(XAxisRevTime, 0.0f)
	ADD_REALPROP(XAxisSpinUpTime,   0.0f)
	ADD_REALPROP(XAxisSpinDownTime, 0.0f)
    ADD_BOOLPROP(XRotateForward, LTTRUE)
	ADD_REALPROP(YAxisRevTime, 0.0f)
	ADD_REALPROP(YAxisSpinUpTime,   0.0f)
	ADD_REALPROP(YAxisSpinDownTime, 0.0f)
    ADD_BOOLPROP(YRotateForward, LTTRUE)
	ADD_REALPROP(ZAxisRevTime, 0.0f)
	ADD_REALPROP(ZAxisSpinUpTime,   0.0f)
	ADD_REALPROP(ZAxisSpinDownTime, 0.0f)
    ADD_BOOLPROP(ZRotateForward, LTTRUE)
	ADD_STRINGPROP(ShadowLights, "")
	ADD_LONGINTPROP(LightFrames, 1)
	ADD_VECTORPROP_VAL(ShadowAxis, 0.0f, 1.0f, 0.0f)
    ADD_CHROMAKEY_FLAG(LTFALSE, 0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(RotatingWorldModel, GameBase, NULL, NULL, 0, CRotatingWorldModelPlugin)

#define RWM_UPDATE_DELTA	0.01f
#define TRIGGER_MSG_ON		"ON"
#define TRIGGER_MSG_OFF		"OFF"


void SetupRWMLightAnimName(char *pOutName, const char *pDoorName, uint32 iDoorLight)
{
	sprintf(pOutName, "RWM_%s_LA_%d", pDoorName, iDoorLight);
}

LTRESULT CRotatingWorldModelPlugin::PreHook_Light(
    ILTPreLight *pInterface,
	HPREOBJECT hObject)
{
	GenericProp gProp;
	char shadowLights[256];
	char className[64];
	char rwmName[64];
	char animName[128];
	HPREOBJECT hShadowLight;
	ConParse cParse;
	KLProps theProps;
	DWORD iStep;
	float fPercent;
	int iAxis;
    uint32 iRwmLight;
	PreLightAnimFrameInfo *pFrame;
	PreLightInfo *pLightInfo;
	PreWorldModelInfo *pWorldModelInfo;
	PreLightAnimFrameInfo frames[MAX_RWM_LIGHT_STEPS+1];
	PreLightInfo lightInfos[MAX_RWM_LIGHT_STEPS+1];
	PreWorldModelInfo worldModelInfos[MAX_RWM_LIGHT_STEPS+1];
    uint32 uLightFrames;

	iAxis = 0;
	iRwmLight = 0;

	pInterface->GetPropGeneric(hObject, "ShadowAxis", &gProp);
	if (gProp.m_Vec.x) iAxis = 1;
	else if (gProp.m_Vec.y) iAxis = 2;
	else if (gProp.m_Vec.z) iAxis = 3;

	pInterface->GetPropGeneric(hObject, "Name", &gProp);

	if (!iAxis) {
		pInterface->CPrint("No shadow axis set for RotatingWorldModel %s, won't light",
			gProp.m_String);
		return LT_OK;
	} else {
		switch(iAxis)
		{
			case 1:
				pInterface->CPrint("Lighting for %s rotated on x axis.", gProp.m_String);
				break;
			case 2:
				pInterface->CPrint("Lighting for %s rotated on y axis.", gProp.m_String);
				break;
			case 3:
				pInterface->CPrint("Lighting for %s rotated on z axis.", gProp.m_String);
				break;
		}
	}

	if (pInterface->GetPropGeneric(hObject, "LightFrames", &gProp) == LT_OK)
		uLightFrames = gProp.m_Long;
	else
		uLightFrames = 1;

	// Go through the ShadowLights.
	iRwmLight = 0;
	pInterface->GetPropGeneric(hObject, "ShadowLights", &gProp);
	SAFE_STRCPY(shadowLights, gProp.m_String);
	cParse.Init(shadowLights);
	while(pInterface->Parse(&cParse) == LT_OK)
	{
		if(cParse.m_nArgs == 0)
			continue;

		// Look for the object and make sure it's a keyframer light.
		if(pInterface->FindObject(cParse.m_Args[0], hShadowLight) != LT_OK ||
			pInterface->GetClassName(hShadowLight, className, sizeof(className)) != LT_OK ||
			stricmp(className, KEYFRAMERLIGHT_CLASSNAME) != 0)
		{
			continue;
		}

		// Read in all the light properties.
		ReadKLProps(pInterface, hShadowLight, &theProps);

		// Get some of the door properties.
		pInterface->GetPropGeneric(hObject, "Name", &gProp);
		SAFE_STRCPY(rwmName, gProp.m_String);

		// Hinged doors don't look good unless they have an odd number of frames.
		if((uLightFrames & 1) == 0)
			++uLightFrames;

        uLightFrames = LTCLAMP(uLightFrames, 2, MAX_RWM_LIGHT_STEPS);
		for(iStep=0; iStep < (uLightFrames+1); iStep++)
		{
			pFrame = &frames[iStep];
			pLightInfo = &lightInfos[iStep];
			pWorldModelInfo = &worldModelInfos[iStep];
			fPercent = (float)iStep / (uLightFrames - 1);

            pFrame->m_bSunLight = LTFALSE;

			pFrame->m_Lights = pLightInfo;
			pFrame->m_nLights = 1;

			pFrame->m_WorldModels = pWorldModelInfo;
			pFrame->m_nWorldModels = 1;

			pLightInfo->m_bDirectional = theProps.m_bDirLight;
			pLightInfo->m_FOV = theProps.m_fFOV;
			pLightInfo->m_Radius = theProps.m_fRadius;
			pLightInfo->m_vInnerColor = theProps.m_vInnerColor;
			pLightInfo->m_vOuterColor = theProps.m_vOuterColor;
			pLightInfo->m_vDirection = theProps.m_vForwardVec;
			pLightInfo->m_vPos = theProps.m_vPos;

			SAFE_STRCPY(pWorldModelInfo->m_WorldModelName, rwmName);

			if(iStep == uLightFrames)
			{
				// The last frame is without the world model for when it's destroyed.
                pWorldModelInfo->m_bInvisible = LTTRUE;
			}
			else
			{
				pInterface->GetWorldModelRuntimePos(hObject, pWorldModelInfo->m_Pos);

				switch (iAxis)
				{
					case 1:
						pInterface->GetMathLT()->SetupEuler(pWorldModelInfo->m_Rot,
							MATH_CIRCLE * fPercent, 0.0f, 0.0f);
						break;
					case 2:
						pInterface->GetMathLT()->SetupEuler(pWorldModelInfo->m_Rot,
							0.0f, MATH_CIRCLE * fPercent, 0.0f);
						break;
					case 3:
						pInterface->GetMathLT()->SetupEuler(pWorldModelInfo->m_Rot,
							0.0f, 0.0f, MATH_CIRCLE * fPercent);
						break;
				}
			}
		}

		SetupRWMLightAnimName(animName, rwmName, iRwmLight);

		pInterface->CreateLightAnim(
			animName,
			frames,
			uLightFrames+1,
			theProps.m_bUseShadowMaps);

		++iRwmLight;
	}

	return LT_OK;
}

LTRESULT CRotatingWorldModelPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::RotatingWorldModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::RotatingWorldModel() : GameBase(OT_WORLDMODEL)
{
	AddAggregate(&m_damage);

    m_hstrSpinUpSound   = LTNULL;
    m_hstrBusySound     = LTNULL;
    m_hstrSpinDownSound = LTNULL;
	m_fSoundRadius		= 1000.0f;
    m_sndLastSound      = LTNULL;
    m_bBoxPhysics       = LTTRUE;

	VEC_INIT(m_vVelocity);
	VEC_INIT(m_vSaveVelocity);
	VEC_INIT(m_vSpinUpTime);
	VEC_INIT(m_vSpinDownTime);
	VEC_INIT(m_vSpinTimeLeft);
	VEC_SET(m_vSign, 1.0f, 1.0f, 1.0f);

	m_fLastTime		= 0.0f;
	m_fStartTime	= 0.0f;

	m_eState		= RWM_NORMAL;

	m_fPitch		= 0.0f;
	m_fYaw			= 0.0f;
	m_fRoll			= 0.0f;

	m_hShadowLightsString = NULL;
	m_nLightAnims = 0;
	m_nShadowAxis = 0;
    m_bFirstUpdate = LTTRUE;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::~RotatingWorldModel()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

RotatingWorldModel::~RotatingWorldModel()
{
 	if (m_hstrSpinUpSound)
	{
		g_pLTServer->FreeString(m_hstrSpinUpSound);
	}
	if (m_hstrSpinDownSound)
	{
		g_pLTServer->FreeString(m_hstrSpinDownSound);
	}
	if (m_hstrBusySound)
	{
		g_pLTServer->FreeString(m_hstrBusySound);
	}
	if (m_hShadowLightsString)
	{
		g_pLTServer->FreeString(m_hShadowLightsString);
	}

	if (m_sndLastSound)
	{
		g_pLTServer->KillSound(m_sndLastSound);
	}

	SetLightAnimRemoved();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 RotatingWorldModel::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				pStruct->m_ObjectType = OT_WORLDMODEL;
				SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
				pStruct->m_SkinName[0] = '\0';

				pStruct->m_Flags |= FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD | FLAG_DONTFOLLOWSTANDING | (m_bBoxPhysics ? FLAG_BOXPHYSICS : 0);

				// Don't go through world if gravity is set...

				if (pStruct->m_Flags & FLAG_GRAVITY)
				{
					pStruct->m_Flags &= ~FLAG_GOTHRUWORLD;
				}
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
            m_bFirstUpdate = LTTRUE;
			break;
		}

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 RotatingWorldModel::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL RotatingWorldModel::ReadProp(ObjectCreateStruct *)
{
 	GenericProp gProp;

    if (!g_pLTServer) return LTFALSE;

	g_pLTServer->GetPropBool("BoxPhysics", &m_bBoxPhysics);

    LTFLOAT fRealVal;
	g_pLTServer->GetPropReal("XAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.x = MATH_CIRCLE / fRealVal;

	g_pLTServer->GetPropReal("YAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.y = MATH_CIRCLE / fRealVal;

	g_pLTServer->GetPropReal("ZAxisRevTime", &fRealVal);
	if (fRealVal > 0.0f) m_vVelocity.z = MATH_CIRCLE / fRealVal;

    LTBOOL bVal = LTFALSE;
	g_pLTServer->GetPropBool("XRotateForward", &bVal);
    m_vSign.x = (bVal == LTTRUE) ? 1.0f : -1.0f;

    bVal = LTFALSE;
	g_pLTServer->GetPropBool("YRotateForward", &bVal);
    m_vSign.y = (bVal == LTTRUE) ? 1.0f : -1.0f;

    bVal = LTFALSE;
	g_pLTServer->GetPropBool("ZRotateForward", &bVal);
    m_vSign.z = (bVal == LTTRUE) ? 1.0f : -1.0f;

    bVal = LTFALSE;
	g_pLTServer->GetPropBool("StartOn", &bVal);
	if (!bVal) m_eState = RWM_OFF;

	g_pLTServer->GetPropReal("SoundRadius", &m_fSoundRadius);

	g_pLTServer->GetPropReal("XAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.x = fRealVal;

	g_pLTServer->GetPropReal("XAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.x = fRealVal;

	g_pLTServer->GetPropReal("YAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.y = fRealVal;

	g_pLTServer->GetPropReal("YAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.y = fRealVal;

	g_pLTServer->GetPropReal("ZAxisSpinUpTime", &fRealVal);
	m_vSpinUpTime.z = fRealVal;

	g_pLTServer->GetPropReal("ZAxisSpinDownTime", &fRealVal);
	m_vSpinDownTime.z = fRealVal;

	char buf[MAX_CS_FILENAME_LEN];
    if (g_pLTServer->GetPropString("SpinUpSound", buf, MAX_CS_FILENAME_LEN) == LT_OK)
	{
		m_hstrSpinUpSound = g_pLTServer->CreateString(buf);
	}

    if (g_pLTServer->GetPropString("BusySound", buf, MAX_CS_FILENAME_LEN) == LT_OK)
	{
		m_hstrBusySound = g_pLTServer->CreateString(buf);
	}

    if (g_pLTServer->GetPropString("SpinDownSound", buf, MAX_CS_FILENAME_LEN) == LT_OK)
	{
		m_hstrSpinDownSound = g_pLTServer->CreateString(buf);
	}

    if (g_pLTServer->GetPropGeneric("LightFrames", &gProp) == LT_OK)
		m_nLightFrames = gProp.m_Long;
	else
		m_nLightFrames = 1;

    if (g_pLTServer->GetPropString("ShadowLights", buf, MAX_CS_FILENAME_LEN) == LT_OK)
	{
        m_hShadowLightsString = g_pLTServer->CreateString(buf);
	}

	if (g_pLTServer->GetPropGeneric("ShadowAxis",&gProp) == LT_OK)
	{
		if (gProp.m_Vec.x) m_nShadowAxis = 1;
		else if (gProp.m_Vec.y) m_nShadowAxis = 2;
		else if (gProp.m_Vec.z) m_nShadowAxis = 3;
	}
	else
		m_nShadowAxis = 1;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::InitialUpdate()
{
    uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_MOVEABLE);

	if (m_eState != RWM_OFF)
	{
		SetNextUpdate(RWM_UPDATE_DELTA);
		SetNormalRotation();
	}

	VEC_COPY(m_vSaveVelocity, m_vVelocity);
}

void RotatingWorldModel::FirstUpdate()
{
	ConParse cParse;
	LAInfo info;

	if (!m_bFirstUpdate) return;
	    m_bFirstUpdate = LTFALSE;

	// Init our light animations.
	if(m_hShadowLightsString)
	{
		char *pShadowLights = g_pLTServer->GetStringData(m_hShadowLightsString);
		if (!pShadowLights)
			return;

		const char *pObjectName = g_pLTServer->GetObjectName(m_hObject);

		char animName[256];

		ConParse cParse;

		cParse.Init(pShadowLights);
        while(g_pLTServer->Common()->Parse(&cParse) == LT_OK)
		{
			SetupRWMLightAnimName(animName, pObjectName, m_nLightAnims);

			m_hLightAnims[m_nLightAnims] = INVALID_LIGHT_ANIM;
            g_pLTServer->GetLightAnimLT()->FindLightAnim(animName, m_hLightAnims[m_nLightAnims]);
			m_nLightAnims++;

			if(m_nLightAnims >= MAX_RWM_LIGHT_ANIMS)
				break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetNormalRotation()
//
//	PURPOSE:	Set the model to normal rotation state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetNormalRotation()
{
 	m_eState = RWM_NORMAL;

	m_fLastTime = g_pLTServer->GetTime();

    StartSound(m_hstrBusySound, LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetOff()
//
//	PURPOSE:	Set the model to off state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetOff()
{
	m_eState = RWM_OFF;

	if (m_sndLastSound)
	{
		g_pLTServer->KillSound(m_sndLastSound);
        m_sndLastSound = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetSpinUp()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetSpinUp()
{
	m_eState = RWM_SPINUP;

	m_fStartTime = g_pLTServer->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinUpTime);

    StartSound(m_hstrSpinUpSound, LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::SetSpinDown()
//
//	PURPOSE:	Set the model to the spin up state
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::SetSpinDown()
{
 	m_eState = RWM_SPINDOWN;

	m_fStartTime = g_pLTServer->GetTime();

	VEC_COPY(m_vSpinTimeLeft, m_vSpinDownTime);

    StartSound(m_hstrSpinDownSound, LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Update()
//
//	PURPOSE:	Update the model
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Update()
{
	// Handle the first update
	if (m_bFirstUpdate)
		FirstUpdate();

    LTFLOAT fUpdateDelta = RWM_UPDATE_DELTA;

	switch (m_eState)
	{
		case RWM_SPINUP:
			UpdateSpinUp();
		break;

		case RWM_SPINDOWN:
			UpdateSpinDown();
		break;

		case RWM_OFF:
			fUpdateDelta = 0.0f;
		break;

		case RWM_NORMAL:
		default:
			UpdateNormalRotation();
		break;
	}

	SetNextUpdate(fUpdateDelta);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::UpdateNormalRotation()
//
//	PURPOSE:	Update normal rotation
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateNormalRotation()
{
    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTFLOAT fTime = g_pLTServer->GetTime();
    LTFLOAT fDeltaTime = fTime - m_fLastTime;

	if (m_vVelocity.x > 0.0f)
	{
		m_fPitch += (m_vSign.x * (m_vVelocity.x * fDeltaTime));
	}

	if (m_vVelocity.y > 0.0f)
	{
		m_fYaw += (m_vSign.y * (m_vVelocity.y * fDeltaTime));
	}

	if (m_vVelocity.z > 0.0f)
	{
		m_fRoll += (m_vSign.z * (m_vVelocity.z * fDeltaTime));
	}

	g_pLTServer->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	if (m_nLightAnims)
	{
        LTFLOAT fPercent, fTemp, fTemp2;
        uint32 nTemp;

		fTemp = 0.0f;

		switch(m_nShadowAxis)
		{
			case 1:
				fTemp = m_fPitch;
				break;
			case 2:
				fTemp = m_fYaw;
				break;
			case 3:
				fTemp = m_fRoll;
				break;
			default:
				fPercent = 0.0f;
				break;
		}

		if (!fTemp)
		{
			SetLightAnimPercent(fPercent);
		}
		else
		{
			/* Black magic to discover what percent off of 'true' we're at! */
			fTemp2 = fTemp / MATH_CIRCLE;
            nTemp = (uint32)fTemp2;
			if (nTemp)
				fTemp2 = fTemp - (MATH_CIRCLE * (float)nTemp);
			else
				fTemp2 = fTemp;

			fPercent = fTemp2 / MATH_CIRCLE;

			SetLightAnimPercent(fPercent);
		}
	}

	m_fLastTime = fTime;
}

void RotatingWorldModel::SetupLoopedLightAnimPosition(LAInfo &info, uint32 nTotalFrames, float fPercent)
{
	float frame0Percent, frame1Percent;

	// Figure out where we are.
	info.m_iFrames[0] = (DWORD)(fPercent * (nTotalFrames - 1));
	info.m_iFrames[1] = info.m_iFrames[0] + 1;
	if(info.m_iFrames[0] >= nTotalFrames)
		info.m_iFrames[0] = 0;

	if(info.m_iFrames[1] >= nTotalFrames)
		info.m_iFrames[1] = 0;

	frame0Percent = (float)info.m_iFrames[0] / (nTotalFrames - 1);
	frame1Percent = (float)info.m_iFrames[1] / (nTotalFrames - 1);

	info.m_fPercentBetween = 0.0f;
	if(frame1Percent - frame0Percent > 0.01f)
		info.m_fPercentBetween = (fPercent - frame0Percent) / (frame1Percent - frame0Percent);
}

void RotatingWorldModel::SetLightAnimPercent(float percent)
{
    uint32 iAnim, nFrames;
	HLIGHTANIM hAnim;
    ILTLightAnim *pLightAnimLT;
	LAInfo info;

    pLightAnimLT = g_pLTServer->GetLightAnimLT();
	for(iAnim=0; iAnim < m_nLightAnims; iAnim++)
	{
		hAnim = m_hLightAnims[iAnim];

		if (hAnim != INVALID_LIGHT_ANIM)
		{
			if(pLightAnimLT->GetLightAnimInfo(hAnim, info) == LT_OK &&
				pLightAnimLT->GetNumFrames(hAnim, nFrames) == LT_OK &&
				nFrames > 0)
			{
				SetupLoopedLightAnimPosition(info, nFrames-1, percent);
				pLightAnimLT->SetLightAnimInfo(hAnim, info);
			}
		}
	}
}

void RotatingWorldModel::SetLightAnimRemoved()
{
    uint32 iAnim, nFrames;
	HLIGHTANIM hAnim;
    ILTLightAnim *pLightAnimLT;
	LAInfo info;


    pLightAnimLT = g_pLTServer->GetLightAnimLT();
	for(iAnim=0; iAnim < m_nLightAnims; iAnim++)
	{
		hAnim = m_hLightAnims[iAnim];

		if (hAnim != INVALID_LIGHT_ANIM)
		{
			if(pLightAnimLT->GetLightAnimInfo(hAnim, info) == LT_OK &&
				pLightAnimLT->GetNumFrames(hAnim, nFrames) == LT_OK &&
				nFrames > 0)
			{
				info.m_iFrames[0] = info.m_iFrames[1] = nFrames - 1;
				info.m_fPercentBetween = 0.0f;
				pLightAnimLT->SetLightAnimInfo(hAnim, info);
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::UpdateSpinUp()
//
//	PURPOSE:	Update spin up
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateSpinUp()
{
    LTBOOL bXDone = LTFALSE, bYDone = LTFALSE, bZDone = LTFALSE;
    LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

	// Calculate current velocity...

	m_vSpinTimeLeft.x -= fDeltaTime;
	if (m_vSaveVelocity.x > 0.0f && m_vSpinTimeLeft.x >= 0.0f)
	{
		m_vVelocity.x = m_vSaveVelocity.x * (m_vSpinUpTime.x - m_vSpinTimeLeft.x) / m_vSpinUpTime.x;
	}
	else
	{
		m_vVelocity.x = m_vSaveVelocity.x;
        bXDone = LTTRUE;
	}

	m_vSpinTimeLeft.y -= fDeltaTime;
	if (m_vSaveVelocity.y > 0.0f && m_vSpinTimeLeft.y >= 0.0f)
	{
		m_vVelocity.y = m_vSaveVelocity.y * (m_vSpinUpTime.y - m_vSpinTimeLeft.y) / m_vSpinUpTime.y;
	}
	else
	{
		m_vVelocity.y = m_vSaveVelocity.y;
        bYDone = LTTRUE;
	}

	m_vSpinTimeLeft.z -= fDeltaTime;
	if (m_vSaveVelocity.z > 0.0f && m_vSpinTimeLeft.z >= 0.0f)
	{
		m_vVelocity.z = m_vSaveVelocity.z * (m_vSpinUpTime.z - m_vSpinTimeLeft.z) / m_vSpinUpTime.z;
	}
	else
	{
		m_vVelocity.z = m_vSaveVelocity.z;
        bZDone = LTTRUE;
	}

	// Call normal update to do the work...

	UpdateNormalRotation();

	if (bXDone && bYDone && bZDone)
	{
		SetNormalRotation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::UpdateSpinDown()
//
//	PURPOSE:	Update spin down
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::UpdateSpinDown()
{
    LTBOOL bXDone = LTFALSE, bYDone = LTFALSE, bZDone = LTFALSE;
    LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

	// Calculate current velocity...

	m_vSpinTimeLeft.x -= fDeltaTime;
	if (m_vSaveVelocity.x > 0.0f && m_vSpinTimeLeft.x >= 0.0f)
	{
		m_vVelocity.x = m_vSaveVelocity.x - (m_vSaveVelocity.x * (m_vSpinDownTime.x - m_vSpinTimeLeft.x) / m_vSpinDownTime.x);
	}
	else
	{
		m_vVelocity.x = 0.0f;
        bXDone = LTTRUE;
	}

	m_vSpinTimeLeft.y -= fDeltaTime;
	if (m_vSaveVelocity.y > 0.0f && m_vSpinTimeLeft.y >= 0.0f)
	{
		m_vVelocity.y = m_vSaveVelocity.y - (m_vSaveVelocity.y * (m_vSpinDownTime.y - m_vSpinTimeLeft.y) / m_vSpinDownTime.y);
	}
	else
	{
		m_vVelocity.y = 0.0f;
        bYDone = LTTRUE;
	}

	m_vSpinTimeLeft.z -= fDeltaTime;
	if (m_vSaveVelocity.z > 0.0f && m_vSpinTimeLeft.z >= 0.0f)
	{
		m_vVelocity.z = m_vSaveVelocity.z - (m_vSaveVelocity.z * (m_vSpinDownTime.z - m_vSpinTimeLeft.z) / m_vSpinDownTime.z);
	}
	else
	{
		m_vVelocity.z = 0.0f;
        bZDone = LTTRUE;
	}

	// Call normal update to do the work...

	UpdateNormalRotation();

	if (bXDone && bYDone && bZDone)
	{
		SetOff();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::TriggerMsg()
//
//	PURPOSE:	Handler trigger messages
//
// --------------------------------------------------------------------------- //

void RotatingWorldModel::HandleTrigger(HOBJECT hSender, const char* szMsg)
{
	if (m_eState == RWM_OFF && stricmp(szMsg, TRIGGER_MSG_ON) == 0)
	{
		SetSpinUp();
		SetNextUpdate(RWM_UPDATE_DELTA);
	}
	else if (m_eState == RWM_NORMAL && stricmp(szMsg, TRIGGER_MSG_OFF) == 0)
	{
		SetSpinDown();
		SetNextUpdate(RWM_UPDATE_DELTA);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::StartSound()
//
//	PURPOSE:	Start the specified sound
//
// --------------------------------------------------------------------------- //

void RotatingWorldModel::StartSound(HSTRING hstrSoundName, LTBOOL bLoop)
{
 	// Stop the last sound if there is one...

	if (m_sndLastSound)
	{
		g_pLTServer->KillSound(m_sndLastSound);
        m_sndLastSound = LTNULL;
	}

	if (!hstrSoundName) return;

	char *pSoundName = g_pLTServer->GetStringData(hstrSoundName);
	if (!pSoundName) return;


    uint32 dwFlags = bLoop ? PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE : 0;

	// Save the handle of the sound (if bLoop)...

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	m_sndLastSound = g_pServerSoundMgr->PlaySoundFromPos(vPos, pSoundName,
		m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, dwFlags);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
 	if (!hWrite) return;

	g_pLTServer->WriteToMessageVector(hWrite, &m_vVelocity);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSaveVelocity);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSign);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSpinUpTime);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSpinDownTime);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSpinTimeLeft);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLastTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fStartTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fPitch);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRoll);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);
	g_pLTServer->WriteToMessageByte(hWrite, m_eState);
	g_pLTServer->WriteToMessageByte(hWrite, m_bBoxPhysics);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrBusySound);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpinUpSound);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrSpinDownSound);
	g_pLTServer->WriteToMessageHString(hWrite, m_hShadowLightsString);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nShadowAxis);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	g_pLTServer->ReadFromMessageVector(hRead, &m_vVelocity);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSaveVelocity);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSign);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSpinUpTime);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSpinDownTime);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSpinTimeLeft);
	m_fLastTime			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fStartTime		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fPitch			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fYaw				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRoll				= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fSoundRadius		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_eState			= (RWMState) g_pLTServer->ReadFromMessageByte(hRead);
    m_bBoxPhysics       = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_hstrBusySound		= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrSpinUpSound	= g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrSpinDownSound = g_pLTServer->ReadFromMessageHString(hRead);
	m_hShadowLightsString = g_pLTServer->ReadFromMessageHString(hRead);
	m_nShadowAxis		= g_pLTServer->ReadFromMessageDWord(hRead);


	if (m_eState == RWM_NORMAL)
	{
        StartSound(m_hstrBusySound, LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingWorldModel::CacheFiles
//
//	PURPOSE:	Cache resources associated with this object
//
// ----------------------------------------------------------------------- //

void RotatingWorldModel::CacheFiles()
{
     char* pFile = LTNULL;
	if (m_hstrBusySound)
	{
		pFile = g_pLTServer->GetStringData(m_hstrBusySound);
		if (pFile)
		{
			g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSpinUpSound)
	{
		pFile = g_pLTServer->GetStringData(m_hstrSpinUpSound);
		if (pFile)
		{
			g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSpinDownSound)
	{
		pFile = g_pLTServer->GetStringData(m_hstrSpinDownSound);
		if (pFile)
		{
			g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

}