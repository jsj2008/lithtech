// ----------------------------------------------------------------------- //
//
// MODULE  : BirdAI.cpp
//
// PURPOSE : BirdAI - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

// 10.4 Incidental Creatures
// This section covers benign creatures whose only purpose 
// in the game is atmosphere or target practice.

// 1.1.210.4.2 Birds
// Birds are your basic crows.  They are carrion eaters, and will feed off any corpses
// laying around.  They fly away when anything approaches.


#include <stdio.h>
#include "BirdAI.h"
#include "ObjectUtilities.h"
#include "cpp_server_de.h"



BEGIN_CLASS(BirdAI)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(SightDistance, 1000.0f) \
    ADD_REALPROP(SenseDistance, 400.0f) \
    ADD_REALPROP(SmellDistance, 300.0f)   \
    ADD_REALPROP(ListenDistance, 0.0f)  \
    ADD_REALPROP(Mass, 2.0f)           \
    ADD_REALPROP(HitPoints, 5.0f)     \
    ADD_REALPROP(RandomHitPoints, 5.0f) \
    ADD_REALPROP(ArmorPoints, 0.0f)     \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIBrain, "WEAK")   \
	ADD_STRINGPROP(AIWeapon1, "MELEE")   \
END_CLASS_DEFAULT(BirdAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		BirdAI::m_bLoadAnims = DTRUE;
CAnim_Sound	BirdAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BirdAI::BirdAI() : AI_Mgr()
{
	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE" );
  	_mbscpy((unsigned char*)m_szAIBrain, (const unsigned char*)"STRONG" );
  	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"MELEE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_nMoveFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD BirdAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0)
				AI_Mgr::ReadProp((ObjectCreateStruct*)pData);  // inside BaseCharacter

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

void BirdAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\bird.abc";
	char* pSkin = "Skins\\Enemies\\bird.dtx";
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);

//	pStruct->m_Flags = FLAG_VISIBLE;
	pStruct->m_Flags = FLAG_VISIBLE | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_SHADOW;	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL BirdAI::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\bird");
		m_bLoadAnims = DFALSE;
	}

	//randomize the size/weight
	DFLOAT fScale = pServerDE->Random(-0.1f, 0.1f);
	VEC_SET(m_vScale,1.0f - fScale, 1.0f + fScale, 1.0f - fScale);

    // Init the objects
	m_InventoryMgr.Init(m_hObject);
    
	m_damage.Init(m_hObject);
	m_damage.SetHitPoints(m_fAIHitPoints + pServerDE->Random(1,m_fAIRandomHP));
	m_damage.SetMaxHitPoints(100.0f);

	m_damage.SetArmorPoints(m_fAIArmorPoints);
	m_damage.SetMaxArmorPoints(200.0f);
	m_damage.SetMass(m_fAIMass);
    
	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Fanatic::MC_NS_Threat
