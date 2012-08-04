// ----------------------------------------------------------------------- //
//
// MODULE  : PropTypeMgr.h
//
// PURPOSE : Definition of prop type mgr
//
// CREATED : 4/27/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_TYPE_MGR_H__
#define __PROP_TYPE_MGR_H__

#include "GameButeMgr.h"
#include "TemplateList.h"

class CPropTypeMgr;
extern CPropTypeMgr* g_pPropTypeMgr;

#define PTMGR_DEFAULT_FILE		"Attributes\\PropTypes.txt"
#define PTMGR_INVALID_ID		-1

#define PROPTYPE_MAX_NAME_LENGTH		32
#define PROPTYPE_MAX_FILE_PATH			64

struct PROPTYPE
{
	PROPTYPE();

    LTBOOL      Init(CButeMgr & buteMgr, char* aTagName);
	void		Cache(CPropTypeMgr* pPropTypeMgr);

    uint32      nId;

	char		szType[PROPTYPE_MAX_NAME_LENGTH];
	char		szFilename[PROPTYPE_MAX_FILE_PATH];
	char		szSkin[PROPTYPE_MAX_FILE_PATH];
	char		szTouchSound[PROPTYPE_MAX_FILE_PATH];
	char		szDebrisType[PROPTYPE_MAX_NAME_LENGTH];

    LTVector    vScale;
    LTVector    vObjectColor;
    LTFLOAT     fAlpha;
    LTBOOL      bVisible;
    LTBOOL      bSolid;
    LTBOOL      bShadow;
	LTBOOL		bGravity;
    LTBOOL      bMoveToFloor;
    LTBOOL      bDetailTexture;
    LTBOOL      bChrome;
    LTBOOL      bChromaKey;
    LTBOOL      bAdditive;
    LTBOOL      bMultiply;
    LTBOOL      bRayHit;

	int			nTouchSoundRadius;
	int			nHitPoints;
};

typedef CTList<PROPTYPE*> PropTypeList;


class CPropTypeMgr : public CGameButeMgr
{
	public :

		CPropTypeMgr();
		~CPropTypeMgr();

		void			CacheAll();
        void            Reload(ILTCSBase *pInterface) { Term(); m_buteMgr.Term(); Init(pInterface); }

        LTBOOL           Init(ILTCSBase *pInterface, const char* szAttributeFile=PTMGR_DEFAULT_FILE);
		void			Term();

        PROPTYPE*       GetPropType(uint32 nId);
		PROPTYPE*		GetPropType(char* pType);

		int				GetNumPropTypes() const { return m_PropTypeList.GetLength(); }

	private :

		PropTypeList	m_PropTypeList;
};


////////////////////////////////////////////////////////////////////////////
//
// CPropTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CPropTypeMgr
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CPropTypeMgrPlugin : public IObjectPlugin
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

		static CPropTypeMgr		sm_PropTypeMgr;
};

#endif // __PROP_TYPE_MGR_H__