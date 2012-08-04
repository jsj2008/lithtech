// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMovement.h
//
// PURPOSE : Shared movement definitions.
//
// CREATED : 11/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHAREDMOVEMENT_H__
#define __SHAREDMOVEMENT_H__

// PlayerPhysicsModel related data...

enum PlayerPhysicsModel
{
	PPM_FIRST=0,
	PPM_NORMAL=0,
	PPM_LURE,
	PPM_NUM_MODELS
};

inline bool IsVehicleModel(PlayerPhysicsModel eModel)
{
	switch (eModel)
	{
		case PPM_LURE :
            return true;
		break;

		default : break;
	}

    return false;
}

char* GetPropertyNameFromPlayerPhysicsModel(PlayerPhysicsModel ePPModel);
PlayerPhysicsModel GetPlayerPhysicsModelFromPropertyName( const char* pPropertyName);



// Maximum containers it will handle the player being in.
#define MAX_TRACKED_CONTAINERS	16


// The client keeps track of a bunch of animations and if its object
// is using any of them it doesn't allow movement.
#define MAX_STILL_ANIMATIONS	4
#define MAX_STILL_ANIM_NAME_LEN	16


// These flags are used in the MID_CLIENT_PLAYER_UPDATE to tell which
// things are in the message.
#define PSTATE_MODELFILENAMES	(1<<0)
#define PSTATE_GRAVITY			(1<<1)
#define PSTATE_CONTAINERTYPE	(1<<2)
#define PSTATE_SPEEDS			(1<<3)
#define PSTATE_PHYSICS_MODEL	(1<<4)
#define PSTATE_POSITION			(1<<5)
#define PSTATE_CAMERA			(1<<6)
#define PSTATE_MOVEMENT			(1<<7)
#define PSTATE_NUMBITS			8

//Flags used for initial client update
#define PSTATE_INITIAL				(PSTATE_MODELFILENAMES | PSTATE_GRAVITY | PSTATE_CONTAINERTYPE | PSTATE_SPEEDS | PSTATE_PHYSICS_MODEL | PSTATE_CAMERA)


//the default gravity to use for the player
#define DEFAULT_PLAYER_GRAVITY	-2000.0f

// Control flags (movement, weapons)...

#define BC_CFLG_FORWARD			(1<<0)
#define BC_CFLG_REVERSE			(1<<1)
#define BC_CFLG_RIGHT			(1<<2)
#define BC_CFLG_LEFT			(1<<3)
#define BC_CFLG_JUMP			(1<<4)
#define BC_CFLG_DUCK			(1<<5)
#define BC_CFLG_STRAFE			(1<<6)
#define BC_CFLG_STRAFE_LEFT		(1<<7)
#define BC_CFLG_STRAFE_RIGHT	(1<<8)
#define BC_CFLG_RUN				(1<<9)
#define BC_CFLG_FIRING			(1<<10)
#define BC_CFLG_MOVING			(1<<11)
#define BC_CFLG_LOOKUP			(1<<12)
#define BC_CFLG_LOOKDOWN		(1<<13)
#define BC_CFLG_ALT_FIRING		(1<<14)
#define BC_CFLG_LEAN_LEFT		(1<<15)
#define BC_CFLG_LEAN_RIGHT		(1<<16)
#define BC_CFLG_GRENADE			(1<<17)


#endif  // __SHAREDMOVEMENT_H__
