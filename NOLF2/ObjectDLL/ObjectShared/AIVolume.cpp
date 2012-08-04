// ----------------------------------------------------------------------- //
//
// MODULE  : AIVolume.cpp
//
// PURPOSE : Implementation of AIGeometry, AISpatialRepresentation, and
//			 AIVolume classes
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <limits.h>
#include "FastHeap.h"
#include "ContainerCodes.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "AIPath.h"
#include "Door.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AIPathKnowledgeMgr.h"
#include "AINodeMgr.h"
#include "AIRegion.h"
#include "AIRegionMgr.h"
#include "AI.h"
#include "DEditColors.h"
#include "DebugLineSystem.h"
#include "RelationMgr.h"
#include "RelationButeMgr.h"
#include "AIStimulusMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "ObjectRelationMgr.h"
#include "CharacterMgr.h"

LINKFROM_MODULE( AIVolume );

// ----------------------------------------------------------------------- //

const static LTFLOAT c_fNeighborThreshhold = 0.5f;

// An average AI is 48 units wide, so we should not allow 
// AIVolumes used for path finding to be smaller than 48x48.
const LTFLOAT AIGeometry::kMinXZDim = 48.f;

const LTFLOAT AIVolume::kApproxVolumeYDim = 48.f;
const LTFLOAT AIVolume::kUnpreferablePathWeight = 500.f;

extern CVarTrack g_ShowVolumesTrack;

// ----------------------------------------------------------------------- //
BEGIN_CLASS( AIGeometry )
	ADD_VECTORPROP_VAL_FLAG(Dims, AIGeometry::kMinXZDim, 8.0f, AIGeometry::kMinXZDim, PF_DIMS)
END_CLASS_DEFAULT_FLAGS( AIGeometry, GameBaseLite, NULL, NULL, CF_HIDDEN | CF_CLASSONLY )

BEGIN_CLASS( AISpatialRepresentation )
	ADD_STRINGPROP_FLAG(Region,			"",			0|PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS( AISpatialRepresentation, AIGeometry, NULL, NULL, CF_HIDDEN | CF_CLASSONLY )

#pragma force_active on
BEGIN_CLASS(AIVolume)

	ADD_DEDIT_COLOR( AIVolume )

    ADD_BOOLPROP(Lit, LTTRUE)
	ADD_STRINGPROP_FLAG(LightSwitchNode,"",			0|PF_OBJECTLINK)
	ADD_BOOLPROP(PreferredPath,	LTFALSE)

END_CLASS_DEFAULT_FLAGS(AIVolume, AISpatialRepresentation, NULL, NULL, CF_CLASSONLY)

#pragma force_active off

  
CMDMGR_BEGIN_REGISTER_CLASS( AIVolume )
  
//	CMDMGR_ADD_MSG( BLINK, 1, NULL, "BLINK" )
//	CMDMGR_ADD_MSG( RESET, 1, NULL, "RESET" )
	CMDMGR_ADD_MSG( LIT, 1, NULL, "LIT" )
	CMDMGR_ADD_MSG( BRIGHT, 1, NULL, "BRIGHT" )
	CMDMGR_ADD_MSG( UNLIT, 1, NULL, "UNLIT" )
	CMDMGR_ADD_MSG( DARK, 1, NULL, "DARK" )
	CMDMGR_ADD_MSG( ENABLE, 1, NULL, "ENABLE" )
	CMDMGR_ADD_MSG( DISABLE, 1, NULL, "DISABLE" )
  
CMDMGR_END_REGISTER_CLASS( AIVolume, AISpatialRepresentation )


#pragma force_active on
BEGIN_CLASS(AIInformationVolume)

	ADD_DEDIT_COLOR( AIVolume )

	ADD_VECTORPROP_VAL_FLAG(Dims, 32.0f, 8.0f, 32.0f, PF_DIMS)
	ADD_STRINGPROP_FLAG(Region,			"",			0|PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(SenseMask,		"None",		0|PF_STATICLIST)
    ADD_BOOLPROP(On, LTTRUE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIInformationVolume, AISpatialRepresentation, NULL, NULL, CF_CLASSONLY, AIInformationVolumePlugin)

CMDMGR_BEGIN_REGISTER_CLASS(AIInformationVolume)	

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS(AIInformationVolume, AISpatialRepresentation)

#pragma force_active off

AIGeometry::AIGeometry() :
	GameBaseLite(false)
{}

bool AIGeometry::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

	m_vCenter = pocs->m_cProperties.GetPropVector( "Pos", LTVector( 0.0f, 0.0f, 0.0f ) );

	// Round VolumePos to integer values.
	// This is necessary to ensure VolumeMgr find neighbors.

	RoundVector( m_vCenter );

	m_vDims = pocs->m_cProperties.GetPropVector( "Dims", LTVector( 1.0f, 1.0f, 1.0f ) );

	// Round dims to integer values.
	// This is necessary to ensure VolumeMgr find neighbors.

	RoundVector( m_vDims );

    m_vFrontTopRight    = m_vCenter + LTVector(m_vDims.x,   m_vDims.y,  m_vDims.z);
    m_vFrontTopLeft     = m_vCenter + LTVector(-m_vDims.x,  m_vDims.y,  m_vDims.z);
    m_vBackTopRight     = m_vCenter + LTVector(m_vDims.x,   m_vDims.y,  -m_vDims.z);
    m_vBackTopLeft      = m_vCenter + LTVector(-m_vDims.x,  m_vDims.y,  -m_vDims.z);
    m_vFrontBottomRight = m_vCenter + LTVector(m_vDims.x,   -m_vDims.y, m_vDims.z);
    m_vFrontBottomLeft  = m_vCenter + LTVector(-m_vDims.x,  -m_vDims.y, m_vDims.z);
    m_vBackBottomRight  = m_vCenter + LTVector(m_vDims.x,   -m_vDims.y, -m_vDims.z);
    m_vBackBottomLeft   = m_vCenter + LTVector(-m_vDims.x,  -m_vDims.y, -m_vDims.z);

	LTVector vAngles;
    if ( g_pLTServer->GetPropRotationEuler( "Rotation", &vAngles ) == LT_OK )
	{
		LTRotation rRot(EXPANDVEC(vAngles));
		m_vForward = rRot.Forward();

		// The volume must either point parallel to x or z.

		if( fabs( m_vForward.x ) > fabs( m_vForward.z ) )
		{
			if( m_vForward.x > 0.f )
			{
				m_vForward = LTVector( 1.f, 0.f, 0.f );
			}
			else {
				m_vForward = LTVector( -1.f, 0.f, 0.f );
			}
		}
		else {
			if( m_vForward.z > 0.f )
			{
				m_vForward = LTVector( 0.f, 0.f, 1.f );
			}
			else {
				m_vForward = LTVector( 0.f, 0.f, -1.f );
			}
		}
	}

	return true;
}

void AIGeometry::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vDims);
	LOAD_VECTOR(m_vForward);
	LOAD_VECTOR(m_vCenter);
	LOAD_VECTOR(m_vFrontTopLeft);
	LOAD_VECTOR(m_vFrontTopRight);
	LOAD_VECTOR(m_vBackTopLeft);
	LOAD_VECTOR(m_vBackTopRight);
	LOAD_VECTOR(m_vFrontBottomLeft);
	LOAD_VECTOR(m_vFrontBottomRight);
	LOAD_VECTOR(m_vBackBottomLeft);
	LOAD_VECTOR(m_vBackBottomRight);
}

void AIGeometry::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vDims);
	SAVE_VECTOR(m_vForward);
	SAVE_VECTOR(m_vCenter);
	SAVE_VECTOR(m_vFrontTopLeft);
	SAVE_VECTOR(m_vFrontTopRight);
	SAVE_VECTOR(m_vBackTopLeft);
	SAVE_VECTOR(m_vBackTopRight);
	SAVE_VECTOR(m_vFrontBottomLeft);
	SAVE_VECTOR(m_vFrontBottomRight);
	SAVE_VECTOR(m_vBackBottomLeft);
	SAVE_VECTOR(m_vBackBottomRight);
}

LTBOOL AIGeometry::Intersects(const AIGeometry* pGeometry) const
{
	if( m_vFrontTopRight.x < pGeometry->m_vBackBottomLeft.x ) return LTFALSE;
	if( m_vBackBottomLeft.x > pGeometry->m_vFrontTopRight.x ) return LTFALSE;

	if( m_vFrontTopRight.z < pGeometry->m_vBackBottomLeft.z ) return LTFALSE;
	if( m_vBackBottomLeft.z > pGeometry->m_vFrontTopRight.z ) return LTFALSE;

	if( m_vFrontTopRight.y < pGeometry->m_vBackBottomLeft.y ) return LTFALSE;
	if( m_vBackBottomLeft.y > pGeometry->m_vFrontTopRight.y ) return LTFALSE;

	return LTTRUE;
}

LTBOOL AIGeometry::Inside2d(const LTVector& vPos, LTFLOAT fThreshhold) const
{
	// More likely to early out on X or Z than Y, so do in that order.

	return ( (vPos.x <= m_vFrontTopRight.x-fThreshhold) &&
			 (vPos.x >= m_vFrontTopLeft.x+fThreshhold) &&
			 (vPos.z <= m_vFrontTopLeft.z-fThreshhold) &&
			 (vPos.z >= m_vBackTopLeft.z+fThreshhold) );
}

LTBOOL AIGeometry::Inside3D( const LTVector& vPos ) const
{
	return ( (vPos.x <= m_vFrontTopRight.x) &&
			 (vPos.x >= m_vFrontTopLeft.x) &&
			 (vPos.z <= m_vFrontTopLeft.z) &&
			 (vPos.z >= m_vBackTopLeft.z) &&
			 (vPos.y <= m_vBackTopLeft.y) &&
			 (vPos.y >= m_vBackBottomLeft.y) );
}


