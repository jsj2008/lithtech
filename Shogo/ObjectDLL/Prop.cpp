// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.cpp
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// ----------------------------------------------------------------------- //

#include "Prop.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "RiotMsgIds.h"
#include "Powerup.h"
#include "Spawner.h"
#include "DebrisFuncs.h"
#include "DebrisTypes.h"
#include "Weapons.h"

BEGIN_CLASS(Prop)
	ADD_ACTIVATION_AGGREGATE()
	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_VISIBLE_FLAG( 1, 0 )
	ADD_SOLID_FLAG( 1, 0 )
	ADD_GRAVITY_FLAG( 1, 0 )
	ADD_SHADOW_FLAG( 0, 0 )
	ADD_BOOLPROP(MoveToFloor, DTRUE)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_COLORPROP(ModelAdd, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP_FLAG( Filename, "", PF_DIMS | PF_LOCALDIMS )
	ADD_STRINGPROP( Skin, "" )
	ADD_STRINGPROP( Masses, "2000" )
	ADD_STRINGPROP( HitPoints, "100" )
	ADD_STRINGPROP( ArmorPoints, "100" )
	ADD_STRINGPROP( DestroyedModels, "" )
	ADD_STRINGPROP( DestroyedSkins, "" )
	ADD_STRINGPROP( DestroyedSounds, "" )
	ADD_REALPROP_FLAG( SoundRadius, 1000.0f, PF_RADIUS )
	ADD_REALPROP( MinNumDebris, 10 )
	ADD_REALPROP( MaxNumDebris, 20 )
	ADD_LONGINTPROP(DebrisType, DBT_METAL_BIG)
	ADD_STRINGPROP( Spawn, "" )
	ADD_REALPROP(Translucency, 1.0f)
	ADD_COLORPROP(TintColor, 0.0f, 0.0f, 0.0f) 
	ADD_BOOLPROP(Chrome, 0)

	PROP_DEFINEGROUP(ExplosionStuff, PF_GROUP1)
		ADD_BOOLPROP_FLAG(CreateExplosion, 0, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(WeaponId, GUN_BULLGUT_ID, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(ExplosionSize, MS_NORMAL, PF_GROUP1)
		ADD_BOOLPROP_FLAG(FireAlongForward, 0, PF_GROUP1)
		ADD_REALPROP_FLAG(DamageFactor, 1.0f, PF_GROUP1)

END_CLASS_DEFAULT(Prop, BaseClass, NULL, NULL)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Prop()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prop::Prop() : BaseClass(OT_MODEL)
{
	m_hstrDestroyedModels = DNULL;
	m_hstrDestroyedSkins = DNULL;
	m_fMass = 100.0f;
	m_fHitPoints = 100.0f;
	m_fArmorPoints = 100.0f;
	m_hstrDestroyedSounds = DNULL;
	m_fSoundRadius = 1000.0f;
	m_nMinNumDebris = 10;
	m_nMaxNumDebris = 20;
	m_hstrSpawn = DNULL;
	m_fLife = 0.0f;
	m_bFirstUpdate = DTRUE;
	m_bMoveToFloor = DTRUE;
	m_bDamageDone  = DFALSE;

	m_hstrMasses = DNULL;
	m_hstrHitPoints = DNULL;
	m_hstrArmorPoints = DNULL;

	m_bActive = DTRUE;
	m_bChrome = DFALSE;


	m_eSurfaceType = ST_METAL;
	m_eDebrisType  = DBT_METAL_BIG;
	m_bReadProp	   = DFALSE;

	VEC_SET(m_vScale, 1.0f, 1.0f, 1.0f);
	VEC_INIT(m_vModelAdd);

	VEC_SET(m_vTintColor, 0.0f, 0.0f, 0.0f) 
	m_fTranslucency = 1.0f;
	
	AddAggregate(&m_Damage);
	AddAggregate(&m_activation);

	// Explosion stuff..

	m_bCreateExplosion		= DFALSE;
	m_nExplosionWeaponId	= GUN_BULLGUT_ID;
	m_eExplosionSize		= MS_NORMAL;
	m_bFireAlongForward		= DFALSE;
	m_fDamageFactor			= 1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::~Prop()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Prop::~Prop()
{
	if( !g_pServerDE )
		return;

	if( m_hstrDestroyedSounds )
		g_pServerDE->FreeString( m_hstrDestroyedSounds );

	if( m_hstrDestroyedModels )
		g_pServerDE->FreeString( m_hstrDestroyedModels );

	if( m_hstrDestroyedSkins )
		g_pServerDE->FreeString( m_hstrDestroyedSkins );

	if( m_hstrMasses )
		g_pServerDE->FreeString( m_hstrMasses );

	if( m_hstrHitPoints )
		g_pServerDE->FreeString( m_hstrHitPoints );

	if( m_hstrArmorPoints )
		g_pServerDE->FreeString( m_hstrArmorPoints );
	
	if( m_hstrSpawn )
		g_pServerDE->FreeString( m_hstrSpawn );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Prop::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{	
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData; 
				ReadProp(pStruct);

				// If this prop is spawned, assume it should be visible (if they
				// specify Visible 0, our parent class will handle it ;)

				if (fData == PRECREATE_STRINGPROP)
				{
					pStruct->m_Flags |= FLAG_VISIBLE;
				}
			}

			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);
			
			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
			}
			CacheFiles( );
			break;
		}

		case MID_UPDATE:
		{
			// Updates only happen once at the beginning and when
			// the life has expired...
			if (m_bFirstUpdate)
			{
				m_bFirstUpdate = DFALSE;

				if ( m_fLife > 0.0f )
				{
					g_pServerDE->SetNextUpdate( m_hObject, m_fLife );
				}
				else
				{
					g_pServerDE->SetNextUpdate( m_hObject, 0.0f );
				}
			}
			else
			{
				g_pServerDE->RemoveObject( m_hObject );
			}

			break;
		}

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


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Prop::ObjectMessageFn( HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead )
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			DDWORD ret;

			if (m_Damage.GetCanDamage())
			{
				// Let Destructable aggregate do its job first...
				ret = BaseClass::ObjectMessageFn(hSender, messageID, hRead);

				if (m_Damage.IsDead() && !m_bDamageDone) 
				{
					Damage();
					m_bDamageDone = DTRUE;
				}

				return ret;
			}
			break;
		}

		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
			g_pServerDE->FreeString(hMsg);
			break;
		}

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return;

	m_bReadProp = DTRUE;
	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("Translucency", &genProp) == DE_OK)
	{
		m_fTranslucency = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("TintColor", &genProp) == DE_OK)
	{
		VEC_COPY(m_vTintColor, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("DestroyedModels", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrDestroyedModels = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DestroyedSkins", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrDestroyedSkins = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("Masses", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrMasses = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrHitPoints = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("ArmorPoints", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrArmorPoints = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("DestroyedSounds", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrDestroyedSounds = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("SoundRadius", &genProp) == DE_OK)
	{
		 m_fSoundRadius = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("MoveToFloor", &genProp) == DE_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Scale", &genProp) == DE_OK)
	{
		 VEC_COPY(m_vScale, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("ModelAdd", &genProp) == DE_OK)
	{
		 VEC_COPY(m_vModelAdd, genProp.m_Vec);
	}

	if (g_pServerDE->GetPropGeneric("Spawn", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrSpawn = g_pServerDE->CreateString(genProp.m_String);
	}


	if (g_pServerDE->GetPropGeneric("DebrisType", &genProp) == DE_OK)
	{
		 m_eDebrisType = (DebrisType) genProp.m_Long;
	}

	m_eSurfaceType = GetDebrisSurfaceType(m_eDebrisType);


	if (g_pServerDE->GetPropGeneric( "MinNumDebris", &genProp ) == DE_OK)
		m_nMinNumDebris = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric( "MaxNumDebris", &genProp ) == DE_OK)
		m_nMaxNumDebris = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric( "CreateExplosion", &genProp ) == DE_OK)
		m_bCreateExplosion = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric( "WeaponId", &genProp ) == DE_OK)
		m_nExplosionWeaponId = (DBYTE)genProp.m_Long;

	if (g_pServerDE->GetPropGeneric( "ExplosionSize", &genProp ) == DE_OK)
		m_eExplosionSize = (ModelSize)genProp.m_Long;
	
	if (g_pServerDE->GetPropGeneric( "FireAlongForward", &genProp ) == DE_OK)
		m_bFireAlongForward = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric( "DamageFactor", &genProp ) == DE_OK)
		m_fDamageFactor = genProp.m_Float;

	if (g_pServerDE->GetPropGeneric( "Chrome", &genProp ) == DE_OK)
		m_bChrome = genProp.m_Bool;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct)
		return;

	// Remove if outside the world

	pStruct->m_Flags |= FLAG_REMOVEIFOUTSIDE;

	if (m_bChrome)
	{
		pStruct->m_Flags |= FLAG_ENVIRONMENTMAP;
	}


	AdjustFlags(pStruct);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::AdjustFlags()
//
//	PURPOSE:	Adjust the flags
//
// ----------------------------------------------------------------------- //

void Prop::AdjustFlags(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// If this prop is one that shouldn't stop the player, set the appropriate
	// flag...

	char* pClientNonSolidModels[] = 
	{
		"Models\\Props\\car1.abc",
		"Models\\Props\\car2.abc",
		"Models\\Props\\car3.abc",
		"Models\\Props\\car4.abc",
		"Models\\Props\\car5_b.abc"
	};

	int nNumModels = sizeof(pClientNonSolidModels) / sizeof(pClientNonSolidModels[0]);

	for (int i=0; i < nNumModels; i++)
	{
		if (stricmp(pClientNonSolidModels[i], pStruct->m_Filename) == 0)
		{
			pStruct->m_Flags |= FLAG_CLIENTNONSOLID;
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void Prop::InitialUpdate(DVector *pMovement)
{
	InitProperties( );

#if 0
	// Currently not supported.  Left in here for possible future
	// support...

	// See if we should adjust our model's color...

	if (m_vModelAdd.x != 0.0f || m_vModelAdd.y != 0.0f || m_vModelAdd.z != 0.0f)
	{
		DDWORD dwUserFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
		g_pServerDE->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_MODELADD);

		// Pack our model add color into the upper 3 bytes of the user data...

		DBYTE r = (DBYTE)m_vModelAdd.x;
		DBYTE g = (DBYTE)m_vModelAdd.y;
		DBYTE b = (DBYTE)m_vModelAdd.z;

		DDWORD dwData = g_pServerDE->GetObjectUserFlags(m_hObject);
		dwData = ((r<<24) | (g<<16) | (b<<8) | (dwData & 0x000F));
		g_pServerDE->SetObjectUserFlags(m_hObject, dwData);
	}
#endif


	DDWORD dwUsrFlgs = g_pServerDE->GetObjectUserFlags(m_hObject);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlgs | USRFLG_MOVEABLE | SurfaceToUserFlag(m_eSurfaceType));

	// Updates are triggered only when we die...
	g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::InitProperties()
//
//	PURPOSE:	Initialize the damage aggregate.
//
// ----------------------------------------------------------------------- //

void Prop::InitProperties()
{
	char szString[256];
	DVector vDims;

	// Set object translucency...

	VEC_DIVSCALAR(m_vTintColor, m_vTintColor, 255.0f/m_fTranslucency);
	g_pServerDE->SetObjectColor(m_hObject, m_vTintColor.x, m_vTintColor.y, 
								m_vTintColor.z, m_fTranslucency);


	// Set some attributes...
	if( m_hstrMasses )
	{
		if( GetNextToken( &m_hstrMasses, szString, 255 ))
			m_fMass = ( DFLOAT )atoi( szString );
	}
	if( m_hstrHitPoints )
	{
		if( GetNextToken( &m_hstrHitPoints, szString, 255 ))
			m_fHitPoints = ( DFLOAT )atoi( szString );
	}
	if( m_hstrArmorPoints )
	{
		if( GetNextToken( &m_hstrArmorPoints, szString, 255 ))
			m_fArmorPoints = ( DFLOAT )atoi( szString );
	}


	// If this object was a level placed prop, then read prop was called and the can
	// damage flag was set there.  Otherwise, we automatically determine the destructable state.
	if( !m_bReadProp )
	{
		if( m_fHitPoints > 0.0f )
			m_Damage.SetCanDamage( DTRUE );
		else
			m_Damage.SetCanDamage( DFALSE );
	}
	else
	{
		// Set the dims based on the current animation...
		g_pServerDE->GetModelAnimUserDims(m_hObject, &vDims, g_pServerDE->GetModelAnimation(m_hObject));
		//g_pServerDE->SetObjectDims(m_hObject, &vDims);

		DVector vNewDims;

		// Set object dims based on scale value...

		vNewDims.x = m_vScale.x * vDims.x;
		vNewDims.y = m_vScale.y * vDims.y;
		vNewDims.z = m_vScale.z * vDims.z;

		g_pServerDE->ScaleObject(m_hObject, &m_vScale);
		g_pServerDE->SetObjectDims(m_hObject, &vNewDims);
	

		// Make sure object starts on floor if the gravity flag is set...

		DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
		if (m_bMoveToFloor && (dwFlags & FLAG_GRAVITY)) 
		{
			MoveObjectToFloor(m_hObject);
		}

		// Now that it's a real object, it won't need readprops anymore...
		m_bReadProp = DFALSE;
	}

	// Setup damage stats...
	m_Damage.Reset( m_fHitPoints, m_fArmorPoints );
	m_Damage.SetMass( m_fMass );
	m_Damage.SetHitPoints( m_fHitPoints );
	m_Damage.SetMaxHitPoints( m_fHitPoints );
	m_Damage.SetArmorPoints( m_fArmorPoints );
	m_Damage.SetMaxArmorPoints( m_fArmorPoints );
	m_Damage.SetCanHeal( DFALSE );
	m_Damage.SetCanRepair( DFALSE );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Damage()
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void Prop::Damage()
{
	char szString[256];
	char szModel[_MAX_PATH+1], szSkin[_MAX_PATH+1];
	char *pszModel, *pszSkin;
	DVector vDims, vOriginalDims, vPos;

	if ( !m_Damage.GetCanDamage() )
		return;

	// Check if not dead yet...
	if ( !m_bActive )
		return;

	// Set prop as inactive as death stuff is taken care of.  This was necessary because
	// the explosion object created by this prop kept hurting damaging the prop thus creating
	// a loop.
	m_bActive = DFALSE;

	// Play sound...
	if( m_hstrDestroyedSounds )
	{
		if( GetNextToken( &m_hstrDestroyedSounds, szString, 255 ))
		{
			DVector vPos;
			g_pServerDE->GetObjectPos( m_hObject, &vPos );
			PlaySoundFromPos( &vPos, szString, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM );
		}
	}

	SpawnItem( );
	
	g_pServerDE->GetObjectDims( m_hObject, &vDims );
	g_pServerDE->GetObjectPos( m_hObject, &vPos );

	CreateDebris(&vPos, &vDims);
	DoExplosion();

	// Object stays around if the skin or model is replaced when it is destroyed...
	if( m_hstrDestroyedSkins || m_hstrDestroyedModels )
	{
		// Get the original...
		g_pServerDE->GetModelFilenames( m_hObject, szModel, _MAX_PATH, szSkin, _MAX_PATH );
		pszModel = szModel;
		pszSkin = szSkin;

		// Use any new...
		if( m_hstrDestroyedSkins && GetNextToken( &m_hstrDestroyedSkins, szSkin, _MAX_PATH ))
			pszSkin = szSkin;
		if( m_hstrDestroyedModels && GetNextToken( &m_hstrDestroyedModels, szModel, _MAX_PATH ))
			pszModel = szModel;

		// Get the dims before the new model changes them...
		g_pServerDE->GetObjectDims( m_hObject, &vOriginalDims );

		// Set them...
		g_pServerDE->SetModelFilenames( m_hObject, pszModel, pszSkin );

		// Don't allow the new dims to be larger in any dimension...
		g_pServerDE->GetModelAnimUserDims( m_hObject, &vDims, g_pServerDE->GetModelAnimation( m_hObject ));

		if( vDims.x > vOriginalDims.x )
			vDims.x = vOriginalDims.x;
		if( vDims.y > vOriginalDims.y )
			vDims.y = vOriginalDims.y;
		if( vDims.z > vOriginalDims.z )
			vDims.z = vOriginalDims.z;
		g_pServerDE->SetObjectDims( m_hObject, &vDims );

		// Only spawn on the first prop...
		if( m_hstrSpawn )
		{
			g_pServerDE->FreeString( m_hstrSpawn );
			m_hstrSpawn = DNULL;
		}

		InitProperties( );

		m_bActive = DTRUE;
	}
	// Otherwise, remove the object...
	else
		g_pServerDE->RemoveObject( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::GetNextToken()
//
//	PURPOSE:	Internal function used to get the next token out of a
//				hString.  This function will kill an hString if it is
//				empty of tokens.
//
// ----------------------------------------------------------------------- //

DBOOL Prop::GetNextToken( HSTRING *hString, char *pszValue, int nLength )
{
	char *pszToken, *pszString;
	HSTRING hNewString = DNULL;

	if( !hString || !*hString)
		return DFALSE;

	pszString = g_pServerDE->GetStringData( *hString );
	if( !pszString && pszString[0] == '\0' )
	{
		g_pServerDE->FreeString( *hString );
		*hString = NULL;
		return DFALSE;
	}

	pszToken = strtok( pszString, " " );
	if( !pszToken )
	{
		g_pServerDE->FreeString( *hString );
		*hString = NULL;
		return DFALSE;
	}

	strncpy( pszValue, pszToken, nLength );
	pszValue[nLength] = '\0';

	pszString = strtok( NULL, "" );
	if( pszString )
		hNewString = g_pServerDE->CreateString( pszString );
	g_pServerDE->FreeString( *hString );
	*hString = hNewString;
	
	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::CreateDebris()
//
//	PURPOSE:	Create debris props
//
// ----------------------------------------------------------------------- //

void Prop::CreateDebris(DVector *pvPos, DVector *pvDims)
{
	if (!pvPos || !pvDims || m_nMaxNumDebris <= 0) return;

	DFLOAT fDimsMag = VEC_MAG(*pvDims) * 0.5f;
	::CreatePropDebris(*pvPos, fDimsMag, m_Damage.GetDeathDir(), 
						m_eDebrisType, m_nMinNumDebris, m_nMaxNumDebris);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::TriggerMsg()
//
//	PURPOSE:	Handler for prop trigger messages.
//
// --------------------------------------------------------------------------- //

void Prop::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg) return;


	if (_stricmp(pMsg, "FIRE") == 0)
	{
		DoExplosion();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::DoExplosion()
//
//	PURPOSE:	Handle doing explosion
//
// ----------------------------------------------------------------------- //

void Prop::DoExplosion()
{
	if (!m_bCreateExplosion) return;

	CWeapons weapons;
	weapons.Init(m_hObject, m_eExplosionSize);
	weapons.SetArsenal(CWeapons::AT_AS_NEEDED);
	weapons.ObtainWeapon(m_nExplosionWeaponId);
	weapons.ChangeWeapon(m_nExplosionWeaponId);
	weapons.AddAmmo(m_nExplosionWeaponId, GetWeaponMaxAmmo(m_nExplosionWeaponId));

	CWeapon* pWeapon = weapons.GetCurWeapon();
	if (!pWeapon) return;

	pWeapon->SetDamageFactor(m_fDamageFactor);
	pWeapon->SetCanLockOnTarget(DFALSE);
		
	DRotation rRot;
	g_pServerDE->GetObjectRotation(m_hObject, &rRot);

	DVector vU, vR, vF, vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
	g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Just blow up in place if we're not supposed to fire along
	// forward vector...

	if (!m_bFireAlongForward)
	{
		pWeapon->SetLifetime(0.0f);
		VEC_SET(vF, 0.0f, -1.0f, 0.0f);  // Fire down
	}

	pWeapon->Fire(m_hObject, vF, vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::SpawnItem()
//
//	PURPOSE:	Spawn an item
//
// ----------------------------------------------------------------------- //

void Prop::SpawnItem( )
{
	DVector vPos;
	DRotation rRot;
	char szSpawn[MAX_CS_FILENAME_LEN+1];

	if( !m_hstrSpawn )
		return;

	g_pServerDE->GetObjectPos( m_hObject, &vPos );
	g_pServerDE->GetObjectRotation( m_hObject, &rRot );

	strncpy( szSpawn, g_pServerDE->GetStringData( m_hstrSpawn ), MAX_CS_FILENAME_LEN );
	szSpawn[MAX_CS_FILENAME_LEN] = '\0';
	SpawnObject( szSpawn, &vPos, &rRot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::CacheFiles
//
//	PURPOSE:	Caches files used by prop.
//
// ----------------------------------------------------------------------- //

void Prop::CacheFiles( )
{
	int i;
	DVector v;

	// Don't cache if the world already loaded...
	if( !( g_pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	for( i = 0; i < GetNumDebrisModels( m_eDebrisType ); i++ )
	{
		g_pServerDE->CacheFile( FT_MODEL, GetDebrisModel( m_eDebrisType, v, i ));
	}
	g_pServerDE->CacheFile( FT_TEXTURE, GetDebrisSkin( m_eDebrisType ));
	for( i = 0; i < GetNumDebrisBounceSounds( m_eDebrisType ); i++ )
	{
		g_pServerDE->CacheFile( FT_SOUND, GetDebrisBounceSound( m_eDebrisType, i ));
	}
	for( i = 0; i < GetNumDebrisExplodeSounds( m_eDebrisType ); i++ )
	{
		g_pServerDE->CacheFile( FT_SOUND, GetDebrisExplodeSound( m_eDebrisType, i ));
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Prop::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedModels);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedSkins);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDestroyedSounds);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpawn);
	pServerDE->WriteToMessageHString(hWrite, m_hstrMasses);
	pServerDE->WriteToMessageHString(hWrite, m_hstrHitPoints);
	pServerDE->WriteToMessageHString(hWrite, m_hstrArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageFloat(hWrite, m_fHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fSoundRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fLife);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageFactor);
	pServerDE->WriteToMessageFloat(hWrite, m_fTranslucency);
	pServerDE->WriteToMessageByte(hWrite, m_nExplosionWeaponId);
	pServerDE->WriteToMessageByte(hWrite, m_nMinNumDebris);
	pServerDE->WriteToMessageByte(hWrite, m_nMaxNumDebris);
	pServerDE->WriteToMessageByte(hWrite, m_bFirstUpdate);
	pServerDE->WriteToMessageByte(hWrite, m_bActive);
	pServerDE->WriteToMessageByte(hWrite, m_bReadProp);
	pServerDE->WriteToMessageByte(hWrite, m_bMoveToFloor);
	pServerDE->WriteToMessageByte(hWrite, m_bDamageDone);
	pServerDE->WriteToMessageByte(hWrite, m_bChrome);
	pServerDE->WriteToMessageByte(hWrite, m_bCreateExplosion);
	pServerDE->WriteToMessageByte(hWrite, m_bFireAlongForward);
	pServerDE->WriteToMessageByte(hWrite, m_eSurfaceType);
	pServerDE->WriteToMessageByte(hWrite, m_eDebrisType);
	pServerDE->WriteToMessageByte(hWrite, m_eExplosionSize);
	pServerDE->WriteToMessageVector(hWrite, &m_vScale);
	pServerDE->WriteToMessageVector(hWrite, &m_vModelAdd);
	pServerDE->WriteToMessageVector(hWrite, &m_vTintColor);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Prop::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrDestroyedModels	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroyedSkins	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDestroyedSounds	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpawn				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrMasses			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrHitPoints			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrArmorPoints		= pServerDE->ReadFromMessageHString(hRead);
	m_fMass					= pServerDE->ReadFromMessageFloat(hRead);
	m_fHitPoints			= pServerDE->ReadFromMessageFloat(hRead);
	m_fArmorPoints			= pServerDE->ReadFromMessageFloat(hRead);
	m_fSoundRadius			= pServerDE->ReadFromMessageFloat(hRead);
	m_fLife					= pServerDE->ReadFromMessageFloat(hRead);
	m_fDamageFactor			= pServerDE->ReadFromMessageFloat(hRead);
	m_fTranslucency			= pServerDE->ReadFromMessageFloat(hRead);
	m_nExplosionWeaponId	= pServerDE->ReadFromMessageByte(hRead);
	m_nMinNumDebris			= pServerDE->ReadFromMessageByte(hRead);
	m_nMaxNumDebris			= pServerDE->ReadFromMessageByte(hRead);
	m_bFirstUpdate			= pServerDE->ReadFromMessageByte(hRead);
	m_bActive				= pServerDE->ReadFromMessageByte(hRead);
	m_bReadProp				= pServerDE->ReadFromMessageByte(hRead);
	m_bMoveToFloor			= pServerDE->ReadFromMessageByte(hRead);
	m_bDamageDone			= pServerDE->ReadFromMessageByte(hRead);
	m_bChrome				= pServerDE->ReadFromMessageByte(hRead);
	m_bCreateExplosion		= pServerDE->ReadFromMessageByte(hRead);
	m_bFireAlongForward		= pServerDE->ReadFromMessageByte(hRead);
	m_eSurfaceType			= (SurfaceType) pServerDE->ReadFromMessageByte(hRead);
	m_eDebrisType			= (DebrisType) pServerDE->ReadFromMessageByte(hRead);
	m_eExplosionSize		= (ModelSize) pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vScale);
	pServerDE->ReadFromMessageVector(hRead, &m_vModelAdd);
	pServerDE->ReadFromMessageVector(hRead, &m_vTintColor);
}

