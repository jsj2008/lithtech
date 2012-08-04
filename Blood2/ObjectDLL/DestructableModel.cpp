// ----------------------------------------------------------------------- //
//
// MODULE  : DestructableModel.cpp
//
// PURPOSE : A Model object that replaces it's model with a new model when
//			 it is damaged and finally destroyed.
//
// CREATED : 10/25/97
//
// ----------------------------------------------------------------------- //

#include "DestructableModel.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"
#include "PhysicalAttributes.h"
#include <mbstring.h>
#include "SoundTypes.h"
#include "SFXMsgIds.h"

BEGIN_CLASS(CDestructableModel)
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_VISIBLE_FLAG( 1, 0 )
	ADD_SOLID_FLAG( 1, 0 )
	ADD_GRAVITY_FLAG( 1, 0 )
	ADD_SHADOW_FLAG( 1, 0 )
	ADD_STRINGPROP_FLAG(InitFilename, "", PF_DIMS | PF_LOCALDIMS)
	ADD_STRINGPROP(InitSkin, "")
	ADD_VECTORPROP(InitDims)
	ADD_STRINGPROP(DamageFilename, "")
	ADD_STRINGPROP(DamageSkin, "")
	ADD_VECTORPROP(DamageDims)			//SCHLEGZ: added damaged state dims
	ADD_STRINGPROP(DestroyFilename, "")
	ADD_STRINGPROP(DestroySkin, "")
	ADD_VECTORPROP(DestroyDims)
	ADD_BOOLPROP( DestroyVisible, DTRUE)
	ADD_BOOLPROP( DestroySolid, DTRUE)
	ADD_BOOLPROP( DestroyGravity, DTRUE)
	ADD_REALPROP(HitPoints, 100.0f)
	ADD_REALPROP(DamageHitPoints, 50.0f)
	ADD_REALPROP(ObjectMass, 100.0f)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_BOOLPROP(Destructable, DTRUE)
	ADD_BOOLPROP(Pushable, DFALSE)
	ADD_LONGINTPROP(SurfaceType, SURFTYPE_UNKNOWN)
	ADD_REALPROP( ModelScale, 1.0f )
	ADD_COLORPROP(TintColor, 0.0f, 0.0f, 0.0f) 
	ADD_BOOLPROP(Chrome, 0)
	ADD_STRINGPROP(SlidingSound, "")
	ADD_DEBRIS_AGGREGATE()
END_CLASS_DEFAULT(CDestructableModel, B2BaseClass, NULL, NULL)

#define PUSHSOUNDEXTRATIME		0.1f
#define STANDINGONCHECKTIME		0.1f

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::CDestructableModel
//
//	PURPOSE:	constructor
//
// --------------------------------------------------------------------------- //

