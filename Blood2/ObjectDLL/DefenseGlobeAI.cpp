// ----------------------------------------------------------------------- //
//
// MODULE  : DefenseGlobeAI.cpp
//
// PURPOSE : DefenseGlobe AI
//
// CREATED : 12/01/97   
//
// ----------------------------------------------------------------------- //

// 9.4.4 Defense Globe

// This small device is activates a few seconds after dropped.  
// It sits stationary on turrets and rotates around, looking for targets.  
// Once it finds a target it unleashes its Pulse Cannon at them.  It only carries 50
// rounds, but will stay active until it is destroyed or all those rounds are gone. 
// Once it is out of ammo it harmlessly self-destructs.

// has Hit points.
// Will attack everything around it, even the player that droped it!

// 01/05/98 UPDATE

// 1.1.1 Defense Globe

// This small device is activates a few seconds after dropped.  It sits stationary on turrets
// and rotates around, looking for targets.  Once it finds a target it unleashes its Pulse Cannon
// at them.  It only carries 50 rounds, but will stay active until it is destroyed or all those
// rounds are gone.  Once it is out of ammo it harmlessly self-destructs.  In single player, once
// this item is dropped it is lost, as it will target the player as well.  In multi-player when
// the weapon runs out of ammo or is destroyed it will return to the player's inventory.  This item
// has 50 health, and can be destroyed.


#include "DefenseGlobeAI.h"
#include "cpp_server_de.h"


//BEGIN_CLASS(CPlayerObj)
//END_CLASS_DEFAULT(CPlayerObj, CBaseCharacter, NULL, NULL)

BEGIN_CLASS(DefenseGlobeAI)
	ADD_BASEAI_AGGREGATE()                  \
    ADD_REALPROP(SightDistance, 400.0f)     \
    ADD_REALPROP(SenseDistance, 250.0f)     \
    ADD_REALPROP(SmellDistance, 0.0f)       \
    ADD_REALPROP(ListenDistance, 0.0f)      \
    ADD_REALPROP(Mass, 17.0f)               \
    ADD_REALPROP(HitPoints, 100.0f)         \
    ADD_REALPROP(RandomHitPoints, 50.0f)    \
    ADD_REALPROP(ArmorPoints, 0.0f)         \
	ADD_STRINGPROP(AIState, "IDLE")       \
	ADD_STRINGPROP(AIBrain, "STRONG")       \
	ADD_STRINGPROP(AIWeapon, "PULSE")       \
END_CLASS_DEFAULT(DefenseGlobeAI, CBaseCharacter, NULL, NULL)
//END_CLASS_DEFAULT_FLAGS(DefenseGlobeAI, CBaseCharacter, NULL, NULL, CF_HIDDEN)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DefenseGlobeAI::DefenseGlobeAI() : CBaseCharacter(OT_MODEL)
{
	AddAggregate(&m_ai);

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"");
  	_mbscpy((unsigned char*)m_szAIBrain, (const unsigned char*)"");
  	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DefenseGlobeAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0)
				AIReadProp((ObjectCreateStruct*)pData);  // inside BaseCharacter

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


	return CBaseCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void DefenseGlobeAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\def_top.abc";
	char* pSkin = "Skins\\Enemies\\def_top.dtx";
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);

//	pStruct->m_Flags = FLAG_VISIBLE | FLAG_SOLID | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_SHADOW;	
	pStruct->m_Flags = FLAG_VISIBLE | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_SHADOW;	


}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL DefenseGlobeAI::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

// Create the Legs
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
    
	DRotation rRot;
    ROT_INIT(rRot);
    
	DVector vPos;
    pServerDE->GetObjectPos(m_hObject, &vPos);
    
    ROT_COPY(theStruct.m_Rotation, rRot);
	VEC_COPY(theStruct.m_Pos, vPos);
    
	theStruct.m_NextUpdate = 0.1f;
	pServerDE->CreateObject(pServerDE->GetClass("DefenseGlobeLegs"), &theStruct);
    

    // set the friction for object    
	pServerDE->SetFrictionCoefficient(m_hObject, 18.0);

    // Init the objects
	m_InventoryMgr.Init(m_hObject);
	m_ai.Init(m_hObject, &m_damage, &m_InventoryMgr, &m_Anim_Sound, m_szAIState, m_szAIBrain, m_szAIWeapon[0]);

    m_ai.SetListenDistance(m_fAIListenDistance);
    m_ai.SetSightDistance( m_fAISightDistance);
    m_ai.SetSenseDistance( m_fAISenseDistance);
    m_ai.SetSmellDistance( m_fAISmellDistance);
    
    
	m_damage.SetHitPoints(m_fAIHitPoints);
	m_damage.SetMaxHitPoints(100.0f);

	m_damage.SetArmorPoints(m_fAIArmorPoints);
	m_damage.SetMaxArmorPoints(200.0f);
	m_damage.SetMass(m_fAIMass);

	m_ai.SetWalkSpeed(1.8f);
	m_ai.SetRunSpeed(4.0f);

    m_Anim_Sound.SetAnimationIndexes(m_hObject);
   
	m_ai.SetDeathAniLength(5.0f);

	// Set the dims...
	DVector vDims;
