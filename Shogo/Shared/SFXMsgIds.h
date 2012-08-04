// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMsgIDs.h
//
// PURPOSE : Special FX message ids
//
// CREATED : 10/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MSG_IDS_H__
#define __SFX_MSG_IDS_H__

// SpecialFX types...
//
// The first BYTE in the message associated with the HMESSAGEREAD parameter 
// passed to CSFXMgr::HandleSFXMsg() must be one of the following values:

// NOTE:  It is important that these remain in this order, SFXMgr.cpp assumes
// this order for making optimizations...New FX should be added at the end
// and SFX_TOTAL_NUMBER should be adjusted to account for them.

#define SFX_POLYGRID_ID				1
#define SFX_PARTICLETRAIL_ID		2
#define	SFX_PARTICLESYSTEM_ID		3
#define SFX_CASTLINE_ID				4
#define SFX_SPARKS_ID				5
#define SFX_TRACER_ID				6
#define SFX_WEAPON_ID				7
#define SFX_DYNAMICLIGHT_ID			8
#define SFX_PARTICLETRAILSEG_ID		9
#define SFX_SMOKE_ID				10
#define SFX_BULLETTRAIL_ID			11
#define SFX_VOLUMEBRUSH_ID			12
#define SFX_SHELLCASING_ID			13
#define SFX_CAMERA_ID				14
#define SFX_PARTICLEEXPLOSION_ID	15
#define SFX_SPRITE_ID				16	
#define SFX_EXPLOSION_ID			17	
#define SFX_DEBRIS_ID				18
#define SFX_DEATH_ID				19
#define SFX_GIB_ID					20
#define SFX_PROJECTILE_ID			21
#define SFX_MARK_ID					22
#define SFX_LIGHT_ID				23
#define SFX_CRITICALHIT_ID			24
#define SFX_UNUSED_ID				25
#define SFX_PICKUPITEM_ID			26
#define SFX_PLAYER_ID				27
#define SFX_LINEBALL_ID				28
#define SFX_ANIMELINES_ID			29
#define SFX_WEAPONSOUND_ID			30
#define SFX_AUTO_ID					31

// Important that this is the same as the last FX defined...

#define SFX_TOTAL_NUMBER			31

#endif // __SFX_MSG_IDS_H__

