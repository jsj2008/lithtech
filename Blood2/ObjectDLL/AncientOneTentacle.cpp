// ----------------------------------------------------------------------- //
//
// MODULE  : AncientOneTentacle.cpp
//
// PURPOSE : Soul Drudge - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "AncientOneTentacle.h"
#include "cpp_server_de.h"

BEGIN_CLASS(AncientOneTentacle)
	ADD_BASEAI_AGGREGATE()
	ADD_STRINGPROP(AIState, "IDLE")     \
END_CLASS_DEFAULT(AncientOneTentacle, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		AncientOneTentacle::m_bLoadAnims = DTRUE;
CAnim_Sound	AncientOneTentacle::m_Anim_Sound;

#define TENTACLE_SCALE 2.5f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AncientOneTentacle::AncientOneTentacle() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 1000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 2000.0f;

	m_fWalkSpeed	= 0.0f;
	m_fRunSpeed		= 0.0f;
	m_fRollSpeed	= 0.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"ANCIENTONE_TENTACLE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DFALSE;
	m_bMoveToGround = DTRUE;

	VEC_SET(m_vScale,TENTACLE_SCALE,TENTACLE_SCALE,TENTACLE_SCALE);

	// [blg]
	m_fAIHitPoints   = 200;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	m_fLastIdle = 0;
	m_fScaleBy = 1.0f;
	m_bHiding = DFALSE;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");

	m_damage.SetApplyDamagePhysics(DFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD AncientOneTentacle::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void AncientOneTentacle::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	int iRand = pServerDE->IntRandom(1,4);

	sprintf(pStruct->m_Filename, "models\\enemies\\ancientone_t%d.abc", iRand);
	sprintf(pStruct->m_SkinName, "skins\\enemies\\ancientone.dtx");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL AncientOneTentacle::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	m_hObject = m_pServerDE->ObjectToHandle(this);
	if (!m_hObject) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\AncientOneTentacle");	
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOneTentacle::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOneTentacle::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		CWeapon *pW = m_InventoryMgr.GetCurrentWeapon();		
		if (pW)
		{
			m_fAttackLoadTime = pW->GetReloadTime();
		}

        bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_STAND;
    }
    else
    {   
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOneTentacle::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOneTentacle::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);				break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);				break;
			default:						SetNewState(STATE_SearchVisualTarget);	break;
		}
	}
	else
	{
		DFLOAT fTemp = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);

		if (fTemp <= 500.0f)
		{
			SetNewState(STATE_AttackClose);
		}
		else
		{
			SetNewState(STATE_Idle);
		}
	}

	return;
}

DDWORD AncientOneTentacle::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	// We can't be damaged by the Ancient One
	if (messageID == MID_DAMAGE)
	{
		HCLASS hAncientOne = m_pServerDE->GetClass("AncientOne");
		HCLASS hClass = m_pServerDE->GetObjectClass(hSender);

		if (!m_pServerDE->IsKindOf(hClass,hAncientOne))
		{
			return AI_Mgr::ObjectMessageFn(hSender, messageID, hRead);			
		}
		else
		{
			return 0;
		}
	} else
	{
		return AI_Mgr::ObjectMessageFn(hSender, messageID, hRead);
	}
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOneTentacle::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOneTentacle::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_ScaleUp();		break;
		case 2:		MC_FaceTarget();	break;
		case 3:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] <= 500.0f || m_nCurMetacmd == MC_FIRE_STAND)
						MC_Fire_Stand();
					else
						SetNewState(STATE_Idle);
					
					break;
		case 4:		ComputeState();									break;
		default:	ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: AncientOneTentacle::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void AncientOneTentacle::AI_STATE_Idle()
{
	int nStimType = ComputeStimuli();

	if(nStimType)
	{
		ComputeState(nStimType);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		ComputeState();		break;
	}

	return;
}

void AncientOneTentacle::AI_STATE_Special1()
{
	switch(Metacmd)
	{
		case 1:
			MC_ScaleUp();
			break;

		case 2:
			m_bHiding = DFALSE;
			SetNewState(STATE_Idle);
			break;
	}
}

void AncientOneTentacle::MC_FadeIn()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FADEIN)
    {        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FADEIN;

		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags = dwFlags | FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha += 1.0/64;
		if(fAlpha >= 1.0f)
			fAlpha = 1.0f;

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

		if(fAlpha >= 1.0f)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;	
}


void AncientOneTentacle::AI_STATE_Special2()
{
	switch(Metacmd)
	{
		case 1:
			MC_ScaleDown();
			break;

		case 2:
			SetNewState(STATE_Special3);
			break;
	}
}

void AncientOneTentacle::AI_STATE_Special4()
{
	switch(Metacmd)
	{
		case 1:
			MC_ScaleDown();
			break;

		case 2:
			m_bHiding = DTRUE;
			SetNewState(STATE_Idle);
			break;
	}
}


