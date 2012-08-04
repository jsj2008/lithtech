// ----------------------------------------------------------------------- //
//
// MODULE  : InventoryTypes.h
//
// PURPOSE : inventory type ids for various inventory objects
//
// CREATED : 10/29/97
//
// ----------------------------------------------------------------------- //

#ifndef __INVENTORYTYPES_H
#define __INVENTORYTYPES_H

// inventory types

#define IT_KEY						1
#define IT_UPGRADE					2
#define IT_ENHANCEMENT				3

// inventory sub-types
// NOTE: none of these should never be zero - when comparing two inventory items, the 
//       Inventory aggregate also compares subtypes unless the subtype is zero.  If one
//		 of the subtypes is zero, the aggregate will take that to mean "match all subtypes"

#define IST_UPGRADE_DAMAGE			1
#define IST_UPGRADE_PROTECTION		2
#define IST_UPGRADE_REGEN			3
#define IST_UPGRADE_HEALTH			4
#define IST_UPGRADE_ARMOR			5
#define IST_UPGRADE_TARGETING		6

#define IST_ENHANCEMENT_DAMAGE					1
#define IST_ENHANCEMENT_MELEEDAMAGE				2
#define IST_ENHANCEMENT_PROTECTION				3
#define IST_ENHANCEMENT_ENERGYPROTECTION		4
#define IST_ENHANCEMENT_PROJECTILEPROTECTION	5
#define IST_ENHANCEMENT_EXPLOSIVEPROTECTION		6
#define IST_ENHANCEMENT_REGEN					7
#define IST_ENHANCEMENT_HEALTH					8
#define IST_ENHANCEMENT_ARMOR					9

#endif

