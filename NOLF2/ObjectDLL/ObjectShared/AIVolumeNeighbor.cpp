// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVolume.h"
#include "AIVolumeNeighbor.h"
#include "AIUtils.h"

// ----------------------------------------------------------------------- //

AISpatialNeighbor::AISpatialNeighbor()
{
}

AISpatialNeighbor::~AISpatialNeighbor()
{
}

void AISpatialNeighbor::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pVolume, AISpatialRepresentation);
	LOAD_VECTOR(m_vConnectionPos);
	LOAD_VECTOR(m_avConnectionEndpoints[0]);
	LOAD_VECTOR(m_avConnectionEndpoints[1]);
	LOAD_VECTOR(m_vConnectionMidpoint);
	LOAD_VECTOR(m_vConnectionPerpDir);
	LOAD_VECTOR(m_vConnectionDir);
	LOAD_FLOAT(m_fConnectionLength);
	LOAD_DWORD(m_cGates);

	m_vecfGateOccupancy.resize(m_cGates);
	for ( uint32 iGate = 0 ; iGate < m_cGates ; iGate++ )
	{
		LOAD_FLOAT(m_vecfGateOccupancy[iGate]);
	}

	LOAD_DWORD_CAST(m_eVolumeConnectionType, VolumeConnectionType);
	LOAD_DWORD_CAST(m_eVolumeConnectionLocation, VolumeConnectionLocation);
}

void AISpatialNeighbor::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pVolume);
	SAVE_VECTOR(m_vConnectionPos);
	SAVE_VECTOR(m_avConnectionEndpoints[0]);
	SAVE_VECTOR(m_avConnectionEndpoints[1]);
	SAVE_VECTOR(m_vConnectionMidpoint);
	SAVE_VECTOR(m_vConnectionPerpDir);
	SAVE_VECTOR(m_vConnectionDir);
	SAVE_FLOAT(m_fConnectionLength);
	SAVE_DWORD(m_cGates);

	for ( uint32 iGate = 0 ; iGate < m_cGates ; iGate++ )
	{
		SAVE_FLOAT(m_vecfGateOccupancy[iGate]);
	}

	SAVE_DWORD(m_eVolumeConnectionType);
	SAVE_DWORD(m_eVolumeConnectionLocation);
}