void AncientOneTentacle::MC_FadeOut()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FADEOUT)
    {        
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FADEOUT;
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 1.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha -= 1.0/64;
		if(fAlpha <= 0.0f)
			fAlpha = 0.0f;

		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, fAlpha);

		if(fAlpha <= 0.0f)
		{
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;	
}

void AncientOneTentacle::AI_STATE_Special3()
{
	switch(Metacmd)
	{
		case 1:
			if ((m_pServerDE->GetTime() - m_fTimeOfDeath) >= DEATH_DURATION)
				Metacmd++;
			break;

		case 2:
			SetNewState(STATE_Special1);
			break;
	}
}

void AncientOneTentacle::MC_ScaleDown()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_SCALEDOWN)
    {        
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_SOLID;
		dwFlags = dwFlags | FLAG_GRAVITY;
		m_pServerDE->SetObjectFlags(m_hObject,dwFlags);

		m_fScaleBy = TENTACLE_SCALE;

		SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[0]);
		m_pServerDE->SetModelLooping(m_hObject, DTRUE);
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_SCALEDOWN;
    }
    else
	{
		DVector vScale;
		DVector vDims;
		DVector vScaleBy;

		m_fScaleBy -= TENTACLE_SCALE/20.0f;

		if (m_fScaleBy <= 0.01f) m_fScaleBy = 0.01f;

		m_pServerDE->GetObjectScale(m_hObject,&vScale);
		m_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, 0);

		VEC_SET(vScaleBy,m_fScaleBy,m_fScaleBy,m_fScaleBy);
		VEC_COPY(vScale,vScaleBy);
		VEC_MUL(vDims,vDims,vScale);

		m_pServerDE->ScaleObject(m_hObject,&vScale);
		m_pServerDE->SetObjectDims(m_hObject,&vDims);

		VEC_COPY(m_vScale,vScale);

		MoveObjectToGround(m_hObject);

		if(m_fScaleBy <= 0.01f)
		{
			Metacmd++;
			return;
		}
    }      
	
    return;	
}

void AncientOneTentacle::MC_ScaleUp()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_SCALEUP)
    {        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_SCALEUP;
    }
    else
	{
		DVector vScale;
		DVector vDims;
		DVector vScaleBy;

		m_fScaleBy += TENTACLE_SCALE/20.0f;

		if (m_fScaleBy >= TENTACLE_SCALE) m_fScaleBy = TENTACLE_SCALE;

		m_pServerDE->GetObjectScale(m_hObject,&vScale);
		m_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, 0);

		VEC_SET(vScaleBy,m_fScaleBy,m_fScaleBy,m_fScaleBy);
		VEC_COPY(vScale,vScaleBy);
		VEC_MUL(vDims,vDims,vScale);

		m_pServerDE->ScaleObject(m_hObject,&vScale);
		m_pServerDE->SetObjectDims(m_hObject,&vDims);

		VEC_COPY(m_vScale,vScale);

		DVector vPos;
		m_pServerDE->GetObjectPos(m_hObject, &vPos);
		vPos.y += 200;
		m_pServerDE->SetObjectPos(m_hObject, &vPos);

		MoveObjectToGround(m_hObject);

		if(m_fScaleBy >= TENTACLE_SCALE)
		{
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags = dwFlags | FLAG_SOLID | FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject,dwFlags);
			SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[0]);
			m_pServerDE->SetModelLooping(m_hObject, DTRUE);

			m_bAnimating = DTRUE;
			Metacmd++;
			return;
		}
    }      
	
    return;	
}


void AncientOneTentacle::MC_Dead()
{
	m_fTimeOfDeath = m_pServerDE->GetTime();
	m_damage.SetHitPoints(200.0f);
	SetNewState(STATE_Special2);
}

void AncientOneTentacle::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fTimeOfDeath);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastIdle);
	pServerDE->WriteToMessageFloat(hWrite, m_fScaleBy);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bHiding);
}

void AncientOneTentacle::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_fTimeOfDeath = pServerDE->ReadFromMessageFloat(hRead);
	m_fLastIdle = pServerDE->ReadFromMessageFloat(hRead);
	m_fScaleBy = pServerDE->ReadFromMessageFloat(hRead);
	m_bHiding = (DBOOL)(pServerDE->ReadFromMessageByte(hRead));
}


void AncientOneTentacle::MC_Idle()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_IDLE)
    {
		DBOOL bRet = DFALSE;

		bRet = SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[0]);
		m_pServerDE->SetModelLooping(m_hObject, DTRUE);

        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_IDLE;
//		if (!m_bHiding) m_fLastIdle = m_pServerDE->GetTime();
	}
	else
	{
		/*
		if (((m_fLastIdle + 30.0f) >= m_pServerDE->GetTime()) && !m_bHiding)
		{
			SetNewState(STATE_Special4);
		}
		*/

		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
            return;
        }
	}

	return;
}