LTBOOL AIGeometry::Inside2dLoose(const LTVector& vPos,
								 LTFLOAT fThreshhold,
								 const AISpatialNeighbor* const pVolumes,
								 uint32 nNeighborCount) const
{
	if ( Inside2d(vPos, fThreshhold) )
	{
		return LTTRUE;
	}

	LTFLOAT fThreshholdSqr = fThreshhold*fThreshhold;

	for ( uint32 iNeighbor = 0 ; iNeighbor < nNeighborCount; iNeighbor++ )
	{
		// For each neighbor, check to see our sphere (pos w/ radius
		// fThreshhold) intersects the connection segment, and is at least
		// fThreshhold units away from the endpoints of the connection segment

		const AISpatialNeighbor& Neighbor = *pVolumes;

		LTVector R = vPos; R.y = 0.0f;

		LTVector P0 = Neighbor.GetConnectionEndpoint1(); P0.y = 0.0f;
		LTVector P1 = Neighbor.GetConnectionEndpoint2(); P1.y = 0.0f;
		LTVector v = Neighbor.GetConnectionDir();
		LTFLOAT fLength = Neighbor.GetConnectionLength();

		LTFLOAT t = (R - P0).Dot(v);

		if ( (t > fThreshhold) && (t < (fLength - fThreshhold)) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}


bool AIGeometry::BoxInside( const LTVector& vCenter, const LTVector& vDims ) const
{
	return (
		Inside3D(vCenter+LTVector(vDims.x/2,		vDims.y/2,	vDims.z/2) ) && 
		Inside3D(vCenter+LTVector(-vDims.x/2,	-vDims.y/2,	-vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(-vDims.x/2,	vDims.y/2,	vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(vDims.x/2,		vDims.y/2,	-vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(-vDims.x/2,	-vDims.y/2,	vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(vDims.x/2,		-vDims.y/2,	-vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(-vDims.x/2,	vDims.y/2,	-vDims.z/2) ) &&
		Inside3D(vCenter+LTVector(vDims.x/2,		-vDims.y/2,	vDims.z/2) ) );
}

LTBOOL AIGeometry::InsideMasked(const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold /* = 0.0f*/) const
{
	// More likely to early out on X or Z than Y, so do in that order.

	// if mask requires the test, and if it is true, then continue.
	// if mask requires test and fails, then return LTFALSE
	// if mask does NOT require the test, doesn't matter

	return (
		((eAxisX & ~iAxisMask ) || ((vPos.x <= m_vFrontTopRight.x) && (vPos.x >= m_vFrontTopLeft.x)))	&&
		((eAxisZ & ~iAxisMask ) || ((vPos.z <= m_vFrontTopLeft.z) && (vPos.z >= m_vBackTopLeft.z)))	&&
		((eAxisY & ~iAxisMask ) || ((vPos.y <= (m_vFrontTopLeft.y+fVerticalThreshhold)) && (vPos.y >= (m_vFrontBottomLeft.y-fVerticalThreshhold))))
	);
}

LTBOOL AIGeometry::Inside(const LTVector& vPos, LTFLOAT fRadius2D, LTFLOAT fVerticalThreshhold) const
{
	// More likely to early out on X or Z than Y, so do in that order.

	if( ((vPos.x - fRadius2D) > m_vFrontTopRight.x) ||
		((vPos.x + fRadius2D) < m_vFrontTopLeft.x) ||
		((vPos.z - fRadius2D) > m_vFrontTopLeft.z) ||
		((vPos.z + fRadius2D) < m_vBackTopLeft.z) ||
		(vPos.y > (m_vFrontTopLeft.y+fVerticalThreshhold)) ||
		(vPos.y < (m_vFrontBottomLeft.y-fVerticalThreshhold)) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------


AISpatialRepresentation::AISpatialRepresentation()
{
	m_hstrRegion = LTNULL;
	m_pRegion = LTNULL;

	m_cNeighbors = 0;
    m_aNeighbors = LTNULL;

	// By default, everyone can use every volume.
	// Subclasses may override these flags for specific volume types.

	m_dwUseBy = kUseBy_All;

	m_bVolumeEnabled = LTTRUE;
}

AISpatialRepresentation::~AISpatialRepresentation()
{
	FREE_HSTRING(m_hstrRegion);

	if ( m_aNeighbors )
	{
		debug_deletea(m_aNeighbors);
		m_aNeighbors = LTNULL;
	}
}

void AISpatialRepresentation::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	// Intentionally do not load/save m_hstrRegion.
	// The region saves and loads its list of volumes.

	LOAD_COBJECT(m_pRegion, AIRegion);
	LOAD_DWORD(m_dwUseBy);
	LOAD_BOOL(m_bVolumeEnabled);

	LOAD_INT(m_cNeighbors);
	m_aNeighbors = debug_newa(AIVolumeNeighbor, m_cNeighbors);
	for ( uint32 iNeighbor = 0 ; iNeighbor < m_cNeighbors ; iNeighbor++ )
	{
		m_aNeighbors[iNeighbor].Load(pMsg);
	}
}

void AISpatialRepresentation::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	// Intentionally do not load/save m_hstrRegion.
	// The region saves and loads its list of volumes.

	SAVE_COBJECT(m_pRegion);
	SAVE_DWORD(m_dwUseBy);
	SAVE_BOOL(m_bVolumeEnabled);

	SAVE_INT(m_cNeighbors);
	for ( uint32 iNeighbor = 0 ; iNeighbor < m_cNeighbors ; iNeighbor++ )
	{
		m_aNeighbors[iNeighbor].Save(pMsg);
	}

}

bool AISpatialRepresentation::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

    if ( g_pLTServer->GetPropGeneric("Region", &g_gp) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrRegion = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	return true;
}

void AISpatialRepresentation::InitialUpdate(float fUpdateType)
{
	super::InitialUpdate(fUpdateType);

	HOBJECT hRegion = LTNULL;
	if ( m_hstrRegion && ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrRegion), hRegion) ) )
	{
		AIError("Volume \"%s\" - Invalid Region \"%s\"", GetName(), ::ToString(m_hstrRegion));
		return;
	}

	if( hRegion )
	{
		if( !IsAIRegion(hRegion) )
		{
			AIError("Volume \"%s\" - Invalid Region \"%s\"", GetName(), ::ToString(m_hstrRegion));
		}
		else
		{
			m_pRegion = (AIRegion*)g_pLTServer->HandleToObject(hRegion);
			AddToRegion( m_pRegion );

			// Forget name of region and do not save it.
			// Region saves/loads list of volumes.

			FREE_HSTRING(m_hstrRegion);
		}
	}
}

void AISpatialRepresentation::InitNeighbors(AISpatialRepresentation** apVolumeNeighbors, uint32 cNeighbors)
{
    LTBOOL abValidNeighbors[32];

	{for ( uint32 iNeighbor = 0 ; iNeighbor < cNeighbors ; iNeighbor++ )
	{
        LTVector vFrontLeft(0,0,0);
        LTVector vBackRight(0,0,0);

		AISpatialRepresentation* pNeighbor = apVolumeNeighbors[iNeighbor];

		vFrontLeft.x = Max<LTFLOAT>(GetFrontTopLeft().x, pNeighbor->GetFrontTopLeft().x);
		vFrontLeft.z = Min<LTFLOAT>(GetFrontTopLeft().z, pNeighbor->GetFrontTopLeft().z);

		vBackRight.x = Min<LTFLOAT>(GetBackTopRight().x, pNeighbor->GetBackTopRight().x);
		vBackRight.z = Max<LTFLOAT>(GetBackTopRight().z, pNeighbor->GetBackTopRight().z);

		// Make sure this isn't an unintended intersection

		if ( vFrontLeft.DistSqr(vBackRight) < c_fNeighborThreshhold )
		{
            abValidNeighbors[iNeighbor] = LTFALSE;
		}
		else
		{
            abValidNeighbors[iNeighbor] = LTTRUE;
			m_cNeighbors++;
		}
	}}

	m_aNeighbors = debug_newa(AIVolumeNeighbor, m_cNeighbors);

	m_cNeighbors = 0;

	{for ( uint32 iNeighbor = 0 ; iNeighbor < cNeighbors ; iNeighbor++ )
	{
		if ( abValidNeighbors[iNeighbor] )
		{
			if( m_aNeighbors[m_cNeighbors].Init(this, apVolumeNeighbors[iNeighbor]) )
			{
				m_cNeighbors++;
			}
		}
	}}
}

LTBOOL AISpatialRepresentation::Inside2dLoose(const LTVector& vPos,LTFLOAT fThreshhold )
{
	return AIGeometry::Inside2dLoose( vPos, fThreshhold, m_aNeighbors, m_cNeighbors );
}

AISpatialNeighbor* AISpatialRepresentation::GetSpatialNeighborByIndex(uint32 iNeighbor)
{
	UBER_ASSERT( iNeighbor <= m_cNeighbors, 
		"GetNeighborByIndex: Attempting to get out of range AIVolumeNeighbor" );

	return &m_aNeighbors[iNeighbor];
}


int AISpatialRepresentation::DrawSelf()
{
	DebugLineSystem& system = LineSystem::GetSystem(this,"ShowVolume");
	system.Clear( );

	// draw this last because the debug string code is hacked in to use the last colors and positions added.
	system	<< LineSystem::Box(GetCenter(), GetDims(), GetDebugColor(), 126);

	for(uint32 i = 0; i < m_cNeighbors; ++i )
	{
		system << LineSystem::Line( m_aNeighbors[i].GetConnectionEndpoint1(),
			                        m_aNeighbors[i].GetConnectionEndpoint2(),
									Color::Green, 126 );
		system << LineSystem::Arrow( m_aNeighbors[i].GetConnectionPos(),
			                         m_aNeighbors[i].GetConnectionPos() + m_aNeighbors[i].GetConnectionPerpDir()*10.0f,
									 Color::Green, 126 );
	}

	// Draw the SpatialRepresentations

	// draw this last because the debug string code is hacked in to use the last colors and positions added.
	LTVector pt = GetCenter() + GetForward();
	system	<< LineSystem::Line(GetCenter(), pt, GetDebugColor(), 126);

	system.SetDebugString(GetName());

	return 0;
}

int AISpatialRepresentation::HideSelf()
{
	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowVolume");
	system.SetDebugString("");
	system.Clear( );
	return 0;
}


LTBOOL AISpatialRepresentation::HasRegion() const
{
	return m_pRegion != LTNULL;
}

AIRegion* AISpatialRepresentation::GetRegion() const
{
//	_ASSERT(HasRegion());
	return m_pRegion;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::Con/Destructor()
//              
//	PURPOSE:	Construct/Destruct
//              
//----------------------------------------------------------------------------

AIInformationVolume::AIInformationVolume()
{
	m_dwSenseMask = 0xffffffff;
	m_eInfoVolumeState = kIVS_Off;
}

AIInformationVolume::~AIInformationVolume()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::ReadProp()
//              
//	PURPOSE:	Read properties.
//              
//----------------------------------------------------------------------------

bool AIInformationVolume::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

	// Sense Mask.

    if ( g_pLTServer->GetPropGeneric("SenseMask", &g_gp) == LT_OK )
	{
		if ( g_gp.m_String[0] && ( stricmp(g_gp.m_String, "None") != 0) )
		{
			uint32 iSenseMask = g_pAIButeMgr->GetSenseMaskIDByName(g_gp.m_String);
			m_dwSenseMask = ~(g_pAIButeMgr->GetSenseMask(iSenseMask)->flagsBlockedSenses);
		}
	}

	// On/Off State.

	LTBOOL bOn = LTFALSE;
	READPROP_BOOL("On", bOn);

	if ( bOn )
	{
		m_eInfoVolumeState = kIVS_On;
	}
	else
	{
		m_eInfoVolumeState = kIVS_Off;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::Load / Save()
//              
//	PURPOSE:	Load and Save.
//              
//----------------------------------------------------------------------------

void AIInformationVolume::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD(m_dwSenseMask);
	LOAD_DWORD_CAST(m_eInfoVolumeState, EnumInfoVolumeState);
}

void AIInformationVolume::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD(m_dwSenseMask);
	SAVE_DWORD(m_eInfoVolumeState);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::GetNeighborByIndex()
//              
//	PURPOSE:	Returns the AIInformationVolumeNeighbor 
//              
//----------------------------------------------------------------------------
AIInformationVolume* AIInformationVolume::GetNeighborByIndex(uint32 iNeighbor)
{
	return (AIInformationVolume*)GetSpatialNeighborByIndex( iNeighbor );
}

void AIInformationVolume::AddToRegion(AIRegion* pRegion)
{
	// Needed?  Currently this is not used, but it ought to be
	// implemented.  Do regions need lists of Information Volumes AND
	// standard volumes?  
//	pRegion->AddVolume( this );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::IsConnected()
//              
//	PURPOSE:	AIInformationVolume are connected ONLY if they are next to
//				eachother
//              
//----------------------------------------------------------------------------
bool AIInformationVolume::IsConnected( AISpatialRepresentation* pVolume )
{
	return ( Intersects(pVolume) == LTTRUE );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolume::OnTrigger()
//              
//	PURPOSE:	Handle trigger messages.
//              
//----------------------------------------------------------------------------
bool AIInformationVolume::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if (super::OnTrigger(hSender, cMsg))
		return true;

	if ( cMsg.GetArg(0) == s_cTok_On )
	{
		AITRACE( AIShowVolumes, ( GetName(), "InfoVolume ON" ) );
		m_eInfoVolumeState = kIVS_On;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Off )
	{
		AITRACE( AIShowVolumes, ( GetName(), "InfoVolume OFF" ) );
		m_eInfoVolumeState = kIVS_Off;
	}
	else if( cMsg.GetArg(0) == s_cTok_Remove )
	{
		AIError( "Attempting to remove AIInformationVolume \"%s\"! Turning Off instead.", GetName() );
		m_eInfoVolumeState = kIVS_Off;
	}
	else 
		return false;

	return true;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIInformationVolumePlugin::PreHook_EditStringList()
//              
//	PURPOSE:	Fill lists in Dedit.
//              
//----------------------------------------------------------------------------

extern CAIButeMgr s_AIButeMgr;

LTRESULT AIInformationVolumePlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	bool bPluginInitted = false;

	if ( !bPluginInitted )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\AIButes.txt", szRezPath);
		s_AIButeMgr.SetInRezFile(LTFALSE);
        s_AIButeMgr.Init(szFile);

		bPluginInitted = true;
	}

	if ( !_strcmpi("SenseMask", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cSenseMasks = s_AIButeMgr.GetNumSenseMasks();
		_ASSERT(cMaxStrings >= cSenseMasks);
		for ( uint32 iSenseMask = 0 ; iSenseMask < cSenseMasks ; iSenseMask++ )
		{
			strcpy(aszStrings[(*pcStrings)++], s_AIButeMgr.GetSenseMask(iSenseMask)->szName);
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// Statics:

const char* AIVolume::ms_aszVolumeTypes[] =
{
	#define VOLUME_TYPE_AS_STRING 1
	#include "AIVolumeTypeEnums.h"
	#undef VOLUME_TYPE_AS_STRING
};

AIVolume::AIVolume()
{
	m_bLit = LTTRUE;
	m_hLightSwitchUseObjectNode = LTNULL;
	m_hstrLightSwitchUseObjectNode = LTNULL;

	m_bLitInitially = LTTRUE;

	m_fPathWeight = 1.f;
	m_hReservedBy = LTNULL;

    m_bHadDoors = LTFALSE;
	m_cDoors = 0;

	for (uint32 nCurDoor = 0; nCurDoor < kMaxDoors; ++nCurDoor)
	{
		m_ahDoors[nCurDoor].SetReceiver(*this);
	}
	
	m_nPathIndex = 0;
}

AIVolume::~AIVolume()
{
	FREE_HSTRING(m_hstrLightSwitchUseObjectNode);
}

uint32 AIVolume::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	return AISpatialRepresentation::ObjectMessageFn(hSender, pMsg );
}

bool AIVolume::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Lit("LIT");
	static CParsedMsg::CToken s_cTok_Bright("BRIGHT");
	static CParsedMsg::CToken s_cTok_Unlit("UNLIT");
	static CParsedMsg::CToken s_cTok_Dark("DARK");
	static CParsedMsg::CToken s_cTok_Enable("ENABLE");
	static CParsedMsg::CToken s_cTok_Disable("DISABLE");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if ( (cMsg.GetArg(0) == s_cTok_Lit) || (cMsg.GetArg(0) == s_cTok_Bright) )
	{
		SetLightState( LTTRUE );
	}
	else if ( (cMsg.GetArg(0) == s_cTok_Unlit) || (cMsg.GetArg(0) == s_cTok_Dark) )
	{
		SetLightState( LTFALSE );
	}
	else if( cMsg.GetArg(0) == s_cTok_Enable )
	{
		m_bVolumeEnabled = LTTRUE;

		HOBJECT hObject = LTNULL;
		AITRACE( AIShowVolumes, ( hObject, "Enabling volume '%s'", GetName() ) );

		// All AIs need to clear existing knowledge of paths.

		//if we're drawing debug stuff, update
		if (g_ShowVolumesTrack.GetFloat())
		{
			HideSelf();
			DrawSelf();
		}

		g_pAIPathMgr->InvalidatePathKnowledge();
	}
	else if( cMsg.GetArg(0) == s_cTok_Disable )
	{
		HOBJECT hObject = LTNULL;
		AITRACE( AIShowVolumes, ( hObject, "Disabling volume '%s'", GetName() ) );

		m_bVolumeEnabled = LTFALSE;
		//if we're drawing debug stuff, update
		if (g_ShowVolumesTrack.GetFloat())
		{
			HideSelf();
			DrawSelf();
		}

		// All AIs need to clear existing knowledge of paths.

		g_pAIPathMgr->InvalidatePathKnowledge();
	}
	else if( cMsg.GetArg(0) == s_cTok_Remove )
	{
		AIError( "Attempting to remove AIVolume \"%s\"! Disabling instead.", GetName() );
		m_bVolumeEnabled = LTFALSE;
		//if we're drawing debug stuff, update
		if (g_ShowVolumesTrack.GetFloat())
		{
			HideSelf();
			DrawSelf();
		}
	}
	else 
		return AISpatialRepresentation::OnTrigger( hSender, cMsg );

	return true;
}


void AIVolume::SetLightState(LTBOOL bLit)
{
	// Nothing changed.

	if( m_bLit == bLit )
	{
		return;
	}

	// Record new light state.

	m_bLit = bLit;

	//if we're drawing debug stuff, update
	if (g_ShowVolumesTrack.GetFloat())
	{
		HideSelf();
		DrawSelf();
	}


	// Do not create stimulus from door volumes.
	// When AIs investigate by walking to the center of the volume, there
	// are pathing issues.

	if( HasDoors() )
	{
		return;
	}

	// Check if an AI activated the light last.
	// If no light switch was listed in the volume, we have no
	// way of checking if the AI lit it, so do not create a 
	// stimulus.

	if( !m_hLightSwitchUseObjectNode )
	{
		return;
	}

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hLightSwitchUseObjectNode );
	if( pNode && ( ( g_pLTServer->GetTime() - pNode->GetNodeLastActivationTime() ) < 0.5f ) )
	{
		return;
	}

	// Create stimulus so AIs notice changes in light state (lit / unlit).

	// Act like this is coming from a random player, since AIVolumes are lite objects
	CPlayerObj *pRandomPlayer = g_pCharacterMgr->FindPlayer();

	// Re-use the AIButeMgr name for the AI instances (trying to use it as a
	// more consistant 'access key' for the AI.
	CDataUser du;
	du.InitData( "GoodCharacter" , pRandomPlayer->m_hObject );

	LTFLOAT fStimRadius = Max( GetDims().x, GetDims().z );

	g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyLightDisturbanceVisible, 1, pRandomPlayer->m_hObject, m_hLightSwitchUseObjectNode, du.GetData(), GetCenter(), fStimRadius, 1.f );
}

void AIVolume::InitialUpdate(float fUpdateType)
{
	super::InitialUpdate(fUpdateType);

	if( !m_pRegion ) 
	{
		AIError("Volume \"%s\" is not in a region!", GetName() );
	}

	if( m_hstrLightSwitchUseObjectNode )
	{
		HOBJECT hObj;
		LTRESULT res = FindNamedObject( m_hstrLightSwitchUseObjectNode, hObj );
		m_hLightSwitchUseObjectNode = hObj;
		if ( LT_OK != res )
		{
			AIError( "Volume \"%s\" - Invalid LightSwitch UseObjectNode \"%s\"", GetName(), ::ToString(m_hstrLightSwitchUseObjectNode) );
			return;
		}
  
		if( !IsAINodeUseObject( m_hLightSwitchUseObjectNode ) )
		{
			AIError("Volume \"%s\" - Invalid LightSwitch UseObjectNode \"%s\"", GetName(), ::ToString(m_hstrLightSwitchUseObjectNode) );
		}
		else
		{
			FREE_HSTRING(m_hstrLightSwitchUseObjectNode);
  
			AINodeUseObject* pNodeLightSwitch = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchUseObjectNode );
			HSTRING hstrCmd = pNodeLightSwitch->GetSmartObjectCommand( kNode_LightSwitch );	
			if( !hstrCmd )
			{
				AIError("Volume \"%s\" - No LightSwitch SmartObject on UseObjectNode \"%s\"", GetName(), ::ToString(m_hstrLightSwitchUseObjectNode) );
			}
		}
	}

	// It is an error if a volume does not list a light switch node, and 
	// is not a special volume, and is next to a volume that does list a 
	// lightswitch node.

	else if( ( m_cDoors == 0 ) &&
			 ( !m_hLightSwitchUseObjectNode ) &&
			 ( GetVolumeType() & ( kVolumeType_BaseVolume | kVolumeType_Junction ) ) )
	{
		AIVolumeNeighbor* pNeighbor;
		for( uint32 iNeighbor = 0; iNeighbor < m_cNeighbors; ++iNeighbor )
		{
			pNeighbor = GetNeighborByIndex( iNeighbor );
			if( pNeighbor )
			{
				if( pNeighbor->GetVolume()->GetLightSwitchUseObjectNodeName() ||
					pNeighbor->GetVolume()->GetLightSwitchUseObjectNode() )
				{	
					AIError( "Volume \"%s\" DOES NOT LIST A LIGHTSWITCH NODE.", GetName() );
				}
			}
		}
	}

  
	if( !m_pRegion )
	{
		AIError("Volume \"%s\" has no Region", GetName() );
	}
}


bool AIVolume::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

    if ( (g_pLTServer->GetPropGeneric("Lit", &g_gp) == LT_OK) )
	{
		m_bLit = g_gp.m_Bool;
		m_bLitInitially = m_bLit;
	}

	if ( g_pLTServer->GetPropGeneric("LightSwitchNode", &g_gp) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrLightSwitchUseObjectNode = g_pLTServer->CreateString( g_gp.m_String );
		}
	}
  
    if ( g_pLTServer->GetPropGeneric("PreferredPath", &g_gp) == LT_OK )
	{
		if( !g_gp.m_Bool )
		{
			m_fPathWeight = kUnpreferablePathWeight;
		}
	}

	return true;
}

void AIVolume::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_fPathWeight);
	LOAD_HOBJECT(m_hReservedBy);
	LOAD_BOOL(m_bHadDoors);
	LOAD_INT(m_cDoors);
	LOAD_BOOL(m_bLit);
	LOAD_BOOL(m_bLitInitially);
	LOAD_HOBJECT(m_hLightSwitchUseObjectNode);

	for ( uint32 iDoor = 0 ; iDoor < kMaxDoors ; iDoor++ )
	{
		LOAD_HOBJECT(m_ahDoors[iDoor]);
		ASSERT((m_ahDoors[iDoor] == 0) || 
			g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(m_ahDoors[iDoor]), g_pLTServer->GetClass("Door")));
	}
}

