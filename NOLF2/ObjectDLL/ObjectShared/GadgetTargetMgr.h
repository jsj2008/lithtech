// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetMgr.h
//
// PURPOSE : The GadgetTargetMgr object
//
// CREATED : 9/4/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__GADGET_TARGET_MGR_H__
#define __GADGET_TARGET_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "TemplateList.h"
	#include "ButeListReader.h"

//
// Forwards...
//

	class CGadgetTargetMgr;
	extern	CGadgetTargetMgr	*g_pGadgetTargetMgr;

//
// Defines...
//

	#define	GTMGR_DEFAULT_FILE		"Attributes\\GadgetTargets.txt"
	#define	GTMGR_INVALID_ID		-1
	#define GTMGR_MAX_NAME_LENGTH	32
	#define GTMGR_MAX_FILE_PATH		64

	#define GT_TYPE_TO_FLAG( type ) (1<<type)


struct GADGETTARGET
{
	GADGETTARGET();
	~GADGETTARGET();

	LTBOOL	Init( CButeMgr &ButeMgr, char *aTagName );

	uint32			nId;

	char			*szName;
	char			*szFileName;
	char			*szDebrisType;
	char			*szDisablingSnd;
	char			*szDisabledSnd;

	CButeListReader	blrGTSkinReader;
	CButeListReader	blrGTRenderStyleReader;

	uint32			nLightType;
	LTVector		vLightScale;
	CButeListReader	blrGTLightRenderStyleReader;

	char			*szLight1FileName;
	CButeListReader	blrGTLight1SkinReader;
	char			*szLight1SocketName;

	char			*szLight2FileName;
	CButeListReader	blrGTLight2SkinReader;
	char			*szLight2SocketName;


	LTVector		vScale;
	LTVector		vObjectColor;
	LTFLOAT			fAlpha;
	int				nHitPts;
	LTBOOL			bSolid;
	LTBOOL			bVisible;
	LTBOOL			bShadow;
	LTBOOL			bMoveToFloor;
	LTBOOL			bAdditive;
	LTBOOL			bMultiply;
	int				nType;			// Defined in GadgetTargetTypes
	LTFLOAT			fMinTime;
	LTFLOAT			fMaxTime;
	LTFLOAT			fSoundRadius;
	uint32			dwCodeID;		// Depending on what type of gadgettarget we are its either the actual
									// 1 - 8 digit code or an ID into Cres.dll for the coded text.
	LTBOOL			bRemoveWhenDisabled;
	LTBOOL			bInfiniteDisables;
	LTBOOL			bInfiniteActivates;

};

typedef CTList<GADGETTARGET*> GadgetTargetList;


class CGadgetTargetMgr : public CGameButeMgr 
{
	public :	// Methods...

		CGadgetTargetMgr();
		~CGadgetTargetMgr();

		void			Reload() { Term(); m_buteMgr.Term(); Init(); }

		LTBOOL			Init( const char *szAttributeFile = GTMGR_DEFAULT_FILE );
		void			Term();

		GADGETTARGET	*GetGadgetTarget( uint32 nID );
		GADGETTARGET	*GetGadgetTarget( char *pName );

		int				GetNumGadgetTargets() const { return m_GadgetTargetList.GetLength(); }

	private :	// Members...

		GadgetTargetList	m_GadgetTargetList;
};



////////////////////////////////////////////////////////////////////////////
//
// CGadgetTargetMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CGadgetTargetMgrPlugin
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CGadgetTargetMgrPlugin : public IObjectPlugin
{
	public:

		CGadgetTargetMgrPlugin( )
		:	IObjectPlugin	(),
			m_dwFilterTypes ( 0 )
		{

		}

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

		void	SetFilterOutTypes( uint32 Types ) { m_dwFilterTypes = Types; }

	protected :

		static CGadgetTargetMgr		sm_GadgetTargetMgr;

	private :

		uint32	m_dwFilterTypes;
};


#endif // __GADGET_TARGET_MGR_H__
