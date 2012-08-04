// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.cpp
//
// PURPOSE : DestructibleModel aggregate
//
// CREATED : 4/23/98
//
// (c) 1998-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DestructibleModel.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "Spawner.h"
#include "ObjectMsgs.h"
#include "Globals.h"
#include "SurfaceDB.h"
#include <stdio.h>
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "Explosion.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "SurfaceFunctions.h"
#include "CollisionsDB.h"
#include "FxDefs.h"					// for SFX_CLIENTFXGROUPINSTANT

extern CAIStimulusMgr* g_pAIStimulusMgr;

//
// Plugin Statics...
//

bool CDestructibleModelPlugin::sm_bInitted = false;


CMDMGR_BEGIN_REGISTER_CLASS( CDestructibleModel )

	ADD_MESSAGE( HIDDEN,	2,	NULL,	MSG_HANDLER( CDestructibleModel, HandleHiddenMsg ), "HIDDEN <1 or 0>", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( CDestructibleModel, CDestructible )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModelPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill in property string list
//
// ----------------------------------------------------------------------- //

CDestructibleModelPlugin::CDestructibleModelPlugin()
{
}

/*virtual*/ CDestructibleModelPlugin::~CDestructibleModelPlugin()
{
}

LTRESULT CDestructibleModelPlugin::PreHook_EditStringList(const char* szRezPath,
	const char* szPropName, char** aszStrings, uint32* pcStrings,
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static CParsedMsg::CToken s_cTok_SurfaceOverride( SURFACE_FLAGS_OVERRIDE_STR );
	static CParsedMsg::CToken s_cTok_CollisionProperty( "CollisionProperty" );
	static CParsedMsg::CToken s_cTok_DestroyedFX( "DestroyedFX" );

	// Tokenize the input for fast searching.
	CParsedMsg::CToken token( szPropName );

	if( token == s_cTok_SurfaceOverride )
	{
		if (CSurfaceDBPlugin::Instance().PopulateStringList(aszStrings, pcStrings,
			cMaxStrings, cMaxStringLength))
		{
			return LT_OK;
		}
	}
	else if( token == s_cTok_CollisionProperty )
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( CollisionProperty ).GetCategory( ), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}
	else if ( token == s_cTok_DestroyedFX )
	{
		return m_FXPlugin.PopulateStringList( szRezPath, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
	}

	return CDestructiblePlugin::PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CDestructibleModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CDestructibleModel::CDestructibleModel()
:	CDestructible			( ),
	m_dwOriginalFlags		( 0 ),
	m_eStimID				( kStimID_Unset ),
	m_fStimRadius			( 0.f ),
	m_nDestroyAlarmLevel	( 0 )
{
	m_DestructibleModelFlags = kDestructibleModelFlag_RemoveOnDeath;
	m_pszDestroyedFXName = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::~CDestructibleModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDestructibleModel::~CDestructibleModel()
{
	// Remove disturbance stimulus.
	if(m_eStimID != kStimID_Unset)
	{
		g_pAIStimulusMgr->RemoveStimulus(m_eStimID);
		m_eStimID = kStimID_Unset;
	}

	delete[] m_pszDestroyedFXName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::~CDestructibleModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SetDestroyedStimulus(float fStimRadius, uint32 nDestroyAlarmLevel)
{
	m_fStimRadius = fStimRadius;
	m_nDestroyAlarmLevel = nDestroyAlarmLevel;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 CDestructibleModel::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(( GenericPropList *) pData );
			}
			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			uint32 nRet = CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
			Save((ILTMessage_Write*)pData, (uint32)fData);
			return nRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			uint32 nRet = CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
			Load((ILTMessage_Read*)pData, (uint32)fData);
			return nRet;
		}
		break;

		default : break;
	}

	return CDestructible::EngineMessageFn(pObject, messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::InitialUpdate
//
//	PURPOSE:	Object's initialupdate.
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::InitialUpdate( const GenericPropList *pProps )
{
	// Set the collisionproperty override.
	char const* pszCollisionProperty = pProps->GetString( "CollisionProperty", "" );
	if( !LTStrEmpty( pszCollisionProperty ))
	{
		HRECORD hRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( CollisionProperty ).GetCategory(), pszCollisionProperty );
		if( hRecord )
		{
			uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hRecord );
			g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );
		}
	}

	char const* pszSurfaceOverride = pProps->GetString( SURFACE_FLAGS_OVERRIDE_STR, "" );
	if( !LTStrEmpty( pszSurfaceOverride ))
	{
		HSURFACE hSurf = g_pSurfaceDB->GetSurface( pszSurfaceOverride );
		SetSurfaceType( hSurf );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::SetSurfaceType
//
//	PURPOSE:	Set the object's surface type...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::SetSurfaceType( HSURFACE hSurfaceOverride )
{
	uint32 dwSurfUsrFlgs = 0;
	SurfaceType eSurfType = ST_UNKNOWN;

	// See if this object is a world model...

	if (GetObjectType(m_hObject) == OT_WORLDMODEL)
	{
		// See if we have a surface override...

		if( hSurfaceOverride && g_pSurfaceDB->GetSurfaceType(hSurfaceOverride) != ST_UNKNOWN)
		{
			eSurfType = g_pSurfaceDB->GetSurfaceType(hSurfaceOverride);
		}
	}

	dwSurfUsrFlgs = SurfaceToUserFlag(eSurfType);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwSurfUsrFlgs, USRFLG_SURFACEMASK);

	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_dwOriginalFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructibleModel::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	uint32 dwRet = CDestructible::ObjectMessageFn(pObject, hSender, pMsg);

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (IsDead() && !( m_DestructibleModelFlags & kDestructibleModelFlag_Destroyed ))
			{
				//read in the damage structure
				DamageStruct ds;
				ds.InitFromMessage(pMsg);

				HandleObjectDestroyed(ds);

				m_DestructibleModelFlags |= kDestructibleModelFlag_Destroyed;
			}
		}
		break;

		default : break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::HandleHiddenMsg
//
//	PURPOSE:	Handle a HIDDEN message...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::HandleHiddenMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_1("1");
	static CParsedMsg::CToken s_cTok_True("TRUE");
	static CParsedMsg::CToken s_cTok_0("0");
	static CParsedMsg::CToken s_cTok_False("FALSE");

	// Game Base will take care of the solid and visible flags for us...

	if( (crParsedMsg.GetArg(1) == s_cTok_1) ||
		(crParsedMsg.GetArg(1) == s_cTok_True) )
	{
		m_DestructibleModelFlags = ( m_DestructibleModelFlags & ~kDestructibleModelFlag_SaveCanDamage ) | ( GetCanDamage() ? kDestructibleModelFlag_SaveCanDamage : 0 );
		m_DestructibleModelFlags = ( m_DestructibleModelFlags & ~kDestructibleModelFlag_SaveNeverDestroy ) | ( GetNeverDestroy() ? kDestructibleModelFlag_SaveNeverDestroy : 0 );

		SetCanDamage(false);
		SetNeverDestroy(true);
	}
	else if((crParsedMsg.GetArg(1) == s_cTok_0) ||
			(crParsedMsg.GetArg(1) == s_cTok_False) )
	{
		SetCanDamage(( m_DestructibleModelFlags & kDestructibleModelFlag_SaveCanDamage ) != 0 );
		SetNeverDestroy(( m_DestructibleModelFlags & kDestructibleModelFlag_SaveNeverDestroy ) != 0 );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::RegisterDestroyedStimulus
//
//	PURPOSE:	Register stimulus for the destroyed model.
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::RegisterDestroyedStimulus()
{
	// Register audio and visual stimulus if specified.

	if( (m_nDestroyAlarmLevel > 0) && (m_fStimRadius > 0.f) )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		// Extract who caused an explosion.

		HOBJECT hDamager = m_hLastDamager;
		if( IsExplosion( hDamager ) )
		{
			Explosion* pExplosion = (Explosion*)g_pLTServer->HandleToObject(m_hLastDamager);
			hDamager = pExplosion->GetFiredFrom();
		}

		// If damager is still not a character (possibly because an explosion was triggered), assume player.
		// Better solution - set the alignment of the damager on the explosion?

		if( !IsCharacter( hDamager ) )
		{
			CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
			if ( pPlayer )
			{
				hDamager = pPlayer->m_hObject;
			}
		}

		if ( IsCharacter(hDamager) )
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hDamager);
			if ( pCharacter )
			{
				// Register the disturbance sound.

				StimulusRecordCreateStruct DisturbanceSoundSCS( kStim_DisturbanceSound, pCharacter->GetAlignment(), vPos, hDamager );
				DisturbanceSoundSCS.m_hStimulusTarget = m_hObject;
				DisturbanceSoundSCS.m_flAlarmScalar = (float)m_nDestroyAlarmLevel;
				DisturbanceSoundSCS.m_flRadiusScalar = m_fStimRadius;
				g_pAIStimulusMgr->RegisterStimulus( DisturbanceSoundSCS );

				// Register the visible disturbance.

				StimulusRecordCreateStruct DisturbanceVisibleSCS( kStim_DisturbanceVisible, pCharacter->GetAlignment(), vPos, hDamager );
				DisturbanceVisibleSCS.m_hStimulusTarget = m_hObject;
				DisturbanceVisibleSCS.m_flAlarmScalar = (float)(m_nDestroyAlarmLevel + 1);
				DisturbanceVisibleSCS.m_flRadiusScalar = m_fStimRadius;
				m_eStimID = g_pAIStimulusMgr->RegisterStimulus( DisturbanceVisibleSCS );
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::ReadProp()
//
//	PURPOSE:	Reads CDestructibleModel properties
//
// --------------------------------------------------------------------------- //

bool CDestructibleModel::ReadProp(const GenericPropList *pProps)
{
	char szString[1024] = "";
	LTStrCpy( szString, pProps->GetString( "DestroyedFX", m_pszDestroyedFXName ? m_pszDestroyedFXName : "" ), LTARRAYSIZE( szString ));
	if( m_pszDestroyedFXName )
	{
		delete[] m_pszDestroyedFXName;
		m_pszDestroyedFXName = NULL;
	}
	if( !LTStrEmpty( szString ))
		m_pszDestroyedFXName = LTStrDup( szString );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!g_pLTServer || !pMsg) return;

	SAVE_BYTE( m_DestructibleModelFlags );
	SAVE_DWORD( m_dwOriginalFlags );
	SAVE_DWORD( m_eStimID );
	SAVE_FLOAT( m_fStimRadius );
	SAVE_DWORD( m_nDestroyAlarmLevel );
	SAVE_CHARSTRING( m_pszDestroyedFXName );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!g_pLTServer || !pMsg) return;

	LOAD_BYTE( m_DestructibleModelFlags );
	LOAD_DWORD( m_dwOriginalFlags );
	LOAD_DWORD_CAST( m_eStimID, EnumAIStimulusID );
	LOAD_FLOAT( m_fStimRadius );
	LOAD_DWORD( m_nDestroyAlarmLevel );
	char szString[2048];
	LOAD_CHARSTRING( szString, LTARRAYSIZE( szString ));
	delete[] m_pszDestroyedFXName;
	m_pszDestroyedFXName = LTStrDup( szString );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::HandleObjectDestroyed
//
//	PURPOSE:	Called when the object has been damaged to the point of being
//				destroyed. This can be overridden by derived classes to apply 
//				any custom destruction behavior that is necessary, but it should
//				call up to this function when it is done with it's operations
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::HandleObjectDestroyed(const DamageStruct& DamageInfo)
{
	RegisterDestroyedStimulus();

	CreateDestroyedFX(DamageInfo);

	if (m_DestructibleModelFlags & kDestructibleModelFlag_RemoveOnDeath)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructibleModel::CreateDestroyedFX
//
//	PURPOSE:	Create any necessary fx based on being destroyed
//
//	NOTE:		We pass in DamageInfo as in the future we may want to use
//				information associated with this to determine what effects to
//				play. For now there is only one effect based on the pos/rot of 
//				the object...
//
// ----------------------------------------------------------------------- //

void CDestructibleModel::CreateDestroyedFX(const DamageStruct& DamageInfo)
{
	// If no destroyed fx is specifed, return...
	if ( LTStrEmpty( m_pszDestroyedFXName ) || !LTStrICmp(m_pszDestroyedFXName, FX_NONE)) return;

	// Use the position/rotation of the object for the fx...
	LTRigidTransform tObjTrans;
	g_pLTServer->GetObjectTransform(m_hObject, &tObjTrans);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
	cMsg.WriteString(m_pszDestroyedFXName);
	cMsg.Writebool( false ); // loop
	cMsg.Writebool( false ); // smooth shutdown
	cMsg.Writebool( false ); // No special parent
	cMsg.WriteLTVector(tObjTrans.m_vPos);
	cMsg.WriteCompLTRotation(tObjTrans.m_rRot);
	cMsg.Writebool( false ); // No target info

	g_pLTServer->SendSFXMessage(cMsg.Read(), 0);
}
