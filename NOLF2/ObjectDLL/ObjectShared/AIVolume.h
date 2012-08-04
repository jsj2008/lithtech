// ----------------------------------------------------------------------- //
//
// MODULE  : AIVolume.h
//
// PURPOSE : Definition of AIGeometry, AISpatialRepresentation, and
//			 AIVolume classes
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AIVOLUME_H__
#define __AIVOLUME_H__

#include "ltengineobjects.h"
#include "AITypes.h"
#include "AINode.h"
#include "ServerUtilities.h"
#include "UberAssert.h"
#include "DebugLineSystem.h"
#include "GameBaseLite.h"

#pragma warning( disable : 4786 )
#include <vector>

class AIRegion;
class AIVolume;
class AINode;
class AINodeSearch;
class CAI;
enum  EnumAIStimulusID;


class AISpatialNeighbor;
class AIVolumeNeighbor;
class AIInformationVolumeNeighbor;

LINKTO_MODULE( AIVolume );


// Axis list for doing tests.
// This will allow us to specify checks only along one axis, or a subset.
// Includes a few masks for the standard configurations.
enum eAxis
{
	eAxisX = 0x01,
	eAxisY = 0x02,
	eAxisZ = 0x04,
};
const int eAxisAll = ( eAxisX | eAxisY | eAxisZ );
const int eAxisHorizontal = ( eAxisX | eAxisZ );
const int eAxisVertical = ( eAxisY );

//----------------------------------------------------------------------------
//              
//	CLASS:		AIGeometry
//              
//	PURPOSE:	Helper class which manages a set of Dims, provided accessors,
//				and supplies intersection tests.
//              
//----------------------------------------------------------------------------
class AIGeometry : public GameBaseLite
{
public:
	typedef GameBaseLite super;

	AIGeometry();

	virtual int Init() { return 0; }
	
	// GameBaseLite overrides

	virtual void Save(ILTMessage_Write *pMsg);
	virtual void Load(ILTMessage_Read *pMsg);

	virtual bool ReadProp(ObjectCreateStruct *pocs);

	// Geometry

	LTBOOL Intersects(const AIGeometry* pGeometry) const;
	LTBOOL InsideMasked(const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold = 0.0f) const;
	LTBOOL Inside(const LTVector& vPos, LTFLOAT fRadius2D, LTFLOAT fVerticalThreshhold) const;
    LTBOOL Inside2d(const LTVector& vPos, LTFLOAT fThreshhold) const;
	LTBOOL Inside3D( const LTVector& vPos ) const;
	LTBOOL Inside2dLoose(const LTVector& vPos,LTFLOAT fThreshhold,
		const AISpatialNeighbor* const pVolumes, uint32 nNeighborCount) const;

	// Simple Accessors
	const LTVector& GetForward() const { return m_vForward; }
	const LTVector& GetCenter() const { return m_vCenter; }
	const LTVector& GetDims() const { return m_vDims; }
	const LTVector& GetFrontTopLeft() const { return m_vFrontTopLeft; }
	const LTVector& GetFrontTopRight() const { return m_vFrontTopRight; }
	const LTVector& GetBackTopLeft() const { return m_vBackTopLeft; }
	const LTVector& GetBackTopRight() const { return m_vBackTopRight; }
	const LTVector& GetFrontBottomLeft() const { return m_vFrontBottomLeft; }
	const LTVector& GetFrontBottomRight() const { return m_vFrontBottomRight; }
	const LTVector& GetBackBottomLeft() const { return m_vBackBottomLeft; }
	const LTVector& GetBackBottomRight() const { return m_vBackBottomRight; }

	bool BoxInside( const LTVector& vCenter, const LTVector& vDims ) const;
	
public:

	static const LTFLOAT kMinXZDim;

private:
	LTVector	m_vDims;
	LTVector	m_vCenter;
	LTVector	m_vFrontTopLeft;
	LTVector	m_vFrontTopRight;
	LTVector	m_vBackTopLeft;
	LTVector	m_vBackTopRight;
	LTVector	m_vFrontBottomLeft;
	LTVector	m_vFrontBottomRight;
	LTVector	m_vBackBottomLeft;
	LTVector	m_vBackBottomRight;

	LTVector	m_vForward;
};


//----------------------------------------------------------------------------
//              
//	CLASS:		AISpatialRepresentation
//              
//	PURPOSE:	Base class for Volumes to be used with the VolumeManagement
//				system.  The AISpatialRepresentation is geometry, is an object,
//				and has some of the fundementals required by the VolumeMgr
//				system, specificly Neighbors.
//              
//	NOTE:		This class should not really inherit from AIGeometry!  It
//				ought to have it as an agregate, and to have BaseClass as..
//				the true base class.  In the interests of not changing
//				a huge number of files, inheritance is a workaround.
//
//----------------------------------------------------------------------------
class AISpatialRepresentation : public AIGeometry
{
public:
	typedef AIGeometry super;