LTBOOL AISpatialNeighbor::Init(AISpatialRepresentation* pThis, AISpatialRepresentation* pNeighbor)
{
	m_pVolume = pNeighbor;

	// Compute the 2d intersection of the two volumes, and compute important
	// things about the geometry of the connection

    LTVector vFrontLeft(0,0,0);
    LTVector vFrontRight(0,0,0);
    LTVector vBackLeft(0,0,0);
    LTVector vBackRight(0,0,0);

	vFrontLeft.x = Max<LTFLOAT>(pThis->GetFrontTopLeft().x, pNeighbor->GetFrontTopLeft().x);
	vFrontLeft.z = Min<LTFLOAT>(pThis->GetFrontTopLeft().z, pNeighbor->GetFrontTopLeft().z);

	vFrontRight.x = Min<LTFLOAT>(pThis->GetFrontTopRight().x, pNeighbor->GetFrontTopRight().x);
	vFrontRight.z = Min<LTFLOAT>(pThis->GetFrontTopRight().z, pNeighbor->GetFrontTopRight().z);

	vBackLeft.x = Max<LTFLOAT>(pThis->GetBackTopLeft().x, pNeighbor->GetBackTopLeft().x);
	vBackLeft.z = Max<LTFLOAT>(pThis->GetBackTopLeft().z, pNeighbor->GetBackTopLeft().z);

	vBackRight.x = Min<LTFLOAT>(pThis->GetBackTopRight().x, pNeighbor->GetBackTopRight().x);
	vBackRight.z = Max<LTFLOAT>(pThis->GetBackTopRight().z, pNeighbor->GetBackTopRight().z);

	// We know connection position (the center of the intersection) easily.

	m_vConnectionPos = (vFrontLeft+vFrontRight+vBackLeft+vBackRight)/4.0f;

	// We need y for vertical movement

#define _A_b pThis->GetFrontBottomRight().y
#define _A_t pThis->GetFrontTopRight().y
#define _B_b pNeighbor->GetFrontBottomRight().y
#define _B_t pNeighbor->GetFrontTopRight().y

	if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b >= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = _A_b; // or _B_t
	}
	else if ( (_A_t <= _B_t) && (_A_t <= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = _A_t; // or _B_b
	}
	else if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = (_A_b + _B_t)/2.0f;
	}
	else if ( (_A_t <= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = (_A_t + _B_b)/2.0f;
	}
	else if ( (_A_t >= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b <= _B_b) )
	{
		m_vConnectionPos.y = (_B_b + _B_t)/2.0f;
	}
	else if ( (_A_t <= _B_t) && (_A_t >= _B_b) && (_A_b <= _B_t) && (_A_b >= _B_b) )
	{
		m_vConnectionPos.y = (_A_b + _A_t)/2.0f;
	}
	else
	{
		m_vConnectionPos.y = -float(INT_MAX);
        DANGER(g_pLTServer, blong);
	}

	// Find the endpoints of the line across the connection, and the vector perpendicular to this

	if ( pThis->InsideMasked(pNeighbor->GetFrontTopLeft(), eAxisAll) || pThis->InsideMasked(pNeighbor->GetBackTopRight(), eAxisAll) ||
		 pThis->InsideMasked(pNeighbor->GetFrontBottomLeft(), eAxisAll) || pThis->InsideMasked(pNeighbor->GetBackBottomRight(), eAxisAll) )
	{
        m_avConnectionEndpoints[0] = vFrontRight + LTVector(0, m_vConnectionPos.y, 0);
        m_avConnectionEndpoints[1] = vBackLeft + LTVector(0, m_vConnectionPos.y, 0);
		m_vConnectionPerpDir = vFrontRight - vBackLeft;
		m_vConnectionDir = m_avConnectionEndpoints[1] - m_avConnectionEndpoints[0];
		m_vConnectionDir.y = 0.0f;
		m_fConnectionLength = VEC_MAG(m_vConnectionDir);
		m_vConnectionDir.Normalize();
	}
	else
	{
        m_avConnectionEndpoints[0] = vFrontLeft + LTVector(0, m_vConnectionPos.y, 0);
        m_avConnectionEndpoints[1] = vBackRight + LTVector(0, m_vConnectionPos.y, 0);
		m_vConnectionPerpDir = vFrontLeft - vBackRight;
		m_vConnectionDir = m_avConnectionEndpoints[1] - m_avConnectionEndpoints[0];
		m_vConnectionDir.y = 0.0f;
		m_fConnectionLength = VEC_MAG(m_vConnectionDir);
		m_vConnectionDir.Normalize();
	}

	m_vConnectionMidpoint = m_avConnectionEndpoints[0] + ( m_vConnectionDir * ( m_fConnectionLength * 0.5f ) );

	LTFLOAT fTemp = m_vConnectionPerpDir[0];
	m_vConnectionPerpDir[0] = m_vConnectionPerpDir[2];
	m_vConnectionPerpDir[2] = fTemp;
	m_vConnectionPerpDir.Normalize();

	// Ensure that perp dir is axis-aligned.

	RoundVector( m_vConnectionPerpDir );

	// Make sure it points into this volume

    LTVector vThisCenter = (pThis->GetFrontTopLeft()+pThis->GetBackTopRight())/2.0f;
    LTVector vThisCenterDir = vThisCenter - m_vConnectionPos;
	vThisCenterDir.y = 0;
	vThisCenterDir.Normalize();

	if ( vThisCenterDir.Dot(m_vConnectionPerpDir) < 0.0f )
	{
		m_vConnectionPerpDir = -m_vConnectionPerpDir;
	}

	m_cGates = (uint32)(m_fConnectionLength/48.0f);


	// Check for invalid neighbors.

	if(m_cGates == 0)
	{
		AIError("Volume has Invalid Neighbor %s -> %s. Connection < 48 units!",
			pThis->GetName(), pNeighbor->GetName() );

		return LTFALSE;
	}

	m_vecfGateOccupancy.resize(m_cGates);

	for ( uint32 iGate = 0 ; iGate < m_cGates ; iGate++ )
	{
		m_vecfGateOccupancy[iGate] = 0.0f;
	}

	if(	m_avConnectionEndpoints[0].z == m_avConnectionEndpoints[1].z )
	{
		m_eVolumeConnectionType = eVolumeConnectionTypeHorizontal;

		if( m_pVolume->GetCenter().z < m_vConnectionPos.z )
		{
			m_eVolumeConnectionLocation = eVolumeConnectionLocationFront;
		}
		else {
			m_eVolumeConnectionLocation = eVolumeConnectionLocationBack;
		}
	}
	else
	{
		m_eVolumeConnectionType = eVolumeConnectionTypeVertical;

		if( m_pVolume->GetCenter().x < m_vConnectionPos.x )
		{
			m_eVolumeConnectionLocation = eVolumeConnectionLocationRight;
		}
		else {
			m_eVolumeConnectionLocation = eVolumeConnectionLocationLeft;
		}
	}

	//g_pLTServer->CPrint("cxn @ %f,%f,%f in %f,%f,%f : %f,%f,%f",
	//	EXPANDVEC(m_vConnectionPos), EXPANDVEC(vThisCenter), EXPANDVEC(m_vConnectionPerpDir));

	return LTTRUE;
}

