// ----------------------------------------------------------------------- //
//
// MODULE  : DEditColors.h
//
// PURPOSE : Colors for classes to show up in DEdit.
//
// CREATED : 6/22/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// MACROS:  Use the first one in this file, and the second one in
//          the actual .cpp after BEGIN_CLASS
//
#define DEFINE_DEDIT_COLOR(class_name, r, g, b) \
	static LTVector GetDeditColor##class_name##() { return LTVector(r, g, b); }

#define ADD_DEDIT_COLOR(class_name) \
	ADD_COLORPROP_FLAG(Color, GetDeditColor##class_name##().x, GetDeditColor##class_name##().y, GetDeditColor##class_name##().z, PT_COLOR|PF_HIDDEN)


//
// COLORS:
//
DEFINE_DEDIT_COLOR( CAIHuman, 255, 255, 0 );
DEFINE_DEDIT_COLOR( AINodeDeath, 255, 0, 0 );
DEFINE_DEDIT_COLOR( AINodeCover, 0, 0, 255 );
DEFINE_DEDIT_COLOR( AINodeVantage, 0, 0, 128 );
DEFINE_DEDIT_COLOR( AINodeView, 0, 0, 128 );
DEFINE_DEDIT_COLOR( AINodePatrol, 255, 255, 0 );
DEFINE_DEDIT_COLOR( AINodeSearch, 255, 160, 0 );
DEFINE_DEDIT_COLOR( AIVolume, 0, 128, 0 );
DEFINE_DEDIT_COLOR( AIVolumeJunction, 0, 255, 0 );
DEFINE_DEDIT_COLOR( AIVolumeHiding, 0, 64, 0 );
DEFINE_DEDIT_COLOR( AIVolumePlayerInfo, 0, 64, 0 );
DEFINE_DEDIT_COLOR( AINodeObstruct, 0, 255, 255 );

DEFINE_DEDIT_COLOR( VolumeEffect, 0, 0, 255 );
DEFINE_DEDIT_COLOR( SnowVolume, 0, 192, 192 );
