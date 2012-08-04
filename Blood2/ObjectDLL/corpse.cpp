
#include <stdio.h>
#include "corpse.h"
#include "ObjectUtilities.h"
#include "weapon.h"
#include "SFXMsgIds.h"
#include "ClientServerShared.h"
#include "ClientGibFX.h"
#include "ClientSplatFX.h"
#include "sparam.h"
#include "PlayerObj.h"
#include "VoiceMgrDefs.h"
#include "BloodServerShell.h"

BEGIN_CLASS(CCorpse)
END_CLASS_DEFAULT_FLAGS(CCorpse, B2BaseClass, NULL, NULL, CF_HIDDEN)

extern CPlayerObj* g_pPlayerObj;


#define CORPSE_REMOVETIME		30.0f


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::Setup()
//
//	PURPOSE:	Setup
//
// --------------------------------------------------------------------------- //

void CCorpse::Setup(HOBJECT hFireObj, DBOOL bLimbLoss)
{
	CServerDE* g_pServerDE = GetServerDE();
	if (!g_pServerDE) return;

	if(hFireObj)
	{
		m_hFireSource = hFireObj;

		DVector		offset;
		VEC_SET(offset, 0.0f, 0.0f, 0.0f);

		HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&offset);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);
		g_pServerDE->WriteToMessageObject(hMessage, m_hFireSource);
		g_pServerDE->WriteToMessageVector(hMessage, &offset);
		g_pServerDE->WriteToMessageFloat(hMessage, 3.0f);
		g_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SCALENUMPARTICLES);
		g_pServerDE->WriteToMessageDWord(hMessage, OBJFX_FLAMING_2);
		g_pServerDE->WriteToMessageDWord(hMessage, 0);
		g_pServerDE->EndMessage(hMessage);

		DRotation rRot;
		ROT_INIT(rRot);

		g_pServerDE->CreateAttachment(m_hObject, m_hFireSource, "torso", &offset, &rRot, &m_hAttach);

		m_fBurnStart = g_pServerDE->GetTime();

		//play ambient looping sound
		PlaySoundInfo playSoundInfo;
		PLAYSOUNDINFO_INIT( playSoundInfo );

		playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB;
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_CTRL_VOL;
		
		_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"sounds\\weapons\\fireloop.wav", _MAX_PATH );
		playSoundInfo.m_hObject = m_hObject;
		playSoundInfo.m_nPriority = SOUNDPRIORITY_AI_MEDIUM;
		playSoundInfo.m_fOuterRadius = 1000;
		playSoundInfo.m_fInnerRadius = 1000 * 0.2f;
		playSoundInfo.m_nVolume = 60;
		
		g_pServerDE->PlaySound( &playSoundInfo );
		m_hLoopSound = playSoundInfo.m_hSound;

		//create the smoke object
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_ObjectType = OT_NORMAL;
		ocStruct.m_NextUpdate = 0.01f;
		g_pServerDE->GetModelNodeTransform(m_hObject, "torso",&ocStruct.m_Pos,&ocStruct.m_Rotation);
		ocStruct.m_Flags = FLAG_FORCECLIENTUPDATE;
		
		HCLASS hClass = g_pServerDE->GetClass("BaseClass");
		BaseClass* pObj = g_pServerDE->CreateObject(hClass, &ocStruct);

		if(pObj)
		{
			m_hSmokeSource = pObj->m_hObject;
		}
	}

	m_bLimbLoss = bLimbLoss;

	return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD CCorpse::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
				if (!Update())
				g_pServerDE->RemoveObject( m_hObject );

			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
			}
			if (fData == INITIALUPDATE_WORLDFILE)
			{
				MoveObjectToGround(m_hObject);
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			OnStringKey((ArgList*)pData);
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CCorpse::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	CServerDE* g_pServerDE = GetServerDE();
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);

			DVector vDir,vPos;
			DFLOAT fDamage;
			DBYTE  nType;
				
			g_pServerDE->ReadFromMessageVector(hRead, &vDir);
			fDamage = g_pServerDE->ReadFromMessageFloat(hRead);
			nType	= g_pServerDE->ReadFromMessageByte(hRead);
			HOBJECT hObj = g_pServerDE->ReadFromMessageObject(hRead);
			g_pServerDE->ReadFromMessageVector(hRead, &vPos);

			VEC_MULSCALAR(vDir,vDir,-1.0f);

			if (!m_bDead)
				return HandleDamage(vDir, fDamage, nType, vPos);
			else
				return 0;
		}

		default : break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::ReadProp()
