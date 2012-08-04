// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeEffect.cpp
//
// PURPOSE : Volume effect implementation:
//           - base class for a volume based effects
//           - axis-aligned volume
//
// CREATED : 1/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "VolumeEffect.h"
#include "DEditColors.h"

LINKFROM_MODULE( VolumeEffect );

BEGIN_CLASS(VolumeEffect)
	ADD_DEDIT_COLOR( VolumeEffect )
	ADD_VECTORPROP_VAL_FLAG(Dims, 128.0f, 128.0f, 128.0f, PF_DIMS, "The Dims define the volume an effect takes place in. It is an area defined in WorldEdit units as a box around the center of the effect object.")
END_CLASS(VolumeEffect, GameBase, "VolumeEffect is used solely as the base for other volume based effects." )


CMDMGR_BEGIN_REGISTER_CLASS( VolumeEffect )
CMDMGR_END_REGISTER_CLASS( VolumeEffect, GameBase )


VolumeEffect::VolumeEffect() : GameBase(OT_NORMAL)
{
	m_vDims.Init( 128.0f, 128.0f, 128.0f );
}

VolumeEffect::~VolumeEffect()
{
}

uint32 VolumeEffect::EngineMessageFn( uint32 messageID, void* pData, float fData )
{
	switch( messageID )
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pOCS = (ObjectCreateStruct*)pData;
			if( !pOCS )
				break;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProp( pOCS );
			}

			// Send full position resolution to make sure things line up correctly.
			pOCS->m_Flags |= FLAG_FULLPOSITIONRES;
		}
		break;

	case MID_INITIALUPDATE:
		InitialUpdate();
		break;

	case MID_UPDATE:
		break;

	case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pData );
		}
		break;

	case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pData );
		}
		break;

	case MID_TOUCHNOTIFY:
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


bool VolumeEffect::ReadProp( ObjectCreateStruct* pOCS )
{
	if( !pOCS )
		return false;

	const GenericPropList *pProps = &pOCS->m_cProperties;
	m_vDims = pProps->GetVector( "Dims", m_vDims );
	return true;
}


void VolumeEffect::InitialUpdate( void )
{
	SetNextUpdate( UPDATE_NEVER );
}

void VolumeEffect::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	SAVE_VECTOR( m_vDims );
}

void VolumeEffect::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	LOAD_VECTOR( m_vDims );
}
