// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_NODE_H_
#define _AI_NODE_H_

#include "ltengineobjects.h"

class AINode : public BaseClass
{
	public :

		AINode();
		virtual ~AINode();

		uint32		EngineMessageFn(uint32 messageID, void *p, LTFLOAT f);
		LTBOOL		ReadProp(ObjectCreateStruct *p);

	public :

		uint32		m_dwSearchFlags;
		uint32		m_dwCoverFlags;
		uint32		m_dwPanicFlags;
		uint32		m_dwVantageFlags;

		LTVector	m_vInitialPitchYawRoll;

		HSTRING		m_hstrCoverObject;
		HSTRING		m_hstrPanicObject;
		HSTRING		m_hstrUseObject;
		HSTRING		m_hstrPickupObject;
		HSTRING		m_hstrBackupCmd;

		LTFLOAT		m_fVantageRadiusSqr;
		LTFLOAT		m_fVantageThreatRadiusSqr;
		LTBOOL		m_bIgnoreVantageDir;
		LTFLOAT		m_fVantageFov;
		HSTRING		m_hstrVantageThreatRadiusReaction;
		HSTRING		m_hstrVantageDamageCmd;

		LTFLOAT		m_fCoverRadiusSqr;
		LTFLOAT		m_fCoverThreatRadiusSqr;
		LTBOOL		m_bIgnoreCoverDir;
		LTFLOAT		m_fCoverFov;
		HSTRING		m_hstrCoverThreatRadiusReaction;
		LTFLOAT		m_fCoverTimeout;
		LTFLOAT		m_fCoverHitpointsBoost;
		HSTRING		m_hstrCoverTimeoutCmd;
		HSTRING		m_hstrCoverDamageCmd;

		LTFLOAT		m_fUseObjectRadiusSqr;
		LTFLOAT		m_fPickupObjectRadiusSqr;
		LTFLOAT		m_fBackupRadiusSqr;

		LTFLOAT		m_fPanicRadiusSqr;

		HSTRING		m_hstrTrainingFailureCmd;

		LTBOOL		m_bPoodle;
};

enum SearchStatus
{
	eSearchStatusOk,
	eSearchStatusSearchedRecently,
	eSearchStatusRequiresPartner,
};

enum CoverStatus
{
	eCoverStatusOk,
	eCoverStatusTooFar,
	eCoverStatusThreatOutsideFOV,
	eCoverStatusThreatInsideRadius,
};

enum VantageStatus
{
	eVantageStatusOk,
	eVantageStatusTooFar,
	eVantageStatusThreatOutsideFOV,
	eVantageStatusThreatInsideRadius,
};

class CAINode
{
	public : // Public member variables

		const static uint32 kInvalidNodeID;

        const static uint32 kSearchFlagShineFlashlight;
        const static uint32 kSearchFlagLookUnder;
        const static uint32 kSearchFlagLookOver;
        const static uint32 kSearchFlagLookLeft;
        const static uint32 kSearchFlagLookRight;
        const static uint32 kSearchFlagAlert1;
        const static uint32 kSearchFlagAlert2;
        const static uint32 kSearchFlagAlert3;
		const static uint32 kSearchFlagAny;
		const static uint32 kNumSearchFlags;

		const static uint32 kCoverFlagDuck;
		const static uint32 kCoverFlagBlind;
		const static uint32 kCoverFlag1WayCorner;
		const static uint32 kCoverFlag2WayCorner;
		const static uint32 kCoverFlagAny;
		const static uint32	kNumCoverFlags;

		const static uint32 kPanicFlagStand;
		const static uint32 kPanicFlagCrouch;
		const static uint32 kPanicFlagAny;
		const static uint32	kNumPanicFlags;

		const static uint32 kVantageFlagVantage;
		const static uint32 kVantageFlagAny;
		const static uint32	kNumVantageFlags;

	public : // Public methods

		CAINode();
		~CAINode();

		void Init(uint32 dwID, const AINode& node);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Verify();

		// Simple accessors

