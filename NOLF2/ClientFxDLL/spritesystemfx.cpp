//------------------------------------------------------------------
//
//   MODULE  : SPRITESYSTEM.CPP
//
//   PURPOSE : Implements class CSpriteSystem
//
//   CREATED : On 10/26/98 At 3:59:54 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "ClientFX.h"
#include "SpriteSystemFX.h"
#include "FastList.h"
#include "CycleTimer.h"
#include "iltspritecontrol.h"
#include "stdio.h"
#include "time.h"

// Globals....

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteSystemProps::CSpriteSystemProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CSpriteSystemProps::CSpriteSystemProps() : 
	m_nSpritesPerEmission			( 5 ),
	m_fGravity						( 0.0f) ,
	m_fEmissionInterval				( 1.0f ),
	m_fMinSpriteLifespan			( 1.0f ),
	m_fMaxSpriteLifespan			( 2.0f ),
	m_nEmissionType					( SS_POINT ),
	m_fMinRadius					( 0.0f ),
	m_fMaxRadius					( 10.0f ),
	m_vPlaneDir						( 0.0f, 1.0f, 0.0f ),
	m_vMinSpriteVelocity			( 0.0f, 0.0f, 0.0f ),
	m_vMaxSpriteVelocity			( 0.0f, 0.0f, 0.0f ),
	m_vMinSpriteRotation			( 0.0f, 0.0f, 0.0f ),
	m_vMaxSpriteRotation			( 0.0f, 0.0f, 0.0f ),
	m_nAlphaType					( 0 ),
	m_fStretchU						( 1.0f ),
	m_fStretchV						( 1.0f ),
	m_nAnimLengthType				( SS_ANIMLEN_SPRITEDEFAULT ),
	m_fAnimSpeed					( 1.0f ),
	m_bInfiniteLife					( LTFALSE ),
	m_bUseSpin						( LTFALSE )
{
	m_szFileName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteSystemProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CSpriteSystemProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Sprite" ))
		{
			fxProp.GetPath( m_szFileName );
		}
		else if( !_stricmp( fxProp.m_sName, "EmissionInterval" ))
		{
			m_fEmissionInterval = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "SpritesPerEmission" ))
		{
			m_nSpritesPerEmission = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MinSpriteRotation" ))
		{
			m_vMinSpriteRotation = fxProp.GetVector();

			m_vMinSpriteRotation.x = MATH_DEGREES_TO_RADIANS( m_vMinSpriteRotation.x );
			m_vMinSpriteRotation.y = MATH_DEGREES_TO_RADIANS( m_vMinSpriteRotation.y );
			m_vMinSpriteRotation.z = MATH_DEGREES_TO_RADIANS( m_vMinSpriteRotation.z );
		}
		else if( !_stricmp( fxProp.m_sName, "MaxSpriteRotation" ))
		{
			m_vMaxSpriteRotation = fxProp.GetVector();

			m_vMaxSpriteRotation.x = MATH_DEGREES_TO_RADIANS( m_vMaxSpriteRotation.x );
			m_vMaxSpriteRotation.y = MATH_DEGREES_TO_RADIANS( m_vMaxSpriteRotation.y );
			m_vMaxSpriteRotation.z = MATH_DEGREES_TO_RADIANS( m_vMaxSpriteRotation.z );
		}
		else if( !_stricmp( fxProp.m_sName, "GravityAcceleration" ))
		{
			m_fGravity = fxProp.GetFloatVal();
		}
		else if (!_stricmp( fxProp.m_sName, "MinSpriteLifespan" ))
		{
			m_fMinSpriteLifespan = fxProp.GetFloatVal();
		}
		else if (!_stricmp( fxProp.m_sName, "MaxSpriteLifespan" ))
		{
			m_fMaxSpriteLifespan = fxProp.GetFloatVal();
		}
		else if (!_stricmp( fxProp.m_sName, "MinRadius" ))
		{
			m_fMinRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxRadius" ))
		{
			m_fMaxRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "EmissionPlane" ))
		{
			m_vPlaneDir = fxProp.GetVector();
			
			m_vPlaneDir.Norm();

			// Get the perpindicular vectors to this plane
			FindPerps(m_vPlaneDir, m_vPerp1, m_vPerp2);
		}
		else if( !_stricmp( fxProp.m_sName, "MinSpriteVelocity" ))
		{
			m_vMinSpriteVelocity = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "MaxSpriteVelocity" ))
		{
			m_vMaxSpriteVelocity = fxProp.GetVector();
		}
		else if( !_stricmp( fxProp.m_sName, "EmissionType" ))
		{
			m_nEmissionType = (uint32)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "AType" ))
		{
			m_nAlphaType = (uint32)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "StretchU" ))
		{
			m_fStretchU = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "StretchV" ))
		{
			m_fStretchV = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "AnimationLength" ))
		{
			m_nAnimLengthType = (uint32)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "AnimationSpeed" ))
		{
			m_fAnimSpeed = fxProp.GetFloatVal();

			//make sure it isn't infinitely fast!
			m_fAnimSpeed = LTMAX(m_fAnimSpeed, 0.001f);
		}
		else if( !_stricmp( fxProp.m_sName, "InfiniteLife" ) )
		{
			m_bInfiniteLife = (LTBOOL)fxProp.GetComboVal();
		}
	}

	m_bUseSpin = ((m_vRotAdd.x != 0) || (m_vRotAdd.y != 0.0f) || (m_vRotAdd.z != 0.0f)) ? true : false;
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CSpriteSystem()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSpriteSystem::CSpriteSystem()
:	CBaseFX							( CBaseFX::eSpriteSystem ),
	m_tmElapsedEmission				( 0.0f ),
	m_pTexArray						( LTNULL ),
	m_vRandomPoint					( 0.0f, 0.0f, 0.0f ), 
	m_nSpriteFrames					( 0 )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSpriteSystem
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSpriteSystem::~CSpriteSystem()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSpriteSystem
//
//------------------------------------------------------------------

