// ----------------------------------------------------------------------- //
//
// MODULE  : IntelMgr.h
//
// PURPOSE : Definition of intel mgr
//
// CREATED : 7/25/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTEL_MGR_H__
#define __INTEL_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"

class CIntelMgr;
extern CIntelMgr* g_pIntelMgr;

#define INTELMGR_DEFAULT_FILE		"Attributes\\IntelItems.txt"
#define INTELMGR_INVALID_ID			255

#define INTEL_MAX_NAME_LENGTH		32
#define INTEL_MAX_FILE_PATH			64

struct INTEL
{
	INTEL();
	~INTEL();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);

    uint8		nId;
	uint32		nDefaultTextId;
    uint8		nPopupId;

	char		*szName;
	char		*szFilename;
	char		*szSkin;
	char		*szPickupSnd;
	char		*szRespawnSnd;
};

typedef CTList<INTEL*> IntelList;


class CIntelMgr : public CGameButeMgr
{
	public :

		CIntelMgr();
		~CIntelMgr();

        void        Reload() { Term(); m_buteMgr.Term(); Init(); }

        LTBOOL      Init(const char* szAttributeFile=INTELMGR_DEFAULT_FILE);
		void		Term();

        INTEL*      GetIntel(uint32 nId);
		INTEL*		GetIntel(char* pName);

		int			GetNumIntels() const { return m_IntelList.GetLength(); }

	private :

		IntelList	m_IntelList;
};

#ifndef __PSX2
////////////////////////////////////////////////////////////////////////////
//
// CIntelMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CIntelMgr
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CIntelMgrPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims);

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CIntelMgr		sm_IntelMgr;
};
#endif

#endif // __INTEL_MGR_H__