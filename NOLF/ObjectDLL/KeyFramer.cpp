// ----------------------------------------------------------------------- //
//
// MODULE  : Keyframer.cpp
//
// PURPOSE : Keyframer implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyFramer.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "ObjectMsgs.h"
#include "SoundMgr.h"
#include "ltbeziercurve.h"
#include "keyframer_light.h"
#include "CVarTrack.h"
#include "CommandMgr.h"

CVarTrack	g_SynchronizeKeyframers;


// How many frames of animation we can have for KeyframerLights.
#define MAX_KEYFRAMER_KEYLIGHT_FRAMES 128
// How many shadow lights are we allowed to have in a Keyframer?
#define MAX_KEYFRAMER_SHADOW_LIGHTS   12


// Makes some code cleaner when calling the bezier routines...
#define CURVE_PTS(keyData1, keyData2) \
	keyData1.m_vPos, keyData1.m_BezierNextCtrl, keyData2.m_BezierPrevCtrl, keyData2.m_vPos


#define KF_UPDATE_DELTA	0.001f

LTFLOAT GetKFWaveValue(LTFLOAT fSpeed, LTFLOAT fPercent, KFWaveType eWaveType)
{
	if (eWaveType == KFWAVE_LINEAR) return fSpeed;

	LTFLOAT fNewSpeed;
	LTFLOAT f10Percent = fSpeed * 0.1f;

	switch (eWaveType)
	{
		case KFWAVE_SINE:
			fNewSpeed = fSpeed * ((LTFLOAT)sin(fPercent * MATH_PI)) + f10Percent;
		break;

		case KFWAVE_SLOWOFF:
			fNewSpeed = fSpeed * ((LTFLOAT)cos(fPercent * MATH_HALFPI)) + f10Percent;
		break;

		case KFWAVE_SLOWON:
			fNewSpeed = fSpeed * ((LTFLOAT)sin(fPercent * MATH_HALFPI)) + f10Percent;
		break;
	}

	return fNewSpeed;
}


