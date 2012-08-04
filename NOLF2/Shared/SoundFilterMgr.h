// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFilterMgr.h
//
// PURPOSE : Definition of debris mgr
//
// CREATED : 7/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FILTER_MGR_H__
#define __SOUND_FILTER_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"

class CSoundFilterMgr;
extern CSoundFilterMgr* g_pSoundFilterMgr;

#define SFM_DEFAULT_FILE		"Attributes\\SoundFilters.txt"
#define SFM_INVALID_ID		-1

#define SFM_MAX_NAME_LENGTH		32
#define SFM_MAX_VAR_NAME_LENGTH	32
#define SFM_MAX_VARIABLES		14

struct SOUNDFILTER
{
	SOUNDFILTER();

    LTBOOL       Init(CButeMgr & buteMgr, char* aTagName);

    uint8       nId;

	char		szName[SFM_MAX_NAME_LENGTH];
	char		szFilterName[SFM_MAX_NAME_LENGTH];

	int			nNumVars;
	char		szVars[SFM_MAX_VARIABLES][SFM_MAX_VAR_NAME_LENGTH];
	LTFLOAT		fValues[SFM_MAX_VARIABLES];
};

typedef CTList<SOUNDFILTER*> SoundFilterList;


class CSoundFilterMgr : public CGameButeMgr
{
	public :

		CSoundFilterMgr();
		~CSoundFilterMgr();

        LTBOOL          Init(const char* szAttributeFile=SFM_DEFAULT_FILE);
		void			Term();

        SOUNDFILTER*    GetFilter(uint8 nId);
		SOUNDFILTER*	GetFilter(char* pName);

		int				GetNumFilters() const { return m_FilterList.GetLength(); }

		inline LTBOOL	IsDynamic(SOUNDFILTER* pFilter)
		{
			return (pFilter && (strcmpi(pFilter->szName, "Dynamic") == 0));
		}

		inline LTBOOL	IsUnFiltered(SOUNDFILTER* pFilter)
		{
			// First record is the dynamic filter definition...
			return (pFilter && (strcmpi(pFilter->szName, "UnFiltered") == 0));
		}

	private :

		SoundFilterList		m_FilterList;
};


////////////////////////////////////////////////////////////////////////////
//
// CSoundFilterMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSoundFilterMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD
#ifndef __PSX2

#include "iobjectplugin.h"

class CSoundFilterMgrPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CSoundFilterMgr		sm_SoundFilterMgr;
};

#endif  // !__PSX2
#endif // _CLIENTBUILD


#endif // __SOUND_FILTER_MGR_H__