#ifndef __GAMEINVITEMS_H__
#define __GAMEINVITEMS_H__

#include "InvItem.h"

#define FLASHLIGHTCHARGETIME	120.0f
#define FLASHLIGHTCHARGE		100
#define GOGGLESCHARGETIME		30.0f
#define GOGGLESCHARGE			100

class CInvFlashlight : public CInvItem
{
	public:

		CInvFlashlight();
		~CInvFlashlight() { Term( ); }
		virtual void Init(HOBJECT hOwner);
		DBOOL CreateLight();
		virtual void Term()
		{
			CInvItem::Term();

			if (!g_pServerDE) return;

			if (m_hLight)
			{
				g_pServerDE->RemoveObject(m_hLight);
				m_hLight = DNULL;
			}
		}

		DBOOL	AddItem(DBYTE nCount);
		int		Activate();
		int		Deactivate();
        
		void	Update();
		virtual void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

	private:

		HOBJECT	m_hLight;
		DFLOAT	m_fDischargeTime;
};


class CInvMedkit : public CInvItem
{
	public:

		CInvMedkit() : CInvItem(INV_MEDKIT) {}
		~CInvMedkit() {}
		virtual void Init(HOBJECT hOwner);
		DBOOL	AddItem(DBYTE nCount);
		int		Activate();
		int		Deactivate() { return 0; }
		DBOOL	ActivateItem(char *msgbuf);
};


class CInvNightGoggles : public CInvItem
{
	public:

		CInvNightGoggles();
		~CInvNightGoggles() { Term( ); }
		virtual void Init(HOBJECT hOwner);
		virtual void Term()
		{
			CInvItem::Term();

			if (!g_pServerDE) return;

/*			if (m_hLight)
			{
				g_pServerDE->RemoveObject(m_hLight);
				m_hLight = DNULL;
			}

			// Kill any old instance of the sound...
			if( m_hLoopingSound )
			{
				g_pServerDE->KillSound( m_hLoopingSound );
				m_hLoopingSound = DNULL;
			}
*/
		}
		
		DBOOL	AddItem(DBYTE nCount);
		int		Activate();
		int		Deactivate();
		void	Update();
		virtual void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
	
	private:

//		HOBJECT	m_hLight;
//		HSOUNDDE m_hLoopingSound;
		DFLOAT	m_fDischargeTime;
};



class CInvBinoculars : public CInvItem
{
	public:

		CInvBinoculars() : CInvItem(INV_BINOCULARS) {}
		virtual void Init(HOBJECT hOwner);
		
		int		Activate();
		int		Deactivate();
};


class CInvTheEye : public CInvItem
{
	public:

		CInvTheEye();
		~CInvTheEye() { Term( ); }
		virtual void Init(HOBJECT hOwner);
		virtual void Term();
		
		int		Activate();
		int		Deactivate();
		int     Dropit();
		void	PickItUp();
		void	SetRotation(DRotation* rot);
		DBOOL	ActivateItem(char *msgbuf);
		HOBJECT	GetEyeObject() const { return m_hSeeingObject; }
		virtual void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

	private:

		HOBJECT	m_hSeeingObject;
};


class CInvKey : public CInvItem
{
	public:

		CInvKey() : CInvItem(INV_KEY) {}
		~CInvKey() {}

		DBOOL Setup(HSTRING hstrIconFile, HSTRING hstrHiIconFile, HSTRING hstrItemName, HSTRING hstrDisplayName, DBYTE byCount)
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return DFALSE;

			m_hstrPic = pServerDE->CopyString(hstrIconFile);
			m_hstrPicH = pServerDE->CopyString(hstrHiIconFile);
			m_hstrItemName = pServerDE->CopyString(hstrItemName);
			m_hstrDisplayName = pServerDE->CopyString(hstrDisplayName);
			m_nCount = byCount;

			return DTRUE;
		}
		
		int		Activate() { return 0; };
		int		Deactivate() { return 0; };
};

class CInvProxBomb : public CInvItem
{
	public:

		CInvProxBomb() : CInvItem(INV_PROXIMITY) {}
		virtual void Init(HOBJECT hOwner);
		
		DBOOL	ActivateItem(char *msgbuf);
		int		Activate();
		int		Deactivate() { return 0; };
};


class CInvRemoteBomb : public CInvItem
{
	public:

		CInvRemoteBomb() : CInvItem(INV_REMOTE) {}
		virtual void Init(HOBJECT hOwner);
		
		DBOOL	ActivateItem(char *msgbuf);
		int		Activate();
		int		Deactivate() { return 0; };
};


class CInvTimeBomb : public CInvItem
{
	public:

		CInvTimeBomb() : CInvItem(INV_TIMEBOMB) {}
		virtual void Init(HOBJECT hOwner);
		
		DBOOL	ActivateItem(char *msgbuf);
		int		Activate();
		int		Deactivate() { return 0; };
};




#endif // __GAMEINVITEMS_H__