BEGIN_CLASS(KeyFramer)
	ADD_KEYFRAMER_PROPERTIES(0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(KeyFramer, BaseClass, LTNULL, LTNULL, 0, KeyframerPlugin)



// Given the key number and base key name, this creates the key name the Keyframer looks for.
static void GetKeyNameString(const char *pBaseKeyName, int nNum, char *pStrKey)
{
	if (0 < nNum && nNum < 10)
	{
		sprintf(pStrKey, "%s0%d", pBaseKeyName, nNum);
	}
	else
	{
		sprintf(pStrKey, "%s%d", pBaseKeyName, nNum);
	}
}

void GetKFModelAnimName(char *pDest, const char *pObjectName)
{
	sprintf(pDest, "KF_%s_MC", pObjectName);
}

int AllocWorldModelsCB(ILTPreLight *pInterface, HPREOBJECT hObject,
						PREWORLDMODEL **pModels)
{
	PREWORLDMODEL *pArray;
	GenericProp gProp;
	ConParse cParse;
	uint32 nObjects, nCounter;
	HPREOBJECT hTestObj;
	char tempStr[128], className[128];

	/* Sanity! */
	if (pInterface->GetPropGeneric(hObject,"ObjectName", &gProp) != LT_OK)
	{
		*pModels = LTNULL;
		return 0;
	}

	nObjects = 0;

	SAFE_STRCPY(tempStr,gProp.m_String);
	cParse.Init(tempStr);
	while(pInterface->Parse(&cParse) == LT_OK)
	{
		if(cParse.m_nArgs != 1 ||
			pInterface->FindObject(cParse.m_Args[0], hTestObj) != LT_OK ||
			pInterface->GetClassName(hTestObj, className, sizeof(className)) != LT_OK)
		{
			continue;
		}

		nObjects++;
	}

	/* Sanity check ! */
	if (!nObjects) return 0;

	pInterface->GetPropGeneric(hObject,"Name",&gProp);

	pInterface->CPrint("Found %d objects to shadow in keyframer '%s'",
		nObjects, gProp.m_String);

//	pInterface->CPrint("  - Allocating memory for keyframer lighting");

	pArray = (PREWORLDMODEL *)malloc(sizeof(PREWORLDMODEL) * nObjects);

	nCounter = 0;

	/* Sanity! */
	pInterface->GetPropGeneric(hObject,"ObjectName",&gProp);

	SAFE_STRCPY(tempStr,gProp.m_String);
	cParse.Init(tempStr);
	while(pInterface->Parse(&cParse) == LT_OK)
	{
		if(cParse.m_nArgs != 1 ||
			pInterface->FindObject(cParse.m_Args[0], hTestObj) != LT_OK ||
			pInterface->GetClassName(hTestObj, className, sizeof(className)) != LT_OK)
		{
			continue;
		}

		pInterface->GetPropGeneric(hTestObj,"Name",&gProp);
		SAFE_STRCPY(pArray[nCounter].m_WorldModelName,gProp.m_String);

//		pInterface->CPrint("  - Generating preliminary info for '%s'",
//			gProp.m_String);

		pInterface->GetWorldModelRuntimePos(hTestObj,pArray[nCounter].m_vPos);

		pInterface->GetPropGeneric(hTestObj,"Rotation",&gProp);
		pArray[nCounter].m_rRot = gProp.m_Rotation;

		nCounter++;
	}

	*pModels = pArray;
	return nObjects;
}


// ----------------------------------------------------------------------- //
// Preprocessor callback.. generates light animations for KeyframerLights.
// ----------------------------------------------------------------------- //
void KeyframerPreprocessorCB(HPREOBJECT hObject, ILTPreLight *pInterface)
{
	GenericProp gProp;
	ConParse cParse;
	char className[32], keyName[128], baseKeyName[128];
	HPREOBJECT hTestObj, hKeyObj;

	KLProps props;

	LTVector vKey1, vKey2;
	LTRotation rKey1, rKey2;
	float t;

	PreLightAnimFrameInfo frameInfos[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	PreWorldModelInfo *modelInfos[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	PreLightInfo lightInfos[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	PreLightInfo shadowInfos[MAX_KEYFRAMER_SHADOW_LIGHTS];
	PreLightAnimFrameInfo *pFrameInfo;
	PreWorldModelInfo *pModelInfo;
	PreLightInfo *pLightInfo;
	uint32 i, nFrames, iStep;
	char animName[256];

	LTVector keyPositions[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	LTRotation keyRotations[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	uint32 keyFrames[MAX_KEYFRAMER_KEYLIGHT_FRAMES];
	LTVector vUp, vRight;
	LTRotation rRot;
	uint32 nKeyPositions;
	uint32 nTotalKeySteps;
	uint32 nShadows;
	uint32 nShadowedModels;
	LTBOOL bIgnoreOffsets;

	PREWORLDMODEL *pWorldModels;

	/* Sanity check and part of below code */
	if (pInterface->GetPropGeneric(hObject, "BaseKeyName", &gProp) != LT_OK)
		return;

	// Get the key information for later use
	nKeyPositions = 0;
	nTotalKeySteps = 0;
	SAFE_STRCPY(baseKeyName, gProp.m_String);
	for(i=0; i < 32000; i++)
	{
		GetKeyNameString(baseKeyName, i, keyName);

		if(pInterface->FindObject(keyName, hKeyObj) != LT_OK)
			break;

		if(nTotalKeySteps >= MAX_KEYFRAMER_KEYLIGHT_FRAMES)
			break;

		pInterface->GetPropGeneric(hKeyObj, "Pos", &gProp);
		keyPositions[nKeyPositions] = gProp.m_Vec;

		pInterface->GetPropGeneric(hKeyObj, "Rotation", &gProp);
		keyRotations[nKeyPositions] = gProp.m_Rotation;

		pInterface->GetPropGeneric(hKeyObj, "LightFrames", &gProp);
		keyFrames[nKeyPositions] = LTCLAMP(gProp.m_Long, 0, MAX_KEYFRAMER_KEYLIGHT_FRAMES);
		nTotalKeySteps += keyFrames[nKeyPositions];

		nKeyPositions++;
	}

	if(nKeyPositions < 2)
		return;

	/* Count and set up our shadow lights */
	nShadows = 0;
	if(pInterface->GetPropGeneric(hObject, "ShadowLights", &gProp) == LT_OK)
	{
		cParse.Init(gProp.m_String);
		while(pInterface->Parse(&cParse) == LT_OK)
		{
			if(cParse.m_nArgs != 1 ||
				pInterface->FindObject(cParse.m_Args[0], hTestObj) != LT_OK ||
				pInterface->GetClassName(hTestObj, className, sizeof(className)) != LT_OK ||
				stricmp(className, KEYFRAMERLIGHT_CLASSNAME) != 0)
			{
				continue;
			}

			ReadKLProps(pInterface, hTestObj, &props);

			pLightInfo = (PreLightInfo *)&(shadowInfos[nShadows]);
			pLightInfo->m_bDirectional = props.m_bDirLight;
			pLightInfo->m_FOV = props.m_fFOV;
			pLightInfo->m_Radius = props.m_fRadius;

			pLightInfo->m_vPos = props.m_vPos;
			pLightInfo->m_vDirection = props.m_vForwardVec;

			pLightInfo->m_vInnerColor = props.m_vInnerColor;
			pLightInfo->m_vOuterColor = props.m_vOuterColor;

			nShadows++;
		}

		pInterface->GetPropGeneric(hObject,"Name",&gProp);
		pInterface->CPrint("Counted %d shadow lights for keyframer '%s'",
			nShadows, gProp.m_String);
	}

	if(pInterface->GetPropGeneric(hObject, "IgnoreOffsets", &gProp) != LT_OK)
		bIgnoreOffsets = LTFALSE;
	else
		bIgnoreOffsets = gProp.m_Bool;

	if(pInterface->GetPropGeneric(hObject, "ObjectName", &gProp) != LT_OK)
		return;

	nShadowedModels = 0;
	if (nShadows)
	{
		nShadowedModels = AllocWorldModelsCB(pInterface, hObject, &pWorldModels);
		if (!nShadowedModels)
		{
			pWorldModels = LTNULL;
			nShadowedModels = 0;
		}
	}

	// Process the animated keyframer lights
	cParse.Init(gProp.m_String);
	while(pInterface->Parse(&cParse) == LT_OK)
	{
		if(cParse.m_nArgs != 1 ||
			pInterface->FindObject(cParse.m_Args[0], hTestObj) != LT_OK ||
			pInterface->GetClassName(hTestObj, className, sizeof(className)) != LT_OK)
		{
			continue;
		}

		/* Are we a keyframer light? */
		if (stricmp(className, KEYFRAMERLIGHT_CLASSNAME) == 0)
		{
			// Get light properties.
			ReadKLProps(pInterface, hTestObj, &props);

			// Pay attention to the IgnoreOffsets flag
			if (!bIgnoreOffsets)
			{
				// Get the position of the light relative to the first frame
				LTMatrix mInverse;
				pInterface->GetMathLT()->SetupTransformationMatrix(mInverse, keyPositions[0], keyRotations[0]);
				mInverse = mInverse.MakeInverseTransform();
				mInverse.Apply(props.m_vPos);
				mInverse.Apply3x3(props.m_vForwardVec);
			}

			// Ok, generate the lighting.
			nFrames = 0;
			for(i=0; i < (nKeyPositions-1); i++)
			{
				vKey1 = keyPositions[i];
				vKey2 = keyPositions[i+1];
				rKey1 = keyRotations[i];
				rKey2 = keyRotations[i+1];

				for(iStep=0; iStep < keyFrames[i]; iStep++)
				{
					if(keyFrames[i] <= 1)
						t = 0.0f;
					else
						t = (float)iStep / (keyFrames[i] - 1);

					if(nFrames >= MAX_KEYFRAMER_KEYLIGHT_FRAMES)
						break;

					pFrameInfo = &frameInfos[nFrames];
					pLightInfo = &lightInfos[nFrames];

					pFrameInfo->m_Lights = pLightInfo;
					pFrameInfo->m_nLights = 1;
					pFrameInfo->m_bSunLight = LTFALSE;

					pLightInfo->m_bDirectional = props.m_bDirLight;
					pLightInfo->m_FOV = props.m_fFOV;
					pLightInfo->m_Radius = props.m_fRadius;

					pInterface->GetMathLT()->InterpolateRotation(rRot, rKey1, rKey2, t);
					pInterface->GetMathLT()->GetRotationVectors(rRot, vRight, vUp, pLightInfo->m_vDirection);

					pLightInfo->m_vPos = vKey1 + (vKey2 - vKey1) * t;

					// Pay attention to the IgnoreOffsets flag
					if (!bIgnoreOffsets)
					{
						// Build a transformation for the interpolated position & orientation
						LTMatrix mTransform;
						pInterface->GetMathLT()->SetupTransformationMatrix(mTransform, pLightInfo->m_vPos, rRot);

						// Bring back the first-frame-relative position and orientation of the light
						pLightInfo->m_vPos = props.m_vPos;
						pLightInfo->m_vDirection = props.m_vForwardVec;

						// Apply the interpolated keyframe transformation
						mTransform.Apply(pLightInfo->m_vPos);
						mTransform.Apply3x3(pLightInfo->m_vDirection);
					}

					pLightInfo->m_vInnerColor = props.m_vInnerColor;
					pLightInfo->m_vOuterColor = props.m_vOuterColor;

					++nFrames;
				}
			}

			pInterface->GetPropGeneric(hTestObj, "Name", &gProp);
			GetKLLightAnimName(animName, gProp.m_String);

			pInterface->CreateLightAnim(animName, frameInfos, nFrames, props.m_bUseShadowMaps);
		}
	}

	if (!nShadows || !nShadowedModels) return;

	// Process the static keyframer lights
	nFrames = 0;
	for(i=0; i < nKeyPositions; i++)
	{
		vKey1 = keyPositions[i];
		vKey2 = keyPositions[(i != (nKeyPositions - 1)) ? i+1 : 0];
		rKey1 = keyRotations[i];
		rKey2 = keyRotations[(i != (nKeyPositions - 1)) ? i+1 : 0];

		for(iStep=0; iStep < keyFrames[i]; iStep++)
		{
			if(keyFrames[i] <= 1)
				t = 0.0f;
			else
				t = (float)iStep / (keyFrames[i] - 1);

			if(nFrames >= MAX_KEYFRAMER_KEYLIGHT_FRAMES)
				break;

			modelInfos[nFrames] = (PreWorldModelInfo *)malloc(sizeof(PreWorldModelInfo) * nShadowedModels);

			pFrameInfo = &frameInfos[nFrames];
			pLightInfo = (PreLightInfo *)(&shadowInfos[0]);
			pModelInfo = modelInfos[nFrames];

			pFrameInfo->m_Lights = pLightInfo;
			pFrameInfo->m_nLights = nShadows;
			pFrameInfo->m_bSunLight = LTFALSE;

			pInterface->GetMathLT()->InterpolateRotation(rRot, rKey1, rKey2, t);

			for (int i2 = 0; i2 < (int)nShadowedModels; i2++)
			{
				pModelInfo[i2].m_Pos = (vKey1 + (vKey2 - vKey1) * t) - (keyPositions[0] - pWorldModels[i2].m_vPos);
				pModelInfo[i2].m_Rot = rRot;
				pModelInfo[i2].m_bInvisible = LTFALSE;
				SAFE_STRCPY(pModelInfo[i2].m_WorldModelName,pWorldModels[i2].m_WorldModelName);
			}
			pFrameInfo->m_WorldModels = pModelInfo;
			pFrameInfo->m_nWorldModels = nShadowedModels;

			++nFrames;
		}
	}

	pInterface->GetPropGeneric(hObject, "Name", &gProp);
	GetKFModelAnimName(animName, gProp.m_String);
	pInterface->GetPropGeneric(hObject, "ShadowMaps", &gProp);
	pInterface->CreateLightAnim(animName, frameInfos, nFrames, gProp.m_Bool);

//	pInterface->CPrint("  - Freeing shadowed model memory.");

	for (i = 0; i < nFrames; i++)
	{
		if (modelInfos[i])
			free(modelInfos[i]);
	}

	if (pWorldModels)
	{
		free(pWorldModels);
	}
}


LTRESULT KeyframerPlugin::PreHook_Light(
    ILTPreLight *pInterface,
	HPREOBJECT hObject)
{
	// Just call thru to the old-style implementation.
	KeyframerPreprocessorCB(hObject, pInterface);
	return LT_OK;
}


// Add/subtract the VALUES of the rotation data.  Note that this has no geometric
// meaning - the keyframer just uses it to store offsets from one rotation to
// the next but it never actually goes BETWEEN rotations with this.
inline void AddRotationValues(
	LTRotation *pOut,
	LTRotation *pRot1,
	LTRotation *pRot2)
{
	pOut->m_Quat[0] = pRot1->m_Quat[0] + pRot2->m_Quat[0];
	pOut->m_Quat[1] = pRot1->m_Quat[1] + pRot2->m_Quat[1];
	pOut->m_Quat[2] = pRot1->m_Quat[2] + pRot2->m_Quat[2];
	pOut->m_Quat[3] = pRot1->m_Quat[3] + pRot2->m_Quat[3];
}

inline void SubtractRotationValues(
	LTRotation *pOut,
	LTRotation *pRot1,
	LTRotation *pRot2)
{
	pOut->m_Quat[0] = pRot1->m_Quat[0] - pRot2->m_Quat[0];
	pOut->m_Quat[1] = pRot1->m_Quat[1] - pRot2->m_Quat[1];
	pOut->m_Quat[2] = pRot1->m_Quat[2] - pRot2->m_Quat[2];
	pOut->m_Quat[3] = pRot1->m_Quat[3] - pRot2->m_Quat[3];
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::KeyFramer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyFramer::KeyFramer() : BaseClass(OT_NORMAL)
{
	m_hKeyframerLightClass = LTNULL;

	m_hstrObjectName	= LTNULL;
	m_hstrTargetName	= LTNULL;
	m_hstrBaseKeyName	= LTNULL;
	m_hstrActiveSnd		= LTNULL;
	m_pObjectList		= LTNULL;
	m_hTargetObject		= LTNULL;
	m_hActiveSnd		= LTNULL;
	m_hActiveSndObj		= LTNULL;
	m_hstrDestCmd		= LTNULL;

	m_bIgnoreOffsets	= LTFALSE;
	m_bStartActive		= LTFALSE;
	m_bStartPaused		= LTFALSE;
	m_bLooping			= LTFALSE;
	m_bActive			= LTFALSE;
	m_bPaused			= LTFALSE;
	m_bFinished			= LTFALSE;
	m_bUseVelocity		= LTFALSE;
	m_bAlignToPath		= LTFALSE;
	m_bFirstUpdate		= LTTRUE;
	m_bPushObjects		= LTTRUE;

	m_pKeys				= LTNULL;
	m_pCurKey			= LTNULL;
	m_pPosition1		= LTNULL;
	m_pPosition2		= LTNULL;
	m_pLastKey			= LTNULL;
	m_pDestinationKey	= LTNULL;

	m_nNumKeys			= 0;
	m_fCurTime			= 0.0f;
	m_fEndTime			= 0.0f;
	m_fTotalPathTime	= 0.0f;
	m_fTotalDistance	= 0.0f;
	m_fVelocity			= 0.0f;
	m_fSoundRadius		= 1000.0f;

	m_fEarliestGoActiveTime		= 0.0f;

	m_eDirection		= KFD_FORWARD;
	m_eWaveform			= KFWAVE_LINEAR;

	m_pOffsets.SetMemCopyable(1);
	m_pOffsets.SetGrowBy(3);
	m_pRotations.SetMemCopyable(1);
	m_pRotations.SetGrowBy(3);

	m_RotationWave = Wave_Sine;

	m_vTargetOffset.Init();
	m_vCurPos.Init();

	m_fKeyPercent = 0.0f;	 // Percent of distance/time between key1 and key2...
	m_pCommands = LTNULL;

	m_hLightAnim = INVALID_LIGHT_ANIM;

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		m_bPausedOnLoad = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::~KeyFramer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyFramer::~KeyFramer()
{
	if (m_hstrObjectName)
	{
        g_pLTServer->FreeString(m_hstrObjectName);
	}

	if (m_hstrTargetName)
	{
        g_pLTServer->FreeString(m_hstrTargetName);
	}

	if (m_hstrBaseKeyName)
	{
        g_pLTServer->FreeString(m_hstrBaseKeyName);
	}

	if (m_hstrActiveSnd)
	{
        g_pLTServer->FreeString(m_hstrActiveSnd);
	}

	if (m_hstrDestCmd)
	{
        g_pLTServer->FreeString(m_hstrDestCmd);
	}

	if (m_hActiveSnd)
	{
        // g_pLTServer->KillSoundLoop(m_hActiveSnd);
		g_pLTServer->KillSound(m_hActiveSnd);
	}

	if (m_pObjectList)
	{
        g_pLTServer->RelinquishList(m_pObjectList);
	}

	while (m_pKeys)
	{
		KEYNODE* pNext = m_pKeys->pNext;
		debug_delete(m_pKeys);
		m_pKeys = pNext;
	}

	if (m_pCommands)
	{
		debug_deletea(m_pCommands);
		m_pCommands = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoActive
//
//	PURPOSE:	Start the KeyFramer going!
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoActive(LTBOOL bReset)
{
	if (m_bActive || !m_pKeys || !m_pLastKey || !m_pObjectList || (m_pObjectList->m_nInList < 1)) return;

	// Go active

	m_bFinished = LTFALSE;
	m_bActive	= LTTRUE;

	if (bReset)
	{
		m_fCurTime	= (m_eDirection == KFD_FORWARD) ? 0.0f : m_fEndTime;
		m_pCurKey	= (m_eDirection == KFD_FORWARD) ? m_pKeys : m_pLastKey;
		m_vCurPos   = (m_eDirection == KFD_FORWARD) ? m_pKeys->keyData.m_vPos : m_pLastKey->keyData.m_vPos;
	}

    SetNextUpdate(m_hObject, KF_UPDATE_DELTA);


    m_fEarliestGoActiveTime = g_pLTServer->GetTime() + KF_UPDATE_DELTA;


	// Start active sound...

	if (m_hstrActiveSnd && g_pServerSoundMgr)
	{
        char* pSound = g_pLTServer->GetStringData(m_hstrActiveSnd);
		if (!pSound) return;

		if (m_hActiveSnd)
		{
            // g_pLTServer->KillSoundLoop(m_hActiveSnd);
			g_pLTServer->KillSound(m_hActiveSnd);
			m_hActiveSnd = LTNULL;
		}

		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		if (pLink && pLink->m_hObject)
		{
			m_hActiveSndObj = pLink->m_hObject;
			m_hActiveSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hActiveSndObj, pSound,
				m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoInActive
//
//	PURPOSE:	Stop the KeyFramer...
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoInActive()
{
	m_bActive = LTFALSE;
    SetNextUpdate(m_hObject, 0.0f);

	if (m_hActiveSnd)
	{
        //g_pLTServer->KillSoundLoop(m_hActiveSnd);
		g_pLTServer->KillSound(m_hActiveSnd);
		m_hActiveSnd = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CreateKeyList
//
//	PURPOSE:	Create our list of keys.
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::CreateKeyList()
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	int numObjects;
	HOBJECT hObject;

	if (!m_hstrObjectName || !m_hstrBaseKeyName || m_pObjectList) return LTFALSE;

	// Make sure our object and base key names are valid...

    char* pNames = g_pLTServer->GetStringData(m_hstrObjectName);
	if (!pNames || !pNames[0]) return LTFALSE;

    char* pBaseKeyName = g_pLTServer->GetStringData(m_hstrBaseKeyName);
	if (!pBaseKeyName || !pBaseKeyName[0]) return LTFALSE;


	// Find all the objects with the given names (m_hstrObjectName may be
	// of the form: name1;name2;name3...

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return LTFALSE;

	ConParse parse;
	parse.Init(pNames);

    m_pObjectList = g_pLTServer->CreateObjectList();
	if (!m_pObjectList) return LTFALSE;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			// Find the objects...

            g_pLTServer->FindNamedObjects(parse.m_Args[0],objArray);

			numObjects = objArray.NumObjects();

			for (int i = 0; i < numObjects; i++)
			{
				hObject = objArray.GetObject(i);
                g_pLTServer->AddObjectToList(m_pObjectList, hObject);
                g_pLTServer->CreateInterObjectLink(m_hObject, hObject);
			}
		}
	}


	// If there aren't any objects, don't continue...

	if (m_pObjectList->m_nInList < 1)
	{
        g_pLTServer->RelinquishList(m_pObjectList);
		m_pObjectList = LTNULL;
		return LTFALSE;
	}


	// Create the list of keys...

	int nNum = 0;
	LTFLOAT fTime = 0.0f;
	KEYNODE* pNode = LTNULL;
	char strKey[128];

	while (1)
	{
		// Create the keyname string...

		GetKeyNameString(pBaseKeyName, nNum, strKey);


		// Find the first key with that name...

		objArray.Reset();
        g_pLTServer->FindNamedObjects(strKey,objArray);
		numObjects = objArray.NumObjects();

		if (!numObjects) break;

		LTBOOL bFoundKey = LTFALSE;

		for (int i = 0; i < numObjects; i++)
		{
			hObject = objArray.GetObject(i);

            if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObject), g_pLTServer->GetClass ("Key")))
			{
				// Add this key to the list...

				if (!m_pKeys)
				{
					m_pKeys = debug_new(KEYNODE);
					pNode = m_pKeys;
				}
				else
				{
					pNode->pNext = debug_new(KEYNODE);
					pNode->pNext->pPrev = pNode;
					pNode = pNode->pNext;
				}


				// Copy the key...

                pNode->keyData.Copy((Key*)g_pLTServer->HandleToObject(hObject));

				fTime += pNode->keyData.m_fTimeStamp;
				pNode->keyData.m_fRealTime = fTime;


				// Remove the key from the world...No longer needed...

                g_pLTServer->RemoveObject(hObject);

				bFoundKey = LTTRUE;

				m_nNumKeys++;


				// Store the last key and key time...

				m_fEndTime = fTime;
				m_pLastKey = pNode;
			}
		}

		// If we didn't find a key, we're done looking...

		if (!bFoundKey)
		{
			break;
		}

		// Increment the counter...

		nNum++;
	}


	// If we didn't find any keys, return...

	if (!m_pKeys)
	{
		return LTFALSE;
	}


	// See if we need to calculate the velocity info...

	if (m_bUseVelocity > 0.0)
	{
		CalculateVelocityInfo();
	}


	// For each object we're controlling, save it's offset from the first key...

	KEYNODE* pFirstPosKey = m_pKeys;

	if (pFirstPosKey)
	{
		// Get the first position key's location

		LTVector vKeyPos;
		LTRotation rKeyRot;

		vKeyPos = pFirstPosKey->keyData.m_vPos;
		rKeyRot = pFirstPosKey->keyData.m_rRot;

		long i = 0;
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink)
		{
			LTVector vObjPos;
            g_pLTServer->GetObjectPos(pLink->m_hObject, &vObjPos);

			m_pOffsets[i] = vObjPos - vKeyPos;

			LTRotation rObjRot;
            g_pLTServer->GetObjectRotation(pLink->m_hObject, &rObjRot);

			SubtractRotationValues(
				&m_pRotations[i],
				&rObjRot,
				&rKeyRot);

			i++;
			pLink = pLink->m_pNext;
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CalculateVelocityInfo
//
//	PURPOSE:	Calculate the velocity info for all the keys if using
//				velocity to calculate position information
//
// ----------------------------------------------------------------------- //

void KeyFramer::CalculateVelocityInfo()
{
	if (m_fTotalPathTime <= 0.0 || !m_pKeys) return;

	m_fTotalDistance = 0.0f;

	// Calculate the total distance between all the keys...

	KEYNODE* pCurKey = m_pKeys;
	while (pCurKey && pCurKey->pNext)
	{
		if( pCurKey->keyData.m_bNextValid && pCurKey->pNext->keyData.m_bPrevValid )
		{
			m_fTotalDistance += Bezier_SegmentLength(
				CURVE_PTS(pCurKey->keyData, pCurKey->pNext->keyData));
			pCurKey->pNext->keyData.m_fDistToLastKey = m_fTotalDistance;
		}
		else
		{
			m_fTotalDistance += VEC_DIST( pCurKey->keyData.m_vPos, pCurKey->pNext->keyData.m_vPos );
			pCurKey->pNext->keyData.m_fDistToLastKey = m_fTotalDistance;
		}

		pCurKey = pCurKey->pNext;
	}


	// Calculate linear velocity...

	m_fVelocity = m_fTotalDistance / m_fTotalPathTime;


	// Set each key's distance to last key...

	pCurKey = m_pKeys;
	while (pCurKey)
	{
		pCurKey->keyData.m_fDistToLastKey = m_fTotalDistance - pCurKey->keyData.m_fDistToLastKey;

		pCurKey = pCurKey->pNext;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ProcessCurrentKey
//
//	PURPOSE:	Processes the current key
//
// ----------------------------------------------------------------------- //

void KeyFramer::ProcessCurrentKey()
{
	if (!m_pCurKey) return;


	// Keep track of the current key...

	KEYNODE* pOldCurKey = m_pCurKey;


	// Set the pos1 and pos2 keys...

	if (pOldCurKey->keyData.m_nKeyType & POSITION_KEY)
	{
		if (m_eDirection == KFD_FORWARD)
		{
			m_pPosition1 = pOldCurKey;
			m_pPosition2 = pOldCurKey->pNext;
		}
		else
		{
			m_pPosition2 = pOldCurKey;
			m_pPosition1 = pOldCurKey->pPrev;
		}
	}


	// Adjust m_pCurKey...

	if (m_eDirection == KFD_FORWARD)
	{
		m_pCurKey = m_pCurKey->pNext;
	}
	else
	{
		m_pCurKey = m_pCurKey->pPrev;
	}


	// Process the key we just passed...

	if (pOldCurKey->keyData.m_nKeyType & SOUND_KEY)
	{
        char* pSound = pOldCurKey->keyData.m_hstrSoundName ? g_pLTServer->GetStringData(pOldCurKey->keyData.m_hstrSoundName) : LTNULL;
		if (pSound)
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, pOldCurKey->keyData.m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}

	if (pOldCurKey->keyData.m_nKeyType & MESSAGE_KEY)
	{
		SendTriggerMsgToObjects(this, pOldCurKey->keyData.m_hstrMessageTarget, pOldCurKey->keyData.m_hstrMessageName);
	}

	if (pOldCurKey->keyData.m_nKeyType & BPRINT_KEY)
	{
        g_pLTServer->BPrint(g_pLTServer->GetStringData(pOldCurKey->keyData.m_hstrBPrintMessage));
	}


	// If we're moving to a destination, see if we have reached it yet...

	if (m_pDestinationKey)
	{
		if (pOldCurKey == m_pDestinationKey)
		{
			ReachedDestination();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReachedDestination
//
//	PURPOSE:	Handle reaching the destination key...
//
// ----------------------------------------------------------------------- //

void KeyFramer::ReachedDestination()
{
	// Default behavoir...Pause the keyframer...

	Pause();


	// If we have a destination command, process it...

	if (m_hstrDestCmd)
	{
        char* pCmd = g_pLTServer->GetStringData(m_hstrDestCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}

        g_pLTServer->FreeString(m_hstrDestCmd);
		m_hstrDestCmd = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 KeyFramer::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProps();
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 KeyFramer::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleLinkBroken()
//
//	PURPOSE:	Handle MID_LINKBROKEN engine message
//
// --------------------------------------------------------------------------- //

void KeyFramer::HandleLinkBroken(HOBJECT hLink)
{
	if (!m_pObjectList || !hLink) return;

	// Kill active sound if object associated with it goes away...

	if (m_hActiveSndObj == hLink)
	{
		if (m_hActiveSnd)
		{
            //g_pLTServer->KillSoundLoop(m_hActiveSnd);
			g_pLTServer->KillSound(m_hActiveSnd);
			m_hActiveSnd = LTNULL;
			m_hActiveSndObj = LTNULL;
		}
	}

	ObjectLink* pLink = m_pObjectList->m_pFirstLink;
	ObjectLink* pPrevious = LTNULL;

	while (pLink)
	{
		if (pLink->m_hObject == hLink)
		{
			m_pObjectList->m_nInList--;

			if (pLink == m_pObjectList->m_pFirstLink)
			{
				//m_pObjectList->m_pFirstLink = LTNULL;
				m_pObjectList->m_pFirstLink = pLink->m_pNext;
			}
			else if (pPrevious)
			{
				pPrevious->m_pNext = pLink->m_pNext;
			}

			// If the list is now empty, remove it...

			if (!m_pObjectList->m_nInList)
			{
                g_pLTServer->RelinquishList(m_pObjectList);
				m_pObjectList = LTNULL;
			}

			return;
		}

		pPrevious = pLink;
		pLink = pLink->m_pNext;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::TriggerMsg()
//
//	PURPOSE:	Process keyframer trigger messages
//
// --------------------------------------------------------------------------- //

void KeyFramer::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "ON") == 0)
			{
				On();
			}
			else if (_stricmp(parse.m_Args[0], "OFF") == 0)
			{
				Off();
			}
			else if (_stricmp(parse.m_Args[0], "PAUSE") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					Pause();
				}
			}
			else if (_stricmp(parse.m_Args[0], "RESUME") == 0)
			{
				Resume();
			}
			else if (_stricmp(parse.m_Args[0], "FORWARD") == 0)
			{
				Forward();
			}
			else if ((_stricmp(parse.m_Args[0], "BACKWARD") == 0) ||
					 (_stricmp(parse.m_Args[0], "REVERSE") == 0))
			{
				Backward();
			}
			else if (_stricmp(parse.m_Args[0], "TOGGLEDIR") == 0)
			{
				ToggleDir();
			}
			else if (_stricmp(parse.m_Args[0], "GOTO") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					if (parse.m_nArgs > 1 && parse.m_Args[1])
					{
						GoToKey(parse.m_Args[1]);
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "MOVETO") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					MoveToKey(parse);
				}
			}
			else if (_stricmp(parse.m_Args[0], "TARGET") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					if (parse.m_nArgs > 1 && parse.m_Args[1])
					{
						SetTarget(parse.m_Args[1]);
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "CLEARTARGET") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					SetTarget(LTNULL);
				}
			}
			else if (_stricmp(parse.m_Args[0], "TARGETOFFSET") == 0)
			{
				if (m_bFirstUpdate)
				{
					DeferCommand(parse);
				}
				else
				{
					if (parse.m_nArgs > 3)
					{
						m_vTargetOffset.x = (LTFLOAT) atof(parse.m_Args[1]);
						m_vTargetOffset.y = (LTFLOAT) atof(parse.m_Args[2]);
						m_vTargetOffset.z = (LTFLOAT) atof(parse.m_Args[3]);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::DeferCommand
//
//	PURPOSE:	Defer a command to be processed later...
//
// ----------------------------------------------------------------------- //

void KeyFramer::DeferCommand(ConParse parse)
{
	// Alloc if we haven't already
	// m_pCommands will get deleted at the end of the first update.
	if(!m_pCommands)
	{
		m_pCommands = debug_newa(char, 1024);
		memset(m_pCommands,0,1024);
	}
	else
	{
		// We already have some commands so throw on a separator
		strcat(m_pCommands,";");
	}

	int i=0;
	while(parse.m_nArgs > i && parse.m_Args[i])
	{
		// Throw on a space if necessary
		if(i > 0)
			strcat(m_pCommands," ");

		strcat(m_pCommands,parse.m_Args[i]);
		i++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReadProps
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::ReadProps()
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("ObjectName", &genProp) == LT_OK)
	{
		SetObjectName(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("TargetName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrTargetName = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("TargetOffset", &genProp) == LT_OK)
	{
		m_vTargetOffset = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("BaseKeyName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrBaseKeyName = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("ActiveSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
             m_hstrActiveSnd = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartPaused", &genProp) == LT_OK)
	{
		m_bStartPaused = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Looping", &genProp) == LT_OK)
	{
		m_bLooping = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("IgnoreOffsets", &genProp) == LT_OK)
	{
		m_bIgnoreOffsets = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PushObjects", &genProp) == LT_OK)
	{
		m_bPushObjects = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("AlignToPath", &genProp) == LT_OK)
	{
		m_bAlignToPath = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("TotalPathTime", &genProp) == LT_OK)
	{
		m_fTotalPathTime = genProp.m_Float;
		m_bUseVelocity = (m_fTotalPathTime > 0.0 ? LTTRUE : LTFALSE);
	}

	// Set up waveform (don't need to check for Linear)...

	m_eWaveform = KFWAVE_LINEAR;

    if (g_pLTServer->GetPropGeneric("Sine", &genProp) == LT_OK)
	{
		m_eWaveform = genProp.m_Bool ? KFWAVE_SINE : m_eWaveform;
	}

    if (g_pLTServer->GetPropGeneric("SlowOff", &genProp) == LT_OK)
	{
		m_eWaveform = genProp.m_Bool ? KFWAVE_SLOWOFF : m_eWaveform;
	}

    if (g_pLTServer->GetPropGeneric("SlowOn", &genProp) == LT_OK)
	{
		m_eWaveform = genProp.m_Bool ? KFWAVE_SLOWON : m_eWaveform;
	}

	m_RotationWave = Wave_Sine;
    if(g_pLTServer->GetPropGeneric("RotationWave", &genProp) == LT_OK)
	{
		m_RotationWave = ParseWaveType(genProp.m_String);
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::SetObjectName
//
//	PURPOSE:	Set the object name...
//
// ----------------------------------------------------------------------- //

void KeyFramer::SetObjectName(char* pName)
{
	if (!pName || !pName[0]) return;

	if (m_hstrObjectName)
	{
        g_pLTServer->FreeString(m_hstrObjectName);
	}

     m_hstrObjectName = g_pLTServer->CreateString(pName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::SetTarget
//
//	PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void KeyFramer::SetTarget(char* pName)
{
	if (m_hTargetObject)
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hTargetObject);
		m_hTargetObject = LTNULL;
	}

	if (!pName) return;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(pName, objArray);

	if (objArray.NumObjects())
	{
		m_hTargetObject = objArray.GetObject(0);
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hTargetObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void KeyFramer::InitialUpdate()
{
	if (!g_SynchronizeKeyframers.IsInitted())
        g_SynchronizeKeyframers.Init(g_pLTServer, "SynchKeyframers", LTNULL, 0.0f);

	char objectName[64], animName[128];

    if(g_pLTServer->GetObjectName(m_hObject, objectName, sizeof(objectName)) == LT_OK)
	{
		GetKFModelAnimName(animName, objectName);
        g_pLTServer->GetLightAnimLT()->FindLightAnim(animName, m_hLightAnim);
	}

    m_hKeyframerLightClass = g_pLTServer->GetClass(KEYFRAMERLIGHT_CLASSNAME);

    SetNextUpdate(m_hObject, KF_UPDATE_DELTA);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

// BL 09/29/00 Added to fix falling off keyframed objects after loading game
extern int32 g_bPlayerUpdated;

void KeyFramer::Update()
{
	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		if ( m_bPausedOnLoad )
		{
			if ( g_bPlayerUpdated < 0 )
			{
				m_bPausedOnLoad = LTFALSE;
			}
			else
			{
				SetNextUpdate(m_hObject, KF_UPDATE_DELTA);
				return;
			}
		}
	}

	// Don't update until time...

	if (g_SynchronizeKeyframers.GetFloat() != 0.0f)
	{
        LTFLOAT fGetTime = g_pLTServer->GetTime();
		if (fGetTime <= m_fEarliestGoActiveTime)
		{
            g_pLTServer->CPrint("KeyframerSynch: To early to update!");
			return;
		}
	}

	// Need to check this here (instead of InitialUpdate)...This insures
	// that all the Keys have been added to the world...

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;

		CreateKeyList();

		if (m_hstrTargetName)
		{
            char* pName = g_pLTServer->GetStringData(m_hstrTargetName);
			SetTarget(pName);
		}

		// Must be active to pause...

		if (m_bStartActive || m_bStartPaused)
		{
			GoActive();
		}

		if (m_bStartPaused)
		{
			Pause();
		}

		// Check for deferred commands
		if (m_pCommands)
		{
			TriggerMsg(m_hObject, m_pCommands);
			debug_deletea(m_pCommands);
			m_pCommands = LTNULL;
		}

		return; // Don't process objects this frame...
	}


	// See if we are even supposed to be here (today)...

	if (!m_bActive)
	{
		GoInActive();
		return;
	}
	else
	{
        SetNextUpdate(m_hObject, KF_UPDATE_DELTA);
	}


	// Increment timer

    float fTime = g_pLTServer->GetFrameTime();
	m_fCurTime += (m_eDirection == KFD_FORWARD) ? fTime : -fTime;


	// Process all keys that we might have passed by (NOTE: we check for
	// m_bActive again because keys we process may pause the keyframer or
	// turn it off).

	while (m_pCurKey && m_bActive)
	{
		if (m_bUseVelocity)
		{
			if (!m_vCurPos.Equals(m_pCurKey->keyData.m_vPos))
            {
				break;
			}
		}
		else
		{
			if (m_eDirection == KFD_FORWARD)
			{
				if (m_pCurKey->keyData.m_fRealTime > m_fCurTime) break;
			}
			else
			{
				if (m_pCurKey->keyData.m_fRealTime < m_fCurTime) break;
			}
		}

		ProcessCurrentKey();
	}

	// Are we at the end of the key list?

	if (!m_pCurKey)
	{
		if (m_eDirection == KFD_FORWARD)
		{
			m_pCurKey  = m_pKeys;
			m_fCurTime = 0.0f;
			m_vCurPos  = m_pKeys->keyData.m_vPos;
		}
		else
		{
			m_pCurKey  = m_pLastKey;
			m_fCurTime = m_fEndTime;
			m_vCurPos  = m_pLastKey->keyData.m_vPos;
		}

		if (!m_bLooping)
		{
			m_bFinished = LTTRUE;
		}
	}

	// Update (move/rotate) all the object(s)...(NOTE: we check for
	// m_bActive again because keys we process may pause the keyframer or
	// turn it off).

	if (m_bActive)
	{
		UpdateObjects();
	}

	if (m_bFinished)
		GoInActive();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::UpdateObjects()
//
//	PURPOSE:	Update the object(s) position(s) / rotation(s)
//
// ----------------------------------------------------------------------- //

void KeyFramer::UpdateObjects(LTBOOL bInterpolate, LTBOOL bTeleport)
{
	if (!m_pPosition1 || !m_pPosition2) return;

	// bAtKey is set to true if we're at a key...

	LTBOOL bAtKey = LTFALSE;


	// Calculate m_vCurPos...

	if (!CalcCurPos(bAtKey)) return;


	// Get the new angle from position 1's angle

	LTRotation rRotNew, rRot1, rRot2;
	rRotNew.Init();
	rRot1 = m_pPosition1->keyData.m_rRot;
	rRot2 = m_pPosition2->keyData.m_rRot;

	if (bAtKey)
	{
		rRotNew = (m_eDirection == KFD_FORWARD) ? rRot2 : rRot1;
	}
	else
	{
        g_pLTServer->InterpolateRotation(&rRotNew, &rRot1, &rRot2, GetWaveFn(m_RotationWave)(m_fKeyPercent));
	}


	// Now add the relative position and rotation to every object in
	// the list...

	long i = 0;
	ObjectLink* pLink = m_pObjectList ? m_pObjectList->m_pFirstLink : LTNULL;
	while (pLink)
	{
		LTVector vOldPos;
        g_pLTServer->GetObjectPos(pLink->m_hObject, &vOldPos);

		// Set object's new position...

		LTVector vPos = m_vCurPos;

		if (!m_bIgnoreOffsets)
		{
			vPos += m_pOffsets[i];
		}

		if (m_bPushObjects && !bTeleport)
		{
            g_pLTServer->MoveObject(pLink->m_hObject, &vPos);
		}
		else
		{
            g_pLTServer->SetObjectPos(pLink->m_hObject, &vPos);
		}


		// See if we can rotate this object...

		//if (CanRotateObject(pLink->m_hObject))
		//{
			// Initialize to normal rotation calculation...

			rRot1 = rRotNew;


			// If we have a target object, align the object to face it...

			if (m_hTargetObject)
			{
				LTVector vTargetPos;
                g_pLTServer->GetObjectPos(m_hTargetObject, &vTargetPos);

				vTargetPos += m_vTargetOffset;

				LTVector vDir = vTargetPos - vPos;
				vDir.Norm();

                g_pLTServer->AlignRotation(&rRot1, &vDir, LTNULL);
			}
			else if (m_bAlignToPath)
			{
				// Align the object to the path, if necessary...

				LTVector vDir = vPos - vOldPos;
				vDir.Norm();

				if (m_eDirection == KFD_BACKWARD)
				{
					vDir = -vDir;
				}

                g_pLTServer->AlignRotation(&rRot1, &vDir, LTNULL);

				// Now adjust the rotation to account for the pitch and roll of
				// the key...

				LTVector vNewPYR;
				LTVector vPYR1 = m_pPosition1->keyData.m_vPitchYawRoll;
				LTVector vPYR2 = m_pPosition2->keyData.m_vPitchYawRoll;
				VEC_LERP(vNewPYR, vPYR1, vPYR2, m_fKeyPercent);

				LTVector vU, vR, vF;
                g_pLTServer->GetRotationVectors(&rRot1, &vU, &vR, &vF);

				// Adjust pitch...

                g_pLTServer->RotateAroundAxis(&rRot1, &vR, vNewPYR.x);

				// Adjust roll...

                g_pLTServer->RotateAroundAxis(&rRot1, &vF, vNewPYR.z);
			}


			if (!m_bIgnoreOffsets && !m_hTargetObject)
			{
				AddRotationValues(
					&rRot1,
					&rRot1,
					&m_pRotations[i]);
			}


			// Set object's new rotation...

			/*
            g_pLTServer->GetObjectRotation( pLink->m_hObject, &rRot2 );

			// See if the object rotated significantly.
			if ( fabs(rRot1.m_Spin - rRot2.m_Spin) > 0.0f || VEC_DISTSQR(rRot1.m_Vec, rRot2.m_Vec) > 0.0f)
			{*/

				if (bInterpolate)
				{
                    g_pLTServer->RotateObject(pLink->m_hObject, &rRot1);
				}
				else
				{
                    g_pLTServer->SetObjectRotation(pLink->m_hObject, &rRot1);
				}

				/* // See if we should rotate the dims of the object.
                if (g_pLTServer->GetObjectType(pLink->m_hObject) == OT_MODEL)
				{
                    HMODELANIM hAni = g_pLTServer->GetModelAnimation(pLink->m_hObject);
					if (hAni != INVALID_ANI)
					{
						LTVector vDims;
                        g_pLTServer->GetModelAnimUserDims( pLink->m_hObject, &vDims, hAni );
						Utils::RotateDims(rRot1, vDims);
                        g_pLTServer->SetObjectDims2(pLink->m_hObject, &vDims);
					}
				}
			}*/

		//}

		// Handle KeyframerLights.
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(pLink->m_hObject), m_hKeyframerLightClass))
		{
			LTVector vLightPos;
            g_pLTServer->GetObjectPos(pLink->m_hObject, &vLightPos);
            UpdateKeyframerLight((KeyframerLight*)g_pLTServer->HandleToObject(pLink->m_hObject), vLightPos);
		}

		i++;
		pLink = pLink->m_pNext;
	}

	// Handle shadowed objects
	if (m_hLightAnim != INVALID_LIGHT_ANIM)
	{
		UpdateShadowObjects();
	}
}

void KeyFramer::CalcLMAnimFrames(uint32 &uFrame1, uint32 &uFrame2, float &fPercent)
{
	uint32 uFrameOffset;
	uint32 uFrameStart, uFrameCount;
	float frame0Percent, frame1Percent;

	// Figure out what light keys we're on.
	KEYNODE *pKeySearch = m_pKeys;
	uFrameStart = 0;
	while (pKeySearch != m_pPosition1)
	{
		uFrameStart += pKeySearch->keyData.m_LightFrames;
		pKeySearch = pKeySearch->pNext;
	}
	uFrameCount = m_pPosition1->keyData.m_LightFrames;

	// See which step we're on and what percentage within that..
	if (uFrameCount == 0)
	{
		fPercent = 0.0f;
		uFrameOffset = 0;
	}
	else if(uFrameCount == 1)
	{
		fPercent = m_fKeyPercent;
		uFrameOffset = 0;
	}
	else
	{
		uFrameOffset = (uint32)(m_fKeyPercent * (uFrameCount - 1));
		frame0Percent = (float)uFrameOffset / (uFrameCount - 1);
		frame1Percent = (float)(uFrameOffset + 1) / (uFrameCount - 1);

		fPercent = 0.0f;
		if(frame1Percent - frame0Percent > 0.01f)
			fPercent = (m_fKeyPercent - frame0Percent) / (frame1Percent - frame0Percent);
	}

	// Return the keys
	uFrame1 = uFrameStart + uFrameOffset;
	uFrame2 = uFrame1 + 1;
}

void KeyFramer::UpdateShadowObjects()
{
	uint32 uLightAnimFrame1, uLightAnimFrame2;
	float fPercent;
	LAInfo info;

	if (m_hLightAnim == INVALID_LIGHT_ANIM) return;

	CalcLMAnimFrames(uLightAnimFrame1, uLightAnimFrame2, fPercent);

	// Set it up!
    if(g_pLTServer->GetLightAnimLT()->GetLightAnimInfo(m_hLightAnim, info) == LT_OK)
	{
		info.m_iFrames[0] = uLightAnimFrame1;
		info.m_iFrames[1] = uLightAnimFrame2;
		info.m_fPercentBetween = fPercent;
		info.m_fBlendPercent = 1.0f;

        g_pLTServer->GetLightAnimLT()->SetLightAnimInfo(m_hLightAnim, info);
	}
}

void KeyFramer::UpdateKeyframerLight(KeyframerLight *pLight, LTVector vLightPos)
{
	uint32 uLightAnimFrame1, uLightAnimFrame2;
	float fPercent;
	LAInfo info;

	if(!pLight)
		return;

	if (pLight->m_hLightAnim == INVALID_LIGHT_ANIM) return;

	CalcLMAnimFrames(uLightAnimFrame1, uLightAnimFrame2, fPercent);

	// Set it up!
    if(g_pLTServer->GetLightAnimLT()->GetLightAnimInfo(pLight->m_hLightAnim, info) == LT_OK)
	{
		info.m_iFrames[0] = uLightAnimFrame1;
		info.m_iFrames[1] = uLightAnimFrame2;
		info.m_fPercentBetween = fPercent;
		info.m_fBlendPercent = 1.0f;
		info.m_vLightPos = vLightPos;
		info.m_vLightColor = pLight->m_vLightColor;
		info.m_fLightRadius = pLight->m_fLightRadius;

        g_pLTServer->GetLightAnimLT()->SetLightAnimInfo(pLight->m_hLightAnim, info);
	}
}


uint32 KeyFramer::GetKeyIndex(KEYNODE *pNode)
{
	KEYNODE *pCur;
	uint32 index;

	index = 0;
	for(pCur=m_pKeys; pCur; pCur=pCur->pNext)
	{
		if(pCur == pNode)
			return index;

		++index;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CalcCurPos()
//
//	PURPOSE:	Calculate m_fCurPos and fKeyPercent
//
// ----------------------------------------------------------------------- //

LTBOOL KeyFramer::CalcCurPos(LTBOOL & bAtKey)
{
	if (!m_pPosition1 || !m_pPosition2) return LTFALSE;

	bAtKey = LTFALSE;

	LTVector vPos1 = m_pPosition1->keyData.m_vPos;
	LTVector vPos2 = m_pPosition2->keyData.m_vPos;

	const float c_fEpsilon = 0.001f;
	LTBOOL bUpdateObjects = LTFALSE;
	float fPos1Time, fPos2Time;

	// See if we should update the objects...

	if (m_bUseVelocity)
	{
		if (m_fVelocity > 0.0 && m_fTotalDistance > 0.0f)
		{
			bUpdateObjects = LTTRUE;
		}
	}
	else // Using time...
	{
		fPos1Time = m_pPosition1->keyData.m_fRealTime;
		fPos2Time = m_pPosition2->keyData.m_fRealTime;

		if (m_fCurTime >= fPos1Time && m_fCurTime <= fPos2Time)
		{
			bUpdateObjects = LTTRUE;
		}
	}

	if (!bUpdateObjects) return LTFALSE;


	// See if we should velocity to calculate m_vCurPos...

	if (m_bUseVelocity)
	{
		float fDistToEnd = m_pPosition2->keyData.m_fDistToLastKey;

		// Calculate distance between pos1 and pos2

		float fDistBetweenKeys;
		if( m_pPosition1->keyData.m_bNextValid && m_pPosition2->keyData.m_bPrevValid )
		{
			fDistBetweenKeys = ( float )fabs( m_pPosition1->keyData.m_fDistToLastKey - m_pPosition2->keyData.m_fDistToLastKey );
		}
		else
		{
			fDistBetweenKeys = vPos2.Dist( vPos1 );
		}

		// Add distance between current pos and pos2 to get full distance
		// to last key...

		fDistToEnd += ( 1 - m_fKeyPercent ) * fDistBetweenKeys;


		// Calculate distance to move...

		float fPathPercent = 1 - (fDistToEnd / m_fTotalDistance);
		float fFrameSpeed = GetKFWaveValue(m_fVelocity, fPathPercent, m_eWaveform);
        float fMoveDist = fFrameSpeed * g_pLTServer->GetFrameTime();
		float fMovePercent = fMoveDist / fDistBetweenKeys;


		// See if we'll move past (or to) the destination pos...

		if (m_eDirection == KFD_FORWARD)
		{
			m_fKeyPercent += fMovePercent;
			if (m_fKeyPercent >= 1.0f)
			{
				bAtKey = LTTRUE;
			}
		}
		else  // KFD_BACKWARD
		{
			m_fKeyPercent -= fMovePercent;
			if (m_fKeyPercent <= 0.0f)
			{
				bAtKey = LTTRUE;
			}
		}


		// Calculate the position to move to...

		if (bAtKey)
		{
			m_vCurPos = (m_eDirection == KFD_FORWARD) ? vPos2 : vPos1;

			// Reset key percent based on the current direction...

			m_fKeyPercent = (m_eDirection == KFD_FORWARD) ? 0.0f : 1.0f;
		}
		else
		{
			if (m_pPosition1->keyData.m_bNextValid && m_pPosition2->keyData.m_bPrevValid )
			{
				Bezier_Evaluate(m_vCurPos, CURVE_PTS(m_pPosition1->keyData, m_pPosition2->keyData), m_fKeyPercent);
			}
			else
			{
				VEC_LERP(m_vCurPos, vPos1, vPos2, m_fKeyPercent);
			}
		}
	}
	else  // Using time to calculate m_vCurPos...
	{
		float fTimeRange = (fPos2Time - fPos1Time);
		if (-c_fEpsilon <= fTimeRange && fTimeRange < c_fEpsilon)
		{
			fTimeRange = 0.0f;
		}

		m_fKeyPercent = fTimeRange != 0.0f ? (m_fCurTime - fPos1Time) / fTimeRange : 0.0f;

		// Get the new position from position 1's position

		if (m_pPosition1->keyData.m_bNextValid && m_pPosition2->keyData.m_bPrevValid)
		{
			Bezier_Evaluate(m_vCurPos, CURVE_PTS(m_pPosition1->keyData, m_pPosition2->keyData), m_fKeyPercent);
		}
		else
		{
			VEC_LERP(m_vCurPos, vPos1, vPos2, m_fKeyPercent);
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoToKey
//
//	PURPOSE:	Set the current key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoToKey(char* pKeyName)
{
	if (!pKeyName) return;

	// Find the specified key...

	KEYNODE* pCurKey = FindKey(pKeyName);

	// Couldn't find the key...

	if (!pCurKey)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::GoToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}


	// Make sure the keyframer isn't paused...

	Resume();


	// Set the current key and current time...

	m_pCurKey  = pCurKey;
	m_fCurTime = pCurKey->keyData.m_fRealTime;
	m_vCurPos  = pCurKey->keyData.m_vPos;


	// Set up m_pPosition1 and m_pPosition2...

	ProcessCurrentKey();


	// Make sure that m_pPosition1 and m_pPosition2 are valid...

	if (!m_pPosition1)
	{
		m_pPosition1 = m_pPosition2;
	}
	else if (!m_pPosition2)
	{
		m_pPosition2 = m_pPosition1;
	}

	if (!m_pPosition1)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::GoToKey() - m_pPosition1 and m_pPosition2 are both LTNULL!!!!");
	}


	// Move/Rotate the objects as necessary...

	UpdateObjects(LTFALSE, LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::MoveToKey
//
//	PURPOSE:	Set the destination key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::MoveToKey(ConParse parse)
{
	if (parse.m_nArgs < 1 || !parse.m_Args[1]) return;

	char* pKeyName = parse.m_Args[1];

	// See if we have a destination command...

	if (parse.m_nArgs > 2)
	{
		if (parse.m_Args[2])
		{
			if (m_hstrDestCmd)
			{
                g_pLTServer->FreeString(m_hstrDestCmd);
			}

            m_hstrDestCmd = g_pLTServer->CreateString(parse.m_Args[2]);
		}
	}


	// This isn't quite right, doesn't work with 4 floor elevator with
	// the last key being the bottom floor...



	// Find the destination key...

	KEYNODE* pCur = (m_eDirection == KFD_FORWARD) ? m_pPosition1 : m_pPosition2;

	LTBOOL bIsAtOrBefore = LTFALSE;
	m_pDestinationKey = FindKey(pKeyName, pCur, &bIsAtOrBefore);


	// Couldn't find the key...

	if (!m_pDestinationKey)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::MoveToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}


	// Make sure the keyframer isn't paused...

	Resume();


	// Check to see if the destination key is the current key...

	if (m_pDestinationKey == pCur)
	{
		// See if we are actually at the key position...

		if (m_vCurPos.Equals(pCur->keyData.m_vPos))
		{
			// Okay, we just need to process the key...

			ProcessCurrentKey();
			return;
		}
		else
		{
			// If we aren't looping, we are someplace between m_pPosition1
			// and m_pPosition2 moving away from m_pCurKey.  So, we just
			// need to switch directions....

			//if (!m_bLooping)
			{
				ToggleDir();
				return;
			}
		}
	}
	else //if (!m_bLooping)
	{
		// Check to see if we need to change directions to get to the
		// destination key...

		if (bIsAtOrBefore)
		{
			if (m_eDirection == KFD_BACKWARD)
			{
				ToggleDir();
			}
		}
		else
		{
			if (m_eDirection == KFD_FORWARD)
			{
				ToggleDir();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::FindKey
//
//	PURPOSE:	Find the specified key
//
// ----------------------------------------------------------------------- //

KEYNODE* KeyFramer::FindKey(char* pKeyName, KEYNODE* pTest, LTBOOL* pbAtOrBefore)
{
	if (!pKeyName) return LTNULL;

	// Find the specified key...

	KEYNODE* pCurKey = m_pKeys;
	for (int i=0; i < m_nNumKeys && pCurKey; i++)
	{
		// Is the key we're searching for at or after the test key?

		if (pTest && pbAtOrBefore)
		{
			if (pCurKey == pTest)
			{
				*pbAtOrBefore = LTTRUE;
			}
		}

		if (pCurKey->keyData.m_hstrName)
		{
            char* pCurKeyName = g_pLTServer->GetStringData(pCurKey->keyData.m_hstrName);
			if (pCurKeyName && _stricmp(pKeyName, pCurKeyName) == 0)
			{
				break;
			}
		}
		pCurKey = pCurKey->pNext;
	}

	return pCurKey;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::On()
//
//	PURPOSE:	Turn keyframer on
//
// --------------------------------------------------------------------------- //

void KeyFramer::On()
{
	m_bPaused = LTFALSE;

	if (m_bFirstUpdate)
	{
		// Can't activate before Update is called...so just start active...

		m_bStartActive = LTTRUE;
	}
	else
	{
		GoActive();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Off()
//
//	PURPOSE:	Turn keyframer off
//
// --------------------------------------------------------------------------- //

void KeyFramer::Off()
{
	GoInActive();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Pause()
//
//	PURPOSE:	Pause keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Pause()
{
	if (m_bActive && !m_bPaused)
	{
		m_bPaused = LTTRUE;
		GoInActive();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Resume()
//
//	PURPOSE:	Resume keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Resume()
{
	if (m_bPaused)
	{
		m_bPaused = LTFALSE;
		GoActive(LTFALSE);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Forward()
//
//	PURPOSE:	Change keyframer direction to forward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Forward()
{
	// Make sure the keypercent starts with the correct value...

	if (m_eDirection == KFD_BACKWARD && m_fKeyPercent > 0.99f)
	{
		m_fKeyPercent = 0.0f;
	}

	m_eDirection = KFD_FORWARD;
	m_pCurKey = m_pPosition2;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Backward()
//
//	PURPOSE:	Change keyframer direction to backward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Backward()
{
	// Make sure the keypercent starts with the correct value...

	if (m_eDirection == KFD_FORWARD && m_fKeyPercent < 0.01f)
	{
		m_fKeyPercent = 1.0f;
	}

	m_eDirection = KFD_BACKWARD;
	m_pCurKey = m_pPosition1;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ToggleDir()
//
//	PURPOSE:	Toggle keyframer direction
//
// --------------------------------------------------------------------------- //

void KeyFramer::ToggleDir()
{
	if (m_eDirection == KFD_FORWARD)
	{
		Backward();
	}
	else
	{
		Forward();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	// Save m_pCommands
    HSTRING hStr = g_pLTServer->CreateString(m_pCommands);
    g_pLTServer->WriteToMessageHString(hWrite, hStr);

	// Make sure we created the keylist (if Save is called before the first
	// update, this may not have been created)...

	CreateKeyList();

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hActiveSndObj);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hTargetObject);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vTargetOffset);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vCurPos);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fEarliestGoActiveTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fTotalPathTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fTotalDistance);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fVelocity);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fCurTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fEndTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fKeyPercent);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrObjectName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrTargetName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrBaseKeyName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrActiveSnd);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDestCmd);
    g_pLTServer->WriteToMessageByte(hWrite, m_bUseVelocity);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartActive);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartPaused);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLooping);
    g_pLTServer->WriteToMessageByte(hWrite, m_bIgnoreOffsets);
    g_pLTServer->WriteToMessageByte(hWrite, m_bActive);
    g_pLTServer->WriteToMessageByte(hWrite, m_bPaused);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFinished);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAlignToPath);
    g_pLTServer->WriteToMessageByte(hWrite, m_bPushObjects);
    g_pLTServer->WriteToMessageByte(hWrite, m_nNumKeys);
    g_pLTServer->WriteToMessageByte(hWrite, m_eDirection);
    g_pLTServer->WriteToMessageByte(hWrite, m_eWaveform);


	// Determine the position in the list of m_pCurKey, m_pPosition1, and
	// m_pPosition2.  Also, save out m_pKeys...

	int nCurKeyIndex    = -1;
	int nPosition1Index = -1;
	int nPosition2Index = -1;
	int nDestKeyIndex   = -1;

    int i;
	KEYNODE* pCurKey = m_pKeys;
    for (i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (m_pCurKey == pCurKey)
		{
			nCurKeyIndex = i;
		}
		if (m_pPosition1 == pCurKey)
		{
			nPosition1Index = i;
		}
		if (m_pPosition2 == pCurKey)
		{
			nPosition2Index = i;
		}
		if (m_pDestinationKey == pCurKey)
		{
			nDestKeyIndex = i;
		}

		pCurKey->keyData.Save(hWrite, dwSaveFlags);
		pCurKey = pCurKey->pNext;
	}

	// Save out the positions of our pointer data members...

    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nCurKeyIndex);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nPosition1Index);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nPosition2Index);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT) nDestKeyIndex);


	// Save the number of objects to be key-framed...

    uint8 nNumInList = m_pObjectList ? m_pObjectList->m_nInList : 0;
    g_pLTServer->WriteToMessageByte(hWrite, nNumInList);

	// Save the offsets and rotations for each object...

	for (i=0; i < nNumInList; i++)
	{
        g_pLTServer->WriteToMessageVector(hWrite, &(m_pOffsets[i]));
        g_pLTServer->WriteToMessageRotation(hWrite, &(m_pRotations[i]));
	}

	// Save the objects we're supposed to key-frame...

	if (m_pObjectList && nNumInList)
	{
		ObjectLink* pLink = m_pObjectList->m_pFirstLink;
		while (pLink)
		{
            g_pLTServer->WriteToLoadSaveMessageObject(hWrite, pLink->m_hObject);
			pLink = pLink->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	// Load m_pCommands
    HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);
	if(hStr)
	{
        char *pstr = g_pLTServer->GetStringData(hStr);
		if(pstr && pstr[0])
		{
			if(m_pCommands)
			{
				debug_deletea(m_pCommands);
			}

			m_pCommands = debug_newa(char, 1024);
			strcpy(m_pCommands,pstr);
		}

        g_pLTServer->FreeString(hStr);
	}

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hActiveSndObj);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hTargetObject);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vTargetOffset);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vCurPos);

    m_fEarliestGoActiveTime = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fTotalPathTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fTotalDistance    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fVelocity         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fCurTime          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fEndTime          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSoundRadius      = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fKeyPercent       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_hstrObjectName    = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrTargetName    = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrBaseKeyName   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrActiveSnd     = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDestCmd       = g_pLTServer->ReadFromMessageHString(hRead);
    m_bUseVelocity      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bStartActive      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bStartPaused      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bLooping          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bIgnoreOffsets    = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bActive           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bPaused           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bFinished         = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bAlignToPath      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bPushObjects      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_nNumKeys          = g_pLTServer->ReadFromMessageByte(hRead);
    m_eDirection        = (KFDirection) g_pLTServer->ReadFromMessageByte(hRead);
    m_eWaveform         = (KFWaveType) g_pLTServer->ReadFromMessageByte(hRead);

	// Build the m_pKeys data member...

    int i;
	KEYNODE* pNode = LTNULL;
    for (i=0; i < m_nNumKeys; i++)
	{
		if (!m_pKeys)
		{
			m_pKeys = debug_new(KEYNODE);
			pNode = m_pKeys;
		}
		else if (pNode)
		{
			pNode->pNext = debug_new(KEYNODE);
			pNode->pNext->pPrev = pNode;
			pNode = pNode->pNext;
		}

		if (pNode)
		{
			pNode->keyData.Load(hRead, dwLoadFlags);
		}
	}

	// Determine the positions of our pointer data members...

    int nCurKeyIndex    = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    int nPosition1Index = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    int nPosition2Index = (int) g_pLTServer->ReadFromMessageFloat(hRead);
    int nDestKeyIndex   = (int) g_pLTServer->ReadFromMessageFloat(hRead);

	KEYNODE* pCurKey = m_pKeys;
	for (i=0; i < m_nNumKeys && pCurKey; i++)
	{
		if (nCurKeyIndex == i)
		{
			m_pCurKey = pCurKey;
		}
		if (nPosition1Index == i)
		{
			m_pPosition1 = pCurKey;
		}
		if (nPosition2Index == i)
		{
			m_pPosition2 = pCurKey;
		}
		if (nDestKeyIndex == i)
		{
			m_pDestinationKey = pCurKey;
		}

		// This will end up being set to the last key...

		m_pLastKey = pCurKey;

		pCurKey = pCurKey->pNext;
	}

	// Load the number of objects we're supposed to key frame...

    uint8 nNumInList = g_pLTServer->ReadFromMessageByte(hRead);

	// Load the offsets and rotations for each object...

	for (i=0; i < nNumInList; i++)
	{
        g_pLTServer->ReadFromMessageVector(hRead, &(m_pOffsets[i]));
        g_pLTServer->ReadFromMessageRotation(hRead, &(m_pRotations[i]));
	}

	// Load the objects we're supposed to key-frame...

	if (nNumInList > 0)
	{
		if (m_pObjectList)
		{
            g_pLTServer->RelinquishList(m_pObjectList);
		}

        m_pObjectList = g_pLTServer->CreateObjectList();

		if (m_pObjectList)
		{
			HOBJECT* hObjectArray = debug_newa(HOBJECT, nNumInList);

			HOBJECT hObj = LTNULL;
			for (int i=0; i < nNumInList; i++)
			{
                g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &hObj);
				if (hObj)
				{
					hObjectArray[i] = hObj;
				}
				else
				{
					hObjectArray[i] = LTNULL;
				}
			}

			// Kind of assy, but need to add the objects in reverse order
			// since AddObjectToList always adds at the front of the list...

			for (i=nNumInList-1; i >= 0; i--)
			{
				if (hObjectArray[i])
				{
                    g_pLTServer->AddObjectToList(m_pObjectList, hObjectArray[i]);
				}
			}

			if (hObjectArray)
			{
				debug_deletea(hObjectArray);
			}
		}
	}


	// If we were active, restart active sound...

	if (m_bActive)
	{
		m_bActive = LTFALSE; // need to clear this first...
		GoActive(LTFALSE);   // don't reset any values...
	}

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		m_bPausedOnLoad = LTTRUE;
	}
}