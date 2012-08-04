
// ----------------------------------------------------------------------- //
//
// MODULE  : BoneLeech.cpp
//
// PURPOSE : BoneLeech - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "BoneLeech.h"
#include "cpp_server_de.h"
#include "playerobj.h"

BEGIN_CLASS(BoneLeech)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 2.0f, 1.5f, 11.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(BoneLeech, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		BoneLeech::m_bLoadAnims = DTRUE;
CAnim_Sound	BoneLeech::m_Anim_Sound;

extern CPlayerObj* g_pPlayerObj;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BoneLeech::BoneLeech() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 300.0f;
	m_fSmellingDist	= 1000.0f;
	m_fSeeingDist	= 500.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 80.0f;
	m_fRollSpeed	= 3.5f;
	m_fJumpSpeed	= 600.0f;

	m_fAIMass		= AI_DEFAULT_MASS / 2.0f;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"BONELEECH_SUCK" );

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSGIB;

	m_bCabal = DFALSE;
	m_bJumping = DFALSE;
	m_fLastDmgTime = 0.0f;

	// [blg]
	m_fAIHitPoints   = 10;
	m_fAIRandomHP    = 0;

	m_fLastDetachTime = 0.0f;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD BoneLeech::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if(fData != PRECREATE_SAVEGAME)
				PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			CacheFiles();
			break;
		}

		case MID_TOUCHNOTIFY:
        {
			HOBJECT hObj = (HOBJECT)pData;

			if (hObj && m_hTarget) {
				if(hObj == m_hTarget && m_nCurMetacmd == MC_FIRE_JUMP)
				{
					HCLASS hClass, hTargetClass;

					hTargetClass = g_pServerDE->GetObjectClass(hObj);
					hClass = g_pServerDE->GetClass("CPlayerObj");

					if (g_pServerDE->IsKindOf(hTargetClass,hClass)) {
						m_hEnemyAttach = m_hTrackObject = m_hTarget;
						SetNewState(STATE_EnemyAttach);
					}
					return 0;
				}
			}
		
			break;
        }

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			DDWORD dwResult = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if(m_hEnemyAttach)
			{
				m_hEnemyAttach = g_pPlayerObj->m_hObject;
				SetNewState(STATE_EnemyAttach);

				DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
				dwFlags |= FLAG_REALLYCLOSE;
				dwFlags &= ~FLAG_GRAVITY;
				m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
			}

			return dwResult;
		}

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD BoneLeech::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if(m_hEnemyAttach)
				return 0;
		}

		default : break;
	}

	return AI_Mgr::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void BoneLeech::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\boneleech.abc";
	char* pSkin = "Skins\\Enemies\\boneleech.dtx";
	
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL BoneLeech::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	VEC_MULSCALAR(m_vScale,m_vScale, 2.0f);

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\boneleech");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BoneLeech::MC_Fire_Jump
// DESCRIPTION	: Hit with choke attack
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BoneLeech::MC_Fire_Jump()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_JUMP)
    {
	    DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_JUMP[4]);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed * 2;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed * 2;

		Move(vVel, MATH_EPSILON);

		m_bJumping = DTRUE;

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_JUMP;
    }
    else
	{
		CollisionInfo collisionInfo;
		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			HCLASS hClass = m_pServerDE->GetClass("CPlayerObj");

			if(m_pServerDE->IsKindOf(hClass, m_pServerDE->GetObjectClass(collisionInfo.m_hObject)))
			{
				m_hEnemyAttach = m_hTrackObject = m_hTarget;
				SetNewState(STATE_EnemyAttach);
			}
			else
			{
				StopVelocity();

				m_bAnimating = DFALSE; 
				Metacmd++;
			}
		}
	}
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BoneLeech::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BoneLeech::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(nStim == 0)
	{
		switch(m_nState)
		{
			case STATE_Idle:				SetNewState(STATE_Idle);			break;
			case STATE_SearchVisualTarget:	SetNewState(STATE_Idle);			break;
			case STATE_SearchSmellTarget:	SetNewState(STATE_Idle);			break;
			default:						SetNewState(STATE_SearchSmellTarget);	break;
		}
	}
	else
	{
		if(nStimType == STIM_SMELL)
			SetNewState(STATE_SearchSmellTarget);
		else
			SetNewState(STATE_AttackClose);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BoneLeech::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BoneLeech::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_JUMP)
	{
		SetNewState(STATE_SearchVisualTarget);
		return;
	}

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		if(VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos) <= 100.0f || m_nCurMetacmd == MC_FIRE_JUMP)
					{
						MC_Fire_Jump();
					}
					else
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()))
						{
							MC_Run();
						}
						else
						{
							MC_Idle();
						}
					}

					break;
		case 3:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BoneLeech::AI_STATE_EnemyAttach
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BoneLeech::AI_STATE_EnemyAttach()
{
	HCLASS hEnemy = DNULL;

	if(Metacmd > 4)
		ComputeState();
	else if(m_hEnemyAttach == DNULL)
		Metacmd = 4;
	else
		hEnemy = m_pServerDE->GetObjectClass(m_hEnemyAttach);

	switch(Metacmd)
	{
		case 1:		if(m_pServerDE->IsKindOf(hEnemy, m_pServerDE->GetClass("CPlayerObj")))
					{
						DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
						dwFlags |= FLAG_REALLYCLOSE;
						dwFlags &= ~FLAG_GRAVITY;
						m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

						m_pServerDE->SetModelFilenames(m_hObject,"models\\enemies\\boneleech_pv.abc",
				 											     "skins\\enemies\\boneleech_pv.dtx");

						HMESSAGEWRITE hMsg = m_pServerDE->StartMessage(g_pPlayerObj->GetClient(), SMSG_BONELEECH_ATTACH);
						m_pServerDE->WriteToMessageObject(hMsg, m_hObject);
						m_pServerDE->WriteToMessageObject(hMsg, m_hEnemyAttach);
						m_pServerDE->EndMessage(hMsg);

						Metacmd++;
					}
					else
					{
						m_nCurMetacmd = 999;
						Metacmd++;
					}

					break;
		case 2:		if(m_bProceedAttach)
						Metacmd++;

					break;
		case 3:		if(m_pServerDE->IsKindOf(hEnemy, m_pServerDE->GetClass("CPlayerObj")) && m_hEnemyAttach)
					{
						CBaseCharacter* pAI = (CBaseCharacter*)m_pServerDE->HandleToObject(m_hEnemyAttach);

						if(pAI->IsDead())
						{
							HMESSAGEWRITE hMsg = m_pServerDE->StartMessage(g_pPlayerObj->GetClient(), SMSG_DETACH_AI);
							m_pServerDE->EndMessage(hMsg);
						}
						else if(m_pServerDE->GetTime() - m_fLastDmgTime >= 1.0f)
						{
							BaseClass *ffObj = m_pServerDE->HandleToObject(m_hObject);
							DFLOAT fDamage = (DFLOAT)m_pServerDE->IntRandom(MAX_DAMAGE/2, MAX_DAMAGE);

							DamageObject(m_hObject, ffObj, m_hEnemyAttach, fDamage * m_InventoryMgr.GetDamageMultiplier(), 
										 m_MoveObj.GetForwardVector(), m_MoveObj.GetPos(), DAMAGE_TYPE_MELEE);

							char szSound[256];
							sprintf(szSound, "%s%d.wav", "bl_burrow_", m_pServerDE->IntRandom(1,6));
							m_pAnim_Sound->PlaySound(m_hEnemyAttach, szSound, 200, 100);

							m_fLastDmgTime = m_pServerDE->GetTime();
						}
					}
					else
					{
						m_nCurMetacmd = 999;
						m_bProceedAttach = DFALSE;
						Metacmd++;
					}

					break;
		case 4:				
					if(m_nCurMetacmd != MC_FIRE_JUMP)
					{
						DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);

						if(!(dwFlags & FLAG_GRAVITY))
						{
							dwFlags &= ~FLAG_REALLYCLOSE;
							dwFlags |= FLAG_GRAVITY;
							m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
						}

						m_pServerDE->SetModelFilenames(m_hObject,"models\\enemies\\boneleech.abc",
				 												 "skins\\enemies\\boneleech.dtx");

						m_fLastDetachTime = m_pServerDE->GetTime();

						DVector vPos,vU,vR,vF;
						DRotation rRot;

						m_pServerDE->GetObjectPos(m_hTrackObject, &m_vTargetPos);
						m_pServerDE->GetObjectRotation(m_hTrackObject, &rRot);
						m_pServerDE->SetObjectRotation(m_hObject, &rRot);
						m_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

						VEC_ADDSCALED(vPos, m_vTargetPos, vF, -100.0f);
						
						MC_FacePos(vPos);
						Metacmd--;

						m_pServerDE->MoveObject(m_hObject, &m_vTargetPos);

					}

					Metacmd++;

					break;

		case 5:
					if ((m_pServerDE->GetTime() - m_fLastDetachTime) > 4.0f)
						Metacmd++;

					break;

		default:	ComputeState();				
					break;
	}

	return;
}

