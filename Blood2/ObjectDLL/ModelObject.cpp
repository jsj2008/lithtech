// ----------------------------------------------------------------------- //
//
// MODULE  : ModelObject.cpp
//
// PURPOSE : ModelObject class - implementation
//
// CREATED : 12/31/97
//
// ----------------------------------------------------------------------- //

#include "ModelObject.h"
#include "cpp_server_de.h"
#include "ObjectUtilities.h"
#include "SharedDefs.h"


BEGIN_CLASS(CModelObject)
END_CLASS_DEFAULT_FLAGS(CModelObject, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::CModelObject
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CModelObject::CModelObject() : BaseClass(OT_MODEL)
{
	m_bFirstUpdate = DTRUE;

	m_bRotate = DFALSE;
	m_bStopRotateOnGround = DTRUE;
	m_fXRotVel = 0.0;
	m_fYRotVel = 0.0;
	m_fZRotVel = 0.0;
	m_fLastTime = 0.0f;
	m_fPitch = 0.0f;
	m_fYaw = 0.0f;
	m_fRoll = 0.0f;

	VEC_INIT(m_vBaseDims);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::~CModelObject
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CModelObject::~CModelObject()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CModelObject::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if( !Update(( DVector * )pData))
			{
				g_pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = ( ObjectCreateStruct * )pData;
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate(( DVector * )pData );
			break;
		}

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, lData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::Setup
//
//	PURPOSE:	Set up a ModelObject with the information needed
//
// ----------------------------------------------------------------------- //

void CModelObject::Setup( DFLOAT fLifeTime, DVector *pvRotationPeriods,
						 DBOOL bStopRotateOnGround, DBOOL bRandomizeRotation)
{
	DFLOAT fUpdateTime;

	// Set the life of the object...
	// Numbers less than and equal to zero means it lasts forever...
	m_fLifeTime = fLifeTime;

	if (bRandomizeRotation)
	{
		m_fPitch	= g_pServerDE->Random(0.0f, PIx2);
		m_fYaw		= g_pServerDE->Random(0.0f, PIx2);
		m_fRoll		= g_pServerDE->Random(0.0f, PIx2);
	}

	if( pvRotationPeriods )
	{
		float mag;
		mag = VEC_MAGSQR( *pvRotationPeriods );
		if( mag > 0.001 )
		{
			m_bRotate = DTRUE;
			m_bStopRotateOnGround = bStopRotateOnGround;
			
			if( pvRotationPeriods->x < -0.001 || 0.001f < pvRotationPeriods->x )
				m_fXRotVel = MATH_CIRCLE / pvRotationPeriods->x;
			if( pvRotationPeriods->y < -0.001 || 0.001f < pvRotationPeriods->y )
				m_fYRotVel = MATH_CIRCLE / pvRotationPeriods->y;
			if( pvRotationPeriods->z < -0.001 || 0.001f < pvRotationPeriods->z )
				m_fZRotVel = MATH_CIRCLE / pvRotationPeriods->z;

		}
	}

	// Only update if we need to...
	if( m_bRotate )
		fUpdateTime = 0.01f;
	else if( m_fLifeTime > 0.0f )
		fUpdateTime = m_fLifeTime;
	else
		fUpdateTime = 0.0f;

	g_pServerDE->SetNextUpdate( m_hObject, fUpdateTime );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::UpdateRotation
//
//	PURPOSE:	Update sprite scaling
//
// ----------------------------------------------------------------------- //

void CModelObject::UpdateRotation()
{
	DFLOAT fTime = g_pServerDE->GetTime();
	DFLOAT fDeltaTime = fTime - m_fLastTime;

	DRotation rRot;
	g_pServerDE->GetObjectRotation( m_hObject, &rRot );

	if( m_fXRotVel < 0.0f || 0.0f < m_fXRotVel )
		m_fPitch += m_fXRotVel * fDeltaTime;
	if( m_fYRotVel < 0.0f || 0.0f < m_fYRotVel )
		m_fYaw += m_fYRotVel * fDeltaTime;
	if( m_fZRotVel < 0.0f || 0.0f < m_fZRotVel )
		m_fRoll += m_fZRotVel * fDeltaTime;

	g_pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
	g_pServerDE->SetObjectRotation(m_hObject, &rRot);	

	m_fLastTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL CModelObject::InitialUpdate(DVector*)
{
	g_pServerDE->GetObjectDims(m_hObject, &m_vBaseDims);

	g_pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.01);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::FirstUpdate
//
//	PURPOSE:	Do First updating
//
// ----------------------------------------------------------------------- //

void CModelObject::FirstUpdate()
{
	m_fStartTime = g_pServerDE->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelObject::Update
//
//	PURPOSE:	Update the ModelObject
//
// ----------------------------------------------------------------------- //

DBOOL CModelObject::Update(DVector* pMovement)
{
	CollisionInfo collisionInfo;

	if( m_bFirstUpdate )
	{
		FirstUpdate();
		m_bFirstUpdate = DFALSE;
	}

	if( m_bRotate ) 
	{
		if( m_bStopRotateOnGround )
		{
			g_pServerDE->GetStandingOn( m_hObject, &collisionInfo );
			
			if( collisionInfo.m_hObject) 
				m_bRotate = DFALSE;
			else
				UpdateRotation();
		}
		else
			UpdateRotation();
	}

	DFLOAT fTime = g_pServerDE->GetTime();

	// Only update if we need to...
	if( m_bRotate )
		g_pServerDE->SetNextUpdate( m_hObject, 0.01f );
	else
		g_pServerDE->SetNextUpdate( m_hObject, m_fLifeTime - ( fTime - m_fStartTime ));

	if( m_fLifeTime > 0.0f )
		return (fTime < m_fStartTime + m_fLifeTime );
	else
		return DTRUE;
}


