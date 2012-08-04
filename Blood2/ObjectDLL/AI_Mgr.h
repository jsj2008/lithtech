
 /****************************************************************************
 ;
 ;	MODULE:		AI_Mgr.h
 ; ;
 ;	HISTORY:	Created by SCHLEGZ on 3/18/98 10:11:58 PM
 ;
 ;	COMMENT:	Copyright (c) 1997, Monolith Productions Inc.
 ;
 ****************************************************************************/

#ifndef __AI_MGR_H__
#define __AI_MGR_H__

#include "basecharacter.h"
#include "cpp_aggregate_de.h"
#include "InventoryMgr.h"
#include "Anim_Sound.h"

#include "AI_Shared.h"
#include "AIScriptList.h"
#include "PathList.h"
#include "physicalattributes.h"

// Don't include this here, causes conflict if you include this header in any function that uses PlaySound
// GK
//#include <windows.h> 

#define ADD_BASEAI_AGGREGATE() \
	ADD_STRINGPROP(SpotTriggerTarget, "") \
	ADD_STRINGPROP(SpotTriggerMessage, "") \
	ADD_STRINGPROP(TriggerRelayTarget, "") \

#define	SIGHT				0
#define SOUND				1
#define SMELL				2
#define SENSE				3
#define THREAT				4
#define HEALTH				5
#define NUM_STIMULI			HEALTH + 1

#define STIM_SIGHT			1
#define STIM_SOUND			2
#define STIM_SMELL			3
#define STIM_SENSE			4

#define STATE_AttackClose			0
#define STATE_AttackFar				1
#define STATE_SearchVisualTarget	2
#define STATE_Escape_Hide			3
#define STATE_Escape_RunAway		4
#define STATE_Idle					5
#define STATE_GuardLocation			6
#define STATE_FindHealth			7
#define STATE_FindAmmo				8
#define STATE_Hide					9
#define STATE_Dodge					10
#define STATE_Teleport				11
#define STATE_EnemyAttach			12
#define STATE_AssistAlly			13
#define STATE_SearchSmellTarget		14
#define STATE_Special1				15
#define STATE_Special2				16
#define STATE_Special3				17
#define STATE_Special4				18
#define STATE_Special5				19
#define STATE_Special6				20
#define STATE_Special7				21
#define STATE_Special8				22
#define STATE_Special9				23
#define STATE_Passive				24

#define STATE_Dying				100
#define STATE_Script			101
#define STATE_Recoil			102
#define STATE_JumpOverObj		103
#define STATE_CrawlUnderObj		104
#define STATE_WalkAroundObj		105
#define STATE_RunAroundObj		106
#define STATE_WalkToPos			107
#define STATE_RunToPos			108

#define STATE_Inactive			999

// Metacmds
#define	MC_IDLE				0
#define MC_WALK				1
#define MC_RUN				2
#define MC_JUMP				3
#define	MC_CROUCH			4
#define MC_CRAWL			5
#define MC_SWIM				6
#define MC_FIRE_STAND		7
#define MC_FIRE_WALK		8
#define MC_FIRE_RUN			9
#define MC_FIRE_JUMP		10
#define MC_FIRE_CROUCH		11
#define MC_FIRE_CRAWL		12
#define MC_ROLL_FORWARD		13
#define MC_ROLL_BACKWARD	14
#define MC_ROLL_LEFT		15
#define MC_ROLL_RIGHT		16
#define MC_TAUNT_BEG		17
#define MC_TAUNT_BOLD		18
#define MC_RECOIL			19
#define MC_DEAD				20
#define MC_DODGE_LEFT		21
#define MC_DODGE_RIGHT		22	
#define MC_EXTRA			23
#define MC_LAYPROXIMITY		24
#define MC_SPECIAL			25