	enum EnumUseFlags
	{
		kUseBy_None		= 0x00,
		kUseBy_AI		= 0x01,
		kUseBy_Player	= 0x02,
		kUseBy_All		= 0xff,
	};

public:
	// Creation/Destruction
	AISpatialRepresentation();
	virtual ~AISpatialRepresentation();
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	void InitNeighbors(AISpatialRepresentation** apVolumeNeighbors,uint32 cNeighbors);

	// Template Methods:

	virtual void AddToRegion(AIRegion* pRegion) { UBER_ASSERT(0, "Fake Pure Virtual Function: Should never get here" ); }
	virtual bool IsConnected( AISpatialRepresentation* pVolume ) { UBER_ASSERT(0, "Fake Pure Virtual Function: Should never get here" ); return false; }

	// GameBaseLite

	virtual bool ReadProp(ObjectCreateStruct *pocs);
	virtual void InitialUpdate(float fUpdateType);

	// Accessors

	LTBOOL Inside2dLoose(const LTVector& vPos,LTFLOAT fThreshhold );

	// Regions

	LTBOOL HasRegion() const;
	AIRegion* GetRegion() const;

	// Neighbors

	uint32 GetNumNeighbors() { return m_cNeighbors; }
	AISpatialNeighbor* GetSpatialNeighborByIndex(uint32 iNeighbor);

	// Use Flags

	uint32 GetUseFlags() const { return m_dwUseBy; }

	// Enabled

	LTBOOL IsVolumeEnabled() const { return m_bVolumeEnabled; }

	// Debug

	void DebugPrintName() { OutputDebugString(::ToString(GetName())); }

	int DrawSelf();
	int HideSelf();
	virtual	DebugLine::Color GetDebugColor() { return Color::Yellow; }

protected:
	uint32				m_cNeighbors;
	AISpatialNeighbor*	m_aNeighbors;
	
	HSTRING				m_hstrRegion;
	AIRegion*			m_pRegion;

	uint32				m_dwUseBy;

	LTBOOL				m_bVolumeEnabled;
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AIInformationVolume
//              
//	PURPOSE:	Volume type to be used for spatial representation and NOT
//				pathing.  AI
//              
//----------------------------------------------------------------------------
class AIInformationVolume : public AISpatialRepresentation
{
	typedef AISpatialRepresentation super;

public:

	enum InformationVolumeType
	{
		eTypeInvalid,
		eTypeBaseInformationVolume,
		eTypePlayerInfo,
	};

public:

	// Ctor/Dtor

	AIInformationVolume();
	virtual ~AIInformationVolume();

	// Engine

	virtual bool ReadProp(ObjectCreateStruct *pocs);
	virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);


	// Template Methods:
	virtual AIInformationVolume* GetNeighborByIndex(uint32 iNeighbor);

	virtual void AddToRegion(AIRegion* pRegion);
	virtual bool IsConnected( AISpatialRepresentation* pVolume );
	
	// Senses

	uint32 GetSenseMask() const { return m_dwSenseMask; }

	// Type 

	virtual InformationVolumeType GetVolumeType() { return eTypeBaseInformationVolume; }

	// Debug

	virtual	DebugLine::Color GetDebugColor() { return Color::Blue; }

	// Simple Accessors

	LTBOOL IsOn() const { return m_eInfoVolumeState == kIVS_On; }

	protected:

		enum EnumInfoVolumeState
		{
			kIVS_On,
			kIVS_Off,
		};

	private :

		EnumInfoVolumeState	m_eInfoVolumeState;
		uint32				m_dwSenseMask;
};

class AIInformationVolumePlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//----------------------------------------------------------------------------

class AIVolume : public AISpatialRepresentation
{
	typedef AISpatialRepresentation super;

	public :

		//
		// ENUM: Types of AI volumes.
		//
		enum EnumVolumeType
		{
			kVolumeType_Invalid			= 0,
			kVolumeType_None			= 0,
			#define VOLUME_TYPE_AS_ENUM 1
			#include "AIVolumeTypeEnums.h"
			#undef VOLUME_TYPE_AS_ENUM
			kVolumeType_All				= 0xFFFFFFFF,
		};

		//
		// STRINGS: const strings for AI volume types.
		//
		static const char* ms_aszVolumeTypes[];

