// ----------------------------------------------------------------------- //
//
// MODULE  : Lift.CPP
//
// PURPOSE : A Lift object
//
// CREATED : 6/11/98
//
// ----------------------------------------------------------------------- //

// Includes...

#include "Lift.h"

BEGIN_CLASS(Lift)
	ADD_LONGINTPROP(StateFlags, (DF_SELFTRIGGER | DF_FORCECLOSE))
	ADD_REALPROP(Speed, 200.0f)
	ADD_REALPROP(MoveDelay, 1.0f)
	ADD_REALPROP(MoveDist, 100.0f)
	ADD_VECTORPROP_VAL(MoveDir, 0.0f, 1.0f, 0.0f)
	ADD_REALPROP(OpenWaitTime, 2.0f)
	ADD_REALPROP(CloseWaitTime, 2.0f)
	ADD_STRINGPROP(OpenStartSound, "Sounds\\Doors\\elestart.wav")
	ADD_STRINGPROP(OpenBusySound, "Sounds\\Doors\\eleloop.wav")
	ADD_STRINGPROP(OpenStopSound, "Sounds\\Doors\\elestop.wav")
	ADD_STRINGPROP(CloseStartSound, "Sounds\\Doors\\elestart.wav")
	ADD_STRINGPROP(CloseBusySound, "Sounds\\Doors\\eleloop.wav")
	ADD_STRINGPROP(CloseStopSound, "Sounds\\Doors\\elestop.wav")
END_CLASS_DEFAULT(Lift, Door, NULL, NULL)
