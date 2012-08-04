// ----------------------------------------------------------------------- //
//
// MODULE  : CPP_CLIENTSHELL_DE.H
//
// PURPOSE : C++ DE client shell class definition
//
// CREATED : 9/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __CPP_CLIENTSHELL_DE_H__
#define __CPP_CLIENTSHELL_DE_H__


#include "clientshell_de.h"



/////////////////////////////////////////////////////////////////////
// C++ ClientShell interface. This class contains the virtual methods 
// that DirectEngine will call to notify you of things.
/////////////////////////////////////////////////////////////////////

// See ClientShellDE.h for function docs.
class CClientShellDE
{
	public :

		CClientShellDE();
		virtual ~CClientShellDE() {}

		CClientDE* GetClientDE() const;


	protected :

		virtual void OnCommandOn(int command) {}
		virtual void OnCommandOff(int command) {}

		virtual void OnKeyDown(int key, int rep) {}
		virtual void OnKeyUp(int key) {}

		virtual void OnMessage(DBYTE messageID, HMESSAGEREAD hMessage) {}

		virtual void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs) {}

		virtual void PreLoadWorld(char *pWorldName) {}
		virtual void OnEnterWorld() {}
		virtual void OnExitWorld() {}

		virtual void SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage) {}

		virtual void OnObjectRemove(HLOCALOBJ hObj) {}

		virtual void PreUpdate() {}
		virtual void Update() {}
		virtual void PostUpdate() {}

		virtual DRESULT OnObjectMove(HLOCALOBJ hObj, DBOOL bTeleport, DVector *pNewPos) {return LT_OK;}
		virtual DRESULT	OnObjectRotate(HLOCALOBJ hObj, DBOOL bTeleport, DRotation *pNewRot) {return LT_OK;}

		virtual DRESULT OnEngineInitialized(struct RMode_t *pMode, DGUID *pAppGuid) {return LT_ERROR;}
		virtual void OnEngineTerm() {}

		virtual void OnEvent( DDWORD dwEventID, DDWORD dwParam ) {}

		virtual DRESULT	OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag) {return LT_OK;}

		virtual void	SRand() {srand(123);}
		virtual void	DemoSerialize(DStream *pStream, DBOOL bLoad) {}

	protected :  // Data members

		// VERY Important that this is the first data member.  Pointers to this 
		// class and pointers to ClientShellDE::m_clientShell MUST BE the same. 		
		ClientShellDE m_clientShell;
	

	private :

		// The following static functions are called by the server, and should
		// NOT be called directly.
		static void	_OnCommandOn(LPCLIENTSHELLDE pShell, int command);
		static void	_OnCommandOff(LPCLIENTSHELLDE pShell, int command);
		static void	_OnKeyDown(LPCLIENTSHELLDE pShell, int key, int rep);
		static void	_OnKeyUp(LPCLIENTSHELLDE pShell, int key);
		static void	_OnMessage(LPCLIENTSHELLDE pShell, DBYTE messageID, HMESSAGEREAD hMessage);
		static void _OnModelKey(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, ArgList *pArgs);
		static void _PreLoadWorld(LPCLIENTSHELLDE pShell, char *pWorldName);
		static void	_OnEnterWorld(LPCLIENTSHELLDE pShell);
		static void	_OnExitWorld(LPCLIENTSHELLDE pShell);
		static void _SpecialEffectNotify(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, HMESSAGEREAD hMessage);
		static void _OnObjectRemove(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj);
		static void	_PreUpdate(LPCLIENTSHELLDE pShell);
		static void	_Update(LPCLIENTSHELLDE pShell);
		static void	_PostUpdate(LPCLIENTSHELLDE pShell);
		static DRESULT _OnObjectMove(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DVector *pNewPos);
		static DRESULT _OnObjectRotate(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DRotation *pNewRot);
		static DRESULT _OnEngineInitialized(LPCLIENTSHELLDE pShell, struct RMode_t *pMode, DGUID *pAppGuid);
		static void _OnEngineTerm(LPCLIENTSHELLDE pShell);
		static void	_OnEvent(LPCLIENTSHELLDE pShell, DDWORD dwEventID, DDWORD dwParam);
		static DRESULT _OnTouchNotify(LPCLIENTSHELLDE pShell, HOBJECT hMain, 
			CollisionInfo *pInfo, float forceMag);
		static void _SRand(LPCLIENTSHELLDE pShell);
		static void	_DemoSerialize(LPCLIENTSHELLDE pShell, DStream *pStream, DBOOL bLoad);
}; 
	
// Inlines...