void AIVolume::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fPathWeight);
	SAVE_HOBJECT(m_hReservedBy);
	SAVE_BOOL(m_bHadDoors);
	SAVE_INT(m_cDoors);
	SAVE_BOOL(m_bLit);
	SAVE_BOOL(m_bLitInitially);
	SAVE_HOBJECT(m_hLightSwitchUseObjectNode);

	for ( uint32 iDoor = 0 ; iDoor < kMaxDoors ; iDoor++ )
	{
		SAVE_HOBJECT(m_ahDoors[iDoor]);
	}
}

int AIVolume::Init()
{
	AIASSERT1(m_fPathWeight > 0.f, LTNULL, "AIVolume::Init: %s : PathWeight must be greater than 0", GetName());

	// Find any doors located in our volume

    HCLASS  hDoor = g_pLTServer->GetClass("Door");
    HOBJECT hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hDoor))
		{
            Door* pDoor = (Door*)g_pLTServer->HandleToObject(hCurObject);
			if ( !pDoor->IsAITriggerable() ) continue;

            LTVector vPos;
            LTVector vDims;

            g_pLTServer->GetObjectPos(hCurObject, &vPos);
            g_pPhysicsLT->GetObjectDims(hCurObject, &vDims);

			if ( InsideMasked(vPos, eAxisAll, vDims.y*1.1f) )
			{
				if ( m_cDoors == kMaxDoors )
				{
                    Warn("Volume \"%s\" has too many doors", GetName());
				}
				else
				{
                    m_bHadDoors = LTTRUE;
					m_ahDoors[m_cDoors++] = hCurObject;
				}
			}
		}
	}

    hCurObject = LTNULL;
    while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hDoor))
		{
            Door* pDoor = (Door*)g_pLTServer->HandleToObject(hCurObject);
			if ( !pDoor->IsAITriggerable() ) continue;

            LTVector vPos;
            LTVector vDims;

            g_pLTServer->GetObjectPos(hCurObject, &vPos);
            g_pPhysicsLT->GetObjectDims(hCurObject, &vDims);

			if ( InsideMasked(vPos, eAxisAll, vDims.y*1.1f) )
			{
				if ( m_cDoors == kMaxDoors )
				{
                    Warn("Volume \"%s\" has too many doors", GetName());
				}
				else
				{
                    m_bHadDoors = LTTRUE;
					m_ahDoors[m_cDoors++] = hCurObject;
				}
			}
		}
	}

	// Validate volume dims. Must be 48x48 if there's no doors. One side must be <= 128 if there are doors

	if ( m_cDoors == 0 )
	{
		if ( GetDims().x < AIGeometry::kMinXZDim * 0.5f )
		{
            Warn("Volume \"%s\" is only %d units wide/x (should be at least 48)", GetName(), (int)GetDims().x*2);
		}

		if ( GetDims().z < AIGeometry::kMinXZDim * 0.5f )
		{
            Warn("Volume \"%s\" is only %d units deep/z (should be at least 48)", GetName(), (int)GetDims().z*2);
		}
	}
	else //	if ( m_cDoors != 0 )
	{
		if ( GetDims().x >= 128 && GetDims().z >= 128 )
		{
            Warn("Volume \"%s\" is a suspiciously large door volume", GetName());
		}
	}

	return 0;
}