//Flags
#define FLAG_CRAWL			0x00000001
#define FLAG_JUMP			0x00000002
#define FLAG_LIMP			0x00000004
#define FLAG_DODGE			0x00000008
#define FLAG_LIMBLOSS		0x00000010
#define FLAG_ALWAYSRECOIL	0x00000020
#define FLAG_ALWAYSGIB		0x00000040
#define FLAG_CANASSIST		0x00000080
#define	FLAG_ALWAYSCLEAR	0x00000100	// 10/26/1998 --Loki
#define FLAG_NOAMMOCOLLECT	0x00000200  // 02/10/1999 --Loki
#define FLAG_NIGHTMAREDEATH 0x00000400	// 02/25/1999 --Loki
#define FLAG_NEVERGIB		0x00000800	// 02/25/1999 --Loki

//obstruction flags
#define BLOCK_UPPER			0x00000001
#define BLOCK_MIDDLE		0x00000002
#define BLOCK_LOWER			0x00000004
#define BLOCK_RIGHT			0x00000010
#define BLOCK_CENTER		0x00000020
#define BLOCK_LEFT			0x00000040

//find object in radius flags
#define FIND_AVOID_TARGET	0x00000001
#define FIND_VISIBLE		0x00000002
#define FIND_FACE_OBJECT	0x00000004
#define FIND_SPECIFIC_OBJ	0x00000008

#define TRIGGER_PLAY_SOUND		        SCRIPT_PLAY_SOUND
#define TRIGGER_SET_STATE				SCRIPT_SET_STATE
#define TRIGGER_PLAY_ANIMATION			SCRIPT_PLAY_ANIMATION
#define TRIGGER_TARGET_OBJECT			SCRIPT_TARGET_OBJECT

#define TRIGGER							"TRIGGER"
#define TRIGGER_SOUND					"SOUND_TRIGGER"

#define TRIGGER_SCRIPT					"SCRIPT"
#define TRIGGER_STYPE_INTERRUPTABLE		"INTERRUPTABLE"

#define SCRIPT_MOVEMENT_WALK			"WALK"
#define SCRIPT_MOVEMENT_RUN				"RUN"
#define SCRIPT_MOVEMENT_FLY				"FLY"

//dodge flags
#define FORWARD				0x00000001
#define BACKWARD			0x00000002
#define RIGHT				0x00000004
#define LEFT				0x00000008

#define ROLL				0x00000010
#define DODGE				0x00000020

//play sound flags
#define	PLAY_INTERRUPT		0x00000001
#define PLAY_WAIT			0x00000002


#define AI_SCRFLG_LOOP					(1<<0)
#define AI_SCRFLG_INT					(1<<1)
#define AI_SCRFLG_OPPORTFIRE			(1<<2)

// Amount of time the ai must wait before going into the idle state from another state
#define WAITFORIDLETIME		120.0f

//externs
class AI_Shared;
class CInventoryMgr;
class CAnimation;

class AI_Mgr : public CBaseCharacter
{
public:

	AI_Mgr();
	~AI_Mgr();

	DBOOL		InitStatics(CAnim_Sound* pAnim_Sound);

	//efficient means to track AI in a level
	DBOOL	m_bCabal;

	static DLink	m_CabalHead;
	static DDWORD	m_dwNumCabal;

	static DLink	m_MonsterHead;
	static DDWORD	m_dwNumMonster;

	DLink			m_Link;

	static DFLOAT	m_fLastFrame;

private:

	void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
	void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

protected:

    AI_Shared   AIShared;					// Shared functions

    // Class pointers
    CAnim_Sound*    m_pAnim_Sound;			// Animation Class

    DFLOAT m_fAIMass;

    DFLOAT m_fAIHitPoints;
	DFLOAT m_fAIRandomHP;
    DFLOAT m_fAIArmorPoints;

    char	m_szAIState[32];
    char	m_szAIWeapon[5][32];
	DFLOAT	m_fAIBullets;
	DFLOAT	m_fAIBMG;
	DFLOAT	m_fAIShells;
	DFLOAT	m_fAIGrenades;
	DFLOAT	m_fAIRockets;
	DFLOAT	m_fAIFlares;
	DFLOAT	m_fAICells;
	DFLOAT	m_fAICharges;
	DFLOAT	m_fAIFuel;
	DFLOAT	m_fProxBombs;
	int		m_nAIStrength;

