// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVehicle.h"
#include "AIVehicleState.h"

BEGIN_CLASS(CAIVehicle)

	ADD_VEHICLEATTACHMENTS_AGGREGATE()

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAIVehicle, CAI, NULL, NULL, CF_HIDDEN, CAIVehiclePlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicle::CAIVehicle()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAIVehicle::CAIVehicle() : CAI()
{
    m_pVehicleState = LTNULL;
	m_pAnimator = &m_Animator;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicle::~CAIVehicle()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIVehicle::~CAIVehicle()
{
	if ( m_pVehicleState )
	{
		FACTORY_DELETE(m_pVehicleState);
        m_pVehicleState = LTNULL;
        m_pState = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicle::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CAIVehicle::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CVehicleAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_VEHICLE));
	}
}