// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisMgr.h
//
// PURPOSE : Definition of debris mgr
//
// CREATED : 3/17/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_MGR_H__
#define __DEBRIS_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"
#include "SurfaceDefs.h"

class CDebrisMgr;
extern CDebrisMgr* g_pDebrisMgr;

#define DEBRISMGR_DEFAULT_FILE		"Attributes\\Debris.txt"
#define DEBRISMGR_INVALID_ID		-1

#define DEBRIS_MAX_NAME_LENGTH		32
#define DEBRIS_MAX_FILE_PATH		64
#define DEBRIS_MAX_MODELS			5
#define DEBRIS_MAX_BOUNCE_SNDS		2
#define DEBRIS_MAX_EXPLODE_SNDS		2

struct DEBRIS
{
	DEBRIS();

    LTBOOL       Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CDebrisMgr* pDebrisMgr);

    uint8       nId;

	char		szName[DEBRIS_MAX_NAME_LENGTH];

	int			nNumModels;
	char		szModel[DEBRIS_MAX_MODELS][DEBRIS_MAX_FILE_PATH];

	char		szSkin[DEBRIS_MAX_FILE_PATH];

	int			nNumBounceSnds;
	char		szBounceSnd[DEBRIS_MAX_BOUNCE_SNDS][DEBRIS_MAX_FILE_PATH];

	int			nNumExplodeSnds;
	char		szExplodeSnd[DEBRIS_MAX_EXPLODE_SNDS][DEBRIS_MAX_FILE_PATH];

	SurfaceType	eSurfaceType;

    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vMinDOffset;
    LTVector     vMaxDOffset;
	float		fMinScale;
	float		fMaxScale;
	float		fMinLifetime;
	float		fMaxLifetime;
	float		fFadetime;
	float		fGravityScale;
	float		fAlpha;
	int			nNumber;
	int			nMinBounce;
	int			nMaxBounce;
    LTBOOL       bRotate;
};

typedef CTList<DEBRIS*> DebrisList;


class CDebrisMgr : public CGameButeMgr
{
	public :

		CDebrisMgr();
		~CDebrisMgr();

		void			CacheAll();
        void            Reload(ILTCSBase *pInterface) { Term(); m_buteMgr.Term(); Init(pInterface); }

        LTBOOL           Init(ILTCSBase *pInterface, const char* szAttributeFile=DEBRISMGR_DEFAULT_FILE);
		void			Term();

        DEBRIS*         GetDebris(uint8 nId);
		DEBRIS*			GetDebris(char* pName);

		int				GetNumDebris() const { return m_DebrisList.GetLength(); }

#if defined(_CLIENTBUILD)

        HOBJECT         CreateDebris(uint8 nDebrisId, LTVector vPos);
        char*           GetExplodeSound(uint8 nDebrisId);
        char*           GetBounceSound(uint8 nDebrisId);

#endif

	private :

		DebrisList		m_DebrisList;
};


////////////////////////////////////////////////////////////////////////////
//
// CDebrisMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CDebrisMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CLIENTBUILD
#include "iobjectplugin.h"

class CDebrisMgrPlugin : public IObjectPlugin
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

		static CDebrisMgr		sm_DebrisMgr;
};

#endif // _CLIENTBUILD


#endif // __DEBRIS_MGR_H__