    // Target object vars					
	HOBJECT		m_hTarget;					
    DVector     m_vTargetPos;				
	HOBJECT		m_hTrackObject;				// Object to Track! Smells for now, could be Weapons,Ammo,Health
    DVector		m_vTrackObjPos;
	DVector		m_vGuardPos;

	DLink*		m_pCurSmell;
	HOBJECT		m_hCurSmell;
	DVector		m_vSmellPos;

	DVector		m_vDestPos;

	float		m_fStimuli[NUM_STIMULI];				//Stimuli set

    int			m_nState;
    int			m_nLastState;
    
	//conditions
	int			m_nDodgeFlags;

	HSTRING		m_hstrSpotTriggerTarget;
	HSTRING		m_hstrSpotTriggerMessage;
	HSTRING		m_hstrTriggerRelayTarget;

    // Adjustable vars
    DFLOAT		m_fHearingDist;				// How far can we hear 
    DFLOAT		m_fSensingDist;				// How far can we sense a player
    DFLOAT		m_fSmellingDist;			// How far can we smell
    DFLOAT		m_fSeeingDist;				// How far can we see
				
  	DFLOAT		m_fWalkSpeed;				// How fast AI walks
	DFLOAT		m_fRunSpeed;			    // How fast AI runs
	DFLOAT		m_fJumpSpeed;				// How fast AI jumps
	DFLOAT		m_fRollSpeed;			
																
    DFLOAT		m_fTimeStart;				// Temp Var
    DFLOAT		m_fLoadTimeStart;			// Temp Var

	DDWORD		m_nCorpseType;

	DFLOAT		m_fAttackLoadTime;
									
    DBOOL		m_bAnimating;
	DBOOL		m_bJumping;

    int			Metacmd;
	int			m_nCurMetacmd;

	DDWORD		m_dwFlags;

	int			m_nBlockFlags;

	HSOUNDDE	m_hCurSound;

	CServerDE*	m_pServerDE;
	DBOOL		m_bSetShutdown;

	DFLOAT		m_fLastUpdate;

	int			m_nInjuredLeg;

	HOBJECT		m_hEnemyAttach;

	DVector		m_vScale;				//size and weight of AI...adds variety

	DBOOL		m_bMoveToGround;

	DBOOL		m_bRemoveMe;			// Remove this object on the next update

	DFLOAT		m_fLastLedgeCheck;

    DDWORD		m_nCurAnimation;		// The current animation
	DVector		m_vDims;

	DBOOL		m_bStartFire;
	HOBJECT		m_hFireSource;

	DBOOL		m_bProceedAttach;

	float		m_fWaitForIdleTime;
	DBOOL		m_bHasTargeted;

	// Scripting
    
	enum AIScriptMovement
	{
		SM_WALK=0, SM_RUN
	};
    
	DBOOL				m_bUpdateScriptCmd;		// Time to get next script command
	AISCRIPTCMD			m_curScriptCmd;			// The current script command
	CAIScriptList		m_scriptCmdList;		// List of all script commands
	int					m_nScriptCmdIndex;		// Index into m_scriptCmdList
	DDWORD				m_dwScriptFlags;		// Flags used in scripting state

	PathList			m_AIPathList;			// Current list of path points
	AIScriptMovement	m_eScriptMovement;		// Current mode of script movement
	DFLOAT				m_fScriptWaitEnd;		// Time when script wait should end

	
public : // Member functions

	void			SetListenDistance(DFLOAT fDistance)			{ m_fHearingDist = fDistance; }
	void			SetSightDistance(DFLOAT fDistance)			{ m_fSeeingDist = fDistance; }
	void			SetSenseDistance(DFLOAT fDistance)			{ m_fSensingDist = fDistance; }
	void			SetSmellDistance(DFLOAT fDistance)			{ m_fSmellingDist = fDistance; }
					
