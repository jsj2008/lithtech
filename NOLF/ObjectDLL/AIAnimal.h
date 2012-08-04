// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_ANIMAL_H__
#define __AI_ANIMAL_H__

#include "AI.h"
#include "AIAnimalState.h"
#include "AnimatorAIAnimal.h"

// Defines

class CAIAnimal : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

 		CAIAnimal();
 		virtual ~CAIAnimal();

		// Animation

		CAnimatorAIAnimal* GetAnimator() { return &m_Animator; }

		// Attachments

		virtual void CreateAttachments();

		// Simple accessors

		HMODELANIM GetDeathAni(LTBOOL bFront);

	protected : // Protected member variables

		CAnimatorAIAnimal	m_Animator;			// Our animator
		CAIAnimalState*		m_pAnimalState;		// Our Animal state
};

class CAIAnimalPlugin : public CAIPlugin
{
	public:

		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_AnimalAttachmentsPlugin; }

	private :

		CAnimalAttachmentsPlugin	m_AnimalAttachmentsPlugin;
};

#endif // __AI_H__