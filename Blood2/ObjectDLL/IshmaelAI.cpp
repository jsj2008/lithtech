// ----------------------------------------------------------------------- //
//
// MODULE  : IshmaelAI.cpp
//
// PURPOSE : IshmaelAI - Definition
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include <mbstring.h>
#include "IshmaelAI.h"
#include "cpp_server_de.h"
#include "sfxmsgids.h"
#include "gameprojectiles.h"
#include "SoundTypes.h"

BEGIN_CLASS(IshmaelAI)
	ADD_STRINGPROP(AIState, "PASSIVE")
	ADD_STRINGPROP(AIWeapon1, "ORB")   \
	ADD_STRINGPROP(AIWeapon2, "HEAL")   \
	ADD_STRINGPROP(AIWeapon3, "SHOCKWAVE")   \
	ADD_VECTORPROP_VAL_FLAG(Dims, 12.0f, 39.0f, 12.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(IshmaelAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		IshmaelAI::m_bLoadAnims = DTRUE;
CAnim_Sound	IshmaelAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IshmaelAI::IshmaelAI()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

IshmaelAI::IshmaelAI() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 1000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 800.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 105.0f;
	m_fJumpSpeed	= 400.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSRECOIL;

	m_bCabal = DTRUE;

	m_fAIHitPoints   = 500;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 500;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"PASSIVE");
	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"ORB");
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"HEAL");
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"SHOCKWAVE" );

	m_fAIBullets  = 500;
	m_fAIBMG	  = 100;
	m_fAIShells   = 150;
	m_fAIGrenades = 50;
	m_fAIRockets  = 100;
	m_fAIFlares   = 100;
	m_fAICells    = 500;
	m_fAICharges  = 500;
	m_fAIFuel	  = 500;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IshmaelAI::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD IshmaelAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we set the skin/filename...

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\Characters\\Ishmael.abc");
				_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\Characters\\Ishmael.dtx");
			}

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			FirstUpdate();
		}
		break;

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FirstUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL IshmaelAI::FirstUpdate()
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE || !m_hObject) return DFALSE;

	//Load up animation indexes if first model instance

    if (m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
		m_Anim_Sound.SetSoundRoot("sounds\\chosen\\Ishmael");

		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::MC_Heal
// DESCRIPTION	: Run the heal  animation and effect
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::MC_Heal()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_HEAL)
    {
		m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_HEAL);
        DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[6]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_HEAL;
    }
    else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			DFLOAT fHP = m_damage.GetHitPoints();
			m_damage.SetHitPoints(fHP * 1.1f);

			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::MC_FadeOut
