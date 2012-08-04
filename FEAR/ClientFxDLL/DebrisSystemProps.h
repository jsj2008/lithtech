//--------------------------------------------------------------------------
// DebrisSystemProps.h
//
// Provides the definition for the debris system properties. This handles 
// defining the properties for FXEdit, loading them in, and validating them.
//
//--------------------------------------------------------------------------

#ifndef __DEBRISSYSTEMPROPS_H__
#define __DEBRISSYSTEMPROPS_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __CLIENTFXPROP_H__
#	include "ClientFxProp.h"
#endif

#ifndef __ILTPHYSICSSIM_H__
#	include "iltphysicssim.h"
#endif

//The different types of emission primitives that are supported
enum EDebrisEmission
{
	eDebrisEmission_Sphere,
	eDebrisEmission_Point,
	eDebrisEmission_Cone,
	eDebrisEmission_Cylinder,
	eDebrisEmission_Box,
};

//The different types of velocity setups
enum EDebrisVelocity
{
	eDebrisVelocity_Random,
	eDebrisVelocity_Center,
};

//The different types of orientation setups
enum EDebrisOrientation
{
	eDebrisOrientation_Random,
	eDebrisOrientation_Position,
	eDebrisOrientation_Parent,
	eDebrisOrientation_Velocity,
};

//The different types of type setup
enum EDebrisType
{
	eDebrisType_Random,
	eDebrisType_Sequential,
};

//The different types of shapes that can be used for a physics piece
enum EDebrisShape
{
	eDebrisShape_Sphere,
	eDebrisShape_Box,
	eDebrisShape_Capsule,
};

//Properties for the particle system
class CDebrisSystemProps : 
	public CBaseFXProps
{
public:

	CDebrisSystemProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//Emission parameters
	uint16				m_nMinDebris;				// The range of debris pieces to create
	uint16				m_nMaxDebris;				// --

	LTEnum<uint8, EDebrisEmission>		m_eEmissionType;	// The emission method
	LTEnum<uint8, EDebrisOrientation>	m_eOrientationType;	// The type of orientation to use
	LTEnum<uint8, EDebrisVelocity>		m_eVelocityType;	// The method of generation for the velocity
	LTEnum<uint8, EDebrisType>			m_eTypeSelection;	// The type of debris used

	//Position emission parameters
	LTVector			m_vEmissionDims;
	float				m_fMinRadius;				// Radius range for emission primitives that use radii
	float				m_fMaxRadius;				// --
	LTVector			m_vEmissionDir;				// direction of the emission
	LTVector			m_vEmissionPerp1;			// These two are perpendicular to the emission dir
	LTVector			m_vEmissionPerp2;			// to form a plane

	//Linear velocity emission parameters
	LTVector			m_vMinLinearVelocity;		// The range that the velocity can be generated within for each axis 
	LTVector			m_vMaxLinearVelocity;		// --

	//Angular velocity emission parameters
	LTVector			m_vMinAngularVelocity;		// The range that the angular velocity can be generated within, in rad/s
	LTVector			m_vMaxAngularVelocity;		// --

	//collision parameters
	float				m_fMinCollisionForce;		// The minimum force required to trigger a collision
	const char*			m_pszCollisionEffect;		// The effect to trigger in response to a collision

	//the type information for each debris type
	struct STypeInfo
	{
		STypeInfo();
		~STypeInfo();

		//called after all the properties have been loaded to validate them
		bool			PostLoadProperties();

        const char*		m_pszEffect;				// The effect name to use for this type
		EDebrisShape	m_eShape;					// The type of shape to use for this debris type
		LTVector		m_vShapeDims;				// The dimensions to use for this shape
		float			m_fMassKg;					// The mass of this type in Kg
		float			m_fDensityG;				// The density of this type in Grams per Cubic Cm
		float			m_fFriction;				// The friction for this type
		float			m_fCOR;						// The coefficient of restitution for this type
		HPHYSICSSHAPE	m_hShape;					// The actual physics shape associated with this type (can be invalid)
	};

	enum				{ k_nMaxDebrisTypes	= 5 };
	STypeInfo			m_Types[k_nMaxDebrisTypes];	// The information for each type
	uint32				m_nNumTypes;				// The number of types filled in
};

#endif
