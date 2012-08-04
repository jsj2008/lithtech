// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_H__
#define __AI_HUMAN_H__

#include "AIClassFactory.h"
#include "AI.h"
#include "AnimationProp.h"
#include "AIStrategy.h"
#include "Searchable.h"

LINKTO_MODULE( AIHuman );

class CHumanAttachmentsPlugin;
class CAttachmentsPlugin;
class CAIHumanStrategy;
class CAIHumanState;
class IMovementModifier;

// Statics

extern LTFLOAT s_fSenseUpdateBasis;


class CAIHuman : public CAI
{
	typedef CAI super;

	public : // Public methods

		// Ctors/Dtors/etc

		CAIHuman();
		virtual ~CAIHuman();

		// Type

		enum eHumanType
		{
			eHT_Invalid,		// Broken Human
			eHT_Ground,			// Walking/running human
			eHT_Swimming,		// Underwater/swimming human
			eHT_Paratrooping,	// Falling human
			eHT_Hovering,		// Hovering human
		};

		void SetHumanType( eHumanType eInstance ){ m_eHumanType = eInstance; }

        LTBOOL IsSwimming() { return ( m_eHumanType == eHT_Swimming ); }
        LTBOOL IsParatrooping() { return ( m_eHumanType == eHT_Paratrooping ); }
		LTBOOL IsHovering() { return ( m_eHumanType == eHT_Hovering ); }
		
		eHumanType m_eHumanType;

		// Attachments

		virtual void CreateAttachments();

		// Senses

        virtual LTBOOL IsObjectVisibleFromKnee(ObjectFilterFn fn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */);
        virtual LTBOOL IsPositionVisibleFromKnee(ObjectFilterFn fn, const LTVector& vSourcePosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */);

		// Weapons

        virtual LTVector GetWeaponPosition(CWeapon *pWeapon);
		EnumAnimProp GetCurrentWeaponProp();

		// Query.

		virtual LTBOOL CanSearch();

		// Movement methods

		void SyncPosition()
		{
			m_bSyncPosition = LTTRUE;
		}

		void SetMovementModifier( IMovementModifier* pMovementModifier );

		LTVector GetKneePosition() const;

		void Turn(LTFLOAT fRadians);

		LTFLOAT GetRotationInterpolation(LTFLOAT fTime) { return (LTFLOAT)sqrt(fTime); }

		// Doors

		void OpenDoors(HOBJECT hDoor1, HOBJECT hDoor2);
		void KickDoors(HOBJECT hDoor1, HOBJECT hDoor2);
		void MarkDoors(HOBJECT hDoor1, HOBJECT hDoor2);
		void UnmarkDoors();
		void CloseDoors();
		LTBOOL HasLastDoors() const { return ( m_hLastDoor[0] || m_hLastDoor[1] ); }
		LTBOOL CanCloseDoors();

/*
		// Sleeping stuff

		void SetSleeping(LTBOOL bSleeping);
*/
		// Death stuff

		virtual void PlayDeathSound();
		virtual HMODELANIM GetDeathAni(LTBOOL bFront);
		const char* GetCrouchDeathAni();
		const char* GetProneDeathAni();
		virtual LTBOOL WasSilentKill();
		virtual EnumAIStateType GetBodyState();

		// Recoil

		void SetCanShortRecoil(LTBOOL bCanShortRecoil) { m_bCanShortRecoil = bCanShortRecoil; }
		LTBOOL CanShortRecoil() const { return m_bCanShortRecoil; }

		// Marking
		
		LTBOOL		HasOnMarkingString() const { return !!m_hstrCmdOnMarking; }
		void		SetOnMarkingString(char* szCmd) { FREE_HSTRING(m_hstrCmdOnMarking); m_hstrCmdOnMarking = g_pLTServer->CreateString(szCmd); }
		const char* GetOnMarkingString() const { return g_pLTServer->GetStringData(m_hstrCmdOnMarking); }
		void		ClearOnMarkingString() { FREE_HSTRING(m_hstrCmdOnMarking); m_hstrCmdOnMarking = LTNULL; }

