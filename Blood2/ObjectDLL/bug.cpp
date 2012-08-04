// ----------------------------------------------------------------------- //
//
// MODULE  : Bug.cpp
//
// PURPOSE : Bug - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "bug.h"
#include "cpp_server_de.h"

BEGIN_CLASS(BugAI)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(SightDistance, 500.0f) \
    ADD_REALPROP(SenseDistance, 500.0f) \
    ADD_REALPROP(SmellDistance, 500.0f)   \
    ADD_REALPROP(Mass, 1.0f)           \
    ADD_REALPROP(HitPoints, 1.0f)     \
    ADD_REALPROP(RandomHitPoints, 0.0f) \
END_CLASS_DEFAULT(BugAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		BugAI::m_bLoadAnims = DTRUE;
CAnim_Sound	BugAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BugAI::BugAI() : AI_Mgr()
{
	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE" );
  	_mbscpy((unsigned char*)m_szAIBrain, (const unsigned char*)"WEAK" );
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

DDWORD BugAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

		case MID_UPDATE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) return 0;

			//gotta keep the bug stuck to whatever surface it is on
			DRotation rRot;
			DVector vU,vR,vF,vVel,vPos;

			pServerDE->GetObjectPos(m_hObject,&vPos);
			pServerDE->GetObjectRotation(m_hObject,&rRot);
			pServerDE->GetRotationVectors(&rRot,&vU,&vR,&vF);
			VEC_MULSCALAR(vVel,vU,-15.0f);

			Move(vVel, MATH_EPSILON);

			//check if there is something below, if not rotate
			IntersectQuery IQuery;
			IntersectInfo ii;
	
			IQuery.m_Flags	  = INTERSECT_OBJECTS;
			IQuery.m_FilterFn = DNULL;

			DVector vTemp;
			VEC_MULSCALAR(vTemp,vU,-2.0f);
			VEC_COPY(IQuery.m_From,vPos);
			VEC_ADD(IQuery.m_To,IQuery.m_From,vTemp);

			if(pServerDE->IntersectSegment(&IQuery, &ii))
			{
				DRotation rRot;
				DVector vDown;

				VEC_MULSCALAR(vDown,vU,-1.0f);
				pServerDE->AlignRotation(&rRot,&vDown,&vF);

				pServerDE->SetObjectRotation(m_hObject,&rRot);
			}
		}
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

void BugAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\bug.abc";
	char* pSkin = "Skins\\Enemies\\bug.dtx";
	
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);

	pStruct->m_Flags = FLAG_VISIBLE | FLAG_SOLID | FLAG_TOUCH_NOTIFY | FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_MODELKEYS;	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL BugAI::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\bug");
		m_bLoadAnims = DFALSE;
	}

	//randomize the size/weight
	DFLOAT fScale = pServerDE->Random(-0.05f, 0.1f);
	VEC_SET(m_vScale,1.0f + fScale, 1.0f, 1.0f + fScale);

    // Init the objects
	m_InventoryMgr.Init(m_hObject);
    
	//Set hit points
	m_damage.Init(m_hObject);
	m_damage.SetHitPoints(m_fAIHitPoints + pServerDE->Random(1,m_fAIRandomHP));
	m_damage.SetMaxHitPoints(m_damage.GetHitPoints());

	//Set armor point
	m_damage.SetArmorPoints(m_fAIArmorPoints);
	m_damage.SetMaxArmorPoints(200.0f);
	m_damage.SetMass(m_fAIMass);

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BugAI::MC_Run
// DESCRIPTION	: Run the run animation
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BugAI::MC_Run()
{
   if (m_bAnimating == DFALSE || m_nCurMetacmd != MC_RUN)
    {
		DBOOL bRet = DFALSE;

    	m_fTimeStart = m_pServerDE->GetTime();

		Move(m_MoveObj.GetForwardVector(),m_fRunSpeed);
        
        m_bAnimating = DTRUE; 
		m_nCurMetacmd = MC_RUN;
    }
    else
    {   
		//Check for obstruction; otherwise continue on
		if(CheckObstructed(m_MoveObj.GetForwardVector(), m_fRunSpeed))
		{
			IntersectQuery IQuery;
			IntersectInfo ii;
	
			IQuery.m_Flags	  = INTERSECT_OBJECTS;
			IQuery.m_FilterFn = DNULL;

			DVector vTemp;
			VEC_MULSCALAR(vTemp,m_MoveObj.GetForwardVector(),m_fRunSpeed);
			VEC_COPY(IQuery.m_From,m_MoveObj.GetPos());
			VEC_ADD(IQuery.m_To,IQuery.m_From,vTemp);

			if(m_pServerDE->IntersectSegment(&IQuery, &ii))
			{
				//climb the object
				DRotation rRot;
				m_pServerDE->AlignRotation(&rRot,&m_MoveObj.GetUpVector(),&ii.m_Plane.m_Normal);

				m_pServerDE->SetObjectRotation(m_hObject,&rRot);

				m_bAnimating = DFALSE; 
				Metacmd++;

				return;
			}
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
// ROUTINE		: BugAI::AI_STATE_Escape_RunAway
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BugAI::AI_STATE_Escape_RunAway()
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
					_mbscpy((unsigned char*)szSound, (const unsigned char*)SOUND_FEAR);
					m_pAnim_Sound->GetSoundPath(szSound,m_pServerDE->IntRandom(1,NUM_BUG_FEAR));

					PlayAISound(szSound, 1000.0f);					

					Metacmd++;			break;
		case 2:		MC_FaceTarget();							break;
		case 3:		m_fRadsLeft = 3.14f;	Metacmd++;			break;
		case 4:		MC_Turn();									break;
		case 5:		MC_Run();									break;
		case 6:		ComputeState();								break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: BugAI::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void BugAI::AI_STATE_Idle()
{
	if(ComputeStimuli())
	{
		ComputeState();
		m_pServerDE->SetNextUpdate(m_hObject,0.1f);
		return;
	}
	else
		m_pServerDE->SetNextUpdate(m_hObject,0.2f);

	switch(Metacmd)
	{
		case 1:		char szSound[256];
					_mbscpy((unsigned char*)szSound, (const unsigned char*)SOUND_IDLE);
					m_pAnim_Sound->GetSoundPath(szSound,m_pServerDE->IntRandom(1,NUM_BUG_IDLE));

					PlayAISound(szSound, 1000.0f);					

					Metacmd++;			break;
		case 2:		MC_Idle();						break;
		case 3:		m_fRadsLeft = m_pServerDE->Random(-1.56f,1.56f);	
					Metacmd++;						break;
		case 4:		MC_Turn();						break;
		case 5:		MC_Run();						break;
		case 6:		SetNewState(STATE_Idle);		break;
	}

	return;
}