// will cause problems with this small...    
//	VEC_SET(vDims, 10, 12, 10);
//  10,12,10  never hits??
//  10,15,10  sometimes it hits...

	VEC_SET(vDims, 2, 2, 2);
	pServerDE->SetObjectDims(m_hObject, &vDims);

	return DTRUE;
}



// -----------------------------------------------------------------------------------
//
// AI_DefenseGlobe
//
// -----------------------------------------------------------------------------------

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_STATE_WaitForEnemy
//
//	PURPOSE:	Idle, wait for enemy
//
// ----------------------------------------------------------------------- //
void AI_DefenseGlobe::AI_STATE_WaitForEnemy()
{

//
// Set the SubState
//
    if (m_nSubState == SS_EMPTY)
    {
        LoopCheck++;
        switch(LoopCheck)
        {
            case 1: if(AI_CheckSeeEnemy()) m_nSubState = SS_SEE_ENEMY; break;        
            default: 
            {   LoopCheck = 0; 
                if(AI_CheckSenseEnemy()) m_nSubState = SS_SENSE_ENEMY; 
            } break;
        }
    }

    if (m_nSubState == SS_EMPTY)   m_nSubState = SS_DEFAULT;


//
// Execute the SubState Meta commands
//
    switch(m_nSubState)
    {
        
        case SS_SEE_ENEMY:
        case SS_SENSE_ENEMY:
        {
            switch(Metacmd)
            {
                case 1: MC_Facetarget();  break;
                case 2: NewState(STATE_AttackEnemy,0,0,0,0); break;
            }                
        } break;
        
        // Make the SS_DEFAULT the default
        default:
        {
            switch(Metacmd)
            {
                case 1: MC_Idle();  break;
                case 2: NewState(STATE_WaitForEnemy,0,0,0,0); break;
            }                
        } break;
    
    }
    
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_STATE_AttackEnemy
//
//	PURPOSE:	Attack the Enemy
//
// ----------------------------------------------------------------------- //
void AI_DefenseGlobe::AI_STATE_AttackEnemy()
{

//
// Set the SubState
//
    if (m_nSubState == SS_EMPTY)
    {
        // Always check each one...    
        if ( AI_CheckIsThereNoTarget() )            m_nSubState = SS_NO_TARGET; 
        else if ( AI_CheckIsTargetOutofSite() )     m_nSubState = SS_TARGET_OUTOSITE;
        else if ( AI_CheckIsTargetDead() )          m_nSubState = SS_TARGET_DEAD;
        else if ( AI_CheckIsTargetNearToHit() )     m_nSubState = SS_TARGET_IS_NEAR_TO_HIT;
        
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
                case 1: MC_Turnl();  break;
                case 2: MC_Turnr();  break;
                case 3: NewState(STATE_AttackEnemy,0,0,0,0); break;
            }                
        } break;
        
        
        case SS_TARGET_OUTOSITE:
        {
            switch(Metacmd)
            {
                case 1: MC_Facetarget();    break;
                case 2: NewState(STATE_AttackEnemy,0,0,0,0); break;
            }                
        } break;
        

        case SS_TARGET_DEAD:
        {
            switch(Metacmd)
            {
                case 1: MC_Facetarget();    break;
                case 2: NewState(STATE_WaitForEnemy,0,0,0,0); break;    // reset the SubState to 0 & Metacmd to 1
            }                
        } break;


        case SS_TARGET_IS_NEAR_TO_HIT:
        {
            switch(Metacmd)
            {
                case 1: MC_Facetarget();    break;
                case 2: MC_Dg_shot_target();   break;
                case 3: NewState(STATE_AttackEnemy,0,0,0,0); break;
            }                
        } break;

        // Make the SS_DEFAULT the default
        default:
        {
            switch(Metacmd)
            {
                case 1: MC_Turnr();  break;
                case 2: MC_Delay33();  break;
                case 3: MC_Facetarget();  break;
                case 4: MC_Turnr();  break;
                case 5: MC_Turnl();  break;
                case 6: NewState(STATE_AttackEnemy,0,0,0,0); break;
            }                
        } break;
        
    }
    
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_STATE_Dying
//
//	PURPOSE:	Dying
//
// ----------------------------------------------------------------------- //
void AI_DefenseGlobe::AI_STATE_Dying()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject ) return;
     
//
// Set the SubState
//
    if (m_nSubState == SS_EMPTY)
    {
        b_Animating = DFALSE; 
   	    m_Condition = DEAD;
        m_nSubState = SS_DIE_GIB;
    }

            
//
// Execute the SubState Meta commands
//
    switch(m_nSubState)
    {

        case SS_DIE_GIB:
        {
            switch(Metacmd)
            {
                case 1: MC_Deadgib();  break;
                case 2: NewState(STATE_Inactive,0,0,0,0); break;
            }                
        } break;

    }
         
}

*/

