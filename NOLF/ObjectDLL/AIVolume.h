// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIVOLUME_H__
#define __AIVOLUME_H__

#include "ltengineobjects.h"

class CAIVolume;
class CAINode;
class CAIRegion;

enum
{
	kMaxViewNodes = 16
};

class AIVolume : public BaseClass
{
	public :

		AIVolume();
		~AIVolume();

		// Stairs/ledge stuff

		LTBOOL HasStairs() const { return m_bStairs; }
		LTBOOL HasLedge() const { return m_bLedge; }
		LTBOOL IsVertical() const { return m_bVertical; }

		const LTVector& GetStairsDir() const { return m_vStairsDir; }
		const LTVector& GetLedgeDir() const { return m_vLedgeDir; }

		HSTRING GetViewNode(uint32 iViewNode) const { return m_ahstrViewNodes[iViewNode]; }

	protected :

		uint32 EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
		LTBOOL ReadProp(ObjectCreateStruct *pData);

	private :

		LTBOOL	m_bStairs;
		LTBOOL	m_bLedge;
		LTBOOL	m_bVertical;
		HSTRING	m_ahstrViewNodes[kMaxViewNodes];

		LTVector	m_vStairsDir;
		LTVector	m_vLedgeDir;
};

class CAIVolumeNeighbor
{
	public :

		CAIVolumeNeighbor();
		~CAIVolumeNeighbor();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Init(CAIVolume* pThis, CAIVolume* pNeighbor);

		int32 GetIndex() const { return m_iVolume; }
		const LTVector& GetConnectionPos() const { return m_vConnectionPos; }
		const LTVector& GetConnectionPerpDir() const { return m_vConnectionPerpDir; }
		const LTVector& GetConnectionEndpoint1() const { return m_avConnectionEndpoints[0]; }
		const LTVector& GetConnectionEndpoint2() const { return m_avConnectionEndpoints[1]; }
		const LTVector& GetConnectionDir() const { return m_vConnectionDir; }
		LTFLOAT GetConnectionLength() const { return m_fConnectionLength; }

	private :

		int32			m_iVolume;
		LTVector		m_vConnectionPos;
		LTVector		m_avConnectionEndpoints[2];
		LTVector		m_vConnectionPerpDir;
		LTVector		m_vConnectionDir;
		LTFLOAT			m_fConnectionLength;
};

class CAIVolume
{
	public :

		// Ctors/Dtors/etc

		CAIVolume();
		~CAIVolume();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Init(int32 iVolume, const AIVolume& vol);
		void InitNeighbors(CAIVolume** apVolumeNeighbors, uint32 cNeighbors);

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Geometry methods

		LTBOOL Intersects(const CAIVolume& vol) const;
		LTBOOL Inside(const LTVector& vPos, LTFLOAT fVerticalThreshhold = 0.0f) const;
        LTBOOL Inside2d(const LTVector& vPos, LTFLOAT fThreshhold) const;
        LTBOOL Inside2dLoose(const LTVector& vPos, LTFLOAT fThreshhold) const;

		// Neighbor methods

		int32 GetNumNeighbors() { return m_cNeighbors; }
		CAIVolumeNeighbor* GetNeighborByIndex(int32 iNeighbor) { return &m_aNeighbors[iNeighbor]; }

		// Door methods

		LTBOOL HasDoors() const { return m_cDoors > 0; }
		LTBOOL HadDoors() const { return m_bHadDoors; }
		int32 GetNumDoors() const { return cm_nMaxDoors; }
		HOBJECT GetDoor(int32 iDoor) { return m_ahDoors[iDoor]; }

		// Lift methods

		LTBOOL HasLift() const { return !!m_hLift; }
		HOBJECT GetLift() { return m_hLift; }

		// Stairs methods

		LTBOOL HasStairs() const { return m_bStairs; }

		// Ledge methods

		LTBOOL HasLedge() const { return m_bLedge; }

		// Vertical methods

		LTBOOL IsVertical() const { return m_bVertical; }

