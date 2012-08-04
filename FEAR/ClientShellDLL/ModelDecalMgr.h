// ----------------------------------------------------------------------- //
//
// MODULE  : ModelDecalMgr.h
//
// PURPOSE : Definition of class used to manage model decals
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MODEL_DECAL_MGR_H__
#define __MODEL_DECAL_MGR_H__

#include "CategoryDB.h"

// Note : This class must not be named "CModelDecalMgr" to avoid a conflict with the engine code
class CGameModelDecalMgr : public ILTObjRefReceiver
{
public:
	// No copying allowed..
	PREVENT_OBJECT_COPYING(CGameModelDecalMgr);

	//////////////////////////////////////////////////////////////////////////
	// Framework-level calls
	
	CGameModelDecalMgr();
	~CGameModelDecalMgr();

	// Serialization
	void Save(ILTMessage_Write* pMsg, SaveDataState eSaveDataState);
	void Load(ILTMessage_Read* pMsg, SaveDataState eLoadDataState);

	// Update, called once per frame by the game shell
	void Update();

	//////////////////////////////////////////////////////////////////////////
	// Decal addition, removal, etc.
	
	// Get a decal type based on the name
	enum { k_nInvalidDecalType = -1 };
	inline uint32 GetDecalType(const char* pDecalName);
	inline uint32 GetDecalType(HRECORD hDecalRecord);

	// Add a decal
	void AddDecal(HOBJECT hModel, HMODELNODE hNode, uint32 nDecalType, const LTVector& vOrigin, const LTVector& vDirection);
	
	// Move decals between models
	void TransferDecals(HOBJECT hSource, HOBJECT hDest);
	
	// Fade the decals for a given model
	void FadeDecals(HOBJECT hModel);

	//////////////////////////////////////////////////////////////////////////
	// Performance level control

	// Get the available number of performance level	
	uint32 GetNumPerformanceLevels() const;
	// Get the name of a given performance level
	const char *GetPerformanceLevelName(uint32 nLevel) const;
	// Get/Set the performance level
	void SetPerformanceLevel(uint32 nLevel);
	uint32 GetPerformanceLevel() const { return m_nPerformanceLevel; }

// Internal types	
private:
	// Material lists, for use by decal types
	typedef std::vector<HMATERIAL> TMaterialList;
	
	// Decal types, which define the parameters for a decal
	struct SDecalType
	{
		// Materials to choose from
		TMaterialList m_aMaterials;
		// Radius & variance
		float	m_fRadius, m_fRadiusVariance;
		// Fading delay & duration
		double	m_fFadeDelay, m_fFadeDuration;
	};
	typedef std::vector<SDecalType> TDecalTypeList;
	
	// Decal tracking structure
	struct SDecal
	{
		// Note: Unsafe obj ref notifier - Refer to notes in ltobjref.h when dealing with this member
		LTObjRefNotifierUnsafe	m_hObject;
		// The engine-side decal
		HMODELDECAL		m_hDecal;
		// The original type
		uint32			m_nType;
		// Fade start time
		double			m_fFadeStartTime;
	};
	typedef std::vector<SDecal> TDecalList;
	
	// Internal structure indicating a decal that was loaded, and will be created on the next update
	struct SLoadRecord
	{
		// Information for finding the object in question...
		LTVector	m_vDims, m_vPos;
		// The decal description to use when we find it
		uint32		m_nDecalType;
		LTMatrix3x4	m_mProjection;
	};
	typedef std::vector<SLoadRecord> TLoadRecordList;
	
	// Internal structure used when trying to find the correct object while loading
	// Used by FindObjectsAfterLoadFilterFn
	struct SFindObjectsAfterLoadInfo
	{
		const SLoadRecord* m_pLoadRecord;
		HOBJECT m_hResult;
	};

	// Model decal database category
	BEGIN_DATABASE_CATEGORY(ModelDecal, "FX/ModelDecal")
		DEFINE_GETRECORDATTRIB(Material, char const*);
		DEFINE_GETRECORDATTRIB(Radius, float);
		DEFINE_GETRECORDATTRIB(RadiusVariance, float);
		DEFINE_GETRECORDATTRIB(FadeDelay, float);
		DEFINE_GETRECORDATTRIB(FadeDuration, float);
	END_DATABASE_CATEGORY();

	// Model decal settings database category
	BEGIN_DATABASE_CATEGORY(ModelDecalSettings, "FX/ModelDecal/Settings")
		DEFINE_GETRECORDATTRIB(Total, float);
		DEFINE_GETRECORDATTRIB(PerModel, float);
		DEFINE_GETRECORDATTRIB(PerSecond, float);
	END_DATABASE_CATEGORY();
	
// Internal functions
private:
	// Create a decal given the internally required information
	void AddDecal(HOBJECT hObject, uint32 nDecalType, const LTMatrix3x4& mProjection);

	// Load the decal types from the database
	void LoadDecalTypes();	
	
	// Clean dead decals from the decal list
	void CleanDecalList();

	// Function used for re-attaching decals to their models after loading
	static void FindObjectAfterLoadFilterFn(HOBJECT hObj, void* pUserData);

	// Load the performance setting information from the database for the current performance level
	void LoadPerformanceSettings();
		
	// Called by the engine whenever a model that has a decal attached gets removed
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	// Remove a decal from all internal lists
	void RemoveDecal(uint32 nIndex);
	
	// Update the fading state of any fading decals
	void UpdateFading();
	
	// Decal delete notification function (static and local)
	static void DecalDeleteCallback(HMODELDECAL hDecal, void *pUser);
	void OnDecalDeleted(HMODELDECAL hDecal);
	
// Internal Data
private:
	// The available decal types
	TDecalTypeList m_aDecalTypes;
	
	// The active decals
	TDecalList m_aDecals;
	
	// List of fading decals
	Tuint32List m_aFadingDecals;

	// Decals pending re-attachment after load
	TLoadRecordList m_aLoadRecords;
	
	// Clean up the decal list on the next update
	bool	m_bCleanOnNextUpdate;
	
	// Time of last decal placement
	double	m_fLastDecalTime;
	
	// Current performance level
	uint32	m_nPerformanceLevel;
	// Current performance settings
	float	m_fMaxDecals, m_fDecalsPerModel, m_fDecalsPerSecond;
	
	// Decal that is in the process of being deleted, stored as an optimization to the deletion handling
	HMODELDECAL	m_hDeletingDecal;
};

// Access variable for using the model decal manager
extern CGameModelDecalMgr* g_pModelDecalMgr;

#endif // __MODEL_DECAL_MGR_H__