INLINE_FN CClientShellDE::CClientShellDE()
{
	// Set up ClientShellDE function pointers...
	m_clientShell.SpecialEffectNotify = _SpecialEffectNotify;
	m_clientShell.OnObjectRemove = _OnObjectRemove;
	m_clientShell.OnCommandOn	= _OnCommandOn;
	m_clientShell.OnCommandOff	= _OnCommandOff;
	m_clientShell.OnKeyDown		= _OnKeyDown;
	m_clientShell.OnKeyUp		= _OnKeyUp;
	m_clientShell.OnMessage		= _OnMessage;
	m_clientShell.PreLoadWorld	= _PreLoadWorld;
	m_clientShell.OnEnterWorld	= _OnEnterWorld;
	m_clientShell.OnModelKey	= _OnModelKey;
	m_clientShell.OnExitWorld	= _OnExitWorld;
	m_clientShell.PreUpdate		= _PreUpdate;
	m_clientShell.Update		= _Update;
	m_clientShell.PostUpdate	= _PostUpdate;
	m_clientShell.OnObjectMove	= _OnObjectMove;
	m_clientShell.OnObjectRotate = _OnObjectRotate;
	m_clientShell.OnEngineInitialized = _OnEngineInitialized;
	m_clientShell.OnEngineTerm = _OnEngineTerm;
	m_clientShell.OnEvent = _OnEvent;
	m_clientShell.OnTouchNotify = _OnTouchNotify;
	m_clientShell.SRand = _SRand;
	m_clientShell.DemoSerialize = _DemoSerialize;
}

INLINE_FN CClientDE* CClientShellDE::GetClientDE() const
{ 
	return (CClientDE*)g_pClientDE;
}

INLINE_FN void CClientShellDE::_OnCommandOn(LPCLIENTSHELLDE pShell, int command)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnCommandOn(command);
}

INLINE_FN void CClientShellDE::_OnCommandOff(LPCLIENTSHELLDE pShell, int command)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnCommandOff(command);
}

INLINE_FN void CClientShellDE::_OnKeyDown(LPCLIENTSHELLDE pShell, int key, int rep)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnKeyDown(key, rep);
}

INLINE_FN void CClientShellDE::_OnKeyUp(LPCLIENTSHELLDE pShell, int key)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnKeyUp(key);
}

INLINE_FN void CClientShellDE::_OnMessage(LPCLIENTSHELLDE pShell, DBYTE messageID, HMESSAGEREAD hMessage)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnMessage(messageID, hMessage);
}

INLINE_FN void CClientShellDE::_OnModelKey(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, ArgList *pArgList)
{
	((CClientShellDE*)pShell)->OnModelKey(hObj, pArgList);
}

INLINE_FN void CClientShellDE::_PreLoadWorld(LPCLIENTSHELLDE pShell, char *pWorldName)
{
	((CClientShellDE*)pShell)->PreLoadWorld(pWorldName);
}

INLINE_FN void CClientShellDE::_OnEnterWorld(LPCLIENTSHELLDE pShell)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnEnterWorld();
}

INLINE_FN void CClientShellDE::_OnExitWorld(LPCLIENTSHELLDE pShell)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->OnExitWorld();
}

INLINE_FN void CClientShellDE::_SpecialEffectNotify(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
	((CClientShellDE*)pShell)->SpecialEffectNotify(hObj, hMessage);
}

INLINE_FN void CClientShellDE::_OnObjectRemove(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj)
{
	((CClientShellDE*)pShell)->OnObjectRemove(hObj);
}

INLINE_FN void CClientShellDE::_PreUpdate(LPCLIENTSHELLDE pShell)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->PreUpdate();
}

INLINE_FN void CClientShellDE::_Update(LPCLIENTSHELLDE pShell)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->Update();
}

INLINE_FN void CClientShellDE::_PostUpdate(LPCLIENTSHELLDE pShell)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	pCShell->PostUpdate();
}

INLINE_FN DRESULT CClientShellDE::_OnObjectMove(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DVector *pNewPos)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	return pCShell->OnObjectMove(hObj, bTeleport, pNewPos);
}

INLINE_FN DRESULT CClientShellDE::_OnObjectRotate(LPCLIENTSHELLDE pShell, HLOCALOBJ hObj, DBOOL bTeleport, DRotation *pNewRot)
{
	CClientShellDE *pCShell = (CClientShellDE*)pShell;
	return pCShell->OnObjectRotate(hObj, bTeleport, pNewRot);
}

INLINE_FN DRESULT CClientShellDE::_OnEngineInitialized(LPCLIENTSHELLDE pShell, struct RMode_t *pMode, DGUID *pAppGuid)
{
	return ((CClientShellDE*)pShell)->OnEngineInitialized(pMode, pAppGuid);
}

INLINE_FN void CClientShellDE::_OnEngineTerm(LPCLIENTSHELLDE pShell)
{
	((CClientShellDE*)pShell)->OnEngineTerm();
}

INLINE_FN void CClientShellDE::_OnEvent(LPCLIENTSHELLDE pShell, DDWORD dwEventID, DDWORD dwParam)
{
	((CClientShellDE*)pShell)->OnEvent( dwEventID, dwParam );
}

INLINE_FN DRESULT CClientShellDE::_OnTouchNotify(LPCLIENTSHELLDE pShell, HOBJECT hMain, 
	CollisionInfo *pInfo, float forceMag)
{
	return ((CClientShellDE*)pShell)->OnTouchNotify(hMain, pInfo, forceMag);
}

INLINE_FN void CClientShellDE::_SRand(LPCLIENTSHELLDE pShell)
{
	((CClientShellDE*)pShell)->SRand();
}

INLINE_FN void CClientShellDE::_DemoSerialize(LPCLIENTSHELLDE pShell, DStream *pStream, DBOOL bLoad)
{
	((CClientShellDE*)pShell)->DemoSerialize(pStream, bLoad);
}

#endif  // __CPP_CLIENTSHELL_DE_H__