void AIVolume::OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj) 
{ 
	ASSERT(m_cDoors); 
	--m_cDoors; 
}

AIVolumeNeighbor* AIVolume::GetNeighborByIndex(uint32 iNeighbor)
{
	return (AIVolumeNeighbor*)GetSpatialNeighborByIndex( iNeighbor );
}

void AIVolume::AddToRegion(AIRegion* pRegion)
{
	pRegion->AddVolume( this );
}

LTBOOL AIVolume::AreDoorsLocked( HOBJECT hChar ) const
{
	_ASSERT(HasDoors());

	// TODO: Make AIVolumeDoor, move this functionality into it!!!
	// If this is a door volume, make sure the doors aren't locked

	LTBOOL bLocked = LTFALSE;

	for ( uint32 iDoor = 0 ; iDoor < kMaxDoors ; iDoor++ )
	{
		HOBJECT hDoor = GetDoor(iDoor);
		if ( hDoor )
		{
			Door* pDoor = (Door*)g_pLTServer->HandleToObject(hDoor);
			if ( pDoor->IsLockedForCharacter( hChar ) )
			{
				bLocked = LTTRUE;
				break;
			}
		}	
	}

	return bLocked;
}

bool AIVolume::IsConnected( AISpatialRepresentation* pVolume )
{
	return ( Intersects(pVolume) == LTTRUE );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolume::CanBuildPathTo()
//              
//	PURPOSE:	By default, check to see if the AI is allowed to use this
//				type of volume.  Returns true if the AI can use the volume,
//				false if it cannot.
//
//				Note: Special case for derived volumes
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL AIVolume::CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext )
{
	return ( pAI->GetCurValidVolumeMask() & GetVolumeType() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolume::CanBuildPathFrom()
//              
//	PURPOSE:	By default, check to see if the AI is allowed to use this
//				type of volume.  Returns true if the AI can use the volume,
//				false if it cannot.
//
//				Note: Special case for derived volumes
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL AIVolume::CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev )
{
	return ( pAI->GetCurValidVolumeMask() & GetVolumeType() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolume::ReserveVolume()
//              
//	PURPOSE:	Set an AI that is reserving this volume, making
//				it unpreferable for others to path thru.
//              
//----------------------------------------------------------------------------

void AIVolume::ReserveVolume(HOBJECT hAI)
{
	// It is OK to over-write previous reservations.
	
	m_hReservedBy = hAI;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolume::ClearVolumeReservation()
//              
//	PURPOSE:	AI is no longer reserving this volume.
//              
//----------------------------------------------------------------------------

void AIVolume::ClearVolumeReservation(HOBJECT hAI)
{
	// Only clear the record if this AI currently has the reservation.

	if( m_hReservedBy == hAI )
	{
		m_hReservedBy = LTNULL;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolume::GetPathWeight()
//              
//	PURPOSE:	Get the weight for using this volume in a path.
//				Lower-weighted volumes are more prefereable.
//				Reserved volumes are less preferable.
//              
//----------------------------------------------------------------------------

LTFLOAT AIVolume::GetPathWeight(LTBOOL bIncludeBaseWeight, LTBOOL bDivergePaths) const
{
	// Include base weight, or just set to 1.
	// This allows AI to ignore preferred paths when alert.

	LTFLOAT fWeight = ( bIncludeBaseWeight ) ? m_fPathWeight : 1.f;

	// If requester wants paths to diverge, check for reserved volumes.

	if( bDivergePaths && m_hReservedBy )
	{
		return fWeight + kUnpreferablePathWeight; 
	}

	return fWeight; 
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumeAmbientLife)
END_CLASS_DEFAULT_FLAGS(AIVolumeAmbientLife, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeAmbientLife )
CMDMGR_END_REGISTER_CLASS( AIVolumeAmbientLife, AIVolume )

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumeLadder)
END_CLASS_DEFAULT_FLAGS(AIVolumeLadder, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeLadder )
CMDMGR_END_REGISTER_CLASS( AIVolumeLadder, AIVolume )


AIVolumeLadder::AIVolumeLadder()
{
}

AIVolumeLadder::~AIVolumeLadder()
{
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumeStairs)
END_CLASS_DEFAULT_FLAGS(AIVolumeStairs, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeStairs )
CMDMGR_END_REGISTER_CLASS( AIVolumeStairs, AIVolume )


void AIVolumeStairs::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AIVolumeStairs::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumeLedge)
END_CLASS_DEFAULT_FLAGS(AIVolumeLedge, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeLedge )
CMDMGR_END_REGISTER_CLASS( AIVolumeLedge, AIVolume )


void AIVolumeLedge::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AIVolumeLedge::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumePlayerInfo)

	ADD_DEDIT_COLOR( AIVolumePlayerInfo )

	ADD_STRINGPROP_FLAG(Region,			"",			0|PF_OBJECTLINK|PF_HIDDEN)

    ADD_BOOLPROP(Hiding, LTFALSE)
    ADD_BOOLPROP(HidingCrouch, LTFALSE)

	PROP_DEFINEGROUP(ViewNodes, PF_GROUP(1)) \

		ADD_STRINGPROP_FLAG(ViewNode1, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode2, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode3, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode4, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode5, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode6, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode7, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode8, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode9, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode10, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode11, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode12, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode13, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode14, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode15, "", PF_GROUP(1)|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(ViewNode16, "", PF_GROUP(1)|PF_OBJECTLINK)

END_CLASS_DEFAULT_FLAGS(AIVolumePlayerInfo, AIInformationVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS(AIVolumePlayerInfo)
CMDMGR_END_REGISTER_CLASS(AIVolumePlayerInfo, AIInformationVolume)

AIVolumePlayerInfo::AIVolumePlayerInfo()
{
	m_bHiding = LTFALSE;
	m_bHidingCrouchRequired = LTFALSE;

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		m_ahstrViewNodes[iViewNode] = LTNULL;
		m_ahViewNodes[iViewNode] = LTNULL;
	}

	// Only the player cares about these volumes.

	m_dwUseBy = kUseBy_Player;
}

AIVolumePlayerInfo::~AIVolumePlayerInfo()
{
	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		FREE_HSTRING(m_ahstrViewNodes[iViewNode]);
	}
}

bool AIVolumePlayerInfo::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp(pocs) )
	{
		return false;
	}

	READPROP_BOOL("Hiding", m_bHiding);
	READPROP_BOOL("HidingCrouch", m_bHidingCrouchRequired);

	// Ensure hiding is enabled if the crouch flag is set.

	if( m_bHidingCrouchRequired )
	{
		m_bHiding = LTTRUE;
	}

	// Read view nodes.

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		char szBuffer[128];
		sprintf(szBuffer, "ViewNode%d", iViewNode+1);

        if ( g_pLTServer->GetPropGeneric( szBuffer, &g_gp) == LT_OK )
		{
			if ( g_gp.m_String[0] )
			{
                m_ahstrViewNodes[iViewNode] = g_pLTServer->CreateString( g_gp.m_String );
			}
		}
	}

	return true;
}