		// Hitpoints

		void BoostHitpoints(LTFLOAT fFactor)
		{
			m_damage.SetHitPoints(m_damage.GetHitPoints()*fFactor);
			m_damage.SetMaxHitPoints(m_damage.GetMaxHitPoints()*fFactor);
		}

		// Strategy methods

		CAIHumanStrategy* AI_FACTORY_NEW_Strategy(EnumAIStrategyType eStrategyType);

		// State methods

		CAIHumanState* AI_FACTORY_NEW_State(EnumAIStateType eStateType);
		virtual void SetState(EnumAIStateType eState, LTBOOL bUnlockAnimation = LTTRUE);
		virtual void ClearAndSetState(EnumAIStateType eState, LTBOOL bUnlockAnimation = LTTRUE);
		
		// [RP] 9/15/02 - Ultra mega super hack to handle transitioning while carrying Cate
		void			TransitioningInMultiplayerWhileCarryingCateTeleportHack( const LTVector &vPos );

	protected : // Protected member functions

		// Engine methods

		virtual LTBOOL ReadProp(ObjectCreateStruct *pInfo);
        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Character stuff

		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);
		virtual void DestroyArmor();
		virtual void CreateArmor();

		// Handlers

        virtual void HandleGadget(uint8 nAmmoID);
		virtual void HandleDamage(const DamageStruct& damage);
		virtual bool HandleCommand(const CParsedMsg &cMsg);
		virtual void HandleTeleport(const LTVector& vPos);
		virtual void HandleTeleport(TeleportPoint* pTeleportPoint);

		virtual void StartDeath();
		virtual void SetUnconscious(bool bUnconscious);

		// Updates

		virtual void PreUpdate();
		virtual void UpdateAnimation();
		virtual void UpdateMovement();
		virtual void UpdateTarget();
		virtual void UpdateNodes();
		virtual void UpdateCharacterFx();

		// Handle the potential goal additions when shot.

		bool	HandleResurrectingGoalConditions(const DamageStruct& damage);

		// 
		LTVector	GetApproximateWeaponPosition(CWeapon* pWeapon);
		LTVector	GetTrueWeaponPosition(CWeapon* pWeapon);

		void SetAIType( const char* const szType );

	protected : // Protected methods

		// Brain methods

		virtual bool	SetBrain( char const* pszBrain );

		void SpawnHolsteredItems();
		void HolsterSpawnedItems();

		// State methods

		void		 ClearState();

	protected : // Member Variables

		// State

		CAIHumanState*	m_pHumanState;			// Current state

		// Movement

		LTBOOL		m_bForceGround;				// Force AI to ground?

		LTVector	m_vLastFindFloorPos;		// Last pos where we called FindFloor.

		LTBOOL		m_bPosDirty;				// Is our position dirty? (do we need to refind the ground)

		LTBOOL		m_bSyncPosition;			// Sync position

		// Node handle cache

		HMODELNODE	m_hHeadNode;

		// Recoil stuff

		LTBOOL		m_bCanShortRecoil;			// Can we do short recoils?

		// Door

		LTObjRef	m_hLastDoor[2];				// Last doors opened.
		LTVector	m_vLastDoorPos[2];

		// Default Attachments

		LTBOOL		m_bUseDefaultAttachments;

		IMovementModifier* m_pMovementModifier;
};

// Plugin
#ifndef __PSX2
class CAIHumanPlugin : public CAIPlugin
{
	public:

		CAIHumanPlugin();
		virtual ~CAIHumanPlugin();

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
		virtual CAttachmentsPlugin* GetAttachmentsPlugin();

	private :

		CHumanAttachmentsPlugin*	m_pHumanAttachmentsPlugin;
		CWeaponMgrPlugin		m_WeaponMgrPlugin;
		CModelButeMgrPlugin		m_ModelButeMgrPlugin;
		CSearchItemPlugin		m_SearchItemPlugin;

};
#endif
#endif // __AI_HUMAN_H__
