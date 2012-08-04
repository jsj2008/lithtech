#include "stdafx.h"
#include "DebrisSystemProps.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemProps::CDebrisSystemProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CDebrisSystemProps::CDebrisSystemProps() : 
	m_nMinDebris(0),
	m_nMaxDebris(0),

	m_eEmissionType(eDebrisEmission_Point),
	m_eOrientationType(eDebrisOrientation_Position),
	m_fMinRadius(0.0f),
	m_fMaxRadius(0.0f),
	m_vEmissionDims(LTVector::GetIdentity()),
	m_vEmissionDir(0.0f, 1.0f, 0.0f),
	m_vEmissionPerp1(0.0f, 0.0f, 1.0f),
	m_vEmissionPerp2(1.0f, 0.0f, 0.0f),

	m_eVelocityType(eDebrisVelocity_Random),
	m_vMinLinearVelocity(LTVector::GetIdentity()),
	m_vMaxLinearVelocity(LTVector::GetIdentity()),

	m_vMinAngularVelocity(LTVector::GetIdentity()),
	m_vMaxAngularVelocity(LTVector::GetIdentity()),

	m_eTypeSelection(eDebrisType_Sequential),

	m_fMinCollisionForce(1000.0f),
	m_pszCollisionEffect(NULL)

{
}

CDebrisSystemProps::STypeInfo::STypeInfo() :
	m_pszEffect(NULL),
	m_eShape(eDebrisShape_Sphere),
	m_vShapeDims(1.0f, 1.0f, 1.0f),
	m_fFriction(0.1f),
	m_fCOR(0.3f),
	m_fMassKg(1.0f),
	m_fDensityG(1.1f),
	m_hShape(INVALID_PHYSICS_SHAPE)
{
}

CDebrisSystemProps::STypeInfo::~STypeInfo()
{
	if(m_hShape != INVALID_PHYSICS_SHAPE)
		g_pLTClient->PhysicsSim()->ReleaseShape(m_hShape);
}

//called after all the properties have been loaded to validate them
bool CDebrisSystemProps::STypeInfo::PostLoadProperties()
{
	//we now need to make a physics shape if we have a client object (we might not if we
	//are in a tool environment)
	if(g_pLTClient)
	{
		switch(m_eShape)
		{
		case eDebrisShape_Sphere:
			m_hShape = g_pLTClient->PhysicsSim()->CreateSphereShape(m_vShapeDims.x, m_fMassKg, m_fDensityG);
			break;
		case eDebrisShape_Box:
			//TODO:JO update this to use a collision radius
			m_hShape = g_pLTClient->PhysicsSim()->CreateBoxShape(m_vShapeDims, m_fMassKg, m_fDensityG, 0.0f);
			break;
		case eDebrisShape_Capsule:
			{
				//create a capsule that has it's line segment centered around the origin, aligned with
				//the Z axis
				float fHalfLength = m_vShapeDims.y * 0.5f;
				m_hShape = g_pLTClient->PhysicsSim()->CreateCapsuleShape(LTVector(0.0f, 0.0f, -fHalfLength), LTVector(0.0f, 0.0f, fHalfLength), m_vShapeDims.x, m_fMassKg, m_fDensityG);
			}
			break;
		default:
			LTERROR( "Error: Unexpected shape type encountered for a debris type");
			return false;
			break;
		};
	}

	return true;
}

