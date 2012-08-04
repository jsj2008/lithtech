// ----------------------------------------------------------------------- //
//
// MODULE  : volumeeffect.h
//
// PURPOSE : Volume Effect support functions - Declaration
//
// CREATED : 1/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUMEEFFECT_H__
#define __VOLUMEEFFECT_H__

#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif


// setup a volume effect
bool ve_Init( LTVolumeEffect* pVE, VolumeEffectInfo* pInfo );
void ve_Term( LTVolumeEffect* pVE );


#endif  // __VOLUMEEFFECT_H__