void BoneLeech::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	m_pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bJumping);
	m_pServerDE->WriteToMessageFloat(hWrite, m_fLastDmgTime);
	m_pServerDE->WriteToMessageFloat(hWrite, m_fLastDetachTime);
}

void BoneLeech::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	m_bJumping = (DBOOL)m_pServerDE->ReadFromMessageByte(hRead);
	m_fLastDmgTime = m_pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDetachTime = m_pServerDE->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void BoneLeech::CacheFiles()
{
	// Sanity checks...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if(!(pServerDE->GetServerFlags() & SS_CACHING))
	{
		return;
	}

	if (!m_hObject) return;


	// Get the model filenames...

	char sModel[256] = { "" };
	char sSkin[256]  = { "" };

	pServerDE->GetModelFilenames(m_hObject, sModel, 255, sSkin, 255);


	// Cache models...

	pServerDE->CacheFile(FT_MODEL, sModel);


	// Cache textures...

	pServerDE->CacheFile(FT_TEXTURE, sSkin);


	// Cache sounds...

	SetCacheDirectory("sounds\\enemies\\boneleech");

	CacheSoundFileRange("bl_burrow_", 1, 3);
	CacheSoundFileRange("bl_death", 1, 3);
	CacheSoundFileRange("bl-death", 1, 3);
	CacheSoundFileRange("bl_move_", 1, 3);
	CacheSoundFileRange("bl_pain_", 1, 3);
	CacheSoundFileRange("bl_spot", 1, 3);
}


