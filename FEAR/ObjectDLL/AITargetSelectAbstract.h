// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectAbstract.h
//
// PURPOSE : AITargetSelectAbstract abstract class declaration
//           AITarget uses AITargetSelect objects to select targets.
//
// CREATED : 12/19/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_ABSTRACT_H__
#define __AITARGETSELECT_ABSTRACT_H__

#include "AIClassFactory.h"

// ----------------------------------------------------------------------- //

//
// ENUM: Types of targets.
//
enum EnumAITargetSelectType
{
	kTargetSelect_InvalidType= -1,
	#define TARGET_TYPE_AS_ENUM 1
	#include "AIEnumTargetTypes.h"
	#undef TARGET_TYPE_AS_ENUM

	kTargetSelect_Count,
};

//
// STRINGS: const strings for target types.
//
static const char* s_aszTargetSelectTypes[] =
{
	#define TARGET_TYPE_AS_STRING 1
	#include "AIEnumTargetTypes.h"
	#undef TARGET_TYPE_AS_STRING
};


//
// Forward declarations...
//

struct AIDB_TargetSelectRecord;


// ----------------------------------------------------------------------- //

class CAITargetSelectAbstract : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( TargetSelect );

		CAITargetSelectAbstract();
		virtual ~CAITargetSelectAbstract();

		// Initialization.

		virtual void	InitTargetSelect( AIDB_TargetSelectRecord* pTargetRecord );

		// Selecting Targets.

		float			GetCost();

		// Validation.

		virtual bool	ValidatePreconditions( CAI* pAI ) { return true; }
		virtual bool	Validate( CAI* pAI );

		// Activation and Completion.

		virtual void	Activate( CAI* pAI );
		virtual void	Deactivate( CAI* pAI ) {}

		// Data Access.

		AIDB_TargetSelectRecord*		GetTargetSelectRecord() { return m_pTargetSelectRecord; }

	protected:

		AIDB_TargetSelectRecord*		m_pTargetSelectRecord;
};

// ----------------------------------------------------------------------- //

#endif
