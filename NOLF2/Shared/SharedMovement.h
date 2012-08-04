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
	PPM_SNOWMOBILE,
	PPM_LURE,
	PPM_LIGHTCYCLE,
	PPM_NUM_MODELS
};

inline LTBOOL IsVehicleModel(PlayerPhysicsModel eModel)
{
	switch (eModel)
	{
		case PPM_SNOWMOBILE :
		case PPM_LURE :
		case PPM_LIGHTCYCLE :
            return LTTRUE;
		break;

		default : break;
	}

    return LTFALSE;
}

#if !defined(_CLIENTBUILD) || defined(__PSX2)

char* GetPropertyNameFromPlayerPhysicsModel(PlayerPhysicsModel ePPModel);
PlayerPhysicsModel GetPlayerPhysicsModelFromPropertyName(char* pPropertyName);

#endif // ! _CLIENTBUILD



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
#define	PSTATE_INTERFACE		(1<<6)

//Flags used for initial client update
#define PSTATE_INITIAL				(PSTATE_MODELFILENAMES | PSTATE_GRAVITY | PSTATE_CONTAINERTYPE | PSTATE_SPEEDS | PSTATE_PHYSICS_MODEL | PSTATE_INTERFACE)

//These flags are used with MID_CLIENT_PLAYER_UPDATE and PSTATE_INTERFACE
#define PSTATE_INT_HIDING		(1<<0)
#define PSTATE_INT_HIDDEN		(1<<1)
#define PSTATE_INT_CARRYING		(1<<2)
#define PSTATE_INT_CAN_DROP		(1<<3)
#define PSTATE_INT_CANTHIDE		(1<<4)
#define PSTATE_INT_CANREVIVE	(1<<5)

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
#define BC_CFLG_ACTIVATE		(1<<12)
#define BC_CFLG_ROLL_LEFT		(1<<13)
#define BC_CFLG_ROLL_RIGHT		(1<<14)
#define BC_CFLG_PIVOT_IN		(1<<15)
#define BC_CFLG_PIVOT_OUT		(1<<16)
#define BC_CFLG_LOOKUP			(1<<17)
#define BC_CFLG_LOOKDOWN		(1<<18)
#define BC_CFLG_ALT_FIRING		(1<<19)
#define BC_CFLG_LEAN_LEFT		(1<<20)
#define BC_CFLG_LEAN_RIGHT		(1<<21)

#ifdef __PSX2
	// [kml] For variable degrees of lookage.
	#define BC_CFLG_LEFT_SHORT		(1<<20)
	#define BC_CFLG_RIGHT_SHORT		(1<<21)
	#define BC_CFLG_LEFT_SHORT_2	(1<<22)
	#define BC_CFLG_RIGHT_SHORT_2	(1<<23)
	#define BC_CFLG_LEFT_SHORT_3	(1<<24)
	#define BC_CFLG_RIGHT_SHORT_3	(1<<25)
	#define BC_CFLG_LOOKUP_SHORT	(1<<26)
	#define BC_CFLG_LOOKDOWN_SHORT	(1<<27)
	#define BC_CFLG_LOOKUP_SHORT_2	(1<<28)
	#define BC_CFLG_LOOKDOWN_SHORT_2 (1<<29)
	#define BC_CFLG_LOOKUP_SHORT_3	(1<<30)
	#define BC_CFLG_LOOKDOWN_SHORT_3 (1<<31)
#endif

#endif  // __SHAREDMOVEMENT_H__
