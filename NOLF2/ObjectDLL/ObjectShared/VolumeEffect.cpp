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

#include "stdafx.h"
#include "VolumeEffect.h"
#include "DEditColors.h"

LINKFROM_MODULE( VolumeEffect );

BEGIN_CLASS(VolumeEffect)
	ADD_DEDIT_COLOR( VolumeEffect )
	ADD_VECTORPROP_VAL_FLAG(Dims, 128.0f, 128.0f, 128.0f, PF_DIMS)
END_CLASS_DEFAULT_FLAGS(VolumeEffect, BaseClass, NULL, NULL, CF_ALWAYSLOAD)


VolumeEffect::VolumeEffect() : BaseClass(OT_NORMAL)
{
	m_vDims.Init( 128.0f, 128.0f, 128.0f );
}

VolumeEffect::~VolumeEffect()
{
}

uint32 VolumeEffect::EngineMessageFn( uint32 messageID, void* pData, LTFLOAT fData )
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
		}
		break;

	case MID_INITIALUPDATE:
		InitialUpdate();
		break;

	case MID_UPDATE:
		break;

	case MID_SAVEOBJECT:
		break;

	case MID_LOADOBJECT:
		break;

	case MID_TOUCHNOTIFY:
		break;
	}

	return BaseClass::EngineMessageFn( messageID, pData, fData );
}


bool VolumeEffect::ReadProp( ObjectCreateStruct* pOCS )
{
	if( !pOCS )
		return false;

	g_pLTServer->GetPropVector( "Dims", &m_vDims );

	return true;
}


void VolumeEffect::InitialUpdate( void )
{
	SetNextUpdate( m_hObject, UPDATE_NEVER );
}
