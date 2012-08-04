// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.cpp
//
// PURPOSE : The ParticleSystemFX object
//
// CREATED : 4/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h" 
	#include "ClientFX.h"
	#include "ParticleSystemFX.h"

//
// Angle macros...
//

#define PI				3.14159f
#define DEG_TO_RAD(x)	(((x) * PI) / 180.0f)

// Macros for accessing varous parts of the user data which holds
// 15 bits: Color key offset
// 15 bits: Scale key offset
// 1 bit: Bounce flag
// 1 bit: Splatter flag
#define KEY_INDEX_MASK				((1<<15) - 1)
#define COLOR_KEY_OFFSET			(0)
#define SCALE_KEY_OFFSET			(15)
#define BOUNCE_FLAG					(1<<30)
#define SPLAT_FLAG					(1<<31)

inline uint32	GetKeyOffset(uint32 nVal, uint32 nOffset)	
{ 
	return (nVal >> nOffset) & KEY_INDEX_MASK; 
}

inline void		SetKeyOffset(uint32& nVal, uint32 nKey, uint32 nOffset)	
{ 
	nVal &= ~(KEY_INDEX_MASK << nOffset);
	nVal |= (nKey & KEY_INDEX_MASK) << nOffset;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::CParticleSystemProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CParticleSystemProps::CParticleSystemProps() : 
	m_vMinVelocity				( 0.0f, 0.0f, 0.0f ),
	m_vMaxVelocity				( 0.0f, 0.0f, 0.0f ),
	m_vPlaneDir					( 0.0f, 0.0f, 0.0f ),
	m_nParticlesPerEmission		( 0 ),
	m_fMinLifeSpan				( 0.0f ),
	m_fMinRadius				( 0.0f ),
	m_fMaxRadius				( 0.0f ),
	m_eType						( PS_eSphere ),
	m_eVelocityType				( PSV_eRandom ),
	m_fPercentToBounce			( 0.0f ),
	m_bFlipOrder				( LTFALSE ),
	m_dwBlendMode				( 0 ),
	m_bObjectSpace				( LTFALSE ),
	m_bLight					( LTFALSE ),
	m_bRotate					( LTFALSE ),
	m_bCollideModels			( LTFALSE ),
	m_bSwarm					( LTFALSE ),
	m_vAcceleration				( 0.0f, 0.0f, 0.0f ),
	m_fFriction					( 0.0f),
	m_bInfiniteLife				( LTFALSE ),
	m_bKillOnSplat				( LTTRUE ),
	m_fPercentToSplat			( 0.0f ),
	m_fEmissionInterval			( 0.0f )
{
	m_szSplatEffect[0] = '\0';
	m_szFileName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CParticleSystemProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	LTFLOAT fGravity = 0.0f;

	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Sprite" ) )
		{
			fxProp.GetPath( m_szFileName );
		}
		else if( !_stricmp( fxProp.m_sName, "Texture" ) )
		{
			if( !m_szFileName[0] )
			{
				fxProp.GetPath( m_szFileName );
			}
		}
		else if( !_stricmp( fxProp.m_sName, "EmissionInterval" ) )
		{
			m_fEmissionInterval = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "ParticlesPerEmission" ) )
		{
			m_nParticlesPerEmission = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "GravityAcceleration" ) ) 
		{
			fGravity = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinParticleLifeSpan" ) )
		{
			m_fMinLifeSpan = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxParticleLifeSpan" ) )
		{	
			m_fMaxLifeSpan = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinRadius" ) )
		{
			m_fMinRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxRadius" ) )
		{
			m_fMaxRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "AdditionalAcceleration") )
		{
			m_vAcceleration = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "EmissionPlane" ) )
		{
			m_vPlaneDir = fxProp.GetVector();
			
			m_vPlaneDir.Norm();

			// Get the perpindicular vectors to this plane
			FindPerps(m_vPlaneDir, m_vPerp1, m_vPerp2);
		}
		else if( !_stricmp( fxProp.m_sName, "MinParticleVelocity" ) )
		{
			m_vMinVelocity = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxParticleVelocity" ) )
		{
			m_vMaxVelocity = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "Type" ) )
		{
			m_eType = (ePSType)fxProp.GetComboVal(); 
		}
		else if( !_stricmp( fxProp.m_sName, "PercentToBounce" ) )
		{
			m_fPercentToBounce = fxProp.GetFloatVal();
			m_fPercentToBounce = LTCLAMP(m_fPercentToBounce, 0.0f, 100.0f);
		}
		else if( !_stricmp( fxProp.m_sName, "FlipRenderingOrder" ) )
		{
			m_bFlipOrder = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "LightParticles" ) )
		{
			m_bLight = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "RotateParticles" ) )
		{
			m_bRotate = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "BlendMode" ) )
		{
			int	nBlendMode = fxProp.GetComboVal();

			if( nBlendMode == 1 )
			{
				m_dwBlendMode = FLAG2_ADDITIVE;
			}
			else if( nBlendMode == 2 )
			{
				m_dwBlendMode = FLAG2_MULTIPLY;
			}
		}
		else if( !_stricmp( fxProp.m_sName, "MoveParticlesWithSystem" ) )
		{
			m_bObjectSpace = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinAngularVelocity" ) )
		{
			m_fMinAngularVelocity = DEG_TO_RAD(fxProp.GetFloatVal());
		}
		else if( !_stricmp( fxProp.m_sName, "MaxAngularVelocity" ) )
		{
			m_fMaxAngularVelocity = DEG_TO_RAD(fxProp.GetFloatVal());
		}
		else if( !_stricmp( fxProp.m_sName, "CollideModels" ) )
		{
			m_bCollideModels = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Friction" ) )
		{
			m_fFriction = fxProp.GetFloatVal();
			m_fFriction = LTCLAMP(m_fFriction, 0.0f, 1.0f);
		}
		else if( !_stricmp( fxProp.m_sName, "VelocityType" ) )
		{
			m_eVelocityType = (ePSVelocityType)fxProp.GetComboVal(); 
		}
		else if( !_stricmp( fxProp.m_sName, "InfiniteLife" ) )
		{
			m_bInfiniteLife = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "SplatEffect" ) )
		{
			fxProp.GetStringVal(m_szSplatEffect);
		}
		else if( !_stricmp( fxProp.m_sName, "SplatPercent" ) )
		{
			m_fPercentToSplat = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "KillOnSplat" ) )
		{
			m_bKillOnSplat = (LTBOOL)fxProp.GetComboVal();
		}
	}

	//adjust the acceleration vector to include gravitational acceleration
	m_vAcceleration.y += fGravity;

	m_bSwarm = ( (m_vRotAdd.x != 0.0f) || (m_vRotAdd.y != 0.0f) || (m_vRotAdd.z != 0.0f) ) ? LTTRUE : LTFALSE ;

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//  PURPOSE:	Standard Constructor
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX( void )
:	CBaseFX						( CBaseFX::eParticleSystemFX ),
	m_nOptCount					( 0 ),
	m_nNumParticles				( 0 ),
	m_nNumBounceParticles		( 0 ),
	m_nNumSplatParticles		( 0 ),
	m_fVisRadius				( PS_DEFAULT_VISRADIUS ),
	m_tmElapsedEmission			( 0.0f ),
	m_bRendered					( false )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::~CParticleSystemFX
