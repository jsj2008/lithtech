// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIVOLUMENEIGHBOR_H__
#define __AIVOLUMENEIGHBOR_H__

class AIVolume;
class AIInformationVolume;

#pragma warning( disable : 4786 )
#include <vector>

enum VolumeGate
{
	eVolumeGateInvalid = -1,
};

enum VolumeConnectionType
{
	eVolumeConnectionTypeVertical,
	eVolumeConnectionTypeHorizontal,
};

enum VolumeConnectionLocation
{
	eVolumeConnectionLocationLeft	= 0x01,
	eVolumeConnectionLocationRight	= 0x02,
	eVolumeConnectionLocationFront	= 0x04,
	eVolumeConnectionLocationBack	= 0x08,
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AISpatialNeighbor
//              
//	PURPOSE:	Base class for all VolumeNeighbors.  Supports the
//				unspecialized functionality of the systems.  Any new Volume
//				systems simple need to specialize this class.
//              
//----------------------------------------------------------------------------
class AISpatialNeighbor
{
	public :

		// Ctor/Dtor

		AISpatialNeighbor();
		~AISpatialNeighbor();

		// Save/Load

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Init

		LTBOOL Init(AISpatialRepresentation* pThis, AISpatialRepresentation* pNeighbor);

		// Id

		virtual AISpatialRepresentation* GetSpatialVolume() const { return m_pVolume; }

		// Connection

		const LTVector& GetConnectionPos() const { return m_vConnectionPos; }
		const LTVector& GetConnectionPerpDir() const { return m_vConnectionPerpDir; }
		const LTVector& GetConnectionEndpoint1() const { return m_avConnectionEndpoints[0]; }
		const LTVector& GetConnectionEndpoint2() const { return m_avConnectionEndpoints[1]; }
		const LTVector& GetConnectionMidpoint() const { return m_vConnectionMidpoint; }
		const LTVector& GetConnectionDir() const { return m_vConnectionDir; }
		LTFLOAT GetConnectionLength() const { return m_fConnectionLength; }
		VolumeConnectionType GetConnectionType() const { return m_eVolumeConnectionType; }
		VolumeConnectionLocation GetConnectionLocation() const { return m_eVolumeConnectionLocation; }

		// Gates

		void FindNearestEntryPoint(const LTVector& vPoint, LTVector* pvEntryPoint, LTFLOAT fWidth);
		VolumeGate AllocateGate(const LTVector& vPoint, LTBOOL* pbGotBest);
		void ReleaseGate(VolumeGate eVolumeGate);
		const LTVector& GetGatePosition(VolumeGate eVolumeGate);

	protected :

		typedef std::vector<LTFLOAT>			CVecFloat;
		typedef std::vector<LTFLOAT>::iterator	CVecFloatIter;

	protected :

		AISpatialRepresentation* m_pVolume;
		LTVector				m_vConnectionPos;
		LTVector				m_avConnectionEndpoints[2];
		LTVector				m_vConnectionMidpoint;
		LTVector				m_vConnectionPerpDir;
		LTVector				m_vConnectionDir;
		LTFLOAT					m_fConnectionLength;
								
		uint32					m_cGates;
		CVecFloat				m_vecfGateOccupancy;

		VolumeConnectionType		m_eVolumeConnectionType;
		VolumeConnectionLocation	m_eVolumeConnectionLocation;
};


//----------------------------------------------------------------------------
//              
//	CLASS:		AIVolumeNeighbor
//              
//	PURPOSE:	Neighbor class for the AI pathing volumes.
//              
//----------------------------------------------------------------------------
class AIVolumeNeighbor : public AISpatialNeighbor
{
public:
	virtual AIVolume* GetVolume() const { return (AIVolume*)AISpatialNeighbor::GetSpatialVolume(); }
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AIInformationVolumeNeighbor
//              
//	PURPOSE:	Neighbor helper class for the Information volume system.  
//              
//----------------------------------------------------------------------------
class AIInformationVolumeNeighbor : public AISpatialNeighbor
{
public:
	virtual AIInformationVolume* GetVolume() const { return (AIInformationVolume*)AISpatialNeighbor::GetSpatialVolume(); }
};

#endif