		// View node methods

		CAINode* FindViewNode() const;
		
		// Enumerations

		void EnumerateSearchNodes(uint32* aiSearchNodes, uint32* pcSearchNodes, const uint32 nMaxSearchNodes) const;

		// Misc

        const char* GetName() const { return g_pLTServer->GetStringData(m_hstrName); }

		const LTVector& GetStairsDir() const { return m_vStairsDir; }
		const LTVector& GetLedgeDir() const { return m_vLedgeDir; }

		LTBOOL HasRegion() const;
		CAIRegion* GetRegion() const;
 
		// Simple accessors

		int32 GetIndex() const { return m_iVolume; }
		const LTVector& GetFrontTopLeft() const { return m_vFrontTopLeft;		}
		const LTVector& GetFrontTopRight() const { return m_vFrontTopRight;		}
		const LTVector& GetBackTopLeft() const { return m_vBackTopLeft;		}
		const LTVector& GetBackTopRight() const { return m_vBackTopRight;		}
		const LTVector& GetFrontBottomLeft() const { return m_vFrontBottomLeft;	}
		const LTVector& GetFrontBottomRight() const { return m_vFrontBottomRight;	}
		const LTVector& GetBackBottomLeft() const { return m_vBackBottomLeft;		}
		const LTVector& GetBackBottomRight() const { return m_vBackBottomRight;	}

		// Pathfinding data accessors

		LTFLOAT GetShortestEstimate() const { return m_fShortestEstimate; }
		void SetShortestEstimate(LTFLOAT fShortestEstimate) { m_fShortestEstimate = fShortestEstimate; }

		CAIVolume* GetNextVolume() const { return m_pPreviousVolume; }
		void SetNextVolume(CAIVolume* pNextVolume) { m_pPreviousVolume = pNextVolume; }

		CAIVolume* GetPreviousVolume() const { return m_pPreviousVolume; }
		void SetPreviousVolume(CAIVolume* pPreviousVolume) { m_pPreviousVolume = pPreviousVolume; }

		const LTVector& GetEntryPosition() const { return m_vEntryPosition; }
		void SetEntryPosition(const LTVector& vEntryPosition) { m_vEntryPosition = vEntryPosition; }

		const LTVector& GetWalkthroughPosition() const { return m_vWalkthroughPosition; }
		void SetWalkthroughPosition(const LTVector& vWalkthroughPosition) { m_vWalkthroughPosition = vWalkthroughPosition; }

	protected :
		
		friend class CAIRegion;
		friend class CAIRegionMgr;
		void SetRegion(uint32 iRegion);

	public :

		enum Constants
		{
			cm_nMaxDoors = 2,
		};

	private :

		HSTRING			m_hstrName;

		int32			m_iVolume;

		uint32			m_adwViewNodes[kMaxViewNodes];

		uint32			m_iRegion;

		LTVector		m_vFrontTopLeft;
		LTVector		m_vFrontTopRight;
		LTVector		m_vBackTopLeft;
		LTVector		m_vBackTopRight;
		LTVector		m_vFrontBottomLeft;
		LTVector		m_vFrontBottomRight;
		LTVector		m_vBackBottomLeft;
		LTVector		m_vBackBottomRight;

		LTBOOL			m_bHadDoors;
		uint32			m_cDoors;
		HOBJECT			m_ahDoors[cm_nMaxDoors];

		HOBJECT			m_hLift;

		LTBOOL			m_bStairs;
		LTBOOL			m_bLedge;
		LTBOOL			m_bVertical;

		LTVector		m_vStairsDir;
		LTVector		m_vLedgeDir;

		uint32					m_cNeighbors;
		CAIVolumeNeighbor*		m_aNeighbors;

		// Pathfinding data members (do not need to be saved)

		LTFLOAT			m_fShortestEstimate;
		CAIVolume*		m_pPreviousVolume;
		LTVector		m_vEntryPosition;
		LTVector		m_vWalkthroughPosition;
};

#endif // __AIVOLUME_H__