// DESCRIPTION	: Run an fade out animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::MC_FadeOut()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FADEOUT)
    {   
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_SOLID;
		m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
		
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FADEOUT;
    }
    else
	{
		DVector vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);

		fAlpha -= 1.0/16;
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

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		if(m_nState == STATE_AttackClose)
		{
			m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_SHOCKWAVE);
		}
		else
		{
			m_InventoryMgr.AddAmmo(AMMO_FOCUS, 20.0f);
			m_InventoryMgr.ChangeWeapon(WEAP_ORB);
		}

        DBOOL bRet = SetAnimation( m_pAnim_Sound->m_nAnim_FIRE_STAND[6]);

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
// ROUTINE		: IshmaelAI::MC_Dead
// DESCRIPTION	: Run death animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::MC_Dead()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DEAD)
    {		
		DVector vVel, vColor;
		DFLOAT fAlpha = 0.0f;

		m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
		m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, 1.0f);

        SetAnimation(m_pAnim_Sound->m_nAnim_DEATH[0]);

		VEC_MULSCALAR(vVel, m_MoveObj.GetUpVector(), 30.0f);

		//turn off gravity
		DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
		dwFlags &= ~FLAG_GRAVITY;
		m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

		Move(vVel, MATH_EPSILON);

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		m_bAnimating = DTRUE;
		m_nCurMetacmd = MC_DEAD;
	}
	else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_bAnimating = DFALSE;
			Metacmd++;

			char* szSound = "sounds\\enemies\\zealot\\magic.wav";
			PlaySoundFromObject(m_hObject, szSound, 2000.0f, SOUNDPRIORITY_MISC_MEDIUM, 
								DFALSE, DTRUE, DTRUE);

			DDWORD nType = EXP_DIVINE_SHOCKBALL;

			DVector vPos, vNormal;

			VEC_COPY(vPos, m_MoveObj.GetPos());
			VEC_COPY(vNormal, m_MoveObj.GetUpVector());

			//********************************************************************
			// Create the explosion type
			HMESSAGEWRITE hMessage = m_pServerDE->StartInstantSpecialEffectMessage(&vPos);
			m_pServerDE->WriteToMessageByte(hMessage, SFX_EXPLOSIONFX_ID);

			m_pServerDE->WriteToMessageVector(hMessage, &vPos);
			m_pServerDE->WriteToMessageVector(hMessage, &vNormal);
			m_pServerDE->WriteToMessageDWord(hMessage, nType);

			m_pServerDE->EndMessage(hMessage);

			//********************************************************************
			// Move all the objects within the radius back and randomly rotate them a little
			ObjectList *ol = m_pServerDE->FindObjectsTouchingSphere(&vPos, LIFELEECH_PUSH_RADIUS);

			if(ol)
			{
				DVector		vObjPos, vObjVel, vUp;
				DRotation	rObjRot;
				ObjectLink* pLink = ol->m_pFirstLink;
				DFLOAT		mag, randRot = MATH_PI * LIFELEECH_ROTATE_AMOUNT;
				HOBJECT		hObj;
				short		objType;

				VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

				while(pLink)
				{
					hObj = pLink->m_hObject;
					pLink = pLink->m_pNext;

					objType = m_pServerDE->GetObjectType(hObj);

					if(hObj == m_hObject) continue;
					if((objType != OT_MODEL) && (objType != OT_WORLDMODEL)) continue;

					// Get the information about the obj
					m_pServerDE->GetObjectPos(hObj, &vObjPos);
					m_pServerDE->GetObjectRotation(hObj, &rObjRot);
					m_pServerDE->GetVelocity(hObj, &vObjVel);

//					m_pServerDE->RotateAroundAxis(&rObjRot, &vUp, m_pServerDE->Random(-randRot, randRot));
//					m_pServerDE->SetObjectRotation(hObj, &rObjRot);

					VEC_SUB(vObjPos, vObjPos, vPos);
					mag = LIFELEECH_PUSH_RADIUS - VEC_MAG(vObjPos);
					if(mag < 0.0f)	mag = 0.0f;
					VEC_NORM(vObjPos);
					VEC_MULSCALAR(vObjPos, vObjPos, mag * 5.0f);
					m_pServerDE->GetGlobalForce(&vUp);
					VEC_MULSCALAR(vUp, vUp, -0.15f);
					VEC_ADD(vObjVel, vObjVel, vUp);
					VEC_ADD(vObjVel, vObjVel, vObjPos);
//					m_pServerDE->SetVelocity(hObj, &vObjVel);

					m_pServerDE->GetObjectPos(hObj, &vObjPos);
					DFLOAT fDamage = (DFLOAT)m_pServerDE->IntRandom(25,50);
					DamageObject(m_hObject, this, hObj, fDamage, vObjVel, vObjPos, DAMAGE_TYPE_EXPLODE);
				}

				m_pServerDE->RelinquishList(ol);
			}

			RemoveMe();
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	if(!nStim)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);			break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);			break;
			case STATE_SearchSmellTarget:	SetNewState(STATE_Idle);			break;
			case STATE_Escape_RunAway:		SetNewState(STATE_Idle);			break;
			case STATE_Escape_Hide:			SetNewState(STATE_Idle);			break;
			default:						SetNewState(STATE_SearchVisualTarget);	break;
		}
	}
	else
	{
		if((m_fStimuli[HEALTH] < 0.1f) && (m_nState != STATE_Escape_Hide))
		{
			SetNewState(STATE_Escape_Hide);
			return;
		}

		if(m_fStimuli[SENSE] < 0.65f || fHeight > m_vDims.y)
			SetNewState(STATE_AttackFar);
		else
			SetNewState(STATE_AttackClose);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::AI_STATE_AttackClose()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	m_fStimuli[SENSE] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;	
		case 2:		if(m_fStimuli[SENSE] <= (m_fSensingDist * 0.20) && m_nCurMetacmd != MC_FIRE_STAND)
					{
						SetNewState(STATE_Teleport);
					}
					else
						Metacmd++;

					break;
		case 3:		MC_FaceTarget();
					Metacmd--;

					if((m_fStimuli[SENSE] >= (m_fSensingDist * 0.35) && m_nCurMetacmd != MC_FIRE_STAND)
						|| fHeight > m_vDims.y)
					{
						ComputeState();
					}
					else
					{
						MC_Fire_Stand();
					}

					break;
		case 4:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::AI_STATE_AttackFar()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	m_fStimuli[SENSE] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;	
		case 2:		if(m_fStimuli[SENSE] > (m_fSensingDist * 0.75) && fHeight <= m_vDims.y)
					{
						MC_Run();
					}
					else
						Metacmd++;

					break;
		case 3:		MC_FaceTarget();
					Metacmd--;

					if(m_fStimuli[SENSE] <= (m_fSensingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						MC_Fire_Stand();
					}
					else
						Metacmd++;

					break;
		case 4:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::AI_STATE_Escape_Hide
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::AI_STATE_Escape_Hide()
{
	switch(Metacmd)
	{
		case 1:		MC_Heal();			break;	
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: IshmaelAI::AI_STATE_Teleport
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void IshmaelAI::AI_STATE_Teleport()
{
	switch(Metacmd)
	{
		case 1:		MC_FadeOut();		break;
		case 2:
		{
					char szSound[256];
					sprintf(szSound, "%s%d.wav", SOUND_LAUGH, m_pServerDE->IntRandom(1,3));
					m_pAnim_Sound->PlaySound(m_hObject, szSound, 1000.0f, 100);					

					Metacmd++;			break;
		}
		case 3:
		{
					IntersectQuery IQuery;
					IntersectInfo IInfo;

					IQuery.m_Flags	  = INTERSECT_OBJECTS;
					IQuery.m_FilterFn = DNULL;

					DVector vDir;
					VEC_SET(vDir, m_pServerDE->Random(-1.0f,1.0f), 0.0f, m_pServerDE->Random(-1.0f,1.0f));

					VEC_COPY(IQuery.m_From, m_MoveObj.GetPos());
					VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, vDir, m_fSensingDist);

					if(m_pServerDE->IntersectSegment(&IQuery, &IInfo))
					{
						VEC_ADDSCALED(IInfo.m_Point, IInfo.m_Point, vDir, -100.0f);
						m_pServerDE->MoveObject(m_hObject, &IInfo.m_Point);
					}
					else
					{
						VEC_ADDSCALED(IInfo.m_Point, IQuery.m_To, vDir, -100.0f);
						m_pServerDE->MoveObject(m_hObject, &IQuery.m_To);
					}

					Metacmd++;
					break;
		}
		case 4:		
		{	
					DVector vColor;
					DFLOAT	fAlpha = 1.0f;

					m_pServerDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
					m_pServerDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z, 1.0f);

					DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
					dwFlags |= FLAG_SOLID;
					m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

					Metacmd++;
					break;
		}
		case 5:		MC_FaceTarget();	break;
		case 6:		ComputeState();		break;
	}

	return;
}
