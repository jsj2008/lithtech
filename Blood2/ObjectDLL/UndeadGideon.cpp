
#include <stdio.h>
#include "UndeadGideon.h"
#include "cpp_server_de.h"
#include "smellhint.h"
#include "PlayerObj.h"


BEGIN_CLASS(UndeadGideon)
    ADD_REALPROP(RandomHitPoints, 50.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_VECTORPROP_VAL_FLAG(Dims, 70.0f, 88.0f, 70.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(UndeadGideon, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		UndeadGideon::m_bLoadAnims = DTRUE;
CAnim_Sound	UndeadGideon::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

UndeadGideon::UndeadGideon() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 500.0f;
	m_fSmellingDist	= 2000.0f;
	m_fSeeingDist	= 1000.0f;

	m_fWalkSpeed	= 180.0f;
	m_fRunSpeed		= 350.0f;
	m_fRollSpeed	= 320.0f;
	m_fJumpSpeed	= 360.0f;

	m_fAIMass		= 3000.0f;

	m_nAIStrength	= 5;

	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"GIDEON_SPEAR" );
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"SKULL" );
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"GIDEON_VOMIT");
	_mbscpy((unsigned char*)m_szAIWeapon[3], (const unsigned char*)"LIGHTNING");
	_mbscpy((unsigned char*)m_szAIWeapon[4], (const unsigned char*)"BEHEMOTH_SHOCKWAVE");

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_JUMP | FLAG_DODGE | FLAG_ALWAYSRECOIL;
	m_bClimbing = DFALSE;

	m_bCabal = DFALSE;

	m_bCreateHealth = DTRUE;

	// [blg]
	m_fAIHitPoints   = 12000;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	m_fFullHealth	 = 0;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD UndeadGideon::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
			if (m_bCreateHealth)
			{
				if (!m_fFullHealth)
				{
					m_fFullHealth = m_damage.GetHitPoints();
					HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
					m_pServerDE->WriteToMessageFloat(hWrite,1.0f);
					m_pServerDE->EndMessage(hWrite);
					m_bCreateHealth = DFALSE;
				}
				else
				{
					DFLOAT fTemp = m_damage.GetHitPoints() / m_fFullHealth;	
					HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
					m_pServerDE->WriteToMessageFloat(hWrite,fTemp);
					m_pServerDE->EndMessage(hWrite);
				}
			}
		}
		break;

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

DDWORD UndeadGideon::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_DAMAGE:
		{
			if (!m_pServerDE) break;
			
			CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
			
			DFLOAT fTemp = m_damage.GetHitPoints() / m_fFullHealth;	
			HMESSAGEWRITE hWrite = m_pServerDE->StartMessage(NULL, SMSG_BOSSHEALTH);
			m_pServerDE->WriteToMessageFloat(hWrite,fTemp);
			m_pServerDE->EndMessage(hWrite);
		}

		default: break;
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

void UndeadGideon::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\Undead_Gideon.abc";
	char* pSkin = "Skins\\Enemies\\Undead_Gideon.dtx";
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

DBOOL UndeadGideon::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	m_hObject = m_pServerDE->ObjectToHandle(this);
	if (!m_hObject) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\Undead_Gideon");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: UndeadGideon::MC_Jump
