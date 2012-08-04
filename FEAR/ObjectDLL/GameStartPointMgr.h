// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPointMgr.h
//
// PURPOSE : Manages gamestartpoint functions.
//
// CREATED : 03/03/06
//
// (c) 1997-2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAMESTARTPOINTMGR_H__
#define __GAMESTARTPOINTMGR_H__

#include "ltobjref.h"

class GameStartPoint;

class GameStartPointMgr : public ILTObjRefReceiver
{
public:
	DECLARE_SINGLETON( GameStartPointMgr );

public:
	// Find a good start point.
	GameStartPoint* GameStartPointMgr::FindStartPoint(CPlayerObj* pPlayer);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	GravitySource
	//
	//	PURPOSE:	Contains information about a gravity source for spawning.
	//
	// ----------------------------------------------------------------------- //
	struct GravitySource
	{
		GravitySource( )
		{
			m_fBuddyModifier = 1.0f;
			m_fEnemyModifier = 1.0f;
			m_eTeamId = INVALID_TEAM;
		}

		// The object that is the gravity source.
		LTObjRefNotifierUnsafe m_hObject;
		// The gravity modifier to apply when source is a buddy.
		float m_fBuddyModifier;
		// The gravity modifier to apply when source is an enemy.
		float m_fEnemyModifier;
		// Team this source is on.
		ETeamId m_eTeamId;
	};

	// Defines the list of managed gravitysources contained by object.
	typedef std::vector< GravitySource > TManagedGravitySourceList;
	TManagedGravitySourceList& GetMangedGravitySourceList( ) { return m_lstManagedGravitySources; }

	// Adds a gravity source to managed list.
	void AddManagedGravitySource( GravitySource const& gravitySource );
	// Removes gravity source based on object.
	void RemoveManagedGravitySource( HOBJECT hObject );
	// Gets the gravity source entry.
	GravitySource* GetManagedGravitySource( HOBJECT hObject );
    
	// Implementing classes will have this function called
	// when HOBJECT ref points to gets deleted.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

private:

	typedef std::vector< GravitySource > TManagedGravitySourceList;

	// Finds the iterator to a gravity source.
	TManagedGravitySourceList::iterator FindGravitySource( HOBJECT hObject );

	// List of managed gravitysources.
	TManagedGravitySourceList m_lstManagedGravitySources;

};

#endif  // __GAMESTARTPOINTMGR_H__
