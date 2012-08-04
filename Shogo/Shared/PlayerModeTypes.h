// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerModeTypes.h
//
// PURPOSE : PlayerObj helper class - Definition
//
// CREATED : 2/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_MODE_TYPES_H__
#define __PLAYER_MODE_TYPES_H__

// Defines....

#define PM_MODE_UNDEFINED			-1	// Undefinied
#define PM_MODE_FOOT				0	// On foot
#define PM_MODE_MCA_AP				1	// Andra Predator
#define PM_MODE_MCA_UE				2	// UCA Enforcer
#define PM_MODE_MCA_AO				3	// Armacham Ordog
#define PM_MODE_MCA_SA				4	// Shogo Akuma
#define PM_MODE_KID					5   // Kid mode
#define PM_CURRENT_MCA				6	// Currently selected mech
#define PM_MULTIPLAYER_MCA			7	// Multiplayer only, use MCA user selected

#define PM_DEFAULT_MECHA_MODE		PM_MODE_MCA_UE	

#endif // __PLAYER_MODE_TYPES_H__