
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
	#define PSTATE_TRACTORBEAM		(1<<1)
	#define PSTATE_GRAVITY			(1<<2)
	#define PSTATE_CONTAINERTYPE	(1<<3)
	#define PSTATE_SPEEDS			(1<<4)
	#define PSTATE_ALL				0x1F



	// Control flags (movement, weapons)...

	#define BC_CFLG_FORWARD			(1<<0)
	#define BC_CFLG_REVERSE			(1<<1)
	#define BC_CFLG_RIGHT			(1<<2)
	#define BC_CFLG_LEFT			(1<<3)
	#define BC_CFLG_JUMP			(1<<4)
	#define BC_CFLG_DOUBLEJUMP		(1<<5)
	#define BC_CFLG_DUCK			(1<<6)
	#define BC_CFLG_STRAFE			(1<<7)
	#define BC_CFLG_STRAFE_LEFT		(1<<8)
	#define BC_CFLG_STRAFE_RIGHT	(1<<9)
	#define BC_CFLG_RUN				(1<<10)
	#define BC_CFLG_FIRING			(1<<11)	
	#define BC_CFLG_MOVING			(1<<12)	
	#define BC_CFLG_POSING			(1<<13)

	#define BC_CFLG_SPECIAL_MOVE	(1<<31)


#endif  // __SHAREDMOVEMENT_H__


