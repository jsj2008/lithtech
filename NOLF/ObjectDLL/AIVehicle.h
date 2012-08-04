// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_VEHICLE_H__
#define __AI_VEHICLE_H__

#include "AI.h"
#include "AIVehicleState.h"
#include "AnimatorAIVehicle.h"

// Defines

class CAIVehicle : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

 		CAIVehicle();
 		virtual ~CAIVehicle();

		// Animation

		CAnimatorAIVehicle* GetAnimator() { return &m_Animator; }

		// Attachments

		virtual void CreateAttachments();

	protected : // Protected member variables

		CAnimatorAIVehicle	m_Animator;				// Our animator
		CAIVehicleState*	m_pVehicleState;		// Our Vehicle state
};

class CAIVehiclePlugin : public CAIPlugin
{
	public:

		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_VehicleAttachmentsPlugin; }

	private :

		CVehicleAttachmentsPlugin	m_VehicleAttachmentsPlugin;
};

#endif // __AI_H__