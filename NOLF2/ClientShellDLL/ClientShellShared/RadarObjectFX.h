// ----------------------------------------------------------------------- //
//
// MODULE  : RadarObjectFX.h
//
// PURPOSE : RadarObject special fx class - Definition
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RADAR_OBJECT_FX_H__
#define __RADAR_OBJECT_FX_H__

//
// Includes...
//

	#include "SpecialFX.h"


class CRadarObjectFX : public CSpecialFX
{
	public: // Methods...

		CRadarObjectFX();
		~CRadarObjectFX();


		virtual LTBOOL	Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual LTBOOL	OnServerMessage( ILTMessage_Read *pMsg );
		
		virtual uint32	GetSFXID() { return SFX_RADAROBJECT_ID; }

        virtual LTBOOL Update() { return !m_bWantRemove; }

	protected: // Members...

		RADAROBJCREATESTRUCT m_cs;
};
	

#endif //__RADAR_OBJECT_FX_H__