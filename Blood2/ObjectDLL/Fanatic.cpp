// ----------------------------------------------------------------------- //
//
// MODULE  : Fanatic.cpp
//
// PURPOSE : Fanatic - Definition
//
// CREATED : 11/11.97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "Fanatic.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(Fanatic)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIWeapon1, "BERETTA")   \
	ADD_STRINGPROP(AIWeapon2, "COMBAT")   \
	ADD_STRINGPROP(AIWeapon3, "ASSAULTRIFLE")   \
    ADD_REALPROP(Bullets, 300.0f)     \
    ADD_REALPROP(BMG, 150.0f)     \
    ADD_REALPROP(Shells, 50.0f)     \
	ADD_REALPROP(Grenades, 50.0f)	\
    ADD_REALPROP(Rockets, 25.0f)     \
    ADD_REALPROP(Flares, 40.0f)     \
    ADD_REALPROP(Cells, 200.0f)     \
    ADD_REALPROP(Charges, 120.0f)     \
    ADD_REALPROP(Fuel, 100.0f)     \
	ADD_REALPROP(Proximity_Bombs, 25.0f)	\
	ADD_VECTORPROP_VAL_FLAG(Dims, 21.0f, 38.0f, 19.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN)
END_CLASS_DEFAULT(Fanatic, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		Fanatic::m_bLoadAnims = DTRUE;
CAnim_Sound	Fanatic::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Fanatic::Fanatic() : AI_Mgr()
{
	m_fHearingDist	= 1500.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1500.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 105.0f;
	m_fJumpSpeed	= 400.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_CRAWL | FLAG_JUMP | FLAG_LIMP | FLAG_DODGE;

	m_bCabal = DTRUE;

	// [blg]
	m_fAIHitPoints   = 50;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"BERETTA");
	_mbscpy((unsigned char*)m_szAIWeapon[1], (const unsigned char*)"COMBAT");
	_mbscpy((unsigned char*)m_szAIWeapon[2], (const unsigned char*)"ASSAULTRIFLE");

	m_fAIBullets  = 300;
	m_fAIBMG	  = 50.0f;
	m_fAIShells   = 50;
	m_fAIGrenades = 50;
	m_fAIRockets  = 25;
	m_fAIFlares   = 40;
	m_fAICells    = 200;
	m_fAICharges  = 120;
	m_fAIFuel	  = 100;

	m_fStartTime = 0.0f;

	m_hAttach		= DNULL;
	m_hSmokeSource	= DNULL;
	m_bStartSmoke	= DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Fanatic::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

		case MID_UPDATE:
		{
			if(m_hSmokeSource && m_bStartSmoke)
			{
				//create the smoke
				DVector		offset;
				VEC_SET(offset, 0.0f, 0.0f, 0.0f);

				HMESSAGEWRITE hMessage = m_pServerDE->StartInstantSpecialEffectMessage(&offset);
				m_pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);
				m_pServerDE->WriteToMessageObject(hMessage, m_hSmokeSource);
				m_pServerDE->WriteToMessageVector(hMessage, &offset);
				m_pServerDE->WriteToMessageFloat(hMessage, 3.0f);
				m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SCALENUMPARTICLES);
				m_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SMOKING_2);
				m_pServerDE->WriteToMessageDWord(hMessage, 0);
				m_pServerDE->EndMessage(hMessage);

				DRotation rRot;
				ROT_INIT(rRot);

				m_bStartSmoke = DFALSE;

				m_pServerDE->CreateAttachment(m_hObject, m_hSmokeSource, "torso", &offset, &rRot, &m_hAttach);
			}

			if(m_nState == STATE_Dying && m_hAttach && m_hSmokeSource)
			{
				m_pServerDE->RemoveAttachment(m_hAttach);
				m_pServerDE->RemoveObject(m_hSmokeSource);

				m_hSmokeSource = DNULL;
				m_hAttach = DNULL;
			}

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

void Fanatic::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\Fanatic.abc";
	char* pSkin = "Skins\\Enemies\\Fanatic.dtx";
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