		static const LTFLOAT kApproxVolumeYDim;
		static const LTFLOAT kUnpreferablePathWeight;

	public :

		// Ctor/Dtor

		AIVolume();
		virtual ~AIVolume();

		// Init

		virtual int Init();


		// Engine

		virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual bool ReadProp(ObjectCreateStruct *pocs);
		virtual void InitialUpdate(float fUpdateType);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Handlers

		virtual void OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// Template Methods:
		virtual void AddToRegion(AIRegion* pRegion);
		virtual bool IsConnected( AISpatialRepresentation* pVolume );

		// Doors

		LTBOOL HasDoors() const { return m_cDoors > 0; }
		LTBOOL HadDoors() const { return m_bHadDoors; }
		int32 GetNumDoors() const { return kMaxDoors; }
		HOBJECT GetDoor(int32 iDoor) const { return m_ahDoors[iDoor]; }
		LTBOOL AreDoorsLocked( HOBJECT hChar ) const;

		// Pathing (finding and building).

		virtual LTBOOL CanBePathDest() { return LTTRUE; }
		virtual LTBOOL CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext );
		virtual LTBOOL CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev );
		virtual LTBOOL CanBuildPathThrough( CAI* pAI, AIVolume* pVolumePrev, AIVolume* pVolumeNext ) { return LTTRUE; }

		virtual LTBOOL IsValidForPath( CAI* pAI, AIVolume* pVolumePrev ) { return LTTRUE; }

		void ReserveVolume(HOBJECT hAI);
		void ClearVolumeReservation(HOBJECT hAI);

		// Lit

		LTBOOL IsLit() const { return m_bLit; }
		LTBOOL WasInitiallyLit() const { return m_bLitInitially; }
		HSTRING GetLightSwitchUseObjectNodeName() const { return m_hstrLightSwitchUseObjectNode; }
		HOBJECT GetLightSwitchUseObjectNode() const { return m_hLightSwitchUseObjectNode; }
		void SetLightState(LTBOOL bLit);

		// Pathfinding

		LTFLOAT GetPathWeight(LTBOOL bIncludeBaseWeight, LTBOOL bDivergePaths) const;

		LTFLOAT GetShortestEstimate() const { return m_fShortestEstimate; }
		void SetShortestEstimate(LTFLOAT fShortestEstimate) { m_fShortestEstimate = fShortestEstimate; }

		AIVolume* GetNextVolume() const { return m_pPreviousVolume; }
		void SetNextVolume(AIVolume* pNextVolume) { m_pPreviousVolume = pNextVolume; }

		AIVolume* GetPreviousVolume() const { return m_pPreviousVolume; }
		void SetPreviousVolume(AIVolume* pPreviousVolume) { m_pPreviousVolume = pPreviousVolume; }

		const LTVector& GetEntryPosition() const { return m_vEntryPosition; }
		void SetEntryPosition(const LTVector& vEntryPosition) { m_vEntryPosition = vEntryPosition; }

		const LTVector& GetWalkthroughPosition() const { return m_vWalkthroughPosition; }
		void SetWalkthroughPosition(const LTVector& vWalkthroughPosition) { m_vWalkthroughPosition = vWalkthroughPosition; }

		uint32 GetPathIndex() const { return m_nPathIndex; }
		void SetPathIndex(uint32 nPathIndex) { m_nPathIndex = nPathIndex; }

		virtual AIVolumeNeighbor* GetNeighborByIndex(uint32 iNeighbor);

		// Type 

		virtual AIVolume::EnumVolumeType GetVolumeType() { return AIVolume::kVolumeType_BaseVolume; }

		// Debug

		virtual	DebugLine::Color GetDebugColor() {if (!IsVolumeEnabled()) return Color::Yellow; if (IsLit()) return Color::Cyan; else return Color::Blue; }


	protected :

		enum
		{
			kMaxDoors = 2,
		};

	protected :
							
		LTFLOAT				m_fPathWeight;
		LTObjRef			m_hReservedBy;

		LTBOOL				m_bHadDoors;
		uint32				m_cDoors;
		LTObjRefNotifier	m_ahDoors[kMaxDoors];
							
		LTBOOL				m_bLitInitially;
		LTBOOL				m_bLit;
		LTObjRef			m_hLightSwitchUseObjectNode;
		HSTRING				m_hstrLightSwitchUseObjectNode;

		// Pathfinding data members (do not need to be saved)

		LTFLOAT				m_fShortestEstimate;
		AIVolume*			m_pPreviousVolume;
		LTVector			m_vEntryPosition;
		LTVector			m_vWalkthroughPosition;
		uint32				m_nPathIndex;
};

