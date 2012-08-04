// ----------------------------------------------------------------------- //
//
// MODULE  : CultistAI.cpp
//
// PURPOSE : CultistAI - Definition
//
// CREATED : 11/11.97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "CultistAI.h"
#include "cpp_server_de.h"

BEGIN_CLASS(CultistAI)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIWeapon1, "BERETTA")   \
    ADD_REALPROP(Bullets, 150.0f)     \
    ADD_REALPROP(Flares, 20.0f)     \
	ADD_BOOLPROP(Male, DTRUE) \
	ADD_VECTORPROP_VAL_FLAG(Dims, 19.0f, 41.0f, 14.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN) \
	ADD_BOOLPROP(ClownSkin, DFALSE) \
	ADD_BOOLPROP(RobeSkin, DFALSE)
END_CLASS_DEFAULT(CultistAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		CultistAI::m_bFemaleAnims = DTRUE;
CAnim_Sound	CultistAI::m_Female_Anim_Sound;
DBOOL		CultistAI::m_bMaleAnims = DTRUE;
CAnim_Sound	CultistAI::m_Male_Anim_Sound;

#ifdef _ADD_ON
DBOOL		CultistAI::m_bRobeAnims = DTRUE;
CAnim_Sound	CultistAI::m_Robe_Anim_Sound;
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CultistAI::CultistAI() : AI_Mgr()
{
	m_fHearingDist	= 1500.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1500.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 105.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_CRAWL | FLAG_JUMP | FLAG_LIMP | FLAG_DODGE | FLAG_ALWAYSRECOIL;

	m_bMale = DTRUE;
	m_bCabal = DTRUE;

	m_bClownSkin = DFALSE;
	m_bRobeSkin  = DFALSE;

	// [blg]
	m_fAIHitPoints   = 30;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;
	m_fAIBullets     = 150;
	m_fAIShells      = 60;
	m_fAIFlares      = 20;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"BERETTA");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CultistAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
				GetServerDE()->GetPropBool("Male", &m_bMale);
				GetServerDE()->GetPropBool("ClownSkin", &m_bClownSkin);
				GetServerDE()->GetPropBool("RobeSkin", &m_bRobeSkin);
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

void CultistAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename= DNULL;
	char* pSkin = DNULL;


	if (m_bClownSkin)
	{
		pFilename = "Models\\Enemies\\m_Cultist1.abc";
		m_bMale   = DTRUE;

		switch(pServerDE->IntRandom(1,3))
		{
			case 1:		pSkin = "Skins_ao\\Enemies_ao\\m_Cultist4.dtx";	break;
			case 2:		pSkin = "Skins_ao\\Enemies_ao\\m_Cultist5.dtx";	break;
			case 3:		pSkin = "Skins_ao\\Enemies_ao\\m_Cultist6.dtx";	break;
		}
	}
	else if (m_bRobeSkin)
	{
		pFilename = "Models_ao\\Enemies_ao\\oldschool.abc";
		pSkin     = "Skins_ao\\Enemies_ao\\oldschool.dtx";
		m_bMale   = DTRUE;
	}
	else if (m_bMale)
	{
		pFilename = "Models\\Enemies\\m_Cultist1.abc";

		switch(pServerDE->IntRandom(1,3))
		{
			case 1:		pSkin = "Skins\\Enemies\\m_Cultist1.dtx";	break;
			case 2:		pSkin = "Skins\\Enemies\\m_Cultist2.dtx";	break;
			case 3:		pSkin = "Skins\\Enemies\\m_Cultist3.dtx";	break;
		}
	}
	else
	{
		pFilename = "Models\\Enemies\\f_Cultist1.abc";

		switch(pServerDE->IntRandom(1,3))
		{
			case 1:		pSkin = "Skins\\Enemies\\f_Cultist1.dtx";	break;
			case 2:		pSkin = "Skins\\Enemies\\f_Cultist2.dtx";	break;
			case 3:		pSkin = "Skins\\Enemies\\f_Cultist3.dtx";	break;
		}
	}

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

DBOOL CultistAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

#ifndef _ADD_ON
	if (m_bRobeSkin) {
		m_bRobeSkin = DFALSE;
		m_bMale = DTRUE;
	}
#endif

	//Load up animation indexes if first model instance
	if((m_bMale || m_bClownSkin) && m_bMaleAnims)
	{
		m_Male_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Male_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Male_Anim_Sound.SetSoundRoot("sounds\\enemies\\m_cultist");
		m_bMaleAnims = DFALSE;
	}
#ifdef _ADD_ON
	if (m_bRobeSkin && m_bRobeAnims)
	{
		m_Robe_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Robe_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Robe_Anim_Sound.SetSoundRoot("sounds_ao\\enemies\\RetroCultist");
		m_bRobeAnims = DFALSE;
	}
#endif
	if(!m_bMale && m_bFemaleAnims && !m_bRobeSkin)
	{
		m_Female_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Female_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Female_Anim_Sound.SetSoundRoot("sounds\\enemies\\f_cultist");
		m_bFemaleAnims = DFALSE;
	}

	m_InventoryMgr.AddDamageMultiplier(0.5f);	

	if((m_bMale || m_bClownSkin) && !m_bRobeSkin)
		AI_Mgr::InitStatics(&m_Male_Anim_Sound);
#ifdef _ADD_ON
	else if (m_bRobeSkin)
		AI_Mgr::InitStatics(&m_Robe_Anim_Sound);
#endif
	else
		AI_Mgr::InitStatics(&m_Female_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CultistAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::ComputeState(int nStimType)
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
		if(m_fStimuli[HEALTH] < 0.25f)
		{
			SetNewState(STATE_Escape_RunAway);
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
// ROUTINE		: CultistAI::STATE_AttackClose
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::AI_STATE_AttackClose()
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
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.25) || m_nCurMetacmd == MC_FIRE_STAND || fHeight > m_vDims.y)
					{
						MC_Fire_Stand();
					}
					else
						Metacmd++;

					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.25) && m_fStimuli[SIGHT] < (m_fSeeingDist * 0.5))
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_STAND)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
								MC_Run();
						}
						else
						{
							MC_Fire_Stand();
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
// ROUTINE		: CultistAI::AI_STATE_AttackFar
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::AI_STATE_AttackFar()
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
		case 3:		if(m_fStimuli[SIGHT] <= (m_fSeeingDist * 0.75) || m_nCurMetacmd == MC_FIRE_STAND || fHeight > m_vDims.y)
					{
						MC_Fire_Stand();
					}
					else
						Metacmd++;

					break;
		case 4:		if(m_fStimuli[SIGHT] > (m_fSeeingDist * 0.75))
					{
						MC_FaceTarget();
						Metacmd--;

						if(!IsLedge(m_MoveObj.GetForwardVector()) && fHeight <= m_vDims.y
							&& m_nCurMetacmd != MC_FIRE_STAND
							)
						{
							if(m_nInjuredLeg)
								MC_Walk();
							else
								MC_Fire_Run();
						}
						else
						{
							MC_Fire_Stand();
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
// ROUTINE		: CultistAI::AI_STATE_Escape
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::AI_STATE_Escape_RunAway()
{
	if(Metacmd > 1)
	{
		int nStimType = ComputeStimuli();

		if(nStimType > 0)
		{
			ComputeState(nStimType);
			return;
		}
	}

	DFLOAT fDim = (DFLOAT)sqrt((m_vDims.x * m_vDims.x) + (m_vDims.z * m_vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist, FIND_VISIBLE | FIND_AVOID_TARGET);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						MC_FacePos(m_vTrackObjPos);

						if(!m_MoveObj.CalculatePath(m_vTrackObjPos))
							SetNewState(STATE_Escape_Hide);
					}
					else
						SetNewState(STATE_AttackFar);
					
					break;
		case 2:	
		{
					DVector* pvPos = m_MoveObj.GetNextPathPoint();
					
					if(pvPos)
						MC_FacePos(*pvPos);	
					else
						SetNewState(STATE_Escape_Hide);

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
// ROUTINE		: CultistAI::AI_STATE_Escape_Hide
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::AI_STATE_Escape_Hide()
{
	if(m_hTarget == DNULL)
	{
		ComputeState();
		return;
	}

	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_Taunt_Beg();		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CultistAI::AI_STATE_Dodge
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CultistAI::AI_STATE_Dodge()
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
						else
							Metacmd++;
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

void CultistAI::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	m_pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bMale);
}

void CultistAI::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	// Load our male flag
	m_bMale = (DBOOL)m_pServerDE->ReadFromMessageByte(hRead);

	// Figure out other flags by the model filenames

#ifdef _ADDON
	char sModel[256] = { "" };
	char sSkin[256]  = { "" };

	m_pServerDE->GetModelFilenames(m_hObject, sModel, 255, sSkin, 255);

	strupr(sModel);
	strupr(sSkin);

	if (strstr(sSkin, "M_CULTIST4")) m_bClownSkin = DTRUE;
	if (strstr(sSkin, "M_CULTIST5")) m_bClownSkin = DTRUE;
	if (strstr(sSkin, "M_CULTIST6")) m_bClownSkin = DTRUE;

	if (strstr(sSkin, "OLDSCHOOL")) m_bRobeSkin = DTRUE;
#endif

	//Load up animation indexes if first model instance
	if((m_bMale || m_bClownSkin) && m_bMaleAnims)
	{
		m_Male_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Male_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Male_Anim_Sound.SetSoundRoot("sounds\\enemies\\m_cultist");
		m_bMaleAnims = DFALSE;
	}
#ifdef _ADD_ON
	if (m_bRobeSkin && m_bRobeAnims)
	{
		m_Robe_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Robe_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Robe_Anim_Sound.SetSoundRoot("sounds_ao\\enemies\\RetroCultist");
		m_bRobeAnims = DFALSE;
	}
#endif
	if((!m_bMale && !m_bClownSkin && !m_bRobeSkin) && m_bFemaleAnims)
	{
		m_Female_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Female_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Female_Anim_Sound.SetSoundRoot("sounds\\enemies\\f_cultist");
		m_bFemaleAnims = DFALSE;
	}

	if((m_bMale || m_bClownSkin) && !m_bRobeSkin)
		AI_Mgr::InitStatics(&m_Male_Anim_Sound);
#ifdef _ADD_ON
	else if (m_bRobeSkin)
		AI_Mgr::InitStatics(&m_Robe_Anim_Sound);
#endif
	else
		AI_Mgr::InitStatics(&m_Female_Anim_Sound);

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void CultistAI::CacheFiles()
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

#ifdef _ADDON
	if (m_bRobeSkin)
	{
		SetCacheDirectory("sounds_ao\\enemies\\retrocultist");

		CacheSoundFileRange("death", 1, 3);
		CacheSoundFileRange("kill", 1, 3);
		CacheSoundFileRange("idle", 1, 3);
		CacheSoundFileRange("pain", 1, 3);
		CacheSoundFileRange("spot", 1, 3);
		CacheSoundFileRange("attack", 1, 3);
		CacheSoundFileRange("taunt", 1, 3);
	}
	else
#endif

	if (m_bMale)
	{
		SetCacheDirectory("sounds\\enemies\\m_cultist");

		CacheSoundFileRange("death", 1, 3);
		CacheSoundFileRange("fear", 1, 3);
		CacheSoundFileRange("idle", 1, 3);
		CacheSoundFileRange("pain", 1, 3);
		CacheSoundFileRange("spot", 1, 3);
		CacheSoundFileRange("attack", 1, 3);
		CacheSoundFileRange("taunt", 1, 3);
		CacheSoundFileRange("anger", 1, 3);
	}
	else
	{
		SetCacheDirectory("sounds\\enemies\\f_cultist");

		CacheSoundFileRange("death", 1, 3);
		CacheSoundFileRange("fear", 1, 3);
		CacheSoundFileRange("idle", 1, 3);
		CacheSoundFileRange("pain", 1, 3);
		CacheSoundFileRange("spot", 1, 3);
		CacheSoundFileRange("attack", 1, 3);
		CacheSoundFileRange("taunt", 1, 3);
		CacheSoundFileRange("anger", 1, 3);
	}
}

void CultistAI::ResetStatics()
{
	m_bMaleAnims = DTRUE;
}