bool CSpriteSystem::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;


	if( GetProps()->m_bUseSpin )
	{	
		m_rSpin.Identity();
	}

	// Allocate the static sprite array, handling the case where the user only wants one emission
	uint32 nNumAllocSprites = GetProps()->m_nSpritesPerEmission;
	if(GetProps()->m_fEmissionInterval > 0.0f)
	{
		nNumAllocSprites = (uint32)(((1.0f / GetProps()->m_fEmissionInterval) * GetProps()->m_nSpritesPerEmission) * GetProps()->m_fMaxSpriteLifespan) + GetProps()->m_nSpritesPerEmission;
	}

	m_collSprites.Init(nNumAllocSprites);

	// Create the base object

	ObjectCreateStruct ocs;
	
	ocs.m_ObjectType		= OT_SPRITE;
	ocs.m_Flags				|= pBaseData->m_dwObjectFlags | FLAG_ROTATEABLESPRITE;
	ocs.m_Flags2			|= pBaseData->m_dwObjectFlags2;
	ocs.m_Pos				= m_vCreatePos;
	ocs.m_Rotation			= m_rCreateRot;
	ocs.m_Scale.Init( 0.0f, 0.0f, 0.0f );
	
	SAFE_STRCPY(ocs.m_Filename, GetProps()->m_szFileName);

	// Develop the Right and Up vectors based off the Forward...

	if( pBaseData->m_vTargetNorm.LengthSquared() > MATH_EPSILON )
	{
		LTVector vR, vU;

		pBaseData->m_vTargetNorm.Normalize();

		if( (1.0f == pBaseData->m_vTargetNorm.y) || (-1.0f == pBaseData->m_vTargetNorm.y) )
		{
			vR = LTVector( 1.0f, 0.0f, 0.0f ).Cross( pBaseData->m_vTargetNorm );
		}
		else
		{
			vR = LTVector( 0.0f, 1.0f, 0.0f ).Cross( pBaseData->m_vTargetNorm );
		}

		vU = pBaseData->m_vTargetNorm.Cross( vR );
		ocs.m_Rotation = LTRotation( pBaseData->m_vTargetNorm, vU );
	}

	
	m_hObject = m_pLTClient->CreateObject(&ocs);
	if( !m_hObject )
		return LTFALSE;

	
	ILTSpriteControl *pControl;
	m_pLTClient->GetSpriteControl(m_hObject, pControl);

	if( pControl )
	{
		uint32 tmSpriteLen;
		
		pControl->GetAnimLength(tmSpriteLen, 0);

		pControl->GetNumFrames(0, m_nSpriteFrames);	
		
		//ok, this takes some convoluted math in order to figure out the length of
		//this sprite. The value of tmSpriteLen is the time FPS * Number of frames,
		//so we can get the number of frames a second, and find the actual time in seconds
		float fFPS = (float)tmSpriteLen / (float)m_nSpriteFrames;

		m_fSpriteLen = m_nSpriteFrames / fFPS;

		if( m_nSpriteFrames )
		{
			m_pTexArray = debug_newa( HTEXTURE, m_nSpriteFrames);

			for (uint32 i = 0; i < m_nSpriteFrames; i ++)
			{
				pControl->GetFrameTextureHandle(m_pTexArray[i], 0, i);
			}
		}
	}
	

	m_bUpdateColour = LTFALSE;
	m_bUpdateScale  = LTFALSE;

	if( GetProps()->m_nEmissionType == SS_POINT )
	{
		// Get a random point on our plane for use in point emision

		m_vRandomPoint = (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
						 (GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

		// Normalize it
		
		m_vRandomPoint.Norm();

		// Now scale it into our desired range
		
		m_vRandomPoint *= GetRandom( GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius );
	}

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CSpriteSystem
//
//------------------------------------------------------------------

void CSpriteSystem::Term()
{
	// Make sure these are released before the object is released...
	if (m_pTexArray)
	{
		for (uint32 i=0; i < m_nSpriteFrames; i++)
		{
			if (m_pTexArray[i])
			{
				m_pLTClient->GetTexInterface()->ReleaseTextureHandle(m_pTexArray[i]);
			}
		}
	}

	if( m_hObject )
	{
		m_pLTClient->RemoveObject( m_hObject );
		m_hObject = LTNULL;
	}

	debug_deletea( m_pTexArray );
	m_pTexArray = NULL;
	m_nSpriteFrames = 0;
}

bool CSpriteSystem::SuspendedUpdate( float tmFrameTime )
{
	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	UpdateSprites(tmFrameTime);
	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CSpriteSystem
//
//------------------------------------------------------------------

bool CSpriteSystem::Update(float tmFrameTime)
{
	if( !m_hObject ) 
		return LTFALSE;

	// Base class update first
		
	if( !CBaseFX::Update( tmFrameTime ) ) 
		return LTFALSE;

	//update our elapsed emission time
	m_tmElapsedEmission += tmFrameTime;

	// Are we ready for another emission?
	if(!IsShuttingDown())
	{
		//if this is the first frame, we need to handle emitting particles
		if(IsInitialFrame())
		{
			AddSprites();
			m_tmElapsedEmission = 0.0f;
		}

		//see if we want to emit any more particles
		if(GetProps()->m_fEmissionInterval > 0.0f)
		{
			while(m_tmElapsedEmission >= GetProps()->m_fEmissionInterval)
			{
				//however, don't bother trying to add these particles if they won't live long enough
				//to show up in the next frame
				if(m_tmElapsedEmission >= GetProps()->m_fMaxSpriteLifespan)
				{
					m_tmElapsedEmission -= GetProps()->m_fEmissionInterval;
					continue;
				}

				// We have emission
				AddSprites();
				m_tmElapsedEmission -= GetProps()->m_fEmissionInterval;

				//see if we should run an update on those particles
				if(m_tmElapsedEmission >= GetProps()->m_fEmissionInterval)
				{
					//we need to run an update
					UpdateSprites(GetProps()->m_fEmissionInterval);
					tmFrameTime -= GetProps()->m_fEmissionInterval;
				}
			}
		}
	}
	
	UpdateSprites( tmFrameTime );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteSystem::AddSprites
//
//  PURPOSE:	Emit sprites
//
// ----------------------------------------------------------------------- //

void CSpriteSystem::AddSprites( )
{
	LTVector	vPos;
	LTVector	vObjPos;
	LTRotation	rObjRot;
	LTMatrix	mObjRot;

	vObjPos = m_vPos;
	m_pLTClient->GetObjectRotation( m_hObject, &rObjRot );
	rObjRot.ConvertToMatrix( mObjRot );

	for( uint32 i = 0; i < GetProps()->m_nSpritesPerEmission; i++ )
	{
		// Get an unused sprite

		SPRITE *pNewSprite = m_collSprites.Alloc();

		if (pNewSprite)
		{
			// What kind of emission do we have?
			switch( GetProps()->m_nEmissionType )
			{
				case SS_SPHERE:
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

				case SS_POINT:
					{
						vPos = m_vRandomPoint;
					}	
					break;

				case SS_CIRCLE:
					{
						vPos =  (GetProps()->m_vPerp1 * GetRandom( -1.0f, 1.0f )) +
								(GetProps()->m_vPerp2 * GetRandom( -1.0f, 1.0f ));

						// Normalize it
						vPos.Norm();

						// Now scale it into our desired range
						vPos *= GetRandom( GetProps()->m_fMinRadius, GetProps()->m_fMaxRadius );
					}
					break;

				case SS_CONE:
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

				case SS_CYLINDER:
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
					assert(!"Error: Unknown sprite system emission type encountered");
					return;

			}

			// Randomize the velocity within our range
			
			pNewSprite->m_vVel.x = GetRandom( GetProps()->m_vMinSpriteVelocity.x, GetProps()->m_vMaxSpriteVelocity.x );
			pNewSprite->m_vVel.y = GetRandom( GetProps()->m_vMinSpriteVelocity.y, GetProps()->m_vMaxSpriteVelocity.y );
			pNewSprite->m_vVel.z = GetRandom( GetProps()->m_vMinSpriteVelocity.z, GetProps()->m_vMaxSpriteVelocity.z );

			// Set the velocity in the correct rotation to keep "localized"

			pNewSprite->m_vVel = mObjRot * pNewSprite->m_vVel;

			// Randomize our rotation...

			pNewSprite->m_vRotAdd.x = GetRandom( GetProps()->m_vMinSpriteRotation.x, GetProps()->m_vMaxSpriteRotation.x );
			pNewSprite->m_vRotAdd.y = GetRandom( GetProps()->m_vMinSpriteRotation.y, GetProps()->m_vMaxSpriteRotation.y );
			pNewSprite->m_vRotAdd.z = GetRandom( GetProps()->m_vMinSpriteRotation.z, GetProps()->m_vMaxSpriteRotation.z );

			pNewSprite->m_rRot		= LTRotation( pNewSprite->m_vRotAdd.x, pNewSprite->m_vRotAdd.y, pNewSprite->m_vRotAdd.z );
			pNewSprite->m_vPos		= vPos + vObjPos;
			pNewSprite->m_fLifespan	= GetRandom( GetProps()->m_fMinSpriteLifespan, GetProps()->m_fMaxSpriteLifespan );
			pNewSprite->m_tmElapsed	= 0.0f;
		}

	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteSystem::UpdateSprites
//
//  PURPOSE:	Manually move and update the particles
//
// ----------------------------------------------------------------------- //

void CSpriteSystem::UpdateSprites( LTFLOAT tmFrameTime )
{
	//if there is no time elapsed, don't bother to update
	if(tmFrameTime < 0.0001f)
		return;

	SPRITE *pSprites = m_collSprites.GetData();
	uint32 nSprites = m_collSprites.GetUsed();
	
	if( !pSprites )
		return;
	
	if (GetProps()->m_bUseSpin)
	{
		// Add on our rotation
		m_rSpin.Rotate( m_rSpin.Right(),	MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.x * tmFrameTime) );
		m_rSpin.Rotate( m_rSpin.Up(),		MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.y * tmFrameTime) );
		m_rSpin.Rotate( m_rSpin.Forward(),	MATH_DEGREES_TO_RADIANS(GetProps()->m_vRotAdd.z * tmFrameTime) );
	}

	// Update the sprites....

	for( uint32 i = 0; i < nSprites; i ++)
	{
		pSprites->m_tmElapsed += tmFrameTime;

		if (pSprites->m_tmElapsed >= pSprites->m_fLifespan)
		{
			if(GetProps()->m_bInfiniteLife)
			{
				//we have infinite life, update the lifepsan
				pSprites->m_tmElapsed = fmodf(pSprites->m_tmElapsed, pSprites->m_fLifespan);
			}
			else
			{
				// Destroy this sprite
				m_collSprites.Free(pSprites);
				continue;
			}
		}
		
		pSprites->m_vPos += (pSprites->m_vVel * tmFrameTime);
		pSprites->m_vVel.y += (GetProps()->m_fGravity * tmFrameTime);

		//add onto our rotation
		pSprites->m_rRot.Rotate( pSprites->m_rRot.Right(),		pSprites->m_vRotAdd.x * tmFrameTime );
		pSprites->m_rRot.Rotate( pSprites->m_rRot.Up(),			pSprites->m_vRotAdd.y * tmFrameTime );
		pSprites->m_rRot.Rotate( pSprites->m_rRot.Forward(),	pSprites->m_vRotAdd.z * tmFrameTime );

		pSprites ++;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpriteSystem::Render
//
//  PURPOSE:	Renders the sprites to the scene using DrawPrim
//
// ----------------------------------------------------------------------- //

bool CSpriteSystem::Render()
{
	//don't bother if we don't have focus
	if( !CBaseFX::Render() ) 
		return false;

	SPRITE *pSprites = m_collSprites.GetData();
	uint32 nSprites = m_collSprites.GetUsed();
	
	if( !pSprites )
		return true;
	
	// Build the camera transform

	LTMatrix	mFull = GetCamTransform(m_pLTClient, m_hCamera);

	uint32 nFlags;
	m_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags, nFlags);

	//matrix holding the representation for this sprite sytem
	LTMatrix mSpin;

	if (GetProps()->m_bUseSpin)
	{
		// Setup rotation

		LTVector	vObjPos;
		LTRotation	rObjRot;
		
		m_pLTClient->GetObjectPos( m_hObject, &vObjPos );
		m_pLTClient->GetObjectRotation( m_hObject, &rObjRot );

		// Setup the swarmming matrix
		SetupRotationAroundPoint( mSpin, rObjRot * m_rSpin, vObjPos ); 
	}

	// Setup the draw primitive...

	ILTDrawPrim *pDrawPrimitive = m_pLTClient->GetDrawPrim();

	if(nFlags & FLAG_REALLYCLOSE)
	{
		mFull.Identity();
		pDrawPrimitive->SetReallyClose(true);
	}

	pDrawPrimitive->SetTransformType(DRAWPRIM_TRANSFORM_CAMERA);

	if (GetProps()->m_nAlphaType)
	{
		pDrawPrimitive->SetAlphaBlendMode(DRAWPRIM_BLEND_MUL_SRCALPHA_ONE);
	}
	else
	{
		pDrawPrimitive->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	}

	pDrawPrimitive->SetColorOp(DRAWPRIM_MODULATE);
	pDrawPrimitive->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	pDrawPrimitive->SetZBufferMode(DRAWPRIM_ZRO);

	LTFLOAT		r, g, b, a;
	LTFLOAT		scale			= 0.2f;
	HTEXTURE	hTexture		= m_pTexArray[0];
	HTEXTURE	hPrevTexture	= NULL;
	uint32		dwWidth			= 0;
	uint32		dwHeight		= 0;
	LTVector	vWorldPos;

	//calculate the duration of the sprite animation (note that this will be overwritten
	//if the animation length is specified to be dependent upon the lifespan of
	//the individual particles)
	float fAnimLength = 1.0f;

	if(GetProps()->m_nAnimLengthType == SS_ANIMLEN_SPRITEDEFAULT)
		fAnimLength = m_fSpriteLen;
	else if(GetProps()->m_nAnimLengthType == SS_ANIMLEN_KEY)
		fAnimLength = GetLifespan();

	//make sure to adjust our animation speed based upon the provided multiplier
	fAnimLength /= GetProps()->m_fAnimSpeed;

	// Update the sprites....

	//tell draw primitive that we are going to be making a lot of calls, so that it doesn't
	//need to constantly reset all the states
	pDrawPrimitive->BeginDrawPrim();

	for( uint32 i = 0; i < nSprites; i ++)
	{
		// Rotate if neccessary
		LTVector vPos = pSprites->m_vPos;

		if( GetProps()->m_bUseSpin )
		{
			MatVMul_InPlace(&mSpin, &vPos);
		}

		// Setup the color and scale
		CalcColour( pSprites->m_tmElapsed, pSprites->m_fLifespan, &r, &g, &b, &a );		
		CalcScale( pSprites->m_tmElapsed, pSprites->m_fLifespan, &scale );

		// Set up the current texture we should be using...
		if (m_nSpriteFrames > 1)
		{
			//see if we need to base the lifespan off of the sprite particle's
			if(GetProps()->m_nAnimLengthType == SS_ANIMLEN_PARTICLE)
				fAnimLength = pSprites->m_fLifespan * GetProps()->m_fAnimSpeed;

			int nIndex = (int)((float)m_nSpriteFrames * pSprites->m_tmElapsed / fAnimLength);
			nIndex = nIndex % m_nSpriteFrames;

			hTexture = m_pTexArray[nIndex];
		}

		// Draw the particle
		if(hPrevTexture != hTexture)
		{
			m_pLTClient->GetTexInterface()->GetTextureDims(hTexture, dwWidth, dwHeight);
			pDrawPrimitive->SetTexture(hTexture);

			hPrevTexture = hTexture;
		}
		
		MatVMul(&vWorldPos, &mFull, &vPos);

		SetupParticle(vWorldPos, scale, (float)dwHeight * scale / (float)dwWidth, pSprites->m_rRot, 
				(uint8)(r * 255.0f), (uint8)(g * 255.0f), (uint8)(b * 255.0f), (uint8)(a * 255.0f));

		pDrawPrimitive->DrawPrim(g_pTris, 2);

		pSprites ++;
	}

	pDrawPrimitive->EndDrawPrim();
	pDrawPrimitive->SetZBufferMode(DRAWPRIM_ZRW);

	if(nFlags & FLAG_REALLYCLOSE)
	{
		pDrawPrimitive->SetReallyClose(false);
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : CSpriteSystem::SetupParticle()
//
//   PURPOSE  : Adds a particle quad into the vertex list
//
//------------------------------------------------------------------

void CSpriteSystem::SetupParticle(const LTVector& vPos, float fWidth, float fHeight, const LTRotation& rSpin, uint8 r, uint8 g, uint8 b, uint8 a)
{	
	LT_POLYGT3 *pTris    = g_pTris;

	if (vPos.z < 0.0f) 
		return;

	fWidth  *= GetProps()->m_fStretchU;
	fHeight *= GetProps()->m_fStretchV;
	
	LTVector	vVert[4];

	vVert[0] = LTVector( vPos.x - fWidth, vPos.y + fHeight, vPos.z );
	vVert[1] = LTVector( vPos.x + fWidth, vPos.y + fHeight, vPos.z );
	vVert[2] = LTVector( vPos.x + fWidth, vPos.y - fHeight, vPos.z );
	vVert[3] = LTVector( vPos.x - fWidth, vPos.y - fHeight, vPos.z );
	
	if( !rSpin.IsIdentity() )
	{
		LTMatrix mSpin;
		SetupRotationAroundPoint( mSpin, rSpin, vPos );
		
		vVert[0] = mSpin * vVert[0];
		vVert[1] = mSpin * vVert[1];
		vVert[2] = mSpin * vVert[2];
		vVert[3] = mSpin * vVert[3];
	}
	
	SetupVert(pTris, 0, vVert[0], r, g, b, a, 0.0f, 0.0f);
	SetupVert(pTris, 1, vVert[1], r, g, b, a, 1.0f, 0.0f);
	SetupVert(pTris, 2, vVert[2], r, g, b, a, 1.0f, 1.0f);

	pTris ++;

	SetupVert(pTris, 0, vVert[0], r, g, b, a, 0.0f, 0.0f);
	SetupVert(pTris, 1, vVert[2], r, g, b, a, 1.0f, 1.0f);
	SetupVert(pTris, 2, vVert[3], r, g, b, a, 0.0f, 1.0f);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetSpriteSystemProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetSpriteSystemProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;
	float fVec[3];
	fVec[0] = 0.0f;
	fVec[1] = 1.0f;
	fVec[2] = 0.0f;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Sprite", "spr|...");
	pList->AddTail(fxProp);

	fxProp.Float("EmissionInterval", 0.01f);
	pList->AddTail(fxProp);

	fxProp.Int("SpritesPerEmission", 5);
	pList->AddTail(fxProp);

	fxProp.Vector( "MinSpriteRotation", fVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MaxSpriteRotation", fVec );
	pList->AddTail( fxProp );

	fxProp.Float( "GravityAcceleration", -500.0f );
	pList->AddTail( fxProp );

	fxProp.Float("MinSpriteLifespan", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Float("MaxSpriteLifespan", 2.0f);
	pList->AddTail(fxProp);

	fxProp.Float( "MinRadius", 0.0f );
	pList->AddTail( fxProp );

	fxProp.Float( "MaxRadius", 10.0f );
	pList->AddTail( fxProp );

	fxProp.Vector( "EmissionPlane", fVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MinSpriteVelocity", fVec );
	pList->AddTail( fxProp );

	fxProp.Vector( "MaxSpriteVelocity", fVec );
	pList->AddTail( fxProp );

	fxProp.Combo( "EmissionType", "0,Sphere,Point,Circle,Cone,Cylinder" );
	pList->AddTail( fxProp );

	fxProp.Combo("AType", "0,Norm,Add");
	pList->AddTail(fxProp);

	fxProp.Float("StretchU", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Float("StretchV", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Combo("AnimationLength", "0,SpriteDefault,ParticleLife,KeyLength");
	pList->AddTail(fxProp);

	fxProp.Float("AnimationSpeed", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Combo("InfiniteLife", "0, No, Yes" );
	pList->AddTail(fxProp);
}

