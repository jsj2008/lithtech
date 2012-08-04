
// ----------------------------------------------------------------------- //
//
// MODULE  : WurmAI.cpp
//
// PURPOSE : WurmAI - Definition
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "WurmAI.h"
#include "cpp_server_de.h"

BEGIN_CLASS(WurmAI)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(HitPoints, 30.0f)     \
    ADD_REALPROP(RandomHitPoints, 50.0f) \
    ADD_REALPROP(ArmorPoints, 0.0f)     \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIBrain, "STRONG")   \
END_CLASS_DEFAULT(WurmAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		WurmAI::m_bLoadAnims = DTRUE;
CAnim_Sound	WurmAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WurmAI::WurmAI() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 1000.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 800.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_nMoveFlags = 0;

	m_bCabal = DTRUE;

	m_bFlying = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD WurmAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

void WurmAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\wurm.abc";
	char* pSkin = "Skins\\Enemies\\wurm.dtx";
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

DBOOL WurmAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	m_hObject = m_pServerDE->ObjectToHandle(this);
	if (!m_hObject) return DFALSE;

    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\wurm");
		m_bLoadAnims = DFALSE;
	}

	//Determine damage modifier
	switch(BrainStrToInt(m_szAIBrain))
	{
		case BRAIN_WEAK:	m_InventoryMgr.AddDamageMultiplier(0.3f);	
							SetSenseDistance(m_fSensingDist * 0.5f);	break;
		case BRAIN_STRONG:	m_InventoryMgr.AddDamageMultiplier(0.4f);	
							SetSenseDistance(m_fSensingDist);			break;
		case BRAIN_BADASS:	m_InventoryMgr.AddDamageMultiplier(0.5f);	
							SetSenseDistance(m_fSensingDist * 1.5f);	break;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::MC_Float
// DESCRIPTION	: Rise into the air
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::MC_Fly(DBOOL bUp)
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FLYING)
    {
        DBOOL bRet = m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_RUN[4],m_vScale);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DFALSE);
        
		DVector vVel;
		VEC_INIT(vVel);

		if(bUp)
		{
			VEC_ADDSCALED(vVel, vVel, m_MoveObj.GetUpVector(), 25.0f);
			VEC_ADDSCALED(vVel, vVel, m_MoveObj.GetForwardVector(), m_fRunSpeed);

			//turn off gravity
			DDWORD dwFlags = m_pServerDE->GetObjectFlags(m_hObject);
			dwFlags &= ~FLAG_GRAVITY;
			m_pServerDE->SetObjectFlags(m_hObject, dwFlags);
		}
		else
		{
			VEC_ADDSCALED(vVel, vVel, m_MoveObj.GetUpVector(), -45.0f);
			VEC_ADDSCALED(vVel, vVel, m_MoveObj.GetForwardVector(), m_fRunSpeed);
		}

		Move(vVel, MATH_EPSILON);

		m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_FLYING;
    }
    else
	{
		if(bUp)
		{
			if(VEC_DIST(m_MoveObj.GetPos(), m_MoveObj.GetLastPos()) <= 0.0f)
			{
				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			IQuery.m_FilterFn = DNULL;
			IQuery.m_pUserData = DNULL;	

			VEC_COPY(IQuery.m_From, m_MoveObj.GetPos());

			if(m_hTarget)
			{
				VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, m_MoveObj.GetUpVector(), -1.0f * (m_vTargetPos.y + 50.0f));
			}
			else
			{
				VEC_ADDSCALED(IQuery.m_To, IQuery.m_From, m_MoveObj.GetUpVector(), -150.0f);
			}

			if(!m_pServerDE->IntersectSegment(&IQuery, &IInfo))
			{
				DVector vVel;
				VEC_INIT(vVel);
				Move(vVel, MATH_EPSILON);

				m_bFlying = DTRUE;

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

				m_bFlying = DFALSE;

				m_bAnimating = DFALSE;
				Metacmd++;
				return;
			}
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::MC_Run
// DESCRIPTION	: Run the run animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::MC_Run()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
	    DBOOL bRet = m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_RUN[4],m_vScale);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

        m_pServerDE->SetModelLooping(m_hObject, DTRUE);

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fRunSpeed))
		{
			NavigateObstacle();
		}

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
    
		//Are we done running?
		if(m_pServerDE->GetModelPlaybackState(m_hObject) & MS_PLAYDONE)
        {
            m_bAnimating = DFALSE; 
            Metacmd++;
        }
    }
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::MC_Fire_Stand
// DESCRIPTION	: Hit with tail swipe
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::MC_Fire_Stand()
{
    if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_FIRE_STAND)
    {
	    DBOOL bRet = m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_FIRE_STAND[4],m_vScale);

		DVector vVel;
		VEC_INIT(vVel);
		Move(vVel, MATH_EPSILON);

		//SCHLEGZ 4/22/98 3:27:12 PM: if we can't stand up, crawl out from underneath
		if(!bRet)
		{
			SetNewState(STATE_CrawlUnderObj);
			return;
		}

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
			return;
		}
    }      
	
    return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::ComputeState()
{
	if(!ComputeStimuli())
	{
		switch(m_nState)
		{
			case STATE_Idle:		SetNewState(STATE_Idle);			break;
			default:				SetNewState(STATE_SearchTarget);	break;
		}
	}
	else
	{
		SetNewState(STATE_AttackClose);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::AI_STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::AI_STATE_AttackClose()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	switch(Metacmd)
	{
		case 1:		char szSound[256];
					_mbscpy((unsigned char*)szSound, (const unsigned char*)SOUND_ANGER);
					m_pAnim_Sound->GetSoundPath(szSound,m_pServerDE->IntRandom(1,NUM_WRM_IDLE));

					PlayAISound(szSound, 1000.0f, PLAY_WAIT);					

					Metacmd++;			break;
		case 2:		MC_FaceTarget();	break;	
		case 3:		if(VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos) <= 100.0f)
					{
						if(!m_bFlying)
							MC_Fly(DTRUE);
						else
							MC_Fire_Stand();
					}
					else
					{
						MC_FaceTarget();
						Metacmd--;

						if(m_bFlying)
							MC_Run();
						else
							MC_Fly(DTRUE);
					}

					break;
		case 4:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: WurmAI::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void WurmAI::AI_STATE_Idle()
{
	if(ComputeStimuli())
	{
		ComputeState();
		return;
	}

	switch(Metacmd)
	{
		case 1:		char szSound[256];
					_mbscpy((unsigned char*)szSound, (const unsigned char*)SOUND_IDLE);
					m_pAnim_Sound->GetSoundPath(szSound,m_pServerDE->IntRandom(1,NUM_WRM_IDLE));

					PlayAISound(szSound, 1000.0f, PLAY_WAIT);					

					Metacmd++;			break;
		case 2:		if(m_bFlying)
						MC_Fly(DFALSE);
					else	
						MC_Idle();			
					
					break;
		case 3:		ComputeState();		break;
	}

	return;
}