DBOOL Fanatic::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\fanatic");
		m_bLoadAnims = DFALSE;
	}

	m_InventoryMgr.AddDamageMultiplier(0.5f);	
    
	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::ComputeState(int nStimType)
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
			case STATE_FindAmmo:			SetNewState(m_nLastState);			break;
			case STATE_FindHealth:			SetNewState(m_nLastState);			break;
			case STATE_Escape_RunAway:		SetNewState(STATE_Idle);
			case STATE_AttackFar:
			case STATE_AttackClose:
			default:						if(m_nLastState == STATE_GuardLocation)
												SetNewState(STATE_GuardLocation);
											else if(fHeight <= m_vDims.y && m_fStimuli[HEALTH] >= 0.50f)
												SetNewState(STATE_SearchVisualTarget);
											else
												SetNewState(STATE_Idle);

											break;
		}
	}
	else
	{
		if(m_fStartTime)
		{
			SetNewState(STATE_Special1);
			Metacmd = 3;
		}
		else if(m_fStimuli[HEALTH] < 0.25f)
		{
			SetNewState(STATE_FindHealth);
			return;
		}
		else if(m_fStimuli[HEALTH] < 0.50f)
		{
			SetNewState(STATE_AttackFar);
			return;
		}
		else
		{
			if(m_fStimuli[SIGHT] <= 0.5f || fHeight > m_vDims.y)
				SetNewState(STATE_AttackFar);
			else
				SetNewState(STATE_AttackClose);
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_AttackClose()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_BestWeapon();	break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.33) || m_nCurMetacmd == MC_FIRE_STAND
						|| m_nCurMetacmd == MC_FIRE_CROUCH || fHeight > m_vDims.y)
					{
						if(m_nDodgeFlags & FORWARD || m_nDodgeFlags & BACKWARD || m_fStimuli[HEALTH] < 0.5f 
							|| m_nCurMetacmd == MC_FIRE_CROUCH || (fHeight > m_vDims.y && !bAbove))
						{
							MC_Fire_Crouch();
						}
						else
						{
							MC_Fire_Stand();
						}
					}
					else
						Metacmd++;
					
					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.33) && m_fStimuli[SIGHT] < (m_fSeeingDist * 0.5))
					{
						MC_FaceTarget();
						Metacmd--;
						
						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_CROUCH)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
							{
								if(m_nDodgeFlags & FORWARD || m_nDodgeFlags & BACKWARD
									|| m_fStimuli[HEALTH] < 0.5f)
									MC_Fire_Crawl();
								else
									MC_Fire_Run();
							}
						}
						else
						{
							MC_Fire_Crouch();
						}
					}
					else
						Metacmd++;

					break;
		case 5:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_AttackFar()
{
	DFLOAT fHeight = (DFLOAT)fabs(m_vTargetPos.y - m_MoveObj.GetPos().y);

	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		if(fHeight <= m_vDims.y)
			SetNewState(STATE_SearchVisualTarget);
		else
			ComputeState();

		return;
	}

	m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
	DBOOL bAbove = (m_vTargetPos.y - m_MoveObj.GetPos().y) > 0;

	switch(Metacmd)
	{
		case 1:		MC_FaceTarget();	break;
		case 2:		MC_BestWeapon();	break;
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND 
						|| m_nCurMetacmd == MC_FIRE_CROUCH || fHeight > m_vDims.y)
					{
						if((fHeight > m_vDims.y && !bAbove) || m_nCurMetacmd == MC_FIRE_CROUCH
							|| m_fStimuli[HEALTH] < 0.5f)
						{
							MC_Fire_Crouch();
						}
						else
						{
							MC_Fire_Stand();
						}
					}
					else
						Metacmd++;

					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.75))
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_CROUCH)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
								MC_Fire_Run();
						}
						else
						{
							MC_Fire_Crouch();
						}
					}
					else
						Metacmd++;

					break;
		case 5:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_Escape
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_Escape_RunAway()
{
	int nStimType = ComputeStimuli();

	if(nStimType > 0)
	{
		SetNewState(STATE_Special1);
		return;
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist, FIND_VISIBLE | FIND_AVOID_TARGET);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						DVector vPos;
						VEC_SET(vPos, m_vTrackObjPos.x, m_MoveObj.GetPos().y, m_vTrackObjPos.z);

						if(VEC_DIST(vPos, m_MoveObj.GetPos()) <= fDim)
							SetNewState(STATE_Special1);
						else
						{
							MC_FacePos(m_vTrackObjPos);

							if(!m_MoveObj.CalculatePath(m_vTrackObjPos))
								SetNewState(STATE_Special1);
						}
					}
					else
						SetNewState(STATE_Special1);
					
					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_RunAway);

					break;
		}
		case 3:		
		{
					DVector vPoint = *m_MoveObj.GetNextPathPoint();
					vPoint.y = m_MoveObj.GetPos().y;

					if(VEC_DIST(vPoint, m_MoveObj.GetPos()) <= fDim/2)
					{
						if(!m_MoveObj.MoveToNextPathPoint())
						{
							MC_FacePos(m_vTargetPos);
						}
						else
						{
							Metacmd = 2;
						}
					}
					else
						MC_Run();

					break;
		}
		case 4:		ComputeState();			break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_Escape
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_Escape_Hide()
{
	switch(Metacmd)
	{
		case 1:		m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vDestPos);
						SetNewState(STATE_RunToPos);
					}
					else
						SetNewState(STATE_AttackClose);
					
					break;
	}

	return;

}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_Dodge
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_Dodge()
{
	switch(Metacmd)
	{
		case 1:		m_nDodgeFlags = CalculateDodge(m_vTrackObjPos);
					Metacmd++;		break;
		case 2:		if(m_nDodgeFlags & ROLL)
					{
						if(m_nDodgeFlags & RIGHT)
							MC_Roll_Right();
						else if(m_nDodgeFlags & LEFT)
							MC_Roll_Left();
						else if(m_nDodgeFlags & FORWARD)
							MC_Roll_Forward();
						else
							MC_Roll_Backward();
					}
					else
					{
						Metacmd++;
					}

					break;
		case 3:		ComputeState();	break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_Special
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_Special1()
{
	switch(Metacmd)
	{
		case 1:		MC_Special();		
					break;
		case 2:		
		{
					m_pServerDE->SetModelNodeHideStatus(m_hObject, "torso_extra1", DTRUE);
			
					m_damage.SetHitPoints(50.0f);
					m_damage.SetArmorPoints(50.0f);
					m_fStartTime = m_pServerDE->GetTime();

					//create the smoke object
					ObjectCreateStruct ocStruct;
					INIT_OBJECTCREATESTRUCT(ocStruct);

					ocStruct.m_ObjectType = OT_NORMAL;
					ocStruct.m_NextUpdate = 0.01f;
					g_pServerDE->GetModelNodeTransform(m_hObject, "torso_extra1",&ocStruct.m_Pos,&ocStruct.m_Rotation);
					ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
					
					HCLASS hClass = g_pServerDE->GetClass("BaseClass");
					BaseClass* pObj = g_pServerDE->CreateObject(hClass, &ocStruct);

					if(pObj)
					{
						m_hSmokeSource = pObj->m_hObject;
					}

					Metacmd++;			
					break;
		}
		case 3:		m_fStimuli[SIGHT] = VEC_DIST(m_MoveObj.GetPos(),m_vTargetPos);
			
					if(m_fStimuli[SIGHT] <= 100.0f || 
						m_pServerDE->GetTime() - m_fStartTime >= 30.0f)
					{
						Metacmd++;
					}
					else
					{
						MC_FaceTarget();
						Metacmd--;

						MC_Run();
					}
					break;
		case 4:		
		{
					ObjectCreateStruct ocStruct;
					INIT_OBJECTCREATESTRUCT(ocStruct);

					ROT_COPY(ocStruct.m_Rotation, m_MoveObj.GetRotation());
					VEC_COPY(ocStruct.m_Pos, m_MoveObj.GetPos());

					CProjectile	*pProject = NULL;

					HCLASS hClass = m_pServerDE->GetClass("CHowitzerShell");
					DFLOAT fVelocity = 250.0f;
					int nRadius = 200;

					if (hClass)
					{
						DVector vDown;
						VEC_MULSCALAR(vDown, m_MoveObj.GetUpVector(), -1.0f);

						if (pProject = (CProjectile*)m_pServerDE->CreateObject(hClass, &ocStruct))
							pProject->Setup(&vDown,	WEAP_HOWITZER, 50.0f, fVelocity, nRadius, m_hObject);
					}

					CreateGibs(m_MoveObj.GetUpVector(), ((int)m_damage.GetMass())>>5, 
								DAMAGE_TYPE_EXPLODE, 50.0f);

					if(m_hAttach && m_hSmokeSource)
					{
						m_pServerDE->RemoveAttachment(m_hAttach);
						m_pServerDE->RemoveObject(m_hSmokeSource);

						m_hSmokeSource = DNULL;
						m_hAttach = DNULL;
					}

					m_bRemoveMe = DTRUE;
					Metacmd++;

					break;
		}
		case 5:		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: Fanatic::AI_STATE_Dying
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void Fanatic::AI_STATE_Dying()
{
	if(m_fStartTime)
	{
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ROT_COPY(ocStruct.m_Rotation, m_MoveObj.GetRotation());
		VEC_COPY(ocStruct.m_Pos, m_MoveObj.GetPos());

		CProjectile	*pProject = NULL;

		HCLASS hClass = m_pServerDE->GetClass("CHowitzerShell");
		DFLOAT fVelocity = 250.0f;
		int nRadius = 200;

		if (hClass)
		{
			DVector vDown;
			VEC_MULSCALAR(vDown, m_MoveObj.GetUpVector(), -1.0f);

			if (pProject = (CProjectile*)m_pServerDE->CreateObject(hClass, &ocStruct))
				pProject->Setup(&vDown,	WEAP_HOWITZER, 50.0f, fVelocity, nRadius, m_hObject);
		}

		CreateGibs(m_MoveObj.GetUpVector(), ((int)m_damage.GetMass())>>5, 
					DAMAGE_TYPE_EXPLODE, 50.0f);

		if(m_hAttach && m_hSmokeSource)
		{
			m_pServerDE->RemoveAttachment(m_hAttach);
			m_pServerDE->RemoveObject(m_hSmokeSource);

			m_hSmokeSource = DNULL;
			m_hAttach = DNULL;
		}

		m_bRemoveMe = DTRUE;
	}
	else
		AI_Mgr::AI_STATE_Dying();

	return;
}

void Fanatic::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);
}

void Fanatic::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_fStartTime = pServerDE->ReadFromMessageFloat(hRead);

	if(m_fStartTime)
	{
		//create the smoke object
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_ObjectType = OT_NORMAL;
		ocStruct.m_NextUpdate = 0.01f;
		g_pServerDE->GetModelNodeTransform(m_hObject, "torso_extra1",&ocStruct.m_Pos,&ocStruct.m_Rotation);
		ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
		
		HCLASS hClass = g_pServerDE->GetClass("BaseClass");
		BaseClass* pObj = g_pServerDE->CreateObject(hClass, &ocStruct);

		if(pObj)
		{
			m_hSmokeSource = pObj->m_hObject;
		}

		m_bStartSmoke = DTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void Fanatic::CacheFiles()
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

	SetCacheDirectory("sounds\\enemies\\fanatic");

	CacheSoundFileRange("death", 1, 3);
	CacheSoundFileRange("fear", 1, 3);
	CacheSoundFileRange("fire", 1, 3);
	CacheSoundFileRange("idle", 1, 3);
	CacheSoundFileRange("pain", 1, 3);
	CacheSoundFileRange("scream", 1, 3);
	CacheSoundFileRange("spot", 1, 3);

	CacheSoundFile("footstep");
}

