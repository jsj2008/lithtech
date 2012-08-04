// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarkerFX.h
//
// PURPOSE : NavMarkerFX - Definition
//
// CREATED : 11/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NAVMARKER_FX_H__
#define __NAVMARKER_FX_H__

#include "SpecialFX.h"
#include "idatabasemgr.h"
#include "TeamMgr.h"
#include "SharedFXStructs.h"

class CHUDNavMarker;
class HUDNavMarker_create;

class CNavMarkerFX : public CSpecialFX
{
	public :
		CNavMarkerFX();
		virtual ~CNavMarkerFX();

        virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual bool CreateObject(ILTClient* pClientDE);
        virtual bool Update();

		virtual bool OnServerMessage( ILTMessage_Read *pMsg ); 

		virtual uint32 GetSFXID() { return SFX_NAVMARKER_ID; }

		virtual void	UpdateClientFX();

		virtual void	SetTarget(HLOCALOBJ hTarget);
		
		void			SetTeamId( uint8 nTeamId );

		virtual bool	IsBroadcast() const {return m_cs.m_bBroadcast; }
		virtual uint8	GetClientID() const {return m_cs.m_nClientID;}


	private :

		void	UpdateData();

		CHUDNavMarker *	m_pHUDItem;
		LTObjRef			m_hTarget;	
		NAVMARKERCREATESTRUCT m_cs;

		// ClientFX played while operating a turret...
		CClientFXLink	m_fxLoop;

		StopWatchTimer		m_LifeTimeTimer;
};

#endif // __NavMarker_FX_H__