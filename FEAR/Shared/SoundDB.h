// ----------------------------------------------------------------------- //
//
// MODULE  : SoundDB.h
//
// PURPOSE : Definition of Sound database
//
// CREATED : 02/23/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUNDDB_H__
#define __SOUNDDB_H__



//
// Includes...
//

#include "GameDatabaseMgr.h"

//
// Defines...
//
const char* const SndDB_fOuterRadius =		"OuterRadius";


//struct used to encapsulate sound initialization information retrieved from database
struct SoundRecord
{
	SoundRecord::SoundRecord()	:	
		m_fInnerRad			( 0.0f ),
		m_fOuterRad			( 0.0f ),
		m_fPitch			( 0.0f ),
		m_fPlayChance		( 0.0f ),
		m_nVolume			( 0 ),
		m_ePriority			( SOUNDPRIORITY_MISC_LOW ),
		m_nFlags			( 0 ),
		m_fMinDelay			( 0.0f ),
		m_fMaxDelay			( 0.0f ),
		m_fDopplerFactor	( 1.0f ),
		m_nMixChannel		( PLAYSOUND_MIX_DEFAULT ),
		m_fSoundSwitchRad	( 0.0f ),
		m_bShouldTestSwitchRadius (false)
	{

	}

	float	m_fInnerRad;				// Inner sound radius
	float	m_fOuterRad;				// Outer sound radius
	float	m_fPitch;					// The pitch of the sound
	float	m_fPlayChance;				// The chance that a sound will play

	uint8			m_nVolume;			// The volume of the sound
	SoundPriority	m_ePriority;		// The priority of the sound

	uint32			m_nFlags;			// Sound flags

	bool			m_bLipSync;			// Will lip sync if able
	float			m_fMinDelay;		// min delay before playing this sound again
	float			m_fMaxDelay;		// max delay before playing this sound again
	float			m_fDopplerFactor;	// amount to adjust the doppler by

	int16			m_nMixChannel;		// output channel for mixer

	float			m_fSoundSwitchRad;	// Radius where it will switch sound files, if applicable
	bool			m_bShouldTestSwitchRadius;		// true if there's a switch radius value to test.


};

//struct containing a weighted list of sound files for random selection
class CSoundSet
{
public:
	CSoundSet();
	virtual ~CSoundSet() {};

	void Init(HRECORD hRecord);
	const char* GetRandomFile();
	const char* GetRandomNotDirtyFile();

	const char* GetRandomAltFile();

	HRECORD	GetRecord() {return m_hRecord;}

private:
	void ClearDirty();

	HRECORD m_hRecord;
	typedef std::vector<bool, LTAllocator<bool, LT_MEM_TYPE_GAMECODE> > boolArray;
	typedef std::vector<uint32, LTAllocator<uint32, LT_MEM_TYPE_GAMECODE> > uint32Array;

	uint32Array	m_vecWeights;
	boolArray	m_vecDirty;
	uint32		m_nTotalWeight;

	uint32Array	m_vecAltWeights;
	boolArray	m_vecAltDirty;
	uint32		m_nAltTotalWeight;
};
typedef std::vector<CSoundSet, LTAllocator<CSoundSet, LT_MEM_TYPE_GAMECODE> > SoundSetArray;


class CSoundDB;
extern CSoundDB* g_pSoundDB;

class CSoundDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CSoundDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term();

	HCATEGORY GetSoundCategory() { return m_hSoundCat; }

	uint32	GetNumSoundRecords();
	HRECORD	GetSoundDBRecord(uint32 nIndex);
	HRECORD	GetSoundDBRecord(const char* pName);

	HRECORD GetCharacterSoundDBRecord(HRECORD hModel, const char* pName);

	//fills in sound record, returns false if record does not exist
	bool	FillSoundRecord(HRECORD hSR, SoundRecord& sr);

	const char* GetRandomSoundFileNotDirty(HRECORD hSR);
	const char* GetRandomSoundFileWeighted(HRECORD hSR);
	const char* GetRandomAltSoundFileWeighted(HRECORD hSR);

private	:	// Members...
	bool	InitSoundSets();


	HCATEGORY	m_hSoundCat;
	HCATEGORY	m_hCharacterSoundCat;

	SoundSetArray	m_vecSoundSets;



};


////////////////////////////////////////////////////////////////////////////
//
// CSoundDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"


class CSoundDBPlugin : public IObjectPlugin
{
private:

	CSoundDBPlugin();
	CSoundDBPlugin( const CSoundDBPlugin &other );
	CSoundDBPlugin& operator=( const CSoundDBPlugin &other );
	~CSoundDBPlugin();


public:

	NO_INLINE static CSoundDBPlugin& Instance() { static CSoundDBPlugin sPlugin; return sPlugin; }

	virtual LTRESULT	PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
		uint32* pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLength);

protected :


};


#endif // _SERVERBUILD



#endif  // __SOUNDDB_H__