int AIVolumePlayerInfo::Init()
{
	super::Init();

	// Get the view volumes

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		if ( m_ahstrViewNodes[iViewNode] )
		{
			AINode* pNode = g_pAINodeMgr->GetNode(m_ahstrViewNodes[iViewNode]);

			if ( pNode )
			{
				m_ahViewNodes[iViewNode] = pNode->m_hObject;
			}
			else
			{
				Warn("Volume \"%s\" has a view node that does not exist", GetName());
			}

			FREE_HSTRING(m_ahstrViewNodes[iViewNode]);
		}
	}

	return 0;
}

AINode* AIVolumePlayerInfo::FindViewNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTBOOL bMustBeUnowned) const
{
	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		HOBJECT hViewNode = m_ahViewNodes[iViewNode];
		AINode* pNode = (hViewNode == LTNULL ? LTNULL : AINode::HandleToObject(hViewNode));
		if ( pNode && ( ( pNode->GetType() == eNodeType ) || pNode->NodeTypeIsActive( eNodeType ) ) )
		{
			if( bMustBeUnowned && pNode->GetNodeOwner() )
			{
				continue;
			}

			// Skip nodes in unreachable volumes.

			if( pPathKnowledgeMgr && 
				( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
			{
				continue;
			}

			// Skip nodes that are not in volumes.

			if( !pNode->GetNodeContainingVolume() )
			{
				continue;
			}

			// Skip node if required alignment does not match.

			if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
				( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
			{
				continue;
			}

			if ( !pNode->IsLockedDisabledOrTimedOut() )
			{
				// Ensure that AI can pathfind to the destination node.
				// Ideally, we would like to do this check for each node as we iterate,
				// but that could result in multiple runs of BuildVolumePath() which
				// is expensive.  So instead we just check the final returned node.
				// The calling code can call this function again later, and will not get
				// this node again.

				if( pAI && pNode )
				{
					AIVolume* pVolumeDest = pNode->GetNodeContainingVolume();
					if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
					{
						return LTNULL;
					}
				}

				return pNode;
			}
		}
	}

	return LTNULL;
}

AINode* AIVolumePlayerInfo::FindOwnedViewNode(CAI* pAI, EnumAINodeType eNodeType, HOBJECT hOwner) const
{
	// Get AIs Path Knowledge.

	CAIPathKnowledgeMgr* pPathKnowledgeMgr = LTNULL;
	if( pAI && pAI->GetPathKnowledgeMgr() )
	{
		pPathKnowledgeMgr = pAI->GetPathKnowledgeMgr();
	}

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		HOBJECT hViewNode = m_ahViewNodes[iViewNode];
		AINode* pNode = (hViewNode == LTNULL ? LTNULL : AINode::HandleToObject(hViewNode));
		if ( pNode && ( ( pNode->GetType() == eNodeType ) || pNode->NodeTypeIsActive( eNodeType ) ) )
		{
			if( pNode->GetNodeOwner() != hOwner )
			{
				continue;
			}

			// Skip nodes in unreachable volumes.

			if( pPathKnowledgeMgr && 
				( pPathKnowledgeMgr->GetPathKnowledge( pNode->GetNodeContainingVolume() ) == CAIPathMgr::kPath_NoPathFound ) )
			{
				continue;
			}

			// Skip nodes that are not in volumes.

			if( !pNode->GetNodeContainingVolume() )
			{
				continue;
			}

			// Skip node if required alignment does not match.
	
			if( ( pNode->GetRequiredRelationTemplateID() != -1 ) &&
				( pNode->GetRequiredRelationTemplateID() != pAI->GetRelationMgr()->GetTemplateID() ) )
			{
				continue;
			}

			if ( !pNode->IsLockedDisabledOrTimedOut() )
			{
				// Ensure that AI can pathfind to the destination node.
				// Ideally, we would like to do this check for each node as we iterate,
				// but that could result in multiple runs of BuildVolumePath() which
				// is expensive.  So instead we just check the final returned node.
				// The calling code can call this function again later, and will not get
				// this node again.

				if( pAI && pNode )
				{
					AIVolume* pVolumeDest = pNode->GetNodeContainingVolume();
					if( !g_pAIPathMgr->HasPath( pAI, pVolumeDest ) )
					{
						return LTNULL;
					}
				}

				return pNode;
			}
		}
	}

	return LTNULL;
}

void AIVolumePlayerInfo::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bHiding);
	LOAD_BOOL(m_bHidingCrouchRequired);

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		LOAD_HOBJECT(m_ahViewNodes[iViewNode]);
	}
}

void AIVolumePlayerInfo::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bHiding);
	SAVE_BOOL(m_bHidingCrouchRequired);

	for ( uint32 iViewNode = 0 ; iViewNode < kMaxViewNodes ; iViewNode++ )
	{
		SAVE_HOBJECT(m_ahViewNodes[iViewNode]);
	}
}

// ----------------------------------------------------------------------- //

const LTFLOAT AIVolumeJumpUp::kJumpUpWidth = 32.f;
const LTFLOAT AIVolumeJumpUp::kJumpUpStepWidth = 24.f;
const LTFLOAT AIVolumeJumpUp::kJumpUpHeightThreshold = 32.f;

BEGIN_CLASS(AIVolumeJumpUp)

	ADD_BOOLPROP_FLAG(OnlyJumpDown,			LTFALSE,		0)
	ADD_BOOLPROP_FLAG(OnlyJumpWhenAlert,	LTFALSE, 		0)
	ADD_BOOLPROP_FLAG(HasRailing,			LTFALSE, 		0)

END_CLASS_DEFAULT_FLAGS(AIVolumeJumpUp, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeJumpUp )
CMDMGR_END_REGISTER_CLASS( AIVolumeJumpUp, AIVolume )


AIVolumeJumpUp::AIVolumeJumpUp()
{
	m_bOnlyJumpDown = LTFALSE;
	m_bOnlyWhenAlert = LTFALSE;
	m_bHasRailing = LTFALSE;
}

AIVolumeJumpUp::~AIVolumeJumpUp()
{
}

bool AIVolumeJumpUp::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

    if ( (g_pLTServer->GetPropGeneric("OnlyJumpDown", &g_gp) == LT_OK) )
	{
		m_bOnlyJumpDown = g_gp.m_Bool;
	}

    if ( (g_pLTServer->GetPropGeneric("OnlyJumpWhenAlert", &g_gp) == LT_OK) )
	{
		m_bOnlyWhenAlert = g_gp.m_Bool;
	}

    if ( (g_pLTServer->GetPropGeneric("HasRailing", &g_gp) == LT_OK) )
	{
		m_bHasRailing = g_gp.m_Bool;
	}

	return true;
}

void AIVolumeJumpUp::InitialUpdate(float fUpdateType)
{
	super::InitialUpdate(fUpdateType);

	if( (GetDims().x != kJumpUpWidth) && (GetDims().z != kJumpUpWidth) )
	{
		AIError("JumpUpVolume %s must have one dimension (x or z) = 64", GetName() );
	}

	// JumpUp volumes must have at least one volume that lines up with its bottom,
	// that is not another JumpUp volume, or it will not be useable for normal
	// pathfinding and an AI may get stuck in it.

	LTBOOL bValid = LTFALSE;
	AIVolumeNeighbor* pNeighbor;
	for( uint32 iNeighbor = 0; iNeighbor < m_cNeighbors; ++iNeighbor )
	{
		pNeighbor = GetNeighborByIndex( iNeighbor );
		if( pNeighbor && pNeighbor->GetVolume() )
		{	
			if( pNeighbor->GetVolume()->GetVolumeType() == kVolumeType_JumpUp )
			{
				continue;
			}
	
			if( pNeighbor->GetVolume()->GetBackBottomLeft().y == GetBackBottomLeft().y )
			{
				bValid = LTTRUE;
				break;
			}
		}
	}

	if( !bValid )
	{
		AIError("INVALID JUMPUP: JumpUpVolume %s must have at least one neighbor whose bottom lines up exactly.", GetName() );
	}
}