CDestructableModel::CDestructableModel() : B2BaseClass(OT_MODEL) 
{ 
	AddAggregate(&m_damage);
	AddAggregate(&m_Debris);

	m_fInitHitPoints		= 100; 
	m_fDamagedHitPoints		= 50;
	m_fMass					= 100;

	m_fAlpha				= 1.0f;
	m_fTargetAlpha			= 1.0f;

	m_bDeadState			= DFALSE;

	VEC_INIT(m_InitDims);
	VEC_INIT(m_DamageDims);		//SCHLEGZ: added damaged state dims
	VEC_INIT(m_DestroyDims);
	m_dwDestroyFlags		= 0;

	m_hstrDamagedFilename	= DNULL;
	m_hstrDestroyFilename	= DNULL;
	m_hstrDamagedSkinName	= DNULL;
	m_hstrDestroySkinName	= DNULL;

	m_bDestroyVisible		= DTRUE;
	m_bDestroySolid			= DTRUE;
	m_bDestroyGravity		= DTRUE;
	m_bDestructable			= DTRUE;
	m_bPushable				= DFALSE;
	m_dwSurfType			= SURFTYPE_UNKNOWN;
	m_bChrome				= DFALSE;
	m_fScale				= 1.0f;
	VEC_SET(m_vTintColor, 0.0f, 0.0f, 0.0f);
	m_fAlphaFadeRate		= 0.0f;
	m_fLastTime				= 0.0f;

	m_bSliding				= DFALSE;
	m_hstrSlidingSound		= DNULL;
	m_hSlidingSound			= DNULL;
	VEC_INIT( m_vLastPos );
	m_nSlidingFrameCounter	= 0;
	m_bStandingOn			= DFALSE;
	m_nStandingOnFrameCounter = 0;

	m_fYaw					= 0;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::~CDestructableModel
//
//	PURPOSE:	destructor
//
// --------------------------------------------------------------------------- //

CDestructableModel::~CDestructableModel()
{ 
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDamagedFilename)
		pServerDE->FreeString(m_hstrDamagedFilename);

	if (m_hstrDamagedSkinName)
		pServerDE->FreeString(m_hstrDamagedSkinName);

	if (m_hstrDestroyFilename)
		pServerDE->FreeString(m_hstrDestroyFilename);

	if (m_hstrDestroySkinName)
		pServerDE->FreeString(m_hstrDestroySkinName);
	
	if (m_hstrSlidingSound)
		pServerDE->FreeString(m_hstrSlidingSound);

	if( m_hSlidingSound )
		g_pServerDE->KillSound( m_hSlidingSound );

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::ReadProp()
//
//	PURPOSE:	Reads properties
//
// --------------------------------------------------------------------------- //

DBOOL CDestructableModel::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE || !pStruct) return DFALSE;

	GenericProp genProp;

	// Read in the destroyed object props.
	if (g_pServerDE->GetPropGeneric("InitFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) _mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("InitSkin", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) _mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("InitDims", &genProp) == DE_OK)
	{
		VEC_COPY(m_InitDims, genProp.m_Vec);
	}

	// Read in the damaged object props.
	if (g_pServerDE->GetPropGeneric("DamageFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrDamagedFilename = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DamageSkin", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrDamagedSkinName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DamageDims", &genProp) == DE_OK)
	{
		VEC_COPY(m_DamageDims, genProp.m_Vec);
	}

 	// Read in the destroyed object props.
	if (g_pServerDE->GetPropGeneric("DestroyFilename", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrDestroyFilename = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DestroySkin", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrDestroySkinName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DestroyDims", &genProp) == DE_OK)
	{
		VEC_COPY(m_DestroyDims, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("DestroyVisible", &genProp) == DE_OK)
	{
		m_bDestroyVisible = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("DestroySolid", &genProp) == DE_OK)
	{
		m_bDestroySolid = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("DestroyGravity", &genProp) == DE_OK)
	{
		m_bDestroyGravity = genProp.m_Bool;
	}

	m_dwDestroyFlags =  (m_bDestroyVisible ? FLAG_VISIBLE : 0) | 
						(m_bDestroySolid ? FLAG_SOLID : 0) |
						(m_bDestroyGravity ? FLAG_GRAVITY : 0);

	if (g_pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
	{
		m_fInitHitPoints = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("DamageHitPoints", &genProp) == DE_OK)
	{
		m_fDamagedHitPoints = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("ObjectMass", &genProp) == DE_OK)
	{
		m_fMass = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Alpha", &genProp) == DE_OK)
	{
		m_fAlpha = m_fTargetAlpha = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("Pushable", &genProp) == DE_OK)
	{
		m_bPushable = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("SurfaceType", &genProp) == DE_OK)
	{
		m_dwSurfType = (DDWORD)genProp.m_Long;
	}

	if (g_pServerDE->GetPropGeneric("Destructable", &genProp) == DE_OK)
	{
		m_bDestructable = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric( "Chrome", &genProp ) == DE_OK)
		m_bChrome = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("ModelScale", &genProp) == DE_OK)
	{
		 m_fScale = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("TintColor", &genProp) == DE_OK)
	{
		VEC_COPY(m_vTintColor, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("SlidingSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrSlidingSound = g_pServerDE->CreateString(genProp.m_String);
	}

/*	if (g_pServerDE->GetPropGeneric("Rotation", &genProp) == DE_OK)
	{
		m_fYaw = genProp.m_Vec.y;
	}
*/
	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::EngineMessageFn()
//
//	PURPOSE:	Handles engine messages.
//
// --------------------------------------------------------------------------- //

DDWORD CDestructableModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	DDWORD dwRet;
	CServerDE* pServerDE = GetServerDE();

	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update( );
		}
		break;

		case MID_PRECREATE:
		{
			dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp(pStruct);

			if (m_bChrome)
			{
				pStruct->m_Flags |= FLAG_ENVIRONMENTMAP;
			}

			if (m_bPushable)
			{
				pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_GRAVITY;
			}

			if (fData == PRECREATE_STRINGPROP)
			{
				pStruct->m_Flags |= FLAG_GRAVITY | FLAG_SOLID | FLAG_SHADOW | FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_MODELGOURAUDSHADE;
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			InitialUpdate(( DDWORD )fData );

			return dwRet;
		}
		break;

		case MID_TOUCHNOTIFY:
        {
			TouchNotify(( HOBJECT )pData, fData );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::InitialUpdate
//
//	PURPOSE:	Initial update.
//
// ----------------------------------------------------------------------- //

void CDestructableModel::InitialUpdate( DDWORD nData )
{
	DRotation rRot;

	if (nData != INITIALUPDATE_SAVEGAME)
	{
		// Set object translucency...
		DVector vTintColor;
		VEC_DIVSCALAR(vTintColor, m_vTintColor, 255.0f);
		g_pServerDE->SetObjectColor(m_hObject, vTintColor.x, vTintColor.y, 
									vTintColor.z, m_fAlpha);

		m_damage.Init(m_hObject);
		m_damage.SetMaxHitPoints( m_fInitHitPoints );
		m_damage.SetHitPoints(m_fInitHitPoints);
		m_damage.SetMass(m_fMass);

		m_Debris.Init(m_hObject);
		// If no damage models, don't allow damage hitpoints
		if (!m_hstrDamagedFilename || !m_hstrDamagedSkinName)
		{
			m_fDamagedHitPoints = 0.0f;
		}
		m_damage.SetApplyDamagePhysics(m_bPushable);

		if (m_bPushable)
		{
			g_pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_PUSHABLE);
			// Mark this object as moveable
			DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
			dwUsrFlags |= USRFLG_MOVEABLE;
			g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
			// Need to be told about all collisions...
			g_pServerDE->SetForceIgnoreLimit( m_hObject, 0.0f );
		}
		else
		{
			g_pServerDE->SetBlockingPriority(m_hObject, BLOCKPRIORITY_NONPUSHABLE);
		}

		// Scale the model.  If the dims property wasn't set, then scale the dims too...
		DVector vScale;
		VEC_SET( vScale, m_fScale, m_fScale, m_fScale );
		g_pServerDE->ScaleObject( m_hObject, &vScale );

		// Try to determine dims based on rotation if InitDims isn't set
		if( VEC_MAGSQR( m_InitDims ) == 0.0f )
		{
			g_pServerDE->GetModelAnimUserDims(m_hObject, &m_InitDims, 0);
			g_pServerDE->GetObjectRotation( m_hObject, &rRot );
			if( VEC_MAGSQR( rRot.m_Vec ) > 0.0f )
			{
				RotateDims( &rRot, &m_InitDims);
			}
			VEC_MULSCALAR( m_InitDims, m_InitDims, m_fScale );
		}

		g_pServerDE->SetObjectDims(m_hObject, &m_InitDims);

		// If the object is less than 10x10x10, then turn off shadows, because the 
		// shadows will be visible through model...
		if( VEC_MAGSQR( m_InitDims ) < 1000.0f )
		{
			DDWORD dwFlags;
			dwFlags = g_pServerDE->GetObjectFlags( m_hObject );
			dwFlags &= ~FLAG_SHADOW;
			g_pServerDE->SetObjectFlags( m_hObject, dwFlags );
		}

		// Mark this object as savable
		DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
		dwFlags |= USRFLG_SAVEABLE;

		if (m_bPushable)
		{
			dwFlags |= USRFLG_SINGULARITY_ATTRACT;
		}

		dwFlags |= (m_dwSurfType << 24);
		
		g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

		g_pServerDE->SetDeactivationTime( m_hObject, 1.0f );

		if( !m_hstrSlidingSound )
			m_hstrSlidingSound = g_pServerDE->CreateString( GetSlidingSound( ));

		// Set damage delay for cascading items
		if (m_Debris.IsExploding())
		{
			m_damage.SetDeathDelay(0.25f);
		}

		HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(this);
		g_pServerDE->WriteToMessageByte( hMessage, SFX_DESTRUCTABLEMODEL );
		g_pServerDE->WriteToMessageCompVector( hMessage, &m_InitDims );
		g_pServerDE->EndMessage(hMessage);
	}
	if (nData == INITIALUPDATE_WORLDFILE)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
		if (dwFlags & FLAG_GRAVITY)
			MoveObjectToGround(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::Update
//
//	PURPOSE:	Initial update.
//
// ----------------------------------------------------------------------- //

void CDestructableModel::Update( )
{
	DFLOAT fTime, fDeltaTime, fAlphaAdjust;
	DVector vPos;
	CollisionInfo colInfo;
	char *pszSlidingSound;
	DBOOL bSlid, bUpdateAgain;

	fTime = g_pServerDE->GetTime( );
	bUpdateAgain = DFALSE;

	// Waiting for death..
	if (m_damage.GetHitPoints() <= 0)
	{
		if (m_damage.IsDead())
			HandleDestruction();
		else
			g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
	}

	// Update the frame counters...
	if( m_nSlidingFrameCounter > 0 )
		m_nSlidingFrameCounter--;
	if( m_nStandingOnFrameCounter > 0 )
		m_nStandingOnFrameCounter--;

	// Check if sound is playing because object is sliding...
	if( m_bSliding )
	{
		g_pServerDE->GetObjectPos( m_hObject, &vPos );

		// Check if still moving...
		bSlid = DFALSE;
		if( VEC_DISTSQR( vPos, m_vLastPos ) > 0.1f )
		{
			VEC_COPY( m_vLastPos, vPos );

			// Check if still on something...
			if( m_bStandingOn )
			{
				// Start a sound, if it's not going already...
				if( !m_hSlidingSound )
				{
					pszSlidingSound = g_pServerDE->GetStringData( m_hstrSlidingSound );
					if( pszSlidingSound )
					{
						m_hSlidingSound = PlaySoundFromObject( m_hObject, pszSlidingSound, 1000.0f, SOUNDPRIORITY_MISC_LOW, DTRUE, DTRUE );
					}
				}

				bSlid = DTRUE;
				
				// Don't stop the sliding sound for at least a little 
				// time after this...
				m_nSlidingFrameCounter = 2;
				bUpdateAgain = DTRUE;
			}
		}

		// Not sliding any more...
		if( !bSlid && m_nSlidingFrameCounter == 0 )
		{
			m_bSliding = DFALSE;
			if( m_hSlidingSound )
			{
				g_pServerDE->KillSound( m_hSlidingSound );
				m_hSlidingSound = DNULL;
			}
		}
		else
		{
			bUpdateAgain = DTRUE;
		}
	}

	// Check if still standing on something...
	if( m_bStandingOn )
	{
		// Check if standing on something...
		g_pServerDE->GetStandingOn( m_hObject, &colInfo );
		if( !colInfo.m_hObject && m_nStandingOnFrameCounter == 0 )
		{
			m_bStandingOn = DFALSE;
		}
	}

	// Adjust the alpha...
	if( fabs( m_fAlpha - m_fTargetAlpha ) > 0.0001f )
	{
		fDeltaTime = fTime - m_fLastTime;
		m_fLastTime = fTime;

		fAlphaAdjust = m_fAlphaFadeRate * fDeltaTime;
		
		// Don't pass the target up...
		if( fabs( fAlphaAdjust ) >= fabs( m_fTargetAlpha - m_fAlpha ))
		{
			m_fAlpha = m_fTargetAlpha;
		}
		else
		{
			m_fAlpha += fAlphaAdjust;
			bUpdateAgain = DTRUE;
		}

		// Set object translucency...
		DVector vTintColor;
		VEC_DIVSCALAR(vTintColor, m_vTintColor, 255.0f);
		g_pServerDE->SetObjectColor(m_hObject, vTintColor.x, vTintColor.y, 
									vTintColor.z, m_fAlpha);

	}

	// Schedule next update...
	if( bUpdateAgain )
	{
		g_pServerDE->SetNextUpdate( m_hObject, 0.001f );
		g_pServerDE->SetDeactivationTime( m_hObject, 1.0f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::TouchNotify
//
//	PURPOSE:	Handles getting touched by something.
//
// ----------------------------------------------------------------------- //

void CDestructableModel::TouchNotify( HOBJECT hObj, DFLOAT fForce )
{
	CollisionInfo colInfo;
	DBOOL bDoUpdate;

	bDoUpdate = DFALSE;

	// If a ai or player is pushing us, and we are on the ground, then
	// start playing a sliding sound...
	if( !m_bSliding && IsBaseCharacter( hObj ))
	{
		m_bSliding = DTRUE;
		m_nSlidingFrameCounter = 2;
		bDoUpdate = DTRUE;
	}

	// Check if standing on something.  If not, then we'll catch that in the update..
	g_pServerDE->GetStandingOn( m_hObject, &colInfo );
	if( fabs( fForce ) > 0.0f && colInfo.m_hObject )
	{
		m_bStandingOn = DTRUE;
		m_nStandingOnFrameCounter = 2;
		bDoUpdate = DTRUE;
	}

	// Schedule next update...
	if( bDoUpdate )
	{
		g_pServerDE->SetNextUpdate( m_hObject, 0.001f );
		g_pServerDE->SetDeactivationTime( m_hObject, 1.0f );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::ObjectMessageFn()
//
//	PURPOSE:	Processes a message from a server object.
//
// --------------------------------------------------------------------------- //

DDWORD CDestructableModel::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch(messageID)
	{
			
		case MID_DAMAGE:
		{
			DDWORD dwRet = B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
			HandleDamage();

			return dwRet;

			// Set an update so we can check for damage.
			//pServerDE->SetNextUpdate(m_hObject, 0.001f);
			break;
		}

		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char *pMsg = g_pServerDE->GetStringData(hMsg);

			HandleTriggerMessage( pMsg );

			pServerDE->FreeString(hMsg);

			break;
		}
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::GetSlidingSound
//
//	PURPOSE:	Choose the sliding sound based on the debris type.
//
// ----------------------------------------------------------------------- //

char *CDestructableModel::GetSlidingSound( )
{
	char *pszSlidingSound;

	pszSlidingSound = DNULL;
	switch( m_Debris.GetType( ))
	{
		case SURFTYPE_STONE:
			pszSlidingSound = "sounds\\events\\pushstone.wav";
			break;
		case SURFTYPE_METAL:
			pszSlidingSound = "sounds\\events\\pushmetal.wav";
			break;
		case SURFTYPE_WOOD:
			pszSlidingSound = "sounds\\events\\pushwood.wav";
			break;
		case SURFTYPE_TERRAIN:
			pszSlidingSound = "sounds\\events\\pushterrain.wav";
			break;
		case SURFTYPE_PLASTIC:
			pszSlidingSound = "sounds\\events\\pushplastic.wav";
			break;
		case SURFTYPE_GLASS:
			pszSlidingSound = "sounds\\events\\pushglass.wav";
			break;
		case SURFTYPE_FLESH:
			pszSlidingSound = "sounds\\events\\pushflesh.wav";
			break;
		case SURFTYPE_LIQUID:
			pszSlidingSound = "sounds\\events\\pushliquid.wav";
			break;
		case SURFTYPE_ENERGY:
			pszSlidingSound = "sounds\\events\\pushenergy.wav";
			break;
		default:
			pszSlidingSound = "sounds\\events\\pushwood.wav";
			break;
			
	}

	return pszSlidingSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructableModel::HandleTriggerMessage( char *pMsg )
{
	int nArgs;
    
    char tokenSpace[64*20];
    char *pTokens[64];
    char *pCommand, *pCommandPos;
	DFLOAT fFadeTime;
	DBOOL bMore;

	pCommand = pMsg;
	bMore = DTRUE;
	while( bMore )
	{
		bMore = g_pServerDE->Parse( pMsg, &pCommandPos, tokenSpace, pTokens, &nArgs );

		// Handle the alpha change command...
		if( _mbsicmp((const unsigned char*) pTokens[0], (const unsigned char*)"Alpha" ) == 0 && nArgs > 1 )
		{
			// Get the new alpha value...
			m_fTargetAlpha = ( DFLOAT )atof( pTokens[1] );

			// Get the optional alpha fade time...
			if( nArgs == 3 )
				fFadeTime = ( DFLOAT )atof( pTokens[2] );
			else
				fFadeTime = 0.0f;

			if( fFadeTime > 0.0f )
				m_fAlphaFadeRate = ( m_fTargetAlpha - m_fAlpha ) / fFadeTime;
			else
				m_fAlphaFadeRate = 1.0e10;

			m_fLastTime = g_pServerDE->GetTime( );
			g_pServerDE->SetNextUpdate( m_hObject, 0.01f );
			g_pServerDE->SetDeactivationTime( m_hObject, 1.0f );
		}

		pCommand = pCommandPos;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::HandleDamage()
//
//	PURPOSE:	Processes a damage message...
//
// --------------------------------------------------------------------------- //

void CDestructableModel::HandleDamage()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (!m_bDeadState && m_bDestructable)
	{
		if (m_damage.GetHitPoints() <= 0)  // m_damage.IsDead())
		{
			// Set next update so we can wait for the cascade delay.
			pServerDE->SetNextUpdate(m_hObject, 0.001f);
//			HandleDestruction();
		}
		else if (m_fDamagedHitPoints > 0 && m_damage.GetHitPoints() <= m_fDamagedHitPoints)
		{
			if (m_hstrDamagedFilename || m_hstrDamagedSkinName)
				pServerDE->SetModelFilenames(m_hObject,
											 pServerDE->GetStringData(m_hstrDamagedFilename), 
											 pServerDE->GetStringData(m_hstrDamagedSkinName));
			DVector vScale;
			VEC_SET( vScale, m_fScale, m_fScale, m_fScale );
			pServerDE->ScaleObject( m_hObject, &vScale );
			if( VEC_MAG( m_DamageDims ) == 0.0f )
			{
				pServerDE->GetModelAnimUserDims(m_hObject, &m_DamageDims, 0);
				VEC_MULSCALAR( m_DamageDims, m_DamageDims, m_fScale );
			}

			pServerDE->SetObjectDims(m_hObject,&m_DamageDims);	//SCHLEGZ: added damaged dims
			// If the object is less than 20x20x20, then turn off shadows, because the 
			// shadows will be visible through model...
			if( VEC_MAGSQR( m_DamageDims ) < 8000.0f )
			{
				DDWORD dwFlags;
				dwFlags = g_pServerDE->GetObjectFlags( m_hObject );
				dwFlags &= ~FLAG_SHADOW;
				g_pServerDE->SetObjectFlags( m_hObject, dwFlags );
			}
			m_fDamagedHitPoints = 0;

			HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
			pServerDE->WriteToMessageByte( hMessage, SFX_DESTRUCTABLEMODEL );
			pServerDE->WriteToMessageCompVector( hMessage, &m_DamageDims );
			pServerDE->EndMessage(hMessage);
		}
	}

}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::HandleDestruction()
//
//	PURPOSE:	Handles the object's destruction
//
// --------------------------------------------------------------------------- //

void CDestructableModel::HandleDestruction()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_bDeadState = DTRUE;

	// No destroyed filenames, so remove
	if (!m_hstrDestroyFilename && !m_hstrDestroySkinName)
	{
		pServerDE->RemoveObject(m_hObject);
	}
	else
	{
		pServerDE->SetModelFilenames(m_hObject, 
									 pServerDE->GetStringData(m_hstrDestroyFilename), 
									 pServerDE->GetStringData(m_hstrDestroySkinName));
		DVector vScale;
		VEC_SET( vScale, m_fScale, m_fScale, m_fScale );
		pServerDE->ScaleObject( m_hObject, &vScale );
		pServerDE->SetObjectFlags(m_hObject, m_dwDestroyFlags);

		if (VEC_MAG(m_DestroyDims) == 0.0f)
		{
			pServerDE->GetModelAnimUserDims(m_hObject, &m_DestroyDims, 0);
			VEC_MULSCALAR( m_DestroyDims, m_DestroyDims, m_fScale );
		}

		pServerDE->SetObjectDims(m_hObject, &m_DestroyDims);
		// If the object is less than 20x20x20, then turn off shadows, because the 
		// shadows will be visible through model...
		if( VEC_MAGSQR( m_DestroyDims ) < 8000.0f )
		{
			DDWORD dwFlags;
			dwFlags = g_pServerDE->GetObjectFlags( m_hObject );
			dwFlags &= ~FLAG_SHADOW;
			g_pServerDE->SetObjectFlags( m_hObject, dwFlags );
		}

		HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte( hMessage, SFX_DESTRUCTABLEMODEL );
		pServerDE->WriteToMessageCompVector( hMessage, &m_DestroyDims );
		pServerDE->EndMessage(hMessage);
	}
	// Create the debris
	DVector vDir;
	m_damage.GetLastDamageDirection(&vDir);
	m_Debris.Create(vDir, m_damage.GetLastDamageAmount());
}

			
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::RotateDims
//
//	PURPOSE:	Try to adjust the dims based on the rotation in m_fYaw
//
// ----------------------------------------------------------------------- //

void CDestructableModel::RotateDims( DRotation *prRot, DVector *pvDims )
{
	DVector vPt[8], vDims;
	DMatrix mMat;
	int i;
	float fVal;

	g_pServerDE->SetupRotationMatrix( &mMat, prRot );

	VEC_SET( vPt[0],  pvDims->x,  pvDims->y,  pvDims->z );
	VEC_SET( vPt[1],  pvDims->x,  pvDims->y, -pvDims->z );
	VEC_SET( vPt[2],  pvDims->x, -pvDims->y,  pvDims->z );
	VEC_SET( vPt[3],  pvDims->x, -pvDims->y, -pvDims->z );
	VEC_SET( vPt[4], -pvDims->x,  pvDims->y,  pvDims->z );
	VEC_SET( vPt[5], -pvDims->x,  pvDims->y, -pvDims->z );
	VEC_SET( vPt[6], -pvDims->x, -pvDims->y,  pvDims->z );
	VEC_SET( vPt[7], -pvDims->x, -pvDims->y, -pvDims->z );

	VEC_INIT( vDims );
	for( i = 0; i < 8; i++ )
	{
		MatVMul_InPlace( &mMat, &vPt[i] );
		fVal = ( float )fabs( vPt[i].x );
		if( vDims.x < fVal )
			vDims.x = fVal;
		fVal = ( float )fabs( vPt[i].y );
		if( vDims.y < fVal )
			vDims.y = fVal;
		fVal = ( float )fabs( vPt[i].z );
		if( vDims.z < fVal )
			vDims.z = fVal;
	}

	VEC_COPY( *pvDims, vDims );
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructableModel::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveGame)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrDamagedFilename);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamagedSkinName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyFilename);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroySkinName);

	pServerDE->WriteToMessageVector(hWrite, &m_InitDims);
	pServerDE->WriteToMessageVector(hWrite, &m_DamageDims);
	pServerDE->WriteToMessageVector(hWrite, &m_DestroyDims);

	pServerDE->WriteToMessageDWord(hWrite, m_dwDestroyFlags);
	pServerDE->WriteToMessageByte(hWrite, m_bDeadState);
	pServerDE->WriteToMessageFloat(hWrite, m_fInitHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamagedHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageFloat(hWrite, m_fAlpha);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);

	pServerDE->WriteToMessageByte(hWrite, m_bDestroyVisible);
	pServerDE->WriteToMessageByte(hWrite, m_bDestroySolid);
	pServerDE->WriteToMessageByte(hWrite, m_bDestroyGravity);
	pServerDE->WriteToMessageByte(hWrite, m_bDestructable);
	pServerDE->WriteToMessageByte(hWrite, m_bPushable);
	pServerDE->WriteToMessageDWord(hWrite, m_dwSurfType);

	pServerDE->WriteToMessageByte(hWrite, m_bChrome);
	pServerDE->WriteToMessageFloat( hWrite, m_fScale );
	pServerDE->WriteToMessageVector(hWrite, &m_vTintColor);
	pServerDE->WriteToMessageFloat(hWrite, m_fTargetAlpha);
	pServerDE->WriteToMessageFloat(hWrite, m_fAlphaFadeRate);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSlidingSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructableModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructableModel::Load(HMESSAGEREAD hRead, DDWORD dwLoadGame)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrDamagedFilename	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDamagedSkinName	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroyFilename	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroySkinName	= pServerDE->ReadFromMessageHString(hRead);

	pServerDE->ReadFromMessageVector(hRead, &m_InitDims);
	pServerDE->ReadFromMessageVector(hRead, &m_DamageDims);
	pServerDE->ReadFromMessageVector(hRead, &m_DestroyDims);
	m_dwDestroyFlags		= pServerDE->ReadFromMessageDWord(hRead);
	m_bDeadState			= pServerDE->ReadFromMessageByte(hRead);
	m_fInitHitPoints		= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamagedHitPoints		= pServerDE->ReadFromMessageFloat(hRead);
	m_fMass					= pServerDE->ReadFromMessageFloat(hRead);
	m_fAlpha				= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw					= pServerDE->ReadFromMessageFloat(hRead);

	m_bDestroyVisible		= pServerDE->ReadFromMessageByte(hRead);
	m_bDestroySolid			= pServerDE->ReadFromMessageByte(hRead);
	m_bDestroyGravity		= pServerDE->ReadFromMessageByte(hRead);
	m_bDestructable			= pServerDE->ReadFromMessageByte(hRead);
	m_bPushable				= pServerDE->ReadFromMessageByte(hRead);
	m_dwSurfType			= pServerDE->ReadFromMessageDWord(hRead);

	m_bChrome				= pServerDE->ReadFromMessageByte(hRead);
	m_fScale				= pServerDE->ReadFromMessageFloat( hRead );
	pServerDE->ReadFromMessageVector(hRead, &m_vTintColor);

	m_fTargetAlpha			= pServerDE->ReadFromMessageFloat(hRead);
	m_fAlphaFadeRate		= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTime				= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrSlidingSound		= pServerDE->ReadFromMessageHString(hRead);
}