// ----------------------------------------------------------------------- //

class AIVolumeAmbientLife : public AIVolume
{
public:
	virtual AIVolume::EnumVolumeType GetVolumeType() { return AIVolume::kVolumeType_AmbientLife; }
};

// ----------------------------------------------------------------------- //

class AIVolumeLadder : public AIVolume
{
	typedef AIVolume super;

	public :

		// Ctor/Dtor

		AIVolumeLadder();
		virtual ~AIVolumeLadder();

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_Ladder; }

	protected :
};

class AIVolumeStairs : public AIVolume
{
	typedef AIVolume super;

	public :

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_Stairs; }

	protected :
};

class AIVolumeLedge : public AIVolume
{
	typedef AIVolume super;

	public :

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_Ledge; }

	protected :
};


class AIVolumePlayerInfo : public AIInformationVolume
{
	typedef AIInformationVolume super;

	public :

		// Ctor/Dtor

		AIVolumePlayerInfo();
		virtual ~AIVolumePlayerInfo();

		// Init

		virtual int Init();

		// Engine

		virtual bool ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// View nodes

		AINode* FindViewNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTBOOL bMustBeUnowned) const;
		AINode* FindOwnedViewNode(CAI* pAI, EnumAINodeType eNodeType, HOBJECT hOwner) const;

		// Handlers

		LTBOOL IsHiding() const { return m_bHiding; }
		LTBOOL IsHidingCrouchRequired() const { return m_bHidingCrouchRequired; }

		// Type

		virtual InformationVolumeType GetVolumeType() { return eTypePlayerInfo; }

	protected :

		enum
		{
			kMaxViewNodes = 16
		};

	protected :

		LTBOOL	m_bHiding;
		LTBOOL	m_bHidingCrouchRequired;

		HSTRING	m_ahstrViewNodes[kMaxViewNodes];
		LTObjRef m_ahViewNodes[kMaxViewNodes];
};

class AIVolumeJumpUp : public AIVolume
{
	typedef AIVolume super;

	public:

	static const LTFLOAT kJumpUpWidth;
	static const LTFLOAT kJumpUpStepWidth;
	static const LTFLOAT kJumpUpHeightThreshold;

	public :

		// Ctor/Dtor

		AIVolumeJumpUp();
		virtual ~AIVolumeJumpUp();

		// Engine

		virtual bool ReadProp(ObjectCreateStruct *pocs);
		virtual void InitialUpdate(float fUpdateType);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Pathing

		virtual LTBOOL CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext );
		virtual LTBOOL CanBuildPathFrom( CAI* pAI, AIVolume* pVolumeNext );
		virtual LTBOOL CanBuildPathThrough( CAI* pAI, AIVolume* pVolumePrev, AIVolume* pVolumeNext );
		virtual LTBOOL IsValidForPath( CAI* pAI, AIVolume* pVolumePrev );
		LTBOOL IsValidForPath( CAI* pAI, AIVolume* pVolumeNext, AIVolume* pVolumePrev );

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_JumpUp; }

		// Simple accessors.

		LTBOOL	HasRailing() const { return m_bHasRailing; }
		LTBOOL	OnlyJumpDown() const { return m_bOnlyJumpDown; }

	protected :

		LTBOOL		m_bOnlyJumpDown;
		LTBOOL		m_bOnlyWhenAlert;
		LTBOOL		m_bHasRailing;
};

class AIVolumeJumpOver : public AIVolume
{
	typedef AIVolume super;

	public :

		// Ctor/Dtor

		AIVolumeJumpOver();
		virtual ~AIVolumeJumpOver();

		// Engine

		virtual void InitialUpdate(float fUpdateType);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Pathing

		virtual LTBOOL CanBePathDest() { return LTFALSE; }
		virtual LTBOOL CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext );
		virtual LTBOOL CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev );

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_JumpOver; }

	protected :

		LTBOOL	m_bValidJumpOver;
};

class AIVolumeJunction : public AIVolume
{
	typedef AIVolume super;

	public :

		enum Constants
		{
			kMaxVolumes = 4,
			kMaxActionsPerVolume = 4,
		};

		struct VolumeActionsRecord
		{
			AIVolume*		pVolume;
			uint8			iVolume;
			LTFLOAT			fTotalChance;
			LTFLOAT			afChances[kMaxActionsPerVolume];
			JunctionAction	aeActions[kMaxActionsPerVolume];
		};

		typedef std::vector<VolumeActionsRecord> VolumeActionList;

	public :

		// Ctor/Dtor