		uint32	GetID() const { return m_dwID; }

		HSTRING GetName() const { return m_hstrName; }

		const LTVector& GetPos() const { return m_vPos; }
		const LTVector& GetUp() { return m_vUp; }
		const LTVector& GetForward() { return m_vForward; }
		const LTVector& GetRight() { return m_vRight; }

		// Lock/unlock

		void	Lock() { m_bLocked = LTTRUE; }
		void	Unlock() { m_bLocked = LTFALSE; }
		LTBOOL	IsLocked() const { return m_bLocked; }

		// Vantage

		uint32	GetVantageFlags() { return m_dwVantageFlags; }
		LTBOOL	IsVantage() const { return m_dwVantageFlags & kVantageFlagAny ? LTTRUE : LTFALSE; }
		VantageStatus	GetVantageStatus(const LTVector& vPos, HOBJECT hThreat) const;

		LTFLOAT	GetVantageRadiusSqr() const { return m_fVantageRadiusSqr; }
		LTFLOAT	GetVantageThreatRadiusSqr() const { return m_fVantageThreatRadiusSqr; }
		HSTRING GetVantageThreatRadiusReaction() const { return m_hstrVantageThreatRadiusReaction; }
		LTBOOL	IsIgnoreVantageDir() const { return m_bIgnoreVantageDir; }

		HSTRING GetVantageDamageCmd() const { return m_hstrVantageDamageCmd; }

		// Search

		void	Search();
		void	SearchReset() { _ASSERT(IsSearchable()); m_bSearched = LTFALSE; }

		uint32	GetSearchFlags() const { return m_dwSearchFlags; }
		LTBOOL	IsSearchable() const { return m_dwSearchFlags & kSearchFlagAny ? LTTRUE : LTFALSE; }
		SearchStatus GetSearchStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Cover

		LTFLOAT	GetCoverRadiusSqr() const { return m_fCoverRadiusSqr; }
		LTFLOAT	GetCoverThreatRadiusSqr() const { return m_fCoverThreatRadiusSqr; }
		HSTRING GetCoverThreatRadiusReaction() const { return m_hstrCoverThreatRadiusReaction; }
		LTBOOL	IsIgnoreCoverDir() const { return m_bIgnoreCoverDir; }

		uint32	GetCoverFlags() { return m_dwCoverFlags; }
		LTBOOL	IsCover() const { return m_dwCoverFlags & kCoverFlagAny ? LTTRUE : LTFALSE; }
		CoverStatus	GetCoverStatus(const LTVector& vPos, HOBJECT hThreat) const;

		LTBOOL	HasCoverObject() { return !!m_hstrCoverObject; }
		HSTRING	GetCoverObject() { return m_hstrCoverObject; }
		void	ClearCoverObject() { FREE_HSTRING(m_hstrCoverObject); }

		LTFLOAT GetCoverHitpointsBoost() const { return m_fCoverHitpointsBoost; }

		LTFLOAT GetCoverTimeout() const { return m_fCoverTimeout; }
		HSTRING GetCoverTimeoutCmd() const { return m_hstrCoverTimeoutCmd; }

		HSTRING GetCoverDamageCmd() const { return m_hstrCoverDamageCmd; }

		// Panic

		uint32	GetPanicFlags() { return m_dwPanicFlags; }
		LTBOOL	IsPanic() const { return m_dwPanicFlags & kPanicFlagAny ? LTTRUE : LTFALSE; }
		LTBOOL	IsPanicFromThreat(const LTVector& vPos, HOBJECT hThreat) const;

		LTFLOAT	GetPanicRadiusSqr() const { return m_fPanicRadiusSqr; }

		LTBOOL	HasPanicObject() { return !!m_hstrPanicObject; }
		HSTRING	GetPanicObject() { return m_hstrPanicObject; }
		void	ClearPanicObject() { FREE_HSTRING(m_hstrPanicObject); }

		// UseObject

		LTBOOL	HasUseObject() { return !!m_hstrUseObject; }
		HSTRING	GetUseObject() { return m_hstrUseObject; }