	void			SetWalkSpeed(DFLOAT fSpeed)					{ m_fWalkSpeed = fSpeed; }
	void			SetRunSpeed(DFLOAT fSpeed)					{ m_fRunSpeed = fSpeed; }
	void			SetJumpSpeed(DFLOAT fSpeed)					{ m_fJumpSpeed = fSpeed; }
	void			SetRollSpeed(DFLOAT fSpeed)					{ m_fRollSpeed = fSpeed; }
					
    virtual int		StateStrToInt(char *pState);
    virtual DDWORD	WeaponStrToInt(char *pWeapon);

	void			DetachFromEnemy()							{ m_hEnemyAttach = DNULL; }
	void			ProceedToAttach()							{ m_bProceedAttach = DTRUE; }

	void			SetCacheDirectory(char* sDir);
	void			CacheSoundFileRange(char* sBase, int nFirst, int nLast);
	void			CacheSoundFile(char* sBase);

	HOBJECT			GetTarget()									{ return m_hTarget; }
protected :

	DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
					
	DBOOL			ReadProp(ObjectCreateStruct *pStruct);
	void			OnStringKey(ArgList* pArgList);
	DBOOL			HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
	virtual DBOOL	HandleDamage();
	void			HandleTouch(HOBJECT hObj);

	virtual	void	SetNewState(int nState);
    DBOOL			SetAnimation(DDWORD nAni);

	void			StopVelocity()
					{
						DVector vVel;
						m_pServerDE->GetVelocity(m_hObject, &vVel);
						vVel.x = vVel.z = 0.0f;
						Move(vVel, MATH_EPSILON);
					}

	//Scripting functions
    void			PlayAnimation(char* pAniName);
    void			SetTarget(char* pTargetName);
    void			SetNewTarget(HOBJECT hNewTarget);
    // Scripting Stuff
    void			UpdateScriptCommand();
    void			SetSetMovementCmd();
    void			SetFollowPathCmd();
    void			UpdateFollowPathCmd();
    void			SetMoveToObjectCmd();
    void			UpdateMoveToObjectCmd();
    void			SetPlaysoundCmd();
    void			SetSetStateCmd();
    void			SetTargetCmd();
    void			SetWaitCmd();
    void			UpdateWaitCmd();
    void			SetPlayAnimationCmd(DBOOL bLooping = DFALSE);
    void			UpdatePlayAnimationCmd();
    // Utility stuff
    void			FacePos(DVector vTargetPos);
    void			FaceObject(HOBJECT hObj);

	void			InsertMe();
               
  	virtual void	InitialUpdate(int nData);
	virtual	DBOOL	AI_Update();

	//State computations
	int				ComputeStimuli();
	DFLOAT			ComputeThreat();
	virtual void	ComputeState(int nStimType = -1)		{ }

	DBOOL			ChooseOpportunityTarget();

