// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNavMeshLinkTypes.h
//
// PURPOSE : Enums and string constants for AI nav mesh types.
//
// CREATED : 06/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_NAV_MESH_LINK_TYPES_H__
#define __AI_ENUM_NAV_MESH_LINK_TYPES_H__


//
// ENUM: Types of links.
//
enum EnumAINavMeshLinkType
{
	kLink_InvalidType = -1,
#define LINK_TYPE_AS_ENUM 1
#include "AIEnumNavMeshLinkTypeValues.h"
#undef LINK_TYPE_AS_ENUM

	kLink_Count,
};

//
// STRINGS: const strings for link types.
//
static const char* s_aszNavMeshLinkTypes[] =
{
#define LINK_TYPE_AS_STRING 1
#include "AIEnumNavMeshLinkTypeValues.h"
#undef LINK_TYPE_AS_STRING
};

#endif // __AI_ENUM_NAV_MESH_LINK_TYPES_H__
