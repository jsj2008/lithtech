// ----------------------------------------------------------------------- //
//
// MODULE  : CPP_SERVERSHELL_DE.H
//
// PURPOSE : C++ DE server shell class definition
//
// CREATED : 9/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __CPP_SERVERSHELL_DE_H__
#define __CPP_SERVERSHELL_DE_H__

#include <stdlib.h>
#include "servershell_de.h"

// Forward declarations...

class BaseClass;
class ServerDE;

/////////////////////////////////////////////////////////////////////
// C++ ServerShellDE interface.  Derive your server object from this.
/////////////////////////////////////////////////////////////////////

class CServerShellDE
{
	public :

		CServerShellDE();
		virtual ~CServerShellDE() {}

		ServerDE* GetServerDE() const;

	protected :  // These methods should be over-written

		virtual DRESULT	ServerAppMessageFn(char *pMsg) {return LT_OK;}

		// Notification when new clients come in.
		// You must create an object to represent the client.
		// It uses the object's position to determine what the client can see.
		virtual void	OnAddClient(HCLIENT hClient) {}
		virtual void	OnRemoveClient(HCLIENT hClient) {}

		virtual LPBASECLASS	OnClientEnterWorld(HCLIENT hClient, void *pClientData, DDWORD clientDataLen)=0;
		virtual void		OnClientExitWorld(HCLIENT hClient) {}

		// Called before and after you switch worlds.
		virtual void	PreStartWorld(DBOOL bSwitchingWorlds) {}
		virtual void	PostStartWorld() {}		

		// Incoming message notification.
		virtual void OnMessage(HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage) {}
		virtual void OnObjectMessage(LPBASECLASS pSender, DDWORD messageID, HMESSAGEREAD hMessage) {}		

		// Command notification.
		virtual void	OnCommandOn(HCLIENT hClient, int command) {}
		virtual void	OnCommandOff(HCLIENT hClient, int command) {}

		// Update loop callback.. do whatever you like in here.
		// Time since the last Update() call is passed in.
		virtual void	Update(DFLOAT timeElapsed) {}

		virtual void	OnPlaybackFinish() {}

		virtual void	CacheFiles() {}

		virtual void	SRand() {srand(123);}


	private :  // Data members

		// VERY Important that this first data member.  Pointers to this class and
		// pointers to CServerShellDE::m_serverShell MUST BE the same. 
		ServerShellDE	m_serverShell;  


	private :

		// The following static functions are called by the server, and should
		// NOT be called directly.
		static DRESULT _ServerAppMessageFn(struct ServerShellDE_t *pShell, char *pMsg);
		
		static void _OnAddClient(struct ServerShellDE_t *pShell, HCLIENT hClient);
		static void _OnRemoveClient(struct ServerShellDE_t *pShell, HCLIENT hClient);

		static LPBASECLASS _OnClientEnterWorld(struct ServerShellDE_t *pShell, HCLIENT hClient, void *pClientData, DDWORD clientDataLen);
		static void _OnClientExitWorld(struct ServerShellDE_t *pShell, HCLIENT hClient);

		static void _PreStartWorld(struct ServerShellDE_t *pShell, DBOOL bSwitchingWorlds);
		static void _PostStartWorld(struct ServerShellDE_t *pShell);
		static void _OnMessage(struct ServerShellDE_t *pShell, HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage);
		static void	_OnObjectMessage(struct ServerShellDE_t *pShell, LPBASECLASS pSender, DDWORD messageID, HMESSAGEREAD hMessage);
		static void _OnCommandOn(struct ServerShellDE_t *pShell, HCLIENT hClient, int command);
		static void _OnCommandOff(struct ServerShellDE_t *pShell, HCLIENT hClient, int command);
		static void _Update(struct ServerShellDE_t *pShell, DFLOAT timeElapsed);
		static void	_OnPlaybackFinish(struct ServerShellDE_t *pShell);
		static void	_CacheFiles(struct ServerShellDE_t *pShell);
		static void _SRand(struct ServerShellDE_t *pShell);
		
};


// Inlines...