LTBOOL AIVolumeJumpUp::CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext )
{
	// Flag set to only allow jumping down.  (Not jumping up).

	if( m_bOnlyJumpDown )
	{
		if( pVolumeNext->GetBackBottomLeft().y > GetBackBottomLeft().y )
		{
			return LTFALSE;
		}
	}

	// Flag set to only use when AI is alert to the enemy's presence.

	if( m_bOnlyWhenAlert && ( pAI->GetAwareness() < kAware_Alert ) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

LTBOOL AIVolumeJumpUp::CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev )
{
	// Flag set to only use when AI is alert to the enemy's presence.

	if( m_bOnlyWhenAlert && ( pAI->GetAwareness() < kAware_Alert ) )
	{
		return LTFALSE;
	}


	// Only allow entry into this volume in the direction of the jump.
	
	AIVolumeNeighbor* pVolumeNeighborPrev = g_pAIVolumeMgr->FindNeighbor( pAI, pVolumePrev, this );
	if( !pVolumeNeighborPrev )
	{
		AIASSERT( pVolumeNeighborPrev, pAI->m_hObject, "AIVolumeJumpUp::CanBuildPathFrom: Could not find prev neighbor." );
		return LTFALSE;
	}

	LTVector vJumpDir = pVolumeNeighborPrev->GetConnectionPerpDir();

	// Jumping volume is not 64 units in the direction of the path.

	vJumpDir *= kJumpUpWidth;
	if( ( vJumpDir.x + GetDims().x != 0.f ) &&
		( vJumpDir.x - GetDims().x != 0.f ) &&	
		( vJumpDir.z + GetDims().z != 0.f ) &&
		( vJumpDir.z - GetDims().z != 0.f ) )
	{
		return LTFALSE;
	}


	return LTTRUE;
}

/*virtual*/ LTBOOL AIVolumeJumpUp::CanBuildPathThrough( CAI* pAI, AIVolume* pVolumePrev, AIVolume* pVolumeNext )
{
	if( IsValidForPath( pAI, pVolumePrev, pVolumeNext ) )
	{
		return ( pAI->GetCurValidVolumeMask() & kVolumeType_JumpUp );
	}

	// Height difference between current and prev are not allowed if
	// this path is not valid for jumping.

	if( pVolumePrev && ( pVolumePrev->GetBackBottomLeft().y != GetBackBottomLeft().y ) )
	{
		return LTFALSE;
	}

	// Height difference between current and next are not allowed if
	// this path is not valid for jumping.

	if( pVolumeNext && ( pVolumeNext->GetBackBottomLeft().y != GetBackBottomLeft().y ) )
	{
		return LTFALSE;
	}

	return ( pAI->GetCurValidVolumeMask() & kVolumeType_BaseVolume );
}


LTBOOL AIVolumeJumpUp::IsValidForPath( CAI* pAI, AIVolume* pVolumePrev )
{
	// Check if volume is valid for jumping in the
	// context of the path being built.

	AIVolume* pVolumeNext = GetNextVolume();

	return IsValidForPath( pAI, pVolumePrev, GetNextVolume() );
}

LTBOOL AIVolumeJumpUp::IsValidForPath(CAI* pAI,AIVolume* pVolumePrev,AIVolume* pVolumeNext)
{
	// Totally invalid if there is no previous.  That means that we can't jump
	// when the path starts out in a jumping volume.
	if( !pVolumePrev )
	{
		return LTFALSE;
	}
	
	// Jumping volume is a dead end if the prev was not high 
	// enough to jump down from.

	if( !pVolumeNext )
	{
		if( pVolumePrev->GetFrontBottomRight().y > GetFrontBottomRight().y )
		{
			return LTTRUE;
		}

		return LTFALSE;
	}

	// Neighbors are not going the same direction.

	AIVolumeNeighbor* pVolumeNeighborPrev = g_pAIVolumeMgr->FindNeighbor( pAI, pVolumePrev, this );
	AIVolumeNeighbor* pVolumeNeighborNext = g_pAIVolumeMgr->FindNeighbor( pAI, this, pVolumeNext );
	if( !( pVolumeNeighborPrev && pVolumeNeighborNext ) )
	{
		AIASSERT( 0, pAI->m_hObject, "AIVolumeJumpUp::IsValidForPath: Could not find prev or next neighbor." );
		return LTFALSE;
	}

	LTVector vJumpDir = pVolumeNeighborPrev->GetConnectionPerpDir();

	if( vJumpDir != pVolumeNeighborNext->GetConnectionPerpDir() )
	{
		return LTFALSE;
	}

	// Jumping volume is not 64 units in the direction of the path.

	vJumpDir *= kJumpUpWidth;
	if( ( vJumpDir.x + GetDims().x != 0.f ) &&
		( vJumpDir.x - GetDims().x != 0.f ) &&
		( vJumpDir.z + GetDims().z != 0.f ) &&
		( vJumpDir.z - GetDims().z != 0.f ) )
	{
		return LTFALSE;
	}

	// Neighbors are at different heights.

	if( pVolumePrev->GetFrontBottomRight().y > pVolumeNext->GetFrontTopRight().y )
	{
		return LTTRUE;
	}

	if( pVolumeNext->GetFrontBottomRight().y > pVolumePrev->GetFrontTopRight().y )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

void AIVolumeJumpUp::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bOnlyJumpDown);
	LOAD_BOOL(m_bOnlyWhenAlert);
	LOAD_BOOL(m_bHasRailing);
}

void AIVolumeJumpUp::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bOnlyJumpDown);
	SAVE_BOOL(m_bOnlyWhenAlert);
	SAVE_BOOL(m_bHasRailing);
}

// ----------------------------------------------------------------------- //

BEGIN_CLASS(AIVolumeJumpOver)
END_CLASS_DEFAULT_FLAGS(AIVolumeJumpOver, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeJumpOver )
CMDMGR_END_REGISTER_CLASS( AIVolumeJumpOver, AIVolume )


AIVolumeJumpOver::AIVolumeJumpOver()
{
	m_bValidJumpOver = LTTRUE;
}

AIVolumeJumpOver::~AIVolumeJumpOver()
{
}

void AIVolumeJumpOver::InitialUpdate(float fUpdateType)
{
	// Count the number of valid neighbors for jumping on each side of the jump.
	// It is only valid to have one valid neighbor on each side.
	// If there are more, the volume may not be used, because it may produce invalid paths.

	uint32 cValidNeighbors[2];
	cValidNeighbors[0] = 0;	// on the negative side of the volume.
	cValidNeighbors[1] = 0; // on the positive side of the volume.

	AIVolumeNeighbor* pNeighbor;
	for( uint32 iNeighbor = 0; iNeighbor < m_cNeighbors; ++iNeighbor )
	{
		pNeighbor = GetNeighborByIndex( iNeighbor );
		if( pNeighbor && pNeighbor->GetVolume() )
		{
			if( ( GetForward().x != 0.f ) && ( pNeighbor->GetConnectionPerpDir().x != 0.f ) )
			{
				if( pNeighbor->GetConnectionPerpDir().x < 0.f )
				{
					cValidNeighbors[0]++;
				}
				else {
					cValidNeighbors[1]++;
				}
				continue;
			}

			if( ( GetForward().z != 0.f ) && ( pNeighbor->GetConnectionPerpDir().z != 0.f ) )
			{
				if( pNeighbor->GetConnectionPerpDir().z < 0.f )
				{
					cValidNeighbors[0]++;
				}
				else {
					cValidNeighbors[1]++;
				}
				continue;
			}
		}
	}

	// Set volume to invalid if there are too many neighbors on either side of the volume.

	if( ( cValidNeighbors[0] > 1 ) || ( cValidNeighbors[1] > 1 ) )
	{
		AIError("INVALID JUMPOVER: JumpUpVolume %s has more than one connecting volume on the same side of the jump. There may only be one connecting volume on each side of the jump.", GetName() );
		m_bValidJumpOver = LTFALSE;
	}
}