		AIVolumeJunction();
		virtual ~AIVolumeJunction();

		// Engine

		virtual bool ReadProp(ObjectCreateStruct *pocs);
		virtual void InitialUpdate(float fUpdateType);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Methods

		void	RecordActionVolumeMask(AIVolume* pLastVolume, uint8* pmskActionVolumes);
		LTBOOL	GetAction(CAI* pAI, uint8* pmskActionVolumes, AIVolume* pLastVolume, AIVolume* pCorrectVolume, AIVolume** ppVolume, JunctionAction* peJunctionAction);

		// Type

		virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_Junction; }

		// Debug

		virtual	DebugLine::Color GetDebugColor() {if (!IsVolumeEnabled()) return Color::Yellow; if (IsLit()) return Color::Red; else return Color::Purple; }

	protected :

		struct VOLUME
		{
			VOLUME()
			{
				m_hstrName = LTNULL;
				m_pVolume = LTNULL;
				m_fJunctionResetTime = 0.f;

				for ( uint32 iAction = 0 ; iAction < kMaxActionsPerVolume ; iAction++ )
				{
					m_aeActions[iAction] = eJunctionActionContinue;
					m_afActionChances[iAction] = 0.0f;
				}
			}

			~VOLUME()
			{
				FREE_HSTRING(m_hstrName);
			}

			void Save(ILTMessage_Write *pMsg)
			{
				SAVE_HSTRING(m_hstrName);
				SAVE_COBJECT(m_pVolume);

				for ( uint32 iAction = 0 ; iAction < kMaxActionsPerVolume ; iAction++ )
				{
					SAVE_DWORD(m_aeActions[iAction]);
					SAVE_FLOAT(m_afActionChances[iAction]);
				}
			}

			void Load(ILTMessage_Read *pMsg)
			{
				LOAD_HSTRING(m_hstrName);
				LOAD_COBJECT(m_pVolume, AIVolume);

				for ( uint32 iAction = 0 ; iAction < kMaxActionsPerVolume ; iAction++ )
				{
					LOAD_DWORD_CAST(m_aeActions[iAction], JunctionAction);
					LOAD_FLOAT(m_afActionChances[iAction]);
				}
			}

			HSTRING			m_hstrName;
			AIVolume*		m_pVolume;
			JunctionAction	m_aeActions[kMaxActionsPerVolume];
			LTFLOAT			m_afActionChances[kMaxActionsPerVolume];
			LTFLOAT			m_fJunctionResetTime;
		};

	protected :

		VOLUME				m_aVolumes[kMaxVolumes];
};


//----------------------------------------------------------------------------
//              
//	CLASS:		AIVolumeTeleport
//              
//	PURPOSE:	Anything that fully enters a Teleport Volume is instantly
//				moved to the volume it references.  
//              
//	NOTES:
//
//----------------------------------------------------------------------------
class AIVolumeTeleport : public AIVolume
{
	typedef AIVolume super;

public:
	static const LTFLOAT kTeleportWidth;
	static const LTFLOAT kTeleportDepth;

public:

	// Ctor/Dtor
	
	AIVolumeTeleport();
	virtual ~AIVolumeTeleport();

	// Engine

	virtual bool ReadProp(ObjectCreateStruct *pocs);
	virtual int Init();

	// Overridden functions:

	virtual bool IsConnected( AISpatialRepresentation* pVolume );
	virtual LTBOOL CanBuildPathFrom( CAI* pAI, AIVolume* pVolumePrev );
	virtual LTBOOL CanBuildPathTo( CAI* pAI, AIVolume* pVolumeNext );

	void DoTeleportObject(CAI*, AIVolumeTeleport* );

protected:

	// Template Methods:
	
	virtual AIVolume::EnumVolumeType GetVolumeType() { return kVolumeType_Teleport; }

	// Modifying Methods:

	void VerifyDims();
	void VerifyDestination();
	void SetupDestinationVolume();
	void ReceiveTeleport( CAI*, const AIVolumeTeleport* const pSender );

	// Non Modifying Methods:

	bool HasDestination() const { return m_pTeleportVolumes != NULL && m_pTeleportVolumes != this; }
	bool IsVolumesTeleportDest(AISpatialRepresentation* pVolume) const { return pVolume == m_pTeleportVolumes; }
	bool ObjectIsInside( HOBJECT) const;

private:
	// Save:

	// Name of the Destination Volume.
	HSTRING				m_hstrDestVolume;

	// Corresponding Teleport Volume which this volume sends things to
	AIVolumeTeleport*	m_pTeleportVolumes;

	// Do not save:
};

#endif
