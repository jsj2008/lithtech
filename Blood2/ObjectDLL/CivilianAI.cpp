// ----------------------------------------------------------------------- //
//
// MODULE  : CivilianAI.cpp
//
// PURPOSE : CivilianAI - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

// 10.4 Incidental Creatures
// This section covers benign creatures whose only purpose 
// in the game is atmosphere or target practice.

// 1.1.310.4.3 Civilians
// Target practice.  We should be able to get away with creating two models
// (male and female) and doing 2-3 skins for each to cover a large variety of Civilians.



#include <stdio.h>
#include "CivilianAI.h"
#include "cpp_server_de.h"

BEGIN_CLASS(CivilianAI)
    ADD_REALPROP(RandomHitPoints, 0.0f) \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIWeapon1, "NONE")   \
	ADD_BOOLPROP(Male, DTRUE) \
	ADD_BOOLPROP(LabTech, DFALSE) \
	ADD_BOOLPROP(Scared, DFALSE) \
	ADD_VECTORPROP_VAL_FLAG(Dims, 21.0f, 37.0f, 21.0f, PF_DIMS | PF_LOCALDIMS | PF_HIDDEN) \
	ADD_BOOLPROP(SororitySkin, DFALSE)
END_CLASS_DEFAULT(CivilianAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		CivilianAI::m_bFemaleAnims = DTRUE;
CAnim_Sound	CivilianAI::m_Female_Anim_Sound;
DBOOL		CivilianAI::m_bMaleAnims = DTRUE;
CAnim_Sound	CivilianAI::m_Male_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CivilianAI::CivilianAI() : AI_Mgr()
{
	m_fHearingDist	= 2000.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 2000.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = FLAG_ALWAYSRECOIL;

	m_bMale = DTRUE;
	m_bCabal = DTRUE;

	// [blg]
	m_fAIHitPoints   = 30;
	m_fAIRandomHP    = 0;

	m_fAICharges	 = 100.0f;		//for Coleman's wacky scientist

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");

	m_bMale = DTRUE;
	m_bLabTech = DFALSE;
	m_bScared = DFALSE;
	m_bSororitySkin = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CivilianAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
				GetServerDE()->GetPropBool("LabTech", &m_bLabTech);
				GetServerDE()->GetPropBool("Scared", &m_bScared);
				GetServerDE()->GetPropBool("SororitySkin", &m_bSororitySkin);
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
// ROUTINE		: CivilianAI::ObjectMessageFn
// DESCRIPTION	: Handle inter-object messages
// RETURN TYPE	: DDWORD 
// PARAMS		: HOBJECT hSender
// PARAMS		: DDWORD messageID
// PARAMS		: HMESSAGEREAD hRead
// ----------------------------------------------------------------------- //

DDWORD CivilianAI::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HSTRING hMsg = m_pServerDE->ReadFromMessageHString(hRead);
			char *pCommand = m_pServerDE->GetStringData(hMsg);

			/* 01/15/1999 Be VERY sure we should run this. -- Loki */
			if ((m_nState != STATE_Escape_Hide) && 
				(m_nState != STATE_Script) && 
				(m_nState != STATE_Special1) &&
				(m_nState != STATE_Special2) &&
				(m_nState != STATE_Special3) &&
				(m_nState != STATE_Special4) &&
				(m_nState != STATE_Special5) &&
				(m_nState != STATE_Special6) &&
				(m_nState != STATE_Special7) &&
				(m_nState != STATE_Special8) &&
				(m_nState != STATE_Special9) &&
				(_mbscmp((const unsigned char*)pCommand, (const unsigned char*)"TRIGGER") == 0))
			{
				m_hTarget = hSender;

				if(m_bLabTech)
					SetNewState(STATE_Escape_Hide);
				else
					SetNewState(STATE_Special1);
			}

			g_pServerDE->FreeString( hMsg );

			break;
		}

		case MID_DAMAGE:
		{
			AI_Mgr::ObjectMessageFn(hSender, messageID, hRead);
			SetNewState(STATE_Escape_RunAway);

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

void CivilianAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename= DNULL;
	char* pSkin = DNULL;

	DFLOAT fDetail = 4;	//m_pServerDE->GetVarValueFloat(m_pServerDE->GetGameConVar("GlobalDetail"));

	if(StateStrToInt(m_szAIState) == STATE_Special8)
	{
		pFilename = "Models\\Enemies\\m_civilian1.abc";
		pSkin = "Skins\\Enemies\\m_civilian4.dtx";
	}
	else if (m_bSororitySkin)
	{
		pFilename = "Models\\Enemies\\f_civilian1.abc";
		pSkin     = "Skins_ao\\Enemies_ao\\f_civilian3.dtx";
	}
	else if(m_bLabTech)
	{
		pFilename = "Models\\Enemies\\labtech.abc";

		if(fDetail <= 0)
		{
			pSkin = "Skins\\Enemies\\labtech1.dtx";
		}
		else
		{
			switch(pServerDE->IntRandom(1,3))
			{
				case 1:		pSkin = "Skins\\Enemies\\labtech1.dtx";	break;
				case 2:		pSkin = "Skins\\Enemies\\labtech2.dtx";	break;
				case 3:		pSkin = "Skins\\Enemies\\labtech3.dtx";	break;
				default:	pSkin = "Skins\\Enemies\\labtech1.dtx";	break;
			}
		}
	}
	else if(m_bMale)
	{
		pFilename = "Models\\Enemies\\m_civilian1.abc";

		if(fDetail <= 0)
		{
			pSkin = "Skins\\Enemies\\m_civilian1.dtx";
		}
		else
		{
			switch(pServerDE->IntRandom(1,3))
			{
				case 1:		pSkin = "Skins\\Enemies\\m_civilian1.dtx";	break;
				case 2:		pSkin = "Skins\\Enemies\\m_civilian2.dtx";	break;
				case 3:		pSkin = "Skins\\Enemies\\m_civilian3.dtx";	break;
				default:	pSkin = "Skins\\Enemies\\m_civilian1.dtx";	break;
			}
		}
	}
	else
	{
		pFilename = "Models\\Enemies\\f_civilian1.abc";

		if(fDetail <= 0)
		{
			pSkin = "Skins\\Enemies\\f_civilian1.dtx";
		}
		else
		{
			switch(pServerDE->IntRandom(1,3))
			{
				case 1:		pSkin = "Skins\\Enemies\\f_civilian1.dtx";	break;
				case 2:		pSkin = "Skins\\Enemies\\f_civilian1.dtx";	break;
				case 3:		pSkin = "Skins\\Enemies\\f_civilian1.dtx";	break;
				default:	pSkin = "Skins\\Enemies\\f_civilian1.dtx";	break;
			}
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

DBOOL CivilianAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
	if(m_bMale || m_bLabTech && m_bMaleAnims)
	{
		m_Male_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Male_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Male_Anim_Sound.SetSoundRoot("sounds\\enemies\\civilian");
		m_bMaleAnims = DFALSE;
	}

	if(!m_bMale && m_bFemaleAnims)
	{
		m_Female_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Female_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Female_Anim_Sound.SetSoundRoot("sounds\\enemies\\f_civ");
		m_bFemaleAnims = DFALSE;
	}

	if(m_bMale || m_bLabTech)
		AI_Mgr::InitStatics(&m_Male_Anim_Sound);
	else
		AI_Mgr::InitStatics(&m_Female_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(!nStim)
	{
		SetNewState(StateStrToInt(m_szAIState));
	}
	else
	{
		if(m_nState == STATE_Escape_Hide)
		{
			SetNewState(STATE_Escape_Hide);
		}
		else if(m_bScared || (m_fStimuli[HEALTH] < 1.0f))
		{
			m_bScared = DTRUE;
			SetNewState(STATE_Escape_RunAway);
		}
		else
			SetNewState(StateStrToInt(m_szAIState));
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Escape_RunAway
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Escape_RunAway()
{
	DVector vDims;
	m_pServerDE->GetObjectDims(m_hObject,&vDims);
	DFLOAT fDim = (DFLOAT)sqrt((vDims.x * vDims.x) + (vDims.z * vDims.z)) + 0.1f;

	switch(Metacmd)
	{
		case 1:		
		{
					m_hTrackObject = FindObjectInRadius(m_pServerDE->GetClass("ExitHint"), m_fSeeingDist, FIND_VISIBLE);

					if(m_hTrackObject)
					{
						m_pServerDE->GetObjectPos(m_hTrackObject,&m_vTrackObjPos);

						DFLOAT fDist = VEC_DIST(m_vTrackObjPos, m_MoveObj.GetPos());
						
						if(fDist <= fDim/2)
						{
							SetNewState(STATE_Escape_Hide);
							break;
						}

						MC_FacePos(m_vTrackObjPos);

						DRotation rRot;
						DVector vU, vR, vF, vMyF;

						m_pServerDE->GetObjectRotation(m_hTrackObject, &rRot);
						m_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

						VEC_COPY(vMyF, m_MoveObj.GetForwardVector());

						DFLOAT fHintDp = (vMyF.x * vF.x) + (vMyF.y * vF.y) + (vMyF.z * vF.z);

						if(fHintDp > 0)
						{					
							if(!m_MoveObj.CalculatePath(m_vTrackObjPos))
								SetNewState(STATE_Escape_Hide);
						}
						else
							SetNewState(STATE_Escape_Hide);
					}
					else
						SetNewState(STATE_Escape_Hide);
					
					break;
		}
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
// ROUTINE		: CivilianAI::AI_STATE_Escape_Hide
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Escape_Hide()
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
// ROUTINE		: CivilianAI::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Idle()
{
	if(m_bScared)
	{
		int nStimType = ComputeStimuli();

		if(nStimType > 0)
		{
			m_pServerDE->SetNextUpdate(m_hObject, 0.001f);
			ComputeState(nStimType);
			return;
		}
	}

	m_pServerDE->SetNextUpdate(m_hObject, 0.1f);

	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special1
// DESCRIPTION	: TALKING state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special1()
{
	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_Talk();			break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special2
// DESCRIPTION	: TYPING state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special2()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(0);		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special3
// DESCRIPTION	: HAND_WARMING state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special3()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(m_pServerDE->IntRandom(1,2));	break;
		case 2:		ComputeState();								break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special4
// DESCRIPTION	: SLEEPING state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special4()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(3);		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special5
// DESCRIPTION	: STUDY state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special5()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(4);		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special6
// DESCRIPTION	: RIGHT HAND SUBWAY state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special6()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(m_pServerDE->IntRandom(5,6));		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special7
// DESCRIPTION	: LEFT HAND SUBWAY state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special7()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(7);		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CivilianAI::AI_STATE_Special8
// DESCRIPTION	: FETAL state
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CivilianAI::AI_STATE_Special8()
{
	switch(Metacmd)
	{
		case 1:		MC_Special(8);		break;
		case 2:		ComputeState();		break;
	}

	return;
}

void CivilianAI::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	DBYTE vals;

	vals = m_bMale | (m_bLabTech * 0x02) | (m_bScared * 0x04);

	m_pServerDE->WriteToMessageByte(hWrite, vals);
}

void CivilianAI::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	DBYTE vals;

	vals = m_pServerDE->ReadFromMessageByte(hRead);

	m_bMale = ((vals & 0x01) != 0);
	m_bLabTech = ((vals & 0x02) != 0);
	m_bScared = ((vals & 0x04) != 0);

	//Load up animation indexes if first model instance
	if(m_bMale || m_bLabTech && m_bMaleAnims)
	{
		m_Male_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Male_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Male_Anim_Sound.SetSoundRoot("sounds\\enemies\\civilian");
		m_bMaleAnims = DFALSE;
	}

	if(!m_bMale && m_bFemaleAnims)
	{
		m_Female_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Female_Anim_Sound.GenerateHitSpheres(m_hObject);

		m_Female_Anim_Sound.SetSoundRoot("sounds\\enemies\\f_civ");
		m_bFemaleAnims = DFALSE;
	}

	if(m_bMale || m_bLabTech)
		AI_Mgr::InitStatics(&m_Male_Anim_Sound);
	else
		AI_Mgr::InitStatics(&m_Female_Anim_Sound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void CivilianAI::CacheFiles()
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
	if (m_bSororitySkin)
	{
		SetCacheDirectory("sounds_ao\\enemies\\femalemisc");

		CacheSoundFileRange("femdie", 1, 3);
		CacheSoundFileRange("femgig", 1, 3);
		CacheSoundFileRange("fempain", 1, 3);
	}
	else
#endif

	if (m_bMale)
	{
		SetCacheDirectory("sounds\\enemies\\civilian");

		CacheSoundFileRange("death", 1, 3);
		CacheSoundFileRange("fear", 1, 3);
		CacheSoundFileRange("idle", 1, 3);
		CacheSoundFileRange("pain", 1, 3);
		CacheSoundFileRange("spot", 1, 3);
	}
	else
	{
		SetCacheDirectory("sounds\\enemies\\f_civ");

		CacheSoundFileRange("death", 1, 3);
		CacheSoundFileRange("fear", 1, 3);
		CacheSoundFileRange("idle", 1, 3);
		CacheSoundFileRange("pain", 1, 3);
		CacheSoundFileRange("spot", 1, 3);
	}
}


