// ----------------------------------------------------------------------- //
//
// MODULE  : Switch.CPP
//
// PURPOSE : A Switch object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "Switch.h"

LINKFROM_MODULE( Switch );

BEGIN_CLASS( Switch )

	// Set AWM Type

	AWM_SET_TYPE_STATIC

	// Overrides...

	// Override the options group...

	ADD_BOOLPROP_FLAG(PlayerActivate, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(AIActivate, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(StartOn, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(TriggerOff, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(RemainOn, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(Locked, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(RotateAway, LTFALSE, PF_GROUP(3))
	ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3))

	// Override the sounds group...

	ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4))
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4))
	ADD_BOOLPROP_FLAG(LoopSounds, LTFALSE, PF_GROUP(4))

	// Override the commands group...

	ADD_STRINGPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)

	// Override the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 1.0f, 0)
	ADD_REALPROP_FLAG(MoveDist, 8.0f, 0)

	// Override the rotation properties...

	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK)
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0)

	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, AWM_DEFAULT_POWERONTIME, 0)
	ADD_REALPROP_FLAG(PowerOffTime, AWM_DEFAULT_POWEROFFTIME, 0)
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0)
	ADD_REALPROP_FLAG(OnWaitTime, AWM_DEFAULT_ONWAITTIME, 0)
	ADD_REALPROP_FLAG(OffWaitTime, AWM_DEFAULT_OFFWAITTIME, 0)

END_CLASS_DEFAULT_FLAGS( Switch, ActiveWorldModel, NULL, NULL, CF_HIDDEN | CF_WORLDMODEL )

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Switch )
CMDMGR_END_REGISTER_CLASS( Switch, ActiveWorldModel )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::Switch()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Switch::Switch() 
:	ActiveWorldModel( )
{
 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::~Switch()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

Switch::~Switch()
{
 
}

