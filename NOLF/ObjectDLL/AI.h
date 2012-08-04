// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_H__
#define __AI_H__

#include "Character.h"
#include "Attachments.h"
#include "AIButeMgr.h"
#include "AIReactions.h"
#include "AISounds.h"

class AIGroup;
class CAIState;
class CAITarget;
class CAISense;
class CAISenseMgr;
class CAISenseRecorder;
class TeleportPoint;

// Defines

#define AI_MAX_WEAPONS		(4)
#define AI_MAX_OBJECTS		(4)

// Classes

class CAI : public CCharacter
{
	public : // Public methods

		// Ctors/Dtors/etc

		CAI();
		virtual ~CAI();

		// Object info

		HOBJECT	GetObject() const { return m_hObject; }

        const char* GetName() const { return g_pLTServer->GetObjectName(m_hObject); }

		uint32 GetDebugLevel() const { return m_nDebugLevel; }

        const LTVector& GetPosition() const { return m_vPos; }
        virtual LTVector GetWeaponPosition(CWeapon *pWeapon);
        const LTVector& GetEyePosition() { return m_vEyePos; }
        const LTVector& GetEyeForward() { return m_vEyeForward; }
		const LTVector& GetTorsoPos() { return m_vTorsoPos; }

        const LTVector& GetDims() const { return m_vDims; }
        LTFLOAT GetRadius() const { return m_fRadius; }

        const LTRotation& GetRotation() const { return m_rRot; }

        virtual LTFLOAT GetRotationInterpolation(LTFLOAT fTime) { return fTime; }

        const LTVector & GetUpVector() const { return m_vUp; }
        const LTVector & GetRightVector() const { return m_vRight; }
        const LTVector & GetForwardVector() const { return m_vForward; }

		// Visiblity. Note: these functions can fill in useful values (distance, dot product, direction) of the point/object in question,
		// in order to save you from having to recompute them yourself (since they can be expensive). HOWEVER, you cannot use the values
		// if the point/object is NOT visible (since it may have skipped out of the function before comuting some of the values). ALSO,
		// in the case of checking to see if an object is visible, there is a degenerate case of the Source Position being INSIDE the
		// object, which does return TRUE, although the values will be somewhat meaningless. They will be fudged to a distance of MATH_EPSILON,
		// a dot product of 1, and a direction of the forward vector of the AI.

        virtual LTBOOL IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

        virtual LTBOOL IsObjectPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

