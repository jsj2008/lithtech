// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicSectorVolumeFX.h
//
// PURPOSE : DynamicSectorVolume special fx class - Definition
//
// CREATED : 4/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DYNAMIC_SECTOR_VOLUME_FX_H__
#define __DYNAMIC_SECTOR_VOLUME_FX_H__

//
// Includes...
//

	#include "SpecialFX.h"


class CDynamicSectorVolumeFX : public CSpecialFX
{
	public : // Methods...

		CDynamicSectorVolumeFX();
		~CDynamicSectorVolumeFX();

		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		virtual uint32 GetSFXID() { return SFX_DYNAMIC_SECTOR_ID; }

		//handle enabling and disabling the rendering settings associated with this volume
		void Enable(bool bEnable);

	protected : // Members...

		void EnableSectors(bool bEnable);

		enum	constants { kMaxSectorIds		= 10};

		bool	m_bEnabled;

		uint8	m_nNumSectorIds;
		uint32	m_nSectorIds[kMaxSectorIds];
};

#endif // __DYNAMIC_SECTOR_VOLUME_FX_H__