//called after all properties have been loaded to allow for post processing of parameters
bool CDebrisSystemProps::PostLoadProperties()
{
	//we need to count up the number of debris types that are valid (have an effect name), 
	// and move those into a flat list, and count those up
	m_nNumTypes = 0;

	//just find each valid one, and move it to the available spot
	for(uint32 nCurrType = 0; nCurrType < k_nMaxDebrisTypes; nCurrType++)
	{
		//see if this one is valid
		if( !LTStrEmpty(m_Types[nCurrType].m_pszEffect) )
		{
			//this one is valid, so move it to a valid spot (this works because m_nNumTypes
			//is always <= nCurrType, so only invalid components will ever get stomped, which
			//we don't care about anyway)
			m_Types[m_nNumTypes] = m_Types[nCurrType];

			//and allow it to perform it's post load update
			m_Types[m_nNumTypes].PostLoadProperties();

			//and move onto the next type
			m_nNumTypes++;
		}
	}

	return CBaseFXProps::PostLoadProperties();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDebrisSystemProps::ReadProps
//
//  PURPOSE:	Read in the property values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CDebrisSystemProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//Emission parameters
	if(LTStrIEquals(pszName, "MinPieces"))
	{
		m_nMinDebris = CFxProp_Int::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MaxPieces"))
	{
		m_nMaxDebris = CFxProp_Int::Load(pStream);
	}

	//Position parameters
	else if(LTStrIEquals(pszName, "EmissionType"))
	{
		m_eEmissionType = (EDebrisEmission)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "OrientationType"))
	{
		m_eOrientationType = (EDebrisOrientation)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "EmissionDims"))
	{
		m_vEmissionDims = CFxProp_Vector::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MinRadius"))
	{
		m_fMinRadius = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MaxRadius"))
	{
		m_fMaxRadius = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "EmissionDir"))
	{
		m_vEmissionDir = CFxProp_Vector::Load(pStream);
		m_vEmissionDir.Normalize();

		// Get the perpendicular vectors to this plane
		FindPerps(m_vEmissionDir, m_vEmissionPerp1, m_vEmissionPerp2);
	}

	//Linear velocity parameters
	else if(LTStrIEquals(pszName, "VelocityType"))
	{
		m_eVelocityType = (EDebrisVelocity)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MinLinearVelocity"))
	{
		m_vMinLinearVelocity = CFxProp_Vector::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MaxLinearVelocity"))
	{
		m_vMaxLinearVelocity = CFxProp_Vector::Load(pStream);
	}

	//Angular velocity parameters
	else if(LTStrIEquals(pszName, "MinAngularVelocity"))
	{
		m_vMinAngularVelocity = CFxProp_Vector::Load(pStream) * MATH_PI / 180.0f;
	}
	else if(LTStrIEquals(pszName, "MaxAngularVelocity"))
	{
		m_vMaxAngularVelocity = CFxProp_Vector::Load(pStream) * MATH_PI / 180.0f;
	}

	//Type emission parameters
	else if(LTStrIEquals(pszName, "TypeSelection"))
	{
		m_eTypeSelection = (EDebrisType)CFxProp_Enum::Load(pStream);
	}

	//Collision parameters
	else if(LTStrIEquals(pszName, "MinCollisionForce"))
	{
		m_fMinCollisionForce = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "CollisionEffect"))
	{
		m_pszCollisionEffect = CFxProp_String::Load(pStream, pszStringTable);
	}

	//now handle the type parameters
	else
	{
		char pszPropName[128];

		for(uint32 nCurrType = 0; nCurrType < k_nMaxDebrisTypes; nCurrType++)
		{
			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Effect_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_pszEffect = CFxProp_String::Load(pStream, pszStringTable);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Shape_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_eShape = (EDebrisShape)CFxProp_Enum::Load(pStream);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "ShapeDims_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_vShapeDims = CFxProp_Vector::Load(pStream);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "MassKg_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_fMassKg = CFxProp_Float::Load(pStream);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "DensityG_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_fDensityG = CFxProp_Float::Load(pStream);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Friction_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_fFriction = CFxProp_Float::Load(pStream);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "COR_%d", nCurrType + 1);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_Types[nCurrType].m_fCOR = CFxProp_Float::Load(pStream);
				return true;
			}
		}

		//if we haven't hit anything, we need to pass it on to the base
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}	

	return true;	
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CDebrisSystemProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectEffect(m_pszCollisionEffect);

	//run through each effect we could possible create and collect their resources
	for(uint32 nCurrType = 0; nCurrType < m_nNumTypes; nCurrType++)
	{
		Collector.CollectEffect(m_Types[nCurrType].m_pszEffect);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetDebrisSystemProps
//
//  PURPOSE:	Returns a list of properties for ths effect to be used in FXEdit
//
// ----------------------------------------------------------------------- //

void fxGetDebrisSystemProps( CFastList<CEffectPropertyDesc> *pList )
{
	CEffectPropertyDesc	fxProp;

	LTVector vY(0, 1.0f, 0);
	LTVector vZero(0, 0, 0);

	// Add the generic "every effect has these" props
	AddBaseProps( pList );


	//Emission properties
	fxProp.SetupTextLine("Emission Properties");
	pList->AddTail( fxProp );

	fxProp.SetupIntMin( "MinPieces", 5, 0, eCurve_None, "The minimum number of pieces of debris to create for this debris system" );
	pList->AddTail( fxProp );

	fxProp.SetupIntMin( "MaxPieces", 5, 0, eCurve_None, "The maximum number of pieces of debris to create for this debris system" );
	pList->AddTail( fxProp );


	//Position parameters
	fxProp.SetupTextLine("Starting Position Properties");
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "EmissionType", "Sphere", "Sphere, Point, Cone, Cylinder, Box", eCurve_None, "The primitive to use for emitting the debris." );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "OrientationType", "Position", "Random, Position, Parent, Velocity", eCurve_None, "The method that should be used for determining the initial orientation of a piece of debris." );
	pList->AddTail( fxProp );

	fxProp.SetupVectorMin( "EmissionDims", vZero, 0.0f, eCurve_None, "This indicates the dimensions of the area to emit in. For box all three are used to determine the half dimensions of the box, for cylinder and cone though, the Y component is used to indicate the height." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MinRadius", 0.0f, 0.0f, eCurve_None, "The minimum radius to use for the sphere, cone, and cylinder emission types.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MaxRadius", 0.0f, 0.0f, eCurve_None, "The maximum radius to use for the sphere, cone, and cylinder emission types.");
	pList->AddTail( fxProp );

	fxProp.SetupVector( "EmissionDir", vY, eCurve_None, "The direction to aim the emission primitive. This is used for cone and cylinder to aim the direction of them." );
	pList->AddTail( fxProp );

	
	//Linear velocity parameters
	fxProp.SetupTextLine("Starting Linear Velocity Properties");
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "VelocityType", "Random", "Random, FromCenter", eCurve_None, "The method to use when determining the initial linear velocity to use for the debris pieces. Random will pick a random value within the range specified below, while FromCenter will use the direction from the center as the velocity direction, and the X component of the below values to determine the speed" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MinLinearVelocity", vZero, eCurve_None, "The minimum range for the velocities to use when randomly determining the piece velocity. This is in cm per second, and only the X is used for FromCenter" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MaxLinearVelocity", vZero, eCurve_None, "The maximum range for the velocities to use when randomly determining the piece velocity. This is in cm per second, and only the X is used for FromCenter" );
	pList->AddTail( fxProp );


	//Angular velocity parameters
	fxProp.SetupTextLine("Starting Angular Velocity Properties");
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MinAngularVelocity", vZero, eCurve_None, "The minimum range for the velocities to use when randomly determining the piece angular velocity. This is in degrees per second" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MaxAngularVelocity", vZero, eCurve_None, "The maximum range for the velocities to use when randomly determining the piece angular velocity. This is in degrees per second" );
	pList->AddTail( fxProp );


	//Type selection
	fxProp.SetupTextLine("Type Selection Properties");
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "TypeSelection", "Random", "Random, Sequential", eCurve_None, "The method to use when determining the initial linear velocity to use for the debris pieces. Random will just randomly pick a type each time, where sequential will go through the list of types in order when creating pieces" );
	pList->AddTail( fxProp );


	//Collision parameters
	fxProp.SetupTextLine("Collision Properties");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MinCollisionForce", 1000.0f, 0.0f, eCurve_None, "The minimum force needed during a collision to trigger a collision effect. This is in Newtons.");
	pList->AddTail( fxProp );

	fxProp.SetupString("CollisionEffect", "", eCurve_None, "The effect to create when a collision that exceeds the specified force is found. This can be left empty to not create any effect");
	pList->AddTail( fxProp );


	//Type parameters
	char pszPropName[128];
	for(uint32 nCurrType = 0; nCurrType < CDebrisSystemProps::k_nMaxDebrisTypes; nCurrType++)
	{
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Debris Type %d Properties", nCurrType + 1);
		fxProp.SetupTextLine(pszPropName);
		pList->AddTail( fxProp );

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Effect_%d", nCurrType + 1);
		fxProp.SetupClientFXRef(pszPropName, "", eCurve_None, "The effect to create to attach to this type. This should be left blank if the type is to be ignored.");
		pList->AddTail( fxProp );

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Shape_%d", nCurrType + 1);
		fxProp.SetupEnum(pszPropName, "Sphere", "Sphere, Box, Capsule", eCurve_None, "The type of shape that should be used to simulate this piece of debris");
		pList->AddTail( fxProp );

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "ShapeDims_%d", nCurrType + 1);
		fxProp.SetupVectorMin(pszPropName, LTVector(1.0f, 1.0f, 1.0f), 0.1f, eCurve_None, "The dimensions of the shape for this type. Only the X value is used for the radius if the shape is a sphere. For a capsule, X represents the radius of the capsule, and Y the length of the capsule.");
		pList->AddTail( fxProp );

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "MassKg_%d", nCurrType + 1);
		fxProp.SetupFloatMin(pszPropName, 1.0f, 0.1f, eCurve_None, "The mass of this debris type in kilograms.");
		pList->AddTail( fxProp );

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "DensityG_%d", nCurrType + 1);
		fxProp.SetupFloatMin(pszPropName, 1.0f, 0.1f, eCurve_None, "The density of this piece of debris measured in grams per cubic centimeter. Water is one gram per cubic centimeter.");
		pList->AddTail( fxProp );

        LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Friction_%d", nCurrType + 1);
		fxProp.SetupFloatMinMax(pszPropName, 0.3f, 0.0f, 1.0f, eCurve_None, "The friction that this object has. The higher this value, the less it will slide along other objects");
		pList->AddTail( fxProp );
		
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "COR_%d", nCurrType + 1);
		fxProp.SetupFloatMinMax(pszPropName, 0.3f, 0.0f, 1.0f, eCurve_None, "The coefficient of restitution for this object. This essentially controls how bouncy the object is. 0 indicates no bounce, 1 indicates bouncing with no energy loss");
		pList->AddTail( fxProp );
	}
}

