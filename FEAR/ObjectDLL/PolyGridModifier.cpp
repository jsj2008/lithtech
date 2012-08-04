// ----------------------------------------------------------------------- //
//
// MODULE  :PolyGridModifier.cpp
//
// PURPOSE : Implementation for PolyGridModifier class
//
// (c) 2002-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PolyGridModifier.h"
#include "iltserver.h"

LINKFROM_MODULE( PolyGridModifier );

BEGIN_CLASS(PolyGridModifier)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS, "Specifies the size of the area that will be effected by this modifier. Note that the Y value is not important, and only X and Z are used.")
	ADD_REALPROP(AccelAmount, 15.0f, "Specifies how far this polygrid will be displaced at a given position. Note that this value is scaled by time to make it frame rate independant.")
	ADD_LONGINTPROP(NumAccelPoints, 10, "For every surface update, it will pick this many points to displace AccelAmount units.")
	ADD_BOOLPROP(StartEnabled, true, "Specifies whether or not this modifier should be enabled at the starting of a level.")
END_CLASS_FLAGS(PolyGridModifier, GameBase, 0, "PolyGridModifiers are used in conjunction with PolyGrids to create disturbances in the PolyGrid object." )

CMDMGR_BEGIN_REGISTER_CLASS( PolyGridModifier )
CMDMGR_END_REGISTER_CLASS( PolyGridModifier, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::PolyGridModifier()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PolyGridModifier::PolyGridModifier() : GameBase(OT_NORMAL)
{
	m_vDims.Init();
	m_fAccelAmount		= 15.0f;
	m_nNumAccelPoints	= 10;
	m_bStartEnabled		= true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Key::~Key()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

PolyGridModifier::~PolyGridModifier()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PolyGridModifier::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
            InitialUpdate((LTVector *)pData);
			break;
		}

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool PolyGridModifier::ReadProp(const GenericPropList *pProps)
{
	m_vDims				= pProps->GetVector("Dims", m_vDims);
	m_fAccelAmount		= pProps->GetReal("AccelAmount", m_fAccelAmount);
	m_nNumAccelPoints	= pProps->GetLongInt("NumAccelPoints", m_nNumAccelPoints);
	m_bStartEnabled		= pProps->GetBool("StartEnabled", m_bStartEnabled);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool PolyGridModifier::InitialUpdate(LTVector* pMovement)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return false;

	//disable updating of this object
	SetNextUpdate( UPDATE_NEVER );

    return true;
}
