#ifndef __INVITEM_H__
#define __INVITEM_H__

#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "SharedDefs.h"
#include "ClientRes.h"
#include <mbstring.h>

// Inventory item base class
class CInvItem
{
	public:

		CInvItem(DBYTE dbItemType)
		{
			m_hOwner = DNULL;
			m_bInitialized = DFALSE;
			m_nType = dbItemType;
			m_nCount = 1;
			m_bIsActive = DFALSE;
			m_hstrPic = DNULL;
			m_hstrPicH = DNULL;
			m_hstrItemName = DNULL;
			m_hstrDisplayName = DNULL;
			dl_TieOff( &m_Link );
		}

		virtual ~CInvItem() 
		{
			Term();
		}

		virtual void Init(HOBJECT hOwner)
		{
			m_hOwner = hOwner;
		}

		virtual void Term()
		{
			if (!g_pServerDE) return;
			m_bInitialized = DFALSE;
			m_nCount = 0;
			if (m_hstrPic)
			{
				g_pServerDE->FreeString(m_hstrPic);
				m_hstrPic = DNULL;
			}
			if (m_hstrPicH)
			{
				g_pServerDE->FreeString(m_hstrPicH);
				m_hstrPicH = DNULL;
			}
			if (m_hstrItemName)
			{
				g_pServerDE->FreeString(m_hstrItemName);
				m_hstrItemName = DNULL;
			}
			if (m_hstrDisplayName)
			{
				g_pServerDE->FreeString(m_hstrDisplayName);
				m_hstrDisplayName = DNULL;
			}

			dl_TieOff( &m_Link );
		}

		virtual void	Update()
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return;

			if (!m_bInitialized)
				m_bInitialized = DTRUE;
		}

		DBYTE	GetType() { return m_nType; }

		HSTRING	GetPic() { return m_hstrPic; }
		HSTRING GetPicH() { return m_hstrPicH; }

		virtual DBOOL AddItem(DBYTE nCount) { return DFALSE; }
        
		virtual DBOOL ActivateItem(char *msgbuf)
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return DFALSE;

			_mbscpy((unsigned char*)msgbuf, (const unsigned char*)GetName());
			if (!IsActive())
			{
				if( Activate())
				{
					HSTRING on = pServerDE->FormatString(IDS_GENERAL_ON);
					char *pszOn;
					pszOn = pServerDE->GetStringData(on);
					if( pszOn )
						_mbscat((unsigned char*)msgbuf, ( const unsigned char * )pszOn );
//					_mbscat((unsigned char*)msgbuf, (const unsigned char*)" on");
					pServerDE->FreeString(on);
				}
				else
					return DFALSE;
			}
			else
			{
				Deactivate();

				HSTRING off = pServerDE->FormatString(IDS_GENERAL_OFF);
				char *pszOff;
				pszOff = pServerDE->GetStringData(off);
				_mbscat((unsigned char*)msgbuf, ( const unsigned char * )pszOff );
//				_mbscat((unsigned char*)msgbuf, (const unsigned char*)" off");
				pServerDE->FreeString(off);
			}

			return DTRUE;
		}

		virtual int Activate() = 0;
		virtual int Deactivate() = 0;
		virtual int	Dropit() { return 0; }
		virtual void PickItUp() { return;}
        
		DBOOL	IsDropped() { return m_bDropped; }
        
		DBOOL	IsActive() { return m_bIsActive; }

		char*	GetName() { return m_hstrItemName ? g_pServerDE->GetStringData(m_hstrItemName) : DNULL; }
		char*	GetDisplayName() { return m_hstrDisplayName ? g_pServerDE->GetStringData(m_hstrDisplayName) : DNULL; }
		DBYTE	GetCount();
		void	SetCount(DBYTE byCount) { m_nCount = byCount; }

		virtual void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void SendActionMessage( );

		DLink	m_Link;

	protected:

		HSTRING m_hstrItemName;		// Item name
		HSTRING m_hstrDisplayName;	// Item name to display
		HSTRING	m_hstrPic;			// Filename of the picture to use for this item
		HSTRING	m_hstrPicH;			// Filename of the highlighted picture for this item
		HOBJECT	m_hOwner;			// The character holding this weapon
		DBOOL	m_bInitialized;		// The item is initialized
        DBOOL   m_bDropped;
		DBYTE	m_nType;			// Item type
		DBOOL	m_bIsActive;		// Item is active
		DBYTE	m_nCount;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvItem::GetCount
//
//	PURPOSE:	Report the charges left.
//
// ----------------------------------------------------------------------- //

inline DBYTE CInvItem::GetCount( )
{
	if( INV_NONE < m_nType && m_nType <= INV_LASTINVITEM )
		return m_nCount;

	return 0;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

inline void CInvItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hOwner);
	pServerDE->WriteToMessageHString(hWrite, m_hstrItemName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDisplayName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPic);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPicH);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bInitialized);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bDropped);
	pServerDE->WriteToMessageByte(hWrite, m_nType);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bIsActive);
	pServerDE->WriteToMessageByte(hWrite, m_nCount);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInvItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

inline void CInvItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	HOBJECT hTmp;
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hTmp);
	m_hstrItemName	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDisplayName = pServerDE->ReadFromMessageHString(hRead);
	m_hstrPic		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPicH		= pServerDE->ReadFromMessageHString(hRead);
	m_bInitialized	= pServerDE->ReadFromMessageByte(hRead);
	m_bDropped		= pServerDE->ReadFromMessageByte(hRead);
	m_nType			= pServerDE->ReadFromMessageByte(hRead);
	m_bIsActive		= pServerDE->ReadFromMessageByte(hRead);
	m_nCount		= pServerDE->ReadFromMessageByte(hRead);

	// Make this item inactive on keepalive loads
	m_bIsActive = DFALSE;
}




#endif // __INVITEM_H__