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

	#include "Stdafx.h"
	#include "Switch.h"

LINKFROM_MODULE( Switch );

BEGIN_CLASS( Switch )

	// Set AWM Type

	AWM_SET_TYPE_STATIC

	// Overrides...

	// Override the options group...

	ADD_BOOLPROP_FLAG(PlayerActivate, true, PF_GROUP(3), "If TRUE the player can directly interact with this object by pressing use.")
	ADD_BOOLPROP_FLAG(AIActivate, true, PF_GROUP(3), "If TRUE this lets the AI know they can interact with this object.  When FALSE the AI will treat the object like it doesn't exist.")
	ADD_BOOLPROP_FLAG(StartOn, false, PF_GROUP(3), "When set to TRUE the object will be turnning on as soon as the game loads.")
	ADD_BOOLPROP_FLAG(TriggerOff, true, PF_GROUP(3), "If this is set to FALSE the player can not directly turn the object off by pressing use.  The object can however be turned off by a message from another object like a switch.  If set to TRUE the player can directly turn off or close the object by pressing use.")
	ADD_BOOLPROP_FLAG(RemainOn, true, PF_GROUP(3), "If this is FALSE the Object will start turnning itself off or close as soon as it turns on or opens.  If TRUE the object will stay on or open untill told to turn off, either by the player or a message.")
	ADD_BOOLPROP_FLAG(ForceMove, false, PF_GROUP(3), "If set to TRUE, the object will not be stopped by the player or AI.  It will continue on its path like there is nothing in the way.")
	ADD_BOOLPROP_FLAG(Locked, false, PF_GROUP(3), "When set to TRUE the object starts in a ""Locked"" state.  It can't be activated or triggered by a message unless it is unlocked.  To unlock an object send it an UNLOCK message.  To lock it again send a LOCK message." )
	ADD_BOOLPROP_FLAG(RotateAway, false, PF_GROUP(3), "If set to TRUE RotatingWorldModels will rotate away from the player or AI that activated it.")
	ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3), "A list of predefined wave types for movement.  Linear is a constant rate, objects will move at the same rate throught its whole path or rotation. SlowOn means the object starts moving slowly then picks up pace and will then stay constant.  SlowOff will move constantly at first but will slow down at the end of the movement or rotation.  Sine starts and ends slowly but looks very smooth and natural.")

	// Override the sounds group...

	ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn on.")
	ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned on.")
	ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn off.")
	ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned off.")
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  If this object is locked and the player or another object tries to activate it, this sound will begin to play.")
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4), "The position that the sounds will be played from, relative to the objects position.  If this is (0.0 0.0 0.0) the sounds will play at the center of the object, (0.0 25.0 0.0) will place the sounds 25 units above the objects center.")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4), "This is the extent of the sounds, they will not be heard beyond this distance from the SoundPos.")
	ADD_BOOLPROP_FLAG(LoopSounds, false, PF_GROUP(4), "When set to TRUE, the sounds will comtinue to loop untill they change state.  When FALSE the sounds will play their full length once and will then be destroyed.")

	// Override the commands group...

	ADD_COMMANDPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when it is fully on.")
	ADD_COMMANDPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execue when it is fully off.")
	ADD_COMMANDPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when starting to turn on.")
	ADD_COMMANDPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this object will execute when starting to turn off.")
	ADD_COMMANDPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "If this object is locked and the player or another object tries to activate it, this command will be executed.")

	// Override the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 1.0f, 0, "This is the direction of movement this object will go when turned on.  This is relative to the objects local coordinates so a MoveDir of (0.0 1.0 0.0) will always move in the objects positive Y direction.")
	ADD_REALPROP_FLAG(MoveDist, 8.0f, 0, "This is the distance the object will move in the direction specified in MoveDir.")

	// Override the rotation properties...

	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK, "The name of a ""Point"" object which this WorldModel will rotate around.  If no valid object name is given the WorldModel will rotate around the position of the bound object." )
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0, "These represent how far the WorldModel will rotate around the RotationPoint in the specified axi when turned on.  (0.0 90.0 0.0) will rotate the WorldModel about the RotationPoint 90 degrees around the WorldModels local Y axis.")
	ADD_VECTORPROP_VAL_FLAG(StartingAngles, 0.0f, 0.0f, 0.0f, 0, "Specifes an optional initial rotation of the WorldModel.  These are in degrees relative to the rotation of the WorldModel when in the off state.  A StartingAngle of (0.0, 15.0, 0.0) will load the WorldModel with a rotation of 15 degrees around the WorldModels' local Y axis in the on direction." )

	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, AWM_DEFAULT_POWERONTIME, 0, "Sets the time in seconds for how long it takes the WorldModel to go from the Off state to the on state.")
	ADD_REALPROP_FLAG(PowerOffTime, AWM_DEFAULT_POWEROFFTIME, 0, "If other than 0.0, sets the time in seconds for how long it takes the WorldModel to go from the On state to the off state.  If this is 0.0 then the PowerOnTime value is used." )
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0, "Amount of delay in seconds between the time the WorldModel is triggered and when it begins its movement.")
	ADD_REALPROP_FLAG(OnWaitTime, AWM_DEFAULT_ONWAITTIME, 0, "Amount of time in seconds that the WorldModel will remain on before turnning off automatically, and the amount of time before the WorldModel can be triggered on again." )
	ADD_REALPROP_FLAG(OffWaitTime, AWM_DEFAULT_OFFWAITTIME, 0, "Amount of time in secomds before the WorldModel can be turned on after being turned off.")

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(Switch, ActiveWorldModel, CF_HIDDEN | CF_WORLDMODEL, DefaultPrefetch<Switch>, "A Switch is a WorldModel that is setup by default to be activated by the player and send a command." )

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Switch::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void Switch::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the world model sounds
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "PowerOnSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OnSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "PowerOffSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OffSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "LockedSound");
}