// DESCRIPTION	: Run the jump animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void UndeadGideon::MC_Jump()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_JUMP)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_JUMP[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed * 3;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed * 3;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_JUMP;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		DVector vVel;

		m_pServerDE->GetVelocity(m_hObject,&vVel);
		vVel.x = m_MoveObj.GetForwardVector().x * m_fRunSpeed * 3;
		vVel.z = m_MoveObj.GetForwardVector().z * m_fRunSpeed * 3;
		Move(vVel, MATH_EPSILON);

		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if (collisionInfo.m_hObject)
		{
			DVector vVel;
			VEC_INIT(vVel);

			Move(vVel, MATH_EPSILON);

			HCLASS hClass = m_pServerDE->GetObjectClass(m_hObject);

			if (!m_pServerDE->IsKindOf(hClass,m_pServerDE->GetClass("CBaseCharacter")))
			{
				m_bAnimating = DFALSE; 
				Metacmd++;
			}
			else
			{
				m_nCurMetacmd = 999;
			}
		}
    }               
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: UndeadGideon::MC_Fire_Stand
// DESCRIPTION	: Run the fire_stand animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void UndeadGideon::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
        DBOOL bRet = DFALSE;
		DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);
		DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

		if(m_nState == STATE_AttackFar)
		{
			if (bAbove && fHeight > m_vDims.y)
			{
				m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_VOMIT);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);
			}
			else
			{
				if (IsRandomChance(70))
				{
					if (IsRandomChance(50))
					{
						m_InventoryMgr.ChangeWeapon(WEAP_SKULL);
						bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MAGIC]);
					}
					else
					{
						m_InventoryMgr.ChangeWeapon(WEAP_DRUDGE_LIGHTNING);
						bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MAGIC]);
					}
				}
				else
				{
					m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_VOMIT);
					bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
				}
			}
		}
		else
		{
			HCLASS hPlayer = m_pServerDE->GetClass("CPlayerObj");
			HCLASS hClass = m_pServerDE->GetObjectClass(m_hTarget);

			if (m_pServerDE->IsKindOf(hClass,hPlayer))
			{
				CPlayerObj *pPlayer = (CPlayerObj *)(m_pServerDE->HandleToObject(m_hTarget));

				if (pPlayer->IsImprisoned())
				{
					if (IsRandomChance(40))
					{
						m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_VOMIT);
						bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_RIFLE]);
					}
					else
					{
						m_InventoryMgr.ChangeWeapon(WEAP_BEHEMOTH_SHOCKWAVE);
						bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MAGIC]);
					}
				}
				else
				{
					m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_SPEAR);
					bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);
				}
			}
			else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_SPEAR);
				bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[TYPE_MELEE]);
			}
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_STAND;
    }
    else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_InventoryMgr.ChangeWeapon(WEAP_GIDEON_SPEAR);

			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: UndeadGideon::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void UndeadGideon::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

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
		else if(m_fStimuli[SIGHT] <= 0.85f || fHeight > m_vDims.y)
			SetNewState(STATE_AttackFar);
		else
			SetNewState(STATE_AttackClose);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: UndeadGideon::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void UndeadGideon::AI_STATE_AttackClose()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;
	
	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		if(m_fStimuli[SIGHT] <= 405.0f || m_nCurMetacmd == MC_FIRE_STAND 
						|| (bAbove && fHeight > m_vDims.y))
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] >= (m_fSeeingDist * 0.33))
							ComputeState();
						else
							MC_Walk();		
					}
					
					break;
		case 3:		ComputeState();									break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: UndeadGideon::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void UndeadGideon::AI_STATE_AttackFar()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL && m_nCurMetacmd != MC_FIRE_STAND && m_nCurMetacmd != MC_JUMP)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;
	
	MC_FaceTarget();
	Metacmd--;
					
	switch(Metacmd)
	{
		case 1:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND 
						|| (bAbove && fHeight > m_vDims.y))
					{
						MC_Fire_Stand();
					}
					else
					{
						if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.20))
							ComputeState();
						else
							MC_Walk();		
					}
					
					break;
		case 2:		if((m_fStimuli[SIGHT] < (m_fSeeingDist * 0.20)) && m_nCurMetacmd != MC_JUMP
						|| (bAbove && fHeight > m_vDims.y))
					{
						ComputeState();
					}
					else if(m_fStimuli[SIGHT] < (m_fSeeingDist * 0.30) || m_nCurMetacmd == MC_JUMP
							|| IsLedge(m_MoveObj.GetForwardVector()))
					{
						MC_Jump();
					}
					else
						MC_Walk();		

					break;
		case 3:		ComputeState();					break;
	}

	return;
}