INLINE_FN CServerShellDE::CServerShellDE()
{
	// Set up ServerShellDE function pointers...
	m_serverShell.ServerAppMessageFn = _ServerAppMessageFn;
	m_serverShell.OnAddClient		= _OnAddClient;
	m_serverShell.OnRemoveClient	= _OnRemoveClient;
	m_serverShell.OnClientEnterWorld = _OnClientEnterWorld;
	m_serverShell.OnClientExitWorld = _OnClientExitWorld;
	m_serverShell.PreStartWorld		= _PreStartWorld;
	m_serverShell.PostStartWorld	= _PostStartWorld;
	m_serverShell.OnMessage			= _OnMessage;
	m_serverShell.OnObjectMessage	= _OnObjectMessage;
	m_serverShell.OnCommandOn		= _OnCommandOn;
	m_serverShell.OnCommandOff		= _OnCommandOff;
	m_serverShell.Update			= _Update;
	m_serverShell.OnPlaybackFinish	= _OnPlaybackFinish;
	m_serverShell.CacheFiles		= _CacheFiles;
	m_serverShell.SRand				= _SRand;
}


INLINE_FN CServerDE* CServerShellDE::GetServerDE() const
{ 
	return (CServerDE*)g_pServerDE;
}


INLINE_FN DRESULT CServerShellDE::_ServerAppMessageFn(struct ServerShellDE_t *pShell, char *pMsg)
{
	return ((CServerShellDE*)pShell)->ServerAppMessageFn(pMsg);
}


INLINE_FN void CServerShellDE::_OnAddClient(struct ServerShellDE_t *pShell, HCLIENT hClient)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnAddClient(hClient);
	}
}

INLINE_FN void CServerShellDE::_OnRemoveClient(struct ServerShellDE_t *pShell, HCLIENT hClient)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnRemoveClient(hClient);
	}
}

INLINE_FN LPBASECLASS CServerShellDE::_OnClientEnterWorld(struct ServerShellDE_t *pShell, HCLIENT hClient, void *pClientData, DDWORD clientDataLen)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		return (LPBASECLASS)pCSShell->OnClientEnterWorld(hClient, pClientData, clientDataLen);
	}

	return NULL;
}

INLINE_FN void CServerShellDE::_OnClientExitWorld(struct ServerShellDE_t *pShell, HCLIENT hClient)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnClientExitWorld(hClient);
	}
}

INLINE_FN void CServerShellDE::_PreStartWorld(struct ServerShellDE_t *pShell, DBOOL bSwitchingWorlds)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->PreStartWorld(bSwitchingWorlds);
	}
}

INLINE_FN void CServerShellDE::_PostStartWorld(struct ServerShellDE_t *pShell)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->PostStartWorld();
	}
}

INLINE_FN void CServerShellDE::_OnMessage(struct ServerShellDE_t *pShell, HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnMessage(hSender, messageID, hMessage);
	}
}

INLINE_FN void CServerShellDE::_OnObjectMessage(struct ServerShellDE_t *pShell, LPBASECLASS pSender, DDWORD messageID, HMESSAGEREAD hMessage)
{
	if(pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnObjectMessage(pSender, messageID, hMessage);
	}
}

INLINE_FN void CServerShellDE::_OnCommandOn(struct ServerShellDE_t *pShell, HCLIENT hClient, int command)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnCommandOn(hClient, command);
	}
}

INLINE_FN void CServerShellDE::_OnCommandOff(struct ServerShellDE_t *pShell, HCLIENT hClient, int command)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnCommandOff(hClient, command);
	}
}

INLINE_FN void CServerShellDE::_Update(struct ServerShellDE_t *pShell, DFLOAT timeElapsed)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->Update(timeElapsed);
	}
}

INLINE_FN void CServerShellDE::_OnPlaybackFinish(struct ServerShellDE_t *pShell)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->OnPlaybackFinish();
	}
}

INLINE_FN void CServerShellDE::_SRand(struct ServerShellDE_t *pShell)
{
	if (pShell)
	{
		CServerShellDE *pCSShell = (CServerShellDE*)pShell;
		pCSShell->SRand();
	}
}

INLINE_FN void CServerShellDE::_CacheFiles(struct ServerShellDE_t *pShell)
{
	((CServerShellDE*)pShell)->CacheFiles();
}


#endif  // __CPP_SERVERSHELL_DE_H__


