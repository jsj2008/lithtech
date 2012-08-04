// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicOccluderVolumeFX.h
//
// PURPOSE : DynamicOccluderVolume special fx class - Definition
//
// CREATED : 4/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DYNAMIC_OCCLUDER_VOLUME_FX_H__
#define __DYNAMIC_OCCLUDER_VOLUME_FX_H__

//
// Includes...
//

	#include "SpecialFX.h"


class CDynamicOccluderVolumeFX : public CSpecialFX
{
	public : // Methods...

		CDynamicOccluderVolumeFX();
		~CDynamicOccluderVolumeFX();

		virtual LTBOOL Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
		virtual LTBOOL OnServerMessage( ILTMessage_Read *pMsg );

		virtual uint32 GetSFXID() { return SFX_DYNAMIC_OCCLUDER_ID; }

		//handle enabling and disabling the rendering settings associated with this volume
		void Enable(bool bEnable);

	protected : // Members...

		void EnableOccluders(bool bEnable);
		void EnableRenderGroups(bool bEnable);

		enum	constants { kMaxOccluderIds		= 10,
							kMaxRenderGroups	= 20};

		bool	m_bEnabled;

		uint8	m_nNumOccluderIds;
		uint32	m_nOccluderIds[kMaxOccluderIds];

		uint8	m_nNumRenderGroups;
		uint8	m_nRenderGroups[kMaxRenderGroups];
};

#endif // __DYNAMIC_OCCLUDER_VOLUME_FX_H__