// ----------------------------------------------------------------------- //
//
// MODULE  : GunMount.cpp
//
// PURPOSE : This object allows a gun to be attached to objects and controlled
//				through triggers.
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "GunMount.h"
#include "ParsedMsg.h"
#include "Arsenal.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"
#include "ServerUtilities.h"

LINKFROM_MODULE( GunMount );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_GUNMOUNT CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_GUNMOUNT 0

#endif

BEGIN_CLASS( GunMount )
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL, "" )
	ADD_STRINGPROP_FLAG(Weapon,	SELECTION_NONE, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Weapon type to use." )
	ADD_BOOLPROP(Hidden, false, "Object is hidden." )
END_CLASS_FLAGS_PLUGIN( GunMount, GameBase, CF_HIDDEN_GUNMOUNT, GunMountPlugin, "Allows placement of controllable weapon in level." )


// Register with the CommandMgr...

CMDMGR_BEGIN_REGISTER_CLASS( GunMount )
ADD_MESSAGE( FIRE,		2, NULL, MSG_HANDLER( GunMount, HandleFireMsg ),	"FIRE [rounds_to_fire]", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( BEGINFIRE, 1, NULL, MSG_HANDLER( GunMount, HandleFireMsg ),	"BEGINFIRE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( ENDFIRE,	1, NULL, MSG_HANDLER( GunMount, HandleFireMsg ),	"ENDFIRE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( VISIBLE,	2, NULL, MSG_HANDLER( GunMount, HandleVisibleMsg ), "VISIBLE <bool>", "Toggles whether the object is visible.", "msg GunMount (VISIBLE 1)<BR>msg GunMount (VISIBLE 0)" )
	ADD_MESSAGE( HIDDEN,	2, NULL, MSG_HANDLER( GunMount, HandleVisibleMsg ),	"HIDDEN <bool>", "Toggles whether the object is visible, solid, and rayhit.", "msg GunMount (HIDDEN 1)<BR>msg GunMount (HIDDEN 0)" )
CMDMGR_END_REGISTER_CLASS( GunMount, GameBase )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GunMount::GunMount
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

GunMount::GunMount( )
{
	AddAggregate( &m_Arsenal );
	m_eFiringState = eNotFiring;
	m_nRoundsToFire = 0;
	m_bVisible = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GunMount::GunMount
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

GunMount::~GunMount( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GunMount::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(&(( ObjectCreateStruct* )pData )->m_cProperties ))
					return 0;
			}
			
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;
		
		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		case MID_UPDATE:
		{
            Update( );
		}
		break;

		default : 
			break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::HandleFireMsg
//
//	PURPOSE:	Handle fire message.
//
// ----------------------------------------------------------------------- //

void GunMount::HandleFireMsg( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_Fire( "FIRE" );
	static CParsedMsg::CToken	s_cTok_BeginFire( "BEGINFIRE" );
	static CParsedMsg::CToken	s_cTok_EndFire( "ENDFIRE" );

	if (cMsg.GetArg(0) == s_cTok_Fire)
	{
		// Check if they specified how many rounds to fire.
		if( cMsg.GetArgCount( ) > 1 )
		{
			m_nRoundsToFire = atoi( cMsg.GetArg( 1 ));
		}
		else
		{
			m_nRoundsToFire = 1;
		}

		StartPreFiring( );
		return;
	}
	else if (cMsg.GetArg(0) == s_cTok_BeginFire)
	{
		m_nRoundsToFire = -1;
		StartPreFiring( );
		return;
	}
	else if (cMsg.GetArg(0) == s_cTok_EndFire)
	{
		StartPostFiring( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::HandleVisibleMsg
//
//	PURPOSE:	Handle visible message.
//
// ----------------------------------------------------------------------- //

void GunMount::HandleVisibleMsg( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken	s_cTok_1("1");
	static CParsedMsg::CToken	s_cTok_True("TRUE");
	static CParsedMsg::CToken	s_cTok_Hidden( "HIDDEN" );

	m_bVisible = (( cMsg.GetArg(1) == s_cTok_1) || (cMsg.GetArg(1) == s_cTok_True));
	if( cMsg.GetArg( 0 ) == s_cTok_Hidden )
		m_bVisible = !m_bVisible;

	SetVisible( m_bVisible );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::ReadProp
//
//	PURPOSE:	Reads properties.
//
// ----------------------------------------------------------------------- //

bool GunMount::ReadProp( const GenericPropList *pProps )
{
	m_sWeapon = pProps->GetString( "Weapon", "" );
    m_bVisible = !pProps->GetBool( "Hidden", m_bVisible );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler.
//
// ----------------------------------------------------------------------- //

void GunMount::InitialUpdate( )
{
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES, ( FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES ));

	CreateWeapon( );

	SetVisible( m_bVisible );

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Update
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //

void GunMount::Update( )
{
	switch( m_eFiringState )
	{
		case eNotFiring:
			break;
		case ePreFiring:
			UpdatePreFiring( );
			break;
		case eFiring:
			UpdateFiring( );
			break;
		case ePostFiring:
			UpdatePostFiring( );
			break;
		default:
			ASSERT( !"GunMount::Update:  Invalid firing state." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::AnimationComplete
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::IsAnimationComplete( )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
	{
		ASSERT( !"GunMount::IsAnimationComplete:  Invalid weapon." );
		return true;
	}

	uint32 dwFlags = 0;
	if( g_pModelLT->GetPlaybackState( pWeapon->GetModelObject( ), MAIN_TRACKER, dwFlags ) != LT_OK )
		return true;

	if( dwFlags & MS_PLAYDONE )
		return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartNotFiring
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartNotFiring( )
{
	m_eFiringState = eNotFiring;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartPreFiring
//
//	PURPOSE:	Starts the prefiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartPreFiring( )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the prefire animation.
	HMODELANIM hAnim = pWeapon->GetPreFireAni( );

	// If they don't have a prefire, just go to firing.
	if( hAnim == INVALID_ANI )
	{
		return StartFiring( );
	}

	// Begin prefire animation.
	pWeapon->PlayAnimation( hAnim, true, false );

	SetNextUpdate( UPDATE_NEXT_FRAME );

	m_eFiringState = ePreFiring;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdatePreFiring
//
//	PURPOSE:	Updates the prefiring state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdatePreFiring( )
{
	// Once the prefire is done, go to firing.
	if( IsAnimationComplete( ))
	{
		StartFiring( );
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartFiring
//
//	PURPOSE:	Starts the firing state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartFiring( )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Play the fire animation if they have it.
	HMODELANIM hAnim = pWeapon->GetFireAni( );
	if( hAnim != INVALID_ANI )
	{
		pWeapon->PlayAnimation( hAnim, true, false );
	}

	// Fire the weapon.
	if( !FireWeapon( ))
	{
		return StartNotFiring( );
	}

	m_eFiringState = eFiring;

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdateFiring
//
//	PURPOSE:	Updates the firing state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdateFiring( )
{
	SetNextUpdate( UPDATE_NEXT_FRAME );

	// If we're done, then go to post fire.
	if( m_nRoundsToFire == 0 )
	{
		StartPostFiring( );
		return;
	}

	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
	{
		StartPostFiring( );
		return;
	}

	// Check if we had a fire anim to go off of.
	HMODELANIM hAnim = pWeapon->GetFireAni( );
	if( hAnim != INVALID_ANI )
	{
		// Check if we're still finishing the fire animation.
		if( !IsAnimationComplete( ))
			return;

		// Start it up for the next fire.
		pWeapon->PlayAnimation( hAnim, true, false );
	}

	// Continue to fire weapon.
	if( !FireWeapon( ))
	{
		StartPostFiring( );
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::StartPostFiring
//
//	PURPOSE:	Starts the postfiring state.
//
// ----------------------------------------------------------------------- //
bool GunMount::StartPostFiring( )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the prefire animation.
	HMODELANIM hAnim = pWeapon->GetPostFireAni( );

	// If they don't have a postfire, just go to notfiring.
	if( hAnim == INVALID_ANI )
	{
		return StartNotFiring( );
	}

	// Begin prefire animation.
	pWeapon->PlayAnimation( hAnim, true, false );

	m_eFiringState = ePostFiring;

	SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::UpdatePostFiring
//
//	PURPOSE:	Updates the postfiring state.
//
// ----------------------------------------------------------------------- //

void GunMount::UpdatePostFiring( )
{
	// Once the prefire is done, go to notfiring.
	if( IsAnimationComplete( ))
	{
		StartNotFiring( );
		return;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::CreateWeapon
//
//	PURPOSE:	Creates the weapon
//
// ----------------------------------------------------------------------- //

bool GunMount::CreateWeapon( )
{
	// Create a weapon.
	CActiveWeapon* pActiveWeapon = m_Arsenal.ActivateWeapon( m_sWeapon.c_str( ), NULL );
	if( pActiveWeapon == NULL )
		return false;

	// Make it our current weapon.
	if( !m_Arsenal.ChangeWeapon( pActiveWeapon ))
		return false;

	// Give us plenty of ammo.
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;
	m_Arsenal.AddAmmo( pWeapon->GetAmmoRecord(), 10000 );

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GunMount::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if( !pMsg ) 
		return;

	SAVE_CHARSTRING( m_sWeapon.c_str( ));
	SAVE_DWORD( m_nRoundsToFire );
	SAVE_BYTE( m_eFiringState );
	SAVE_bool( m_bVisible );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GunMount::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if( !pMsg )
		return;

	char szString[256];

	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sWeapon = szString;

	LOAD_DWORD_CAST( m_nRoundsToFire, int32 );
	LOAD_BYTE_CAST( m_eFiringState, FiringState );
	LOAD_bool( m_bVisible );

	// Restart the firing state.
	switch( m_eFiringState )
	{
		case eNotFiring:
		case ePostFiring:
			StartNotFiring( );
			break;
		case ePreFiring:
		case eFiring:
			StartPreFiring( );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::Fire
//
//	PURPOSE:	Fires the weapon.
//
// ----------------------------------------------------------------------- //

bool GunMount::FireWeapon( )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;

	// Get the direction we're firing.
	LTRotation rot;
	g_pLTServer->GetObjectRotation( m_hObject, &rot );
	LTVector vDir = rot.Forward( );

	LTVector vPos;
	if( !GetFirePos( vPos ))
	{
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
	}

	// Tell the weapon to fire.
	WeaponFireInfo weaponFireInfo;
	static uint8 s_nCount = GetRandom( 0, 255 );
	s_nCount++;

	weaponFireInfo.hFiredFrom	= m_hObject;
	weaponFireInfo.hFiringWeapon = pWeapon->GetModelObject( );
	weaponFireInfo.vPath		= vDir;
	weaponFireInfo.vFirePos		= vPos;
	weaponFireInfo.vFlashPos	= vPos;
	weaponFireInfo.nSeed		= (uint8)GetRandom( 2, 255 );
	weaponFireInfo.nPerturbCount	= s_nCount;
	weaponFireInfo.nFireTimestamp = g_pLTServer->GetRealTimeMS( );

	WeaponState eWeaponState = pWeapon->UpdateWeapon( weaponFireInfo, true );
	if( eWeaponState == W_FIRED )
	{
		// Count the round as fired.
		if( m_nRoundsToFire > 0 )
			m_nRoundsToFire--;

	}

	// Keep ammo count up and clip loaded.  The weapon could have refused
	// firing due to weapon fire time limitations, but it still decrements
	// ammo.
	m_Arsenal.AddAmmo( pWeapon->GetAmmoRecord(), 10 );
	pWeapon->ReloadClip( false );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::GetFirePos
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //

bool GunMount::GetFirePos( LTVector& vPos )
{
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( !pWeapon )
		return false;

	HATTACHMENT hAttachment;
    if( g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK )
	{
		return false;
	}

	HMODELSOCKET hSocket;
    if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), "Flash", hSocket) != LT_OK)
	{
		return false;
	}

	LTTransform transform;
	g_pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);
	vPos = transform.m_vPos;
	
	return true;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GunMount::GetFirePos
//
//	PURPOSE:	Changes visibility on gun.
//
// ----------------------------------------------------------------------- //
void GunMount::SetVisible( bool bVisible )
{
	m_bVisible = bVisible;

	CWeapon* pWeapon = m_Arsenal.GetCurWeapon( );
	if( pWeapon )
	{
		g_pCommonLT->SetObjectFlags( pWeapon->GetModelObject( ), OFT_Flags, m_bVisible ? ( FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE ) : FLAG_FORCECLIENTUPDATE, ( FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE ));
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	GunMountPlugin
//              
//	PURPOSE:	WorldEdit Plugin module.
//              
//----------------------------------------------------------------------------


LTRESULT GunMountPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static CArsenalPlugin arsenalPlugin;

	if( arsenalPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT GunMountPlugin::PreHook_Dims(const char* szRezPath,
									  const char* szPropName, 
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims,
										 const char* pszObjName, 
										 ILTPreInterface *pInterface)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Remove the , that is put into some weapon names.
	char szModifiedPropValue[256];
	LTStrCpy( szModifiedPropValue, szPropValue, LTARRAYSIZE(szModifiedPropValue) );
	strtok( szModifiedPropValue, "," );

	HATTRIBUTE hAttrib = NULL;
	HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( szModifiedPropValue );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	HATTRIBUTE hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_RightHandWeapon );

	hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
	const char *pszHHModel = g_pWeaponDB->GetString( hAttrib );
	if( !pszHHModel[0] )
	{
		// Check the left model...
		hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_LeftHandWeapon );

		hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
		pszHHModel = g_pWeaponDB->GetString( hAttrib );
		if( !pszHHModel[0] )
		{
			return LT_UNSUPPORTED;
		}
	}

	LTStrCpy( szModelFilenameBuf, pszHHModel, LTARRAYSIZE(szModifiedPropValue) );

	return LT_OK;
}
