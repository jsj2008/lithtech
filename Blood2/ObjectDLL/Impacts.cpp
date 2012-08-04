//----------------------------------------------------------
//
// MODULE  : IMPACTS.CPP
//
// PURPOSE : defines classes for little impacts upon walls etc
//
// CREATED : 9/25/97
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include <string.h>
#include <mbstring.h>
#include "serverobj_de.h"
#include "Impacts.h"
#include "cpp_server_de.h"


BEGIN_CLASS(CImpact)
END_CLASS_DEFAULT_FLAGS(CImpact, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::CImpact
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CImpact::CImpact() : BaseClass(OT_SPRITE)
{
	m_bPlaySound	= DTRUE;
	m_bUpdateScale	= DTRUE;
	m_bFirstUpdate	= DTRUE;

	VEC_INIT(m_vScaleMin);
	VEC_INIT(m_vScaleMax);
	VEC_SET(m_vScale, 0.1f, 0.1f, 0.0f);

	m_fDuration		= 0.3f;
	m_fStartTime	= 0.0f;

	m_hSoundName	= DNULL;
	m_fSoundVol		= 200;
	m_bRotateable	= DFALSE;
	m_bFade			= DFALSE;

	red = green = blue = alpha = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::~CImpact
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CImpact::~CImpact()
{
	if (m_hSoundName)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (!pServerDE) return;
		
		pServerDE->FreeString(m_hSoundName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CImpact::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_SkinName[0] = '\0';
				pStruct->m_NextUpdate = 0.1f;
				VEC_COPY(pStruct->m_Scale, m_vScale)

				if (pStruct->m_Filename[0] == '\0')
				{
					_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"sprites\\pinpuncherimpact.spr");
				}
			}			
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::Setup
//
//	PURPOSE:	Set up a impact with the information needed
//
// ----------------------------------------------------------------------- //

void CImpact::Setup(DVector vNormal, DVector vScaleMin, DVector vScaleMax, char* pSound,
					DFLOAT fSoundVol, DFLOAT fDuration, DBOOL bRotateable, DBOOL bFade)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	VEC_COPY(m_vScaleMin, vScaleMin);
	VEC_COPY(m_vScaleMax, vScaleMax);

	if (m_vScaleMin.x == m_vScaleMax.x && m_vScaleMin.y == m_vScaleMax.y)
		m_bUpdateScale = DFALSE;

	if (pSound)
		m_hSoundName = pServerDE->CreateString(pSound);

	m_fSoundVol		= fSoundVol;
	m_fDuration		= fDuration;
	m_bRotateable	= bRotateable;
	m_bFade			= bFade;

	pServerDE->GetObjectColor(m_hObject, &red, &green, &blue, &alpha);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::UpdateScale
//
//	PURPOSE:	Update sprite scaling
//
// ----------------------------------------------------------------------- //

void CImpact::UpdateScale()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fStartTime;
	if (m_fDuration <= 0) return;

	DVector vRange;
	VEC_SUB(vRange, m_vScaleMax, m_vScaleMin);

	m_vScale.x = m_vScaleMin.x + (fDeltaTime * vRange.x / m_fDuration);
	m_vScale.y = m_vScaleMin.y + (fDeltaTime * vRange.y / m_fDuration);
	m_vScale.z = m_vScaleMin.z + (fDeltaTime * vRange.z / m_fDuration);

	if (m_vScale.x > m_vScaleMax.x) m_vScale.x = m_vScaleMax.x;
	if (m_vScale.y > m_vScaleMax.y) m_vScale.y = m_vScaleMax.y;
	if (m_vScale.z > m_vScaleMax.z) m_vScale.z = m_vScaleMax.z;

	pServerDE->ScaleObject(m_hObject, &m_vScale);

	if(m_bFade)
	{
		float	newAlpha = alpha - (fDeltaTime * alpha / m_fDuration);
		if(newAlpha < 0.0f)		newAlpha = 0.0f;
		pServerDE->SetObjectColor(m_hObject, red, green, blue, newAlpha);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL CImpact::InitialUpdate(DVector*)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.1);

	if (m_bRotateable)
		pServerDE->SetObjectFlags(m_hObject, FLAG_VISIBLE | FLAG_ROTATEABLESPRITE);
	else
		pServerDE->SetObjectFlags(m_hObject, FLAG_VISIBLE);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::FirstUpdate
//
//	PURPOSE:	Do First updating
//
// ----------------------------------------------------------------------- //

void CImpact::FirstUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_fStartTime = pServerDE->GetTime();

	if(m_hSoundName)
	{
		char* pSoundFile = pServerDE->GetStringData(m_hSoundName);

		DVector vPos;
		pServerDE->GetObjectPos(m_hObject, &vPos);
//		PlaySoundFromPos(&vPos, pSoundFile, m_fSoundVol, SOUNDTYPE_MISC, SOUNDPRIORITY_HIGH );
	}

	pServerDE->ScaleObject(m_hObject, &m_vScaleMin);

	if (m_bUpdateScale)
		VEC_COPY(m_vScale, m_vScaleMin);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CImpact::Update
//
//	PURPOSE:	Update the impact
//
// ----------------------------------------------------------------------- //

DBOOL CImpact::Update(DVector* pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if(!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.1);

	if(m_bFirstUpdate)
	{
		FirstUpdate();
		m_bFirstUpdate = DFALSE;
	}

	if(m_bUpdateScale) 
		UpdateScale();

	if(m_fDuration <= 0.0f)
		return DTRUE;
	else
		return (pServerDE->GetTime() < m_fStartTime + m_fDuration);
}





void impactric_PostPropRead(CImpactRicochet *pObj, ObjectCreateStruct *pStruct)
{
	pStruct->m_ObjectType = OT_SPRITE;
//	pStruct->m_SkinName[0] = '\0';
	pStruct->m_Flags = FLAG_VISIBLE;
	pStruct->m_NextUpdate = 0.1f;

	VEC_INIT(pObj->m_normal)
	
	pObj->m_bFirstUpdate = DTRUE;
	pObj->m_bPlaySound   = DTRUE;
	pObj->m_duration = (8.0f/15.0f);

//	_mbscpy((unsigned char*)pObj->m_strSound, (const unsigned char*)"Sounds\\ricocht2.wav");
}


DBOOL impactric_InitialUpdate(CImpactRicochet *pImpact, DVector *pMovement)
{
	HOBJECT hObj = ((BaseClass*)pImpact)->m_hObject;
	DVector scale;

	VEC_SET(scale, 0.250f, 0.250f, 0.250f)
	g_pServerDE->ScaleObject(hObj, &scale);

	pImpact->m_StartTime = g_pServerDE->GetTime();
	g_pServerDE->SetNextUpdate(hObj, .1f);
//	DVector vel;
//	VEC_SET(vel, 0.0f, 15.0f, 0.0f)
//	g_pServerDE->SetVelocity(hObj, &vel);

	if (pImpact->m_bPlaySound)
    {
//        PlaySoundInfo theSoundInfo;
//
//	    theSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;
//        _mbscpy((unsigned char*)theSoundInfo.m_szSoundName, (const unsigned char*)pImpact->m_strSound);
//	    theSoundInfo.m_hObject = hObj;
//    	theSoundInfo.m_fRadius = 400;
//	    theSoundInfo.m_nVolume = 100;
//    
//        g_pServerDE->PlaySound(&theSoundInfo);
//		g_pServerDE->PlaySound3D(hObj, pImpact->m_strSound, 400, 0, 0, 0);

    }
    
	return DTRUE;
}


DBOOL impactric_Update(CImpactRicochet *pImpact, DVector *pMovement)
{
	HOBJECT hObj = ((BaseClass*)pImpact)->m_hObject;

	g_pServerDE->SetNextUpdate(((BaseClass*)pImpact)->m_hObject, .1f);
	return ((g_pServerDE->GetTime() - pImpact->m_StartTime) < pImpact->m_duration);
}


DDWORD impactric_EngineMessageFn(CImpactRicochet *pObj, DDWORD messageID, void *pData, float lData)
{
	DDWORD bResult = 1;
	switch (messageID)
	{
		case MID_PRECREATE:
			impactric_PostPropRead(pObj, (ObjectCreateStruct*)pData);
			break;

		case MID_INITIALUPDATE:
			bResult = impactric_InitialUpdate(pObj, (DVector*)pData);
			break;

		case MID_UPDATE:
			bResult = impactric_Update(pObj, (DVector*)pData);
			break;
	}

	if (bResult)
		return bc_EngineMessageFn((BaseClass*)pObj, messageID, pData, lData);
	else
		g_pServerDE->RemoveObject(((BaseClass*)pObj)->m_hObject);
	return bResult;
}

DDWORD impactric_ObjectMessageFn(CImpactRicochet *pObj, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	return bc_ObjectMessageFn((BaseClass*)pObj, hSender, messageID, hRead);
}



BEGIN_CLASS(CImpactRicochet)
END_CLASS_DEFAULT_FLAGS(CImpactRicochet, BaseClass, impactric_EngineMessageFn, impactric_ObjectMessageFn, CF_HIDDEN)

