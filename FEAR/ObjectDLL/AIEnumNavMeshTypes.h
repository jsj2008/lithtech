// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumNavMeshTypes.h
//
// PURPOSE : Enums and string constants for AI nav mesh types.
//
// CREATED : 06/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ENUM_NAV_MESH_TYPES_H__
#define __AI_ENUM_NAV_MESH_TYPES_H__

#include "AINodeCluster.h"


enum ENUM_NMNavMeshID	{ kNMNavMesh_Invalid = -1, };
enum ENUM_NMLinkID		{ kNMLink_Invalid = -1, };
enum ENUM_NMPolyID		{ kNMPoly_Invalid = -1, };
enum ENUM_NMNormalID	{ kNMNormal_Invalid = -1, };
enum ENUM_NMEdgeID		{ kNMEdge_Invalid = -1, };
enum ENUM_NMComponentID { kNMComponent_Invalid = -1, };
enum ENUM_NMSensoryComponentID { kNMSensoryComponent_Invalid = -1, };
enum ENUM_AIRegionID	{ kAIRegion_Invalid = -1, };

typedef std::vector<ENUM_NMComponentID, LTAllocator<ENUM_NMComponentID, LT_MEM_TYPE_OBJECTSHELL> >	NMCOMPONENT_LIST;


enum ENUM_NMPolyFlag
{
	kNMPolyFlag_None	= 0x00,
	kNMPolyFlag_Lit		= 0x01,
	kNMPolyFlag_Marked	= 0x02,
};

enum ENUM_NMEdgeType
{
	kNMEdgeType_Invalid = -1,
	kNMEdgeType_Border,
	kNMEdgeType_Shared,
};

struct AIREGION_DATA
{
	ENUM_AIRegionID		eAIRegionID;
	uint32				cNMPolys;
	ENUM_NMPolyID*		pNMPolyIDs;
	LTVector			vBoundingSphereCenter;
	float				fBoundingSphereRadius;
};

struct NAVMESH_LINK_DATA
{
	ENUM_NMLinkID	eNMlinkID;
	uint32			cBoundaryVerts;
	LTVector*		pBoundaryVerts;
};

struct AINODE_CLUSTER_DATA
{
	const char*				pszName;
	EnumAINodeClusterID		eAINodeClusterID;
};

#endif // __AI_ENUM_NAV_MESH_TYPES_H__