//
//  PURPOSE:	Standard Destructor
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::~CParticleSystemFX( void )
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Init
//
//  PURPOSE:	Creates and initialises the Particle system
//
// ----------------------------------------------------------------------- //

bool CParticleSystemFX::Init( ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	// Perform base class initialization
	if( !CBaseFX::Init( pLTClient, pData, pProps) ) 
		return false;

	ObjectCreateStruct	ocs;
	
	// Create Particle System
	ocs.m_ObjectType	= OT_PARTICLESYSTEM;
	ocs.m_Flags			|= FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE | pData->m_dwObjectFlags;
	ocs.m_Flags2		|= pData->m_dwObjectFlags2 | GetProps()->m_dwBlendMode;
	ocs.m_Pos			= m_vCreatePos;
	ocs.m_Rotation		= m_rCreateRot;

	// Develop the Right and Up vectors based off the Forward...

	if( pData->m_vTargetNorm.LengthSquared() > MATH_EPSILON )
	{
		LTVector vR, vU;

		pData->m_vTargetNorm.Normalize();

		if( (1.0f == pData->m_vTargetNorm.y) || (-1.0f == pData->m_vTargetNorm.y) )
		{
			vR = LTVector( 1.0f, 0.0f, 0.0f ).Cross( pData->m_vTargetNorm );
		}
		else
		{
			vR = LTVector( 0.0f, 1.0f, 0.0f ).Cross( pData->m_vTargetNorm );
		}

		vU = pData->m_vTargetNorm.Cross( vR );
		ocs.m_Rotation = LTRotation( pData->m_vTargetNorm, vU );
	}

	m_hObject = m_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
		return LTFALSE;

	uint32		dwWidth, dwHeight;
	HSURFACE	hScreen = m_pLTClient->GetScreenSurface();
	
	m_pLTClient->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);
    m_fVisRadius /= ((LTFLOAT)dwWidth);

	//The flags for the particle system
	uint32 nFlags = PS_DUMB;

	if(GetProps()->m_bRotate)
		nFlags |= PS_USEROTATION;

	if(GetProps()->m_bLight)
		nFlags |= PS_LIGHT;

	if(GetProps()->m_bCollideModels)
		nFlags |= PS_COLLIDE;

	if(!GetProps()->m_bObjectSpace)
		nFlags |= PS_WORLDSPACE;

	m_pLTClient->SetupParticleSystem(	m_hObject, 
										GetProps()->m_szFileName, 
										0.0f, 
										nFlags, 
										m_fVisRadius );

	// Dont let the base class update theese
	m_bUpdateColour	= LTFALSE;
	m_bUpdateScale	= LTFALSE;

	if( GetProps()->m_eType == PS_ePoint )
	{
		// Get a random point on our plane for use in point emision
		m_vRandomPoint = (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
						 (GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

		// Normalize it
		m_vRandomPoint.Norm();

		// Now scale it into our desired range
		m_vRandomPoint *= GetRandom( GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::UpdateParticleColor
//
//  Given a particle, it will calculate the color of it where RGB range 
//	from 0..255, and A ranges from 0..1.
//
// ----------------------------------------------------------------------- //
void CParticleSystemFX::UpdateParticleColor(LTParticle* pLTParticle)
{
	assert(pLTParticle);

	float tmActual = (1.0f - pLTParticle->m_Lifetime / pLTParticle->m_TotalLifetime);

	FX_COLOURKEY*	pKeys = GetProps()->m_pColorKeys;
	uint32			nNumKeys = GetProps()->m_nNumColorKeys;

	// Locate the keyframe, we can start at the cached keyframe since particle
	// lifetimes only move forward (this is stored in the high word of the user
	// data)
	uint32 nCurrColour = GetKeyOffset(pLTParticle->m_nUserData, COLOR_KEY_OFFSET);

	for (; nCurrColour + 1 < nNumKeys; nCurrColour++)
	{
		FX_COLOURKEY& endKey	= pKeys[nCurrColour + 1];

		if (tmActual < endKey.m_tmKey)
		{
			FX_COLOURKEY& startKey	= pKeys[nCurrColour];

			// Use this and the previous key to compute the colour

			float tmDist = endKey.m_tmKey - startKey.m_tmKey;

			//note that the distance should always be greater than 0
			assert(tmDist > 0.0f);
			
			float ratio = (tmActual - startKey.m_tmKey) / tmDist;
				
			pLTParticle->m_Color.x = (startKey.m_red + ((endKey.m_red - startKey.m_red) * ratio));
			pLTParticle->m_Color.y = (startKey.m_green + ((endKey.m_green - startKey.m_green) * ratio));
			pLTParticle->m_Color.z = (startKey.m_blue + ((endKey.m_blue - startKey.m_blue) * ratio));
			pLTParticle->m_Alpha   = 1.0f - (startKey.m_alpha + (endKey.m_alpha - startKey.m_alpha) * ratio) / 255.0f;

			//all done calculating colors, might as well bail
			break;
		}
	}

	//save this color keyframe for next time...
	SetKeyOffset(pLTParticle->m_nUserData, nCurrColour, COLOR_KEY_OFFSET);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::CalcParticleScale
//
//  Given a particle, it will calculate the scale for it based upon the
//	scale keyframes
//
// ----------------------------------------------------------------------- //
void CParticleSystemFX::UpdateParticleScale(LTParticle* pLTParticle)
{
	assert(pLTParticle);

	float tmActual = (1.0f - pLTParticle->m_Lifetime / pLTParticle->m_TotalLifetime);

	FX_SCALEKEY*	pKeys = GetProps()->m_pScaleKeys;
	uint32			nNumKeys = GetProps()->m_nNumScaleKeys;

	// Locate the keyframe
	uint32 nCurrScaleKey = GetKeyOffset(pLTParticle->m_nUserData, SCALE_KEY_OFFSET);

	for (; nCurrScaleKey + 1 < nNumKeys; nCurrScaleKey++)
	{
		FX_SCALEKEY& endKey		= pKeys[nCurrScaleKey + 1];

		if (tmActual < endKey.m_tmKey)
		{
			FX_SCALEKEY& startKey	= pKeys[nCurrScaleKey];

			// Use this and the previous key to compute the colour

			float tmDist = endKey.m_tmKey - startKey.m_tmKey;
			
			assert(tmDist > 0.0f);

			float ratio	 = (endKey.m_scale - startKey.m_scale) / tmDist;
			pLTParticle->m_Size = startKey.m_scale + (ratio * (tmActual - startKey.m_tmKey));
			
			//got the scale, bail
			break;
		}
	}
	
	//cache this scale for next time...
	SetKeyOffset(pLTParticle->m_nUserData, nCurrScaleKey, SCALE_KEY_OFFSET);
}


bool CParticleSystemFX::SuspendedUpdate( float tmFrameTime )
{
	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	UpdateParticles(tmFrameTime);
	return true;
}

bool CParticleSystemFX::Render()
{
	m_bRendered = true;
	return CBaseFX::Render();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CParticleSystemFX::Update( float tmFrameTime )
{
	ASSERT( m_hObject );
	
	uint32 dwFlags;

	// Was this system drawn last frame? Dont update if it was not
	m_pLTClient->Common()->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );
	if( m_bRendered && !(dwFlags & FLAG_WASDRAWN ) )
	{
		return LTTRUE;
	}

	//allow the base effect to handle any updates
	if(!CBaseFX::Update( tmFrameTime ))
		return false;

	// Are we ready for another emission?
	if(!IsShuttingDown())
	{
		//update our emission time
		m_tmElapsedEmission += tmFrameTime;

		//if this is the first frame, we need to handle emitting particles
		if(IsInitialFrame())
		{
			AddParticles();
			m_tmElapsedEmission = 0.0f;
		}

		//see if we want to emit any more particles
		float fEmissionInterval = GetProps()->m_fEmissionInterval;
		if(fEmissionInterval > 0.0f)
		{
			//we will only be able to see particles from the point of one emission interval
			//minus the maximum lifespan
			float tmStartEmissions = fEmissionInterval + GetProps()->m_fMaxLifeSpan;

			while(m_tmElapsedEmission >= fEmissionInterval)
			{
				//however, don't bother trying to add these particles if they won't live long enough
				//to show up in the next frame
				if(m_tmElapsedEmission >= tmStartEmissions)
				{
					m_tmElapsedEmission -= fEmissionInterval;
					continue;
				}

				// We have emission
				AddParticles();
				m_tmElapsedEmission -= fEmissionInterval;

				//see if we should run an update on those particles
				if(m_tmElapsedEmission >= fEmissionInterval)
				{
					//we need to run an update
					UpdateParticles(fEmissionInterval);
					tmFrameTime -= fEmissionInterval;
				}
			}
		}
	}

	UpdateParticles( tmFrameTime );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::AddParticles
//
//  PURPOSE:	Emit Particles
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::AddParticles( )
{
	LTVector	vPos;
	LTVector	vVel;
	LTVector	vColor( GetProps()->m_pColorKeys[0].m_red, GetProps()->m_pColorKeys[0].m_green, GetProps()->m_pColorKeys[0].m_blue );
	LTFLOAT		fTempLifeSpan;

	LTRotation	rRot;
	LTMatrix	mMat;

	
	if( !GetProps()->m_bObjectSpace )
	{
		// Get the rotation of the system so we can apply it to the velocity of each particle...

		m_pLTClient->GetObjectRotation( m_hObject, &rRot );
		rRot.ConvertToMatrix( mMat );
	}


	for( uint32 i = 0; i < GetProps()->m_nParticlesPerEmission; i++ )
	{
		// What kind of emission do we have?
		switch( GetProps()->m_eType )
		{
			case PS_eSphere:
				{
					vPos.x = GetRandom( -1.0f, 1.0f );
					vPos.y = GetRandom( -1.0f, 1.0f ); 
					vPos.z = GetRandom( -1.0f, 1.0f ); 

					// Normalize it
					vPos.Norm();

					// Now scale it into our desired range
					vPos *= GetRandom( GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius );
					
				}
				break;

			case PS_ePoint:
				{
					vPos = m_vRandomPoint;
				}	
				break;

			case PS_eCircle:
				{
					vPos =  (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
							(GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

					// Normalize it
					vPos.Norm();

					// Now scale it into our desired range
					vPos *= GetRandom( GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius );
				}
				break;

			case PS_eCone:
				{
					//the real trick here is evenly distributing the particles
					//inside of the cone since if we just blindly picked a height, the
					//particles would bunch up at the bottom. This is done by
					//using a square, since it bunches numbers more towards the lower
					//end of the spectrum, so if the lower end corresponds to the end
					//of the cone, it will tend to bunch it up more there
					float fRandom = GetRandom(0.0f, (float)sqrt(GetProps()->m_fMaxRadius));
					float fOffset = GetProps()->m_fMaxRadius - fRandom * fRandom;

					//generate a point on that circle
					vPos =  (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
							(GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

					// Normalize it
					vPos.Norm();

					//find the radius of the cone at that point
					float fRadius = GetProps()->m_fMinRadius * fOffset / GetProps()->m_fMaxRadius;

					// Now scale it into our desired range
					vPos *= GetRandom(0.0f, fRadius);

					//now offset it appropriately
					vPos += (GetProps()->m_vPlaneDir * fOffset);
				}
				break;

			case PS_eCylinder:
				{
					//position it on a circle
					vPos =  (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
							(GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

					// Normalize it
					vPos.Norm();

					// Now scale it into our desired range
					vPos *= GetRandom(0.0f, GetProps()->m_fMinRadius);

					//offset it along the height
					vPos += GetProps()->m_vPlaneDir * GetRandom(0.0f, GetProps()->m_fMaxRadius);
				}
				break;

			default:
				assert(!"Unknown particle emission type");
				break;

		}
		
		// Randomize the velocity within our range
		switch(GetProps()->m_eVelocityType)
		{
		case PSV_eRandom:
			{
				vVel.x = GetRandom( GetProps()->m_vMinVelocity.x, GetProps()->m_vMaxVelocity.x );
				vVel.y = GetRandom( GetProps()->m_vMinVelocity.y, GetProps()->m_vMaxVelocity.y );
				vVel.z = GetRandom( GetProps()->m_vMinVelocity.z, GetProps()->m_vMaxVelocity.z );
			}
			break;
		case PSV_eCenter:
			{
				//velocity direction is based upon position from 0, 0, 0
				float fVelocityScale = GetRandom(GetProps()->m_vMinVelocity.x, GetProps()->m_vMaxVelocity.x) / vPos.Mag();
				vVel = vPos * fVelocityScale;
			}
			break;
		default:
			assert(!"Unknown particle velocity type");
			break;
		}

		
		if( !GetProps()->m_bObjectSpace )
		{
			//convert velocity into world space
			vVel = mMat * vVel;

			//also convert our position
			vPos += m_vPos;
		}

		fTempLifeSpan = GetRandom( GetProps()->m_fMinLifeSpan, GetProps()->m_fMaxLifeSpan );

		// Try and add the new particle to the system
		LTParticle* pParticle = m_pLTClient->AddParticle( m_hObject, &vPos, &vVel, &vColor, fTempLifeSpan );
		if( !pParticle )
		{
			return;
		}

		pParticle->m_nUserData	= 0;
		pParticle->m_Alpha		= GetProps()->m_pColorKeys[0].m_alpha;
		pParticle->m_Size		= GetProps()->m_pScaleKeys[m_nCurrScaleKey].m_scale;

		//update our counts
		m_nNumParticles++;

		// Randomize the angle information if needed
		if(GetProps()->m_bRotate)
		{
			pParticle->m_fAngle				= GetRandom(0.0f, 2 * PI);
			pParticle->m_fAngularVelocity	= GetRandom(GetProps()->m_fMinAngularVelocity, GetProps()->m_fMaxAngularVelocity);
		}

		//determine if we want this particle to bounce
		if((GetProps()->m_fPercentToBounce > 0.001f) && (GetRandom(0.0f, 100.0f) < GetProps()->m_fPercentToBounce))
		{
			//this particle should bounce
			pParticle->m_nUserData |= BOUNCE_FLAG;
			m_nNumBounceParticles++;
		}

		//determine if we want this particle to splat
		if(GetProps()->m_szSplatEffect[0] && (GetProps()->m_fPercentToSplat > 0.001f) && (GetRandom(0.0f, 100.0f) < GetProps()->m_fPercentToSplat))
		{
			//this particle should bounce
			pParticle->m_nUserData |= SPLAT_FLAG;
			m_nNumSplatParticles++;
		}


		// To change the rendering order, move the newly added particle from the tail to the head.

		if( GetProps()->m_bFlipOrder )
		{
			LTParticle	*pHead = LTNULL;
			LTParticle	*pTail = LTNULL;

			if( m_pLTClient->GetParticles( m_hObject, &pHead, &pTail ) )
			{
				if( pHead != pParticle )
				{
					// Move the Cur particle to the head...

					pParticle->m_pPrev->m_pNext	= pParticle->m_pNext;
					pTail->m_pPrev				= pParticle->m_pPrev;
					pParticle->m_pNext			= pHead->m_pNext;
					pHead->m_pNext->m_pPrev		= pParticle;
					pHead->m_pNext				= pParticle;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::RemoveParticle
//
//  PURPOSE:	Removes the passed in particle from the system and maintains
//				appropriate counts
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::RemoveParticle(LTParticle* pParticle)
{
	// Disable this particle
 	m_pLTClient->RemoveParticle( m_hObject, pParticle );

	//update our counts
	assert(m_nNumParticles > 0);
	m_nNumParticles--;
	
	if(pParticle->m_nUserData & BOUNCE_FLAG)
	{
		m_nNumBounceParticles--;
	}
	if(pParticle->m_nUserData & SPLAT_FLAG)
	{
		m_nNumSplatParticles--;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::UpdateParticles
//
//  PURPOSE:	Manually move and update the particles
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::UpdateParticles( LTFLOAT tmFrame )
{
	LTVector				vGravity = GetProps()->m_vAcceleration * tmFrame;

	//find the frictional coefficient (this is what we multiply the velocity vector by).
	//this needs to be scaled based upon the time delta so that the friction amount is
	//reduced by the amount specified per second
	LTFLOAT					fFrictionCoef = (float)pow(1.0f - GetProps()->m_fFriction, tmFrame);

	// Check to see if this is a swarming system and then setup our matrix if it is
	if( GetProps()->m_bSwarm )
	{
		// Setup the swarmming matrix
		SetupRotation( m_matSwarm, LTRotation(GetProps()->m_vRotAdd.x * tmFrame, GetProps()->m_vRotAdd.y * tmFrame, GetProps()->m_vRotAdd.z * tmFrame)); 
	}

	// Since the particles pos and velocity is relative to the system we need to convert gravity to world space...
	if(GetProps()->m_bObjectSpace)
	{
		LTRotation rRot;
		m_pLTClient->GetObjectRotation( m_hObject, &rRot );
		LTMatrix mInvObjSpace;
		rRot.Conjugate().ConvertToMatrix( mInvObjSpace );

		vGravity = mInvObjSpace * vGravity;
	}

	LTParticle	*pHead = LTNULL;
	LTParticle	*pTail = LTNULL;

	if( !m_pLTClient->GetParticles( m_hObject, &pHead, &pTail ) )
		return;

	LTParticle  *pLTParticle = pHead;
	LTParticle  *pNext;

	while(pLTParticle != pTail)
	{
		//cache the next in case this particle is removed
		pNext = pLTParticle->m_pNext;

		//update the lifetime
		pLTParticle->m_Lifetime -= tmFrame;
		
		// Check for expiration
		if( pLTParticle->m_Lifetime <= 0.0f )
		{
			if(GetProps()->m_bInfiniteLife)
			{
				//this particle has died, but resurrect it since it lives forever
				pLTParticle->m_Lifetime = pLTParticle->m_TotalLifetime - fmodf(-pLTParticle->m_Lifetime, pLTParticle->m_TotalLifetime); 				

				//reset the color and scale keys so that they won't get messed up
				SetKeyOffset(pLTParticle->m_nUserData, 0, COLOR_KEY_OFFSET);
				SetKeyOffset(pLTParticle->m_nUserData, 0, SCALE_KEY_OFFSET);
			}
			else
			{
				RemoveParticle(pLTParticle);
			}
		}
		else
		{
			// Give the particle an update

			// Update the angle if appropriate
			pLTParticle->m_fAngle	+= pLTParticle->m_fAngularVelocity * tmFrame;
			pLTParticle->m_Pos		+= pLTParticle->m_Velocity * tmFrame;

			// Move it
			if( GetProps()->m_bObjectSpace && GetProps()->m_bSwarm)
			{
				pLTParticle->m_Pos = m_matSwarm * pLTParticle->m_Pos;
			}

			//update the velocity, applying gravity and friction
			pLTParticle->m_Velocity.x = pLTParticle->m_Velocity.x * fFrictionCoef + vGravity.x;
			pLTParticle->m_Velocity.y = pLTParticle->m_Velocity.y * fFrictionCoef + vGravity.y;
			pLTParticle->m_Velocity.z = pLTParticle->m_Velocity.z * fFrictionCoef + vGravity.z;

			// Color it and scale it
			UpdateParticleColor(pLTParticle);
			UpdateParticleScale(pLTParticle);
		}

		pLTParticle = pNext;
	}

	//bounce is broken out of the above loop since it was rarely used, so it was 
	//just adding an additional if per particle as well as adding a lot of code 
	//to the inner loop
	if( (m_nNumBounceParticles > 0) || (m_nNumSplatParticles > 0) )
	{
		ClientIntersectQuery	iQuery;
		ClientIntersectInfo		iInfo;

		//we need to re-get the head and tail pointers (in case they were removed because of lifespan)
		if( !m_pLTClient->GetParticles( m_hObject, &pHead, &pTail ) )
			return;

		//figure out our transform if we are in object space
		LTMatrix mObjTransform;
		if(GetProps()->m_bObjectSpace)
		{
			// Setup rotation
			LTRotation	rObjRot;
			LTVector	vObjPos;
	
			m_pLTClient->GetObjectPos( m_hObject, &vObjPos );
			m_pLTClient->GetObjectRotation( m_hObject, &rObjRot );

			// Setup the swarmming matrix
			SetupRotationAroundPoint( mObjTransform, rObjRot, vObjPos ); 
		}
		
		LTParticle* pNext = NULL;
		for(pLTParticle = pHead; pLTParticle != pTail; pLTParticle = pNext)
		{
			//save the next particle in case it gets removed
			pNext = pLTParticle->m_pNext;

			//make sure that this particle is set to bounce
			if(!(pLTParticle->m_nUserData & (BOUNCE_FLAG | SPLAT_FLAG)))
				continue;

			//see if we need to convert our points into world space
			if(GetProps()->m_bObjectSpace)
			{
				LTVector vPos	= m_vPos + pLTParticle->m_Pos;
				iQuery.m_From	= mObjTransform * vPos;
				iQuery.m_To		= mObjTransform * (vPos + pLTParticle->m_Velocity * tmFrame);
			}
			else
			{
				iQuery.m_From	= pLTParticle->m_Pos;
				iQuery.m_To		= pLTParticle->m_Pos + pLTParticle->m_Velocity * tmFrame;
			}

			if( m_pLTClient->IntersectSegment( &iQuery, &iInfo ) )
			{
				//handle bounce
				if(pLTParticle->m_nUserData & BOUNCE_FLAG)
				{
					LTVector& vVel			= pLTParticle->m_Velocity;
					const LTVector& vNormal = iInfo.m_Plane.m_Normal;

					//reflect the velocity over the normal
					vVel -= vNormal * (2.0f * vVel.Dot(vNormal));

					//apply some hack coefficient of restitution
					vVel *= 0.75f;
				}

				//handle splat
				if(pLTParticle->m_nUserData & SPLAT_FLAG)
				{
					//alright, we now need to create a splat effect

					//first build up an orientation space
					LTVector vForward = iInfo.m_Plane.m_Normal;

					//create a random vector
					LTVector vUp(GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f));

					//make sure we have a magnitude
					if(vUp.MagSqr() < 0.1f)
					{
						vUp.Init(0.0f, 1.0f, 0.0f);
					}
					else
					{
						vUp.Normalize();
					}

					//now see if we are too close to our other input vector
					if(vUp.Dot(vForward) > 0.95f)
					{
						vUp.Init(vUp.x, vUp.z, -vUp.y);
					}

					//reorthogonalize
					LTVector vRight = vForward.Cross(vUp);
					vRight.Normalize();

					vUp = vRight.Cross(vForward);
					vUp.Normalize();

					//build up a matrix
					LTMatrix mMat;
					mMat.SetBasisVectors(&vRight, &vUp, &vForward);

					LTRotation rRot;
					rRot.ConvertFromMatrix(mMat);


					CLIENTFX_CREATESTRUCT CreateStruct(GetProps()->m_szSplatEffect, 0, iInfo.m_Point, rRot);
					CreateNewFX(CreateStruct, true);					

					//see if we need to kill the particle
					if(GetProps()->m_bKillOnSplat)
					{
						//we do....
						RemoveParticle(pLTParticle);
					}
				}

				//NOTE: Particle should not be used from this point or beyond since
				//it can be removed above
			}
		}
	}

	if( ++m_nOptCount == PS_OPTIMIZE_COUNT )
	{
		m_pLTClient->OptimizeParticles( m_hObject );
		m_nOptCount = 0;
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::Term
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CParticleSystemFX::Term( void )
{
	if( m_hObject )
		m_pLTClient->RemoveObject( m_hObject );
		
	m_hObject = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetParticleSystemProps
//
//  PURPOSE:	Returns a list of properties for ths effect to be used in FxED
//
// ----------------------------------------------------------------------- //

void fxGetParticleSystemProps( CFastList<FX_PROP> *pList )
{
	FX_PROP	fxProp;
	float	fYVec[3];
	fYVec[0] = 0.0f;
	fYVec[1] = 1.0f;
	fYVec[2] = 0.0f;

	float	fZeroVec[3];
	fZeroVec[0] = 0.0f;
	fZeroVec[1] = 0.0f;
	fZeroVec[2] = 0.0f;

	// Add the generic "every effect has theese" props
	AddBaseProps( pList );

	// Add specific Particle System Props
	
	fxProp.Path( "Sprite", "spr|..." );
	pList->AddTail( fxProp );

	fxProp.Path( "Texture", "dtx|..." );
	pList->AddTail( fxProp );

	fxProp.Float( "EmissionInterval", 0.01f );
	pList->AddTail( fxProp );
	
	fxProp.Int( "ParticlesPerEmission", 5 );
	pList->AddTail( fxProp );

	fxProp.Float( "GravityAcceleration", -500.0f );
	pList->AddTail( fxProp );

	fxProp.Vector( "AdditionalAcceleration", fZeroVec );
	pList->AddTail( fxProp );
	
	fxProp.Float( "MinParticleLifeSpan", 2.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxParticleLifeSpan", 3.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MinRadius", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxRadius", 10.0f );
	pList->AddTail( fxProp );

	fxProp.Vector( "EmissionPlane", fYVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MinParticleVelocity", fYVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MaxParticleVelocity", fYVec );
	pList->AddTail( fxProp );

	fxProp.Combo( "Type", "0, Sphere, Point, Circle, Cone, Cylinder" );
	pList->AddTail( fxProp );

	fxProp.Float( "PercentToBounce", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "FlipRenderingOrder", "0, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "LightParticles", "0, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "RotateParticles", "0, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Float( "MinAngularVelocity", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxAngularVelocity", 360.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "BlendMode", "0, None, Additive, Multiply" );
	pList->AddTail( fxProp );

	fxProp.Combo( "MoveParticlesWithSystem", "1, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Combo( "CollideModels", "0, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Float( "Friction", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "VelocityType", "0, Random, FromCenter" );
	pList->AddTail( fxProp );

	fxProp.Combo( "InfiniteLife", "0, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.Float( "SplatPercent", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Combo( "KillOnSplat", "1, No, Yes" );
	pList->AddTail( fxProp );

	fxProp.String( "SplatEffect", "");
	pList->AddTail( fxProp );

}