LTBOOL AIVolumeJumpOver::CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext )
{
	if( !m_bValidJumpOver )
	{
		AIError("INVALID JUMPOVER: JumpUpVolume %s cannot be used for paths because it has more than one connecting volume on the same side of the jump. There may only be one connecting volume on each side of the jump.", GetName() );
		return LTFALSE;
	}

	// Check if the connection matches the jumping direction, 
	// specified by the forward vector of the volume.

	AIVolumeNeighbor* pVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor( pAI, this, pVolumeNext );
	if( pVolumeNeighbor )
	{
		if( ( GetForward().x != 0.f ) && ( pVolumeNeighbor->GetConnectionPerpDir().x != 0.f ) )
		{
			return LTTRUE;
		}

		if( ( GetForward().z != 0.f ) && ( pVolumeNeighbor->GetConnectionPerpDir().z != 0.f ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

LTBOOL AIVolumeJumpOver::CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev )
{
	if( !m_bValidJumpOver )
	{
		AIError("INVALID JUMPOVER: JumpUpVolume %s cannot be used for paths because it has more than one connecting volume on the same side of the jump. There may only be one connecting volume on each side of the jump.", GetName() );
		return LTFALSE;
	}

	// Can the AI use this type of volume?

	if ( !( pAI->GetCurValidVolumeMask() & GetVolumeType() ) )
	{
		return LTFALSE;
	}

	// Check if the connection matches the jumping direction, 
	// specified by the forward vector of the volume.

	AIVolumeNeighbor* pVolumeNeighbor = g_pAIVolumeMgr->FindNeighbor( pAI, pVolumePrev, this );
	if( pVolumeNeighbor )
	{
		if( ( GetForward().x != 0.f ) && ( pVolumeNeighbor->GetConnectionPerpDir().x != 0.f ) )
		{
			return LTTRUE;
		}

		if( ( GetForward().z != 0.f ) && ( pVolumeNeighbor->GetConnectionPerpDir().z != 0.f ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

void AIVolumeJumpOver::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_BOOL(m_bValidJumpOver);
}

void AIVolumeJumpOver::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_BOOL(m_bValidJumpOver);
}

// ----------------------------------------------------------------------- //

struct AIVolumeJunctionPlugin : public CEditStringPlugin
{
	AIVolumeJunctionPlugin()
	{
		Register("Action", s_aszJunctionActions, s_cJunctionActions);
	}

	LTBOOL DoesPropNameMatch(const char* szPropNameRegistered, const char* szPropNameIncoming)
	{
		return !strcmp(szPropNameRegistered, "Action");
	}
};

BEGIN_CLASS(AIVolumeJunction)
	
	ADD_DEDIT_COLOR( AIVolumeJunction )

	ADD_STRINGPROP_FLAG(VolumeNorth,						"",			PF_OBJECTLINK) \

	PROP_DEFINEGROUP(VolumeNorthActions, PF_GROUP(2)) \

		ADD_STRINGPROP_FLAG(VolumeNorthAction1,				"",			PF_GROUP(2)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeNorthAction1Chance,			1.0f,		PF_GROUP(2)) \
		ADD_STRINGPROP_FLAG(VolumeNorthAction2,				"",			PF_GROUP(2)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeNorthAction2Chance,			0.0f,		PF_GROUP(2)) \
		ADD_STRINGPROP_FLAG(VolumeNorthAction3,				"",			PF_GROUP(2)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeNorthAction3Chance,			0.0f,		PF_GROUP(2)) \
		ADD_STRINGPROP_FLAG(VolumeNorthAction4,				"",			PF_GROUP(2)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeNorthAction4Chance,			0.0f,		PF_GROUP(2)) \
	
	ADD_STRINGPROP_FLAG(VolumeEast,							"",			PF_OBJECTLINK) \
	
	PROP_DEFINEGROUP(VolumeEastActions, PF_GROUP(3)) \

		ADD_STRINGPROP_FLAG(VolumeEastAction1,				"",			PF_GROUP(3)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeEastAction1Chance,			1.0f,		PF_GROUP(3)) \
		ADD_STRINGPROP_FLAG(VolumeEastAction2,				"",			PF_GROUP(3)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeEastAction2Chance,			0.0f,		PF_GROUP(3)) \
		ADD_STRINGPROP_FLAG(VolumeEastAction3,				"",			PF_GROUP(3)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeEastAction3Chance,			0.0f,		PF_GROUP(3)) \
		ADD_STRINGPROP_FLAG(VolumeEastAction4,				"",			PF_GROUP(3)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeEastAction4Chance,			0.0f,		PF_GROUP(3)) \

	ADD_STRINGPROP_FLAG(VolumeSouth,						"",			PF_OBJECTLINK) \

	PROP_DEFINEGROUP(VolumeSouthActions, PF_GROUP(4)) \

		ADD_STRINGPROP_FLAG(VolumeSouthAction1,				"",			PF_GROUP(4)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeSouthAction1Chance,			1.0f,		PF_GROUP(4)) \
		ADD_STRINGPROP_FLAG(VolumeSouthAction2,				"",			PF_GROUP(4)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeSouthAction2Chance,			0.0f,		PF_GROUP(4)) \
		ADD_STRINGPROP_FLAG(VolumeSouthAction3,				"",			PF_GROUP(4)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeSouthAction3Chance,			0.0f,		PF_GROUP(4)) \
		ADD_STRINGPROP_FLAG(VolumeSouthAction4,				"",			PF_GROUP(4)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeSouthAction4Chance,			0.0f,		PF_GROUP(4)) \

	ADD_STRINGPROP_FLAG(VolumeWest,							"",			PF_OBJECTLINK) \

	PROP_DEFINEGROUP(VolumeWestActions, PF_GROUP(5)) \

		ADD_STRINGPROP_FLAG(VolumeWestAction1,				"",			PF_GROUP(5)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeWestAction1Chance,			1.0f,		PF_GROUP(5)) \
		ADD_STRINGPROP_FLAG(VolumeWestAction2,				"",			PF_GROUP(5)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeWestAction2Chance,			0.0f,		PF_GROUP(5)) \
		ADD_STRINGPROP_FLAG(VolumeWestAction3,				"",			PF_GROUP(5)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeWestAction3Chance,			0.0f,		PF_GROUP(5)) \
		ADD_STRINGPROP_FLAG(VolumeWestAction4,				"",			PF_GROUP(5)|PF_STATICLIST) \
		ADD_REALPROP_FLAG(VolumeWestAction4Chance,			0.0f,		PF_GROUP(5)) \

END_CLASS_DEFAULT_FLAGS_PLUGIN(AIVolumeJunction, AIVolume, NULL, NULL, CF_CLASSONLY, AIVolumeJunctionPlugin)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeJunction )
CMDMGR_END_REGISTER_CLASS( AIVolumeJunction, AIVolume )


AIVolumeJunction::AIVolumeJunction()
{
}

AIVolumeJunction::~AIVolumeJunction()
{
}

bool AIVolumeJunction::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

	const char* szDirs[4] = { "North", "South", "East", "West" };

	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		char szProperty[128];
		sprintf(szProperty, "Volume%s", szDirs[iVolume]);
		READPROP_HSTRING(szProperty, m_aVolumes[iVolume].m_hstrName);

		for ( uint32 iVolumeAction = 0 ; iVolumeAction < kMaxActionsPerVolume ; iVolumeAction++ )
		{
			sprintf(szProperty, "Volume%sAction%d", szDirs[iVolume], iVolumeAction+1);
			READPROP_STRINGENUM(szProperty, m_aVolumes[iVolume].m_aeActions[iVolumeAction], s_aszJunctionActions, s_aeJunctionActions, s_cJunctionActions);

			sprintf(szProperty, "Volume%sAction%dChance", szDirs[iVolume], iVolumeAction+1);
			READPROP_FLOAT(szProperty, m_aVolumes[iVolume].m_afActionChances[iVolumeAction]);
		}
	}

	return true;
}

void AIVolumeJunction::InitialUpdate(float fUpdateType)
{
	super::InitialUpdate(fUpdateType);

	if (fUpdateType == INITIALUPDATE_SAVEGAME)
		return;
	
	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		if ( m_aVolumes[iVolume].m_hstrName )
		{
			ILTBaseClass *pObject;
			if ( LT_OK == FindNamedObject(g_pLTServer->GetStringData(m_aVolumes[iVolume].m_hstrName), pObject) )
			{
				m_aVolumes[iVolume].m_pVolume = (AIVolume*)pObject;
			}
			else {
				AIError("JunctionVolume has Invalid Neighbor %s -> %s",
					GetName(), ::ToString(m_aVolumes[iVolume].m_hstrName) );
			}
		}
		else
		{
			m_aVolumes[iVolume].m_pVolume = LTNULL;
		}
	}
}

void AIVolumeJunction::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		m_aVolumes[iVolume].Load(pMsg);
	}
}

void AIVolumeJunction::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; iVolume++ )
	{
		m_aVolumes[iVolume].Save(pMsg);
	}
}

void AIVolumeJunction::RecordActionVolumeMask(AIVolume* pLastVolume, uint8* pmskActionVolumes)
{
	// VolumeAction mask flags which directions from 
	// the junction actually have a volume. 
	// Do not record the last volume the AI came from, 
	// because the AI does not need to explore where he's 
	// already been.

	*pmskActionVolumes = 0;

	for ( uint32 iVolume = 0 ; iVolume < kMaxVolumes ; ++iVolume )
	{
		if ( m_aVolumes[iVolume].m_pVolume && (m_aVolumes[iVolume].m_pVolume != pLastVolume) )
		{
			*pmskActionVolumes |= (1 << iVolume);
		}
	}
}

LTBOOL AIVolumeJunction::GetAction(CAI* pAI, uint8* pmskActionVolumes, AIVolume* pLastVolume, AIVolume* pCorrectVolume, AIVolume** ppVolume, JunctionAction* peJunctionAction)
{
	LTFLOAT	fTotalChance;
	VOLUME* pVolume;
	int iVolumeActionRecord = 0;
	int iCorrectVolume = -1;

	static VolumeActionList	lstVolumeActions;
	lstVolumeActions.reserve(kMaxVolumes);

	LTFLOAT fCurTime = g_pLTServer->GetTime();

	// Look for volumes with valid actions.

	LTBOOL bSplitUp = LTFALSE;
	for ( int iVolume = 0 ; iVolume < kMaxVolumes ; ++iVolume )
	{
		// Ignore volumes that have been explored recently by others.

		if( ( *pmskActionVolumes & (1 << iVolume) ) && 
			( fCurTime < m_aVolumes[iVolume].m_fJunctionResetTime ) )
		{
			*pmskActionVolumes &= ~(1 << iVolume);
			bSplitUp = LTTRUE;
		}

		// Ignore volumes that I have already explored.

		if ( (*pmskActionVolumes & (1 << iVolume)) && m_aVolumes[iVolume].m_pVolume )
		{
			pVolume = &(m_aVolumes[ iVolume ]);
			fTotalChance = 0.f;

			lstVolumeActions[iVolumeActionRecord].pVolume = pVolume->m_pVolume;
			lstVolumeActions[iVolumeActionRecord].iVolume = iVolume;

			// Look for valid actions for this volume.

			for ( int iAction = 0 ; iAction < kMaxActionsPerVolume ; ++iAction )
			{
				LTBOOL bValid = LTFALSE;
				switch ( pVolume->m_aeActions[iAction] )
				{
					case eJunctionActionSearch:
					case eJunctionActionContinue:
					case eJunctionActionNothing:
						bValid = LTTRUE;
						break;

					// Peaking is only valid into volumes that are at 90 degree angles.
					case eJunctionActionPeek:
						{
							AIVolumeNeighbor* pNeighborCurrent = g_pAIVolumeMgr->FindNeighbor( pAI, pLastVolume, this );
							AIVolumeNeighbor* pNeighborNext = g_pAIVolumeMgr->FindNeighbor( pAI, this, pVolume->m_pVolume );

							if ( pNeighborCurrent && pNeighborNext )
							{
								if ( pNeighborCurrent->GetConnectionType() != pNeighborNext->GetConnectionType() )
								{
									bValid = LTTRUE;
								}
							}
						}
						break;
				}
	
				// Record the action. Set percent chance to zero if invalid.
				
				lstVolumeActions[iVolumeActionRecord].aeActions[iAction] = pVolume->m_aeActions[iAction];
				if(bValid)
				{
					lstVolumeActions[iVolumeActionRecord].afChances[iAction] = pVolume->m_afActionChances[iAction];
					fTotalChance += lstVolumeActions[iVolumeActionRecord].afChances[iAction];
				}
				else {
					lstVolumeActions[iVolumeActionRecord].afChances[iAction] = 0.f;
				}
			}

			// Overwrite record if zero chance of taking an action in that volume.

			if( fTotalChance > 0.f )
			{
				// Record finding the correct volume as a choice.

				if( pVolume->m_pVolume == pCorrectVolume )
				{
					iCorrectVolume = iVolumeActionRecord;
				}

				lstVolumeActions[iVolumeActionRecord].fTotalChance = fTotalChance;
				++iVolumeActionRecord;
			}
		}
	}

	// No valid volumes were found.

	if( iVolumeActionRecord == 0 )
	{
		return LTFALSE;
	}

	// Choose a volume.
	int iVolume = 0;

	if( iVolumeActionRecord == 1 )
	{
		if( iVolume == iCorrectVolume )
		{
			AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose the correct volume from junction %s\n", 
					GetName() ) );
		}
		else {
			AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose a random volume from junction %s\n", 
					GetName()) );
		}
	}
	else if( ( iCorrectVolume > -1 ) && pAI->HasTarget() ) 
	{
		LTFLOAT fStealth = 1.f;

		// Evasion skill enhances the player's stealth.
		// Stealth decreases from 1 to 0, 0 being the most stealthy.

		HOBJECT hTarget = pAI->GetTarget()->GetObject();
		if( IsPlayer(hTarget) )
		{
			CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hTarget);
			if( pPlayer )
			{
				fStealth = pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_STEALTH,StealthModifiers::eEvasion);
			}
		}
		
		// Chance of choosing correct volume is ( 1 / # of volumes ) decreased by stealth.

		LTFLOAT fCorrectChance = ( RAISE_BY_DIFFICULTY( 1.f / (LTFLOAT)iVolumeActionRecord ) ) * fStealth;
		if( fCorrectChance >= GetRandom(0.0f, 1.0f) )
		{
			iVolume = iCorrectVolume;

			AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose the correct volume from junction %s\n", 
					GetName() ) );
		}
		else {
			// Randomly select a valid volume, skipping the correct volume.
	
			iVolume = GetRandom(iVolumeActionRecord - 2);
			if( iVolume >= iCorrectVolume )
			{
				++iVolume;
			}

			AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose a random volume from junction %s\n", 
					GetName()) );
		}
	}
	else 
	{
		// Randomly select a valid volume.
	
		iVolume = GetRandom(iVolumeActionRecord - 1);

		AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose a random volume from junction %s\n", 
				GetName() ) );
	}


	// Randomly select a valid action from the volume.

	LTFLOAT fChance = GetRandom(0.f, lstVolumeActions[iVolume].fTotalChance);

	fTotalChance = 0.f;
	int iAction;
	for ( iAction = 0 ; iAction < kMaxActionsPerVolume ; ++iAction )
	{
		if( fChance < fTotalChance)
		{
			break;
		}

		fTotalChance += lstVolumeActions[iVolume].afChances[iAction];
	}

	// Record the selections.

	*pmskActionVolumes &= ~(1 << lstVolumeActions[iVolume].iVolume);
	*ppVolume = lstVolumeActions[iVolume].pVolume;
	*peJunctionAction = lstVolumeActions[iVolume].aeActions[iAction - 1];

	AITRACE(AIShowJunctions, ( pAI->m_hObject, "Chose junction action volume: %s\n", 
			(*ppVolume)->GetName() ) );

	if( bSplitUp && ( *peJunctionAction != eJunctionActionPeek ) )
	{
		pAI->PlaySound( kAIS_SplitUp, LTFALSE );
	}

	// Timestamp chosen volume.

	for ( iVolume = 0 ; iVolume < kMaxVolumes ; ++iVolume )
	{
		if( m_aVolumes[iVolume].m_pVolume == *ppVolume )
		{
			m_aVolumes[iVolume].m_fJunctionResetTime = fCurTime + g_pAIButeMgr->GetResetTimers()->fJunctionResetTimer;
			break;
		}
	}

	return LTTRUE;
}