//
//	PURPOSE:	Reads properties
//
// --------------------------------------------------------------------------- //

DBOOL CCorpse::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void CCorpse::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags = FLAG_VISIBLE | FLAG_RAYHIT | FLAG_GRAVITY | FLAG_MODELKEYS | FLAG_REMOVEIFOUTSIDE;

	if(pStruct->m_UserData)
		_mbscpy((unsigned char*)m_szSoundDir, (const unsigned char*)pStruct->m_UserData);

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void CCorpse::InitialUpdate(DVector *pMovement)
{
	if (!g_pServerDE) return;

	m_damage.Init( m_hObject );

	// Setup damage stats...
	m_damage.SetMass( m_fMass );
	m_damage.SetHitPoints( m_fHitPoints );
	m_damage.SetMaxHitPoints( m_fHitPoints );
	m_damage.SetArmorPoints( 0.0f );
	m_damage.SetMaxArmorPoints( 0.0f );
	m_damage.SetApplyDamagePhysics(DFALSE);

	GenerateHitSpheres();

	DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	dwFlags |= (SURFTYPE_FLESH << 24);

	dwFlags |= USRFLG_SAVEABLE  | USRFLG_SINGULARITY_ATTRACT;

	g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

	g_pServerDE->SetNextUpdate( m_hObject, 0.01f );

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::Update
//
//	PURPOSE:	Update until the death anim is done..
//
// ----------------------------------------------------------------------- //

DBOOL CCorpse::Update()
{
	if (m_bDead || !g_pServerDE) return DFALSE;

	DDWORD dwState	= g_pServerDE->GetModelPlaybackState(m_hObject);
	DFLOAT fTime = g_pServerDE->GetTime();

	if(m_fRemoveTime && m_fRemoveTime <= fTime) return DFALSE;

	if(m_fSmokeStart == 0.0f && m_hFireSource && fTime - m_fBurnStart > 5.0f)
	{
		g_pServerDE->RemoveAttachment(m_hAttach);
		g_pServerDE->RemoveObject(m_hFireSource);

		m_hFireSource = DNULL;

		if(m_hLoopSound)
		{
			g_pServerDE->KillSound(m_hLoopSound);
			m_hLoopSound = DNULL;
		}

		//create the smoke
		DVector		offset;
		VEC_SET(offset, 0.0f, 0.0f, 0.0f);

		HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&offset);
		g_pServerDE->WriteToMessageByte(hMessage, SFX_OBJECTFX_ID);
		g_pServerDE->WriteToMessageObject(hMessage, m_hSmokeSource);
		g_pServerDE->WriteToMessageVector(hMessage, &offset);
		g_pServerDE->WriteToMessageFloat(hMessage, 3.0f);
		g_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SCALENUMPARTICLES);
		g_pServerDE->WriteToMessageDWord(hMessage, OBJFX_SMOKING_2);
		g_pServerDE->WriteToMessageDWord(hMessage, 0);
		g_pServerDE->EndMessage(hMessage);

		DRotation rRot;
		ROT_INIT(rRot);

		g_pServerDE->CreateAttachment(m_hObject, m_hSmokeSource, "torso", &offset, &rRot, &m_hAttach);

		m_fSmokeStart = fTime;

		g_pServerDE->DebugOut("SmokeStart: %f\r\n", m_fSmokeStart);
	}
	else if(m_hFireSource == DNULL && m_hSmokeSource && fTime - m_fSmokeStart > 5.0f)
	{
		g_pServerDE->DebugOut("SmokeEnd: %f\r\n", fTime);

		g_pServerDE->RemoveAttachment(m_hAttach);
		g_pServerDE->RemoveObject(m_hSmokeSource);

		m_hSmokeSource = DNULL;
		m_hAttach = DNULL;
	}

	// Done with any death animation
	if (dwState & MS_PLAYDONE)
	{
		CreateBloodPool();

		if (g_pBloodServerShell->IsMultiplayerGame())
			m_fRemoveTime = fTime + CORPSE_REMOVETIME - 0.1f;

		if(m_hAttach == DNULL)
		{
			if (g_pBloodServerShell->IsMultiplayerGame())
				g_pServerDE->SetNextUpdate(m_hObject, CORPSE_REMOVETIME);
			else
				g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		else
			g_pServerDE->SetNextUpdate(m_hObject, 0.1f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.1f);
	}
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::HandleDamage
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

DBOOL CCorpse::HandleDamage(DVector vDir, DFLOAT fDamage, int nType, DVector vPos)
{

//	int nNodeHit = SetProperNode(CalculateHitLimb(vDir,vPos));
	if (!m_hObject) return DFALSE;

	int nNodeHit = -1;

	if(m_bLimbLoss)
	{
		nNodeHit = SetProperNode(CalculateHitLimb(vDir,vPos));

		//sanity check
		if(nNodeHit == -1 && !(nType & DAMAGE_TYPE_EXPLODE))
			return DFALSE;
	}

	if (m_damage.IsDead()) 
	{
		m_bDead = DTRUE;
		CreateGibs(vDir, ((int)m_damage.GetMass())>>5, nType, fDamage);
		// Set update to remove on next frame.
		g_pServerDE->SetNextUpdate(m_hObject, 0.1f);

		if(m_hAttach)
			g_pServerDE->RemoveAttachment(m_hAttach);

		if(m_hFireSource)
		{
			g_pServerDE->RemoveObject(m_hFireSource);
			m_hFireSource = DNULL;
		}

		if(m_hLoopSound)
		{
			g_pServerDE->KillSound(m_hLoopSound);
			m_hLoopSound = DNULL;
		}

		if (IsRandomChance(7))
		{
			g_pPlayerObj->PlayVoiceGroupEventOnClient(VME_BIGGIB, DTRUE);	// [blg]
		}
	}
	else if (nNodeHit >= 0 && nNodeHit < NUM_ALL_NODES && m_bLimbLoss)
	{
		if(IsRandomChance(50) && AIShared.HideLimb(m_hObject,nNodeHit))
		{
			AIShared.CreateLimb(m_hObject, nNodeHit, vDir);
//			AIShared.CreateLimb(vDir);

			//Create an arterial blood spurt
//			CreateBloodSpurt(nNodeHit);
		}
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
// ROUTINE		: CCorpse::CalculateHitLimb
//
// DESCRIPTION	: 
//
// RETURN TYPE	: int
//
// ----------------------------------------------------------------------- //

int CCorpse::CalculateHitLimb(DVector vDir, DVector vPos)
{
	int nNode = -1;
	DFLOAT fNodeDist = 0.0f, fDist = 999.0f, fTemp = 0.0f;
	DVector vShot, vNewShot, vTemp, vObjDims, vNodePos, vZ;
	DFLOAT fX, fY, ft;
	DBOOL bStatus = DFALSE;
	DRotation rRot;

	g_pServerDE->GetModelAnimUserDims(m_hObject, &vObjDims, g_pServerDE->GetModelAnimation(m_hObject));

	vTemp.x = (float)fabs(vDir.x);
	vTemp.y = (float)fabs(vDir.y);
	vTemp.z = (float)fabs(vDir.z);

	if(vTemp.x > vTemp.y && vTemp.x > vTemp.z)
	{
		fTemp = vObjDims.x / vTemp.x;
	}
	else if(vTemp.y > vTemp.x  && vTemp.y > vTemp.z)
	{
		fTemp = vObjDims.y / vTemp.y;
	}
	else if(vTemp.z > vTemp.x  && vTemp.z > vTemp.y)
	{
		fTemp = vObjDims.z / vTemp.z;
	}

	VEC_MULSCALAR(vNewShot,vDir,fTemp);
	VEC_ADD(vShot,vPos,vNewShot);

	DVector vC;
	VEC_SUB(vC,vShot,vPos);

	fX = 1 / VEC_DOT(vC,vC);
	fY = fX * -(VEC_DOT(vC,vPos));
	
	for(int i = 0; i < NUM_STD_NODES; i++)
	{
		g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[i], &bStatus);

		if(!bStatus)
		{
			DBOOL bRet = g_pServerDE->GetModelNodeTransform(m_hObject, szNodes[i], &vNodePos, &rRot);

			ft = VEC_DOT(vC,vNodePos) * fX + fY;

			if(ft >= 0.0f && ft <= 1.0f)
			{
				VEC_ADDSCALED(vZ,vPos,vC, ft);

				fNodeDist = VEC_DIST(vNodePos, vZ);

				if(fNodeDist < fDist && fNodeDist <= m_fHitSpheres[i])
				{
					fDist = fNodeDist;
					nNode = i;
				}
			}
		}
	}

	//Do we leave a pass through mark behind us?
	if(nNode != -1)
	{
//		CWeapon weap(WEAP_BERETTA);

//		VEC_MULSCALAR(vTemp,vDir,-1.0f);
//		weap.AddSparks(vPos, vTemp, m_fHitPoints, m_hObject, SURFTYPE_FLESH);	
//		weap.AddBloodSpurt(vPos, vTemp, m_fHitPoints, m_hObject, SURFTYPE_FLESH);
		
//		vTemp.x *= -1.0f;
//		vTemp.z *= -1.0f;
//		weap.AddBloodSpurt(vPos, vTemp, m_fHitPoints, m_hObject, SURFTYPE_FLESH);	
	}

	return nNode;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CCorpse::SetProperNode
// DESCRIPTION	: Set the hit node to the parent node of the limb
// RETURN TYPE	: int 
// PARAMS		: int nNode
// ----------------------------------------------------------------------- //

int CCorpse::SetProperNode(int nNode)
{
	switch(nNode)
	{
		case 0:		return NODE_NECK;
		case 1:		return NODE_NECK;
		case 2:		return NODE_TORSO;
		case 3:		return NODE_TORSO;
		case 4:		return NODE_RARM;
		case 5:		return NODE_RARM;
		case 6:		return NODE_RARM;
		case 7:		return NODE_LARM;
		case 8:		return NODE_LARM;
		case 9:		return NODE_LARM;
		case 10:	return NODE_LLEG;
		case 11:	return NODE_LLEG;
		case 12:	return NODE_LLEG;
		case 13:	return NODE_LLEG;
		case 14:	return NODE_RLEG;
		case 15:	return NODE_RLEG;
		case 16:	return NODE_RLEG;
		case 17:	return NODE_RLEG;
		default:	return -1;
	}

	return -1;
}

void CCorpse::GenerateHitSpheres()
{
	DVector vNodePos[NUM_STD_NODES];
	DRotation rRot;

	for(int i = 0; i < NUM_STD_NODES; i++)
	{
		g_pServerDE->GetModelNodeTransform(m_hObject, szNodes[i], &vNodePos[i], &rRot);
	}

	//HEAD/NECK
	m_fHitSpheres[0] = VEC_DIST(vNodePos[0],vNodePos[1]) * 0.66f;
	m_fHitSpheres[1] = m_fHitSpheres[0];

	//TORSO/PELVIS
	m_fHitSpheres[2] = VEC_DIST(vNodePos[2],vNodePos[3]) * 0.66f;
	m_fHitSpheres[3] = m_fHitSpheres[2];

	//R_ARM
	m_fHitSpheres[4] = VEC_DIST(vNodePos[4],vNodePos[5]) * 0.66f;
	m_fHitSpheres[5] = VEC_DIST(vNodePos[5],vNodePos[6]) * 0.66f;
	m_fHitSpheres[6] = m_fHitSpheres[5];

	//L_ARM
	m_fHitSpheres[7] = VEC_DIST(vNodePos[7],vNodePos[8]) * 0.66f;
	m_fHitSpheres[8] = VEC_DIST(vNodePos[8],vNodePos[9]) * 0.66f;
	m_fHitSpheres[9] = m_fHitSpheres[8];

	//L_LEG
	m_fHitSpheres[10] = VEC_DIST(vNodePos[10],vNodePos[11]) * 0.66f;
	m_fHitSpheres[11] = VEC_DIST(vNodePos[11],vNodePos[12]) * 0.66f;
	m_fHitSpheres[12] = VEC_DIST(vNodePos[12],vNodePos[13]) * 0.66f;
	m_fHitSpheres[13] = m_fHitSpheres[12];

	//R_LEG
	m_fHitSpheres[14] = VEC_DIST(vNodePos[14],vNodePos[15]) * 0.66f;
	m_fHitSpheres[15] = VEC_DIST(vNodePos[15],vNodePos[16]) * 0.66f;
	m_fHitSpheres[16] = VEC_DIST(vNodePos[16],vNodePos[17]) * 0.66f;
	m_fHitSpheres[17] = m_fHitSpheres[16];

	return;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: CCorpse::CreateGibs
// DESCRIPTION	: create some nastiness
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

DBOOL CCorpse::CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage)
{
//	HCLASS hClass = g_pServerDE->GetClass( "CClientGibFX" );
//	if( !hClass )
//		return DFALSE;


	DFLOAT fVel = 50.0f + fDamage;

	vDir.y -= 1.0f;
	VEC_NORM(vDir);

	VEC_MULSCALAR(vDir, vDir, fVel);

//	ObjectCreateStruct ocStruct;
//	INIT_OBJECTCREATESTRUCT(ocStruct);

	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
//	VEC_COPY(ocStruct.m_Pos, vPos);
//	g_pServerDE->GetObjectRotation(m_hObject, &ocStruct.m_Rotation);

//	CClientGibFX* pGib = (CClientGibFX*)g_pServerDE->CreateObject(hClass, &ocStruct);

	DVector vDims;
	g_pServerDE->GetObjectDims(m_hObject, &vDims);

/*	if(nType & DAMAGE_TYPE_EXPLODE)
		pGib->Setup(&vPos, &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, nNumGibs);
	else
		pGib->Setup(&vPos, &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, nNumGibs); 
*/
	SetupClientGibFX(&vPos, &vDir, &vDims, (SURFTYPE_FLESH/10) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, nNumGibs);

	// Create body parts

	if(m_bLimbLoss)
	{
		// 100 % create a head..
		DBOOL bNodeStatus;
		if (g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[NODE_NECK], &bNodeStatus) == DE_OK)
		{
			if (!bNodeStatus)	// not hidden
				AIShared.CreateLimb(m_hObject, NODE_NECK, vDir);
		}

		if (g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[NODE_RARM], &bNodeStatus) == DE_OK)
		{
			if (!bNodeStatus && IsRandomChance(80))
				AIShared.CreateLimb(m_hObject, NODE_RARM, vDir);
		}

		if (g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[NODE_LARM], &bNodeStatus) == DE_OK)
		{
			if (!bNodeStatus && IsRandomChance(80))
				AIShared.CreateLimb(m_hObject, NODE_LARM, vDir);
		}

		if (g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[NODE_RLEG], &bNodeStatus) == DE_OK)
		{
			if (!bNodeStatus && IsRandomChance(60))
				AIShared.CreateLimb(m_hObject, NODE_RLEG, vDir);
		}

		if (g_pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[NODE_LLEG], &bNodeStatus) == DE_OK)
		{
			if (!bNodeStatus && IsRandomChance(60))
				AIShared.CreateLimb(m_hObject, NODE_LLEG, vDir);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCorpse::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	for (int i=0; i < NUM_STD_NODES; i++)
		pServerDE->WriteToMessageFloat(hWrite, m_fHitSpheres[i]);

	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageFloat(hWrite, m_fHitPoints);

	pServerDE->WriteToMessageString(hWrite, m_szSoundDir);
	pServerDE->WriteToMessageByte(hWrite, m_bLimbLoss);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCorpse::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCorpse::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	for (int i=0; i < NUM_STD_NODES; i++)
		m_fHitSpheres[i] = pServerDE->ReadFromMessageFloat(hRead);

	m_fMass			= pServerDE->ReadFromMessageFloat(hRead);
	m_fHitPoints	= pServerDE->ReadFromMessageFloat(hRead);

	_mbscpy((unsigned char*)m_szSoundDir, (const unsigned char*)pServerDE->ReadFromMessageString(hRead));

	m_bLimbLoss		= pServerDE->ReadFromMessageByte(hRead);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CCorpse::OnStringKey
// DESCRIPTION	: read and parse string keys
// RETURN TYPE	: void 
// PARAMS		: ArgList* pArgList
// ----------------------------------------------------------------------- //

void CCorpse::OnStringKey(ArgList* pArgList)
{
	if (!g_pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	char szTemp[32];
	szTemp[0] = '\0';

	if (m_szSoundDir[0] != '\0' && Sparam_Get(szTemp,pKey,"play_sound"))
	{
		char szSound[256];
		char szTmp[32];

		szSound[0] = '\0';
		szTmp[0]   = '\0';

		if(Sparam_Get(szTmp,pKey,"sound_random"))
		{
			int nMax = atoi(szTmp);
			int nNum = g_pServerDE->IntRandom(1,NRES(nMax));
			sprintf(szSound, "%s\\%s%d.wav", m_szSoundDir, szTemp, nNum);
		}
		else
		{
			sprintf(szSound, "%s\\%s.wav", m_szSoundDir, szTemp);
		}

		DFLOAT fRadius = 1000.0f;

		if(Sparam_Get(szTmp,pKey,"sound_radius"))
		{
			fRadius = (DFLOAT)atof(szTmp);
		}

		int nVol = 100;

		if(Sparam_Get(szTmp,pKey,"sound_volume"))
		{
			nVol = atoi(szTmp);
		}

		PlayAISound(szSound, fRadius, 0, nVol);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CCorpse::PlayAISound
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: char* szSound
// PARAMS		: DFLOAT fRadius
// ----------------------------------------------------------------------- //

DBOOL CCorpse::PlayAISound(char* szSound, DFLOAT fRadius, DDWORD dwFlags, int nVol)
{
	if (m_hCurSound)
	{
		DBOOL bIsSoundDone;

		if( !( dwFlags & PLAY_INTERRUPT))
			return DFALSE;
		if(  ((dwFlags & PLAY_WAIT) && g_pServerDE->IsSoundDone(m_hCurSound, &bIsSoundDone) == LT_OK && !bIsSoundDone))
			return DFALSE;

		g_pServerDE->KillSound( m_hCurSound );
		m_hCurSound = DNULL;
	}

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_REVERB;

//	playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
//	playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
//	playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
//	playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;

	if(nVol < 100)
		playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)szSound, _MAX_PATH );
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_AI_MEDIUM;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
	playSoundInfo.m_nVolume = nVol;
	
	g_pServerDE->PlaySound( &playSoundInfo );

	m_hCurSound = playSoundInfo.m_hSound;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
// ROUTINE		: CCorpse::CreateBloodPool
// DESCRIPTION	: 
// RETURN TYPE	: void
// PARAMS		: none
// ----------------------------------------------------------------------- //

void CCorpse::CreateBloodPool()
{
	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	IntersectQuery iq;
	IntersectInfo  ii;

	g_pServerDE->GetObjectPos(m_hObject, &iq.m_From);
	VEC_COPY(iq.m_To, iq.m_From);
	iq.m_To.y -= 40.0f;

	iq.m_Flags = IGNORE_NONSOLID;
	iq.m_FilterFn = NULL;
	iq.m_pUserData = NULL;	

	if (g_pServerDE->IntersectSegment(&iq, &ii))
	{
		// Only care if we hit the world
		if (ii.m_hObject == g_pServerDE->GetWorldObject())
		{
			HCLASS hClass = g_pServerDE->GetClass("CClientSplatFX");
			ii.m_Point.y += 0.1f;

			CClientSplatFX *pSplat = DNULL;
			if (hClass)
			{
				VEC_COPY(ocStruct.m_Pos, ii.m_Point);
				pSplat = (CClientSplatFX *)g_pServerDE->CreateObject(hClass, &ocStruct);
			}

			if (pSplat)
			{
				pSplat->Setup( &ii.m_Point, &ii.m_Plane.m_Normal, 0.10f, 0.02f);
			}
		}
	}
}