//
//	PURPOSE:	This function is the primary definition of the AI's
//				behavior/tactics.
//
// ----------------------------------------------------------------------- //
/*        
void AI_Bird::MC_NS_Threat()
{
	AI_ComputeThreat();

	//We are dying so go into appropriate death state
	if(m_Condition == DYING)
	{
		NewState(STATE_Dying,0,0,0,0);
		return;
	}

    // Analyze the threat level
    switch(m_AIThreat)
    {
        case THREAT_IGNORE :  
        {
			if(m_nState != STATE_FindCarrion)
			{
				NewState(STATE_FindCarrion,0,0,0,0);	
			}

			break;
        }
    
        case THREAT_LOW :  
        {
			break;
        }
        
        case THREAT_MED :  
        {
			if(m_nState != STATE_EscapeEnemy && AI_CheckSenseEnemy())
			{
				NewState(STATE_EscapeEnemy,0,0,0,0);	
			}

			break;
        }
        
        case THREAT_HIGH   :  
        {
			if(m_nState != STATE_EscapeEnemy && AI_CheckSenseEnemy())
			{
				NewState(STATE_EscapeEnemy,0,0,0,0);	
			}

			break;
        }        
    }

	return;
}

void AI_Bird::AI_STATE_FindCarrion()
{
//
// Set the SubState
//
    if (m_nSubState == SS_EMPTY)
    {
		if(AI_CheckSenseEnemy()) 
		{
			m_nSubState = SS_SENSE_ENEMY; 
		}
    }

//
// Execute the SubState Meta commands
//
    switch(m_nSubState)
    {
        case SS_SENSE_ENEMY:
        {
            switch(Metacmd)
            {
                case 1: MC_Facetarget();    break;
                case 2: MC_Scared();        break;
                case 3: MC_Turn180();       break;		
                case 4: MC_Jump();           break;
                case 5: MC_Run();           break;
                case 6: NewState(STATE_EscapeEnemy,0,0,0,0); break;
            }                
        } break;
        
        // Make the SS_DEFAULT the default
        default:
        {
			AI_Mgr::AI_STATE_FindCarrion();
			return;
        }
	}

	MC_NS_Threat();

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_STATE_EscapeEnemy
//
//	PURPOSE:	Escape the Enemy
//
// ----------------------------------------------------------------------- //
void AI_Bird::AI_STATE_EscapeEnemy()
{


//
// Set the SubState
//
    if (m_nSubState == SS_EMPTY)
    {
        // Always check each one...    
        if ( AI_CheckSenseEnemy() )					m_nSubState = SS_SENSE_ENEMY;
        else if ( AI_CheckIsThereNoTarget() )       m_nSubState = SS_NO_TARGET; 
        else if ( AI_CheckIsTargetOutofSite() )     m_nSubState = SS_TARGET_OUTOSITE;        
    }

    if (m_nSubState == SS_EMPTY)   m_nSubState = SS_DEFAULT;

//
// Execute the SubState Meta commands
//
    switch(m_nSubState)
    {

        case SS_NO_TARGET:
        {
            switch(Metacmd)
            {
                case 1: if(m_fHeight)
							MC_Crouch();  
						else
							MC_Idle();

						break;
                case 2: NewState(STATE_FindCarrion,0,0,0,0); break;
            }                
        } break;
        
        
        case SS_TARGET_OUTOSITE:
        {
            switch(Metacmd)
            {
				case 1: if(m_fHeight)
							MC_Crouch();		
						else
							MC_Idle();

						break;
                case 2: MC_Taunt();			break;
                case 3: MC_Walk();          break;
                case 4: MC_Walk();          break;
                case 5: NewState(STATE_FindCarrion,0,0,0,0); break;
            }                
        } break;

        case SS_SENSE_ENEMY:
        {
            switch(Metacmd)
            {
                case 1: MC_Scared();        break;
                case 2: if(m_fHeight)
							MC_Run();
						else 
							MC_Jump();

						break;
				case 3: MC_Turnlr();		break;
				case 4: MC_Run();			break;
                case 5: NewState(STATE_EscapeEnemy,0,0,0,0); break;
            }                
        } break;


        // Make the SS_DEFAULT the default
        default:
        {
            switch(Metacmd)
            {
				case 1:	if(m_fHeight)
							MC_Crouch();
						else
							MC_Idle();

						break;
                case 2: m_nSubState = SS_EMPTY; 
						AI_Mgr::AI_STATE_FindCarrion(); return;
            }                
        } break;
    }
    
	MC_NS_Threat();

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MC_Jump
//
//	PURPOSE:	This is a modified jump for birds that puts them into
//				the air so that they can then fly(run)
//
// ----------------------------------------------------------------------- //       

void AI_Bird::MC_Jump()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vPos,m_vDown;
	static bAnimating = DFALSE;
    
    if (b_Animating == DFALSE || bAnimating == DFALSE)
    {
    	m_fTimeStart = pServerDE->GetTime();
    
		CWeapon *pW = m_pInventoryMgr->GetCurrentWeapon();
		m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_RUN[pW->GetFireType()]);

		//Calculate our desired take-off speed
    	pServerDE->GetObjectPos(m_hObject, &vPos);

        float speed = 400.0f;
		float upSpeed = 200.0f;
		
		vPos.y = m_vForward.y + upSpeed;
        vPos.x = m_vForward.x * speed;
		vPos.z = m_vForward.z * speed;

		//Turn off gravity so we can now fly
		pServerDE->SetObjectFlags(m_hObject,FLAG_VISIBLE | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_STAIRSTEP | FLAG_SHADOW);

		PlaySoundFromObject(m_hObject, "Sounds\\jump.wav", 400, SOUNDTYPE_AI, SOUNDPRIORITY_MEDIUM);
		Move(vPos, MATH_EPSILON);

		//Randomize the end flying height
		float fDist = AIShared.DistToWorld(m_hObject, m_vUp);
		m_fHeight = pServerDE->Random(fDist/2,fDist-10.0f);

        b_Animating = bAnimating = DTRUE; 
    }
    else
    {	
		VEC_INIT(m_vDown);
	
		//Get the down vector
		VEC_MULSCALAR(m_vDown, m_vUp, -1.0f);

		//Check if we are at desired altitude
		float fDist = AIShared.DistToWorld(m_hObject, m_vDown); 
        if ( fDist >= m_fHeight )
        {				
			//Set up velocity to zero so we don't keep flying up
			VEC_INIT(m_vUp);
			Move(m_vUp, MATH_EPSILON);

            b_Animating = bAnimating = DFALSE; 
            Metacmd++;
            return;
        }
    }                
    return;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MC_Crouch
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void AI_Bird::MC_Crouch()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vPos,m_vDown;
	static bAnimating = DFALSE;
    
    if (b_Animating == DFALSE || bAnimating == DFALSE)
    {
    	m_fTimeStart = pServerDE->GetTime();

	    CWeapon *pW = m_pInventoryMgr->GetCurrentWeapon();		
        m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_CROUCH[pW->GetFireType()]);

    	pServerDE->GetObjectPos(m_hObject, &vPos);

        float speed = 800.0f;
		
		vPos.y = m_vForward.y;
        vPos.x = m_vForward.x * speed;
		vPos.z = m_vForward.z * speed;

		//Turn on gravity so we can now land
		pServerDE->SetObjectFlags(m_hObject,FLAG_VISIBLE | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_SHADOW);

		Move(vPos, MATH_EPSILON);

        b_Animating = bAnimating = DTRUE; 
    }
	else
	{		
		//Get the down vector
		VEC_MULSCALAR(m_vDown, m_vUp, -1.0f);

		//Check if we are at desired altitude
		float fDist = AIShared.DistToWorld(m_hObject, m_vDown); 
        if ( fDist <= 10.0f )
        {
			m_fTimeStart = pServerDE->GetTime();
    
			m_pAnim_Sound->SetAnimation(m_hObject, m_pAnim_Sound->m_nAnim_IDLE[0]);

			//Set up velocity to zero so we don't keep flying up
			VEC_INIT(m_vUp);
			Move(m_vUp, MATH_EPSILON);

			m_fHeight = 0;

            b_Animating = bAnimating = DFALSE; 
            Metacmd++;
            return;
		}
	}

	return;
}

  */