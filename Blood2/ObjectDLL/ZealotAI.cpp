// ----------------------------------------------------------------------- //
//
// MODULE  : ZealotAI.cpp
//
// PURPOSE : ZealotAI - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "ZealotAI.h"
#include "cpp_server_de.h"
#include "sfxmsgids.h"
#include "gameprojectiles.h"
#include "SoundTypes.h"

BEGIN_CLASS(ZealotAI)
   ADD_REALPROP(RandomHitPoints, 50.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_BOOLPROP(Divine, DFALSE)
	ADD_VECTORPROP_VAL_FLAG(Dims, 12.0f, 44.0f, 12.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(ZealotAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		ZealotAI::m_bLoadAnims = DTRUE;
CAnim_Sound	ZealotAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ZealotAI::ZealotAI() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 3000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 800.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 200.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	strcpy(m_szAIWeapon[0], "HEAL" );
	strcpy(m_szAIWeapon[1], "SHIELD" );
	strcpy(m_szAIWeapon[2], "ENERGY_BLAST" );
	strcpy(m_szAIWeapon[3], "GROUND_FIRE" );
	strcpy(m_szAIWeapon[4], "SHOCKWAVE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSRECOIL;

	m_bCabal = DTRUE;

	m_bFloat = DFALSE;
	m_hStaffEffect = DNULL;
	m_hAttach = DNULL;
	m_bStartStaffEffect = DFALSE;
	m_bDivine = DFALSE;

	// [blg]
	m_fAIHitPoints   = 100;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 50;

	strcpy(m_szAIState, "IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ZealotAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0)
			{
				GetServerDE()->GetPropBool("Divine", &m_bDivine);
			}

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

void ZealotAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\Zealot.abc";
	char* pSkin = DNULL;

	if(m_bDivine)
		pSkin = "Skins\\Enemies\\ZealotDivine.dtx";
	else
		pSkin = "Skins\\Enemies\\Zealot.dtx";

	strcpy(pStruct->m_Filename, pFilename);
	strcpy(pStruct->m_SkinName, pSkin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL ZealotAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\Zealot");
		m_bLoadAnims = DFALSE;
	}

	if(m_bDivine)
	{
		m_fAIHitPoints *= 2.0f;
		m_fAIRandomHP *= 2.0f;

		m_InventoryMgr.AddDamageMultiplier(1.5f);	
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_Heal
// DESCRIPTION	: Run the heal  animation and effect
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Heal()
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
			m_damage.SetHitPoints(fHP * 1.5f);

			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_Shield
// DESCRIPTION	: Run the shield  animation and effect
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Shield(HOBJECT hObject)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_SHIELD)
    {
		m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_SHIELD);
        DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[6]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_SHIELD;
    }
    else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_Float
// DESCRIPTION	: Rise into the air
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Float(DBOOL bUp)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FLOAT)
    {
        DBOOL bRet = SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[0]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		DVector vVel;
//		m_pServerDE->GetVelocity(m_hObject, &vVel);

		if(bUp)
		{
			VEC_MULSCALAR(vVel, m_MoveObj.GetUpVector(), 45.0f);

			//turn off gravity
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
		}
		else
		{
			VEC_MULSCALAR(vVel, m_MoveObj.GetUpVector(), -65.0f);
		}

		Move(vVel, MATH_EPSILON);

		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FLOAT;
    }
    else
	{
		if(bUp)
		{
			IntersectQuery IQuery;
			IntersectInfo IInfo;

			IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			IQuery.m_FilterFn = DNULL;
			IQuery.m_pUserData = DNULL;	

			VEC_COPY(IQuery.m_From, m_MoveObj.GetPos());
			VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, m_MoveObj.GetUpVector(), -75.0f);
			
			if(!m_pServerDE->IntersectSegment(&IQuery, &IInfo))
			{
				DVector vVel;
				VEC_INIT(vVel);
				Move(vVel, MATH_EPSILON);

				m_bFloat = DTRUE;

				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}
		else
		{
			DVector vVel;
			m_pServerDE->GetVelocity(m_hObject, &vVel);

			if(vVel.y >= -0.1f)
			{
				//turn on gravity
				DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
				dwFlags |= FLAG_GRAVITY;
				m_pServerDE->SetObjectFlags(m_hObject, dwFlags);

				m_bFloat = DFALSE;

				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_FadeOut
// DESCRIPTION	: Run an fade out animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_FadeOut()
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
// ROUTINE		: ZealotAI::MC_Idle
// DESCRIPTION	: Run an idle animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Idle()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_IDLE)
    {
		DBOOL bRet = DFALSE;

		if(m_bFloat)
			bRet = SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[0]);
		else
			bRet = SetAnimation(m_pAnim_Sound->m_nAnim_IDLE[1]);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

		m_pServerDE->SetModelLooping(m_hObject, DFALSE);

        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_IDLE;
	}
	else
	{
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE;
            Metacmd++;
            return;
        }
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_Fire_Stand
// DESCRIPTION	: Run the energy blast, shockwave, or fire animation and effect
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
		DBOOL bRet = DFALSE;

		if(m_nState == STATE_AttackClose)
		{
			if(m_pServerDE->IntRandom(1,2) == 1)
			{
				m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_GROUNDFIRE);
			}
			else
			{
				m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_SHOCKWAVE);
			}

			bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[5]);
		}
		else
		{
			m_InventoryMgr.ChangeWeapon(WEAP_ZEALOT_ENERGYBLAST);
	        bRet = SetAnimation(m_pAnim_Sound->m_nAnim_FIRE_STAND[4]);
		}

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);

		//create the spellcasting effect
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_ObjectType = OT_NORMAL;
		m_pServerDE->GetModelNodeTransform(m_hObject,"r_hand_extra2",&ocStruct.m_Pos,&ocStruct.m_Rotation);
		ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
		
		HCLASS hClass = m_pServerDE->GetClass("BaseClass");
		BaseClass* pObj = m_pServerDE->CreateObject(hClass, &ocStruct);
		m_hStaffEffect = pObj->m_hObject;

		m_bStartStaffEffect = DTRUE;
        
		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FIRE_STAND;
    }
    else
	{
		if(m_bStartStaffEffect)
		{
			DVector		offset;
			VEC_SET(offset, 0.0f, 0.0f, 0.0f);

			HMESSAGEWRITE hMessage = m_pServerDE->StartInstantSpecialEffectMessage(&offset);
			m_pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);
			m_pServerDE->WriteToMessageObject(hMessage, m_hStaffEffect);
			m_pServerDE->WriteToMessageVector(hMessage, &offset);
			m_pServerDE->WriteToMessageFloat(hMessage, 2.0f);
			m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SCALENUMPARTICLES);

			if(m_bDivine)
				m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_FIRETRAIL_1);
			else
				m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_FIRETRAIL_2);

			m_pServerDE->WriteToMessageDWord(hMessage, 0);
			m_pServerDE->EndMessage(hMessage);

			DRotation rRot;
			ROT_INIT(rRot);
			m_pServerDE->CreateAttachment(m_hObject, m_hStaffEffect, "r_hand_extra2", &offset, &rRot, &m_hAttach);

			m_bStartStaffEffect = DFALSE;
		}

		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)		
		{
			m_pServerDE->RemoveAttachment(m_hAttach);
			m_pServerDE->RemoveObject(m_hStaffEffect);

			m_hAttach = DNULL;
			m_hStaffEffect = DNULL;

			m_bAnimating = DFALSE;
			Metacmd++;
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::MC_Dead
// DESCRIPTION	: Run death animation based on node hit
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::MC_Dead()
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

			DDWORD nType = 0;
			
			if(!m_bDivine)
				nType = EXP_SHOCKBALL;
			else
				nType = EXP_DIVINE_SHOCKBALL;

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

			DFLOAT fDamage = (DFLOAT)m_pServerDE->IntRandom(25,50);
			DamageObjectsInRadius(m_hObject, this, m_MoveObj.GetPos(), LIFELEECH_PUSH_RADIUS,
								  fDamage, DAMAGE_TYPE_EXPLODE);

			if(m_hStaffEffect)
			{
				m_pServerDE->RemoveAttachment(m_hAttach);
				m_pServerDE->RemoveObject(m_hStaffEffect);

				m_hAttach = DNULL;
				m_hStaffEffect = DNULL;
			}

			RemoveMe();
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

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
		if((m_fStimuli[HEALTH] < 0.5f) && (m_nState != STATE_Escape_Hide))
		{
			SetNewState(STATE_Escape_Hide);
			return;
		}

		if(m_fStimuli[SENSE] >= 0.85f)
			SetNewState(STATE_AttackClose);
		else
			SetNewState(STATE_AttackFar);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::AI_STATE_AttackClose()
{
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
		case 2:		if(m_fStimuli[SENSE] <= (m_fSensingDist * 0.10) && m_nCurMetacmd != MC_FIRE_STAND)
					{
						SetNewState(STATE_Teleport);
					}
					else
						Metacmd++;

					break;
		case 3:		MC_FaceTarget();
					Metacmd--;

					if(m_fStimuli[SENSE] >= (m_fSensingDist * 0.15) && m_nCurMetacmd != MC_FIRE_STAND)
						ComputeState();
					else
					{
						if(m_bFloat || m_nCurMetacmd == MC_FLOAT)
							MC_Float(DFALSE);
						else
							MC_Fire_Stand();
					}

					break;
		case 4:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::AI_STATE_AttackFar()
{
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
		case 2:		if(m_fStimuli[SENSE] > (m_fSensingDist * 0.75))
					{
						if(m_bFloat)
							MC_Run();
						else
							MC_Float(DTRUE);
					}
					else
						Metacmd++;

					break;
		case 3:		MC_FaceTarget();
					Metacmd--;

					if(m_fStimuli[SENSE] <= (m_fSensingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND)
					{
						if(m_bFloat || m_nCurMetacmd == MC_FLOAT)
							MC_Float(DFALSE);
						else
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
// ROUTINE		: ZealotAI::AI_STATE_Escape_Hide
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::AI_STATE_Escape_Hide()
{
	switch(Metacmd)
	{
		case 1:		MC_Heal();			break;	
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: ZealotAI::AI_STATE_Teleport
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void ZealotAI::AI_STATE_Teleport()
{
	switch(Metacmd)
	{
		case 1:		MC_FadeOut();		break;
		case 2:
		{
					char szSound[256];
					sprintf(szSound, "%s%d.wav", SOUND_LAUGH, m_pServerDE->IntRandom(1,3));
					m_pAnim_Sound->PlaySound(m_hObject, szSound, 2000.0f, 100);					

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

void ZealotAI::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DBYTE val;

	val = m_bFloat | (m_bStartStaffEffect * 0x02) | (m_bDivine * 0x04);
	pServerDE->WriteToMessageByte(hWrite,val);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxHeight);
//	pServerDE->WriteToMessageObject(hWrite, m_hStaffEffect);
}

void ZealotAI::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DBYTE val;

	val = m_pServerDE->ReadFromMessageByte(hRead);
	m_bFloat = val & 0x01;
	m_bStartStaffEffect = ((val & 0x02) != 0);
	m_bDivine = ((val & 0x04) != 0);

	m_fMaxHeight = pServerDE->ReadFromMessageFloat(hRead);
//	m_hStaffEffect = pServerDE->ReadFromMessageObject(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void ZealotAI::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\zealot");

	CacheSoundFileRange("death", 1, 3);
	CacheSoundFileRange("anger", 1, 3);
	CacheSoundFileRange("fear", 1, 3);
	CacheSoundFileRange("idle", 1, 3);
	CacheSoundFileRange("laugh", 1, 3);
	CacheSoundFileRange("pain", 1, 3);
	CacheSoundFileRange("spot", 1, 3);

	CacheSoundFile("shield");
	CacheSoundFile("magic");
}


