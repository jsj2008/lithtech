// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIAnimal.h"
#include "AIAnimalState.h"

BEGIN_CLASS(CAIAnimal)

	ADD_ANIMALATTACHMENTS_AGGREGATE()

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAIAnimal, CAI, NULL, NULL, CF_HIDDEN, CAIAnimalPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimal::CAIAnimal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAIAnimal::CAIAnimal() : CAI()
{
    m_pAnimalState = LTNULL;
	m_pAnimator = &m_Animator;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimal::~CAIAnimal()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIAnimal::~CAIAnimal()
{
	if ( m_pAnimalState )
	{
		FACTORY_DELETE(m_pAnimalState);
        m_pAnimalState = LTNULL;
        m_pState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimal::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CAIAnimal::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CAnimalAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_ANIMAL));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimal::GetDeathAni()
//
//	PURPOSE:	Get the death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIAnimal::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hAni = INVALID_MODEL_ANIM;

	if ( m_pState )
	{
		if ( INVALID_MODEL_ANIM != (hAni = m_pState->GetDeathAni(bFront)) )
		{
			return hAni;
		}
	}

	return CAI::GetDeathAni(bFront);
}