void AISpatialNeighbor::FindNearestEntryPoint(const LTVector& vPoint, LTVector* pvEntryPoint, LTFLOAT fWidth)
{
	// Divide the connection into openings of some width.

	long cOpenings = (long)(m_fConnectionLength / fWidth);

	// Only one opening possible.

	if( cOpenings <= 1 )
	{
		*pvEntryPoint = m_vConnectionMidpoint;
		return;
	}

	// Find the opening nearest to the specified point,
	// and return that openings midpoint.

	long iOpening;
	LTFLOAT	fMin;

	switch( m_eVolumeConnectionType )
	{
		// Vertical connection.

		case eVolumeConnectionTypeVertical:
			fMin = Min( m_avConnectionEndpoints[0].z, m_avConnectionEndpoints[1].z );
			iOpening = (long)( ( vPoint.z - fMin ) / fWidth );

			// Point is above or below the connection.

			if( iOpening < 0 )
			{
				iOpening = 1;
			}
			else if( iOpening >= cOpenings )
			{
				iOpening = cOpenings - 1;
			}

			// Calculate the opening's midpoint.

			*pvEntryPoint = m_avConnectionEndpoints[0];
			pvEntryPoint->z = fMin + ( ((LTFLOAT)iOpening) * fWidth ) + ( fWidth * 0.5f );
			break;

		// Horizontal connection.

		case eVolumeConnectionTypeHorizontal:
			fMin = Min( m_avConnectionEndpoints[0].x, m_avConnectionEndpoints[1].x );
			iOpening = (long)( ( vPoint.x - fMin ) / fWidth );

			// Point is above or below the connection.

			if( iOpening < 0 )
			{
				iOpening = 0;
			}
			else if( iOpening >= cOpenings )
			{
				iOpening = cOpenings - 1;
			}

			// Calculate the opening's midpoint.

			*pvEntryPoint = m_avConnectionEndpoints[0];
			pvEntryPoint->x = fMin + ( ((LTFLOAT)iOpening) * fWidth ) + ( fWidth * 0.5f );
			break;
	}
}

VolumeGate AISpatialNeighbor::AllocateGate(const LTVector& vPoint, LTBOOL* pbGotBest)
{
	// Find the gate nearest to vPoint with the smallest occupancy

	LTFLOAT fPointDist = (vPoint - m_avConnectionEndpoints[0]).Mag();
	int32 iGateNearest = (int32)((24.0f+fPointDist)/48.0f);
	iGateNearest = Min<int32>(m_cGates-1, iGateNearest);

	LTFLOAT fGateBest = (LTFLOAT)INT_MAX;
	int32 iGateBest = -1;

	for ( int32 iGate = 0 ; iGate < (int32)m_cGates ; iGate++ )
	{
		LTFLOAT fGate = m_vecfGateOccupancy[iGate] + (LTFLOAT)fabs((float)(iGate - iGateNearest));
		if ( fGate < fGateBest )
		{
			fGateBest = fGate;
			iGateBest = iGate;

			if ( fGate == 0.0f ) break;
		}
	}

	_ASSERT(iGateBest != -1);

	m_vecfGateOccupancy[iGateBest] += 1.0f;

	if( pbGotBest )
	{
		*pbGotBest = ( iGateBest == iGateNearest );
	}

	return (VolumeGate)iGateBest;
}

void AISpatialNeighbor::ReleaseGate(VolumeGate eVolumeGate)
{
	m_vecfGateOccupancy[eVolumeGate] -= 1.0f;
}

const LTVector& AISpatialNeighbor::GetGatePosition(VolumeGate eVolumeGate)
{
	static LTVector vGatePosition(0.0f, 0.0f, 0.0f);
	vGatePosition = m_avConnectionEndpoints[0] + m_vConnectionDir*(24.0f + 48.0f*LTFLOAT(eVolumeGate));
	return vGatePosition;
}
