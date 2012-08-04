// ----------------------------------------------------------------------- //
//
// MODULE  : Key.cpp
//
// PURPOSE : Key implementation for Keyframer class
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyGridModifier.h"
#include "iltserver.h"

LINKFROM_MODULE( PolyGridModifier );

#pragma force_active on
BEGIN_CLASS(PolyGridModifier)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS)
	ADD_REALPROP(AccelAmount, 15.0f)
	ADD_LONGINTPROP(NumAccelPoints, 10)
	ADD_BOOLPROP(StartEnabled, LTTRUE)
END_CLASS_DEFAULT_FLAGS(PolyGridModifier, BaseClass, NULL, NULL, 0)
#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::PolyGridModifier()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PolyGridModifier::PolyGridModifier() : BaseClass(OT_NORMAL)
{
	m_vDims.Init();
	m_fAccelAmount		= 15.0f;
	m_nNumAccelPoints	= 10;
	m_bStartEnabled		= LTTRUE;
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

uint32 PolyGridModifier::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL PolyGridModifier::ReadProp(ObjectCreateStruct *)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	pServerDE->GetPropVector("Dims", &m_vDims);

	pServerDE->GetPropReal("AccelAmount", &m_fAccelAmount);

	int32 nVal;
	if(pServerDE->GetPropLongInt("NumAccelPoints", &nVal) == LT_OK)
	{
		m_nNumAccelPoints = nVal;
	}

	bool bFlag;
	pServerDE->GetPropBool("StartEnabled", &bFlag);
	m_bStartEnabled = (bFlag ? LTTRUE : LTFALSE);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGridModifier::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL PolyGridModifier::InitialUpdate(LTVector* pMovement)
{
    ILTServer* pServerDE = GetServerDE();
    if (!pServerDE) return LTFALSE;

	//disable updating of this object
	SetNextUpdate (m_hObject, UPDATE_NEVER);

    return LTTRUE;
}