		LTFLOAT	GetUseObjectRadiusSqr() const { return m_fUseObjectRadiusSqr; }

		// PickupObject

		LTBOOL	HasPickupObject() { return !!m_hstrPickupObject; }
		HSTRING	GetPickupObject() { return m_hstrPickupObject; }

		LTFLOAT	GetPickupObjectRadiusSqr() const { return m_fPickupObjectRadiusSqr; }

		// BackupCmd

		LTBOOL	HasBackupCmd() { return !!m_hstrBackupCmd; }
		HSTRING	GetBackupCmd() { return m_hstrBackupCmd; }

		LTFLOAT	GetBackupRadiusSqr() const { return m_fBackupRadiusSqr; }

		// TrainingFailureCmd

		LTBOOL	HasTrainingFailureCmd() { return !!m_hstrTrainingFailureCmd; }
		HSTRING	GetTrainingFailureCmd() { return m_hstrTrainingFailureCmd; }

		// Poodle

		LTBOOL	IsPoodle() { return m_bPoodle; }

	private :

		LTVector	m_vPos;						// Node position
		LTVector	m_vUp;						// Node up vector
		LTVector	m_vForward;					// Node forward vector
		LTVector	m_vRight;					// Node right vector
		uint32		m_dwID;						// Node ID
		HSTRING		m_hstrName;					// Node name

		LTBOOL		m_bLocked;					// Has someone locked this node for exclusive use?

		uint32		m_dwVantageFlags;			// Node Vantage availabilites
		LTFLOAT		m_fVantageRadiusSqr;		// How close you have to be to the node to use it as Vantage
		LTFLOAT		m_fVantageThreatRadiusSqr;	// If our threat gets this close, ditch the node
		LTBOOL		m_bIgnoreVantageDir;		// Do vantage regardless of vantage dir?
		LTFLOAT		m_fVantageFovDp;			// FOV of vantage dir in dot product form
		HSTRING		m_hstrVantageThreatRadiusReaction;
		HSTRING		m_hstrVantageDamageCmd;

		uint32		m_dwSearchFlags;			// Node Search availabilites
		LTBOOL		m_bSearched;				// Have we been searched?

		uint32		m_dwCoverFlags;				// Node cover availabilites
		HSTRING		m_hstrCoverObject;			// The name of the cover object
		LTFLOAT		m_fCoverRadiusSqr;			// How close you have to be to the node to use it as cover
		LTFLOAT		m_fCoverThreatRadiusSqr;	// If our threat gets this close, ditch the node
		LTBOOL		m_bIgnoreCoverDir;			// Do cover regardless of dir?
		LTFLOAT		m_fCoverFovDp;				// FOV of vover dir in dot product form
		HSTRING		m_hstrCoverThreatRadiusReaction;
		LTFLOAT		m_fCoverTimeout;
		HSTRING		m_hstrCoverTimeoutCmd;
		HSTRING		m_hstrCoverDamageCmd;

		LTFLOAT		m_fCoverHitpointsBoost;

		uint32		m_dwPanicFlags;				// Node Panic availabilites
		HSTRING		m_hstrPanicObject;			// The name of the Panic object
		LTFLOAT		m_fPanicRadiusSqr;			// How close you have to be to the node to use it as panic

		HSTRING		m_hstrUseObject;			// The name of the use object
		LTFLOAT		m_fUseObjectRadiusSqr;		// How close you have to be to the node to use it as UseObject

		HSTRING		m_hstrPickupObject;			// The name of the Pickup object
		LTFLOAT		m_fPickupObjectRadiusSqr;		// How close you have to be to the node to Pickup it as PickupObject

		HSTRING		m_hstrBackupCmd;			// The backup command
		LTFLOAT		m_fBackupRadiusSqr;			// How close you have to be to the node to use it as Backup

		HSTRING		m_hstrTrainingFailureCmd;

		LTBOOL		m_bPoodle;					// Is this a poodle node?
};

#endif // _AI_NODE_H_