void UndeadGideon::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	m_pServerDE->WriteToMessageFloat(hWrite, m_fFullHealth);
}

void UndeadGideon::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	m_fFullHealth = m_pServerDE->ReadFromMessageFloat(hRead);
}


DBOOL UndeadGideon::Fire(DBOOL bAltFire)
{
	DVector vPos, vDir;
	DRotation rRot;

	// Sanity check (GK 9/18/98)
	if (!m_InventoryMgr.GetCurrentWeapon())
		return DFALSE;

	m_pServerDE->GetModelNodeTransform(m_hObject,"head",&vPos,&rRot);
	VEC_SUB(vDir, m_vTargetPos, vPos);
	VEC_NORM(vDir);

	vDir.x = m_MoveObj.GetForwardVector().x;
	vDir.z = m_MoveObj.GetForwardVector().z;

	m_pServerDE->AlignRotation(&rRot, &vDir, DNULL);

	DDWORD m_nFiredWeapon = m_InventoryMgr.FireCurrentWeapon(&vPos, &rRot, bAltFire);

	return DTRUE;
}

void UndeadGideon::MC_Dodge_Left()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_LEFT)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_JUMP[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel, vLeft;
		VEC_MULSCALAR(vLeft, m_MoveObj.GetRightVector(), -1.0f);
		
		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = vLeft.x * m_fRunSpeed;
		vVel.z = vLeft.z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_DODGE_LEFT;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		DVector vVel;

		m_pServerDE->GetVelocity(m_hObject,&vVel);

		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if ((collisionInfo.m_hObject == m_pServerDE->GetWorldObject()) && (vVel.y <= 0.0f))
		{
			DVector vVel;
			VEC_INIT(vVel);

			Move(vVel, MATH_EPSILON);

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

void UndeadGideon::MC_Dodge_Right()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_DODGE_RIGHT)
    {
        SetAnimation(m_pAnim_Sound->m_nAnim_JUMP[TYPE_MELEE]);

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		DVector vVel;

		vVel.y = m_MoveObj.GetUpVector().y * m_fJumpSpeed;
		vVel.x = m_MoveObj.GetRightVector().x * m_fRunSpeed;
		vVel.z = m_MoveObj.GetRightVector().z * m_fRunSpeed;

		Move(vVel, MATH_EPSILON);

		m_nCurMetacmd = MC_DODGE_RIGHT;
        m_bAnimating = DTRUE; 
    }
    else
    {        
		CollisionInfo collisionInfo;
		DVector vVel;

		m_pServerDE->GetVelocity(m_hObject,&vVel);

		m_pServerDE->GetStandingOn(m_hObject, &collisionInfo);

		if ((collisionInfo.m_hObject == m_pServerDE->GetWorldObject()) && (vVel.y <= 0.0f))
		{
			DVector vVel;
			VEC_INIT(vVel);

			Move(vVel, MATH_EPSILON);

			m_bAnimating = DFALSE; 
			Metacmd++;
		}
    }               
	
    return;
}

void UndeadGideon::AI_STATE_Dodge()
{
	switch(Metacmd)
	{
		case 1:		m_nDodgeFlags = CalculateDodge(m_vTrackObjPos);
					Metacmd++;		break;
		case 2:		if(m_nDodgeFlags & RIGHT)
						MC_Dodge_Right();
					else if(m_nDodgeFlags & LEFT)
						MC_Dodge_Left();
					else
						Metacmd++;

					break;
		case 3:		SetNewState(STATE_Idle);	break;
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void UndeadGideon::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\undeadgid");

	CacheSoundFileRange("foot", 1, 3);
	CacheSoundFileRange("spit", 1, 3);
	CacheSoundFileRange("stab", 1, 3);
}