	//Helper functions
	DBOOL			CheckObstructed(DVector vDir, DFLOAT fSpeed);
	int				CalculateObstruction(IntersectInfo* IInfo = DNULL);
	DFLOAT			CalculateTurn(IntersectInfo* IInfo = DNULL);
	DBOOL			NavigateObstacle();
	HOBJECT			FindObjectInRadius(HCLASS hObjectTest, DFLOAT fRange, DDWORD dwFlags = 0);
	DBOOL			CanSee( HOBJECT hTarget );
	DBOOL			Move(DVector &vDir, DFLOAT fSpeed);
	DBOOL			Jump(DFLOAT m_fYSpeed, DFLOAT m_fZSpeed);
	virtual DBOOL	Fire(DBOOL bAltFire = DFALSE);
	DBOOL			CheckClearShot(DVector &vPos, DRotation &rRot, DBOOL bAltFire, DDWORD dwFlags = 0);
	DBOOL			IsClearXZ(DVector vDir, DFLOAT fDist);
	int				CalculateDodge(DVector vPos);
	DBOOL			IsLedge(DVector vDir);
	DBOOL			CreateBloodSpurt(int nNodeHit);
	DBOOL			CreateCorpse();
	DBOOL			CreateLimb(DVector vDir, int nLimbType = 0);
	DBOOL			CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage);
	void			RemoveMe();

	//Action functions
	virtual void	MC_Idle();
	virtual	void	MC_Talk();
	virtual void	MC_Walk();
	virtual void	MC_Run();
	virtual void	MC_Jump();
	virtual void	MC_Crouch();
	virtual void	MC_Crawl();
	virtual void	MC_Swim();
	virtual void	MC_Fire_Stand();
	virtual void	MC_Fire_Walk();
	virtual void	MC_Fire_Run();
	virtual void	MC_Fire_Jump();
	virtual void	MC_Fire_Crouch();
	virtual void	MC_Fire_Crawl();
	virtual void	MC_Roll_Forward(DBOOL bLoop = DFALSE);
	virtual void	MC_Roll_Backward();
	virtual void	MC_Roll_Left();
	virtual void	MC_Roll_Right();
	virtual void	MC_Dodge_Left();
	virtual void	MC_Dodge_Right();
	virtual void	MC_Taunt_Beg();
	virtual void	MC_Taunt_Bold();
	virtual void	MC_Recoil();
	virtual void	MC_Dead();
	virtual void	MC_Special(int nIndex = 0);

	virtual void	MC_FaceTarget();
	virtual void	MC_FaceTrackObj();
	virtual void	MC_FacePos(DVector vPos);
	virtual void	MC_BestWeapon();
	virtual void	MC_LayProximity();

	virtual void	MC_Extra(const char *lpszParam = DNULL);

	virtual void	Script_Walk();
	virtual void	Script_Fire_Walk();
	virtual void	Script_Run();
	virtual void	Script_Fire_Run();
	virtual DBOOL	Script_Fire_Stand();

	//State functions
	virtual void	AI_STATE_Dying();
    virtual void	AI_STATE_Script();
	virtual void	AI_STATE_Recoil();
	virtual void	AI_STATE_WalkAroundObj();
	virtual void	AI_STATE_RunAroundObj();
	virtual void	AI_STATE_JumpOverObj();
	virtual void	AI_STATE_CrawlUnderObj();
	virtual void	AI_STATE_WalkToPos();
	virtual void	AI_STATE_RunToPos();

	virtual void	AI_STATE_AttackClose()				{ AI_STATE_AttackFar(); }
	virtual void	AI_STATE_AttackFar()				{ AI_STATE_AttackClose(); }
	virtual void	AI_STATE_SearchVisualTarget();
	virtual void	AI_STATE_SearchSmellTarget();
	virtual void	AI_STATE_Escape_Hide()				{ }
	virtual void	AI_STATE_Escape_RunAway()			{ }
	virtual void	AI_STATE_GuardLocation();
	virtual void	AI_STATE_Idle();
	virtual void	AI_STATE_Dodge()					{ }
	virtual void	AI_STATE_Teleport()					{ }
	virtual void	AI_STATE_EnemyAttach()				{ }
	virtual void	AI_STATE_AssistAlly();
	virtual void	AI_STATE_FindAmmo();
	virtual void	AI_STATE_FindHealth();
	virtual void	AI_STATE_Special1()					{ }
	virtual void	AI_STATE_Special2()					{ }
	virtual void	AI_STATE_Special3()					{ }
	virtual void	AI_STATE_Special4()					{ }
	virtual void	AI_STATE_Special5()					{ }
	virtual void	AI_STATE_Special6()					{ }
	virtual void	AI_STATE_Special7()					{ }
	virtual void	AI_STATE_Special8()					{ }
	virtual void	AI_STATE_Passive();
};

#endif __AI_MGR_H__