BEGIN_CLASS(AIVolumeTeleport)

	ADD_DEDIT_COLOR( AIVolume )

	ADD_STRINGPROP_FLAG(DestinationVolume,	"",			0|PF_OBJECTLINK)

END_CLASS_DEFAULT_FLAGS(AIVolumeTeleport, AIVolume, NULL, NULL, CF_CLASSONLY)

CMDMGR_BEGIN_REGISTER_CLASS( AIVolumeTeleport )
CMDMGR_END_REGISTER_CLASS( AIVolumeTeleport, AIVolume )

const LTFLOAT AIVolumeTeleport::kTeleportWidth = 64.f;
const LTFLOAT AIVolumeTeleport::kTeleportDepth = 128.f;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::AIVolumeTeleport()
//              
//	PURPOSE:	Construct to handle initialization of variables.
//              
//----------------------------------------------------------------------------
AIVolumeTeleport::AIVolumeTeleport()
{
	m_hstrDestVolume = NULL;
	m_pTeleportVolumes = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::~AIVolumeTeleport()
//              
//	PURPOSE:	Destructor to handle cleanup and deallocation.
//              
//----------------------------------------------------------------------------
AIVolumeTeleport::~AIVolumeTeleport()
{
	FREE_HSTRING(m_hstrDestVolume);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::DoTeleportObject()
//              
//	PURPOSE:	Teleports the object from one volume to another.
//              
//----------------------------------------------------------------------------
void AIVolumeTeleport::DoTeleportObject(CAI* pAI,
									   AIVolumeTeleport* pDestVolume)
{
	AIASSERT1(pAI, LTNULL, "%s : DoTeleportObject on NULL AI", GetName() );
	AIASSERT1(pDestVolume, LTNULL, "%s : Teleporting to oblivion! No Dest Volume", GetName() );

	if ( !ObjectIsInside(pAI->m_hObject) )
	{
		Warn( "%s is teleporting %s which is not fully inside", GetName(), ::ToString(pAI->m_hObject) );
	}
	if ( !IsVolumesTeleportDest(pDestVolume) )
	{
		Warn( "%s requested teleport from %s to non-neighbor: %s.", ::ToString(pAI->m_hObject), pDestVolume->GetName(), GetName() );
	}

	pDestVolume->ReceiveTeleport(pAI, this);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::ReceiveTeleport()
//              
//	PURPOSE:	Does the teleport of the object, to its center.
//              
//----------------------------------------------------------------------------
void AIVolumeTeleport::ReceiveTeleport(CAI* pAI,
									  const AIVolumeTeleport* const pSender)
{
	AIASSERT1(pAI, LTNULL, "%s : RecieveTeleport passed NULL AI", GetName() );
	AIASSERT1(pSender, LTNULL, "%s : RecieveTeleport passed NULL sending volume", GetName() );

	// Update the objects internal representation of its position, so that
	// the AI knows that it was teleported.  Without this, the AI will 
	char szMsg[1024];
	sprintf( szMsg, "TELEPORT %f,%f,%f ", GetCenter().x, GetCenter().y, GetCenter().z );
	SendTriggerMsgToObject(this, pAI->m_hObject, LTFALSE, szMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::ReadProp()
//              
//	PURPOSE:	Set up the DestinationVolumes
//              
//----------------------------------------------------------------------------
bool AIVolumeTeleport::ReadProp(ObjectCreateStruct *pocs)
{
	if ( !super::ReadProp( pocs ) )
	{
		return false;
	}

	if ( g_pLTServer->GetPropGeneric("DestinationVolume", &g_gp) == LT_OK )
	{
		if ( g_gp.m_String[0] )
		{
			m_hstrDestVolume = g_pLTServer->CreateString( g_gp.m_String );
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::Init()
//              
//	PURPOSE:	Called by the pathMgr before the path is created.
//
//				Handle hooking up the destination volumes, and verifying that
//				this volumes is set up correctly.
//              
//----------------------------------------------------------------------------
int AIVolumeTeleport::Init()
{
	VerifyDims();

	SetupDestinationVolume();

	if ( HasDestination())
	{
		VerifyDestination();
	}

	return AIVolume::Init();
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::IsConnected()
//              
//	PURPOSE:	Used to determine if two volumes are connected for first pass
//				at pathfinding.  If they are connected, then waypoints from
//				one to the other can later be generated.
//
//				IsConnected is called during the creation of neighbors.
//              
//----------------------------------------------------------------------------
bool AIVolumeTeleport::IsConnected( AISpatialRepresentation* pVolume )
{
	// A Teleport volume is connected to a volume if it they share a side,
	// OR if it is the destination of the teleportation.
	return ( Intersects(pVolume) == LTTRUE || IsVolumesTeleportDest(pVolume));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::CanBuildPathTo()
//              
//	PURPOSE:	Returns true if the AI can build the path into this volume,
//				false if it cannot.  This prevents AIs without the ability to
//				teleport from using the teleport volumes to teleport.  
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL AIVolumeTeleport::CanBuildPathTo(CAI* pAI,AIVolume* pVolumeNext)
{
	if (pVolumeNext && pVolumeNext->GetVolumeType() == GetVolumeType()) 
	{
		// If this is a teleport volume, and if the next is a teleport volume,
		// then the ability to teleport is required
		return ( pAI->GetCurValidVolumeMask() & GetVolumeType() );
	}
	else
	{
		// If the next is NOT a teleport volume, then we only need to be able
		// to traverse standard volumes
		return ( pAI->GetCurValidVolumeMask() & kVolumeType_BaseVolume );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::CanBuildPathFrom()
//              
//	PURPOSE:	Returns true if the AI can build the path out of this volume,
//				false if it cannot.  This prevents AIs without the ability to
//				teleport from using the teleport volumes to teleport.  
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL AIVolumeTeleport::CanBuildPathFrom(CAI* pAI,AIVolume* pVolumePrev)
{
	if (pVolumePrev && pVolumePrev->GetVolumeType() == GetVolumeType()) 
	{
		// If this is a teleport volume, and if the next is a teleport volume,
		// then the ability to teleport is required
		return ( pAI->GetCurValidVolumeMask() & GetVolumeType() );
	}
	else
	{
		// If the next is NOT a teleport volume, then we only need to be able
		// to traverse standard volumes
		return ( pAI->GetCurValidVolumeMask() & kVolumeType_BaseVolume );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::SetupDestinationVolume()
//              
//	PURPOSE:	Sets up the destination volume.
//              
//----------------------------------------------------------------------------
void AIVolumeTeleport::SetupDestinationVolume()
{
	// Get the Handle to the Destination Volume.
	ILTBaseClass *pDestVolume;
	if ( LT_OK != FindNamedObject(g_pLTServer->GetStringData(m_hstrDestVolume), pDestVolume) )
	{
		AIError("Volume \"%s\" - Unable to find Destination \"%s\"", GetName(), ::ToString(m_hstrDestVolume));
		return;
	}

	if (pDestVolume && pDestVolume->m_hObject)
	{
		// Hey, you're not really a volume, since they're a GameBaseLite..
		return;
	}

	// Make sure that it really is a Destination Volume
	static HCLASS hTest  = g_pLTServer->GetClass( "AIVolumeTeleport" );
	if ( !g_pLTServer->IsKindOf( ((GameBaseLite*)pDestVolume)->GetClass(), hTest ))
	{
		AIError("TeleportVolume \"%s\" - Has destination: \"%s\" which is not a TeleportVolume", GetName(), ::ToString(m_hstrDestVolume));
		return;
	}

	m_pTeleportVolumes = (AIVolumeTeleport*)pDestVolume;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::VerifyDims()
//              
//	PURPOSE:	Insure that the local dims are valid.
//              
//----------------------------------------------------------------------------
void AIVolumeTeleport::VerifyDims()
{
	if( (GetDims().x != kTeleportWidth) && (GetDims().z != kTeleportWidth) )
	{
		AIError("JumpUpVolume %s must have one dimension (x or z) = %f", GetName(), kTeleportWidth );
		return;
	}

	if( (GetDims().x != kTeleportDepth) && (GetDims().z != kTeleportDepth) )
	{
		AIError("JumpUpVolume %s must have one dimension (x or z) = %f", GetName(), kTeleportDepth );
		return;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AIVolumeTeleport::VerifyDestination()
//              
//	PURPOSE:	Verify that the destination volume meets the requirements set
//				by this volume.
//
//	TODO:		Handle failure.  Right now it is reported but not enforced
//              
//----------------------------------------------------------------------------
void AIVolumeTeleport::VerifyDestination()
{
	AIASSERT1( m_pTeleportVolumes, LTNULL,
		"%s : Unable to verify volume because there is no Destination Volume", GetName() );

	if ( !GetDims().NearlyEquals( m_pTeleportVolumes->GetDims() ) )
	{
		AIError("TeleportVolume \"%s\" - Has differing Dims: \"%s\"", GetName(), ::ToString(m_hstrDestVolume));
		return;
	}
}

bool AIVolumeTeleport::ObjectIsInside( HOBJECT hObject ) const
{
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(hObject, &vDims);
	LTVector vCenter;
    g_pLTServer->GetObjectPos(hObject, &vCenter);
		
	// If so, teleport them
	if ( BoxInside( vCenter, vDims ) )
	{
		return true;
	}
	return false;
}

