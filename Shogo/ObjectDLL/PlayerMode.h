// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMode.h
//
// PURPOSE : PlayerObj helper class - Definition
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_MODE_H__
#define __PLAYER_MODE_H__

#include "basedefs_de.h"
#include "DamageTypes.h"
#include "PlayerModeTypes.h"
#include "ModelFuncs.h"

#define PM_MCA_MOD_NONE				0	// Modifier 1
#define PM_MCA_MOD_1				1	// Modifier 1
#define PM_MCA_MOD_2				2	// Modifier 2
#define PM_MCA_MOD_3				3	// Modifier 3

#define PM_MCA_BSPEED_1				320.0	// MCA Bipedal speed 1
#define PM_MCA_BSPEED_2				380.0	// MCA Bipedal speed 2
#define PM_MCA_BSPEED_3				400.0	// MCA Bipedal speed 3

#define PM_MCA_JSPEED_1				550.0	// MCA Jump speed 1
#define PM_MCA_JSPEED_2				550.0	// MCA Jump speed 2
#define PM_MCA_JSPEED_3				550.0	// MCA Jump speed 3

#define PM_MCA_VSPEED_1				420.0	// Vehicle speed 1
#define PM_MCA_VSPEED_2				460.0	// Vehicle speed 2
#define PM_MCA_VSPEED_3				500.0	// Vehicle speed 3

#define PM_FOOT_WALKSPEED			200.0	// Walk speed on foot
#define PM_FOOT_RUNSPEED			400.0	// Run speed on foot
#define PM_FOOT_JUMPSPEED			550.0	// Jump speed on foot


class CPlayerObj;
class CServerDE;

// Class Definition...

class CPlayerMode
{
	public :

		// Constructor
		CPlayerMode();

		DBOOL Init(CPlayerObj* pMyObj)
		{
			m_pMyObj = pMyObj;
			return DTRUE;
		}

		// Member Functions
		
		DFLOAT	GetMass();
		DFLOAT	GetMaxHitPts();
		DFLOAT	GetBaseHitPts();
		DFLOAT	GetMaxArmorPts();
		DFLOAT	GetBaseArmorPts();
		DVector GetCameraOffset()	const { return m_vCameraOffset; }
		DFLOAT  GetJumpSpeed()		const { return m_fJumpSpeed; }
		char*	GetModelFilename()	const { return GetModel(m_nModelId); }
		char*	GetSkinFilename()	const;
		DBYTE	GetMode()			const { return m_nMode; }
		DBYTE	GetModelId()		const { return m_nModelId; }

		char*  GetDamageSound(DamageType eType);
		char*  GetDeathSound();
		DFLOAT GetRunVelocity() const;
		DFLOAT GetWalkVelocity() const;
		DFLOAT GetDimsScale() const;

		DBYTE SetMode(DBYTE nNewMode, DBOOL bBipedal);
		void  SetJumpSpeed(DFLOAT fSpeed) { m_fJumpSpeed = fSpeed; }
		void  SetCameraOffset(DVector vOffset);
		void  SetRunVelocity(DFLOAT fVel)  { m_fRunVel = fVel; }
		void  SetWalkVelocity(DFLOAT fVel) { m_fWalkVel = fVel; }

		DBOOL IsBipedalMode() const { return m_bBipedal;}
		DBOOL IsOnFoot() const;

		DFLOAT AdjustFriction(DFLOAT fBaseFriction) const;
		DFLOAT AdjustArmor(DFLOAT fBaseArmor) const;

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

	private :

		void SetModeKid();
		void SetModeOnFoot();
		void SetModeMcaAP(DBOOL bBipedal);
		void SetModeMcaUE(DBOOL bBipedal);
		void SetModeMcaAO(DBOOL bBipedal);
		void SetModeMcaSA(DBOOL bBipedal);

//		char* GetSADamageSound(DamageType eType);
//		char* GetAPDamageSound(DamageType eType);
//		char* GetUEDamageSound(DamageType eType);
//		char* GetAODamageSound(DamageType eType);
		char* GetMechDamageSound(DamageType eType);
		char* GetKidDamageSound(DamageType eType);
		char* GetOnFootDamageSound(DamageType eType);

//		char* GetSADeathSound();
//		char* GetAPDeathSound();
//		char* GetUEDeathSound();
//		char* GetAODeathSound();
		char* GetMechDeathSound();
		char* GetKidDeathSound();
		char* GetOnFootDeathSound();

		
	private : // Member Variables

		CPlayerObj* m_pMyObj;		// Object I'm associated with

		DBYTE	m_nModelId;			// What model do we use?
		DBYTE	m_nMode;			// Current player mode
		DBOOL	m_bBipedal;			// MCA Only (bipedal or vehical)
		DFLOAT	m_fJumpSpeed;		// Jump velocity
		DFLOAT	m_fRunVel;			// Run velocity
		DFLOAT	m_fWalkVel;			// Walk velocity

		DVector m_vCameraOffset;	// Camera offset from player
		DFLOAT	m_FovX;				// Camera field of view in X direction
		DFLOAT	m_FovY;				// Camera field of view in Y direction

		// MCA Modifiers...

		DBYTE	m_nArmor;
		DBYTE	m_nVehicleHandling;
};

#endif // __PLAYER_MODE_H__