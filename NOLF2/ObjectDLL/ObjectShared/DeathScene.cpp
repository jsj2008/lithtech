
//----------------------------------------------------------------------------
//              
//	MODULE:		DeathScene.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	07.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __DEATHSCENE_H__
#include "DeathScene.h"		
#endif

#include "Body.h"
#include "ObjectRelationMgr.h"

// Forward declarations

// Globals

// Statics

CDeathScene::CDeathScene()
{
	m_eCharacterDeath = CD_NORMAL;
	m_pObjectRelationMgr = debug_new(CObjectRelationMgr);
	m_pObjectRelationMgr->ClearRelationSystem();

    m_hSceneObject = LTNULL;

	m_fNoiseVolume = 0.0f;
	m_fNoiseTime = 0.0f;

	m_fLastPainTime		= -(float)(INT_MAX);
	m_fLastPainVolume	= 0.0f;

	m_bWasPlayer = false;
}

CDeathScene::~CDeathScene()
{
	if ( m_pObjectRelationMgr != NULL )
	{
		debug_delete( m_pObjectRelationMgr );
		m_pObjectRelationMgr = NULL;
	}
}

LTFLOAT CDeathScene::GetNoiseVolume()
{
	return m_fNoiseVolume;
}

LTFLOAT CDeathScene::GetNoiseTime()
{
	return m_fNoiseTime;
}

LTFLOAT CDeathScene::GetLastPainVolume()
{
	return m_fLastPainVolume;
}

LTFLOAT CDeathScene::GetLastPainTime()
{
	return m_fLastPainTime;
}

LTVector CDeathScene::GetPosition()
{
   LTVector vPos;
   g_pLTServer->GetObjectPos(m_hSceneObject, &vPos);
   return vPos;
}

HOBJECT CDeathScene::GetObject()
{
	return m_hSceneObject;
}

LTBOOL CDeathScene::WasPlayer()
{
	return m_bWasPlayer;
}

CObjectRelationMgr& GetObjectRelationMgr();

const RelationData& CDeathScene::GetRelationData()
{
	return m_pObjectRelationMgr->GetData();
}

CharacterDeath CDeathScene::GetCharacterDeath()
{
	return m_eCharacterDeath;
}

void CDeathScene::Save(ILTMessage_Write *pMsg)
{
    if ( !pMsg || !g_pLTServer ) return;

	SAVE_DWORD(m_eCharacterDeath);
	SAVE_HOBJECT(m_hSceneObject);
	SAVE_FLOAT(m_fNoiseVolume);
    SAVE_TIME(m_fNoiseTime);
    SAVE_TIME(m_fLastPainTime);
    SAVE_FLOAT(m_fLastPainVolume);
	SAVE_BOOL(m_bWasPlayer);
	
	m_pObjectRelationMgr->Save(pMsg);
}

void CDeathScene::Load(ILTMessage_Read *pMsg)
{
    if ( !pMsg || !g_pLTServer ) return;

    LOAD_DWORD_CAST(m_eCharacterDeath, CharacterDeath);
	LOAD_HOBJECT(m_hSceneObject);
    LOAD_FLOAT(m_fNoiseVolume);
    LOAD_TIME(m_fNoiseTime);
    LOAD_TIME(m_fLastPainTime);
    LOAD_FLOAT(m_fLastPainVolume);
	LOAD_BOOL(m_bWasPlayer);

	m_pObjectRelationMgr->Load(pMsg);
}

void CDeathScene::Set(CCharacter *pCharacter, Body *pBody)
{
	_ASSERT(pCharacter && pBody);
	if ( !pCharacter || !pBody ) return;

	if ( IsPlayer( pCharacter->m_hObject ) )
	{
		m_bWasPlayer = true;
	}

	m_eCharacterDeath = pCharacter->GetDeathType();

	// Copy the data describing the Character to the Body.
	UBER_ASSERT( pCharacter->GetRelationMgr() != NULL,
		"CDeathScene::Set Characters relation is NULL" );
	m_pObjectRelationMgr->SetData() = pCharacter->GetRelationMgr()->GetData();

	m_hSceneObject = pBody->m_hObject;
}


void CDeathScene::SetNoise(LTFLOAT fVolume, LTFLOAT fTime)
{
	m_fNoiseVolume = fVolume;
	m_fNoiseTime = fTime;
}

void CDeathScene::SetPain(LTFLOAT fVolume, LTFLOAT fTime)
{
	m_fLastPainTime = fTime;
	m_fLastPainVolume = fVolume;
}
