// ----------------------------------------------------------------------- //
//
// MODULE  : SearchItemMgr.h
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SEARCH_ITEM_MGR_H_)
#define _SEARCH_ITEM_MGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"


#define SI_DEFAULT_FILE		"Attributes\\SearchItems.txt"
#define SI_MAX_NAME			32
#define SI_MAX_FILE_PATH	64
#define SI_MAX_ITEMS		64
#define SI_MAX_OBJECTS		30

#define SI_INVALID_ID		(0xFF)  // Key ID which is guaranteed to be invalid

class CSearchItemMgr;
extern CSearchItemMgr* g_pSearchItemMgr;

struct SEARCH_ITEM
{
	SEARCH_ITEM();
	uint8	nId;
	char	szName[SI_MAX_NAME];
	uint16	nTextId;
	char	szIcon[SI_MAX_FILE_PATH];
};



struct SEARCH_SET
{
		SEARCH_SET();
		uint8	nId;
		char	szName[SI_MAX_NAME];

		// [KLS 7/13/02] - Updated to support new search object types and the ability to
		// weight the likelihood of finding a particular object.
		
		enum SearchObjectType 
		{ 
			eUnknownObjectType,
			eItemObjectType,
			eAmmoObjectType,
			eWeaponObjectType,
			eGearObjectType
		};

		struct SearchObjectResult
		{
			SearchObjectResult()
			{
				eType	= eUnknownObjectType;
				nId		= SI_INVALID_ID;
				nAmount	= 0;
			}

			SearchObjectType	eType;
			uint8				nId;
			uint32				nAmount;
		};

		bool	GetRandomSearchObjectInfo(SearchObjectResult & soResult, float fJunkModifier);

		// All of the following percents must add up to 100!
		uint8	nItemPercent;
		uint8	nAmmoPercent;
		uint8	nWeaponPercent;
		uint8	nGearPercent;

		uint8	nItems;
		uint8	anItems[SI_MAX_ITEMS];

		uint8	nAmmos;
		uint8	anAmmos[SI_MAX_OBJECTS];
		uint32	anAmmoAmounts[SI_MAX_OBJECTS];

		uint8	nWeapons;
		uint8	anWeapons[SI_MAX_OBJECTS];

		uint8	nGears;
		uint8	anGears[SI_MAX_OBJECTS];

	private :

		SearchObjectType GetRandomSearchObjectType(float fJunkModifier = 1.0f);
};


class CSearchItemMgr : public CGameButeMgr
{
public:
	CSearchItemMgr();
	virtual ~CSearchItemMgr();

    LTBOOL      Init(const char* szAttributeFile=SI_DEFAULT_FILE);
	void		Term();

	uint8		GetNumItems() {return m_ItemArray.size();}
	uint8		GetNumSets() {return m_SetArray.size();}

	LTBOOL		IsValidItem(uint8 nID) {return nID < m_ItemArray.size();}
	LTBOOL		IsValidSet(uint8 nID) {return nID < m_SetArray.size();}

	SEARCH_ITEM*	GetItem(uint8 nID);
	SEARCH_ITEM*	GetItem(const char *pszName);

	SEARCH_SET*		GetSet(uint8 nID);
	SEARCH_SET*		GetSet(const char *pszName);

protected:
	typedef std::vector<SEARCH_ITEM *> ItemArray;
	ItemArray m_ItemArray;

	typedef std::vector<SEARCH_SET *> SetArray;
	SetArray m_SetArray;



};


#ifndef _CLIENTBUILD
#ifndef __PSX2
////////////////////////////////////////////////////////////////////////////
//
// CSearchItemMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSearchItemMgr
//
////////////////////////////////////////////////////////////////////////////
#include "iobjectplugin.h"

class CSearchItemMgrPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

        LTBOOL PopulateStringList(char** aszStrings, uint32* pcStrings,
            const uint32 cMaxStrings, const uint32 cMaxStringLength);

	protected :

		static CSearchItemMgr		sm_SearchItemMgr;
};
#endif
#endif



#endif // !defined(_SEARCH_ITEM_MGR_H_)