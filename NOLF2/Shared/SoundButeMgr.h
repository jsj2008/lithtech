// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeMgr.h
//
// PURPOSE : The SoundButeMgr
//
// CREATED : 11/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_BUTE_MGR_H__
#define __SOUND_BUTE_MGR_H__

//
// Includes...
//

	#include "HierarchicalButeMgr.h"
	#include "SoundTypes.h"

	#pragma warning (disable : 4786)
	#include <deque>


//
// Defines...
//

	#define SOUND_BUTES_DEFAULT_FILE	"Attributes\\SoundButes.txt"
	#define SOUND_BUTES_STRING_SIZE		64
	#define INVALID_SOUND_BUTE			-1


//
// Globals...
//
	
	class CSoundButeMgr;
	extern CSoundButeMgr* g_pSoundButeMgr;



struct SoundFiles
{
	typedef std::deque<char *>	SoundFileList;
	typedef std::deque<LTFLOAT>	WeightList;

	SoundFiles()
	{
		m_nNumSounds = 0;
		memset(m_szName,0,sizeof(m_szName));
		memset(m_szParent,0,sizeof(m_szParent));
	}

	~SoundFiles();

	uint32			m_nNumSounds;		// The number of sounds in the set

	// Our name in the bute file.
	char	      m_szName[SOUND_BUTES_STRING_SIZE];			// Sound FX name

	// Parent information
	char	m_szParent[SOUND_BUTES_STRING_SIZE];	// The name of the parent of this character

	// The list of sounds and their weights.
	SoundFileList m_szSounds;									// Skin filenames
	WeightList	  m_fWeights;									// Random chance weights
};


struct SoundBute
{
	SoundBute::SoundBute()
		:	m_fInnerRad		( 0.0f ),
			m_fOuterRad		( 0.0f ),
			m_fPitch		( 0.0f ),
			m_fPlayChance	( 0.0f ),
			m_nVolume		( 0 ),
			m_ePriority		( SOUNDPRIORITY_MISC_LOW ),
			m_nFlags		( 0 ),
			m_fMinDelay		( 0.0f ),
			m_fMaxDelay		( 0.0f )
	{
		
	}

	LTFLOAT	m_fInnerRad;				// Inner sound radius
	LTFLOAT	m_fOuterRad;				// Outer sound radius
	LTFLOAT	m_fPitch;					// The pitch of the sound
	LTFLOAT	m_fPlayChance;				// The chance that a sound will play

	uint8			m_nVolume;			// The volume of the sound
	SoundPriority	m_ePriority;		// The priority of the sound

	uint32			m_nFlags;			// Sound flags

	LTBOOL			m_bLipSync;			// Will lip sync if able
	LTFLOAT			m_fMinDelay;		// min delay before playing this sound again
	LTFLOAT			m_fMaxDelay;		// max delay before playing this sound again

};



class CSoundButeMgr : public CHierarchicalButeMgr
{
	public:

		CSoundButeMgr();
		virtual ~CSoundButeMgr();

		LTBOOL		Init( const char* szAttributeFile = SOUND_BUTES_DEFAULT_FILE );
		void		Term();

		LTBOOL		WriteFile()						{ return m_buteMgr.Save(); }
		void		Reload()						{ m_buteMgr.Parse(m_strAttributeFile); }

		int			GetNumSoundButes()				const { return m_nNumSoundButes; }

		int			GetSoundSetFromName(const char *szName)		const;

		const char* GetRandomSoundFile(uint32 nSet);
		const char* GetRandomSoundFileWeighted(uint32 nSet);

		char*		GetSoundButeName(int nSet)		{ CheckSound(nSet); return m_pSoundFiles[nSet].m_szName; }
		char*		GetSoundButeParent(int nSet)	{ CheckSound(nSet); return m_pSoundFiles[nSet].m_szParent; }

		//-------------------------------------------------------------------------------//

		void		CheckSound(int nSet)			const { ASSERT(nSet < m_nNumSoundButes); ASSERT(m_pSoundButes); }

		//-------------------------------------------------------------------------------//
		// Special function to fill in a SoundButes structure

		const SoundBute & GetSoundBute(int nSet)		const { CheckSound(nSet); return m_pSoundButes[nSet];  }
		const SoundBute & GetSoundBute(const char *szName)	const;

		const SoundFiles & GetSoundFiles(int nSet) const { CheckSound(nSet); return m_pSoundFiles[nSet]; }
		const SoundFiles & GetSoundFiles(const char *szName)	const;

	private:

		static bool CountTags(const char *szTagName, void *pData);
		static bool LoadButes(const char *szTagName, void *pData);

		// Loading functions (called from Init)
		void LoadSoundButes(const char * szTagName, SoundBute * pButes, SoundFiles * pFiles);

	private:

		// Sound attribute set variables
		int				m_nNumSoundButes;
		SoundBute		*m_pSoundButes;
		SoundFiles		*m_pSoundFiles;


		IndexTable    m_IndexTable;
};

////////////////////////////////////////////////////////////////////////////
//
// CSoundButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use SoundMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD

#include "iobjectplugin.h"

class CSoundButeMgrPlugin : public IObjectPlugin
{
	public:

		CSoundButeMgrPlugin()	{}

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength,
			const char* szParent = LTNULL);

	protected :

		static LTBOOL				sm_bInitted;
		static CSoundButeMgr	sm_ButeMgr;
};

#endif // _CLIENTBUILD

#endif // _SOUND_BUTE_MGR_H_
