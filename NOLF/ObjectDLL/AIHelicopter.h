// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIHELICOPTER_H__
#define __AIHELICOPTER_H__

#include "AIVehicle.h"
#include "AISteering.h"

class AI_Helicopter : public CAIVehicle
{
	public : // Public methods

		// Ctors/Dtors/etc

 		AI_Helicopter();
 		~AI_Helicopter();

		// Movement

		void Start() { m_fSpeed = m_fFlySpeed; }
		void Stop() { m_fSpeed = 0.0f; }
		LTFLOAT GetSpeed() { return m_fSpeed; }

		// Doors

		void OpenRightDoor();
		void CloseRightDoor();
		LTBOOL IsRightDoorOpen();

		// Attachments

		virtual void CreateAttachments();

		void SetSearchlightRotation(LTRotation& rRot);
		LTRotation GetSearchlightRotation();

		void SetSearchlightPosition(LTVector& vPos);
		LTVector GetSearchlightPosition();

		void SetGunnerRotation(LTRotation& rRot);
		LTRotation GetGunnerRotation();

		void SetGunnerPosition(LTVector& vPos);
		LTVector GetGunnerPosition();

		LTRotation GetAttachmentRotation(HOBJECT hObject);
		LTVector GetAttachmentPosition(HOBJECT hObject);

	protected : // Protected member functions

		// Engine methods

		LTBOOL ReadProp(ObjectCreateStruct *pData);
		void InitialUpdate();
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		// Character stuff

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs);
        LTBOOL CanLipSync() { return LTFALSE; }

		// Update methods

		void PostUpdate();

		void UpdateAnimation();
		void UpdateAnimator();

		// Handlers

		void HandleDamage(const DamageStruct& damage);
		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleBrokenLink(HOBJECT hLink);

		// State methods

		void SetState(CAIHelicopterState::AIHelicopterStateType eState);

		// Precomputation stuff

		void ComputeSquares();

		// Attachments

		void InitAttachments();

	protected : // Protected member variables

		// State

		CAIHelicopterState*	m_pHelicopterState;	// Our Helicopter state

		// Doors

		LTBOOL m_bWantsRightDoorOpened;
		LTBOOL m_bWantsRightDoorClosed;
		LTBOOL m_bRightDoorOpened;
		LTBOOL m_bWantsLeftDoorOpened;
		LTBOOL m_bWantsLeftDoorClosed;
		LTBOOL m_bLeftDoorOpened;

		// Movement

		LTFLOAT			m_fSpeed;		// Our current movement speed
		LTFLOAT			m_fFlySpeed;	// Our "fly" (max) speed

		// Searchlight

		LTRotation		m_rRotSearchlight;		// The searchlight's rotation
		LTVector		m_vPosSearchlight;		// The searchlight's position
		int				m_iObjectSearchLight;	// The searchlight's index in m_apObjects

		// Gunners

		LTRotation		m_rRotGunner;			// The gunner's rotation
		LTVector		m_vPosGunner;			// The gunner's position
		int				m_iObjectGunner;		// The gunner's index in m_apObjects;

		LTFLOAT			m_fDeathDelay;
		HSTRING			m_hstrDeathMessage;
		HSTRING			m_hstrDeath0_3rdMessage;
		HSTRING			m_hstrDeath1_3rdMessage;
		HSTRING			m_hstrDeath2_3rdMessage;
		LTBOOL			m_bExploded;
};

DEFINE_ALIGNMENTS(Helicopter);

class CAIHelicopterPlugin : public CAIVehiclePlugin
{
	public:

		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_HelicopterAttachmentsPlugin; }

	private :

		CHelicopterAttachmentsPlugin	m_HelicopterAttachmentsPlugin;
};

#endif // __AIHelicopter_H__