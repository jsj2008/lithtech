// ----------------------------------------------------------------------- //
//
// MODULE  : KeyMgr.h
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _KEY_MGR_H_
#define _KEY_MGR_H_

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "LTObjRef.h"
#include "IDList.h"
#include "ButeListReader.h"

#define KEY_DEFAULT_FILE	"Attributes\\KeyItems.txt"
#define KEY_MAX_NAME		32
#define KEY_MAX_FILE_PATH	64

#define KEY_INVALID_ID		(0xFFFFFFFF)  // Key ID which is guaranteed to be invalid

class CKeyMgr;
extern CKeyMgr* g_pKeyMgr;


struct KEY
{
	KEY();
	uint16	nId;
	char	szName[KEY_MAX_NAME];
	uint16	nNameId;
	uint16  nDescriptionId;
	char	szImage[KEY_MAX_FILE_PATH];
	char	szFilename[KEY_MAX_FILE_PATH];

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;

	char	szFXName[KEY_MAX_NAME];
	char	szSoundName[KEY_MAX_FILE_PATH];
};

class CKeyMgr : public CGameButeMgr, public ILTObjRefReceiver
{
  public:

	CKeyMgr();
	virtual ~CKeyMgr();

    LTBOOL      Init(const char* szAttributeFile=KEY_DEFAULT_FILE);
	void		Term();
	
	void		Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
	void		Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

	uint16		GetNumKeys() {return m_KeyArray.size();}

	LTBOOL		IsValidKey(uint16 nID) {return nID < m_KeyArray.size();}
	KEY*		GetKey(uint16 nID);
	KEY*		GetKey(const char *pszName);

	void		AddKeyControl( HOBJECT hObj, uint16 nKeyId );
	bool		CanCharacterControlObject( HOBJECT hChar, HOBJECT hObj );

	// Implementing classes will have this function called
	// when HOBJECT ref points to gets deleted.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
	
	struct KeyControlMapEntry
	{
		IDList				m_IDList;
		LTObjRefNotifier	m_ObjRef;
	};
	
	typedef std::map< HOBJECT, KeyControlMapEntry > KeyControlMap;
	KeyControlMap const& GetKeyControlMap() { return m_mapKeyControl; }

  protected:

	typedef std::vector<KEY *> KeyArray;
	KeyArray m_KeyArray;

	KeyControlMap m_mapKeyControl;
};


#ifndef _CLIENTBUILD
////////////////////////////////////////////////////////////////////////////
//
// CKeyMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CKeyMgr
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CKeyMgrPlugin : public IObjectPlugin
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

		static CKeyMgr		sm_KeyMgr;
};
#endif

#endif // !defined(_KEY_MGR_H_)