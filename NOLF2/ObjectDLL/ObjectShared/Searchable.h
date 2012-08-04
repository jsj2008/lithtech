// ----------------------------------------------------------------------- //
//
// MODULE  : Searchable.h
//
// PURPOSE : Searchable class
//
// CREATED : 12/17/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCHABLE_H__
#define __SEARCHABLE_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"
#include "SearchItemMgr.h"

// Use ADD_SEARCHABLE_AGGREGATE() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_SEARCHABLE_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_SEARCHABLE_AGGREGATE(group, flags) \
	PROP_DEFINEGROUP(SearchProperties, (group) | (flags)) \
		ADD_STRINGPROP_FLAG(SpecificItems, "", (group) | (flags)) \
		ADD_STRINGPROP_FLAG(RandomItemSet, "", (group) | (flags) | PF_STATICLIST  ) \
		ADD_STRINGPROP_FLAG(SearchSoundName, "Interface\\Snd\\SearchBodyLoop.wav", (group) | (flags) | PF_FILENAME) \
		ADD_REALPROP_FLAG(SearchSoundRadius, 200.0f, (group) | (flags) | PF_RADIUS)

class CPickupItemRef
{
public:
	CPickupItemRef() : m_hItem(NULL), m_bForcePickup(false) {}
	LTObjRef	m_hItem;
	bool		m_bForcePickup;
};


class CSearchable : public IAggregate
{
	public :

		CSearchable();
		virtual ~CSearchable();

		bool AddPickupItem(HOBJECT hItem, bool bForcePickup = false);
		void RemovePickupItem(HOBJECT hItem);
		void ClearPickupItems();
		void ResetPickupItems();

		bool IsEnabled()			{ return m_bEnabled; }
		void Enable(bool bEnable);

		bool HasItem() {return (m_nPickupItems > 0 || !m_bGaveLastItem);}

		void CopySearchProperties(const CSearchable* pSearchable);

		SEARCH_SET*	GetRandomItemSet() const {return m_pRandomItemSet;}
		void		SetRandomItemSet(char* szSetName);
		
		const HSTRING GetSpecificItem() const { return m_hstrSpecificItem; }

		void	StartSound();
		void	StopSound();

		void	SetIsBody(bool bBody) {m_bIsBody = bBody;}

	protected :

		enum Constants
		{
			kMaxPickupItems = 32,
		};

        uint32 EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		
		void	CalcSearchTime();
		void	CalcTotalTime();

		LTObjRef	m_hObject;

		SEARCH_SET*	m_pRandomItemSet;

		SEARCH_SET::SearchObjectType m_eSearchObjectType;


		uint8			m_nPickupItems;
		CPickupItemRef	m_ahPickupItems[kMaxPickupItems];

		LTFLOAT		m_fTotalTime;
		LTFLOAT		m_fRemainingTime;
		LTFLOAT		m_fNextFindTime;

		HSTRING		m_hstrSpecificItem;
		bool		m_bGaveLastItem;

		bool		m_bEnabled;
		bool		m_bIsBody;

		HMODELANIM	m_hPreSearchAnimIndex;
		uint32		m_dwPreSearchAnimTime;
		bool		m_bPreSearchAnimLoop;
		bool		m_bPreSearchAnimPlaying;

		HSTRING		m_hstrSoundName;
        HLTSOUND    m_hSound;
        LTFLOAT		m_fSoundRadius;         // Radius of search sound

		bool		m_bSearchStarted;


	private :

        LTBOOL ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pInfo);
		void InitialUpdate(LPBASECLASS pObject);

		void	HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);
		void	StartSearch( HOBJECT hSender );
		void	EndSearch();

		bool	GiveLastSearchItem(HOBJECT hSender);

        void	Save(ILTMessage_Write *pMsg);
        void	Load(ILTMessage_Read *pMsg);
};


#ifndef __PSX2
class CSearchItemPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :

	  	  CSearchItemMgrPlugin m_SearchItemMgrPlugin;
};

#endif

#endif // __SEARCHABLE_H__