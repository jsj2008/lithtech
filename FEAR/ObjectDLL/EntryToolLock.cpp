// ----------------------------------------------------------------------- //
//
// MODULE  : EntryToolLock.h
//
// PURPOSE : World model entry tool lock object
//
// CREATED : 12/10/03
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "EntryToolLock.h"

// ----------------------------------------------------------------------- //

LINKFROM_MODULE( EntryToolLock );

// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_EntryToolLock 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_EntryToolLock CF_HIDDEN

#endif

BEGIN_CLASS( EntryToolLock )

	ADD_BOOLPROP_FLAG(Position, true, 0, "If true, the player will be moved into the proper position before playing the animation.")
	ADD_STRINGPROP_FLAG(EntryTool, "None", PF_STATICLIST, "Entry Tool required to open this lock.")

	ADD_STRINGPROP_FLAG(Animation, "", PF_HIDDEN, "")
	ADD_STRINGPROP_FLAG(ActivationType, "", PF_HIDDEN, "")

END_CLASS_FLAGS_PLUGIN( EntryToolLock, SpecialMove, CF_WORLDMODEL | CF_HIDDEN_EntryToolLock, CEntryToolLockPlugin, "This class defines an object that will cause the player to need to use an entry tool to unlock." )

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( EntryToolLock )

CMDMGR_END_REGISTER_CLASS( EntryToolLock, SpecialMove )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::EntryToolLock()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EntryToolLock::EntryToolLock() : SpecialMove()
{
	m_rEntryTool = NULL;
	m_bPosition = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::~EntryToolLock()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

EntryToolLock::~EntryToolLock()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::OnReleased()
//
//	PURPOSE:	Automatically turn off when animation finishes.
//
// ----------------------------------------------------------------------- //

void EntryToolLock::OnReleased()
{
	SpecialMove::OnReleased();
	SetEnabled(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::ReadProp()
//
//	PURPOSE:	Read properties from object data
//
// ----------------------------------------------------------------------- //

void EntryToolLock::ReadProp( const GenericPropList *pProps )
{
	SpecialMove::ReadProp(pProps);
	if( !pProps ) return;

	m_bPosition = pProps->GetBool( "Position", m_bPosition );

	const char* pszEntryTool = pProps->GetString( "EntryTool", "" );
	if( pszEntryTool && pszEntryTool[0] )
	{
		m_rEntryTool = g_pWeaponDB->GetWeaponRecord(pszEntryTool);
	}

	// parent overrides...
	m_eAnimation = kAP_ACT_Fire;
	m_ActivateTypeHandler.SetActivateType(g_pWeaponDB->GetActionIcon(m_rEntryTool));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::WriteSFXMsg()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void EntryToolLock::WriteSFXMsg(CAutoMessage& cMsg)
{
	SpecialMove::WriteSFXMsg(cMsg);
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_rEntryTool );
	cMsg.Writebool( m_bPosition );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void EntryToolLock::Save( ILTMessage_Write* pMsg, uint32 nFlags )
{
	SpecialMove::Save(pMsg, nFlags);
	if( !pMsg ) return;
	SAVE_HRECORD( m_rEntryTool );
	SAVE_BOOL( m_bPosition );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EntryToolLock::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void EntryToolLock::Load( ILTMessage_Read* pMsg, uint32 nFlags )
{
	SpecialMove::Load(pMsg, nFlags);
	if( !pMsg ) return;
	LOAD_HRECORD( m_rEntryTool, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_BOOL( m_bPosition );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEntryToolLockPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CEntryToolLockPlugin::PreHook_EditStringList(	const char* szRezPath,
													   const char* szPropName,
													   char** aszStrings,
													   uint32* pcStrings,
													   const uint32 cMaxStrings,
													   const uint32 cMaxStringLength)
{
	// EntryTool.

	if ( LTStrIEquals( "EntryTool", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "<none>" );

		uint32 nTools = g_pWeaponDB->GetNumWeapons();
		for (uint32 i=0; i<nTools; i++)
		{
			HRECORD hTool = g_pWeaponDB->GetWeaponRecord(i);	//!!ARL: Filter out non-entry tools
			strcpy( aszStrings[(*pcStrings)++], g_pLTDatabase->GetRecordName(hTool) );
		}
#if 0
		// Alphabetize the strings, skipping the 'None' entry which is always first.
		if (*pcStrings > 1)
		{
			qsort( aszStrings+1, (*pcStrings)-1, sizeof( char * ), CaseInsensitiveCompare );		
		}
#endif
		return LT_OK;
	}

	// Unsupported.

	return LT_UNSUPPORTED;
};

