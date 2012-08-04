// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMode.cpp
//
// PURPOSE : PlayerObj helper class - Implementation
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#include "PlayerMode.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "CharacterAttributes.h"
#include "RiotObjectUtilities.h"
#include "RiotServerShell.h"

// Used to set camera offsets...

static DVector s_vOnfootCamera;
static DVector s_vKidCamera;
static DVector s_vMcaAPCamera;
static DVector s_vMcaUECamera;
static DVector s_vMcaAOCamera;
static DVector s_vMcaSACamera;
static DVector s_vMcaAPVehicleCamera;
static DVector s_vMcaUEVehicleCamera;
static DVector s_vMcaAOVehicleCamera;
static DVector s_vMcaSAVehicleCamera;

extern CRiotServerShell* g_pRiotServerShellDE;
extern CVarTrack g_RammingDamageTrack;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::CPlayerMode
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

CPlayerMode::CPlayerMode()
{
	// Initialize statics...

	VEC_SET(s_vOnfootCamera, 0.0, 34.0, 0.0);
	VEC_SET(s_vKidCamera, 0.0, 16.0, 0.0);
	VEC_SET(s_vMcaAPCamera, 0.0, 28.0, 0.0);
	VEC_SET(s_vMcaUECamera, 0.0, 28.0, 0.0);
	VEC_SET(s_vMcaAOCamera, 0.0, 28.0, 0.0);
	VEC_SET(s_vMcaSACamera, 0.0, 28.0, 0.0);
	VEC_SET(s_vMcaAPVehicleCamera, 0.0, 10.0, 0.0);
	VEC_SET(s_vMcaUEVehicleCamera, 0.0, 10.0, 0.0);
	VEC_SET(s_vMcaAOVehicleCamera, 0.0, 10.0, 0.0);
	VEC_SET(s_vMcaSAVehicleCamera, 0.0, 10.0, 0.0);

	m_pMyObj			= DNULL;

	m_nModelId			= MI_PLAYER_ONFOOT_ID;

	m_nMode				= PM_MODE_UNDEFINED;
	m_bBipedal			= DFALSE;
	m_fWalkVel			= PM_FOOT_WALKSPEED;
	m_fRunVel			= PM_FOOT_RUNSPEED;
	m_fJumpSpeed		= PM_FOOT_JUMPSPEED;

	m_nArmor			= PM_MCA_MOD_NONE;
	m_nVehicleHandling	= PM_MCA_MOD_NONE;

	m_FovX				= MATH_HALFPI;
	m_FovY				= m_FovX / float(640.0 / 480.0);

	VEC_INIT(m_vCameraOffset);	

	// SetMode(PM_MODE_UNDEFINED, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetMode
//
//	PURPOSE:	Set the current mode
//
//	@cmember change the player mode
//	@parm the new mode
//	@parm is it a bipedal mode
//	@rdef the old mode
//
// ----------------------------------------------------------------------- //

DBYTE CPlayerMode::SetMode(DBYTE nNewMode, DBOOL bBipedal)
{
	int nOldMode = m_nMode;
		
	if (PM_MODE_FOOT <= nNewMode && nNewMode <= PM_MODE_KID)
	{
		m_nMode = nNewMode;
	}
	else
	{
		return m_nMode;
	}

	switch(m_nMode)
	{
		case PM_MODE_MCA_AP : SetModeMcaAP(bBipedal); break;
		case PM_MODE_MCA_UE : SetModeMcaUE(bBipedal); break;
		case PM_MODE_MCA_AO : SetModeMcaAO(bBipedal); break;
		case PM_MODE_MCA_SA : SetModeMcaSA(bBipedal); break;
		case PM_MODE_KID    : SetModeKid(); break;

		case PM_MODE_FOOT :	  
		default :			  SetModeOnFoot(); break;
	}


	// Update the Camera FOV to reflect the new player mode...

	CServerDE* pServerDE = BaseClass::GetServerDE();

	if (pServerDE && m_pMyObj)
	{
		// Update client's mode info...
		HMESSAGEWRITE hMessage = pServerDE->StartMessage(m_pMyObj->GetClient(), MID_PLAYER_MODECHANGE);
		pServerDE->WriteToMessageVector(hMessage, &m_vCameraOffset);
		pServerDE->WriteToMessageByte(hMessage, m_nMode);
		pServerDE->WriteToMessageByte(hMessage, !IsBipedalMode());
		pServerDE->WriteToMessageFloat(hMessage, m_FovX);
		pServerDE->WriteToMessageFloat(hMessage, m_FovY);
		pServerDE->EndMessage(hMessage);
	}


	return nOldMode;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetRunVeloctiy
//
//	PURPOSE:	Get the maximum velocity when running
//
// @cmember get the maximum velocity when running
// @rdef the maximum run velocity
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetRunVelocity() const
{
	return m_fRunVel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetWalkVelocity
//
//	PURPOSE:	Get the maximum velocity when walking
//
// @cmember get the maximum velocity when walking
// @rdef the maximum run velocity
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetWalkVelocity() const
{
	return m_fWalkVel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::AdjustFriction
//
//	PURPOSE:	Adjust the friction based on the current mode
//
// @cmember adjust the friction based on the current mode
// @rdef the base friction coefficient
//
// ----------------------------------------------------------------------- //

float CPlayerMode::AdjustFriction(float fBaseFriction) const
{
	float fNewFriction = fBaseFriction;

	if (m_nMode == PM_MODE_FOOT)
	{
		fNewFriction = 5.0;
	}
	else if (m_bBipedal)
	{
		fNewFriction = 5.0;
	}
	else
	{
		double fFrictionAdjust[4] = { 1.0, 0.85, 1.0, 1.15 };
		fNewFriction = float(fBaseFriction * fFrictionAdjust[m_nVehicleHandling]);
	}

	return fNewFriction;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::AdjustArmor
//
//	PURPOSE:	Adjust the armor based on the current mode
//
// @cmember adjust the armor based on the current mode
// @rdef the base armor value
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::AdjustArmor(DFLOAT fBaseArmor) const
{
	DFLOAT fNewArmor = fBaseArmor;

	switch(m_nArmor)
	{
		case PM_MCA_MOD_NONE :
			fNewArmor = fBaseArmor;
		break;

		case PM_MCA_MOD_1 :
			fNewArmor = fBaseArmor * (DFLOAT)0.85;
		break;

		case PM_MCA_MOD_2 :
			fNewArmor = fBaseArmor;
		break;

		case PM_MCA_MOD_3 :
			if (fBaseArmor <= 0.0) fBaseArmor = 1;
			fNewArmor = fBaseArmor * (DFLOAT)1.15;
		break;
		
		default :			 
			fNewArmor = fBaseArmor;
	}

	return fNewArmor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeOnFoot
//
//	PURPOSE:	Change the player mode to on-foot
//
// @cmember change the player mode to on foot
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeOnFoot()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return;

	m_nModelId = MI_PLAYER_ONFOOT_ID;

	// Make Multiplayer on-foot speed faster...

	GameType eGameType = g_pRiotServerShellDE->GetGameType();
	DFLOAT fVal = (eGameType == SINGLE ) ? 1.0f : 1.25f;

	m_fWalkVel		= (DFLOAT)(PM_FOOT_WALKSPEED * fVal);
	m_fRunVel		= (DFLOAT)(PM_FOOT_RUNSPEED * fVal);
	m_fJumpSpeed	= PM_FOOT_JUMPSPEED;

	VEC_COPY(m_vCameraOffset, s_vOnfootCamera);

	m_bBipedal			= DTRUE;
	m_nArmor			= PM_MCA_MOD_NONE;
	m_nVehicleHandling	= PM_MCA_MOD_NONE;

	m_FovX				= MATH_HALFPI;
	m_FovY				= m_FovX / float(640.0 / 480.0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeKid
//
//	PURPOSE:	Change the player mode to kid
//
// @cmember change the player mode to kid
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeKid()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return;

	m_nModelId = MI_PLAYER_KID_ID;

	// Make Multiplayer on-foot speed faster...

	GameType eGameType = g_pRiotServerShellDE->GetGameType();
	DFLOAT fVal = (eGameType == SINGLE ) ? 1.0f : 1.25f;

	m_fWalkVel		= (DFLOAT)(PM_FOOT_WALKSPEED * fVal);
	m_fRunVel		= (DFLOAT)(PM_FOOT_RUNSPEED * fVal);
	m_fJumpSpeed	= PM_FOOT_JUMPSPEED;
	
	VEC_COPY(m_vCameraOffset, s_vKidCamera);

	m_bBipedal			= DTRUE;
	m_nArmor			= PM_MCA_MOD_NONE;
	m_nVehicleHandling	= PM_MCA_MOD_NONE;

	m_FovX				= MATH_HALFPI;
	m_FovY				= m_FovX / float(640.0 / 480.0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeMcaAP
//
//	PURPOSE:	Change the player mode to Andra Predator
//
// @cmember change the player mode to Andra Predator
// @parm bipedal mode
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeMcaAP(DBOOL bBipedal)
{
	m_bBipedal			= bBipedal;

	m_nArmor			= PM_MCA_MOD_3;
	m_nVehicleHandling	= PM_MCA_MOD_3;

	m_nModelId = MI_PLAYER_PREDATOR_ID;

	if (bBipedal)
	{
		m_fRunVel	 = PM_MCA_BSPEED_1;
		m_fWalkVel   = PM_MCA_BSPEED_1;
		m_fJumpSpeed = PM_MCA_JSPEED_1;
		m_FovX		 = MATH_HALFPI;
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaAPCamera);
	}
	else
	{
		m_fRunVel	 = PM_MCA_VSPEED_1;
		m_fWalkVel   = PM_MCA_VSPEED_1;
		m_fJumpSpeed = PM_MCA_JSPEED_1;
		m_FovX		 = float(MATH_PI * .6);
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaAPVehicleCamera);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeMcaUE
//
//	PURPOSE:	Change the player mode to UCA Enforcer
//
// @cmember change the player mode to UCA Enforcer
// @parm bipedal mode
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeMcaUE(DBOOL bBipedal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bBipedal			= bBipedal;

	m_nArmor			= PM_MCA_MOD_2;
	m_nVehicleHandling	= PM_MCA_MOD_2;

	m_nModelId = MI_PLAYER_ENFORCER_ID;

	if (bBipedal)
	{
		m_fRunVel	 = PM_MCA_BSPEED_2;
		m_fWalkVel   = PM_MCA_BSPEED_2;
		m_fJumpSpeed = PM_MCA_JSPEED_2;
		m_FovX		 = MATH_HALFPI;
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaUECamera);
	}
	else
	{
		m_fRunVel	 = PM_MCA_VSPEED_2;
		m_fWalkVel   = PM_MCA_VSPEED_2;
		m_fJumpSpeed = PM_MCA_JSPEED_2;
		m_FovX		 = float(MATH_PI * .6);
		m_FovY		 = m_FovX / float(640.0 / 480.0);
		
		VEC_COPY(m_vCameraOffset, s_vMcaUEVehicleCamera);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeMcaAO
//
//	PURPOSE:	Change the player mode to Armacham Ordog
//
// @cmember change the player mode to Armacham Ordog
// @parm bipedal mode
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeMcaAO(DBOOL bBipedal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bBipedal			= bBipedal;
	m_nArmor			= PM_MCA_MOD_2;
	m_nVehicleHandling	= PM_MCA_MOD_1;

	m_nModelId = MI_PLAYER_ORDOG_ID;

	if (bBipedal)
	{
		m_fRunVel	 = PM_MCA_BSPEED_2;
		m_fWalkVel   = PM_MCA_BSPEED_2;
		m_fJumpSpeed = PM_MCA_JSPEED_2;
		m_FovX		 = MATH_HALFPI;
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaAOCamera);
	}
	else
	{
		m_fRunVel	 = PM_MCA_VSPEED_3;
		m_fWalkVel   = PM_MCA_VSPEED_3;
		m_fJumpSpeed = PM_MCA_JSPEED_2;
		m_FovX		 = float(MATH_PI * .6);
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaAOVehicleCamera);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetModeMcaSA
//
//	PURPOSE:	Change the player mode to Shogo Akuma
//
// @cmember change the player mode to Shogo Akuma
// @parm bipedal mode
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetModeMcaSA(DBOOL bBipedal)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bBipedal			= bBipedal;
	m_nArmor			= PM_MCA_MOD_1;
	m_nVehicleHandling	= PM_MCA_MOD_3;

	m_nModelId = MI_PLAYER_AKUMA_ID;

	if (bBipedal)
	{
		m_fRunVel	 = PM_MCA_BSPEED_3;
		m_fWalkVel   = PM_MCA_BSPEED_3;
		m_fJumpSpeed = PM_MCA_JSPEED_3;
		m_FovX		 = MATH_HALFPI;
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaSACamera);
	}
	else
	{
		m_fRunVel	 = PM_MCA_VSPEED_3;
		m_fWalkVel   = PM_MCA_VSPEED_3;
		m_fJumpSpeed = PM_MCA_JSPEED_3;
		m_FovX		 = float(MATH_PI * .6);
		m_FovY		 = m_FovX / float(640.0 / 480.0);

		VEC_COPY(m_vCameraOffset, s_vMcaSAVehicleCamera);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::IsOnFoot
//
//	PURPOSE:	Determine if player is on foot
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerMode::IsOnFoot() const
{
	return (m_nMode == PM_MODE_FOOT || m_nMode == PM_MODE_KID);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::SetCameraOffset
//
//	PURPOSE:	Set the player's camera offset
//
// ----------------------------------------------------------------------- //

void CPlayerMode::SetCameraOffset(DVector vOffset)
{
	// Save the offset for future updates...

	switch(m_nMode)
	{
		case PM_MODE_MCA_AP :
			if (m_bBipedal) 
			{
				VEC_COPY(s_vMcaAPCamera, vOffset);
			}
			else 
			{
				VEC_COPY(s_vMcaAPVehicleCamera, vOffset);
			}
		break;

		case PM_MODE_MCA_UE :
			if (m_bBipedal)
			{
				VEC_COPY(s_vMcaUECamera, vOffset);
			}
			else 
			{
				VEC_COPY(s_vMcaUEVehicleCamera, vOffset);
			}
		break;

		case PM_MODE_MCA_AO : 
			if (m_bBipedal) 
			{
				VEC_COPY(s_vMcaAOCamera, vOffset);
			}
			else 
			{
				VEC_COPY(s_vMcaAOVehicleCamera, vOffset);
			}
		break;

		case PM_MODE_MCA_SA : 
			if (m_bBipedal) 
			{
				VEC_COPY(s_vMcaSACamera, vOffset);
			}
			else 
			{
				VEC_COPY(s_vMcaSAVehicleCamera, vOffset);
			}
		break;

		case PM_MODE_FOOT :	  
			VEC_COPY(s_vOnfootCamera, vOffset);
		break;

		case PM_MODE_KID:	  
			VEC_COPY(s_vKidCamera, vOffset);
		break;

		default : break;
	}

	VEC_COPY(m_vCameraOffset, vOffset);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetMass
//
//	PURPOSE:	Set the player's mass
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetMass()
{
	DFLOAT fRet;

	switch(m_nMode)
	{
		case PM_MODE_MCA_AP :
		case PM_MODE_MCA_UE :
		case PM_MODE_MCA_AO : 
		case PM_MODE_MCA_SA : 
			fRet = m_bBipedal ? CA_PLAYER_MECHA_MASS : (g_RammingDamageTrack.GetFloat(1.0f) < 1.0f ? CA_PLAYER_MECHA_MASS : CA_PLAYER_MECHA_MASS * 1.5f);
		break;

		case PM_MODE_FOOT :	  
		case PM_MODE_KID :	  
		default : 
			fRet = CA_PLAYER_ONFOOT_MASS;
		break;
	}

	return fRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetSkinFilename
//
//	PURPOSE:	Set the player's skin filename
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetSkinFilename() const 
{
	DBOOL bMulti = (g_pRiotServerShellDE->GetGameType() != SINGLE);
	return GetSkin(m_nModelId, UCA, MS_NORMAL, bMulti); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetMaxHitPts
//
//	PURPOSE:	Set the player's mass
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetMaxHitPts()
{
	DFLOAT fRet;

	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
			fRet = CA_PLAYER_AKUMA_MAX_HITPTS;
		break;

		case PM_MODE_MCA_AP :
			fRet = CA_PLAYER_PREDATOR_MAX_HITPTS;
		break;

		case PM_MODE_MCA_AO : 
			fRet = CA_PLAYER_ORDOG_MAX_HITPTS;
		break;

		case PM_MODE_MCA_UE :
			fRet = CA_PLAYER_ENFORCER_MAX_HITPTS;
		break;

		case PM_MODE_KID :	  
			fRet = CA_PLAYER_KID_MAX_HITPTS;
		break;

		case PM_MODE_FOOT :	  
		default : 
			fRet = CA_PLAYER_ONFOOT_MAX_HITPTS;
		break;
	}

	return fRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetBaseHitPts
//
//	PURPOSE:	Set the player's starting hit points
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetBaseHitPts()
{
	DFLOAT fRet;

	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
			fRet = CA_PLAYER_AKUMA_HITPTS;
		break;

		case PM_MODE_MCA_AP :
			fRet = CA_PLAYER_PREDATOR_HITPTS;
		break;

		case PM_MODE_MCA_AO : 
			fRet = CA_PLAYER_ORDOG_HITPTS;
		break;

		case PM_MODE_MCA_UE :
			fRet = CA_PLAYER_ENFORCER_HITPTS;
		break;

		case PM_MODE_KID :	  
			fRet = CA_PLAYER_KID_HITPTS;
		break;

		case PM_MODE_FOOT :	  
		default : 
			fRet = CA_PLAYER_ONFOOT_HITPTS;
		break;
	}

	return fRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetMaxArmorPts
//
//	PURPOSE:	Set the player's maximum armor points
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetMaxArmorPts()
{
	DFLOAT fRet;

	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
			fRet = CA_PLAYER_AKUMA_MAX_ARMOR;
		break;

		case PM_MODE_MCA_AP :
			fRet = CA_PLAYER_PREDATOR_MAX_ARMOR;
		break;

		case PM_MODE_MCA_AO : 
			fRet = CA_PLAYER_ORDOG_MAX_ARMOR;
		break;

		case PM_MODE_MCA_UE :
			fRet = CA_PLAYER_ENFORCER_MAX_ARMOR;
		break;

		case PM_MODE_KID :	  
			fRet = CA_PLAYER_KID_MAX_ARMOR;
		break;

		case PM_MODE_FOOT :	  
		default : 
			fRet = CA_PLAYER_ONFOOT_MAX_ARMOR;
		break;
	}

	return fRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetBaseArmorPts
//
//	PURPOSE:	Set the player's starting armor points
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetBaseArmorPts()
{
	DFLOAT fRet;

	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
			fRet = CA_PLAYER_AKUMA_ARMOR;
		break;

		case PM_MODE_MCA_AP :
			fRet = CA_PLAYER_PREDATOR_ARMOR;
		break;

		case PM_MODE_MCA_AO : 
			fRet = CA_PLAYER_ORDOG_ARMOR;
		break;

		case PM_MODE_MCA_UE :
			fRet = CA_PLAYER_ENFORCER_ARMOR;
		break;

		case PM_MODE_KID :	  
			fRet = CA_PLAYER_KID_ARMOR;
		break;

		case PM_MODE_FOOT :	  
		default : 
			fRet = CA_PLAYER_ONFOOT_ARMOR;
		break;
	}

	return fRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetDamageSound
//
//	PURPOSE:	Determine what damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetDamageSound(DamageType eType)
{
	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
		case PM_MODE_MCA_AP :
		case PM_MODE_MCA_AO : 
		case PM_MODE_MCA_UE :
			return GetMechDamageSound(eType);
		break;

		case PM_MODE_KID :	  
			return GetKidDamageSound(eType);
		break;

		case PM_MODE_FOOT :	  
		default : 
			break;
		break;
	}

	return GetOnFootDamageSound(eType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetMechDamageSound
//
//	PURPOSE:	Determine what SA damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetMechDamageSound(DamageType eType)
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\Mech\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			s_FileBuffer[0] = '\0';
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav", "pain2.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}
/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetAPDamageSound
//
//	PURPOSE:	Determine what AP damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetAPDamageSound(DamageType eType)
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Predator\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			s_FileBuffer[0] = '\0';
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetAODamageSound
//
//	PURPOSE:	Determine what AO damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetAODamageSound(DamageType eType)
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Ordog\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			s_FileBuffer[0] = '\0';
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetUEDamageSound
//
//	PURPOSE:	Determine what UE damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetUEDamageSound(DamageType eType)
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Enforcer\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			s_FileBuffer[0] = '\0';
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}
*/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetKidDamageSound
//
//	PURPOSE:	Determine what Kid damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetKidDamageSound(DamageType eType)
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\Kid\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			char* ChokeSounds[] = { "choke.wav" };
	
			int nSize = (sizeof(ChokeSounds)/sizeof(ChokeSounds[0])) - 1;
			strcat(s_FileBuffer, ChokeSounds[GetRandom(0, nSize)]);
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav", "pain2.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetOnFootDamageSound
//
//	PURPOSE:	Determine what OnFoot damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetOnFootDamageSound(DamageType eType)
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\OnFoot\\");

	switch (eType)
	{
		case DT_CHOKE:
		{
			char* ChokeSounds[] = { "choke.wav", "choke2.wav" };
	
			int nSize = (sizeof(ChokeSounds)/sizeof(ChokeSounds[0])) - 1;
			strcat(s_FileBuffer, ChokeSounds[GetRandom(0, nSize)]);
		}
		break;

		case DT_ELECTROCUTE:
		{
			char* Sounds[] = { "playerelec1.wav", "playerelec2.wav" };
	
			int nSize = (sizeof(Sounds)/sizeof(Sounds[0])) - 1;
			strcat(s_FileBuffer, Sounds[GetRandom(0, nSize)]);
		}
		break;

		default:
		{
			char* PainSounds[] =  { "pain1.wav", "pain2.wav", "pain3.wav", "pain4.wav", "pain5.wav" };
	
			int nSize = (sizeof(PainSounds)/sizeof(PainSounds[0])) - 1;
			strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetDeathSound
//
//	PURPOSE:	Determine what death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetDeathSound()
{
	switch(m_nMode)
	{
		case PM_MODE_MCA_SA : 
		case PM_MODE_MCA_AO : 
		case PM_MODE_MCA_AP :
		case PM_MODE_MCA_UE :
			return GetMechDeathSound();
		break;

		case PM_MODE_KID :	  
			return GetKidDeathSound();
		break;

		case PM_MODE_FOOT :	  
		default : 
			break;
		break;
	}

	return GetOnFootDeathSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetMechDeathSound
//
//	PURPOSE:	Determine what SA death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetMechDeathSound()
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\Mech\\");

	char* DeathSounds[] =  { "death1.wav", "death2.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}
/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetAPDeathSound
//
//	PURPOSE:	Determine what AP death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetAPDeathSound()
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Predator\\");

	char* DeathSounds[] =  { "death1.wav", "death2.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetAODeathSound
//
//	PURPOSE:	Determine what AO death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetAODeathSound()
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Ordog\\");

	char* DeathSounds[] =  { "death1.wav", "death2.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetUEDeathSound
//
//	PURPOSE:	Determine what UE death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetUEDeathSound()
{
	strcpy(s_FileBuffer, "Sounds\\Player\\Enforcer\\");

	char* DeathSounds[] =  { "death1.wav", "death2.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetKidDeathSound
//
//	PURPOSE:	Determine what Kid death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetKidDeathSound()
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\Kid\\");

	char* DeathSounds[] =  { "death1.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetOnFootDeathSound
//
//	PURPOSE:	Determine what OnFoot death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerMode::GetOnFootDeathSound()
{
	SAFE_STRCPY(s_FileBuffer, "Sounds\\Player\\OnFoot\\");

	char* DeathSounds[] =  { "death1.wav", "death2.wav", "death3.wav", "death4.wav", "death5.wav" };
	
	int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
	strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::GetDimsScale
//
//	PURPOSE:	Get the dims scale for the particular mode
//
// ----------------------------------------------------------------------- //

DFLOAT CPlayerMode::GetDimsScale() const
{
	DFLOAT fScale = 1.0f;

	switch(m_nMode)
	{
		case PM_MODE_MCA_AP : 
			fScale = 0.8f;
		break;

		case PM_MODE_MCA_UE : 
			fScale = 1.0f;
		break;

		case PM_MODE_MCA_AO : 
			fScale = 1.0f;
		break;

		case PM_MODE_MCA_SA : 
			fScale = 1.2f;
		break;

		case PM_MODE_KID    : 
			fScale = 1.0f;
		break;

		case PM_MODE_FOOT :	  
			fScale = 1.1f;
		break;

		default : break;
	}

	return fScale;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerMode::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_nModelId);
	pServerDE->WriteToMessageByte(hWrite, m_nMode);
	pServerDE->WriteToMessageByte(hWrite, m_nArmor);
	pServerDE->WriteToMessageByte(hWrite, m_nVehicleHandling);
	pServerDE->WriteToMessageByte(hWrite, m_bBipedal);
	pServerDE->WriteToMessageFloat(hWrite, m_fJumpSpeed);
	pServerDE->WriteToMessageFloat(hWrite, m_fRunVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fWalkVel);
	pServerDE->WriteToMessageFloat(hWrite, m_FovX);
	pServerDE->WriteToMessageFloat(hWrite, m_FovY);
	pServerDE->WriteToMessageVector(hWrite, &m_vCameraOffset);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMode::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerMode::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nModelId			= pServerDE->ReadFromMessageByte(hRead);
	m_nMode				= pServerDE->ReadFromMessageByte(hRead);
	m_nArmor			= pServerDE->ReadFromMessageByte(hRead);
	m_nVehicleHandling	= pServerDE->ReadFromMessageByte(hRead);
	m_bBipedal			= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_fJumpSpeed		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRunVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_fWalkVel			= pServerDE->ReadFromMessageFloat(hRead);
	m_FovX				= pServerDE->ReadFromMessageFloat(hRead);
	m_FovY				= pServerDE->ReadFromMessageFloat(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vCameraOffset);
}