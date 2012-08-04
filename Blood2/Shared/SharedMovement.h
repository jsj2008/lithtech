
// Shared movement definitions.

#ifndef __SHAREDMOVEMENT_H__
#define __SHAREDMOVEMENT_H__


	// Maximum containers it will handle the player being in.
	#define MAX_TRACKED_CONTAINERS	4
	
	
	// The client keeps track of a bunch of animations and if its object
	// is using any of them it doesn't allow movement.
	#define MAX_STILL_ANIMATIONS	4
	#define MAX_STILL_ANIM_NAME_LEN	16


	// These flags are used in the MID_PHYSICS_UPDATE to tell which 
	// things are in the message.
	#define PSTATE_MODELFILENAMES	(1<<0)
	#define PSTATE_ADDVELOCITY		(1<<1)
	#define PSTATE_GRAVITY			(1<<2)
	#define PSTATE_CONTAINERTYPE	(1<<3)
	#define PSTATE_SPEEDS			(1<<4)
	#define PSTATE_CROUCH			(1<<5)	// Special flag to tell the player to stand..
	#define PSTATE_ALL				(PSTATE_MODELFILENAMES | PSTATE_GRAVITY | PSTATE_CONTAINERTYPE | PSTATE_SPEEDS | PSTATE_CROUCH)



// Control flags
#define  CTRLFLAG_RUN			0x0001
#define  CTRLFLAG_JUMP			0x0002
#define  CTRLFLAG_CROUCH		0x0004
#define  CTRLFLAG_FORWARD		0x0008
#define  CTRLFLAG_BACKWARD		0x0010
#define  CTRLFLAG_LEFT			0x0020
#define  CTRLFLAG_RIGHT			0x0040
#define  CTRLFLAG_STRAFE		0x0080
#define  CTRLFLAG_STRAFERIGHT	0x0100	
#define  CTRLFLAG_STRAFELEFT	0x0200
#define  CTRLFLAG_FIRE			0x0400
#define  CTRLFLAG_ALTFIRE		0x0800
#define  CTRLFLAG_GRAB			0x1000
#define  CTRLFLAG_TAUNT			0x2000

// Client position info flags
#define		CLIENTMOVEMENTFLAG_ONGROUND		(1<<0)



#endif  // __SHAREDMOVEMENT_H__


