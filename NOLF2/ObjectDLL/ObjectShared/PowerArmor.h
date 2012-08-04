//----------------------------------------------------------------------------
//              
//	MODULE:		PowerArmor.h
//              
//	PURPOSE:	PowerArmor declaration
//              
//	CREATED:	13.03.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __POWERARMOR_H__
#define __POWERARMOR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "Prop.h"
#include "IHitBoxUser.h"

// Forward declarations

// Globals

// Statics

LINKTO_MODULE( PowerArmor );

const char KEY_TURNON[]			= "ARMOR_ON";
const char KEY_TURNOFF[]		= "ARMOR_OFF";

//----------------------------------------------------------------------------
//              
//	CLASS:		PowerArmor
//              
//	PURPOSE:	Armor which can be turned on and off by the owner at will.
//				Does NOT actually take damage -- instead, the AI can route
//				some of the damage to the PowerArmor for reduction.
//              
//----------------------------------------------------------------------------
class PowerArmor : public Prop
{
	public:
		// Public members

		// Ctors/Dtors/etc
		PowerArmor();
		virtual ~PowerArmor();
		

	protected:
		// Protected members

		// Engine Interaction:
		virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		void Update(void);

		void HandleOn();
		void HandleOff();
		void HandleTurningOn();
		void HandleTurningOff();

	private:
		// Private members

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		PowerArmor(const PowerArmor& rhs) {}
		PowerArmor& operator=(const PowerArmor& rhs ) {}

		// Save:
		enum ePA_State
		{
			kPA_On,
			kPA_Off,
			kPA_TurningOn,
			kPA_TurningOff,
		};

		ePA_State	m_eState;		// What is the armor doing?
		LTBOOL		m_bDisabled;	// Does this armor function?
};


//----------------------------------------------------------------------------
//              
//	CLASS:		ShellArmor
//              
//	PURPOSE:	Special Armor which DOES take damage.  Using it results in a 
//				big performance hit, because it collides with the owner 
//				constantly.  ShellArmor has a hitbox, just like a normal
//				character and done in depth hit detection on its own, unlike
//				the standard PowerArmor which relies on the user notifiying it
//				when it should turn off.
//              
//----------------------------------------------------------------------------
class ShellArmor : public Prop, public IHitBoxUser
{
	public:
		ShellArmor();
		virtual ~ShellArmor();

		// Engine Interaction:
		virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual uint32  ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		virtual void Load(ILTMessage_Read *pMsg);

		// IHitBoxUser Interaction:
		virtual CAttachments* GetAttachments();
		virtual ModelSkeleton GetModelSkeleton() const;
		virtual void SetModelNodeLastHit(ModelNode eModelNode );
		virtual LTBOOL UsingHitDetection() const;
		virtual HATTACHMENT GetAttachment() const;
		virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);
		virtual float GetNodeRadius( ModelSkeleton eModelSkeleton, ModelNode eModelNode );
		virtual void UpdateClientHitBox() {};

	protected:
		enum eShieldState
		{
			eShieldNormal,
			eShieldAttack,
			eShieldBlock,
		};

		void	ReadShieldProps(ObjectCreateStruct *pData);

		void	Update();
		void	CreateHitBox();
		void	HandleDeath();
		void	HandleModelString(ArgList* pArgList);

		void	SetState(eShieldState eNewState);
		void	SetAnimation(const char* const szAnimationName);

		LTBOOL	CanDamage() const;

	private:
		// Private members
		
		// Handle to the armors' hitbox object
		LTObjRef		m_hHitBox;			

		// Handle to the object the armor is attached to
		LTObjRef		m_hParentObject;

		// Handle to the currently playing animation
		HMODELANIM		m_hCurrentAnimation;

		// Animation specified 'state' for this shield.
		eShieldState	m_eShieldState;

		ModelId			m_eModelId;

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		ShellArmor(const PowerArmor& rhs) {}
		ShellArmor& operator=(const PowerArmor& rhs ) {}
};
#endif // __POWERARMOR_H__

