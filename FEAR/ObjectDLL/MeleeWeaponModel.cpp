// ----------------------------------------------------------------------- //
//
// MODULE  : CMeleeWeaponModel.cpp
//
// PURPOSE : CMeleeWeaponModel implementation
//
// CREATED : 10/19/04
//
// (c) 2004-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "MeleeWeaponModel.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "Weapon.h"


LINKFROM_MODULE( MeleeWeaponModel );


BEGIN_CLASS(CMeleeWeaponModel)
END_CLASS_FLAGS(CMeleeWeaponModel, GameBase, CF_HIDDEN, "A hidden weapon model used for rigid body melee collision detection.")


CMDMGR_BEGIN_REGISTER_CLASS( CMeleeWeaponModel )
CMDMGR_END_REGISTER_CLASS( CMeleeWeaponModel, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMeleeWeaponModel::CMeleeWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CMeleeWeaponModel::CMeleeWeaponModel() : GameBase(OT_MODEL)
{
	m_pParent = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMeleeWeaponModel::~CMeleeWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMeleeWeaponModel::~CMeleeWeaponModel()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMeleeWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CMeleeWeaponModel::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = FLAG_NOTINWORLDTREE;
				pStruct->m_eGroup = ePhysicsGroup_NonSolid;
			}
			break;
		}

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMeleeWeaponModel::Setup()
//
//	PURPOSE:	Setup our weapon data.
//
// ----------------------------------------------------------------------- //

void CMeleeWeaponModel::Setup( CWeapon* pParent, HWEAPON hWeapon, bool bBlocking )
{
	m_pParent = pParent;
	bool bUseAIData = pParent ? IsAI( pParent->GetObject() ) : !USE_AI_DATA;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, bUseAIData );
	HATTRIBUTE hAttrib = g_pWeaponDB->GetAttribute( hWpnData, bBlocking ? WDB_WEAPON_sBlockModel : WDB_WEAPON_sMeleeModel );
	const char *pszMeleeModel = g_pWeaponDB->GetString( hAttrib );
	if( pszMeleeModel[0] )
	{
		ObjectCreateStruct ocs;
		ocs.SetFileName( pszMeleeModel );
		g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMeleeWeaponModel::GetOwner()
//
//	PURPOSE:	Get the handle for the dude that's weilding this melee weapon thing.
//
// ----------------------------------------------------------------------- //

HOBJECT CMeleeWeaponModel::GetOwner() const
{
	if (!m_pParent)
		return NULL;

	if (!m_pParent->GetArsenal())
		return NULL;

	return m_pParent->GetArsenal()->GetObject();
}