        virtual LTBOOL IsPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);
        virtual LTBOOL IsPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL);

		// Group methods

		AIGroup* GetGroup() { return m_pGroup; }
		void SetGroup(AIGroup* pGroup) { m_pGroup = pGroup; }

		// State methods

        void ChangeState(const char* szNextStateMessage, ...);

		// Senses functions

		LTFLOAT GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		CAISenseMgr* GetSenseMgr() { return m_pSenseMgr; }
		LTBOOL IsAlert() const;

		// Enemy Target stuff

		void Target(HOBJECT hObject);
        LTBOOL FaceTarget();
		CAITarget* GetTarget();
        LTBOOL HasTarget();

		// Awareness

        LTFLOAT GetAwareness() { return m_fAwareness; }
        void SetAwareness(LTFLOAT fAwareness) { m_fAwareness = fAwareness; }

		// Accuracy/lag methods

		void UpdateAccuracy();

        void SetAccuracyModifier(LTFLOAT fModifier, LTFLOAT fTime);
        LTFLOAT GetAccuracy() { return RAISE_BY_DIFFICULTY(m_fAccuracy)-(RAISE_BY_DIFFICULTY(m_fAccuracy)*LOWER_BY_DIFFICULTY(m_fAccuracyModifier)*(m_fAccuracyModifierTimer/m_fAccuracyModifierTime)); }
        LTFLOAT GetLag() { return m_fLag; }

		// Activation

		void UpdateUserFlagCanActivate();

		// Object linking helpers

        void Link(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->CreateInterObjectLink(m_hObject, hObject); }
        void Unlink(HOBJECT hObject) { if ( g_pLTServer && hObject ) g_pLTServer->BreakInterObjectLink(m_hObject, hObject); }

		// Static filter functions for intersect segments

        static LTBOOL DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static LTBOOL BodyFilterFn(HOBJECT hObj, void *pUserData);
        static LTBOOL ShootThroughFilterFn(HOBJECT hObj, void *pUserData);
        static LTBOOL ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData);
        static LTBOOL SeeThroughFilterFn(HOBJECT hObj, void *pUserData);
        static LTBOOL SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData);

        LTBOOL CanSeeThrough() { return m_bSeeThrough; }
        LTBOOL CanShootThrough() { return m_bShootThrough; }

		// Orientation/movement methods

        LTBOOL FaceObject(HOBJECT hObj);
        LTBOOL FaceDir(const LTVector& vTargetDir);
        LTBOOL FacePos(const LTVector& vTargetPos);

		// Sound methods

		virtual void PlaySound(AISound ais) { PlayDialogSound(GetSound(this, ais), GetCharacterSoundType(ais)); }
		virtual void PlaySound(char *pSound) { PlayDialogSound(pSound); }
		virtual char* GetDeathSound();
		virtual char* GetDeathSilentSound();
		virtual char* GetDamageSound(DamageType eType);
		virtual void PlayDamageSound(DamageType eType) {}

		// Weapon methods

		CWeapon* GetCurrentWeapon() { return m_apWeapons[0]; }
		int GetNumWeapons() { return m_cWeapons; }
		CWeapon* GetWeapon(int iWeapon) { _ASSERT(iWeapon >= 0 && iWeapon < m_cWeapons); return m_apWeapons[iWeapon]; }
		CAttachmentPosition* GetWeaponPosition(int iWeapon) { _ASSERT(iWeapon >= 0 && iWeapon < m_cWeapons); return m_apWeaponPositions[iWeapon]; }

		// Object methods (attachments)

		BaseClass* GetCurrentObject() { return m_apObjects[0]; }
		int GetNumObjects() { return m_cObjects; }
		BaseClass* GetObject(int iObject) { _ASSERT(iObject >= 0 && iObject < m_cObjects); return m_apObjects[iObject]; }
		CAttachmentPosition* GetObjectPosition(int iObject) { _ASSERT(iObject >= 0 && iObject < m_cObjects); return m_apObjectPositions[iObject]; }

		// Damage stuff

        void SetInvincible(LTBOOL bInvincible) { m_damage.SetCanDamage(!bInvincible); }
        LTFLOAT ComputeDamageModifier(ModelNode eModelNode);
		virtual LTBOOL CanBeDamagedAsAttachment();

		// Senses/Reactions

        virtual void DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual);
		void DidReaction(CAISense* pAISense, BOOL bIndividual);

		HSTRING GetGroupSenseReaction(CAISense* pAISense) { return GetSenseReaction(&m_GroupReactions, pAISense, m_bFirstReaction); }
		HSTRING GetIndividualSenseReaction(CAISense* pAISense)
		{
			HSTRING hstrReaction = GetSenseReaction(&m_IndividualReactions, pAISense, m_bFirstReaction);
			if ( hstrReaction ) return hstrReaction;
			else return GetGroupSenseReaction(pAISense);
		}

		// Cinematic trigger

		LTBOOL IsControlledByCinematicTrigger() const { return !!m_hCinematicTrigger; }
		void StopCinematicTrigger()
		{
            SendTriggerMsgToObject(this, m_hCinematicTrigger, LTFALSE, "OFF");
			Unlink(m_hCinematicTrigger);
			m_hCinematicTrigger = LTNULL;
		}

		void LinkCinematicTrigger(HOBJECT hCinematicTrigger);
		void UnlinkCinematicTrigger(HOBJECT hCinematicTrigger);

		// Client solid

		void SetClientSolid(LTBOOL bClientSolid) 
		{
			uint32 nFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			g_pLTServer->SetObjectUserFlags(m_hObject, bClientSolid ? nFlags | USRFLG_AI_CLIENT_SOLID : nFlags & ~USRFLG_AI_CLIENT_SOLID);
		}


	protected : // Protected methods

		friend class CAISenseMgr;

		// Update methods

		virtual void PreUpdate();
		virtual void Update();
		virtual void PostUpdate();

		virtual void UpdateAnimation();
		virtual void UpdateOnGround();
		virtual void UpdateTarget();
		virtual void UpdateMovement() {}
		virtual void UpdatePosition();
		virtual void UpdateNodes() {}
		virtual void UpdateState();
		virtual void UpdateCharacterFx() {}
		virtual void UpdateMusic();

		// Handler methods

		virtual void HandleTeleport(const LTVector& vPos);
		virtual void HandleTeleport(TeleportPoint* pTeleportPoint);
		virtual void HandleDamage(const DamageStruct& damage);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void HandleModelString(ArgList* pArgList);
		virtual void HandleBrokenLink(HOBJECT hLink);
		virtual void HandleTargetDied() {}
		virtual void HandleTrigger(HOBJECT hSender, const char* szMsg);
        virtual LTBOOL HandleCommand(char **pTokens, int nArgs);
		void HandleCommandParameters(char **pTokens, int nArgs);
		void HandleAttach();
		void HandleDetach();

		// Senses/Reactions

		void HandleSense(CAISense* pAISense);
        HSTRING GetSenseReaction(CAIReactions* pAIReactions, CAISense* pAISense, LTBOOL bFirstReaction);

		// Engine methods

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        virtual LTBOOL  ReadProp(ObjectCreateStruct *pInfo);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

        virtual LTBOOL  ProcessTriggerMsg(const char* pMsg);
        virtual LTBOOL  ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);

		virtual void	InitialUpdate();
        virtual void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		virtual void	CacheFiles();

		virtual void	InitAttachments();

		// Precomputation stuff

		virtual void ComputeSquares();

	protected : // Member Variables

		HSTRING		m_hstrCinematicExtension;
		HSTRING		m_hstrBodySkinExtension;

		// Misc

        LTBOOL       m_bCheapMovement;           // Are we using cheap movement?
        LTBOOL       m_bFirstUpdate;             // Is this before/during our first update?
        char         m_szQueuedCommands[1024];   // Our queued commands
		LTBOOL		m_bInitializedAttachments;

		// Weapons

		CWeapon*				m_apWeapons[AI_MAX_WEAPONS];			// An array of pointers to our weapons
		CAttachmentPosition*	m_apWeaponPositions[AI_MAX_WEAPONS];	// An array of pointers to the attachemnts for the weapons
		int						m_cWeapons;								// How many weapons we have

		// Objects

		BaseClass*				m_apObjects[AI_MAX_OBJECTS];			// An array of pointers to our Objects
		CAttachmentPosition*	m_apObjectPositions[AI_MAX_OBJECTS];	// An array of pointers to the attachemnts for the Objects
		int						m_cObjects;								// How many Objects we have

		// State

        CAIState*    m_pState;                   // Current state
        HSTRING      m_hstrNextStateMessage;     // Next state message
        LTFLOAT      m_fNextStateTime;           // Next state change time

		// Object information

        LTVector    m_vPos;                     // Current pos this frame
        LTRotation  m_rRot;                     // Object's rotation
        LTVector    m_vRight;                   // Object's right vector
        LTVector    m_vUp;                      // Object's up vector
        LTVector    m_vForward;                 // Object's forward vector

        LTVector    m_vEyePos;                  // The position of our eye
        LTVector    m_vEyeForward;              // Our eye's forward vector
		LTVector	m_vTorsoPos;

        LTVector    m_vDims;                    // Object's dims
        LTFLOAT     m_fRadius;                  // Radius of circle circumscribed by our bounding box

        LTRotation  m_rTargetRot;               // Our target rotation
        LTRotation  m_rStartRot;                // Our starting rotation
        LTVector    m_vTargetRight;             // Object's target right vector
        LTVector    m_vTargetUp;                // Object's target up vector
        LTVector    m_vTargetForward;           // Object's target forward vector
        LTBOOL      m_bRotating;                // Are we rotating?
        LTFLOAT     m_fRotationTime;            // Time for our rotation interpolation
        LTFLOAT     m_fRotationTimer;           // Timer for our rotation interpolation
        LTFLOAT     m_fRotationSpeed;           // Our rotation speed

		// Senses

		CAISenseRecorder*	m_pAISenseRecorder;		// The AI sense recorder
		CAISenseMgr*		m_pSenseMgr;			// Sense manager
		CAIReactions		m_IndividualReactions;	// Individual sense reactions
		CAIReactions		m_GroupReactions;		// Group sense reactions
        LTBOOL              m_bFirstReaction;       // Our first reaction? (including goal satisfaction)
		LTFLOAT				m_fSenseUpdateRate;		// How quickly we update our senses.

		// Target

        CAITarget*  m_pTarget;                  // Enemy Target
        LTBOOL      m_bShootThrough;            // Do we shoot through shoot-throughables at our target?
        LTBOOL      m_bSeeThrough;              // Do we see through see-throughables at any stimuli? (VERY EXPENSIVE!!!)

		// Accuracy/lag modififers

        LTFLOAT     m_fBaseAccuracy;
        LTFLOAT     m_fAccuracy;
        LTFLOAT     m_fAccuracyIncreaseRate;
        LTFLOAT     m_fAccuracyDecreaseRate;
        LTFLOAT     m_fAccuracyModifier;
        LTFLOAT     m_fAccuracyModifierTime;
        LTFLOAT     m_fAccuracyModifierTimer;
        LTFLOAT     m_fBaseLag;
        LTFLOAT     m_fLag;
        LTFLOAT     m_fLagIncreaseRate;
        LTFLOAT     m_fLagDecreaseRate;
        LTFLOAT     m_fLagTimer;

		// Awareness

        LTFLOAT     m_fAwareness;               // How quickly we adjust
		LTFLOAT		m_fFOVBias;					// Bias to our FOV

		// Attributes

		HSTRING		m_hstrAttributeTemplate;	// Attribute Template name

		// Commands

		HSTRING		m_hstrCmdInitial;			// Initial command (one-shot)
		HSTRING		m_hstrCmdActivateOn;		// ActivateOn command
		HSTRING		m_hstrCmdActivateOff;		// ActivateOff command

		// Activation

        LTBOOL      m_bActivated;               // Is our activation ON or OFF
		LTBOOL		m_bAlwaysActivate;			// Do we always activate regardless of our state
		LTBOOL		m_bPreserveActiveCmds;		// Should we not delete our active cmd's when used?

		// Deactivation

		LTBOOL		m_bDeactivated;
		LTBOOL		m_bReactivate;

		// Group

		AIGroup*	m_pGroup;					// The group we're in

		// Cinematic controller

		HOBJECT		m_hCinematicTrigger;		// Handle to our controlling cinematic triggers

		// Debug stuff

		uint32		m_nDebugLevel;				// Debug level
};

class CAIPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_AttachmentsPlugin; }
		virtual void GetReactions(REACTIONSTRUCT** ppReactions, int* pcReactions) { *ppReactions = g_aAIReactions; *pcReactions = g_cAIReactions; }

	private :

		CAttachmentsPlugin	m_AttachmentsPlugin;
		CModelButeMgrPlugin m_ModelStylePlugin;
};

#endif // __AI_H__