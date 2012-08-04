
#ifndef __PHYSICAL_ATTRIBUTES_H__
#define __PHYSICAL_ATTRIBUTES_H__


// Player attributes	
#define PA_DEFAULT_MASS				250.0f		// Mass of the player
#define PA_MIN_DAMAGE_FORCE			3000.0f		// Minimum collision force to apply damage


// AI Attributes
#define AI_DEFAULT_MASS				250.0f


// Mass defines
#define MAX_MASS					100000.0f
#define MIN_FRICTION				5.0f		
#define MAX_FRICTION				15.0f		
#define MIN_FORCE					0.0f

#define BLOCKPRIORITY_PLAYER		50
#define BLOCKPRIORITY_AI			BLOCKPRIORITY_PLAYER
#define BLOCKPRIORITY_PLAYER_AIRBORNE	40
#define BLOCKPRIORITY_PUSHABLE		40
#define BLOCKPRIORITY_NONPUSHABLE	100
#define BLOCKPRIORITY_MAX			255


// Multiplier
#define PA_DAMAGE_VEL_MUTLIPLIER	2500



#endif  // __PHYSICAL_ATTRIBUTES_H__