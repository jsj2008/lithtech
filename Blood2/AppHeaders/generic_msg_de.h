// ----------------------------------------------------------------------- //
//
// MODULE  : Generic_msg_de.h
//
// PURPOSE : Generic game message ids
//
// CREATED : 10/8/97
//
// ----------------------------------------------------------------------- //

#ifndef __GENERIC_MSG_DE_H__
#define __GENERIC_MSG_DE_H__

// Defines...

// All of the following messages are sent between objects:
// (i.e., Object <-> Object)

// For MID_TRIGGER, the message should contain 1 HSTRING that represents
// the trigger functionality (e.g. "CLOSE")
//
#define MID_TRIGGER					1000

// For MID_DAMAGE, the message should contain (in order) 1 DVector that
// represents the direction the damage came from, and 1 DFLOAT that represents
// the damage amount
//
#define MID_DAMAGE					1001

// For MID_REPAIR, the message should contain 1 DFLOAT that represents the
// repair amount
//
#define MID_REPAIR					1002

// For MID_HEAL, the message should contain 1 DFLOAT that represents the
// heal amount
//
#define MID_HEAL					1003

// For MID_ADDWEAPON, the message should contain (in order), 1 DBYTE that
// represents the weapon id (this id may be game specific), and 1 DFLOAT
// that represents the amount of ammo the weapon contains
//
#define	MID_ADDWEAPON				1004

// For MID_PICKEDUP, the message should contain 1 float that represents
// the time to remain invisible.  Specify -1.0 to use the object's default
// respawn time.
#define MID_PICKEDUP				1005

// For MID_INVENTORYITEMTOUCH, the message should contain (in order),
// 1 DBYTE that specifies the type of item, and 1 HSTRING that specifies
// the name of the item
#define MID_INVENTORYITEMTOUCH		1006

// For MID_INVENTORYITEMNEEDED, the message should contain (in order),
// 1 DBYTE that specifies the type of item needed, and 1 HSTRING that 
// identifies the name of the item needed
#define MID_INVENTORYITEMQUERY		1007

// For MID_INVENTORYITEMHAVE, the message should contain (in order),
// 1 DBYTE that specifies the type of item, and 1 HSTRING that identifies
// the name of the item (should both be the same as in the query).
#define MID_INVENTORYQUERYRESPONSE	1008

// For MID_AMMO, the message should contain (in order), 1 DBYTE that
// represents the ammo type (this id may be game specific), and 1 DFLOAT
// that represents the amount of ammo.
//
#define	MID_ADDAMMO					1009

#endif // __GENERIC_MSG_DE_H__